//+++2002-01-14
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
//---2002-01-14

/************************************************************************/
/*									*/
/*  Test oz_dev_disk_fs' wildscan_match routine				*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <string.h>

// rc<0> set if the fn should be output as is
// rc<1> set if the fn directory needs to be scanned, then rc<:2> has number of wc chars to skip

struct { char *wc, *fn; int rc; } testcases[] = {
	"abc",		"def",		 0, 	// simple strings that don't match
	"a?c",  	"axc",		 1, 	// match
	"ab?", 		"ab/",		 0, 	// no match, no scan
	"abc/def", 	"abc/", 	18, 	// no match, scan, 4 wc chars skipped
	"a*c/def", 	"axyzc/", 	18, 	// no match, scan, 4 wc chars skipped
	"~",		"abc",		 1, 	// match
	"~",		"abc/",		 3, 	// match, scan, no wc chars skipped
	"ab~",		"abcde/",	11, 	// match, scan, 2 wc chars skipped
	"ab~ef/jkl", 	"abcd/",	10,	// no match, scan, 2 wc chars skipped
	"ab~ef/jkl",	"abcdef/", 	10, 	// no match, scan, 2 wc chars skipped
	"ab~ef/*",	"abcd/", 	10, 	// no match, scan, 2 wc chars skipped
	"ab~ef/*",	"abcdef/", 	11, 	// match, scan, 2 wc chars skipped
	NULL,		NULL,		 0 };

static int wildscan_match (const char *wildcard, const char *filename);

int main ()

{
  char *fn, *p, *wc;
  int bb, dc, i, rc;

  for (i = 0; testcases[i].wc != NULL; i ++) {
    wc = testcases[i].wc;			// get wildcard string to be tested
    fn = testcases[i].fn;			// get filename string to be tested
    rc = wildscan_match (wc, fn);		// perform the test
    dc = -2;					// start with don't care just checking the 'matched' bit
    if (fn[strlen(fn)-1] == '/') {		// see if filename string is a directory
      dc = -4;					// ... then the scan bit has to be good, too
      if (testcases[i].rc & 2) dc = 0;		// ... and if we're expecting a positive scan, all bits must be good
    }
    bb = (rc ^ testcases[i].rc) & ~ dc;		// find out which bits are bad, if any
    if (bb != 0) {				// see if any bits are bad
      printf ("wildscan_match ('%s', '%s') returned %d, should be %d\n", wc, fn, rc, rc ^ bb);
    } else if (rc & 2) {			// if it matched, see if it should have matched on an tilde
      p = strchr (wc, '~');
      if (p != NULL) {
        if ((rc >> 2) != (p - wc)) {		// if tilde, make sure offset is correct
           printf ("wildscan_match ('%s', '%s') returned offset %d, tilde offset %d\n", rc >> 2, p - wc);
        }
      }
    }
  }
  printf ("Test complete.\n");
  return (0);
}


/************************************************************************/
/*									*/
/*  Return whether the 'filename' matches 'wildcard'			*/
/*									*/
/*    Input:								*/
/*									*/
/*	wildcard = wildcard string pointer				*/
/*	filename = filename string pointer				*/
/*									*/
/*    Output:								*/
/*									*/
/*	wildscan_match<0> = 0 : don't output it				*/
/*	                    1 : output it				*/
/*	              <1> = 0 : don't scan it				*/
/*	                    1 : scan it after skipping <:2> chars	*/
/*	if <1> is set and the string has a '~' in it, the <:2> has the 	*/
/*	offset of the '~'						*/
/*									*/
/************************************************************************/

static int wildscan_match (const char *wildcard, const char *filename)

{
  char c;
  const char *fn, *wc;
  int rc;

  wc = wildcard;
  fn = filename;

  /* Skip chars at the beginning that match exactly        */
  /* Also skip non-delimiters in fn that match a '?' in wc */

  while (((c = *wc) != 0) && ((*fn == c) || ((c == '?') && (*fn != '/')))) { wc ++; fn ++; }

  /* If we reached the end of the wildcard string and the filename, then it matches, else it doesn't */

  if (c == 0) return (*fn == 0);				// return if we used the whole wildcard string

  /* An '*' matches any number of non-delimiters in filename */

  if (c == '*') {
    c = *(++ wc);						// get character following the '*'
								// optimizations ...
    if (c == 0) return (1);					// if '*' was last thing in wildcard, it matches
    if (c == '/') return (fn[strlen(fn)-1] == '/');		// also a match if '*/' and filename is a directory
    do {							// pound it out the hard way ...
      rc = wildscan_match (wc, fn);				// see if we get a match with rest of filename as is
      if (rc & 3) {
        rc += (wc - wildcard) << 2;				// if so, return total wildcard chars to skip
        return (rc);
      }
      c = *(++ fn);						// otherwise, swallow a filename char
    } while ((c != 0) && (c != '/'));				// ... and try matching if there was one to swallow
    return (0);							// nothing left to swallow, return failure status
  }

  /* An '~' matches any number of chars in filename, including delimiters */

  if (c == '~') {
    rc = (wc - wildcard) << 2;					// number of chars to skip, not including the '~'
    c  = *(++ wc);						// get character following the '~'
								// optimizations ...
    if (c == 0) return (rc | 3);				// if '~' was last thing in wildcard, it matches, output and scan it
    do {							// pound it out the hard way ...
      if (wildscan_match (wc, fn) & 1) {			// see if we get a match with rest of filename as is
        return (rc | 3);					// if so, output and scan it
      }
      c = *(++ fn);						// otherwise, swallow a filename char
    } while ((c != 0) && (c != '/'));				// ... and try matching if there was one to swallow
    if (c == 0) return (0);					// if end of filename and still no match, it doesn't match
    return (rc | 2);						// filename is a directory, it could possibly match if we scan it too
  }

  /* If we reached the end of filename string, and it was a directory, then return saying that it ought to be scanned */
  /* This accounts for the case of wildcard='abc/def' and filename='abc/', we want to scan directory 'abc/' for 'def' */

  if ((fn > filename) && (fn[0] == 0) && (fn[-1] == '/')) {
    rc = (wc - wildcard) << 2;
    return (rc | 2);
  }

  /* A simple non-matching character */

  return (0);
}
