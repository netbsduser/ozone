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

/************************************************************************/
/*									*/
/*  This structure is built from the loadparams.dat file by the loader 	*/
/*  program when the system is booted.					*/
/*									*/
/************************************************************************/

#ifndef _OZ_LDR_PARAMS_H
#define _OZ_LDR_PARAMS_H

#include "oz_knl_hw.h"

	/* Param block must be at most 4096 bytes long */

#define OZ_LDR_PARAMS_SIZ 4096

typedef struct {

	char load_device[96];		/* device name the system was booted from, eg, "floppy.p0" */
	char load_dir[32];		/* directory the system was booted from, eg, "/ozone/binaries/" */
	char load_fstemp[32];		/* boot device filesystem template device, eg, "oz_dfs" */
	char load_script[32];		/* loader script name, eg, "startup.ldr" */

	char kernel_image[32];		/* kernel image name, eg, "oz_kernel_486.oz" */

	char startup_image[32];		/* startup image name, eg, "oz_cli.oz" */
	char startup_input[32];		/* startup input file, eg, "startup.cli" */
	char startup_output[32];	/* startup output file, eg, "console:" */
	char startup_error[32];		/* startup error file, eg, "console:" */
	char startup_params[64];	/* startup param string, eg, "full" */

	uLong kernel_stack_size;	/* kernel stack size (in bytes) */

	uLong def_user_stack_size;	/* default user stack size (in bytes) */

        uLong clock_rate;		/* 0 : measure clockrate at startup; else : this is the clock rate */

	uLong nonpaged_pool_size;	/* non-paged pool size (in bytes) */
	uLong system_pages;		/* total number of system pages */
	uLong cpu_disable;		/* disable these cpus (bitmask of cpu numbers) */
	uLong debug_init;		/* do a breakpoint at beg of oz_knl_boot_firstcpu */
	uLong knl_exception;		/* 0 : enter debugger; 1 : write crashdump; 2 : immediate reboot */

	Long tz_offset_rtc;		/* timezone offset for reading or writing RTC */
					/* (eg, -5*3600 if RTC is eastern standard time) */
	Long dummy1;

	uLong uniprocessor;		/* 0 : smp capable kernel; 1 : uniprocessor kernel */
	uLong memory_megabytes;		/* 0 : compute memory size; else : this is the size of physical memory */
	uLong monochrome;		/* 0 : color monitor; 1 : monochrome monitor */

        char signature[32];		/* make this boot block unique on all volumes */
					/* (yyyy-mm-dd@hh:mm:ss.fffffff it was written) */

	char extras[3596];		/* fills to end of 4096-byte block */
					/* contains list of "name=value" strings */
					/* that can be set by loader 'extra' commands */
					/* that can be retrieved with oz_knl_extra_get* routines */

} OZ_Loadparams;

extern OZ_Loadparams oz_ldr_paramblock;

typedef enum { ptype_string, 
               ptype_ulong,
               ptype_long,
               ptype_pointer
             } OZ_Loadparamtype;

typedef struct { const char *pname;	/* parameter name string */
                 int   pmods;		/* 0 = no mod allowed via console, 1 = mod allowed via console */
                 OZ_Loadparamtype ptype; /* parameter type */
                 const char *pdflt;	/* parameter default value string */
                 int   psize;		/* size (in bytes) of parameter */
                 void *paddr;		/* parameter address in loadparams */
               } OZ_Loadparamtable;


extern const OZ_Loadparamtable oz_ldr_paramtable[];

#endif
