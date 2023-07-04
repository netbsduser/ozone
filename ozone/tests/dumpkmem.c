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
/*  Dump kernel memory to output					*/
/*									*/
/*	dumpkmem <size> <address>					*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_crtl_malloc.h"
#include "oz_knl_status.h"
#include "oz_sys_callknl.h"
#include "oz_sys_condhand.h"
#include "oz_sys_io_fs.h"
#include "oz_util_start.h"

typedef struct { uLong size;
                 void *addr;
                 uByte buff[1];
               } Pb;

static uLong getkmem (OZ_Procmode cprocmode, void *pbv);
static uLong getkmem_try (void *pbv);

uLong oz_util_main (int argc, char *argv[])

{
  char c;
  int i, j, usedup;
  OZ_Pointer addr;
  Pb *pb;
  uLong size, sts;

  if (argc != 3) goto usage;

  /* Get size and address from command line args */

  size = oz_hw_atoz (argv[1], &usedup);
  if ((usedup == 0) || (argv[1][usedup] != 0)) goto usage;
  addr = oz_hw_atoz (argv[2], &usedup);
  if ((usedup == 0) || (argv[2][usedup] != 0)) goto usage;

  /* Allocate user-mode buffer to copy stuff to */

  pb = malloc (size + sizeof *pb);
  pb -> size = size;
  pb -> addr = (void *)addr;

  /* Call kernel mode routine to copy kernel memory to user memory */

  sts = oz_sys_callknl (getkmem, pb);
  if (sts != OZ_SUCCESS) oz_sys_io_fs_printf (oz_util_h_error, "error %u getting memory contents\n", sts);
  else {

    /* Dump out the user-mode copy of the kernel memory */

    for (i = 0; i < size; i += 16) {
      for (j = 16; -- j >= 0;) {
        if ((j & 3) == 3) oz_sys_io_fs_printf (oz_util_h_output, " ");
        if (i + j >= size) oz_sys_io_fs_printf (oz_util_h_output, "  ");
        else oz_sys_io_fs_printf (oz_util_h_output, "%2.2X", pb -> buff[i+j]);
      }
      oz_sys_io_fs_printf (oz_util_h_output, " : %8.8X : '", addr + i);
      for (j = 0; (j < 16) && (i + j < size); j ++) {
        c = pb -> buff[i+j] & 0x7F;
        if ((c == 127) || (c < ' ')) c = '.';
        oz_sys_io_fs_printf (oz_util_h_output, "%c", c);
      }
      oz_sys_io_fs_printf (oz_util_h_output, "'\n");
    }
  }
  return (sts);

usage:
  oz_sys_io_fs_printf (oz_util_h_error, "usage: dumpmem <size> <address>\n");
  return (OZ_BADPARAM);
}

static uLong getkmem (OZ_Procmode cprocmode, void *pbv)

{
  return (oz_sys_condhand_try (getkmem_try, pbv, oz_sys_condhand_rtnanysig, NULL));
}

static uLong getkmem_try (void *pbv)

{
  Pb *pb;

  pb = pbv;
  memcpy (pb -> buff, pb -> addr, pb -> size);
  return (OZ_SUCCESS);
}
