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

#ifndef _OZ_SYS_SPAWN_H
#define _OZ_SYS_SPAWN_H

#include "ozone.h"
#include "oz_knl_handle.h"

OZ_HW_SYSCALL_DCL_13 (spawn, OZ_Handle, h_job, 
                             const char *, image, 
                             OZ_Handle, h_input, 
                             OZ_Handle, h_output, 
                             OZ_Handle, h_error, 
                             OZ_Handle, h_init, 
                             OZ_Handle, h_exit, 
                             const char *, defdir, 
                             int, nparams, 
                             const char **, paramv, 
                             const char *, name, 
                             OZ_Handle *, thread_h_r, 
                             OZ_Handle *, process_h_r)

#endif
