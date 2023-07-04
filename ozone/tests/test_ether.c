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
/*  This program continually sends test packets on an ethernet device	*/
/*  It also is continually receiving them				*/
/*									*/
/*	ethertest <device_name> <ether_addr> <packet_size>		*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_crtl_malloc.h"
#include "oz_io_console.h"
#include "oz_io_ether.h"
#include "oz_knl_hw.h"
#include "oz_knl_status.h"
#include "oz_sys_dateconv.h"
#include "oz_sys_handle.h"
#include "oz_sys_io.h"
#include "oz_sys_io_fs.h"
#include "oz_util_start.h"

#define NBUFFS (10)

static int packetsize;
static Long numreceived, seqreceived, firstrcvseq;
static Long numxmitted,  seqxmitted;
static OZ_Handle h_ether;
static uByte protocol[OZ_IO_ETHER_PROTOSIZE] = { 0x11, 0x45 };
static uByte targetenaddr[OZ_IO_ETHER_ADDRSIZE];

static void receive_ast (void *rxbuffv, uLong status, OZ_Mchargs *mchargs);
static void transmit_ast (void *rxbuffv, uLong status, OZ_Mchargs *mchargs);

uLong oz_util_main (int argc, char *argv[])

{
  char *p;
  int i, j;
  OZ_IO_console_ctrlchar console_ctrlchar;
  OZ_IO_ether_buf rxbuffs[NBUFFS], txbuffs[NBUFFS];
  OZ_IO_ether_open ether_open;
  OZ_IO_ether_receive ether_receive;
  OZ_IO_ether_transmit ether_transmit;
  uByte ctrlchar;
  uLong sts;

  if (argc != 4) {
    oz_sys_io_fs_printf (oz_util_h_error, "usage: ethertest <ether_dev_name> <ethernet_addr> <packet_size>\n");
    oz_sys_io_fs_printf (oz_util_h_error, "	<ethernet_addr> is in form xx-xx-xx-...\n");
    oz_sys_io_fs_printf (oz_util_h_error, "	<packet_size> can be zero for maximum\n");
    return (OZ_MISSINGPARAM);
  }

  /* Assign channel to ethernet controller */

  sts = oz_sys_io_assign (OZ_PROCMODE_KNL, &h_ether, argv[1], OZ_LOCKMODE_CW);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printf (oz_util_h_error, "error %u assigning channel to %s\n", sts, argv[1]);
    return (sts);
  }

  /* Get target ethernet address */

  p = argv[2];
  for (i = 0; i < sizeof targetenaddr; i ++) {
    targetenaddr[i] = oz_hw_atoz (p, &j);
    if ((j == 0) || (p[j] != ((i == sizeof targetenaddr - 1) ? 0 : '-'))) {
      oz_sys_io_fs_printf (oz_util_h_error, "bad ethernet address %s at %s\n", argv[2], p);
      return (OZ_BADPARAM);
    }
  }

  /* Get packet size to send out */

  packetsize = oz_hw_atoi (argv[3], &j);
  if ((j == 0) || (argv[3][j] != 0) || (packetsize > OZ_IO_ETHER_MAXDATA)) {
    oz_sys_io_fs_printf (oz_util_h_error, "bad packet size %s\n", argv[3]);
    return (OZ_BADPARAM);
  }
  if (packetsize == 0) packetsize = OZ_IO_ETHER_MAXDATA;

  /* Open the particular ethernet protocol we are going to use */

  memset (&ether_open, 0, sizeof ether_open);
  memcpy (ether_open.proto, protocol, sizeof ether_open.proto);
  sts = oz_sys_io (OZ_PROCMODE_KNL, h_ether, 0, OZ_IO_ETHER_OPEN, sizeof ether_open, &ether_open);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printf (oz_util_h_error, "error %u opening channel to %s\n", sts, argv[1]);
    return (sts);
  }

  /* Start the receives going */

  numreceived = 0;
  seqreceived = 0;
  firstrcvseq = 0;

  memset (&ether_receive, 0, sizeof ether_receive);
  for (i = 0; i < NBUFFS; i ++) {
    ether_receive.size = sizeof rxbuffs[i];
    ether_receive.buff = rxbuffs + i;
    sts = oz_sys_io_start (OZ_PROCMODE_KNL, h_ether, NULL, 0, receive_ast, rxbuffs + i, OZ_IO_ETHER_RECEIVE, sizeof ether_receive, &ether_receive);
    if (sts != OZ_STARTED) receive_ast (rxbuffs + i, sts, NULL);
  }

  /* Start the transmits going */

  numxmitted = 0;
  seqxmitted = 0;

  memset (txbuffs, 0, sizeof txbuffs);

  memset (&ether_transmit, 0, sizeof ether_transmit);
  ether_transmit.size = sizeof *txbuffs;

  for (i = 0; i < NBUFFS; i ++) {
    ether_transmit.buff = txbuffs + i;
    txbuffs[i].dlen = packetsize;
    memcpy (txbuffs[i].dstaddr, targetenaddr, sizeof txbuffs[i].dstaddr);
    memcpy (txbuffs[i].proto,   protocol,     sizeof txbuffs[i].proto);
    *(Long *)(txbuffs[i].data + 0) = oz_hw_atomic_inc_long (&seqxmitted, 1);
    sts = oz_sys_io_start (OZ_PROCMODE_KNL, h_ether, NULL, 0, transmit_ast, txbuffs + i, OZ_IO_ETHER_TRANSMIT, sizeof ether_transmit, &ether_transmit);
    if (sts != OZ_STARTED) transmit_ast (txbuffs + i, sts, NULL);
  }

  memset (&console_ctrlchar, 0, sizeof console_ctrlchar);
  console_ctrlchar.mask[0]  = (1 << 7) || (1 << 3);	/* control-G, control-C */
  console_ctrlchar.terminal = 1;
  console_ctrlchar.ctrlchar = &ctrlchar;

  oz_sys_io_fs_printf (oz_util_h_console, "press control-G for stats, control-C to terminate\n");

