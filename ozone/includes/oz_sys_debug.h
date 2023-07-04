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

#ifndef _OZ_SYS_DEBUG_H
#define _OZ_SYS_DEBUG_H

#include "ozone.h"
#include "oz_knl_hw.h"

#ifdef _OZ_SYS_DEBUG_C
typedef struct OZ_Debug OZ_Debug;
#else
typedef void OZ_Debug;
#endif

#define OZ_DEBUG_MAXCPUS 32
#define OZ_DEBUG_NBREAKS 16
#define OZ_DEBUG_SIZE (128+16*OZ_DEBUG_NBREAKS)

#define OZ_DEBUG_MD(maproto,namestr,name) namestr, ((OZ_Pointer)&(maproto -> name)) - (OZ_Pointer)(maproto), sizeof maproto -> name

typedef struct {
	const char *name;	/* name of mchargs/mchargx register (excluding the %) */
	OZ_Pointer offs;	/* byte offset of register in mchargs/mchargx struct */
	int size;		/* size in bytes of register in mchargs/mchargx struct */
} OZ_Debug_mchargsdes;

typedef struct {
	Long (*getcur) (void *cp);								/* get current 'cpu' index */
	void (*print) (void *cp, char *fmt, ...);						/* print a message on console */
	void (*abort) (void *cp, OZ_Sigargs *sigargs, OZ_Mchargs *mchargs, void *mchargx);	/* internal error abort */
	int (*readbpt) (void *cp, OZ_Breakpoint *bptaddr, OZ_Breakpoint *oldcontents);		/* read breakpoint (not known to be readable) */
	char *(*writebpt) (void *cp, OZ_Breakpoint *bptaddr, OZ_Breakpoint opcode);		/* write breakpoint (not known to be readable) */
	void (*halt) (void *cp);								/* halt all other 'cpu''s */
	int (*debugchk) (void *cp);								/* polling routine to check for interrupt */
	int (*getcon) (void *cp, uLong size, char *buff);					/* read command line from console */
	void (*printaddr) (void *cp, void *addr);						/* print an address as nicely as possible */
	int (*printinstr) (void *cp, uByte *pc);						/* print an instruction as nicely as possible */
												/* (only first byte known to be readable) */
        int (*cvtsymbol) (void *cp, char *name, OZ_Pointer *symaddr, int *symsize);		/* convert symbol to address */
	int (*readmem) (void *cp, void *buff, uLong size, void *addr);				/* read memory (not known to be readable) */
	int (*writemem) (void *cp, void *buff, uLong size, void *addr);				/* write memory (not known to be writable) */
	void (*lockbreak) (void *cp);								/* lock breakpoint data */
	void (*lockprint) (void *cp);								/* lock printing data */
	void (*unlkbreak) (void *cp);								/* unlock breakpoint data */
	void (*unlkprint) (void *cp);								/* unlock printing data */
	void (*suspend) (void *cp);								/* cpu is in halt mode, wait for a resume */
	void (*resume) (void *cp);								/* resume all suspended cpus */
	const OZ_Debug_mchargsdes *mchargsdes;							/* standard machine arguments descriptor table */
	const OZ_Debug_mchargsdes *mchargxdes;							/* extended machine arguments descriptor table */
	int mchargxsiz;										/* size of extended machine arguments struct */
} OZ_Debug_callback;

int oz_sys_debug_init (const char *prompt, const OZ_Debug_callback *cb, void *cp, int dc_size, OZ_Debug *dc);
int oz_sys_debug_exception (const char *prompt, OZ_Debug *dc, OZ_Sigargs *sigargs, OZ_Mchargs *mchargs, void *mchargx);
void oz_sys_debug_halted (const char *prompt, OZ_Debug *dc, OZ_Mchargs *mchargs, void *mchargx);

#endif
