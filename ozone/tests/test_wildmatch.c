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

typedef unsigned int uLong;
#define WILD_MATCH_MATCHES 0x01
#define WILD_MATCH_DOSUBDR 0x02


static uLong wild_match (char *filespec, char *wildspec);
static uLong wild_match2 (char *fp, char *fe, char *wp, char *we);

int main ()

{
  char filespec[64], wildspec[64];
  uLong sts;

loop:
  printf ("filespec> "); gets (filespec);
  printf ("wildspec> "); gets (wildspec);
  sts = wild_match (filespec, wildspec);
  printf ("  sts 0x%x\n", sts);
  if (sts & WILD_MATCH_MATCHES) printf ("    MATCHES\n");
  if (sts & WILD_MATCH_DOSUBDR) printf ("    DOSUBDR\n");
  goto loop;
}


/************************************************************************/
/*									*/
/*  See if a given filespec matches a given wildcard spec		*/
/*									*/
/*    Input:								*/
/*									*/
/*	filespec = filespec to be matched				*/
/*	wildspec = wildcard spec to match against			*/
/*									*/
/*    Output:								*/
/*									*/
/*	wild_match & WILD_MATCH_MATCHES : this filespec matches the wildcard
/*	wild_match & WILD_MATCH_DOSUBDR : go into the 'filespec' directory
/*									*/
/*	? : match any single character except /				*/
/*	* : match any string of characters except /			*/
/*	... : match any string of characters, including /		*/
/*									*/
/************************************************************************/

static uLong wild_match (char *filespec, char *wildspec)

{
  char fc, *fe, *fp, wc, *we, *wp;

  /* Strip off as many matching characters off the end as we can */
  /* This helps with optimizations later                         */

  fp = filespec;
  wp = wildspec;

  fe = fp + strlen (fp);
  we = wp + strlen (wp);

  while ((fe > fp) && (we > wp)) {
    fc = *(-- fe);
    wc = *(-- we);
    if (fc == wc) continue;
    if ((wc != '?') || (fc == '/')) {
      fe ++;
      we ++;
      break;
    }
  }

  return (wild_match2 (fp, fe, wp, we));
}

static uLong wild_match2 (char *fp, char *fe, char *wp, char *we)

{
  char fc, wc;
  uLong submatch;

  /* Skip over as many directly matching characters as possible. */
  /* Also do the '?' matching that matches a single non-/ char.  */

compare:
  while (1) {
    if (wp == we) goto endofwild;
    wc = *wp;
    if ((wp + 3 <= we) && (wc == '.') && (wp[1] == '.') && (wp[2] == '.')) goto elipses;
    if (fp == fe) break;
    fc = *fp;
    if ((fc != wc) && ((wc != '?') || (fc == '/'))) break;
    fp ++;
    wp ++;
  }

  /* Only hope is the wildcard char is an asterisk.  Otherwise we fail. */

  if (wc != '*') return (0);

  /* Asterisk matches any number of any character except /.                   */
  /* So repeatedly try to match by gobbling non / chars from filespec string. */

  while ((wp < we) && ((wc = *(++ wp)) == '*')) {}	/* skip redundant *'s in wildcard spec */
  if ((wp == we) || (wc == '/')) {			/* optimiszation: if * is at end or a directory follows it ... */
    while ((fp < fe) && (*fp != '/')) fp ++;		/* ... just skip over all non-/ characters up to a / or end of string */
    goto compare;					/* ... then resume direct comparison from there */
  }
  while (1) {
    submatch = wild_match2 (fp, fe, wp, we);		/* try to match what's left of both strings */
    if (submatch & WILD_MATCH_MATCHES) break;		/* if they match, return success status */
    if ((*fp == '/') || (*fp == 0)) break;		/* no match, if at directory we can't skiip anything */
    fp ++;						/* not at directory, skip over the char and try again */
  }
  return (submatch);					/* return match/nomatch, passing along the DOSUBDIR flag */

  /* Elipses match any string including directory slashes */

elipses:
  wp += 3;						/* point past the ... */
  while ((wp < we) && ((wc = *(++ wp)) == '*')) {}	/* skip redundant *'s in wildcard spec */
  if (wp == we) return (WILD_MATCH_MATCHES | WILD_MATCH_DOSUBDR);
  while (1) {
    submatch = wild_match2 (fp, fe, wp, we);		/* try to match what's left of both strings */
    if (submatch & WILD_MATCH_MATCHES) break;		/* if they match, return success status */
    if (fp == fe) break;				/* no match, done if no more filepsec string */
    fp ++;						/* skip over the char and try again */
  }
  return (submatch | WILD_MATCH_DOSUBDR);		/* return match/nomatch, force the DOSUBDIR flag because of the ...'s */

  /* If we reached the end of both strings, we have a match.                                              */
  /* Don't do any sub-directory because it won't match (we've reached the end of the wildcard as we are). */
  /* Special case: If wildcard string is exhausted and all that's left of filespec string is just "/", we match */
  /* This is to allow a directory to be included in its parent directory                                        */

endofwild:
  if (fp == fe) return (WILD_MATCH_MATCHES);
  if ((*fp == '/') && (fp + 1 == fe)) return (WILD_MATCH_MATCHES);
  return (0);
}
