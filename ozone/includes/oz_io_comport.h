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
/*  I/O function codes for device class "comport"			*/
/*									*/
/************************************************************************/

#ifndef _OZ_IO_COMPORT_H
#define _OZ_IO_COMPORT_H

#include "oz_io_conpseudo.h"
#include "oz_io_console.h"
#include "oz_knl_devio.h"

#define OZ_IO_COMPORT_CLASSNAME "comport"
#define OZ_IO_COMPORT_BASE (0x00000100)
#define OZ_IO_COMPORT_MASK (0xFFFFFF00)

/* Send port parameters to class driver and get class paramters back                                                         */
/* (Although this is in the 'comport' I/O class, it is implemented by the 'console class driver' and the 'ppp class driver') */

#define OZ_IO_COMPORT_SETUP OZ_IO_DW(OZ_IO_COMPORT_BASE,1)

typedef struct { /* Port driver fills these in and passes them to class driver */

                 OZ_Devunit *port_devunit;					/* port device unit pointer */
                 void *port_param;						/* parameter that class driver is to pass back when it calls port routines */
                 void (*port_read_start) (void *port_param, int start);		/* class driver calls this when it starts and finishes a read request */
                 uLong (*port_disp_start) (void *port_param, void *write_param, uLong size, char *buff); /* class driver calls this to start displaying a string */
                 void (*port_disp_suspend) (void *port_param, int suspend);	/* class driver calls this to suspend/resume displaying the string */
                 void (*port_kbd_rah_full) (void *port_param, int full);	/* class driver calls this when read-ahead buffer is full/empty */
                 void (*port_terminate) (void *port_param);			/* class driver calls this when applications device is closed */
										/* class driver shall not make any more calls to port driver after this */
										/* (except to unlock the database) */
                 uLong (*port_getsetmode) (void *port_param, void *getset_param, uLong size, OZ_Console_modebuff *buff); /* get/set modes */
                 void *port_lkprm;						/* parameter for lockdb/unlkdb routines (eg, pointer to smplock struct) */
                 uLong (*port_lockdb) (void *port_lkprm);			/* routine to lock the database (eg, oz_hw_smplock_wait) */
                 void (*port_unlkdb) (void *port_lkprm, uLong savipl);		/* routine to unlock the database (eg, oz_hw_smplock_clr) */

                 /* Class driver fills these in and passes them back to port driver                                  */
                 /* Port driver shall not call these routines once it has deassigned the channel to the class driver */

                 const OZ_Devfunc *class_functab;				/* class application level function table pointer */
                 void *class_devex;						/* class application level device extension */
                 void *class_param;						/* parameter that port driver is to pass back when it calls class routines */
                 void (*class_kbd_char) (void *class_param, char ch);		/* port driver calls this when it has a keyboard character to process */
                 void (*class_displayed) (void *class_param, void *write_param, uLong status); /* port driver calls this when it has finished displaying a string */
                 void (*class_gotsetmode) (void *class_param, void *getset_param, uLong status); /* call this when done doing getsetmode */
                 uLong (*class_setmode) (void *class_param, OZ_IO_conpseudo_setmode *conpseudo_setmode); /* call this to tell class driver what terminal is doing */
               } OZ_IO_comport_setup;

#endif
