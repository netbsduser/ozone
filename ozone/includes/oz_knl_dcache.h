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

#ifndef _OZ_KNL_DCACHE_H
#define _OZ_KNL_DCACHE_H

#include "ozone.h"
#include "oz_knl_cache.h"
#include "oz_knl_devio.h"
#include "oz_knl_hw.h"
#include "oz_knl_lock.h"

#ifdef _OZ_KNL_DCACHE_C
typedef struct OZ_Dcache OZ_Dcache;
#else
typedef void OZ_Dcache;
#endif

typedef struct OZ_Dcmpb OZ_Dcmpb;

struct OZ_Dcmpb { OZ_Dcache *dcache;			// disk cache
                  int writing;				// 0=cache pages will not be modified; 1=cache pages may be modified
                  OZ_Dbn virtblock;			// corresponding virtual block number
                  uLong nbytes;				// number of bytes available in the cache page
                  OZ_Dbn logblock;			// corresponding logical block number
                  uLong blockoffs;			// byte offset in the virtual/logical block being mapped
                  OZ_Mempage phypage;			// physical page number of the cache page
                  uLong pageoffs;			// byte offset in the physical page for the byte at logblock.blockoffset
                  uLong (*entry) (OZ_Dcmpb *dcmpb, uLong status); // completion routine entrypoint
                  void *param;				// completion routine parameter
                  int writethru;			// if set, data is written thru to magnetic media immediately
                  int ix4kbuk;				// if set, cache page contains an IX database 4k-size bucket
							// the following are for oz_knl_dcache_map internal use:
                  OZ_Cachepage *cachepage;		// - cache page the request was found in
                  void *cachepagex;			// - cache page extension
                  OZ_Lockmode cachepagelm;		// - cache page lock mode
                };

OZ_Dcache *oz_knl_dcache_init (OZ_Iochan *iochan, uLong blocksize, uLong (*reval_entry) (void *reval_param, OZ_Dcache *dcache), void *reval_param);
void oz_knl_dcache_term (OZ_Dcache *dcache, int abort);
uLong oz_knl_dcache_readw (OZ_Dcache *dcache, uLong size, void *buff, OZ_Dbn slbn, uLong offs);
uLong oz_knl_dcache_writew (OZ_Dcache *dcache, uLong size, const void *buff, OZ_Dbn slbn, uLong offs, int writethru);
uLong oz_knl_dcache_map (OZ_Dcmpb *dcmpb);
uLong oz_knl_dcache_prefetch (OZ_Dcache *dcache, OZ_Dbn logblock, int ix4kbuk);
uLong oz_knl_dcache_pfmap (OZ_Dcache *dcache, OZ_Dbn logblock, OZ_Mempage *phypage_r);
uLong oz_knl_dcache_pfupd (OZ_Dcache *dcache, OZ_Dbn logblock, OZ_Mempage phypage, int writethru);
void oz_knl_dcache_pfrel (OZ_Dcache *dcache, OZ_Mempage phypage);
void oz_knl_dcache_stats (OZ_Dcache *dcache, uLong *nincache_r, uLong *ndirties_r, OZ_Datebin *dirty_interval_r, uLong *avgwriterate_r);

#endif
