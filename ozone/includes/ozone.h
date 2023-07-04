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

#ifndef _OZONE_H
#define _OZONE_H

/* This points to an address that is not valid in any context */

#ifndef NULL
#define NULL ((void *)0)
#endif

/* This is an 8-bit quantity */

typedef char Byte;
typedef unsigned char uByte;

/* This is an 16-bit quantity */

typedef short Word;
typedef unsigned short uWord;

/* This is an 32-bit quantity */

typedef int Long;
typedef unsigned int uLong;

/* This is an 64-bit quantity */

typedef long long Quad;
typedef unsigned long long uQuad;

/* Pointer to a character string */

typedef char * Charp;

/* This is a disk block number */

typedef uLong OZ_Dbn;

/* Copyright message */

extern const char oz_sys_copyright [];

#endif
