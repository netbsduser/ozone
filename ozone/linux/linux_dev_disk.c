//+++2002-08-17
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
//---2002-08-17

/************************************************************************/
/*									*/
/*  Use a linux file as a hard drive					*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_knl_devio.h"
#include "oz_knl_hw.h"
#include "oz_knl_kmalloc.h"
#include "oz_knl_procmode.h"
#include "oz_knl_sdata.h"
#include "oz_knl_section.h"
#include "oz_knl_spte.h"
#include "oz_knl_status.h"

#include "oz_io_disk.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define L2_DISK_BLOCK_SIZE (devex -> l2bs)
#define DISK_BLOCK_SIZE (1 << devex -> l2bs)

/* Device extension structure */

typedef struct { char diskname[OZ_DEVUNIT_NAMESIZE];	/* linux device name */
                 int fd;				/* fd of device */
                 int readonly;				/* 0: read/write; 1: read-only */
                 int l2bs;				/* block size */
                 struct stat stat;			/* device's stat */
                 OZ_Dbn sec;				/* sectors-per-track */
                 OZ_Dbn trk;				/* tracks-per-cylinder */
                 OZ_Dbn lbnoffs;			/* target's lbn offset */
               } Devex;

/* Function table */

static uLong linux_disk_start (OZ_Devunit *devunit, void *devexv, OZ_Iochan *iochan, void *chnexv, OZ_Procmode procmode, 
                               OZ_Ioop *ioop, void *iopexv, uLong funcode, uLong as, void *ap);

static OZ_Devfunc linux_disk_functable = { sizeof (Devex), 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, linux_disk_start, NULL };

/* Internal static data */

static int initialized = 0;
static OZ_Devclass *devclass;
static OZ_Devdriver *devdriver;
static OZ_Devunit *devunit;

/* Internal routines */

static int parcpy_from_u (void *dst, void *u_src, uLong len, uLong u_len, OZ_Procmode procmode);
static int memcpy_from_u (void *dst, void *u_src, uLong len, OZ_Procmode procmode);
static int memcpy_to_u (void *u_dst, void *src, uLong len, OZ_Procmode procmode);
static int memcmp_to_u (void *u_dst, void *src, uLong len, OZ_Procmode procmode, int *x);
static int strncpy_from_u (void *dst, void *u_src, uLong len, OZ_Procmode procmode);

/************************************************************************/
/*									*/
/*  Boot-time initialization routine					*/
/*									*/
/************************************************************************/

void linux_dev_disk_init ()

