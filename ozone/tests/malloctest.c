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
#include "oz_knl_hw.h"
#include "oz_knl_status.h"
#include "oz_util_start.h"

void *malloc (int size);
void free (void *buff);

uLong oz_util_main (int argc, char *argv[])

{
  int blocksize, i, quantity;
  OZ_Datebin delta, start, stop;
  void *temp;

  if (argc != 3) {
    oz_sys_io_fs_printf (oz_util_h_error, "usage: malloctest <size> <number>\n");
    return (OZ_MISSINGPARAM);
  }

  blocksize = atoi (argv[1]);
  quantity  = atoi (argv[2]);

  start = oz_hw_tod_getnow ();
  for (i = 0; i < quantity; i ++) {
    temp = malloc (blocksize);
    free (temp);
  }
  stop = oz_hw_tod_getnow ();
  oz_sys_io_fs_printf (oz_util_h_error, "start %t, stop %t\n", start, stop);
  delta = stop - start;
  oz_sys_io_fs_printf (oz_util_h_error, "elapsed time %#t\n", delta);

  return (OZ_SUCCESS);
}
