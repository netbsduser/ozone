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

#ifndef _OZ_UTIL_START_H
#define _OZ_UTIL_START_H

#include "ozone.h"
#include "oz_knl_handle.h"

extern OZ_Handle oz_util_h_console;		/* points to OZ_CONSOLE */
extern OZ_Handle oz_util_h_error;		/* points to OZ_ERROR */
extern OZ_Handle oz_util_h_input;		/* points to OZ_INPUT */
extern OZ_Handle oz_util_h_output;		/* points to OZ_OUTPUT */

uLong oz_util_main (int argc, char *argv[]);	/* argv = from OZ_PARAMS */

#endif
