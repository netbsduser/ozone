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

#include "oz_sys_condhand.h"
#include "oz_sys_io.h"

static uLong condhand (void *chparam, int unwinding, OZ_Sigargs sigargs[], OZ_Mchargs *mchargs);
static void traceback (void *param, OZ_Mchargs *mchargs);

int _start ()

{
  int fd, rc;
  uLong status, sts;
  OZ_Handle coniochan;
  OZ_IO_console_write console_write;

  oz_sys_condhand_create (condhand, NULL);

  sts = oz_sys_io_assign (OZ_PROCMODE_USR, &coniochan, "console", OZ_LOCKMODE_CW);
  if (sts != OZ_SUCCESS) oz_sys_printkp (0, "y.c: error %u assigning channel to console\n", sts);
  else {
    memset (&console_write, 0, sizeof console_write);
    console_write.size = 7;
    console_write.buff = "Rarp!\n";
    sts = oz_sys_io_startwait (OZ_PROCMODE_USR, coniochan, 0, OZ_IO_CONSOLE_WRITE, sizeof console_write, &console_write);
    if (sts == OZ_STARTED) sts = status;
    if (sts != OZ_SUCCESS) oz_sys_printkp (0, "y.c: error %u rarping to console\n", sts);
  }

  fd = open ("OZ_INPUT", O_RDONLY);
  oz_sys_printkp (1, "y.c: open OZ_INPUT fd %d, errno %d> ", fd, errno);

  fd = open ("OZ_OUTPUT", O_WRONLY | O_APPEND);
  oz_sys_printkp (1, "y.c: open OZ_OUTPUT fd %d, errno %d> ", fd, errno);

  fd = open ("OZ_ERROR", O_WRONLY | O_APPEND);
  oz_sys_printkp (1, "y.c: open OZ_ERROR fd %d, errno %d> ", fd, errno);

  rc = printf ("(involuntary)\n");
  oz_sys_printkp (1, "y.c: printf return code %d, errno %d> ", rc, errno);

  oz_sys_printkp (0, "y.c: calling fflush for stdout\n");
  rc = fflush (stdout);
  oz_sys_printkp (1, "y.c: fflush return code %d, errno %d> ", rc, errno);

  return (OZ_SUCCESS);
}

static uLong condhand (void *chparam, int unwinding, OZ_Sigargs sigargs[], OZ_Mchargs *mchargs)

{
  if (!unwinding) {
    oz_sys_printkp (0, "y.c: Condition %u at %x ...\n", sigargs[1], mchargs -> eip);
    oz_hw_traceback (traceback, NULL);
    oz_sys_printkp (1, "y.c: unwinding> ");
  }
  return (OZ_UNWIND);
}

static void traceback (void *param, OZ_Mchargs *mchargs)

{
  oz_sys_printkp (0, "y.c:      %x    %x\n", mchargs -> eip, mchargs -> ebp);
}
