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
/*  Test PS/2 mouse							*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_knl_status.h"
#include "oz_sys_callknl.h"

/* Aux controller ports */

#define AUX_INPUT_PORT	0x60		/* Aux device output buffer */
#define AUX_OUTPUT_PORT	0x60		/* Aux device input buffer */
#define AUX_COMMAND	0x64		/* Aux device command buffer */
#define AUX_STATUS	0x64		/* Aux device status reg */

/* Commands written to AUX_COMMAND */

#define AUX_CMD_WRITE	0x60		/* value to write to controller */
					/* - the next byte written to AUX_OUTPUT_PORT is a 'command byte' */
#define AUX_DISABLE	0xa7		/* disable aux device */
#define AUX_ENABLE	0xa8		/* enable aux device */
#define AUX_MAGIC_WRITE	0xd4		/* value to send aux device data */
					/* - the next byte written to AUX_OUTPUT_PORT is transmitted to mouse */

/* Aux controller status bits                                               */
/* - bit definitions:                                                       */
/*   <0> : 0=no byte is waiting in AUX_INPUT_PORT                           */
/*         1=there is a byte waiting in AUX_INPUT_PORT to be read           */
/*   <1> : 0=it is ok to write a new byte to AUX_COMMAND or AUX_OUTPUT_PORT */
/*         1=ctrlr is busy with byte in AUX_COMMAND or AUX_OUTPUT_PORT      */
/*   <2> : 0=self test failed                                               */
/*         1=self test passed                                               */
/*   <3> : 0=last byte written was to AUX_DATA_PORT                         */
/*         1=last byte written was to AUX_COMMAND                           */
/*   <4> : 0=keyboard is disabled                                           */
/*         1=keyboard is enabled                                            */
/*   <5> : 0=byte in AUX_INPUT_PORT is from the keyboard                    */
/*         1=byte in AUX_INPUT_PORT is from the mouse                       */
/*   <6> : timeout error flag                                               */
/*   <7> : parity error flag                                                */

#define AUX_OBUF_FULL	0x21		/* output buffer (from device) full */
#define AUX_IBUF_FULL	0x02		/* input buffer (to device) full */

/* Aux controller commands - sent following AUX_CMD_WRITE */
/* - bit definitions:                                     */
/*   <0> : 0=disable keyboard interrupt                   */
/*         1=enable keyboard interrupt                    */
/*   <1> : 0=disable mouse interrupt                      */
/*         1=enable mouse interrupt                       */
/*   <2> : 0=self test failed                             */
/*         1=self test succeeded                          */
/*   <3> : 0=PC/AT inhibit override (1=enabled always)    */
/*         (must be 0 on PS/2 systems)                    */
/*   <4> : 0=no action                                    */
/*         1=disable keyboard                             */
/*   <5> : 0=no action                                    */
/*         1=disable mouse                                */
/*   <6> : pc compatibility mode                          */
/*         1=translate kbd codes to PC scan codes         */
/*   <7> : must be zero                                   */

#define AUX_INTS_ON	0x47		/* 'command byte' : enable controller mouse interrupts */
#define AUX_INTS_OFF	0x65		/* 'command byte' : disable controller mouse interrupts */

/* Aux device commands - sent following AUX_MAGIC_WRITE */

#define AUX_SET_RES	0xe8		/* set resolution */
#define AUX_SET_SCALE11	0xe6		/* set 1:1 scaling */
#define AUX_SET_SCALE21	0xe7		/* set 2:1 scaling */
#define AUX_GET_SCALE	0xe9		/* get scaling factor */
#define AUX_SET_STREAM	0xea		/* set stream mode */
#define AUX_SET_SAMPLE	0xf3		/* set sample rate */
#define AUX_ENABLE_DEV	0xf4		/* enable aux device */
#define AUX_DISABLE_DEV	0xf5		/* disable aux device */
#define AUX_RESET	0xff		/* reset aux device */

/* Other defines */

#define MAX_RETRIES	60		/* some aux operations take long time */
#define AUX_IRQ		12
#define AUX_BUF_SIZE	2048

/* Internal subroutines */

static uLong inknl (OZ_Procmode cprocmode, void *dummy);
static uLong getkeychar (void *dummy);
static int aux_write_ack (int val);
static int aux_write_cmd (int val);
static int aux_write_dev (int val);
static int poll_aux_status (void);

int main ()

{
  uLong sts;

  oz_sys_io_fs_printerror ("starting...\n");
  sts = oz_sys_callknl (inknl, NULL);
  oz_sys_io_fs_printerror ("status %u\n", sts);
  return (sts);
}

static uLong inknl (OZ_Procmode cprocmode, void *dummy)

