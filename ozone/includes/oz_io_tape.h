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

/************************************************************************/
/*									*/
/*  I/O function codes for device class "tape"				*/
/*									*/
/************************************************************************/

#ifndef _OZ_IO_TAPE_H
#define _OZ_IO_TAPE_H

#include "oz_knl_devio.h"

#define OZ_IO_TAPE_CLASSNAME "tape"
#define OZ_IO_TAPE_BASE (0x00000C00)
#define OZ_IO_TAPE_MASK (0xFFFFFF00)

#define OZ_IO_TAPE_ALLDENSMAX (256)	// max length of all densities string (incl null)
#define OZ_IO_TAPE_DENSMAX (16)		// max length of density string (incl null)
#define OZ_IO_TAPE_MTYPEMAX (16)	// max length of media type string (incl null)

#include "oz_knl_hw.h"

/* Set volume valid bit */

#define OZ_IO_TAPE_SETVOLVALID OZ_IO_DR(OZ_IO_TAPE_BASE,1)

typedef struct { int valid;			/* 0=turn offline (unload); 1=turn online (load) */
					/* unload only: */
                 int eject;			/* 0=don't eject; 1=spin down and eject */
                 int immed;			/* 0=wait until unload completes */
						/* 1=complete upon initiation of unload */
					/* load only: */
                 const char *density;		/*   NULL or "" : default density */
						/*         else : set this density */
               } OZ_IO_tape_setvolvalid;

/* Write block */

#define OZ_IO_TAPE_WRITEBLOCK OZ_IO_DW(OZ_IO_TAPE_BASE,2)

typedef struct { uLong size;			/* size (in bytes) to write */
                 const void *buff;		/* buffer address */
               } OZ_IO_tape_writeblock;

/* Read block */

#define OZ_IO_TAPE_READBLOCK OZ_IO_DR(OZ_IO_TAPE_BASE,3)

typedef struct { uLong size;			/* size (in bytes) to read */
                 void *buff;			/* buffer address */
                 uLong *rlen;			/* where to return length actually read */
                 int inhretries;		/* inhibit retries */
               } OZ_IO_tape_readblock;

/* Get tape info, part 1 */

#define OZ_IO_TAPE_GETINFO1 OZ_IO_DN(OZ_IO_TAPE_BASE,4)

typedef struct { uLong fileno;			/* number of filemarks before tape head (0 when rewound) */
                 uLong blockno;			/* number of blocks since last filemark */
                 uLong gapno;			/* total blocks + filemarks before tape head (0 when rewound) */
                 uLong tapeflags;		/* tape status flags */
                 uLong minblocksize;		/* minimum block size the drive can process */
                 uLong maxblocksize;		/* maximum block size the drive can process */
                 uWord tapcapman;		/* tape capacity = tapcapman * 2 ** tapcapexp bytes */
                 uWord tapcapexp;
                 int tappossiz;			/* size of tape position block (return 0 if not supported) */
                 int densize;			/* sizeof denbuff */
                 char *denbuff;			/* where to return current density */
               } OZ_IO_tape_getinfo1;

#define OZ_IO_TAPE_FLAG_INEOT (0x00000001)	/* in logical end-of-tape area */
#define OZ_IO_TAPE_FLAG_ATBOT (0x00000002)	/* at beginning-of-tape */
#define OZ_IO_TAPE_FLAG_ATEOD (0x00000004)	/* at end-of-data spot */
#define OZ_IO_TAPE_FLAG_WTLOK (0x00000008)	/* write locked (read-only) */

/* Rewind tape */

#define OZ_IO_TAPE_REWIND OZ_IO_DR(OZ_IO_TAPE_BASE,5)

typedef struct { int immed;			/* 0: wait till rewind completes; 1: complete immediately */
               } OZ_IO_tape_rewind;

/* Skip records/files */

#define OZ_IO_TAPE_SKIP OZ_IO_DR(OZ_IO_TAPE_BASE,6)

typedef struct { int files;			/* 0: skip 'count' records; 1: skip 'count' files */
                 int count;			/* number of records/files to skip (pos or neg) */
                 int *skipped;			/* where to return actual number of records/files skipped */
               } OZ_IO_tape_skip;		/* (same sign as 'count') */

/* Write filemark */

#define OZ_IO_TAPE_WRITEMARK OZ_IO_DW(OZ_IO_TAPE_BASE,7)

typedef struct { int endoftape;			/* 0: write a mark and position tape head after it */
						/* 1: write two marks and position tape head between them */
               } OZ_IO_tape_writemark;

/* Get tape position */

#define OZ_IO_TAPE_GETPOS OZ_IO_DR(OZ_IO_TAPE_BASE,8)

typedef struct { int tappossiz;			/* as returned by getinfo1 */
                 void *tapposbuf;		/* where to write tape position string */
               } OZ_IO_tape_getpos;

/* Set tape position */

#define OZ_IO_TAPE_SETPOS OZ_IO_DR(OZ_IO_TAPE_BASE,9)

typedef struct { int tappossiz;			/* as returned by getinfo1 */
                 const void *tapposbuf;		/* tape position string as returned by getpos */
               } OZ_IO_tape_setpos;

/* Get tape info, part 2 (no tape need be loaded) */

#define OZ_IO_TAPE_GETINFO2 OZ_IO_DN(OZ_IO_TAPE_BASE,10)

typedef struct { int mtypesize;			/* sizeof mtypebuff */
                 char *mtypebuff;		/* media type string (eg, "8mm", "dlt", etc) */
                 int alldensize;		/* sizeof alldenbuff */
                 char *alldenbuff;		/* where to return list of densities drive is capable of */
						/* separated by a single space (eg, "6250 1600 800") */
						/* first one is default density */
                 uLong bufalign;		/* buffer alignment: */
						/* 0: byte; 1: word; 3: long; 7: quad */
               } OZ_IO_tape_getinfo2;

#endif
