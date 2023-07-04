//+++2001-10-06
//    Copyright (C) 2001, Mike Rieker, Beverly, MA USA
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
//---2001-10-06

#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* Basic data types */

typedef char Byte;		/* 8-bit signed */
typedef unsigned char uByte;	/* 8-bit unsigned */
typedef short Word;		/* 16-bit signed */
typedef unsigned short uWord;	/* 16-bit unsigned */
typedef int Long;		/* 32-bit signed */
typedef unsigned int uLong;	/* 32-bit unsigned */

typedef double Doub;		/* 64-bit floating point */

/* Host byte order <-> Network byte order conversions */

#define H2NW(h,n) do { (n)[0] = (h) >> 8; (n)[1] = (h) & 0xff; } while (0)
#define H2NL(h,n) do { (n)[0] = (h) >> 24; (n)[1] = (h) >> 16; (n)[2] = (h) >> 8; (n)[3] = (h); } while (0)
#define N2HW(n) (((n)[0] << 8) | (n)[1])
#define N2HL(n) (((n)[0] << 24) | ((n)[1] << 16) | ((n)[2] << 8) | (n)[3])

#define MALLOC mymalloc
#define FREE myfree
#define memclr(b,s) memset ((b), 0, (s))

#define ENADDRSIZE 6		/* bytes in an ethernet address */
#define IPADDRSIZE 4		/* bytes in an ip address */
#define DATASIZE 1500		/* max size an ethernet controller can send or receive */
#define TPS 10			/* timer resolution (ticks per second) */
#define NAMESIZE 64		/* max parameter name string size (including null) */
#define VIRTHOSTNAMESIZE 256	/* virtual host name string size */
#define DYNTIMER_INT (5 * TPS)	/* how often to adjust throttles (ticks) */

/* Ethernet protocol numbers */

#define PROTO_ARP 0x0806
#define PROTO_IP  0x0800

/* IP related stuff */

#define PROTO_IP_ICMP 1		/* ICMP message type */
#define PROTO_IP_IGMP 2		/* IGMP message type (not used) */
#define PROTO_IP_TCP 6		/* TCP message type */
#define PROTO_IP_UDP 17		/* UDP message type */

#define IP_FLAGS_CE   0x8000	/* congestion */
#define IP_FLAGS_DF   0x4000	/* don't fragment packet */
#define IP_FLAGS_MF   0x2000	/* more fragments follow */
#define IP_FLAGS_OFFS 0x1fff	/* fragment offset */
#define IP_FLAGS_SHFT 3		/* fragment offset shift */
#define IP_FRAG_LIFE (30 * TPS)	/* fragment lifetime (ticks) */

#define IP_LOOPBACK 127		/* ip loopback network address */

/* ICMP related stuff */

#define PROTO_IP_ICMP_PINGRPL 0		/* ping reply message */
#define PROTO_IP_ICMP_DESTUNR 3		/* destination unreachable */
#define PROTO_IP_ICMP_SQUENCH 4		/* source quench */
#define PROTO_IP_ICMP_PINGREQ 8		/* ping request message */
#define PROTO_IP_ICMP_TIMEEXC 11	/* time-to-live exceeded message */
#define PROTO_IP_ICMP_PRMPROB 12	/* parameter problem message */
#define PROTO_IP_ICMP_TIMEREQ 13	/* current system time request */
#define PROTO_IP_ICMP_TIMERPL 14	/* current system time reply */

#define ICMP_BOUNCE_TOTALEN (576)	/* maximum total length on an ICMP bounce message */
#define ICMP_BOUNCE_TTL (127)		/* ICMP bounce message time-to-live */
#define ICMP_BOUNCE_INTERVAL (1)	/* wait this many ticks between sending ICMP bounce messages on a given device */

/* Copy and compare ip and ethernet addresses */

#define ISNIPAD(x) (*(uLong *)(x) == 0)
#define CEQIPAD(x,y) (*(uLong *)(x) == *(uLong *)(y))
#define CEQIPADM(x,y,m) ((*(uLong *)(x) ^ *(uLong *)(y)) & *(uLong *)(m) == 0)
#define CEQIPADMM(x,y,m,n) ((*(uLong *)(x) ^ *(uLong *)(y)) & *(uLong *)(m) & *(uLong *)(n) == 0)
#define CGTIPAD(x,y) (memcmp (x,y,IPADDRSIZE) > 0)
#define CPYIPAD(d,s) *(uLong *)(d) = *(uLong *)(s)

#define CPYENAD(d,s) memcpy (d, s, ENADDRSIZE)

/* Format of an ICMP packet */

