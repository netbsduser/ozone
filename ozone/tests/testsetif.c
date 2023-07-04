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

typedef unsigned char uByte;

#define oz_hw_atomic_setif_ptr(loc,new,old) {                     \
        uByte __suc;                                              \
        asm ("lock\n"                                             \
             "cmpxchgl %%edx,(%%ecx)\n"                           \
             "setz %%al\n"      : "=a" (__suc)                    \
                                : "c" (loc), "d" (new), "a" (old) \
                                : "cc", "memory");                \
        __suc;                                                    \
}

int testit (volatile unsigned int *loc, unsigned int new, unsigned int old)

{
  return (oz_hw_atomic_setif_ptr (loc, new, old));
}
