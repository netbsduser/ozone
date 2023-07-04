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

#include "ozone.h"
#include "oz_dev_dpar.h"
#include "oz_dev_scsi.h"
#include "oz_io_disk.h"
#include "oz_io_fs.h"
#include "oz_io_scsi.h"
#include "oz_knl_devio.h"
#include "oz_knl_event.h"
#include "oz_knl_hw.h"
#include "oz_knl_kmalloc.h"
#include "oz_knl_sdata.h"
#include "oz_knl_status.h"
#include "oz_ldr_params.h"

#define BLOCKSIZE 512
#define DT_CD ('C' + ('D' << 8))
#define DT_FD ('F' + ('D' << 8))
#define DT_HD ('H' + ('D' << 8))

#pragma pack(1)
typedef struct { uByte code[BLOCKSIZE-2-64-32];
                 struct { char string[8]; } partnames[4];
                 struct { uByte flag;
                          uByte fill1[3];
                          uByte ptype;
                          uByte fill2[3];
                          uLong start;
                          uLong count;
                        } partitions[4];
                 uWord magic;
               } Partition;
#pragma pack(8)

typedef struct Diskctx Diskctx;

struct Diskctx { Diskctx *next;
                 const char *devname;
                 OZ_Iochan *diskiochan;
                 OZ_Iochan *fsiochan;
                 OZ_IO_scsi_getinfo1 scsi_getinfo1;
                 uQuad align1;
                 OZ_Loadparams parambuff;
                 uQuad align2;
                 Partition partblockbuff;
                 uByte theend;
               };

#define OFFSETOF(field) (char *)&(diskctx.field)-(char *)&diskctx

int main ()

{
  Diskctx diskctx;

  printf ("          next %4.4X\n", OFFSETOF (next));
  printf ("       devname %4.4X\n", OFFSETOF (devname));
  printf ("    diskiochan %4.4X\n", OFFSETOF (diskiochan));
  printf ("      fsiochan %4.4X\n", OFFSETOF (fsiochan));
  printf (" scsi_getinfo1 %4.4X\n", OFFSETOF (scsi_getinfo1));
  printf ("        align1 %4.4X\n", OFFSETOF (align1));
  printf ("     parambuff %4.4X\n", OFFSETOF (parambuff));
  printf ("        align2 %4.4X\n", OFFSETOF (align2));
  printf (" partblockbuff %4.4X\n", OFFSETOF (partblockbuff));
  printf ("        theend %4.4X\n", OFFSETOF (theend));

  return (0);
}
