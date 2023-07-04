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

/************************************************************************/
/*									*/
/*  Definitions for hardware-specific type "axp"			*/
/*									*/
/************************************************************************/

#ifndef _OZ_HW_AXP_H
#define _OZ_HW_AXP_H

#define globaldef
#define globalref extern

#define oz_hw_getrtnadr __builtin_return_address

/* Max CPU's we can handle */

#define OZ_HW_MAXCPUS (16)

/* Default address to link user executables at */

#define OZ_HW_PROCLINKBASE 0x100000	// (changing requires relinking user executables)
#define OZ_HW_L2PAGESIZEMAX (16)	// max page size possible (changing requires relinking user executables)

/* Page size */

#define OZ_HW_L2PAGESIZE (13)		// LOG2 page size on this CPU
#define OZ_HW_PAGESIZE (8192)
#define OZ_HW_PAGEMASK (8191)

#define OZ_HWAXP_L2VASIZE (4 * OZ_HW_L2PAGESIZE - 9)		// LOG2 of virtual address space size
#define OZ_HWAXP_L2VPSIZE (3 * OZ_HW_L2PAGESIZE - 9)		// LOG2 of virtual page number size
#define OZ_HWAXP_L1PTSIZE (1ULL << OZ_HW_L2PAGESIZE)		// number of bytes in the whole L1 pagetable (8KB)
#define OZ_HWAXP_L2PTSIZE (1ULL << (2 * OZ_HW_L2PAGESIZE - 3))	// number of bytes in the whole L2 pagetable (8MB)
#define OZ_HWAXP_L3PTSIZE (1ULL << (3 * OZ_HW_L2PAGESIZE - 6))	// number of bytes in the whole L3 pagetable (8GB)

#define OZ_HWAXP_VAGAPBEG ( 1ULL << (OZ_HWAXP_L2VASIZE - 1))	// beginning of the virtual address gap
#define OZ_HWAXP_VAGAPEND (-1ULL << (OZ_HWAXP_L2VASIZE - 1))	// end of the virtual address gap

uQuad oz_hwaxp_sysbasva;					// this and above is common to all processes
								// - includes kernel's data pages, 8GB/superpage
								//            kernel's L3 pages, 8MB/superpage
								//            kernel's L2 pages, 8KB/superpage
								//   does not include the L1 entries, as, although 
								//     they are the same value for every process, 
								//     they are on a different physical page

/* Where to put dynamically allocated stuff in virtual address space */

#define OZ_HW_DVA_PIMAGES_VA OZ_HW_PROCLINKBASE
#define OZ_HW_DVA_PIMAGES_AT OZ_MAPSECTION_ATBEG

#define OZ_HW_DVA_SIMAGES_VA oz_hwaxp_sysbasva
#define OZ_HW_DVA_SIMAGES_AT OZ_MAPSECTION_ATBEG

#define OZ_HW_DVA_KNLHEAP_VA OZ_HW_PROCLINKBASE
#define OZ_HW_DVA_KNLHEAP_AT OZ_MAPSECTION_ATEND

#define OZ_HW_DVA_USRHEAP_VA OZ_HW_PROCLINKBASE
#define OZ_HW_DVA_USRHEAP_AT OZ_MAPSECTION_ATBEG

#define OZ_HW_DVA_USRSTACK_VA OZ_HW_PROCLINKBASE
#define OZ_HW_DVA_USRSTACK_AT OZ_MAPSECTION_ATEND

/* Macro to extract the offset in a page of a pointer */

#define OZ_HW_VADDRTOPOFFS(vaddr) (uLong)(((Quad)(vaddr)) & OZ_HW_PAGEMASK)

/* Macro to convert a virtual address (of type void *) to a virtual page number (of type OZ_Mempage) */
/* The page number comes out with the high-order bits cleared                                        */

#define OZ_HW_VADDRTOVPAGE(vaddr) (OZ_Mempage)((((Quad)(vaddr)) >> OZ_HW_L2PAGESIZE) & ((1 << (OZ_HWAXP_L2VASIZE - OZ_HW_L2PAGESIZE)) - 1))

/* Macro to convert a virtual page number (of type OZ_Mempage) to a virtual address (of type void *) */
/* The address comes out with the high-order bits sign-extended                                      */

#define OZ_HW_VPAGETOVADDR(vpage) (void *)((((Quad)(vpage)) << (64 - OZ_HWAXP_L2VASIZE + OZ_HW_L2PAGESIZE)) >> (64 - OZ_HWAXP_L2VASIZE))

/* Base virt address of system common and per-process space */

#define OZ_HW_BASE_SYSC_VA oz_hwaxp_sysbasva				/* system common addresses are at the top */
#define OZ_HW_BASE_SYSC_VP OZ_HW_VADDRTOVPAGE (oz_hwaxp_sysbasva)	/* system common base page number */
#define OZ_HW_BASE_PROC_VA (0)						/* process addresses are at the bottom */
#define OZ_HW_BASE_PROC_VP (0)						/* process base page number */

#define OZ_HW_ISSYSADDR(__x) (((OZ_Pointer)(__x)) >= oz_hwaxp_sysbasva)
#define OZ_HW_ISSYSPAGE(__x) ((__x) >= OZ_HW_BASE_SYSC_VP)

/* Endianness */

#define OZ_HW_LITTLEENDIAN 1
#define OZ_HW_BIGENDIAN 0

/* Number of virtual address bits */

#define OZ_HW_64BIT 1

/* Non-zero to include memoury colouring (keeps phypages */
/* aligned with processor cache, see oz_knl_phymem.c)    */

#define OZ_HW_MEMORYCOLORING 1

/* Holds a quadword date */

typedef unsigned long long OZ_Datebin;

/* Add, Subtract, Compare two datebin's */

#define OZ_HW_DATEBIN_ADD(__s,__a,__b) (__s) = (__a) + (__b)
#define OZ_HW_DATEBIN_SUB(__d,__s,__m) (__d) = (__s) - (__m)
#define OZ_HW_DATEBIN_CMP(__s,__m) ((__s) < (__m) ? -1 : ((__s) > (__m)))

/* Clear and test a datebin */

