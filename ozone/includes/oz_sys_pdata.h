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

#ifndef _OZ_SYS_PDATA_H
#define _OZ_SYS_PDATA_H

#include "oz_knl_event.h"
#include "oz_knl_handle.h"
#include "oz_knl_image.h"
#include "oz_knl_malloc.h"
#include "oz_knl_process.h"

typedef struct {

  /* This data is initialized by the hardware process initialization routines and does not change */

  OZ_Hw_pageprot rwpageprot;		// page protection code for this processor mode
					// to access pages for reading and writing
					// - filled in by process hw init routine
					//   and is in essence what this page is set to

  OZ_Procmode procmode;			// processor mode that owns this data

  OZ_Procmode pprocmode;		// parameter to pass to oz_sys_pdata_... routines
					// same as 'procmode' except for oz_s_systempdata 
					// ... where it is OZ_PROCMODE_SYS

  /* Used by oz_sys_pdata_malloc/oz_sys_pdata_free */

  Long volatile mem_lock_flag;		// 0=unlocked, 1=locked, 3=locked w/waiters
  OZ_Event *volatile mem_lock_event;	// locking event flag - kernel
  OZ_Handle volatile mem_lock_handle;	// locking event flag - not kernel
  OZ_Memlist *volatile mem_list;	// memory list for oz_malloc, etc

  /* Used by kernel mode entries only */

  OZ_Handletbl *volatile handletbl;	// process' handle table pointer
  OZ_Imagelist *volatile imagelist;	// list of images loaded in process

  OZ_Processid processid;		// this process' id
  OZ_Process *process;			// this process' struct pointer
					// - this does not have a refcount, as the pdata array 
					//   will be gone long before the process is freed off

} OZ_Pdata;

/* Use the oz_sys_pdata_pointer routine to access the array                                            */
/* Each element is one page long and is protected from access by outer modes                           */
/* The array is initialized to all zeroes by process create code                                       */
/* The system process has no such array, but has oz_s_systempdata (double mapped) for kernel mode only */

globalref union { OZ_Pdata data;
                  uByte fill[1<<OZ_HW_L2PAGESIZEMAX];
                } oz_sys_pdata_array[1+OZ_PROCMODE_MAX-OZ_PROCMODE_MIN];

/* These are alternatives to oz_sys_pdata_pointer */

	// usable when it is known that the caller is in kernel mode

#define OZ_SYS_PDATA_FROM_KNL(__procmode) (&(oz_sys_pdata_array[(__procmode)-OZ_PROCMODE_MIN].data))

	// usable when the caller is in kernel mode and wants access to system-global pdata

#define OZ_SYS_PDATA_SYSTEM (&oz_s_systempdata)

/* These routines access the pdata */

OZ_Pdata *oz_sys_pdata_pointer (OZ_Procmode procmode);
void *oz_sys_pdata_malloc (OZ_Procmode procmode, uLong size);
void oz_sys_pdata_free (OZ_Procmode procmode, void *buff);
uLong oz_sys_pdata_valid (OZ_Procmode procmode, void *buff);
uLong oz_sys_pdata_copied (void);
void oz_sys_pdata_cleanup (void);

#endif
