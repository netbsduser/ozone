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

#include "ozone.h"
#include "oz_crtl_malloc.h"
#include "oz_io_fs.h"
#include "oz_knl_status.h"
#include "oz_sys_io.h"
#include "oz_sys_io_fs.h"
#include "oz_util_start.h"

uLong oz_util_main (int argc, char *argv[])

{
  int usedup;
  OZ_Dbn svbn;
  OZ_Handle h_iochan;
  OZ_IO_fs_open fs_open;
  OZ_IO_fs_readblocks fs_readblocks;
  uLong sts, size;

  if (argc != 4) {
    oz_sys_io_fs_printf (oz_util_h_error, "usage: ix4kbuktest <filename> <size> <svbn>\n");
    return (OZ_MISSINGPARAM);
  }

  memset (&fs_open, 0, sizeof fs_open);
  fs_open.name = argv[1];
  fs_open.lockmode = OZ_LOCKMODE_CR;

  sts = oz_sys_io_fs_open (sizeof fs_open, &fs_open, 0, &h_iochan);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printf (oz_util_h_error, "error %u opening %s\n", sts, argv[1]);
    return (sts);
  }

  size = oz_hw_atoi (argv[2], &usedup);
  svbn = oz_hw_atoi (argv[3], &usedup);

  memset (&fs_readblocks, 0, sizeof fs_readblocks);
  fs_readblocks.size = size;
  fs_readblocks.buff = malloc (size);
  fs_readblocks.svbn = svbn;
  fs_readblocks.ix4kbuk = 1;

  sts = oz_sys_io (OZ_PROCMODE_KNL, h_iochan, 0, OZ_IO_FS_READBLOCKS, sizeof fs_readblocks, &fs_readblocks);
  oz_sys_io_fs_printf (oz_util_h_error, "ix4kbuktest read status %u\n", sts);
  return (sts);
}
