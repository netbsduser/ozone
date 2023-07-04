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
/*  Comport Pass-Thru program						*/
/*									*/
/*	comportpt <comport>						*/
/*									*/
/************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

int main (int argc, char *argv[])

{
  char *comname, kbtoline[256], linetott[256];
  fd_set readfds, writefds;
  int breakflag, comfd, kbtoline_l, linetott_l, rc;
  struct termios port_modified, port_original, term_modified, term_original;

  comname = NULL;
  breakflag = 0;
  for (rc = 1; rc < argc; rc ++) {
    if (strcasecmp (argv[rc], "-break") == 0) {
      breakflag = 1;
      continue;
    }
    if (argv[rc][0] == '-') goto usage;
    if (comname != NULL) goto usage;
    comname = argv[rc];
  }
  if (comname == NULL) goto usage;

  comfd = open (comname, O_RDWR);
  if (comfd < 0) {
    fprintf (stderr, "error opening %s: %s\n", comname, strerror (errno));
    return (-1);
  }

  if (tcgetattr (comfd, &port_original) < 0) {
    fprintf (stderr, "error getting comport attributes: %s\n", strerror (errno));
    return (-1);
  }

  if (tcgetattr (0, &term_original) < 0) {
    fprintf (stderr, "error getting terminal attributes: %s\n", strerror (errno));
    return (-1);
  }

  port_modified = port_original;
  term_modified = term_original;

  cfmakeraw (&port_modified);
  cfmakeraw (&term_modified);

  cfsetispeed (&port_modified, B9600);
  cfsetospeed (&port_modified, B9600);

//  term_modified.c_iflag = (term_modified.c_iflag & ~(INLCR | IGNCR | ICRNL | IUCLC)) | IXON | IXOFF | IGNBRK;
//  term_modified.c_oflag = (term_modified.c_oflag & ~(OLCUC | ONLCR | OCRNL | ONOCR | ONLRET | OFILL));
//  term_modified.c_cflag = (term_modified.c_cflag & ~(CSIZE | CSTOPB | PARENB | CLOCAL)) | CS8 | CREAD;
//  term_modified.c_lflag = (term_modified.c_lflag & ~(ISIG | ICANON | ECHO | TOSTOP));

  if (tcsetattr (comfd, TCSANOW, &port_modified) < 0) {
    fprintf (stderr, "error setting comport attributes: %s\n");
    return (-1);
  }

  if (tcsetattr (0, TCSANOW, &term_modified) < 0) {
    fprintf (stderr, "error setting terminal attributes: %s\n");
    return (-1);
  }

  if (breakflag) {
    if (tcsendbreak (comfd, 1) >= 0) fprintf (stderr, "break has been sent\r\n");
    else fprintf (stderr, "error sending break: %s\r\n", strerror (errno));
  }

  kbtoline_l = 0;	// keyboard-to-line buffer is empty
  linetott_l = 0;	// line-to-screen buffer is empty

  while (1) {

    /* Wait for something to do */

    FD_ZERO (&readfds);
    if (kbtoline_l < sizeof kbtoline) FD_SET (0, &readfds);	// maybe I can read from the keyboard
    if (linetott_l < sizeof linetott) FD_SET (comfd, &readfds);	// maybe I can read from the comport
    FD_ZERO (&writefds);
    if (kbtoline_l != 0) FD_SET (comfd, &writefds);		// maybe there's something to write to the comport
    if (linetott_l != 0) FD_SET (1, &writefds);			// maybe there's something to write to the screen
    rc = select (comfd + 1, &readfds, &writefds, NULL, NULL);
    if (rc < 0) {
      fprintf (stderr, "select error: %s\n", strerror (errno));
      return (-1);
    }

    /* Check for something to read from the keyboard */

    if (FD_ISSET (0, &readfds)) {
      rc = read (0, kbtoline + kbtoline_l, sizeof kbtoline - kbtoline_l);
      if (rc < 0) {
        fprintf (stderr, "error reading from stdin: %s\n", strerror (errno));
        return (-1);
      }
      if (memchr (kbtoline + kbtoline_l, '\\' - '@', rc) != NULL) break;
      kbtoline_l += rc;
    }

    /* Check for something to read from the comport */

    if (FD_ISSET (comfd, &readfds)) {
      rc = read (comfd, linetott + linetott_l, sizeof linetott - linetott_l);
      if (rc < 0) {
        fprintf (stderr, "error reading from comport: %s\n", strerror (errno));
        return (-1);
      }
      linetott_l += rc;
    }

    /* Check for something to write to the screen */

    if (FD_ISSET (1, &writefds)) {
      rc = write (1, linetott, linetott_l);
      if (rc < 0) {
        fprintf (stderr, "error writing from stdout: %s\n", strerror (errno));
        return (-1);
      }
      linetott_l -= rc;
      if (linetott_l > 0) memmove (linetott, linetott + rc, linetott_l);
    }

    /* Check for something to write to the comport */

    if (FD_ISSET (comfd, &writefds)) {
      rc = write (comfd, kbtoline, kbtoline_l);
      if (rc < 0) {
        fprintf (stderr, "error writing from stdout: %s\n", strerror (errno));
        return (-1);
      }
      kbtoline_l -= rc;
      if (kbtoline_l > 0) memmove (kbtoline, kbtoline + rc, kbtoline_l);
    }
  }

  fprintf (stderr, "\r\n");
  if (tcsetattr (comfd, TCSANOW, &port_original) < 0) fprintf (stderr, "error restoring comport attributes: %s\n");
  if (tcsetattr (0, TCSANOW, &term_original) < 0) fprintf (stderr, "error restoring terminal attributes: %s\n");
  return (0);

usage:
  fprintf (stderr, "usage: comportpt [-break] <comport device, eg, /dev/cua0>\n");
  return (-1);
}