#define OZ_HW_DATEBIN_CLR(__d) (__d) = 0
#define OZ_HW_DATEBIN_TST(__d) ((__d) != 0)

/* Holds an Iota time */

typedef unsigned long long OZ_Iotatime;

/* Where the PARAMS block is in the boot file         */
/* These values must match those in ozone_boot_486.s, */
/* oz_loader_486.s, oz_preloader_486.s                */

//?? #define OZ_LDR_PRLDSEG 0x0800	/* preloader gets loaded here (it is VBN 2) */
//?? #define OZ_LDR_PRMBASE 0x9000	/* the params block gets loaded here */
#define OZ_LDR_PARAMS_VBN 0	//?? haven't decided where to put it yet

/* Crash dumper allows us to have a memory hole - these are phypage numbers */

#define OZ_HW_CRASH_HOLEBEG 0x0A0
#define OZ_HW_CRASH_HOLEEND 0x100

/* Approximate rate of the oz_s_quickies counter */

#define OZ_HW_QUICKIES (OZ_TIMER_RESOLUTION*100/1821)

/* This signed integer type can hold a pointer */

typedef uQuad OZ_Pointer;

/* This unsigned integer type can hold an physical address */

typedef uQuad OZ_Phyaddr;

/* Macro to say whether or not an hardware page table entry has an extra bit to distinguish */
/* OZ_SECTION_PAGESTATE_VALID_* states from the OZ_SECTION_PAGESTATE_WRITEINPROG state.     */
/* Set to '1' if it does have this ability, set to '0' if not.                              */
/* See oz_knl_section.h and oz_knl_section.c for more information.                          */

#define OZ_HW_WRITEINPROG_VALID 1

/* Values returned by oz_hw_pte_read... and passed to oz_hw_pte_write... for page protections */

typedef uByte OZ_Hw_pageprot;

#define OZ_HW_PAGEPROT_NA (0)
#define OZ_HW_PAGEPROT_KW (1)
#define OZ_HW_PAGEPROT_UR (2)
#define OZ_HW_PAGEPROT_UW (3)
#define OZ_HW_PAGEPROT_MAX (4)

/* Size (in bytes) of thread hardware context block */
/* Must match THCTX__SIZE in oz_params_axp.s        */

#define OZ_THREAD_HW_CTX_SIZE 288

/* Size (in bytes) of process hardware context block */
/* Must match sizeof Prctx in oz_hw_process_axp.c    */

#define OZ_PROCESS_HW_CTX_SIZE 24

/* Size (in bytes) of smp lock block */

#define OZ_SMPLOCK_SIZE 4
typedef struct { Long opaque; } OZ_Smplock;

/* Value returned by oz_hw_smplock_wait and oz_hw_setsoftint when */
/* there were no smp locks set and softint delivery was enabled   */

#define OZ_SMPLOCK_NULL 0

/* Value returned by oz_hw_smplock_wait and oz_hw_setsoftint when */
/* there were no smp locks set and softint delivery was inhibited */

#define OZ_SMPLOCK_SOFTINT 1

/* All the other SMP lock levels - this implementation allows 8 bits for the smp level */

#define OZ_SMPLOCK_LEVEL_SH 0x10		/* shutdown handler table */
#define OZ_SMPLOCK_LEVEL_HT 0x18		/* handle tables */
#define OZ_SMPLOCK_LEVEL_VL 0x20		/* filesystem volume locks */
#define OZ_SMPLOCK_LEVEL_DV 0x28		/* devices */
#define OZ_SMPLOCK_LEVEL_PT 0x30		/* per-process page table */
#define OZ_SMPLOCK_LEVEL_GP 0x40		/* global sections page table */
#define OZ_SMPLOCK_LEVEL_PM 0x48		/* physical memory tables */
#define OZ_SMPLOCK_LEVEL_CP 0x49		/* cache private lock */
#define OZ_SMPLOCK_LEVEL_PS 0x4A		/* per-process state */
#define OZ_SMPLOCK_LEVEL_TF 0x4C		/* thread family list */
#define OZ_SMPLOCK_LEVEL_EV 0x4E		/* individual event flag state */
#define OZ_SMPLOCK_LEVEL_TP 0x50		/* individual thread private lock */
#define OZ_SMPLOCK_LEVEL_TC 0x52		/* thread COM queue lock */
#define OZ_SMPLOCK_LEVEL_PR 0x54		/* process lists */
#define OZ_SMPLOCK_LEVEL_SE 0x60		/* security structs */
#define OZ_SMPLOCK_LEVEL_ID 0x64		/* id numbers (<NP, >PR, >TF) */
#define OZ_SMPLOCK_LEVEL_NP 0x68		/* non-paged pool */
#define OZ_SMPLOCK_LEVEL_QU 0x70		/* quota */

#define OZ_SMPLOCK_LEVEL_IPLS 0xC0		/* IPL's use 0xC0..0xDF */
						/* must match oz_params_486.s */
#define OZ_SMPLOCK_LEVEL_IPL22 0xD6

#define OZ_SMPLOCK_LEVEL_UDI_STANDARD 0xF2	/* two lock levels used by UDI */
#define OZ_SMPLOCK_LEVEL_UDI_SIMPLE   0xF3

#define OZ_SMPLOCK_LEVEL_LG 0xFA		/* log routines */
#define OZ_SMPLOCK_LEVEL_HI 0xFC		/* lowipl routines */

/* Thread priority definitions */

#define OZ_THREADPRIO_MAXIMUM 0xFFFFFF	/* this implementation allows 24 bits */
#define OZ_THREADPRIO_STARTUP 1000000	/* this is the startup thread's priority */

/* Wait loop macros - see oz_knl_thread.c oz_knl_thread_wait routine */

#define OZ_HW_WAITLOOP_INIT oz_hw_waitloop_init ()			// enable softints but inhibit hardints
#define OZ_HW_WAITLOOP_BODY oz_hw_waitloop_body ()			// enable all ints, wait for int, inhib all ints
#define OZ_HW_WAITLOOP_TERM oz_hw_waitloop_term ()			// enable hardints but inhibit softints
#define OZ_HW_WAITLOOP_WAKE(cpuidx) oz_hw_waitloop_wake (cpuidx)	// wake the indicated cpu from waitloop body

