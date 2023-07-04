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

/************************************************************************/
/*									*/
/*  Console port driver for linux					*/
/*									*/
/*  It handles the physical I/O to the keyboard and screen and passes 	*/
/*  the data to the high-level console class driver			*/
/*									*/
/************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "ozone.h"
#include "oz_dev_timer.h"
#include "oz_io_comport.h"
#include "oz_io_console.h"
#include "oz_io_fs.h"
#include "oz_knl_devio.h"
#include "oz_knl_hw.h"
#include "oz_knl_kmalloc.h"
#include "oz_knl_procmode.h"
#include "oz_knl_sdata.h"
#include "oz_knl_status.h"

typedef struct { OZ_Devunit *devunit;			/* console_#n devunit */
                 OZ_Iochan *conclass_iochan;		/* the class driver I/O channel */
                 OZ_IO_comport_setup comport_setup;	/* setup parameters */
                 int suspwriteflag;			/* set to suspend writing */
                 volatile int readinprog;		/* set when a read is in progress */
               } Devex;

typedef struct { uByte class_area[1];
               } Chnex;

typedef struct { uByte class_area[1];
               } Iopex;

static uLong ttyport_assign (OZ_Devunit *devunit, void *devexv, OZ_Iochan *iochan, void *chnexv, OZ_Procmode procmode);
static int ttyport_deassign (OZ_Devunit *devunit, void *devexv, OZ_Iochan *iochan, void *chnexv);
static void ttyport_abort (OZ_Devunit *devunit, void *devexv, OZ_Iochan *iochan, void *chnexv, OZ_Ioop *ioop, void *iopexv, OZ_Procmode procmode);
static uLong ttyport_start (OZ_Devunit *devunit, void *devexv, OZ_Iochan *iochan, void *chnex, OZ_Procmode procmode, 
                                OZ_Ioop *ioop, void *iopexv, uLong funcode, uLong as, void *ap);

static OZ_Devfunc ttyport_functable = { sizeof (Devex), sizeof (Chnex), sizeof (Iopex), 0, NULL, NULL, NULL, ttyport_assign, 
                                            ttyport_deassign, ttyport_abort, ttyport_start, NULL };

static int ttyifd = 0;	/* default: use stdin */
static int ttyofd = 1;	/* default: use stdout */
static int ttyisa = 0;
static OZ_Devclass *devclass;
static OZ_Devdriver *devdriver;
static Devex *devexs[1];
static Devex *cdevex = NULL;
static struct termios initial_settings, running_settings;
static int initialized = 0;

static void conport_read_start (void *devexv, int start);
static uLong conport_disp_start (void *devexv, void *write_param, uLong size, char *buff);
static void conport_disp_suspend (void *devexv, int suspend);
static void conport_kbd_rah_full (void *devexv, int full);
static void conport_terminate (void *devexv);
static uLong conport_getsetmode (void *devexv, void *getset_param, uLong size, OZ_Console_modebuff *buff);
static Devex *initdev (int vidx);

/************************************************************************/
/*									*/
/*  Boot-time initialization routine					*/
/*									*/
/************************************************************************/

void linux_dev_ttyport_init ()

{
  Devex *devex;

  if (!initialized) {
    oz_knl_printk ("linux_dev_ttyport_init\n");

    memset (devexs, 0, sizeof devexs);				/* aint got no devices yet */

    devclass  = oz_knl_devclass_create (OZ_IO_COMPORT_CLASSNAME, OZ_IO_COMPORT_BASE, OZ_IO_COMPORT_MASK, "linux_dev_ttyport");
    devdriver = oz_knl_devdriver_create (devclass, "linux_dev_ttyport");

    devex = initdev (0);					/* init devex struct */
    if (devex == NULL) return;					/* return if can't do it yet */
    ttyport_functable.chn_exsize += devex -> comport_setup.class_functab -> chn_exsize;
    ttyport_functable.iop_exsize += devex -> comport_setup.class_functab -> iop_exsize;
    cdevex = devex;						/* current screen = this one-and-only screen */
    initialized = 1;						/* we are now initialized */
  }
}

void linux_dev_ttyport_shut ()

{
  if (ttyisa) {
    if (tcsetattr (ttyifd, TCSANOW, &initial_settings) < 0) {
      fprintf (stderr, "error restoring terminal settings: %s\n", strerror (errno));
      exit (-1);
    }
  }
  if (ttyifd != 0) close (ttyifd);
  if (ttyofd != 1) close (ttyofd);
}

