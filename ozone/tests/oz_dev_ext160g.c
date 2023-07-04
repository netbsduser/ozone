//+++2003-03-01
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
//---2003-03-01

/************************************************************************/
/*									*/
/*  Extend the storagebitmap file to 1960 blocks and set the 		*/
/*  totalcluster count to 8004326					*/
/*									*/
/************************************************************************/

#define OZ_VDFS_VERSION 1

#include "ozone.h"
#include "oz_dev_timer.h"
#include "oz_dev_vdfs.h"
#include "oz_hw_bootblock.h"
#include "oz_io_console.h"
#include "oz_io_disk.h"
#include "oz_io_fs.h"
#include "oz_knl_dcache.h"
#include "oz_knl_devio.h"
#include "oz_knl_event.h"
#include "oz_knl_hw.h"
#include "oz_knl_kmalloc.h"
#include "oz_knl_procmode.h"
#include "oz_knl_sdata.h"
#include "oz_knl_section.h"
#include "oz_knl_security.h"
#include "oz_knl_shuthand.h"
#include "oz_knl_spte.h"
#include "oz_knl_status.h"
#include "oz_knl_thread.h"
#include "oz_knl_userjob.h"
#include "oz_sys_dateconv.h"
#include "oz_sys_xprintf.h"

#define VOLNAME_MAX 64
#define FILENAME_MAX 64
#define SECATTR_MAX 256

#define HOMEBLOCK_VERSION 1
#define HEADER_VERSION 1

#define EXTEND_NOTRUNC OZ_FS_EXTFLAG_NOTRUNC
#define EXTEND_NOEXTHDR OZ_FS_EXTFLAG_NOEXTHDR

#define EXT_DIRCOUNT ((uLong)(-1))	// header.dircount value indicating it is an extension header
					// otherwise it is a prime header

/* These filenumbers are special use and are the same for all volumes       */
/* They all have the seq == 0, so they can't be entered, removed or deleted */
/* They are all entered in the root directory of the volume                 */

/* The numbers start at 1 and count from there.  SACRED_FIDNUM_COUNT is the highest numbered (inclusive). */

#define SACRED_FIDNUM_ROOTDIRECTORY (1)
#define SACRED_FIDNUM_INDEXHEADERS  (2)
#define SACRED_FIDNUM_INDEXBITMAP   (3)
#define SACRED_FIDNUM_STORAGEBITMAP (4)
#define SACRED_FIDNUM_BOOTHOMEBLOCK (5)

#define SACRED_FIDNUM_COUNT (5)

/* Variable areas in the file header */

#define OZ_FS_HEADER_AREA_FILNAME (0)
#define OZ_FS_HEADER_AREA_FILATTR (1)
#define OZ_FS_HEADER_AREA_SECATTR (2)
#define OZ_FS_HEADER_AREA_POINTER (3)
#define OZ_FS_HEADER_NAREAS (4)

/* Point to the pointers in a file block */

#define POINTERS(__filex) (Pointer *)(__filex -> header.area + __filex -> header.areas[OZ_FS_HEADER_AREA_POINTER].offs)

/* Point to filename string in an header */

#define FILENAME(__header) (char *)((__header).area + (__header).areas[OZ_FS_HEADER_AREA_FILNAME].offs)

/* Get the end-of-file block and byte numbers from an header */

#define FILATTRS(__header) ((Filattr *)((__header).area + (__header).areas[OZ_FS_HEADER_AREA_FILATTR].offs))

/* Test the directory bit */

#define GET_FATTRFL(__header) FILATTRS (__header) -> filattrflags
#define IS_DIRECTORY(__header) ((GET_FATTRFL (__header) & OZ_FS_FILATTRFLAG_DIRECTORY) != 0)

/* Test the write-thru flag bits */

#define FIS_WRITETHRU(__header) ((GET_FATTRFL (__header) & OZ_FS_FILATTRFLAG_WRITETHRU) != 0)
#define VIS_WRITETHRU(__volume) ((__volume -> mountflags & OZ_FS_MOUNTFLAG_WRITETHRU) != 0)

/* Our File-id structure */

#define Seq 24
struct OZ_VDFS_Fileid { uLong num;		/* vbn in indexheader file */
                        unsigned seq : Seq;	/* re-use sequence number for the header block */
                        uByte rvn;		/* volume number (starting at 1) in the volume set */
                      };

/* On-disk directory pointers */

	/* Directories are made of an array of cluster-sized 'blocks'.  */
	/* The eof pointer always includes all allocated blocks.        */

        /* In the directory as a whole as well as within each block, */
	/* filenames (excluding version number) are sorted in        */
	/* ascending order by name, then descending version number.  */

	/* Each block starts with a null-terminated filename (excluding    */
	/* version nubmer).  The null-terminated filename is followed by   */
	/* an zero-terminated array of Dirpnt's, sorted by descending      */
	/* version number.  The last element of a Dirpnt array is a dummy  */
        /* with a version number of zero.  The array of Dirpnt's are       */
        /* followed by the next null-terminated filename, etc., until      */
        /* either a null filename is found or the end-of-block is reached. */

	/* Filenames do no span block boundaries.  Dirpnt arrays do not span  */
	/* block boundaries.  If a filename/dirpnt_array does not fit in a    */
	/* single directory block, the Dirpnt array is split into more than   */
	/* one array and a separate filename/dirpnt_array entry is made for   */
	/* each fragment.  Thus, a given filename may appear in the directory */
	/* more than once (but with different version numbers).               */

typedef struct { uLong version;			/* file version number */
                 OZ_VDFS_Fileid fileid;		/* corresponding file-id */
               } Dirpnt;

/* On-disk file header structure */

typedef struct { OZ_Datebin create_date;		/* date the file was created */
                 OZ_Datebin access_date;		/* date the file was last accessed (set by oz_dev_vdfs_readvirtblock, oz_dev_vdfs_writevirtblock, writefilattr) */
                 OZ_Datebin change_date;		/* date the attributes or the data were last changed (set by oz_dev_vdfs_writevirtblock, writefilattr) */
                 OZ_Datebin modify_date;		/* date the data was last changed (set by oz_dev_vdfs_writevirtblock only) */
                 OZ_Datebin expire_date;		/* date the file will expire (external archiving system use) */
                 OZ_Datebin backup_date;		/* date the file was last backed up (external backup system use) */
                 OZ_Datebin archive_date;		/* date the file was archived (external archiving system use) */
                 uLong eofblock;			/* last virtual block number that contains valid data */
                 uWord eofbyte;				/* number of bytes in the eofblock that contain valid data */
							/* this number is in range 0..blocksize-1, inclusive */
                 uWord filattrflags;			/* file attribute flags, OZ_FS_FILATTRFLAG_... */
               } Filattr;

typedef struct { OZ_Dbn blockcount;			/* number of contiguous blocks */
                 OZ_Dbn logblock;			/* starting block number */
               } Pointer;

typedef struct { uWord headerver;			/* header version */
                 uWord checksum;			/* checksum such that all words total to zero */
                 OZ_VDFS_Fileid fileid;			/* file id */
                 OZ_VDFS_Fileid extid;			/* extension id, zero if none */
                 OZ_VDFS_Fileid dirid;			/* dircount=EXT_DIRCOUNT : previous header's fileid */
							/*                  else : (original) directory fileid */
                 uLong dircount;			/* EXT_DIRCOUNT : this is an extension header */
							/*         else : number of directory entries that point to file */
                 struct { uWord size, offs; } areas[OZ_FS_HEADER_NAREAS];
                 uByte area[1];
               } Header;

/* On-disk home block */

typedef struct { uWord homeversion;			/* file system version number */
                 uWord checksum;			/* checksum such that all uWords in homeblock total to zero */
                 char volname[VOLNAME_MAX];		/* volume name (null terminated) */
                 uLong blocksize;			/* size in bytes of disk blocks on this volume */
                 uLong clusterfactor;			/* storage bitmap cluster factor */
                 OZ_Dbn clustertotal;			/* total number of clusters on this volume */
                 OZ_Dbn clustersfree;			/* number of free clusters on this volume */
                 OZ_Dbn indexhdrlbn;			/* logical block number of index header file header */
                 OZ_Datebin lastwritemount;		/* date/time last mounted for write */
							/* set when mounted, cleared to zeroes when dismounted */
                 uLong initflags;			/* initialization flags */
               } Homeblock;

/* In-memory volume extension info */

struct OZ_VDFS_Volex { OZ_VDFS_File *indexbitmap;	/* pointer to index bitmap header in openfiles list */
                       OZ_VDFS_File *indexheaders;	/* pointer to index file header in openfiles list */
                       OZ_VDFS_File *rootdirectory;	/* pointer to root directory header in openfiles list */
                       OZ_VDFS_File *storagebitmap;	/* pointer to storage bitmap header in openfiles list */
                       uByte *dirblockbuff3;		/* address of dirblockbuff (used by enter_file and remove_file) */
                       union {				/* used by various mutually exclusive functions: */
                         struct {			// lookup_file function
                           char fname[FILENAME_MAX];	// - filename we're looking for, without ;version, but with null
                           char *name_r;		// - where to return the resultant filename, including ;version
                           Dirpnt partialdirpnt;	// - partially build dirpnt
                           volatile enum {		// - scan state
                             LOOKUP_FILE_STATE_GATHERNAME, 
                             LOOKUP_FILE_STATE_SKIPDIRPNTS, 
                             LOOKUP_FILE_STATE_MATCHVERSION, 
                             LOOKUP_FILE_STATE_DONE
                           } state;
                           OZ_VDFS_File *dirfile;	// - directory we're scanning
                           int dirpntbytesaved;		// - number of bytes of dirpnt saved in partialdirpnt
                           int level;			// - level we're at in the directory tree
                           int namelen;			// - length of fname, including the null
                           int nbytes_name_gathered;	// - number of bytes of the fname that have been gathered so far
							//   -1 if we haven't read the 'same chars' byte yet
                           int versign;			// - 0: positive version, -1: negative version
                           OZ_Dbn nclusters;		// - number of clusters in the directory
                           OZ_Dbn multiplier;		// - cluster we're currently scanning
                           OZ_Dcmpb dcmpb;		// - disk cache map parameter block
                           OZ_VDFS_Fileid *fileid_r;	// - where to return the found file-id
                           uLong version;		// - version number (abs value)
                           uLong status;		// - completion status
                           OZ_Dbn lastbuckvbn;		// - last bucket vbn we looked at
                           OZ_Dbn thisbuckvbn;		// - this bucket vbn we're looking at
                           char lastname[FILENAME_MAX];	// - last filename scanned in bucket
                         } lf;
                       } v;

