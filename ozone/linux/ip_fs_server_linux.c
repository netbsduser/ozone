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

/************************************************************************/
/*									*/
/*  ip fs server for oz_dev_ip_fs (that runs on linux)			*/
/*									*/
/*	./ip_fs_server_linux <port_number> <filename_prefix>		*/
/*									*/
/************************************************************************/

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ozone.h"
#include "oz_dev_ip_fs.h"
#include "oz_io_fs.h"
#include "oz_knl_status.h"
#include "oz_sys_dateconv.h"

#define DISK_BLOCK_SIZE 512
#define REPLY_SEQ_MOD 256
#define MAXFILES 1024
#define MAXWILDS 256

int alphasort ();

/* Fileid consists of the entire filespec string (as Linux doesn't have an 'open-by-id' routine) */

typedef struct { char fidstr[256];	/* filename string including basename */
               } Fileid;

/* Directory read context consists of the output from a scandir call */

typedef struct { int nextent;			/* next entry in namelist to process */
                 int numents;			/* total number of entries in namelist */
                 struct dirent **namelist;	/* list of directory entries */
                 Fileid entryfid;		/* last found entry's file id */
               } Dirread;

/* A File context block consists of the fd and the fileid */

typedef struct { int fd;		/* fd it is open on */
                 Dirread *dirread;	/* OZ_IO_FS_READDIR context pointer */
                 char filest[256];	/* filename string including basename */
               } File;

/* In-memory wildcard searching routine context */

#define WILD_MATCH_MATCHES 0x1
#define WILD_MATCH_DOSUBDR 0x2

typedef struct Wild Wild;

struct Wild { Wild *next;		/* next (next inner directory) */
              Wild *prev;		/* previous (next outer directory) */
              Fileid dirid;		/* file-id of this directory */
              File *dirfile;		/* pointer to file node if directory is open */
              Dirread *dirread;		/* directory read context pointer */
              int fatal;		/* a fatal error occurred opening/reading this directory */

              int firstwc;		/* offset to first wildcard within 'spec' for topmost */
					/* else points to end of 'spec' */
              char spec[1];		/* the complete wildcard spec for topmost */
					/* else the directory's complete filespec string */

					/* spec..firstwc: the complete dirspec string for this level */
					/* topmost only: spec: the complete wildcard spec string */
            };

/* Static data */

static char *basename;			/* "/home/mrieker/ozone" from argv[2] */
static int basenamelen;			/* strlen (basename) */
static File openfiles[MAXFILES];	/* handles 1..MAXFILES-1 */
static Wild *wildcards[MAXWILDS];	/* handles MAXFILES..MAXFILES+MAXWILDS-1 */

/* Internal routines */

static uLong wild_init (const char *spec, Wild **wild_r);
static uLong wild_next (Wild *topwild, char **spec_r, Fileid *fileid_r);
static void wild_term (Wild *topwild);
static uLong wild_match (char *filespec, char *wildspec);
static uLong wild_match2 (char *fp, char *fe, char *wp, char *we);
static uLong getdirid (const char *filespec, Fileid *dirid_r, const char **fname_r);
static uLong open_by_fid (Fileid *fileid, OZ_Secaccmsk secaccmsk, File **file_r);
static uLong lookup_file (File *dirfile, int namelen, const char *name, Fileid *fileid_r);
static void close_file (File *file);
static uLong read_dir_next (File *dirfile, Dirread **dirread_r, char **fname_r, Fileid **fileid_r);
static read_dir_close (Dirread **dirread_r);
static OZ_Datebin unix_time_to_datebin (time_t unix_time);

int main (int argc, char *argv[])

