//+++2002-08-17
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
//---2002-08-17

/************************************************************************/
/*									*/
/*  General kernel heap memory allocation routines			*/
/*									*/
/************************************************************************/

#ifndef _OZ_KNL_KMALLOC_H
#define _OZ_KNL_KMALLOC_H

#include "oz_knl_malloc.h"

#define OZ_KNL_NPPMALLOC(__size) oz_knl_nppmallo (__size, 0)
#define OZ_KNL_NPPMALLOQ(__size) oz_knl_nppmallo (__size, 2)
#define OZ_KNL_NPPVALID(__adrs) oz_knl_nppvalid (__adrs)
#define OZ_KNL_NPPFREE(__adrs) oz_knl_nppfree (__adrs)
#define OZ_KNL_NPPFREESIZ(__size,__adrs) oz_knl_nppfreesiz (__size, __adrs)
#define OZ_KNL_PCMALLOC(__size) oz_knl_nppmallo (__size, 1)
#define OZ_KNL_PCMALLOQ(__size) oz_knl_nppmallo (__size, 3)

#define OZ_KNL_PGPMALLOC(__size) oz_knl_pgpmalloq (__size, 0)
#define OZ_KNL_PGPMALLOQ(__size) oz_knl_pgpmalloq (__size, 1)
#define OZ_KNL_PGPVALID(__adrs) oz_knl_pgpvalid (__adrs)
#define OZ_KNL_PGPFREE(__adrs) oz_knl_pgpfree (__adrs)

void *oz_knl_nppmallo (OZ_Memsize size, int flags);
void oz_knl_nppvalid (void *adrs);
void oz_knl_nppfree (void *adrs);
void oz_knl_nppfreesiz (OZ_Memsize size, void *adrs);
void oz_knl_npp_dump (void);

void *oz_knl_pgpmalloq (OZ_Memsize size, int quotaed);
void oz_knl_pgpvalid (void *adrs);
void oz_knl_pgpfree (void *adrs);

#endif
