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
/*  Event flags are simple flags that a thread can wait for		*/
/*									*/
/*    Properties:							*/
/*									*/
/*	     set <=0 : flag is clear					*/
/*	          >0 : flag is set					*/
/*	refcount = number of things pointing to this event block	*/
/*	    astq = list of asts that will be executed when event flag is set
/*									*/
/*    Operations:							*/
/*									*/
/*	  create : create a new event flag, sets ref count=1		*/
/*	     set : change event flag state (0=clear; 1=set)		*/
/*	 increfc : modify reference count, free event flag if zero	*/
/*	 queuecb : queue callback to be delivered if/when flag is set	*/
/*	    wait : current thread waits for any one of a list of 	*/
/*	           events to be set					*/
/*									*/
/************************************************************************/

#ifndef _OZ_KNL_EVENT_H
#define _OZ_KNL_EVENT_H

#define OZ_EVENT_NAMESIZE 32

#ifdef _OZ_KNL_EVENT_C
typedef struct OZ_Event OZ_Event;
#else
typedef void OZ_Event;
#endif
typedef struct OZ_Eventlist OZ_Eventlist;

#include "oz_knl_ast.h"
#include "oz_knl_security.h"
#include "oz_knl_thread.h"

struct OZ_Eventlist { OZ_Event *event;		/* put pointer to event flag you want to wait for here */
                      OZ_Eventlist *neventlist;	/* leave these alone for oz_knl_event_waitlist to use */
                      OZ_Eventlist **peventlist;
                      OZ_Thread *thread;
                    };

void oz_knl_event_init (void);
uLong oz_knl_event_create (int name_l, const char *name, OZ_Secattr *secattr, OZ_Event **event_r);
void oz_knl_event_rename (OZ_Event *event, int name_l, const char *name);
uLong oz_knl_event_setimint (OZ_Event *event, OZ_Datebin interval, OZ_Datebin basetime);
OZ_Datebin oz_knl_event_getimint (OZ_Event *event);
OZ_Datebin oz_knl_event_getimnxt (OZ_Event *event);
Long oz_knl_event_inc (OZ_Event *event, Long value);
Long oz_knl_event_set (OZ_Event *event, Long value);
Long oz_knl_event_increfc (OZ_Event *event, Long inc);
uLong oz_knl_event_queuecb (OZ_Event *event, void (*entry) (void *param, OZ_Event *event), void *param);
uLong oz_knl_event_waitone (OZ_Event *event);
uLong oz_knl_event_waitlist (uLong nevents, OZ_Eventlist *eventlist, OZ_Procmode procmode, int si);
OZ_Secattr *oz_knl_event_getsecattr (OZ_Event *event);
const char *oz_knl_event_getname (OZ_Event *event);
void oz_knl_event_dump (void);

#endif
