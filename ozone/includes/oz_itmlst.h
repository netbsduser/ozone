//+++2002-01-14
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
//---2002-01-14

#ifndef _OZ_ITMLST_H
#define _OZ_ITMLST_H

typedef struct { uWord size;	// size of buff
                 uWord item;	// code of item
                 void *buff;	// pointer to item value
               } OZ_Itmlst2;

typedef struct { uWord size;	// size of buff
                 uWord item;	// code of item
                 void *buff;	// pointer to item value
                 uWord *rlen;	// where to return actual length of item
               } OZ_Itmlst3;

#endif
