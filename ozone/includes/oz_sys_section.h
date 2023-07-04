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

#ifndef _OZ_SYS_SECTION_H
#define _OZ_SYS_SECTION_H

#include "ozone.h"

#include "oz_knl_handle.h"
#include "oz_knl_hw.h"
#include "oz_knl_procmode.h"
#include "oz_knl_section.h"

OZ_HW_SYSCALL_DCL_6 (section_create, OZ_Procmode, procmode, OZ_Handle, h_file, OZ_Mempage, npages, OZ_Dbn, vbnstart, OZ_Section_type, sectype, OZ_Handle *, h_section_r)
OZ_HW_SYSCALL_DCL_4 (section_setpageprot, OZ_Mempage, npages, OZ_Mempage, svpage, OZ_Hw_pageprot, pageprot, OZ_Hw_pageprot *, pageprot_r)

#endif
