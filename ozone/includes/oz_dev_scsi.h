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

#ifndef _OZ_DEV_SCSI_H
#define _OZ_DEV_SCSI_H

#include "oz_io_scsi.h"
#include "oz_knl_devio.h"

typedef struct { OZ_Iochan *ctrl_iochan;
                 uLong inqlen;
                 char *inqbuf;
                 char *unit_devname;
                 char *unit_devdesc;
               } OZ_Scsi_init_pb;

typedef OZ_Devunit *(*OZ_Scsi_init_entry) (void *param, OZ_Scsi_init_pb *pb);

uLong oz_dev_scsi_cvtdoio2pp (OZ_Ioop *ioop, OZ_Procmode procmode, OZ_IO_scsi_doio *doio, OZ_IO_scsi_doiopp *doiopp);
void oz_dev_scsi_scan (OZ_Devunit *ctrl_devunit, uLong max_scsi_id);
OZ_Devunit *oz_dev_scsi_auto (void *prefix, OZ_Devunit *host_devunit, const char *devname, const char *suffix);
OZ_Devunit *oz_dev_scsi_scan1 (OZ_Devunit *ctrl_devunit, uLong scsi_id);
void oz_dev_scsi_class_add (OZ_Scsi_init_entry entry, void *param);
void oz_dev_scsi_class_rem (OZ_Scsi_init_entry entry, void *param);

#endif
