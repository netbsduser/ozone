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
/*  I/O function codes for device class "ip"				*/
/*									*/
/************************************************************************/

#ifndef _OZ_IO_IP_H
#define _OZ_IO_IP_H

#include "oz_knl_devio.h"
#include "oz_knl_process.h"
#include "oz_knl_thread.h"

#define OZ_IO_IP_CLASSNAME "ip"
#define OZ_IO_IP_BASE (0x00000700)
#define OZ_IO_IP_MASK (0xFFFFFF00)

#define OZ_IO_IP_ADDRSIZE (4)			/* number of bytes in an ip address */
#define OZ_IO_IP_PORTSIZE (2)			/* number of bytes in an udp or tcp port number */
#define OZ_IO_IP_MAXDATA (1500)			/* maximum number of data bytes possible in a packet */
#define OZ_IO_IP_DNSNAMMAX (256)		/* maximum dns name string length (including null) */
#define OZ_IO_IP_DNSNUMMAX (8)			/* maximum number of ip's to save per name */

#define OZ_IO_IP_DEV "ip"			/* the one and only device name */

#include "oz_knl_hw.h"

/* Host byte order <-> Network byte order conversions */

#define OZ_IP_H2NW(h,n) { (n)[0] = (h) >> 8; (n)[1] = (h) & 0xff; }
#define OZ_IP_H2NL(h,n) { (n)[0] = (h) >> 24; (n)[1] = (h) >> 16; (n)[2] = (h) >> 8; (n)[3] = (h); }
#define OZ_IP_N2HW(n) (((n)[0] << 8) | (n)[1])
#define OZ_IP_N2HL(n) (((n)[0] << 24) | ((n)[1] << 16) | ((n)[2] << 8) | (n)[3])

/* Format of an ICMP packet */

typedef struct { uByte type;			/* type */
                 uByte code;			/* code */
                 uByte cksm[2];			/* checksum */
                 uByte raw[1];			/* raw icmp data */
               } OZ_IO_ip_icmppkt;

/* Format of an UDP packet */

typedef struct { uByte sport[OZ_IO_IP_PORTSIZE]; /* source port number */
                 uByte dport[OZ_IO_IP_PORTSIZE]; /* destination port number */
                 uByte length[2];		/* length */
                 uByte cksm[2];			/* checksum */
                 uByte raw[1];			/* raw udp data */
               } OZ_IO_ip_udppkt;

/* Format of a TCP packet */

typedef struct { uByte sport[OZ_IO_IP_PORTSIZE]; /* source port number */
                 uByte dport[OZ_IO_IP_PORTSIZE]; /* destination port number */
                 uByte seq[4];			/* seq for start of data in this packet */
                 uByte ack[4];			/* ack of data received */
                 uByte flags[2];		/* flags */
                 uByte wsize[2];		/* remaining window size */
                 uByte cksm[2];			/* checksum */
                 uByte urgent[2];		/* urgent data offset */
                 uByte raw[1];			/* raw tcp data */
               } OZ_IO_ip_tcppkt;

/* Format of an IP packet */

typedef struct { uByte hdrlenver;		/* header length / version */
                 uByte typeofserv;		/* type of service */
                 uByte totalen[2];		/* total length */
                 uByte ident[2];		/* identifier */
                 uByte flags[2];		/* flags word */
                 uByte ttl;			/* time-to-live */
                 uByte proto;			/* protocol */
                 uByte hdrcksm[2];		/* header checksum */
                 uByte srcipaddr[OZ_IO_IP_ADDRSIZE]; /* source ip address */
                 uByte dstipaddr[OZ_IO_IP_ADDRSIZE]; /* destination ip address */
                 union { uByte raw[1];		/* raw ip data */
                         OZ_IO_ip_icmppkt icmp;
                         OZ_IO_ip_udppkt udp;
                         OZ_IO_ip_tcppkt tcp;
                       } dat;
               } OZ_IO_ip_ippkt;

/* Debug flags only available to kernel mode callers */

extern uLong oz_dev_ip_debug;

/* Checksumming utilities in the driver (callable from user mode) */