{
  char *fn;
  int cliaddrlen, i, newfd, rc, rpllen, udpfd;
  Fileid *fileid;
  Long pos;
  uLong portno, rlen, sts;
  OZ_Ip_fs_req reqbuf;
  OZ_Ip_fs_rpl *rplbuf, rplbufs[REPLY_SEQ_MOD];
  struct sockaddr_in cliaddr, myaddr;
  struct stat statbuf;
  Wild *wild;

  uByte readrecbuf[sizeof reqbuf.u.readrec.trmbuff + sizeof rplbuf -> u.readrec.buff];

  if (argc != 3) {
    fprintf (stderr, "usage: ip_fs_server_linux <portno> <filename_prefix>\n");
    return (-1);
  }

  // fprintf (stderr, "*: sizeof rplbuf %u\n", sizeof *rplbuf);

  portno = atoi (argv[1]);
  basename = argv[2];
  basenamelen = strlen (basename);

  udpfd = socket (AF_INET, SOCK_DGRAM, 0);
  if (udpfd < 0) {
    fprintf (stderr, "error creating udp socket: %s\n", strerror (errno));
    return (-1);
  }

  memset (&myaddr, 0, sizeof myaddr);
  myaddr.sin_family      = AF_INET;
  myaddr.sin_port        = htons (portno);
  myaddr.sin_addr.s_addr = INADDR_ANY;

  if (bind (udpfd, (void *)&myaddr, sizeof myaddr) < 0) {
    fprintf (stderr, "error binding udp socket: %s\n", strerror (errno));
    return (-1);
  }

  /* Set up impossible sequence numbers in rplbufs so they will all be invalid */

  for (sts = 0; sts < REPLY_SEQ_MOD; sts ++) {
    rplbufs[sts].seq = sts + 1;
  }

  /* We haven't opened any files yet */

  memset (openfiles, 0, sizeof openfiles);
  memset (wildcards, 0, sizeof wildcards);

  /* Read in request */

recvloop:
  memset (&cliaddr, 0, sizeof cliaddr);
  cliaddrlen = sizeof cliaddr;
  rc = recvfrom (udpfd, &reqbuf, sizeof reqbuf, 0, (void *)&cliaddr, &cliaddrlen);
  if (rc < 0) {
    fprintf (stderr, "error receiving request: %s\n", strerror (errno));
    return (-1);
  }

  /* Skip duplicate request */

  rpllen = sizeof *rplbuf;									/* set up default reply length */
  rplbuf = rplbufs + (reqbuf.seq % REPLY_SEQ_MOD);						/* point to reply buffer */
  if ((reqbuf.func != OZ_IO_FS_MOUNTVOL) && (reqbuf.seq == rplbuf -> seq)) goto send_reply;	/* it is a duplicate if seq matches */

  /* Process new request */

  rplbuf -> seq = reqbuf.seq;									/* save new sequence number in reply buffer */

  // printf ("ip_fs_server_linux*: reqbuf.func %x\n", reqbuf.func);

  switch (reqbuf.func) {									/* dispatch to process function code */

    /* Mount - wipe saved reply table and close all open files */

    case OZ_IO_FS_MOUNTVOL: {
      printf ("mount received\n");
      for (sts = 0; sts < REPLY_SEQ_MOD; sts ++) {
        rplbufs[sts].seq = sts + 1;
      }
      for (sts = 0; sts < MAXFILES; sts ++) {
        if (openfiles[sts].fd != 0) close (openfiles[sts].fd);
        read_dir_close (&(openfiles[sts].dirread));
      }
      memset (openfiles, 0, sizeof openfiles);
      rplbuf -> seq    = reqbuf.seq;
      rplbuf -> status = OZ_SUCCESS;
      break;
    }

    /* Open an existing file */

    case OZ_IO_FS_OPEN: {
      if (reqbuf.handle != 0) close (reqbuf.handle);
      rplbuf -> u.open.handle = 0;
      fn = malloc (strlen (basename) + strlen (reqbuf.u.open.name) + 1);
      strcpy (fn, basename);
      strcat (fn, reqbuf.u.open.name);
      newfd = open (fn, O_RDONLY);
      if (newfd < 0) {
        fprintf (stderr, "error opening %s: %s\n", fn, strerror (errno));
        if (errno == ENOENT) rplbuf -> status = OZ_NOSUCHFILE;
        else rplbuf -> status = OZ_IOFAILED;
      } else {
        if (newfd > MAXFILES) {
          fprintf (stderr, "newfd %d exceeds maximum\n", newfd);
          return (-1);
        }
        openfiles[newfd].fd = newfd;
        strncpy (openfiles[newfd].filest, fn, sizeof openfiles[newfd].filest);
        rplbuf -> status = OZ_SUCCESS;
        rplbuf -> u.open.handle = newfd;
      }
      free (fn);
      break;
    }

    /* Read blocks from file */

    case OZ_IO_FS_READBLOCKS: {
      rplbuf -> status = OZ_FILENOTOPEN;
      if (reqbuf.handle == 0) break;
      rplbuf -> status = OZ_IOFAILED;
      rplbuf -> u.readblocks.rlen = 0;
      if (lseek (reqbuf.handle, (reqbuf.u.readblocks.svbn - 1) * DISK_BLOCK_SIZE, SEEK_SET) < 0) {
        fprintf (stderr, "error seeking to %u: %s\n", (reqbuf.u.readblocks.svbn - 1) * DISK_BLOCK_SIZE, strerror (errno));
      } else {
        rlen = reqbuf.u.readblocks.size;
        if (rlen > sizeof rplbuf -> u.readblocks.buff) rlen = sizeof rplbuf -> u.readblocks.buff;
        rc = read (reqbuf.handle, rplbuf -> u.readblocks.buff, rlen);
        if (rc < 0) {
          fprintf (stderr, "error reading %u at %u: %s\n", rlen, (reqbuf.u.readblocks.svbn - 1) * DISK_BLOCK_SIZE, strerror (errno));
        } else {
          rplbuf -> status = OZ_ENDOFFILE;
          if (rc > 0) {
            rplbuf -> status = OZ_SUCCESS;
            i = (rc + DISK_BLOCK_SIZE - 1) & -DISK_BLOCK_SIZE;
            memset (rplbuf -> u.readblocks.buff + rc, 0, i - rc);
            rc = i;
          }
          rplbuf -> u.readblocks.rlen = rc;
        }
      }
      rpllen = rplbuf -> u.readblocks.buff + rplbuf -> u.readblocks.rlen - (uByte *)rplbuf;
      break;
    }

    /* Read record from file */

    case OZ_IO_FS_READREC: {
      if (reqbuf.handle == 0) {
        rplbuf -> status = OZ_FILENOTOPEN;
        break;
      }
      rplbuf -> status = OZ_IOFAILED;
      rplbuf -> u.readblocks.rlen = 0;

      /* If starting position supplied, position file at that spot, otherwise get current position */

      if (reqbuf.u.readrec.atblock != 0) {
        pos = (reqbuf.u.readrec.atblock - 1) * DISK_BLOCK_SIZE + reqbuf.u.readrec.atbyte;
        if (lseek (reqbuf.handle, pos, SEEK_SET) < 0) {
          fprintf (stderr, "error seeking to %d before readrec: %s\n", pos, strerror (errno));
          break;
        }
      } else {
        pos = lseek (reqbuf.handle, 0, SEEK_CUR);
      }

      /* If terminator given, ... */

      if (reqbuf.u.readrec.trmsize != 0) {

        /* Read enough to hold both full record buffer and full terminator */

        rlen = reqbuf.u.readrec.size + reqbuf.u.readrec.trmsize;
        if (rlen > sizeof readrecbuf) rlen = sizeof readrecbuf;
        rc = read (reqbuf.handle, readrecbuf, rlen);
        if (rc < 0) {
          fprintf (stderr, "error reading at %d: %s\n", pos, strerror (errno));
          break;
        }

        /* End-of-file if nothing read */

        rplbuf -> status = OZ_ENDOFFILE;
        rlen = rc;
        if (rc > 0) {

          /* Assume we won't find terminator */

          rplbuf -> status = OZ_NOTERMINATOR;
          if (rc >= reqbuf.u.readrec.trmsize) {

            /* Scan for terminator and change status to success if found */

            for (rlen = 0; (rlen < sizeof rplbuf -> u.readrec.buff) && (rlen <= rc - reqbuf.u.readrec.trmsize); rlen ++) {
              if (memcmp (readrecbuf + rlen, reqbuf.u.readrec.trmbuff, reqbuf.u.readrec.trmsize) == 0) {
                rplbuf -> status = OZ_SUCCESS;
                break;
              }
            }
          }
        }

        /* Save return length and copy what we got to reply buffer */

        rplbuf -> u.readrec.rlen = rlen;
        memcpy (rplbuf -> u.readrec.buff, readrecbuf, rlen);
      }

      /* No terminator given, read the whole buffer in */

      else {
        rlen = reqbuf.u.readrec.size;
        if (rlen > sizeof rplbuf -> u.readrec.buff) rlen = sizeof rplbuf -> u.readrec.buff;
        rc = read (reqbuf.handle, rplbuf -> u.readrec.buff, rlen);
        if (rc < 0) {
          fprintf (stderr, "error reading at %d: %s\n", pos, strerror (errno));
          break;
        }
        rplbuf -> status = OZ_ENDOFFILE;
        rlen = rc;
        if (rc > 0) rplbuf -> status = OZ_SUCCESS;
        rplbuf -> u.readrec.rlen = rlen;
      }

      /* Position just past what was found */

      if (rplbuf -> status == OZ_SUCCESS) rlen += reqbuf.u.readrec.trmsize;
      if (lseek (reqbuf.handle, pos + rlen, SEEK_SET) < 0) {
        fprintf (stderr, "error seeking to %d after readrec: %s\n", pos + rlen, strerror (errno));
        break;
      }

      rpllen = rplbuf -> u.readrec.buff + rplbuf -> u.readrec.rlen - (uByte *)rplbuf;
      break;
    }

    /* Close file */

    case OZ_IO_FS_CLOSE: {
      if (reqbuf.handle == 0) {
        rplbuf -> status = OZ_FILENOTOPEN;
        break;
      }
      if (reqbuf.handle < MAXFILES) {
        close (reqbuf.handle);
        openfiles[reqbuf.handle].fd = 0;
        read_dir_close (&(openfiles[reqbuf.handle].dirread));
        rplbuf -> status = OZ_SUCCESS;
        break;
      }
      if (reqbuf.handle < MAXWILDS + MAXFILES) {
        if (wildcards[reqbuf.handle-MAXFILES] != NULL) {
          wild_term (wildcards[reqbuf.handle-MAXFILES]);
          wildcards[reqbuf.handle-MAXFILES] = NULL;
        }
        rplbuf -> status = OZ_SUCCESS;
        break;
      }
      rplbuf -> status = OZ_FILENOTOPEN;
      break;
    }

    /* Get info, part 1 */

    case OZ_IO_FS_GETINFO1: {
      if (reqbuf.handle == 0) {
        rplbuf -> status = OZ_FILENOTOPEN;
        break;
      }
      if (fstat (reqbuf.handle, &statbuf) < 0) {
        fprintf (stderr, "error statting file: %s\n", strerror (errno));
        rplbuf -> status = OZ_IOFAILED;
        break;
      }
      memset (&(rplbuf -> u.getinfo1), 0, sizeof rplbuf -> u.getinfo1);
      rplbuf -> u.getinfo1.blocksize = DISK_BLOCK_SIZE;
      rplbuf -> u.getinfo1.eofblock  = (statbuf.st_size / DISK_BLOCK_SIZE) + 1;
      rplbuf -> u.getinfo1.eofbyte   = statbuf.st_size % DISK_BLOCK_SIZE;
      rplbuf -> u.getinfo1.hiblock   = statbuf.st_blocks;
      pos = lseek (reqbuf.handle, 0, SEEK_CUR);
      if (pos >= 0) {
        rplbuf -> u.getinfo1.curblock = (pos / DISK_BLOCK_SIZE) + 1;
        rplbuf -> u.getinfo1.curbyte  = pos % DISK_BLOCK_SIZE;
      }
      if (S_ISDIR (statbuf.st_mode)) rplbuf -> u.getinfo1.filattrflags = OZ_FS_FILATTRFLAG_DIRECTORY;
      rplbuf -> u.getinfo1.access_date = unix_time_to_datebin (statbuf.st_atime);
      rplbuf -> u.getinfo1.modify_date = unix_time_to_datebin (statbuf.st_mtime);
      rplbuf -> u.getinfo1.change_date = unix_time_to_datebin (statbuf.st_ctime);
      rplbuf -> status = OZ_SUCCESS;
      break;
    }

    /* Read next directory entry */

    case OZ_IO_FS_READDIR: {
      rplbuf -> status = OZ_FILENOTOPEN;
      if (reqbuf.handle != 0) {
        rplbuf -> status = read_dir_next (openfiles + reqbuf.handle, &(openfiles[reqbuf.handle].dirread), &fn, &fileid);
      }
      if (rplbuf -> status != OZ_SUCCESS) {
        printf ("readdir*: '%s' status %u\n", openfiles[reqbuf.handle].filest, rplbuf -> status);
        rplbuf -> u.readdir.name[0] = 0;
      } else {
        printf ("readdir*: '%s' '%s'\n", openfiles[reqbuf.handle].filest, fn);
        strncpy (rplbuf -> u.readdir.name, fn, sizeof rplbuf -> u.readdir.name);
      }
      break;
    }

    /* Perform wildcard scan */

    case OZ_IO_FS_WILDSCAN: {

      rplbuf -> u.wildscan.handle = reqbuf.handle;

      /* Can't do it if file is open on channel */

      if (((rplbuf -> u.wildscan.handle != 0) && (rplbuf -> u.wildscan.handle < MAXFILES)) || (rplbuf -> u.wildscan.handle >= MAXFILES + MAXWILDS)) {
        rplbuf -> status = OZ_FILEALREADYOPEN;
        break;
      }

      /* Get current associated wildcard context */

      wild = NULL;
      if (rplbuf -> u.wildscan.handle != 0) {
        wild = wildcards[rplbuf->u.wildscan.handle-MAXFILES];
      }

      /* Maybe it is being re-initialized */

      if (reqbuf.u.wildscan.init || (wild == NULL)) {
        if (wild != NULL) {
          wild_term (wild);
          wildcards[rplbuf->u.wildscan.handle-MAXFILES] = NULL;
          rplbuf -> u.wildscan.handle = 0;
        }
        for (pos = MAXFILES; pos < MAXFILES + MAXWILDS; pos ++) {
          if (wildcards[pos-MAXFILES] == NULL) break;
        }
        if (pos == MAXFILES + MAXWILDS) {
          fprintf (stderr, "no more available wilds entries\n");
          rplbuf -> status = OZ_HANDTBLFULL;
          break;
        }
        rplbuf -> status = wild_init (reqbuf.u.wildscan.wild, &wild);
        if (rplbuf -> status != OZ_SUCCESS) break;
        rplbuf -> u.wildscan.handle = pos;
        wildcards[pos-MAXFILES] = wild;
      }

      /* Get first/next matching entry */

      rplbuf -> status = wild_next (wild, &fn, NULL);
      if (fn != NULL) {
        strncpy (rplbuf -> u.wildscan.spec, fn, sizeof rplbuf -> u.wildscan.spec);
        free (fn);
      }

      /* If reached the end, close it */

      if (rplbuf -> status == OZ_ENDOFFILE) {
        wild_term (wild);
        wildcards[rplbuf->u.wildscan.handle-MAXFILES] = NULL;
        rplbuf -> u.wildscan.handle = 0;
      }
      break;
    }

    /* Set current file position */

    case OZ_IO_FS_SETCURPOS: {
      if (reqbuf.handle == 0) {
        rplbuf -> status = OZ_FILENOTOPEN;
        break;
      }
      if (lseek (reqbuf.handle, ((reqbuf.u.setcurpos.atblock - 1) * DISK_BLOCK_SIZE) + reqbuf.u.setcurpos.atbyte, SEEK_SET) < 0) {
        fprintf (stderr, "error seeking to %u: %s\n", ((reqbuf.u.setcurpos.atblock - 1) * DISK_BLOCK_SIZE) + reqbuf.u.setcurpos.atbyte, strerror (errno));
        rplbuf -> status = OZ_IOFAILED;
        break;
      }
      rplbuf -> status = OZ_SUCCESS;
      break;
    }

    /* Who knows what */

    default: {
      fprintf (stderr, "bad I/O function code 0x%8.8x\n", reqbuf.func);
      rplbuf -> status = OZ_BADIOFUNC;
      break;
    }
  }

  /* Send reply back to client */

send_reply:
  if (sendto (udpfd, rplbuf, rpllen, 0, (void *)&cliaddr, cliaddrlen) < 0) {
    fprintf (stderr, "error sending reply: %s\n", strerror (errno));
    return (-1);
  }

  goto recvloop;
}

