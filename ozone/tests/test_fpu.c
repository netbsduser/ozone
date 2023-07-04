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
/*  Test floating point context switching				*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_io_fs.h"
#include "oz_knl_status.h"
#include "oz_sys_condhand.h"
#include "oz_sys_event.h"
#include "oz_sys_thread.h"
#include "oz_util_start.h"

/**#define NOFPUSE**/

#define PRINT /** oz_sys_io_fs_printerror ( **/ oz_sys_printkp (0,
#define MAXTHREADS 16

static char threadnames[MAXTHREADS*32];

static uLong testit (void *dummy);

uLong oz_util_main (int argc, char *argv[])

{
  int i, n;
  uLong exitsts, sts;
  OZ_Handle h_exitevent, h_threads[MAXTHREADS];

  if (argc != 2) {
    oz_sys_io_fs_printerror ("usage: fptest <nthreads>\n");
    return (OZ_MISSINGPARAM);
  }
  n = oz_hw_atoi (argv[1], NULL);
  if (n > MAXTHREADS) n = MAXTHREADS;

  sts = oz_sys_event_create (OZ_PROCMODE_KNL, "fptest exit", &h_exitevent);
  if (sts != OZ_SUCCESS) oz_sys_condhand_signal (2, sts, 0);

  /* Create more threads than we could have for cpus */

  for (i = 1; i < n; i ++) {
    oz_sys_sprintf (32, threadnames + (i * 32), "fptest %d", i);
    sts = oz_sys_thread_create (OZ_PROCMODE_KNL, 0, 0, 0, 0, 
                                0, h_exitevent, 0, 
                                testit, threadnames + (i * 32), 
                                OZ_ASTMODE_ENABLE, threadnames + (i * 32), 
                                h_threads + i);
    if (sts != OZ_SUCCESS) {
      oz_sys_io_fs_printerror ("error %u starting thread %d\n", sts, i);
    }
  }

  /* Do the test in this thread, too */

  oz_sys_sprintf (32, threadnames, "fptest main");
  testit (threadnames);

  /* Wait for threads to exit */

#if 00
waitloop:
  for (i = 1; i < n; i ++) {
    if (h_threads[i] == 0) continue;
    sts = oz_sys_thread_getexitsts (h_threads[i], &exitsts);
    if (sts == OZ_FLAGWASCLR) {
      oz_sys_event_wait (OZ_PROCMODE_KNL, h_exitevent, 0);
      oz_sys_event_set (OZ_PROCMODE_KNL, h_exitevent, 0, NULL);
      goto waitloop;
    }
    if (sts != OZ_SUCCESS) {
      PRINT "fptest: error %u waiting for thread %d\n", sts, i);
      return (sts);
    }
    h_threads[i] = 0;
    if (exitsts != OZ_SUCCESS) {
      PRINT "fptest: thread %u exit status %u\n", i, exitsts);
    }
  }
#endif

  PRINT "fptest: exiting\n");

  return (OZ_SUCCESS);
}

static uLong testit (void *threadname)

{
#ifndef NOFPUSE
  double a;
#endif
  int i, j;
  Long id;

  PRINT "%s: stack %p\n", threadname, &id);

  /* Loop through incrementing a and i in step with each other.  Any given */
  /* thread should be interrupted by the others and it should move from cpu */
  /* to cpu. */

#ifdef NOFPUSE
  j = 0;
#else
  a = 0.0;
#endif
  for (i = 0; i < 50000000; i ++) {
    if ((i % 1000000) == 0) PRINT "%s: %d\n", threadname, i);
#ifndef NOFPUSE
    j = a;
#endif
    if (i != j) {
      PRINT "%s: i = %d, a = %d\n", threadname, i, j);
      /** oz_hw_debug_bpt (); **/
#ifdef NOFPUSE
      j = i;
#else
      a = i;
#endif
    }
#ifdef NOFPUSE
    j ++;
#else
    a ++;
#endif
  }

  PRINT "%s: exiting\n", threadname);

  return (OZ_SUCCESS);
}