uWord oz_dev_ip_ipcksm   (const OZ_IO_ip_ippkt *ip);
uWord oz_dev_ip_icmpcksm (OZ_IO_ip_ippkt *ip);
uWord oz_dev_ip_udpcksm  (OZ_IO_ip_ippkt *ip);
uWord oz_dev_ip_tcpcksm  (OZ_IO_ip_ippkt *ip);
uWord oz_dev_ip_gencksm  (uLong nwords, const void *words, uWord start);

/************************************************************************/
/*  HARDWARE INTERFACE CONTROL						*/
/************************************************************************/

/* Add hardware interface */

#define OZ_IO_IP_HWADD OZ_IO_DW(OZ_IO_IP_BASE,1)

typedef struct { const char *devname;		/* device name (like an ethernet device) */
               } OZ_IO_ip_hwadd;

/* Remove hardware interface */

#define OZ_IO_IP_HWREM OZ_IO_DW(OZ_IO_IP_BASE,2)

typedef struct { const char *devname;		/* device name (like an ethernet device) */
               } OZ_IO_ip_hwrem;

/* Add IP address and mask to hardware interface */

#define OZ_IO_IP_HWIPAMADD OZ_IO_DW(OZ_IO_IP_BASE,3)

typedef struct { uLong addrsize;		/* OZ_IO_IP_ADDRSIZE */
                 const char *devname;		/* device name to add it to */
                 const uByte *hwipaddr;		/* hardware interface's ip address */
                 const uByte *nwipaddr;		/* network's ip address */
                 const uByte *nwipmask;		/* network's ip mask */
               } OZ_IO_ip_hwipamadd;

/* Remove IP address and mask from hardware interface */

#define OZ_IO_IP_HWIPAMREM OZ_IO_DW(OZ_IO_IP_BASE,4)

typedef struct { uLong addrsize;		/* OZ_IO_IP_ADDRSIZE */
                 const char *devname;		/* device name to remove it from */
                 const uByte *hwipaddr;		/* hardware interface's ip address */
                 const uByte *nwipaddr;		/* network's ip address */
               } OZ_IO_ip_hwipamrem;

/* List hardware interfaces */

#define OZ_IO_IP_HWLIST OZ_IO_DN(OZ_IO_IP_BASE,5)

typedef struct { uLong devnamesize;		/* size of device name buffer */
                 char *devnamebuff;		/* address of device name buffer */
               } OZ_IO_ip_hwlist;

/* List hardware interface ip address and mask */

#define OZ_IO_IP_HWIPAMLIST OZ_IO_DN(OZ_IO_IP_BASE,6)

typedef struct { uLong addrsize;		/* OZ_IO_IP_ADDRSIZE */
                 const char *devname;		/* device name */
                 uByte *hwipaddr;		/* hardware interface's ip address */
                 uByte *nwipaddr;		/* network's ip address */
                 uByte *nwipmask;		/* network's ip mask */
               } OZ_IO_ip_hwipamlist;

/************************************************************************/
/*  ARP CACHE CONTROL							*/
/************************************************************************/

/* Add arp entry */

#define OZ_IO_IP_ARPADD OZ_IO_DW(OZ_IO_IP_BASE,11)

typedef struct { uLong addrsize;		/* OZ_IO_IP_ADDRSIZE */
                 uLong enaddrsize;		/* OZ_IO_IP_ENADDRSIZE */
                 const uByte *ipaddr;
                 const uByte *enaddr;
                 const char *devname;
                 uLong timeout;			/* (in milliseconds, 0 for default, -1 for never) */
               } OZ_IO_ip_arpadd;

/* Remove arp entry */

#define OZ_IO_IP_ARPREM OZ_IO_DW(OZ_IO_IP_BASE,12)

typedef struct { uLong addrsize;		/* OZ_IO_IP_ADDRSIZE */
                 uLong enaddrsize;		/* OZ_IO_IP_ENADDRSIZE */
                 const uByte *ipaddr;
                 const uByte *enaddr;
                 const char *devname;
               } OZ_IO_ip_arprem;

/* List arp entries */

#define OZ_IO_IP_ARPLIST OZ_IO_DN(OZ_IO_IP_BASE,13)

typedef struct { uLong addrsize;		/* OZ_IO_IP_ADDRSIZE */
                 uLong enaddrsize;		/* OZ_IO_IP_ENADDRSIZE */
                 uByte *ipaddr;
                 uByte *enaddr;
                 const char *devname;
                 uLong *timeout;		/* (in milliseconds) */
               } OZ_IO_ip_arplist;