/************************************************************************/
/*									*/
/*  Wildcard scanning initialization routine				*/
/*									*/
/*    Input:								*/
/*									*/
/*	spec = wildcard specification					*/
/*									*/
/*    Output:								*/
/*									*/
/*	wild_init = OZ_SUCCESS : successful initialization		*/
/*	                  else : error status				*/
/*	*wild_r = wildcard search context block				*/
/*									*/
/************************************************************************/

static uLong wild_init (const char *spec, Wild **wild_r)

{
  char savech1, savech2;
  const char *fname;
  int i, l;
  uLong sts;
  Wild *wild;

  l = strlen (spec);						/* get total length of wildcard spec string */
  wild = malloc (l + sizeof *wild);				/* allocate a 'wild' context block */
  memcpy (wild -> spec, spec, l);				/* copy in the wildcard spec string */
  wild -> spec[l] = 0;						/* null terminate it */

  wild -> dirfile = NULL;					/* clear all our links */
  wild -> dirread = NULL;
  wild -> next    = NULL;
  wild -> prev    = NULL;
  wild -> fatal   = 0;

  for (i = 0; i < l; i ++) {					/* scan the wildcard spec string for first wildcard char */
    if (wild -> spec[i] == '?') break;
    if (wild -> spec[i] == '*') break;
    if (memcmp (wild -> spec + i, "...", 3) == 0) break;
  }
  while (i > 0) {						/* point just past / of innermost non-wild directory name - this is the top dir to scan */
    if (wild -> spec[i-1] == '/') break;
    -- i;
  }
  if (i == 0) {
    free (wild);						/* it must have at least one / in it */
    return (OZ_BADFILENAME);
  }
  wild -> firstwc = i;						/* save pointer just past that / */

  savech1 = wild -> spec[i];					/* parse it to get the dirid of that directory */
  savech2 = wild -> spec[i+1];
  wild -> spec[i]   = 'x';					/* (give it some dummy filename to return in wild -> fname) */
  wild -> spec[i+1] = 0;
  sts = getdirid (wild -> spec, &(wild -> dirid), &fname);	/* fname basically points to the 'x' that we just put in there */
  wild -> spec[i]   = savech1;
  wild -> spec[i+1] = savech2;
  if (sts != OZ_SUCCESS) {
    wild -> spec[i] = 0;
    fprintf (stderr, "error %u looking up directory %s\n", sts, wild -> spec);
    free (wild);
    return (sts);
  }
  *wild_r = wild;
  return (OZ_SUCCESS);
}

