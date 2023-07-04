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

#ifndef _OZ_DEV_LIO_H
#define _OZ_DEV_LIO_H

#ifdef _OZ_DEV_LIO_C
typedef struct OZ_Liod OZ_Liod;
typedef struct OZ_Lior OZ_Lior;
#else
typedef void OZ_Liod;
typedef void OZ_Lior;
#endif

#include "oz_knl_devio.h"
#include "oz_knl_procmode.h"

OZ_Liod *oz_dev_lio_init (OZ_Iochan *iochan);
void oz_dev_lio_term (OZ_Liod *liod);
void oz_dev_lio_abort (OZ_Liod *liod, OZ_Iochan *iochan, OZ_Ioop *ioop, OZ_Procmode procmode);
OZ_Lior *oz_dev_lio_start (OZ_Liod *liod, OZ_Ioop *ioop, OZ_Procmode procmode);
void oz_dev_lio_io (OZ_Lior *lior, OZ_Procmode procmode, 
                     void (*astentry) (void *astparam, uLong status), void *astparam, 
                     uLong funcode, uLong as, void *ap);
void oz_dev_lio_done (OZ_Lior *lior, uLong status, void (*finentry) (void *finparam, int finok, uLong *status_r), void *finparam);

#endif
