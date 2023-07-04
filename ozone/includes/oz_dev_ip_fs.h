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

#ifndef _OZ_DEV_IP_FS_H
#define _OZ_DEV_IP_FS_H

#include "oz_io_fs.h"

typedef struct { Long seq;	/* request's sequence number */
                 uLong func;	/* OZ_IO_FS_... function code */
                 uLong handle;	/* handle indicating file open on channel (as returned in rpl.u.fs_open.handle) */
                 union { struct { char name[256]; } open;
                         struct { uLong size; uLong svbn; } readblocks;
                         struct { uLong size; uLong trmsize; uLong pmtsize; uLong atblock; uLong atbyte; char pmtbuff[256]; char trmbuff[16]; } readrec;
                         struct { int init; char wild[256]; } wildscan;
                         struct { uLong atblock; uLong atbyte; } setcurpos;
                       } u;
               } OZ_Ip_fs_req;

typedef struct { Long seq;	/* reply's sequence number */
                 uLong status;	/* disk I/O status */
                 union { struct { uLong handle; } open;
                         struct { uLong rlen; uByte buff[4096]; } readblocks;
                         struct { uLong rlen; uByte buff[4096]; } readrec;
                         struct { OZ_Datebin access_date; OZ_Datebin change_date; OZ_Datebin modify_date; uLong blocksize; uLong eofblock; uLong eofbyte; uLong hiblock; uLong curblock; uLong curbyte; uLong filattrflags; } getinfo1;
                         struct { char name[256]; } readdir;
                         struct { uLong handle; char spec[256]; } wildscan;
                       } u;
               } OZ_Ip_fs_rpl;

#endif
