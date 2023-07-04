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
/*  Dump out a particular device's pci config space registers		*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_dev_pci.h"
#include "oz_knl_status.h"
#include "oz_sys_callknl.h"
#include "oz_sys_io_fs_printf.h"
#include "oz_util_start.h"

static OZ_Dev_Pci_Conf pciconfp;
static uLong pciconfigspace[1];

static uLong findentry (OZ_Procmode cprocmode, void *dummy);

uLong oz_util_main (int argc, char *argv[])

{
  uLong sts;

  memset (&pciconfp, 0, sizeof pciconfp);

  for (pciconfp.pcibus = 0; pciconfp.pcibus < 3; pciconfp.pcibus ++) {
    for (pciconfp.pcidev = 0; pciconfp.pcidev < 32; pciconfp.pcidev ++) {
      sts = oz_sys_callknl (findentry, NULL);
      if (sts != OZ_SUCCESS) oz_sys_io_fs_printf (oz_util_h_error, "dumppciconfig: error %u reading pci config space\n");
      else if ((pciconfigspace[0] != 0) && (pciconfigspace[0] != 0xFFFFFFFF)) {
        oz_sys_io_fs_printf (oz_util_h_error, "dumppciconfig: pcidev %u didvid %X\n", pciconfp.pcidev, pciconfigspace[0]);
      }
    }
  }
  return (sts);
}

static uLong findentry (OZ_Procmode cprocmode, void *dummy)

{
  int i;

  for (i = 0; i < 1; i ++) {
    pciconfigspace[i] = oz_dev_pci_conf_inl (&pciconfp, i * 4);
  }

  return (OZ_SUCCESS);
}
