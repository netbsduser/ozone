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
/*  This makes a linux directory accessible as a filesystem		*/
/*									*/
/************************************************************************/

#include "ozone.h"
#include "oz_io_fs.h"
#include "oz_knl_devio.h"
#include "oz_knl_hw.h"
#include "oz_knl_kmalloc.h"
#include "oz_knl_procmode.h"
#include "oz_knl_sdata.h"
#include "oz_knl_section.h"
#include "oz_knl_spte.h"
#include "oz_knl_status.h"
#include "oz_sys_recio.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define DISK_BLOCK_SIZE 512
#define FILESPEC_SIZE 256

#define FLD_UW(base,ofst) *((uWord *)(((uByte *)(base)) + (ofst)))

/* Channel extension structure */

typedef struct { int fd;			/* -1 : no file open; else : fd of file */
                 OZ_Recio_filex *recio_filex;	/* record I/O per-file extension */
                 OZ_Recio_chnex *recio_chnex;	/* record I/O per-channel extension */
                 DIR *dir;			/* file opened is a directory */
                 char name[FILESPEC_SIZE];	/* filespec string */
               } Chnex;

/* Function table */

static uLong fs_linux_assign (OZ_Devunit *devunit, void *devexv, OZ_Iochan *iochan, void *chnexv, OZ_Procmode procmode);
static int fs_linux_deassign (OZ_Devunit *devunit, void *devexv, OZ_Iochan *iochan, void *chnexv);
static uLong fs_linux_start (OZ_Devunit *devunit, void *devexv, OZ_Iochan *iochan, void *chnexv, OZ_Procmode procmode, 
                             OZ_Ioop *ioop, void *iopexv, uLong funcode, uLong as, void *ap);

static const OZ_Devfunc fs_linux_functable = { 0, sizeof (Chnex), 0, 0, 
                                               NULL, NULL, NULL, fs_linux_assign, fs_linux_deassign, 
                                               NULL, fs_linux_start, NULL };

/* Internal static data */

static int initialized = 0;
static OZ_Devclass *devclass;
static OZ_Devdriver *devdriver;
static OZ_Devunit *devunit;

/* Internal routines */

static int parcpy_from_u (void *dst, const void *u_src, uLong len, uLong u_len, OZ_Procmode procmode);
static int memcpy_from_u (void *dst, const void *u_src, uLong len, OZ_Procmode procmode);
static int memcpy_to_u (void *u_dst, const void *src, uLong len, OZ_Procmode procmode);
static int memcmp_to_u (void *u_dst, const void *src, uLong len, OZ_Procmode procmode, int *x);
static int strncpy_from_u (void *dst, const void *u_src, uLong len, OZ_Procmode procmode);

/* Record I/O callback table */

static uLong riocb_extend (void *chnex, void *filex, OZ_Dbn exblk);
static void riocb_seteof (void *chnex, void *filex, OZ_Dbn efblk, uLong efbyt);
static uLong riocb_write (void *chnex, void *filex, OZ_Dbn vbn, uLong size, uByte *buff);
static uLong riocb_read (void *chnex, void *filex, OZ_Dbn vbn, uLong size, uByte *buff);
static void *riocb_malloc (void *chnex, void *filex, uLong size, const char *file, int line);
static void riocb_free (void *chnex, void *filex, void *buff);

static OZ_Recio_call riocb_table = { riocb_extend, riocb_seteof, riocb_write, riocb_read, riocb_malloc, riocb_free };

/************************************************************************/
/*									*/
/*  Boot-time initialization routine					*/
/*									*/
/************************************************************************/

void oz_dev_fs_linux_init ()

{
  if (!initialized) {
    oz_knl_printk ("oz_fs_linux_init\n");
    initialized = 1;
    devclass  = oz_knl_devclass_create (OZ_IO_FS_CLASSNAME, OZ_IO_FS_BASE, OZ_IO_FS_MASK, "fs_linux");
    devdriver = oz_knl_devdriver_create (devclass, "fs_linux");
    devunit   = oz_knl_devunit_create (devdriver, "fs_linux", "Linux filesystem", &fs_linux_functable, 0, NULL); /* create devunit struct */
  }
}

/************************************************************************/
/*									*/
/*  Channel was just assigned						*/
/*									*/
/*  Initialize the fd to indicate no file open				*/
/*									*/
/************************************************************************/

static uLong fs_linux_assign (OZ_Devunit *devunit, void *devexv, OZ_Iochan *iochan, void *chnexv, OZ_Procmode procmode)

