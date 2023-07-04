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

//  cc -DOZ_HW_TYPE_486 -I../includes -g -o idno_test idno_test.c oz_knl_idno.c

#include "ozone.h"
#include "oz_knl_idno.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

int main ()

{
  char givenout[65536];
  int i;
  uLong idno;

  memset (givenout, 0, sizeof givenout);
  givenout[0] = 1;

  for (i = 0; i < 4095; i ++) {
    idno = oz_knl_idno_alloc (&idno);
    printf (" %u ", idno);
    if (idno > sizeof givenout) {
      printf (" - too big for givenout array\n");
      abort ();
    }
    if (givenout[idno]) {
      printf (" - already given out\n");
      abort ();
    }
    givenout[idno] = 1;
    if ((i % 3) == 2) oz_knl_idno_free (idno);
  }
  printf ("\n");
  return (0);
}

void oz_knl_printk (char *format, ...)

{
  va_list ap;

  va_start (ap, format);
  vfprintf (stdout, format, ap);
  va_end (ap);
}

void oz_crash (char *format, ...)

{
  va_list ap;

  va_start (ap, format);
  vfprintf (stdout, format, ap);
  va_end (ap);
  printf ("\n");
  abort ();
}

typedef int OZ_Smplock;
typedef uLong OZ_Memsize;

OZ_Smplock oz_s_smplock_id;

uLong oz_hw_smplock_wait (OZ_Smplock *smplock)

{
  return (1);
}

void oz_hw_smplock_clr (OZ_Smplock *smplock, uLong oldlevel)

{ }

void *oz_knl_nppmalloq (OZ_Memsize size, int quotaed)

{
  return (malloc (size));
}

void oz_knl_nppfree (void *adrs)

{
  free (adrs);
}