/* Unsigned value that can hold what the pte holds a physical page number in */
/* It is also used to hold virtual page numbers                              */

typedef uLong OZ_Mempage;

/* Unsigned value that can hold the offset within a page of where a particular byte is */

typedef uLong OZ_Mempofs;	/* note - could really be just a uWord but this keeps things longword aligned */

/* A page table entry - opaque struct of correct size */

typedef uQuad OZ_Pagentry;

/* Pagetable entry contents */

#define OZ_HWAXP_PTE__NA 0x0000		// no access
#define OZ_HWAXP_PTE__KR 0x0101		// kernel mode read-only
#define OZ_HWAXP_PTE__KW 0x1101		// kernel mode read/write
#define OZ_HWAXP_PTE__UR 0x0901		// kernel/user read-only
#define OZ_HWAXP_PTE__UW 0x9901		// kernel/user read/write
#define OZ_HWAXP_PTE__XX 0xFF01		// mask for all protection bits

#define OZ_HWAXP_PTE_M_GBL 0x0010	// set to mark global to all processes (ie, a system page)
#define OZ_HWAXP_PTE_V_GH 5		// granularity hint is in <06:05>
#define OZ_HWAXP_PTE_X_GH 3		// - two bits wide

#define OZ_HWAXP_PTE_V_PS 16		// pagestate (software page state)
#define OZ_HWAXP_PTE_X_PS 15		// - four bits wide
#define OZ_HWAXP_PTE_V_CP 20		// curprot (current protection)
#define OZ_HWAXP_PTE_X_CP 15		// - four bits wide
#define OZ_HWAXP_PTE_V_RP 24		// reqprot (requested protection)
#define OZ_HWAXP_PTE_X_RP 15		// - four bits wide
#define OZ_HWAXP_PTE_V_PP 32		// physical page number
#define OZ_HWAXP_PTE_X_PP 0xFFFFFFFF	// - 32 bits wide

#define OZ_HWAXP_PTE_PKW (OZ_HWAXP_PTE__KW + (OZ_SECTION_PAGESTATE_VALID_W << OZ_HWAXP_PTE_V_PS) + (OZ_HW_PAGEPROT_KW << OZ_HWAXP_PTE_V_CP) + (OZ_HW_PAGEPROT_KW << OZ_HWAXP_PTE_V_RP))
#define OZ_HWAXP_PTE_GKW (OZ_HWAXP_PTE_PKW + OZ_HWAXP_PTE_M_GBL)

/* A physical page number that will never be used */

#define OZ_PHYPAGE_NULL 0

/* A virtual page number that will never be mapped to anything */

#define OZ_VIRTPAGE_NULL 0

/* Determine if a particular range of addesses is accessible */
/* in the current memory mapping by the given processor mode */

int oz_hw_probe (uLong size, const void *adrs, OZ_Procmode mode, int write);
int oz_hw_prober_strz (const void *adrz, OZ_Procmode mode);
#define OZ_HW_READABLE(__size,__adrs,__mode) oz_hw_probe ((__size), (__adrs), (__mode), 0)
#define OZ_HW_WRITABLE(__size,__adrs,__mode) oz_hw_probe ((__size), (__adrs), (__mode), 1)
#define OZ_HW_READABLE_STRZ(__adrs,__mode) oz_hw_prober_strz ((__adrs), (__mode))

/* Make sure all reads after this happen after any accesses before it */
/* Make sure all writes before this complete before any accesses after it */

#define OZ_HW_MB asm volatile ("mb" : : : "memory")

/* Write breakpoint */

typedef uLong OZ_Breakpoint;
#define OZ_OPCODE_BPT 0x80

/* Create and Delete a thread's kernel stack */

#define OZ_HW_KSTACKINTHCTX 1			// 0 = oz_hw_thread_initctx will create its own kernel stack
						// 1 = oz_knl_thread_create will add space for kernel stack to hw ctx block
						// -- only applies to kernel, loader must always allocate kernel stack
uLong oz_hw486_kstack_create (void *thctx, void **sysvaddr_r);
void oz_hw486_kstack_delete (void *thctx, void *sysvaddr);

/* The sigargs and mchargs for condition handlers */

typedef struct OZ_Mchargs OZ_Mchargs;
typedef struct OZ_Mchargx_knl OZ_Mchargx_knl;
typedef struct OZ_Mchargx_usr OZ_Mchargx_usr;

	/* Signal argument array elements */

typedef uQuad OZ_Sigargs;

	/* Standard Machine arguments - describe state of processor at time of an exception or interrupt */
	/* Must match offsets in oz_params_axp.s */

struct OZ_Mchargs { uQuad r0,r1;
                    uQuad p2,p3,p4,p5;
                    uQuad r8,r9,r10,r11,r12,r13,r14,r15;
                    uQuad r16,r17,r18,r19,r20,r21,r22,r23;
                    uQuad r24,r25,r26,r27,r28,r29,r30;
                    uQuad unq,r2,r3,r4,r5,r6,r7;
                    uQuad pc,ps;
                  };

	/* Extended Machine arguments for kernel mode */
	/* They are not saved/restored automatically at the time of an exception or interrupt */
	/* Fetched via oz_hw_mchargx_knl_fetch, Stored via oz_hw_mchargx_knl_store */
	/* Must match MCHXK_... offsets in oz_params_486.s */

struct OZ_Mchargx_knl { uQuad asn_ro, asten_rw, astsr_rw, datfx_wo, fen_rw, ipir_wo, mces_rw;
                        uQuad pcbb_ro, prbr_rw, ptbr_ro, scbb_rw, sirr_wo;
                        uQuad sisr_ro, tbia_wo, tbiap_wo, tbis_wo, tbisd_wo, tbisi_wo;
                        uQuad usp_rw, vptb_rw, whami_ro;
                      };

	/* Extended Machine arguments for user mode */
	/* They are not saved/restored automatically at the time of an exception or interrupt */
	/* Fetched via oz_hw_mchargx_usr_fetch, Stored via oz_hw_mchargx_usr_store */
	/* Must match MCHXU_... offsets in oz_params_486.s */