{
  ((Chnex *)chnexv) -> fd  = -1;
  ((Chnex *)chnexv) -> dir = NULL;
  ((Chnex *)chnexv) -> recio_filex = NULL;
  ((Chnex *)chnexv) -> recio_chnex = NULL;
  ((Chnex *)chnexv) -> name[0] = 0;
  return (OZ_SUCCESS);
}

/************************************************************************/
/*									*/
/*  An OZONE i/o channel is being deassigned				*/
/*									*/
/*  Here we close any open linux file					*/
/*									*/
/************************************************************************/

static int fs_linux_deassign (OZ_Devunit *devunit, void *devexv, OZ_Iochan *iochan, void *chnexv)

{
  Chnex *chnex;

  chnex = chnexv;

  if (chnex -> fd >= 0) {
    oz_sys_recio_termchnex (chnex -> recio_chnex, chnex -> recio_filex);
    chnex -> recio_chnex = NULL;
    oz_sys_recio_termfilex (chnex -> recio_filex, NULL, NULL);
    chnex -> recio_filex = NULL;
    close (chnex -> fd);
    chnex -> fd = -1;
  }
  if (chnex -> dir != NULL) {
    closedir (chnex -> dir);
    chnex -> dir = NULL;
  }

  return (0);
}

/************************************************************************/
/*									*/
/*  Start performing a disk i/o function				*/
/*									*/
/************************************************************************/

static uLong fs_linux_start (OZ_Devunit *devunit, void *devexv, OZ_Iochan *iochan, void *chnexv, OZ_Procmode procmode, 
                             OZ_Ioop *ioop, void *iopexv, uLong funcode, uLong as, void *ap)

