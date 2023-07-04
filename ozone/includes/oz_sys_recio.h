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

#ifndef _OZ_SYS_RECIO_H
#define _OZ_SYS_RECIO_H

#include "ozone.h"

#ifdef _OZ_RECIO_C
typedef struct OZ_Recio_chnex OZ_Recio_chnex;
typedef struct OZ_Recio_filex OZ_Recio_filex;
#else
typedef void OZ_Recio_chnex;
typedef void OZ_Recio_filex;
#endif

/* Callback table */

typedef struct { uLong (*(extend)) (void *chnex, void *filex, OZ_Dbn exblk);
                 void (*(seteof)) (void *chnex, void *filex, OZ_Dbn efblk, uLong efbyt);
                 uLong (*(write)) (void *chnex, void *filex, OZ_Dbn vbn, uLong size, uByte *buff);
                 uLong (*(read)) (void *chnex, void *filex, OZ_Dbn vbn, uLong size, uByte *buff);
                 void *(*(malloc)) (void *chnex, void *filex, uLong size, const char *file, int line);
                 void (*(free)) (void *chnex, void *filex, void *buff);
               } OZ_Recio_call;

/* Global routine entrypoints */

OZ_Recio_filex *oz_sys_recio_initfilex (void *filex, OZ_Recio_call *call, uLong diskblksize, OZ_Dbn efblk, uLong efbyt);
void oz_sys_recio_termfilex (OZ_Recio_filex *recio_filex, OZ_Dbn *efblk_r, uLong *efbyt_r);
OZ_Recio_chnex *oz_sys_recio_initchnex (OZ_Recio_filex *recio_filex, void *chnex);
void oz_sys_recio_termchnex (OZ_Recio_chnex *recio_chnex, OZ_Recio_filex *recio_filex);
void oz_sys_recio_getcurent (OZ_Recio_chnex *recio_chnex, OZ_Recio_filex *recio_filex, OZ_Dbn *curblk_r, uLong *curbyt_r);
uLong oz_sys_recio_write (OZ_Recio_chnex *recio_chnex, OZ_Recio_filex *recio_filex, OZ_IO_fs_writerec *writerec);
uLong oz_sys_recio_read (OZ_Recio_chnex *recio_chnex, OZ_Recio_filex *recio_filex, OZ_IO_fs_readrec *readrec);
uLong oz_sys_recio_setcurpos (OZ_Recio_chnex *recio_chnex, OZ_Recio_filex *recio_filex, OZ_IO_fs_setcurpos *setcurpos);

#endif
