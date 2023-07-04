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

typedef unsigned int uLong;
typedef unsigned char OZ_Procmode;
typedef unsigned int OZ_Handle;
typedef struct { uLong value; uLong flags; } OZ_Logvalue;

#define OZ_SYSCALL_logname_create 20




#define OZ_HWAXP_SYSCALL_SIZEOF(tn) (((sizeof (tn)) + 7) & -8)

#define OZ_HW_SYSCALL_DEF_7(name,t1,v1,t2,v2,t3,v3,t4,v4,t5,v5,t6,v6,t7,v7)	\
uLong oz_sys_##name (t1 v1, t2 v2, t3 v3, t4 v4, t5 v5, t6 v6, t7 v7)		\
										\
{										\
  asm volatile ("\n"								\
	"	mov	%0,$6\n"						\
	"	mov	%1,$7\n"						\
	"	call_pal 0x83 # CHMK\n"						\
		:								\
		: "i" (OZ_HWAXP_SYSCALL_SIZEOF(t1)				\
                      +OZ_HWAXP_SYSCALL_SIZEOF(t2)				\
                      +OZ_HWAXP_SYSCALL_SIZEOF(t3)				\
                      +OZ_HWAXP_SYSCALL_SIZEOF(t4)				\
                      +OZ_HWAXP_SYSCALL_SIZEOF(t5)				\
                      +OZ_HWAXP_SYSCALL_SIZEOF(t6)				\
                      +OZ_HWAXP_SYSCALL_SIZEOF(t7)), 				\
		  "i" (OZ_SYSCALL_##name));					\
  return (0);									\
}										\
										\
uLong oz_syscall_##name (t1 v1, t2 v2, t3 v3, t4 v4, t5 v5, OZ_Procmode cprocmode, t6 v6, void *__dummy_pc__)

OZ_HW_SYSCALL_DEF_7 (logname_create, 
                     OZ_Handle, h_lognamtbl, 
                     const char *, name, 
                     OZ_Procmode, procmode, 
                     uLong, lognamatr, 
                     uLong, nvalues, 
                     OZ_Logvalue, values[], 
                     OZ_Handle *, h_logname_r)
{
  return (1145);
}
