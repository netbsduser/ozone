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

#include <stdio.h>
#include <string.h>

static void putline (char *linebuff, int *llwb);

int main ()

{
  char linebuff[65536], *p, *q;
  int lastlinewasblank;

  lastlinewasblank = 1;
  while (fgets (linebuff, sizeof linebuff, stdin) != NULL) {
    while ((p = strstr (linebuff, "/*")) != NULL) {
lookforend:
      q = strstr (p, "*/");
      if (q == NULL) {
        *p = 0;
        putline (linebuff, &lastlinewasblank);
        if (fgets (linebuff, sizeof linebuff, stdin) == NULL) return (0);
        p = linebuff;
        goto lookforend;
      }
      q += 2;
      while (*q != 0) *(p ++) = *(q ++);
      *p = 0;
    }
    putline (linebuff, &lastlinewasblank);
  }
  return (0);
}

static void putline (char *linebuff, int *llwb)

{
  char *p, *q;

  while (((p = strstr (linebuff, " \n")) != NULL) || ((p = strstr (linebuff, "	\n")) != NULL)) {
    q = p + 1;
    while (*q != 0) *(p ++) = *(q ++);
    *p = 0;
  }
  if (strcmp (linebuff, "\n") == 0) {
    if (!*llwb) fputs (linebuff, stdout);
    *llwb = 1;
  } else {
    fputs (linebuff, stdout);
    *llwb = 0;
  }
}
