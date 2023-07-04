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

#include "ozone.h"
#include "oz_knl_status.h"
#include "oz_sys_callknl.h"
#include "oz_sys_io_fs_printf.h"
#include "oz_util_start.h"

static uLong doit (OZ_Procmode cprocmode, void *dummy);

uLong oz_util_main (int argc, char *argv[])

{
  uLong sts;

  sts = oz_sys_callknl (doit, NULL);
  return (sts);
}

static uLong doit (OZ_Procmode cprocmode, void *dummy)

{
  uLong *scan;

  for (scan = (uLong *)0xC0000; scan < (uLong *)0xD0000; scan ++) {
    if ((*scan == 'PMID') || (*scan == 'DIMP')) {
      oz_sys_io_fs_dumpmem (oz_util_h_output, 32, scan);
    }
  }
  return (OZ_SUCCESS);
}
