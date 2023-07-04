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
/*  Dump some process' page table					*/
/*									*/
/*	dumpppt <process-id> [<vplo>-<vphi>]...				*/
/*									*/
/*  Requires kernel image dumppt_knl.oz be loaded			*/
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

typedef enum { INIT, PTEOUT, NOACCESS, LOAD, PTEVALID } State;

static const char *pagestatetbl[8], *prottbl[OZ_HW_PAGEPROT_MAX];
static Dpptpb dpptpb;

static uLong dumpppt_range (OZ_Mempage start, OZ_Mempage stop);

uLong oz_util_main (int argc, char *argv[])

{
  int i, j, k;
  OZ_Mempage start, stop;
  uLong sts;

  if (argc < 2) goto usage;

  dpptpb.pid = atoi (argv[1]);
  if (dpptpb.pid == 0) goto usage;

  pagestatetbl[OZ_SECTION_PAGESTATE_PAGEDOUT]    = "pagedout";
  pagestatetbl[OZ_SECTION_PAGESTATE_READINPROG]  = "readinprog";
  pagestatetbl[OZ_SECTION_PAGESTATE_READFAILED]  = "readfailed";
  pagestatetbl[OZ_SECTION_PAGESTATE_WRITEFAILED] = "writefailed";
  pagestatetbl[OZ_SECTION_PAGESTATE_WRITEINPROG] = "writeinprog";
  pagestatetbl[OZ_SECTION_PAGESTATE_VALID_R]     = "valid_r";
  pagestatetbl[OZ_SECTION_PAGESTATE_VALID_W]     = "valid_w";
  pagestatetbl[OZ_SECTION_PAGESTATE_VALID_D]     = "valid_d";

  prottbl[OZ_HW_PAGEPROT_NA] = "NA";
  prottbl[OZ_HW_PAGEPROT_KW] = "KW";
  prottbl[OZ_HW_PAGEPROT_UR] = "UR";
  prottbl[OZ_HW_PAGEPROT_UW] = "UW";
#ifdef OZ_HW_PAGEPROT_KR
  prottbl[OZ_HW_PAGEPROT_KR] = "KR";
#endif
#ifdef OZ_HW_PAGEPROT_KWUR
  prottbl[OZ_HW_PAGEPROT_KWUR] = "KWUR";
#endif

  if (argc == 2) {
    sts = dumpppt_range (OZ_HW_BASE_PROC_VP, OZ_HW_BASE_SYSC_VP);
  } else {
    for (i = 2; i < argc; i ++) {
      start = oz_hw_atoz (argv[i], &j);
      if (argv[i][j++] != '-') goto usage;
      stop  = oz_hw_atoz (argv[i] + j, &k);
      if (argv[i][j+k] != 0) goto usage;
      sts = dumpppt_range (start, stop + 1);
      if (sts != OZ_SUCCESS) break;
    }
  }

  return (sts);

usage:
  oz_sys_io_fs_printf (oz_util_h_error, "usage: dumpppt <process-id> [<vplo>-<vphi>]...\n");
  return (OZ_BADPARAM);
}

static uLong dumpppt_range (OZ_Mempage start, OZ_Mempage stop)

