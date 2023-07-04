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
/*  I/O function codes for device class "fs" (filesystem)		*/
/*									*/
/************************************************************************/

#ifndef _OZ_IO_FS_H
#define _OZ_IO_FS_H

typedef struct OZ_IO_fs_getinfo1 OZ_IO_fs_getinfo1;
typedef struct OZ_IO_fs_getinfo2 OZ_IO_fs_getinfo2;
typedef struct OZ_IO_fs_getinfo3 OZ_IO_fs_getinfo3;

#include "ozone.h"
#include "oz_itmlst.h"
#include "oz_knl_devio.h"
#include "oz_knl_hw.h"

#define OZ_IO_FS_CLASSNAME "fs"
#define OZ_IO_FS_BASE (0x00000600)
#define OZ_IO_FS_MASK (0xFFFFFF00)

#define OZ_FS_MAXFNLEN 256			/* maximum complete filename string length */
#define OZ_FS_MAXFIDLN 256			/* maximum length of a fileid buffer */

#define OZ_FS_INITFLAG_WRITETHRU (0x1)		/* flush all writes through to magnetic media immediately */

#define OZ_FS_MOUNTFLAG_READONLY (0x1)		/* mount it read-only */
#define OZ_FS_MOUNTFLAG_NOCACHE (0x2)		/* do not enable cache */
#define OZ_FS_MOUNTFLAG_WRITETHRU (0x4)		/* flush all writes through to magnetic media immediately */
#define OZ_FS_MOUNTFLAG_VERIFY (0x8)		/* force verification on mount */

#define OZ_FS_EXTFLAG_NOTRUNC (0x1)		/* don't truncate on extend */
#define OZ_FS_EXTFLAG_NOEXTHDR (0x2)		/* don't allocate extension header */

#define OZ_FS_FILATTRFLAG_DIRECTORY (0x1)	/* file is a directory file */
#define OZ_FS_FILATTRFLAG_WRITETHRU (0x2)	/* flush all writes through to magnetic media immediately */

						/* file specification substrings - */
typedef struct { uLong dirsize;			/* size of resultant directory substring (including all /'s) */
                 uLong namsize;			/* size of resultant name string */
                 uLong typsize;			/* size of resultant type string (including the .) */
                 uLong versize;			/* size of resultant version string (including the ;) */
               } OZ_FS_Subs;

#include "oz_knl_hw.h"
#include "oz_knl_lock.h"

/* Initialize volume */

#define OZ_IO_FS_INITVOL OZ_IO_DW(OZ_IO_FS_BASE,1)

typedef struct { const char *devname;		/* device name that volume is in */
                 const char *volname;		/* volume name to write to device */
                 uLong clusterfactor;		/* storage bitmap cluster factor */
                 uLong secattrsize;		/* security attributes buffer size */
                 const void *secattrbuff;	/* security attributes buffer address */
                 uLong initflags;		/* option flags */
               } OZ_IO_fs_initvol;

/* Mount volume */

#define OZ_IO_FS_MOUNTVOL OZ_IO_DR(OZ_IO_FS_BASE,2)

typedef struct { const char *devname;		/* device name that volume is in */
                 uLong mountflags;		/* option flags */
                 uLong secattrsize;		/* set device security to this when mounted */
                 const void *secattrbuff;
               } OZ_IO_fs_mountvol;

/* Dismount volume */

#define OZ_IO_FS_DISMOUNT OZ_IO_DR(OZ_IO_FS_BASE,3)

typedef struct { int unload;			/* 0=leave spun up; 1=spin down and unload */
               } OZ_IO_fs_dismount;

/* Create a new file */

#define OZ_IO_FS_CREATE OZ_IO_DW(OZ_IO_FS_BASE,4)

typedef struct { const char *name;		/* null terminated fsname to create */
                 OZ_Lockmode lockmode;		/* lock mode to open with */
                 uLong filattrflags;		/* file attribute flags */
                 uLong secattrsize;		/* security attributes buffer size */
                 const void *secattrbuff;	/* security attributes buffer address */
                 uLong rnamesize;		/* resultant name buffer size */
                 char *rnamebuff;		/* resultant name buffer address */
                 OZ_FS_Subs *rnamesubs;		/* resultant name substring sizes */
                 int ignclose;			/* 1: ignore OZ_IO_FS_CLOSE calls (ie, leave file open on channel) */
                 int newversion;		/* force this to have highest version number */
                 uLong fileidsize;		/* size of fileidbuff (or zero if not wanted) */
                 void *fileidbuff;		/* where to return resultant fileid */
               } OZ_IO_fs_create;

