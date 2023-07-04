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
/*  Date conversion routines						*/
/*  The oz_sys_datebin_encode/decode routines are in oz_knl_hw.h	*/
/*									*/
/************************************************************************/

#ifndef _OZ_SYS_DATECONV_H
#define _OZ_SYS_DATECONV_H

/* Timer definitions */

#define OZ_TIMER_RESOLUTION 10000000
#define OZ_DATELONG_FRACTION 0
#define OZ_DATELONG_SECOND 1
#define OZ_DATELONG_DAYNUMBER 2
#define OZ_DATELONG_ELEMENTS 3

typedef enum { OZ_DATEBIN_TZCONV_UTC2LCL = 0, 	/* convert UTC datebin to local timezone */
               OZ_DATEBIN_TZCONV_LCL2UTC = 1, 	/* convert local timezone datebin to UTC */
               OZ_DATEBIN_TZCONV_UTC2OFS = 2, 	/* convert UTC datebin to given offset timezone */
               OZ_DATEBIN_TZCONV_OFS2UTC = 3	/* convert given offset timezone datebin to UTC */
             } OZ_Datebin_tzconv;

#include "ozone.h"
#include "oz_knl_hw.h"

uLong oz_sys_daynumber_decode (uLong daynumber);						/* convert daynumber to yyyymmdd */
uLong oz_sys_daynumber_weekday (uLong daynumber);						/* convert daynumber to day-of-week (0=Sun, 1=Mon, etc) */
uLong oz_sys_daynumber_encode (uLong yyyymmdd);							/* convert yyyymmdd to daynumber */
int oz_sys_datebin_decstr (int delta, OZ_Datebin datebin, int size, char *buff);		/* convert datebin to string */
int oz_sys_datebin_encstr (int size, const char *buff, OZ_Datebin *datebin_r);			/* convert string to datebin */
int oz_sys_datebin_encstr2 (int size, const char *buff, OZ_Datebin *datebin_r, OZ_Datebin now);	/* convert string to datebin */
void oz_sys_datebin_decode (OZ_Datebin datebin, uLong datelongs[OZ_DATELONG_ELEMENTS]);		/* convert datebin to datelongs */
OZ_Datebin oz_sys_datebin_encode (const uLong datelongs[OZ_DATELONG_ELEMENTS]);			/* convert datelongs to datebin */
OZ_Datebin oz_sys_datebin_tzconv (OZ_Datebin datebin, OZ_Datebin_tzconv tzconv, Long offset);	/* perform timezone conversion on datebin */

#endif
