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

#ifndef _OZ_CRTL_FIO_H
#define _OZ_CRTL_FIO_H

#include <stdarg.h>
#include <sys/types.h>

/* Make sure NULL is defined */

#ifndef NULL
#define NULL ((void *)0)
#endif

/* EOF indicator return value */

#define EOF (-1)

/* Buffering modes: none, line, full */

#define _IONBF (1)
#define _IOLBF (2)
#define _IOFBF (3)

/* Default block buffer size */

#define BUFSIZ (4096)

/* File structure */

#ifdef _OZ_CRTL_FIO_C
typedef struct FILE FILE;
#else
typedef void FILE;
#endif

/* File position (offset from beginning of file) */

typedef long fpos_t;

/* Static files */

extern FILE *stdin, *stdout, *stderr;

/* Routines defined by macros */

#define getc(__stream) fgetc (__stream)
#define getchar() fgetc (stdin)
#define putc(__c,__stream) fputc (__c, __stream)
#define putchar(__c) fputc (__c, stdout)
#define setbuf(__stream,__buf) setbuffer (__stream, __buf, BUFSIZ)
#define setbuffer(__stream,__buf,__size) setvbuf (__stream, __buf, (__buf == NULL) ? _IOFBF : _IONBF, __size)
#define setlinebuf(__stream) setvbuf (__stream, NULL, _IOLBF, 0)

/* Dummy out _unlocked versions for now (used by gcc) */

#define fputc_unlocked fputc
#define fputs_unlocked fputs
#define putc_unlocked(__c,__stream) fputc (__c, __stream)

/* Routines */

FILE *fopen (const char *path, const char *mode);
FILE *fdopen (int fildes, const char *mode);
FILE *freopen (const char *path, const char *mode, FILE *stream);
int fclose (FILE *stream);
int fread (void *ptr, size_t size, size_t nmemb, FILE *stream);
int fwrite (const void *ptr, size_t size, size_t nmemb, FILE *stream);
int sprintf (char *str, const char *format, ...);
int vsprintf (char *str, const char *format, va_list ap);
int asprintf (char **buf, const char *format, ...);
int vasprintf (char **buf, const char *format, va_list ap);
int printf (const char *format, ...);
int vprintf (const char *format, va_list ap);
int fprintf (FILE *stream, const char *format, ...);
int vfprintf (FILE *stream, const char *format, va_list ap);
int sscanf (const char *str, const char *format, ...);
int vsscanf (const char *str, const char *format, va_list ap);
int scanf (const char *format, ...);
int fscanf (FILE *file, const char *format, ...);
int vscanf (const char *format, va_list ap);
int vfscanf (FILE *stream, const char *format, va_list ap);
char *gets (char *s);
char *fgets (char *s, int size, FILE *stream);
int fgetc (FILE *stream);
int ungetc (int c, FILE *stream);
int puts (const char *s);
int fputs (const char *s, FILE *stream);
int fputc (int c, FILE *stream);
int fgetpos (FILE *stream, fpos_t *pos);
int fsetpos (FILE *stream, fpos_t *pos);
void rewind (FILE *stream);
long ftell (FILE *stream);
int fseek (FILE *stream, long offset, int whence);
int fflush (FILE *stream);
int fpurge (FILE *stream);
int setvbuf (FILE *stream, char *buf, int mode, size_t size);
void clearerr (FILE *stream);
int feof (FILE *stream);
int ferror (FILE *stream);
int fileno (FILE *stream);


#endif