                       Homeblock homeblock;		// homeblock - MUST be last
                     };

/* In-memory file extension info */

struct OZ_VDFS_Filex { OZ_VDFS_Filex *next;		// next extension file pointer
                       Long headerdirty;		// this extension header is dirty
                       OZ_Dbn blocksinhdr;		// number of blocks mapped by this header
							// (total of all its pointer blockcounts)
                       Header header;			// on-disk header contents
                     };



/* Vector routines */

static int dfs_is_directory (OZ_VDFS_File *file);
static int dfs_fis_writethru (OZ_VDFS_File *file, OZ_Dbn virtblock, uLong blockoffs, uLong size, const void *buff);
static int dfs_vis_writethru (OZ_VDFS_Volume *volume);
static const char *dfs_get_volname (OZ_VDFS_Volume *volume);
static uLong dfs_getinfo1 (OZ_VDFS_Iopex *iopex);
static void dfs_wildscan_continue (OZ_VDFS_Chnex *chnex);
static uLong dfs_getinfo2 (OZ_VDFS_Iopex *iopex);
static uLong dfs_getinfo3 (OZ_VDFS_Iopex *iopex);
static uLong dfs_init_volume (OZ_VDFS_Devex *devex, OZ_VDFS_Volume *volume, OZ_IO_disk_getinfo1 *disk_getinfo1, int volnamelen, const char *volname, uLong clusterfactor, uLong secattrsize, const void *secattrbuff, uLong initflags, OZ_VDFS_Iopex *iopex);
static uLong dfs_mount_volume (OZ_VDFS_Devex *devex, OZ_VDFS_Volume *volume, OZ_IO_disk_getinfo1 *disk_getinfo1, uLong mountflags, OZ_VDFS_Iopex *iopex);
static uLong dfs_dismount_volume (OZ_VDFS_Devex *devex, OZ_VDFS_Volume *volume, int unload, int shutdown, OZ_VDFS_Iopex *iopex);
static uLong dfs_get_rootdirid (OZ_VDFS_Devex *devex, OZ_VDFS_Fileid *rootdirid_r);
static const OZ_VDFS_Fileid *dfs_get_fileid (OZ_VDFS_File *file);
static uLong dfs_lookup_file (OZ_VDFS_File *dirfile, int namelen, const char *name, OZ_VDFS_Fileid *fileid_r, char *name_r, OZ_VDFS_Iopex *iopex);
static uLong dfs_enter_file (OZ_VDFS_File *dirfile, const char *dirname, int namelen, const char *name, int newversion, OZ_VDFS_File *file, const OZ_VDFS_Fileid *fileid, char *name_r, OZ_VDFS_Iopex *iopex);
static uLong dfs_remove_file (OZ_VDFS_File *dirfile, const char *name, char *name_r, OZ_VDFS_Iopex *iopex);
static void dfs_returnspec (char *spec, uLong size, char *buff, OZ_FS_Subs *subs);
static uLong dfs_create_file (OZ_VDFS_Volume *volume, int namelen, const char *name, uLong filattrflags, OZ_VDFS_Fileid *dirid, OZ_VDFS_File *file, OZ_VDFS_Fileid **fileid_r, OZ_VDFS_Iopex *iopex);
static OZ_VDFS_File *dfs_findopenfile (OZ_VDFS_Volume *volume, const OZ_VDFS_Fileid *fileid);
static uLong dfs_open_file (OZ_VDFS_Volume *volume, const OZ_VDFS_Fileid *fileid, OZ_VDFS_File *file, OZ_VDFS_Iopex *iopex);
static uLong dfs_set_file_attrs (OZ_VDFS_File *file, uLong numitems, const OZ_Itmlst2 *itemlist, OZ_VDFS_Iopex *iopex);
static uLong dfs_close_file (OZ_VDFS_File *file, OZ_VDFS_Iopex *iopex);
static uLong dfs_extend_file (OZ_VDFS_File *file, OZ_Dbn nblocks, uLong extflags, OZ_VDFS_Iopex *iopex);
static uLong dfs_write_dirty_header (OZ_VDFS_File *dirtyfile, Long alf, OZ_VDFS_Volume *volume, OZ_VDFS_Iopex *iopex);
static uLong dfs_writehomeblock (OZ_VDFS_Volume *volume, OZ_VDFS_Iopex *iopex);
static uLong dfs_map_vbn_to_lbn (OZ_VDFS_File *file, OZ_Dbn virtblock, OZ_Dbn *nblocks_r, OZ_Dbn *logblock_r);
static void dfs_mark_header_dirty (OZ_VDFS_File *dirtyfile);

/* Internal routines */

static void calc_home_block (OZ_VDFS_Volume *volume);
static int dirisnotempty (OZ_VDFS_File *dirfile, OZ_VDFS_Iopex *iopex);
static uLong make_header_room (OZ_VDFS_File *file, OZ_VDFS_Filex *filex, uWord roomsize, int narea, int exthdr, OZ_VDFS_Filex **filex_r, OZ_VDFS_Iopex *iopex);
static uLong allocate_blocks (OZ_VDFS_Volume *volume, OZ_Dbn nblocks, OZ_Dbn startlbn, OZ_Dbn *nblocks_r, OZ_Dbn *logblock_r, OZ_VDFS_Iopex *iopex);
static uLong read_header_block (const OZ_VDFS_Fileid *fileid, OZ_Dbn hdrlbn, int exthdr, OZ_VDFS_Fileid *lastfid, OZ_VDFS_Filex *filex, OZ_VDFS_Iopex *iopex);
static void mark_exthdr_dirty (OZ_VDFS_Filex *dirtyfilex, OZ_VDFS_File *dirtyfile);
static uLong write_header_block (OZ_VDFS_Volume *volume, OZ_VDFS_Filex *filex, OZ_VDFS_Iopex *iopex);
static void write_dirty_homeboy (OZ_VDFS_Volume *volume, OZ_VDFS_Iopex *iopex);
static int validate_header (Header *header, OZ_VDFS_Volume *volume, OZ_VDFS_Iopex *iopex);

/************************************************************************/
/*									*/
/*  Boot-time initialization routine					*/
/*									*/
/************************************************************************/

static const OZ_VDFS_Vector vector = { sizeof (OZ_VDFS_Fileid), 
                                       VOLNAME_MAX, FILENAME_MAX, SECATTR_MAX, 
                                       1, 			// it does versions

                                       dfs_close_file, 		// close a file
                                       dfs_create_file, 	// create a new file
                                       dfs_dismount_volume, 	// dismount volume
                                       dfs_enter_file, 		// enter a new name in a directory
                                       dfs_extend_file, 	// extend a file
                                       dfs_findopenfile, 	// see if a file is already open
                                       dfs_get_rootdirid, 	// get root directory id
                                       dfs_get_volname, 	// get volume name
                                       dfs_getinfo2, 		// get name of file open on a channel
                                       dfs_init_volume, 	// initialize a volume
                                       dfs_lookup_file, 	// look up a particular file in a directory
                                       dfs_mount_volume, 	// mount volume
                                       dfs_open_file, 		// open a file
                                       dfs_remove_file, 	// remove name from directory
                                       dfs_set_file_attrs, 	// write a file's attributes
                                       dfs_write_dirty_header, 	// flush file's header(s) to disk
                                       dfs_writehomeblock, 	// flush volume's header to disk

                                       dfs_fis_writethru, 	// see if file is a 'writethru' file
                                       dfs_get_fileid, 		// get file id
                                       dfs_getinfo1, 		// get info about the file open on channel
                                       dfs_getinfo3, 		// get info about the volume
                                       dfs_is_directory, 	// see if file is a directory
                                       dfs_map_vbn_to_lbn, 	// map a file's vbn to equivalent lbn 
                                       dfs_mark_header_dirty, 	// mark (prime) header dirty
                                       dfs_returnspec, 		// return filespec string/substrings
                                       dfs_vis_writethru, 	// see if volume is a 'writethru' volume
                                       dfs_wildscan_continue };	// scan directory block for a particular wildcard match

void oz_dev_dfs_init ()

{
  oz_dev_vdfs_init (OZ_VDFS_VERSION, "oz_ext160g", &vector);
}

/************************************************************************/
/*									*/
/*  Return whether the file is a directory or not			*/
/*									*/
/*    Input:								*/
/*									*/
/*	file = file to make determination about				*/
/*									*/
/*    Output:								*/
/*									*/
/*	dfs_is_directory = 0 : file is not a directory			*/
/*	                   1 : file is a directory			*/
/*									*/
/************************************************************************/

