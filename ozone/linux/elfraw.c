//+++2001-10-06
//    Copyright (C) 2001, Mike Rieker, Beverly, MA USA
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
//---2001-10-06

/************************************************************************/
/*									*/
/*  Get the raw contents of an elf file					*/
/*  Elf object must have been linked with -d and -r			*/
/*									*/
/*	elfraw <base_addr> <raw_image> <elf_object>			*/
/*									*/
/************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../sources/elf.h"

#include "oz_image.h"

#define MEMORY_PAGE_SIZE (4096)

typedef struct Global  Global;
typedef struct Reloc   Reloc;
typedef struct Section Section;
typedef struct Undef   Undef;

struct Global { Global *next;			/* next in globals list */
                int index;			/* index in elf sh_... struct list */
                Section *section;		/* NULL : value is absolute, else : value relative to start of section */
                OZ_Pointer value;		/* symbol's value */
                char *name;			/* symbol's name (points to name in objbuf) */
              };

typedef struct { enum { INDEX_NULL, INDEX_GLOBAL, INDEX_SECTION } type;
                 void *pntr;
               } Index;

struct Reloc { Reloc *next;			/* next in relocs list */
               Section *section;		/* section that relocation is done in */
               OZ_Image_Reloc imrel;		/* output image format relocation info */
             };

struct Section { Section *next;			/* next in sections list */
                 int index;			/* index in the elf file of the sh_... struct */
                 OZ_Pointer vaddr;		/* virtual address to map the section at */
                 uLong fileoffs;		/* offset in objbuf of data */
                 uLong size;			/* number of bytes in section */
                 int writable;			/* section is writable */
                 int nobits;			/* demand-zero section */
               };

struct Undef { Undef *next;			/* next in undefs list */
               int index;			/* index in elf sh_... struct list */
               char *name;			/* symbol's name (points to name in objbuf) */
             };

static Global  *globals  = NULL;
static Reloc   *relocs   = NULL;
static Section *sections = NULL;
static Undef   *undefs   = NULL;

static void writeat (int outfd, int pos, int size, void *buff);
static int global_compare (const void *v1, const void *v2);

int main (int argc, char *argv[])