/************************************************************************/
/*  ROUTE TABLE CONTROL							*/
/************************************************************************/

/* Add routing entry */

#define OZ_IO_IP_ROUTEADD OZ_IO_DW(OZ_IO_IP_BASE,21)

typedef struct { uLong addrsize;		/* OZ_IO_IP_ADDRSIZE */
                 const uByte *gwipaddr;		/* ip address of gateway device to that network */
                 const uByte *nwipaddr;		/* network's ip address */
                 const uByte *nwipmask;		/* network's ip mask */
               } OZ_IO_ip_routeadd;

/* Remove routing entry */

#define OZ_IO_IP_ROUTEREM OZ_IO_DW(OZ_IO_IP_BASE,22)

typedef struct { uLong addrsize;		/* OZ_IO_IP_ADDRSIZE */
                 const uByte *gwipaddr;		/* ip address of gateway device to that network */
                 const uByte *nwipaddr;		/* network's ip address */
                 const uByte *nwipmask;		/* network's ip mask */
               } OZ_IO_ip_routerem;

/* List routing entries */

#define OZ_IO_IP_ROUTELIST OZ_IO_DN(OZ_IO_IP_BASE,23)

typedef struct { uLong addrsize;		/* OZ_IO_IP_ADDRSIZE */
                 uByte *gwipaddr;		/* ip address of gateway device to that network */
                 uByte *nwipaddr;		/* network's ip address */
                 uByte *nwipmask;		/* network's ip mask */
                 uLong devnamesize;		/* size of device name buffer */
                 char *devnamebuff;		/* address of device name buffer that router is connected to */
                 uByte *hwipaddr;		/* ip address of interface that sends to the router */
               } OZ_IO_ip_routelist;

/************************************************************************/
/*  FILTER LISTS							*/
/************************************************************************/

typedef enum { OZ_IO_IP_FILTER_INPUT = 1, 
               OZ_IO_IP_FILTER_FORWARD = 2,
               OZ_IO_IP_FILTER_OUTPUT = 3
             } OZ_IO_ip_filter_listid;

#define OZ_IO_IP_FILTER_ACTION_ACCEPT 0x1
#define OZ_IO_IP_FILTER_ACTION_TRACE  0x2

/* Add filter */

#define OZ_IO_IP_FILTERADD OZ_IO_DW(OZ_IO_IP_BASE,26)

typedef struct { OZ_IO_ip_filter_listid listid;
                 int index;
                 uLong action;
                 uLong size;
                 const OZ_IO_ip_ippkt *data;
                 const OZ_IO_ip_ippkt *mask;
               } OZ_IO_ip_filteradd;

/* Remove filter */

#define OZ_IO_IP_FILTERREM OZ_IO_DW(OZ_IO_IP_BASE,27)

typedef struct { OZ_IO_ip_filter_listid listid;
                 int index;
                 uLong action;
                 uLong size;
                 const OZ_IO_ip_ippkt *data;
                 const OZ_IO_ip_ippkt *mask;
               } OZ_IO_ip_filterrem;

/* List filter */

#define OZ_IO_IP_FILTERLIST OZ_IO_DN(OZ_IO_IP_BASE,28)

typedef struct { OZ_IO_ip_filter_listid listid;
                 int index;
                 uLong *action_r;
                 uLong size;
                 OZ_IO_ip_ippkt *data;
                 OZ_IO_ip_ippkt *mask;
               } OZ_IO_ip_filterlist;

/************************************************************************/
/*  RAW IP DATAGRAM ACCESS						*/
/************************************************************************/

/* Bind socket for IP datagram reception */

#define OZ_IO_IP_IPBIND OZ_IO_DR(OZ_IO_IP_BASE,31)

typedef struct { uLong addrsize;		/* OZ_IO_IP_ADDRSIZE */
                 const uByte *lclipaddr;	/* local interface's ip address (or NULL for any) */
                 const uByte *remipaddr;	/* remote interface's ip address (or NULL for any) */
                 int passiton;			/* 0: do not pass it on for normal processing */
						/* 1: pass it on for normal processing */
                 uByte proto;			/* IP protocol number (or 0 for any) */
               } OZ_IO_ip_ipbind;

/* Transmit IP packet */

