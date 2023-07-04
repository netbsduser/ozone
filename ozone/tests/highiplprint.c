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

#include "ozone.h"
#include "oz_knl_hw.h"
#include "oz_knl_printk.h"
#include "oz_knl_sdata.h"
#include "oz_knl_status.h"
#include "oz_sys_callknl.h"
#include "oz_util_start.h"

static uLong highiplprint_knl (OZ_Procmode cprocmode, void *dummy);

uLong oz_util_main (int argc, char *argv[])

{
  uLong sts;

  sts = oz_sys_callknl (highiplprint_knl, NULL);
  return (sts);
}

static uLong highiplprint_knl (OZ_Procmode cprocmode, void *dummy)

{
  OZ_Datebin now;
  uLong hi;

  hi = oz_hw_smplock_wait (&oz_s_smplock_hi);
  now = oz_hw_tod_getnow ();
  oz_knl_printk ("this is the date/time at a high ipl: %t\n", now);
  oz_hw_smplock_clr (&oz_s_smplock_hi, hi);
  return (OZ_SUCCESS);
}
