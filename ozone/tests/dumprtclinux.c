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

/************************************************************************/
/*									*/
/*  Dump out RTC ram							*/
/*									*/
/************************************************************************/

#include <stdio.h>

static unsigned char rtcram[384];

int main (int argc, char *argv[])

{
  int i;
  unsigned char b;

  if (iopl (3) < 0) {
    perror ("error enabling usermode io");
    return (-1);
  }

  for (i = 0; i < 128; i ++) {
    asm volatile ("cli");
    asm volatile ("outb %%al,$0x70" : : "a" (i));
    asm volatile ("inb  $0x71,%%al" : "=a" (b));
    asm volatile ("sti");
    if ((i & 3) == 0) printf (" ");
    printf ("%2.2X", b);
    if ((i & 31) == 31) printf ("\n");
  }

  return (0);
}