/************************************************************************/
/*									*/
/*  Get next wildcard match						*/
/*									*/
/*    Input:								*/
/*									*/
/*	topwild = wildcard context pointer				*/
/*									*/
/*    Output:								*/
/*									*/
/*	wild_next = OZ_SUCCESS : entry found				*/
/*	          OZ_ENDOFFILE : no more matching entries		*/
/*	                  else : directory read error			*/
/*	*spec_r = pointer to malloc'd complete filespec			*/
/*									*/
/************************************************************************/

static uLong wild_next (Wild *topwild, char **spec_r, Fileid *fileid_r)

{
  char *fname, *tmpbuf;
  File *file;
  Fileid *fileid;
  int i;
  uLong sts;
  Wild *botwild, *newwild;

  *spec_r = NULL;

  /* If we had a fatal error on the top, return endoffile so caller will know to stop */

  if (topwild -> fatal) return (OZ_ENDOFFILE);

  /* Skip to innermost directory */

  for (botwild = topwild; botwild -> next != NULL; botwild = botwild -> next) {}

  /* Find next matching entry */

  while (1) {

    /* If the innermost directory is not open, open it                                          */
    /* (Last time around found this directory name itself and newly created this botwild block) */

openbot:

    if (botwild -> dirfile == NULL) {
      sts = open_by_fid (&(botwild -> dirid), OZ_SECACCMSK_READ, &(botwild -> dirfile));
      if (sts != OZ_SUCCESS) {
        fprintf (stderr, "error %u opening directory %s\n", sts, botwild -> spec);
        botwild -> fatal = 1;
        botwild = botwild -> prev;
        if (botwild != NULL) {
          free (botwild -> next);
          botwild -> next = NULL;
        }
        return (sts);
      }
    }

    /* Scan through the directory to find a matching entry */

    while ((sts = read_dir_next (botwild -> dirfile, &(botwild -> dirread), &fname, &fileid)) == OZ_SUCCESS) { /* read entry */
      i = botwild -> firstwc + strlen (fname);
      tmpbuf = malloc (i + 1);									/* make composite spec */
      memcpy (tmpbuf, botwild -> spec, botwild -> firstwc);
      strcpy (tmpbuf + botwild -> firstwc, fname);
      sts = wild_match (tmpbuf + topwild -> firstwc, topwild -> spec + topwild -> firstwc);	/* see if it matches original wildcard spec */
      if ((sts & WILD_MATCH_DOSUBDR) && (tmpbuf[i-1] == '/')) {					/* see if it is a subdirectory name */
        newwild = malloc (i + 1 + sizeof *newwild);						/* it is, alloc an inner wildcard context block */
        botwild -> next    = newwild;								/* link it on old innermost block */
        newwild -> prev    = botwild;
        newwild -> next    = NULL;
        newwild -> dirid   = *fileid;								/* save the fileid of the new inner directory */
        newwild -> dirread = NULL;								/* we haven't read anything from it yet */
        newwild -> dirfile = NULL;								/* we haven't opened the subdirectory yet */
        newwild -> fatal   = 0;
        strcpy (newwild -> spec, tmpbuf);							/* save complete spec of the subdirectory */
        newwild -> firstwc = i;									/* point to the end of the spec */
        if (!(sts & WILD_MATCH_MATCHES)) {							/* see if the subdir spec itself matches the wildcard */
          botwild = newwild;									/* if not, start processing the subdirectory */
          free (tmpbuf);									/* free off the temp subdir name buffer */
          goto openbot;										/* ... go process it */
        }
      }
      if (sts & WILD_MATCH_MATCHES) {								/* see if spec matches wildcard */
        *spec_r = tmpbuf;									/* if so, return pointer to complete filespec string */
        if (fileid_r != NULL) *fileid_r = *fileid;
        return (OZ_SUCCESS);									/* successful */
      }
      free (tmpbuf);										/* doesn't match, release the temp buffer */
    }

    /* Reached end of sub-directory */

    if (sts != OZ_ENDOFFILE) {
      fprintf (stderr, "error %u reading directory %s\n", sts, botwild -> dirread);
      read_dir_close (&(botwild -> dirread));							/* read error, done reading this directory */
      close_file (botwild -> dirfile);								/* close the directory file itself */
      botwild -> dirfile = NULL;								/* don't have directory file open anymore */
      botwild -> fatal = 1;
      botwild = botwild -> prev;								/* free off this directory's context block */
      if (botwild != NULL) {									/* ... if it is not the top one */
        free (botwild -> next);
        botwild -> next = NULL;
      }
      return (sts);
    }
    read_dir_close (&(botwild -> dirread));							/* normal end, done reading this directory */
    close_file (botwild -> dirfile);								/* close the directory file itself */
    botwild -> dirfile = NULL;									/* don't have directory file open anymore */
    if (botwild -> prev == NULL) return (OZ_ENDOFFILE);						/* if this is topmost directory, we're all done searching */
    botwild = botwild -> prev;									/* not topmost, free off innermost directory block */
    free (botwild -> next);
    botwild -> next = NULL;
  }												/* ... then continue processing next outer directory where we left off */
}