/* Open an existing file */

#define OZ_IO_FS_OPEN OZ_IO_DN(OZ_IO_FS_BASE,5)

typedef struct { const char *name;		/* null terminated fsname to open (or NULL to open by fileid) */
                 OZ_Lockmode lockmode;		/* lock mode to open with */
                 uLong rnamesize;		/* if name != NULL, resultant name buffer size */
                 char *rnamebuff;		/* if name != NULL, resultant name buffer address */
                 OZ_FS_Subs *rnamesubs;		/* if name != NULL, resultant name substring sizes */
                 int ignclose;			/* 1: ignore OZ_IO_FS_CLOSE calls (ie, leave file open on channel) */
                 uLong fileidsize;		/* size of fileidbuff (or zero if not wanted) */
                 void *fileidbuff;		/* if name == NULL, file is opened by this file */
						/*            else, where to return resultant fileid */
               } OZ_IO_fs_open;

/* Close a file open on the channel */

#define OZ_IO_FS_CLOSE OZ_IO_DN(OZ_IO_FS_BASE,6)

typedef struct { uLong numitems;		// number of elements in itemlist array
                 const OZ_Itmlst2 *itemlist;	// file attributes to write out to file on close
               } OZ_IO_fs_close;

/* Enter alias name */

#define OZ_IO_FS_ENTER OZ_IO_DN(OZ_IO_FS_BASE,7)

typedef struct { const char *name;
                 uLong rnamesize;		/* resultant name buffer size */
                 char *rnamebuff;		/* resultant name buffer address */
                 OZ_FS_Subs *rnamesubs;		/* resultant name substring sizes */
                 int newversion;		/* force this to have highest version number */
               } OZ_IO_fs_enter;

/* Remove name, possibly deleting file */

#define OZ_IO_FS_REMOVE OZ_IO_DN(OZ_IO_FS_BASE,8)

typedef struct { const char *name;
                 uLong rnamesize;		/* resultant name buffer size */
                 char *rnamebuff;		/* resultant name buffer address */
                 OZ_FS_Subs *rnamesubs;		/* resultant name substring sizes */
               } OZ_IO_fs_remove;

/* Rename file */

#define OZ_IO_FS_RENAME OZ_IO_DN(OZ_IO_FS_BASE,9)

typedef struct { const char *oldname;
                 const char *newname;
                 uLong oldrnamesize;		/* old resultant name buffer size */
                 char *oldrnamebuff;		/* old resultant name buffer address */
                 uLong newrnamesize;		/* new resultant name buffer size */
                 char *newrnamebuff;		/* new resultant name buffer address */
                 OZ_FS_Subs *oldrnamesubs;	/* resultant name substring sizes */
                 OZ_FS_Subs *newrnamesubs;	/* resultant name substring sizes */
                 int newversion;		/* force this to have highest version number */
               } OZ_IO_fs_rename;

/* Extend file */

#define OZ_IO_FS_EXTEND OZ_IO_DW(OZ_IO_FS_BASE,10)

typedef struct { OZ_Dbn nblocks;		/* new total number of blocks to have */
                 uLong extflags;
                 OZ_Dbn eofblock;		/* set eof block (0 to leave as is) */
                 uLong eofbyte;			/* set eof byte (iff eofblock != 0) */
               } OZ_IO_fs_extend;

/* Write blocks - is not influenced by nor does it influence the current or eof position                          */
/*                end-of-file is the end of the allocated area                                                    */
/*                status returns: OZ_SUCCESS : the entire amount of data was written                              */
/*                              OZ_ENDOFFILE : some data was not written because it went off end of allcated area */
/*                                      else : some data was not written because of some I/O error                */

#define OZ_IO_FS_WRITEBLOCKS OZ_IO_DW(OZ_IO_FS_BASE,11)

typedef struct { uLong size;			/* size (in bytes) to write */
                 const void *buff;		/* buffer address */
                 OZ_Dbn svbn;			/* starting virtual block number */
                 uLong offs;			/* byte offset in first block to start writing at */
                 int writethru;			/* write blocks thru cache to magnetic media */
                 int ix4kbuk;
               } OZ_IO_fs_writeblocks;

