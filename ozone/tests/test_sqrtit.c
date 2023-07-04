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

int main ()
{
  char inbuf[32];
  unsigned int c, n, s, t;

  gets (inbuf);
  s = atoi (inbuf);
  n = 0;
  for (c = s / 2 + 1; (t = (s / c + c + 1) / 2) != c; c = t) n ++;
  printf ("s %u, c %u, c*c %u, n %u\n", s, c, c * c, n);
  return (1);
}
