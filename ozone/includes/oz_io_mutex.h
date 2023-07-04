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
/*  I/O function codes for device class "mutex"				*/
/*									*/
/************************************************************************/

#ifndef _OZ_IO_MUTEX_H
#define _OZ_IO_MUTEX_H

#include "oz_knl_devio.h"
#include "oz_knl_lock.h"

#define OZ_IO_MUTEX_CLASSNAME "mutex"
#define OZ_IO_MUTEX_BASE (0x00000900)
#define OZ_IO_MUTEX_MASK (0xFFFFFF00)

#define OZ_IO_MUTEX_TEMPLATE "mutex"

/* Create and access a lock */

#define OZ_IO_MUTEX_CREATE OZ_IO_DW(OZ_IO_MUTEX_BASE,1)

typedef struct { uLong namesize;		/* size of lock's name */
                 const void *namebuff;		/* lock's name */
               } OZ_IO_mutex_create;

/* Lookup and access a lock */

#define OZ_IO_MUTEX_LOOKUP OZ_IO_DR(OZ_IO_MUTEX_BASE,2)

typedef struct { uLong namesize;		/* size of lock's name */
                 const void *namebuff;		/* lock's name */
               } OZ_IO_mutex_lookup;

/* Set the new mode of a lock */

#define OZ_IO_MUTEX_SETMODE OZ_IO_DW(OZ_IO_MUTEX_BASE,3)

#define OZ_IO_MUTEX_SETMODE_FLAG_NOQUEUE (0x00000001)
#define OZ_IO_MUTEX_SETMODE_FLAG_EXPRESS (0x00000002)

typedef struct { OZ_Lockmode newmode;		/* new mode that we want */
                 uLong flags;			/* flags */
               } OZ_IO_mutex_setmode;

/* Say we are willing to give up lock on demand */

#define OZ_IO_MUTEX_UNBLOCK OZ_IO_DR(OZ_IO_MUTEX_BASE,4)

/* All done with lock */

#define OZ_IO_MUTEX_CLOSE OZ_IO_DN(OZ_IO_MUTEX_BASE,5)

/* Get info about lock */

#define OZ_IO_MUTEX_GETINFO1 OZ_IO_DN(OZ_IO_MUTEX_BASE,6)

typedef struct { uLong namesize;		/* size of namebuff */
                 void *namebuff;		/* where to return name string */
                 uLong *namerlen;		/* where to return name string length */
                 OZ_Lockmode curmode;		/* current mode that this channel has on lock */
                 Long active_readers;		/* number of those that have read access to resource */
                 Long active_writers;		/* number of those that have write access to resource */
                 Long block_readers;		/* number of those that are blocking readers */
                 Long block_writers;		/* number of those that are blocking writers */
               } OZ_IO_mutex_getinfo1;

#endif
