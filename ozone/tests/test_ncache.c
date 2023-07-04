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
/*  Test the oz_knl_ncache routines					*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_io_ip.h"
#include "oz_knl_event.h"
#include "oz_knl_kmalloc.h"
#include "oz_knl_ncache.h"
#include "oz_knl_status.h"

#include <stdio.h>

typedef struct { uLong numel;
                 uByte array[OZ_IO_IP_ADDRSIZE*OZ_KNL_NCACHE_MAXNUMEL];
                 char name[OZ_KNL_NCACHE_MAXNAMLEN];
               } Kp;

static uLong initlz (OZ_Procmode cprocmode, void *dummy);
static uLong lookup (OZ_Procmode cprocmode, void *kpv);

int main ()

{
  Kp kp;
  uLong i, j, sts;

  printf ("initializing name cache ...\n");
  sts = oz_sys_callknl (initlz, NULL);
  if (sts != OZ_SUCCESS) {
    printf ("error %u initializing name cache\n", sts);
    return (sts);
  }

loop:
  printf ("\nname to lookup? ");
  if (fgets (kp.name, sizeof kp.name, stdin) == NULL) return (OZ_SUCCESS);
  i = strlen (kp.name);
  if ((i > 0) && (kp.name[i-1] == '\n')) kp.name[--i] = 0;
  if (i == 0) goto loop;

  printf ("looking up '%s'\n", kp.name);
  sts = oz_sys_callknl (lookup, &kp);
  if (sts != OZ_SUCCESS) printf ("error %u looking up name\n", sts);
  else {
    for (i = 0; i < kp.numel; i ++) {
      printf ("  [%u]:", i);
      for (j = 0; j < OZ_IO_IP_ADDRSIZE; j ++) {
        printf ("%c%u", (j == 0) ? ' ' : '.', kp.array[i*OZ_IO_IP_ADDRSIZE+j]);
      }
      printf ("\n");
    }
  }

  goto loop;
}

static uLong initlz (OZ_Procmode cprocmode, void *dummy)

{
  int si;

  si = oz_hw_cpu_setsoftint (0);
  oz_knl_ncache_init ();	/* make sure it is initialized */
  oz_hw_cpu_setsoftint (si);
  return (OZ_SUCCESS);		/* successful */
}

static uLong lookup (OZ_Procmode cprocmode, void *kpv)

{
  int si;
  Kp kp;
  uLong sts;

  si = oz_hw_cpu_setsoftint (0);
  strcpy (kp.name, ((Kp *)kpv) -> name);
  sts = oz_knl_ncache_readw (kp.name, (sizeof kp.array) / OZ_IO_IP_ADDRSIZE, OZ_IO_IP_ADDRSIZE, kp.array, &kp.numel);
  memcpy (kpv, &kp, sizeof kp);
  oz_hw_cpu_setsoftint (si);
  return (sts);
}
