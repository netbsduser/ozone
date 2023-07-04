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

/************************************************************************/
/*									*/
/*  Dump out RTC ram							*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_dev_isa.h"
#include "oz_knl_hw.h"
#include "oz_knl_status.h"
#include "oz_sys_callknl.h"
#include "oz_sys_io_fs_printf.h"
#include "oz_util_start.h"

static uByte rtcram[384];

static uLong readrtcram (OZ_Procmode cprocmode, void *dummy);

uLong oz_util_main (int argc, char *argv[])

{
  uLong sts;

  // First, try to use 'setuseriopl' so we can do I/O instructions from user mode

#ifdef OZ_HW_TYPE_486
  sts = oz_hw486_setuseriopl (1);
  oz_sys_io_fs_printerror ("setuseriopl status %u\n", sts);
  if (sts == OZ_FLAGWASCLR) {
    readrtcram (OZ_PROCMODE_USR, NULL);
    oz_sys_io_fs_dumpmem (0, sizeof rtcram, rtcram);
  }

  // Otherwise, do via callknl routine

  else {
#endif
    sts = oz_sys_callknl (readrtcram, NULL);
    oz_sys_io_fs_printerror ("callknl status %u\n", sts);
    if (sts == OZ_SUCCESS) oz_sys_io_fs_dumpmem (0, sizeof rtcram, rtcram);
#ifdef OZ_HW_TYPE_486
  }
#endif

  return (sts);
}

static uLong readrtcram (OZ_Procmode cprocmode, void *dummy)

{
  int i;

  for (i = 0; i < 128; i ++) {
    oz_dev_isa_outb (i, 0x70);
    rtcram[i] = oz_dev_isa_inb (0x71);
  }

  for (i = 0; i < 256; i ++) {
    oz_dev_isa_outb (i, 0x72);
    rtcram[i+128] = oz_dev_isa_inb (0x73);
  }

  return (OZ_SUCCESS);
}