{
  char *p;
  uByte *rbuf;
  Chnex *chnex;
  int how;
  uLong rlen, rofs, sts, wlen;
  OZ_Mempage npages, svpage;
  struct dirent *de;
  struct stat statbuf;

  union { struct { OZ_IO_fs_open p; } open;
          struct { OZ_IO_fs_create p; } create;
          struct { OZ_IO_fs_writeblocks p; } writeblocks;
          struct { OZ_IO_fs_readblocks p; } readblocks;
          struct { OZ_IO_fs_writerec p; } writerec;
          struct { OZ_IO_fs_readrec p; } readrec;
          struct { OZ_IO_fs_getinfo1 p; } getinfo1;
          struct { OZ_IO_fs_readdir p; } readdir;
        } u;

  chnex = chnexv;

  /* Process individual functions */

  switch (funcode) {

    /* Open a file */

    case OZ_IO_FS_OPEN: {
      if ((chnex -> fd >= 0) || (chnex -> dir != NULL)) return (OZ_FILEALREADYOPEN);
      if (!parcpy_from_u (&(u.open.p), ap, sizeof u.open.p, as, procmode)) return (OZ_ACCVIO);
      if (!strncpy_from_u (chnex -> name, u.open.p.name, sizeof chnex -> name, procmode)) return (OZ_ACCVIO);
      p = chnex -> name + strlen (chnex -> name);
      if (p[-1] == '/') goto open_dir;

      /* If it doesn't end in /, assume it is a regular file */

      how = O_RDONLY;
      if (OZ_LOCK_ALLOW_TEST (u.open.p.lockmode, OZ_LOCK_ALLOWS_SELF_WRITE)) how = O_RDWR;
      chnex -> fd = open (chnex -> name, how);
      if (chnex -> fd < 0) {
        if (errno == ENOENT) return (OZ_NOSUCHFILE);
        oz_knl_printk ("oz_fs_linux_start: errno %d opening file %s\n", errno, chnex -> name);
        return (OZ_IOFAILED);
      }
      if (fstat (chnex -> fd, &statbuf) < 0) {
        oz_knl_printk ("oz_fs_linux_start: errno %d statting %s\n", errno, chnex -> name);
        close (chnex -> fd);
        chnex -> fd = -1;
        return (OZ_IOFAILED);
      }
      u.getinfo1.p.eofblock = (statbuf.st_size / DISK_BLOCK_SIZE) + 1;
      u.getinfo1.p.eofbyte  = statbuf.st_size % DISK_BLOCK_SIZE;
      chnex -> recio_filex = oz_sys_recio_initfilex (NULL, &riocb_table, DISK_BLOCK_SIZE, u.getinfo1.p.eofblock, u.getinfo1.p.eofbyte);
      chnex -> recio_chnex = oz_sys_recio_initchnex (chnex -> recio_filex, chnex);
      return (OZ_SUCCESS);

      /* It ends in /, assume it is a directory */

open_dir:
      chnex -> dir = opendir (chnex -> name);
      if (chnex -> dir == NULL) {
        if (errno == ENOENT) return (OZ_NOSUCHFILE);
        oz_knl_printk ("oz_fs_linux_start: errno %d opening directory %s\n", errno, chnex -> name);
        return (OZ_IOFAILED);
      }
      return (OZ_SUCCESS);
    }

    /* Create a file */

    case OZ_IO_FS_CREATE: {
      if ((chnex -> fd >= 0) || (chnex -> dir != NULL)) return (OZ_FILEALREADYOPEN);
      if (!parcpy_from_u (&(u.create.p), ap, sizeof u.create.p, as, procmode)) return (OZ_ACCVIO);
      if (!strncpy_from_u (chnex -> name, u.create.p.name, sizeof chnex -> name, procmode)) return (OZ_ACCVIO);
      chnex -> fd = open (chnex -> name, O_CREAT | O_RDWR, 0640);
      if (chnex -> fd < 0) {
        oz_knl_printk ("oz_fs_linux_start: errno %d creating file %s\n", errno, chnex -> name);
        return (OZ_IOFAILED);
      }
      if (fstat (chnex -> fd, &statbuf) < 0) {
        oz_knl_printk ("oz_fs_linux_start: errno %d statting %s\n", errno, chnex -> name);
        close (chnex -> fd);
        chnex -> fd = -1;
        return (OZ_IOFAILED);
      }
      u.getinfo1.p.eofblock = (statbuf.st_size / DISK_BLOCK_SIZE) + 1;
      u.getinfo1.p.eofbyte  = statbuf.st_size % DISK_BLOCK_SIZE;
      chnex -> recio_filex = oz_sys_recio_initfilex (NULL, &riocb_table, DISK_BLOCK_SIZE, u.getinfo1.p.eofblock, u.getinfo1.p.eofbyte);
      chnex -> recio_chnex = oz_sys_recio_initchnex (chnex -> recio_filex, chnex);
      return (OZ_SUCCESS);
    }

    /* Close file */

    case OZ_IO_FS_CLOSE: {
      if (chnex -> fd >= 0) {
        oz_sys_recio_termchnex (chnex -> recio_chnex, chnex -> recio_filex);
        chnex -> recio_chnex = NULL;
        oz_sys_recio_termfilex (chnex -> recio_filex, NULL, NULL);
        chnex -> recio_filex = NULL;
        close (chnex -> fd);
        chnex -> fd = -1;
        return (OZ_SUCCESS);
      }
      if (chnex -> dir != NULL) {
        closedir (chnex -> dir);
        chnex -> dir = NULL;
        return (OZ_SUCCESS);
      }
      return (OZ_FILENOTOPEN);
    }

    /* Write blocks to the file */

    case OZ_IO_FS_WRITEBLOCKS: {
      if (!parcpy_from_u (&(u.writeblocks.p), ap, sizeof u.writeblocks.p, as, procmode)) return (OZ_ACCVIO);
      if (chnex -> fd < 0) return (OZ_FILENOTOPEN);
      if (lseek (chnex -> fd, (u.writeblocks.p.svbn - 1) * DISK_BLOCK_SIZE, SEEK_SET) < 0) {
        oz_knl_printk ("oz_fs_linux_start: errno %d positioning %s to vbn %u\n", errno, chnex -> name, u.writeblocks.p.svbn);
        return (OZ_IOFAILED);
      }
      wlen = write (chnex -> fd, u.writeblocks.p.buff, u.writeblocks.p.size);
      if (wlen < 0) {
        oz_knl_printk ("oz_fs_linux_start: errno %d writing to %s\n", errno, chnex -> name);
        return (OZ_IOFAILED);
      }
      if (wlen != u.writeblocks.p.size) {
        oz_knl_printk ("oz_fs_linux_start: only wrote %d instead of %u\n", wlen, u.writeblocks.p.size);
        return (OZ_IOFAILED);
      }
      return (OZ_SUCCESS);
    }

    /* Read blocks from the file */

    case OZ_IO_FS_READBLOCKS: {
      if (!parcpy_from_u (&(u.readblocks.p), ap, sizeof u.readblocks.p, as, procmode)) return (OZ_ACCVIO);
      if (chnex -> fd < 0) return (OZ_FILENOTOPEN);
      if (lseek (chnex -> fd, (u.readblocks.p.svbn - 1) * DISK_BLOCK_SIZE, SEEK_SET) < 0) {
        oz_knl_printk ("oz_fs_linux_start: errno %d positioning %s to vbn %u\n", errno, chnex -> name, u.readblocks.p.svbn);
        return (OZ_IOFAILED);
      }
      rlen = read (chnex -> fd, u.readblocks.p.buff, u.readblocks.p.size);
      if (rlen < 0) {
        oz_knl_printk ("oz_fs_linux_start: errno %d reading from %s\n", errno, chnex -> name);
        return (OZ_IOFAILED);
      }
      if (rlen != u.readblocks.p.size) {
        oz_knl_printk ("oz_fs_linux_start: only read %d instead of %u\n", rlen, u.readblocks.p.size);
        return (OZ_IOFAILED);
      }
      return (OZ_SUCCESS);
    }

    /* Write record to the file */

    case OZ_IO_FS_WRITEREC: {
      if (!parcpy_from_u (&(u.writerec.p), ap, sizeof u.writerec.p, as, procmode)) return (OZ_ACCVIO);
      if (chnex -> fd < 0) return (OZ_FILENOTOPEN);
      sts = oz_sys_recio_write (chnex -> recio_chnex, chnex -> recio_filex, &u.writerec.p);
      return (sts);
    }

    /* Read record from the file */

    case OZ_IO_FS_READREC: {
      if (!parcpy_from_u (&(u.readrec.p), ap, sizeof u.readrec.p, as, procmode)) return (OZ_ACCVIO);
      if (chnex -> fd < 0) return (OZ_FILENOTOPEN);
      sts = oz_sys_recio_read (chnex -> recio_chnex, chnex -> recio_filex, &u.readrec.p);
      return (sts);
    }

    /* Get info part 1 */

    case OZ_IO_FS_GETINFO1: {
      if (!OZ_HW_WRITABLE (as, ap, procmode)) return (OZ_ACCVIO);
      memset (&u.getinfo1.p, 0, sizeof u.getinfo1.p);
      if (chnex -> fd >= 0) {
        if (fstat (chnex -> fd, &statbuf) < 0) {
          oz_knl_printk ("oz_fs_linux_start: errno %d statting %s\n", errno, chnex -> name);
          return (OZ_IOFAILED);
        }
        u.getinfo1.p.blocksize = DISK_BLOCK_SIZE;
        u.getinfo1.p.eofblock  = (statbuf.st_size / DISK_BLOCK_SIZE) + 1;
        u.getinfo1.p.eofbyte   = statbuf.st_size % DISK_BLOCK_SIZE;
        u.getinfo1.p.hiblock   = (statbuf.st_size + DISK_BLOCK_SIZE - 1) / DISK_BLOCK_SIZE;
        u.getinfo1.p.curbyte   = lseek (chnex -> fd, 0, SEEK_CUR);
        u.getinfo1.p.curblock  = (u.getinfo1.p.curbyte / DISK_BLOCK_SIZE) + 1;
        u.getinfo1.p.curbyte  %= DISK_BLOCK_SIZE;
      } else if (chnex -> dir != NULL) {
        u.getinfo1.p.filattrflags = OZ_FS_FILATTRFLAG_DIRECTORY;
      } else return (OZ_FILENOTOPEN);
      if (as <= sizeof u.getinfo1.p) {
        memcpy (ap, &(u.getinfo1.p), as);
      } else {
        memcpy (ap, &(u.getinfo1.p), sizeof u.getinfo1.p);
        memset (((uByte *)ap) + sizeof u.getinfo1.p, 0, as - sizeof u.getinfo1.p);
      }
      return (OZ_SUCCESS);
    }

    /* Read directory entry */

    case OZ_IO_FS_READDIR: {
      if (!parcpy_from_u (&(u.readdir.p), ap, sizeof u.readdir.p, as, procmode)) return (OZ_ACCVIO);
      if (chnex -> dir == NULL) return (OZ_FILENOTOPEN);				/* a directory must be open on the channel */
      do {
        de = readdir (chnex -> dir);							/* read next directory entry */
        if (de == NULL) return (OZ_ENDOFFILE);						/* if end of dir, return end-of-file status */
        rlen = strlen (de -> d_name);
      } while ((rlen == 0) 								// don't return null entries
            || (de -> d_name[rlen-1] == '~') 						// don't return anything ending in ~ (edt version files)
            || (strcmp (de -> d_name, ".") == 0) 					// don't return '.' entry (the directory itself)
            || (strcmp (de -> d_name, "..") == 0));					// don't return '..' entry (the parent directory)
      strncpy (u.readdir.p.filenambuff, de -> d_name, u.readdir.p.filenamsize);		/* ok, return the found name */
      rlen = strlen (u.readdir.p.filenambuff);						/* make the complete filespec */
      p = OZ_KNL_NPPMALLOC (strlen (chnex -> name) + rlen + 1);
      strcpy (p, chnex -> name);
      strcat (p, de -> d_name);
      if ((stat (p, &statbuf) >= 0) && S_ISDIR (statbuf.st_mode)) {			/* see if it is a directory */
        if (rlen < u.readdir.p.filenamsize - 1) u.readdir.p.filenambuff[rlen] = '/';	/* if so, tack a '/' on the end of name */
      }
      OZ_KNL_NPPFREE (p);								/* free temp buffer */
      return (OZ_SUCCESS);								/* successful */
    }

    /* Who knows what */

    default: {
      return (OZ_BADIOFUNC);
    }
  }
}

