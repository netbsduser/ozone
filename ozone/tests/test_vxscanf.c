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

/* Copyright (C) 1991, 92, 93, 94, 95, 96, 97 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include "../locale/localeinfo.h"
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <libc-lock.h>

#ifdef	__GNUC__
#define	HAVE_LONGLONG
#define	LONGLONG	long long
#else
#define	LONGLONG	long
#endif

/* Those are flags in the conversion format. */
# define LONG		0x001	/* l: long or double */
# define LONGDBL	0x002	/* L: long long or long double */
# define SHORT		0x004	/* h: short */
# define SUPPRESS	0x008	/* *: suppress assignment */
# define POINTER	0x010	/* weird %p pointer (`fake hex') */
# define NOSKIP		0x020	/* do not skip blanks */
# define WIDTH		0x040	/* width was given */
# define GROUP		0x080	/* ': group numbers */
# define MALLOC		0x100	/* a: malloc strings */

# define TYPEMOD	(LONG|LONGDBL|SHORT)


#ifdef USE_IN_LIBIO
# include <libioP.h>
# include <libio.h>

# undef va_list
# define va_list	_IO_va_list
# define ungetc(c, s)	((void) ((int) c != EOF && --read_in),		      \
			_IO_ungetc (c, s))
# define inchar()	((c = _IO_getc_unlocked (s)), \
			 (void) (c != EOF && ++read_in), c)
# define encode_error()	do {						      \
			  if (errp != NULL) *errp |= 4;			      \
			  _IO_funlockfile (s);				      \
			  __set_errno (EILSEQ);				      \
			  return done;					      \
			} while (0)
# define conv_error()	do {						      \
			  if (errp != NULL) *errp |= 2;			      \
			  _IO_funlockfile (s);				      \
			  return done;					      \
			} while (0)
# define input_error()	do {						      \
			  _IO_funlockfile (s);				      \
			  if (errp != NULL) *errp |= 1;			      \
			  return done ?: EOF;				      \
			} while (0)
# define memory_error()	do {						      \
			  _IO_funlockfile (s);				      \
			  __set_errno (ENOMEM);				      \
			  return EOF;					      \
			} while (0)
# define ARGCHECK(s, format)						      \
  do									      \
    {									      \
      /* Check file argument for consistence.  */			      \
      CHECK_FILE (s, EOF);						      \
      if (s->_flags & _IO_NO_READS)					      \
	{								      \
	  __set_errno (EBADF);						      \
	  return EOF;							      \
	}								      \
      else if (format == NULL)						      \
	{								      \
	  MAYBE_SET_EINVAL;						      \
	  return EOF;							      \
	}								      \
    } while (0)
# define LOCK_STREAM(S)							      \
  __libc_cleanup_region_start ((void (*) (void *)) &_IO_funlockfile, (S));    \
  _IO_flockfile (S)
# define UNLOCK_STREAM __libc_cleanup_region_end (1)
#else
# define ungetc(c, s)	((void) (c != EOF && --read_in), ungetc (c, s))
# define inchar()	((c = getc (s)), (void) (c != EOF && ++read_in), c)
# define encode_error()	do {						      \
			  funlockfile (s);				      \
			  __set_errno (EILSEQ);				      \
			  return done;					      \
			} while (0)
# define conv_error()	do {						      \
			  funlockfile (s);				      \
			  return done;					      \
			} while (0)
# define input_error()	do {						      \
			  funlockfile (s);				      \
			  return done ?: EOF;				      \
			} while (0)
# define memory_error()	do {						      \
			  funlockfile (s);				      \
			  __set_errno (ENOMEM);				      \
			  return EOF;					      \
			} while (0)
# define ARGCHECK(s, format)						      \
  do									      \
    {									      \
      /* Check file argument for consistence.  */			      \
      if (!__validfp (s) || !s->__mode.__read)				      \
	{								      \
	  __set_errno (EBADF);						      \
	  return EOF;							      \
	}								      \
      else if (format == NULL)						      \
	{								      \
	  __set_errno (EINVAL);						      \
	  return EOF;							      \
	}								      \
    } while (0)