/************************************************************************/
/*									*/
/*  Terminate wildcard search context					*/
/*									*/
/*    Input:								*/
/*									*/
/*	topwild = previously established wildcard context		*/
/*									*/
/*    Output:								*/
/*									*/
/*	topwild no longer valid						*/
/*									*/
/************************************************************************/

static void wild_term (Wild *topwild)

{
  Wild *botwild;


  for (botwild = topwild; botwild -> next != NULL; botwild = botwild -> next) {}	/* skip to innermost directory */

  do {
    read_dir_close (&(botwild -> dirread));						/* close any directory scanning context that might be open */
    if (botwild -> dirfile != NULL) close_file (botwild -> dirfile);			/* close any directory file that might be open */
    topwild = botwild -> prev;								/* point to next outer directory, if any */
    free (botwild);									/* free off the innermost directory */
    botwild = topwild;									/* start working on the next outer directory, if any */
  } while (botwild != NULL);
}

/************************************************************************/
/*									*/
/*  See if a given filespec matches a given wildcard spec		*/
/*									*/
/*    Input:								*/
/*									*/
/*	filespec = filespec to be matched				*/
/*	wildspec = wildcard spec to match against			*/
/*									*/
/*    Output:								*/
/*									*/
/*	wild_match & WILD_MATCH_MATCHES : this filespec matches the wildcard
/*	wild_match & WILD_MATCH_DOSUBDR : go into the 'filespec' directory
/*									*/
/*	? : match any single character except /				*/
/*	* : match any string of characters except /			*/
/*	... : match any string of characters, including /		*/
/*									*/
/************************************************************************/