/************************************************************************/
/*									*/
/*  Record I/O callback routines					*/
/*									*/
/************************************************************************/

/* Extend file to include the exblk block */

static uLong riocb_extend (void *chnexv, void *filexv, OZ_Dbn exblk)

{
  return (OZ_SUCCESS);
}

/* Set file's end-of-file point */

static void riocb_seteof (void *chnexv, void *filexv, OZ_Dbn efblk, uLong efbyt)

{
  Chnex *chnex;

  chnex = chnexv;

  ftruncate (chnex -> fd, (efblk - 1) * DISK_BLOCK_SIZE + efbyt);
}

/* Write blocks to file */

static uLong riocb_write (void *chnexv, void *filexv, OZ_Dbn vbn, uLong size, uByte *buff)

{
  Chnex *chnex;
  int rc;

  chnex = chnexv;

  rc = lseek (chnex -> fd, (vbn - 1) * DISK_BLOCK_SIZE, SEEK_SET);
  if (rc < 0) {
    oz_knl_printk ("oz_dev_fs_linux riocb_write: errno %d seeking to vbn %u\n", errno, vbn);
    return (OZ_IOFAILED);
  }
  rc = write (chnex -> fd, buff, size);
  if (rc < 0) {
    oz_knl_printk ("oz_dev_fs_linux riocb_write: errno %d writing %u bytes at vbn %u\n", errno, size, vbn);
    return (OZ_IOFAILED);
  }
  return (OZ_SUCCESS);
}

