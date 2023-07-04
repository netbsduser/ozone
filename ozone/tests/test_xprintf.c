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
#include "oz_knl_status.h"
#include "oz_sys_dateconv.h"
#include "oz_sys_xprintf.h"

int main ()

{
  char buff[32];

#if 00
  OZ_Datebin now;

  now.x = 0x12345678;
  now.y = 0;

  oz_sys_sprintf (sizeof buff, buff, "%#t", now);
  printf ("%s\n", buff);
#endif

  oz_sys_sprintf (sizeof buff, buff, "%d", 0x80000000);
  printf ("%s\n", buff);

  return (0);
}  
