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

#include "ozone.h"
#include "oz_io_fs.h"
#include "oz_knl_status.h"
#include "oz_sys_handle.h"
#include "oz_sys_io.h"
#include "oz_sys_io_fs.h"
#include "oz_util_start.h"

uLong oz_util_main (int argc, char *argv[])

{
  uLong sts;
  OZ_Dbn nblocks;
  OZ_Handle h_chan1;
  OZ_IO_fs_create fs_create;
  OZ_IO_fs_extend fs_extend;

  if (argc != 2) {
    oz_sys_io_fs_printerror ("usage: exttest <filename>\n");
    return (OZ_MISSINGPARAM);
  }

  memset (&fs_create, 0, sizeof fs_create);
  fs_create.lockmode = OZ_LOCKMODE_CW;

  fs_create.name = argv[1];
  sts = oz_sys_io_fs_create (sizeof fs_create, &fs_create, 0, &h_chan1);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printerror ("error %u creating %s\n", sts, fs_create.name);
    return (sts);
  }

  memset (&fs_extend, 0, sizeof fs_extend);
  nblocks = 0;
extloop:
  nblocks ++;

  if (nblocks % 100 == 0) oz_sys_io_fs_printerror ("nblocks %u\n", nblocks);

  fs_extend.nblocks = nblocks;
  sts = oz_sys_io (OZ_PROCMODE_KNL, h_chan1, 0, 
                   OZ_IO_FS_EXTEND, sizeof fs_extend, &fs_extend);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printerror ("error %u extending %s to %u blocks\n", sts, fs_create.name, nblocks);
    return (sts);
  }

  goto extloop;
}
