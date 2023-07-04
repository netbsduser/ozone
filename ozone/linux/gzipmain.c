//+++2002-05-10
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
//---2002-05-10

/************************************************************************/
/*									*/
/*  GZIP utiliti							*/
/*  The idiot linux gzip utility tries to zip itself along with my file	*/
/*									*/
/************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gzip.h"

static char *pn = "gzip";

static char *input_file, *output_file;
static int h_in, h_out;
static int exstat;

static int readroutine (void *dummy, int siz, char *buf, int *len, char **pnt);
static int writeroutine (void *dummy, int siz, char *buf);
static void errorroutine (void *dummy, int code, char *msg);
static void *mallocroutine (void *dummy, int size);
static void freeroutine (void *dummy, void *buff);

int main (int argc, char *argv[])

{
  char *p;
  int fc;

  if (argc > 0) {
    pn = argv[0];
    p  = strrchr (pn, '/');
    if (p != NULL) pn = ++ p;
  }
  if (argc != 3) {
    fprintf (stderr, "usage: %s <input> <output>\n", pn);
    return (-1);
  }

  fc = GZIP_FUNC_DUMMY;
  if (strncasecmp (pn, "gzip", 4) == 0) fc = GZIP_FUNC_COMPRESS;
  if (strncasecmp (pn, "gunzip", 6) == 0) fc = GZIP_FUNC_EXPAND;
  if (fc == GZIP_FUNC_DUMMY) {
    fprintf (stderr, "%s: program name must begin with gzip or gunzip\n", pn);
    return (-1);
  }

  input_file = argv[1];
  h_in = open (input_file, O_RDONLY);
  if (h_in < 0) {
    fprintf (stderr, "%s: error %u opening input file %s\n", pn, errno, input_file);
    return (-1);
  }

  output_file = argv[2];
  h_out = open (output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (h_out < 0) {
    fprintf (stderr, "%s: error %u creating output file %s\n", pn, errno, output_file);
    return (-1);
  }

  exstat = 0;
  gzip (readroutine, writeroutine, errorroutine, mallocroutine, freeroutine, NULL, fc, 6);

  if (exstat == 0) {
    if (close (h_out) < 0) {
      fprintf (stderr, "%s: error %u closing output file %s\n", pn, errno, output_file);
      exstat = -1;
    }
  }

  return (0);
}

static int readroutine (void *dummy, int siz, char *buf, int *len, char **pnt)

{
  int rlen;

  rlen = read (h_in, buf, siz);
  if (rlen < 0) {
    fprintf (stderr, "%s: error %u reading %s\n", pn, errno, input_file);
    exstat = -1;
    return (0);
  }

  *len = rlen;
  *pnt = buf;
  return (1);
}

static int writeroutine (void *dummy, int siz, char *buf)

{
  int wlen;

  wlen = write (h_out, buf, siz);
  if (wlen < 0) {
    fprintf (stderr, "%s: error %u writing %s\n", pn, errno, output_file);
    exstat = -1;
    return (0);
  }
  if (wlen != siz) {
    fprintf (stderr, "%s: only wrote %u bytes of %u to %s\n", pn, wlen, siz, output_file);
    exstat = -1;
    return (0);
  }
  return (1);
}

static void errorroutine (void *dummy, int code, char *msg)

{
  fprintf (stderr, "%s: gzip internal error %d: %s\n", pn, code, msg);
  exstat = -1;
}

static void *mallocroutine (void *dummy, int size)

{
  return (malloc (size));
}

static void freeroutine (void *dummy, void *buff)

{
  free (buff);
}
