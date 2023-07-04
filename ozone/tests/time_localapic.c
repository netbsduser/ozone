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

#include "ozone.h"
#include "oz_knl_status.h"
#include "oz_sys_io_fs_printf.h"
#include "oz_sys_callknl.h"
#include "oz_util_start.h"

static uLong doitknl (OZ_Procmode cprocmode, void *dummy);

static uLong starttime, stoptime;

uLong oz_util_main (int argc, char *argv[])

{
  uLong sts;

  sts = oz_sys_callknl (doitknl, NULL);
  oz_sys_io_fs_printf (oz_util_h_output, "final status: %u\n", sts);
  if (sts == OZ_SUCCESS) oz_sys_io_fs_printf (oz_util_h_error, "elapsed time %u\n", stoptime - starttime);
  return (sts);
}

static uLong doitknl (OZ_Procmode cprocmode, void *dummy)

{
  asm volatile ("cli\n\t"
                "rdtsc\n\t"
                "movl %eax,%ebx\n\t"
                "movl 0x1080,%eax\n\t"
                "movl %eax,0x1080\n\t"
                "rdtsc\n\t"
                "sti\n\t"
                "movl %eax,stoptime\n\t"
                "movl %ebx,starttime");
  return (OZ_SUCCESS);
}
