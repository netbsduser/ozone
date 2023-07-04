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
#include "oz_knl_spte.h"
#include "oz_knl_status.h"
#include "oz_sys_io_fs_printf.h"
#include "oz_sys_callknl.h"
#include "oz_util_start.h"

static uLong doitknl (OZ_Procmode cprocmode, void *datarrayv);

uLong oz_util_main (int argc, char *argv[])

{
  uLong datarray[0x40], index, sts;

  sts = oz_sys_callknl (doitknl, datarray);
  oz_sys_io_fs_printf (oz_util_h_output, "final status: %u\n", sts);
  if (sts != OZ_SUCCESS) oz_sys_io_fs_printf (oz_util_h_error, "error %u mapping ioapic\n", sts);
  else {
    for (index = 0; index < 0x40; index += 4) {
      oz_sys_io_fs_printf (oz_util_h_output, "%8.8x %8.8x  %8.8x %8.8x : %2.2x\n", 
                           datarray[index+3], datarray[index+2], datarray[index+1], datarray[index+0], index);
    }
  }
  return (sts);
}

static uLong doitknl (OZ_Procmode cprocmode, void *datarrayv)

{
  int si;
  uLong *datarray, index, sts;
  void *sysvaddr;

  datarray = datarrayv;

  si  = oz_hw_cpu_setsoftint (0);
  sts = oz_knl_spte_alloc (1, &sysvaddr, NULL, NULL);
  if (sts == OZ_SUCCESS) {
    oz_hw_map_iopage (0xFEC00, sysvaddr);
    for (index = 0; index < 0x40; index ++) {
      ((uLong *)sysvaddr)[0] = index;
      datarray[index] = ((uLong *)sysvaddr)[4];
    }
  }
  oz_hw_cpu_setsoftint (si);
  return (sts);
}
