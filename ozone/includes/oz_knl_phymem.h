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

#ifndef _OZ_KNL_PHYMEM_H
#define _OZ_KNL_PHYMEM_H

#include "oz_knl_hw.h"
#include "oz_knl_sdata.h"

void oz_knl_phymem_init (uLong nppsize, OZ_Mempage ffvirtpage, OZ_Mempage ffphyspage);
OZ_Mempage oz_knl_phymem_allocpagew (OZ_Phymem_pagestate pagestate, OZ_Mempage virtpage);
OZ_Mempage oz_knl_phymem_allocpage (OZ_Phymem_pagestate pagestate, OZ_Mempage virtpage);
OZ_Mempage oz_knl_phymem_allocontig (OZ_Mempage count, OZ_Phymem_pagestate pagestate, OZ_Mempage virtpage);
void oz_knl_phymem_freepage (OZ_Mempage phypage);

#endif
