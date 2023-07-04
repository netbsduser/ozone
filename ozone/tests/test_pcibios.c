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

/************************************************************************/
/*									*/
/*  Scan the bus for 32-bit bios entry					*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_knl_status.h"
#include "oz_sys_callknl.h"
#include "oz_sys_io_fs_printf.h"

typedef struct { uLong biosentry;
                 uLong in_eax, in_ebx, in_ecx, in_edx;
                 uLong out_ef, out_eax, out_ebx, out_ecx, out_edx;
               } Params;

uLong callpcibios (OZ_Procmode cprocmode, void *params);

static uLong findentry (OZ_Procmode cprocmode, void *biosentry_r);

int main ()

{
  uLong sts;
  Params p;

  memset (&p, 0, sizeof p);

  sts = oz_sys_callknl (findentry, &p.biosentry);
  oz_sys_io_fs_printerror ("testpcibios: status %u, biosentry %8.8x\n", sts, p.biosentry);
  if ((sts == OZ_SUCCESS) && (p.biosentry != 0)) {
    p.in_eax = 0x49435024;	/* '$PCI' */
    p.in_ebx = 0;
    sts = oz_sys_callknl (callpcibios, &p);
    oz_sys_io_fs_printerror ("testpcibios: sts %u, ef %8.8x, eax %8.8x, ebx %8.8x, ecx %8.8x, edx %8.8x\n", sts, p.out_ef, p.out_eax, p.out_ebx, p.out_ecx, p.out_edx);
  }
  return (sts);
}

static uLong findentry (OZ_Procmode cprocmode, void *biosentry_r)

{
  uByte *b, cksm;
  int i;
  OZ_Pointer p;

  oz_sys_io_fs_printerror ("testpcibios: scanning\n");
  for (p = 0x0e0000; p < 0x100000; p += 16) {
    b = (uByte *)p;
    if ((b[0] == '_') && (b[1] == '3') && (b[2] == '2') && (b[3] == '_')) {
      oz_sys_io_fs_printerror ("testpcibios: found _32_ at %x\n", p);
      cksm = 0;
      for (i = 0; i < 16; i ++) cksm += b[i];
      oz_sys_io_fs_printerror ("testpcibios: checksum %x\n", cksm);
      oz_sys_io_fs_printerror ("testpcibios: contents: ");
      for (i = 16; -- i >= 0;) oz_sys_io_fs_printerror (" %2.2x", b[i]);
      oz_sys_io_fs_printerror ("\n");
      if (cksm == 0) *(uLong *)biosentry_r = *(uLong *)(b + 4);
    }
  }
  oz_sys_io_fs_printerror ("testpcibios: finished\n");
  return (OZ_SUCCESS);
}
