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
/*  I/O function codes for device class "pipe"				*/
/*									*/
/************************************************************************/

#ifndef _OZ_IO_PIPE_H
#define _OZ_IO_PIPE_H

#include "oz_knl_devio.h"

#define OZ_IO_PIPE_CLASSNAME "pipe"
#define OZ_IO_PIPE_BASE (0x00000A00)
#define OZ_IO_PIPE_MASK (0xFFFFFF00)

#define OZ_IO_PIPE_BUFSIZ (4096)

/* Device names */

#define OZ_IO_NULL_DEVICE "null"
#define OZ_IO_ZERO_DEVICE "zero"
#define OZ_IO_PIPER_TEMPLATE "piper"
#define OZ_IO_PIPES_TEMPLATE "pipes"

/* I/O select codes */

#define OZ_SE_PIPE_READREADY  (OZ_IO_PIPE_BASE | 0x01)	/* the driver has a packet that needs to be read */
#define OZ_SE_PIPE_WRITEREADY (OZ_IO_PIPE_BASE | 0x02)	/* the driver is able to accept data for writing */

#endif