struct OZ_Mchargx_usr { uWord ds, es, fs, gs, ss, pad1;	/* data and stack segment registers */
							/* ?? someday add hw debug and floating point, etc */
                      };

/* The jmpbuf used by setjmp/longjmp routines */
/* Must match glibc jmp_buf definition        */
/* Must match oz_knl_ctrl_486.s definition    */

typedef struct { uLong ebx, esi, edi, ebp, esp, eip; } OZ_Hw_jmpbuf;

/* Define OZ_HW_SYSCALL_DCL_(0..16) and OZ_HW_SYSCALL_DEF_(0..16) macros */
/* The oz_hw486_syscall.h file is generated by oz_hw_486_syscall_h_gen.c */

#include "oz_hwaxp_syscall.h"

/* Hardware restart parameter block */

typedef struct {
  uQuad hwrpbpa;   	// hwrbp's physical address
  uQuad magic;     	// "HWRPB"
  uQuad revision;  	// revision
  uQuad hwrpbsiz;  	// hwrpb size (includes all variable fields)
  uQuad pricpuid;  	// primary cpu id (its WHAMI)
  uQuad pagesize;  	// page size
  uLong pabits;    	// number of physical address bits
  uLong vaexbits;  	// number of virt address extension bits
  uQuad maxvalasn; 	// maximum valid asn
  uQuad syssernum; 	// system serial number
  uQuad pad1;
  uQuad systype;   	// system type
  uQuad sysvar;    	// system variation
  uQuad sysrevis;  	// system revision
  uQuad intclkfrq; 	// interval clock interrupt frequency
  uQuad cycounfrq; 	// cycle counter frequency
  uQuad vptbase;   	// virtual page table base
  uQuad pad2;
  uQuad tbhintofs; 	// offset to translation buffer hint block
  uQuad numprcslt; 	// number of processor slots
  uQuad cpusltsiz; 	// per-cpu slot size
  uQuad cpusltofs; 	// offset to per-cpu slots
  uQuad numofctbs; 	// number of ctbs
  uQuad sizeofctb; 	// ctb size
  uQuad ctbtoffs;  	// offset to console terminal block table
  uQuad ccrboffs;  	// offset to console callback routine block
  uQuad mddtoffs;  	// offset to memory data descriptor table
  uQuad cdboffs;   	// offset to configuration data block (if present)
  uQuad frutblofs; 	// offset to fru table (if present)
  uQuad tssrva;    	// virtual address of terminal save state routine
  uQuad tssrpv;    	// procedure value of terminal save state routine
  uQuad trsrva;    	// virtual address of terminal restore state routine
  uQuad trsrpv;    	// procedure value of terminal restore state routine
  uQuad cpurrva;   	// virtual address of cpu restart routine
  uQuad cpurrpv;   	// procedure value of cpu restart routine
  uQuad pad3, pad4;
  uQuad checksum;  	// checksum of the above
  uQuad rxrdybm;   	// rxrdy bitmask
  uQuad txrdybm;   	// txrdy bitmask
  uQuad dsrdbtofs; 	// offset to dynamic system recognition data block table
} OZ_Hwaxp_Hwrpb;

extern OZ_Hwaxp_Hwrpb *oz_hwaxp_hwrpb;

/* Hardware Process Control Block */
/* Must be 128-byte aligned       */

typedef struct {
  uQuad ksp;	// kernel stack pointer
  uQuad esp;	// exec stack pointer
  uQuad ssp;	// super stack pointer
  uQuad usp;	// user stack pointer
  uQuad ptbr;	// pagetable base register
  uQuad asn;	// address space number
  uQuad ast;	// ast enable and status bits
  uQuad fen;	// floating point enable, perf mon enab, data align trap
  uQuad cpc;	// charged process cycles (32 bits)
  uQuad uniq;	// process unique value
  uQuad pal[6];	// pal scratch area
} OZ_Hwaxp_Hwpcb;

/* Per-CPU database */
/* Must match CPU_... symbols in oz_params_axp.s */

#define OZ_HWAXP_NTEMPSPTES 2		// number of spte's pointed to by tempspte

typedef union {
  struct {
    uLong priority;			// current smp lock level <31:24>; thread priority <23:00>
    uLong invlpg_cpus;			// bitmask of cpu's that haven't invalidated yet
    OZ_Pagentry *tempspte;		// points to two spte's for use by oz_hw_phys_... routines
    uByte *tempspva;			// virtual address mapped by CPU_L_TEMPSPTE spts
    void *thctx;			// current thread hardware context pointer
    void *prctx;			// current process hardware context pointer
    uQuad invlpg_vadr;			// virtual address to be invalidated
    uQuad quantim;			// quantum timer expiration (absolute biased RDTSC value)
    uQuad rdtscbias;			// bias to keep RDTSC the same on all CPU's
    OZ_Hwaxp_Hwpcb *hwpcb_va[2];	// virt adrs' of two 128-byte aligned HWPCB's
    uQuad hwpcb_pa[2];			// phys adrs' of two 128-byte aligned HWPCB's
    uLong acthwpcb;			// which hwpcb is active (0 or 1)
    uByte hwpcb[1];			// the two HWPCB's are in here, 128-byte aligned
  } db;
  uByte pad[1024];			// struct must be a power of 2 size, and hold two 128-byte aligned HWPCB's
} OZ_Hwaxp_Cpudb;

extern OZ_Hwaxp_Cpudb oz_hwaxp_cpudb[OZ_HW_MAXCPUS];

/* Dispatch routine */

#define OZ_HWAXP_DISPATCH_GETC 1
#define OZ_HWAXP_DISPATCH_PUTS 2
#define OZ_HWAXP_DISPATCH_PROCESS_KEYCODE 6

extern uQuad oz_hwaxp_dispatchent, oz_hwaxp_dispatchr27;

/* Flag set suspends the video output routines */

extern int oz_hw_video_suspend;

/* System Pagetable Entry indicating a page that is read/write by kernel mode */

extern uQuad const oz_hwaxp_spte_kw;

/* System process' Level 1 pagetable's physical page number */

extern OZ_Mempage oz_hwaxp_sysl1ptpp;

/* Every process has its pagetables at these virtual addresses */

