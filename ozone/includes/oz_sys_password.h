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

#ifndef _OZ_SYS_PASSWORD_H
#define _OZ_SYS_PASSWORD_H

#define OZ_PASSWORD_MAX (32)

typedef enum { OZ_PASSWORD_CODE_PASSHASH, 
               OZ_PASSWORD_CODE_SECKEYSP, 
               OZ_PASSWORD_CODE_DEFCRESECATTRP, 
               OZ_PASSWORD_CODE_QUOTASTRP, 
               OZ_PASSWORD_CODE_IMAGENAMEP, 
               OZ_PASSWORD_CODE_DEFDIRP, 
               OZ_PASSWORD_CODE_DEFBASEPRI, 
               OZ_PASSWORD_CODE_MAXBASEPRI
             } OZ_Password_code;

typedef struct { OZ_Password_code code;
                 uLong size;
                 void *buff;
                 uLong *rlen;
               } OZ_Password_item;

#include "oz_knl_hw.h"
#include "oz_sys_hash.h"

#define OZ_PASSWORD_HASHSIZE OZ_SYS_HASH_STRSIZE

uLong oz_sys_password_getbyusername (const char *username, int nitems, OZ_Password_item *items, int *index_r, 
                                     void *(*ment) (void *mprm, uLong osize, void *obuff, uLong nsize), void *mprm);
void oz_sys_password_hashit (const char *password, int hashsize, char *hashbuff);
OZ_HW_SYSCALL_DCL_2 (password_change, const char *, oldpw, const char *, newpw)

#endif
