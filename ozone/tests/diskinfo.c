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
#include "oz_io_disk.h"
#include "oz_knl_status.h"
#include "oz_sys_io.h"
#include "oz_util_start.h"

uLong oz_util_main (int argc, char *argv[])

{
  OZ_Handle h_iochan;
  OZ_IO_disk_getinfo1 disk_getinfo1;
  uLong sts;

  if (argc != 2) {
    oz_sys_io_fs_printf (oz_util_h_error, "usage: diskinfo <diskdevice>\n");
    return (OZ_MISSINGPARAM);
  }

  sts = oz_sys_io_assign (OZ_PROCMODE_KNL, &h_iochan, argv[1], OZ_LOCKMODE_NL);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printf (oz_util_h_error, "error %u assigning channel to %s\n", sts, argv[1]);
    return (sts);
  }

  memset (&disk_getinfo1, 0, sizeof disk_getinfo1);
  sts = oz_sys_io (OZ_PROCMODE_KNL, h_iochan, 0, OZ_IO_DISK_GETINFO1, sizeof disk_getinfo1, &disk_getinfo1);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printf (oz_util_h_error, "error %u getting disk %s info\n", sts, argv[1]);
  } else {
    oz_sys_io_fs_printf (oz_util_h_output, "           blocksize: %u\n", disk_getinfo1.blocksize);
    oz_sys_io_fs_printf (oz_util_h_output, "         totalblocks: %u\n", disk_getinfo1.totalblocks);
    oz_sys_io_fs_printf (oz_util_h_output, "  parthoststartblock: %u\n", disk_getinfo1.parthoststartblock);
    oz_sys_io_fs_printf (oz_util_h_output, "     parthostdevname: %s\n", disk_getinfo1.parthostdevname);
    oz_sys_io_fs_printf (oz_util_h_output, "            bufalign: %u\n", disk_getinfo1.bufalign);
    oz_sys_io_fs_printf (oz_util_h_output, "         ramdisk_map: %p\n", disk_getinfo1.ramdisk_map);
  }

  return (sts);
}
