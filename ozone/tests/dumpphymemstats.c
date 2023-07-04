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

#include "ozone.h"
#include "oz_knl_status.h"
#include "oz_sys_callknl.h"

extern uLong oz_s_phymem_stat_freel2;
extern uLong oz_s_phymem_stat_freel1;
extern uLong oz_s_phymem_stat_freeany;
extern uLong oz_s_phymem_stat_cachel2;
extern uLong oz_s_phymem_stat_cachel1;
extern uLong oz_s_phymem_stat_cacheany;

static uLong freel2;
static uLong freel1;
static uLong freeany;
static uLong cachel2;
static uLong cachel1;
static uLong cacheany;

static uLong getstats (OZ_Procmode cprocmode, void *dummy);

int main ()

{
  uLong sts;

  sts = oz_sys_callknl (getstats, NULL);
  oz_sys_io_fs_printerror ("freel2 %u, freel1 %u, freeany %u\n", freel2, freel1, freeany);
  oz_sys_io_fs_printerror ("cachel2 %u, cachel1 %u, cacheany %u\n", cachel2, cachel1, cacheany);
  return (sts);
}

static uLong getstats (OZ_Procmode cprocmode, void *dummy)

{
  freel2   = oz_s_phymem_stat_freel2;
  freel1   = oz_s_phymem_stat_freel1;
  freeany  = oz_s_phymem_stat_freeany;
  cachel2  = oz_s_phymem_stat_cachel2;
  cachel1  = oz_s_phymem_stat_cachel1;
  cacheany = oz_s_phymem_stat_cacheany;
  return (OZ_SUCCESS);
}
