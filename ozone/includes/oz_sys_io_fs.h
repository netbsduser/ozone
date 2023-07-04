//+++2002-03-18
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
//---2002-03-18

#ifndef _OZ_SYS_IO_FS_H
#define _OZ_SYS_IO_FS_H

#define OZ_SYS_IO_FS_WILDSCAN_TERMINAL 0x1	// don't do any logical name translation
#define OZ_SYS_IO_FS_WILDSCAN_DELAYDIR 0x2	// delay directory name until contents have been processed
#define OZ_SYS_IO_FS_WILDSCAN_DIRLIST  0x4	// if just a directory name given, list out the whole directory

#include "ozone.h"

#include "oz_io_fs.h"
#include "oz_knl_handle.h"
#include "oz_knl_lock.h"

uLong oz_sys_io_fs_create (int fs_create_len, OZ_IO_fs_create *fs_create, int terminal, OZ_Handle *h_iochan_r);
uLong oz_sys_io_fs_create2 (int fs_create_len, OZ_IO_fs_create *fs_create, int terminal, const char *def_dir, OZ_Handle *h_iochan_r);
uLong oz_sys_io_fs_open (int fs_open_len, OZ_IO_fs_open *fs_open, int terminal, OZ_Handle *h_iochan_r);
uLong oz_sys_io_fs_open2 (int fs_open_len, OZ_IO_fs_open *fs_open, int terminal, const char *def_dir, OZ_Handle *h_iochan_r);
uLong oz_sys_io_fs_wildscan (const char *name, int terminal, uLong (*entry) (void *param, const char *devname, const char *wildcard, const char *instance, OZ_Handle h_ioch, uLong fileidsize, void *fileidbuff, OZ_FS_Subs *wildsubs, OZ_FS_Subs *instsubs), void *param);
uLong oz_sys_io_fs_wildscan2 (const char *name, int terminal, const char *def_dir, uLong (*entry) (void *param, const char *devname, const char *wildcard, const char *instance, OZ_Handle h_ioch, uLong fileidsize, void *fileidbuff, OZ_FS_Subs *wildsubs, OZ_FS_Subs *instsubs), void *param);
uLong oz_sys_io_fs_wildscan3 (const char *name, uLong options, const char *def_dir, uLong (*entry) (void *param, const char *devname, const char *wildcard, const char *instance, OZ_Handle h_ioch, uLong fileidsize, void *fileidbuff, OZ_FS_Subs *wildsubs, OZ_FS_Subs *instsubs), void *param);
uLong oz_sys_io_fs_parse (const char *name, int terminal, uLong (*entry) (void *param, const char *devname, const char *filname, OZ_Handle h_iochan), void *param);
uLong oz_sys_io_fs_parse2 (const char *name, int terminal, const char *def_dir, uLong (*entry) (void *param, const char *devname, const char *filname, OZ_Handle h_iochan), void *param);
uLong oz_sys_io_fs_parse3 (const char *name, int terminal, const char *def_dir, OZ_Procmode lnmprocmode, uLong (*entry) (void *param, const char *devname, const char *filname, OZ_Handle h_iochan), void *param);
uLong oz_sys_io_fs_assign (const char *devname, OZ_Lockmode lockmode, OZ_Handle *h_iochan_r);

#endif
