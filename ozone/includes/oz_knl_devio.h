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
/*  Device driver database routines					*/
/*									*/
/************************************************************************/

#ifndef _OZ_KNL_DEVIO_H
#define _OZ_KNL_DEVIO_H

#define OZ_DEVCLASS_NAMESIZE (32)	/* max length of device class name string (incl null) */
#define OZ_DEVDRIVER_NAMESIZE (32)	/* max length of device driver name string (incl null) */
#define OZ_DEVUNIT_NAMESIZE (64)	/* max length of device unit name string (incl null) */
#define OZ_DEVUNIT_DESCSIZE (64)	/* max length of device unit description string (incl null) */
#define OZ_DEVUNIT_ALIASIZE (OZ_DEVCLASS_NAMESIZE+12) /* max length of alias name string (incl null) */

#define OZ_DEVUNIT_ALIAS_CHAR '_'	/* character that alias names begin with */

#define OZ_IO_FR (1)	/* function code bit zero set - channel must have been assigned with other than NL mode lock */
#define OZ_IO_FW (2)	/* function code bit one set - channel must have been assigned with some write-style lock */

#define OZ_IO_FS (4)	/* function code step value */

#define OZ_IO_DN(__base,__seqn) ((uLong)((__base) + (__seqn) * OZ_IO_FS))		/* neither read nor write */
#define OZ_IO_DR(__base,__seqn) ((uLong)((__base) + (__seqn) * OZ_IO_FS + OZ_IO_FR))	/* define a read function code */
#define OZ_IO_DW(__base,__seqn) ((uLong)((__base) + (__seqn) * OZ_IO_FS + OZ_IO_FW))	/* define a write function code */

#ifdef _OZ_KNL_DEVIO_C

typedef struct OZ_Devunit OZ_Devunit;
typedef struct OZ_Devdriver OZ_Devdriver;
typedef struct OZ_Devclass OZ_Devclass;
typedef struct OZ_Ioop OZ_Ioop;
typedef struct OZ_Iochan OZ_Iochan;
typedef struct OZ_Ioselect OZ_Ioselect;

#else

typedef void OZ_Devunit;
typedef void OZ_Devdriver;
typedef void OZ_Devclass;
typedef void OZ_Ioop;
typedef void OZ_Iochan;
typedef void OZ_Ioselect;

#endif

#include "oz_knl_event.h"
#include "oz_knl_ioselcode.h"
#include "oz_knl_lock.h"
#include "oz_knl_process.h"
#include "oz_knl_procmode.h"
#include "oz_knl_section.h"
#include "oz_knl_security.h"
#include "oz_knl_thread.h"

/* Function table provided by drivers - routines are called with kernel mode ast delivery inhibited (unless otherwise stated) */
/* The only required routine is the 'start' routine - all others are optional                                                 */

typedef struct { 
        uLong dev_exsize;		/* size of devex area - zero if none wanted */
        uLong chn_exsize;		/* size of chnex area - zero if none wanted */
        uLong iop_exsize;		/* size of iopex area - zero if none wanted */
        uLong sel_exsize;		/* size of selex area - zero if none wanted */

	/* called to shutdown all driver functions - all I/O is to be aborted */
	/* this is called by the loader before it jumps to the kernel         */
	/* also called by 'reboot' and 'shutdown' commands                    */
	/* return 0 if need to be called back after calling all other drivers */
        /* return 1 if shutdown is complete and don't need to be called again */
	/* this routine is called at softint level                            */

        int (*shutdown) (OZ_Devunit *devunit, void *devex);

	/* called when a channel is about to be assigned to possibly create a 'cloned' device */
        /* example use would be to create a new pipe device from a template pipe              */
	/* ** this routine is called with the dv smplock set **                               */

	uLong (*clonecre) (OZ_Devunit *template_devunit, void *template_devex, int template_cloned, OZ_Procmode procmode, OZ_Devunit **cloned_devunit);

	/* called when last channel is deassigned from a device to possibly 'delete' the device */
        /* example use would be to delete a pipe when everyone has finished using it            */
        /* caller guarantees that no I/O's are in progress on the device                        */
	/* ** this routine is called with the dv smplock set **                                 */

	int (*clonedel) (OZ_Devunit *cloned_devunit, void *devexv, int cloned);

	/* called when a channel has just been assigned to a device */
        /* example use would be to initialize the chnex area        */
	/* it could also do some special access privilege checking  */

	uLong (*assign) (OZ_Devunit *devunit, void *devexv, OZ_Iochan *iochan, void *chnexv, OZ_Procmode procmode);

	/* called when a channel is being deassigned from a device                                    */
        /* example use would be to free off all resources in the chnex area (like open file pointers) */
        /* caller guarantees that no I/O's are in progress from this channel                          */

	int (*deassign) (OZ_Devunit *devunit, void *devexv, OZ_Iochan *iochan, void *chnexv);

	/* abort any i/o requests in progress on the channel -                                                                            */
        /* - if an i/o request is aborted, the oz_knl_iodone routine must still be called (although it could supply status of OZ_ABORTED) */
	/* - if not supplied, threads will have to wait for the i/o to complete before the channel will deassign or thread will exit      */
	/* - it is ok to omit for drivers whose functions always complete in a short finite time (like disk drivers), but otherwise it    */
        /*   should be implemented                                                                                                        */

	void (*abort) (OZ_Devunit *devunit, void *devexv, OZ_Iochan *iochan, void *chnexv, OZ_Ioop *ioop, void *iopexv, OZ_Procmode procmode);

	/* start an i/o request on a channel, call oz_knl_iodone when complete */

	uLong (*start) (OZ_Devunit *devunit, 
                        void *devexv, 
                        OZ_Iochan *iochan, 
                        void *chnexv, 
                        OZ_Procmode procmode, 
                        OZ_Ioop *ioop, 
                        void *iopexv, 
                        uLong funcode, 
                        uLong as, 
                        void *ap);

	/* call oz_knl_ioseldone(hi) when a condition becomes true */

	uLong (*select) (OZ_Devunit *devunit, void *devexv, OZ_Iochan *iochan, void *chnexv, OZ_Procmode procmode, 
	                 OZ_Ioselect *ioselect, void *selexv, uLong *ioselcode, int arm);

} OZ_Devfunc;

