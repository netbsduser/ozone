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

#ifndef _OZ_SYS_IMAGE_H
#define _OZ_SYS_IMAGE_H

#include "ozone.h"

#include "oz_knl_handle.h"
#include "oz_knl_procmode.h"

OZ_HW_SYSCALL_DCL_6 (image_load, OZ_Procmode, procmode, char *, imagename, int, sysimage, void **, baseaddr_r, void **, startaddr_r, OZ_Handle *, h_image_r)

#endif
