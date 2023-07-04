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

#ifndef _OZ_KNL_HW_H
#define _OZ_KNL_HW_H

#include "ozone.h"

#include "oz_knl_procmode.h"

#include "oz_knl_hw_dep.h"

/* Other includes needed for routine prototypes */

#include "oz_knl_process.h"
#include "oz_knl_section.h"
#include "oz_knl_thread.h"
#include "oz_sys_debug.h"

/************************************************************************/
/*									*/
/*  Output a string to the console terminal				*/
/*									*/
/*    Input:								*/
/*									*/
/*	size = number of characters to output				*/
/*	buff = characters to output					*/
/*									*/
/*	ipl = possibly quite high					*/
/*									*/
/************************************************************************/

void oz_hw_putcon (uLong size, const char *buff);

/************************************************************************/
/*									*/
/*  Read a string from the console terminal				*/
/*									*/
/*    Input:								*/
/*									*/
/*	size = size of the buff						*/
/*	buff = where to return null-terminated string			*/
/*	pmtsize = prompt string size					*/
/*	pmtbuff = prompt string address					*/
/*									*/
/*	ipl = possibly quite high					*/
/*									*/
/*    Output:								*/
/*									*/
/*	oz_hw_getcon = 0 : end-of-file char on input			*/
/*	               1 : no eof char on input				*/
/*	*buff = filled in with null-terminated response string		*/
/*									*/
/************************************************************************/

int oz_hw_getcon (uLong size, char *buff, uLong pmtsize, const char *pmtbuff);

/************************************************************************/
/*									*/
/*  Boot all the alternate cpu's					*/
/*									*/
/*  The cpus then call oz_knl_boot_anothercpu () with softint delivery 	*/
/*  inhibited when they are ready to process threads.  The 		*/
/*  oz_knl_boot_anothercpu routine never returns.			*/
/*									*/
/************************************************************************/

void oz_hw_cpu_bootalts (Long cpucount);

/************************************************************************/
/*									*/
/*  Get current cpu index						*/
/*									*/
/*    Output:								*/
/*									*/
/*	oz_hw_cpu_getcur = cpu index in range 0..oz_s_numcpus-1		*/
/*									*/
/*    Note:								*/
/*									*/
/*	This routine should only be called at OZ_SMPLOCK_SOFTINT or 	*/
/*	above, or although it returns the current cpu index when it 	*/
/*	ran, the cpu could be switched by the time it returns and the 	*/
/*	cpu index is actually used for something.  Therefore, this 	*/
/*	routine should crash if the current cpu is at OZ_SMPLOCK_NULL.	*/
/*									*/
/************************************************************************/

Long oz_hw_cpu_getcur (void);

/************************************************************************/
/*									*/
/*  Generate software interrupt on either this or another cpu		*/
/*									*/
/*    Input:								*/
/*									*/
/*	cpuidx = cpu index of cpu to be interrupted			*/
/*									*/
/*    Output:								*/
/*									*/
/*	Causes the target cpu to do the following:			*/
/*									*/
/*	  1) inhibit software interrupt delivery			*/
/*	  2) call oz_knl_lowipl_handleint				*/
/*	  3) enable software interrupt delivery				*/
/*	  4) call oz_knl_thread_handleint				*/
/*	  5) break out of oz_hw_waitint call if in it			*/
/*									*/
/************************************************************************/

void oz_hw_cpu_lowiplint (Long cpuidx);
void oz_hw_cpu_reschedint (Long cpuidx);

/************************************************************************/
/*									*/
/*  Same as above except it picks the lowest priority cpu		*/
/*									*/
/*    Output:								*/
/*									*/
/*	oz_hw_cpu_softintlowest = cpu index that got wacked		*/
/*									*/
/************************************************************************/

Long oz_hw_cpu_softintlowest (void);

/************************************************************************/
/*									*/
/*  Set the execution priority of the current cpu's thread		*/
/*									*/
/************************************************************************/

void oz_hw_cpu_sethreadprio (uLong priority);

/************************************************************************/
/*									*/
/*  Wait for oz_hw_cpu_softint call by a cpu				*/
/*									*/
/*    Input:								*/
/*									*/
/*	waitq = pointer to wait queue					*/
/*	the routine can keep waiting as long as this pointer is null	*/
/*	otherwise, it should return					*/
/*									*/
/*	ipl = softint delivery inhibited				*/
/*									*/
/*    Output:								*/
/*									*/
/*	returns when this or another cpu has done an oz_hw_cpu_softint 	*/
/*	for this cpu							*/
/*									*/
/************************************************************************/

void oz_hw_cpu_waitint (void *waitq);

/************************************************************************/
/*									*/
/*  Set software interrupt delivery enable flag				*/
/*									*/
/*    Input:								*/
/*									*/
/*	enb = 0 : inhibit software interrupt delivery			*/
/*	      1 : enable software interrupt delivery			*/
/*									*/
/*    Output:								*/
/*									*/
/*	oz_hw_cpu_setsoftint = 0 : delivery was previously inhibited	*/
/*	                       1 : delivery was previously enabled	*/
/*									*/
/*    Note:								*/
/*									*/
/*	This routine crashes with enb=1 if any smp locks are held	*/
/*									*/
/************************************************************************/

