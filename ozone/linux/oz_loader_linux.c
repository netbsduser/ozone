//+++2004-01-03
//    Copyright (C) 2001,2002,2003,2004  Mike Rieker, Beverly, MA USA
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
//---2004-01-03

#define _OZ_DEV_TIMER_C

#include "ozone.h"
#include "oz_dev_timer.h"
#include "oz_knl_hw.h"
#include "oz_knl_status.h"
#include "oz_ldr_loader.h"
#include "oz_sys_dateconv.h"

#include <errno.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>

uByte oz_hw486_cpudb[4096];	// bigger than it'll ever be
const uLong oz_SUCCESS = OZ_SUCCESS;

static time_t timewarp = 0;

void *malloc (int size);

static time_t timer_check (void);

int main ()

{
  uLong kstacksize, param_size, systempages, tempmemsize;
  OZ_Loadparams loadparams;
  OZ_Mempage cachearea, cachesize, phypages, startphypage;
  void *sysbaseva, *tempmemaddr;

  memset (&loadparams, 0, sizeof loadparams);
  strcpy (loadparams.load_device, "fs_linux");
  strcpy (loadparams.load_script, "make_floppy.ldr");

  loadparams.kernel_stack_size = 16384;
  sysbaseva    = NULL;
  phypages     = 1024;
  tempmemsize  = 1024 * 1024;
  tempmemaddr  = malloc (tempmemsize);
  startphypage = 0x100000 >> OZ_HW_L2PAGESIZE;

  cachesize   = 1024 * 1024;			/* number of bytes desired */
  cachearea   = (uLong)(malloc (cachesize));	/* allocate memory */
  cachesize  += cachearea;			/* point to end of allocated area */
  cachearea  += (1 << OZ_HW_L2PAGESIZE) - 1;	/* round up to page boundary */
  cachearea >>= OZ_HW_L2PAGESIZE;		/* get starting page number */
  cachesize >>= OZ_HW_L2PAGESIZE;		/* get ending page number */
  cachesize  -= cachearea;			/* get number of pages */

  oz_ldr_start (&loadparams, sysbaseva, startphypage, phypages, tempmemsize, tempmemaddr, &kstacksize, &systempages, cachesize, cachearea, 0, 0);

  return (0);
}

static int softintpending = 0;
static uLong smplocklevel = OZ_SMPLOCK_SOFTINT;

typedef struct { uByte level;
                 Byte lockcount;
               } Smplock;

OZ_Mempage oz_hw_ldr_knlpage_basevp (OZ_Hw_pageprot pageprot, OZ_Mempage npagem, OZ_Mempage svpage)

{
  return (svpage);
}

/* Called when kernel pages are loaded                                                */
/* We use mmap to alloc memory at correct address                                     */
/* Although we don't actually want to run kernel, it is used for image loader testing */

void oz_hw_ldr_knlpage_maprw (OZ_Mempage npagem, OZ_Mempage svpage)

{
  void *mappedat, *wantedat;

  wantedat = OZ_HW_VPAGETOVADDR (svpage);
  mappedat = mmap (wantedat, npagem << OZ_HW_L2PAGESIZE, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_PRIVATE | MAP_ANON, 0, 0);
  if (mappedat == MAP_FAILED) {
    oz_crash ("oz_hw_ldr_knlpage_maprw: mmap of %u at %p failed: %s", npagem, wantedat, strerror (errno));
  }
  if (mappedat != wantedat) {
    oz_crash ("oz_hw_ldr_knlpage_maprw: mmap of %u at %p mapped at %p", npagem, wantedat, mappedat);
  }
}

void oz_hw_ldr_knlpage_setro (OZ_Mempage npagem, OZ_Mempage svpage)

{ }

/* Get the current cpu id.  Since there is only one running, just call it zero. */

Long oz_hw_cpu_getcur (void)

{
  return (0);
}