#define OZ_IO_IP_IPTRANSMIT OZ_IO_DW(OZ_IO_IP_BASE,32)

typedef struct { uLong pktsize;
                 const OZ_IO_ip_ippkt *ippkt;
               } OZ_IO_ip_iptransmit;

/* Receive IP packet */

#define OZ_IO_IP_IPRECEIVE OZ_IO_DR(OZ_IO_IP_BASE,33)

typedef struct { uLong addrsize;		/* OZ_IO_IP_ADDRSIZE */
                 uLong pktsize;
                 OZ_IO_ip_ippkt *ippkt;
               } OZ_IO_ip_ipreceive;

/* Get info, part 1 */

#define OZ_IO_IP_IPGETINFO1 OZ_IO_DN(OZ_IO_IP_BASE,34)

typedef struct { uLong addrsize;		/* OZ_IO_IP_ADDRSIZE */
                 uByte *lclipaddr;		/* where to return local interface's ip address */
                 uByte *remipaddr;		/* where to return remote interface's ip address */
                 Long recvpend;			/* # of pending receives, -1 if not bound */
                 uLong recvcount;		/* # of receives queued */
                 uLong transcount;		/* # of transmits queued */
                 OZ_Threadid threadid;		/* threadid of last io */
                 int passiton;			/* 0: do not pass it on for normal processing */
						/* 1: pass it on for normal processing */
                 uByte proto;			/* IP protocol number (or 0 for any) */
               } OZ_IO_ip_ipgetinfo1;

/************************************************************************/
/*  UDP SOCKET ORIENTED DATAGRAM ACCESS					*/
/************************************************************************/

/* Bind to UDP socket for incoming data */

#define OZ_IO_IP_UDPBIND OZ_IO_DR(OZ_IO_IP_BASE,41)

typedef struct { uLong addrsize;		/* OZ_IO_IP_ADDRSIZE */
                 uLong portsize;		/* OZ_IO_IP_PORTSIZE */
                 const uByte *lclipaddr;	/* local interface's ip address, or NULL for any */
                 const uByte *lclportno;	/* local port number to bind to, or NULL to pick ephemeral */
                 const uByte *remipaddr;	/* remote interface's ip address, or NULL for any */
                 const uByte *remportno;	/* remote interface's port number, or NULL for any */
               } OZ_IO_ip_udpbind;

/* Transmit UDP packet */

#define OZ_IO_IP_UDPTRANSMIT OZ_IO_DW(OZ_IO_IP_BASE,42)

typedef struct { uLong addrsize;		/* OZ_IO_IP_ADDRSIZE */
                 uLong portsize;		/* OZ_IO_IP_PORTSIZE */
                 uLong rawsize;			/* size of data */
                 const void *rawbuff;		/* address of data */
                 const uByte *dstipaddr;	/* destination ip address */
                 const uByte *dstportno;	/* destination port number */
               } OZ_IO_ip_udptransmit;

/* Receive UDP packet */

#define OZ_IO_IP_UDPRECEIVE OZ_IO_DR(OZ_IO_IP_BASE,43)

typedef struct { uLong addrsize;		/* OZ_IO_IP_ADDRSIZE */
                 uLong portsize;		/* OZ_IO_IP_PORTSIZE */
                 uLong rawsize;			/* size of data buffer */
                 void *rawbuff;			/* address of data buffer */
                 uLong *rawrlen;		/* where to return data length received */
                 uByte *srcipaddr;		/* where to return source ip address */
                 uByte *srcportno;		/* where to return source udp port */
               } OZ_IO_ip_udpreceive;

/* Get info, part 1 */

#define OZ_IO_IP_UDPGETINFO1 OZ_IO_DN(OZ_IO_IP_BASE,44)

typedef struct { uLong addrsize;		/* OZ_IO_IP_ADDRSIZE */
                 uLong portsize;		/* OZ_IO_IP_PORTSIZE */
                 uByte *lclipaddr;		/* where to return local interface's ip address */
                 uByte *lclportno;		/* where to return local interface's port number */
                 uByte *remipaddr;		/* where to return remote interface's ip address */
                 uByte *remportno;		/* where to return remote interface's port number */
                 Long recvpend;			/* # of pending receives, -1 if not bound */
                 uLong recvcount;		/* # of receives queued */
                 uLong transcount;		/* # of transmits queued */
                 OZ_Threadid threadid;		/* threadid of last io */
               } OZ_IO_ip_udpgetinfo1;

