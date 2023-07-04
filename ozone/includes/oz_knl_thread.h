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

#ifndef _OZ_KNL_THREAD_H
#define _OZ_KNL_THREAD_H

#define OZ_THREAD_NAMESIZE (64)

/* Thread states */

typedef enum { OZ_THREAD_STATE_RUN, 		/* currently running on a CPU */
						/* thread -> cpuidx = cpu currently running on */
               OZ_THREAD_STATE_COM, 		/* waiting for a CPU, thread is in oz_threadq_com queue */
               OZ_THREAD_STATE_WEV, 		/* waiting for an event, thread is in oz_threadq_wev queue */
						/* thread -> nevents = number of events in events array */
						/* thread -> events  = array of event pointers */
               OZ_THREAD_STATE_INI, 		/* just initialized by oz_knl_thread_create */
               OZ_THREAD_STATE_ZOM,		/* thread has exited, but its refcount is not yet zero */
               OZ_THREAD_STATE_MAX		/* max number of states */
             } OZ_Thread_state;

/* Thread ID number */

typedef uLong OZ_Threadid;

/* Exithandler and Thread blocks */

#ifdef _OZ_KNL_THREAD_C
typedef struct OZ_Exhand OZ_Exhand;
typedef struct OZ_Thread OZ_Thread;
#else
typedef void OZ_Exhand;
typedef void OZ_Thread;
#endif

typedef uLong (*OZ_Thread_entry) (void *thparam);
typedef void (*OZ_Exhand_entry) (void *param, uLong status);

#include "oz_knl_process.h"

/* Includes needed for routine prototypes */

#include "oz_knl_ast.h"
#include "oz_knl_devio.h"
#include "oz_knl_event.h"
#include "oz_knl_hw.h"
#include "oz_knl_lowipl.h"
#include "oz_knl_procmode.h"
#include "oz_knl_security.h"

/* Global entrypoints */

void oz_knl_thread_init (void);
void oz_knl_thread_cpuinit (void);
uLong oz_knl_thread_create (OZ_Process *process, 
                            uLong priority, 
                            OZ_Seckeys *seckeys, 
                            OZ_Event *initevent, 
                            OZ_Event *exitevent, 
                            OZ_Mempage stacksize, 
                            OZ_Thread_entry thentry, 
                            void *thparam, 
                            OZ_Astmode knlastmode, 
                            int name_l, 
                            const char *name, 
                            OZ_Secattr *secattr, 
                            OZ_Thread **thread_r);
void oz_knl_thread_start (OZ_Thread *thread);
uLong oz_knl_exhand_create (OZ_Exhand_entry exhentry, void *exhparam, OZ_Procmode procmode, OZ_Thread *thread, OZ_Exhand **exhand_r);
int oz_knl_exhand_dequeue (OZ_Procmode procmode, OZ_Exhand_entry *exhentry_r, void **exhparam_r);
void oz_knl_exhand_delete (OZ_Exhand *exhand);
void oz_knl_thread_orphan (OZ_Thread *thread);
int oz_knl_thread_suspend (OZ_Thread *thread);
int oz_knl_thread_resume (OZ_Thread *thread);
void oz_knl_thread_abort (OZ_Thread *thread, uLong abortsts);
void oz_knl_thread_exit (uLong status);
OZ_Event *oz_knl_thread_ioevent (void);
uLong oz_knl_thread_incios (OZ_Thread *thread, uLong inc);
uLong oz_knl_thread_incpfs (OZ_Thread *thread, uLong inc);
OZ_Threadid oz_knl_thread_getid (OZ_Thread *thread);
const char *oz_knl_thread_getname (OZ_Thread *thread);
OZ_Thread *oz_knl_thread_getparent (OZ_Thread *thread);
uLong oz_knl_thread_getexitsts (OZ_Thread *thread, uLong *exitsts_r);
OZ_Event *oz_knl_thread_getexitevent (OZ_Thread *thread);
OZ_Event *oz_knl_thread_getwevent (OZ_Thread *thread, uLong index);
Long oz_knl_thread_increfc (OZ_Thread *thread, Long inc);
void oz_knl_thread_queueast (OZ_Ast *ast, uLong aststs);
void oz_knl_thread_quantimex (Long cpuidx);
void oz_knl_thread_handleint (void);
void oz_knl_thread_wakewev (OZ_Thread *thread, uLong wakests);
uLong oz_knl_thread_wait (void);
void oz_knl_thread_setseckeys (OZ_Thread *thread, OZ_Seckeys *seckeys);
void oz_knl_thread_setdefcresecattr (OZ_Thread *thread, OZ_Secattr *secattr);
void oz_knl_thread_setsecattr (OZ_Thread *thread, OZ_Secattr *secattr);
OZ_Seckeys *oz_knl_thread_getseckeys (OZ_Thread *thread);
OZ_Secattr *oz_knl_thread_getsecattr (OZ_Thread *thread);
OZ_Secattr *oz_knl_thread_getdefcresecattr (OZ_Thread *thread);
OZ_Process *oz_knl_thread_getprocess (OZ_Thread *thread);
OZ_Process *oz_knl_thread_getprocesscur (void);
void *oz_knl_thread_gethwctx (OZ_Thread *thread);
OZ_Thread *oz_knl_thread_getcur (void);
OZ_Astmode oz_knl_thread_setast (OZ_Procmode procmode, OZ_Astmode astmode);
uLong oz_knl_thread_getbasepri (OZ_Thread *thread);
uLong oz_knl_thread_setbasepri (OZ_Thread *thread, uLong newprio);
uLong oz_knl_thread_getcurprio (OZ_Thread *thread);
uLong oz_knl_thread_inccurprio (OZ_Thread *thread, Long incprio);
uLong oz_knl_thread_setcurprio (OZ_Thread *thread, uLong newprio);
void oz_knl_thread_checkknlastq (Long cpuidx, OZ_Mchargs *mchargs);
int oz_knl_thread_chkpendast (OZ_Procmode procmode);
uLong oz_knl_thread_deqast (OZ_Procmode procmode, int express, OZ_Astentry *astentry, void **astparam, uLong *aststs);
OZ_Thread *oz_knl_thread_setcurstate (OZ_Thread_state state, uLong nevents, OZ_Eventlist *eventlist);
OZ_Ioop **oz_knl_thread_getioopqp (OZ_Thread *thread);
OZ_Thread_state oz_knl_thread_getstate (OZ_Thread *thread);
OZ_Datebin oz_knl_thread_gettis (OZ_Thread *thread, OZ_Thread_state state);
int oz_knl_thread_abortpend (OZ_Thread *thread);
OZ_Thread *oz_knl_thread_getnext (OZ_Thread *lastthread, OZ_Process *process);
uLong oz_knl_thread_count (OZ_Process *process);
OZ_Thread *oz_knl_thread_getbyid (OZ_Threadid threadid);
OZ_Devunit **oz_knl_thread_getdevalloc (OZ_Thread *thread);
void oz_knl_thread_dump_all (void);
void oz_knl_thread_dump_process (OZ_Process *process);
void oz_knl_thread_dump (OZ_Thread *thread);
void oz_knl_thread_tracedump (OZ_Thread *thread);
void oz_knl_thread_validate (void);

#endif
