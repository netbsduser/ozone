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
/*  I/O function codes for device class "scsi"				*/
/*									*/
/*  These are targeted for scsi controller drivers (eg, 		*/
/*  oz_dev_lsil875_486), but some class drivers (eg, oz_dev_disk_scsi) 	*/
/*  support some of them as well.					*/
/*									*/
/************************************************************************/

#ifndef _OZ_IO_SCSI_H
#define _OZ_IO_SCSI_H

#define OZ_IO_SCSI_CLASSNAME "scsi"
#define OZ_IO_SCSI_BASE (0x00000B00)
#define OZ_IO_SCSI_MASK (0xFFFFFF00)

#include "oz_knl_devio.h"
#include "oz_knl_hw.h"
#include "oz_knl_lock.h"

/* Select the scsi_id on the bus that this channel will communicate with */

#define OZ_IO_SCSI_OPEN OZ_IO_DN(OZ_IO_SCSI_BASE,1)

typedef struct { uLong scsi_id;
                 OZ_Lockmode lockmode;
               } OZ_IO_scsi_open;

/* Perform an I/O to the scsi device, supplying virtual address of data buffer */

#define OZ_IO_SCSI_DOIO OZ_IO_DW(OZ_IO_SCSI_BASE,2)

typedef struct { uLong cmdlen;		/* command length */
                 const uByte *cmdbuf;	/* command buffer */
                 uLong datasize;	/* data buffer size */
                 void *databuff;	/* data buffer address */
                 uLong optflags;	/* option flags */
                 uByte *cmpflags;	/* where to return completion flags */
                 uByte *status;		/* where to return scsi status byte */
                 uLong *datarlen;	/* where to return data length transferred */
                 uLong timeout;		/* timeout (0=none, else milliseconds) */
               } OZ_IO_scsi_doio;

#define OZ_IO_SCSI_OPTFLAG_WRITE      0x1	/* - it is a 'write' operation (memory-to-device transfer) */
#define OZ_IO_SCSI_OPTFLAG_NEGO_WIDTH 0x2	/* - negotiate transfer width and speed */
#define OZ_IO_SCSI_OPTFLAG_DISCONNECT 0x4	/* - allow target to disconnect during command */
#define OZ_IO_SCSI_OPTFLAG_NEGO_SYNCH 0x8	/* - negotiate synchronous transfer */

/* Perform an I/O to the scsi device, supplying array of physical page numbers for the data buffer (kernel mode only) */

#define OZ_IO_SCSI_DOIOPP OZ_IO_DW(OZ_IO_SCSI_BASE,3)

typedef struct { uLong cmdlen;		/* command length */
                 const uByte *cmdbuf;	/* command buffer */
                 uLong datasize;	/* data buffer size */
                 const OZ_Mempage *dataphypages; /* pointer to array of physical page numbers */
                 uLong databyteoffs;	/* byte offset in the first physical page to start at */
                 uLong optflags;	/* option flags */
                 uByte *cmpflags;	/* where to return completion flags */
                 uByte *status;		/* where to return scsi status byte */
                 uLong *datarlen;	/* where to return data length transferred */
                 uLong timeout;		/* timeout (0=none, else milliseconds) */
               } OZ_IO_scsi_doiopp;

/* Get info about the scsi controller and device */

#define OZ_IO_SCSI_GETINFO1 OZ_IO_DN(OZ_IO_SCSI_BASE,4)

typedef struct { uByte max_scsi_id;	/* maximum scsi id allowed (exclusive) */
                 uByte ctrl_scsi_id;	/* controller's scsi id */
                 uByte open_scsi_id;	/* what scsi id is open on the channel (-1 if closed) */
                 uByte open_width;	/* if open, negotiated width */
                 uByte open_speed;	/* if open, negotiated speed */
                 uByte open_raofs;	/* if open, negotiated req/ack offset */
               } OZ_IO_scsi_getinfo1;

/* Reset scsi controller */

#define OZ_IO_SCSI_RESET OZ_IO_DW(OZ_IO_SCSI_BASE,5)

/* Set controller as the crash dump device (kernel mode only) */

#define OZ_IO_SCSI_CRASH OZ_IO_DW(OZ_IO_FS_BASE,6)

typedef struct { uLong (*crashentry) (void *crashparam, 	/* entrypoint to call to perform commands */
                                      OZ_IO_scsi_doiopp *scsi_doiopp); /* pointer to parameter block */
                 void *crashparam;				/* param to pass to crashentry routine */
               } OZ_IO_scsi_crash;

#endif
