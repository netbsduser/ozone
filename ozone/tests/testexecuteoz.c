//+++2002-05-10
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
//---2002-05-10

#include <stdio.h>

//int execute_oz (int verbose_flag, int argbuf_index, const char **argbuf);

int main (int argc, char **argv)

{
  return (execute_oz (1, argc - 1, argv + 1));
}

/************************************************************************/
/*									*/
/*  Execute the command specified by the arguments on the current line 	*/
/*  of spec.  When using pipes, this includes several piped-together 	*/
/*  commands with `|' between them.					*/
/*									*/
/*    Input:								*/
/*									*/
/*	argbuf_index = number of elements in argbuf			*/
/*	argbuf = list of arguments					*/
/*									*/
/*    Output:								*/
/*									*/
/*	Return 0 if successful, -1 if failed.				*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_io_pipe.h"
#include "oz_knl_status.h"
#include "oz_sys_event.h"
#include "oz_sys_handle.h"
#include "oz_sys_io.h"
#include "oz_sys_io_fs_printf.h"
#include "oz_sys_spawn.h"

int execute_oz (int verbose_flag, int argbuf_index, const char **argbuf)

{
  char imagename[256];
  int i, j, k, l, rc;
  int n_commands;		/* # of commands */
  char *string;
  uLong sts;
  OZ_Handle h_event, h_last_input, h_this_output;
  struct command
    {
      int argc;			/* count of args */
      const char **argv;	/* vector of args ([0] is prog name) */
      OZ_Handle h_thread;	/* thread handle */
      uLong exitsts;		/* exit status */
    };

  struct command *commands;	/* each command buffer with above info.  */

  /* Count number of commands */

  n_commands = 1;
  for (i = 0; i < argbuf_index; i++) if (strcmp (argbuf[i], "|") == 0) n_commands ++;

  /* Get storage for each command.  */

  commands = (struct command *) alloca (n_commands * sizeof *commands);

  /* Start each command */

  oz_sys_event_create (OZ_PROCMODE_KNL, "gcc spawn", &h_event);
  h_last_input = 0;
  j = 0;
  k = 0;
  for (i = 0; i < argbuf_index; i++) {
    if (i == k) commands[j].argv = argbuf + i;
    if (strcmp (argbuf[i], "|") == 0) {
      commands[j].argc = i - k;
      sprintf (imagename, "%s.oz", commands[j].argv[0]);
      if (verbose_flag) {
        oz_sys_io_fs_printerror ("gcc: spawning %s:\n", imagename);
        for (l = 0; l < commands[j].argc; l ++) {
          oz_sys_io_fs_printerror (" %s", commands[j].argv[l]);
        }
        oz_sys_io_fs_printerror ("\n");
      }
      sts = oz_sys_io_assign (OZ_PROCMODE_KNL, &h_this_output, OZ_IO_PIPES_TEMPLATE, OZ_LOCKMODE_EX);
      if (sts != OZ_SUCCESS) {
        oz_sys_io_fs_printerror ("gcc: error %u creating pipe\n", sts);
        return (-1);
      }
      sts = oz_sys_spawn (0, imagename, h_last_input, h_this_output, 0, 0, h_event, NULL, 
                          commands[j].argc, commands[j].argv, commands[j].argv[0], &(commands[j].h_thread), NULL);
      if (sts != OZ_SUCCESS) {
        oz_sys_io_fs_printerror ("gcc: error %u spawning %s command\n", sts, commands[j].argv[0]);
        return (-1);
      }
      if (h_last_input != 0) oz_sys_handle_release (OZ_PROCMODE_KNL, h_last_input);
      h_last_input = h_this_output;
      j ++;
      k = i + 1;
    }
  }

  commands[j].argc = i - k;
  sprintf (imagename, "%s.oz", commands[j].argv[0]);
  if (verbose_flag) {
    oz_sys_io_fs_printerror ("gcc: spawning %s:\n", imagename);
    for (l = 0; l < commands[j].argc; l ++) {
      oz_sys_io_fs_printerror (" %s", commands[j].argv[l]);
    }
    oz_sys_io_fs_printerror ("\n");
  }
  sts = oz_sys_spawn (0, imagename, h_last_input, 0, 0, 0, h_event, NULL, 
                      commands[j].argc, commands[j].argv, commands[j].argv[0], &(commands[j].h_thread), NULL);
  if (sts != OZ_SUCCESS) {
    oz_sys_io_fs_printerror ("gcc: error %u spawning %s command\n", sts, commands[j].argv[0]);
    return (-1);
  }
  if (h_last_input != 0) oz_sys_handle_release (OZ_PROCMODE_KNL, h_last_input);

  /* Wait for them all to exit */

  for (j = 0; j < n_commands; j ++) {
    while ((sts = oz_sys_thread_getexitsts (commands[j].h_thread, &(commands[j].exitsts))) == OZ_FLAGWASCLR) {
      oz_sys_event_wait (OZ_PROCMODE_KNL, h_event, 0);
      oz_sys_event_set (OZ_PROCMODE_KNL, h_event, 0, NULL);
    }
    if (sts != OZ_SUCCESS) {
      oz_sys_io_fs_printerror ("gcc: error %u waiting for %s command\n", sts, commands[j].argv[0]);
      return (-1);
    }
    oz_sys_handle_release (OZ_PROCMODE_KNL, commands[j].h_thread);
    commands[j].h_thread = 0;
  }

  /* Check their exit statuses */

  rc = 0;
  for (j = 0; j < n_commands; j ++) {
    if (commands[j].exitsts != OZ_SUCCESS) {
      oz_sys_io_fs_printerror ("gcc: exit status %u from %s command\n", commands[j].exitsts, commands[j].argv[0]);
      rc = -1;
    }
  }

  return (rc);
}