static int dfs_is_directory (OZ_VDFS_File *file)

{
  return (IS_DIRECTORY (file -> filex -> header));
}

/************************************************************************/
/*									*/
/*  Return whether the file forces cache writethru mode			*/
/*									*/
/*    Input:								*/
/*									*/
/*	file = file to make determination about				*/
/*	virtblock = virtual block being written				*/
/*	blockoffs = offset in the block we're starting at		*/
/*	size = size of the 'buff' being written				*/
/*	buff = data being written					*/
/*									*/
/*    Output:								*/
/*									*/
/*	dfs_fis_writethru = 0 : file can use writeback mode		*/
/*	                    1 : file forces writethru mode		*/
/*									*/
/************************************************************************/

static int dfs_fis_writethru (OZ_VDFS_File *file, OZ_Dbn virtblock, uLong blockoffs, uLong size, const void *buff)

{
  return (1);
}

/************************************************************************/
/*									*/
/*  Return whether the volume forces cache writethru mode		*/
/*									*/
/*    Input:								*/
/*									*/
/*	volume = volume to make determination about			*/
/*									*/
/*    Output:								*/
/*									*/
/*	dfs_vis_writethru = 0 : volume can use writeback mode		*/
/*	                    1 : volume forces writethru mode		*/
/*									*/
/************************************************************************/

static int dfs_vis_writethru (OZ_VDFS_Volume *volume)

{
  return (1);
}

/************************************************************************/
/*									*/
/*  Retrieve the volume's label						*/
/*									*/
/*    Input:								*/
/*									*/
/*	volume = volume to get the label of				*/
/*									*/
/*    Output:								*/
/*									*/
/*	dfs_get_volname = pointer to null-terminated label string	*/
/*									*/
/************************************************************************/

static const char *dfs_get_volname (OZ_VDFS_Volume *volume)

{
  return (volume -> volex -> homeblock.volname);
}

/************************************************************************/
/*									*/
/*  Get information part 1						*/
/*									*/
/************************************************************************/

static uLong dfs_getinfo1 (OZ_VDFS_Iopex *iopex)

{
  Filattr filattr;
  OZ_VDFS_Filex *filex;

  filex = iopex -> chnex -> file -> filex;

  movc4 (filex -> header.areas[OZ_FS_HEADER_AREA_FILATTR].size, filex -> header.area + filex -> header.areas[OZ_FS_HEADER_AREA_FILATTR].offs, sizeof filattr, &filattr);
  iopex -> u.getinfo1.p.filattrflags = filattr.filattrflags;
  iopex -> u.getinfo1.p.create_date  = filattr.create_date;
  iopex -> u.getinfo1.p.access_date  = filattr.access_date;
  iopex -> u.getinfo1.p.change_date  = filattr.change_date;
  iopex -> u.getinfo1.p.modify_date  = filattr.modify_date;
  iopex -> u.getinfo1.p.expire_date  = filattr.expire_date;
  iopex -> u.getinfo1.p.backup_date  = filattr.backup_date;
  iopex -> u.getinfo1.p.archive_date = filattr.archive_date;
  if (iopex -> u.getinfo1.p.fileidbuff != NULL) {
    movc4 (sizeof filex -> header.fileid, &(filex -> header.fileid), iopex -> u.getinfo1.p.fileidsize, iopex -> u.getinfo1.p.fileidbuff);
  }
  return (OZ_SUCCESS);
}

/************************************************************************/
/*									*/
/*  Continue scanning the current directory block			*/
/*									*/
/*  The directory is locked on input and is unlocked on return		*/
/*									*/
/************************************************************************/

static void dfs_wildscan_continue (OZ_VDFS_Chnex *chnex)

{
}

/************************************************************************/
/*									*/
/*  Get information part 2						*/
/*									*/
/************************************************************************/

static uLong dfs_getinfo2 (OZ_VDFS_Iopex *iopex)

{
  return (OZ_NOTSUPINLDR);
}

/************************************************************************/
/*									*/
/*  Get information part 3 (no file need be open)			*/
/*									*/
/************************************************************************/

static uLong dfs_getinfo3 (OZ_VDFS_Iopex *iopex)

{
  return (OZ_NOTSUPINLDR);
}

/************************************************************************/
/*									*/
/*  Initialize a volume							*/
/*									*/
/*    Input:								*/
/*									*/
/*	volnamelen = length of volume name string			*/
/*	volname    = volume name string (not null terminated)		*/
/*	clusterfactor = cluster factor					*/
/*	secattrsize/buff = security attributes for the volume		*/
/*									*/
/*    Output:								*/
/*									*/
/*	init_volume = OZ_SUCCESS : successful				*/
/*	                    else : error status				*/
/*									*/
/************************************************************************/

static uLong dfs_init_volume (OZ_VDFS_Devex *devex, OZ_VDFS_Volume *volume, OZ_IO_disk_getinfo1 *disk_getinfo1, int volnamelen, const char *volname, uLong clusterfactor, uLong secattrsize, const void *secattrbuff, uLong initflags, OZ_VDFS_Iopex *iopex)

{
  return (OZ_NOTSUPINLDR);
}

/************************************************************************/
/*									*/
/*  Mount a volume							*/
/*									*/
/*    Output:								*/
/*									*/
/*	mount_volume = OZ_SUCCESS : successful				*/
/*	                     else : error status			*/
/*	*volume_r = volume pointer					*/
/*									*/
/************************************************************************/

static uLong dfs_mount_volume (OZ_VDFS_Devex *devex, OZ_VDFS_Volume *volume, OZ_IO_disk_getinfo1 *disk_getinfo1, uLong mountflags, OZ_VDFS_Iopex *iopex)

{
  Header *headerbuff;
  OZ_Dbn fno, vbn;
  OZ_VDFS_Fileid fileid;
  OZ_VDFS_Volex *volex;
  OZ_Secaccmsk secaccmsk;
  Pointer *pointer;
  uLong *blockbuff, i, sts;
  uWord cksm;

  /* Allocate volex (volume extension) struct */

  volex = OZ_KNL_PGPMALLOQ (((uByte *)&(volex -> homeblock)) + devex -> blocksize - (uByte *)volex);
  if (volex == NULL) return (OZ_EXQUOTAPGP);
  memset (volex, 0, ((uByte *)&(volex -> homeblock)) - (uByte *)volex);
  volume -> volex = volex;

  /* Calculate homeblock location */

  calc_home_block (volume);

  /* Read the homey and validate it */

  sts = oz_dev_vdfs_readlogblock (volume -> hb_logblock, 0, devex -> blocksize, &(volex -> homeblock), iopex);
  if (sts != OZ_SUCCESS) goto rtnerr;
  sts = OZ_BADHOMEBLKVER;
  if (volex -> homeblock.homeversion != HOMEBLOCK_VERSION) goto rtnerr;
  cksm = 0;
  for (i = 0; i < sizeof volex -> homeblock / sizeof (uWord); i ++) {
    cksm += ((uWord *)&(volex -> homeblock))[i];
  }
  sts = OZ_BADHOMEBLKCKSM;
  if (cksm != 0) goto rtnerr;
  if (volex -> homeblock.blocksize != disk_getinfo1 -> blocksize) {
    oz_dev_vdfs_printk (iopex, "oz_dev_dfs mount: volume blocksize %u, disk blocksize %u\n", volex -> homeblock.blocksize, disk_getinfo1 -> blocksize);
    sts = OZ_BADBLOCKSIZE;
    goto rtnerr;
  }

  /* Force mountflags to do WRITETHRU mode so we only have to test one bit */

  volume -> mountflags |= OZ_FS_MOUNTFLAG_WRITETHRU;

  /* Allocate directory block buffer (used by lookup_file, extend_file and remove_file routines) */

  volume -> dirblocksize  = volex -> homeblock.clusterfactor * volex -> homeblock.blocksize;
  volume -> clusterfactor = volex -> homeblock.clusterfactor;
  volex  -> dirblockbuff3 = OZ_KNL_PGPMALLOQ (volume -> dirblocksize * 3);
  if (volex -> dirblockbuff3 == NULL) {
    oz_dev_vdfs_printk (iopex, "oz_dev_dfs mount: no quota for %u byte dirblockbuff3\n", volume -> dirblocksize * 3);
    sts = OZ_EXQUOTAPGP;
    goto rtnerr;
  }

  /* Determine security access mask to open files with */

  secaccmsk = OZ_SECACCMSK_LOOK | OZ_SECACCMSK_READ | OZ_SECACCMSK_WRITE;

  /* Open the indexheaders file.  This must be the first file opened as it is how we figure out where all other file headers are. */

  fileid.num = SACRED_FIDNUM_INDEXHEADERS;
  fileid.seq = 0;
  fileid.rvn = 1;
  sts = oz_dev_vdfs_open_file (volume, &fileid, secaccmsk, &(volex -> indexheaders), iopex);
  if (sts != OZ_SUCCESS) goto rtnerr;

  /* Open the storage bitmap file */

  fileid.num = SACRED_FIDNUM_STORAGEBITMAP;
  fileid.seq = 0;
  fileid.rvn = 1;
  sts = oz_dev_vdfs_open_file (volume, &fileid, secaccmsk, &(volex -> storagebitmap), iopex);
  if (sts != OZ_SUCCESS) goto rtnerr;

  /* Extend the storage bitmap file to 1960 blocks */

  sts = dfs_extend_file (volex -> storagebitmap, 1960, 0, iopex);
  if (sts != OZ_SUCCESS) {
    oz_dev_vdfs_printk (iopex, "oz_dev_dfs: error %u extending storagebitmap to 1960 blocks\n", sts);
    goto rtnerr;
  }

  /* Write storage bitmap header out to disk */

  FILATTRS (volex -> storagebitmap -> filex -> header) -> eofblock = 1961;
  sts = write_header_block (volume, volex -> storagebitmap -> filex, iopex);
  if (sts != OZ_SUCCESS) {
    oz_dev_vdfs_printk (iopex, "oz_dev_dfs: error %u writing storagebitmap header back to disk\n", sts);
    goto rtnerr;
  }
  oz_dev_vdfs_printk (iopex, "oz_dev_dfs: storage bitmap successfully extended to 1960 blocks\n");

  /* Now write the home block with the correct number of total clusters available */

  volex -> homeblock.clustertotal = 8004326;
  sts = dfs_writehomeblock (volume, iopex);
  if (sts != OZ_SUCCESS) {
    oz_dev_vdfs_printk (iopex, "oz_dev_dfs: error %u writing homeblock\n", sts);
  } else {
    oz_dev_vdfs_printk (iopex, "oz_dev_dfs: homeblock written with new total clusters\n");
    sts = OZ_FLAGWASSET;	// successful, but don't return OZ_SUCCESS so oz_dev_vdfs thinks we failed
  }

rtnerr:
  if (volex -> dirblockbuff3 != NULL) OZ_KNL_PGPFREE (volex -> dirblockbuff3);
  OZ_KNL_PGPFREE (volex);
  volume -> volex = NULL;
  return (sts);
}

