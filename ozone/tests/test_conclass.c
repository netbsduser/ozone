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

int main ()

{
  oz_dev_conclass_init ();
}



void oz_knl_printk (const char *format, ...)

{
  va_list ap;

  va_start (ap, format);
  vprintf (format, ap);
  va_end (ap);
}

void *oz_knl_devclass_create ()

{
  return ("");
}

void *oz_knl_devdriver_create ()

{
  return ("");
}

void *oz_knl_devunit_create (void *devdriver, void *devname, void *devdesc, OZ_Functab *functab, int dummy1, void *dummy2)

{
  save_functab = functab;
  return (malloc (save_functab -> devex_size));
}

void oz_knl_iochan_increfc ()

{}

void oz_crash (const char *format, ...)

{
  va_list ap;

  va_start (ap, format);
  vprintf (format, ap);
  va_end (ap);
  printf ("\n");
}

void oz_knl_iodone (void *ioop, uLong status, void *dummy1, void *dummy2, void *dummy3)

{
  *((uLong *)ioop) = status;
  sys$wake (0, 0);
}

