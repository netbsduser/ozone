//+++2003-11-18
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
//---2003-11-18

#ifndef _OZ_KNL_SECTION_H
#define _OZ_KNL_SECTION_H

#define OZ_SECTION_TYPE_SHRWRT 0x0001	/* SET: writes modify common shared page, CLR: writes modify private copy of page */
#define OZ_SECTION_TYPE_SECFIL 0x0002	/* SET: write modifications to section file, CLR: write modifications to page file */
#define OZ_SECTION_TYPE_GLOBAL 0x0004	/* SET: multiple mappings share common pages, CLR: multiple mappings are independent */
#define OZ_SECTION_TYPE_EXPDWN 0x0008	/* SET: expansion goes to lower addresses, CLR: expansion goes to higher addresses */
#define OZ_SECTION_TYPE_ZEROES 0x0010	/* SET: initial content is zeroes, CLR: initial content comes from section file */
#define OZ_SECTION_TYPE_PAGTBL 0x0020	/* SET: section holds hardware page table entries, CLR: section contains no hw pte's */

typedef uLong OZ_Section_type;

typedef enum { 

	/* The following states must cause a pagefault when accessing the page */
	/* They use the physical address bits to store additional information  */

	OZ_SECTION_PAGESTATE_PAGEDOUT = 0, 	/* contents are in file */
						/* pte.phypage = 0 : contents are in section file */
						/*            else : non-zero page file page number where contents are saved */
						/* use a zero, so that a zeroed page of pagetable entries results in paged-out state */
	OZ_SECTION_PAGESTATE_READINPROG, 	/* contents are being read from section file or page file */
						/* pte.phypage = physical page number being read into */
	OZ_SECTION_PAGESTATE_READFAILED, 	/* error reading from section file or page file */
						/* pte.phypage = I/O error status */
	OZ_SECTION_PAGESTATE_WRITEFAILED, 	/* error writing to section file or page file */
						/* pte.phypage = I/O error status */

	/* The following state can have the cpu accessing the page without any problem - */
	/* If the hardware cannot store the bits anywhere with the page being valid,     */
        /* then make it invalid and save the state somewhere                             */

	OZ_SECTION_PAGESTATE_WRITEINPROG, 	/* contents in physical memory, */
						/* but are currently being written to section file or page file */
						/* pte.phypage = physical page number */
						/* oz_s_phymem_page[pte.phypage].pfpage = page number in the page file (0 for section file) */

	/* The following states must allow the cpu to access the page */

	OZ_SECTION_PAGESTATE_VALID_R, 		/* contents are in physical memory, and match what's in file */
						/* pte.phypage = physical page number */
						/* oz_s_phymem_page[pte.phypage].pfpage = block number in the page file (0 for section file) */
						/* this state will never be given with any write enabled protection code, */
						/* ... thus making the page read-only */
						/* this state is initially given to a read/write page, then when someone tries to write  */
						/* ... to the page, its state is changed to VALID_W, so we know when the page is dirtied */

	OZ_SECTION_PAGESTATE_VALID_W, 		/* contents are in physical memory, but have been modified, */
						/* and thus need to be written back to section file or page file */
						/* pte.phypage = physical page number */
						/* oz_s_phymem_page[pte.phypage].pfpage = block number in the page file (0 for section file) */
						/* this state will always be given with some write enabled protection code */

	OZ_SECTION_PAGESTATE_VALID_D		/* contents are in physical memory, but have been modified, */
						/* and thus need to be written back to section file or page file */
						/* pte.phypage = physical page number */
						/* oz_s_phymem_page[pte.phypage].pfpage = block number in the page file (0 for section file) */
						/* this state will never be given with any write enabled protection code, */
						/* ... thus making the page read-only */
						/* this state is given to a page that was VALID_W but needs to be read-only */
						/* ... like when a page is being copied for a fork */


             } OZ_Section_pagestate;

/* Section expansion direction */

typedef enum { OZ_SECTION_EXPUP, 
               OZ_SECTION_EXPDN
             } OZ_Section_expand;

/* Dummy section structure definitions */

#ifdef _OZ_KNL_SECTION_C
typedef struct OZ_Section OZ_Section;
typedef struct OZ_Seclock OZ_Seclock;
typedef struct OZ_Seclkwz OZ_Seclkwz;
#else
typedef void OZ_Section;
typedef void OZ_Seclock;
typedef void OZ_Seclkwz;
#endif

/* Includes needed for routine prototypes */

#include "oz_knl_devio.h"
#include "oz_knl_hw.h"
#include "oz_io_fs.h"
#include "oz_knl_process.h"
#include "oz_knl_quota.h"
#include "oz_knl_security.h"

/* IEEE dma elements */

typedef struct { uLong phyaddr;	// physical address for segment
                 uLong bytecnt;	// number of bytes for segment
				// - if top bit set, this element 
				//   is a link to another array
               } OZ_Ieeedma32;

typedef struct { uQuad phyaddr;	// physical address for segment
                 uQuad bytecnt;	// number of bytes for segment
				// - if top bit set, this element 
				//   is a link to another array
               } OZ_Ieeedma64;

