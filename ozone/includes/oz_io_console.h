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
/*  I/O function codes for device class "console"			*/
/*									*/
/************************************************************************/

#ifndef _OZ_IO_CONSOLE_H
#define _OZ_IO_CONSOLE_H

#include "oz_knl_devio.h"

#define OZ_IO_CONSOLE_CLASSNAME "console"
#define OZ_IO_CONSOLE_BASE (0x00000300)
#define OZ_IO_CONSOLE_MASK (0xFFFFFF00)

typedef struct { uLong columns;		/* number of columns per line */
                 uLong rows;		/* number of rows per screen or page */
                 int linewrap;		/* -1: cursor stops at right margin */
					/*  1: cursor wraps to next line */
               } OZ_Console_modebuff;

typedef struct { uLong size;		/* size of buffer */
                 uWord *buff;		/* buffer to return screen data in */
                 uLong ncols;		/* number of columns in display */
                 uLong nrowstot;	/* total number of possible rows */
                 uLong nrowscur;	/* current number of rows */
                 uLong rowtop;		/* top row (0 based, inclusive) currently on screen */
                 uLong rowbot;		/* bottom row (0 based, inclusive) currently on screen */
                 uLong lastrowseq;	/* sequence number of last row */
               } OZ_Console_screenbuff;

/* Device that processes OZ_IO_COMPORT_SETUP function */

#define OZ_IO_CONSOLE_SETUPDEV "conclass"

/* Write a line to console */

#define OZ_IO_CONSOLE_WRITE OZ_IO_DW(OZ_IO_CONSOLE_BASE,1)

typedef struct { uLong size;
                 const void *buff;
                 uLong trmsize;
                 const void *trmbuff;
               } OZ_IO_console_write;

/* Read a line from console */

#define OZ_IO_CONSOLE_READ OZ_IO_DR(OZ_IO_CONSOLE_BASE,2)

typedef struct { uLong size;		/* read buffer size */
                 void *buff;		/* read buffer address */
                 uLong *rlen;		/* where to return length read */
                 uLong pmtsize;		/* prompt buffer size */
                 const void *pmtbuff;	/* prompt buffer address */
                 uLong timeout;		/* 0 = no timeout, else milliseconds */
                 int noecho;		/* 0 = echo, 1 = don't echo */
               } OZ_IO_console_read;

/* Wait for a control character */

#define OZ_IO_CONSOLE_CTRLCHAR OZ_IO_DR(OZ_IO_CONSOLE_BASE,3)

typedef struct { uLong mask[8];		/* 256 bit character mask */
                 int wiperah;		/* 0 : leave read-ahead alone; 1 : wipe read-ahead */
                 int terminal;		/* 0 : process character normally; 1 : don't process normally */
                 int abortread;		/* 0 : leave reads intact; 1 : abort pending reads */
                 uByte *ctrlchar;	/* where to return the character */
               } OZ_IO_console_ctrlchar;

/* Write a buffer to comport */

#define OZ_IO_CONSOLE_PUTDAT OZ_IO_DW(OZ_IO_CONSOLE_BASE,4)

typedef struct { uLong size;
                 const void *buff;
               } OZ_IO_console_putdat;

/* Read whatever there is in the comport */

#define OZ_IO_CONSOLE_GETDAT OZ_IO_DR(OZ_IO_CONSOLE_BASE,5)

typedef struct { uLong size;
                 void *buff;
                 uLong *rlen;
               } OZ_IO_console_getdat;

/* Sense modes - retrieves existing modes into the 'buff' */

#define OZ_IO_CONSOLE_GETMODE OZ_IO_DR(OZ_IO_CONSOLE_BASE,6)

typedef struct { uLong size;
                 OZ_Console_modebuff *buff;
               } OZ_IO_console_getmode;

/* Set modes - modifies modes that have non-zero values from 'buff' then  */
/* retrieves all existing modes (after modification) back into the 'buff' */

#define OZ_IO_CONSOLE_SETMODE OZ_IO_DW(OZ_IO_CONSOLE_BASE,7)

typedef struct { uLong size;
                 OZ_Console_modebuff *buff;
               } OZ_IO_console_setmode;

/* Get screen contents */

#define OZ_IO_CONSOLE_GETSCREEN OZ_IO_DN(OZ_IO_CONSOLE_BASE,8)

typedef struct { uLong size;
                 OZ_Console_screenbuff *buff;
               } OZ_IO_console_getscreen;

#endif
