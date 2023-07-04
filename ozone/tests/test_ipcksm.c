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
#include <stdlib.h>
#include <string.h>

#define NWORDS 256

typedef unsigned char uByte;
typedef unsigned short uWord;
typedef unsigned int uLong;
typedef unsigned long long uQuad;

uQuad rdtsc ();
uWord gencksm (uLong nwords, const void *words, uWord start);

int main ()

{
  int i, seq;
  uLong avgnew, avgold, tnew, told;
  uQuad t0, t1, t2;
  uWord cs1, cs2, nwords, start, words[NWORDS];

  seq = 0;
  nwords = 0;
  start  = 0;
  memset (words, 0, sizeof words);

  avgnew = 0;
  avgold = 0;

loop:
  for (i = 0; i < NWORDS; i += 8) {
    sc_onewayhash (16, words + i, words + ((i + NWORDS - 8) % NWORDS));
  }

  start  = words[NWORDS-2];
  nwords = (words[NWORDS-1] % (NWORDS-1));

#if 00
  printf ("seq %d: nwords %u, start %u\n", seq, nwords, start); fflush (stdout);
#endif
  t0  = rdtsc ();
  cs1 = gencksm (nwords, words, start);
  t1  = rdtsc ();
  cs2 = oz_dev_ip_gencksmb (nwords * 2, words, start);
  t2  = rdtsc ();

  told = t1 - t0;
  tnew = t2 - t1;
  printf ("start: %4.4x  nwords: %3u  old: %4u new: %4u    %3d%%\n", start, nwords, told, tnew, tnew * 100 / told);

  avgnew += tnew;
  avgold += told;

  if (cs2 != cs1) {
    printf ("error: seq %d: nwords %u, start %4.4x, cs1 %4.4x, cs2 %4.4x\n", seq, nwords, start, cs1, cs2);
  }

  if (++ seq % 20 == 0) printf ("                %6u avg: old: %4u new %4u    %3d%%\n", 
                        seq, avgold / seq, avgnew / seq, avgnew * 100 / avgold);

  goto loop;
}

/************************************************************************/
/*									*/
/*  This routine generates a one-way hash value given an arbitrary 	*/
/*  string								*/
/*									*/
/************************************************************************/


typedef struct { uLong d[5];		/* current hashing data */
                 uLong count;		/* total bytecount so far */
                 uLong ll;		/* length in 'lb' */
                 uByte lb[64];		/* leftover buffer */
               } Hctx;

/* The SHS f()-functions */

#define F1(x,y,z) ((x & y) | (~x & z))		/* rounds  0-19 */
#define F2(x,y,z) (x ^ y ^ z)			/* rounds 20-39 */
#define F3(x,y,z) ((x & y) | (x & z) | (y & z))	/* rounds 40-59 */
#define F4(x,y,z) (x ^ y ^ z)			/* rounds 60-79 */

/* The SHS Mysterious constants */

#define K1 0x5a827999
#define K2 0x6ed9eba1
#define K3 0x8f1bbcdc
#define K4 0xca62c1d6

/* SHS initial values */

#define H0INIT 0x67452301
#define H1INIT 0xefcdab89
#define H2INIT 0x98badcfe
#define H3INIT 0x10325476
#define H4INIT 0xc3d2e1f0

/* 32-bit rotate - kludged with shifts */

#define ROT32(n,x) ((x << n) | (x >> (32 - n)))

/************************************************************************/
/*									*/
/*  Internal routine to perofrm the SHS transformation			*/
/*									*/
/************************************************************************/

static void transform (uByte buff[64], uLong digest[5])

{
  uByte *ip;
  uLong a, b, c, d, e, t, w[80];
  int i;

  /* Make 16 longs out of 64 bytes (endian-independent) */

  ip = buff;
  for (i = 0; i < 16; i ++) {
    w[i] = *(ip ++);
    w[i] = (w[i] << 8) + *(ip ++);
    w[i] = (w[i] << 8) + *(ip ++);
    w[i] = (w[i] << 8) + *(ip ++);
  }

  /* Expand the 16 longs into 64 more longs */

  for (; i < 80; i ++) w[i] = w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16];

  /* Set up first buffer */

  a = digest[0];
  b = digest[1];
  c = digest[2];
  d = digest[3];
  e = digest[4];

  /* Serious mangling, divided into four sub-rounds */

  for (i = 0; i < 20; i ++) {
    t = ROT32 (5, a) + F1 (b, c, d) + e + w[i] + K1;
    e = d; d = c; c = ROT32 (30, b); b = a; a = t;
  }

  for (; i < 40; i ++) {
    t = ROT32 (5, a) + F2 (b, c, d) + e + w[i] + K2;
    e = d; d = c; c = ROT32 (30, b); b = a; a = t;
  }

  for (; i < 60; i ++) {
    t = ROT32 (5, a) + F3 (b, c, d) + e + w[i] + K3;
    e = d; d = c; c = ROT32 (30, b); b = a; a = t;
  }

  for (; i < 80; i ++) {
    t = ROT32 (5, a) + F4 (b, c, d) + e + w[i] + K4;
    e = d; d = c; c = ROT32 (30, b); b = a; a = t;
  }

  /* Build message digest */

  digest[0] += a;
  digest[1] += b;
  digest[2] += c;
  digest[3] += d;
  digest[4] += e;
}