/* Read blocks - is not influenced by nor does it influence the current or eof position                       */
/*               end-of-file is the end of the allocated area                                                 */
/*               status returns: OZ_SUCCESS : the entire amount of data was read                              */
/*                             OZ_ENDOFFILE : some data was not read because it went off end of allcated area */
/*                                     else : some data was not read because of some I/O error                */

#define OZ_IO_FS_READBLOCKS OZ_IO_DR(OZ_IO_FS_BASE,12)

typedef struct { uLong size;			/* size (in bytes) to read */
                 void *buff;			/* buffer address */
                 OZ_Dbn svbn;			/* starting virtual block number */
                 uLong offs;			/* byte offset in first block to start reading at */
                 int ix4kbuk;
               } OZ_IO_fs_readblocks;

/* Write record to current position - extends file if necessary and adjusts eof position to include the record */

#define OZ_IO_FS_WRITEREC OZ_IO_DW(OZ_IO_FS_BASE,13)

typedef struct { uLong size;			/* data buffer size */
                 const void *buff;		/* data buffer address */
                 uLong trmsize;			/* terminator buffer size */
                 const void *trmbuff;		/* terminator buffer address */
                 int append;			/* position to end-of-file before writing */
                 int truncate;			/* set end-of-file position immediately following data written */
                 uLong *wlen;			/* where to return length actually written */
						/* (doesn't include terminator) */
                 OZ_Dbn atblock;		/* position to this block before writing (if non-zero) */
                 uLong atbyte;			/* position to this byte before writing (if .atblock non-zero) */
                 int writethru;			/* write blocks thru cache to magnetic media */
               } OZ_IO_fs_writerec;

/* Read record -                                                              */
/* - from current position                                                    */
/* - stops at eof point                                                       */
/* - with terminator:                                                         */
/*   - status returns: OZ_SUCCESS : terminator was found                      */
/*                                  *rlen = number of bytes up to terminator  */
/*                OZ_NOTERMINATOR : terminator was not found                  */
/*                                  *rlen = size (ie, whole buffer filled)    */
/*                   OZ_ENDOFFILE : end of file hit                           */
/*                                  *rlen = number of bytes up to end-of-file */
/* - with no terminator:                                                      */
/*   - status returns: OZ_SUCCESS : terminator was found                      */
/*                                  *rlen = size (ie, whole buffer filled)    */
/*                   OZ_ENDOFFILE : end of file hit                           */
/*                                  *rlen = number of bytes up to end-of-file */

#define OZ_IO_FS_READREC OZ_IO_DR(OZ_IO_FS_BASE,14)

typedef struct { uLong size;			/* data buffer size */
                 void *buff;			/* data buffer address */
                 uLong trmsize;			/* terminator buffer size */
                 const void *trmbuff;		/* terminator buffer address */
                 uLong *rlen;			/* where to return length actually read */
						/* (doesn't include terminator) */
                 uLong pmtsize;			/* prompt string size */
                 const void *pmtbuff;		/* prompt buffer address */
                 OZ_Dbn atblock;		/* position to this block before reading (if non-zero) */
                 uLong atbyte;			/* position to this byte before reading (if .atblock non-zero) */
               } OZ_IO_fs_readrec;

/* Write physical pages out to section fs (can only be called from kernel mode) */
/* Same status as OZ_IO_FS_WRITEBLOCKS                                          */

#define OZ_IO_FS_PAGEWRITE OZ_IO_DW(OZ_IO_FS_BASE,15)

typedef struct { OZ_Dbn startblock;		/* starting block number */
                 OZ_Mempage pagecount;		/* number of pages to write */
                 const OZ_Mempage *pagearray;	/* array of physical page numbers */
                 int writethru;			/* write blocks thru cache to magnetic media */
               } OZ_IO_fs_pagewrite;

/* Read physical pages in from section fs (can only be called from kernel mode) */
/* Same status as OZ_IO_FS_READBLOCKS                                           */

#define OZ_IO_FS_PAGEREAD OZ_IO_DR(OZ_IO_FS_BASE,16)

typedef struct { OZ_Dbn startblock;		/* starting block number */
                 OZ_Mempage pagecount;		/* number of pages to read */
                 const OZ_Mempage *pagearray;	/* array of physical page numbers */
               } OZ_IO_fs_pageread;

/* Get file info, part 1 */

#define OZ_IO_FS_GETINFO1 OZ_IO_DN(OZ_IO_FS_BASE,17)

