//+++2001-10-06
//    Copyright (C) 2001, Mike Rieker, Beverly, MA USA
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
//---2001-10-06

/************************************************************************/
/*									*/
/*  Test PS/2 mouse - using the device driver				*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_io_mouse.h"
#include "oz_knl_status.h"
#include "oz_sys_handle.h"
#include "oz_sys_io.h"

int main ()

{
  uByte buff[256], triple[3];
  OZ_Handle h_mouse;
  OZ_IO_mouse_read mouse_read;
  uLong i, rlen, sts, t;

  sts = oz_sys_io_assign (OZ_PROCMODE_KNL, &h_mouse, "ps2mouse", OZ_LOCKMODE_EX);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printerror ("error %u assigning channel to ps2mouse\n", sts);
    return (sts);
  }

  memset (&mouse_read, 0, sizeof mouse_read);
  mouse_read.size = sizeof buff;
  mouse_read.buff = buff;
  mouse_read.rlen = &rlen;

  t = 0;

loop:
  sts = oz_sys_io (OZ_PROCMODE_KNL, h_mouse, 0, OZ_IO_MOUSE_READ, sizeof mouse_read, &mouse_read);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printerror ("error %u reading mouse data\n", sts);
    return (sts);
  }
  for (i = 0; i < rlen; i ++) {
    triple[t++] = buff[i];
    if (t == 3) {
      oz_sys_io_fs_printerror (" %c %c %c  %c%2.2x  %c%2.2x\n", 
                                 triple[0] & 0x01 ? 'l' : ' ', 
                                 triple[0] & 0x04 ? 'm' : ' ', 
                                 triple[0] & 0x02 ? 'r' : ' ',
                                 triple[0] & 0x10 ? '-' : ' ', triple[1], 
                                 triple[0] & 0x20 ? '-' : ' ', triple[2]);
      t = 0;
    }
  }
  goto loop;
}