static uLong wild_match (char *filespec, char *wildspec)

{
  char fc, *fe, *fp, wc, *we, *wp;

  /* Strip off as many matching characters off the end as we can */
  /* This helps with optimizations later                         */

  fp = filespec;
  wp = wildspec;

  fe = fp + strlen (fp);
  we = wp + strlen (wp);

  while ((fe > fp) && (we > wp)) {
    fc = *(-- fe);
    wc = *(-- we);
    if (fc == wc) continue;
    if ((wc != '?') || (fc == '/')) {
      fe ++;
      we ++;
      break;
    }
  }

  return (wild_match2 (fp, fe, wp, we));
}

static uLong wild_match2 (char *fp, char *fe, char *wp, char *we)

{
  char fc, wc;
  uLong submatch;

  /* Skip over as many directly matching characters as possible. */
  /* Also do the '?' matching that matches a single non-/ char.  */

compare:
  while (1) {
    if (wp == we) goto endofwild;
    wc = *wp;
    if ((wp + 3 <= we) && (wc == '.') && (wp[1] == '.') && (wp[2] == '.')) goto elipses;
    if (fp == fe) break;
    fc = *fp;
    if ((fc != wc) && ((wc != '?') || (fc == '/'))) break;
    fp ++;
    wp ++;
  }

  /* Only hope is the wildcard char is an asterisk.  Otherwise we fail. */

  if (wc != '*') return (0);

  /* Asterisk matches any number of any character except /.                   */
  /* So repeatedly try to match by gobbling non / chars from filespec string. */

  while ((wp < we) && ((wc = *(++ wp)) == '*')) {}	/* skip redundant *'s in wildcard spec */
  if ((wp == we) || (wc == '/')) {			/* optimiszation: if * is at end or a directory follows it ... */
    while ((fp < fe) && (*fp != '/')) fp ++;		/* ... just skip over all non-/ characters up to a / or end of string */
    goto compare;					/* ... then resume direct comparison from there */
  }
  while (1) {
    submatch = wild_match2 (fp, fe, wp, we);		/* try to match what's left of both strings */
    if (submatch & WILD_MATCH_MATCHES) break;		/* if they match, return success status */
    if ((*fp == '/') || (*fp == 0)) break;		/* no match, if at directory we can't skiip anything */
    fp ++;						/* not at directory, skip over the char and try again */
  }
  return (submatch);					/* return match/nomatch, passing along the DOSUBDIR flag */

  /* Elipses match any string including directory slashes */

elipses:
  wp += 3;						/* point past the ... */
  while ((wp < we) && ((wc = *(++ wp)) == '*')) {}	/* skip redundant *'s in wildcard spec */
  if (wp == we) return (WILD_MATCH_MATCHES | WILD_MATCH_DOSUBDR);
  while (1) {
    submatch = wild_match2 (fp, fe, wp, we);		/* try to match what's left of both strings */
    if (submatch & WILD_MATCH_MATCHES) break;		/* if they match, return success status */
    if (fp == fe) break;				/* no match, done if no more filepsec string */
    fp ++;						/* skip over the char and try again */
  }
  return (submatch | WILD_MATCH_DOSUBDR);		/* return match/nomatch, force the DOSUBDIR flag because of the ...'s */

  /* If we reached the end of both strings, we have a match.                                              */
  /* Don't do any sub-directory because it won't match (we've reached the end of the wildcard as we are). */
  /* Special case: If wildcard string is exhausted and all that's left of filespec string is just "/", we match */
  /* This is to allow a directory to be included in its parent directory                                        */

endofwild:
  if (fp == fe) return (WILD_MATCH_MATCHES);
  if ((*fp == '/') && (fp + 1 == fe)) return (WILD_MATCH_MATCHES);
  return (0);
}

