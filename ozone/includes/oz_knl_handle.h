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

#ifndef _OZ_KNL_HANDLE_H
#define _OZ_KNL_HANDLE_H

#include "ozone.h"

#define OZ_HANDLE_DEFAULT_MAXENTRIES (65536)
#define OZ_HANDLE_DEFAULT_COUNTINC (256)

typedef uLong OZ_Handle;

#ifdef _OZ_KNL_HANDLE_C
typedef struct OZ_Handletbl OZ_Handletbl;
#else
typedef void OZ_Handletbl;
#endif

#include "oz_knl_objtype.h"
#include "oz_knl_procmode.h"
#include "oz_knl_security.h"
#include "oz_knl_thread.h"

void oz_knl_handle_init (void);
uLong oz_knl_handletbl_create (void);
void oz_knl_handletbl_delete (void);
uLong oz_knl_handle_assign (void *object, OZ_Procmode procmode, OZ_Handle *handle_r);
void oz_knl_handle_tablecopied (void);
OZ_Handle oz_knl_handletbl_statsget (OZ_Handle **handles, void ***objects, OZ_Objtype **objtypes, OZ_Thread ***threads, 
                                     OZ_Procmode **procmodes, OZ_Secaccmsk **secaccmsks, Long **refcounts);
void oz_knl_handletbl_statsfree (OZ_Handle numentries, OZ_Handle *handles, void **objects, OZ_Objtype *objtypes, 
                                 OZ_Thread **threads, OZ_Procmode *procmodes, OZ_Secaccmsk *secaccmsks, Long *refcounts);
uLong oz_knl_handle_takeout (OZ_Handle handle, OZ_Procmode procmode, OZ_Secaccmsk secaccmsk, OZ_Objtype objtype, void **object_r, OZ_Secaccmsk *secaccmsk_r);
void oz_knl_handle_putback (OZ_Handle handle);
uLong oz_knl_handle_getsecaccmsk (OZ_Handle handle, OZ_Procmode procmode, OZ_Secaccmsk *secaccmsk_r);
uLong oz_knl_handle_setsecaccmsk (OZ_Handle handle, OZ_Procmode procmode, OZ_Secaccmsk secaccmsk);
OZ_Handle oz_knl_handle_next (OZ_Handle handle, OZ_Procmode procmode);
uLong oz_knl_handle_setthread (OZ_Handle handle, OZ_Procmode procmode, OZ_Thread *thread);
void oz_knl_handle_release_all (OZ_Thread *thread, OZ_Procmode procmode);
uLong oz_knl_handle_release (OZ_Handle handle, OZ_Procmode procmode);
Long oz_knl_handle_objincrefc (OZ_Objtype objtype, void *object, Long inc);

#endif
