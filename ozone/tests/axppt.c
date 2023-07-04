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

typedef unsigned long long uQuad;

int main ()

{
  char c, inbuf[32];
  int i;
  uQuad datava, l1pteidx, l1pteva, l2pteidx, l2pteva, l3pteidx, l3pteva;

  while (1) {
    printf ("\nData VA: ");
    gets (inbuf);
    datava = 0;
    for (i = 0; (c = inbuf[i]) != 0; i ++) {
      if ((c >= '0') && (c <= '9')) c -= '0';
      else if ((c >= 'a') && (c <= 'f')) c -= 'a' - 10;
      else if ((c >= 'A') && (c <= 'F')) c -= 'A' - 10;
      else break;
      datava <<= 4;
      datava  += c;
    }
    if (c == 0) {
      datava  &= 0x7FFFFFFFFFF;	// va's only have 43 bits

      l3pteidx = datava >> 13;	// index of the L3 pte
      l2pteidx = datava >> 23;	// index of the L2 pte
      l1pteidx = datava >> 33;	// index of the L1 pte

      l1pteva  = 0x200802000 + l1pteidx * 8;
      l2pteva  = 0x200800000 + l2pteidx * 8;
      l3pteva  = 0x200000000 + l3pteidx * 8;

      printf ("data %16llX  l1pteva %16llX  l2pteva %16llX  l3pteva %16llX\n", datava, l1pteva, l2pteva, l3pteva);
    }
  }
}
