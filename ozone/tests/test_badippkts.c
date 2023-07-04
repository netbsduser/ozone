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
/*  Snoop the ethernet and print out bad ip packets			*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_io_ether.h"
#include "oz_io_ip.h"
#include "oz_knl_status.h"
#include "oz_sys_io.h"
#include "oz_sys_io_fs.h"
#include "oz_util_start.h"

#define IPADDRSIZE (4)
typedef uLong Ipad;

#define IPPKT(__ether_buff) ((Ippkt *)((__ether_buff) -> data))

#define H2NW(h,n) { (n)[0] = (h) >> 8; (n)[1] = (h) & 0xff; }
#define H2NL(h,n) { (n)[0] = (h) >> 24; (n)[1] = (h) >> 16; (n)[2] = (h) >> 8; (n)[3] = (h); }
#define N2HW(n) (((n)[0] << 8) | (n)[1])
#define N2HL(n) (((n)[0] << 24) | ((n)[1] << 16) | ((n)[2] << 8) | (n)[3])

#define CLRIPAD(d) *(Ipad *)(d) = 0                                             /* clear an ip address */
#define ZERIPAD(d) (*(Ipad *)(d) == 0)                                          /* test ip address for zero */
#define CEQIPAD(x,y) (*(Ipad *)(x) == *(Ipad *)(y))                             /* compare ip addresses */
#define CEQIPADM(x,y,m) (((*(Ipad *)(x) ^ *(Ipad *)(y)) & *(Ipad *)(m)) == 0)   /* compare ip addresses with mask */
#define CPYIPAD(d,s) *(Ipad *)(d) = *(Ipad *)(s)                                /* copy ip address */
#define CPYIPADM(d,s,m) *(Ipad *)(d) = *(Ipad *)(s) & *(Ipad *)(m)              /* copy ip address and mask it */

#define PROTO_ARP 0x0806
#define PROTO_IP  0x0800

#define PROTO_IP_ICMP 1         /* ICMP message type */
#define PROTO_IP_IGMP 2         /* IGMP message type (not used) */
#define PROTO_IP_TCP 6          /* TCP message type */
#define PROTO_IP_UDP 17		/* UDP message type */

#define Icmppkt OZ_IO_ip_icmppkt
#define Udppkt OZ_IO_ip_udppkt
#define Tcppkt OZ_IO_ip_tcppkt
#define Ippkt OZ_IO_ip_ippkt

static uWord ip_ipcksm (const Ippkt *ip);
static uWord ip_icmpcksm (const Ippkt *ip);
static uWord ip_udpcksm (Ippkt *ip);
static uWord ip_tcpcksm (Ippkt *ip);
static uWord ip_gencksm (uLong nwords, const void *words, uWord start);

uLong oz_util_main (int argc, char *argv[])

{
  char *devname;
  const char *em;
  Ippkt *ippkt;
  OZ_Handle h_iochan;
  OZ_IO_ether_open ether_open;
  OZ_IO_ether_receive ether_receive;
  OZ_IO_ether_buf etherbuf;
  uLong sts;

  if (argc != 2) {
    oz_sys_io_fs_printf (oz_util_h_error, "usage: badippkt <ether_dev_name>\n");
    return (OZ_MISSINGPARAM);
  }
  devname = argv[1];
  sts = oz_sys_io_assign (OZ_PROCMODE_KNL, &h_iochan, devname, OZ_LOCKMODE_CW);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printf (oz_util_h_error, "error %u assigning channel to %s\n", sts, devname);
    return (sts);
  }

  memset (&ether_open, 0, sizeof ether_open);
  ether_open.promis = 1;
  H2NW (PROTO_IP, ether_open.proto);

  memset (&ether_receive, 0, sizeof ether_receive);
  ether_receive.size = sizeof etherbuf;
  ether_receive.buff = &etherbuf;

receivepkt:
  sts = oz_sys_io (OZ_PROCMODE_KNL, h_iochan, 0, OZ_IO_ETHER_RECEIVE, sizeof ether_receive, &ether_receive);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printf (oz_util_h_error, "error %u receiving from %s\n", sts, devname);
    return (sts);
  }

  ippkt = IPPKT (&etherbuf);

  em = "bad hdrlenver";
  if (ippkt -> hdrlenver != 0x45) goto printpkt;
  em = "bad IP checksum";
  if (ip_ipcksm (ippkt) != 0xFFFF) goto printpkt;
  switch (ippkt -> proto) {
    case PROTO_IP_ICMP: {
      em = "bad ICMP checksum";
      if (ip_icmpcksm (ippkt) != 0xFFFF) goto printpkt;
      break;
    }
    case PROTO_IP_UDP: {
      em = "bad UDP checksum";
      if (ip_udpcksm (ippkt) != 0xFFFF) goto printpkt;
      break;
    }
    case PROTO_IP_TCP: {
      em = "bad TCP checksum";
      if (ip_tcpcksm (ippkt) != 0xFFFF) goto printpkt;
      break;
    }
    default: {
      em = "unknown IP protocol";
      goto printpkt;
    }
  }

  goto receivepkt;

