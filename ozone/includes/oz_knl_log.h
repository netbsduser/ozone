//+++2002-05-10
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
//---2002-05-10

#ifndef _OZ_KNL_LOG_H
#define _OZ_KNL_LOG_H

#ifdef _OZ_KNL_LOG_C
typedef struct OZ_Log OZ_Log;
#else
typedef void OZ_Log;
#endif

#include "oz_knl_hw.h"
#include <stdarg.h>

uLong oz_knl_log_create (const char *name, OZ_Log **log_r);
Long oz_knl_log_increfc (OZ_Log *log, Long inc);
void oz_knl_log_print (OZ_Log *log, const char *file, int line, const char *format, ...);
void oz_knl_log_vprint (OZ_Log *log, const char *file, int line, const char *format, va_list ap);
void oz_knl_log_wait (OZ_Log *log);
uLong oz_knl_log_remove (OZ_Log *log, Long *lostlines_r, OZ_Datebin *datebin_r, 
                         const char **file_r, int *line_r, int size, char *buffer);

#endif