/************************************************************************/
/*									*/
/*  We just pass all the functab calls directly to the class driver	*/
/*									*/
/*  Class drivers do not have clonecre/clonedel.  They use 		*/
/*  OZ_IO_COMPORT_SETUP in place of clonecre, and deassigning that 	*/
/*  channel takes the place of clonedel.				*/
/*									*/
/************************************************************************/

/* A channel is being assigned to the device */

static uLong ttyport_assign (OZ_Devunit *devunit, void *devexv, OZ_Iochan *iochan, void *chnexv, OZ_Procmode procmode)

{
  Devex *devex;

  devex = devexv;
  return ((*(devex -> comport_setup.class_functab -> assign)) (devunit, devex -> comport_setup.class_devex, iochan, ((Chnex *)chnexv) -> class_area, procmode));
}

/* A channel is being deassigned from the device */

static int ttyport_deassign (OZ_Devunit *devunit, void *devexv, OZ_Iochan *iochan, void *chnexv)

{
  Devex *devex;

  devex = devexv;
  return ((*(devex -> comport_setup.class_functab -> deassign)) (devunit, devex -> comport_setup.class_devex, iochan, ((Chnex *)chnexv) -> class_area));
}

/* Abort an I/O function */

static void ttyport_abort (OZ_Devunit *devunit, void *devexv, OZ_Iochan *iochan, void *chnexv, OZ_Ioop *ioop, void *iopexv, OZ_Procmode procmode)

{
  Devex *devex;

  devex = devexv;
  (*(devex -> comport_setup.class_functab -> abort)) (devunit, devex -> comport_setup.class_devex, iochan, ((Chnex *)chnexv) -> class_area, ioop, ((Iopex *)iopexv) -> class_area, procmode);
}

/* Start an I/O function */

static uLong ttyport_start (OZ_Devunit *devunit, void *devexv, OZ_Iochan *iochan, void *chnexv, OZ_Procmode procmode, 
                            OZ_Ioop *ioop, void *iopexv, uLong funcode, uLong as, void *ap)

{
  Devex *devex;

  devex = devexv;
  return ((*(devex -> comport_setup.class_functab -> start)) (devunit, devex -> comport_setup.class_devex, iochan, 
                                                              ((Chnex *)chnexv) -> class_area, procmode, ioop, 
                                                              ((Iopex *)iopexv) -> class_area, funcode, as, ap));
}

/************************************************************************/
/*									*/
/*  This routine is called by the class driver when it is starting or 	*/
/*  finishing a read request						*/
/*									*/
/************************************************************************/

static void conport_read_start (void *devexv, int start)

{
  char bl;
  Devex *devex;
  int rc;

  devex = devexv;

  if (start == devex -> readinprog) oz_crash ("oz_dev_conport_read_start: start %d when already that", start);
  devex -> readinprog = start;
  if (start) {

    /* Loop reading from ttyifd and calling class_kbd_char routine.  Eventually, it */
    /* will call us back with start=0 so we will know when the read is complete.    */

    while (devex -> readinprog) {
      rc = read (ttyifd, &bl, 1);
      if (rc <= 0) oz_crash ("oz_dev_conport_read_start: rc %d, errno %d reading tty", rc, errno);
      if ((bl == 10) && !ttyisa) bl = 13;
      (*(cdevex -> comport_setup.class_kbd_char)) (cdevex -> comport_setup.class_param, bl);
    }
  }
}

/************************************************************************/
/*									*/
/*  This routine is called by the class driver when it wants to 	*/
/*  display something.							*/
/*									*/
/*    Input:								*/
/*									*/
/*	size = size of message to display				*/
/*	buff = address of message to display				*/
/*									*/
/*    Output:								*/
/*									*/
/*	conport_disp_start = OZ_SUCCESS : completed synchronously	*/
/*	                   OZ_QUEUEFULL : can't accept new request	*/
/*									*/
/************************************************************************/

static uLong conport_disp_start (void *devexv, void *write_param, uLong size, char *buff)

{
  Devex *devex;

  devex = devexv;
  if (devex -> suspwriteflag) return (OZ_QUEUEFULL);		/* if we're suspended, don't accept anything */
  write (ttyofd, buff, size);					/* ok, output it */
  return (OZ_SUCCESS);						/* it completed synchronously */
}

/************************************************************************/
/*									*/
/*  The class driver calls this routine when it wants us to stop 	*/
/*  displaying whatever it has told us to display, or when it wants us 	*/
/*  to resume.								*/
/*									*/
/************************************************************************/

