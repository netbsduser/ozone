//+++2003-11-18
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
//---2003-11-18

#include <stdio.h>

static void fixulong (void *xul)

{
  int i;
  unsigned char *xub;

  xub = xul;
  for (i = 0; i < 4; i ++) {
    if ((xub[i] < ' ') || (xub[i] >= 127)) xub[i] = '.';
  }
}

int main ()

{
  unsigned int basindex, index, maxidx, outeax, outebx, outecx, outedx;

  for (basindex = 0;; basindex += 0x80000000) {
    maxidx = basindex;
    for (index = basindex; index <= maxidx; index ++) {
      asm ("cpuid" : "=a" (outeax), "=b" (outebx), "=c" (outecx), "=d" (outedx) : "a" (index));
      if (index == basindex) maxidx = outeax;
      printf ("[%8.8X]: %8.8X %8.8X %8.8X %8.8X\n", index, outeax, outebx, outecx, outedx);
      fixulong (&outeax);
      fixulong (&outebx);
      fixulong (&outecx);
      fixulong (&outedx);
      printf ("         %4.4s %4.4s %4.4s %4.4s\n", &outeax, &outebx, &outecx, &outedx);
    }
    if (basindex == 0x80000000) break;
  }
  return (0);
}