/* Read blocks from file */

static uLong riocb_read (void *chnexv, void *filexv, OZ_Dbn vbn, uLong size, uByte *buff)

{
  Chnex *chnex;
  int rc;

  chnex = chnexv;

  rc = lseek (chnex -> fd, (vbn - 1) * DISK_BLOCK_SIZE, SEEK_SET);
  if (rc < 0) {
    oz_knl_printk ("oz_dev_fs_linux riocb_read: errno %d seeking to vbn %u\n", errno, vbn);
    return (OZ_IOFAILED);
  }
  rc = read (chnex -> fd, buff, size);
  if (rc < 0) {
    oz_knl_printk ("oz_dev_fs_linux riocb_read: errno %d reading %u bytes at vbn %u\n", errno, size, vbn);
    return (OZ_IOFAILED);
  }
  if (rc < size) memset (buff + rc, 0xDB, size - rc);
  return (OZ_SUCCESS);
}

/* Allocate memory block */

static void *riocb_malloc (void *chnexv, void *filexv, uLong size, const char *file, int line)

{
  return (OZ_KNL_PGPMALLOC (size));
}

/* Free memory block */

static void riocb_free (void *chnexv, void *filexv, void *buff)

{
  OZ_KNL_PGPFREE (buff);
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

static int parcpy_from_u (void *dst, const void *u_src, uLong len, uLong u_len, OZ_Procmode procmode)

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

static int memcpy_from_u (void *dst, const void *u_src, uLong len, OZ_Procmode procmode)

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

static int memcpy_to_u (void *u_dst, const void *src, uLong len, OZ_Procmode procmode)

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

static int memcmp_to_u (void *u_dst, const void *src, uLong len, OZ_Procmode procmode, int *x)

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

static int strncpy_from_u (void *dst, const void *u_src, uLong len, OZ_Procmode procmode)

{
  char *dp;
  const char *usp;
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
