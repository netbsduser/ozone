//+++2001-10-24
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
//---2001-10-24

#ifndef _OZ_UTIL_FILESEL_H
#define _OZ_UTIL_FILESEL_H

typedef struct OZ_Filesel OZ_Filesel;

const char *const oz_util_filesel_usage;

OZ_Filesel *oz_util_filesel_init (void);
const char *oz_util_filesel_errmsg (OZ_Filesel *filesel);
void oz_util_filesel_term (OZ_Filesel *filesel);
int oz_util_filesel_parse (OZ_Filesel *filesel, int argc, char *argv[]);
uLong oz_util_filesel_check (OZ_Filesel *filesel, OZ_Handle h_file, const char *fspec);

#endif