extern OZ_Pagentry *oz_hwaxp_l1ptbase;
extern OZ_Pagentry *oz_hwaxp_l2ptbase;
extern OZ_Pagentry *oz_hwaxp_l3ptbase;

/* Limits of physical memory */

extern OZ_Mempage oz_hwaxp_botphypage;
extern OZ_Mempage oz_hwaxp_topphypage;

/* Extra smplock routines */

uLong oz_hwaxp_smplock_wait_atipl (OZ_Smplock *smplock);
void oz_hw_smplock_clr_atipl (OZ_Smplock *smplock, uLong old);

/* SCB access */

extern struct { uQuad p2; uQuad p3; } oz_hwaxp_scb[0x800];			// raw hardware SCB

void oz_hwaxp_scb_setc (uLong scboffs, 						// offset 0..7FF0 (not index)
                        void (*entry) (void *param, OZ_Mchargs *mchargs), 	// routine to call
                        void *param, 						// parameter to pass
                        void (**oldent) (void *param, OZ_Mchargs *mchargs), 	// previous routine
                        void **oldprm);						// what it was passed

/* I/O space access routines */

OZ_Mempage oz_hw_get_iopage (void);
void oz_hw_map_iopage (OZ_Mempage phypage, void *svad);

uByte oz_dev_isa_inb (uLong ioaddr);
uWord oz_dev_isa_inw (uLong ioaddr);
uLong oz_dev_isa_inl (uLong ioaddr);

void oz_dev_isa_outb (uByte data, uLong ioaddr);
void oz_dev_isa_outw (uWord data, uLong ioaddr);
void oz_dev_isa_outl (uLong data, uLong ioaddr);

uByte oz_dev_pci_inb (uLong ioaddr);
uWord oz_dev_pci_inw (uLong ioaddr);
uLong oz_dev_pci_inl (uLong ioaddr);

void oz_dev_pci_outb (uByte data, uLong ioaddr);
void oz_dev_pci_outw (uWord data, uLong ioaddr);
void oz_dev_pci_outl (uLong data, uLong ioaddr);

uByte oz_dev_pci_rdb (uLong memaddr);
uWord oz_dev_pci_rdw (uLong memaddr);
uLong oz_dev_pci_rdl (uLong memaddr);

void oz_dev_pci_wtb (uByte data, uLong memaddr);
void oz_dev_pci_wtw (uWord data, uLong memaddr);
void oz_dev_pci_wtl (uLong data, uLong memaddr);

/* Video routines */

void oz_hw_video_init (void);
uLong oz_hw_video_putstring (uLong size, const char *buff);
void oz_hw_video_updcursor (void);
int oz_hw_video_putchar (char c);
void oz_hw_video_linedn (void);
void oz_hw_video_lineup (void);

/* PAL calls */

#define OZ_HWAXP_PAL(pal) 				\
{							\
  asm volatile ("call_pal "#pal				\
                : 					\
                :					\
                : "$0", "$1", "$16", "$17", "memory");	\
}

#define OZ_HWAXP_PAL_R0(pal) 				\
({							\
  register uQuad __r0 asm ("$0");			\
  asm volatile ("call_pal "#pal				\
                : "=r" (__r0)				\
                :					\
                : "$1", "$16", "$17", "memory");	\
  __r0;							\
})

#define OZ_HWAXP_PAL_R0R16(pal,v16) 			\
({							\
  register uQuad __r0 asm ("$0");			\
  register uQuad __r16 asm ("$16") = (uQuad)(v16); 	\
  asm volatile ("call_pal "#pal				\
                : "=r" (__r0)				\
                : "r" (__r16)				\
                : "$1", "$16", "$17", "memory");	\
  __r0;							\
})

#define OZ_HWAXP_PAL_R0R16R17R18(pal,v16,v17,v18)	\
({							\
  register uQuad __r0 asm ("$0");			\
  register uQuad __r16 asm ("$16") = (uQuad)(v16); 	\
  register uQuad __r17 asm ("$17") = (uQuad)(v17); 	\
  register uQuad __r18 asm ("$18") = (uQuad)(v18); 	\
  asm volatile ("call_pal "#pal				\
                : "=r" (__r0)				\
                : "r" (__r16), "r" (__r17), "r" (__r18)	\
                : "$1", "$16", "$17", "$18", "memory");	\
  __r0;							\
})

#define OZ_HWAXP_PAL_R16(pal,v16) 			\
{							\
  register uQuad __r16 asm ("$16") = (uQuad)(v16); 	\
  asm volatile ("call_pal "#pal				\
                : 					\
                : "r" (__r16)				\
                : "$0", "$1", "$16", "$17", "memory");	\
}

#define OZ_HWAXP_PAL_R16R17(pal,v16,v17) 		\
{							\
  register uQuad __r16 asm ("$16") = (uQuad)(v16); 	\
  register uQuad __r17 asm ("$17") = (uQuad)(v17); 	\
  asm volatile ("call_pal "#pal				\
                : 					\
                : "r" (__r16), "r" (__r17)		\
                : "$0", "$1", "$16", "$17", "memory");	\
}

#define OZ_HWAXP_BPT()            OZ_HWAXP_PAL       (0x80)		// Breakpoint
#define OZ_HWAXP_BUGCHK()         OZ_HWAXP_PAL       (0x81)		// Bugcheck
#define OZ_HWAXP_CFLUSH(phypage)  OZ_HWAXP_PAL_R16   (0x01,phypage)	// Cache Flush
#define OZ_HWAXP_CHME()           OZ_HWAXP_PAL       (0x82)		// Change mode to EXEC
#define OZ_HWAXP_CHMK()           OZ_HWAXP_PAL       (0x83)		// Change mode to KERNEL
#define OZ_HWAXP_CHMS()           OZ_HWAXP_PAL       (0x84)		// Change mode to SUPER
#define OZ_HWAXP_CHMU()           OZ_HWAXP_PAL       (0x85)		// Change mode to USER
#define OZ_HWAXP_DRAINA()         OZ_HWAXP_PAL       (0x02)		// Drain Aborts
#define OZ_HWAXP_GENTRAP(code)    OZ_HWAXP_PAL_R16   (0xAA,code)	// Generate Software Trap
#define OZ_HWAXP_HALT()           OZ_HWAXP_PAL       (0x00)		// Halt processor
#define OZ_HWAXP_HALT_R16(code)   OZ_HWAXP_PAL_R16   (0x00,code)	// Halt processor (with code in R16)
#define OZ_HWAXP_IMB()            OZ_HWAXP_PAL       (0x86)		// Instruction Memory Barrier
#define OZ_HWAXP_LDQP(addr)       OZ_HWAXP_PAL_R0R16 (0x03,addr)	// Load from physical

