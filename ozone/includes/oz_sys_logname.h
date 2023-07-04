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

#ifndef _OZ_SYS_LOGNAME_H
#define _OZ_SYS_LOGNAME_H

#include "oz_knl_handle.h"
#include "oz_knl_hw.h"
#include "oz_knl_procmode.h"

OZ_HW_SYSCALL_DCL_7 (logname_create, OZ_Handle, h_lognamtbl, const char *, name, OZ_Procmode, procmode, uLong, lognamatr, uLong, nvalues, OZ_Logvalue, values[], OZ_Handle *, h_logname_r);
OZ_HW_SYSCALL_DCL_7 (logname_lookup, OZ_Handle, h_lognamtbl, OZ_Procmode, procmode, const char *, name, OZ_Procmode *, procmode_r, uLong *, lognamatr_r, uLong *, nvalues_r, OZ_Handle *, h_logname_r);
OZ_HW_SYSCALL_DCL_10 (logname_getattr, OZ_Handle, h_logname, uLong, namesize, char *, namebuff, uLong *, namerlen, OZ_Procmode *, procmode_r, uLong *, lognamatr_r, uLong *, nvalues_r, OZ_Handle *, h_lognamtbl_r, uLong, index, uLong *, logvalatr_r);
OZ_HW_SYSCALL_DCL_9 (logname_getval, OZ_Handle, h_logname, uLong, index, uLong *, logvalatr_r, uLong, size, char *, buff, uLong *, rlen, OZ_Handle, *h_object_r, OZ_Objtype, objtype, OZ_Objtype *, objtype_r);
OZ_HW_SYSCALL_DCL_2 (logname_gettblent, OZ_Handle, h_lognamtbl, OZ_Handle *, h_logname_r);
OZ_HW_SYSCALL_DCL_1 (logname_delete, OZ_Handle, h_logname);

#endif