int oz_hw_cpu_setsoftint (int enb);

/************************************************************************/
/*									*/
/*  Set hardware interrupt delivery					*/
/*									*/
/************************************************************************/

int oz_hw_cpu_sethwints (int enb);

/************************************************************************/
/*									*/
/*  Start quantum timer for thread on current cpu			*/
/*									*/
/*    Input:								*/
/*									*/
/*	quantum = delta time from now to call oz_knl_thread_quantimex	*/
/*	iotanow = the current iota time					*/
/*									*/
/************************************************************************/

void oz_hw_quantimer_start (OZ_Iotatime quantum, OZ_Iotatime iotanow);

/************************************************************************/
/*									*/
/*  Initialize an smp lock to be owned by no cpu			*/
/*									*/
/*    Input:								*/
/*									*/
/*	smplocksize = size of smplock struct				*/
/*	smplock = pointer to smp lock structure				*/
/*	level   = relative lock level					*/
/*									*/
/*    Output:								*/
/*									*/
/*	*smplock = initialized, owned by no cpu				*/
/*									*/
/************************************************************************/

void oz_hw_smplock_init (int smplocksize, OZ_Smplock *smplock, uLong level);

/************************************************************************/
/*									*/
/*  Lock an smp lock, wait if owned by another cpu			*/
/*  Also implies inhibiting software interrupt delivery on this cpu	*/
/*									*/
/*    Input:								*/
/*									*/
/*	smplock = pointer to smp lock structure				*/
/*									*/
/*    Output:								*/
/*									*/
/*	oz_hw_smplock_wait = previous smp lock level			*/
/*	                     OZ_SMPLOCK_NULL if none and software 	*/
/*	                       interrupt delivery was enabled		*/
/*	                     OZ_SMPLOCK_SOFTINT if none and software 	*/
/*	                       interrupt delivery was inhibited		*/
/*	                     else some indication of previous level	*/
/*									*/
/*    Note:								*/
/*									*/
/*	This routine crashes if a lock of higher rank is alreay locked 	*/
/*	by the calling cpu.						*/
/*									*/
/*	The same lock may be locked by the same cpu more than once at 	*/
/*	a time, a count is maintained and it is only truly released 	*/
/*	when the count goes zero.					*/
/*									*/
/************************************************************************/

uLong oz_hw_smplock_wait (OZ_Smplock *smplock);

/************************************************************************/
/*									*/
/*  Release the lock of an smp structure				*/
/*									*/
/*    Input:								*/
/*									*/
/*	smplock = pointer to smplock structure				*/
/*	old     = as returned by corresponding oz_hw_smplock_wait call	*/
/*	          if OZ_SMPLOCK_NULL, implies enabling software interrupt delivery
/*									*/
/*    Output:								*/
/*									*/
/*	smp lock count decremented					*/
/*	level restored to 'old'						*/
/*									*/
/************************************************************************/

void oz_hw_smplock_clr (OZ_Smplock *smplock, uLong old);

/************************************************************************/
/*									*/
/*  Get the smplock level the current cpu is at				*/
/*									*/
/************************************************************************/

uLong oz_hw_cpu_smplevel (void);

/************************************************************************/
/*									*/
/*  Get the level of an smp lock					*/
/*									*/
/************************************************************************/

uLong oz_hw_smplock_level (OZ_Smplock *smplock);

/************************************************************************/
/*									*/
/*  Get the cpu that holds an smp lock.  Return -1 if unowned.		*/
/*									*/
/************************************************************************/

Long oz_hw_smplock_cpu (OZ_Smplock *smplock);

/************************************************************************/
/*									*/
/*  Crash if current smplevel is not right				*/
/*									*/
/************************************************************************/

#define OZ_KNL_CHKSMPLEVEL(__level) do { uLong __curlev = oz_hw_cpu_smplevel (); if (__curlev != __level) oz_crash ("%s %d: level is %u, should be %u", __FILE__, __LINE__, __curlev, __level); } while (0)

/************************************************************************/
/*									*/
/*  Fill a buffer with random data					*/
/*									*/
/************************************************************************/

void oz_hw_random_fill (uLong size, void *buff);

/************************************************************************/
/*									*/
/*  Get return address of a particular frame				*/
/*									*/
/*    Input:								*/
/*									*/
/*	frame_index = index of frame to return the return address from	*/
/*	              0 gets the rtn address of call to oz_hw_getrtnadr	*/
/*									*/
/*    Output:								*/
/*									*/
/*	oz_hw_getrtnadr = return address of that frame			*/
/*	                  NULL if no more frames			*/
/*									*/
/************************************************************************/

void *oz_hw_getrtnadr (int frame_index);

/************************************************************************/
/*									*/
/*  Atomically increment a Long						*/
/*									*/
/*    Input:								*/
/*									*/
/*	value = pointer to long to be incremented			*/
/*	inc = -1 : subtract from long					*/
/*	       0 : do nothing, just return *value			*/
/*	       1 : add to long						*/
/*									*/
/*    Output:								*/
/*									*/
/*	*value = incremented by inc, even in an smp environment		*/
/*	oz_hw_atomic_inc_long = new *value				*/
/*									*/
/************************************************************************/

