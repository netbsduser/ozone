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
/*  Print out the measured quicky frequency				*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_knl_sdata.h"
#include "oz_knl_status.h"
#include "oz_sys_callknl.h"
#include "oz_util_start.h"

static OZ_Datebin boottime, now;
static uLong quickies;

static uLong getknlstuff (OZ_Procmode cprocmode, void *dummy);

uLong oz_util_main (int argc, char *argv[])

{
  uLong sts;
  uQuad ticksperquicky;

  sts = oz_sys_callknl (getknlstuff, NULL);
  if (sts != OZ_SUCCESS) oz_sys_io_fs_printf (oz_util_h_output, "error %u getting stuff\n", sts);
  else {
    oz_sys_io_fs_printf (oz_util_h_output, "      boottime %t\n", boottime);
    oz_sys_io_fs_printf (oz_util_h_output, "           now %t\n", now);
    oz_sys_io_fs_printf (oz_util_h_output, "      quickies %u\n", quickies);
    if (quickies != 0) {
      ticksperquicky  = now - boottime;
      ticksperquicky /= quickies;
      oz_sys_io_fs_printf (oz_util_h_output, "ticksperquicky %u\n", (uLong)ticksperquicky);
    }
  }
  return (sts);
}

static uLong getknlstuff (OZ_Procmode cprocmode, void *dummy)

{
  boottime = oz_s_boottime;
  now      = oz_hw_tod_getnow ();
  quickies = oz_s_quickies;
  return (OZ_SUCCESS);
}
