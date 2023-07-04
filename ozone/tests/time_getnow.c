//+++2002-01-14
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
//---2002-01-14

#include <stdio.h>
#include "ozone.h"
#include "oz_knl_hw.h"

int main ()

{
  int i;
  OZ_Datebin now;
  uLong avg, deltas[10000], min, max;

  now = oz_hw_timer_getnow ();
  for (i = 0; i < 10000; i ++) {
    deltas[i] = oz_hw_timer_getnow () - now;
    now += deltas[i];
  }

  avg = min = max = deltas[0];
  for (i = 1; i < 10000; i ++) {
    if (deltas[i] < min) min = deltas[i];
    if (deltas[i] > max) max = deltas[i];
    avg += deltas[i];
  }
  printf ("avg %u, min %u, max %u\n", avg / 10000, min, max);
  return (0);
}
