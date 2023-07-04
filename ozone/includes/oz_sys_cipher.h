//+++2002-08-17
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
//---2002-08-17

#ifndef _OZ_SYS_CIPHER_H
#define _OZ_SYS_CIPHER_H

#define OZ_SYS_CIPHER_KEYSIZE 16
#define OZ_SYS_CIPHER_BLKSIZE 8

extern const int oz_sys_cipher_ctxsize;
void oz_sys_cipher_encinit (uByte uk[OZ_SYS_CIPHER_KEYSIZE], void *ekv);
void oz_sys_cipher_decinit (uByte uk[OZ_SYS_CIPHER_KEYSIZE], void *dkv);
void oz_sys_cipher_encrypt (void *ekv, uByte feedback[OZ_SYS_CIPHER_BLKSIZE], int len, void *in, void *out);
void oz_sys_cipher_decrypt (void *dkv, uByte feedback[OZ_SYS_CIPHER_BLKSIZE], int len, void *in, void *out);

#endif
