//+++2003-03-01
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
//---2003-03-01

// Test ftp to 65.85.95.150 port 35
// assumes there is an entry in ~/.netrc

#define NUMFORKS 3

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main ()

{
  char diffcmd[64], linebuff[4096];
  fd_set readfdmask;
  FILE *readpipefiles[NUMFORKS], *subproc;
  int exstat, i, mypid, pipedes[2], rc, readpipefds[NUMFORKS], subpids[NUMFORKS], writepipefds[NUMFORKS];

  /* Create pipes for sub-processes to write to */

  for (i = 0; i < NUMFORKS; i ++) {
    if (pipe (pipedes) < 0) {
      fprintf (stderr, "error creating pipe: %s\n", strerror (errno));
      return (-1);
    }
    readpipefds[i] = pipedes[0];
    writepipefds[i] = pipedes[1];
  }

  /* Create sub-processes */

  fflush (stderr);
  fflush (stdout);
  for (i = 0; i < NUMFORKS; i ++) {
    rc = fork ();
    if (rc == 0) goto subprocess;
    if (rc < 0) {
      fprintf (stderr, "error forking: %s\n", strerror (errno));
      return (-1);
    }
    subpids[i] = rc;
    close (writepipefds[i]);
    readpipefiles[i] = fdopen (readpipefds[i], "r");
    if (readpipefiles[i] == NULL) {
      fprintf (stderr, "error opening pipe as file: %s\n", strerror (errno));
      return (-1);
    }
    sleep (10 / NUMFORKS);
  }

  /* Read from sub-processes and print */

  while (1) {
    rc = 0;
    FD_ZERO (&readfdmask);
    for (i = 0; i < NUMFORKS; i ++) {
      if (rc <= readpipefds[i]) rc = readpipefds[i] + 1;
      FD_SET (readpipefds[i], &readfdmask);
    }
    rc = select (rc, &readfdmask, NULL, NULL, NULL);
    if (rc < 0) {
      fprintf (stderr, "error selecting: %s\n", strerror (errno));
      return (-1);
    }
    for (i = 0; i < NUMFORKS; i ++) {
      if (FD_ISSET (readpipefds[i], &readfdmask)) {
        if (fgets (linebuff, sizeof linebuff, readpipefiles[i]) == NULL) goto subprocexited;
        printf ("%d [%5d]: %s", i, subpids[i], linebuff);
      }
    }
  }

subprocexited:
  fprintf (stderr, "subprocess %d exited\n", subpids[i]);
  for (i = 0; i < NUMFORKS; i ++) kill (subpids[i], 9);
  return (0);

  /* Sub-process */

subprocess:
  if (dup2 (writepipefds[i], 1) < 0) {
    fprintf (stderr, "error re-directing stdout to pipe: %s\n", strerror (errno));
    return (-1);
  }
  if (dup2 (writepipefds[i], 2) < 0) {
    fprintf (stdout, "error re-directing stderr to pipe: %s\n", strerror (errno));
    return (-1);
  }
  while (i >= 0) {
    close (readpipefds[i]);
    close (writepipefds[i]);
    -- i;
  }
  mypid = getpid ();
  sprintf (diffcmd, "diff %u.tmp ftptest.dat > /dev/null", mypid);

loop:
  printf ("\nftp -v 65.85.95.150 35\n");
  fflush (stdout);
  fflush (stderr);
  subproc = popen ("ftp -v 65.85.95.150 35", "w");
  if (subproc == NULL) {
    fprintf (stderr, "error creating ftp process: %s\n", strerror (errno));
    exit (-1);
  }
  fprintf (subproc, "binary\n");
  fprintf (subproc, "get ftptest.dat %u.tmp\n", mypid);
  fprintf (subproc, "quit\n");
  pclose (subproc);
  printf ("\n%s\n", diffcmd);
  fflush (stdout);
  fflush (stderr);
  rc = system (diffcmd);
  fflush (stderr);
  printf ("diff rc %d\n", rc);
  fflush (stdout);
  fflush (stderr);
  if (rc == 0) goto loop;
  exit (rc);
}
