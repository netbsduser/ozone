//+++2001-10-06
//    Copyright (C) 2001, Mike Rieker, Beverly, MA USA
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
//---2001-10-06

/************************************************************************/
/*									*/
/*  System page table entry allocation routines				*/
/*									*/
/************************************************************************/

#ifndef _OZ_KNL_SPTE_H
#define _OZ_KNL_SPTE_H

#include "oz_knl_hw.h"

uLong oz_knl_spte_alloc (OZ_Mempage count, void **sysvaddr, OZ_Mempage *sysvpage, OZ_Section **section_r);
void oz_knl_spte_free (OZ_Mempage count, OZ_Mempage sysvpage);

#endif
