//+++2003-03-01
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
//---2003-03-01

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

static OZ_Dev_pci_conf_p pciconfp;
static uLong pciconfigspace[64];

static uLong findentry (OZ_Procmode cprocmode, void *dummy);

uLong oz_util_main (int argc, char *argv[])

{
  uLong sts;

  sts = OZ_MISSINGPARAM;
  if (argc != 4) oz_sys_io_fs_printerror ("dumppciconfig: usage: dumppciconfig <bus> <dev> <func>\n");
  else {
    memset (&pciconfp, 0, sizeof pciconfp);
    pciconfp.pcibus  = atoi (argv[1]);
    pciconfp.pcidev  = atoi (argv[2]);
    pciconfp.pcifunc = atoi (argv[3]);

    sts = oz_sys_callknl (findentry, NULL);
    if (sts == OZ_SUCCESS) oz_sys_io_fs_dumpmem (0, sizeof pciconfigspace, pciconfigspace);
    else oz_sys_io_fs_printerror ("dumppciconfig: error %u reading pci config space\n", sts);
  }
  return (sts);
}

static uLong findentry (OZ_Procmode cprocmode, void *dummy)

{
  int i;

  for (i = 0; i < 64; i ++) {
    pciconfigspace[i] = oz_dev_pci_conf_inl (&pciconfp, i * 4);
  }

  return (OZ_SUCCESS);
}
