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

#ifndef _OZ_DEV_VDFS_H
#define _OZ_DEV_VDFS_H

#if OZ_VDFS_VERSION != 4
  error : caller is version OZ_VDFS_VERSION but I am version 4
#endif

#include "oz_io_disk.h"
#include "oz_io_fs.h"
#include "oz_knl_dcache.h"
#include "oz_knl_devio.h"
#include "oz_knl_shuthand.h"

#include <stdarg.h>

typedef struct OZ_VDFS_Chnex    OZ_VDFS_Chnex;
typedef struct OZ_VDFS_Devex    OZ_VDFS_Devex;
typedef struct OZ_VDFS_File     OZ_VDFS_File;
typedef struct OZ_VDFS_Fileid   OZ_VDFS_Fileid;
typedef struct OZ_VDFS_Iopex    OZ_VDFS_Iopex;
typedef struct OZ_VDFS_Vector   OZ_VDFS_Vector;
typedef struct OZ_VDFS_Volume   OZ_VDFS_Volume;
typedef struct OZ_VDFS_Wildscan OZ_VDFS_Wildscan;

#ifdef _OZ_DEV_VDFS_C
typedef void OZ_VDFS_Filex;
typedef void OZ_VDFS_Volex;
#else
typedef struct OZ_VDFS_Filex OZ_VDFS_Filex;
typedef struct OZ_VDFS_Volex OZ_VDFS_Volex;
#endif

#define OZ_VDFS_ALF_M_EOF 0x01	// efblk/efbyt needs to be updated from attrlock_efblk/efbyt
#define OZ_VDFS_ALF_M_MDT 0x02	// modify_date needs to be updated from attrlock_date
#define OZ_VDFS_ALF_M_CDT 0x04	// change_date needs to be updated from attrlock_date
#define OZ_VDFS_ALF_M_ADT 0x08	// access_date needs to be updated from attrlock_date

struct OZ_VDFS_File { OZ_VDFS_File *next, **prev;	/* next file in openfiles list (last in list is NULL) */
                      OZ_VDFS_File *nextdirty;		/* pointer to next dirty file block on volumes dirtyfiles list */
                      OZ_VDFS_Volume *volume;		/* volume it is on */
                      OZ_Secattr *secattr;		/* file's security attributes */
                      uLong refcount;			/* number of create_file, open_by_... minus number of close_file */
                      uLong refc_read;			/* number of channel locks that allow read access to the file */
                      uLong refc_write;			/* number of channel locks that allow write access to the file */
                      uLong deny_read;			/* number of channel locks that deny read access to the file */
                      uLong deny_write;			/* number of channel locks that deny write access to the file */

                      OZ_VDFS_Iopex *recio_qh;		/* queue of record I/O (readrec/writerec) requests for file */
                      OZ_VDFS_Iopex **recio_qt;
                      OZ_VDFS_Iopex *recio_ip;		/* recio request in progress */
                      OZ_Smplock recio_vl;		/* smp lock for recio_qh/_qt/_ip, also used by wildscans list */

                      OZ_Smplock attrlock_vl;		/* set to interlock modifications of attrlock_* */
                      Long attrlock_flags;		/* list of attributes to be modified */
                      OZ_Dbn attrlock_efblk;		/* end-of-file pointer to be written to header */
                      uLong attrlock_efbyt;
                      OZ_Datebin attrlock_date;		/* date/time to be written to header by attrlock_flags */

                      int truncpend;			/* set if a truncate operation is pending */
                      OZ_Dbn truncblocks;		/* what block number to truncate to */
                      OZ_Dbn allocblocks;		/* actual number of blocks currently allocated to the file */

                      OZ_Event *dirlockf;		/* event flag used to synchronize directory access */
                      volatile Long dirlockr;		/* number of threads that are reading the directory */
							/* top bit (31) set if directory is being written */

                      OZ_VDFS_Wildscan *wildscans;	/* list of wildscan contexts for this file (sync using recio_vl) */
                      int headerdirty;			/* header needs to be written out */

                      OZ_VDFS_Filex *filex;		/* fs specific data */
                    };

/* In-memory volume info */

struct OZ_VDFS_Volume { OZ_VDFS_File *openfiles;		/* list of open files on the device */
                        uLong nopenfiles;			/* number of open files (number of files on 'openfiles' list) */
                        uLong mountflags;			/* flags it was mounted with */
                        OZ_VDFS_Devex *devex;			/* device it is mounted on */
                        OZ_Smplock dirtyfiles_vl;		/* smplock for dirtyfiles list */
                        OZ_VDFS_File *dirtyfiles;		/* list of file headers that need to be written to disk */
                        int dirty;				/* home block needs to be written */
                        OZ_Dbn bb_nblocks;			/* number of blocks in the boot block */
                        OZ_Dbn bb_logblock;			/* starting logical block number of the boot block */
                        OZ_Dbn hb_logblock;			/* logical block number of the home block */
                        uLong dirblocksize;			/* directory block size (used by wildscan to read directory) */
                        OZ_Dbn clusterfactor;			/* cluster factor */

