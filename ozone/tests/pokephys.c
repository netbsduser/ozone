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

/************************************************************************/
/*									*/
/*  Write a value to physical memory					*/
/*									*/
/*	pokephys <address> <value>					*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_knl_hw.h"
#include "oz_knl_status.h"
#include "oz_sys_callknl.h"
#include "oz_sys_io_fs_printf.h"
#include "oz_util_start.h"

#ifdef OZ_HW_TYPE_486
extern uLong oz_hw486_dynphypagbeg;
#endif

static OZ_Phyaddr phyaddr;
static uLong value;

static uLong pokephys_knl (OZ_Procmode cprocmode, void *dummy);

uLong oz_util_main (int argc, char *argv[])

{
  int usedup;
  uLong sts;

  if (argc != 3) goto usage;

  phyaddr = oz_hw_atoz (argv[1], &usedup);
  if (argv[1][usedup] != 0) goto usage;
  value   = oz_hw_atoz (argv[2], &usedup);
  if (argv[2][usedup] != 0) goto usage;

#ifdef OZ_HW_TYPE_486
  if ((phyaddr >> OZ_HW_L2PAGESIZE) < oz_hw486_dynphypagbeg) {
    oz_sys_io_fs_printf (oz_util_h_error, "phyaddr %X is in static area below phypage %X\n", phyaddr, oz_hw486_dynphypagbeg);
    return (OZ_BADPARAM);
  }
#endif

  sts = oz_sys_callknl (pokephys_knl, NULL);
  if (sts != OZ_SUCCESS) oz_sys_io_fs_printf (oz_util_h_error, "error %u poking physical memory\n", sts);
  return (sts);

usage:
  oz_sys_io_fs_printf (oz_util_h_error, "usage: pokephys <phyaddr> <value>\n");
  return (OZ_MISSINGPARAM);
}

static uLong pokephys_knl (OZ_Procmode cprocmode, void *dummy)

{
  int si;
  OZ_Mempage phypages[2];

  phypages[0] = phyaddr >> OZ_HW_L2PAGESIZE;
  phypages[1] = phypages[0] + 1;

  si = oz_hw_cpu_setsoftint (0);
  oz_hw_phys_movefromvirt (sizeof value, &value, phypages, phyaddr % (1 << OZ_HW_L2PAGESIZE));
  oz_hw_cpu_setsoftint (si);
  return (OZ_SUCCESS);
}
