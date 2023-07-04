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

#ifndef _OZ_SYS_EVENT_H
#define _OZ_SYS_EVENT_H

#include "ozone.h"

#include "oz_knl_handle.h"
#include "oz_knl_hw.h"
#include "oz_knl_procmode.h"

OZ_HW_SYSCALL_DCL_3 (event_create, OZ_Procmode, procmode, const char *, name, OZ_Handle *, h_event_r)
OZ_HW_SYSCALL_DCL_3 (event_setimint, OZ_Handle, h_event, OZ_Datebin, interval, OZ_Datebin, basetime)
OZ_HW_SYSCALL_DCL_2 (event_getimint, OZ_Handle, h_event, OZ_Datebin *, interval_r)
OZ_HW_SYSCALL_DCL_2 (event_getimnxt, OZ_Handle, h_event, OZ_Datebin *, nextwhen_r)
OZ_HW_SYSCALL_DCL_4 (event_inc, OZ_Procmode, procmode, OZ_Handle, h_event, Long, inc, Long *, value_r)
OZ_HW_SYSCALL_DCL_4 (event_set, OZ_Procmode, procmode, OZ_Handle, h_event, Long, value, Long *, value_r)
OZ_HW_SYSCALL_DCL_3 (event_wait, OZ_Procmode, procmode, OZ_Handle, h_event, int, astloop)
OZ_HW_SYSCALL_DCL_4 (event_nwait, OZ_Procmode, procmode, uLong, nevents, OZ_Handle *, h_events, int, astloop)
OZ_HW_SYSCALL_DCL_5 (event_ast, OZ_Procmode, procmode, OZ_Handle, h_event, OZ_Astentry, astentry, void *, astparam, int, express)

#endif