#ifndef oz_hw_atomic_inc_long
Long oz_hw_atomic_inc_long (volatile Long *value, Long inc);
#endif

#ifndef OZ_HW_ATOMIC_INCBY1_LONG
#define OZ_HW_ATOMIC_INCBY1_LONG(loc) oz_hw_atomic_inc_long (&(loc),  1)
#endif

#ifndef OZ_HW_ATOMIC_DECBY1_LONG
#define OZ_HW_ATOMIC_DECBY1_LONG(loc) oz_hw_atomic_inc_long (&(loc), -1)
#endif

/************************************************************************/
/*									*/
/*  Atomically or a Long						*/
/*									*/
/*    Input:								*/
/*									*/
/*	value = pointer to long to be set 				*/
/*	inc = new value to or into long					*/
/*									*/
/*    Output:								*/
/*									*/
/*	*value = set to new value, even in an smp environment		*/
/*	oz_hw_atomic_or_long = old *value				*/
/*									*/
/************************************************************************/

Long oz_hw_atomic_or_long (volatile Long *value, Long inc);

/************************************************************************/
/*									*/
/*  Atomically and a Long						*/
/*									*/
/*    Input:								*/
/*									*/
/*	value = pointer to long to be set 				*/
/*	inc = new value to and into long				*/
/*									*/
/*    Output:								*/
/*									*/
/*	*value = set to new value, even in an smp environment		*/
/*	oz_hw_atomic_and_long = old *value				*/
/*									*/
/************************************************************************/

Long oz_hw_atomic_and_long (volatile Long *value, Long inc);

/************************************************************************/
/*									*/
/*  Atomically test and set a Long					*/
/*									*/
/*    Input:								*/
/*									*/
/*	value = pointer to long to be set 				*/
/*	set = new value to store in long				*/
/*									*/
/*    Output:								*/
/*									*/
/*	*value = set to new value, even in an smp environment		*/
/*	oz_hw_atomic_set_long = old *value				*/
/*									*/
/************************************************************************/

#ifndef oz_hw_atomic_set_long
Long oz_hw_atomic_set_long (volatile Long *value, Long set);
#endif

/************************************************************************/
/*									*/
/*  Atomically test and conditionally set a Long			*/
/*									*/
/*    Input:								*/
/*									*/
/*	value = pointer to long to be set 				*/
/*	new = new value to store in long				*/
/*	old = expected contents of long					*/
/*									*/
/*    Output:								*/
/*									*/
/*	oz_hw_atomic_set_long = 0 : *value was not 'old'		*/
/*	                        1 : *value was 'old', now is 'new'	*/
/*									*/
/************************************************************************/

#ifndef oz_hw_atomic_setif_long
int oz_hw_atomic_setif_long (volatile Long *value, Long new, Long old);
#endif

/* Same thing, but does it with pointers */

#ifndef oz_hw_atomic_setif_ptr
int oz_hw_atomic_setif_ptr (void *volatile *value, void *new, void *old);
#endif

/************************************************************************/
/*									*/
/*  Initialize thread hardware context block				*/
/*									*/
/*    Input:								*/
/*									*/
/*	thread_hw_ctx = pointer to hardware context block		*/
/*	stacksize = number of pages to allocate for user stack		*/
/*	            0 if thentry is to be called in kernel mode		*/
/*	thentry   = entrypoint to be passed to oz_thread_init		*/
/*	            0 if initializing the current cpu as a thread	*/
/*	thparam   = parameter to be passed to oz_thread_init		*/
/*	process_hw_ctx = corresponding process hardware context block	*/
/*	thread    = thread software context block pointer		*/
/*									*/
/*	smplock = softint delivery inhibited				*/
/*									*/
/*    Output:								*/
/*									*/
/*	*thread_hw_ctx = context block set up				*/
/*									*/
/*    Note:								*/
/*									*/
/*	context block should be set up such that when the context is 	*/
/*	loaded by oz_hw_thread_switchctx, the following happens:	*/
/*									*/
/*	routine oz_knl_thread_start (thread) is called in kernel mode	*/
/*	routine thentry (thparam) is called in user mode		*/
/*	routine oz_sys_thread_exit is called in user mode		*/
/*									*/
/************************************************************************/

uLong oz_hw_thread_initctx (void *thread_hw_ctx, 
                            OZ_Mempage stacksize, 
                            OZ_Thread_entry thentry, 
                            void *thparam, 
                            void *process_hw_ctx, 
                            OZ_Thread *thread);

/************************************************************************/
/*									*/
/*  Terminate as much about the thread as possible in its own context	*/
/*									*/
/*    Input:								*/
/*									*/
/*	thread_hw_ctx  = pointer to hardware context block		*/
/*									*/
/*	smplock = softint delivery inhibited				*/
/*									*/
/************************************************************************/

void oz_hw_thread_exited (void *thread_hw_ctx);