{
  int i, j;
  uByte cb;

  oz_hw_cpu_sethwints (0);

#if defined INITIALIZE_DEVICE
  oz_knl_printk ("sending aux_enable command\n");
  if (!poll_aux_status ()) goto timedout;			/* wait for chip ready */
  oz_hw_outb (AUX_ENABLE, AUX_COMMAND);				/* enable mouse interface */

  oz_knl_printk ("sending sample rate\n");
  if (aux_write_ack (AUX_SET_SAMPLE) < 0) goto timedout;	/* set sample rate to ... */
  if (aux_write_ack (100) < 0) goto timedout;			/* ... 100 samples/sec */

  oz_knl_printk ("sending resolution\n");
  if (aux_write_ack (AUX_SET_RES) < 0) goto timedout;		/* set resolution to ... */
  if (aux_write_ack (3) < 0) goto timedout;			/* ... 8 counts per mm */

  oz_knl_printk ("sending scaling\n");
  if (aux_write_ack (AUX_SET_SCALE21) < 0) goto timedout;	/* 2:1 scaling */
#endif

  /* Disable device */

  oz_knl_printk ("sending aux_disable\n");
  if (!poll_aux_status ()) goto timedout;		/* wait for chip ready */
  oz_hw_outb (AUX_DISABLE, AUX_COMMAND);		/* disable mouse interface */

  oz_knl_printk ("sending aux_ints_off\n");
  if (!aux_write_cmd (AUX_INTS_OFF)) goto timedout;	/* turn off aux interrupts */

  /* Turn it on */

  oz_knl_printk ("sending aux_enable\n");
  if (!poll_aux_status ()) goto timedout;		/* wait for device idle */
  oz_hw_outb (AUX_ENABLE, AUX_COMMAND);			/* enable aux */

  oz_knl_printk ("sending aux_enable_dev\n");
  if (!aux_write_dev (AUX_ENABLE_DEV)) goto timedout;	/* enable aux device */
  if (!aux_write_cmd (AUX_INTS_ON)) goto timedout;	/* enable controller ints */

  /* Scan for 5 seconds */

  oz_knl_printk ("scanning\n");
  oz_hw_stl_delay (5000000, getkeychar, NULL);
  oz_knl_printk ("\ndone\n");

  /* Turn the mouse off */

  poll_aux_status ();
  aux_write_cmd (AUX_INTS_OFF);			/* disable controller ints */
  oz_hw_outb (AUX_DISABLE, AUX_COMMAND);	/* disable mouse interface */

  oz_hw_cpu_sethwints (1);
  return (OZ_SUCCESS);

  /* Failure, device timed out */

timedout:
  oz_hw_cpu_sethwints (1);
  return (OZ_TIMEDOUT);
}

/* This routine is called for 5 seconds to display anything that comes in */

static uLong getkeychar (void *dummy)

{
  uByte ah, al;

  al = oz_hw_inb (AUX_STATUS);		/* read status byte */
  oz_knl_printk ("\r %2.2x", al);	/* print it out */
  if (al & 1) {				/* see if anything in input port */
    ah = oz_hw_inb (AUX_INPUT_PORT);	/* read and print it */
    oz_knl_printk (" %c%2.2x\n", (al & 0x20) ? 'm' : 'k', ah);
  }
  return (0);				/* keep coming back until the 5 seconds are up */
}

/************************************************************************/
/*									*/
/*  Write to device & handle returned ack				*/
/*									*/
/*    Output:								*/
/*									*/
/*	aux_write_ack = < 0 : failure					*/
/*	               else : ack code					*/
/*									*/
/************************************************************************/

#if defined INITIALIZE_DEVICE
static int aux_write_ack (int val)

{
  int retries;
  uByte db, st;

  if (!aux_write_cmd (val)) return (-1);	/* write command code */
  for (retries = 1000000; -- retries > 0;) {
    st = oz_hw_inb (AUX_STATUS);		/* check status */
    if (!(st & 0x01)) continue;			/* keep looping until we get something */
    db = oz_hw_inb (AUX_INPUT_PORT);		/* read it */
    if (!(st & 0x20)) continue;			/* skip if from keyboard (it gets lost) */
    return (db);				/* from mouse, return it */
  }
  return (-1);					/* took too long, return failure */
}
#endif /* INITIALIZE_DEVICE */

/************************************************************************/
/*									*/
/*  Write 'command byte' in the controller				*/
/*									*/
/************************************************************************/

static int aux_write_cmd (int val)

{
  if (!poll_aux_status ()) return (0);		/* wait for chip ready */
  oz_hw_outb (AUX_CMD_WRITE, AUX_COMMAND);	/* tell it we are going to write a command byte */
  if (!poll_aux_status ()) return (0);		/* wait for chip ready */
  oz_hw_outb (val, AUX_OUTPUT_PORT);		/* send it the command byte */
  return (1);
}

/************************************************************************/
/*									*/
/*  Transmit data byte to mouse						*/
/*									*/
/************************************************************************/

static int aux_write_dev (int val)

{
  if (!poll_aux_status ()) return (0);		/* wait for chip ready */
  oz_hw_outb (AUX_MAGIC_WRITE, AUX_COMMAND);	/* write command that says next byte gets xmtd to mouse */
  if (!poll_aux_status ()) return (0);		/* wait for chip ready */
  oz_hw_outb (val, AUX_OUTPUT_PORT);		/* write data to mouse */
  return (1);
}

/************************************************************************/
/*									*/
/*  Wait for there to be nothing in input or output ports		*/
/*  If there is something waiting to be read, flush it			*/
/*									*/
/*    Output:								*/
/*									*/
/*	poll_aux_status = 0 : failed (timed out)			*/
/*	                  1 : successful				*/
/*									*/
/************************************************************************/

static int poll_aux_status (void)

{
  int retries;
  uByte st;

  for (retries = 1000000; -- retries > 0;) {	/* only wait so long */
    st = oz_hw_inb (AUX_STATUS);		/* read the status port */
    if (!(st & 0x02)) return (1);		/* if it is able to accept something, we're done */
    if (st & 0x01) oz_hw_inb (AUX_INPUT_PORT);	/* else, if it is hung-up with an byte, flush it */
  }
  return (0);					/* ran through all retries, return failure */
}
