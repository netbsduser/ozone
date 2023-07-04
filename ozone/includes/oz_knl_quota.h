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

#ifndef _OZ_KNL_QUOTA_H
#define _OZ_KNL_QUOTA_H

typedef enum { OZ_QUOTATYPE_END, 	/* end of a quotalist */
               OZ_QUOTATYPE_NPP, 	/* bytes of non-paged pool */
               OZ_QUOTATYPE_PGP, 	/* bytes of paged pool */
               OZ_QUOTATYPE_PHM,	/* pages of physical memory */

               OZ_QUOTATYPE_MAX
             } OZ_Quotatype;

#define OZ_KNL_QUOTA_DEFAULT oz_knl_quota_getcpudef ()

#ifdef _OZ_KNL_QUOTA_C
typedef struct OZ_Quota OZ_Quota;
#else
typedef void OZ_Quota;
#endif

uLong oz_knl_quota_create (int quotastr_l, const char *quotastr, OZ_Quota **quota_r);
uLong oz_knl_quota_string (OZ_Quota *quota, int usages, int bufsize, char *bufaddr);
void oz_knl_quota_update (OZ_Quota *quota, OZ_Quota *newquota);
Long oz_knl_quota_increfc (OZ_Quota *quota, Long inc);
OZ_Quota *oz_knl_quota_setcpudef (OZ_Quota *quota);
OZ_Quota *oz_knl_quota_getcpudef (void);
OZ_Quota *oz_knl_quota_fromobj (void *quotaobj);
int oz_knl_quota_debit (OZ_Quota *quota, OZ_Quotatype quotatype, Long amount);
void oz_knl_quota_credit (OZ_Quota *quota, OZ_Quotatype quotatype, Long amount, Long inc);

#endif
