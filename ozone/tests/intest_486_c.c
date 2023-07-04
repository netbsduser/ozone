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
/*  Keyboard interrupt test routine.  It tests the status register 	*/
/*  then reads and deciphers the scan code and translates it to ascii.	*/
/*									*/
/************************************************************************/

typedef unsigned char uByte;
typedef char Byte;
typedef unsigned short uWord;

#define KB_CP 0x64	/* keyboard command port */
#define KB_DP 0x60	/* keyboard data port */

#define VIDEOMEM ((uWord *)(0xB8000))
#define VIDEOLINESIZE 80
#define VIDEOPAGESIZE (25*80)

/************************************************************************/
/*									*/
/*  Keyboard scan code translation tables				*/
/*									*/
/************************************************************************/

static uByte keyboard_offs = 0;		/* <6> = 0 : ctrl key released */
					/*       1 : ctrl key pressed */
					/* <7> = 0 : shift key released */
					/*       1 : shift key pressed */

static uByte keyboard_e0 = 0;		/* 0 : last keycode was not E0 */
					/* 1 : last keycode was E0 */

/* Single character scancode translation */

#define RSH -1		/* right shift key */
#define LSH -1		/* left shift key (treat same as right) */
#define LAL  0		/* left alt key (ignore) */
#define CL   0		/* caps lock (ignore) */
#define CTR -2		/* ctrl key */
#define CSC -3		/* control-shift-C (call debugger) */
#define CSD -4		/* control-shift-D (enter diag mode) */
#define CSU -5		/* control-shift-U (scroll up a line) */
#define CSJ -6		/* control-shift-J (scroll down a line) */
#define KPS -7		/* keypad star (return PF3 multibyte) */
#define CSL -8		/* control-shift-L (login console) */
#define CS0 -10		/* control-shift-0 or -~ (select screen 0) */
#define CS1 -11		/* control-shift-1 (select screen 1) */
#define CS2 -12		/* control-shift-2 (select screen 2) */
#define CS3 -13		/* control-shift-3 (select screen 3) */
#define CS4 -14		/* control-shift-4 (select screen 4) */
#define CS5 -15		/* control-shift-5 (select screen 5) */
#define CS6 -16		/* control-shift-6 (select screen 6) */
#define CS7 -17		/* control-shift-7 (select screen 7) */
#define CS8 -18		/* control-shift-8 (select screen 8) */
#define CS9 -19		/* control-shift-9 (select screen 9) */

	/*0  x1  x2  x3  x4  x5  x6  x7  x8  x9  xA  xB  xC  xD  xE  xF*/

static const Byte keyboard_table[256] = {

	/* shift up & ctrl up */

	  0, 27,'1','2','3','4','5','6','7','8','9','0','-','=',127,  9,		/* 0x                                                      */
	'q','w','e','r','t','y','u','i','o','p','[',']', 13,CTR,'a','s',		/* 1x  cr=carriage return; ctr=ctrl-key                    */
	'd','f','g','h','j','k','l',';', 39,'`',LSH,92, 'z','x','c','v',		/* 2x  lsh=left-shift-key; 39=apostrophe, 92=backslash     */
	'b','n','m',',','.','/',RSH,KPS,LAL,' ', CL, 0,   0,  0,  0,  0,		/* 3x  rsh=right-shift-key; cl=caps lock; lal=left-alt-key */

	/* shift up & ctrl down */

	  0, 27,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8,  9,		/* 0x                                                      */
	 17, 23,  5, 18, 20, 25, 21,  9, 15, 16, 27, 29, 13,CTR,  1, 19,		/* 1x  cr=carriage return; ctr=ctrl-key                    */
	  4,  6,  7,  8, 10, 11, 12,  0,  0,  0,LSH, 28, 26, 24,  3, 22,		/* 2x  lsh=left-shift-key                                  */
	  2, 14, 13,',','.','/',RSH,KPS,LAL,  0, CL,  0,  0,  0,  0,  0,		/* 3x  rsh=right-shift-key; cl=caps lock; lal=left-alt-key */

	/* shift down & ctrl up */

	  0, 27,'!','@','#','$','%','^','&','*','(',')','_','+',127,  9,		/* 0x                                                      */
	'Q','W','E','R','T','Y','U','I','O','P','{','}', 13,CTR,'A','S',		/* 1x  return; ctr=ctrl-key                                */
	'D','F','G','H','J','K','L',':','"','~',LSH,'|','Z','X','C','V',		/* 2x  lsh=left-shift-key                                  */
	'B','N','M','<','>','?',RSH,KPS,LAL,' ', CL,  0,  0,  0,  0,  0,		/* 3x  rsh=right-shift-key; cl=caps lock; lal=left-alt-key */

	/* shift down & ctrl down */

	  0,  0,CS1,CS2,CS3,CS4,CS5,CS6,CS7,CS8,CS9,CS0,  0,  0,  8,  9,		/* 0x                                                      */
	  0,  0,  0,  0,  0,  0,CSU,  0,  0,  0,  0,  0,  0,CTR,  0,  0,		/* 1x  return; ctr=ctrl-key                                */
	CSD,  0,  0,  0,CSJ,  0,CSL,  0,  0,CS0,LSH,  0,  0,  0,CSC,  0,		/* 2x  lsh=left-shift-key; csc=control-shift-C             */
	  0,  0,  0,  0,  0,  0,RSH,KPS,LAL,  0, CL,  0,  0,  0,  0,  0			/* 3x  rsh=right-shift-key; cl=caps lock; lal=left-alt-key */
	};

