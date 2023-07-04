//+++2002-05-10
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
//---2002-05-10

#include <stdio.h>

int main ()

{
  int a, b, c, d;
  long long aa, bb, cc, dd;

  for (a = -20; a <= 20; a += 40) {
    for (b = -7; b <= 7; b += 14) {
      aa = a;
      bb = b;
      c  = a % b;
      cc = aa % bb;
      d  = a / b;
      dd = aa / bb;
      printf ("%3d %% %2d = %2d %2lld    %3d / %2d = %2d %2lld\n", a, b, c, cc, a, b, d, dd);
    }
  }

  return (0);
}
