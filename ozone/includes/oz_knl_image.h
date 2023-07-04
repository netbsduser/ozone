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

#ifndef _OZ_KNL_IMAGE_H
#define _OZ_KNL_IMAGE_H

#ifdef _OZ_KNL_IMAGE_C
typedef struct OZ_Image OZ_Image;
typedef struct OZ_Imagelist OZ_Imagelist;
#else
typedef void OZ_Image;
typedef void OZ_Imagelist;
#endif

typedef struct OZ_Image_Args OZ_Image_Args;
typedef struct OZ_Image_Refd_image OZ_Image_Refd_image;
typedef struct OZ_Image_Secload OZ_Image_Secload;

#include "oz_io_fs.h"
#include "oz_knl_devio.h"
#include "oz_knl_event.h"
#include "oz_knl_procmode.h"
#include "oz_knl_section.h"
#include "oz_knl_security.h"

#define OZ_IMAGE_HDRBUFSIZE (512)

struct OZ_Image_Refd_image { OZ_Image_Refd_image *next;		/* pointer to next in list */
                             OZ_Image *image;			/* image that was referenced */
                           };

struct OZ_Image_Secload { OZ_Image_Secload *next;		/* pointer to next in list */
                          OZ_Section *section;			/* section that was created */
                          OZ_Mempage npages;			/* number of pages in the section */
                          OZ_Mempage svpage;			/* starting virtual page of the section */
                          int writable;				/* section is mapped read/write */
                          OZ_Seclock *seclock;			/* if not NULL, section is locked in memory */
                        };

struct OZ_Image_Args { char *imagename;				/* points to image name string */
                       int sysimage;				/* 0 = process image; 1 = system image */
                       int level;				/* 0 = loading root image; else loading shareable image */
                       OZ_Procmode procmode;			/* processor mode of caller */
                       OZ_Procmode mprocmode;			/* procmode of memory structs (OZ_PROCMODE_KNL or _SYS) */
                       OZ_Event *ioevent;			/* i/o event flag */
                       OZ_Iochan *iochan;			/* i/o channel pointing to image */
                       OZ_IO_fs_getinfo1 *fs_getinfo1;		/* getinfo1 of image file */
                       OZ_Secattr *secattr;			/* security attributes to apply to sections */
                       OZ_Image_Secload *secloads;		/* list of sections created */
                       OZ_Image_Refd_image *refd_images;	/* list of images referred to by this image */
                       void *baseaddr;				/* base address the image was loaded at */
                       void *startaddr;				/* start address (entrypoint) of the image */
                       void *imagex;				/* image format specific struct pointer */
                       uByte hdrbuf[OZ_IMAGE_HDRBUFSIZE];	/* contains image header */
                     };

typedef struct { const char *filetype;							/* filetype, including the dot */
                 uLong (*load) (OZ_Image_Args *imageargs);				/* this routine loads an image */
                 int (*lookup) (void *imagexv, char *symname, OZ_Pointer *symvalu);	/* this routine looks up a symbol in an image */
                 void (*unload) (void *imagexv, OZ_Procmode mprocmode);			/* this routine finishes unloading an image */
                 void *(*symscan) (void *imagexv, void *lastsym, char **symname_r, OZ_Pointer *symvalu_r); /* scan symbol table */
               } OZ_Image_Hand;

void oz_knl_imagelist_close (void);
OZ_Image *oz_knl_image_next (OZ_Image *lastimage, int sysimage);
OZ_Image_Secload *oz_knl_image_secloads (OZ_Image *image);
const char *oz_knl_image_name (OZ_Image *image);
uLong oz_knl_image_load (OZ_Procmode procmode, char *imagename, int sysimage, int level, void **baseaddr_r, void **startaddr_r, OZ_Image **image_r);
int oz_knl_image_lookup (OZ_Image *image, char *symname, OZ_Pointer *symvalu);
void *oz_knl_image_symscan (OZ_Image *image, void *lastsym, char **symname_r, OZ_Pointer *symvalu_r);
Long oz_knl_image_increfc (OZ_Image *image, Long inc);
uLong oz_knl_image_lockinmem (OZ_Image *image, OZ_Procmode procmode);
OZ_Secattr *oz_knl_image_getsecattr (OZ_Image *image);
uLong oz_knl_image_read (OZ_Image_Args *imageargs, OZ_Dbn vbn, uLong size, void *buff);
uLong oz_knl_image_read2 (OZ_Image_Args *imageargs, OZ_Dbn vbn, uLong offs, uLong size, void *buff);

#endif