static void conport_disp_suspend (void *devexv, int suspend)

{
  ((Devex *)devexv) -> suspwriteflag = suspend;	/* set new value for the flag */
}

/************************************************************************/
/*									*/
/*  The class driver calls this routine when its read-ahead buffer is 	*/
/*  full.								*/
/*									*/
/*    Input:								*/
/*									*/
/*	full = 0 : read-ahead buffer is no longer full			*/
/*	       1 : read-ahead buffer is full				*/
/*									*/
/************************************************************************/

static void conport_kbd_rah_full (void *devexv, int full)

{}

/************************************************************************/
/*									*/
/*  The class driver calls this when all channels have been deassigned 	*/
/*  from the device.  We don't try to clean up, just leave stuff as is.	*/
/*									*/
/************************************************************************/

static void conport_terminate (void *devexv)

{}

/************************************************************************/
/*									*/
/*  Get / Set modes							*/
/*									*/
/************************************************************************/

static uLong conport_getsetmode (void *devexv, void *getset_param, uLong size, OZ_Console_modebuff *buff)

{
  /* ?? do we want to update termios with the stuff ?? */
  return (OZ_SUCCESS);
}

/************************************************************************/
/*									*/
/*  Send a string to class driver as if it came from keyboard		*/
/*									*/
/************************************************************************/

void oz_dev_keyboard_send (void *devexv, int size, char *buff)

{
  Devex *devex;
  uLong kb;

  kb = oz_hw_smplock_wait (&oz_s_smplock_dv);
  devex = devexv;
  if ((devex != NULL) && initialized) {
    while (-- size >= 0) {
      (*(devex -> comport_setup.class_kbd_char)) (devex -> comport_setup.class_param, *(buff ++));
    }
  }
  oz_hw_smplock_clr (&oz_s_smplock_dv, kb);
}

/************************************************************************/
/*									*/
/*  This routine reads a line from the keyboard with interrupts 	*/
/*  disabled (used for kernel debugging)				*/
/*									*/
/*    Input:								*/
/*									*/
/*	size = max length to read from keyboard				*/
/*	buff = buffer to read them into					*/
/*	pmtsize = prompt string size					*/
/*	pmtbuff = prompt string buffer					*/
/*									*/
/*    Output:								*/
/*									*/
/*	oz_hw_getcon = 0 : end-of-file terminator			*/
/*	               1 : normal terminator				*/
/*	*buff = filled with null-terminated string			*/
/*									*/
/************************************************************************/

int oz_hw_getcon (uLong size, char *buff, uLong pmtsize, const char *pmtbuff)

{
  char c;
  int i, rc;

  oz_hw_putcon (pmtsize, pmtbuff);

  i = 0;
  while (1) {
    rc = read (ttyifd, &c, 1);
    if (rc <= 0) {
      buff[i] = 0;
      return (0);
    }
    if ((c == 10) && !ttyisa) c = 13;
    if (c == 13) {
      write (ttyofd, "\r\n", 2);
      buff[i] = 0;
      return (1);
    }
    if ((c == 4) || (c == 26)) {
      write (ttyofd, "\r\n", 2);
      buff[i] = 0;
      return (0);
    }
    while ((c == 21) && (i > 0)) {
      write (ttyofd, "\010 \010", 3);
      -- i;
      continue;
    }
    if ((c == 127) && (i > 0)) {
      write (ttyofd, "\010 \010", 3);
      -- i;
      continue;
    }
    if ((c >= ' ') && (i + 1 < size)) {
      write (ttyofd, &c, 1);
      buff[i++] = c;
    }
  }
}

/************************************************************************/
/*									*/
/*  Write a message to console with interrupts disabled			*/
/*									*/
/************************************************************************/

void oz_hw_putcon (uLong size, const char *buff)

{
  char c;

  while (size > 0) {
    c = *(buff ++);
    if (c == '\n') write (ttyofd, "\r\n", 2);
    else write (ttyofd, &c, 1);
    -- size;
  }
}

/************************************************************************/
/*									*/
/*  This routine is called by the debugger on the primary cpu when it 	*/
/*  is waiting for another cpu to execute.  It checks to see if 	*/
/*  control-shift-C has been pressed.					*/
/*									*/
/************************************************************************/

int oz_knl_console_debugchk (void)

{
  return (0);
}