/************************************************************************/
/*									*/
/*  Terminate thread hardware context block				*/
/*									*/
/*    Input:								*/
/*									*/
/*	thread_hw_ctx  = pointer to hardware context block		*/
/*	process_hw_ctx = pointer to process hardware context block	*/
/*									*/
/*	smplock = ts							*/
/*									*/
/*    Output:								*/
/*									*/
/*	*thread_hw_ctx = voided out					*/
/*									*/
/************************************************************************/

void oz_hw_thread_termctx (void *thread_hw_ctx, 
                           void *process_hw_ctx);

/************************************************************************/
/*									*/
/*  Switch thread hardware context					*/
/*									*/
/*    Input:								*/
/*									*/
/*	old_thread_hwctx   = pointer to old thread's hw context block	*/
/*	*new_thread_hwctx  = new thread's hw context			*/
/*									*/
/*	smplock = ts							*/
/*									*/
/*    Output:								*/
/*									*/
/*	*old_thread_hwctx  = old thread's hw context			*/
/*									*/
/*    Note:								*/
/*									*/
/*	old_thread_hwctx and new_thread_hwctx will never be the same	*/
/*									*/
/************************************************************************/

void oz_hw_thread_switchctx (void *old_thread_hwctx, 
                             void *new_thread_hwctx);

/************************************************************************/
/*									*/
/*  Return number of bytes available on current thread's kernel stack	*/
/*									*/
/************************************************************************/

uLong oz_hw_thread_kstackleft (void);

/************************************************************************/
/*									*/
/*  Update hardware ast state						*/
/*									*/
/*    Input:								*/
/*									*/
/*	thread_hwctx = thread's hardware context block			*/
/*	procmode = processor mode the new state applies to		*/
/*	aststate = 0 : there are no deliverable ast's pending		*/
/*	           1 : there are deliverable ast's pending		*/
/*	cpuidx = -1 : the thread is not running in any cpu now		*/
/*	       else : the thread is running in the given cpu		*/
/*									*/
/*    Note:								*/
/*									*/
/*	This should tirgger the hardware to deliver ast's when the cpu 	*/
/*	has softints enabled and is at the corresponding processor mode	*/
/*									*/
/************************************************************************/

void oz_hw_thread_aststate (void *thread_hwctx, OZ_Procmode procmode, int state, Long cpuidx);

/************************************************************************/
/*									*/
/*  Initialize process context block					*/
/*									*/
/*    Input:								*/
/*									*/
/*	process_hw_ctx = pointer to process hardware context block	*/
/*	process  = pointer to process block				*/
/*	sysproc  = 0 : normal process					*/
/*	           1 : oz_s_systemproc process				*/
/*	copyproc = 0 : create an empty process				*/
/*	           1 : create process with pagetables identical to 	*/
/*	               current process					*/
/*									*/
/*	ipl = softint							*/
/*									*/
/************************************************************************/

uLong oz_hw_process_initctx (void *process_hw_ctx, OZ_Process *process, int sysproc, int copyproc);

/************************************************************************/
/*									*/
/*  Switch process context on the current cpu				*/
/*									*/
/*    Input:								*/
/*									*/
/*	old_hw_ctx = pointer to old process hardware context block	*/
/*	new_hw_ctx = pointer to new process hardware context block	*/
/*	smp lock = ts							*/
/*									*/
/*    Output:								*/
/*									*/
/*	per-process memory is mapped to new_hw_ctx process		*/
/*									*/
/*    Note:								*/
/*									*/
/*	old_hw_ctx and new_hw_ctx will never be the same		*/
/*									*/
/************************************************************************/

void oz_hw_process_switchctx (void *old_hw_ctx, void *new_hw_ctx);

/************************************************************************/
/*									*/
/*  Terminate current normal process context block			*/
/*  System process hardware context block is never terminated		*/
/*									*/
/*  Note that the final thread (routine cleanupproc in 			*/
/*  oz_knl_process_increfc) is still active in the process		*/
/*									*/
/*    Input:								*/
/*									*/
/*	process_hw_ctx = pointer to process hardware context block	*/
/*	process = pointer to process block				*/
/*									*/
/*	ipl = softint							*/
/*									*/
/************************************************************************/

void oz_hw_process_termctx (void *process_hw_ctx, OZ_Process *process);

/************************************************************************/
/*									*/
/*  Make sure an hardware context block is sane				*/
/*									*/
/************************************************************************/

void oz_hw_process_validate (void *process_hw_ctx, OZ_Process *process);

/************************************************************************/
/*									*/
/*  Initialize non-paged pool memory					*/
/*									*/
/*    Input:								*/
/*									*/
/*	*numpages = number of pages required				*/
/*	*physpage = first available physical page			*/
/*	*virtpage = first available virtual page			*/
/*									*/
/*    Output:								*/
/*									*/
/*	*numpages = actual number of pages allocated			*/
/*	*physpage = actual starting physical page			*/
/*	*virtpage = actual starting virtual page			*/
/*									*/
/************************************************************************/

