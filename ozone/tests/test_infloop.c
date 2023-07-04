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
/*  This program runs in an infinite loop				*/
/*  Its priority should decrease until it reaches 1			*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_knl_hw.h"
#include "oz_knl_status.h"
#include "oz_sys_handle_getinfo.h"
#include "oz_util_start.h"

uLong oz_util_main (int argc, char *argv[])

{
  uLong basepri, curprio, sts;
  OZ_Datebin now;

  OZ_Handle_item items[2] = { OZ_HANDLE_CODE_THREAD_BASEPRI, sizeof basepri, &basepri, 
                              OZ_HANDLE_CODE_THREAD_CURPRIO, sizeof curprio, &curprio };

  while (1) {
    now = oz_hw_timer_getnow ();
    sts = oz_sys_handle_getinfo (0, 2, items, NULL);
    if (sts != OZ_SUCCESS) {
      oz_sys_io_fs_printf (oz_util_h_error, "infloop: error %u getting priority info\n", sts);
      return (sts);
    }
    oz_sys_io_fs_printf (oz_util_h_output, "infloop: %t: base pri %3u, cur prio %3u\n", now, basepri, curprio);
  }
}