struct OZ_IO_fs_getinfo1 { uLong blocksize;		/* number of bytes in a block */
                           OZ_Dbn eofblock;		/* end of file block number */
                           uLong eofbyte;		/* end of file byte number */
                           OZ_Dbn hiblock;		/* number of allocated blocks */
                           OZ_Dbn curblock;		/* current block number */
                           uLong curbyte;		/* current byte number */
                           uLong filattrflags;		/* file attribute flags */
                           OZ_Datebin create_date;	/* date the file was created */
                           OZ_Datebin access_date;	/* date the file was last accessed (set by readvirtblock, writevirtblock, writefilattr) */
                           OZ_Datebin change_date;	/* date the attributes or the data were last changed (set by writevirtblock, writefilattr) */
                           OZ_Datebin modify_date;	/* date the data was last changed (set by writevirtblock only) */
                           OZ_Datebin expire_date;	/* date the file will expire (external archiving system use) */
                           OZ_Datebin backup_date;	/* date the file was last backed up (external backup system use) */
                           OZ_Datebin archive_date;	/* date the file was archived (external archiving system use) */
                           uLong secattrsize;		/* size (in bytes) of file's security attributes */
                           uLong fileidsize;		/* size of fileidbuff */
                           void *fileidbuff;		/* where to return fileid */
							// callable from kernel mode only -
                           uLong (*knlpfmap) (OZ_Iochan *iochan, OZ_Dbn vbn, OZ_Mempage *phypage_r);	// get pointer to cache page
                           uLong (*knlpfupd) (OZ_Iochan *iochan, OZ_Dbn vbn, OZ_Mempage phypage);	// write cache page updates
                           void (*knlpfrel) (OZ_Iochan *iochan, OZ_Mempage phypage);			// release cache page
                         };

/* Read directory entry */

#define OZ_IO_FS_READDIR OZ_IO_DR(OZ_IO_FS_BASE,18)

typedef struct { uLong filenamsize;		/* size of filenambuff */
                 char *filenambuff;		/* where to return filename that is read from directory */
                 uLong fileidsize;		/* size of fileidbuff */
                 void *fileidbuff;		/* where to return fileid that is read from directory */
                 OZ_FS_Subs *filenamsubs;	/* resultant name substring sizes */
               } OZ_IO_fs_readdir;

/* Get security attributes */

#define OZ_IO_FS_GETSECATTR OZ_IO_DN(OZ_IO_FS_BASE,19)

typedef struct { uLong size;			/* size of the buffer */
                 void *buff;			/* address of the buffer */
                 uLong *rlen;			/* length actually return in buffer */
               } OZ_IO_fs_getsecattr;

/* Write boot block from file open on channel */

#define OZ_IO_FS_WRITEBOOT OZ_IO_DW(OZ_IO_FS_BASE,20)

typedef struct { uLong secpertrk;		/* override disk sectors/track */
                 uLong trkpercyl;		/* override disk tracks/cylinder */
               } OZ_IO_fs_writeboot;

/* Set readrec/writerec current position */

#define OZ_IO_FS_SETCURPOS OZ_IO_DN(OZ_IO_FS_BASE,21)

typedef struct { OZ_Dbn atblock;		/* virtual block number */
                 uLong atbyte;			/* byte number within that block */
               } OZ_IO_fs_setcurpos;

/* Perform wildcard scan */

#define OZ_IO_FS_WILDSCAN OZ_IO_DR(OZ_IO_FS_BASE,22)

typedef struct { const char *wild;		/* wildcard spec to scan */
                 int init;			/* 0 : continue from where left off; 1 : initialize processing */
                 uLong size;			/* size of 'buff' */
                 char *buff;			/* buffer to return instance in */
                 uLong fileidsize;		/* size of fileidbuff */
                 void *fileidbuff;		/* where to return fileid that is read from directory */
                 uLong wildsize;		/* size of 'wildbuff' */
                 char *wildbuff;		/* where to return parsed wildcard spec */
                 OZ_FS_Subs *subs;		/* instance name substring sizes */
                 OZ_FS_Subs *wildsubs;		/* wildcard spec substring sizes */
                 int delaydir;			/* 0: output dir name before dir contents */
						/* 1: output dir name after dir contents */
                 int dirlist;			/* 0: normal scan */
						/* 1: if just a dir name given, list out whole dir contents */
               } OZ_IO_fs_wildscan;

/* Get file info, part 2 */

#define OZ_IO_FS_GETINFO2 OZ_IO_DN(OZ_IO_FS_BASE,23)