/* Calculate volume's home block location */

static void calc_home_block (OZ_VDFS_Volume *volume)

{
  if (volume -> bb_logblock > 0) volume -> hb_logblock = volume -> bb_logblock - 1;	/* if there's room just before the boot block, put it there */
  else volume -> hb_logblock = volume -> bb_logblock + volume -> bb_nblocks;		/* otherwise, put it just after the boot block */
}

/************************************************************************/
/*									*/
/*  Dismount volume							*/
/*									*/
/*    Input:								*/
/*									*/
/*	volume = volume to be dismounted				*/
/*	unload = 0 : leave volume online				*/
/*	         1 : unload volume (if possible)			*/
/*									*/
/*    Output:								*/
/*									*/
/*	volume dismounted						*/
/*									*/
/************************************************************************/

static uLong dfs_dismount_volume (OZ_VDFS_Devex *devex, OZ_VDFS_Volume *volume, int unload, int shutdown, OZ_VDFS_Iopex *iopex)

{
  return (OZ_NOTSUPINLDR);
}

/************************************************************************/
/*									*/
/*  Get root directory's fileid						*/
/*									*/
/************************************************************************/

static uLong dfs_get_rootdirid (OZ_VDFS_Devex *devex, OZ_VDFS_Fileid *rootdirid_r)

{
  rootdirid_r -> num = SACRED_FIDNUM_ROOTDIRECTORY;
  rootdirid_r -> rvn = 1;
  rootdirid_r -> seq = 0;
  return (OZ_SUCCESS);
}

/************************************************************************/
/*									*/
/*  Get pointer to a file's fileid					*/
/*									*/
/************************************************************************/

static const OZ_VDFS_Fileid *dfs_get_fileid (OZ_VDFS_File *file)