{
  char buff[256], c, linuxname[OZ_DEVUNIT_NAMESIZE], *p, *q, unitdesc[OZ_DEVUNIT_DESCSIZE], unitname[OZ_DEVUNIT_NAMESIZE];
  Devex *devex;
  int fd, i, l2bs, rc, readonly;
  OZ_Dbn lbnoffs, sec, trk;
  struct stat stat;
  uLong blocksize;

  if (!initialized) {
    oz_knl_printk ("linux_disk_init\n");
    initialized = 1;

    devclass  = oz_knl_devclass_create (OZ_IO_DISK_CLASSNAME, OZ_IO_DISK_BASE, OZ_IO_DISK_MASK, "linux_disk");
    devdriver = oz_knl_devdriver_create (devclass, "linux_disk");

    oz_knl_printk ("\nlinux_disk_init: enter linuxname[:secpertrk[:trkpercyl[:lbnoffset[:blocksize]]]] (EXIT when done)\n");
    while (oz_hw_getcon (sizeof buff, buff, 21, "linux_dev_disk_init> ")) {
      if (buff[0] == 0) continue;
      if (strcasecmp (buff, "exit") == 0) break;

      /* Parse out linuxname[:sec[:trk[:lbnoffs[:blocksize]]]] */

      sec = 0;
      trk = 0;
      lbnoffs = 0;
      l2bs = 9;
      blocksize = 512;

      p = strchr (buff, ':');
      if (p == NULL) {
        strncpy (linuxname, buff, sizeof linuxname);
      } else {
        *(p ++) = 0;
        q = p;
        strncpy (linuxname, buff, sizeof linuxname);
        sec = strtoul (p, &p, 0);
        if (*p == ':') trk = strtoul (p + 1, &p, 0);
        if (*p == ':') lbnoffs = strtoul (p + 1,  &p, 0);
        if (*p == ':') blocksize = strtoul (p + 1, &p, 0);
        if (*p != 0) {
          oz_knl_printk ("linux_dev_disk_init: bad sec:trk:lbnoffs:blocksize '%s'\n", q);
          continue;
        }
        for (l2bs = 0; l2bs < OZ_HW_L2PAGESIZE; l2bs ++) {
          if ((blocksize >> l2bs) & 1) break;
        }
        if ((blocksize >> l2bs) != 1) {
          oz_knl_printk ("linux_dev_disk_init: blocksize %u must be power of 2 between 1 and %u\n", 
		blocksize, (1 << OZ_HW_L2PAGESIZE));
          continue;
        }
      }

      /* Open the device we are going to be using */

      readonly = 0;
      fd = open (linuxname, O_RDWR);
      if ((fd < 0) && (errno == EROFS)) {
        readonly = 1;
        fd = open (linuxname, O_RDONLY);
      }
      if (fd < 0) {
        oz_knl_printk ("linux_dev_disk_init: error %d opening device %s: %s\n", errno, linuxname, strerror (errno));
        continue;
      }

      /* Stat it to get its size */

      if (fstat (fd, &stat) < 0) {
        oz_knl_printk ("linux_dev_disk_init: error %u statting device %s: %s\n", errno, linuxname, strerror (errno));
        close (fd);
        fd = -1;
        continue;
      }

      /* It must either be a regular file or block device */

      if (!S_ISREG (stat.st_mode) && !S_ISBLK (stat.st_mode)) {
        oz_knl_printk ("linux_dev_disk_init: %s is neither regular file nor block device\n", linuxname);
        close (fd);
        fd = -1;
        continue;
      }

      /* Find size of block device (regular files already have stat.st_size already filled in) */

      if (S_ISBLK (stat.st_mode)) {
        i = sizeof stat.st_size * 8 - 1;
        stat.st_size = 0;					/* clear it out to start with */
        while (-- i >= l2bs) {
          stat.st_size += (1 << i);				/* set the bit we test for */
          if (lseek (fd, stat.st_size, SEEK_SET) >= 0) {	/* seek to the prospective spot */
            rc = read (fd, buff, 1);				/* try to read a byte from it */
            if (rc > 0) continue;				/* if successful, leave the bit set */
          }
          stat.st_size -= (1 << i);				/* failed to seek or read, remove the bit */
        }
        if (stat.st_size == 0) {
          oz_knl_printk ("linux_dev_disk_init: error determining size of %s: %s\n", linuxname, strerror (errno));
          close (fd);
          fd = -1;
          continue;
        }
        stat.st_size ++;					/* include the one byte we did read */
        if (stat.st_size < 0) stat.st_size --;			/* if overflow, exclude it */
      }

      /* Either way, round size down to a blocksize */

      stat.st_size = (stat.st_size >> l2bs) << l2bs;

      /* Make up unit name and unit description strings */

      strcpy (unitdesc, "Linux disk ");
      p = unitdesc + strlen (unitdesc);
      strcpy (p, linuxname);
      q = unitname;
      i = 0;
      while ((c = *(p ++)) != 0) {
        if (((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'))) { *(q ++) = c; i = 1; }
        else if (i) *(q ++) = '_';
      }
      *q = 0;

      /* Create the device unit struct */

      devunit = oz_knl_devunit_create (devdriver, unitname, unitdesc, &linux_disk_functable, 0, NULL);
      if (devunit == NULL) close (fd);

      /* Fill in the device unit extension info */

      else {
        devex = oz_knl_devunit_ex (devunit);
        devex -> fd = fd;
        devex -> l2bs = l2bs;
        devex -> readonly = readonly;
        devex -> stat = stat;
        strncpy (devex -> diskname, linuxname, sizeof devex -> diskname);
        devex -> sec = sec;
        devex -> trk = trk;
        devex -> lbnoffs = lbnoffs;
        oz_knl_printk ("linux_disk_init: linux name: %s\n", linuxname);
        oz_knl_printk ("                 ozone name: %s\n", unitname);
        oz_knl_printk ("                sec per trk: %u\n", sec);
        oz_knl_printk ("                trk per cyl: %u\n", trk);
        oz_knl_printk ("                 lbn offset: %u\n", lbnoffs);
        oz_knl_printk ("                 block size: %u (%u)\n", DISK_BLOCK_SIZE, L2_DISK_BLOCK_SIZE);
        oz_knl_printk ("                     blocks: %u\n", stat.st_size >> l2bs);
      }
    }
    oz_knl_printk ("\n");
  }
}

/************************************************************************/
/*									*/
/*  Start performing a disk i/o function				*/
/*									*/
/************************************************************************/

static uLong linux_disk_start (OZ_Devunit *devunit, void *devexv, OZ_Iochan *iochan, void *chnexv, OZ_Procmode procmode, 
                               OZ_Ioop *ioop, void *iopexv, uLong funcode, uLong as, void *ap)

{
  uLong size;
  Devex *devex;
  OZ_IO_disk_getinfo1    disk_getinfo1;
  OZ_IO_disk_readblocks  disk_readblocks;
  OZ_IO_disk_readpages   disk_readpages;
  OZ_IO_disk_writeblocks disk_writeblocks;
  OZ_IO_disk_writepages  disk_writepages;

  devex = devexv;

  /* Process individual functions */

  switch (funcode) {

    /* Set volume valid bit one way or the other (noop for us) */

    case OZ_IO_DISK_SETVOLVALID: {
      if (devex -> fd <= 0) return (OZ_NOTMOUNTED);
      return (OZ_SUCCESS);
    }

    /* Write blocks to the disk */

    case OZ_IO_DISK_WRITEBLOCKS: {
      if (devex -> readonly) return (OZ_WRITELOCKED);
      if (!parcpy_from_u (&disk_writeblocks, ap, sizeof disk_writeblocks, as, procmode)) return (OZ_ACCVIO);
      if (devex -> fd <= 0) return (OZ_NOTMOUNTED);
      if (lseek (devex -> fd, disk_writeblocks.slbn << L2_DISK_BLOCK_SIZE, SEEK_SET) < 0) {
        oz_knl_printk ("error %u seeking to lbn %u of disk %s\n", errno, disk_writeblocks.slbn, devex -> diskname);
        return (OZ_IOFAILED);
      }
      if (write (devex -> fd, disk_writeblocks.buff, disk_writeblocks.size) < 0) {
        oz_knl_printk ("error %u writing disk %s\n", errno, devex -> diskname);
        return (OZ_IOFAILED);
      }
      return (OZ_SUCCESS);
    }

    /* Read blocks from the disk */

    case OZ_IO_DISK_READBLOCKS: {
      if (!parcpy_from_u (&disk_readblocks, ap, sizeof disk_readblocks, as, procmode)) return (OZ_ACCVIO);
      if (devex -> fd <= 0) return (OZ_NOTMOUNTED);
      if (lseek (devex -> fd, disk_readblocks.slbn << L2_DISK_BLOCK_SIZE, SEEK_SET) < 0) {
        oz_knl_printk ("error %u seeking to lbn %u\n", errno, disk_readblocks.slbn, devex -> diskname);
        return (OZ_IOFAILED);
      }
      if (read (devex -> fd, disk_readblocks.buff, disk_readblocks.size) < 0) {
        oz_knl_printk ("error %u reading disk %s\n", errno, devex -> diskname);
        return (OZ_IOFAILED);
      }
      return (OZ_SUCCESS);
    }

    /* Get info part 1 */

    case OZ_IO_DISK_GETINFO1: {
      if (!OZ_HW_WRITABLE (as, ap, procmode)) return (OZ_ACCVIO);
      if (devex -> fd <= 0) return (OZ_NOTMOUNTED);
      memset (&disk_getinfo1, 0, sizeof disk_getinfo1);
      disk_getinfo1.blocksize          = DISK_BLOCK_SIZE;
      disk_getinfo1.totalblocks        = devex -> stat.st_size >> L2_DISK_BLOCK_SIZE;
      disk_getinfo1.parthoststartblock = devex -> lbnoffs;
      disk_getinfo1.secpertrk          = devex -> sec;
      disk_getinfo1.trkpercyl          = devex -> trk;
      if ((devex -> trk != 0) && (devex -> sec != 0)) {
        disk_getinfo1.cylinders        = disk_getinfo1.totalblocks / devex -> trk / devex -> sec;
      }
      if (as <= sizeof disk_getinfo1) {
        memcpy (ap, &disk_getinfo1, as);
      } else {
        memcpy (ap, &disk_getinfo1, sizeof disk_getinfo1);
        memset (((uByte *)ap) + sizeof disk_getinfo1, 0, as - sizeof disk_getinfo1);
      }
      return (OZ_SUCCESS);
    }

    /* Write from physical page(s) (Kernel mode only) */

    case OZ_IO_DISK_WRITEPAGES: {
      if (devex -> readonly) return (OZ_WRITELOCKED);
      if (!parcpy_from_u (&disk_writepages, ap, sizeof disk_writepages, as, procmode)) return (OZ_ACCVIO);
      if (devex -> fd <= 0) return (OZ_NOTMOUNTED);
      if (lseek (devex -> fd, disk_writepages.slbn << L2_DISK_BLOCK_SIZE, SEEK_SET) < 0) {
        oz_knl_printk ("error %u seeking to lbn %u\n", errno, disk_writepages.slbn, devex -> diskname);
        return (OZ_IOFAILED);
      }
      disk_writepages.pages  += disk_writepages.offset >> OZ_HW_L2PAGESIZE;
      disk_writepages.offset &= (1 << OZ_HW_L2PAGESIZE) - 1;
      while (disk_writepages.size > 0) {
        size = (1 << OZ_HW_L2PAGESIZE) - disk_writepages.offset;
        if (size > disk_writepages.size) size = disk_writepages.size;
        if (write (devex -> fd, (void *)((disk_writepages.pages[0] << OZ_HW_L2PAGESIZE) + disk_writepages.offset), size) < 0) {
          oz_knl_printk ("linux_dev_disk: errno %u writing disk %s\n", errno, devex -> diskname);
          return (OZ_IOFAILED);
        }
        disk_writepages.size  -= size;
        disk_writepages.offset = 0;
        disk_writepages.pages ++;
      }
      return (OZ_SUCCESS);
    }

    /* Read into physical page(s) (Kernel mode only) */

    case OZ_IO_DISK_READPAGES: {
      if (!parcpy_from_u (&disk_readpages, ap, sizeof disk_readpages, as, procmode)) return (OZ_ACCVIO);
      if (devex -> fd <= 0) return (OZ_NOTMOUNTED);
      if (lseek (devex -> fd, disk_readpages.slbn << L2_DISK_BLOCK_SIZE, SEEK_SET) < 0) {
        oz_knl_printk ("error %u seeking to lbn %u\n", errno, disk_readpages.slbn, devex -> diskname);
        return (OZ_IOFAILED);
      }
      disk_readpages.pages  += disk_readpages.offset >> OZ_HW_L2PAGESIZE;
      disk_readpages.offset &= (1 << OZ_HW_L2PAGESIZE) - 1;
      while (disk_readpages.size > 0) {
        size = (1 << OZ_HW_L2PAGESIZE) - disk_readpages.offset;
        if (size > disk_readpages.size) size = disk_readpages.size;
        if (read (devex -> fd, (void *)((disk_readpages.pages[0] << OZ_HW_L2PAGESIZE) + disk_readpages.offset), size) < 0) {
          oz_knl_printk ("linux_dev_disk: errno %u reading disk %s\n", errno, devex -> diskname);
          return (OZ_IOFAILED);
        }
        disk_readpages.size  -= size;
        disk_readpages.offset = 0;
        disk_readpages.pages ++;
      }
      return (OZ_SUCCESS);
    }

    /* Who knows what */

    default: {
      return (OZ_BADIOFUNC);
    }
  }
}

/************************************************************************/
/*									*/
/*  Copy a param block from user mode (testing it for readabiliti)	*/
/*									*/
/*    Input:								*/
/*									*/
/*	dst = internal buffer address to copy to			*/
/*	u_src = user mode buffer to copy from				*/
/*	len = length of destination buffer				*/
/*	u_len = user mode buffer length					*/
/*	procmode = processor mode					*/
/*									*/
/*    Output:								*/
/*									*/
/*	parcpy_from_u = 0 : u_src buffer not readable by procmode	*/
/*	                1 ; copy complete				*/
/*	copied user src to dst						*/
/*									*/
/************************************************************************/

static int parcpy_from_u (void *dst, void *u_src, uLong len, uLong u_len, OZ_Procmode procmode)

{
  int ok;

  ok = OZ_HW_READABLE (u_len, u_src, procmode);		/* make sure source is readable before doing anything else */
  if (ok) {
    if (u_len >= len) memcpy (dst, u_src, len);		/* if source same length or longer than dst, copy dest's size worth */
    else {
      memcpy (dst, u_src, u_len);			/* source shorter, copy the whole source */
      memset (((uByte *)dst) + u_len, 0, len - u_len);	/* zero pad the destination */
    }
  }
  return (ok);
}
/************************************************************************/
/*									*/
/*  Copy a buffer from user mode (testing it for readabiliti)		*/
/*									*/
/*    Input:								*/
/*									*/
/*	dst = internal buffer address to copy to			*/
/*	u_src = user mode buffer to copy from				*/
/*	len = length of buffer to copy					*/
/*	procmode = processor mode					*/
/*									*/
/*    Output:								*/
/*									*/
/*	memcpy_from_u = 0 : u_src buffer not readable by procmode	*/
/*	                1 ; copy complete				*/
/*	copied user src to dst						*/
/*									*/
/************************************************************************/

static int memcpy_from_u (void *dst, void *u_src, uLong len, OZ_Procmode procmode)

{
  int ok;

  ok = OZ_HW_READABLE (len, u_src, procmode);
  if (ok) memcpy (dst, u_src, len);
  return (ok);
}

/************************************************************************/
/*									*/
/*  Copy a buffer to user mode (testing it for writabiliti)		*/
/*									*/
/*    Input:								*/
/*									*/
/*	u_dst = user mode buffer address to copy to			*/
/*	src = internal buffer to copy from				*/
/*	len = length of buffer to copy					*/
/*	procmode = processor mode					*/
/*									*/
/*    Output:								*/
/*									*/
/*	memcpy_to_u = 0 : u_dst buffer not writable by procmode		*/
/*	              1 : copy complete					*/
/*	copied src to user dst						*/
/*									*/
/************************************************************************/

static int memcpy_to_u (void *u_dst, void *src, uLong len, OZ_Procmode procmode)

{
  int ok;

  ok = OZ_HW_WRITABLE (len, u_dst, procmode);
  if (ok) memcpy (u_dst, src, len);
  return (ok);
}

/************************************************************************/
/*									*/
/*  Compare a buffer to user mode buffer (testing it for readabiliti)	*/
/*									*/
/*    Input:								*/
/*									*/
/*	u_dst = user mode buffer address to compare			*/
/*	src = internal buffer to compare				*/
/*	len = length of buffer to compare				*/
/*	procmode = processor mode					*/
/*									*/
/*    Output:								*/
/*									*/
/*	memcmp_to_u = 0 : u_dst buffer not readable by procmode		*/
/*	              1 : compare complete				*/
/*	*x = result of compare						*/
/*									*/
/************************************************************************/

static int memcmp_to_u (void *u_dst, void *src, uLong len, OZ_Procmode procmode, int *x)

{
  int ok;

  ok = OZ_HW_READABLE (len, u_dst, procmode);
  if (ok) *x = memcmp (u_dst, src, len);
  return (ok);
}

/************************************************************************/
/*									*/
/*  Copy null terminated string from user buffer to internal buffer	*/
/*									*/
/*    Input:								*/
/*									*/
/*	dst = internal buffer address to copy to			*/
/*	u_src = user mode buffer to copy from				*/
/*	len = length of buffer to copy					*/
/*	procmode = processor mode					*/
/*									*/
/*    Output:								*/
/*									*/
/*	strncpy_from_u = 0 : u_src buffer not readable by procmode	*/
/*	                 1 ; copy complete				*/
/*	copied user src to dst						*/
/*									*/
/************************************************************************/

static int strncpy_from_u (void *dst, void *u_src, uLong len, OZ_Procmode procmode)

{
  char *dp, *usp;
  int l;

  dp = dst;
  usp = u_src;
  for (l = 0; l < len; l ++) {
    if (!OZ_HW_READABLE (1, usp, procmode)) return (0);
    if ((*(dp ++) = *(usp ++)) == 0) return (1);
  }
  *(-- dp) = 0;
  return (1);
}
