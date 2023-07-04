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

#include <stdio.h>

typedef unsigned long long uQuad;
typedef unsigned long uLong;
typedef unsigned short uWord;

static int hexflag;

static uQuad readquad (const char *value, const char *name);
static uQuad scale64x64s64 (uQuad n1, uQuad n2, uQuad d);

int main (int argc, char *argv[])

{
  uQuad n1, n2, d, q;

  if (argc != 5) {
    printf ("usage: scale64x64s64 h/d <n1> <n2> <d>\n");
    return (-1);
  }

  hexflag = 0;
  if (argv[1][0] == 'h') hexflag = 1;

  n1 = readquad (argv[2], "n1");
  n2 = readquad (argv[3], "n2");
  d  = readquad (argv[4], " d");

  q = scale64x64s64 (n1, n2, d);

  printf ("\n");
  if (hexflag) {
    printf ("n1: %16.16llX\n", n1);
    printf ("n2: %16.16llX\n", n2);
    printf (" d: %16.16llX\n", d);
    printf (" q: %16.16llX\n", q);
  } else {
    printf ("n1: %20lld\n", n1);
    printf ("n2: %20lld\n", n2);
    printf (" d: %20lld\n", d);
    printf (" q: %20lld\n", q);
  }
  printf ("\n");
  return (0);
}

static uQuad readquad (const char *value, const char *name)

{
  char c;
  const char *p;
  uQuad a;

loop:
  a = 0;
  for (p = value; (c = *p) != 0; p ++) {
    if ((c >= '0') && (c <= '9')) c -= '0';
    else if (hexflag && (c >= 'A') && (c <= 'F')) c -= 'A' - 10;
    else if (hexflag && (c >= 'a') && (c <= 'f')) c -= 'a' - 10;
    else {
      printf ("bad %s\n", name);
      exit (-1);
    }
    if (hexflag) a = (a * 16) + c;
            else a = (a * 10) + c;
  }
  return (a);
}

/************************************************************************/
/*									*/
/*  Scale a 64-bit unsigned by the quotient of two quads		*/
/*									*/
/*  Computes n1 * n2 / d						*/
/*									*/
/************************************************************************/

static uQuad scale64x64s64 (uQuad n1, uQuad n2, uQuad d)

{
  int i, j;
  uQuad q, r;
  uLong w1[2], w2[2], p[4];

  w1[0] = n1;
  w1[1] = n1 >> 32;

  w2[0] = n2;
  w2[1] = n2 >> 32;

  /* Compute p = w1 * w2 */

  memset (p, 0, sizeof p);
  for (i = 0; i < 2; i ++) {
    q = 0;
    for (j = 0; j < 2; j ++) {
      q += ((uQuad)(w1[i])) * w2[j];
      q += p[i+j];
      p[i+j] = q;
      q >>= 32;
    }
    p[i+j] = q;
  }

  {
    char ps[40];
    uLong px[4];

    i = 40;
    ps[--i] = 0;
    memcpy (px, p, sizeof px);
    do {
      r = 0;
      for (j = 4; -- j >= 0;) {
        r   <<= 32;
        r    += px[j];
        px[j] = r / 10;
        r    %= 10;
      }
      ps[--i] = r + '0';
    } while ((px[0] | px[1] | px[2] | px[3]) != 0);
    printf ("p: %8.8X:%8.8X:%8.8X:%8.8X  %s\n", p[3], p[2], p[1], p[0], ps + i);
  }

  /* Compute q = p / d */

  r = (((uQuad)p[3]) << 32) + p[2];
  if (r >= d) abort (); // oz_sys_condhand_signal (10, OZ_ARITHOVER, 8, r, d, n1, n2);
  q = (((uQuad)p[1]) << 32) + p[0];
  for (i = 64; -- i >= 0;) {
    j = ((r & 0x8000000000000000ULL) != 0);
    r += r;
    if (q & 0x8000000000000000ULL) r |= 1;
    q += q;
    if (j || (r >= d)) {
      r -= d;
      q |= 1;
    }
  }
  if ((r & 0x8000000000000000ULL) || (r + r >= d)) q ++;
  return (q);
}