typedef struct { uByte type;		/* type */
                 uByte code;		/* code */
                 uByte cksm[2];		/* checksum */
                 uByte raw[1024];	/* raw icmp data */
               } Icmppkt;

/* Format of an IP packet */

typedef struct { uByte hdrlenver;		/* header length / version */
                 uByte typeofserv;		/* type of service */
                 uByte totalen[2];		/* total length */
                 uByte ident[2];		/* identifier */
                 uByte flags[2];		/* flags word */
                 uByte ttl;			/* time-to-live */
                 uByte proto;			/* protocol */
                 uByte hdrcksm[2];		/* header checksum */
                 uByte srcipaddr[IPADDRSIZE];	/* source ip address */
                 uByte dstipaddr[IPADDRSIZE];	/* destination ip address */
                 union { uByte raw[DATASIZE];	/* raw ip data */
                         Icmppkt icmp;
                       } dat;
               } Ippkt;

/* Internal subroutine prototypes */

static int getipaddr (char *p, uByte ipaddr[IPADDRSIZE]);
static int getipaddrnum (char *p, uByte ipaddr[IPADDRSIZE]);
static void printipaddr (const uByte ipaddr[IPADDRSIZE]);
static void sprintipaddr (char *buf, const uByte ipaddr[IPADDRSIZE]);
static uWord ipcksm (Ippkt *ip);
static uWord icmpcksm (Icmppkt *icmp, int icmplen);
static uWord gencksm (int wordcount, void *buffer, uWord start);

/************************************************************************/
/*									*/
/*  Main program							*/
/*									*/
/************************************************************************/

int main (int argc, char *argv[])

{
  int ip_fd, rcvadrl, rcvlen, sndlen;
  int i, j;
  Icmppkt sndbuf;
  Ippkt rcvbuf;
  uLong count, sequence;
  struct timeval tbeg, tend;
  struct sockaddr_in rcvadr, sndadr;
  uWord cksm, totalen;

  if (argc != 4) {
    printf ("usage: pingtest <dstipaddr> <length> <count>\n");
    return (-1);
  }

  memset (&sndadr, 0, sizeof sndadr);

  if (!getipaddr (argv[1], (uByte *)&sndadr.sin_addr.s_addr)) {
    printf ("bad ip address %s\n", argv[2]);
    return (-1);
  }

  totalen = atoi (argv[2]);
  if (totalen <   64) totalen =   64;
  if (totalen > 1024) totalen = 1024;

  count = atoi (argv[3]);

  /* Create a socket to handle IP packets */

  ip_fd = socket (AF_INET, SOCK_RAW, PROTO_IP_ICMP);
  if (ip_fd < 0) {
    printf ("error creating IP socket\n%s\n", strerror (errno));
    return (-1);
  }

  /* Set up fixed portion of send buffer */

  cksm = getpid ();

  memset (&sndbuf, 0, sizeof sndbuf);
  sndbuf.type = PROTO_IP_ICMP_PINGREQ;

  /* Loop as long as not terminated */

  gettimeofday (&tbeg, NULL);
  for (sequence = 1; sequence <= count; sequence ++) {
    H2NL (sequence, sndbuf.raw);
    H2NW (0, sndbuf.cksm);
    cksm = icmpcksm (&sndbuf, totalen - 20);
    H2NW (cksm, sndbuf.cksm);
    sndlen = sendto (ip_fd, &sndbuf, totalen - 20, 0, (void *)&sndadr, sizeof sndadr);
    if (sndlen < 0) {
      printf ("error sending buffer\n%s\n", strerror (errno));
      return (-1);
    }

    while (1) {
      memset (&rcvadr, 0, sizeof rcvadr);
      rcvadrl = sizeof rcvadr;
      rcvlen = recvfrom (ip_fd, &rcvbuf, sizeof rcvbuf, 0, (void *)&rcvadr, &rcvadrl);
      if (rcvlen < 0) {
        printf ("receive error\n%s\n", strerror (errno));
        return (-1);
      }

      if (rcvlen == totalen) {
        for (i = 0; i < 8; i ++) if (rcvbuf.dat.icmp.raw[i] != sndbuf.raw[i]) break;
        if (i == 8) break;
      }
      printf ("bad packet %u\n", sequence);
    }
  }
  gettimeofday (&tend, NULL);

  tend.tv_sec  -= tbeg.tv_sec;
  tend.tv_usec -= tbeg.tv_usec;
  while (tend.tv_usec < 0) {
    tend.tv_usec += 1000000;
    tend.tv_sec --;
  }

  printf ("echoed %d bytes in %d.%6.6d seconds\n", totalen * count, tend.tv_sec, tend.tv_usec);
  printf ("average %f bytes per second\n", ((Doub)totalen) * ((Doub)count) / (((Doub)tend.tv_sec) + ((Doub)tend.tv_usec) / 1000000.0));

  return (0);
}

