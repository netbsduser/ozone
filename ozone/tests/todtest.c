//+++2001-10-24
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
//---2001-10-24

/************************************************************************/
/*									*/
/*  Test the oz_hw486_tod.s routines as user-mode routines in linux	*/
/*									*/
/************************************************************************/

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

unsigned long long oz_hw_tod_getnow (void);

const int oz_TIMER_RESOLUTION = 10000000;
unsigned int oz_s_boottime[2];

void oz_knl_printk (const char *format, ...);

int main ()

{
  int i;
  unsigned int calib;
  unsigned long long now;

  printf ("\nThis should print out the current date/time as fetched from the RTC\n");
  if (iopl (3) < 0) {
    fprintf (stderr, "iopl error: %s\n", strerror (errno));
    return (-1);
  }
  printf ("calling oz_hw486_tod_init\n");
  oz_hw486_tod_init ();
  iopl (0);

  printf ("\nThis should print out the time the computer was booted\n");
  printf ("calling oz_hw486_tod_sync_firstcpu\n");
  asm ("xorl %eax,%eax");
  oz_hw486_tod_sync_firstcpu ();

  printf ("\nThis should print out the current time for the next five seconds\n");
  for (i = 0; i < 5; i ++) {
    sleep (1);
    now = oz_hw_tod_getnow ();
    oz_knl_printk ("now %##t\n", now);
  }

  printf ("\nThis should cause the time to increment by 1.1 seconds for each one actual second\n");
  printf ("calling oz_hw_tod_calibrate\n");
  calib = oz_hw_tod_calibrate (0);
  printf ("old calib %u\n", calib);
  oz_hw_tod_calibrate ((calib / 11) * 10);
  printf ("new calib %u\n", oz_hw_tod_calibrate (0));
  for (i = 0; i < 5; i ++) {
    sleep (1);
    now = oz_hw_tod_getnow ();
    oz_knl_printk ("now %##t\n", now);
  }

  printf ("\nThis should cause the time to increment by 0.9 secondS for every actual one second\n");
  oz_hw_tod_calibrate ((calib / 10) * 11);
  printf ("new calib %u\n", oz_hw_tod_calibrate (0));
  for (i = 0; i < 5; i ++) {
    sleep (1);
    now = oz_hw_tod_getnow ();
    oz_knl_printk ("now %##t\n", now);
  }

  printf ("\nDone.\n");
  return (0);
}

static unsigned int printbuf (void *dummy, unsigned int *size, char **buff);

void oz_knl_printk (const char *format, ...)

{
  char buf[256];
  va_list ap;

  va_start (ap, format);
  oz_sys_vxprintf (printbuf, NULL, sizeof buf, buf, NULL, format, ap);
  va_end (ap);
}

static unsigned int printbuf (void *dummy, unsigned int *size, char **buff)

{
  fwrite (*buff, *size, 1, stdout);
  return (1);
}

int oz_hw_getrtcoffset (void)

{
  return (0);
}

int oz_hw_cpu_getcur (void)

{
  return (0);
}

char *strncpyz (char *dst, char *src, int len)

{
  strncpy (dst, src, len);
  dst[len-1] = 0;
  return (dst);
}

int oz_sys_gettimezone (void)

{
  return (-14400);
}
