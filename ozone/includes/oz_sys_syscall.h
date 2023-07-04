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
/*  This header file contains all the 'syscall' routine codes		*/
/*									*/
/************************************************************************/

#ifndef _OZ_SYSCALL_SYSCALL_H
#define _OZ_SYSCALL_SYSCALL_H

#include "ozone.h"

#define OZ_SYSCALL_thread_deqast 0
#define OZ_SYSCALL_thread_setast 1
#define OZ_SYSCALL_thread_exiteh 2
#define OZ_SYSCALL_event_create 3
#define OZ_SYSCALL_event_inc 4
#define OZ_SYSCALL_event_set 5
#define OZ_SYSCALL_event_wait 6
#define OZ_SYSCALL_event_nwait 7
#define OZ_SYSCALL_io_assign 8
#define OZ_SYSCALL_io_abort 9
#define OZ_SYSCALL_io_start 10
#define OZ_SYSCALL_process_create 11
#define OZ_SYSCALL_process_mapsection 12
#define OZ_SYSCALL_handle_release 13
#define OZ_SYSCALL_section_create 14
#define OZ_SYSCALL_thread_create 15
#define OZ_SYSCALL_image_load 16
#define OZ_SYSCALL_thread_getcondhand 17
#define OZ_SYSCALL_condhand_delframe 18
#define OZ_SYSCALL_spawn 19
#define OZ_SYSCALL_logname_create 20
#define OZ_SYSCALL_callknl 21
#define OZ_SYSCALL_logname_getattr 22
#define OZ_SYSCALL_condhand_create 23
#define OZ_SYSCALL_event_ast 24
#define OZ_SYSCALL_logname_creobj 25
#define OZ_SYSCALL_iochan_getunitname 26
#define OZ_SYSCALL_iochan_getclassname 27
#define OZ_SYSCALL_process_getid 28
#define OZ_SYSCALL_logname_lookup 29
#define OZ_SYSCALL_logname_getobj 30
#define OZ_SYSCALL_logname_getval 31
#define OZ_SYSCALL_thread_abort 32
#define OZ_SYSCALL_thread_getexitsts 33
#define OZ_SYSCALL_logname_gettblent 34
#define OZ_SYSCALL_logname_delete 35
#define OZ_SYSCALL_thread_getexitevent 36
#define OZ_SYSCALL_thread_suspend 37
#define OZ_SYSCALL_thread_resume 38
#define OZ_SYSCALL_thread_getname 39
#define OZ_SYSCALL_handle_getinfo 52
#define OZ_SYSCALL_process_unmapsec 53
#define OZ_SYSCALL_handle_next 54
#define OZ_SYSCALL_gettimezonex 55
#define OZ_SYSCALL_process_getsecfromvpage 56
#define OZ_SYSCALL_section_setpageprot 57
#define OZ_SYSCALL_ast 58
#define OZ_SYSCALL_exhand_create 59
#define OZ_SYSCALL_thread_getbyid 60
#define OZ_SYSCALL_iosel_start 61
#define OZ_SYSCALL_thread_setseckeys 62
#define OZ_SYSCALL_thread_setdefcresecattr 63
#define OZ_SYSCALL_thread_setsecattr 64
#define OZ_SYSCALL_password_change 65
#define OZ_SYSCALL_thread_setbasepri 66
#define OZ_SYSCALL_handle_setthread 67
#define OZ_SYSCALL_thread_queueast 68
#define OZ_SYSCALL_process_makecopy 69
#define OZ_SYSCALL_thread_orphan 70
#define OZ_SYSCALL_io_alloc 71
#define OZ_SYSCALL_io_realloc 72
#define OZ_SYSCALL_io_dealloc 73
#define OZ_SYSCALL_job_create 74
#define OZ_SYSCALL_event_setimint 75
#define OZ_SYSCALL_process_getbyid 76
#define OZ_SYSCALL_process_mapsections 77
#define OZ_SYSCALL_io_chancopy 78
#define OZ_SYSCALL_tzconv 79
#define OZ_SYSCALL_io_wait 80
#define OZ_SYSCALL_io_waitagain 81
#define OZ_SYSCALL_io_waitsetef 82
#define OZ_SYSCALL_handle_retrieve 83

#define OZ_SYSCALL_MAX 84

#endif
