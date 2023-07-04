//+++2003-11-18
//    Copyright (C) 2001,2002,2003  Mike Rieker, Beverly, MA USA
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
//---2003-11-18

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "elf.h"

typedef struct { int value; char *string; } Typestring;

static Typestring shtypestrings[] = { SHT_NULL, "null", 
                                  SHT_PROGBITS, "progbits", 
                                    SHT_SYMTAB, "symtab", 
                                    SHT_STRTAB, "strtab", 
                                      SHT_RELA, "rela", 
                                      SHT_HASH, "hash", 
                                   SHT_DYNAMIC, "dynamic", 
                                      SHT_NOTE, "note", 
                                    SHT_NOBITS, "nobits", 
                                       SHT_REL, "rel", 
                                     SHT_SHLIB, "shlib", 
                                    SHT_DYNSYM, "dynsym", 
                                       SHT_NUM, "num", 
                                             0, NULL };

#define PTRFMT "%8.8X%8.8X"
#define PTRSWP(p) (unsigned int)((p) >> 32),(unsigned int)(p)

static char *pn = "elf64dump";

static char *namestring (unsigned char *objbuf, Elf64_Shdr *strshtab, int offset);
static char *typestring (Typestring *typestrings, int typevalue);

int main (int argc, char *argv[])

{
  char *objname;
  int i, j, k, objfd, rc;
  struct stat objstat;
  unsigned char *objbuf;
  unsigned int objsiz;

  Elf64_Ehdr *elfhdrpnt;
  Elf64_Rel  *elfrel;
  Elf64_Rela *elfrela;
  Elf64_Shdr *elfshpnt, *elfshpnt0;
  Elf64_Sym  *elfsym;

  if (argc > 0) pn = argv[0];

  if (argc != 2) {
    fprintf (stderr, "usage: %s <elf64objfile>\n", pn);
    return (-1);
  }
  objname = argv[1];

  /* Read file into memory */

  objfd = open (objname, O_RDONLY);
  if (objfd < 0) {
    fprintf (stderr, "%s: error opening input object file %s: %s\n", pn, objname, strerror (errno));
    return (15);
  }
  if (fstat (objfd, &objstat) < 0) {
    fprintf (stderr, "%s: error statting input object file %s: %s\n", pn, objname, strerror (errno));
    return (16);
  }
  objsiz = objstat.st_size;
  objbuf = malloc (objsiz);
  rc = read (objfd, objbuf, objsiz);
  if (rc < 0) {
    fprintf (stderr, "%s: error reading input object file %s: %s\n", pn, objname, strerror (errno));
    return (17);
  }
  if (rc != objsiz) {
    fprintf (stderr, "%s: only read %d of %u bytes from input object file %s: %s\n", pn, rc, objsiz, objname);
    return (18);
  }
  close (objfd);

  elfhdrpnt = (void *)objbuf;
  if ((elfhdrpnt -> e_ident[0] != 127) 
   || (elfhdrpnt -> e_ident[1] != 'E') 
   || (elfhdrpnt -> e_ident[2] != 'L') 
   || (elfhdrpnt -> e_ident[3] != 'F') 
   || (elfhdrpnt -> e_type != ET_REL)) {
    fprintf (stderr, "%s: input file %s is not an elf relocatable\n", pn, objname);
    return (19);
  }

  printf ("Header:\n");
  printf ("    e_entry: " PTRFMT "\n", PTRSWP (elfhdrpnt -> e_entry));
  printf ("    e_shoff: " PTRFMT "\n", PTRSWP (elfhdrpnt -> e_shoff));
  printf ("  e_machine: %8.8X\n", elfhdrpnt -> e_machine);

  elfshpnt0 = (void *)(objbuf + elfhdrpnt -> e_shoff);
  for (i = 0; i < elfhdrpnt -> e_shnum; i ++) {
    elfshpnt = elfshpnt0 + i;
    printf ("\n");
    printf ("    Section %d:\n", i);
    printf ("        sh_name %s\n", "??");//namestring (objbuf, ??, elfshpnt -> sh_name));
    printf ("        sh_type %d (%s)\n", elfshpnt -> sh_type, typestring (shtypestrings, elfshpnt -> sh_type));
    printf ("       sh_flags " PTRFMT "\n", PTRSWP (elfshpnt -> sh_flags));
    printf ("        sh_addr " PTRFMT "\n", PTRSWP (elfshpnt -> sh_addr));
    printf ("      sh_offset " PTRFMT "\n", PTRSWP (elfshpnt -> sh_offset));
    printf ("        sh_size " PTRFMT "\n", PTRSWP (elfshpnt -> sh_size));
    switch (elfshpnt -> sh_type) {
      case SHT_SYMTAB:
      case SHT_DYNSYM: {
        k = elfshpnt -> sh_link;
        if ((k < elfhdrpnt -> e_shnum) && (elfshpnt0[k].sh_type == SHT_STRTAB)) {
          for (j = 0; j < elfshpnt -> sh_size / elfshpnt -> sh_entsize; j ++) {
            elfsym = (void *)(objbuf + elfshpnt -> sh_offset + j * elfshpnt -> sh_entsize);
            printf ("      %5u  %32s  %2.2X  %5u  " PTRFMT "  " PTRFMT "\n", 
		j, namestring (objbuf, elfshpnt0 + k, elfsym -> st_name), 
		elfsym -> st_info, elfsym -> st_shndx, PTRSWP (elfsym -> st_value), PTRSWP (elfsym -> st_size));
          }
        }
        break;
      }
      case SHT_STRTAB: {
        for (j = 0; j < elfshpnt -> sh_size; j += k + 1) {
          k = strlen (objbuf + elfshpnt -> sh_offset + j);
          printf ("      %5u  %s\n", j, objbuf + elfshpnt -> sh_offset + j);
        }
        break;
      }
      case SHT_RELA: {
        printf ("      Symbol table section %u, data section %u\n", elfshpnt -> sh_link, elfshpnt -> sh_info);
        printf ("        %16s  %16s  %16s\n", "offset", "info", "addend");
        for (j = 0; j < elfshpnt -> sh_size / elfshpnt -> sh_entsize; j ++) {
          elfrela = (void *)(objbuf + elfshpnt -> sh_offset + j * elfshpnt -> sh_entsize);
          printf ("        " PTRFMT "  " PTRFMT "  " PTRFMT "\n", 
		PTRSWP (elfrela -> r_offset), PTRSWP (elfrela -> r_info), PTRSWP (elfrela -> r_addend));
        }
        break;
      }
      case SHT_DYNAMIC: {
        break;
      }
      case SHT_REL: {
        break;
      }
    }
  }

  return (0);
}

static char *namestring (unsigned char *objbuf, Elf64_Shdr *strshtab, int offset)

{
  if (offset >= strshtab -> sh_size) return ("");
  return (objbuf + strshtab -> sh_offset + offset);
}

static char *typestring (Typestring *typestrings, int typevalue)

{
  int i;

  for (i = 0; typestrings[i].string != NULL; i ++) if (typestrings[i].value == typevalue) return (typestrings[i].string);
  return ("");
}
