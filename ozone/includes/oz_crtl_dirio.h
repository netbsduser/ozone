//+++2001-10-06
//    Copyright (C) 2001, Mike Rieker, Beverly, MA USA
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
//---2001-10-06

#ifndef _OZ_CRTL_DIRIO_H
#define _OZ_CRTL_DIRIO_H

#ifdef _OZ_CRTL_DIRIO_C
typedef struct DIR DIR;
#else
typedef void DIR;
#endif

struct dirent { char *d_name; };

DIR *opendir (const char *name);
struct dirent *readdir (DIR *dir);
int closedir (DIR *dir);

#endif
