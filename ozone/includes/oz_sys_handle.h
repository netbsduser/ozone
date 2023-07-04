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

#ifndef _OZ_SYSCALL_HANDLE_H
#define _OZ_SYSCALL_HANDLE_H

#include "ozone.h"

#include "oz_knl_handle.h"
#include "oz_knl_hw.h"
#include "oz_knl_procmode.h"

OZ_HW_SYSCALL_DCL_3 (handle_next, OZ_Procmode, procmode, OZ_Handle, handle, OZ_Handle *, handle_r)
OZ_HW_SYSCALL_DCL_3 (handle_setthread, OZ_Procmode, procmode, OZ_Handle, h, OZ_Handle, h_thread)
OZ_HW_SYSCALL_DCL_2 (handle_release, OZ_Procmode, procmode, OZ_Handle, h)
OZ_HW_SYSCALL_DCL_4 (handle_retrieve, OZ_Handle, h_remote, OZ_Handle, h_process, uLong, secaccmsk, OZ_Handle *, h_local_r)

#include "oz_sys_handle_getinfo.h"

#endif
