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
/*  I/O function codes for device class "timer"				*/
/*									*/
/************************************************************************/

#ifndef _OZ_IO_TIMER_H
#define _OZ_IO_TIMER_H

#include "ozone.h"
#include "oz_knl_devio.h"
#include "oz_knl_hw.h"

#define OZ_IO_TIMER_DEV "timer"
#define OZ_IO_TIMER_CLASSNAME "timer"
#define OZ_IO_TIMER_BASE (0x00000D00)
#define OZ_IO_TIMER_MASK (0xFFFFFF00)

/* Wait until a particular time comes */

#define OZ_IO_TIMER_WAITUNTIL OZ_IO_DN(OZ_IO_TIMER_BASE,1)

typedef struct { OZ_Datebin datebin;
               } OZ_IO_timer_waituntil;

#endif
