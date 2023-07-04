//+++2003-03-01
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
//---2003-03-01

#ifndef _OZ_KNL_SDATA_H
#define _OZ_KNL_SDATA_H

#include "oz_knl_cache.h"
#include "oz_knl_devio.h"
#include "oz_knl_hw.h"
#include "oz_knl_image.h"
#include "oz_knl_malloc.h"
#include "oz_knl_security.h"
#include "oz_knl_userjob.h"
#include "oz_ldr_loader.h"
#include "oz_sys_pdata.h"

typedef enum { OZ_PHYMEM_PAGESTATE_FREE, 	/* - page is available for use */
               OZ_PHYMEM_PAGESTATE_ALLOCSECT, 	/* - allocated to a section */
               OZ_PHYMEM_PAGESTATE_ALLOCACHE, 	/* - allocated to cache */
               OZ_PHYMEM_PAGESTATE_ALLOCPERM	/* - permanently allocated (kernel image, pool) */
             } OZ_Phymem_pagestate;

typedef struct { OZ_Phymem_pagestate state;		/* physical page's state */

                 union { struct { OZ_Mempage nextpage;		/* - next physical page in list */
                                } f;			/* pagestate FREE */

                         struct { OZ_Mempage pfpage;		/* page within page file being transferred */
                                  Long ptrefcount;		/* number of pagetable entries pointing to the page */
								/* - global sections: when it goes zero, page out the page */
								/*                    update with the section's gp lock set */
								/* - private sections: = 1 : normal operation */
								/*                     > 1 : copy-on-write count */
								/*                     update atomically */
                                } s;			/* pagestate ALLOCSECT */

                         OZ_Cachepage c;		/* pagestate ALLOCACHE */
                       } u;
               } OZ_Phymem_page;

globalref OZ_Phymem_page *oz_s_phymem_pages;
globalref OZ_Event *oz_s_freephypagevent;
globalref OZ_Memsize oz_s_npptotal;
globalref OZ_Memsize oz_s_nppinuse;
globalref OZ_Memsize oz_s_npppeak;
globalref OZ_Memsize oz_s_pgptotal;
globalref OZ_Memsize oz_s_pgpinuse;
globalref OZ_Memsize oz_s_pgppeak;
globalref OZ_Process *oz_s_systemproc;
globalref OZ_User *oz_s_systemuser;
globalref OZ_Job *oz_s_systemjob;
globalref OZ_Logname *oz_s_systemdirectory;
globalref OZ_Logname *oz_s_systemtable;

globalref int oz_s_inloader;
globalref Long oz_s_cpucount;
globalref volatile uLong oz_s_cpusavail;	// volatile for oz_knl_shutdown ()
globalref OZ_Mempage oz_s_phymem_totalpages;
globalref OZ_Mempage oz_s_phymem_freepages;
globalref OZ_Mempage oz_s_phymem_l1pages;
globalref OZ_Mempage oz_s_phymem_l2pages;
globalref void *oz_s_sysmem_baseva;
globalref uLong oz_s_sysmem_pagtblsz;
globalref uLong oz_s_sysmem_pagtblfr;
globalref OZ_Loadparams oz_s_loadparams;
globalref OZ_Datebin oz_s_boottime;
globalref OZ_Iochan *oz_s_coniochan;
globalref int oz_s_shutdown;
globalref uLong oz_s_quickies;
globalref OZ_Image *oz_s_kernelimage;

globalref OZ_Smplock oz_s_smplock_sh;
globalref OZ_Smplock oz_s_smplock_dv;
globalref OZ_Smplock oz_s_smplock_pm;
globalref OZ_Smplock oz_s_smplock_ts;
globalref OZ_Smplock oz_s_smplock_ev;
globalref OZ_Smplock oz_s_smplock_se;
globalref OZ_Smplock oz_s_smplock_id;
globalref OZ_Smplock oz_s_smplock_np;
globalref OZ_Smplock oz_s_smplock_qu;
globalref OZ_Smplock oz_s_smplock_hi;

globalref uLong oz_s_syscallmax;
globalref void *oz_s_syscalltbl[]; // gcc bug gives error in oz_sys_syscall.c: uLong (*oz_s_syscalltbl[]) ();

globalref const Charp oz_s_logname_directorynames[4];
globalref const Charp oz_s_logname_defaulttables;
globalref const Charp oz_s_logname_deftblnames[5];

globalref OZ_Secattr *oz_s_secattr_callknl;
globalref OZ_Secattr *oz_s_secattr_sysdev;
globalref OZ_Secattr *oz_s_secattr_syslogname;
globalref OZ_Secattr *oz_s_secattr_tempdev;

globalref OZ_Pdata oz_s_systempdata;	// this is defined in the hw-dependent code
					// it is double mapped such that it has a global system address
					// and is also mapped at the same per-process address as 
					// oz_sys_pdata_array[OZ_PROCMODE_KNL] whenever the system 
					// process (oz_s_systemproc) is current so oz_sys_pdata_pointer
					// doesn't have to test to see if oz_s_systemproc is current

void oz_knl_sdata_init1 (void);
void oz_knl_sdata_init2 (void);

#endif
