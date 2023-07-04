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

#include "ozone.h"

#include "oz_sys_io_fs.h"
#include "oz_sys_process.h"
#include "oz_sys_section.h"

#define L2PAGESIZE 12

int main ()

{
  int i;
  uLong checksum, *checksums, pagebuf[1<<(L2PAGESIZE-2)];
  uLong rlen, sts;
  OZ_Dbn numblocks;
  OZ_Handle h_iochan, h_section;
  OZ_IO_fs_getinfo1 fs_getinfo1;
  OZ_IO_fs_open fs_open;
  OZ_IO_fs_readblocks fs_readblocks;
  OZ_Mempage npagem, numpages, svpage;

  /* Open the test file for read only */

  memset (&fs_open, 0, sizeof fs_open);
  fs_open.name = "pciide_pm.4.fs:/oz_cli.oz";
  fs_open.lockmode = OZ_LOCKMODE_CR;
  sts = oz_sys_io_fs_open (sizeof fs_open, &fs_open, 0, &h_iochan);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printf (oz_util_h_error, 
                         "pftest: error %u opening %s\n", 
                         sts, fs_open.name);
    return (sts);
  }

  /* Get the number of blocks in the file */

  memset (&fs_getinfo1, 0, sizeof fs_getinfo1);
  sts = oz_sys_io_startwait (OZ_PROCMODE_KNL, h_iochan, 0, 
                             OZ_IO_FS_GETINFO1, sizeof fs_getinfo1, 
                             &fs_getinfo1);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printf (oz_util_h_error, 
                         "pftest: error %u getting file info\n", 
                         sts);
    return (sts);
  }

  /* Allocate an checksum array */

  numblocks = fs_getinfo1.eofblock - 1;
  numpages  = (numblocks * fs_getinfo1.blocksize) >> L2PAGESIZE;
  checksums = malloc (numpages * sizeof *checksums);
  fs_readblocks.size = sizeof pagebuff;
  fs_readblocks.buff = pagebuff;
  fs_readblocks.svbn = 1;
  fs_readblocks.rlen = &rlen;

  /* Read the file, page by page, calculating the checksum for each */
  
  for (npagem = 0; npagem < numpages; npagem ++) {
    sts = oz_sys_io_startwait (OZ_PROCMODE_KNL, h_iochan, 0, 
                               OZ_IO_FS_READBLOCKS, sizeof fs_readblocks, 
                               &fs_readblocks);
    if (sts != OZ_SUCCESS) {
      oz_sys_io_fs_printf (oz_util_h_error, 
                           "pftest: error %u reading vbn %u\n", 
                           sts, fs_readblocks.svbn);
      return (sts);
    }
    if (rlen != fs_readblocks.size) {
      oz_sys_io_fs_printf (oz_util_h_error, 
                           "pftest: read %u bytes instead of %u\n", 
                           rlen, fs_readblocks.size);
      return (-1);
    }
    checksum = 0;
    for (i = 0; i < sizeof pagebuf / sizeof *pagebuff; i ++) {
      checksum += pagebuf[i];
    }
    checksums[npagem] = checksum;
    fs_readblocks.svbn += fs_readblocks.size / fs_getinfo1.blocksize;
  }

  /* Create a memory section consisting of the file */

  sts = oz_sys_section_create (OZ_PROCMODE_KNL, h_iochan, 0, 1, 
                               OZ_SECTION_TYPE_RDONLY, OZ_SECTION_PAGEPROT_UR, 
                               0, &h_section);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printf (oz_util_h_error, 
                         "pftest: error %u creating section\n", 
                         sts);
    return (sts);
  }

  /* Map that section to memory */

  npagem = numpages;
  svpage = OZ_HW_VADDRTOVPAGE (&svpage);
  sts = oz_sys_process_mapsection (OZ_PROCMODE_KNL, h_section, &npagem, 
                                   &svpage, 0);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printf (oz_util_h_error, 
                         "pftest: error %u mapping section\n", 
                         sts);
    return (sts);
  }

  svaddr = OZ_HW_VPAGETOVADDR (svpage);
  oz_sys_io_fs_printf (oz_util_h_output, 
                       "pftest: %u pages mapped at %p\n", 
                       npagem, svaddr);

  ??
}