/************************************************************************/
/*									*/
/*  Set up new device							*/
/*									*/
/*  Devices are created on demand (either via control-shift-n or 	*/
/*  autogen) because the vctx block is such a pig			*/
/*									*/
/************************************************************************/

static Devex *initdev (int vidx)

{
  char unitdesc[20], unitname[12], *ttname;
  Devex *devex;
  OZ_Devunit *devunit;
  OZ_Iochan *conclass_iochan;
  uLong dv, sts;

  /* Assign I/O channel to console class driver setup device - if not there, try later */

  sts = oz_knl_iochan_crbynm (OZ_IO_CONSOLE_SETUPDEV, OZ_LOCKMODE_CW, OZ_PROCMODE_KNL, NULL, &conclass_iochan);
  if (sts != OZ_SUCCESS) {
    oz_knl_printk ("oz_dev_console_init: error %u assigning channel to console class device 'conclass'\n", sts);
    return (NULL);
  }

  /* If device already exists, just return pointer */

  dv = oz_hw_smplock_wait (&oz_s_smplock_dv);
  devex = devexs[0];
  if (devex != NULL) {
    oz_hw_smplock_clr (&oz_s_smplock_dv, dv);
    oz_knl_iochan_increfc (conclass_iochan, -1);
    return (devex);
  }

  /* Create the devunit */

  devunit = oz_knl_devunit_create (devdriver, "console", "linux tty port", &ttyport_functable, 0, NULL);

  /* Set up keyboard context block */

  devexs[0] = devex = oz_knl_devunit_ex (devunit);
  memset (devex, 0, sizeof *devex);
  oz_hw_smplock_clr (&oz_s_smplock_dv, dv);

  devex -> devunit = devunit;
  devex -> conclass_iochan = conclass_iochan;

  /* Set up tty device */

  fflush (stdout);
  fflush (stderr);

  ttname = getenv ("TTYNAME");
  if ((ttname == NULL) || (ttname[0] == 0)) ttname = "/dev/tty";
  ttyifd = open (ttname, O_RDONLY);
  if (ttyifd < 0) {
    fprintf (stderr, "error opening terminal %s: %s\n", ttname, strerror (errno));
    exit (-1);
  }
  ttyisa = isatty (ttyifd);
  if (ttyisa) {
    ttyofd = open (ttname, O_WRONLY);
    if (ttyofd < 0) {
      fprintf (stderr, "error opening terminal %s for output: %s\n", ttname, strerror (errno));
      exit (-1);
    }
    if (tcgetattr (ttyifd, &initial_settings) < 0) {
      fprintf (stderr, "error getting terminal %s settings: %s\n", ttname, strerror (errno));
      exit (-1);
    }
    running_settings = initial_settings;
    running_settings.c_iflag &= ~(INLCR | IGNCR | ICRNL | IUCLC);     /* really screw it up by turning everything off */
    running_settings.c_oflag &= ~(OPOST | OLCUC | ONLCR | OCRNL | ONOCR | ONLRET | OFILL);
    running_settings.c_lflag &= ~(ISIG | ICANON | ECHO);
    if (tcsetattr (ttyifd, TCSANOW, &running_settings) < 0) {
      fprintf (stderr, "error setting terminal %s setting: %s\n", ttname, strerror (errno));
      exit (-1);
    }
  }

  /* Send port parameters to class driver, get class driver parameters back */

  devex -> comport_setup.port_devunit      = devunit;
  devex -> comport_setup.port_param        = devex;
  devex -> comport_setup.port_read_start   = conport_read_start;
  devex -> comport_setup.port_disp_start   = conport_disp_start;
  devex -> comport_setup.port_disp_suspend = conport_disp_suspend;
  devex -> comport_setup.port_kbd_rah_full = conport_kbd_rah_full;
  devex -> comport_setup.port_terminate    = conport_terminate;
  devex -> comport_setup.port_getsetmode   = conport_getsetmode;
  devex -> comport_setup.port_lkprm        = &oz_s_smplock_dv;
  devex -> comport_setup.port_lockdb       = (void *)oz_hw_smplock_wait;
  devex -> comport_setup.port_unlkdb       = (void *)oz_hw_smplock_clr;

  sts = oz_knl_io (devex -> conclass_iochan, OZ_IO_COMPORT_SETUP, sizeof devex -> comport_setup, &(devex -> comport_setup));
  if (sts != OZ_SUCCESS) oz_crash ("oz_dev_console_init: error %u setting up console device\n", sts);

  return (devex);
}
