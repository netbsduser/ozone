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

/************************************************************************/
/*									*/
/*  DNS query client for linux						*/
/*									*/
/*	dnsclient <host_name>						*/
/*									*/
/************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "ozone.h"

#define DNS_PORTNO 53
#define DNS_SERVER 209,113,172,16

static uByte server_ipaddr[] = { DNS_SERVER };

static void printmessage (uByte *buf);
static uByte *printnamestring (uByte *p, uByte *buf);

int main (int argc, char *argv[])

{
  uByte *countp, *p, reqbuf[512], rplbuf[512];
  char *q;
  int serveraddrlen, rc, udpfd;
  struct sockaddr_in serveraddr;
  time_t ident;

  if (argc != 2) {
    fprintf (stderr, "usage: dnsclient <host_name>\n");
    return (-1);
  }

  udpfd = socket (AF_INET, SOCK_DGRAM, 0);
  if (udpfd < 0) {
    fprintf (stderr, "error creating udp socket: %s\n", strerror (errno));
    return (-1);
  }

  memset (&serveraddr, 0, sizeof serveraddr);  
  serveraddr.sin_family = AF_INET;

  if (bind (udpfd, (void *)&serveraddr, sizeof serveraddr) < 0) {
    fprintf (stderr, "error binding udp socket: %s\n", strerror (errno));
    return (-1);
  }

  time (&ident);

  p = reqbuf;

  /** Header section **/

  /* Set up ident word = 16 lsb's of current time */

  *(p ++) = ident;
  *(p ++) = ident >> 8;

  /* Set up flag word = 0x0100 = recursion desired */

  *(p ++) = 0x01;
  *(p ++) = 0x00;

  /* Set up QDCOUNT = 1 (just one query) */

  *(p ++) = 0;
  *(p ++) = 1;

  /* Set up ANCOUNT = 0 (no answers in this message) */

  *(p ++) = 0;
  *(p ++) = 0;

  /* Set up NSCOUNT = 0 */

  *(p ++) = 0;
  *(p ++) = 0;

  /* Set up ARCOUNT = 0 */

  *(p ++) = 0;
  *(p ++) = 0;

  /** Query section **/

  /* Host name (baron.nii.net -> <5>baron<3>nii<3>net<0>) */

  countp = p;
  *(p ++) = 0;
  for (q = argv[1]; *q != 0; q ++) {
    if (*q != '.') {
      (*countp) ++;
      *(p ++) = *q;
    } else if (*countp != 0) {
      countp = p;
      *(p ++) = 0;
    }
  }
  if (*countp != 0) *(p ++) = 0;

  /* Type = 1 (A) : host address */

  *(p ++) = 0;
  *(p ++) = 1;

  /* Class = 1 (IN) : internet */

  *(p ++) = 0;
  *(p ++) = 1;

  /* Print request message */

  printf ("\nRequest:\n");
  printmessage (reqbuf);

  /* Send the request */

  memset (&serveraddr, 0, sizeof serveraddr);
  serveraddr.sin_family      = AF_INET;
  serveraddr.sin_port        = htons (DNS_PORTNO);
  serveraddr.sin_addr.s_addr = *(uLong *)server_ipaddr;

  if (sendto (udpfd, reqbuf, p - reqbuf, 0, (void *)&serveraddr, sizeof serveraddr) < 0) {
    fprintf (stderr, "error sending request: %s\n", strerror (errno));
    return (-1);
  }

  /* Read the reply */

  memset (&serveraddr, 0, sizeof serveraddr);
  serveraddrlen = sizeof serveraddr;
  rc = recvfrom (udpfd, &rplbuf, sizeof rplbuf, 0, (void *)&serveraddr, &serveraddrlen);
  if (rc < 0) {
    fprintf (stderr, "error receiving request: %s\n", strerror (errno));
    return (-1);
  }

  /* Print out results */

  printf ("\nReply:\n");
  printmessage (rplbuf);

  return (0);
}

#define GETLONG(__l,__p) do { __l = *(__p ++) << 24; __l |= *(__p ++) << 16; __l |= *(__p ++) << 8; __l |= *(p ++); } while (0)
#define GETWORD(__w,__p) do { __w = *(__p ++) << 8; __w |= *(p ++); } while (0)

static void printmessage (uByte *buf)

{
  uByte *p;
  uLong ancount, arcount, class, flags, i, ident, j, nscount, qdcount, rdlength, ttl, type;

  p = buf;

  GETWORD (ident, p);
  printf ("  ident = %x\n", ident);

  GETWORD (flags, p);
  printf ("  flags = %x :", flags);
  printf (" qr=%u", (flags >> 15) & 1);
  printf (" opcode=%u", (flags >> 11) & 15);
  printf (" aa=%u", (flags >> 10) & 1);
  printf (" tc=%u", (flags >> 9) & 1);
  printf (" rd=%u", (flags >> 8) & 1);
  printf (" ra=%u", (flags >> 7) & 1);
  printf (" z=%u",  (flags >> 4) & 7);
  printf (" rcode=%u\n", flags & 15);

  GETWORD (qdcount, p);
  printf ("  qdcount = %u\n", qdcount);

  GETWORD (ancount, p);
  printf ("  ancount = %u\n", ancount);

  GETWORD (nscount, p);
  printf ("  nscount = %u\n", nscount);

  GETWORD (arcount, p);
  printf ("  arcount = %u\n", arcount);

  for (i = 0; i < qdcount; i ++) {
    printf ("  qd[%u]:\n    ", i);
    while (*p != 0) {
      p = printnamestring (p, buf);
      if (*p != 0) printf (".");
    }
    p ++;
    printf ("\n");
    GETWORD (type, p);
    printf ("    type = %u\n", type);
    GETWORD (class, p);
    printf ("    class = %u\n", class);
  }

  for (i = 0; i < ancount; i ++) {
    printf ("  an[%u]:\n    ", i);
    p = printnamestring (p, buf);
    printf ("\n");
    GETWORD (type, p);
    printf ("    type = %u\n", type);
    GETWORD (class, p);
    printf ("    class = %u\n", class);
    GETLONG (ttl, p);
    printf ("    ttl = %u\n", ttl);
    GETWORD (rdlength, p);
    printf ("    rdlength = %u\n", rdlength);
    printf ("    rd =");
    for (j = 0; j < rdlength; j ++) {
      printf (" %2.2x(%u)", *p, *p);
      p ++;
    }
    printf ("\n");
  }
}

static uByte *printnamestring (uByte *p, uByte *buf)

{
  uLong nchars, offset;

  nchars = *(p ++);
  if ((nchars & 0xc0) == 0xc0) {
    offset  = (nchars & 0x3f) << 8;
    offset |= *(p ++);
    nchars  = buf[offset++];
    printf ("%*.*s", nchars, nchars, buf + offset);
  } else {
    printf ("%*.*s", nchars, nchars, p);
    p += nchars;
  }

  return (p);
}
