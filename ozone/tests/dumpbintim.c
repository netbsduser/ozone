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

int main ()

{
  OZ_Datebin now;

  now = oz_hw_timer_getnow ();

  oz_sys_io_fs_printerror ("the time is now %8.8X.%8.8X = %t\n", (uLong)(now >> 32), (uLong)now, now);

  now = 0x01C223349251F269ULL;
  oz_sys_io_fs_printerror ("    then it was %8.8X.%8.8X = %t\n", (uLong)(now >> 32), (uLong)now, now);
  return (1);
}
