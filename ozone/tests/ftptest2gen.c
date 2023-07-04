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

#include <stdio.h>

int main ()

{
  int number, this1, this2;

  for (number = 0; number < 65536 * 4; number ++) {
    this1 = number / 16;
    this2 = number % 16;
    if ((this1 + 1) % (this2 + 4) == 0) { printf ("\n"); number ++; }
    else printf ("%c", '@' + this2 + (this1 % 8));
  }
  return (0);
}
