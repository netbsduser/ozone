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
/*  Dump physical memory to output					*/
/*									*/
/*	dumppmem <size> <address>					*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_crtl_malloc.h"
#include "oz_knl_kmalloc.h"
#include "oz_knl_status.h"
#include "oz_sys_callknl.h"
#include "oz_sys_condhand.h"
#include "oz_sys_io_fs.h"
#include "oz_util_start.h"

typedef struct { uLong size;
                 OZ_Phyaddr addr;
                 uByte buff[1];
               } Pb;

static uLong getpmem (OZ_Procmode cprocmode, void *pbv);

uLong oz_util_main (int argc, char *argv[])

{
  char c;
  int i, j, usedup;
  OZ_Phyaddr addr;
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
  pb -> addr = addr;

  /* Call kernel mode routine to copy physical memory to user memory */

  sts = oz_sys_callknl (getpmem, pb);
  if (sts != OZ_SUCCESS) oz_sys_io_fs_printf (oz_util_h_error, "error %u getting memory contents\n", sts);
  else {

    /* Dump out the user-mode copy of the physical memory */

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
  oz_sys_io_fs_printf (oz_util_h_error, "usage: dumppmem <size> <address>\n");
  return (OZ_BADPARAM);
}

static uLong getpmem (OZ_Procmode cprocmode, void *pbv)

{
  int si;
  OZ_Mempage begpage, endpage, i, *phypages;
  Pb *pb;
  uLong byteoffs;

  pb = pbv;

  begpage  = pb -> addr >> OZ_HW_L2PAGESIZE;
  endpage  = (pb -> addr + pb -> size - 1) >> OZ_HW_L2PAGESIZE;
  if (endpage < begpage) return (OZ_BADBUFFERSIZE);

  si = oz_hw_cpu_setsoftint (0);

  phypages = OZ_KNL_NPPMALLOQ ((endpage - begpage + 1) * sizeof *phypages);
  if (phypages == NULL) {
    oz_hw_cpu_setsoftint (si);
    return (OZ_EXQUOTANPP);
  }

  for (i = 0; i <= begpage - endpage; i ++) phypages[i] = begpage + i;

  byteoffs = pb -> addr & ((1 << OZ_HW_L2PAGESIZE) - 1);
  oz_hw_phys_movetovirt (pb -> size, pb -> buff, phypages, byteoffs);
  OZ_KNL_NPPFREE (phypages);

  oz_hw_cpu_setsoftint (si);

  return (OZ_SUCCESS);
}
