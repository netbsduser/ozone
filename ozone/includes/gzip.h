//+++2002-03-18
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
//---2002-03-18

/************************************************************************/
/*									*/
/*  Callable version of gzip 						*/
/*									*/
/************************************************************************/

#ifndef _GZIP_H
#define _GZIP_H

int gzip (int (*readroutine)     (void *param, int siz, char *buf, int *len, char **pnt), 
          int (*writeroutine)    (void *param, int siz, char *buf), 
          void (*errorroutine)   (void *param, int code, char *msg), 
          void *(*mallocroutine) (void *param, int size), 
          void (*freeroutine)    (void *param, void *buff), 
          void *callparam, 
          int funcode, 
          int complevel);

/* Function codes */

#define GZIP_FUNC_DUMMY    (0)
#define GZIP_FUNC_COMPRESS (1)
#define GZIP_FUNC_EXPAND   (2)

/* Return values from GZIP  -                 */
/*    OK - everything was successful          */
/* RDERR - readroutine returned error status  */
/* WTERR - writeroutine returned error status */
/* ERROR - errorroutine was called            */

#define GZIP_RTN_OK    (0)
#define GZIP_RTN_RDERR (1)
#define GZIP_RTN_WTERR (2)
#define GZIP_RTN_ERROR (3)

/* Error codes passed to errorroutine */

#define GZIP_ERROR_BADPAKLVL   (1)
#define GZIP_ERROR_UNKNMETH    (2)
#define GZIP_ERROR_INNOTGZIP   (3)
#define GZIP_ERROR_UNKNFLAGS   (4)
#define GZIP_ERROR_INCLITREE   (5)
#define GZIP_ERROR_INCDISTREE  (6)
#define GZIP_ERROR_BLKVANSHD   (7)
#define GZIP_ERROR_OUTOFMEM    (8)
#define GZIP_ERROR_INVCOMPDAT  (9)
#define GZIP_ERROR_BUGCHECK   (10)
#define GZIP_ERROR_CRCERROR   (11)
#define GZIP_ERROR_LENERROR   (12)
#define GZIP_ERROR_COMPEOF    (13)
#define GZIP_ERROR_BADFUNCODE (14)

#endif