                        OZ_VDFS_Volex *volex;			/* fs specific data */
                      };

/* In-memory wildcard searching routine context */

struct OZ_VDFS_Wildscan { OZ_VDFS_Wildscan *nextouter;			// pointer to next outer level OZ_VDFS_Wildscan (or NULL if this is outermost)
                          OZ_VDFS_Wildscan *hastilde;			// NULL : no outerlevel was matched by a tilde
									// else : outerlevel that was matched by a tilde
                          OZ_VDFS_Wildscan *next_dirfile;		// next wildscan for the directory file (dirfile -> wildscans list)
                          OZ_VDFS_Wildscan **prev_dirfile;		// prev wildscan for the directory file (dirfile -> wildscans list)
                          char wildspec[OZ_FS_MAXFNLEN];		// wildcard being scanned for within this directory 
									// ... (without leading / or trailing ;version)
                          char basespec[OZ_FS_MAXFNLEN];		// name of this directory
                          char lastname[OZ_FS_MAXFNLEN];		// last filename scanned in directory
                          int ver_incldirs;				// set if directories are to be included in scan
                          int ver_inclallfiles;				// set if all files are to be included in scan
                          int ver_number;				// = 0 : just process latest version of files
									// > 0 : just process this exact version number
									// < 0 : just process the nth oldest version
                          int ver_count;				// used as a counter for neg version processing
                          int ver_output;				// set if blockoffs is pointing to a dirpnt (else is points to filename)
                          OZ_Iochan *iochan;				// I/O channel with the directory file opened on it
                          int rtndirnametocaller;			// set if we have yet to return directory name to caller
                          uLong blockoffs;				// offset in blockbuff of next entry to process
                          OZ_Dbn blockvbn;				// vbn of block in blockbuff
                          void *wildx;					// fs specific data
                          char blockbuff[1];				// directory block buffer (actual size is volume -> dirblocksize)
                        };

/* Device extension structure */

struct OZ_VDFS_Devex { OZ_Devunit *devunit;			/* corresponding devunit */
                       int shutdown;				/* device is being shut down - don't process any more requests */
                       OZ_Smplock smplock_vl;			/* smp lock for iopexqh/iopexqt */
                       const OZ_VDFS_Vector *vector;		/* entrypoint table for fs-specific routines */
                       OZ_Devdriver *devdriver;			/* device driver pointer */
                       const char *drivername;			/* driver name string pointer */
                       Long clonumber;				/* clone device number (template dev only) */
                       OZ_Event *event;				/* event flag to set when queuing something */
                       OZ_Event *ioevent;			/* event flag used for internal I/O requests */
                       OZ_VDFS_Iopex *iopexqh, **iopexqt;	/* list of i/o requests */
                       OZ_Iochan *volatile master_iochan;	/* I/O channel assigned to disk drive */
                       OZ_Dcache *dcache;			/* disk cache context pointer */
                       uLong blocksize;				/* disk's block size */
                       uLong bufalign;				/* buffer alignment required by disk driver */
                       OZ_Dbn totalblocks;			/* total blocks on disk */
                       OZ_VDFS_Volume *volume;			/* volume block */
                       OZ_Shuthand *shuthand;			/* pointer to shutdown handler */
                       OZ_Event *shortcutev;			/* event flag to set when decrementing shortcuts and shortcuts < 0 */
                     };

/* Channel extension structure */

struct OZ_VDFS_Chnex { OZ_Iochan *iochan;			// pointer to corresponding driver-independent channel struct
                       OZ_VDFS_File *file;			/* file that is open on the channel */
                       OZ_Lockmode lockmode;			/* lock mode the file was opened with */
                       int ignclose;				/* ignore OZ_IO_FS_CLOSE calls - only close when deassigned */
                       OZ_VDFS_Chnex *scassochnex;		/* NULL : normal channel */
								/* else : this channel is being used to process a shortcut function */

                       OZ_VDFS_Wildscan *wildscan;		// any wild card context active on channel
                       OZ_VDFS_Iopex *wild_iopex;		// wildcard scanning iopex currently active
                       OZ_VDFS_File *wild_dirlocked;		// directory file struct that is currently locked
                       void (*wild_lockentry) (OZ_VDFS_Chnex *chnex); // callback when directory successfully locked
                       int wild_nested;				// call nesting level
                       OZ_Lowipl *wild_lowipl;			// lowipl struct used to reset nesting level

