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

typedef int Long;

static inline Long oz_hw_atomic_set_long (volatile Long *loc, Long new)

{
  Long old;

  asm ("xchgl %0,%2\n"
       : "=r" (old)
       : "0" (new), "m" (*loc)
       : "cc", "memory");

  return (old);
}

int main ()

{
  Long oldv, value;

  value = 5;
  oldv = oz_hw_atomic_set_long (&value, 8);
  printf ("oldv=%d, value=%d\n", oldv, value);
  return (0);
}

