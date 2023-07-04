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
int main ()
{
  char buff[4096];
  int i, n, q;

  n = 0;
  while (gets (buff) != NULL) {
    n ++;
    q = 0;
    for (i = 0; buff[i] != 0; i ++) if (buff[i] == '"') q ++;
    if (q & 1) printf ("%6d: %s\n", n, buff);
  }
  return (0);
}
