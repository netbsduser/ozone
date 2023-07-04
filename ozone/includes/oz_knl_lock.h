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

#ifndef _OZ_KNL_LOCK_H
#define _OZ_KNL_LOCK_H

typedef enum { OZ_LOCKMODE_NL = 0,	/*            NulL : i can't do anything,  others can read or write */
               OZ_LOCKMODE_CR = 1,	/*  ConcurrentRead : i can read,           others can read or write */
               OZ_LOCKMODE_CW = 2,	/* ConcurrentWrite : i can read and write, others can read or write */
               OZ_LOCKMODE_PR = 3,	/*   ProtectedRead : i can read,           others can read */
               OZ_LOCKMODE_PW = 4,	/*  ProtectedWrite : i can read and write, others can read */
               OZ_LOCKMODE_EX = 5,	/*       EXclusive : i can read and write, others can't do anything */
               OZ_LOCKMODE_XX = 6	/* - number of different lock modes */
             } OZ_Lockmode;

#define OZ_LOCK_ALLOW_TEST(__lockmode,__forwhat) (((1 << (__lockmode)) & __forwhat) != 0)
#define OZ_LOCK_ALLOWS_SELF_READ (076)
#define OZ_LOCK_ALLOWS_SELF_WRITE (064)
#define OZ_LOCK_ALLOWS_OTHERS_READ (037)
#define OZ_LOCK_ALLOWS_OTHERS_WRITE (007)

#ifdef _OZ_KNL_LOCK_TABLES

	/* given mode_a and mode_b, this says whether the two locks can co-exist */
	/* they_can = (oz_lock_compat[mode_a] >> mode_b) & 1                     */

static const uByte oz_lock_compat[6] = { 077,	/* NL - others can do NL, CR, CW, PR, PW, EX */
                                         037,	/* CR - others can do NL, CR, CW, PR, PW     */
                                         007,	/* CW - others can do NL, CR, CW             */
                                         013,	/* PR - others can do NL, CR,     PR         */
                                         003,	/* PW - others can do NL, CR                 */
                                         001 };	/* EX - others can do NL                     */

	/* will converting from oldmode to newmode possibly let other locks be granted */
	/* they_possibly_can = (oz_lock_dncnvt[oldmode] >> newmode) & 1                */

static const uByte oz_lock_dncnvt[6] = { 000,	/* NL -> nothing            */
                                         001,	/* CR -> NL                 */
                                         013,	/* CW -> NL, CR, PR         */
                                         007,	/* PR -> NL, CR, CW         */
                                         017,	/* PW -> NL, CR, CW, PR     */
                                         037 };	/* EX -> NL, CR, CW, PR, PW */

	/* will converting from oldmode to newmode possibly cause a wait */
	/* it_might = (oz_lock_upcnvt[oldmode] >> newmode) & 1           */

static const uByte oz_lock_upcnvt[6] = { 076,	/* NL -> CR, CW, PR, PW, EX */
                                         074,	/* CR -> CW, PR, PW, EX     */
                                         070,	/* CW -> PR, PW, EX         */
                                         064,	/* PR -> CW, PW, EX         */
                                         040,	/* PW -> EX                 */
                                         000 };	/* EX -> nothing            */

#endif
#endif
