//+++2002-03-18
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
//---2002-03-18

/************************************************************************/
/*									*/
/*  Print a debugging message to the console				*/
/*									*/
/************************************************************************/

#ifndef _OZ_KNL_KNL_PRINTK_H
#define _OZ_KNL_KNL_PRINTK_H

#include <stdarg.h>

#include "oz_knl_hw.h"

void oz_knl_printkaddr (void *addr);
int oz_knl_printinstr (int il, const uByte *ib, const uByte *pc);
void oz_hw_pause (const char *prompt);
void oz_crash (const char *fmt, ...) __attribute__ ((noreturn));
void oz_knl_dumpmem (uLong size, const void *buff);
void oz_knl_dumpmem2 (uLong size, const void *buff, OZ_Pointer addr);
void oz_knl_printkp (const char *fmt, ...);
void oz_knl_printk (const char *fmt, ...);
void oz_knl_printkv (const char *fmt, va_list ap);

#endif
