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
/*  I/O function codes for device class "mouse"				*/
/*									*/
/************************************************************************/

#ifndef _OZ_IO_MOUSE_H
#define _OZ_IO_MOUSE_H

#include "ozone.h"
#include "oz_knl_devio.h"
#include "oz_knl_hw.h"

#define OZ_IO_MOUSE_CLASSNAME "mouse"
#define OZ_IO_MOUSE_BASE (0x00000800)
#define OZ_IO_MOUSE_MASK (0xFFFFFF00)

/* Read scancodes from the mouse */

#define OZ_IO_MOUSE_READ OZ_IO_DN(OZ_IO_MOUSE_BASE,1)

typedef struct { uLong size;		/* size of 'buff' */
                 void *buff;		/* where to return scan codes */
                 uLong *rlen;		/* where to return number of bytes read (required) */
               } OZ_IO_mouse_read;

#endif
