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

#ifndef _OZ_SYSCALL_IO_H
#define _OZ_SYSCALL_IO_H

#include "ozone.h"

#include "oz_knl_ast.h"
#include "oz_knl_devio.h"
#include "oz_knl_handle.h"
#include "oz_knl_lock.h"
#include "oz_knl_procmode.h"

OZ_HW_SYSCALL_DCL_3 (io_alloc, const char *, devname, OZ_Handle, h_alloc, OZ_Objtype, objtype)
OZ_HW_SYSCALL_DCL_3 (io_realloc, const char *, devname, OZ_Handle, h_alloc, OZ_Objtype, objtype)
OZ_HW_SYSCALL_DCL_1 (io_dealloc, const char *, devname)
OZ_HW_SYSCALL_DCL_4 (io_assign, OZ_Procmode, procmode, OZ_Handle *, h_iochan_r, const char *, devname, OZ_Lockmode, lockmode)
OZ_HW_SYSCALL_DCL_4 (io_chancopy, OZ_Procmode, procmode, OZ_Handle, h_iochan, OZ_Lockmode, lockmode, OZ_Handle *, h_iochan_r)
OZ_HW_SYSCALL_DCL_2 (io_abort, OZ_Procmode, procmode, OZ_Handle, h_iochan);
uLong oz_sys_io (OZ_Procmode procmode, OZ_Handle h_iochan, OZ_Handle h_event, uLong funcode, uLong as, void *ap);
OZ_HW_SYSCALL_DCL_6 (io_wait, OZ_Procmode, procmode, OZ_Handle, h_iochan, volatile uLong *, status_r, uLong, funcode, uLong, as, void *, ap);
OZ_HW_SYSCALL_DCL_0 (io_waitagain);
OZ_HW_SYSCALL_DCL_1 (io_waitsetef, Long, value);
OZ_HW_SYSCALL_DCL_9 (io_start, OZ_Procmode, procmode, OZ_Handle, h_iochan, volatile uLong *, status_r, OZ_Handle, h_event, OZ_Astentry, astentry, void *, astparam, uLong, funcode, uLong, as, void *, ap)
OZ_HW_SYSCALL_DCL_3 (iochan_getunitname, OZ_Handle, h_iochan, uLong, size, char *, buff)
OZ_HW_SYSCALL_DCL_3 (iochan_getclassname, OZ_Handle, h_iochan, uLong, size, char *, buff)
uLong oz_syscall_iosel_start ();

#endif
