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

#ifndef _OZ_DEV_CONSOLE_486_H
#define _OZ_DEV_CONSOLE_486_H

#include "oz_knl_hw.h"

extern OZ_Smplock *oz_dev_keyboard_smplock;			/* points to keyboard smp lock */

void oz_dev_console_init ();					/* standard driver initialization routine */
void oz_dev_keyboard_send (void *devexv, int size, char *buff);	/* send a string to class driver as if it came from keyboard */
char oz_dev_keyboard_getc (int ignmb);				/* get character from keyboard */

#endif