#define OZ_HWAXP_MFPR_ASN()       OZ_HWAXP_PAL_R0    (0x06)		// Address Space Number
#define OZ_HWAXP_MFPR_ASTEN()     OZ_HWAXP_PAL_R0    (0x26)		// AST Enable
#define OZ_HWAXP_MFPR_ASTSR()     OZ_HWAXP_PAL_R0    (0x27)		// AST Summary Register
#define OZ_HWAXP_MFPR_ESP()       OZ_HWAXP_PAL_R0    (0x1E)		// Executive Stack Pointer
#define OZ_HWAXP_MFPR_FEN()       OZ_HWAXP_PAL_R0    (0x1B)		// Floating Point Enable
#define OZ_HWAXP_MFPR_IPL()       OZ_HWAXP_PAL_R0    (0x0E)		// Interrupt Priority Level
#define OZ_HWAXP_MFPR_MCES()      OZ_HWAXP_PAL_R0    (0x10)		// Machine Check Error Summary
#define OZ_HWAXP_MFPR_PCBB()      OZ_HWAXP_PAL_R0    (0x12)		// Privileged Context Block Base
#define OZ_HWAXP_MFPR_PRBR()      OZ_HWAXP_PAL_R0    (0x13)		// Processor Base Register
#define OZ_HWAXP_MFPR_PTBR()      OZ_HWAXP_PAL_R0    (0x15)		// Page Table Base Register
#define OZ_HWAXP_MFPR_SCBB()      OZ_HWAXP_PAL_R0    (0x16)		// System Control Block Base
#define OZ_HWAXP_MFPR_SISR()      OZ_HWAXP_PAL_R0    (0x19)		// Software Interrupt Summary Register
#define OZ_HWAXP_MFPR_SSP()       OZ_HWAXP_PAL_R0    (0x20)		// Supervisor Stack Pointer
#define OZ_HWAXP_MFPR_TBCHK(addr) OZ_HWAXP_PAL_R0R16 (0x1A,addr)	// Translation Buffer Check
#define OZ_HWAXP_MFPR_USP()       OZ_HWAXP_PAL_R0    (0x22)		// User Stack Pointer
#define OZ_HWAXP_MFPR_VPTB()      OZ_HWAXP_PAL_R0    (0x29)		// Virtual Page Table
#define OZ_HWAXP_MFPR_WHAMI()     OZ_HWAXP_PAL_R0    (0x3F)		// Who Am I

#define OZ_HWAXP_MTPR_ASTEN(mask) OZ_HWAXP_PAL_R16   (0x07,mask)	// AST Enable
#define OZ_HWAXP_MTPR_ASTSR(mask) OZ_HWAXP_PAL_R16   (0x08,mask)	// AST Summary Register
#define OZ_HWAXP_MTPR_DATFX(val)  OZ_HWAXP_PAL_R16   (0x2E,val)		// Data Alignment Trap Fixup
#define OZ_HWAXP_MTPR_ESP(addr)   OZ_HWAXP_PAL_R16   (0x1F,addr)	// Executive Stack Pointer
#define OZ_HWAXP_MTPR_FEN(val)    OZ_HWAXP_PAL_R16   (0x0C,val)		// Floating Point Enable
#define OZ_HWAXP_MTPR_IPIR(num)   OZ_HWAXP_PAL_R16   (0x0D,num)		// Interprocessor Interrupt Request
#define OZ_HWAXP_MTPR_IPL(val)    OZ_HWAXP_PAL_R0R16 (0x0F,val)		// Interrupt Priority Level
#define OZ_HWAXP_MTPR_MCES(val)   OZ_HWAXP_PAL_R0R16 (0x11,val)		// Machine Check Error Summary
#define OZ_HWAXP_MTPR_PRBR(val)   OZ_HWAXP_PAL_R0R16 (0x14,val)		// Processor Base Register
#define OZ_HWAXP_MTPR_SCBB(addr)  OZ_HWAXP_PAL_R16   (0x17,addr)	// System Control Block Base
#define OZ_HWAXP_MTPR_SIRR(val)   OZ_HWAXP_PAL_R0R16 (0x18,val)		// Software Interrupt Request Register
#define OZ_HWAXP_MTPR_SSP(addr)   OZ_HWAXP_PAL_R16   (0x21,addr)	// Supervisor Stack Pointer
#define OZ_HWAXP_MTPR_TBIA()      OZ_HWAXP_PAL       (0x1B)		// Translation Buffer Invalidate All
#define OZ_HWAXP_MTPR_TBIAP()     OZ_HWAXP_PAL       (0x1C)		// Translation Buffer Invalidate All Process
#define OZ_HWAXP_MTPR_TBIS(addr)  OZ_HWAXP_PAL_R16   (0x1D,addr)	// Translation Buffer Invalidate Single
#define OZ_HWAXP_MTPR_TBISD(addr) OZ_HWAXP_PAL_R16   (0x24,addr)	// Translation Buffer Invalidate Single Data
#define OZ_HWAXP_MTPR_TBISI(addr) OZ_HWAXP_PAL_R16   (0x25,addr)	// Translation Buffer Invalidate Single Instruction
#define OZ_HWAXP_MTPR_USP(addr)   OZ_HWAXP_PAL_R16   (0x23,addr)	// User Stack Pointer
#define OZ_HWAXP_MTPR_VPTB(addr)  OZ_HWAXP_PAL_R16   (0x2A,addr)	// Virtual Page Table

#define OZ_HWAXP_PROBER(addr,len,mode) OZ_HWAXP_PAL_R0R16R17R18 (0x8F,addr,len,mode)
#define OZ_HWAXP_PROBEW(addr,len,mode) OZ_HWAXP_PAL_R0R16R17R18 (0x90,addr,len,mode)

