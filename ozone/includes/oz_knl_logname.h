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

#ifndef _OZ_KNL_LOGNAME_H
#define _OZ_KNL_LOGNAME_H

#define OZ_LOGNAMATR_TABLE       (0x01)
#define OZ_LOGNAMATR_NOSUPERSEDE (0x04)
#define OZ_LOGNAMATR_NOOUTERMODE (0x08)

#define OZ_LOGVALATR_OBJECT      (0x02)
#define OZ_LOGVALATR_TERMINAL    (0x10)

#define OZ_LOGNAME_SIZEMAX 4096
#define OZ_LOGNAME_MAXNAMSZ 256
#define OZ_LOGNAME_MAXLEVEL 8
#define OZ_LOGNAME_TABLECHR '%'
#define OZ_LOGNAME_TABLESTR "%"

typedef struct { uLong attr;
                 void *buff;
               } OZ_Logvalue;

#ifdef _OZ_KNL_LOGNAME_C
typedef struct OZ_Logname OZ_Logname;
typedef struct OZ_Lognamesearch OZ_Lognamesearch;
#else
typedef void OZ_Logname;
typedef void OZ_Lognamesearch;
#endif

#include "ozone.h"
#include "oz_knl_objtype.h"
#include "oz_knl_procmode.h"
#include "oz_knl_security.h"

uLong oz_knl_logname_search (OZ_Lognamesearch **lognamesearch_r, 
                             OZ_Procmode procmode, 
                             const char *tablename, 
                             const char *name, 
                             OZ_Logname **logname_r, 
                             uLong *index_r, 
                             OZ_Procmode *procmode_r, 
                             uLong *lognamatr_r, 
                             uLong *nvalues_r, 
                             const OZ_Logvalue **values_r, 
                             OZ_Logname **lognamtbl_r);
uLong oz_knl_logname_creobj (OZ_Logname *lognamtbl, OZ_Procmode procmode, OZ_Seckeys *seckeys, OZ_Secattr *secattr, uLong lognamatr, int name_l, const char *name, void *object, OZ_Logname **logname_r);
uLong oz_knl_logname_crestr (OZ_Logname *lognamtbl, OZ_Procmode procmode, OZ_Seckeys *seckeys, OZ_Secattr *secattr, uLong lognamatr, int name_l, const char *name, const char *string, OZ_Logname **logname_r);
uLong oz_knl_logname_create (OZ_Logname *lognamtbl, OZ_Procmode procmode, OZ_Seckeys *seckeys, OZ_Secattr *secattr, uLong lognamatr, int name_l, const char *name, uLong nvalues, const OZ_Logvalue values[], OZ_Logname **logname_r);
uLong oz_knl_logname_lookup (OZ_Logname *lognamtbl, OZ_Procmode procmode, int name_l, const char *name, 
                             OZ_Procmode *procmode_r, uLong *lognamatr_r, uLong *nvalues_r, 
                             const OZ_Logvalue *values_r[], OZ_Logname **logname_r, OZ_Logname **lognamtbl_r);
uLong oz_knl_logname_parse (OZ_Logname *lognamtbl, OZ_Procmode procmode, int name_l, const char *name, OZ_Logname **lognamtbl_r, int *name_l_r, const char **name_r);
uLong oz_knl_logname_objstr (OZ_Logname *logname, uLong index, int buflen, char *buffer);
uLong oz_knl_logname_getobj (OZ_Logname *logname, uLong index, OZ_Objtype objtype, void **object_r);
uLong oz_knl_logname_getval (OZ_Logname *logname, OZ_Procmode *procmode_r, uLong *lognamatr_r, uLong *nvalues_r, 
                             const OZ_Logvalue *values_r[], OZ_Logname **lognamtbl_r);
uLong oz_knl_logname_gettblent (OZ_Logname *lognamtbl, OZ_Logname **logname_r);
const char *oz_knl_logname_getname (OZ_Logname *logname);
OZ_Secattr *oz_knl_logname_getsecattr (OZ_Logname *logname);
Long oz_knl_logname_increfc (OZ_Logname *logname, Long inc);
void oz_knl_logname_delete (OZ_Logname *logname);
uLong oz_knl_logname_incobjrefc (void *object, Long inc, char **objtypstr);
void oz_knl_logname_dump (int level, OZ_Logname *logname);

#endif