/************************************************************************/
/*									*/
/*  Get directory id from filespec string				*/
/*									*/
/*    Input:								*/
/*									*/
/*	filespec = null terminated filespec string			*/
/*	           /<dir>/<dir>/.../<fname>[/]				*/
/*									*/
/*    Output:								*/
/*									*/
/*	getdirid = OZ_SUCCESS : success					*/
/*	                 else : error status				*/
/*	*dirid_r = directory-id						*/
/*	*fname_r = pointer to <fname>[/] portion of input string	*/
/*									*/
/************************************************************************/

typedef struct Dirid { struct Dirid *next;
                       Fileid dirid;
                     } Dirid;

static uLong getdirid (const char *filespec, Fileid *dirid_r, const char **fname_r)

{
  const char *p, *q;
  Dirid *dirid, *dirids;
  File *dirfile;
  uLong sts;

  dirids = NULL;

  /* Make sure it starts with an '/' */

  p = filespec;
  if (*p != '/') return (OZ_BADFILENAME);

  /* Find last occurrence of '//'.  Start at that point.  This will allow fs independent parsers to just    */
  /* tack a full filespec to follow a default directory, ie, if the default directory is /h1/mrieker/, and  */
  /* the user specifies /etc/passwd, the parser can simply put /h1/mrieker//etc/passwd and get /etc/passwd. */
  /* If the user specifies .profile, the parser simply puts /h1/mrieker/.profile to get the desired file.   */

  for (q = p; *q != 0; q ++) if ((q[0] == '/') && (q[1] == '/')) p = q + 1;
  p ++;

  /* Start with the root directory id */

  strcpy (dirid_r -> fidstr, basename);
  strcat (dirid_r -> fidstr, "/");

  /* If accessing "/", that means the root directory itself */

  if (*p == 0) {
    *fname_r = "/";
    return (OZ_SUCCESS);
  }

  /* Keep repeating as long as there are more '/'s in the filepsec (not including any slash that might be the last char in filespec) */

  while (((q = strchr (p, '/')) != NULL) && (q[1] != 0)) {	/* point q at the next slash */
    if ((q == p + 1) && (p[0] == '.')) {			/* './' is a no-op */
      p = q + 1;
      continue;
    }
    if ((q == p + 2) && (p[0] == '.') && (p[1] == '.')) {	/* '../' goes up one level, if not at root */
      if (dirids != NULL) {
        *dirid_r = dirids -> dirid;
        dirid = dirids -> next;
        free (dirids);
        dirids = dirid;
      }
      p = q + 1;
      continue;
    }
    dirid = malloc (sizeof *dirid);				/* push the previous directory's directory id on stack in case of subsequent '../' */
    dirid -> next  = dirids;
    dirid -> dirid = *dirid_r;
    dirids = dirid;
    sts = open_by_fid (dirid_r, OZ_SECACCMSK_LOOK, &dirfile);	/* open the previous directory */
    if (sts != OZ_SUCCESS) goto cleanup;			/* abort if failed to open */
    q ++;							/* increment past the slash */
    sts = lookup_file (dirfile, q - p, p, dirid_r);		/* lookup that file in the previous directory */
    close_file (dirfile);					/* close previous directory */
    if (sts != OZ_SUCCESS) goto cleanup;			/* abort if failed to find new directory in previous directory */
    p = q;							/* point to next thing in input string */
  }

  /* Don't allow <fname> of '.', './', '..' or '../' so they can't create files named as such, because we use those names specially above */

  if ((p[0] == '.') && ((p[1] == 0) || (p[1] == '/') || ((p[1] == '.') && ((p[2] == 0) || (p[2] == '/'))))) {
    sts = OZ_BADFILENAME;
    goto cleanup;
  }

  /* Return pointer to <fname> and return success status */

  *fname_r = p;
  sts = OZ_SUCCESS;

cleanup:
  while ((dirid = dirids) != NULL) {
    dirids = dirid -> next;
    free (dirid);
  }
  return (sts);
}

/************************************************************************/
/*									*/
/*  Open file by file id						*/
/*									*/
/*    Input:								*/
/*									*/
/*	volume    = volume struct pointer				*/
/*	fileid    = file id to be opened				*/
/*	secaccmsk = security access mask bits				*/
/*									*/
/*    Output:								*/
/*									*/
/*	open_by_fid = OZ_SUCCESS : successful completion		*/
/*	                    else : error status				*/
/*	*file_r = pointer to file struct				*/
/*									*/
/************************************************************************/

static uLong open_by_fid (Fileid *fileid, OZ_Secaccmsk secaccmsk, File **file_r)

{
  File *file;
  int fd;

  *file_r = NULL;

  fd = open (fileid -> fidstr, O_RDONLY);
  if (fd < 0) {
    if (errno == ENOENT) return (OZ_NOSUCHFILE);
    fprintf (stderr, "error opening %s: %s\n", fileid -> fidstr, strerror (errno));
    return (OZ_IOFAILED);
  }

  file = malloc (sizeof *file);
  file -> fd = fd;
  strcpy (file -> filest, fileid -> fidstr);
  *file_r = file;
  return (OZ_SUCCESS);
}

/************************************************************************/
/*									*/
/*  Lookup a file in a directory					*/
/*									*/
/*    Input:								*/
/*									*/
/*	dirfile = directory file					*/
/*	namelen = length of *name string				*/
/*	name    = name to lookup (not necessarily null terminated)	*/
/*									*/
/*    Output:								*/
/*									*/
/*	lookup_file = OZ_SUCCESS : successful				*/
/*	                    else : error status				*/
/*	*fileid_r = file-id of found file				*/
/*									*/
/************************************************************************/

static uLong lookup_file (File *dirfile, int namelen, const char *name, Fileid *fileid_r)

