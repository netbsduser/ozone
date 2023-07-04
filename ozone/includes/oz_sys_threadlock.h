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

#ifndef _OZ_SYS_THREADLOCK_H
#define _OZ_SYS_THREADLOCK_H

typedef struct { volatile Long flag;
                 volatile OZ_Handle h_event;
                 const char *name;
               } OZ_Threadlock;

#define OZ_SYS_THREADLOCK_INIT(__threadlock,__name) OZ_Threadlock __threadlock = { 0, 0, __name }
#define OZ_SYS_THREADLOCK_WAIT(__threadlock) oz_sys_threadlock_wait (&__threadlock)
#define OZ_SYS_THREADLOCK_CLR(__threadlock,__tl) oz_sys_threadlock_clr (&__threadlock, __tl)

int oz_sys_threadlock_wait (OZ_Threadlock *threadlock);
void oz_sys_threadlock_clr (OZ_Threadlock *threadlock, int restore);

#endif
