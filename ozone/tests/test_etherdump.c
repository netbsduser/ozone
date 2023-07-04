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
/*  This program dumps all packets received on an ethernet device	*/
/*									*/
/*	etherdump <device_name>						*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_crtl_malloc.h"
#include "oz_io_ether.h"
#include "oz_knl_status.h"
#include "oz_sys_handle.h"
#include "oz_sys_io.h"
#include "oz_util_start.h"

#define NBUFFS (10)

typedef struct { struct { OZ_IO_ether_buf ether;
                          uByte pad[OZ_IO_ETHER_MAXDATA];
                        } buf;
               } Rxbuff;

static OZ_Handle h_iochan;

static void receive_ast (void *rxbuffv, uLong status, OZ_Mchargs *mchargs);

uLong oz_util_main (int argc, char *argv[])

{
  int i;
  uLong sts;
  OZ_Handle h_event;
  OZ_IO_ether_open ether_open;
  OZ_IO_ether_receive ether_receive;
  Rxbuff *rxbuffs;

  if (argc != 2) {
    oz_sys_io_fs_printf (oz_util_h_error, "usage: etherdump <ether_dev_name>\n");
    return (OZ_MISSINGPARAM);
  }

  sts = oz_sys_io_assign (OZ_PROCMODE_KNL, &h_iochan, argv[1], OZ_LOCKMODE_CW);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printf (oz_util_h_error, "error %u assigning channel to %s\n", sts, argv[1]);
    return (sts);
  }

  rxbuffs = malloc (NBUFFS * sizeof *rxbuffs);

  memset (&ether_open, 0, sizeof ether_open);
  ether_open.promis  = 1;

  sts = oz_sys_io (OZ_PROCMODE_KNL, h_iochan, 0, OZ_IO_ETHER_OPEN, sizeof ether_open, &ether_open);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printf (oz_util_h_error, "error %u opening channel to %s\n", sts, argv[1]);
    return (sts);
  }

  memset (&ether_receive, 0, sizeof ether_receive);
  for (i = 0; i < NBUFFS; i ++) {
    ether_receive.size = sizeof rxbuffs[i].buf;
    ether_receive.buff = &(rxbuffs[i].buf.ether);
    sts = oz_sys_io_start (OZ_PROCMODE_KNL, h_iochan, NULL, 0, receive_ast, rxbuffs + i, OZ_IO_ETHER_RECEIVE, sizeof ether_receive, &ether_receive);
    if (sts != OZ_STARTED) {
      oz_sys_io_fs_printf (oz_util_h_error, "error %u queueing receive request\n", sts);
      return (sts);
    }
  }

  sts = oz_sys_event_create (OZ_PROCMODE_KNL, "hang forever", &h_event);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printf (oz_util_h_error, "error %u creating event flag\n", sts);
    return (sts);
  }

  while (1) { oz_sys_event_wait (OZ_PROCMODE_KNL, h_event, 0); oz_sys_event_set (OZ_PROCMODE_KNL, h_event, 0, NULL); }
  return (OZ_SUCCESS);
}

static void receive_ast (void *rxbuffv, uLong status, OZ_Mchargs *mchargs)

{
  uLong sts;
  OZ_IO_ether_receive ether_receive;
  Rxbuff *rxbuff;

  rxbuff = rxbuffv;

  if (status != OZ_SUCCESS) {
    oz_sys_io_fs_printf (oz_util_h_error, "error %u receiving buffer\n", status);
    return;
  }

  oz_sys_io_fs_printf (oz_util_h_output, "msg len: %u\n", rxbuff -> buf.ether.dlen);
  oz_sys_io_fs_printf (oz_util_h_output, "  %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x -> %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x : %4.4x\n", 
	rxbuff -> buf.ether.srcaddr[0], rxbuff -> buf.ether.srcaddr[1], rxbuff -> buf.ether.srcaddr[2], rxbuff -> buf.ether.srcaddr[3], rxbuff -> buf.ether.srcaddr[4], rxbuff -> buf.ether.srcaddr[5], 
	rxbuff -> buf.ether.dstaddr[0], rxbuff -> buf.ether.dstaddr[1], rxbuff -> buf.ether.dstaddr[2], rxbuff -> buf.ether.dstaddr[3], rxbuff -> buf.ether.dstaddr[4], rxbuff -> buf.ether.dstaddr[5], 
	(rxbuff -> buf.ether.proto[0] << 8) + rxbuff -> buf.ether.proto[1]);
  oz_sys_io_fs_printf (oz_util_h_output, "  %8.8x %8.8x %8.8x %8.8x : 0000\n", 
	*(uLong *)(rxbuff -> buf.ether.data + 12), *(uLong *)(rxbuff -> buf.ether.data +  8), 
	*(uLong *)(rxbuff -> buf.ether.data +  4), *(uLong *)(rxbuff -> buf.ether.data +  0));
  oz_sys_io_fs_printf (oz_util_h_output, "  %8.8x %8.8x %8.8x %8.8x : 0010\n", 
	*(uLong *)(rxbuff -> buf.ether.data + 28), *(uLong *)(rxbuff -> buf.ether.data + 24), 
	*(uLong *)(rxbuff -> buf.ether.data + 20), *(uLong *)(rxbuff -> buf.ether.data + 16));

  memset (&ether_receive, 0, sizeof ether_receive);
  ether_receive.size = sizeof rxbuff -> buf;
  ether_receive.buff = &(rxbuff -> buf.ether);
  sts = oz_sys_io_start (OZ_PROCMODE_KNL, h_iochan, NULL, 0, receive_ast, rxbuff, OZ_IO_ETHER_RECEIVE, sizeof ether_receive, &ether_receive);
  if (sts != OZ_STARTED) oz_sys_io_fs_printf (oz_util_h_error, "error %u queueing receive request\n", sts);
}
