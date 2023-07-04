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
/*  Bootp server that runs on linux					*/
/*  Used for booting the Alpha						*/
/*									*/
/************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define N2HL(nl) ((((((nl[0] << 8) + nl[1]) << 8) + nl[2]) << 8) + nl[3])
#define N2HW(nw) ((nw[0] << 8) + nw[1])

typedef unsigned char uByte;
typedef unsigned short uWord;
typedef unsigned int uLong;

typedef struct Bootp {

  /* Ethernet header */

  uByte dstenaddr[6];
  uByte srcenaddr[6];
  uByte enproto[2];

  /* IP header */

  uByte hdrlenver;
  uByte typeofserv;
  uByte totalen[2];
  uByte ident[2];
  uByte flags[2];
  uByte ttl;
  uByte ipproto;
  uByte hdrcksm[2];
  uByte srcipaddr[4];
  uByte dstipaddr[4];

  /* UDP header */

  uByte srcport[2];
  uByte dstport[2];
  uByte length[2];
  uByte cksm[2];

  /* BOOTP packet */

  uByte msgtype;	// 1=request, 2=reply
  uByte hwtype;		// 1=ethernet
  uByte hwaddrlen;	// hardware address length, 6=ethernet
  uByte hops;		// hop count (incremented by routers)
  uByte xid[4];		// transaction ID (random number)
  uByte seconds[2];	// seconds since started requesting
  uByte pad1[2];
  uByte ciaddr[4];	// client ip address
  uByte yiaddr[4];	// server's idea of client's ip address
  uByte siaddr[4];	// server's ip address
  uByte giaddr[4];	// gateway's ip address
  uByte chaddr[16];	// client hardware address, filled in by client
  uByte sname[64];	// optional server host name, null terminated
  uByte file[128];	// boot file name, null terminated string
  uByte vend[64];	// optional vendor-specific area
} Bootp;

/* Internal routines */

static void dumppacket (Bootp *pkt);
static char *ipaddrstr (uByte ipbin[4]);
static char *enaddrstr (uByte enbin[6]);

uWord oz_dev_ip_udpcksm (Bootp *ip);
uWord oz_dev_ip_gencksm (uLong nwords, const void *words, uWord start);

int main (int argc, char *argv[])

{
  Bootp reqbuf, rplbuf;
  int cliaddrlen, i, rc, rawfd;
  struct ifreq ifr;
  struct sockaddr cliaddr, svraddr;
  time_t now;
  uWord cksm;

  rawfd = socket (AF_INET, SOCK_PACKET, htons (0x0800));
  if (rawfd < 0) {
    fprintf (stderr, "error creating raw socket: %s\n", strerror (errno));
    return (-1);
  }

  memset (&svraddr, 0, sizeof svraddr);
  svraddr.sa_family      = AF_INET;
  strncpy (svraddr.sa_data, "eth0", sizeof svraddr.sa_data);

  if (bind (rawfd, &svraddr, sizeof svraddr) < 0) {
    fprintf (stderr, "error binding raw socket: %s\n", strerror (errno));
    return (-1);
  }

  memset (&ifr, 0, sizeof ifr);
  strncpy (ifr.ifr_name, "eth0", sizeof ifr.ifr_name);
  if (ioctl (rawfd, SIOCGIFHWADDR, &ifr) < 0) {
    fprintf (stderr, "error getting eth0 ethernet address: %s\n", strerror (errno));
    return (-1);
  }

  /* Read in bootp request */

recvloop:
  rc = recv (rawfd, &reqbuf, sizeof reqbuf, 0);
  if (rc < 0) {
    fprintf (stderr, "error receiving request: %s\n", strerror (errno));
    return (-1);
  }

  /* See if it even looks like a bootp request */

  if (rc != sizeof reqbuf) goto recvloop;

  if (reqbuf.enproto[0] != 0x08) goto recvloop;
  if (reqbuf.enproto[1] != 0x00) goto recvloop;
  if (reqbuf.hdrlenver  != 0x45) goto recvloop;
  if (reqbuf.ipproto    != 17)   goto recvloop;

  if (reqbuf.srcport[0] !=  0) goto recvloop;
  if (reqbuf.srcport[1] != 68) goto recvloop;
  if (reqbuf.dstport[0] !=  0) goto recvloop;
  if (reqbuf.dstport[1] != 67) goto recvloop;

  /* Print it out */

  time (&now);
  printf ("\nReceived %s", ctime (&now));
  dumppacket (&reqbuf);

  /* Make sure it's a request from the computer we're expecting */

  if (reqbuf.msgtype != 1) goto recvloop;
  if ((strcasecmp (enaddrstr (reqbuf.chaddr), "00-00-F8-7A-59-85") != 0) 
   && (strcasecmp (enaddrstr (reqbuf.chaddr), "00-E0-29-0E-26-80") != 0)) goto recvloop;

  /* Send reply */

  rplbuf = reqbuf;

  rplbuf.msgtype   = 2;
  rplbuf.yiaddr[0] = 192;	// requestor's (your) ip address
  rplbuf.yiaddr[1] = 168;
  rplbuf.yiaddr[2] = 2;
  rplbuf.yiaddr[3] = 102;
  rplbuf.siaddr[0] = 192;	// my (server) ip address
  rplbuf.siaddr[1] = 168;
  rplbuf.siaddr[2] = 2;
  rplbuf.siaddr[3] = 101;

  memcpy (rplbuf.dstenaddr, reqbuf.srcenaddr, 6);
  memcpy (rplbuf.srcenaddr, ifr.ifr_hwaddr.sa_data, 6);
  memcpy (rplbuf.dstipaddr, rplbuf.yiaddr, 4);
  memcpy (rplbuf.srcipaddr, reqbuf.dstipaddr, 4);
  memcpy (rplbuf.dstport, reqbuf.srcport, 2);
  memcpy (rplbuf.srcport, reqbuf.dstport, 2);

  strcpy (rplbuf.sname, "ozone.nii.net");
  strcpy (rplbuf.file, "/tftpboot/oz_loader_axp.eb");

  rplbuf.hdrcksm[0] = 0;
  rplbuf.hdrcksm[1] = 0;
  cksm = oz_dev_ip_gencksm ((rplbuf.hdrlenver & 15) * 2, &rplbuf.hdrlenver, 0xFFFF);
  rplbuf.hdrcksm[0] = cksm >> 8;
  rplbuf.hdrcksm[1] = cksm & 0xFF;

  rplbuf.cksm[0] = 0;
  rplbuf.cksm[1] = 0;
  cksm = oz_dev_ip_udpcksm (&rplbuf);
  rplbuf.cksm[0] = cksm >> 8;
  rplbuf.cksm[1] = cksm & 0xFF;

  printf ("\n");
  dumppacket (&rplbuf);

  memset (&cliaddr, 0, sizeof cliaddr);
  cliaddr.sa_family = AF_INET;
  strncpy (cliaddr.sa_data, "eth0", sizeof cliaddr.sa_data);

  rc = sendto (rawfd, &rplbuf, sizeof rplbuf, 0, &cliaddr, sizeof cliaddr);
  if (rc < 0) {
    fprintf (stderr, "error sending reply: %s\n", strerror (errno));
    return (-1);
  }

  if (rc != sizeof rplbuf) {
    fprintf (stderr, "sent %d bytes instead of %d\n", rc, sizeof rplbuf);
    return (-1);
  }

  goto recvloop;
}

