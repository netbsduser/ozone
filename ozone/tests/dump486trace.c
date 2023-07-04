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

/************************************************************************/
/*									*/
/*  Dump oz_hw486_trace_... routine buffer				*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_crtl_malloc.h"
#include "oz_knl_status.h"
#include "oz_sys_callknl.h"
#include "oz_sys_io_fs.h"
#include "oz_util_start.h"

typedef struct { OZ_Datebin when;
                 void      *rtn;
                 uLong      p1, p2;
                 uLong      pad[3];
               } Tb;

extern Tb oz_hw486_trace_buf[];
extern uLong oz_hw486_trace_idx;

static Tb *tracebuf;
static uLong traceidx, tracesiz;

static uLong gettrace (OZ_Procmode cprocmode, void *dummy);

uLong oz_util_main (int argc, char *argv[])

{
  uLong i, j, sts;

  /* Allocate user-mode buffer to copy stuff to and make sure it's all faulted in */

  tracesiz = (uByte *)&oz_hw486_trace_idx - (uByte *)oz_hw486_trace_buf;
  tracebuf = malloc (tracesiz);
  memset (tracebuf, 0, tracesiz);

  /* Call kernel mode routine to copy kernel memory to user memory */

  sts = oz_sys_callknl (gettrace, NULL);
  if (sts != OZ_SUCCESS) oz_sys_io_fs_printf (oz_util_h_error, "error %u getting memory contents\n", sts);
  else {
    tracesiz /= sizeof *tracebuf;

    /* Dump out the user-mode copy of the kernel memory */

    for (i = traceidx; i -- > 0;) {
      if (traceidx - i > tracesiz) break;
      j = i % tracesiz;
      oz_sys_io_fs_printf (oz_util_h_output, "  %t  %8.8X  %8.8X  @  %p\n", 
	tracebuf[j].when, tracebuf[j].p1, tracebuf[j].p2, tracebuf[j].rtn);
    }
  }
  return (sts);
}

static uLong gettrace (OZ_Procmode cprocmode, void *dummy)

{
  asm volatile ("cli");
  traceidx = oz_hw486_trace_idx;
  memcpy (tracebuf, oz_hw486_trace_buf, tracesiz);
  asm volatile ("sti");
  return (OZ_SUCCESS);
}
