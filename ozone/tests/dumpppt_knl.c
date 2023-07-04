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

/************************************************************************/
/*									*/
/*  Dump some process' page table - kernel image			*/
/*									*/
/*	dumpppt_knl (cprocmode, &dpptpb)				*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_knl_hw.h"
#include "oz_knl_process.h"
#include "oz_knl_sdata.h"
#include "oz_knl_section.h"
#include "oz_knl_status.h"
#include "oz_sys_callknl.h"
#include "oz_sys_io_fs_printf.h"
#include "oz_sys_xprintf.h"
#include "oz_util_start.h"

#include "dumpppt.h"

uLong dumpppt_knl (OZ_Procmode cprocmode, void *dpptpbv)

{
  Dpptpb dpptpb;
  int si;
  OZ_Process *process, *saveprocess;
  uLong pt, sts;

  dpptpb   = *(Dpptpb *)dpptpbv;							// get parameter block on kernel stack
											// - so it will be here when we switch procs
  si       = oz_hw_cpu_setsoftint (0);							// so we don't get aborted with process refcount incremented
  process  = oz_knl_process_frompid (dpptpb.pid);					// get process object
  sts = OZ_NOSUCHPROCESS;								// assume pid is bad
  if (process != NULL) {
    saveprocess = oz_knl_process_getcur ();						// save calling process
    oz_knl_process_setcur (process);							// address new process
    pt = oz_knl_process_lockpt (process);						// lock its pagetable
    dpptpb.pteva = oz_hw_pte_readany (dpptpb.virtpage, &dpptpb.pagestate, &dpptpb.phypage, &dpptpb.curprot, &dpptpb.reqprot);
    if ((dpptpb.pteva == NULL) && ((dpptpb.pagestate == OZ_SECTION_PAGESTATE_VALID_R) 	// see if there is an assoc phys page
                                || (dpptpb.pagestate == OZ_SECTION_PAGESTATE_VALID_W) 
                                || (dpptpb.pagestate == OZ_SECTION_PAGESTATE_VALID_D))) {
      if (dpptpb.phypage >= oz_s_phymem_totalpages) dpptpb.phypagestate.state = -1;
      else dpptpb.phypagestate = oz_s_phymem_pages[dpptpb.phypage];			// if so, read its state, too
    }
    oz_knl_process_unlkpt (process, pt);						// unlock process' pagetable
    oz_knl_process_setcur (saveprocess);						// restore calling process
    oz_knl_process_increfc (process, -1);						// done with target process object
    *(Dpptpb *)dpptpbv = dpptpb;							// return parameter block
    sts = OZ_SUCCESS;									// we did it
  }
  oz_hw_cpu_setsoftint (si);								// restore softints
  return (sts);
}
