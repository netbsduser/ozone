//+++2002-05-10
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
//---2002-05-10

#ifndef _OZ_KNL_USERJOB_H
#define _OZ_KNL_USERJOB_H

#define OZ_USERNAME_MAX (32)
#define OZ_JOBNAME_MAX (64)

#ifdef _OZ_KNL_USERJOB_C
typedef struct OZ_User OZ_User;
typedef struct OZ_Job OZ_Job;
#else
typedef void OZ_User;
typedef void OZ_Job;
#endif

#include "ozone.h"

#include "oz_knl_devio.h"
#include "oz_knl_logname.h"
#include "oz_knl_quota.h"

void oz_knl_userjob_init (void);
uLong oz_knl_user_create (const char *username, OZ_Quota *quota, OZ_Seckeys *seckeys, uLong maxbasepri, OZ_User **user_r);
uLong oz_knl_job_create (OZ_User *user, const char *name, OZ_Job **job_r);
Long oz_knl_user_increfc (OZ_User *user, Long inc);
Long oz_knl_job_increfc (OZ_Job *job, Long inc);

OZ_User *oz_knl_job_getuser (OZ_Job *job);
char *oz_knl_job_getname (OZ_Job *job);
OZ_Logname *oz_knl_job_getlognamdir (OZ_Job *job);
OZ_Logname *oz_knl_job_getlognamtbl (OZ_Job *job);
OZ_Job *oz_knl_job_getnext (OZ_Job *lastjob, OZ_User *user);
uLong oz_knl_job_count (OZ_User *user);
OZ_Devunit **oz_knl_job_getdevalloc (OZ_Job *job);
char *oz_knl_user_getname (OZ_User *user);
OZ_Quota *oz_knl_user_getquota (OZ_User *user);
OZ_Logname *oz_knl_user_getlognamdir (OZ_User *user);
OZ_Logname *oz_knl_user_getlognamtbl (OZ_User *user);
uLong oz_knl_user_count (void);
OZ_User *oz_knl_user_getnext (OZ_User *lastuser);
OZ_Seckeys *oz_knl_user_getseckeys (OZ_User *user);
uLong oz_knl_user_getmaxbasepri (OZ_User *user);
OZ_Devunit **oz_knl_user_getdevalloc (OZ_User *user);

#endif