                       volatile Long shortcuts;			/*  0 : no shortcut I/O's in progress */
								/* >0 : number of shortcut I/O's in progress, don't allow channel to close */
								/* <0 : channel being closed, don't allow *ANY* I/O's to start or queue up */
#if 111
                       volatile Long closeshorts;		// what 'shortcuts' was when it was set to -1 by be_close routine
                       OZ_VDFS_Iopex *shortcut_iopexs;		// debugging: list of shortcut iopex's
#endif

                       OZ_Dbn cur_blk;				/* record I/O current position */
                       uLong cur_byt;
                     };

/* I/O Operation extension structure */

struct OZ_VDFS_Iopex { OZ_Ioop *ioop;				/* i/o operation struct pointer */
                       OZ_VDFS_Devex *devex;			/* corresponding devex struct pointer */
                       OZ_VDFS_Chnex *chnex;			/* corresponding chnex struct pointer */
                       OZ_VDFS_Iopex *next;			/* next in devex -> iopexqh/iopexqt */
                       OZ_Procmode procmode;			/* requestor's processor mode */
                       uLong funcode;				/* i/o function code */
                       uLong (*backend) (OZ_VDFS_Iopex *iopex, OZ_VDFS_Chnex *chnex, OZ_VDFS_Devex *devex);
#if 111
                       OZ_VDFS_Iopex *shortcut_next;		// debugging: next in chnex -> shortcut_iopexs' list
                       OZ_VDFS_Iopex **shortcut_prev;		// debugging: prev in chnex -> shortcut_iopexs' list
#endif
                       uLong as;				/* argument block size */
                       void *ap;				/* argument block pointer */
                       int aborted;				// flagged for abort
                       union { struct { OZ_IO_fs_initvol p; } initvol;
                               struct { OZ_IO_fs_mountvol p; } mountvol;
                               struct { OZ_IO_fs_dismount p; } dismount;
                               struct { OZ_IO_fs_create p; } create;
                               struct { OZ_IO_fs_open p; } open;
                               struct { OZ_IO_fs_close p; } close;
                               struct { OZ_IO_fs_enter p; } enter;
                               struct { OZ_IO_fs_remove p; } remove;
                               struct { OZ_IO_fs_rename p; } rename;
                               struct { OZ_IO_fs_extend p; } extend;
                               struct { OZ_IO_fs_writeblocks p;		// request parameter block
                                        const OZ_Mempage *phypages;	// buffer physical page array
                                        uLong phyoffs;			// offset in first physical page
                                        uLong status;			// completion status
                                        OZ_Dcmpb dcmpb;			// oz_knl_dcache_map parameter block
                                      } writeblocks;
                               struct { OZ_IO_fs_readblocks p;		// request parameter block
                                        const OZ_Mempage *phypages;	// buffer physical page array
                                        uLong phyoffs;			// offset in first physical page
                                        uLong status;			// completion status
                                        OZ_Dcmpb dcmpb;			// oz_knl_dcache_map parameter block
                                        void *temp;			// noncache read - temp block buffer
                                        OZ_Dbn nblocks;			//                 number of blocks read this time
                                      } readblocks;
                               struct { OZ_IO_fs_writerec p;
                                        const OZ_Mempage *phypages;	// buffer physical page array
                                        uLong byteoffs;			// offset in first physical page
                                        const OZ_Mempage *wlen_phypages; // wlen physical page array
                                        uLong wlen_byteoffs;		// offset in first physical page
                                        OZ_Dbn efblk;			// end-of-file block number
                                        uLong efbyt;			// end-of-file byte number
                                        uLong wlen;			// length written out so far
                                        uLong trmwlen;			// number of termintor bytes written out so far
                                        const OZ_Mempage *trmphypages;	// terminator buffer phypage array
                                        uLong trmbyteoffs;		// terminator buffer phypage offset
                                        int updateof;			// eof position needs to be updated on completion
                                        uByte trmdata[4];		// terminator buffer for short buffers
                                        uLong status;			// completion status
                                        OZ_Dcmpb dcmpb;			// oz_knl_dcache_map parameter block
                                      } writerec;
                               struct { OZ_IO_fs_readrec p;		// request parameter block
                                        const OZ_Mempage *phypages;	// buffer physical page array
                                        uLong byteoffs;			// offset in first physical page
                                        const OZ_Mempage *rlen_phypages; // rlen physical page array
                                        uLong rlen_byteoffs;		// offset in first physical page
                                        OZ_Dbn efblk;			// end-of-file block number
                                        uLong efbyt;			// end-of-file byte number
                                        uLong rlen;			// length read in so far
                                        uLong trmseen;			// number of termintor bytes seen so far
                                        uByte *trmbuff;			// copy of terminator buffer
                                        uByte trmdata[4];		// terminator buffer for short buffers
                                        uLong status;			// completion status
                                        OZ_Dcmpb dcmpb;			// oz_knl_dcache_map parameter block
                                      } readrec;
                               struct { OZ_IO_fs_pagewrite p; } pagewrite;
                               struct { OZ_IO_fs_pageread p; } pageread;
                               struct { OZ_IO_fs_getinfo1 p; } getinfo1;
                               struct { OZ_IO_fs_readdir p; } readdir;
                               struct { OZ_IO_fs_getsecattr p; } getsecattr;
                               struct { OZ_IO_fs_writeboot p; } writeboot;
                               struct { OZ_IO_fs_setcurpos p; } setcurpos;
                               struct { OZ_IO_fs_wildscan p; } wildscan;
                               struct { OZ_IO_fs_getinfo2 p; } getinfo2;
                               struct { OZ_IO_fs_getinfo3 p; } getinfo3;
                               struct { OZ_IO_fs_parse p; } parse;
                               struct { OZ_IO_fs_verifyvol p; } verifyvol;
                             } u;
                     };