/* Routine prototypes */

void oz_knl_section_init ();
uLong oz_knl_section_create (OZ_Iochan *file, 
                             OZ_Mempage npages, 
                             OZ_Dbn rbnstart, 
                             OZ_Section_type sectype, 
                             OZ_Secattr *secattr, 
                             OZ_Section **section_r);
Long oz_knl_section_increfc (OZ_Section *section, Long inc);
uLong oz_knl_section_copypages (OZ_Process *newprocess, OZ_Mempage npages, OZ_Mempage vpage, OZ_Section **section_r, 
                                OZ_Procmode ownermode, OZ_Hw_pageprot mapprot, uLong mapsecflags);
OZ_Mempage oz_knl_section_getsecnpages (OZ_Section *section);
OZ_Section_type oz_knl_section_getsectype (OZ_Section *section);
OZ_Secattr *oz_knl_section_getsecattr (OZ_Section *section);
OZ_Quota *oz_knl_section_getquota (OZ_Section *section);
void oz_knl_section_expand (OZ_Section *section, OZ_Mempage npages);
uLong oz_knl_section_mapproc (OZ_Section *section, OZ_Process *process);
void oz_knl_section_unmapproc (OZ_Section *section, OZ_Process *process);
uLong oz_knl_section_uget (OZ_Procmode procmode, uLong size, const void *usrc, void *kdst);
uLong oz_knl_section_ugetz (OZ_Procmode procmode, uLong size, const void *usrc, void *kdst, uLong *len_r);
uLong oz_knl_section_uput (OZ_Procmode procmode, uLong size, const void *ksrc, void *udst);
#define oz_knl_section_iolockr(__procmode,__size,__buff,__seclock_r,__npages_r,__phypages_r,__byteoffs_r) oz_knl_section_iolock(__procmode,__size,__buff,0,__seclock_r,__npages_r,__phypages_r,__byteoffs_r)
#define oz_knl_section_iolockw(__procmode,__size,__buff,__seclock_r,__npages_r,__phypages_r,__byteoffs_r) oz_knl_section_iolock(__procmode,__size,__buff,1,__seclock_r,__npages_r,__phypages_r,__byteoffs_r)
uLong oz_knl_section_iolockz (OZ_Procmode procmode, uLong size, const void *buff, uLong *rlen, OZ_Seclock **seclock_r, OZ_Mempage *npages_r, const OZ_Mempage **phypages_r, uLong *byteoffs_r);
uLong oz_knl_section_iolock_ieeedma32 (OZ_Procmode procmode, uLong size, const void *buff, int writing, OZ_Seclock **seclock_r, 
                                       OZ_Mempage *nelements_r, const OZ_Ieeedma32 **ieeedma32s_va_r, OZ_Phyaddr *ieeedma32s_pa_r);
uLong oz_knl_section_iolock_ieeedma64 (OZ_Procmode procmode, uLong size, const void *buff, int writing, OZ_Seclock **seclock_r, 
                                       OZ_Mempage *nelements_r, const OZ_Ieeedma64 **ieeedma64s_va_r, OZ_Phyaddr *ieeedma64s_pa_r);
uLong oz_knl_section_iolock  (OZ_Procmode procmode, uLong size, const void *buff, int writing, OZ_Seclock **seclock_r, 
                              OZ_Mempage *npages_r, const OZ_Mempage **phypages_r, uLong *byteoffs_r);
void oz_knl_section_iounlk (OZ_Seclock *seclock);
uLong oz_knl_section_getpageprot (OZ_Mempage vpage, OZ_Hw_pageprot *pageprot_r);
uLong oz_knl_section_setpageprot (OZ_Mempage npages, OZ_Mempage svpage, OZ_Hw_pageprot pageprot, OZ_Section *initsec, OZ_Hw_pageprot *pageprot_r);
uLong oz_knl_section_faultpage (OZ_Procmode procmode, OZ_Mempage vpage, int writing);
int oz_knl_section_unmappage (OZ_Mempage vpage, uLong ptsave);

#define oz_knl_section_blockr(__procmode,__size,__buff,__seclock_r) oz_knl_section_iolockr(__procmode,__size,__buff,__seclock_r,NULL,NULL,NULL)
#define oz_knl_section_blockw(__procmode,__size,__buff,__seclock_r) oz_knl_section_iolockw(__procmode,__size,__buff,__seclock_r,NULL,NULL,NULL)
#define oz_knl_section_blockz(__procmode,__size,__buff,__rlen,__seclock_r) oz_knl_section_iolockz(__procmode,__size,__buff,__rlen,__seclock_r,NULL,NULL,NULL)
#define oz_knl_section_block(__procmode,__size,__buff,__writing,__seclock_r) oz_knl_section_iolock(__procmode,__size,__buff,__writing,__seclock_r,NULL,NULL,NULL)
#define oz_knl_section_bunlock(__seclock_r) oz_knl_section_iounlk(__seclock_r)

#endif