/************************************************************************/
/*									*/
/*  Get ip address in numeric or symbolic form				*/
/*									*/
/*    Input:								*/
/*									*/
/*	p = pointer to null terminated ip address string		*/
/*									*/
/*    Output:								*/
/*									*/
/*	getipaddr = 0 : failure						*/
/*	            1 : success						*/
/*	*ipaddr = filled in with ip address				*/
/*									*/
/************************************************************************/

static int getipaddr (char *p, uByte ipaddr[IPADDRSIZE])

{
  struct hostent *he;

  /* If it is numeric, just do it that way */

  if (getipaddrnum (p, ipaddr)) return (1);

  /* Otherwise, look it up in /etc/hosts or dns */

  he = gethostbyname (p);
  if (he == NULL) return (0);
  if (he -> h_addrtype != AF_INET) {
    printf ("address type %d of %s is not AF_INET\n", he -> h_addrtype, p);
    return (0);
  }
  if (he -> h_length != IPADDRSIZE) {
    printf ("address length %d of %s is not %d\n", he -> h_length, p, IPADDRSIZE);
    return (0);
  }
  CPYIPAD (ipaddr, he -> h_addr);
  return (1);
}

/************************************************************************/
/*									*/
/*  Get ip address in numeric form only					*/
/*									*/
/*    Input:								*/
/*									*/
/*	p = pointer to null terminated ip address string		*/
/*									*/
/*    Output:								*/
/*									*/
/*	getipaddrnum = 0 : failure					*/
/*	               1 : success					*/
/*	*ipaddr = filled in with ip address				*/
/*									*/
/************************************************************************/

static int getipaddrnum (char *p, uByte ipaddr[IPADDRSIZE])

{
  char *q;
  int i;
  uLong v;

  for (i = 0; i < IPADDRSIZE; i ++) {
    v = strtoul (p, &q, 0);
    if (v > 255) return (0);
    if ((*q != '.') && (*q != 0)) return (0);
    if ((*q != 0) && (i >= IPADDRSIZE - 1)) return (0);
    if ((*q == 0) && (i != IPADDRSIZE - 1)) return (0);
  }
  return (1);
}

/************************************************************************/
/*									*/
/*  Print an ip address							*/
/*									*/
/************************************************************************/

static void printipaddr (const uByte ipaddr[IPADDRSIZE])

{
  char buf[4*IPADDRSIZE+4];

  sprintipaddr (buf, ipaddr);
  fputs (buf, stdout);
}

/************************************************************************/
/*									*/
/*  Print an ip address into a buffer					*/
/*									*/
/************************************************************************/

static void sprintipaddr (char *buf, const uByte ipaddr[IPADDRSIZE])

{
  int i;

  sprintf (buf, "%u", ipaddr[0]);
  for (i = 1; i < IPADDRSIZE; i ++) {
    sprintf (buf + strlen (buf), ".%u", ipaddr[i]);
  }
}

/************************************************************************/
/*									*/
/*  Generate an ip header checksum					*/
/*									*/
/************************************************************************/

static uWord ipcksm (Ippkt *ip)

{
  return (gencksm ((ip -> hdrlenver & 15) * 2, ip, 0xffff));
}

/************************************************************************/
/*									*/
/*  Generate an icmp message checksum					*/
/*									*/
/************************************************************************/

static uWord icmpcksm (Icmppkt *icmp, int icmplen)

{
  if (icmplen & 1) ((uByte *)icmp)[icmplen++] = 0;
  return (gencksm (icmplen / 2, icmp, 0xffff));
}

/************************************************************************/
/*									*/
/*  Generate IP-style checksum						*/
/*									*/
/************************************************************************/

static uWord gencksm (int wordcount, void *buffer, uWord start)

{
  uByte *r3;
  int r2;
  uLong r0, r1;

  r0 = 0xffff & ~start;				/* get one's comp of start value */
  r3 = buffer;					/* point to data buffer */
  for (r2 = wordcount; -- r2 >= 0;) {		/* loop for each word */
    r1 = *(r3 ++);				/* get a byte, zero extended */
    r1 <<= 8;					/* shift it over */
    r1 |= *(r3 ++);				/* insert second byte */
    r0 += r1;					/* add to accumulator */
  }
  while ((r1 = r0 >> 16) != 0) r0 = (r0 & 0xffff) + r1; /* process end-around carries */
  r0 ^= 0xffff;					/* get one's comp of result */
  if (r0 == 0) r0 = 0xffff;			/* if zero, return FFFF */
  return (r0);
}