void oz_hw_pool_init (OZ_Mempage *numpages, OZ_Mempage *physpage, OZ_Mempage *virtpage);

/************************************************************************/
/*									*/
/*  Expand a page table of the current process				*/
/*  - set new entries to 'no access'					*/
/*									*/
/*    Input:								*/
/*									*/
/*	expand      = expansion direction				*/
/*	              OZ_SECTION_EXPUP = expand upward			*/
/*	              OZ_SECTION_EXPDN = expand downward		*/
/*	maxpages    = maximum number of pages allowed in table		*/
/*	*npages     = current number of pages				*/
/*	              (initially 0)					*/
/*	*vpage      = starting virtual page number (low address)	*/
/*	              (initially 0)					*/
/*	*pagentries = address of page table				*/
/*	              (initially NULL)					*/
/*	newpages    = new number of pages required			*/
/*	newvpage    = starting address of new pages required		*/
/*	process_hw_ctx = process hardware ctx block pointer		*/
/*									*/
/*	smp lock = mp							*/
/*									*/
/*    Output:								*/
/*									*/
/*	*npages     = new total number of pages in table		*/
/*	*vpage      = new starting virtual page number			*/
/*	*pagentries = new address of page table				*/
/*									*/
/*    Note:								*/
/*									*/
/*	If an implementation allows for page tables themselves to be 	*/
/*	pageable, then this routine may call oz_knl_section_create to 	*/
/*	create a pageable section.  Then the page table entries would 	*/
/*	be written to that section instead of directly to memory.	*/
/*									*/
/************************************************************************/

void oz_hw_pte_expand (OZ_Section_expand expand, 
                       OZ_Mempage maxpages, 
                       OZ_Mempage *npages, 
                       OZ_Mempage *vpage, 
                       void **pagentries, 
                       OZ_Mempage newpages, 
                       OZ_Mempage newvpage, 
                       void *process_hw_ctx);

/************************************************************************/
/*									*/
/*  Delete a page table from the current process that was created by 	*/
/*  oz_hw_pte_expand							*/
/*									*/
/*    Input:								*/
/*									*/
/*	expand     = expansion direction				*/
/*	             OZ_SECTION_EXPUP = expand upward			*/
/*	             OZ_SECTION_EXPDN = expand downward			*/
/*	maxpages   = maximum number of pages allowed in table		*/
/*	npages     = current number of pages				*/
/*	vpage      = starting virtual page number (low address)		*/
/*	pagentries = address of page table				*/
/*	process_hw_ctx = process hardware ctx block pointer		*/
/*									*/
/*    Output:								*/
/*									*/
/*	space released that table occupied				*/
/*									*/
/*    Note:								*/
/*									*/
/*	all pt entries in the table should already be 'no-access'	*/
/*									*/
/************************************************************************/

void oz_hw_pte_delete (OZ_Section_expand expand, 
                       OZ_Mempage maxpages, 
                       OZ_Mempage npages, 
                       OZ_Mempage vpage, 
                       void *pagentries, 
                       void *process_hw_ctx);

/************************************************************************/
/*									*/
/*  An pagetable page is about to be paged out or unmapped.  Check it 	*/
/*  to see that all pages it maps are also paged out or unmapped.  If 	*/
/*  it finds any that aren't, return the vpage of one that isn't.	*/
/*									*/
/*    Input:								*/
/*									*/
/*	vpage = virt page number of pagetable page that is about to be 	*/
/*	        paged out or unmapped					*/
/*	unmap = 0 : paging it out, just check for 			*/
/*	            pagestate=PAGEDOUT and curport=NA			*/
/*	        1 : unmapping it, also check for phypage=0		*/
/*									*/
/*    Output:								*/
/*									*/
/*	oz_hw_pte_checkptpage = 0 : all pages are ok			*/
/*	                     else : at least this page needs attention	*/
/*									*/
/************************************************************************/

OZ_Mempage oz_hw_pte_checkptpage (OZ_Mempage vpage, int unmap);

/************************************************************************/
/*									*/
/*  Read hardware pte							*/
/*									*/
/*    Input:								*/
/*									*/
/*	vpn = virtual page number to read the pte of			*/
/*									*/
/*    Output:								*/
/*									*/
/*	oz_hw_pte_read = NULL : pte read successfully			*/
/*	                 else : this vaddr must be faulted in first	*/
/*	*pagestate_r = page's software state				*/
/*	*phypage_r   = corresponding physical page number		*/
/*	*curprot_r   = page's current protection code			*/
/*	*reqprot_r   = requested page protection code			*/
/*									*/
/************************************************************************/

	/* this version functions as described above */

void *oz_hw_pte_readany (OZ_Mempage vpn, 
                         OZ_Section_pagestate *pagestate_r, 
                         OZ_Mempage *phypage_r, 
                         OZ_Hw_pageprot *curprot_r, 
                         OZ_Hw_pageprot *reqprot_r);

	/* this one only reads system pte's, and crashes if it can't */

void oz_hw_pte_readsys (OZ_Mempage vpn,
                        OZ_Section_pagestate *pagestate_r,
                        OZ_Mempage *phypage_r,
                        OZ_Hw_pageprot *curprot_r,
                        OZ_Hw_pageprot *reqprot_r);

