//+++2002-08-17
//    Copyright (C) 2001,2002  Mike Rieker, Beverly, MA USA
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; version 2 of the License.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//---2002-08-17

/************************************************************************/
/*									*/
/*  Dump out info about an oz image					*/
/*									*/
/************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ozone.h"
#include "oz_image.h"

#define L2PAGESIZE 12

typedef struct Dump Dump;
struct Dump { Dump *next;
              uLong size;
              uLong addr;
            };

static int readat (int fd, uLong offset, uLong size, void *buff);

int main (int argc, char *argv[])

{
  uByte dumpbuff[32];
  char *fn, *ozgblstr, *ozrefstr, *ozundstr, *p;
  Dump *dump, *dumps, **ldump;
  int fd, i, j;
  int flag_g, flag_h, flag_r, flag_s, flag_u, flag_x;
  OZ_Image_Globl *ozgbldes;
  OZ_Image_Headr ozimghdr;
  OZ_Image_Refim *ozrefdes;
  OZ_Image_Reloc *ozreldes;
  OZ_Image_Sectn *ozsecdes;
  OZ_Image_Undef *ozunddes;

  fn = NULL;
  flag_g = 0;
  flag_h = 0;
  flag_r = 0;
  flag_s = 0;
  flag_u = 0;
  flag_x = 0;

  ldump = &dumps;

  for (i = 1; i < argc; i ++) {
    if (argv[i][0] != '-') {
      fn = argv[i];
      continue;
    }
    switch (argv[i][1]) {
      case 'd': {
        if (i + 2 >= argc) {
          fprintf (stderr, "missing size addr from -d option\n");
          return (-1);
        }
        dump = malloc (sizeof *dump);
        dump -> size = strtoul (argv[i+1], &p, 16);
        if (*p != 0) {
          fprintf (stderr, "bad dump size %s\n", argv[i+1]);
          return (-1);
        }
        dump -> addr = strtoul (argv[i+2], &p, 16);
        if (*p != 0) {
          fprintf (stderr, "bad dump addr%s\n", argv[i+2]);
          return (-1);
        }
        *ldump = dump;
        ldump  = &(dump -> next);
        i += 2;
        break;
      }
      case 'g': flag_g = 1; break;	/* global symbols */
      case 'h': flag_h = 1; break;	/* header */
      case 'r': flag_r = 1; break;	/* referenced shareables */
      case 's': flag_s = 1; break;	/* sections */
      case 'u': flag_u = 1; break;	/* undefined symbols */
      case 'x': flag_x = 1; break;	/* relocations */
      default: {
        fprintf (stderr, "unknown option %s\n", argv[i]);
        return (-1);
      }
    }
  }
  *ldump = NULL;

  if (fn == NULL) {
    fprintf (stderr, "usage: dumpozimage [-d size addr] [-g] [-h] [-r] [-s] [-u] [-x] <imagefile>\n");
    return (-1);
  }

  fd = open (fn, O_RDONLY);
  if (fd < 0) {
    fprintf (stderr, "error opening %s: %s\n", fn, strerror (errno));
    return (-1);
  }

  if (!readat (fd, 0, sizeof ozimghdr, &ozimghdr)) return (-1);

  if (memcmp (ozimghdr.magic, "oz_image", sizeof ozimghdr.magic) != 0) {
    fprintf (stderr, "bad magic string\n");
    return (-1);
  }

  if (ozimghdr.fxhdrsize != sizeof ozimghdr) {
    fprintf (stderr, "ozimghdr.fxhdrsize %d, sizeof ozimghdr %d\n", ozimghdr.fxhdrsize, sizeof ozimghdr);
    if (ozimghdr.fxhdrsize < sizeof ozimghdr) memset (((uByte *)&ozimghdr) + ozimghdr.fxhdrsize, 0, sizeof ozimghdr - ozimghdr.fxhdrsize);
  }

  if (flag_h) {
    printf ("Header:\n");
    printf ("  basevpage 0x%x\n", ozimghdr.basevpage);
    printf ("  startaddr 0x%x\n", ozimghdr.startaddr);
    printf ("  imagesize 0x%x\n", ozimghdr.imagesize);
    printf ("  imageoffs 0x%x\n", ozimghdr.imageoffs);
    printf ("\n");
  }

  if (flag_r && (ozimghdr.refdesnum != 0)) {
    printf ("\nReferenced shareables:\n");
    ozrefdes = malloc (ozimghdr.refdesnum * sizeof *ozrefdes);
    ozrefstr = malloc (ozimghdr.refstrsiz);
    if (!readat (fd, ozimghdr.refdesoff, ozimghdr.refdesnum * sizeof *ozrefdes, ozrefdes)) return (-1);
    if (!readat (fd, ozimghdr.refstroff, ozimghdr.refstrsiz, ozrefstr)) return (-1);
    for (i = 0; i < ozimghdr.refdesnum; i ++) {
      printf ("  [%2d]  %s\n", i, ozrefstr + ozrefdes[i].refim_namof);
    }
  }

  if (flag_g && (ozimghdr.gbldesnum != 0)) {
    printf ("\nGlobal symbols:\n");
    ozgbldes = malloc (ozimghdr.gbldesnum * sizeof *ozgbldes);
    ozgblstr = malloc (ozimghdr.gblstrsiz);
    if (!readat (fd, ozimghdr.gbldesoff, ozimghdr.gbldesnum * sizeof *ozgbldes, ozgbldes)) return (-1);
    if (!readat (fd, ozimghdr.gblstroff, ozimghdr.gblstrsiz, ozgblstr)) return (-1);
    for (i = 0; i < ozimghdr.gbldesnum; i ++) {
      printf ("  0x%8.8x  %c%c %s\n", ozgbldes[i].globl_value, 
	(ozgbldes[i].globl_flags & OZ_IMAGE_GLOBL_FLAG_RELOC) ? '+' : ' ', 
	(ozgbldes[i].globl_flags & OZ_IMAGE_GLOBL_FLAG_ENTRY) ? '@' : ' ', 
	ozgblstr + ozgbldes[i].globl_namof);
    }
  }

  if (flag_u && (ozimghdr.unddesnum != 0)) {
    printf ("\nUndefined symbols:\n");
    ozunddes = malloc (ozimghdr.unddesnum * sizeof *ozunddes);
    ozundstr = malloc (ozimghdr.undstrsiz);
    if (!readat (fd, ozimghdr.unddesoff, ozimghdr.unddesnum * sizeof *ozunddes, ozunddes)) return (-1);
    if (!readat (fd, ozimghdr.undstroff, ozimghdr.undstrsiz, ozundstr)) return (-1);
    for (i = 0; i < ozimghdr.unddesnum; i ++) {
      printf ("  %3d)  %s+[%2d]\n", i, ozundstr + ozunddes[i].undef_namof, ozunddes[i].undef_refim);
    }
  }

  if (flag_x && (ozimghdr.reldesnum != 0)) {
    printf ("\nRelocations:\n");
    if (!flag_u) {
      ozunddes = malloc (ozimghdr.unddesnum * sizeof *ozunddes);
      ozundstr = malloc (ozimghdr.undstrsiz);
      if (!readat (fd, ozimghdr.unddesoff, ozimghdr.unddesnum * sizeof *ozunddes, ozunddes)) return (-1);
      if (!readat (fd, ozimghdr.undstroff, ozimghdr.undstrsiz, ozundstr)) return (-1);
    }
    ozreldes = malloc (ozimghdr.reldesnum * sizeof *ozreldes);
    if (!readat (fd, ozimghdr.reldesoff, ozimghdr.reldesnum * sizeof *ozreldes, ozreldes)) return (-1);
    for (i = 0; i < ozimghdr.reldesnum; i ++) {
      switch (ozreldes[i].reloc_type) {
        case OZ_IMAGE_RELOC_IMGBASE: {
          printf ("  [%5u] 0x%8.8x: .long x+baseaddr\n", i, ozreldes[i].reloc_vaddr);
          break;
        }
        case OZ_IMAGE_RELOC_IMGBASE_NEGPC: {
          printf ("  [%5u] 0x%8.8x: .long x+baseaddr-.\n", i, ozreldes[i].reloc_vaddr);
          break;
        }
        case OZ_IMAGE_RELOC_UNDEF: {
          printf ("  [%5u] 0x%8.8x: .long x+%s\n", i, ozreldes[i].reloc_vaddr, ozundstr + ozunddes[ozreldes[i].reloc_indx].undef_namof);
          break;
        }
        case OZ_IMAGE_RELOC_UNDEF_NEGPC: {
          printf ("  [%5u] 0x%8.8x: .long x+%s-.\n", i, ozreldes[i].reloc_vaddr, ozundstr + ozunddes[ozreldes[i].reloc_indx].undef_namof);
          break;
        }
        default: {
          printf ("  [%5u] 0x%8.8x:  unknown type %d\n", i, ozreldes[i].reloc_vaddr, ozreldes[i].reloc_type);
          break;
        }
      }
    }
  }

  if (flag_s && (ozimghdr.secdesnum != 0)) {
    printf ("\nSections:\n");
    ozsecdes = malloc (ozimghdr.secdesnum * sizeof *ozsecdes);
    if (!readat (fd, ozimghdr.secdesoff, ozimghdr.secdesnum * sizeof *ozsecdes, ozsecdes)) return (-1);
    for (i = 0; i < ozimghdr.secdesnum; i ++) {
      printf ("  0x%5.5x pages at 0x%5.5x  %s\n", ozsecdes[i].sectn_pages, ozsecdes[i].sectn_vpage, ozsecdes[i].sectn_write ? "rw" : "ro");
    }
  }

  if (dumps != NULL) {
    printf ("\nDumps:\n");
    if (!flag_s) {
      ozsecdes = malloc (ozimghdr.secdesnum * sizeof *ozsecdes);
      if (!readat (fd, ozimghdr.secdesoff, ozimghdr.secdesnum * sizeof *ozsecdes, ozsecdes)) return (-1);
    }
    for (dump = dumps; dump != NULL; dump = dump -> next) {
      printf ("\n");
      for (i = 0; i < dump -> size; i += sizeof dumpbuff) {
        if (!readat (fd, ozimghdr.imageoffs + dump -> addr + i - (ozimghdr.basevpage << L2PAGESIZE), sizeof dumpbuff, dumpbuff)) break;
        printf ("  ");
        for (j = sizeof dumpbuff; -- j >= 0;) {
          if ((j & 3) == 3) printf (" ");
          if (i + j >= dump -> size) printf ("  ");
          else printf ("%2.2x", dumpbuff[j]);
        }
        printf (" : %8.8x : '", dump -> addr + i);
        for (j = 0; (j < sizeof dumpbuff) && (i + j < dump -> size); j ++) {
          dumpbuff[j] &= 127;
          if ((dumpbuff[j] < ' ') || (dumpbuff[j] == 127)) dumpbuff[j] = '.';
          putchar (dumpbuff[j]);
        }
        printf ("\n");
      }
    }
  }

  return (0);
}

static int readat (int fd, uLong offset, uLong size, void *buff)

{
  int rc;

  if (lseek (fd, offset, SEEK_SET) < 0) {
    fprintf (stderr, "error positioning to offset %u: %s\n", offset, strerror (errno));
    return (0);
  }

  rc = read (fd, buff, size);
  if (rc < 0) {
    fprintf (stderr, "error reading at offset %u: %s\n", offset, strerror (errno));
    return (0);
  }

  if (rc < size) {
    fprintf (stderr, "only read %d out of %d bytes at offset %u\n", rc, size, offset);
    return (0);
  }

  return (1);
}
