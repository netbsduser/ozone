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

#include "oz_knl_lock.h"

static unsigned char compatible[OZ_LOCKMODE_XX], downconvert[OZ_LOCKMODE_XX], upconvert[OZ_LOCKMODE_XX];

int main ()

{
  OZ_Lockmode oldmode, newmode;

    memset (compatible, 0, sizeof compatible);
    for (oldmode = 0; oldmode < OZ_LOCKMODE_XX; oldmode ++) {
      for (newmode = 0; newmode < OZ_LOCKMODE_XX; newmode ++) {
        if (OZ_LOCK_ALLOW_TEST (oldmode, OZ_LOCK_ALLOWS_SELF_READ)  && !OZ_LOCK_ALLOW_TEST (newmode, OZ_LOCK_ALLOWS_OTHERS_READ))  continue;
        if (OZ_LOCK_ALLOW_TEST (oldmode, OZ_LOCK_ALLOWS_SELF_WRITE) && !OZ_LOCK_ALLOW_TEST (newmode, OZ_LOCK_ALLOWS_OTHERS_WRITE)) continue;
        if (OZ_LOCK_ALLOW_TEST (newmode, OZ_LOCK_ALLOWS_SELF_READ)  && !OZ_LOCK_ALLOW_TEST (oldmode, OZ_LOCK_ALLOWS_OTHERS_READ))  continue;
        if (OZ_LOCK_ALLOW_TEST (newmode, OZ_LOCK_ALLOWS_SELF_WRITE) && !OZ_LOCK_ALLOW_TEST (oldmode, OZ_LOCK_ALLOWS_OTHERS_WRITE)) continue;
        compatible[oldmode] |= (1 << newmode);
      }
    }

    for (oldmode = 0; oldmode < OZ_LOCKMODE_XX; oldmode ++) {
      printf ("compatible[%d] = 0%2.2o\n", oldmode, compatible[oldmode]);
    }

    /* This table tells us if a lock is converted from the subscript's mode to the bitmask's mode, it will possibly allow other locks to be granted - */
    /* - if the new mode allows something by others that the old mode did not, then it is a down-conversion                                           */

    memset (downconvert, 0, sizeof downconvert);
    for (oldmode = 0; oldmode < OZ_LOCKMODE_XX; oldmode ++) {
      for (newmode = 0; newmode < OZ_LOCKMODE_XX; newmode ++) {
        if  (OZ_LOCK_ALLOW_TEST (oldmode, OZ_LOCK_ALLOWS_SELF_READ)    && !OZ_LOCK_ALLOW_TEST (newmode, OZ_LOCK_ALLOWS_SELF_READ))    downconvert[oldmode] |= (1 << newmode);
        if  (OZ_LOCK_ALLOW_TEST (oldmode, OZ_LOCK_ALLOWS_SELF_WRITE)   && !OZ_LOCK_ALLOW_TEST (newmode, OZ_LOCK_ALLOWS_SELF_WRITE))   downconvert[oldmode] |= (1 << newmode);
        if (!OZ_LOCK_ALLOW_TEST (oldmode, OZ_LOCK_ALLOWS_OTHERS_READ)  &&  OZ_LOCK_ALLOW_TEST (newmode, OZ_LOCK_ALLOWS_OTHERS_READ))  downconvert[oldmode] |= (1 << newmode);
        if (!OZ_LOCK_ALLOW_TEST (oldmode, OZ_LOCK_ALLOWS_OTHERS_WRITE) &&  OZ_LOCK_ALLOW_TEST (newmode, OZ_LOCK_ALLOWS_OTHERS_WRITE)) downconvert[oldmode] |= (1 << newmode);
      }
    }

    for (oldmode = 0; oldmode < OZ_LOCKMODE_XX; oldmode ++) {
      printf ("downconvert[%d] = 0%2.2o\n", oldmode, downconvert[oldmode]);
    }

    /* This table tells us if a lock is converted from the subscript's mode to the bitmask's mode, it will possibly be blocked */
    /* - if the new mode requires something that the old mode did not, then it is an up-conversion                             */

    memset (upconvert, 0, sizeof upconvert);
    for (oldmode = 0; oldmode < OZ_LOCKMODE_XX; oldmode ++) {
      for (newmode = 0; newmode < OZ_LOCKMODE_XX; newmode ++) {
        /* The new mode requires that block_reads == 0 (ie, that this be allowed to read) */
        if (!OZ_LOCK_ALLOW_TEST (oldmode, OZ_LOCK_ALLOWS_SELF_READ)    &&  OZ_LOCK_ALLOW_TEST (newmode, OZ_LOCK_ALLOWS_SELF_READ))    upconvert[oldmode] |= (1 << newmode);
        /* The new mode requires that block_writes == 0 (ie, that this be allowed to write) */
        if (!OZ_LOCK_ALLOW_TEST (oldmode, OZ_LOCK_ALLOWS_SELF_WRITE)   &&  OZ_LOCK_ALLOW_TEST (newmode, OZ_LOCK_ALLOWS_SELF_WRITE))   upconvert[oldmode] |= (1 << newmode);
        /* The new mode requires that active_readers == 0 (ie, there be no other readers) */
        if  (OZ_LOCK_ALLOW_TEST (oldmode, OZ_LOCK_ALLOWS_OTHERS_READ)  && !OZ_LOCK_ALLOW_TEST (newmode, OZ_LOCK_ALLOWS_OTHERS_READ))  upconvert[oldmode] |= (1 << newmode);
        /* The new mode requires that active_writers == 0 (ie, there be no other writers) */
        if  (OZ_LOCK_ALLOW_TEST (oldmode, OZ_LOCK_ALLOWS_OTHERS_WRITE) && !OZ_LOCK_ALLOW_TEST (newmode, OZ_LOCK_ALLOWS_OTHERS_WRITE)) upconvert[oldmode] |= (1 << newmode);
      }
    }

    for (oldmode = 0; oldmode < OZ_LOCKMODE_XX; oldmode ++) {
      printf ("upconvert[%d] = 0%2.2o\n", oldmode, upconvert[oldmode]);
    }

  return (0);
}