struct OZ_IO_fs_getinfo2 { uLong filnamsize;	/* size of filnambuff */
                           char *filnambuff;	/* pointer to where to return filename string */
                           OZ_FS_Subs *filnamsubs; /* resultant name substring sizes */
                         };

/* Get file info, part 3 (no file need be open) */

#define OZ_IO_FS_GETINFO3 OZ_IO_DN(OZ_IO_FS_BASE,24)

struct OZ_IO_fs_getinfo3 { uLong blocksize;	/* number of bytes per block */
                           uLong clusterfactor;	/* number of blocks per cluster */
                           OZ_Dbn clustersfree;	/* number of free clusters */
                           OZ_Dbn clustertotal;	/* number of total clusters */
                           uLong undersize;	/* size of underbuff */
                           char *underbuff;	/* where to return underlying device name */
                           uLong nincache;	/* number of pages in cache */
                           uLong ndirties;	/* number of dirty pages in cache */
                           uLong fileidsize;	/* size of file-id blocks */
                           uLong fileidstrsz;	/* max size of file-id string (incl null) */
                           void (*fidtoa) (const void *fileid, int size, char *buff);
                           int (*atofid) (const char *buff, void *fileid);
                           OZ_Datebin dirty_interval;
                           uLong mountflags;	/* flags the volume was mounted with */
                           uLong avgwriterate;	/* average cache write rate */
                           int versions;	/* filesystem files have versions */
                         };

/* Get ready for system shutdown (flush caches, etc) (kernel mode only) */

#define OZ_IO_FS_SHUTDOWN OZ_IO_DN(OZ_IO_FS_BASE,25)

/* Set open file as the crash dump file (kernel mode only) */

#define OZ_IO_FS_CRASH OZ_IO_DW(OZ_IO_FS_BASE,26)

typedef struct { uLong (*crashentry) (void *crashparam, 	/* entrypoint to call to write blocks */
                                      OZ_Dbn vbn, 		/* - vbn in crash file to start writing to */
                                      uLong size, 		/* - number of bytes to write (multiple of block size) */
                                      OZ_Mempage phypage, 	/* - physical page to start writing from */
                                      uLong offset);		/* - offset in first physical page to start at */
                 void *crashparam;				/* param to pass to crashentry routine */
                 uLong blocksize;				/* disk block size */
                 OZ_Dbn filesize;				/* number of blocks in the file */
               } OZ_IO_fs_crash;

/* Parse name string */

#define OZ_IO_FS_PARSE OZ_IO_DN(OZ_IO_FS_BASE,27)

typedef struct { const char *name;		/* null terminated fsname to parse */
                 uLong rnamesize;		/* resultant name buffer size */
                 char *rnamebuff;		/* resultant name buffer address */
                 OZ_FS_Subs *rnamesubs;		/* resultant name substring sizes */
               } OZ_IO_fs_parse;

/* Get file attributes */

#define OZ_IO_FS_GETATTR OZ_IO_DR(OZ_IO_FS_BASE,28)

#define OZ_FSATTR_CREATE_DATE (1)
#define OZ_FSATTR_ACCESS_DATE (2)
#define OZ_FSATTR_CHANGE_DATE (3)
#define OZ_FSATTR_MODIFY_DATE (4)
#define OZ_FSATTR_EXPIRE_DATE (5)
#define OZ_FSATTR_BACKUP_DATE (6)
#define OZ_FSATTR_ARCHIVE_DATE (7)
#define OZ_FSATTR_EOFBLOCK (8)
#define OZ_FSATTR_EOFBYTE (9)
#define OZ_FSATTR_FILATTRFLAGS (10)
#define OZ_FSATTR_SECATTR (11)

typedef struct { uLong numitems;
                 const OZ_Itmlst2 *itemlist;
               } OZ_IO_fs_getattr;

/* Set file attributes */

#define OZ_IO_FS_SETATTR OZ_IO_DW(OZ_IO_FS_BASE,29)

typedef struct { uLong numitems;
                 const OZ_Itmlst2 *itemlist;
               } OZ_IO_fs_setattr;

/* Verify volume */

#define OZ_IO_FS_VERIFYVOL OZ_IO_DW(OZ_IO_FS_BASE,30)

typedef struct { int readonly;		/* 0: report and fix errors; 1: just report errors, don't fix */
               } OZ_IO_fs_verifyvol;

#endif
