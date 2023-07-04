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

#include <stdio.h>

long int xxstrtol_internal (const char *nptr, char **endptr, int base, int dummy)

{
  char c;
  const char *p;
  int negative;
  long int accum, newaccum;
  unsigned long int pmo;

  pmo = 0;								/* largest possible integer value */
  pmo --;
  pmo /= 2;
  p = nptr;
  while ((c = *p) != 0 && (c <= ' ')) p ++;				/* skip leading spaces */
  negative = 0;								/* assume result is positive */
  if ((c == '+') || (c == '-')) {					/* check for a leading sign */
    if (c == '-') negative = 1;						/* if a -, set negative flag */
    c = *(++ p);							/* get the following char */
  }
  if ((base == 0) || (base == 16)) {					/* if base 0 or 16, ... */
    if ((c == '0') && ((p[1] == 'x') || (p[1] == 'X'))) {		/* check for leading 0x or 0X */
      base = 16;							/* in which case force base 16 */
      p += 2;								/* skip over the 0x or 0X */
      c = *p;								/* get the following char */
    }
  }
  if ((base == 0) && (c == '0')) base = 8;				/* if base 0 and begins with a '0', force base 8 */
  if (base == 0) base = 10;						/* if still no base, use base 10 */
  pmo /= base;								/* pre-multiply overflow value */
  accum = 0;								/* clear the accumulator */
  while (accum <= pmo) {						/* make sure we can possibly take another digit */
    if ((c >= '0') && (c <= '9') && (c < '0' + base)) {			/* see if 0..9 and within range of base */
      newaccum = accum * base + (c - '0');				/* ok, stuff in accumulator */
    } else if ((base > 10) && (c >= 'a') && (c < 'a' + base - 10)) {	/* maybe the base requires a letter */
      newaccum = accum * base + (c - 'a' + 10);
    } else if ((base > 10) && (c >= 'A') && (c < 'A' + base - 10)) {	/* ... allow capitals, too */
      newaccum = accum * base + (c - 'A' + 10);
    } else break;							/* doesn't convert, done scanning */
    if (newaccum <= accum) break;					/* stop if overflow */
    c = *(++ p);							/* ok, get next character */
    accum = newaccum;
  }
  if (endptr != NULL) *endptr = p;					/* maybe return pointer to first unconverted char */
  if (negative) accum = - accum;					/* maybe result is negative */
  return (accum);							/* return result */
}

unsigned long int xxstrtoul_internal (const char *nptr, char **endptr, int base, int dummy)

{
  char c;
  const char *p;
  unsigned long int accum, newaccum, pmo;

  pmo = 0;								/* largest possible integer value */
  pmo --;
  p = nptr;
  while ((c = *p) != 0 && (c <= ' ')) p ++;				/* skip leading spaces */
  if ((base == 0) || (base == 16)) {					/* if base 0 or 16, ... */
    if ((c == '0') && ((p[1] == 'x') || (p[1] == 'X'))) {		/* check for leading 0x or 0X */
      base = 16;							/* in which case force base 16 */
      p += 2;								/* skip over the 0x or 0X */
      c = *p;								/* get the following char */
    }
  }
  if ((base == 0) && (c == '0')) base = 8;				/* if base 0 and begins with a '0', force base 8 */
  if (base == 0) base = 10;						/* if still no base, use base 10 */
  pmo /= base;								/* pre-multiply overflow value */
  accum = 0;								/* clear the accumulator */
  while (accum <= pmo) {						/* make sure we can possibly take another digit */
    if ((c >= '0') && (c <= '9') && (c < '0' + base)) {			/* see if 0..9 and within range of base */
      newaccum = accum * base + (c - '0');				/* ok, stuff in accumulator */
    } else if ((base > 10) && (c >= 'a') && (c < 'a' + base - 10)) {	/* maybe the base requires a letter */
      newaccum = accum * base + (c - 'a' + 10);
    } else if ((base > 10) && (c >= 'A') && (c < 'A' + base - 10)) {	/* ... allow capitals, too */
      newaccum = accum * base + (c - 'A' + 10);
    } else break;							/* doesn't convert, done scanning */
    if (newaccum <= accum) break;					/* stop if overflow */
    c = *(++ p);							/* ok, get next character */
    accum = newaccum;
  }
  if (endptr != NULL) *endptr = p;					/* maybe return pointer to first unconverted char */
  return (accum);							/* return result */
}

int main ()

{
  char buff[1024], *p;
  unsigned long int value;

  while (gets (buff) != NULL) {
    value = xxstrtoul_internal (buff, &p, 0, 0);
    printf ("value %u, p %s\n", value, p);
  }
  return (0);
}