/************************************************************************/
/*									*/
/*  Write hardware pte							*/
/*									*/
/*    Input:								*/
/*									*/
/*	vpn = virtual page number to write the pte of			*/
/*	pagestate = software page state					*/
/*	            (just save this value and return it via oz_hw_pte_read)
/*	phypage   = physical page number				*/
/*	curprot   = protection to set page to				*/
/*	reqprot   = software's requested page protection		*/
/*	            (just save this value and return it via oz_hw_pte_read)
/*									*/
/*    Output:								*/
/*									*/
/*	pte written, cache entry invalidated				*/
/*									*/
/*    Note:								*/
/*									*/
/*	this routine does not check to see if the pte is in memory or 	*/
/*	not, so it will crash if it is above softint.  so if you want 	*/
/*	to check if it is in, you will have to set the corresponding 	*/
/*	pt lock then call oz_hw_pte_readany to see if it is there.	*/
/*									*/
/************************************************************************/

/* This one just invalidates the cache on the current cpu - it is used only for upgrades in protection  */
/* and then only where the other cpu's can recover from a pagefault if they have the old value cached   */

void oz_hw_pte_writecur (OZ_Mempage vpn, 
                         OZ_Section_pagestate pagestate, 
                         OZ_Mempage phypage, 
                         OZ_Hw_pageprot curprot, 
                         OZ_Hw_pageprot reqprot);

/* This one invalidates the cache on all cpus - it is used under all other circumstances */

void oz_hw_pte_writeall (OZ_Mempage vpn, 
                         OZ_Section_pagestate pagestate, 
                         OZ_Mempage phypage, 
                         OZ_Hw_pageprot curprot, 
                         OZ_Hw_pageprot reqprot);

/************************************************************************/
/*									*/
/*  Print pagetable entries corresponding to a given virtual address	*/
/*									*/
/*    Input:								*/
/*									*/
/*	vaddr = virtual address						*/
/*									*/
/************************************************************************/

void oz_hw_pte_print (void *vaddr);

/************************************************************************/
/*									*/
/*  These routines manipulate data using physical addresses		*/
/*									*/
/************************************************************************/

/************************************************************************/
/*									*/
/*  Initialize a physical page to known values				*/
/*									*/
/*    Input:								*/
/*									*/
/*	phypage = physical page number					*/
/*	ptvirtpage = 0 : normal data page, fill with zeroes		*/
/*	          else : will be mapped as this particular virtual 	*/
/*	                 page in current process as a pagetable page	*/
/*	                 - initialize as a page of pte's		*/
/*									*/
/*    Output:								*/
/*									*/
/*	physical page phypage initialized				*/
/*									*/
/************************************************************************/

void oz_hw_phys_initpage (OZ_Mempage phypage, OZ_Mempage ptvirtpage);

/************************************************************************/
/*									*/
/*  Map a physical page to virtual address				*/
/*									*/
/*    Input:								*/
/*									*/
/*	phypage = physical page to map to virtual address		*/
/*	smplevel >= softint						*/
/*									*/
/*    Output:								*/
/*									*/
/*	oz_hw_phys_mappage = virtual address the page is mapped to	*/
/*	*savepte = old pte contents for that virtual address		*/
/*									*/
/*    Note:								*/
/*									*/
/*	You must not call any of the other physical page accessing 	*/
/*	routines to copy to/from the virtual address returned by this 	*/
/*	routine.  Use oz_hw_phys_movephys instead.  This is because 	*/
/*	those other routines temporarily destroy the mapping made by 	*/
/*	this routine while they are accessing their physical page(s).	*/
/*									*/
/************************************************************************/

void *oz_hw_phys_mappage (OZ_Mempage phypage, OZ_Pagentry *savepte);

/************************************************************************/
/*									*/
/*  Unmap the page mapped by oz_hw_phys_mappage				*/
/*									*/
/*    Input:								*/
/*									*/
/*	savepte = as returned by oz_hw_phys_mappage			*/
/*									*/
/************************************************************************/

void oz_hw_phys_unmappage (OZ_Pagentry savepte);

/************************************************************************/
/*									*/
/*  The copy routines' physical addresses are described using an array 	*/
/*  of physical page numbers and a starting byte offset in the first 	*/
/*  physical page.  Once those bytes of the first physical page are 	*/
/*  used up, they go on to the beginning of the second physical page, 	*/
/*  and so on through the array of physical pages.  If the supplied 	*/
/*  offset is .ge. the page size, then the initial page number is 	*/
/*  taken from a later element in the phypages array.			*/
/*									*/
/************************************************************************/

/************************************************************************/
/*									*/
/*  Copy data from virtual address to physical addresses		*/
/*									*/
/*    Input:								*/
/*									*/
/*	nbytes   = number of bytes to copy				*/
/*	vaddr    = virtual address to copy from				*/
/*	phypages = address of physical page numbers to copy to		*/
/*	byteoffs = byte offset in first physical page			*/
/*									*/
/*    Output:								*/
/*									*/
/*	bytes copied to the physical pages				*/
/*									*/
/************************************************************************/