/* Routines */

	/* boot-time initialization and shutdown routines */

void oz_knl_devinit (void);
void oz_knl_devshut (void);

	/* driver initialization routines - called by driver initialization routine to set up device database */

OZ_Devclass *oz_knl_devclass_create (const char *classname, uLong funcodbase, uLong funcodmask, const char *drivername);
OZ_Devdriver *oz_knl_devdriver_create (OZ_Devclass *devclass, const char *drivername);
OZ_Devunit *oz_knl_devunit_create (OZ_Devdriver *devdriver, const char *unitname, const char *unitdesc, const OZ_Devfunc *functable, int cloned, OZ_Secattr *secattr);
int oz_knl_devunit_rename (OZ_Devunit *devunit, const char *unitname, const char *unitdesc);
void *oz_knl_devunit_ex (OZ_Devunit *devunit);
void oz_knl_devunit_autogen (OZ_Devunit *host_devunit, 
                             OZ_Devunit *(*auto_entry) (void *auto_param, 
                                                        OZ_Devunit *host_devunit, 
                                                        const char *devname, 
                                                        const char *suffix), 
                             void *auto_param);

	/* i/o processing routines - called by outsiders that want to perform i/o - they can be called at softint or less     */
	/* they can also be called by drivers themselves - for instance an file system driver would call a disk driver, and a */
        /* disk driver would call an scsi driver, etc.                                                                        */

void oz_knl_devunit_setsecattr (OZ_Devunit *devunit, OZ_Secattr *newsecattr);
OZ_Secattr *oz_knl_devunit_getsecattr (OZ_Devunit *devunit);
const char *oz_knl_devunit_devname (OZ_Devunit *devunit);
const char *oz_knl_devunit_devdesc (OZ_Devunit *devunit);
int oz_knl_devunit_aliasname (OZ_Devunit *devunit, uLong size, char *buff);
const char *oz_knl_devunit_classname (OZ_Devunit *devunit);
OZ_Devunit *oz_knl_devunit_lookup (const char *devname);
Long oz_knl_devunit_increfc (OZ_Devunit *devunit, Long inc);
uLong oz_knl_devunit_count (void);
OZ_Devunit *oz_knl_devunit_getnext (OZ_Devunit *lastdevunit);
int oz_knl_devunit_unassigned (OZ_Devunit *devunit);
const OZ_Devfunc *oz_knl_devunit_functable (OZ_Devunit *devunit);

uLong oz_knl_devunit_alloc (OZ_Devunit *devunit, void *alloc_obj);
uLong oz_knl_devunit_realloc (OZ_Devunit *devunit, void *alloc_obj);
uLong oz_knl_devunit_dealloc (OZ_Devunit *devunit, void *alloc_obj);
void *oz_knl_devunit_getalloc (OZ_Devunit *devunit);
void oz_knl_devunit_dallocall (OZ_Devunit **devalloc);

uLong oz_knl_iochan_crbynm (const char *devname, OZ_Lockmode lockmode, OZ_Procmode procmode, OZ_Secattr *secattr, OZ_Iochan **iochan_r);
uLong oz_knl_iochan_create (OZ_Devunit *devunit, OZ_Lockmode lockmode, OZ_Procmode procmode, OZ_Secattr *secattr, OZ_Iochan **iochan_r);
uLong oz_knl_iochan_copy (OZ_Iochan *iochan, OZ_Lockmode lockmode, OZ_Procmode procmode, OZ_Iochan **iochan_r);
OZ_Devunit *oz_knl_iochan_getdevunit (OZ_Iochan *iochan);
uLong oz_knl_iochan_readcount (OZ_Iochan *iochan);
uLong oz_knl_iochan_writecount (OZ_Iochan *iochan);
void *oz_knl_iochan_ex (OZ_Iochan *iochan);
OZ_Lockmode oz_knl_iochan_getlockmode (OZ_Iochan *iochan);
OZ_Threadid oz_knl_iochan_getlastiotid (OZ_Iochan *iochan);
OZ_Secattr *oz_knl_iochan_getsecattr (OZ_Iochan *iochan);
void oz_knl_iochan_setsecattr (OZ_Iochan *iochan, OZ_Secattr *secattr);
OZ_Iochan *oz_knl_iochan_getnext (OZ_Iochan *lastiochan, OZ_Devunit *devunit);
uLong oz_knl_iochan_count (OZ_Devunit *devunit);

