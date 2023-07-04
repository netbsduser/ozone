//+++2002-05-10
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
//---2002-05-10

/************************************************************************/
/*									*/
/*  Test file removes							*/
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

static uLong deletefile (void *dummy, const char *devname, const char *filename, OZ_Handle h_iochan);

int main (int argc, char *argv[])

{
  char name[256];
  int i, j, jswap, k;
  uLong sts;

  if ((argc != 2) || (argv[1][strlen(argv[1])-1] != '/')) {
    printf ("usage: testfilrem <disk:directory/>\n");
    return (-1);
  }

  for (k = 1; k <= 99; k ++) {
    for (j = 1; j <= 99; j ++) {
      jswap = (j / 10) + ((j % 10) * 10);
      for (i = 1; i <= 99; i ++) {
        sprintf (name, "%s%dasdfasdf.%dqwerqwer;%d", argv[1], i, k, jswap);
        printf ("%s\n", name);
        sts = oz_sys_io_fs_parse (name, 0, deletefile, NULL);
        if (sts != OZ_SUCCESS) {
          printf ("error %u removing %s\n", sts, name);
          return (sts);
        }
      }
    }
  }

  return (0);
}

static uLong deletefile (void *dummy, const char *devname, const char *filename, OZ_Handle h_iochan)

{
  uLong sts;
  OZ_Handle h_file;
  OZ_IO_fs_remove fs_remove;

  if (h_iochan != 0) return (OZ_NOSUCHFILE);

  sts = oz_sys_io_fs_assign (devname, OZ_LOCKMODE_NL, &h_file);
  if (sts == OZ_SUCCESS) {
    memset (&fs_remove, 0, sizeof fs_remove);
    fs_remove.name = filename;
    sts = oz_sys_io (OZ_PROCMODE_KNL, h_file, 0, OZ_IO_FS_REMOVE, sizeof fs_remove, &fs_remove);
    oz_sys_handle_release (OZ_PROCMODE_KNL, h_file);
  }
  return (sts);
}