/************************************************************************/
/*  TCP CONNECTION ORIENTED STREAM ACCESS				*/
/************************************************************************/

/* Connect to a TCP server */

#define OZ_IO_IP_TCPCONNECT OZ_IO_DN(OZ_IO_IP_BASE,51)

typedef struct { uLong addrsize;		/* OZ_IO_IP_ADDRSIZE */
                 uLong portsize;		/* OZ_IO_IP_PORTSIZE */
                 const uByte *dstipaddr;	/* destination ip address */
                 const uByte *dstportno;	/* destination port number */
                 uLong timeout;			/* timeout (in milliseconds) or zero for no timeout */
                 const uByte *srcipaddr;	/* source ip address (defaults to what most directly connects) */
                 const uByte *srcportno;	/* source port number (defaults to an unused number) */
                 uLong windowsize;		/* receive window buffer size (or 0 for default) */
                 uByte *windowbuff;		/* receive window buffer (or NULL to have it in system space) */
                 int window99s;			/* window buffer will be filled with 0x99's */
               } OZ_IO_ip_tcpconnect;

/* Close TCP connection */

typedef struct { int abort;
               } OZ_IO_ip_tcpclose;

#define OZ_IO_IP_TCPCLOSE OZ_IO_DN(OZ_IO_IP_BASE,52)

/* Listen for and accept an incoming TCP connection */

#define OZ_IO_IP_TCPLISTEN OZ_IO_DW(OZ_IO_IP_BASE,54)

typedef struct { uLong addrsize;		/* OZ_IO_IP_ADDRSIZE */
                 uLong portsize;		/* OZ_IO_IP_PORTSIZE */
                 uByte *lclipaddr;		/* local interface's ip address, or zeroes for any */
                 uByte *lclportno;		/* local port number to bind to */
                 uByte *remipaddr;		/* remote interface's ip address, or NULL for any */
                 uByte *remportno;		/* remote interface's port number, or NULL for any */
                 uByte *srcipaddr;		/* where to return source ip address */
                 uByte *srcportno;		/* where to return source tcp port */
                 uLong windowsize;		/* receive window buffer size (or 0 for default) */
                 uByte *windowbuff;		/* receive window buffer (or NULL to have it in system space) */
                 int window99s;			/* window buffer will be filled with 0x99's */
               } OZ_IO_ip_tcplisten;

/* Transmit TCP packet */

#define OZ_IO_IP_TCPTRANSMIT OZ_IO_DW(OZ_IO_IP_BASE,56)

typedef struct { uLong rawsize;			/* size of data */
                 const void *rawbuff;		/* address of data */
                 int debugme;
               } OZ_IO_ip_tcptransmit;

/* Receive TCP packet */

#define OZ_IO_IP_TCPRECEIVE OZ_IO_DR(OZ_IO_IP_BASE,57)

typedef struct { uLong rawsize;			/* size of data buffer (max size to get) */
                 void *rawbuff;			/* address of data buffer (or NULL to not copy, just leave it in windowbuff) */
                 uLong *rawrlen;		/* where to return data length received */
               } OZ_IO_ip_tcpreceive;

/* Get info, part 1 */

#define OZ_IO_IP_TCPGETINFO1 OZ_IO_DN(OZ_IO_IP_BASE,58)

