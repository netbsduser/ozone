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

/************************************************************************/
/*									*/
/*  This program tests the fork routine					*/
/*									*/
/*  It forks 'NFORKS' times.  Each fork prints out a message then 	*/
/*  exits.  The main program waits for each to exit and prints out 	*/
/*  their exits statuses then exits.					*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_knl_status.h"
#include "oz_sys_event.h"
#include "oz_sys_fork.h"
#include "oz_sys_thread.h"
#include "oz_util_start.h"

#define NFORKS 3
#define NCOUNT 1000

uLong oz_util_main (int argc, char *argv[])

{
  char forkname[16];
  int i;
  OZ_Handle h_exitevent, h_threads[NFORKS];
  uLong exitsts, sts;

  static Long counter = 0;

  sts = oz_sys_event_create (OZ_PROCMODE_KNL, "fork exiting", &h_exitevent);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printf (oz_util_h_error, "forktest: error %u creating event flag\n", sts);
    return (sts);
  }

  for (i = 0; i < NFORKS; i ++) {
    sprintf (forkname, "fork_%d", i);
    sts = oz_sys_fork (0, 0, h_exitevent, forkname, h_threads + i);
    if (sts == OZ_SUBPROCESS) {
      oz_sys_io_fs_printf (oz_util_h_error, "forktest: this is forked process %s\n", forkname);
      for (i = 0; i < NCOUNT; i ++) {
        oz_hw_atomic_inc_long (&counter, 1);
      }
      oz_sys_io_fs_printf (oz_util_h_error, "forktest: %s final count %d\n", forkname, counter);
      oz_sys_thread_exit (OZ_SUCCESS);
    }
    if (sts != OZ_SUCCESS) {
      oz_sys_io_fs_printf (oz_util_h_error, "forktest: error %u forking %s\n", sts, forkname);
      return (sts);
    }
  }

  for (i = 0; i < NFORKS; i ++) {
    while ((sts = oz_sys_thread_getexitsts (h_threads[i], &exitsts)) == OZ_FLAGWASCLR) {
      oz_sys_event_wait (OZ_PROCMODE_KNL, h_exitevent, 0);
      oz_sys_event_set (OZ_PROCMODE_KNL, h_exitevent, 0, NULL);
    }
    if (sts != OZ_SUCCESS) {
      oz_sys_io_fs_printf (oz_util_h_error, "forktest: error %u waiting for forked process %d\n", sts, i);
      return (sts);
    }
    oz_sys_io_fs_printf (oz_util_h_error, "forktest: forked process %d exited with status %u\n", i, exitsts);
  }

  return (OZ_SUCCESS);
}
