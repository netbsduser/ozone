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

#ifndef _OZ_IMAGE_H
#define _OZ_IMAGE_H

#include "ozone.h"
#include "oz_knl_hw.h"

/* If a 64-bit image is being generated on a 32-bit host, */
/* make sure the 64-bit loader doesn't try to put in      */
/* extra padding that the 32-bit host generator won't     */

#pragma pack (1)

/* Image header */

#define OZ_IMAGE_NULLSTARTADDR ((OZ_Pointer)-1)

typedef struct { char magic[8];				/* "oz_image", "oz_imaxp" */
                 uLong fxhdrsize;			/* sizeof (OZ_Image_Header) */
                 OZ_Mempage basevpage;			/* base virtual page of image (0 for dynamic) */
                 OZ_Pointer startaddr;			/* image transfer address */
                 uLong imagesize;			/* total number of memory bytes used by image */
                 uLong imageoffs;			/* offset of image data from very beginning of file */
                 uLong refdessiz;			/* referenced shareable descriptor size */
                 uLong refdesnum;			/* referenced shareable descriptor count */
                 uLong refdesoff;			/* referenced shareable descriptor offset from very beginning of file */
                 uLong refstrsiz;			/* referenced shareable string area size */
                 uLong refstroff;			/* referenced shareable string area offset from very beginning of file */
                 uLong gbldessiz;			/* global symbol descriptor size */
                 uLong gbldesnum;			/* global symbol descriptor count */
                 uLong gbldesoff;			/* global symbol descriptor offset from very beginning of file */
                 uLong gblstrsiz;			/* global symbol string area size */
                 uLong gblstroff;			/* global symbol string area offset from very beginning of file */
                 uLong unddessiz;			/* undefined symbol descriptor size */
                 uLong unddesnum;			/* undefined symbol descriptor count */
                 uLong unddesoff;			/* undefined symbol descriptor offset from very beginning of file */
                 uLong undstrsiz;			/* undefined symbol string area size */
                 uLong undstroff;			/* undefined symbol string area offset from very beginning of file */
                 uLong reldessiz;			/* relocation descriptor size */
                 uLong reldesnum;			/* relocation descriptor count */
                 uLong reldesoff;			/* relocation descriptor offset from very beginning of file */
                 uLong secdessiz;			/* section descriptor size */
                 uLong secdesnum;			/* section descriptor count */
                 uLong secdesoff;			/* section descriptor offset from very beginning of file */
               } OZ_Image_Headr;

/* Referenced shareable image descriptors */

typedef struct { uLong refim_namof;			/* offset in string buffer for name string */
                 OZ_Pointer refim_image;		/* used by loader - address of image struct */
               } OZ_Image_Refim;

/* Global symbol descriptors */

#define OZ_IMAGE_GLOBL_FLAG_RELOC 0x1	// value is relative to image base address (dynamic images only)
#define OZ_IMAGE_GLOBL_FLAG_ENTRY 0x2	// value is a function entrypoint

typedef struct { uLong globl_namof;			/* offset in string buffer for name string */
                 OZ_Pointer globl_value;		/* the symbols value */
                 uLong globl_flags;			/* flags, see above */
               } OZ_Image_Globl;

/* Undefined symbol descriptors */

typedef struct { uLong undef_namof;			/* offset in string buffer for name string */
                 uLong undef_refim;			/* image number that defines the symbol */
                 OZ_Pointer undef_value;		/* used by loader - symbols value */
               } OZ_Image_Undef;

/* Relocation descriptors */

typedef enum { OZ_IMAGE_RELOC_IMGBASE, OZ_IMAGE_RELOC_IMGBASE_NEGPC, OZ_IMAGE_RELOC_UNDEF, OZ_IMAGE_RELOC_UNDEF_NEGPC } OZ_Image_Reloc_Type;

typedef struct { OZ_Pointer reloc_vaddr;		/* virtual address of location to relocate */
                 OZ_Image_Reloc_Type reloc_type;	/* relocation type */
                 uLong reloc_indx;			/* index to apply reloc */
               } OZ_Image_Reloc;

/* Section descriptors */

typedef struct { OZ_Mempage sectn_pages;		/* number of memory pages */
                 OZ_Mempage sectn_vpage;		/* starting virtual page, relative to header's basevpage */
                 int sectn_write;			/* 0 : page is read-only, 1 : page is read/write */
               } OZ_Image_Sectn;

#pragma nopack

#endif
