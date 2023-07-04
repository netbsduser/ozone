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
#include "oz_knl_process.h"
#include "oz_knl_thread.h"
#include "oz_sys_callknl.h"
#include "oz_sys_io_fs_printf.h"
#include "oz_util_start.h"

uLong oz_util_main (int argc, char *argv[])

{
  uLong sts;

  sts = oz_sys_callknl (ts_knl, NULL);
  return (sts);
}

static uLong ts_knl (OZ_Procmode cprocmode, void *dummy)

{
  OZ_Process *lastprocess, *process;
  OZ_Thread *lastthread, *thread;

  process = NULL;
processloop:
  lastprocess = process;
  process = oz_knl_process_getnext (lastprocess, NULL);
  if (lastprocess != NULL) oz_knl_process_increfc (lastprocess, -1);
  if (process == NULL) return (OZ_SUCCESS);
  oz_sys_io_fs_printf ("process %p %s\n", process, oz_knl_process_getname (process));
  thread = NULL;
threadloop:
  lastthread = thread;
  thread = oz_knl_thread_getnext (lastthread, oz_knl_process_getthreadqp (process));
  if (lastthread != NULL) oz_knl_thread_increfc (lastthread, -1);
  if (thread == NULL) goto processloop;
  oz_sys_io_fs_printf ("  thread %p %s\n", thread, oz_knl_thread_getname (thread));
  goto threadloop;
}
