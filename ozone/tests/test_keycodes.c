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
/*  Display keycodes							*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_knl_status.h"
#include "oz_sys_callknl.h"

#define KB_CP 0x64	/* keyboard command port */
#define KB_DP 0x60	/* keyboard data port */

static uLong inknl (OZ_Procmode cprocmode, void *dummy);
static int write_cp (char *what, uByte data);
static int read_dp (char *what, uByte *data_r);
static int write_dp (char *what, uByte data);
static uLong getkeychar (void *dummy);

int main ()

{
  uLong sts;

  sts = oz_sys_callknl (inknl, NULL);
  oz_sys_io_fs_printerror ("status %u\n", sts);
  return (sts);
}

static uLong inknl (OZ_Procmode cprocmode, void *dummy)

{
  int i, j;
  uByte cb;

  oz_hw_cpu_sethwints (0);

  if (!write_cp ("enable keyboard", 0xae)) return (OZ_TIMEDOUT);
  if (!write_cp ("enable mouse", 0xa8)) return (OZ_TIMEDOUT);

  if (!write_cp ("write mouse command", 0xd3)) return (OZ_TIMEDOUT);
  if (!write_dp ("write mouse data", 0x69)) return (OZ_TIMEDOUT);

#if 00
  if (!write_cp ("read command byte", 0x20)) return (OZ_TIMEDOUT);
  if (!read_dp ("get command byte", &cb)) return (OZ_TIMEDOUT);

  cb |= 3;
  if (!write_cp ("write command byte", 0x60)) return (OZ_TIMEDOUT);
  if (!write_dp ("put command byte", cb)) return (OZ_TIMEDOUT);
#endif

  oz_knl_printk ("scanning\n");
  oz_hw_stl_delay (5000000, getkeychar, NULL);
  oz_knl_printk ("\ndone\n");
  oz_hw_cpu_sethwints (1);

  return (OZ_SUCCESS);
}

/* Write to command port, checking for ready first */

static int write_cp (char *what, uByte data)

{
  int i;
  uByte al;

  oz_knl_printk ("%s: ", what);
  for (i = 0; i < 1000000; i ++) {
    al = oz_hw_inb (KB_CP);
    if (!(al & 0x02)) {
      oz_hw_outb (data, KB_CP);
      oz_knl_printk ("%2.2x\n", data);
      return (1);
    }
  }
  oz_knl_printk ("timeout\n");
  oz_hw_cpu_sethwints (1);
  return (0);
}

/* Read from the data port, checking for ready first */

static int read_dp (char *what, uByte *data_r)

{
  int i;
  uByte al;

  oz_knl_printk ("%s: ", what);
  for (i = 0; i < 1000000; i ++) {
    al = oz_hw_inb (KB_CP);
    if (al & 0x01) {
      *data_r = oz_hw_inb (KB_DP);
      oz_knl_printk ("%2.2x\n", *data_r);
      return (1);
    }
  }
  oz_knl_printk ("timeout\n");
  oz_hw_cpu_sethwints (1);
  return (0);
}

/* Write to the data port, checking for ready first */

static int write_dp (char *what, uByte data)

{
  int i;
  uByte al;

  oz_knl_printk ("%s: ", what);
  for (i = 0; i < 1000000; i ++) {
    al = oz_hw_inb (KB_CP);
    if (!(al & 0x02)) {
      oz_hw_outb (data, KB_DP);
      oz_knl_printk ("%2.2x\n", data);
      return (1);
    }
  }
  oz_knl_printk ("timeout\n");
  oz_hw_cpu_sethwints (1);
  return (0);
}

/* This routine is called for 5 seconds to display anything that comes in */

static uLong getkeychar (void *dummy)

{
  uByte ah, al;

  while (1) {
    al = oz_hw_inb (KB_CP);
    oz_knl_printk ("\r %2.2x", al);
    if ((al & 0x21) == 0) return (0);
    oz_knl_printk (" ");
    if (al & 0x01) oz_knl_printk ("k");
    if (al & 0x20) oz_knl_printk ("m");
    ah = oz_hw_inb (KB_DP);
    oz_knl_printk ("%2.2x\n", ah);
  }
}
