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
/*  I/O function codes for device class "ether"				*/
/*									*/
/*  These codes are processed by hardware ethernet drivers.  They can 	*/
/*  also be processed by any driver (like PPP) that wishes to 		*/
/*  interface with the 'ip' driver, so long as it returns an 		*/
/*  appropriate size for maxmsgsize of the getinfo1 function.		*/
/*									*/
/************************************************************************/

#ifndef _OZ_IO_ETHER_H
#define _OZ_IO_ETHER_H

#include "oz_knl_devio.h"

#define OZ_IO_ETHER_CLASSNAME "ether"
#define OZ_IO_ETHER_BASE (0x00000500)
#define OZ_IO_ETHER_MASK (0xFFFFFF00)

#define OZ_IO_ETHER_MAXADDR (20)			/* maximum number of bytes in an ethernet address */
							/* (this is the largest value that any driver will */
							/*  return in addrsize of the getinfo1 function) */
#define OZ_IO_ETHER_MAXPROTO (2)			/* maximum number of bytes in an ethernet protocol number */
							/* (this is the largest value that any driver will */
							/*  return in protosize of the getinfo1 function) */
#define OZ_IO_ETHER_MAXDATA (1500)			/* maximum number of data bytes possible */
							/* (this is the largest value that any driver will */
							/*  return in maxmsgsize of the getinfo1 function) */
#define OZ_IO_ETHER_MAXBUFF (OZ_IO_ETHER_MAXADDR*2+OZ_IO_ETHER_MAXPROTO+OZ_IO_ETHER_MAXDATA+4) /* max total buffer size possible */

#include "oz_knl_hw.h"

#define OZ_IO_ETHER_OPEN OZ_IO_DW(OZ_IO_ETHER_BASE,1)

typedef struct { int   promis;				/* set means promiscuous mode */
                 uByte proto[OZ_IO_ETHER_MAXPROTO];	/* receive protocol */
               } OZ_IO_ether_open;

#define OZ_IO_ETHER_CLOSE OZ_IO_DN(OZ_IO_ETHER_BASE,2)

#define OZ_IO_ETHER_RECEIVE OZ_IO_DR(OZ_IO_ETHER_BASE,3)

typedef struct { uLong   size;				/* size of buffer */
                 void   *buff;				/* address of buffer */
                 uLong  *dlen;				/* where to return length of data (not incl any header items) */
                 void   *rcvfre;			/* kernel mode only: last receive buffer to free off (pointer returned in previous rcvdrv) */
                 void  **rcvdrv_r;			/* kernel mode only: where to return driver buffer pointer */
                 uByte **rcveth_r;			/* kernel mode only: where to return ether buffer pointer */
							/*                   - points to ethernet buffer within rcvdrv buffer */
               } OZ_IO_ether_receive;

#define OZ_IO_ETHER_RECEIVEFREE OZ_IO_DR(OZ_IO_ETHER_BASE,4)

typedef struct { void *rcvfre;				/* kernel mode only: receive buffer to free off (pointer returned in rcvdrv) */
               } OZ_IO_ether_receivefree;

#define OZ_IO_ETHER_TRANSMITALLOC OZ_IO_DW(OZ_IO_ETHER_BASE,5)

typedef struct { uLong  *xmtsiz_r;			/* kernel mode only: where to return size of buffer's data area */
                 void  **xmtdrv_r;			/* kernel mode only: where to return driver buffer pointer */
                 uByte **xmteth_r;			/* kernel mode only: where to return ether buffer pointer */
               } OZ_IO_ether_transmitalloc;

#define OZ_IO_ETHER_TRANSMIT OZ_IO_DW(OZ_IO_ETHER_BASE,6)

typedef struct { uLong   size;				/* total size of buffer (incl header items) */
                 void   *buff;				/* address of buffer */
                 uLong   dlen;				/* length of data in the buffer (not incl any header items) */
                 void   *xmtdrv;			/* kernel mode only: driver buffer pointer */
                 uLong  *xmtsiz_r;			/* kernel mode only: where to return size of new buffer's data area */
                 void  **xmtdrv_r;			/* kernel mode only: where to return driver new buffer pointer */
                 uByte **xmteth_r;			/* kernel mode only: where to return ether new buffer pointer */
               } OZ_IO_ether_transmit;

#define OZ_IO_ETHER_GETINFO1 OZ_IO_DN(OZ_IO_ETHER_BASE,7)

typedef struct { uLong  enaddrsize;			/* size of the enaddrbuff */
                 uByte *enaddrbuff;			/* where to return this interface's address */
                 uLong  datasize;			/* maximum size of data that can be sent or received (excl header info) */
                 uLong  buffsize;			/* maximum size of total buffer that can be sent or received (incl header) */
                 uLong  dstaddrof;			/* offset of destination address in a buffer */
                 uLong  srcaddrof;			/* offset of source address in a buffer */
                 uLong  protooffs;			/* offset of protocol in a buffer */
                 uLong  dataoffset;			/* offset of data in a buffer */
                 uWord  arphwtype;			/* hardware type for ARP */
                 uByte  addrsize;			/* size of addresses (dstaddr and srcaddr) */
                 uByte  protosize;			/* size of protocol */
                 uLong  rcvmissed;			/* number of receives missed on this channel */
               } OZ_IO_ether_getinfo1;

/* I/O select codes */

#define OZ_SE_ETHER_RCVREADY (OZ_IO_ETHER_BASE | 0x01)	/* the driver has a packet that needs to be received */
#define OZ_SE_ETHER_XMTREADY (OZ_IO_ETHER_BASE | 0x02)	/* the driver is able to accept a packet for transmit */

#endif