printpkt:
  oz_sys_io_fs_printf (oz_util_h_error, "packet error: %s\n", em);
  oz_sys_io_fs_dumpmem (sizeof etherbuf.srcaddr, etherbuf.srcaddr);
  oz_sys_io_fs_dumpmem (sizeof etherbuf.dstaddr, etherbuf.dstaddr);
  oz_sys_io_fs_dumpmem (sizeof etherbuf.proto,   etherbuf.proto);
  oz_sys_io_fs_dumpmem (etherbuf.dlen,           etherbuf.data);
  goto receivepkt;
}

/************************************************************************/
/*									*/
/*  IP checksum routines						*/
/*									*/
/************************************************************************/

/* Generate an ip header checksum */

static uWord ip_ipcksm (const Ippkt *ip)

{
  return (ip_gencksm ((ip -> hdrlenver & 15) * 2, ip, 0xffff));
}

/* Generate an icmp message checksum */

static uWord ip_icmpcksm (const Ippkt *ip)

{
  uLong icmplen;

  icmplen  = N2HW (ip -> totalen);
  icmplen -= ip -> dat.raw - ((uByte *)ip);
  icmplen /= 2;

  return (ip_gencksm (icmplen, ip -> dat.raw, 0xffff));
}

/* Generate an udp message checksum */

static uWord ip_udpcksm (Ippkt *ip)

{
  int l;
  uLong cksm;
  struct { uByte srcipaddr[IPADDRSIZE];
           uByte dstipaddr[IPADDRSIZE];
           uByte zero;
           uByte proto;
           uByte udplen[2];
         } pseudo;

  CPYIPAD (pseudo.srcipaddr, ip -> srcipaddr);
  CPYIPAD (pseudo.dstipaddr, ip -> dstipaddr);
  pseudo.zero  = 0;
  pseudo.proto = PROTO_IP_UDP;
  pseudo.udplen[0] = ip -> dat.udp.length[0];
  pseudo.udplen[1] = ip -> dat.udp.length[1];

  l = N2HW (pseudo.udplen);
  if (l & 1) ip -> dat.raw[l++] = 0;

  cksm = ip_gencksm (sizeof pseudo / 2, &pseudo, 0xffff);
  cksm = ip_gencksm (l / 2, ip -> dat.raw, cksm);

  return (cksm);
}

/* Generate a tcp message checksum */

static uWord ip_tcpcksm (Ippkt *ip)

{
  int l;
  uLong cksm, tcplen;
  struct { uByte srcipaddr[IPADDRSIZE];
           uByte dstipaddr[IPADDRSIZE];
           uByte zero;
           uByte proto;
           uByte tcplen[2];
         } pseudo;

  tcplen = ((uByte *)ip) + N2HW (ip -> totalen) - ip -> dat.raw;

  CPYIPAD (pseudo.srcipaddr, ip -> srcipaddr);
  CPYIPAD (pseudo.dstipaddr, ip -> dstipaddr);
  pseudo.zero  = 0;
  pseudo.proto = PROTO_IP_TCP;
  H2NW (tcplen, pseudo.tcplen);

  if (tcplen & 1) ip -> dat.raw[tcplen++] = 0;

  cksm = ip_gencksm (sizeof pseudo / 2, &pseudo, 0xffff);
  cksm = ip_gencksm (tcplen / 2, ip -> dat.raw, cksm);

  return (cksm);
}

/* Generate checksum for a list of network byte order words */

static uWord ip_gencksm (uLong nwords, const void *words, uWord start)

{
  const uByte *r2;
  uLong r0, r1;

  r0 = 0xffff & ~ start;		/* get one's comp of start value */
  r2 = words;				/* point to array of words in network byte order */
  for (r1 = nwords; r1 != 0; -- r1) {	/* repeat as long as there is more to do */
    r0 += *(r2 ++) << 8;		/* add in high order byte */
    r0 += *(r2 ++);			/* add in low order byte */
  }
  while ((r1 = r0 >> 16) != 0) {	/* get end-around carries */
    r0 = (r0 & 0xffff) + r1;		/* add them back around */
  }					/* should only happen a total of up to 2 times */
  r0 = 0xffff & ~ r0;			/* get one's comp of result */
  if (r0 == 0) r0 = 0xffff;		/* if zero, return 0xffff (neg zero) */
  return (r0);
}
