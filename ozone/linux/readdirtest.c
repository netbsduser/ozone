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

#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>

int main ()

{
  DIR *df;
  struct dirent *de;

  df = opendir ("../sources/");
  if (df == NULL) {
    perror ("error opening ../sources/");
    return (-1);
  }

  while ((de = readdir (df)) != NULL) {
    printf ("  <%s>\n", de -> d_name);
  }

  return (0);
}
