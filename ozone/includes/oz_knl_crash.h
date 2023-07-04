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

#ifndef _OZ_KNL_CRASH_H
#define _OZ_KNL_CRASH_H

#include "oz_knl_devio.h"
#include "oz_knl_hw.h"

typedef struct { char magic[8];			/* "oz_crash" */
                 uLong version;			/* 1 */
                 OZ_Datebin when;		/* when it crashed */
                 uLong headersize;		/* size of the header block (in bytes) */
                 uLong blocksize;		/* disk block size */
                 uLong l2pagesize;		/* log2 of physical page size */
                 OZ_Dbn filesize;		/* size of file (in blocks) */
                 Long cpuidx;			/* cpu index of cpu that crashed */
                 OZ_Sigargs *sigargs;		/* signal args at time of crash */
                 OZ_Mchargs **mchargs;		/* pointer to array of original machine arg pointers at time of crash */
                 OZ_Mchargx_knl **mchargx_knl;	/* pointer to array of original kernel machine arg extension pointers */
                 OZ_Mempage holebeg;		/* start of memory hole */
                 OZ_Mempage holeend;		/* end of memory hole */
                 OZ_Mempage numpages;		/* total number of pages (including the hole pages) */
                 OZ_Mchargs mchargs_cpy;	/* copy of the machine args at time of crash */
                 OZ_Mchargx_knl mchargx_knl_cpy; /* copy of the kernel machine arg extensions */
               } OZ_Crash_block;

uLong oz_knl_crash_set (OZ_Iochan *iochan);
int oz_knl_crash_dump (Long cpuidx, OZ_Sigargs *sigargs, OZ_Mchargs **mchargs, OZ_Mchargx_knl **mchargx_knl);

#endif
