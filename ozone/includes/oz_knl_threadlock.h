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

#ifndef _OZ_KNL_THREADLOCK_H
#define _OZ_KNL_THREADLOCK_H

#ifdef _OZ_KNL_THREADLOCK_C
typedef struct OZ_Threadlock OZ_Threadlock;
#else
typedef void OZ_Threadlock;
#endif

uLong oz_knl_threadlock_create (const char *name, OZ_Threadlock **threadlock_r);
void oz_knl_threadlock_delete (OZ_Threadlock *threadlock);
void oz_knl_threadlock_ex (OZ_Threadlock *threadlock);
void oz_knl_threadlock_ex_pr (OZ_Threadlock *threadlock);
void oz_knl_threadunlk_ex (OZ_Threadlock *threadlock);
void oz_knl_threadlock_pr (OZ_Threadlock *threadlock);
void oz_knl_threadunlk_pr (OZ_Threadlock *threadlock);

#endif
