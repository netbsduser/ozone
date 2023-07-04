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

#ifndef _OZ_SYS_HASH_H
#define _OZ_SYS_HASH_H

#define OZ_SYS_HASH_BINSIZE (16)	/* binary hash size (bytes) */
#define OZ_SYS_HASH_CTXSIZE (128)	/* hashing context block size */
#define OZ_SYS_HASH_STRSIZE (36)	/* printable string size (incl null) */

void oz_sys_hash_init (void *hctxv);
void oz_sys_hash_data (void *hctxv, int count, const void *buffer);
void oz_sys_hash_term (void *hctxv, uByte digest[OZ_SYS_HASH_BINSIZE]);
void oz_sys_hash (int count, const void *buffer, uByte digest[OZ_SYS_HASH_BINSIZE]);
void oz_sys_hash_bin2str (uByte hash[OZ_SYS_HASH_BINSIZE], char buff[OZ_SYS_HASH_STRSIZE]);
int oz_sys_hash_str2bin (const char buff[OZ_SYS_HASH_STRSIZE], uByte hash[OZ_SYS_HASH_BINSIZE]);

#endif
