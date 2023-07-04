//+++2002-08-17
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
//---2002-08-17

/************************************************************************/
/*									*/
/*  General memory allocation routines					*/
/*									*/
/************************************************************************/

#ifndef _OZ_KNL_MALLOC_H
#define _OZ_KNL_MALLOC_H

typedef uLong OZ_Memsize;

#ifdef _OZ_KNL_MALLOC_C
typedef struct OZ_Memlist OZ_Memlist;
#else
typedef void OZ_Memlist;
#endif

void *oz_malloc (OZ_Memlist *memlist, OZ_Memsize size, OZ_Memsize *size_r, int pc);	// not locked
void *oz_malloc_lk (OZ_Memlist *memlist, OZ_Memsize size, OZ_Memsize *size_r, int pc);	// already have lock
OZ_Memsize oz_valid (OZ_Memlist *memlist, void *adrs);
OZ_Memsize oz_free (OZ_Memlist *memlist, void *adrs);
OZ_Memlist *oz_freesiz (OZ_Memlist *memlist, 
                        OZ_Memsize size, 
                        void *adrs, 
                        uLong (*lock) (void *lockprm), 
                        void (*unlk) (void *lockprm, uLong lk), 
                        void *lockprm);
void oz_mlvalid (OZ_Memlist *memlist);

#endif
