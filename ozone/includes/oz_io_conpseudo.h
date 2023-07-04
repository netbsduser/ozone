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
/*  I/O function codes for device class "conpseudo"			*/
/*									*/
/************************************************************************/

#ifndef _OZ_IO_CONPSEUDO_H
#define _OZ_IO_CONPSEUDO_H

#include "oz_io_console.h"
#include "oz_knl_devio.h"

#define OZ_IO_CONPSEUDO_CLASSNAME "conpseudo"
#define OZ_IO_CONPSEUDO_BASE (0x00000200)
#define OZ_IO_CONPSEUDO_MASK (0xFFFFFF00)

typedef enum { OZ_CONPSEUDO_TERMINATE, 
               OZ_CONPSEUDO_SCR_SUSPEND, 
               OZ_CONPSEUDO_SCR_RESUME, 
               OZ_CONPSEUDO_SCR_ABORT, 
               OZ_CONPSEUDO_KBD_SUSPEND, 
               OZ_CONPSEUDO_KBD_RESUME, 
               OZ_CONPSEUDO_GETSETMODE, 
               OZ_CONPSEUDO_KBD_READSTARTED, 
               OZ_CONPSEUDO_KBD_READFINISHED
             } OZ_Conpseudo_event;

#define OZ_IO_CONPSEUDO_DEV "conpseudo"		/* our template device */

/* Perform device set-up */

#define OZ_IO_CONPSEUDO_SETUP OZ_IO_DN(OZ_IO_CONPSEUDO_BASE,1)

typedef struct { const char *portname;		/* what we want the user accessible device to be called */
                 const char *classname;		/* what class driver's access device is called */
               } OZ_IO_conpseudo_setup;

/* Get some screen data */

#define OZ_IO_CONPSEUDO_GETSCRDATA OZ_IO_DN(OZ_IO_CONPSEUDO_BASE,2)

typedef struct { uLong size;
                 char *buff;
                 uLong *rlen;
               } OZ_IO_conpseudo_getscrdata;

/* Put some keyboard data */

#define OZ_IO_CONPSEUDO_PUTKBDDATA OZ_IO_DN(OZ_IO_CONPSEUDO_BASE,3)

typedef struct { uLong size;
                 char *buff;
               } OZ_IO_conpseudo_putkbddata;

/* Get console event */

#define OZ_IO_CONPSEUDO_GETEVENT OZ_IO_DN(OZ_IO_CONPSEUDO_BASE,4)

typedef struct { OZ_Conpseudo_event *event;
               } OZ_IO_conpseudo_getevent;

/* Fetch get/set mode request */

#define OZ_IO_CONPSEUDO_FETCHGSMODEREQ OZ_IO_DN(OZ_IO_CONPSEUDO_BASE,5)

typedef struct { uLong size;
                 OZ_Console_modebuff *buff;
                 void **reqid_r;
               } OZ_IO_conpseudo_fetchgsmodereq;

/* Post get/set mode request */

#define OZ_IO_CONPSEUDO_POSTGSMODEREQ OZ_IO_DN(OZ_IO_CONPSEUDO_BASE,6)

typedef struct { uLong size;
                 OZ_Console_modebuff *buff;
                 uLong status;
                 void *reqid;
               } OZ_IO_conpseudo_postgsmodereq;

/* Tell class driver to mode */

#define OZ_IO_CONPSEUDO_SETMODE OZ_IO_DN(OZ_IO_CONPSEUDO_BASE,7)

typedef struct { uLong size;
                 OZ_Console_modebuff *buff;
               } OZ_IO_conpseudo_setmode;

#endif