/* Called by interrupt routines to break oz_hw_cpu_waitint out of wait. */
/* We set a flag saying we were called.  oz_hw_cpu_waitint just skips through when it sees this flag. */

void oz_hw_cpu_lowiplint (Long cpuidx)

{
  softintpending = 1;
}

void oz_hw_cpu_reschedint (Long cpuidx)

{
  softintpending = 1;
}

/* Called when there is nothing to do.  In this version, we just exit */
/* and let whoever check again, since we have nothing better to do.   */

void oz_hw_cpu_waitint (void *waitq)

{
  time_t nsecs;

  if (smplocklevel != OZ_SMPLOCK_SOFTINT) oz_crash ("oz_hw_cpu_waitint: attempting to wait at smplocklevel %u", smplocklevel);
  while ((nsecs = timer_check ()) > 0) {
    softintpending = 1;
//    oz_knl_printk ("oz_hw_cpu_waitint: pretending to sleep for %d\n", nsecs);
    timewarp += nsecs;					// don't actually wait, just pretend it has elapsed
							// we get these when the cache fills so just flush immediately
//    oz_knl_printk ("oz_hw_cpu_waitint: awake\n");
  }
  oz_hw_cpu_setsoftint (1);
  if (!softintpending) {
//    oz_knl_printk ("oz_hw_cpu_waitint: sleeping\n");
//    oz_knl_printk ("oz_hw_cpu_waitint: awake\n");
  }
  else softintpending = 0;
  oz_hw_cpu_setsoftint (0);
}

/* This enables or disables softint's from being delivered */
/* In this version, if we are enabling them, and there are */
/* no smplocks, call the softint processing routines       */

int oz_hw_cpu_setsoftint (int enb)

{
  int oldenable;

  oldenable = (smplocklevel == OZ_SMPLOCK_NULL);
  if ((smplocklevel != OZ_SMPLOCK_NULL) && (smplocklevel != OZ_SMPLOCK_SOFTINT)) oz_crash ("oz_hw_cpu_setsoftint: called at smplocklevel %u", smplocklevel);
  smplocklevel = enb ? OZ_SMPLOCK_NULL : OZ_SMPLOCK_SOFTINT;
  if (smplocklevel == OZ_SMPLOCK_NULL) {
    smplocklevel = OZ_SMPLOCK_SOFTINT;
    oz_knl_lowipl_handleint ();
    if (smplocklevel != OZ_SMPLOCK_SOFTINT) oz_crash ("oz_hw_cpu_setsoftint: one of the handleints returned at level %u", smplocklevel);
    smplocklevel = OZ_SMPLOCK_NULL;
  }

  return (oldenable);
}

/* Initialize an smp lock structure, give it level 'level' */

void oz_hw_smplock_init (int smplocksize, OZ_Smplock *smplock, uLong level)

{
  Smplock *sl;

  if (sizeof *sl > smplocksize) oz_crash ("oz_loader_linux oz_hw_smplock_init: sizeof (Smplock) %d > smplocksize %d", sizeof *sl, smplocksize);
  if (level >= 256) oz_crash ("oz_loader_linux oz_hw_smplock_init: level %u >= 256", level);

  sl = ((Smplock *)smplock);
  sl -> level = level;
  sl -> lockcount = 0;
}

/* Set an smp lock, wait if someone else has it.  Since there is */
/* only one cpu here, it should always be immediately granted.   */

uLong oz_hw_smplock_wait (OZ_Smplock *smplock)

{
  uLong oldlevel;
  Smplock *sl;

  sl = ((Smplock *)smplock);

  oldlevel = smplocklevel;
  if (oldlevel > sl -> level) oz_crash ("oz_hw_smplock_wait: old smp lock level %u too great for %u", oldlevel, sl -> level);
  if (sl -> lockcount > 0 && oldlevel < sl -> level) oz_crash ("oz_hw_smplock_wait: old smp lock level %u too small for %u", oldlevel, sl -> level);
  smplocklevel = sl -> level;
  sl -> lockcount ++;

  return (oldlevel);
}