#if 1
      /* XXX For now !!! */
# define flockfile(S) /* nothing */
# define funlockfile(S) /* nothing */
# define LOCK_STREAM(S)
# define UNLOCK_STREAM
#else
# define LOCK_STREAM(S)							      \
  __libc_cleanup_region_start (&__funlockfile, (S));			      \
  __flockfile (S)
# define UNLOCK_STREAM __libc_cleanup_region_end (1)
#endif
#endif


/* Read formatted input from S according to the format string
   FORMAT, using the argument list in ARG.
   Return the number of assignments made, or -1 for an input error.  */
#ifdef USE_IN_LIBIO
int
_IO_vfscanf (s, format, argptr, errp)
     _IO_FILE *s;
     const char *format;
     _IO_va_list argptr;
     int *errp;
#else
int
__vfscanf (FILE *s, const char *format, va_list argptr)
#endif
{
  va_list arg;
  register const char *f = format;
  register unsigned char fc;	/* Current character of the format.  */
  register size_t done = 0;	/* Assignments done.  */
  register size_t read_in = 0;	/* Chars read in.  */
  register int c = 0;		/* Last char read.  */
  register int width;		/* Maximum field width.  */
  register int flags;		/* Modifiers for current format element.  */

  /* Status for reading F-P nums.  */
  char got_dot, got_e, negative;
  /* If a [...] is a [^...].  */
  char not_in;
  /* Base for integral numbers.  */
  int base;
  /* Signedness for integral numbers.  */
  int number_signed;
  /* Decimal point character.  */
  wchar_t decimal;
  /* The thousands character of the current locale.  */
  wchar_t thousands;
  /* Integral holding variables.  */
  union
    {
      long long int q;
      unsigned long long int uq;
      long int l;
      unsigned long int ul;
    } num;
  /* Character-buffer pointer.  */
  char *str = NULL;
  wchar_t *wstr = NULL;
  char **strptr = NULL;
  size_t strsize = 0;
  /* We must not react on white spaces immediately because they can
     possibly be matched even if in the input stream no character is
     available anymore.  */
  int skip_space = 0;
  /* Workspace.  */
  char *tw;			/* Temporary pointer.  */
  char *wp = NULL;		/* Workspace.  */
  size_t wpmax = 0;		/* Maximal size of workspace.  */
  size_t wpsize;		/* Currently used bytes in workspace.  */
#define ADDW(Ch)							    \
  do									    \
    {									    \
      if (wpsize == wpmax)						    \
	{								    \
	  char *old = wp;						    \
	  wpmax = UCHAR_MAX > 2 * wpmax ? UCHAR_MAX : 2 * wpmax;	    \
	  wp = (char *) alloca (wpmax);					    \
	  if (old != NULL)						    \
	    memcpy (wp, old, wpsize);					    \
	}								    \
      wp[wpsize++] = (Ch);						    \
    }									    \
  while (0)

#ifdef __va_copy
  __va_copy (arg, argptr);
#else
  arg = (va_list) argptr;
#endif

  ARGCHECK (s, format);

  /* Figure out the decimal point character.  */
  if (mbtowc (&decimal, _NL_CURRENT (LC_NUMERIC, DECIMAL_POINT),
	      strlen (_NL_CURRENT (LC_NUMERIC, DECIMAL_POINT))) <= 0)
    decimal = (wchar_t) *_NL_CURRENT (LC_NUMERIC, DECIMAL_POINT);
  /* Figure out the thousands separator character.  */
  if (mbtowc (&thousands, _NL_CURRENT (LC_NUMERIC, THOUSANDS_SEP),
	      strlen (_NL_CURRENT (LC_NUMERIC, THOUSANDS_SEP))) <= 0)
    thousands = (wchar_t) *_NL_CURRENT (LC_NUMERIC, THOUSANDS_SEP);

  /* Lock the stream.  */
  LOCK_STREAM (s);

  /* Run through the format string.  */
  while (*f != '\0')
    {
      unsigned int argpos;
      /* Extract the next argument, which is of type TYPE.
	 For a %N$... spec, this is the Nth argument from the beginning;
	 otherwise it is the next argument after the state now in ARG.  */
#ifdef __va_copy
# define ARG(type)     (argpos == 0 ? va_arg (arg, type) :		      \
                        ({ unsigned int pos = argpos;			      \
                           va_list arg;					      \
                          __va_copy (arg, argptr);			      \
                           while (--pos > 0)				      \
                             (void) va_arg (arg, void *);		      \
                           va_arg (arg, type);				      \
                         }))
#else
# if 0
      /* XXX Possible optimization.  */
#  define ARG(type)	(argpos == 0 ? va_arg (arg, type) :		      \
			 ({ va_list arg = (va_list) argptr;		      \
			    arg = (va_list) ((char *) arg		      \
					     + (argpos - 1)		      \
					     * __va_rounded_size (void *));   \
			    va_arg (arg, type);				      \
			 }))
