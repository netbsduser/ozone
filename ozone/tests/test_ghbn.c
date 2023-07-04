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

#include "ozone.h"
#include "oz_knl_status.h"
#include "oz_sys_gethostipaddr.h"
#include "oz_util_start.h"
#include <stdio.h>

uLong oz_util_main (int argc, char *argv[])

{
  char hostname[256];
  uByte ipaddr[4];
  uLong sts;

  while (gets (hostname) != NULL) {
    sts = oz_sys_gethostipaddr (hostname, sizeof ipaddr, ipaddr);
    if (sts != OZ_SUCCESS) printf ("error %u\n");
    else printf ("%u.%u.%u.%u\n", ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);
  }

  return (OZ_SUCCESS);
}
