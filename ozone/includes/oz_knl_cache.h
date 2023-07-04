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

#ifndef _OZ_KNL_CACHE_H
#define _OZ_KNL_CACHE_H

#include "ozone.h"
#include "oz_knl_hw.h"

#define OZ_KNL_CACHE_CHECKSUM 1 // 0

#define OZ_KNL_CACHE_PAGESIZE (1 << OZ_HW_L2PAGESIZE)

#define OZ_KNL_CACHE_EXSIZE ((sizeof (void *)) * 2 + sizeof (OZ_Mempage) + sizeof (Long))
				// oz_dev_ipdns requires '2'
				// oz_knl_dcache requires '16'

#ifdef _OZ_KNL_CACHE_C
typedef struct OZ_Cache OZ_Cache;
#else
typedef void OZ_Cache;
#endif

typedef struct OZ_Cachepage OZ_Cachepage;
typedef uLong OZ_Cachekey;

struct OZ_Cachepage { OZ_Cachepage *next_hash, **prev_hash;	/* next/prev in the cache -> hash_table entry */
                      OZ_Cachepage *next_old, **prev_old;	/* next/prev in the oldpages list */
                      OZ_Cachekey key;				/* starting lbn at beginning of page */
                      OZ_Cache *cache;				/* cache it belongs to */
                      Long refcount;				/* count of number of threads currently referencing page */
                      Long lockcount;				/* -1: locked for read/write access by a thread */
								/*  0: not being accessed at all */
								/* >0: this many threads have it accessed read-only */
#if OZ_KNL_CACHE_CHECKSUM
                      uLong checksum;				// page checksum (valid only when refcount==0)
                      Long usecount;				// number of times checked out
                      OZ_Datebin loaded;			// when it was originally created
#endif
                      uByte pagex[OZ_KNL_CACHE_EXSIZE];		/* caller's per-page data area */
                    };

extern OZ_Mempage oz_knl_cache_pagecount;

OZ_Cache *oz_knl_cache_init (const char *name, uLong exsize, int (*memfullentry) (void *memfullparam), void *memfullparam);
void oz_knl_cache_term (OZ_Cache *cache);
OZ_Cachepage *oz_knl_cache_find (OZ_Cache *cache, OZ_Cachekey key, OZ_Lockmode lockmode, void **pagex_r, OZ_Mempage *phypage_r);
OZ_Cachepage *oz_knl_cache_find_async (OZ_Cache *cache, OZ_Cachekey key, OZ_Lockmode lockmode, void **pagex_r, OZ_Mempage *phypage_r, void (*entry) (void *param, OZ_Cache *cache, OZ_Cachepage *cachepage), void *param);
void *oz_knl_cache_pagex (OZ_Cachepage *page);
OZ_Mempage oz_knl_cache_phypage (OZ_Cachepage *page);
OZ_Cachekey oz_knl_cache_key (OZ_Cachepage *page);
void oz_knl_cachepage_increfcby1 (OZ_Mempage phypage);
void oz_knl_cache_conv (OZ_Cache *cache, OZ_Cachepage *page, OZ_Lockmode oldmode, OZ_Lockmode newmode);
int oz_knl_cache_conv_async (OZ_Cache *cache, OZ_Cachepage *page, OZ_Lockmode oldmode, OZ_Lockmode newmode, void (*entry) (void *param, OZ_Cache *cache, OZ_Cachepage *cachepage), void *param);
void oz_knl_cache_done (OZ_Cache *cache, OZ_Cachepage *page, OZ_Lockmode lockmode);
OZ_Mempage oz_knl_cache_freepage (OZ_Mempage l2);
void oz_knl_cache_stats (OZ_Cache *cache, uLong *nincache_r);
const  char *oz_knl_cache_getname (OZ_Cache *cache);
uLong oz_knl_cache_smplock_wait (OZ_Mempage phypage);
void oz_knl_cache_smplock_clr (OZ_Mempage phypage, uLong level);

#endif
