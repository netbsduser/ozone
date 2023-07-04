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
/*  Dump out all IP pakets on eth0					*/
/*									*/
/************************************************************************/

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
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PROTO_ARP 0x0806
#define PROTO_IP  0x0800

int main (int argc, char *argv[])

{
  unsigned char buff[2048];
  int i, ip_fd, j, rc;

  /* Create a socket to handle IP packets */

  ip_fd = socket (AF_INET, SOCK_PACKET, htons (PROTO_IP));
  if (ip_fd < 0) {
    perror ("error creating IP socket");
    return (-1);
  }

  /* Bind it to our device */

  memclr (&saddr, sizeof saddr);
  saddr.sa_family = AF_INET;
  strncpy (saddr.sa_data, "eth0", sizeof saddr.sa_data);

  if (bind (ip_fd, &saddr, sizeof saddr) < 0) {
    perror ("error binding to eth0");
    return (-1);
  }

  /* Read and print */

loop:
  rc = read (ip_fd, buff, sizeof buff);
  if (rc < 0) {
    perror ("error reading network");
    return (-1);
  }
  if (rc == 0) {
    fprintf (stderr, "eof reading network\n");
    return (-1);
  }

  for (i = 0; i < rc; i += 16) {
    for (j = 16; -- j >= 0;) {
      if ((i + j) >= rc) printf ("   ");
      else printf (" %2.2x", buff[i+j]);
    }
    printf (" : %4.4x\n", i);
  }

  goto loop;
}
