//+++2003-03-01
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
//---2003-03-01

#ifndef _OZ_SYS_THREAD_H
#define _OZ_SYS_THREAD_H

#include "ozone.h"

#include "oz_knl_ast.h"
#include "oz_knl_handle.h"
#include "oz_knl_hw.h"
#include "oz_knl_procmode.h"
#include "oz_knl_thread.h"

OZ_HW_SYSCALL_DCL_11 (thread_create, OZ_Procmode, procmode, OZ_Handle, h_process, uLong, priority, OZ_Handle, h_initevent, OZ_Handle, h_exitevent, OZ_Mempage, stacksize, OZ_Thread_entry, thentry, void *, thparam, OZ_Astmode, knlastmode, const char *, name, OZ_Handle *, h_thread_r)
OZ_HW_SYSCALL_DCL_1 (thread_deqast, void *, capbv)
OZ_HW_SYSCALL_DCL_1 (thread_setast, OZ_Astmode, astmode)
uLong oz_sys_thread_exit (uLong status);
OZ_HW_SYSCALL_DCL_3 (thread_exiteh, uLong, status, OZ_Exhand_entry *, exhentry_r, void **, exhparam_r)
OZ_HW_SYSCALL_DCL_2 (thread_abort, OZ_Handle, h_thread, uLong, abortsts)
OZ_HW_SYSCALL_DCL_3 (thread_setseckeys, OZ_Handle, h_thread, uLong, seckeyssize, const void *, seckeysbuff)
OZ_HW_SYSCALL_DCL_3 (thread_setdefcresecattr, OZ_Handle, h_thread, uLong, secattrsize, const void *, secattrbuff)
OZ_HW_SYSCALL_DCL_3 (thread_setsecattr, OZ_Handle, h_thread, uLong, secattrsize, const void *, secattrbuff)
OZ_HW_SYSCALL_DCL_2 (thread_setbasepri, OZ_Handle, h_thread, uLong, basepri)
OZ_HW_SYSCALL_DCL_3 (thread_getname, OZ_Handle, h_thread, uLong, size, char *, buff)
OZ_HW_SYSCALL_DCL_2 (thread_getexitsts, OZ_Handle, h_thread, uLong *, exitsts_r)
OZ_HW_SYSCALL_DCL_2 (thread_getexitevent, OZ_Handle, h_thread, OZ_Handle *, h_event_r)
OZ_HW_SYSCALL_DCL_1 (thread_orphan, OZ_Handle, h_thread)
OZ_HW_SYSCALL_DCL_1 (thread_suspend, OZ_Handle, h_thread)
OZ_HW_SYSCALL_DCL_1 (thread_resume, OZ_Handle, h_thread)
OZ_HW_SYSCALL_DCL_2 (thread_getbyid, OZ_Threadid, threadid, OZ_Handle *, h_thread_r)
OZ_HW_SYSCALL_DCL_6 (thread_queueast, OZ_Procmode, procmode, OZ_Handle, h_thread, OZ_Astentry, astentry, void *, astparam, int, express, uLong, status)
uLong oz_sys_thread_newusrstk (void);

#endif
