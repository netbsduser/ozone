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

#ifndef _OZ_KNL_OBJTYPE_H
#define _OZ_KNL_OBJTYPE_H

/* The OZ_Objtype is the first item of any object-like */
/* structure and must be one of the following codes:   */

typedef enum { OZ_OBJTYPE_UNKNOWN, 		//  0
               OZ_OBJTYPE_AST, 			//  1
               OZ_OBJTYPE_EVENT, 		//  2
               OZ_OBJTYPE_PROCESS, 		//  3
               OZ_OBJTYPE_SECATTR, 		//  4
               OZ_OBJTYPE_SECKEYS, 		//  5
               OZ_OBJTYPE_SECTION, 		//  6
               OZ_OBJTYPE_THREAD, 		//  7
               OZ_OBJTYPE_KTHREAD, 		//  8
               OZ_OBJTYPE_EXHAND, 		//  9
               OZ_OBJTYPE_DEVUNIT, 		//  A
               OZ_OBJTYPE_DEVDRIVER, 		//  B
               OZ_OBJTYPE_DEVCLASS, 		//  C
               OZ_OBJTYPE_IOOP, 		//  D
               OZ_OBJTYPE_IOCHAN, 		//  E
               OZ_OBJTYPE_IOSELECT, 		//  F
               OZ_OBJTYPE_LOWIPL,		// 10
               OZ_OBJTYPE_HANDLETBL,		// 11
               OZ_OBJTYPE_IMAGE,		// 12
               OZ_OBJTYPE_LOGNAME,		// 13
               OZ_OBJTYPE_LOGNAMESEARCH,	// 14
               OZ_OBJTYPE_CONDHAND,		// 15
               OZ_OBJTYPE_TIMER,		// 16
               OZ_OBJTYPE_IOPARSE,		// 17
               OZ_OBJTYPE_SECLOCK,		// 18
               OZ_OBJTYPE_DPAR,			// 19
               OZ_OBJTYPE_USER,			// 1A
               OZ_OBJTYPE_JOB,			// 1B
               OZ_OBJTYPE_QUOTA,		// 1C
               OZ_OBJTYPE_CACHE,		// 1D
               OZ_OBJTYPE_DCACHE,		// 1E
               OZ_OBJTYPE_NCACHE,		// 1F
               OZ_OBJTYPE_LIOD,			// 20
               OZ_OBJTYPE_LIOR,			// 21
               OZ_OBJTYPE_SHUTHAND,		// 22
               OZ_OBJTYPE_SECLKWZ, 		// 23
               OZ_OBJTYPE_LOG, 			// 24
               OZ_OBJTYPE_THREADLOCK, 		// 25
               OZ_OBJTYPE_ISAIRQ, 		// 26
               OZ_OBJTYPE_PCIIRQ, 		// 27
               OZ_OBJTYPE_PCICONF, 		// 28
               OZ_OBJTYPE_PCIDMA32MAP, 		// 29
               OZ_OBJTYPE_PCIDMA64MAP, 		// 30

               OZ_OBJTYPE_MAX			// 31
             } OZ_Objtype;

/* This macro verifies the object type of a structure */

#define OZ_KNL_CHKOBJTYPE(o,t) \
	do { OZ_Objtype *__o = (void *)(o); \
	     if ((__o != NULL) && (*__o != t)) oz_crash ("%s %d: object type %d not expected %d", __FILE__, __LINE__, *__o, t); \
	   } while (0)

/* This macro gets the object type of a structure */

#define OZ_KNL_GETOBJTYPE(__o) (*(OZ_Objtype *)(__o))

#endif