{
  char *elfsymnam;
  char *p, zeroes[MEMORY_PAGE_SIZE];
  uByte *objbuf;
  Elf32_Ehdr *elfhdrpnt;
  Elf32_Rel *elfrel;
  Elf32_Shdr *elfshpnt;
  Elf32_Sym *elfsym;
  Global *global, **globall;
  Index *indexl;
  int errorflag, highestindex, i, j, k, objfd, objsiz, outfd, rc, shrfd;
  uLong sectoffs, *valuepnt;
  OZ_Mempage pages, vpage;
  OZ_Pointer baseaddr, nextaddr;
  Reloc *reloc;
  Section **lsection, *section;
  struct stat objstat;
  Undef *undef;

  if (argc != 4) {
    fprintf (stderr, "usage: elfraw <baseaddr> <output_raw_image_file> <input_elf_reloc_file> \n");
    return (-1);
  }

  /* Decode base address */

  baseaddr = strtoul (argv[1], &p, 16);
  if (*p != 0) {
    fprintf (stderr, "bad base address %s\n", argv[1]);
    return (-1);
  }

  /* Read elf object file into memory */

  objfd = open (argv[3], O_RDONLY);
  if (objfd < 0) {
    perror ("error opening input object file");
    return (-1);
  }
  if (fstat (objfd, &objstat) < 0) {
    perror ("error statting input object file");
    return (-1);
  }
  objsiz = objstat.st_size;
  objbuf = malloc (objsiz);
  rc = read (objfd, objbuf, objsiz);
  if (rc < 0) {
    perror ("error reading input object file");
    return (-1);
  }
  if (rc < objsiz) {
    fprintf (stderr, "only read %d of %u bytes from input object file\n", rc, objsiz);
    return (-1);
  }
  close (objfd);

  elfhdrpnt = (void *)objbuf;
  if ((elfhdrpnt -> e_ident[0] != 127) 
   || (elfhdrpnt -> e_ident[1] != 'E') 
   || (elfhdrpnt -> e_ident[2] != 'L') 
   || (elfhdrpnt -> e_ident[3] != 'F') 
   || (elfhdrpnt -> e_type != ET_REL)) {
    fprintf (stderr, "input file %s is not an elf relocatable\n", argv[3]);
    return (-1);
  }

  /* Build lists of stuff from the input object file */

  highestindex = 0;

  globals  = NULL;
  sections = NULL;
  relocs   = NULL;
  undefs   = NULL;

  errorflag = 0;

  for (i = 0; i < elfhdrpnt -> e_shnum; i ++) {
    elfshpnt = (void *)(objbuf + elfhdrpnt -> e_shoff + i * elfhdrpnt -> e_shentsize);

    /* Memory sections */

    if (((elfshpnt -> sh_type == SHT_PROGBITS) || (elfshpnt -> sh_type == SHT_NOBITS)) && (elfshpnt -> sh_flags & SHF_ALLOC)) {
      if (elfshpnt -> sh_addr != 0) {
        fprintf (stderr, "input section %d has address %x\n", i, elfshpnt -> sh_addr);
        errorflag = 1;
        continue;
      }
      if (i >= highestindex) highestindex = i + 1;
      for (lsection = &sections; (section = *lsection) != NULL; lsection = &(section -> next)) {}
      section = malloc (sizeof *section);
      section -> next     = NULL;
      section -> index    = i;
      section -> vaddr    = 0;
      section -> fileoffs = elfshpnt -> sh_offset;
      section -> size     = elfshpnt -> sh_size;
      section -> writable = ((elfshpnt -> sh_flags & SHF_WRITE) != 0);
      section -> nobits   = (elfshpnt -> sh_type == SHT_NOBITS);
      *lsection = section;
    }

    /* Symbol table */

    if ((elfshpnt[0].sh_type == SHT_SYMTAB) && (elfshpnt[1].sh_type == SHT_STRTAB)) {
      for (j = 0; j < elfshpnt[0].sh_size / elfshpnt[0].sh_entsize; j ++) {			/* loop through each symbol */
        elfsym = (void *)(objbuf + elfshpnt[0].sh_offset + j * elfshpnt[0].sh_entsize);		/* point to elf symbol table entry */
        elfsymnam = objbuf + elfshpnt[1].sh_offset + elfsym -> st_name;				/* point to symbol name string */
        if ((ELF_ST_BIND (elfsym -> st_info) != STB_GLOBAL) && (ELF_ST_BIND (elfsym -> st_info) != STB_WEAK)) continue; /* skip other than global symbols */
        if ((Word)(elfsym -> st_shndx) < 0) {							/* check for negative sh index */
          if (j >= highestindex) highestindex = j + 1;
          global = malloc (sizeof *global);							/* if so, it is an absolute symbol value definition */
          global -> next    = globals;
          global -> index   = j;
          global -> section = NULL;
          global -> value   = elfsym -> st_value;
          global -> name    = elfsymnam;
          globals = global;
        } else if (elfsym -> st_shndx == 0) {							/* check for zero sh index */
          if (strcmp ("OZ_IMAGE_BASEADDR", objbuf + elfshpnt[1].sh_offset + elfsym -> st_name) == 0) goto got_undef;
          if (strcmp ("OZ_IMAGE_NEXTADDR", objbuf + elfshpnt[1].sh_offset + elfsym -> st_name) == 0) goto got_undef;
          fprintf (stderr, "undefined symbol %s\n", objbuf + elfshpnt[1].sh_offset + elfsym -> st_name); /* undefined, barf */
          errorflag = 1;
          continue;
got_undef:
          if (j >= highestindex) highestindex = j + 1;
          undef = malloc (sizeof *undef);
          undef -> next  = undefs;
          undef -> index = j;
          undef -> name  = elfsymnam;
          undefs = undef;
        } else {
          for (section = sections; section != NULL; section = section -> next) {		/* section index > 0, find it in list */
            if (section -> index == (Word)(elfsym -> st_shndx)) break;
          }
          if (section == NULL) {
            fprintf (stderr, "bad section index %d for symbol %s\n", elfsym -> st_shndx, elfsymnam); /* not found, barf */
            errorflag = 1;
            continue;
          }
          if (j >= highestindex) highestindex = j + 1;
          global = malloc (sizeof *global);							/* ok, put relocatable global on list */
          global -> next    = globals;
          global -> index   = j;
          global -> section = section;
          global -> value   = elfsym -> st_value;
          global -> name    = elfsymnam;
          globals = global;
        }
      }
    }
  }

  /* Fill in index list from sections, globals, undefines, and check for duplicate use */

  indexl = malloc (highestindex * sizeof *indexl);
  memset (indexl, 0, highestindex * sizeof *indexl);

  for (global = globals; global != NULL; global = global -> next) {
    i = global -> index;
    if (indexl[i].type != 0) {
      fprintf (stderr, "duplicate use of index %d\n", i);
      errorflag = 1;
    } else {
      indexl[i].type = INDEX_GLOBAL;
      indexl[i].pntr = global;
    }
  }

  for (section = sections; section != NULL; section = section -> next) {
    i = section -> index;
    if (indexl[i].type != 0) {
      fprintf (stderr, "duplicate use of index %d\n", i);
      errorflag = 1;
    } else {
      indexl[i].type = INDEX_SECTION;
      indexl[i].pntr = section;
    }
  }

  /* Assign memory addresses to sections */

  nextaddr = baseaddr;
  for (i = 0; i < 2; i ++) {							/* do read-only, then read/write sections */
    nextaddr = (nextaddr + MEMORY_PAGE_SIZE - 1) & -MEMORY_PAGE_SIZE;		/* page-align for section type */
    for (section = sections; section != NULL; section = section -> next) {	/* scan for all matching sections */
      if (section -> writable == i) {
        elfshpnt = (void *)(objbuf + elfhdrpnt -> e_shoff + section -> index * elfhdrpnt -> e_shentsize);
        nextaddr = ((nextaddr + elfshpnt -> sh_addralign - 1) / elfshpnt -> sh_addralign) * elfshpnt -> sh_addralign;
        section -> vaddr = nextaddr;						/* assign virtual address */
        nextaddr += section -> size;						/* increment for next section */
      }
    }
  }
  nextaddr = (nextaddr + MEMORY_PAGE_SIZE - 1) & -MEMORY_PAGE_SIZE;		/* page-align */

  /* Make global symbols relative to beginning of image, not each section */
  /* This way, there are fewer relocations to be done at load time        */

  for (global = globals; global != NULL; global = global -> next) {
    section = global -> section;
    if (section == NULL) continue;					/* leave symbols that have no section alone */
    global -> value += section -> vaddr;				/* ok, add section base to symbol value */
    global -> section = NULL;						/* symbol is now at its final resting place */
  }

  /* Create two global symbols giving the base and next available address */

  global = malloc (sizeof *global);
  global -> next    = globals;
  global -> index   = 0;
  global -> section = NULL;
  global -> value   = nextaddr;
  global -> name    = "OZ_IMAGE_NEXTADDR";
  globals = global;

  global = malloc (sizeof *global);
  global -> next    = globals;
  global -> index   = 0;
  global -> section = NULL;
  global -> value   = baseaddr;
  global -> name    = "OZ_IMAGE_BASEADDR";
  globals = global;

  /* Process relocations after everything else is in place */

  for (i = 0; i < elfhdrpnt -> e_shnum; i ++) {
    elfshpnt = (void *)(objbuf + elfhdrpnt -> e_shoff + i * elfhdrpnt -> e_shentsize);
    if (elfshpnt -> sh_type != SHT_REL) continue;

    for (section = sections; section != NULL; section = section -> next) {			/* find memory section the relocation table applies to */
      if (section -> index == elfshpnt -> sh_info) break;
    }
    if (section == NULL) continue;								/* sometimes there are bogus sections we don't care about */
												/* ... so we don't care about their relocations either */

    for (j = 0; j < elfshpnt -> sh_size / elfshpnt -> sh_entsize; j ++) {			/* loop through each relocation */
      elfrel = (void *)(objbuf + elfshpnt -> sh_offset + j * elfshpnt -> sh_entsize);		/* point to relocation */
      sectoffs = elfrel -> r_offset;								/* offset of relocation in section */
      valuepnt = (uLong *)(objbuf + section -> fileoffs + sectoffs);				/* point to value to be relocated */
      k = elfrel -> r_info >> 8;								/* get index of what to relocate by */
      reloc = malloc (sizeof *reloc);
      reloc -> imrel.reloc_vaddr = sectoffs + section -> vaddr;					/* output image address of data to be relocated */
      switch (elfrel -> r_info & 0xff) {							/* decode relocation type */
        case 1: {										/* .LONG SYMBOL */
          switch (indexl[k].type) {
            case INDEX_NULL: {
              goto skip_reloc;
            }
            case INDEX_GLOBAL: {
              global = indexl[k].pntr;
              *valuepnt += global -> value;							/* - its a global defined by this image */
              if (global -> section == NULL) goto skip_reloc;					/* - if absolute value, no more relocing */
              reloc -> imrel.reloc_type = OZ_IMAGE_RELOC_IMGBASE;				/* - relocate by image base */
              break;
            }
            case INDEX_SECTION: {
              *valuepnt += ((Section *)(indexl[k].pntr)) -> vaddr;
              goto skip_reloc;
            }
          }
          break;
        }
        case 2: {										/* .LONG SYMBOL-. */
          switch (indexl[k].type) {
            case INDEX_NULL: {
              *valuepnt -= section -> vaddr + sectoffs;
              goto skip_reloc;
            }
            case INDEX_GLOBAL: {
              global = indexl[k].pntr;
              *valuepnt -= section -> vaddr + sectoffs;
              *valuepnt += global -> value;							/* - its a global defined by this image */
              if ((global -> section != NULL) || (baseaddr != 0)) goto skip_reloc;		/* - if relative value, no more relocing */
              reloc -> imrel.reloc_type = OZ_IMAGE_RELOC_IMGBASE_NEGPC;				/* - relocate by negative image base */
              break;
            }
            case INDEX_SECTION: {
              *valuepnt -= section -> vaddr + sectoffs;
              *valuepnt += ((Section *)(indexl[k].pntr)) -> vaddr;
              goto skip_reloc;
            }
          }
          break;
        }
        case 8: {										/* .LONG .+x */
          *valuepnt += section -> vaddr;
          goto skip_reloc;
        }
        default: {
          fprintf (stderr, "unknown relocation %u.%u type %u\n", i, j, elfrel -> r_info & 0xff);
          errorflag = 1;
          goto skip_reloc;
        }
      }
      reloc -> next = relocs;
      relocs = reloc;
skip_reloc:;
    }
  }

  if (errorflag) return (-1);

  /* Sort global symbol list 'alphabetically'                     */
  /* This way, the loader can use a binary search to look them up */

  i = 0;											/* count number of global symbols we have */
  for (global = globals; global != NULL; global = global -> next) i ++;
  globall = malloc (i * sizeof *globall);							/* allocate memory for an array of pointers */
  i = 0;											/* fill in the array with pointers */
  for (global = globals; global != NULL; global = global -> next) globall[i++] = global;
  qsort (globall, i, sizeof *globall, global_compare);						/* sort the elements using the symbol names */
  globals = NULL;										/* re-build the linked list */
  while (i > 0) {
    global = globall[--i];
    global -> next = globals;
    globals = global;
  }
  free (globall);										/* free off the array of pointers */

  /* Print out in-memory structs */

  printf ("\n");
  printf ("Base memory address 0x%x\n", baseaddr);
  printf ("Next memory address 0x%x\n", nextaddr);

  printf ("\nGlobals:\n");
  for (global = globals; global != NULL; global = global -> next) {
    printf ("  %8.8x  %s\n", global -> value, global -> name);
  }

  printf ("\nRelocs:\n");
  for (reloc = relocs; reloc != NULL; reloc = reloc -> next) {
    switch (reloc -> imrel.reloc_type) {
      case OZ_IMAGE_RELOC_IMGBASE: {
        printf ("  %8.8x:  .long  x+imagebase\n", reloc -> imrel.reloc_vaddr);
        break;
      }
      case OZ_IMAGE_RELOC_IMGBASE_NEGPC: {
        printf ("  %8.8x:  .long  x-.\n", reloc -> imrel.reloc_vaddr);
        break;
      }
      default: {
        printf ("  unknown type %d\n", reloc -> imrel.reloc_type);
        break;
      }
    }
  }

  printf ("\nSections:\n");
  for (section = sections; section != NULL; section = section -> next) {
    printf ("%5u  %8.8x %8.8x  %8u  %s  %s\n", 
		section -> index, section -> size, section -> vaddr, section -> fileoffs, section -> writable ? "rw" : "ro", section -> nobits ? "dz" : "");
  }

  /* Write output file */

  outfd = open (argv[2], O_CREAT | O_TRUNC | O_WRONLY, 0640);					/* create output file */
  if (outfd < 0) {
    fprintf (stderr, "error creating output file %s: %s\n", argv[2], strerror (errno));
    return (-1);
  }

  memset (zeroes, 0, MEMORY_PAGE_SIZE);								/* zero fill file so we get zeroes in the gaps */
  for (i = 0; i < nextaddr - baseaddr; i += MEMORY_PAGE_SIZE) writeat (outfd, i, MEMORY_PAGE_SIZE, zeroes);

  for (section = sections; section != NULL; section = section -> next) {			/* write the section data */
    if (!(section -> nobits)) {
      printf ("Writing 0x%8.8x bytes at vaddr 0x%8.8x to file offset 0x%8.8x\n", section -> size, section -> vaddr, section -> vaddr - baseaddr);
      writeat (outfd, section -> vaddr - baseaddr, section -> size, objbuf + section -> fileoffs);
    }
  }

  close (outfd);
  return (0);
}

/* Write to output file at a specific position */

static void writeat (int outfd, int pos, int size, void *buff)

{
  int rc;

  if (lseek (outfd, pos, SEEK_SET) < 0) {
    fprintf (stderr, "error positioning to %d: %s\n", pos, strerror (errno));
    exit (-1);
  }
  rc = write (outfd, buff, size);
  if (rc < 0) {
    fprintf (stderr, "error writing %d bytes at position %d: %s\n", size, pos, strerror (errno));
    exit (-1);
  }
  if (rc < size) {
    fprintf (stderr, "only wrote %d bytes of %d at position %d\n", rc, size, pos);
    exit (-1);
  }
}

/* qsort compare routine for global symbols */

static int global_compare (const void *v1, const void *v2)

{
  Global *g1, *g2;

  g1 = *(Global **)v1;
  g2 = *(Global **)v2;

  return (strcmp (g1 -> name, g2 -> name));
}