# else
#  define ARG(type)	(argpos == 0 ? va_arg (arg, type) :		      \
			 ({ unsigned int pos = argpos;			      \
			    va_list arg = (va_list) argptr;		      \
			    while (--pos > 0)				      \
			      (void) va_arg (arg, void *);		      \
			    va_arg (arg, type);				      \
			  }))
# endif
#endif

      if (!isascii (*f))
	{
	  /* Non-ASCII, may be a multibyte.  */
	  int len = mblen (f, strlen (f));
	  if (len > 0)
	    {
	      do
		{
		  c = inchar ();
		  if (c == EOF)
		    input_error ();
		  else if (c != *f++)
		    {
		      ungetc (c, s);
		      conv_error ();
		    }
		}
	      while (--len > 0);
	      continue;
	    }
	}

      fc = *f++;
      if (fc != '%')
	{
	  /* Remember to skip spaces.  */
	  if (isspace (fc))
	    {
	      skip_space = 1;
	      continue;
	    }

	  /* Read a character.  */
	  c = inchar ();

	  /* Characters other than format specs must just match.  */
	  if (c == EOF)
	    input_error ();

	  /* We saw white space char as the last character in the format
	     string.  Now it's time to skip all leading white space.  */
	  if (skip_space)
	    {
	      while (isspace (c))
		if (inchar () == EOF && errno == EINTR)
		  conv_error ();
	      skip_space = 0;
	    }

	  if (c != fc)
	    {
	      ungetc (c, s);
	      conv_error ();
	    }

	  continue;
	}

      /* This is the start of the conversion string. */
      flags = 0;

      /* Initialize state of modifiers.  */
      argpos = 0;

      /* Prepare temporary buffer.  */
      wpsize = 0;

      /* Check for a positional parameter specification.  */
      if (isdigit (*f))
	{
	  argpos = *f++ - '0';
	  while (isdigit (*f))
	    argpos = argpos * 10 + (*f++ - '0');
	  if (*f == '$')
	    ++f;
	  else
	    {
	      /* Oops; that was actually the field width.  */
	      width = argpos;
	      flags |= WIDTH;
	      argpos = 0;
	      goto got_width;
	    }
	}

      /* Check for the assignment-suppressing and the number grouping flag.  */
      while (*f == '*' || *f == '\'')
	switch (*f++)
	  {
	  case '*':
	    flags |= SUPPRESS;
	    break;
	  case '\'':
	    flags |= GROUP;
	    break;
	  }

      /* We have seen width. */
      if (isdigit (*f))
	flags |= WIDTH;

      /* Find the maximum field width.  */
      width = 0;
      while (isdigit (*f))
	{
	  width *= 10;
	  width += *f++ - '0';
	}
    got_width:
      if (width == 0)
	width = -1;

      /* Check for type modifiers.  */
      while (*f == 'h' || *f == 'l' || *f == 'L' || *f == 'a' || *f == 'q')
	switch (*f++)
	  {
	  case 'h':
	    /* int's are short int's.  */
	    if (flags & TYPEMOD)
	      /* Signal illegal format element.  */
	      conv_error ();
	    flags |= SHORT;
	    break;
	  case 'l':
	    if (flags & (SHORT|LONGDBL))
	      conv_error ();
	    else if (flags & LONG)
	      {
		/* A double `l' is equivalent to an `L'.  */
		flags &= ~LONG;
		flags |= LONGDBL;
	      }
	    else
	      /* int's are long int's.  */
	      flags |= LONG;
	    break;
	  case 'q':
	  case 'L':
	    /* double's are long double's, and int's are long long int's.  */
	    if (flags & TYPEMOD)
	      /* Signal illegal format element.  */
	      conv_error ();
	    flags |= LONGDBL;
	    break;
	  case 'a':
	    if (flags & TYPEMOD)
	      /* Signal illegal format element.  */
	      conv_error ();
	    /* String conversions (%s, %[) take a `char **'
	       arg and fill it in with a malloc'd pointer.  */
	    flags |= MALLOC;
	    break;
	  }

      /* End of the format string?  */
      if (*f == '\0')
	conv_error ();

      /* We must take care for EINTR errors.  */
      if (c == EOF && errno == EINTR)
	input_error ();

      /* Find the conversion specifier.  */
      fc = *f++;
      if (skip_space || (fc != '[' && fc != 'c' && fc != 'C' && fc != 'n'))
	{
	  /* Eat whitespace.  */
	  do
	    if (inchar () == EOF && errno == EINTR)
	      input_error ();
	  while (isspace (c));
	  ungetc (c, s);
	  skip_space = 0;
	}

      switch (fc)
	{
	case '%':	/* Must match a literal '%'.  */
	  c = inchar ();
	  if (c != fc)
	    {
	      ungetc (c, s);
	      conv_error ();
	    }
	  break;

	case 'n':	/* Answer number of assignments done.  */
	  /* Corrigendum 1 to ISO C 1990 describes the allowed flags
	     with the 'n' conversion specifier.  */
	  if (!(flags & SUPPRESS))
	    {
	      /* Don't count the read-ahead.  */
	      if (flags & LONGDBL)
		*ARG (long long int *) = read_in;
	      else if (flags & LONG)
		*ARG (long int *) = read_in;
	      else if (flags & SHORT)
		*ARG (short int *) = read_in;
	      else
		*ARG (int *) = read_in;

#ifdef NO_BUG_IN_ISO_C_CORRIGENDUM_1
	      /* We have a severe problem here.  The ISO C standard
		 contradicts itself in explaining the effect of the %n
		 format in `scanf'.  While in ISO C:1990 and the ISO C
		 Amendement 1:1995 the result is described as

		   Execution of a %n directive does not effect the
		   assignment count returned at the completion of
		   execution of the f(w)scanf function.

		 in ISO C Corrigendum 1:1994 the following was added:

		   Subclause 7.9.6.2
		   Add the following fourth example:
		     In:
		       #include <stdio.h>
		       int d1, d2, n1, n2, i;
		       i = sscanf("123", "%d%n%n%d", &d1, &n1, &n2, &d2);
		     the value 123 is assigned to d1 and the value3 to n1.
		     Because %n can never get an input failure the value
		     of 3 is also assigned to n2.  The value of d2 is not
		     affected.  The value 3 is assigned to i.

		 We go for now with the historically correct code fro ISO C,
		 i.e., we don't count the %n assignments.  When it ever
		 should proof to be wrong just remove the #ifdef above.  */
	      ++done;
#endif
	    }
	  break;

	case 'c':	/* Match characters.  */
	  if ((flags & LONG) == 0)
	    {
	      if (!(flags & SUPPRESS))
		{
		  str = ARG (char *);
		  if (str == NULL)
		    conv_error ();
		}

	      c = inchar ();
	      if (c == EOF)
		input_error ();

	      if (width == -1)
		width = 1;

	      if (!(flags & SUPPRESS))
		{
		  do
		    *str++ = c;
		  while (--width > 0 && inchar () != EOF);
		}
	      else
		while (--width > 0 && inchar () != EOF);

	      if (width > 0)
		/* I.e., EOF was read.  */
		--read_in;

	      if (!(flags & SUPPRESS))
		++done;

	      break;
	    }
	  /* FALLTHROUGH */
	case 'C':
	  /* Get UTF-8 encoded wide character.  Here we assume (as in
	     other parts of the libc) that we only have to handle
	     UTF-8.  */
	  {
	    wint_t val;
	    size_t cnt = 0;
	    int first = 1;

	    if (!(flags & SUPPRESS))
	      {
		wstr = ARG (wchar_t *);
		if (str == NULL)
		  conv_error ();
	      }

	    do
	      {
#define NEXT_WIDE_CHAR(First)						      \
		c = inchar ();						      \
		if (c == EOF)						      \
		  /* EOF is only an error for the first character.  */	      \
		  if (First)						      \
		    input_error ();					      \
		  else							      \
		    {							      \
		      --read_in;					      \
		      break;						      \
		    }							      \
		val = c;						      \
		if (val >= 0x80)					      \
		  {							      \
		    if ((c & 0xc0) == 0x80 || (c & 0xfe) == 0xfe)	      \
		      encode_error ();					      \
		    if ((c & 0xe0) == 0xc0)				      \
		      {							      \
			/* We expect two bytes.  */			      \
			cnt = 1;					      \
			val &= 0x1f;					      \
		      }							      \
		    else if ((c & 0xf0) == 0xe0)			      \
		      {							      \
			/* We expect three bytes.  */			      \
			cnt = 2;					      \
			val &= 0x0f;					      \
		      }							      \
		    else if ((c & 0xf8) == 0xf0)			      \
		      {							      \
			/* We expect four bytes.  */			      \
			cnt = 3;					      \
			val &= 0x07;					      \
		      }							      \
		    else if ((c & 0xfc) == 0xf8)			      \
		      {							      \
			/* We expect five bytes.  */			      \
			cnt = 4;					      \
			val &= 0x03;					      \
		      }							      \
		    else						      \
		      {							      \
			/* We expect six bytes.  */			      \
			cnt = 5;					      \
			val &= 0x01;					      \
		      }							      \
		    							      \
		    do							      \
		      {							      \
			c = inchar ();					      \
			if (c == EOF					      \
			    || (c & 0xc0) == 0x80 || (c & 0xfe) == 0xfe)      \
			  encode_error ();				      \
			val <<= 6;					      \
			val |= c & 0x3f;				      \
		      }							      \
		    while (--cnt > 0);					      \
		  }							      \
									      \
		if (!(flags & SUPPRESS))				      \
		  *wstr++ = val;					      \
		first = 0

		NEXT_WIDE_CHAR (first);
	      }
	    while (--width > 0);

	    if (width > 0)
	      /* I.e., EOF was read.  */
	      --read_in;

	    if (!(flags & SUPPRESS))
	      ++done;
	  }
	  break;

	case 's':		/* Read a string.  */
	  if (flags & LONG)
	    /* We have to process a wide character string.  */
	    goto wide_char_string;

#define STRING_ARG(Str, Type)						      \
	  if (!(flags & SUPPRESS))					      \
	    {								      \
	      if (flags & MALLOC)					      \
		{							      \
		  /* The string is to be stored in a malloc'd buffer.  */     \
		  strptr = ARG (char **);				      \
		  if (strptr == NULL)					      \
		    conv_error ();					      \
		  /* Allocate an initial buffer.  */			      \
		  strsize = 100;					      \
		  *strptr = malloc (strsize * sizeof (Type));		      \
		  Str = (Type *) *strptr;				      \
		}							      \
	      else							      \
		Str = ARG (Type *);					      \
	      if (Str == NULL)						      \
		conv_error ();						      \
	    }
	  STRING_ARG (str, char);

	  c = inchar ();
	  if (c == EOF)
	    input_error ();

	  do
	    {
	      if (isspace (c))
		{
		  ungetc (c, s);
		  break;
		}
#define	STRING_ADD_CHAR(Str, c, Type)					      \
	      if (!(flags & SUPPRESS))					      \
		{							      \
		  *Str++ = c;						      \
		  if ((flags & MALLOC) && (char *) Str == *strptr + strsize)  \
		    {							      \
		      /* Enlarge the buffer.  */			      \
		      Str = realloc (*strptr, strsize * 2 * sizeof (Type));   \
		      if (Str == NULL)					      \
			{						      \
			  /* Can't allocate that much.  Last-ditch effort.  */\
			  Str = realloc (*strptr,			      \
					 (strsize + 1) * sizeof (Type));      \
			  if (Str == NULL)				      \
			    {						      \
			      /* We lose.  Oh well.			      \
				 Terminate the string and stop converting,    \
				 so at least we don't skip any input.  */     \
			      ((Type *) (*strptr))[strsize] = '\0';	      \
			      ++done;					      \
			      conv_error ();				      \
			    }						      \
			  else						      \
			    {						      \
			      *strptr = (char *) Str;			      \
			      Str = ((Type *) *strptr) + strsize;	      \
			      ++strsize;				      \
			    }						      \
			}						      \
		      else						      \
			{						      \
			  *strptr = (char *) Str;			      \
			  Str = ((Type *) *strptr) + strsize;		      \
			  strsize *= 2;					      \
			}						      \
		    }							      \
		}
	      STRING_ADD_CHAR (str, c, char);
	    } while ((width <= 0 || --width > 0) && inchar () != EOF);

	  if (!(flags & SUPPRESS))
	    {
	      *str = '\0';
	      ++done;
	    }
	  break;

	case 'S':
	  /* Wide character string.  */
	wide_char_string:
	  {
	    wint_t val;
	    int first = 1;
	    STRING_ARG (wstr, wchar_t);

	    do
	      {
		size_t cnt = 0;
		NEXT_WIDE_CHAR (first);

		if (iswspace (val))
		  {
		    /* XXX We would have to push back the whole wide char
		       with possibly many bytes.  But since scanf does
		       not make a difference for white space characters
		       we can simply push back a simple <SP> which is
		       guaranteed to be in the [:space:] class.  */
		    ungetc (' ', s);
		    break;
		  }

		STRING_ADD_CHAR (wstr, val, wchar_t);
		first = 0;
	      }
	    while (width <= 0 || --width > 0);

	    if (!(flags & SUPPRESS))
	      {
		*wstr = L'\0';
		++done;
	      }
	  }
	  break;

	case 'x':	/* Hexadecimal integer.  */
	case 'X':	/* Ditto.  */
	  base = 16;
	  number_signed = 0;
	  goto number;

	case 'o':	/* Octal integer.  */
	  base = 8;
	  number_signed = 0;
	  goto number;

	case 'u':	/* Unsigned decimal integer.  */
	  base = 10;
	  number_signed = 0;
	  goto number;

	case 'd':	/* Signed decimal integer.  */
	  base = 10;
	  number_signed = 1;
	  goto number;

	case 'i':	/* Generic number.  */
	  base = 0;
	  number_signed = 1;

	number:
	  c = inchar ();
	  if (c == EOF)
	    input_error ();

	  /* Check for a sign.  */
	  if (c == '-' || c == '+')
	    {
	      ADDW (c);
	      if (width > 0)
		--width;
	      c = inchar ();
	    }

	  /* Look for a leading indication of base.  */
	  if (width != 0 && c == '0')
	    {
	      if (width > 0)
		--width;

	      ADDW (c);
	      c = inchar ();

	      if (width != 0 && tolower (c) == 'x')
		{
		  if (base == 0)
		    base = 16;
		  if (base == 16)
		    {
		      if (width > 0)
			--width;
		      c = inchar ();
		    }
		}
	      else if (base == 0)
		base = 8;
	    }

	  if (base == 0)
	    base = 10;

	  /* Read the number into workspace.  */
	  while (c != EOF && width != 0)
	    {
	      if (base == 16 ? !isxdigit (c) :
		  ((!isdigit (c) || c - '0' >= base) &&
		   !((flags & GROUP) && base == 10 && c == thousands)))
		break;
	      ADDW (c);
	      if (width > 0)
		--width;

	      c = inchar ();
	    }

	  /* The just read character is not part of the number anymore.  */
	  ungetc (c, s);

	  if (wpsize == 0 ||
	      (wpsize == 1 && (wp[0] == '+' || wp[0] == '-')))
	    /* There was no number.  */
	    conv_error ();

	  /* Convert the number.  */
	  ADDW ('\0');
	  if (flags & LONGDBL)
	    {
	      if (number_signed)
		num.q = __strtoq_internal (wp, &tw, base, flags & GROUP);
	      else
		num.uq = __strtouq_internal (wp, &tw, base, flags & GROUP);
	    }
	  else
	    {
	      if (number_signed)
		num.l = __strtol_internal (wp, &tw, base, flags & GROUP);
	      else
		num.ul = __strtoul_internal (wp, &tw, base, flags & GROUP);
	    }
	  if (wp == tw)
	    conv_error ();

	  if (!(flags & SUPPRESS))
	    {
	      if (! number_signed)
		{
		  if (flags & LONGDBL)
		    *ARG (unsigned LONGLONG int *) = num.uq;
		  else if (flags & LONG)
		    *ARG (unsigned long int *) = num.ul;
		  else if (flags & SHORT)
		    *ARG (unsigned short int *)
		      = (unsigned short int) num.ul;
		  else
		    *ARG (unsigned int *) = (unsigned int) num.ul;
		}
	      else
		{
		  if (flags & LONGDBL)
		    *ARG (LONGLONG int *) = num.q;
		  else if (flags & LONG)
		    *ARG (long int *) = num.l;
		  else if (flags & SHORT)
		    *ARG (short int *) = (short int) num.l;
		  else
		    *ARG (int *) = (int) num.l;
		}
	      ++done;
	    }
	  break;

	case 'e':	/* Floating-point numbers.  */
	case 'E':
	case 'f':
	case 'g':
	case 'G':
	  c = inchar ();
	  if (c == EOF)
	    input_error ();

	  /* Check for a sign.  */
	  if (c == '-' || c == '+')
	    {
	      negative = c == '-';
	      if (inchar () == EOF)
		/* EOF is only an input error before we read any chars.  */
		conv_error ();
	      if (width > 0)
		--width;
	    }
	  else
	    negative = 0;

	  got_dot = got_e = 0;
	  do
	    {
	      if (isdigit (c))
		ADDW (c);
	      else if (got_e && wp[wpsize - 1] == 'e'
		       && (c == '-' || c == '+'))
		ADDW (c);
	      else if (wpsize > 0 && !got_e && tolower (c) == 'e')
		{
		  ADDW ('e');
		  got_e = got_dot = 1;
		}
	      else if (c == decimal && !got_dot)
		{
		  ADDW (c);
		  got_dot = 1;
		}
	      else if ((flags & GROUP) && c == thousands && !got_dot)
		ADDW (c);
	      else
		{
		  /* The last read character is not part of the number
		     anymore.  */
		  ungetc (c, s);
		  break;
		}
	      if (width > 0)
		--width;
	    }
	  while (width != 0 && inchar () != EOF);

	  if (wpsize == 0)
	    conv_error ();

	  /* Convert the number.  */
	  ADDW ('\0');
	  if (flags & LONGDBL)
	    {
	      long double d = __strtold_internal (wp, &tw, flags & GROUP);
	      if (!(flags & SUPPRESS) && tw != wp)
		*ARG (long double *) = negative ? -d : d;
	    }
	  else if (flags & LONG)
	    {
	      double d = __strtod_internal (wp, &tw, flags & GROUP);
	      if (!(flags & SUPPRESS) && tw != wp)
		*ARG (double *) = negative ? -d : d;
	    }
	  else
	    {
	      float d = __strtof_internal (wp, &tw, flags & GROUP);
	      if (!(flags & SUPPRESS) && tw != wp)
		*ARG (float *) = negative ? -d : d;
	    }

	  if (tw == wp)
	    conv_error ();

	  if (!(flags & SUPPRESS))
	    ++done;
	  break;

	case '[':	/* Character class.  */
	  if (flags & LONG)
	    {
	      STRING_ARG (wstr, wchar_t);
	      c = '\0';		/* This is to keep gcc quiet.  */
	    }
	  else
	    {
	      STRING_ARG (str, char);

	      c = inchar ();
	      if (c == EOF)
		input_error ();
	    }

	  if (*f == '^')
	    {
	      ++f;
	      not_in = 1;
	    }
	  else
	    not_in = 0;

	  /* Fill WP with byte flags indexed by character.
	     We will use this flag map for matching input characters.  */
	  if (wpmax < UCHAR_MAX)
	    {
	      wpmax = UCHAR_MAX;
	      wp = (char *) alloca (wpmax);
	    }
	  memset (wp, 0, UCHAR_MAX);

	  fc = *f;
	  if (fc == ']' || fc == '-')
	    {
	      /* If ] or - appears before any char in the set, it is not
		 the terminator or separator, but the first char in the
		 set.  */
	      wp[fc] = 1;
	      ++f;
	    }

	  while ((fc = *f++) != '\0' && fc != ']')
	    {
	      if (fc == '-' && *f != '\0' && *f != ']' &&
		  (unsigned char) f[-2] <= (unsigned char) *f)
		{
		  /* Add all characters from the one before the '-'
		     up to (but not including) the next format char.  */
		  for (fc = f[-2]; fc < *f; ++fc)
		    wp[fc] = 1;
		}
	      else
		/* Add the character to the flag map.  */
		wp[fc] = 1;
	    }
	  if (fc == '\0')
	    {
	      if (!(flags & LONG))
		ungetc (c, s);
	      conv_error();
	    }

	  if (flags & LONG)
	    {
	      wint_t val;
	      int first = 1;

	      do
		{
		  size_t cnt = 0;
		  NEXT_WIDE_CHAR (first);
		  if (val > 255 || wp[val] == not_in)
		    {
		      /* XXX We have a problem here.  We read a wide
			 character and this possibly took several
			 bytes.  But we can only push back one single
			 character.  To be sure we don't create wrong
			 input we push it back only in case it is
			 representable within one byte.  */
		      if (val < 0x80)
			ungetc (val, s);
		      break;
		    }
		  STRING_ADD_CHAR (wstr, val, wchar_t);
		  if (width > 0)
		    --width;
		  first = 0;
		}
	      while (width != 0);

	      if (first)
		conv_error ();

	      if (!(flags & SUPPRESS))
		{
		  *wstr = L'\0';
		  ++done;
		}
	    }
	  else
	    {
	      num.ul = read_in - 1; /* -1 because we already read one char.  */
	      do
		{
		  if (wp[c] == not_in)
		    {
		      ungetc (c, s);
		      break;
		    }
		  STRING_ADD_CHAR (str, c, char);
		  if (width > 0)
		    --width;
		}
	      while (width != 0 && inchar () != EOF);

	      if (read_in == num.ul)
		conv_error ();

	      if (!(flags & SUPPRESS))
		{
		  *str = '\0';
		  ++done;
		}
	    }
	  break;

	case 'p':	/* Generic pointer.  */
	  base = 16;
	  /* A PTR must be the same size as a `long int'.  */
	  flags &= ~(SHORT|LONGDBL);
	  flags |= LONG;
	  number_signed = 0;
	  goto number;
	}
    }

  /* The last thing we saw int the format string was a white space.
     Consume the last white spaces.  */
  if (skip_space)
    {
      do
	c = inchar ();
      while (isspace (c));
      ungetc (c, s);
    }

  /* Unlock stream.  */
  UNLOCK_STREAM;

  return done;
}

#ifdef USE_IN_LIBIO
int
__vfscanf (FILE *s, const char *format, va_list argptr)
{
  return _IO_vfscanf (s, format, argptr, NULL);
}
#endif

weak_alias (__vfscanf, vfscanf)