void oz_knl_iorundown (OZ_Thread *thread, OZ_Procmode procmode);
void oz_knl_ioabort (OZ_Iochan *iochan, OZ_Procmode procmode);
void oz_knl_ioabort2 (OZ_Iochan *iochan, OZ_Procmode procmode, OZ_Ioop *ioop);
uLong oz_knl_io (OZ_Iochan *iochan, uLong funcode, uLong as, void *ap);
uLong oz_knl_iostart (OZ_Iochan *iochan, 
                      OZ_Procmode procmode, 
                      void (*iopostent) (void *iopostpar, uLong status),
                      void *iopostpar,
                      volatile uLong *status_r,
                      OZ_Event *event,
                      OZ_Astentry astentry, 
                      void *astparam,
                      uLong funcode, 
                      uLong as, 
                      void *ap);
uLong oz_knl_iostart2 (int sysio, 
                       OZ_Iochan *iochan, 
                       OZ_Procmode procmode, 
                       void (*iopostent) (void *iopostpar, uLong status),
                       void *iopostpar,
                       volatile uLong *status_r,
                       OZ_Event *event,
                       OZ_Astentry astentry, 
                       void *astparam,
                       uLong funcode, 
                       uLong as, 
                       void *ap);
uLong oz_knl_iostart3 (int sysio, 
                       OZ_Ioop **ioop_r, 
                       OZ_Iochan *iochan, 
                       OZ_Procmode procmode, 
                       void (*iopostent) (void *iopostpar, uLong status),
                       void *iopostpar,
                       volatile uLong *status_r,
                       OZ_Event *event,
                       OZ_Astentry astentry, 
                       void *astparam,
                       uLong funcode, 
                       uLong as, 
                       void *ap);
OZ_Thread *oz_knl_ioop_getthread (OZ_Ioop *ioop);
OZ_Process *oz_knl_ioop_getprocess (OZ_Ioop *ioop);
#define oz_knl_ioop_lockr(__ioop,__size,__buff,__phypages_r,__npages_r,__byteoffs_r) oz_knl_ioop_lock(__ioop,__size,__buff,0,__phypages_r,__npages_r,__byteoffs_r)
#define oz_knl_ioop_lockw(__ioop,__size,__buff,__phypages_r,__npages_r,__byteoffs_r) oz_knl_ioop_lock(__ioop,__size,__buff,1,__phypages_r,__npages_r,__byteoffs_r)
uLong oz_knl_ioop_lockz (OZ_Ioop *ioop, uLong size, const void *buff, uLong *rlen, const OZ_Mempage **phypages_r, OZ_Mempage *npages_r, uLong *byteoffs_r);
uLong oz_knl_ioop_lock (OZ_Ioop *ioop, uLong size, const void *buff, int writing, const OZ_Mempage **phypages_r, OZ_Mempage *npages_r, uLong *byteoffs_r);
uLong oz_knl_ioselect (int sysio, 
                       OZ_Ioselect **ioselect_r, 
                       uLong numchans, 
                       OZ_Iochan **iochans, 
                       uLong *ioselcodes, 
                       uLong *senses, 
                       OZ_Procmode procmode, 
                       void (*selpostent) (void *selpostpar, 
                                           uLong numchans, 
                                           OZ_Iochan **iochans, 
                                           uLong *ioselcodes, 
                                           uLong *senses, 
                                           OZ_Procmode procmode), 
                       void *selpostpar);
Long oz_knl_iochan_increfc (OZ_Iochan *iochan, Long inc);

	/* routines that a driver calls when it completes an operation - they are to be called with softint delivery inhibited */
	/* (oz_knl_iodonehi and oz_knl_ioseldonehi can be called above softint level) */

void oz_knl_iodonehi (OZ_Ioop *ioop, uLong status, void *kthread, void (*finentry) (void *finparam, int finok, uLong *status_r), void *finparam);
void oz_knl_iodone (OZ_Ioop *ioop, uLong status, void *kthread, void (*finentry) (void *finparam, int finok, uLong *status_r), void *finparam);
Long oz_knl_ioop_increfc (OZ_Ioop *ioop, Long inc);
void oz_knl_ioseldonehi (OZ_Ioselect *ioselect, uLong *ioselcode, uLong status);
void oz_knl_ioseldone (OZ_Ioselect *ioselect, uLong *ioselcode, uLong status);

	/* misc routines */

int oz_knl_ioabortok (OZ_Ioop *ioop, OZ_Iochan *iochan, OZ_Procmode procmode, OZ_Ioop *aioop);

void oz_knl_devdump (int verbose);

	/* this routine is not defined by oz_knl_devio.c but is called by it to init device drivers */

void oz_dev_inits (void);

#endif
