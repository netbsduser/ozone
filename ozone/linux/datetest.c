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
#include "ozone.h"
#include "oz_sys_dateconv.h"

int main ()

{
  char buff[256], *p;
  uLong datelongs[OZ_DATELONG_ELEMENTS], dow;
  OZ_Datebin datebin;

  while (1) {
    printf ("   : ");
    fgets (buff, sizeof buff, stdin);
    p = strchr (buff, '\n');
    if (p != NULL) *p = 0;
    if (!oz_sys_datebin_encstr (strlen (buff), buff, &datebin)) {
      printf ("bad conversion");
      continue;
    }
    printf ("hex: %8.8x:%8.8x\n", datebin);
    oz_sys_datebin_decstr (0, datebin, sizeof buff, buff);
    printf ("abs: %s\n", buff);
    oz_sys_datebin_decstr (1, datebin, sizeof buff, buff);
    printf ("del: %s\n", buff);
    oz_sys_datebin_decode (datebin, datelongs);
    printf ("fraction: %u   second: %u   daynumber: %u\n", datelongs[OZ_DATELONG_FRACTION], datelongs[OZ_DATELONG_SECOND], datelongs[OZ_DATELONG_DAYNUMBER]);
    dow = oz_sys_daynumber_weekday (datelongs[OZ_DATELONG_DAYNUMBER]);
    printf ("dow: %u\n", dow);
  }
}

Long oz_sys_gettimezone (void)

{
  return (0);
}
