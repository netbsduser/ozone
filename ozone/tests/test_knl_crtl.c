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

#include <stdio.h>

char *strchr ();
char *strrchr ();

int main ()

{
  char buf[16], *p;
  int x;

  x = memcmp ("aaaaaaaaaaab", "aaaaaaaaaaac", 12);
  printf ("memcmp = -1 : %d\n", x);

  memcpy (buf, "123456789abcdef", 16);
  printf ("memcpy = 123456789abcdef : %s\n", buf);

  memmove (buf + 4, buf, 10);
  printf ("memmove = 1234123456789af : %s\n", buf);

  memset (buf, '*', sizeof buf - 1);
  printf ("memset = *************** : %s\n", buf);

  x = strcasecmp ("AbCdEFg", "aBCdEfG");
  printf ("strcasecmp = 0 : %d\n", x);

  strcpy (buf, "abcd");
  strcat (buf, "efghij");
  printf ("strcpy/strcat = abcdefghij : %s\n", buf);

  strcpy (buf, "123453456789");
  p = strchr (buf, '3');
  printf ("strchr = %p : %p\n", buf + 2, p);
  p = strchr (buf, '0');
  printf ("strchr = %p : %p\n", NULL, p);

  p = strrchr (buf, '3');
  printf ("strrchr = %p : %p\n", buf + 5, p);

  x = strlen ("abcd");
  printf ("strlen = 4 : %d\n", x);

  x = strcmp ("abcdefg", "abcdefh");
  printf ("strcmp = -1 : %d\n", x);

  x = strncasecmp ("AbCdEFg", "aBCdExG", 5);
  printf ("strncasecmp = 0 : %d\n", x);

  x = strncmp ("abcdefgh", "abcd", 5);
  printf ("strncmp = %d : %d\n", 'e', x);

  x = strnlen ("abcdefgh", 6);
  printf ("strnlen = 6 : %d\n", x);

  strncpy (buf, "abcdefghijklmnopqrstuvwxyz", sizeof buf);
  printf ("strncpy = abcdefghijklmno : %s\n", buf);
  
  return (x);
}

#if 0

 ##  4(%esp) = dest
 ##  8(%esp) = src
 ## 12(%esp) = n

	.align	4
	.globl	strncpy
strncpy:

#endif
