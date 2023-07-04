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
/*  Dump out PIIX4's config registers, bus 0, dev 4, func 0		*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_dev_pci.h"
#include "oz_knl_status.h"
#include "oz_sys_callknl.h"
#include "oz_sys_io_fs_printf.h"
#include "oz_util_start.h"

typedef struct { int pcibus;
                 int pcidev;
                 int pcifunc;
               } Pci_Conf;

static uLong doitknl (OZ_Procmode cprocmode, void *array);
static uLong pci_conf_inl (Pci_Conf *pciconfp, uByte confadd);

uLong oz_util_main (int argc, char *argv[])

{
  int i, j;
  uLong array[256], sts;

  sts = oz_sys_callknl (doitknl, array);
  if (sts != OZ_SUCCESS) oz_sys_io_fs_printf (oz_util_h_output, "error %u returned from oz_sys_callknl\n", sts);
  else {
    for (i = 0; i < 64; i += 4) {
      for (j = 4; -- j >= 0;) oz_sys_io_fs_printf (oz_util_h_output, "  %8.8X", array[i+j]);
      oz_sys_io_fs_printf (oz_util_h_output, " : %2.2X\n", i * 4);
    }
  }
  return (sts);
}

static uLong doitknl (OZ_Procmode cprocmode, void *array)

{
  int i;
  Pci_Conf pciconfp;

  memset (&pciconfp, 0, sizeof pciconfp);

  pciconfp.pcidev = 4;

  for (i = 0; i < 64; i ++) {
    ((uLong *)array)[i] = pci_conf_inl (&pciconfp, i * 4);
  }

  return (OZ_SUCCESS);
}

#include "oz_knl_hw.h"

#define CONFADD  (0x0CF8)	/* PCI configuration space address I/O port */
				/*     <31> = 1 to enable */
				/*  <16:23> = bus number */
				/*  <11:15> = device number */
				/*   <8:10> = function number */
				/*    <7:2> = register number */
				/*    <0:1> = zeroes */
#define CONFDATA (0x0CFC)	/* PCI configuration space data I/O port */

static uLong pci_conf_inl (Pci_Conf *pciconfp, uByte confadd)

{
  uLong value;

  switch (confadd & 3) {
    case 0: oz_hw486_outl (0x80000000 | (pciconfp -> pcibus << 16) | (pciconfp -> pcidev << 11) | (pciconfp -> pcifunc << 8) | confadd, CONFADD); value = oz_hw486_inl (CONFDATA); break;
    default: return (0x69696969);
  }
  return (value);
}