{
  int i, rc;
  struct stat statbuf;

  i = strlen (dirfile -> filest);
  if (i + namelen >= sizeof fileid_r -> fidstr) {
    fprintf (stderr, "filespec string %s%*.*s too long\n", dirfile -> filest, namelen, namelen, name);
    return (OZ_IOFAILED);
  }
  memcpy (fileid_r -> fidstr, dirfile -> filest, i);
  memcpy (fileid_r -> fidstr + i, name, namelen);
  fileid_r -> fidstr[i+namelen] = 0;

  rc = stat (fileid_r -> fidstr, &statbuf);
  if (rc < 0) {
    if (errno == ENOENT) return (OZ_NOSUCHFILE);
    fprintf (stderr, "error statting %s: %s\n", fileid_r -> fidstr, strerror (errno));
    return (OZ_IOFAILED);
  }

  return (OZ_SUCCESS);
}

/************************************************************************/
/*									*/
/*  Close file								*/
/*									*/
/************************************************************************/

static void close_file (File *file)

{
  close (file -> fd);
  free (file);
}

/************************************************************************/
/*									*/
/*  Read directory entry						*/
/*									*/
/*    Input:								*/
/*									*/
/*	dirfile = directory file to read				*/
/*	**dirread_r = directory read context				*/
/*	              NULL to start					*/
/*									*/
/*    Output:								*/
/*									*/
/*	read_dir_next = OZ_SUCCESS : success				*/
/*	              OZ_ENDOFFILE : end of directory			*/
/*	                      else : error status			*/
/*	**dirread_r = updated to point to next entry			*/
/*	**fname_r   = directory name string (null terminated)		*/
/*	**fileid_r  = points to file-id in the directory		*/
/*									*/
/************************************************************************/

static uLong read_dir_next (File *dirfile, Dirread **dirread_r, char **fname_r, Fileid **fileid_r)

{
  char *fn;
  Dirread *dirread;
  int rc;
  struct dirent **namelist;
  struct stat statbuf;

  /* If initializing context, scan the directory into memory and set up dirread context */

  dirread = *dirread_r;
  if (dirread == NULL) {
    rc = scandir (dirfile -> filest, &namelist, NULL, alphasort);
    if (rc < 0) {
      if (errno == ENOENT) return (OZ_NOSUCHFILE);
      fprintf (stderr, "error scanning directory %s: %s\n", dirfile -> filest, strerror (errno));
      return (OZ_IOFAILED);
    }
    dirread = malloc (sizeof *dirread);
    dirread -> nextent  = 0;
    dirread -> numents  = rc;
    dirread -> namelist = namelist;
    *dirread_r = dirread;
  }

  /* Return pointer to filename string and increment index for next time */

  do {
    if (dirread -> nextent >= dirread -> numents) return (OZ_ENDOFFILE);
    fn = dirread -> namelist[dirread->nextent++] -> d_name;
  } while ((strcmp (fn, ".") == 0) || (strcmp (fn, "..") == 0) || (fn[strlen(fn)-1] == '~'));

  /* Fill in fileid */

  if (strlen (dirfile -> filest) + strlen (fn) + 1 >= sizeof dirread -> entryfid.fidstr) {
    fprintf (stderr, "filespec string %s%s too long\n", dirfile -> filest, fn);
    return (OZ_IOFAILED);
  }

  strcpy (dirread -> entryfid.fidstr, dirfile -> filest);
  strcat (dirread -> entryfid.fidstr, fn);

  /* If it is a directory, append a "/" to filename */

  if ((stat (dirread -> entryfid.fidstr, &statbuf) >= 0) && S_ISDIR (statbuf.st_mode)) {
    strcat (dirread -> entryfid.fidstr, "/");
  }

  /* Return name and fid pointers */

  *fname_r  = dirread -> entryfid.fidstr + strlen (dirfile -> filest);
  *fileid_r = &(dirread -> entryfid);

  return (OZ_SUCCESS);
}

/************************************************************************/
/*									*/
/*  Close any directory read context that might be open			*/
/*									*/
/*    Input:								*/
/*									*/
/*	*dirread_r = previous context set up by read_dir_next		*/
/*	             or NULL if none was set up				*/
/*									*/
/*    Output:								*/
/*									*/
/*	context freed off						*/
/*	*dirread_r = NULL						*/
/*									*/
/************************************************************************/

static read_dir_close (Dirread **dirread_r)

{
  Dirread *dirread;
  int i;

  dirread = *dirread_r;
  if (dirread != NULL) {
    for (i = 0; i < dirread -> numents; i ++) free (dirread -> namelist[i]);
    free (dirread -> namelist);
    free (dirread);
    *dirread_r = NULL;
  }
}

/************************************************************************/
/*									*/
/*  Convert a unix time to a datebin time				*/
/*									*/
/************************************************************************/

static OZ_Datebin unix_time_to_datebin (time_t unix_time)

{
  uLong datelongs[OZ_DATELONG_ELEMENTS];
  OZ_Datebin datebin;

  static uLong basedaynumber = 0;

  if (basedaynumber == 0) basedaynumber = oz_sys_daynumber_encode ((1970 << 16) | (1 << 8) | 1);

  memset (datelongs, 0, sizeof datelongs);
  datelongs[OZ_DATELONG_SECOND] = unix_time % 86400;
  datelongs[OZ_DATELONG_DAYNUMBER] = unix_time / 86400 + basedaynumber;
  return (oz_sys_datebin_encode (datelongs));
}

Long oz_sys_gettimezone (void)

{
  return (0);
}

uLong oz_sys_tzconv (OZ_Datebin in, uLong h_tzfilein, OZ_Datebin *out, int tznameoutl, char *tznameout)

{
  return (OZ_BADTZFILE);
}

void oz_crash ()

{
  abort ();
}
