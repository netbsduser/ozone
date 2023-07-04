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

#ifndef _OZ_SYS_IOPARSE_H
#define _OZ_SYS_IOPARSE_H

#define OZ_IOPARSE_EXPSIZE 256

#include "ozone.h"

#ifdef _OZ_SYS_IOPARSE_C
typedef struct OZ_Ioparse OZ_Ioparse;
#else
typedef void OZ_Ioparse;
#endif

uLong oz_sys_ioparse_init (const char *partspec, const char *defspec, OZ_Ioparse **expand_r);
uLong oz_sys_ioparse_next (OZ_Ioparse *expand, uLong expsize, char *expbuff);
void oz_sys_ioparse_term (OZ_Ioparse *expand);

#endif
