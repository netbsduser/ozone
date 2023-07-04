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

#ifndef _OZ_DEV_ISA_H
#define _OZ_DEV_ISA_H

/************************************************************************/
/*									*/
/*  Accessing ISA bus							*/
/*									*/
/************************************************************************/

/* Internal struct definitions */

#ifdef _OZ_DEV_ISA_C
typedef struct OZ_Dev_Isa_Irq OZ_Dev_Isa_Irq;
#else
typedef void OZ_Dev_Isa_Irq;
#endif

/* Entrypoints */

#include "oz_knl_hw.h"

OZ_Dev_Isa_Irq *oz_dev_isa_irq_alloc (uLong irq, void (*entry) (void *param, OZ_Mchargs *mchargs), void *param);
OZ_Smplock *oz_dev_isa_irq_smplock (OZ_Dev_Isa_Irq *isairq);
void oz_dev_isa_irq_reset (OZ_Dev_Isa_Irq *isairq, void (*entry) (void *param, OZ_Mchargs *mchargs), void *param);
void oz_dev_isa_irq_free (OZ_Dev_Isa_Irq *isairq);

#endif
