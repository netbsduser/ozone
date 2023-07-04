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

#include "ozone.h"
#include "oz_crtl_fio.h"
#include "oz_knl_status.h"
#include "oz_sys_io_fs_printf.h"

int main ()

{
  char buff[256];
  int i;

  oz_sys_io_fs_printerror ("\n>");
  while (fgets (buff, sizeof buff, stdin) != NULL) {
    for (i = 0; buff[i] != 0; i ++) oz_sys_io_fs_printerror (" %2.2x", buff[i] & 0xff);
    oz_sys_io_fs_printerror ("\n>");
  }

  return (OZ_SUCCESS);
}
