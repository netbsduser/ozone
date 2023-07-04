//+++2002-01-14
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
//---2002-01-14

#ifndef _OZ_LDR_LOADER_H
#define _OZ_LDR_LOADER_H

#include "oz_knl_hw.h"
#include "oz_ldr_params.h"

void *oz_ldr_start (OZ_Loadparams *loadparams_p, 

                    void *sysbaseva, 
                    OZ_Mempage startphypage, 
                    OZ_Mempage phypages,
                    uLong tempmemsize, 
                    void *tempmemaddr, 

                    uLong *kstacksize, 
                    uLong *systempages, 

                    OZ_Mempage cacheareasize, 
                    OZ_Mempage cacheareabase, 

                    uLong ntempsptes, 
                    OZ_Mempage tempsptes);

#endif
