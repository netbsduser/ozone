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
/*  Test the getpid, getppid, getpgrp routines				*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_knl_status.h"
#include "oz_sys_event.h"
#include "oz_sys_fork.h"
#include "oz_sys_io_fs_printf.h"
#include "oz_sys_thread.h"

static void printinfo (char *name);

int main ()

{
  OZ_Handle h_event_exit, h_thread_fork;
  uLong exitsts, sts;

  printinfo ("starting");

  sts = oz_sys_event_create (OZ_PROCMODE_KNL, "getpidtest fork", &h_event_exit);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printerror ("error %u creating event flag\n", sts);
    return (sts);
  }

  sts = oz_sys_fork (0, 0, 0, "getpidtest fork", &h_thread_fork);

  if (sts == OZ_SUBPROCESS) {
    printinfo ("forked");
    oz_sys_thread_exit (OZ_SUCCESS);
  }

  if (sts != OZ_SUCCESS) oz_sys_io_fs_printerror ("error %u forking\n", sts);
  else {
    while ((sts = oz_sys_thread_getexitsts (h_thread_fork, &exitsts)) == OZ_FLAGWASCLR) {
      oz_sys_event_wait (OZ_PROCMODE_KNL, h_event_exit, 0);
      oz_sys_event_set (OZ_PROCMODE_KNL, h_event_exit, 0, NULL);
    }
    if (sts != OZ_SUCCESS) oz_sys_io_fs_printerror ("error %u getting exit status\n", sts);
    else oz_sys_io_fs_printerror ("exit status %u\n", exitsts);
  }

  return (sts);
}

static void printinfo (char *name)

{
  int mypgrp, mypid, myppid;

  mypid  = getpid ();
  myppid = getppid ();
  mypgrp = getpgrp ();

  oz_sys_io_fs_printerror ("%s pid %d, ppid %d, pgrp %d\n", name, mypid, myppid, mypgrp);
}
