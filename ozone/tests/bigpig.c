//+++2002-08-17
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
//---2002-08-17

#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[])

{
  char *pig, *runt;
  int n;

  if (argc != 2) {
    fprintf (stderr, "usage: bigpig <numberofiterations>\n");
    return (-1);
  }

  n = atoi (argv[1]);
  printf ("number of iterations: %d\n", n);

  pig  = malloc (300000000);
  while (n > 0) {
    memset (pig, -- n, 300000000);
  }
  runt = malloc (16);
  printf ("pig %p, runt %p\n", pig, runt);
  free (pig);
  free (runt);
  return (0);
}
