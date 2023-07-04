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
/*  I/O function codes for device class "disk"				*/
/*									*/
/************************************************************************/

#ifndef _OZ_IO_DISK_H
#define _OZ_IO_DISK_H

#include "oz_knl_dcache.h"
#include "oz_knl_devio.h"

#define OZ_IO_DISK_CLASSNAME "disk"
#define OZ_IO_DISK_BASE (0x00000400)
#define OZ_IO_DISK_MASK (0xFFFFFF00)

#include "oz_knl_hw.h"

/* Set volume valid bit */

#define OZ_IO_DISK_SETVOLVALID OZ_IO_DR(OZ_IO_DISK_BASE,1)

typedef struct { int valid;			/* 0=turn offline; 1=turn online */
                 int unload;			/* 0=don't eject; 1=spin down and eject */
               } OZ_IO_disk_setvolvalid;

/* Write blocks */

#define OZ_IO_DISK_WRITEBLOCKS OZ_IO_DW(OZ_IO_DISK_BASE,2)

typedef struct { uLong size;			/* size (in bytes) to write (multiple of block size) */
                 const void *buff;		/* buffer address */
                 OZ_Dbn slbn;			/* starting logical block number */
                 int inhretries;		/* inhibit retries */
                 int writethru;			/* write through to magnetic media (bypassing drive's internal cache) */
               } OZ_IO_disk_writeblocks;

/* Read blocks */

#define OZ_IO_DISK_READBLOCKS OZ_IO_DR(OZ_IO_DISK_BASE,3)

typedef struct { uLong size;			/* size (in bytes) to read (multiple of block size) */
                 void *buff;			/* buffer address */
                 OZ_Dbn slbn;			/* starting logical block number */
                 int inhretries;		/* inhibit retries */
               } OZ_IO_disk_readblocks;

/* Get file info, part 1 */

#define OZ_IO_DISK_GETINFO1 OZ_IO_DN(OZ_IO_DISK_BASE,4)

typedef struct { uLong blocksize;				/* number of bytes in a block */
                 OZ_Dbn totalblocks;				/* number of blocks on disk */
                 OZ_Dbn parthoststartblock;			/* partition host starting logical block number */
                 char parthostdevname[OZ_DEVUNIT_NAMESIZE];	/* partition host disk device name (null string if not a partition) */
                 uLong secpertrk;				/* number of sectors in a track */
                 uLong trkpercyl;				/* number of tracks in a cylinder */
                 uLong cylinders;				/* number of cylinders in drive */
                 uLong bufalign;				/* buffer alignment required: */
								/* 0: byte; 1: word; 3: long; 7: quad */
                 uLong (*ramdisk_map) (OZ_Iochan *iochan, OZ_Dcmpb *dcmpb); /* NULL: not a ramdisk; else: pointer to ramdisk map routine */
                 uLong parttypesize;				/* size of partition type buffer */
                 char *parttypebuff;				/* pointer to partition type buffer */
                 uLong (*ramdisk_pfmap) (OZ_Iochan *iochan, OZ_Dbn logblock, OZ_Mempage *phypage_r); /* get pointer to ramdisk page */
               } OZ_IO_disk_getinfo1;

/* Write from physical page(s) (Kernel mode only) */

#define OZ_IO_DISK_WRITEPAGES OZ_IO_DW(OZ_IO_DISK_BASE,5)

typedef struct { uLong size;			/* number of bytes to write (multiple of block size) */
                 const OZ_Mempage *pages;	/* points to array of physical page numbers */
                 uLong offset;			/* byte offset into first page */
                 OZ_Dbn slbn;			/* starting logical block number */
                 int inhretries;		/* inhibit retries */
                 int writethru;			/* write through to magnetic media (bypassing drive's internal cache) */
               } OZ_IO_disk_writepages;

/* Read into physical page(s) (Kernel mode only) */

#define OZ_IO_DISK_READPAGES OZ_IO_DR(OZ_IO_DISK_BASE,6)

typedef struct { uLong size;			/* number of bytes to read (multiple of block size) */
                 const OZ_Mempage *pages;	/* points to array of physical page numbers */
                 uLong offset;			/* byte offset into first page */
                 OZ_Dbn slbn;			/* starting logical block number */
                 int inhretries;		/* inhibit retries */
                 int ix4kbuk;			/* if set, reading 4k IX database bucket page */
               } OZ_IO_disk_readpages;

/* Set disk as the crash dump device (kernel mode only) */

#define OZ_IO_DISK_CRASH OZ_IO_DW(OZ_IO_FS_BASE,7)

typedef struct { uLong (*crashentry) (void *crashparam, 	/* entrypoint to call to write blocks */
                                      OZ_Dbn lbn, 		/* - lbn on disk to start writing to */
                                      uLong size, 		/* - number of bytes to write (multiple of block size) */
                                      OZ_Mempage phypage, 	/* - physical page to start writing from */
                                      uLong offset);		/* - offset in first physical page to start at */
                 void *crashparam;				/* param to pass to crashentry routine */
                 uLong blocksize;				/* disk block size */
               } OZ_IO_disk_crash;

/* Format disk media */

#define OZ_IO_DISK_FORMAT OZ_IO_DW(OZ_IO_FS_BASE,8)

typedef struct { const char *paramstr;				/* pointer to parameter string - see driver for settings */
               } OZ_IO_disk_format;

#endif
