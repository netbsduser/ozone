//+++2002-05-10
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
//---2002-05-10

#ifndef _OZ_SYS_CONDHAND_H
#define _OZ_SYS_CONDHAND_H

#include "oz_knl_hw.h"

typedef void *OZ_Chparam;
typedef uLong (*OZ_Chentry) (OZ_Chparam chparam, OZ_Sigargs sigargs[], OZ_Mchargs *mchargs);

/************************************************************************/
/*  These routines are provided in oz_sys_condhand.c			*/
/************************************************************************/

void oz_sys_condhand_default (OZ_Sigargs sigargs[], OZ_Mchargs *mchargs);
uLong oz_sys_condhand_rtnanysig (OZ_Chparam chparam, OZ_Sigargs sigargs[], OZ_Mchargs *mchargs);

/************************************************************************/
/*  These routines are provided by the hardware layer			*/
/************************************************************************/

/* Create an arbitrary trap condition as specified by the call arguments */

void oz_sys_condhand_signal (OZ_Sigargs nargs, OZ_Sigargs arg1, ...);
void oz_sys_condhand_signalv (OZ_Sigargs sigargs[]);

/* Try a routine under the protection of a condition handler */

uLong oz_sys_condhand_try (uLong (*tryentry) (void *tryparam), void *tryparam, OZ_Chentry chentry, OZ_Chparam chparam);

/* Call currently active condition handler */

void oz_sys_condhand_call (OZ_Sigargs sigargs[], OZ_Mchargs *mchargs);

#endif