static void dumppacket (Bootp *pkt)

{
  printf ("       msgtype %u\n", pkt -> msgtype);
  printf ("        hwtype %u\n", pkt -> hwtype);
  printf ("     hwaddrlen %u\n", pkt -> hwaddrlen);
  printf ("          hops %u\n", pkt -> hops);
  printf ("           xid 0x%X\n", N2HL (pkt -> xid));
  printf ("       seconds %u\n", N2HW (pkt -> seconds));
  printf ("        ciaddr %s\n", ipaddrstr (pkt -> ciaddr));
  printf ("        yiaddr %s\n", ipaddrstr (pkt -> yiaddr));
  printf ("        siaddr %s\n", ipaddrstr (pkt -> siaddr));
  printf ("        giaddr %s\n", ipaddrstr (pkt -> giaddr));
  printf ("        chaddr %s\n", enaddrstr (pkt -> chaddr));
  printf ("         sname %s\n", pkt -> sname);
  printf ("          file %s\n", pkt -> file);
}

static char *ipaddrstr (uByte ipbin[4])

{
  static char outbuf[32];

  sprintf (outbuf, "%u.%u.%u.%u", ipbin[0], ipbin[1], ipbin[2], ipbin[3]);
  return (outbuf);
}

static char *enaddrstr (uByte enbin[6])

{
  static char outbuf[32];

  sprintf (outbuf, "%2.2X-%2.2X-%2.2X-%2.2X-%2.2X-%2.2X", enbin[0], enbin[1], enbin[2], enbin[3], enbin[4], enbin[5]);
  return (outbuf);
}

/************************************/
/* Generate an udp message checksum */
/************************************/

uWord oz_dev_ip_udpcksm (Bootp *ip)

{
  uLong cksm, udplen;
  struct { uByte srcipaddr[4];
           uByte dstipaddr[4];
           uByte zero;
           uByte proto;
           uByte udplen[2];
         } pseudo;

  memcpy (pseudo.srcipaddr, ip -> srcipaddr, 4);
  memcpy (pseudo.dstipaddr, ip -> dstipaddr, 4);
  pseudo.zero  = 0;
  pseudo.proto = 17;
  pseudo.udplen[0] = ip -> length[0];
  pseudo.udplen[1] = ip -> length[1];

  udplen = N2HW (pseudo.udplen);
  if (udplen & 1) ip -> srcport[udplen++] = 0;

  cksm = oz_dev_ip_gencksm (sizeof pseudo / 2, &pseudo, 0xFFFF);
  cksm = oz_dev_ip_gencksm (udplen / 2, ip -> srcport, cksm);

  return (cksm);
}

/************************************************************************/
/*									*/
/*  Generate IP-style checksum for a list of network byte order words	*/
/*  Crude but effective							*/
/*									*/
/************************************************************************/

uWord oz_dev_ip_gencksm (uLong nwords, const void *words, uWord start)

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
