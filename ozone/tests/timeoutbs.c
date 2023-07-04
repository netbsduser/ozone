//+++2003-11-18
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
//---2003-11-18

#include <ozone.h>
#include <oz_knl_status.h>
#include <oz_sys_callknl.h>
#include <oz_sys_io_fs_printf.h>
#include <oz_util_start.h>

static uLong doit (OZ_Procmode cprocmode, void *dummy);

uLong starttime, stoptime;

uLong oz_util_main (int argc, char *argv[])

{
  uLong sts;

  sts = oz_sys_callknl (doit, NULL);
  oz_sys_io_fs_printf (oz_util_h_output, "status %u\n", sts);
  oz_sys_io_fs_printf (oz_util_h_output, "starttime %X, stoptime %X, diff %u\n", starttime, stoptime, stoptime - starttime);
  return (sts);
}

static uLong doit (OZ_Procmode cprocmode, void *dummy)

{
  asm volatile (
	"cli\n"
	"rdtsc\n"
	"movl %eax,%ecx\n"
	"xorl %eax,%eax\n"
	"cli\n"
//	"outb $0x21\n"
//	"outb $0xA1\n"
	"rdtsc\n"
	"sti\n"
	"movl %ecx,starttime\n"
	"movl %eax,stoptime\n"
  );
  return (OZ_SUCCESS);
}