loop:
  sts = oz_sys_io (OZ_PROCMODE_KNL, oz_util_h_console, 0, OZ_IO_CONSOLE_CTRLCHAR, sizeof console_ctrlchar, &console_ctrlchar);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printf (oz_util_h_error, "error %u reading control-char\n");
    return (sts);
  }
  oz_sys_io_fs_printf (oz_util_h_console, "num received %d, seq received %d\n", numreceived, seqreceived - firstrcvseq);
  oz_sys_io_fs_printf (oz_util_h_console, "num xmitted  %d, seq xmitted  %d\n", numxmitted,  seqxmitted);
  if (ctrlchar != 3) goto loop;

  return (OZ_SUCCESS);
}

/* This routine gets called when a buffer is received */

static void receive_ast (void *rxbuffv, uLong status, OZ_Mchargs *mchargs)

{
  OZ_IO_ether_buf *rxbuff;
  OZ_IO_ether_receive ether_receive;

  rxbuff = rxbuffv;

  /* Check receive status */

loop:
  if (status != OZ_SUCCESS) {
    oz_sys_io_fs_printf (oz_util_h_error, "error %u receiving buffer\n", status);
    return;
  }

  /* If coming from our target, count it*/

  if (memcmp (rxbuff -> srcaddr, targetenaddr, OZ_IO_ETHER_ADDRSIZE) == 0) {
    seqreceived = *(Long *)(rxbuff -> data + 0);
    if (numreceived == 0) firstrcvseq = seqreceived - 1;
    numreceived ++;
  }

  /* Start a replacement receive */

  memset (&ether_receive, 0, sizeof ether_receive);
  ether_receive.size = sizeof *rxbuff;
  ether_receive.buff = rxbuff;
  status = oz_sys_io_start (OZ_PROCMODE_KNL, h_ether, NULL, 0, receive_ast, rxbuff, OZ_IO_ETHER_RECEIVE, sizeof ether_receive, &ether_receive);
  if (status != OZ_STARTED) goto loop;
}

/* This routine gets called when a buffer is transmitted */

static void transmit_ast (void *txbuffv, uLong status, OZ_Mchargs *mchargs)

{
  OZ_IO_ether_buf *txbuff;
  OZ_IO_ether_transmit ether_transmit;

  txbuff = txbuffv;

  /* Check transmit status */

loop:
  if (status != OZ_SUCCESS) {
    oz_sys_io_fs_printf (oz_util_h_error, "error %u transmitting buffer\n", status);
    return;
  }

  numxmitted ++;

  /* Start a replacement transmit */

  memset (&ether_transmit, 0, sizeof ether_transmit);
  ether_transmit.size = sizeof *txbuff;
  ether_transmit.buff = txbuff;
  *(Long *)(txbuff -> data + 0) = oz_hw_atomic_inc_long (&seqxmitted, 1);
  status = oz_sys_io_start (OZ_PROCMODE_KNL, h_ether, NULL, 0, transmit_ast, txbuff, OZ_IO_ETHER_TRANSMIT, sizeof ether_transmit, &ether_transmit);
  if (status != OZ_STARTED) goto loop;
}