#define OZ_HWAXP_RD_PS()         OZ_HWAXP_PAL_R0     (0x91)		// Read Processor Status
#define OZ_HWAXP_READ_UNQ()      OZ_HWAXP_PAL_R0     (0x9E)		// Read Unique register
#define OZ_HWAXP_REI()           OZ_HWAXP_PAL        (0x92)		// Return from exception or interrupt
#define OZ_HWAXP_RSCC()          OZ_HWAXP_PAL_R0     (0x9D)		// Read System Cycle Counter
#define OZ_HWAXP_STQP(addr,val)  OZ_HWAXP_PAL_R16R17 (0x04,addr,val)	// Store to physical
#define OZ_HWAXP_SWPCTX(hwpcbpa) OZ_HWAXP_PAL_R16    (0x05,hwpcbpa)	// Swap Privileged Context
#define OZ_HWAXP_SWASTEN(mask)   OZ_HWAXP_PAL_R0R16  (0x9B,mask)	// Swap AST Enable
#define OZ_HWAXP_WR_PS_SW(mask)  OZ_HWAXP_PAL_R16    (0x9C,mask)	// Write Processor Status Software Field
#define OZ_HWAXP_WRITE_UNQ(val)  OZ_HWAXP_PAL_R16    (0x9F,val)		// Write Unique register
#define OZ_HWAXP_WTINT(n)        OZ_HWAXP_PAL_R0R16  (0x3E,n)		// Wait for interrupt

/* Other instructions */

#define OZ_HWAXP_RPCC() ({ uQuad pcc; asm volatile ("rpcc %0" : "=r"(pcc)); pcc; })

/* Make it official and efficient: cpuid = whami */

#ifdef __alpha__	// in case this is elfconv built on i386

extern inline Long oz_hw_cpu_getcur (void)

{
  return (OZ_HWAXP_MFPR_WHAMI ());
}

#endif

/* Inline atomic routines */

#define OZ_HW_ATOMIC_INCBY1_LONG(loc)  oz_hw_atomic_inc_long (&loc,  1)
#define OZ_HW_ATOMIC_DECBY1_LONG(loc)  oz_hw_atomic_inc_long (&loc, -1)
#define OZ_HW_ATOMIC_INCBY1_ULONG(loc) oz_hw_atomic_inc_ulong (&loc, 1)

#ifdef __alpha__	// in case this is elfconv built on i386

extern inline Long oz_hw_atomic_inc_long (volatile Long *loc, Long inc)

{
  Long new, old;

  asm volatile ("mb\n"
	"1:	ldl_l	%1,%2\n"	// get *loc into old
	"	addl	%1,%3,%0\n"	// add the increment into new
	"	stl_c	%0,%2\n"	// store new back into *loc
	"	beq	%0,2f\n"	// repeat if *loc might have changed
	"	mb\n"
	"	.subsection 2\n"
	"2:	br	1b\n"
	"	.previous"
		: "=&r" (new), 		// %0
		  "=&r" (old)		// %1
		: "m" (*loc), 		// %2
		  "r" (inc)		// %3
		: "memory");

  return (old + inc);
}

extern inline uLong oz_hw_atomic_inc_ulong (volatile uLong *loc, uLong inc)

{
  uLong new, old;

  asm volatile ("mb\n"
	"1:	ldl_l	%1,%2\n"	// get *loc into old
	"	addl	%1,%3,%0\n"	// add the increment into new
	"	stl_c	%0,%2\n"	// store new back into *loc
	"	beq	%0,2f\n"	// repeat if *loc might have changed
	"	mb\n"
	"	.subsection 2\n"
	"2:	br	1b\n"
	"	.previous"
		: "=&r" (new), 		// %0
		  "=&r" (old)		// %1
		: "m" (*loc), 		// %2
		  "r" (inc)		// %3
		: "memory");

  return (old + inc);
}

extern inline Long oz_hw_atomic_or_long (volatile Long *loc, Long inc)

{
  Long new, old;

  asm volatile ("mb\n"
	"1:	ldl_l	%0,%2\n"	// get *loc into old
	"	or	%0,%3,%1\n"	// create new
	"	stl_c	%1,%2\n"	// store new back into *loc
	"	beq	%1,2f\n"	// repeat if *loc might have changed
	"	mb\n"
	"	.subsection 2\n"
	"2:	br	1b\n"
	"	.previous"
		: "=&r" (old), 		// %0
		  "=&r" (new)		// %1
		: "m" (*loc), 		// %2
		  "r" (inc)		// %3
		: "memory");

  return (old);
}

extern inline uQuad oz_hw_atomic_or_uquad (volatile uQuad *loc, uQuad inc)

{
  uQuad new, old;

  asm volatile ("mb\n"
	"1:	ldq_l	%0,%2\n"	// get *loc into old
	"	or	%0,%3,%1\n"	// create new
	"	stq_c	%1,%2\n"	// store new back into *loc
	"	beq	%1,2f\n"	// repeat if *loc might have changed
	"	mb\n"
	"	.subsection 2\n"
	"2:	br	1b\n"
	"	.previous"
		: "=&r" (old), 		// %0
		  "=&r" (new)		// %1
		: "m" (*loc), 		// %2
		  "r" (inc)		// %3
		: "memory");

  return (old);
}

extern inline Long oz_hw_atomic_and_long (volatile Long *loc, Long inc)

{
  Long new, old;

  asm volatile ("mb\n"
	"1:	ldl_l	%0,%2\n"	// get *loc into old
	"	and	%0,%3,%1\n"	// create new
	"	stl_c	%1,%2\n"	// store new back into *loc
	"	beq	%1,2f\n"	// repeat if *loc might have changed
	"	mb\n"
	"	.subsection 2\n"
	"2:	br	1b\n"
	"	.previous"
		: "=&r" (old), 		// %0
		  "=&r" (new)		// %1
		: "m" (*loc), 		// %2
		  "r" (inc)		// %3
		: "memory");

  return (old);
}

extern inline Long oz_hw_atomic_set_long (volatile Long *loc, Long new)

