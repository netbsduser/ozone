//+++2002-01-14
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
//---2002-01-14

/************************************************************************/
/*									*/
/*  Test file extensions						*/
/*									*/
/************************************************************************/

#include <errno.h>
#include <stdio.h>

#include "ozone.h"
#include "oz_io_fs.h"
#include "oz_knl_status.h"
#include "oz_sys_handle.h"
#include "oz_sys_io.h"
#include "oz_sys_io_fs.h"
#include "oz_sys_io_fs_printf.h"

int main (int argc, char *argv[])

{
  char name[256];
  int i, j;
  OZ_Handle h_iochan;
  OZ_IO_fs_create fs_create;
  uLong sts;

  if ((argc != 2) || (argv[1][strlen(argv[1])-1] != '/')) {
    printf ("usage: testfilext <disk:directory/>\n");
    return (-1);
  }

  memset (&fs_create, 0, sizeof fs_create);
  fs_create.name = name;
  fs_create.lockmode = OZ_LOCKMODE_PW;

  for (j = 0; j < 1000; j ++) {
    for (i = 0; i < 1000; i ++) {
      sprintf (name, "%s%dasdfasdf.%dqwerqwer", argv[1], i, j);
      printf ("%s\n", name);
      sts = oz_sys_io_fs_create (sizeof fs_create, &fs_create, 0, &h_iochan);
      if (sts != OZ_SUCCESS) {
        printf ("error %u creating %s\n", sts, name);
        return (sts);
      }
      sts = oz_sys_io_fs_printf (h_iochan, "this is file '%s'\n", name);
      if (sts != OZ_SUCCESS) {
        printf ("error %u writing %s\n", sts, name);
        return (sts);
      }
      sts = oz_sys_io (OZ_PROCMODE_KNL, h_iochan, 0, OZ_IO_FS_CLOSE, 0, NULL);
      if (sts != OZ_SUCCESS) {
        printf ("error %u closing %s\n", sts, name);
        return (sts);
      }
      sts = oz_sys_handle_release (OZ_PROCMODE_KNL, h_iochan);
      if (sts != OZ_SUCCESS) {
        printf ("error %u releasing handle\n", sts);
        return (sts);
      }
    }
  }

  return (0);
}