/* Clear the smp lock, return to previous level                                          */
/* If previous level is zero and soft ints enabled, call the softint processing routines */

void oz_hw_smplock_clr (OZ_Smplock *smplock, uLong old)

{
  Smplock *sl;

  sl = ((Smplock *)smplock);

  sl -> lockcount --;
  if (sl -> lockcount < 0) oz_crash ("oz_hw_smplock_clr: lock %u count went negative (%d)", sl -> level, sl -> lockcount);
  if (old > sl -> level) oz_crash ("oz_hw_smplock_clr: old level %u too great for %u", old, sl -> level);
  if ((sl -> lockcount != 0) && (old < sl -> level)) oz_crash ("oz_hw_smplock_clr: old level %u too small for %u", old, sl -> level);
  smplocklevel = old;
  if (smplocklevel == OZ_SMPLOCK_NULL) oz_hw_cpu_setsoftint (1);
}

uLong oz_hw_cpu_smplevel (void)

{
  return (smplocklevel);
}

/* Should never really do pte stuff - these are just stubs that crash */

void oz_hw_pte_writecur (OZ_Mempage vpn,
                         OZ_Section_pagestate pagestate,
                         OZ_Mempage phypage,
                         OZ_Hw_pageprot curprot,
                         OZ_Hw_pageprot reqprot)

{
  oz_crash ("oz_hw_pte_writecur: cannot be called in loader");
}

void oz_hw_pte_writeall (OZ_Mempage vpn,
                         OZ_Section_pagestate pagestate,
                         OZ_Mempage phypage,
                         OZ_Hw_pageprot curprot,
                         OZ_Hw_pageprot reqprot)

{
  oz_crash ("oz_hw_pte_writeall: cannot be called in loader");
}

/* We can read a system pte, though - just supply a write-enabled identity mapping */

void oz_hw_pte_readsys (OZ_Mempage vpn,
                        OZ_Section_pagestate *pagestate_r,
                        OZ_Mempage *phypage_r,
                        OZ_Hw_pageprot *curprot_r,
                        OZ_Hw_pageprot *reqprot_r)

{
  if (pagestate_r != NULL) *pagestate_r = OZ_SECTION_PAGESTATE_VALID_W;
  if (phypage_r   != NULL) *phypage_r   = vpn;
  if (curprot_r   != NULL) *curprot_r   = OZ_HW_PAGEPROT_KW;
  if (reqprot_r   != NULL) *reqprot_r   = OZ_HW_PAGEPROT_KW;
}

/* Probe buffer addressibility - these assume that if the pointer is not null, the buffer is addressible. */

int oz_hw_probe (uLong size, const void *buff, OZ_Procmode procmode, int write)

{
  return (buff != NULL);
}

int oz_hw_readable_strz (void *buff, OZ_Procmode procmode)

{
  return (buff != NULL);
}

#if 000
/* Write message to console */

void oz_hw_putcon (uLong size, const char *buff)

{
  fwrite (buff, size, 1, stdout);
}

/* Read line from console */

int oz_hw_getcon (uLong size, char *buff, uLong pmtsize, const char *pmtbuff)

{
  int c, i;

  fwrite (pmtbuff, pmtsize, 1, stdout);
  for (i = 0; i < size - 1; i ++) {
    c = fgetc (stdin);
    if (c == EOF) {
      fputc ('\n', stdout);
      return (0);
    }
    if (c == '\n') break;
    buff[i] = c;
  }
  buff[i] = 0;
  return (1);
}
#endif

/* Crash */

void oz_hw_crash (void)

{
  linux_dev_ttyport_shut ();
  exit (0);
}

/* Convert integer to asciz string */

void oz_hw_itoa (uLong valu, uLong size, char *buff)

{
  char temp[3*sizeof valu];
  int i;

  i = sizeof temp;
  temp[--i] = 0;
  do {
    temp[--i] = (valu % 10) + '0';
    valu /= 10;
  } while (valu != 0);
  strncpy (buff, temp + i, size);
}

