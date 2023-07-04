//+++2001-10-06
//    Copyright (C) 2001, Mike Rieker, Beverly, MA USA
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
//---2001-10-06

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

#include "ozone.h"

#include "oz_io_console.h"

#include "oz_knl_handle.h"
#include "oz_knl_procmode.h"
#include "oz_knl_status.h"

#include "oz_sys_io.h"

int main (int argc, char *argv[])

{
  int fd, rc;
  uLong status, sts;
  OZ_Handle coniochan;
  OZ_IO_console_write console_write;

  sts = oz_sys_io_assign (OZ_PROCMODE_USR, &coniochan, "console", OZ_LOCKMODE_CW);
  if (sts != OZ_SUCCESS) oz_sys_printkp (0, "z.c: error %u assigning channel to console\n", sts);
  else {
    memset (&console_write, 0, sizeof console_write);
    console_write.size = 7;
    console_write.buff = "Rarp!\n";
    sts = oz_sys_io (OZ_PROCMODE_USR, coniochan, 0, OZ_IO_CONSOLE_WRITE, sizeof console_write, &console_write);
    if (sts != OZ_SUCCESS) oz_sys_printkp (0, "z.c: error %u rarping to console\n", sts);
  }

  rc = printf ("(involuntary)\n");
  oz_sys_printkp (1, "z.c: printf return code %d, errno %d> ", rc, errno);

  return (OZ_SUCCESS);
}
