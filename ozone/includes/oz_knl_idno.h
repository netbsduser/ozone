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

#ifndef _OZ_KNL_IDNO_H
#define _OZ_KNL_IDNO_H

#include "oz_knl_objtype.h"

void  oz_knl_idno_init  (void);
uLong oz_knl_idno_alloc (void *object);
void *oz_knl_idno_find  (uLong idno, OZ_Objtype objtype);
void  oz_knl_idno_free  (uLong idno);

#endif
