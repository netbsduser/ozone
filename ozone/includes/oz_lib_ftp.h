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

#ifndef _OZ_LIB_FTP_H
#define _OZ_LIB_FTP_H

typedef struct OZ_Ftp_ctx OZ_Ftp_ctx;

#include "oz_io_ip.h"

OZ_Ftp_ctx *oz_lib_ftp_init (uByte remipaddr[OZ_IO_IP_ADDRSIZE], uByte remportno[OZ_IO_IP_PORTSIZE], 
                             uByte lclportno[OZ_IO_IP_PORTSIZE], const char *defaultdir);
void oz_lib_ftp_term (OZ_Ftp_ctx *ftpctx);
void oz_lib_ftp_set_defaultdir (OZ_Ftp_ctx *ftpctx, const char *defaultdir);
void oz_lib_ftp_set_transmode (OZ_Ftp_ctx *ftpctx, char transmode);
void oz_lib_ftp_set_dataport (OZ_Ftp_ctx *ftpctx, uByte ipaddr[OZ_IO_IP_ADDRSIZE], uByte portno[OZ_IO_IP_PORTSIZE]);
void oz_lib_ftp_set_filestruct (OZ_Ftp_ctx *ftpctx, char filestruct);
void oz_lib_ftp_set_reptype (OZ_Ftp_ctx *ftpctx, char reptype, char repsubtype);
void oz_lib_ftp_retrieve (OZ_Ftp_ctx *ftpctx, 
                          const char *filename, 
                          void (*astentry) (void *astparam, const char *complmsg), 
                          void *astparam);
void oz_lib_ftp_store (OZ_Ftp_ctx *ftpctx, 
                       const char *filename, 
                       void (*astentry) (void *param, const char *complmsg), 
                       void *astparam);

#endif