struct OZ_VDFS_Vector {
	int fileid_size;				// zero if fileid can't be passed to OZ_IO_FS_OPEN_FILE calls
							// else sizeof filesystem's actual fileid struct
	int volname_max, filename_max, secattr_max;	// max sizes of various things
	int versions;					// zero if fs doesn't do file versions, one if fs does

	// These routines are only called in kernel thread context
	// Thus there can only be one of them active on a volume at a time

	uLong (*close_file) (OZ_VDFS_File *file, OZ_VDFS_Iopex *iopex);
	uLong (*create_file) (OZ_VDFS_Volume *volume, int namelen, const char *name, uLong filattrflags, OZ_VDFS_Fileid *dirid, OZ_VDFS_File *file, OZ_VDFS_Fileid **fileid_r, OZ_VDFS_Iopex *iopex);
	uLong (*dismount_volume) (OZ_VDFS_Devex *devex, OZ_VDFS_Volume *volume, int unload, int shutdown, OZ_VDFS_Iopex *iopex);
	uLong (*enter_file) (OZ_VDFS_File *dirfile, const char *dirname, int namelen, const char *name, int newversion, OZ_VDFS_File *file, const OZ_VDFS_Fileid *fileid, char *name_r, OZ_VDFS_Iopex *iopex);
	uLong (*extend_file) (OZ_VDFS_File *file, OZ_Dbn nblocks, uLong extflags, OZ_VDFS_Iopex *iopex);
	OZ_VDFS_File *(*findopenfile) (OZ_VDFS_Volume *volume, const OZ_VDFS_Fileid *fileid);
	uLong (*get_rootdirid) (OZ_VDFS_Devex *devex, OZ_VDFS_Fileid *rootdirid_r);
	const char *(*get_volname) (OZ_VDFS_Volume *volume);
	uLong (*getinfo2) (OZ_VDFS_Iopex *iopex);
	uLong (*init_volume) (OZ_VDFS_Devex *devex, OZ_VDFS_Volume *volume, int volnamelen, const char *volname, uLong clusterfactor, uLong secattrsize, const void *secattrbuff, uLong initflags, OZ_VDFS_Iopex *iopex);
	uLong (*lookup_file) (OZ_VDFS_File *dirfile, int namelen, const char *name, OZ_VDFS_Fileid *fileid_r, char *name_r, OZ_VDFS_Iopex *iopex);
	uLong (*mount_volume) (OZ_VDFS_Devex *devex, OZ_VDFS_Volume *volume, uLong mountflags, OZ_VDFS_Iopex *iopex);
	uLong (*open_file) (OZ_VDFS_Volume *volume, const OZ_VDFS_Fileid *fileid, OZ_VDFS_File *file, OZ_VDFS_Iopex *iopex);
	uLong (*remove_file) (OZ_VDFS_File *dirfile, const char *name, char *name_r, OZ_VDFS_Iopex *iopex);
	uLong (*set_file_attrs) (OZ_VDFS_File *file, uLong numitems, const OZ_Itmlst2 *itemlist, OZ_VDFS_Iopex *iopex);
	uLong (*write_dirty_header) (OZ_VDFS_File *dirtyfile, Long alf, OZ_Datebin now, OZ_VDFS_Volume *volume, OZ_VDFS_Iopex *iopex);
	uLong (*writehomeblock) (OZ_VDFS_Volume *volume, OZ_VDFS_Iopex *iopex);
	uLong (*verifyvol) (OZ_VDFS_Iopex *iopex, OZ_VDFS_Devex *devex);