/************************************************************************/
/*									*/
/*  Get keyboard char from interface chip				*/
/*									*/
/*    Input:								*/
/*									*/
/*	ignmb = 0 : process multibyte sequence in progress or possibly start new one
/*	        1 : flush any multibyte sequence in progress and don't start new one
/*									*/
/*    Output:								*/
/*									*/
/*	oz_dev_keyboard_getc = 0 : no character available		*/
/*	                     > 0 : ascii key code			*/
/*	                     < 0 : special key code			*/
/*									*/
/************************************************************************/

char keyboard_getc (void)

{
  Byte bl;
  uByte al;

  /* Get scan code from keyboard */

retry:
  al = oz_hw_inb (KB_CP);			/* check the command port */
  if ((al & 0x21) != 0x01) return (0);		/* if no low bit, no keyboard char present */
						/* ... also make sure there is no mouse bit set */
  al = oz_hw_inb (KB_DP);			/* ok, read the keyboard char */

  /* Ignore keypad area of keyboard and E0 codes */

  if (al & 0x40) goto retry;

  /* Use single character translation table */

  bl = keyboard_table[(al&0x3F)|keyboard_offs];	/* ok, translate given current shift/ctrl state */
  if (bl == 0) goto retry;			/* if 0 entry, ignore it & try again */
  if (bl < 0) goto special;			/* if neg, it is a special code */
  if (al & 0x80) goto retry;			/* if key-up, ignore and try again */
  return (bl);					/* ok, just return the character itself */

special:
  switch (bl) {
    case LSH: {
      al = (al & 0x80) ^ 0x80;				/* shift key, mask and invert the key-up bit */
      keyboard_offs = (keyboard_offs & 0x7F) | al;	/* store in keyboard_offs<7> */
      break;
    }
    case CTR: {
      al = (al & 0x80) ^ 0x80;				/* control key, mask and invert the key-up bit */
      keyboard_offs = (keyboard_offs & 0xBF) | (al >> 1); /* store in keyboard_offs<6> */
      break;
    }
    default: { /* control-shift-C, -D, -L, etc */
      if (al & 0x80) goto retry;			/* ignore key-up */
      return (bl);					/* return the code to caller */
    }
  }
  goto retry;
}

/************************************************************************/
/*									*/
/*  Output a character to screen					*/
/*									*/
/************************************************************************/

void oz_dev_video_putchar (uByte c)

{
  int i;
  uWord cursor, videoio;

  videoio = 0x3B4;
  if (oz_hw_inb (0x3CC) & 1) videoio = 0x3D4;

  oz_hw_outb (0x0E, videoio);
  cursor  = oz_hw_inb (videoio + 1) << 8;
  oz_hw_outb (0x0F, videoio);
  cursor += oz_hw_inb (videoio + 1);

  if (c == '\n') cursor = ((cursor / VIDEOLINESIZE) + 1) * VIDEOLINESIZE;

  if (cursor < VIDEOLINESIZE*12) cursor = VIDEOLINESIZE*12;
  while (cursor >= VIDEOPAGESIZE) {
    for (i = VIDEOLINESIZE*12; i < VIDEOPAGESIZE-VIDEOLINESIZE; i ++) VIDEOMEM[i] = VIDEOMEM[i+VIDEOLINESIZE];
    cursor -= VIDEOLINESIZE;
    while (i < VIDEOPAGESIZE) VIDEOMEM[i++] = 0x0400;
  }

  if (c >= ' ') VIDEOMEM[cursor++] = 0x0400 | c;

  oz_hw_outb (0x0E, videoio);
  oz_hw_outb (cursor >> 8, videoio + 1);
  oz_hw_outb (0x0F, videoio);
  oz_hw_outb (cursor, videoio + 1);
}
