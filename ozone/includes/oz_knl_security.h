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

#ifndef _OZ_KNL_SECURITY_H
#define _OZ_KNL_SECURITY_H

#ifdef _OZ_KNL_SECURITY_C
typedef struct OZ_Secattr OZ_Secattr;
typedef struct OZ_Seckeys OZ_Seckeys;
#else
typedef void OZ_Secattr;
typedef void OZ_Seckeys;
#endif

typedef uLong OZ_Secaccmsk;
typedef uLong OZ_Secaction;
typedef uLong OZ_Secident;

typedef struct { const char *name; uLong valu; } OZ_Secacclas;

#define OZ_SECACCMSK_LOOK (0x1)
#define OZ_SECACCMSK_READ (0x2)
#define OZ_SECACCMSK_WRITE (0x4)

#define OZ_SECACTION_DENY (0x1)
#define OZ_SECACTION_ALARM (0x2)

extern const OZ_Secacclas oz_sys_secacclas_default[];

uLong oz_knl_secattr_fromname (int namesiz, const char *namestr, int *namelen_r, const OZ_Secacclas *secacclas, OZ_Secattr **secattr_r);
uLong oz_sys_secattr_str2bin (int str_l, 
                              const char *str,
                              const OZ_Secacclas *secacclas,
                              void *(*ment) (void *mprm, uLong osize, void *obuff, uLong nsize),
                              void *mprm,
                              uLong *size_r,
                              void **buff_r);
uLong oz_sys_secattr_bin2str (uLong size,
                              const void *buffv,
                              const OZ_Secacclas *secacclas,
                              void *(*ment) (void *mprm, uLong osize, void *obuff, uLong nsize),
                              void *mprm,
                              char **buff_r);
uLong oz_knl_secattr_validate (uLong size, const void *buff, const OZ_Secacclas *secacclas);
uLong oz_knl_secattr_create (uLong size, const void *buff, const OZ_Secacclas *secacclas, OZ_Secattr **secattr_r);
Long oz_knl_secattr_increfc (OZ_Secattr *secattr, Long inc);
uLong oz_knl_secattr_getsize (OZ_Secattr *secattr);
void *oz_knl_secattr_getbuff (OZ_Secattr *secattr);

uLong oz_sys_seckeys_str2bin (int str_l, 
                              const char *str,
                              void *(*ment) (void *mprm, uLong osize, void *obuff, uLong nsize),
                              void *mprm,
                              uLong *size_r,
                              void **buff_r);
uLong oz_sys_seckeys_bin2str (uLong size,
                              const void *buffv,
                              void *(*ment) (void *mprm, uLong osize, void *obuff, uLong nsize),
                              void *mprm,
                              char **buff_r);
uLong oz_knl_seckeys_create (uLong size, const void *buff, OZ_Seckeys *seckey_limit, OZ_Seckeys **seckeys_r);
Long oz_knl_seckeys_increfc (OZ_Seckeys *seckeys, Long inc);
uLong oz_knl_seckeys_getsize (OZ_Seckeys *seckeys);
void *oz_knl_seckeys_getbuff (OZ_Seckeys *seckeys);
int oz_knl_seckeys_differ (OZ_Seckeys *oldkeys, OZ_Seckeys *newkeys);

OZ_Secaccmsk oz_knl_security_getsecaccmsk (const OZ_Seckeys *seckeys, const OZ_Secattr *secattr);
uLong oz_knl_security_check (OZ_Secaccmsk secaccmsk, const OZ_Seckeys *seckeys, const OZ_Secattr *secattr);

#endif