/************************************************************************/
/*									*/
/*  This is the global routine to do the hashing			*/
/*									*/
/*    Input:								*/
/*									*/
/*	count   = number of input bytes					*/
/*	*buffer = input bytes						*/
/*									*/
/*    Output:								*/
/*									*/
/*	*digest = output bytes						*/
/*									*/
/************************************************************************/

void *sc_owh_init ()

{
  Hctx *hctx;

  hctx = malloc (sizeof *hctx);

  /* Set the digest vars to their initial values */

  hctx -> d[0] = H0INIT;
  hctx -> d[1] = H1INIT;
  hctx -> d[2] = H2INIT;
  hctx -> d[3] = H3INIT;
  hctx -> d[4] = H4INIT;

  /* No "left over" data */

  hctx -> ll    = 0;
  hctx -> count = 0;

  return (hctx);
}

void sc_owh_data (void *hctxv, int count, uByte *buffer)

{
  uByte *ip;
  Hctx *hctx;
  uLong i, j;

  hctx = hctxv;
  i    = count;
  ip   = buffer;

  hctx -> count += count;

  /* If less than 64 to do, just put in leftover buffer and do it later */

  if (hctx -> ll + i < 64) {
    memcpy (hctx -> lb + hctx -> ll, ip, i);
    hctx -> ll += i;
    return;
  }

  /* Process any left-over data with enough input to fill to 64 bytes */

  if (hctx -> ll != 0) {
    j = 64 - hctx -> ll;
    memcpy (hctx -> lb + hctx -> ll, ip, j);
    transform (hctx -> lb, hctx -> d);
    i  -= j;
    ip += j;
  }

  /* Process remaining input data in 64 byte chunks */

  while (i >= 64) {
    transform (ip, hctx -> d);
    i  -= 64;
    ip += 64;
  }

  /* Put the rest of the input in the leftover buffer, if any */

  hctx -> ll = i;
  memcpy (hctx -> lb, ip, i);
}

void sc_owh_term (void *hctxv, uByte digest[16])

{
  uByte *op;
  Hctx *hctx;
  uLong i;

  hctx = hctxv;
  i = hctx -> ll;

  /* Set the first char of padding to 0x80.  This is    */
  /* safe since there is always at least one byte free. */

  hctx -> lb[i++] = 0x80;

  /* If block has more than 60, pad with zeroes and process it */

  if (i > 60) {
    memset (hctx -> lb + i, 0, 64 - i);
    transform (hctx -> lb, hctx -> d);
    i = 0;
  }

  /* Pad block with zeroes to 60 bytes */

  memset (hctx -> lb + i, 0, 60 - i);

  /* Append length and process */

  hctx -> lb[60] = hctx -> count >> 24;
  hctx -> lb[61] = hctx -> count >> 16;
  hctx -> lb[62] = hctx -> count >>  8;
  hctx -> lb[63] = hctx -> count;

  transform (hctx -> lb, hctx -> d);

  /* Change 160-bit key to 128-bits */

  i = hctx -> d[4];
  hctx -> d[0] += i & K1;
  hctx -> d[1] += i & K2;
  hctx -> d[2] += i & K3;
  hctx -> d[3] += i & K4;

  /* Copy to output (endian-independent) */

  op = digest;
  for (i = 0; i < 4; i ++) {
    *(op ++) = hctx -> d[i] >> 24;
    *(op ++) = hctx -> d[i] >> 16;
    *(op ++) = hctx -> d[i] >>  8;
    *(op ++) = hctx -> d[i];
  }

  /* Free off context block */

  free (hctx);
}

int sc_onewayhash (int count, uByte *buffer, uByte digest[16])

{
  void *hctxv;

  hctxv = sc_owh_init ();
  sc_owh_data (hctxv, count, buffer);
  sc_owh_term (hctxv, digest);
}


/************************************************************************/
/*									*/
/*  Generate IP-style checksum for a list of network byte order words	*/
/*  Crude but effective							*/
/*									*/
/************************************************************************/

uWord gencksm (uLong nwords, const void *words, uWord start)

{
  const uByte *r2;
  uLong r0, r1;

  r0 = 0xffff & ~ start;		/* get one's comp of start value */
  r2 = words;				/* point to array of words in network byte order */
  for (r1 = nwords; r1 != 0; -- r1) {	/* repeat as long as there is more to do */
    r0 += *(r2 ++) << 8;		/* add in high order byte */
    r0 += *(r2 ++);			/* add in low order byte */
  }
  while ((r1 = r0 >> 16) != 0) {	/* get end-around carries */
    r0 = (r0 & 0xffff) + r1;		/* add them back around */
  }					/* should only happen a total of up to 2 times */
  r0 = 0xffff & ~ r0;			/* get one's comp of result */
  if (r0 == 0) r0 = 0xffff;		/* if zero, return 0xffff (neg zero) */
  return (r0);
}

