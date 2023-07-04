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

#ifndef _OZ_KNL_PROCESS_H
#define _OZ_KNL_PROCESS_H

#define OZ_PROCESS_NAMESIZE (64)

typedef enum { OZ_PAGETABLE_EXPUP, 			/* pagetable addresses expand upward */
               OZ_PAGETABLE_EXPDN			/* pagetable addresses expand downward */
             } OZ_Pagetable_expand;

#define OZ_MAPSECTION_EXACT  (0x01)	/* map section at exact address given */
#define OZ_MAPSECTION_SYSTEM (0x02)	/* this is a system section being mapped */
#define OZ_MAPSECTION_ATBEG  (0x04)	/* force mapping section at beginning of pagetable */
#define OZ_MAPSECTION_ATEND  (0x08)	/* force mapping section at end of pagetable */
#define OZ_MAPSECTION_NOCOPY (0x10)	/* do not copy section mapping on a 'process copy' operation */

typedef uLong OZ_Processid;

#ifdef _OZ_KNL_PROCESS_C
typedef struct OZ_Process OZ_Process;
typedef struct OZ_Secmap  OZ_Secmap;
#else
typedef void OZ_Process;
typedef void OZ_Secmap;
#endif

#include "oz_knl_devio.h"
#include "oz_knl_hw.h"
#include "oz_knl_image.h"
#include "oz_knl_logname.h"
#include "oz_knl_process.h"
#include "oz_knl_section.h"
#include "oz_knl_security.h"
#include "oz_knl_thread.h"
#include "oz_knl_userjob.h"

typedef struct { OZ_Section *section;		// section to map
                 OZ_Mempage npagem;		// number of pages in section to map, 0 for the whole thing
                 OZ_Mempage svpage;		// starting virtual page to map it at
                 OZ_Procmode ownermode;		// owner mode of the pages (ie, who can unmap them or modify their protection)
                 OZ_Hw_pageprot pageprot;	// protection to apply to the pages when mapped
               } OZ_Mapsecparam;

void oz_knl_process_init (void);
uLong oz_knl_process_create (OZ_Job *job, int sysproc, int copyproc, int name_l, const char *name, OZ_Secattr *secattr, OZ_Process **process_r);
int oz_knl_process_cleanup (OZ_Process *process);
Long oz_knl_process_increfc (OZ_Process *process, Long inc);
void oz_knl_process_setcur (OZ_Process *process);
OZ_Process *oz_knl_process_getcur (void);
OZ_Thread **oz_knl_process_getthreadqp (OZ_Process *process, int ifnormal);
OZ_Job *oz_knl_process_getjob (OZ_Process *process);
const char *oz_knl_process_getname (OZ_Process *process);
OZ_Process *oz_knl_process_getnext (OZ_Process *lastprocess, OZ_Job *job);
uLong oz_knl_process_count (OZ_Job *job);
OZ_Processid oz_knl_process_getid (OZ_Process *process);
OZ_Process *oz_knl_process_frompid (OZ_Processid pid);
OZ_Logname *oz_knl_process_getlognamdir (OZ_Process *process);
OZ_Logname *oz_knl_process_getlognamtbl (OZ_Process *process);
OZ_Secattr *oz_knl_process_getsecattr (OZ_Process *process);
OZ_Imagelist *oz_knl_process_getimagelist (OZ_Process *process);
void *oz_knl_process_gethwctx (OZ_Process *process);
OZ_Devunit **oz_knl_process_getdevalloc (OZ_Process *process);
uLong oz_knl_process_createpagetable (OZ_Process *process, OZ_Pagetable_expand expand, OZ_Mempage maxpages, OZ_Mempage basepage);
void oz_knl_process_deletepagetable (OZ_Mempage vpage);
uLong oz_knl_process_mapsection (OZ_Section *section, 
                                 OZ_Mempage *npagem, 
                                 OZ_Mempage *svpage, 
                                 uLong mapsecflag, 
                                 OZ_Procmode ownermode, 
                                 OZ_Hw_pageprot pageprot);
uLong oz_knl_process_mapsections (uLong mapsecflags, 
                                  int nsections, 
                                  OZ_Mapsecparam *mapsecparams);
uLong oz_knl_process_unmapsec (OZ_Mempage vpage);
OZ_Mempage oz_knl_process_getsecfromvpage (OZ_Process *process, 
                                           OZ_Mempage vpage, 
                                           OZ_Section **section, 
                                           OZ_Mempage *pageoffs, 
                                           OZ_Hw_pageprot *pageprot, 
                                           OZ_Procmode *procmode, 
                                           uLong *mapsecflags);
OZ_Mempage oz_knl_process_getsecfromvpage2 (OZ_Process *process, 
                                            OZ_Mempage *svpage, 
                                            OZ_Section **section, 
                                            OZ_Mempage *pageoffs, 
                                            OZ_Hw_pageprot *pageprot, 
                                            OZ_Procmode *procmode, 
                                            uLong *mapsecflags);
uLong oz_knl_process_lockpt (OZ_Process *process);
void oz_knl_process_unlkpt (OZ_Process *process, uLong pt);
OZ_Seclock **oz_knl_process_seclocks (OZ_Process *process, OZ_Mempage npages, OZ_Mempage svpage);
int oz_knl_process_nseclocks (OZ_Process *process, OZ_Mempage vpage, int inc);
OZ_Seclkwz **oz_knl_process_seclkwzs (OZ_Process *process, OZ_Mempage vpage);
int oz_knl_process_nseclkwzs (OZ_Process *process, OZ_Mempage vpage, int inc);
uLong oz_knl_process_lockps (OZ_Process *process);
void oz_knl_process_unlkps (OZ_Process *process, uLong ps);
void oz_knl_process_dump_all (void);
void oz_knl_process_dump (OZ_Process *process, int dothreads);
void oz_knl_process_validateall (void);

#endif
