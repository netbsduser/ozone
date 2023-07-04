//+++2001-11-18
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
//---2001-11-18

/************************************************************************/
/*									*/
/*  AST blocks - used to cause any thread in the system to call an 	*/
/*               arbitrary routine					*/
/*									*/
/*    Properties:							*/
/*									*/
/*	next,prev : pointers to next/prev in a list of ast blocks	*/
/*	   thread : target thread					*/
/*	 procmode : target processor mode (kernel, user, etc)		*/
/*	 astentry : target routine entrypoint				*/
/*	 astparam : target routine pointer parameter			*/
/*	  express : deliver even if ast delivery inhibited		*/
/*	   aststs : target routine status parameter			*/
/*									*/
/*    Operations:							*/
/*									*/
/*	create : mallocs and initializes an ast block			*/
/*	insert : inserts ast block in arbitrary ast list		*/
/*	remove : removes ast block from arbitrary ast list		*/
/*	delete : frees the ast block					*/
/*									*/
/*    Note:								*/
/*									*/
/*	Only the thread routines should call the delete function.  All 	*/
/*	others should call oz_thread_queueast to queue the ast to the 	*/
/*	intended target thread.  They can use a failure status to 	*/
/*	indicate that the operation has been cancelled, but they 	*/
/*	should always queue the ast to the thread, even though the 	*/
/*	application program requested that the operation be cancelled.	*/
/*									*/
/************************************************************************/

#ifndef _OZ_KNL_AST_H
#define _OZ_KNL_AST_H

/* Dummy ast block definition */

#ifdef _OZ_KNL_AST_C
typedef struct OZ_Ast OZ_Ast;
#else
typedef void OZ_Ast;
#endif

/* Ast enable flags */

typedef enum { OZ_ASTMODE_RDONLY, 
               OZ_ASTMODE_ENABLE, 
               OZ_ASTMODE_INHIBIT, 
               OZ_ASTMODE_INHEXP
             } OZ_Astmode;

/* Ast entry prototype */

#include "oz_knl_hw_dep.h"
typedef void (*OZ_Astentry) (void *astparam, uLong aststs, OZ_Mchargs *mchargs);

/* Includes needed for routine prototypes */

#include "oz_knl_hw.h"
#include "oz_knl_procmode.h"
#include "oz_knl_thread.h"

/* Global routine prototypes */

uLong oz_knl_ast_create (OZ_Thread *thread, 
                         OZ_Procmode procmode, 
                         OZ_Astentry astentry, 
                         void *astparam, 
                         int express, 
                         OZ_Ast **ast_r);
OZ_Ast **oz_knl_ast_insert (OZ_Ast *ast, OZ_Ast **list);
void oz_knl_ast_remove (OZ_Ast *ast);
void oz_knl_ast_delete (OZ_Ast *ast, OZ_Astentry *astentry, void **astparam, uLong *aststs);
OZ_Ast *oz_knl_ast_getnext (OZ_Ast *ast);
OZ_Procmode oz_knl_ast_getprocmode (OZ_Ast *ast);
OZ_Thread *oz_knl_ast_getthread (OZ_Ast *ast);
int oz_knl_ast_getexpress (OZ_Ast *ast);
void oz_knl_ast_setstatus (OZ_Ast *ast, uLong aststs);
int oz_knl_ast_validateq (OZ_Ast **queueh, OZ_Ast **queuet);
void oz_knl_ast_dumplist (int indent, OZ_Ast *astl);
void oz_knl_ast_dumpone (int indent, OZ_Ast *ast);

#endif
