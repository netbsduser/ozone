//+++2003-03-01
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
//---2003-03-01

#include "ozone.h"
#include "oz_knl_cache.h"
#include "oz_knl_sdata.h"
#include "oz_knl_status.h"
#include "oz_sys_callknl.h"
#include "oz_sys_io_fs_printf.h"
#include "oz_util_start.h"

static uLong main_knl (OZ_Procmode cprocmode, void *dummy);

uLong oz_util_main (int argc, char *argv[])

{
  uLong sts;

  sts = oz_sys_callknl (main_knl, NULL);
  if (sts != OZ_SUCCESS) oz_sys_io_fs_printf (oz_util_h_error, "error %u calling main_knl\n", sts);
  return (sts);
}

static uLong main_knl (OZ_Procmode cprocmode, void *dummy)

{
  const char *name;
  int i;
  OZ_Cachepage cachepage;
  OZ_Mempage phypage;
  OZ_Pagentry savepte;
  uLong cksm, cp, pm, *vaddr;

  pm = oz_hw_smplock_wait (&oz_s_smplock_pm);
  for (phypage = 0; phypage < oz_s_phymem_totalpages; phypage ++) {
    if (oz_s_phymem_pages[phypage].state == OZ_PHYMEM_PAGESTATE_ALLOCACHE) {
      cp = oz_knl_cache_smplock_wait (phypage);
      cachepage = oz_s_phymem_pages[phypage].u.c;
      cksm = 0;
      if (cachepage.refcount == 0) {
        vaddr = oz_hw_phys_mappage (phypage, &savepte);
        for (i = 0; i < (1 << OZ_HW_L2PAGESIZE) / 4; i ++) {
          cksm += *(vaddr ++);
        }
        oz_hw_phys_unmappage (savepte);
      }
      oz_knl_cache_smplock_clr (phypage, cp);
      oz_hw_smplock_clr (&oz_s_smplock_pm, pm);
      name = oz_knl_cache_getname (cachepage.cache);
      if (cachepage.refcount != 0) {
        oz_sys_io_fs_printf (oz_util_h_output, "[%5X]  %3d  %3d  %8.8X           %8.8X %s\n", phypage, 
		cachepage.refcount, cachepage.lockcount, cachepage.checksum, cachepage.key, name);
      } else {
        oz_sys_io_fs_printf (oz_util_h_output, "[%5X]  %3d  %3d  %8.8X %8.8X  %8.8X %s\n", phypage, 
		cachepage.refcount, cachepage.lockcount, cachepage.checksum, cksm, cachepage.key, name);
        if (cachepage.checksum != cksm) return (OZ_BUGCHECK);
      }
      pm = oz_hw_smplock_wait (&oz_s_smplock_pm);
    }
  }
  oz_hw_smplock_clr (&oz_s_smplock_pm, pm);

  return (OZ_SUCCESS);
}