{
  Long old, tmp;

  asm volatile ("mb\n"
	"1:	ldl_l	%0,%3\n"	// get *loc into old
	"	mov	%2,%1\n"	// get new into tmp
	"	stl_c	%1,%3\n"	// store tmp into *loc
	"	beq	%1,2f\n"	// repeat if *loc might have changed
	"	mb\n"
	"	.subsection 2\n"
	"2:	br	1b\n"
	"	.previous"
		: "=&r" (old), 		// %0
		  "=&r" (tmp)		// %1
		: "r" (new), 		// %2
		  "m" (*loc)		// %3
		: "memory");

  return (old);
}

extern inline int oz_hw_atomic_setif_long (volatile Long *loc, Long new, Long old)

{
  Long dif, tmp;

  asm volatile ("mb\n"
	"1:	ldl_l	%0,%2\n"	// get sample into dif
	"	mov	%3,%1\n"	// get new into tmp
	"	subl	%0,%4,%0\n"	// then subtract old
	"	bne	%0,3f\n"	// exit if different
	"	stl_c	%1,%2\n"	// store tmp into memory
	"	beq	%1,2f\n"	// repeat if it might have changed
	"	mb\n"
	"3:\n"
	"	.subsection 2\n"
	"2:	br	1b\n"
	"	.previous"
		: "=&r" (dif), 		// %0
		  "=&r" (tmp)		// %1
		: "m" (*loc), 		// %2
		  "r" (new), 		// %3
		  "r" (old)		// %4
		: "memory");

  return (dif == 0);
}

extern inline int oz_hw_atomic_setif_ulong (volatile uLong *loc, uLong new, uLong old)

{
  uLong dif, tmp;

  asm volatile ("mb\n"
	"1:	ldl_l	%0,%2\n"	// get sample into dif
	"	mov	%3,%1\n"	// get new into tmp
	"	subl	%0,%4,%0\n"	// then subtract old
	"	bne	%0,3f\n"	// exit if different
	"	stl_c	%1,%2\n"	// store tmp into memory
	"	beq	%1,2f\n"	// repeat if it might have changed
	"	mb\n"
	"3:\n"
	"	.subsection 2\n"
	"2:	br	1b\n"
	"	.previous"
		: "=&r" (dif), 		// %0
		  "=&r" (tmp)		// %1
		: "m" (*loc), 		// %2
		  "r" (new), 		// %3
		  "r" (old)		// %4
		: "memory");

  return (dif == 0);
}

extern inline int oz_hw_atomic_setif_ptr (void *volatile *loc, void *new, void *old)

{
  OZ_Pointer dif, tmp;

  asm volatile ("mb\n"
	"1:	ldq_l	%0,%2\n"	// get sample into dif
	"	mov	%3,%1\n"	// get new into tmp
	"	subl	%0,%4,%0\n"	// then subtract old
	"	bne	%0,3f\n"	// exit if different
	"	stq_c	%1,%2\n"	// store tmp into memory
	"	beq	%1,2f\n"	// repeat if it might have changed
	"	mb\n"
	"3:\n"
	"	.subsection 2\n"
	"2:	br	1b\n"
	"	.previous"
		: "=&r" (dif), 		// %0
		  "=&r" (tmp)		// %1
		: "m" (*loc), 		// %2
		  "r" (new), 		// %3
		  "r" (old)		// %4
		: "memory");

  return (dif == 0);
}

#endif

/************************************************************************/
/*									*/
/*  Macros for use by the debugger					*/
/*									*/
/************************************************************************/

#if defined (_OZ_SYS_DEBUG_C)

#define BPTBACKUP (sizeof (OZ_Breakpoint))				/* how much to back up pc by when we hit a breakpoint */
#define SETSINGSTEP(__mchargs) 0					/* set single-step mode in the mchargs */
#define CLRSINGSTEP(__mchargs) 0					/* clear single-step mode from the mchargs */
#define TSTSINGSTEP(__mchargs) 0					/* test to see if single step mode enabled in mchargs */
#define GETNXTINSAD(__mchargs) ((OZ_Breakpoint *)(__mchargs.pc))	/* get next instruction's address */
#define GETSTACKPTR(__mchargs) (__mchargs.r30)				/* get the stack pointer */
#define CMPSTACKPTR(__mchargs,__oldptr) (__mchargs.r30 >= __oldptr)	/* return TRUE iff mchargs stack pointer is at same or outer level than oldptr */
#define GETFRAMEPTR(__mchargs) 0					/* get the frame pointer */

#define TRACEBACK (dc -> cb -> print) (dc -> cp, "  traceback not implemented yet\n");

#endif

/************************************************************************/
/*									*/
/*  C Runtime library-like routines provided by hardware layer		*/
/*									*/
/************************************************************************/

void bcopy (const void *src, void *dest, unsigned long n);
void bzero (void *s, unsigned long n);

void *memchr    (const void *src, int c, unsigned long len);
void *memchrnot (const void *src, int c, unsigned long len);
int   memcmp    (const void *left, const void *right, unsigned long len);
void *memcpy    (void *dst, const void *src, unsigned long len);
void *memmove   (void *dst, const void *src, unsigned long len);
void *memset    (void *dst, int val, unsigned long len);
void  movc4     (unsigned long slen, const void *src, unsigned long dlen, void *dst);

int           strcasecmp  (const char *left, const char *right);
char         *strcat      (char *dst, const char *src);
char         *strchr      (const char *src, int c);
int           strcmp      (const char *left, const char *right);
char         *strcpy      (char *dst, const char *src);
unsigned long strlen      (const char *src);
int           strncasecmp (const char *left, const char *right, unsigned long len);
char         *strncat     (char *dst, const char *src, unsigned long len);
int           strncmp     (const char *left, const char *right, unsigned long len);
char         *strncpy     (char *dst, const char *src, unsigned long len);	/* doesn't guarantee a null terminator */
char         *strncpyz    (char *dst, const char *src, unsigned long len);	/* guarantees at least one null terminator */
unsigned long strnlen     (const char *src, unsigned long len);			/* maximum length of 'len' */
char         *strrchr     (const char *src, int c);
char         *strstr      (const char *haystack, const char *needle);

#endif