{
  return (&(file -> filex -> header.fileid));
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
/*	           OZ_NOSUCHFILE : entry not found			*/
/*	                    else : error status				*/
/*	*dirvbn_r = dir vbn that the entry was found in (or last vbn looked at)
/*	*fileid_r = file-id of found file				*/
/*	*name_r   = name found						*/
/*									*/
/*    Note:								*/
/*									*/
/*	This routine does not do wildcard scanning, it just finds a 	*/
/*	particular file (like for an 'open' type request).		*/
/*									*/
/************************************************************************/

static uLong dfs_lookup_file (OZ_VDFS_File *dirfile, int namelen, const char *name, OZ_VDFS_Fileid *fileid_r, char *name_r, OZ_VDFS_Iopex *iopex)

{
  return (OZ_NOTSUPINLDR);
}

/************************************************************************/
/*									*/
/*  Enter a file in a directory						*/
/*									*/
/*    Input:								*/
/*									*/
/*	dirfile    = directory file					*/
/*	dirname    = directory name (diag only)				*/
/*	namelen    = length of name to enter				*/
/*	name       = name to enter					*/
/*	newversion = make sure name is the highest version		*/
/*	file       = open file pointer (or NULL if not open)		*/
/*	fileid     = the file's id					*/
/*									*/
/*    Output:								*/
/*									*/
/*	enter_file = OZ_SUCCESS : successful				*/
/*	                   else : error status				*/
/*	*name_r = filled in with resultant name (incl version)		*/
/*									*/
/************************************************************************/

static uLong dfs_enter_file (OZ_VDFS_File *dirfile, const char *dirname, int namelen, const char *name, int newversion, OZ_VDFS_File *file, const OZ_VDFS_Fileid *fileid, char *name_r, OZ_VDFS_Iopex *iopex)

{
  return (OZ_NOTSUPINLDR);
}

/************************************************************************/
/*									*/
/*  Remove a file from a directory					*/
/*									*/
/*    Input:								*/
/*									*/
/*	dirfile = directory file					*/
/*	name    = name to remove (must include absolute version number)	*/
/*									*/
/*    Output:								*/
/*									*/
/*	remove_file = OZ_SUCCESS : successful				*/
/*	                    else : error status				*/
/*									*/
/************************************************************************/

static uLong dfs_remove_file (OZ_VDFS_File *dirfile, const char *name, char *name_r, OZ_VDFS_Iopex *iopex)

{
  return (OZ_NOTSUPINLDR);
}

/************************************************************************/
/*									*/
/*  Determine if the given file is a directory, and if so, if it has 	*/
/*  any entries in it							*/
/*									*/
/************************************************************************/

static int dirisnotempty (OZ_VDFS_File *dirfile, OZ_VDFS_Iopex *iopex)

{
  return (1);
}

/************************************************************************/
/*									*/
/*  Return parsed filespec string					*/
/*									*/
/************************************************************************/

static void dfs_returnspec (char *spec, uLong size, char *buff, OZ_FS_Subs *subs)

{
  char *p, *q, *r;

  if (size > 0) movc4 (strlen (spec), spec, size, buff);	/* if buffer given, return the string */

  if (subs != NULL) {						/* see if substring sizes wanted */
    memset (subs, 0, sizeof *subs);
    if (strcmp (spec, "/") == 0) {				/* make sure we handle this case correctly */
      subs -> namsize = 1;					/* ... it is the root directory as a file */
      return;
    }
    p = strrchr (spec, '/');					/* find the last / in the spec */
    if (p == NULL) p = spec;					/* if none, point to beginning */
    else if (p[1] != 0) p ++;					/* ... but make sure we're after it */
    else {
      while (p > spec) {					/* last char was last slash, ... */
        if (p[-1] == '/') break;				/* ... so we consider the last dirname */
        -- p;							/* ... to be the file name */
      }
      subs -> dirsize = p - spec;				/* ... no type or version */
      subs -> namsize = strlen (p);				/* ... even if the name has a . in it */
      return;
    }
    subs -> dirsize = p - spec;					/* directory is all up to that point including last / */
    q = strrchr (p, ';');					/* find the last ; in the spec */
    if (q == NULL) q = p + strlen (p);				/* if none, point to end of spec */
    subs -> versize = strlen (q);				/* version is length of that part */
    r = strrchr (p, '.');					/* find the last . in the string */
    if ((r == NULL) || (r > q)) r = q;				/* if none, point at ; */
    subs -> typsize = q - r;					/* type is starting at . up to ; */
    subs -> namsize = r - p;					/* name is after directory up to . */
  }
}

/************************************************************************/
/*									*/
/*  Create file								*/
/*									*/
/*    Input:								*/
/*									*/
/*	volume  = volume struct pointer					*/
/*	namelen = length of file name string				*/
/*	name    = file name string (not null terminated)		*/
/*	filattrflags = file attribute flags				*/
/*	dirid   = directory id						*/
/*	file    = file block pointer					*/
/*	file -> secattr = security attributes				*/
/*									*/
/*    Output:								*/
/*									*/
/*	create_file = OZ_SUCCESS : successful creation			*/
/*	                    else : error status				*/
/*	file -> filex = filled in					*/
/*									*/
/************************************************************************/

static uLong dfs_create_file (OZ_VDFS_Volume *volume, int namelen, const char *name, uLong filattrflags, OZ_VDFS_Fileid *dirid, OZ_VDFS_File *file, OZ_VDFS_Fileid **fileid_r, OZ_VDFS_Iopex *iopex)

{
  return (OZ_NOTSUPINLDR);
}

/************************************************************************/
/*									*/
/*  See if the given file is already open				*/
/*									*/
/*    Input:								*/
/*									*/
/*	volume = volume to check					*/
/*	fileid = file to check for					*/
/*									*/
/*    Output:								*/
/*									*/
/*	dfs_findopenfile = NULL : file is not already open		*/
/*	                   else : pointer to file struct		*/
/*									*/
/************************************************************************/

static OZ_VDFS_File *dfs_findopenfile (OZ_VDFS_Volume *volume, const OZ_VDFS_Fileid *fileid)

{
  OZ_VDFS_File *file;

  for (file = volume -> openfiles; file != NULL; file = file -> next) {
    if (memcmp (&(file -> filex -> header.fileid), fileid, sizeof *fileid) == 0) break;
  }
  return (file);
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
/*	dfs_open_file = OZ_SUCCESS : successful completion		*/
/*	                      else : error status			*/
/*	file -> filex = filled in with fs dependent struct		*/
/*	     -> secattr = filled in with file's secattrs		*/
/*	     -> attrlock_efblk,_efbyt = file's end-of-file pointer	*/
/*	     -> allocblocks = number of blocks allocated to file	*/
/*									*/
/************************************************************************/

static uLong dfs_open_file (OZ_VDFS_Volume *volume, const OZ_VDFS_Fileid *fileid, OZ_VDFS_File *file, OZ_VDFS_Iopex *iopex)

{
  OZ_Dbn hdrlbn, nblocks;
  OZ_VDFS_Filex *extfilex, *filex;
  OZ_VDFS_Volex *volex;
  uByte *secbuff;
  uLong i, secsize, sts;

  volex = volume -> volex;

  /* Figure out where the header's logical block is.  For the index file, it is in */
  /* the home block.  All others use the file number as the vbn in the index file. */

  if ((fileid -> num == SACRED_FIDNUM_INDEXHEADERS) && (fileid -> seq == 0)) {
    hdrlbn = volex -> homeblock.indexhdrlbn;
  } else {
    sts = dfs_map_vbn_to_lbn (volume -> volex -> indexheaders, fileid -> num, &nblocks, &hdrlbn);
    if (sts != OZ_SUCCESS) return (OZ_INVALIDFILENUM);
  }

  /* Read the header from disk */

  filex = OZ_KNL_PGPMALLOQ (((uByte *)&(filex -> header)) + volex -> homeblock.blocksize - (uByte *)filex);
  if (filex == NULL) return (OZ_EXQUOTAPGP);
  file -> filex = filex;
  sts = read_header_block (fileid, hdrlbn, 0, NULL, filex, iopex);
  if (sts != OZ_SUCCESS) goto rtnerr;

  /* Return the end-of-file pointer */

  file -> attrlock_efblk = FILATTRS (filex -> header) -> eofblock;
  file -> attrlock_efbyt = FILATTRS (filex -> header) -> eofbyte;

  /* Read in all the extension headers.  If error anywhere, undo everything.  Also count the number of allocated blocks. */

  file -> allocblocks = filex -> blocksinhdr;
  secsize = filex -> header.areas[OZ_FS_HEADER_AREA_SECATTR].size;
  while (filex -> header.extid.num != 0) {

    /* Get lbn of extension header */

    sts = dfs_map_vbn_to_lbn (volume -> volex -> indexheaders, filex -> header.extid.num, &nblocks, &hdrlbn);
    if (sts != OZ_SUCCESS) goto rtnerr;

    /* Allocate a struct to read the header into */

    extfilex = OZ_KNL_PGPMALLOQ (((uByte *)&(extfilex -> header)) + volex -> homeblock.blocksize - ((uByte *)extfilex));
    sts = OZ_EXQUOTAPGP;
    if (extfilex == NULL) goto rtnerr;
    filex -> next = extfilex;

    /* Read in the header and make sure it is valid */

    sts = read_header_block (&(filex -> header.extid), hdrlbn, 1, &(filex -> header.fileid), extfilex, iopex);
    if (sts != OZ_SUCCESS) goto rtnerr;

    /* Accumulate various things and link to next extension header */

    file -> allocblocks += extfilex -> blocksinhdr;
    secsize += extfilex -> header.areas[OZ_FS_HEADER_AREA_SECATTR].size;
    filex = extfilex;
  }
  filex = file -> filex;

  /* Make an secattr struct based on what's in the file's header(s) */

  file -> secattr = NULL;
  if (secsize != 0) {
    secbuff = OZ_KNL_PGPMALLOQ (secsize);					/* malloc a temp buffer */
    sts = OZ_EXQUOTAPGP;
    if (secbuff == NULL) goto rtnerr;
    i = 0;									/* copy in all the attributes */
    for (extfilex = filex; extfilex != NULL; extfilex = extfilex -> next) {
      memcpy (secbuff + i, extfilex -> header.area + extfilex -> header.areas[OZ_FS_HEADER_AREA_SECATTR].offs, extfilex -> header.areas[OZ_FS_HEADER_AREA_SECATTR].size);
      i += extfilex -> header.areas[OZ_FS_HEADER_AREA_SECATTR].size;
    }
    if (i != secsize) oz_crash ("oz_disk_fs open_by_lbn: security attribute size changed");
    sts = oz_knl_secattr_create (secsize, secbuff, NULL, &(file -> secattr));	/* create a kernel struct for it */
    OZ_KNL_PGPFREE (secbuff);							/* free off temp buffer */
    if (sts != OZ_SUCCESS) goto rtnerr;
  }

  return (OZ_SUCCESS);

  /* Error, free stuff off and return error status */

rtnerr:
  while ((filex = file -> filex) != NULL) {
    file -> filex = filex -> next;
    OZ_KNL_PGPFREE (filex);
  }
  return (sts);
}

/************************************************************************/
/*									*/
/*  Set file attributes							*/
/*									*/
/*    Input:								*/
/*									*/
/*	file = file to have the attributes set				*/
/*	numitems = number of elements in itemlist array			*/
/*	itemlist = array of items to set				*/
/*	iopex = I/O request being processed				*/
/*									*/
/*    Output:								*/
/*									*/
/*	set_file_attrs = completion status				*/
/*									*/
/************************************************************************/

static uLong dfs_set_file_attrs (OZ_VDFS_File *file, uLong numitems, const OZ_Itmlst2 *itemlist, OZ_VDFS_Iopex *iopex)

{
  return (OZ_NOTSUPINLDR);
}

/************************************************************************/
/*									*/
/*  Close file, delete if marked for delete				*/
/*									*/
/*    Input:								*/
/*									*/
/*	file = file to be closed					*/
/*									*/
/*    Output:								*/
/*									*/
/*	all filex structs freed off					*/
/*	file possibly deleted						*/
/*									*/
/************************************************************************/

static uLong dfs_close_file (OZ_VDFS_File *file, OZ_VDFS_Iopex *iopex)

{
  return (OZ_NOTSUPINLDR);
}

/************************************************************************/
/*									*/
/*  Extend or truncate a file						*/
/*									*/
/*    Input:								*/
/*									*/
/*	file     = file block pointer of file to extended / truncated	*/
/*	nblocks  = new total number of blocks				*/
/*	extflags = EXTEND_NOTRUNC : don't truncate			*/
/*	          EXTEND_NOEXTHDR : no extension header			*/
/*									*/
/*    Output:								*/
/*									*/
/*	extend_file = OZ_SUCCESS : extend was successful		*/
/*	                    else : error status				*/
/*									*/
/************************************************************************/

static uLong dfs_extend_file (OZ_VDFS_File *file, OZ_Dbn nblocks, uLong extflags, OZ_VDFS_Iopex *iopex)

{
  OZ_Dbn logblock, relblock, startlbn;
  OZ_VDFS_Filex *extfilex, *extfilex2;
  OZ_VDFS_Volex *volex;
  OZ_VDFS_Volume *volume;
  Pointer *pointer;
  uLong i, nfound, savei, sts;

  volume = file -> volume;
  volex  = volume -> volex;

  /* Get number of blocks rounded up by cluster factor */

  relblock = ((nblocks + volex -> homeblock.clusterfactor - 1) / volex -> homeblock.clusterfactor) * volex -> homeblock.clusterfactor;

  /* Find the extension to extend at or truncate at */

  extfilex2 = NULL;
  for (extfilex = file -> filex; extfilex != NULL; extfilex = extfilex -> next) {
    if (extfilex -> blocksinhdr >= relblock) goto truncate_it;
    relblock -= extfilex -> blocksinhdr;
    extfilex2 = extfilex;
  }

  /*****************************/
  /* File needs to be extended */
  /*****************************/

  while (relblock != 0) {

    /* Get lbn last used in file */

    pointer  = POINTERS (extfilex2);
    startlbn = 0;
    if (extfilex2 -> header.areas[OZ_FS_HEADER_AREA_POINTER].size > 0) {
      pointer += extfilex2 -> header.areas[OZ_FS_HEADER_AREA_POINTER].size / sizeof *pointer;
      startlbn = pointer[-1].logblock + pointer[-1].blockcount;
    }

    /* Make room in the header for the new pointer */

    sts = make_header_room (file, extfilex2, sizeof (Pointer), OZ_FS_HEADER_AREA_POINTER, !(extflags & EXTEND_NOEXTHDR), &extfilex2, iopex);
    if (sts != OZ_SUCCESS) return (sts);

    /* Allocate the blocks */

    sts = allocate_blocks (volume, relblock, startlbn, &nfound, &logblock, iopex);
    if (sts != OZ_SUCCESS) return (sts);

    /* Set up the new pointer */

    pointer = POINTERS (extfilex2) + extfilex2 -> header.areas[OZ_FS_HEADER_AREA_POINTER].size / sizeof *pointer; /* point to where to put new pointer */

    if ((extfilex2 -> header.areas[OZ_FS_HEADER_AREA_POINTER].size >= sizeof *pointer) 				/* if there is at least one pointer there ... */
     && (pointer[-1].blockcount + pointer[-1].logblock == logblock)) {						/* ... and it ends where new one starts ... */
      pointer[-1].blockcount += nfound;										/* it's contiguous with the last allocation */
    } else {
      pointer -> blockcount = nfound;										/* not contiguous, make a new pointer */
      pointer -> logblock   = logblock;
      extfilex2 -> header.areas[OZ_FS_HEADER_AREA_POINTER].size += sizeof *pointer;				/* increase pointer area size */
    }

    /* Increment number of blocks in the header */

    file -> allocblocks      += nfound;
    extfilex2 -> blocksinhdr += nfound;

    /* Decrement number of blocks wanted */

    relblock -= nfound;
  }

  return (OZ_SUCCESS);

  /******************************/
  /* File needs to be truncated */
  /******************************/

truncate_it:
  return (OZ_SUCCESS);
}

/************************************************************************/
/*									*/
/*  Make room in header for one of the size/offs items			*/
/*									*/
/*    Input:								*/
/*									*/
/*	filex    = header to make room in				*/
/*	roomsize = amount of room to add				*/
/*	narea    = area to add room size to				*/
/*	exthdr   = 0 : don't add extension header			*/
/*	           1 : ok to add extension header if needed		*/
/*									*/
/*    Output:								*/
/*									*/
/*	make_header_room = OZ_SUCCESS : room successfully created	*/
/*	                         else : error status			*/
/*	**filex_r = file extension header the room is in		*/
/*	size not updated to include new room, caller must do that	*/
/*									*/
/************************************************************************/

static uLong hdrareausedsize (OZ_VDFS_Filex *filex);

static uLong make_header_room (OZ_VDFS_File *file, OZ_VDFS_Filex *filex, uWord roomsize, int narea, int exthdr, OZ_VDFS_Filex **filex_r, OZ_VDFS_Iopex *iopex)

{
  int i;
  OZ_VDFS_Filex *extfilex;
  OZ_VDFS_Volume *volume;
  uLong areasize, areaoffs, size, sts;

  volume = iopex -> devex -> volume;

  /* Round up roomsize to longword boundary */

  roomsize = (roomsize + 3) & -4;

  /* Calculate size of the entire 'area[]' in the header for the blocksize, rounded down to longword size */

  areasize = (((uByte *)&(filex -> header)) + volume -> volex -> homeblock.blocksize - filex -> header.area) & -4;

  /* Make sure they're not asking for too much */

  if (roomsize > areasize) oz_crash ("oz_dev_dfs make_header_room: area %u too big for header (max %u)", roomsize, areasize);

  /* If there isn't enough room for new stuff, allocate and link up an extension header */

  if (hdrareausedsize (filex) + roomsize > areasize) {
    return (OZ_FILEHDRFULL);				// maybe caller doesn't want one
  }

  *filex_r = filex;

  /* It will fit in the current header */

  /* Move stuff that follows it all the way to the end of the header.  This will    */
  /* allow for maximal expansion of the area without having to move anything again. */

  for (i = OZ_FS_HEADER_NAREAS; -- i > narea;) {
    size = filex -> header.areas[i].size;
    if (size != 0) {
      size = (size + 3) & -4;
      areasize -= size;
      if (filex -> header.areas[i].offs != areasize) {
        memmove (filex -> header.area + areasize, filex -> header.area + filex -> header.areas[i].offs, size);
      }
    }
    filex -> header.areas[i].offs = areasize;
  }

  /* Move it and all stuff before it to the beginning of the header (same reason) */

  areaoffs = 0;
  for (i = 0; i <= narea; i ++) {
    size = (filex -> header.areas[i].size + 3) & -4;
    if ((size != 0) && (filex -> header.areas[i].offs != areaoffs)) {
      memmove (filex -> header.area + areaoffs, filex -> header.area + filex -> header.areas[i].offs, size);
    }
    filex -> header.areas[i].offs = areaoffs;
    areaoffs += size;
  }

  /* Now there should be at least 'roomsize' bytes left */

  if (areaoffs + roomsize > areasize) oz_crash ("oz_dev_dfs make_header_room: failed to make room");

  /* Garbage fill all the new room that was made */

  memset (filex -> header.area + areaoffs, 0xEA, areasize - areaoffs);

  /* Header needs to be written back to disk */

  mark_exthdr_dirty (filex, file);

  return (OZ_SUCCESS);
}

static uLong hdrareausedsize (OZ_VDFS_Filex *filex)

{
  int i;
  uLong usedsize;

  usedsize = 0;
  for (i = 0; i < OZ_FS_HEADER_NAREAS; i ++) usedsize += (filex -> header.areas[i].size + 3) & -4;
  return (usedsize);
}

/************************************************************************/
/*									*/
/*  Allocate storage blocks						*/
/*									*/
/*    Input:								*/
/*									*/
/*	volume   = volume block pointer					*/
/*	nblocks  = number of wanted blocks				*/
/*	startlbn = requested starting lbn				*/
/*									*/
/*    Output:								*/
/*									*/
/*	allocate_blocks = OZ_SUCCESS : successful allocation		*/
/*	                        else : error status			*/
/*	*nblocks_r  = number of blocks actually allocated		*/
/*	*logblock_r = starting logical block number			*/
/*									*/
/************************************************************************/

static uLong allocate_blocks (OZ_VDFS_Volume *volume, OZ_Dbn nblocks, OZ_Dbn startlbn, OZ_Dbn *nblocks_r, OZ_Dbn *logblock_r, OZ_VDFS_Iopex *iopex)

{
  int bitmapblockdirty, need_bitmap_block;
  OZ_Dbn best_cluster, best_count, bitinbmblock, bitmapvbn, cluster, ncontig_clusters, nstart_cluster, nwanted_clusters, startcluster;
  OZ_VDFS_Volex *volex;
  uLong *bitmapblock, bitsinblock, sts;

  volex = volume -> volex;

  bitsinblock = volex -> homeblock.blocksize * 8;
  bitmapblock = OZ_KNL_PGPMALLOQ (volex -> homeblock.blocksize);
  if (bitmapblock == NULL) return (OZ_EXQUOTAPGP);

  /* Get cluster number to start looking at - 1 */

  startcluster = startlbn / volex -> homeblock.clusterfactor;
  if (startcluster == 0) startcluster = volex -> homeblock.clustertotal;
  startcluster --;

  /* Calculate the number of wanted clusters */

  nwanted_clusters = (nblocks + volex -> homeblock.clusterfactor - 1) / volex -> homeblock.clusterfactor;

  /* Search the bitmap file for the number of wanted clusters, starting with the cluster wanted */

  best_count = 0;								/* haven't found anything yet */
  need_bitmap_block = 1;							/* we need to read the bitmap block */
  ncontig_clusters = 0;								/* no contiguous free clusters found yet */
  for (cluster = startcluster + 1; cluster != startcluster; cluster ++) {	/* start at requested cluster, loop through till we're back at same spot */
    if (cluster == volex -> homeblock.clustertotal) {				/* wrap around the cluster number */
      cluster = 0;
      ncontig_clusters = 0;							/* if we do wrap, start all over counting free contiguous blocks */
    }
    bitinbmblock = cluster % bitsinblock;					/* compute which bit in current bitmap block we want to test */
    if (need_bitmap_block || (bitinbmblock == 0)) {				/* read bitmap block if first time through loop or reached start of new one */
      bitmapvbn = cluster / bitsinblock + 1;
      sts = oz_dev_vdfs_readvirtblock (volex -> storagebitmap, bitmapvbn, 0, volex -> homeblock.blocksize, bitmapblock, iopex, 0);
      if (sts != OZ_SUCCESS) {
        oz_dev_vdfs_printk (iopex, "oz_dev_dfs allocate_blocks: error %u reading storage bitmap block %u\n", sts, bitmapvbn);
        goto cleanup;
      }
      need_bitmap_block = 0;
    }
    if (bitmapblock[bitinbmblock/32] & (1 << (bitinbmblock % 32))) {		/* if bit is set, cluster is allocated, ... */
      ncontig_clusters = 0;							/* ... so start counting contiguous free clusters over again */
    } else {
      if (ncontig_clusters == 0) nstart_cluster = cluster;			/* if first free cluster in a row, save the starting cluster number */
      ncontig_clusters ++;							/* anyway, increment number of free contiguous clusters */
      if (ncontig_clusters == nwanted_clusters) goto found_it;			/* if we have as many as requested, stop looking now */
      if (ncontig_clusters > best_count) {					/* otherwise, if this is the best we have found so far, remember where it is */
        best_count   = ncontig_clusters;
        best_cluster = nstart_cluster;
      }
    }
  }

  /* Couldn't find as much as requested, use what we got (if anything) */

  if (best_count == 0) return (OZ_DISKISFULL);

  ncontig_clusters = best_count;
  nstart_cluster   = best_cluster;

  /* Use ncontig_clusters starting at nstart_cluster */

found_it:

  /* Return the block count and starting block number */

  *nblocks_r  = ncontig_clusters * volex -> homeblock.clusterfactor;
  *logblock_r = nstart_cluster * volex -> homeblock.clusterfactor;

  /* Set the bits in the bit map */

  bitmapblockdirty = 0;
  for (cluster = nstart_cluster; cluster < nstart_cluster + ncontig_clusters; cluster ++) {
    bitinbmblock = cluster % bitsinblock;
    if (cluster / bitsinblock + 1 != bitmapvbn) {
      if (bitmapblockdirty) {
        sts = oz_dev_vdfs_writevirtblock (volex -> storagebitmap, bitmapvbn, 0, volex -> homeblock.blocksize, bitmapblock, iopex, 0);
        if (sts != OZ_SUCCESS) {
          oz_dev_vdfs_printk (iopex, "oz_dev_dfs allocate_blocks: error %u writing storage bitmap block %u\n", sts, cluster / bitsinblock);
          goto cleanup;
        }
      }
      bitmapblockdirty = 0;
      bitmapvbn = cluster / bitsinblock + 1;
      sts = oz_dev_vdfs_readvirtblock (volex -> storagebitmap, bitmapvbn, 0, volex -> homeblock.blocksize, bitmapblock, iopex, 0);
      if (sts != OZ_SUCCESS) {
        oz_dev_vdfs_printk (iopex, "oz_dev_dfs allocate_blocks: error %u reading storage bitmap block %u\n", sts, cluster / bitsinblock + 1);
        goto cleanup;
      }
    }
    if ((1 << (bitinbmblock % 32)) & bitmapblock[bitinbmblock/32]) {
      oz_dev_vdfs_printk (iopex, "oz_dev_dfs allocate_blocks: bitmap bit already set\n");
      oz_dev_vdfs_printk (iopex, "oz_dev_dfs allocate_blocks:   nblocks %u, startlbn %u\n", nblocks, startlbn);
      oz_dev_vdfs_printk (iopex, "oz_dev_dfs allocate_blocks:   *nblocks_r %u, *logblock_r %u\n", *nblocks_r, *logblock_r);
      oz_dev_vdfs_printk (iopex, "oz_dev_dfs allocate_blocks:   cluster %u, bitinbmblock %u\n", cluster, bitinbmblock);
      sts = OZ_BUGCHECK;
      goto cleanup;
    }
    bitmapblock[bitinbmblock/32] |= 1 << (bitinbmblock % 32);
    if (volex -> homeblock.clustersfree == 0) {
      oz_dev_vdfs_printk (iopex, "oz_dev_dfs allocate_blocks: clustersfree was zero\n");
      sts = OZ_BUGCHECK;
      goto cleanup;
    }
    volex -> homeblock.clustersfree --;
    volume -> dirty  = 1;
    bitmapblockdirty = 1;
  }

  if (bitmapblockdirty) {
    sts = oz_dev_vdfs_writevirtblock (volex -> storagebitmap, bitmapvbn, 0, volex -> homeblock.blocksize, bitmapblock, iopex, 0);
    if (sts != OZ_SUCCESS) {
      oz_dev_vdfs_printk (iopex, "oz_dev_dfs allocate_blocks: error %u writing storage bit map block %u\n", sts, cluster / bitsinblock + 1);
    }
  }

cleanup:
  OZ_KNL_PGPFREE (bitmapblock);
  return (sts);
}

/************************************************************************/
/*									*/
/*  Read header block into the given file struct			*/
/*									*/
/*    Input:								*/
/*									*/
/*	fileid = file-id of the header block				*/
/*	hdrlbn = header logical block number				*/
/*	exthdr = 0 : prime header, 1 : extension header			*/
/*	filex  = where to read header block into			*/
/*									*/
/*    Output:								*/
/*									*/
/*	read_header_block = OZ_SUCCESS : successfully read		*/
/*	                          else : read error status		*/
/*	fixed portion of file struct cleared				*/
/*	header read into filex -> header				*/
/*									*/
/************************************************************************/

static uLong read_header_block (const OZ_VDFS_Fileid *fileid, OZ_Dbn hdrlbn, int exthdr, OZ_VDFS_Fileid *lastfid, OZ_VDFS_Filex *filex, OZ_VDFS_Iopex *iopex)

{
  uLong i, sts;
  OZ_Dbn nblocks;
  OZ_VDFS_Volex *volex;
  OZ_VDFS_Volume *volume;
  Pointer *pointer;
  uWord cksm;

  volume = iopex -> devex -> volume;
  volex  = volume -> volex;

  /* Clear out fixed portion of header */

  memset (filex, 0, ((uByte *)&(filex -> header)) - (uByte *)filex);

  /* Read the header block from the indexheaders file on disk into the filex struct */

  sts = oz_dev_vdfs_readlogblock (hdrlbn, 0, volex -> homeblock.blocksize, &(filex -> header), iopex);
  if (sts != OZ_SUCCESS) return (sts);

  /* Validate the header block's checksum */

  cksm = 0;
  for (i = volex -> homeblock.blocksize / sizeof (uWord); i > 0;) {
    cksm += ((uWord *)&(filex -> header))[--i];
  }
  if (cksm != 0) {
    oz_dev_vdfs_printk (iopex, "oz_dev_dfs read_header_block: bad header checksum (%u,%u,%u)  hdrlbn %u\n", fileid -> num, fileid -> rvn, fileid -> seq, hdrlbn);
    oz_knl_dumpmem2 (volex -> homeblock.blocksize, &(filex -> header), 0);
    return (OZ_BADHDRCKSM);
  }

  /* Make sure its file-id is ok */

  if (memcmp (&(filex -> header.fileid), fileid, sizeof *fileid) != 0) {
    oz_dev_vdfs_printk (iopex, "oz_dev_dfs read_header_block: expected fid (%u,%u,%u) got (%u,%u,%u)  hdrlbn %u\n", 
                        fileid -> num, fileid -> rvn, fileid -> seq, 
                        filex -> header.fileid.num, filex -> header.fileid.rvn, filex -> header.fileid.seq, 
                        hdrlbn);
    return (OZ_FILEDELETED);
  }

  /* Make sure the header looks nice */

  if (!validate_header (&(filex -> header), volume, iopex)) return (OZ_FILECORRUPT);

  /* Make sure its header extension sequence is ok */

  if ((filex -> header.dircount == EXT_DIRCOUNT) ^ exthdr) {
    oz_dev_vdfs_printk (iopex, "oz_dev_dfs read_header_block: ext header (%u,%u,%u) dircount is %u\n", fileid -> num, fileid -> rvn, fileid -> seq, filex -> header.dircount);
    return (OZ_FILECORRUPT);
  }
  if ((lastfid != NULL) && (memcmp (&(filex -> header.dirid), lastfid, sizeof *lastfid) != 0)) {
    oz_dev_vdfs_printk (iopex, "oz_dev_dfs read_header_block: ext header (%u,%u,%u) says it follows (%u,%u,%u) instead of (%u,%u,%u)\n", 
	fileid -> num, fileid -> rvn, fileid -> seq, 
	filex -> header.dirid.num, filex -> header.dirid.rvn, filex -> header.dirid.seq, 
	lastfid -> num, lastfid -> rvn, lastfid -> seq);
    return (OZ_FILECORRUPT);
  }

  /* Set up other filex struct stuff */

  pointer = POINTERS (filex);
  for (i = filex -> header.areas[OZ_FS_HEADER_AREA_POINTER].size / sizeof (Pointer); i > 0; -- i) {
    filex -> blocksinhdr += (pointer ++) -> blockcount;
  }

  return (OZ_SUCCESS);
}

/************************************************************************/
/*									*/
/*  Mark file header is dirty so it will be written to disk		*/
/*									*/
/*    Input:								*/
/*									*/
/*	file = header block to write					*/
/*									*/
/*    Note:								*/
/*									*/
/*	This routine may be called from outside the kernel thread as 	*/
/*	it provides the required synchronization			*/
/*									*/
/************************************************************************/

/* This internal version marks the specific prime or extension header dirty */

static void mark_exthdr_dirty (OZ_VDFS_Filex *dirtyfilex, OZ_VDFS_File *dirtyfile)

{
  oz_dev_vdfs_mark_header_dirty (dirtyfile);				// put on queue of files that need header written out
  if (dirtyfilex != dirtyfile -> filex) {				// see if we're marking prime header dirty
    OZ_HW_ATOMIC_DECBY1_LONG (dirtyfile -> filex -> headerdirty);	// if not, decrement prime header's dirty count
    OZ_HW_ATOMIC_INCBY1_LONG (dirtyfilex -> headerdirty);		// ... and increment extension header's dirty count
  }
}

/* This routine is called by oz_dev_vdfs to mark the prime header dirty (it changed a date or eof position, etc) */

static void dfs_mark_header_dirty (OZ_VDFS_File *dirtyfile)

{
  OZ_HW_ATOMIC_INCBY1_LONG (dirtyfile -> filex -> headerdirty);
}

/* This routine is called by oz_dev_vdfs to write out all headers that are marked dirty    */
/* The individual filex->headerdirty flags will be set for those headers that need writing */

static uLong dfs_write_dirty_header (OZ_VDFS_File *dirtyfile, Long alf, OZ_VDFS_Volume *volume, OZ_VDFS_Iopex *iopex)

{
  uByte *cleanheader;
  Filattr *filattr;
  OZ_Datebin now;
  OZ_VDFS_Filex *dirtyfilex;
  uLong sts, vl;

  dirtyfilex = dirtyfile -> filex;
  filattr = FILATTRS (dirtyfilex -> header);

  /* Maybe the eof pointer in the header needs updating */

  if (alf & OZ_VDFS_ALF_M_EOF) {					// see if recio needs us to update the eof poistion
    vl = oz_hw_smplock_wait (&(dirtyfile -> attrlock_vl));		// ok, lock it so we get consistent efblk/efbyt values
    filattr -> eofblock = dirtyfile -> attrlock_efblk;			// store them in file's header block
    filattr -> eofbyte  = dirtyfile -> attrlock_efbyt;
    oz_hw_smplock_clr (&(dirtyfile -> attrlock_vl), vl);
    dirtyfilex -> headerdirty = 1;
  }

  /* Maybe some dates in the header need updating */

  if (alf & (OZ_VDFS_ALF_M_MDT | OZ_VDFS_ALF_M_CDT | OZ_VDFS_ALF_M_ADT)) { // see if any of the dates are to be modified
    now = oz_hw_tod_getnow ();						// if so, get current date/time
    if (alf & OZ_VDFS_ALF_M_MDT) filattr -> modify_date = now;		// modify the requested values
    if (alf & OZ_VDFS_ALF_M_CDT) filattr -> change_date = now;
    if (alf & OZ_VDFS_ALF_M_ADT) filattr -> access_date = now;
    dirtyfilex -> headerdirty = 1;
  }

  /* Loop through the headers (prime and extension), writing those what are dirty */

  do {
    if (dirtyfilex -> headerdirty) {					// see if the header is dirty
      sts = write_header_block (volume, dirtyfilex, iopex);		// write the header out to disk
      if (sts != OZ_SUCCESS) {
        oz_dev_vdfs_printk (iopex, "oz_dev_dfs write_dirty_header: error %u writing header (%u,%u,%u)\n", 
                            sts, dirtyfilex -> header.fileid.num, dirtyfilex -> header.fileid.rvn, dirtyfilex -> header.fileid.seq);
      }
    } else {
    }
  } while ((dirtyfilex = dirtyfilex -> next) != NULL);			// check next extension in file

  return (sts);
}

static uLong write_header_block (OZ_VDFS_Volume *volume, OZ_VDFS_Filex *filex, OZ_VDFS_Iopex *iopex)

{
  OZ_VDFS_Volex *volex;
  uLong i, sts;
  uWord cksm;

  volex = volume -> volex;

  /* Calculate the header block's new checksum */

  cksm = 0;
  filex -> header.checksum = 0;
  for (i = volex -> homeblock.blocksize / sizeof (uWord); i > 0;) {
    cksm -= ((uWord *)&(filex -> header))[--i];
  }
  filex -> header.checksum = cksm;

  /* We should only be writing nice headers */

  if (!validate_header (&(filex -> header), volume, iopex)) oz_crash ("oz_dev_dfs write_header_block: writing corrupt header");

  /* It is no longer dirty */

  filex -> headerdirty = 0;

  /* Write to disk */

  sts = oz_dev_vdfs_writevirtblock (volex -> indexheaders, filex -> header.fileid.num, 0, volex -> homeblock.blocksize, &(filex -> header), iopex, 0);
  return (sts);
}

/************************************************************************/
/*									*/
/*  Write homeblock to volume						*/
/*									*/
/*    Input:								*/
/*									*/
/*	volume = volume who's homeblock to write			*/
/*	iopex = I/O operation in progress				*/
/*									*/
/*    Output:								*/
/*									*/
/*	writehomeblock = write status					*/
/*									*/
/************************************************************************/

static uLong dfs_writehomeblock (OZ_VDFS_Volume *volume, OZ_VDFS_Iopex *iopex)

{
  int i;
  OZ_VDFS_Volex *volex;
  uLong sts;
  uWord cksm;

  volex = volume -> volex;

  volex -> homeblock.checksum = 0;
  cksm = 0;
  for (i = 0; i < (sizeof volex -> homeblock) / sizeof (uWord); i ++) {
    cksm -= ((uWord *)&(volex -> homeblock))[i];
  }
  volex -> homeblock.checksum = cksm;

  sts = oz_dev_vdfs_writelogblock (volume -> hb_logblock, 0, iopex -> devex -> blocksize, &(volex -> homeblock), 0, iopex);
  return (sts);
}

/************************************************************************/
/*									*/
/*  Map a virtual block number to a logical block number		*/
/*									*/
/*    Input:								*/
/*									*/
/*	file = pointer to file node of the file				*/
/*	virtblock = virtual block number				*/
/*	            vbns start at 1 for first block in file		*/
/*									*/
/*    Output:								*/
/*									*/
/*	map_vbn_to_lbn = OZ_SUCCESS : successful			*/
/*	                 OZ_VBNZERO : vbn zero requested		*/
/*	               OZ_ENDOFFILE : virtblock is past end of file	*/
/*	*nblocks_r  = number of blocks mapped by pointer		*/
/*	*logblock_r = first logical block number			*/
/*									*/
/************************************************************************/

static uLong dfs_map_vbn_to_lbn (OZ_VDFS_File *file, OZ_Dbn virtblock, OZ_Dbn *nblocks_r, OZ_Dbn *logblock_r)

{
  OZ_VDFS_Filex *extfilex;
  uLong i, n;
  OZ_Dbn relblock;
  Pointer *pointer;

  if (virtblock == 0) return (OZ_VBNZERO);
  relblock = virtblock - 1;

  /* Find extension header that maps the virtual block */

  for (extfilex = file -> filex; extfilex != NULL; extfilex = extfilex -> next) {
    if (extfilex -> blocksinhdr > relblock) goto found_extension;
    relblock -= extfilex -> blocksinhdr;
  }
  return (OZ_ENDOFFILE);
found_extension:

  /* Find the pointer that maps the virtual block */

  pointer = POINTERS (extfilex);
  n = extfilex -> header.areas[OZ_FS_HEADER_AREA_POINTER].size / sizeof *pointer;
  for (i = 0; i < n; i ++) {
    if (pointer -> blockcount > relblock) goto found_pointer;
    relblock -= pointer -> blockcount;
    pointer ++;
  }
  oz_crash ("oz_dev_dfs map_vbn_to_lbn: ran off end of pointers");
found_pointer:

  /* Found pointer, relblock is number of blocks to skip in the pointer */

  *nblocks_r  = pointer -> blockcount - relblock;	/* return number of blocks in the pointer starting with requested block */
  *logblock_r = pointer -> logblock + relblock;		/* return logical block number corresponding with the requested vbn */

  return (OZ_SUCCESS);
}  

/************************************************************************/
/*									*/
/*  Validate an file header						*/
/*									*/
/*    Note:  This routine is called by the file open routine to check 	*/
/*	the on-disk header						*/
/*									*/
/************************************************************************/

static int validate_header (Header *header, OZ_VDFS_Volume *volume, OZ_VDFS_Iopex *iopex)

{
  char *p;
  int i;
  Filattr filattr;
  OZ_VDFS_Volex *volex;
  Pointer *pointer;
  uLong lastoffs;

  volex = volume -> volex;

  /* Make sure areas are in order and don't overlap */

  lastoffs = 0;
  for (i = 0; i < OZ_FS_HEADER_NAREAS; i ++) {
    if (header -> areas[i].offs < lastoffs) {
      oz_dev_vdfs_printk (iopex, "oz_dev_dfs validate_header: area[%d].offs %u before last offset %u\n", i, header -> areas[i].offs, lastoffs);
      return (0);
    }
    lastoffs = header -> areas[i].offs + header -> areas[i].size;
  }
  if (header -> area + lastoffs - (uByte *)header > volex -> homeblock.blocksize) {
    oz_dev_vdfs_printk (iopex, "oz_dev_dfs validate_header: last offset %u beyond end of header\n", lastoffs);
    return (0);
  }

  /* Make sure each area contents are ok */

  if (header -> areas[OZ_FS_HEADER_AREA_FILNAME].size != 0) {
    p = (char *)(header -> area + header -> areas[OZ_FS_HEADER_AREA_FILNAME].offs);
    if (strlen (p) >= header -> areas[OZ_FS_HEADER_AREA_FILNAME].size) {
      oz_dev_vdfs_printk (iopex, "oz_dev_dfs validate_header: filename missing terminating null\n");
      return (0);
    }
  }

  if (header -> areas[OZ_FS_HEADER_AREA_POINTER].size != 0) {
    if (header -> areas[OZ_FS_HEADER_AREA_POINTER].size % sizeof *pointer != 0) {
      oz_dev_vdfs_printk (iopex, "oz_dev_dfs validate_header: pointer area size %u not multiple of pointer size\n", header -> areas[OZ_FS_HEADER_AREA_POINTER].size);
      return (0);
    }
    pointer = (Pointer *)(header -> area + header -> areas[OZ_FS_HEADER_AREA_POINTER].offs);
    for (i = 0; i < header -> areas[OZ_FS_HEADER_AREA_POINTER].size / sizeof *pointer; i ++) {
      if (pointer -> blockcount == 0) {
        oz_dev_vdfs_printk (iopex, "oz_dev_dfs validate_header: pointer has blockcount zero\n");
        return (0);
      }
      if (pointer -> blockcount % volex -> homeblock.clusterfactor != 0) {
        oz_dev_vdfs_printk (iopex, "oz_dev_dfs validate_header: pointer has blockcount %u not multiple of clusterfactor\n", pointer -> blockcount);
        return (0);
      }
      if (pointer -> blockcount / volex -> homeblock.clusterfactor >= volex -> homeblock.clustertotal) {
        oz_dev_vdfs_printk (iopex, "oz_dev_dfs validate_header: pointer has blockcount %u larger than whole disk\n", pointer -> blockcount);
        return (0);
      }
      if (pointer -> logblock % volex -> homeblock.clusterfactor != 0) {
        oz_dev_vdfs_printk (iopex, "oz_dev_dfs validate_header: pointer has logblock %u not multiple of clusterfactor\n", pointer -> logblock);
        return (0);
      }
      if (pointer -> logblock / volex -> homeblock.clusterfactor >= volex -> homeblock.clustertotal) {
        oz_dev_vdfs_printk (iopex, "oz_dev_dfs validate_header: pointer has logblock %u off end of disk\n", pointer -> logblock);
        return (0);
      }
      if (pointer -> blockcount / volex -> homeblock.clusterfactor + pointer -> logblock / volex -> homeblock.clusterfactor > volex -> homeblock.clustertotal) {
        oz_dev_vdfs_printk (iopex, "oz_dev_dfs validate_header: pointer has logblock %u+%u off end of disk\n", pointer -> logblock, pointer -> blockcount);
        return (0);
      }
    }
  }
  return (1);
}