/* Convert integer to hexadecimal string */

void oz_hw_ztoa (uLong valu, uLong size, char *buff)

{
  char temp[3*sizeof valu];
  int i;

  i = sizeof temp;
  temp[--i] = 0;
  do {
    temp[--i] = (valu % 16) + '0';
    if (temp[i] > '9') temp[i] += 'A' - '9' - 1;
    valu /= 16;
  } while (valu != 0);
  strncpyz (buff, temp + i, size);
}

/* Get current utc */

OZ_Datebin oz_hw_tod_getnow (void)

{
  uLong basedaynumber;
  uLong nowlongs[OZ_DATELONG_ELEMENTS];
  OZ_Datebin nowbin;
  struct timeval nowtime;

  gettimeofday (&nowtime, NULL);
  nowtime.tv_sec += timewarp;
  memset (nowlongs, 0, sizeof nowlongs);
  basedaynumber = oz_sys_daynumber_encode ((1970 << 16) | (1 << 8) | 1);
  nowlongs[OZ_DATELONG_DAYNUMBER] = basedaynumber + nowtime.tv_sec / 86400;
  nowlongs[OZ_DATELONG_SECOND]    = nowtime.tv_sec % 86400;
#if OZ_TIMER_RESOLUTION < 1000000
  error : code below assumes OZ TIMER RESOLUTION >= 1,000,000
#endif
#if (OZ_TIMER_RESOLUTION % 1000000) != 0
  error : code below assumes OZ TIMER RESOLUTION is multiple of 1,000,000
#endif
  nowlongs[OZ_DATELONG_FRACTION]  = nowtime.tv_usec * (OZ_TIMER_RESOLUTION / 1000000);
  nowbin = oz_sys_datebin_encode (nowlongs);
  return (nowbin);
}

/* Convert string to integer */

uLong oz_hw_atoi (const char *s, int *usedup)

{
  char c;
  const char *p;
  uLong accum;

  accum = 0;
  for (p = s; (c = *p) != 0; p ++) {
    if (c < '0') break;
    if (c > '9') break;
    accum = accum * 10 + c - '0';
  }

  if (usedup != NULL) *usedup = p - s;
  return (accum);
}

/* Convert string to hexadecimal integer */

uLong oz_hw_atoz (const char *s, int *usedup)

{
  char c;
  const char *p;
  uLong accum;

  accum = 0;
  for (p = s; (c = *p) != 0; p ++) {
    if ((c >= 'A') && (c <= 'F')) c -= 'A' - 10;
    else if ((c >= 'a') && (c <= 'f')) c -= 'a' - 10;
    else if ((c < '0') || (c > '9')) break;
    else c -= '0';
    accum = accum * 16 + c;
  }

  if (usedup != NULL) *usedup = p - s;
  return (accum);
}

/************************************************************************/
/*									*/
/*  Timer routines							*/
/*									*/
/************************************************************************/

struct OZ_Timer { OZ_Timer *next;
                  time_t when;
                  void (*entry) (void *param, OZ_Timer *timer);
                  void *param;
                };

static OZ_Timer *timers = NULL;

OZ_Timer *oz_knl_timer_alloc (void)

{
  return (malloc (sizeof (OZ_Timer)));
}

void oz_knl_timer_insert (OZ_Timer *timer, OZ_Datebin datebin, void (*entry) (void *param, OZ_Timer *timer), void *param)

{
  OZ_Datebin delta, now;
  uLong datelongs[OZ_DATELONG_ELEMENTS], secs;

  now = oz_hw_tod_getnow ();				/* get current date/time */
  OZ_HW_DATEBIN_SUB (delta, datebin, now);		/* get delta time to wait */
  oz_sys_datebin_decode (delta, datelongs);		/* see how many seconds that is */
  secs = datelongs[OZ_DATELONG_DAYNUMBER] * 86400 + datelongs[OZ_DATELONG_SECOND];
  timer -> when  = time (NULL) + timewarp + secs;	/* save when it expires */
  timer -> entry = entry;				/* save what to call */
  timer -> param = param;				/* save what to pass it */
  timer -> next  = timers;				/* link to list of timers */
  timers = timer;
}

