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

typedef unsigned int uLong;

static uLong when (char *buffer);

int main ()

{
  char buffer[4096];
  uLong lasttime, thistime;

  gets (buffer);
  lasttime = when (buffer);
  while (gets (buffer) != NULL) {
    thistime = when (buffer);
    printf ("%10u %s\n", thistime - lasttime, buffer);
    lasttime = thistime;
  }
  return (0);
}

static uLong when (char *buffer)

{
  uLong hour, minute, second, microsec;

  sscanf (buffer, "%u:%u:%u.%u", &hour, &minute, &second, &microsec);
  return (((hour * 60 + minute) * 60 + second * 1000000) + microsec);
}
