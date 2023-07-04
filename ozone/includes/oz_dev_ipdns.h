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

#ifndef _OZ_DEV_IPDNS_H
#define _OZ_DEV_IPDNS_H

#include "oz_knl_devio.h"

uLong oz_dev_ipdns_init (int iopexsize);
void oz_dev_ipdns_abort (OZ_Iochan *iochan, OZ_Ioop *ioop, OZ_Procmode procmode);
uLong oz_dev_ipdns_dnssvradd (OZ_Procmode procmode, uLong as, void *ap, OZ_Ioop *ioop);
uLong oz_dev_ipdns_dnssvrrem (OZ_Procmode procmode, uLong as, void *ap, OZ_Ioop *ioop);
uLong oz_dev_ipdns_dnssvrlist (OZ_Procmode procmode, uLong as, void *ap, OZ_Ioop *ioop);
uLong oz_dev_ipdns_dnslookup (OZ_Procmode procmode, uLong as, void *ap, OZ_Ioop *ioop, void *iopexv);

#endif