void oz_hw_phys_movefromvirt (uLong nbytes, const void *vaddr, const OZ_Mempage *phypages, uLong byteoffs);

/************************************************************************/
/*									*/
/*  Copy data from physical address to virtual addresses		*/
/*									*/
/*    Input:								*/
/*									*/
/*	nbytes   = number of bytes to copy				*/
/*	vaddr    = virtual address to copy to				*/
/*	phypages = address of physical page numbers to copy from	*/
/*	byteoffs = byte offset in first physical page			*/
/*									*/
/*    Output:								*/
/*									*/
/*	bytes copied from the physical pages				*/
/*									*/
/************************************************************************/

void oz_hw_phys_movetovirt (uLong nbytes, void *vaddr, const OZ_Mempage *phypages, uLong byteoffs);

/************************************************************************/
/*									*/
/*  Copy data from one range of physical addresses to another		*/
/*									*/
/*    Input:								*/
/*									*/
/*	nbytes    = number of bytes to copy				*/
/*	src_pages = array of source physical page numbers		*/
/*	src_offs  = byte offset in src_pages[0] page to start copying	*/
/*	dst_pages = array of destination physical page numbers		*/
/*	dst_offs  = byte offset in dst_pages[0] page to start at	*/
/*									*/
/*    Output:								*/
/*									*/
/*	bytes copied from the physical pages				*/
/*									*/
/************************************************************************/

void oz_hw_phys_movephys (uLong nbytes, const OZ_Mempage *src_pages, uLong src_offs, const OZ_Mempage *dst_pages, uLong dst_offs);

/************************************************************************/
/*									*/
/*  Map kernel memory for the loader					*/
/*									*/
/************************************************************************/

/* If kernel is dynamic image, get base virtual page to load it at */

OZ_Mempage oz_hw_ldr_knlpage_basevp (OZ_Hw_pageprot pageprot, OZ_Mempage npagem, OZ_Mempage svpage);

/* Map the given pages for read/write access to kernel mode only */

void oz_hw_ldr_knlpage_maprw (OZ_Mempage npagem, OZ_Mempage svpage);

/* Set the given pages to read-only by all access modes */

void oz_hw_ldr_knlpage_setro (OZ_Mempage npagem, OZ_Mempage svpage);

/************************************************************************/
/*									*/
/*  Loader's boot device scan routine					*/
/*									*/
/************************************************************************/

int oz_hw_bootscan (volatile int *abortflag, OZ_Event *abortevent, int verbose);

/************************************************************************/
/*									*/
/*  Get current 'iota' time						*/
/*									*/
/************************************************************************/

OZ_Iotatime oz_hw_tod_iotanow (void);

/************************************************************************/
/*									*/
/*  Convert delta 'iota' time to delta system time			*/
/*									*/
/************************************************************************/

OZ_Datebin oz_hw_tod_diota2sys (OZ_Iotatime iotatime);

OZ_Iotatime oz_hw_tod_dsys2iota (OZ_Datebin systime);

/************************************************************************/
/*									*/
/*  Convert absolute 'iota' time to absolute system time		*/
/*									*/
/************************************************************************/

OZ_Datebin oz_hw_tod_aiota2sys (OZ_Iotatime iotatime);

OZ_Iotatime oz_hw_tod_asys2iota (OZ_Datebin systime);

/************************************************************************/
/*									*/
/*  Get current time rate						*/
/*									*/
/*    Output:								*/
/*									*/
/*	oz_hw_tod_getrate = current time rate				*/
/*									*/
/*    Note:								*/
/*									*/
/*	Units are implementation dependent				*/
/*	The higher the number, the slower time passes			*/
/*									*/
/************************************************************************/

uLong oz_hw_tod_getrate (void);

/************************************************************************/
/*									*/
/*  Set time rate							*/
/*									*/
/*    Input:								*/
/*									*/
/*	newrate = new time rate						*/
/*									*/
/*    Note:								*/
/*									*/
/*	Units are implementation dependent				*/
/*	The higher the number, the slower time passes			*/
/*	Setting the value to zero will crash the kernel			*/
/*									*/
/************************************************************************/

void oz_hw_tod_setrate (uLong newrate);

/************************************************************************/
/*									*/
/*  Get current date/time quadword					*/
/*									*/
/*    Output:								*/
/*									*/
/*	oz_hw_tod_getnow = current date/time quadword			*/
/*									*/
/*    Note:								*/
/*									*/
/*	This routine may be called from either user or kernel mode	*/
/*									*/
/************************************************************************/

OZ_Datebin oz_hw_tod_getnow (void);

/************************************************************************/
/*									*/
/*  Set current date/time quadword					*/
/*									*/
/*    Input:								*/
/*									*/
/*	newdatebin  = new date/time quadword				*/
/*	olddatebin  = old date/time quadword				*/
/*									*/
/************************************************************************/

void oz_hw_tod_setnow (OZ_Datebin newdatebin, OZ_Datebin olddatebin);

