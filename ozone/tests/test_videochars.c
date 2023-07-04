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
/*  Display the video adapter's character set				*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_knl_status.h"
#include "oz_sys_callknl.h"

static const char hexdigits[] = "0123456789ABCDEF";

static uLong inknl (OZ_Procmode cprocmode, void *dummy);

int main ()

{
  uLong sts;

  sts = oz_sys_callknl (inknl, NULL);
  oz_sys_io_fs_printerror ("status %u\n", sts);
  return (sts);
}

#define VIDEOBASE ((uWord *)0xb8000)    /* base address of video memory */
#define VIDEOLINESIZE 80                /* number of characters per line */
#define VIDEOPAGESIZE 80*25             /* number of characters per page */
#define VIDEOLINESAVE 500               /* number of lines to save off top of screen */

#define STARTLINE 5

static uLong inknl (OZ_Procmode cprocmode, void *dummy)

{
  int i, j;

  memset (VIDEOBASE, 0, VIDEOPAGESIZE * 2);

  for (j = 0; j < 16; j ++) {
    VIDEOBASE[(STARTLINE-1)*VIDEOLINESIZE+8+j] = 0x0700 + hexdigits[j];
  }

  for (i = 0; i < 16; i ++) {
    VIDEOBASE[(STARTLINE+i)*VIDEOLINESIZE+7] = 0x0700 + hexdigits[i];
    for (j = 0; j < 16; j ++) {
      VIDEOBASE[(STARTLINE+i)*VIDEOLINESIZE+8+j] = 0x7000 + (i*16) + j;
    }
  }

  return (OZ_SUCCESS);
}
