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
#include "oz_io_fs.h"
#include "oz_knl_status.h"
#include "oz_sys_io.h"
#include "oz_sys_io_fs.h"
#include "oz_util_start.h"

uLong oz_util_main (int argc, char *argv[])

{
  char buff[4096];
  OZ_Handle h_filechan, h_nullchan;
  OZ_IO_fs_open fs_open;
  OZ_IO_fs_readrec fs_readrec;
  OZ_IO_fs_writerec fs_writerec;
  uLong rlen, sts;

  if (argc != 2) {
    oz_sys_io_fs_printf (oz_util_h_error, "usage: copynull <filename>\n");
    return (OZ_MISSINGPARAM);
  }

  sts = oz_sys_io_assign (OZ_PROCMODE_KNL, &h_nullchan, "null", OZ_LOCKMODE_CW);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printf (oz_util_h_error, "error %u assigning channel to null:\n", sts);
    return (sts);
  }

  memset (&fs_open, 0, sizeof fs_open);
  fs_open.name = argv[1];
  fs_open.lockmode = OZ_LOCKMODE_CR;
  sts = oz_sys_io_fs_open (sizeof fs_open, &fs_open, 0, &h_filechan);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printf (oz_util_h_error, "error %u opening %s\n", sts, argv[1]);
    return (sts);
  }

  memset (&fs_readrec, 0, sizeof fs_readrec);
  fs_readrec.size = sizeof buff;
  fs_readrec.buff = buff;
  fs_readrec.rlen = &fs_writerec.size;

  memset (&fs_writerec, 0, sizeof fs_writerec);
  fs_writerec.buff = buff;

  while ((sts = oz_sys_io (OZ_PROCMODE_KNL, h_filechan, 0, OZ_IO_FS_READREC, sizeof fs_readrec, &fs_readrec)) == OZ_SUCCESS) {
    sts = oz_sys_io (OZ_PROCMODE_KNL, h_nullchan, 0, OZ_IO_FS_WRITEREC, sizeof fs_writerec, &fs_writerec);
    if (sts != OZ_SUCCESS) {
      oz_sys_io_fs_printf (oz_util_h_error, "error %u writing null:\n", sts);
      return (sts);
    }
  }
  if (sts == OZ_ENDOFFILE) sts = OZ_SUCCESS;
  else oz_sys_io_fs_printf (oz_util_h_error, "error %u reading %s\n", sts, argv[1]);
  return (sts);
}