typedef struct { uLong addrsize;		/* OZ_IO_IP_ADDRSIZE */
                 uLong portsize;		/* OZ_IO_IP_PORTSIZE */
                 uByte *lclipaddr;		/* where to return local interface's ip address */
                 uByte *lclportno;		/* where to return local interface's port number */
                 uByte *remipaddr;		/* where to return remote interface's ip address */
                 uByte *remportno;		/* where to return remote interface's port number */
                 Long recvpend;			/* # of pending receives, -1 if not bound */
                 Long transpend;		/* # of pending transmits (queued, maybe sent, but not acked) */
                 uLong recvcount;		/* # of receives queued */
                 uLong transcount;		/* # of transmits queued */
                 OZ_Threadid threadid;		/* threadid of last io */
                 uLong rcvwindowsize;		/* size of receive window buffer */
                 uByte *rcvwindowbuff;		/* address of receive window buffer */
                 OZ_Processid rcvwindowpid;	/* process that window buffer was created in */
                 uLong rcvwindowrem;		/* index of start of valid data in receive window */
                 uLong rcvwindownxt;		/* index of next data to retrieve from receive window */
                 uLong rcvwindowins;		/* index of end of next available byte in receive window */

                 uLong seq_lastacksent;		/* last seq we told it it could send */
                 uLong seq_receivedok;		/* last seq it sent us contiguously */

                 uLong seq_lastrcvdack;		/* last data of ours it acked */
                 uLong seq_nexttransmit;	/* next data we have queued to transmit */
                 uLong seq_lastrcvdwsize;	/* last data it said we could send */
                 uLong seq_nextuserdata;	/* what the next queued data seq will be */

                 uLong maxsendsize;		/* largest data that will go in a packet (not incl headers) */
                 uLong lastwsizesent;		/* last window size that was sent */

                 uLong state;			/* state bits */

               } OZ_IO_ip_tcpgetinfo1;

#define OZ_IO_IP_TCPSTATE_CLOSED  0x01		/* - tcpclose has been called */
#define OZ_IO_IP_TCPSTATE_RCVDFIN 0x02		/* - a FIN has been received */
#define OZ_IO_IP_TCPSTATE_RCVDSYN 0x04		/* - a SYN has been received */
#define OZ_IO_IP_TCPSTATE_RESET   0x08		/* - connection has been reset (via rcvd RST or other error) */
#define OZ_IO_IP_TCPSTATE_SENTRST 0x10		/* - a RST has been transmitted */
#define OZ_IO_IP_TCPSTATE_SENTFIN 0x20		/* - a FIN has been transmitted */
#define OZ_IO_IP_TCPSTATE_SENTSYN 0x40		/* - a SYN has been transmitted */

/* Set up a new TCP window */

#define OZ_IO_IP_TCPWINDOW OZ_IO_DW(OZ_IO_IP_BASE,59)

typedef struct { uLong windowsize;		/* receive window buffer size (or 0 for default) */
                 uByte *windowbuff;		/* receive window buffer (or NULL to have it in system space) */
                 int window99s;			/* window buffer will be filled with 0x99's */
               } OZ_IO_ip_tcpwindow;

/************************************************************************/
/*  DOMAIN NAME SERVER ACCESS						*/
/************************************************************************/

/* Add DNS server address to list */

#define OZ_IO_IP_DNSSVRADD OZ_IO_DW(OZ_IO_IP_BASE,71)

typedef struct { uLong addrsize;		/* OZ_IO_IP_ADDRSIZE */
                 uLong portsize;		/* OZ_IO_IP_PORTSIZE */
                 uByte *ipaddr;			/* ip address */
                 uByte *portno;			/* port number */
               } OZ_IO_ip_dnssvradd;

/* Remove DNS server address from list */

#define OZ_IO_IP_DNSSVRREM OZ_IO_DW(OZ_IO_IP_BASE,72)

typedef struct { uLong addrsize;		/* OZ_IO_IP_ADDRSIZE */
                 uLong portsize;		/* OZ_IO_IP_PORTSIZE */
                 uByte *ipaddr;			/* ip address */
                 uByte *portno;			/* port number */
               } OZ_IO_ip_dnssvrrem;

/* List DNS servers */

#define OZ_IO_IP_DNSSVRLIST OZ_IO_DN(OZ_IO_IP_BASE,73)

typedef struct { uLong addrsize;		/* OZ_IO_IP_ADDRSIZE */
                 uLong portsize;		/* OZ_IO_IP_PORTSIZE */
                 uByte *ipaddr;			/* ip address */
                 uByte *portno;			/* port number */
               } OZ_IO_ip_dnssvrlist;

/* Translate hostname to ip address */

#define OZ_IO_IP_DNSLOOKUP OZ_IO_DN(OZ_IO_IP_BASE,74)

typedef struct { const char *name;		/* the host name to be looked up */
                 uLong elsiz;			/* array element size (OZ_IO_IP_ADDRSIZE) */
                 uLong maxel;			/* maximum number of elements to return */
                 uByte *array;			/* where to return the ip addresses */
                 uLong *numel_r;		/* number of elements actually returned */
               } OZ_IO_ip_dnslookup;

#endif
