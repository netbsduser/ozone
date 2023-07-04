//+++2003-03-01
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
//---2003-03-01

#ifndef _OZ_CRTL_FD_H
#define _OZ_CRTL_FD_H

#include "oz_knl_handle.h"

#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct { int (*fxstat) (int ver, int fildes, struct stat *buf);
                 off_t (*lseek) (int fd, off_t offset, int whence);
                 ssize_t (*read) (int fd, __ptr_t buf, size_t nbytes);
                 ssize_t (*write) (int fd, const __ptr_t buf, size_t nbytes);
                 int (*vioctl) (int fd, int request, va_list ap);
                 int (*close) (int fd);
               } OZ_Crtl_fd_driver;

typedef struct { OZ_Handle h_iochan;
                 OZ_Handle h_event;
                 const OZ_Crtl_fd_driver *driver;
                 int flags1, flags2;
                 void *point1, *point2;
                 char allocated;
                 char append;
                 char isatty;
               } OZ_Crtl_fd_array;

extern OZ_Crtl_fd_array *oz_crtl_fd_array;

int oz_crtl_fd_alloc (void);
int oz_crtl_fd_check (int fd);
int oz_crtl_fd_free (int fd);

#endif
