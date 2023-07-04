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

#ifndef _OZ_KNL_PROCMODE_H
#define _OZ_KNL_PROCMODE_H

/* Processor mode - kernel or user */

typedef enum { OZ_PROCMODE_KNL, 	/* kernel mode */
               OZ_PROCMODE_USR,		/* user mode */
               OZ_PROCMODE_SYS		/* special mode flag to pass to oz_sys_pdata_... routines */
					/* ... to indicate we want the system process data        */
             } OZ_Procmode;

#define OZ_PROCMODE_MIN OZ_PROCMODE_KNL
#define OZ_PROCMODE_MAX OZ_PROCMODE_USR

#endif