{
  char curprotstr[8], pagestatestr[12], reqprotstr[8];
  const char *oldstring, *newstring;
  OZ_Mempage oldvpage;
  State oldstate, newstate;
  uLong sts;

  oz_sys_io_fs_printf (oz_util_h_output, "Pid %u from %8.8X to %8.8X\n", dpptpb.pid, start, stop - 1);

  oldstate = newstate = INIT;								// starting state for last page scanned
  for (dpptpb.virtpage = start; (dpptpb.virtpage != stop) && (OZ_HW_VPAGETOVADDR (dpptpb.virtpage) != NULL); dpptpb.virtpage ++) {
    sts = oz_sys_callknl (dumpppt_knl, &dpptpb);					// read pte and phypage states
    if (sts != OZ_SUCCESS) {
      oz_sys_io_fs_printf (oz_util_h_error, "error %u getting pid %u virtpage %X state\n", sts, dpptpb.pid, dpptpb.virtpage);
      return (sts);
    }
    if (dpptpb.pteva != NULL) { newstate = PTEOUT; newstring = "pte paged out"; }	// decode this entry's state
    else if ((dpptpb.pagestate == OZ_SECTION_PAGESTATE_PAGEDOUT) && (dpptpb.phypage == 0) && (dpptpb.curprot == OZ_HW_PAGEPROT_NA)) { newstate = LOAD; newstring = "ready load"; }
    else if ((dpptpb.curprot == OZ_HW_PAGEPROT_NA) && (dpptpb.reqprot == OZ_HW_PAGEPROT_NA)) { newstate = NOACCESS; newstring = "no access"; }
    else { newstate = PTEVALID; newstring = "pte valid"; }
    if ((newstate != oldstate) || ((dpptpb.virtpage & 0xFFFF) == 0)) {			// see if different than last or it's been a while
      if ((oldstate != INIT) && (oldstate != PTEVALID)) {				// print out range of old stuff we skipped
        oz_sys_io_fs_printf (oz_util_h_output, "  %8.8X..%8.8X: %s\n", oldvpage, dpptpb.virtpage - 1, oldstring);
      }
      oldstate  = newstate;
      oldstring = newstring;
      oldvpage  = dpptpb.virtpage;
    }
    if (newstate == PTEVALID) {								// only detail out valid pte's

      /* Make string for pagestate */

      if (dpptpb.pagestate < sizeof pagestatetbl / sizeof pagestatetbl[0]) strcpy (pagestatestr, pagestatetbl[dpptpb.pagestate]);
      else oz_sys_sprintf (sizeof pagestatestr, pagestatestr, "%d", dpptpb.pagestate);

      /* Make strings for current and requested protections */

      if (dpptpb.curprot < sizeof prottbl / sizeof prottbl[0]) strcpy (curprotstr, prottbl[dpptpb.curprot]);
      else oz_sys_sprintf (sizeof curprotstr, curprotstr, "?%d", dpptpb.curprot);
      if (dpptpb.reqprot < sizeof prottbl / sizeof prottbl[0]) strcpy (reqprotstr, prottbl[dpptpb.reqprot]);
      else oz_sys_sprintf (sizeof reqprotstr, reqprotstr, "?%d", dpptpb.reqprot);

      /* Print out pte contents */

      oz_sys_io_fs_printf (oz_util_h_output, "  %8.8X: %-12s %8.8X %-4s %-4s", 
		dpptpb.virtpage, pagestatestr, dpptpb.phypage, curprotstr, reqprotstr);

      /* If there is one, print out corresponding physical page state */

      if ((dpptpb.pagestate == OZ_SECTION_PAGESTATE_VALID_R) 
       || (dpptpb.pagestate == OZ_SECTION_PAGESTATE_VALID_W) 
       || (dpptpb.pagestate == OZ_SECTION_PAGESTATE_VALID_D)) {
        switch (dpptpb.phypagestate.state) {
          case OZ_PHYMEM_PAGESTATE_FREE: {
            oz_sys_io_fs_printf (oz_util_h_output, "  FREE\n");
            break;
          }
          case OZ_PHYMEM_PAGESTATE_ALLOCSECT: {
            oz_sys_io_fs_printf (oz_util_h_output, "  ALLOCSECT  ptrefcount %d\n", dpptpb.phypagestate.u.s.ptrefcount);
            break;
          }
          case OZ_PHYMEM_PAGESTATE_ALLOCACHE: {
            oz_sys_io_fs_printf (oz_util_h_output, "  ALLOCACHE  refcount %d, lockcount %d\n", dpptpb.phypagestate.u.c.refcount, dpptpb.phypagestate.u.c.lockcount);
            break;
          }
          case OZ_PHYMEM_PAGESTATE_ALLOCPERM: {
            oz_sys_io_fs_printf (oz_util_h_output, "  ALLOCPERM\n");
            break;
          }
          default: {
            oz_sys_io_fs_printf (oz_util_h_output, "  ?%d\n", dpptpb.phypagestate.state);
            break;
          }
        }
      } else {
        oz_sys_io_fs_printf (oz_util_h_output, "\n");
      }
    }
  }
  if ((oldstate != INIT) && (oldstate != PTEVALID)) {
    oz_sys_io_fs_printf (oz_util_h_output, "  %8.8X..%8.8X: %s\n", oldvpage, dpptpb.virtpage - 1, oldstring);
  }
  return (OZ_SUCCESS);
}
