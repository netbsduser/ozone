//+++2001-11-18
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
//---2001-11-18

#include <stdio.h>
#include "ozone.h"
#include "oz_knl_status.h"
#include "oz_sys_thread.h"

static void ast1 (void *dummy1, uLong dummy2, OZ_Mchargs *dummy3);
static void ast2 (void *dummy1, uLong dummy2, OZ_Mchargs *dummy3);

int main ()

{
  uLong sts;

  sts = oz_sys_thread_queueast (OZ_PROCMODE_USR, 0, ast1, NULL, OZ_PENDING);
  printf ("queueast ast1 sts %u\n", sts);
  return (sts);
}

static void ast1 (void *dummy1, uLong dummy2, OZ_Mchargs *dummy3)

{
  uLong sts;

  printf ("entered ast1\n");
  sts = oz_sys_thread_queueast (OZ_PROCMODE_USR, 0, ast2, NULL, OZ_PENDING);
  printf ("queueast ast2 sts %u\n", sts);
}

static void ast2 (void *dummy1, uLong dummy2, OZ_Mchargs *dummy3)

{
  printf ("in ast2\n");
}
