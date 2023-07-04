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
/*  Write a value to kernel memory					*/
/*									*/
/*	pokevirt <address> <value>					*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_knl_hw.h"
#include "oz_knl_status.h"
#include "oz_sys_callknl.h"
#include "oz_sys_condhand.h"
#include "oz_sys_io_fs_printf.h"
#include "oz_util_start.h"

static uLong *virtaddr;
static uLong value;

static uLong pokevirt_knl (OZ_Procmode cprocmode, void *dummy);
static uLong pokevirt_try (void *dummy);

uLong oz_util_main (int argc, char *argv[])

{
  int usedup;
  uLong sts;

  if (argc != 3) goto usage;

  virtaddr = (void *)oz_hw_atoz (argv[1], &usedup);
  if (argv[1][usedup] != 0) goto usage;
  value   = oz_hw_atoz (argv[2], &usedup);
  if (argv[2][usedup] != 0) goto usage;

  sts = oz_sys_callknl (pokevirt_knl, NULL);
  if (sts != OZ_SUCCESS) oz_sys_io_fs_printf (oz_util_h_error, "error %u poking virtual memory\n", sts);
  return (sts);

usage:
  oz_sys_io_fs_printf (oz_util_h_error, "usage: pokevirt <phyaddr> <value>\n");
  return (OZ_BADPARAM);
}

static uLong pokevirt_knl (OZ_Procmode cprocmode, void *dummy)

{
  return (oz_sys_condhand_try (pokevirt_try, NULL, oz_sys_condhand_rtnanysig, NULL));
}

static uLong pokevirt_try (void *dummy)

{
  *virtaddr = value;
  return (OZ_SUCCESS);
}