/************************************************************************/
/*									*/
/*  Software timing loop routines					*/
/*									*/
/************************************************************************/

/* Initialize timing */

void oz_hw_stl_init (void);

/* Microsend delay, calling the 'entry' routine along the way  */
/* Breaks out of the delay if 'entry' returns a non-zero value */

uLong oz_hw_stl_microwait (uLong microseconds, uLong (*entry) (void *param), void *param);

/* Nanosecond delay */

void oz_hw_stl_nanowait (uLong nanoseconds);


/************************************************************************/
/*									*/
/*  Set the datebin of the next event					*/
/*  When this time is reached, call oz_knl_timer_timeisup		*/
/*									*/
/*    Input:								*/
/*									*/
/*	datebin = datebin of next event					*/
/*	smplock = tm							*/
/*									*/
/*    Output:								*/
/*									*/
/*	oz_knl_timer_timeisup will be called at or just after the 	*/
/*	given datebin with the tm smplock set				*/
/*									*/
/************************************************************************/

void oz_hw_timer_setevent (OZ_Datebin datebin);

/************************************************************************/
/*									*/
/*  Ascii <-> Integer conversions					*/
/*									*/
/************************************************************************/

void oz_hw_itoa (uLong valu, uLong size, char *buff);
void oz_hw_ztoa (uLong valu, uLong size, char *buff);
uLong oz_hw_atoi (const char *s, int *usedup);
uLong oz_hw_atoz (const char *s, int *usedup);

/************************************************************************/
/*									*/
/*  Write breakpoint							*/
/*									*/
/************************************************************************/

void oz_hw_debug_bpt (void);
char const *oz_hw_debug_writebpt (OZ_Breakpoint *bptaddr, OZ_Breakpoint opcode);
void oz_hw_debug_halt (void);

/************************************************************************/
/*									*/
/*  Crash the computer - does something like a breakpoint with 		*/
/*  hardware interrupts inhibited which causes a fatal kernel 		*/
/*  exception								*/
/*									*/
/************************************************************************/

void oz_hw_crash (void) __attribute__ ((noreturn));

/************************************************************************/
/*									*/
/*  Reboot or Halt							*/
/*									*/
/************************************************************************/

void oz_hw_reboot (void) __attribute__ ((noreturn));
void oz_hw_halt   (void) __attribute__ ((noreturn));

/************************************************************************/
/*									*/
/*  Return 1 if in kernel mode, 0 otherwise				*/
/*									*/
/************************************************************************/

int oz_hw_inknlmode (void);

/************************************************************************/
/*									*/
/*  Perform traceback of call frames					*/
/*									*/
/*    Input:								*/
/*									*/
/*	entry = entrypoint of callback routine				*/
/*	param = parameter for callback routine				*/
/*	maxframes = max number of frames (or -1 for all)		*/
/*	mchargs = initial machine args (or NULL to create own)		*/
/*	readmem = routine to read frames (or NULL for local)		*/
/*									*/
/*    Output:								*/
/*									*/
/*	Callback routine is called with parameter and machine args for 	*/
/*	each call frame.  The machine arguments are filled in as best 	*/
/*	as can be done for the calling standard in use, but at least 	*/
/*	the return and frame addresses are filled in for each frame.	*/
/*									*/
/************************************************************************/

void oz_hw_traceback (void (*entry) (void *param, OZ_Mchargs *mchargs), 
                      void *param, 
                      uLong maxframes, 
                      OZ_Mchargs *mchargs, 
                      int (*readmem) (void *param, void *buff, uLong size, void *addr));

/************************************************************************/
/*									*/
/*  Print machine arguments						*/
/*									*/
/************************************************************************/

uLong oz_hw_mchargs_print (uLong (*entry) (void *param, const char *format, ...), void *param, int full, OZ_Mchargs *mchargs);

/************************************************************************/
/*									*/
/*  Machine argument (standard and extended) descriptor tables		*/
/*									*/
/************************************************************************/

extern const OZ_Debug_mchargsdes oz_hw_mchargs_des[];		// standard machine arguments
extern const OZ_Debug_mchargsdes oz_hw_mchargx_knl_des[];	// extended kernel-mode machine arguments
extern const OZ_Debug_mchargsdes oz_hw_mchargx_usr_des[];	// extended user-mode machine arguments

/************************************************************************/
/*									*/
/*  Fetch and store extended machine arguments 				*/
/*  (on current cpu only)						*/
/*									*/
/************************************************************************/

void oz_hw_mchargx_knl_fetch (OZ_Mchargx_knl *mchargx_knl);				// transfer from cpu to mchargx_knl struct
void oz_hw_mchargx_knl_store (OZ_Mchargx_knl *mchargx_knl, OZ_Mchargx_knl *mask);	// transfer from mchargx_knl struct to cpu

void oz_hw_mchargx_usr_fetch (OZ_Mchargx_usr *mchargx_usr);				// transfer from cpu to mchargx_usr struct
void oz_hw_mchargx_usr_store (OZ_Mchargx_usr *mchargx_usr, OZ_Mchargx_usr *mask);	// transfer from mchargx_usr struct to cpu

#endif