	// These routines can be called either from the kernel thread or from a shortcut
	// Thus there may be more than one of them being called on a volume at a time

	int (*fis_writethru) (OZ_VDFS_File *file, OZ_Dbn virtblock, uLong blockoffs, uLong size, const void *buff);
	const OZ_VDFS_Fileid *(*get_fileid) (OZ_VDFS_File *file);
	uLong (*getinfo1) (OZ_VDFS_Iopex *iopex);
	uLong (*getinfo3) (OZ_VDFS_Iopex *iopex);
	int (*is_directory) (OZ_VDFS_File *file);
	uLong (*map_vbn_to_lbn) (OZ_VDFS_File *file, OZ_Dbn virtblock, OZ_Dbn *nblocks_r, OZ_Dbn *logblock_r);
	void (*mark_header_dirty) (OZ_VDFS_File *dirtyfile);
	void (*returnspec) (char *spec, uLong size, char *buff, OZ_FS_Subs *subs);
	int (*vis_writethru) (OZ_VDFS_Volume *volume);
	void (*wildscan_continue) (OZ_VDFS_Chnex *chnex);
	void (*wildscan_terminate) (OZ_VDFS_Chnex *chnex);
};

void oz_dev_vdfs_init (int version, const char *drivername, const OZ_VDFS_Vector *vector);
void oz_dev_vdfs_wildscan_readdir (OZ_VDFS_Chnex *chnex);
void oz_dev_vdfs_wildscan_direof (OZ_VDFS_Chnex *chnex);
void oz_dev_vdfs_wildscan_startsubdir (OZ_VDFS_Chnex *chnex, const char *name, OZ_VDFS_Fileid *fileid, int wildscan_match_rc);
int oz_dev_vdfs_wildscan_match (OZ_VDFS_Wildscan *wildscan, const char *filename);
void oz_dev_vdfs_wildscan_output (OZ_VDFS_Chnex *chnex, char *filename, uLong version, const OZ_VDFS_Fileid *fileid);
void oz_dev_vdfs_wildscan_iodonex (OZ_VDFS_Iopex *iopex, uLong status);
void oz_dev_vdfs_lock_dirfile_for_write (OZ_VDFS_File *dirfile);
void oz_dev_vdfs_unlk_dirfile_for_write (OZ_VDFS_File *dirfile);
uLong oz_dev_vdfs_open_file (OZ_VDFS_Volume *volume, const OZ_VDFS_Fileid *fileid, OZ_Secaccmsk secaccmsk, OZ_VDFS_File **file_r, OZ_VDFS_Iopex *iopex);
uLong oz_dev_vdfs_close_file (OZ_VDFS_File *file, OZ_VDFS_Iopex *iopex);
void oz_dev_vdfs_mark_header_dirty (OZ_VDFS_File *file);
void oz_dev_vdfs_write_dirty_headers (OZ_VDFS_Volume *volume, OZ_VDFS_Iopex *iopex);
uLong oz_dev_vdfs_readvirtblock (OZ_VDFS_File *file, OZ_Dbn virtblock, uLong blockoffs, uLong size, void *buff, OZ_VDFS_Iopex *iopex, int updates);
uLong oz_dev_vdfs_readlogblock (OZ_Dbn logblock, uLong blockoffs, uLong size, void *buff, OZ_VDFS_Iopex *iopex);
uLong oz_dev_vdfs_writevirtblock (OZ_VDFS_File *file, OZ_Dbn virtblock, uLong blockoffs, uLong size, const void *buff, OZ_VDFS_Iopex *iopex, int updates);
uLong oz_dev_vdfs_writelogblock (OZ_Dbn logblock, uLong blockoffs, uLong size, const void *buff, int writethru, OZ_VDFS_Iopex *iopex);
uLong oz_dev_vdfs_dcache_map_vbn_to_lbn (OZ_Dcmpb *dcmpb, OZ_VDFS_File *file);
uLong oz_dev_vdfs_dcache_map_blocks (OZ_Dcmpb *dcmpb);
void oz_dev_vdfs_printk (OZ_VDFS_Iopex *iopex, const char *format, ...);
void oz_dev_vdfs_vprintk (OZ_VDFS_Iopex *iopex, const char *format, va_list ap);

#endif
