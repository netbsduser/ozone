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

/************************************************************************/
/*									*/
/*  Dump out smplock clash counters					*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_knl_status.h"
#include "oz_sys_callknl.h"
#include "oz_sys_condhand.h"
#include "oz_sys_io_fs_printf.h"
#include "oz_util_start.h"

extern uQuad oz_hw486_smplev_clash[256];	// location of kernel mode buffer
static uQuad smplev_clash[256];			// user mode buffer

static uLong knlgetit (OZ_Procmode cprocmode, void *dummy);

uLong oz_util_main (int argc, char *argv[])

{
  int i;
  uLong sts;

  sts = oz_sys_callknl (knlgetit, NULL);
  if (sts != OZ_SUCCESS) oz_sys_condhand_signal (2, sts, 0);

  for (i = 0; i < 256; i ++) {
    if (smplev_clash[i] != 0) {
      oz_sys_io_fs_printf (oz_util_h_output, "smplevel[%2.2X] %8.8X.%8.8X\n", 
			i, (uLong)(smplev_clash[i] >> 32), (uLong)(smplev_clash[i]));
    }
  }

  return (OZ_SUCCESS);
}

static uLong knlgetit (OZ_Procmode cprocmode, void *dummy)

{
  memcpy (smplev_clash, oz_hw486_smplev_clash, sizeof smplev_clash);	// copy kernel data to user buffer
  return (OZ_SUCCESS);							// successful
}
