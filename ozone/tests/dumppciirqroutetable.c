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
/*  Scan the BIOS ROM area for PCI IRQ routing table			*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_knl_status.h"
#include "oz_sys_callknl.h"
#include "oz_sys_io_fs_printf.h"
#include "oz_util_start.h"

static uLong findentry (OZ_Procmode cprocmode, void *dummy);

uLong oz_util_main (int argc, char *argv[])

{
  uLong sts;

  sts = oz_sys_callknl (findentry, NULL);
  oz_sys_io_fs_printerror ("final status %u\n", sts);
  return (sts);
}

static uLong findentry (OZ_Procmode cprocmode, void *dummy)

{
#if 111
  oz_knl_printk ("dumppciirqroute: %d\n", oz_dev_pci_dumpirqroute ());
#else
  int i;
  OZ_Pointer p, o;
  uByte *b, cksm;
  uWord size;

  oz_sys_io_fs_printf (oz_util_h_output, "scanning\n");
  for (p = 0xF0000; p < 0xFFFFF; p += 16) {
    b = (uByte *)p;
    if ((b[0] == '$') && (b[1] == 'P') && (b[2] == 'I') && (b[3] == 'R')) {
      size = (b[7] << 8) + b[6];
      oz_sys_io_fs_printf (oz_util_h_output, "found $PIR at %x, size 0x%x\n", p, size);
      cksm = 0;
      for (i = 0; i < size; i ++) cksm += b[i];
      oz_sys_io_fs_printf (oz_util_h_output, "checksum 0x%x\n", cksm);
      if (cksm == 0) {
        oz_knl_dumpmem (size, b);
        for (o = 32; o < size; o += 16) {
          b = (uByte *)p + o;
          oz_sys_io_fs_printf (oz_util_h_output, "  bus %u, device %u=0x%x, slot %u\n", b[0], b[1], b[1], b[14]);
          for (i = 0; i < 4; i ++) {
             oz_sys_io_fs_printf (oz_util_h_output, "    int %c: link %x, bitmap %2.2x%2.2x\n", 'A' + i, b[i*3+2], b[i*3+4], b[i*3+3]);
          }
        }
        return (OZ_SUCCESS);
      }
    }
  }
  oz_sys_io_fs_printf (oz_util_h_output, "no $PIR found\n");
#endif
  return (OZ_SUCCESS);
}
