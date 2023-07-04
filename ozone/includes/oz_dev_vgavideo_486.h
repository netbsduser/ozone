//+++2003-03-01
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
//---2003-03-01

/************************************************************************/
/*									*/
/*  Global data and routines provided by oz_dev_vgavideo_486.c for use 	*/
/*  by oz_dev_console_486.c						*/
/*									*/
/************************************************************************/

#ifndef _OZ_DEV_VGAVIDEO_486_H
#define _OZ_DEV_VGAVIDEO_486_H

#include "oz_io_console.h"

typedef struct OZ_Vctx OZ_Vctx;

extern int oz_dev_video_multicpu;		/* 0: output uniprocessor style; 1: output multiprocessor style */

extern int oz_dev_video_mode_cursor_key;	/* 0: send <esc>[x, 1: send <esc>Ox for arrow keys */
extern int oz_dev_video_mode_keypad_app;	/* 0: send numeric keypad codes, 1: send escape keypad codes */
extern int oz_dev_video_mode_local_echo;	/* 0: remote echo, 1: local echo */
extern int oz_dev_video_mode_newline;		/* 0: return key sends just return and lf/ff/vt stay in same column */
						/* 1: return key sends cr and lf, and lf/ff/vt reset to first column */

void oz_dev_video_init (void);					/* boot-time initialization routine */
OZ_Vctx *oz_dev_video_initctx (void *devexv, int vidx);		/* init other displays */
OZ_Vctx *oz_dev_video_switch (OZ_Vctx *vctx);			/* switch video screens */
void oz_dev_video_setmode (OZ_Vctx *vctx, uLong size, OZ_Console_modebuff *buff); /* set modes */
void oz_dev_video_getscreen (OZ_Vctx *vctx, uLong size, OZ_Console_screenbuff *buff); /* get screen contents */
void oz_dev_video_putstring (OZ_Vctx *vctx, uLong size, const char *buff); /* write string to screen */
void oz_dev_video_updcursor (OZ_Vctx *vctx);			/* update cursor position on screen */
void oz_dev_video_putchar (OZ_Vctx *vctx, char c);		/* write single character to screen */
void oz_dev_video_linedn (void);				/* used by control-shift-J to move down a line */
void oz_dev_video_lineup (void);				/* used by control-shift-U to move up a line */
void oz_dev_vgavideo_blank (int blank);				/* blank or unblank screen */

#endif