int oz_knl_timer_remove (OZ_Timer *timer)

{
  OZ_Timer **ltimer, *ntimer;

  for (ltimer = &timers; (ntimer = *ltimer) != timer; ltimer = &(ntimer -> next)) {
    if (ntimer == NULL) return (0);
  }
  *ltimer = ntimer -> next;
  return (1);
}

void oz_knl_timer_free (OZ_Timer *timer)

{
  oz_knl_timer_remove (timer);
  free (timer);
}

static time_t timer_check (void)

{
  OZ_Timer **ltimer, *ntimer;
  time_t now, soonest;

  soonest = 1000000000;
  now = time (NULL) + timewarp;					/* see what time it is now */
scan:
  for (ltimer = &timers; (ntimer = *ltimer) != NULL;) {		/* loop through all waiting timers */
    if (ntimer -> when <= now) {				/* see if its time is up */
      *ltimer = ntimer -> next;					/* if so, unlink from list */
      (*(ntimer -> entry)) (ntimer -> param, ntimer);		/* call the routine */
      soonest = 0;						/* remember we did something */
      goto scan;						/* routine may have re-queued itself or altered the list in some way */
    }
    ltimer = &(ntimer -> next);					/* otherwise, just go on to next entry */
    if (ntimer -> when - now < soonest) soonest = ntimer -> when - now;
  }

  if (timers == NULL) return (0);
  return (soonest);
}

/************************************************************************/
/*									*/
/*  Misc routines							*/
/*									*/
/************************************************************************/

void oz_hw_print_pte (void *vaddr)

{
  oz_knl_printk ("oz_hw_print_pte: vaddr %p\n", vaddr);
}

void oz_hw_debug_watch (void *vaddr)

{ }

void *oz_hw_getrtnadr (int level)

{
  return (NULL);
}

void oz_knl_timer_validate (void)

{ }

int oz_hw_inknlmode (void)

{
  return (1);
}

void *oz_hw_phys_mappage (OZ_Mempage physpage, OZ_Pagentry *oldpte)

{
  OZ_Pointer vaddr;

  vaddr = physpage << 12;
  return ((void *)vaddr);
}

void oz_hw_phys_unmappage (OZ_Pagentry oldpte)

{ }

void oz_hw_phys_movefromvirt (uLong nbytes, const void *vaddr, const OZ_Mempage *phypages, uLong byteoffs)

{
  memcpy ((char *)(phypages[0] << 12) + byteoffs, vaddr, nbytes);
}

void oz_hw_phys_movetovirt (uLong nbytes, void *vaddr, const OZ_Mempage *phypages, uLong byteoffs)

{
  memcpy (vaddr, (char *)(phypages[0] << 12) + byteoffs, nbytes);
}

void oz_hw_phys_movephys (uLong nbytes, const OZ_Mempage *src_pages, uLong src_offs, const OZ_Mempage *dst_pages, uLong dst_offs)

{
  char *dstaddr;
  const char *srcaddr;

  srcaddr = (char *)(src_pages[0] << 12) + src_offs;
  dstaddr = (char *)(dst_pages[0] << 12) + dst_offs;

  memcpy (dstaddr, srcaddr, nbytes);
}

void oz_dev_timer_init (void)

{ }

void oz_hw_diag (void)

{ }

void oz_hw486_chkhwien (void)

{ }

void oz_dev_vgavideo_blank (int blank)

{ }

int oz_hw_bootscan (volatile int *abortflag, OZ_Event *abortevent, int verbose)

{
  oz_knl_printk ("oz_hw_bootscan not supported in linux, foo\n");
}
