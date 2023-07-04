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
#include "oz_knl_hw.h"
#include "oz_knl_procmode.h"
#include "oz_knl_status.h"
#include "oz_sys_callknl.h"
#include "oz_sys_io_fs_printf.h"
#include "oz_util_start.h"

static uLong size;
static void *addr;

static uLong dumpmem_knl (OZ_Procmode cprocmode, void *dummy);

uLong oz_util_main (int argc, char *argv[])

{
  int usedup;
  uLong sts;

  if (argc != 3) goto usage;

  size = oz_hw_atoz (argv[1], &usedup);
  if ((usedup == 0) || (argv[1][usedup] != 0)) goto usage;
  addr = (void *)oz_hw_atoz (argv[2], &usedup);
  if ((usedup == 0) || (argv[2][usedup] != 0)) goto usage;

  sts = oz_sys_callknl (dumpmem_knl, NULL);
  if (sts != OZ_SUCCESS) oz_sys_io_fs_printf (oz_util_h_error, "error %u dumping memory\n", sts);
  return (sts);

usage:
  oz_sys_io_fs_printf (oz_util_h_error, "usage: dumpmem <size> <address>\n");
  return (OZ_MISSINGPARAM);
}

static uLong dumpmem_knl (OZ_Procmode cprocmode, void *dummy)

{
  if (!OZ_HW_READABLE (size, addr, OZ_PROCMODE_KNL)) return (OZ_ACCVIO);
  oz_sys_io_fs_dumpmem (size, addr);
  return (OZ_SUCCESS);
}
