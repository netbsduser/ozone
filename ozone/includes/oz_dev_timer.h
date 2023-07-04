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

#ifndef _OZ_DEV_TIMER_H
#define _OZ_DEV_TIMER_H

#include "ozone.h"

#ifdef _OZ_DEV_TIMER_C
typedef struct OZ_Timer OZ_Timer;
#else
typedef void OZ_Timer;
#endif

#include "oz_knl_hw.h"

extern OZ_Smplock *oz_hw_smplock_tm;

void oz_dev_timer_init (void);
OZ_Timer *oz_knl_timer_alloc (void);
void oz_knl_timer_insert (OZ_Timer *timer, OZ_Datebin datebin, void (*entry) (void *param, OZ_Timer *timer), void *param);
int oz_knl_timer_remove (OZ_Timer *timer);
void oz_knl_timer_free (OZ_Timer *timer);
void oz_knl_timer_timeisup (void);

void oz_knl_timer_validate (void);

#endif
