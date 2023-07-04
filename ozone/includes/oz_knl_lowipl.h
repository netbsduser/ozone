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
/*  Lowipls are used by device drivers when they are at high ipl to 	*/
/*  get to a low ipl (like softint level)				*/
/*									*/
/*    The three operations available are:				*/
/*									*/
/*	alloc - allocate a new lowipl struct (must be called at 	*/
/*	        smplock <= np, ie, well ahead of when it will be used)	*/
/*									*/
/*	call - call a routine at low ipl (must be called at smplock 	*/
/*	       <= hi level, ie, just about anywhere)			*/
/*									*/
/*	free - frees off a lowipl struct that is no longer needed	*/
/*									*/
/************************************************************************/

#ifndef _OZ_KNL_LOWIPL_H
#define _OZ_KNL_LOWIPL_H

#ifdef _OZ_KNL_LOWIPL_C
typedef struct OZ_Lowipl OZ_Lowipl;
#else
typedef void OZ_Lowipl;
#endif

#include "oz_knl_hw.h"

globalref OZ_Lowipl *volatile oz_knl_lowipl_lowipls;

OZ_Lowipl *oz_knl_lowipl_alloc (void);
void oz_knl_lowipl_call (OZ_Lowipl *lowipl, void (*entry) (void *param, OZ_Lowipl *lowipl), void *param);
void oz_knl_lowipl_free (OZ_Lowipl *lowipl);
void oz_knl_lowipl_handleint (void);

#endif
