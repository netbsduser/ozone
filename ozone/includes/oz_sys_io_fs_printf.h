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

#ifndef _OZ_SYS_IO_FS_PRINTF_H
#define _OZ_SYS_IO_FS_PRINTF_H

#include <stdarg.h>

#include "oz_knl_handle.h"

uLong oz_sys_io_fs_dumpmem (OZ_Handle h_iochan, uLong size, const void *buff);
void oz_sys_io_fs_printerror (const char *format, ...);
void oz_sys_io_fs_printerrorv (const char *format, va_list ap);
OZ_Handle oz_sys_io_fs_get_h_error (void);
uLong oz_sys_io_fs_printf (OZ_Handle h_iochan, const char *format, ...);
uLong oz_sys_io_fs_vprintf (OZ_Handle h_iochan, const char *format, va_list ap);

#endif
