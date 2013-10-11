#include "fs.h"
#include "ext.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* constant of how many bits in one freemap entry */
#define SFS_NBITS_IN_FREEMAP_ENTRY (sizeof(u32)*8)

/* in-memory superblock (in consistent with the disk copy) */
static sfs_superblock_t sb;
/* freemap, u32 array (in consistent with the disk copy) */
static u32 *freemap;
/* file descriptor table */
static fd_struct_t fdtable[SFS_MAX_OPENED_FILES];

/* 
 * Flush the in-memory freemap to disk 
 */
static void sfs_flush_freemap()
{
	printf("sfs_flush_freemap\n");
	size_t i;
	blkid bid = 1;
	char *p = (char *)freemap;
	/* TODO: write freemap block one by one */
	i = sb.nfreemap_blocks;
	int j;
	for (j=1; j <= i; j++)
	{
		sfs_write_block(&p, j);
	}
}

/* 
 * Allocate a free block, mark it in the freemap and flush the freemap to disk
 */
static blkid sfs_alloc_block()
{
	u32 size = sb.nfreemap_blocks * BLOCK_SIZE / sizeof(u32);	
	u32 i, j;
	/* TODO: find a freemap entry that has a free block */
	/* TODO: find out which bit in the entry is zero,
	   set the bit, flush and return the bid
	*/
	printf("sfs_alloc_block\n");
	for(i=1; i<size; i++)
	{
		printf("i = %d\n", i);
		printf("freemap = %d\n", &freemap);
		j = (*freemap & (1 << i));
		printf("here\n");
		j = j >> i;
		printf("j = %d\n", j);
		if (j == 0)
		{
			// Found a free block
			freemap = (*freemap | (1 << i));
			sfs_flush_freemap();
			printf("after flushing\n");

			return i;
		}
	}
	
	return 0;
}

/*
 * Free a block, unmark it in the freemap and flush
 */
static void sfs_free_block(blkid bid)
{
	/* TODO find the entry and bit that correspond to the block */
	int entry_loc;
	int bit_loc;
	/* TODO unset the bit and flush the freemap */
	freemap = (*freemap & ~(1 << entry_loc));
	sfs_flush_freemap();
}

/* 
 * Resize a file.
 * This file should be opened (in the file descriptor table). The new size
 * should be larger than the old one (not supposed to shrink a file)
 */
static void sfs_resize_file(int fd, u32 new_size)
{
	/* the length of content that can be hold by a full frame (in bytes) */
	int frame_size = BLOCK_SIZE * SFS_FRAME_COUNT;
	/* old file size */
	int old_size = fdtable[fd].inode.size;
	/* how many frames are used before resizing */
	int old_nframe = (old_size + frame_size -1) / frame_size;
	/* how many frames are required after resizing */
	int new_nframe = (new_size + frame_size - 1) / frame_size;
	int i, j;
	blkid frame_bid = 0;
	sfs_inode_frame_t frame;

	/* TODO: check if new frames are required */
	
	/* TODO: allocate a full frame */

	/* TODO: add the new frame to the inode frame list
	   Note that if the inode is changed, you need to write it to the disk
	*/
}

/*
 * Get the bids of content blocks that hold the file content starting from cur
 * to cur+length. These bids are stored in the given array.
 * The caller of this function is supposed to allocate the memory for this
 * array. It is guaranteed that cur+length<size
 * 
 * This function returns the number of bids being stored to the array.
 */
static u32 sfs_get_file_content(blkid *bids, int fd, u32 cur, u32 length)
{
	/* the starting block of the content */
	u32 start;
	/* the ending block of the content */
	u32 end;
	u32 i;
	sfs_inode_frame_t frame;

	/* TODO: find blocks between start and end.
	   Transverse the frame list if needed
	*/
	return 0;
}

/*
 * Find the directory of the given name.
 *
 * Return block id for the directory or zero if not found
 */
static blkid sfs_find_dir(char *dirname)
{
	blkid dir_bid = 0;
	sfs_dirblock_t dir;
	/* TODO: start from the sb.first_dir, treverse the linked list */
	sfs_superblock_t *sb = sfs_print_info();
	dir_bid = sb->first_dir;
	while (dir_bid != 0) 
	{
		sfs_read_block((char*)&dir, dir_bid);
		if (dir.dir_name == dirname)
		{
			return dir_bid;
		}
		dir_bid = dir.next_dir;
	}
	return 0;
}

/*
 * Create a SFS with one superblock, one freemap block and 1022 data blocks
 *
 * The freemap is initialized be 0x3(11b), meaning that
 * the first two blocks are used (sb and the freemap block).
 *
 * This function always returns zero on success.
 */
int sfs_mkfs()
{
	/* one block in-memory space for freemap (avoid malloc) */
	static char freemap_space[BLOCK_SIZE];
	int i;
	sb.magic = SFS_MAGIC;
	sb.nblocks = 1024;
	sb.nfreemap_blocks = 1;
	sb.first_dir = 0;
	for (i = 0; i < SFS_MAX_OPENED_FILES; ++i) {
		/* no opened files */
		fdtable[i].valid = 0;
	}
	sfs_write_block(&sb, 0);
	freemap = (u32 *)freemap_space;
	memset(freemap, 0, BLOCK_SIZE);
	/* just to enlarge the whole file */
	sfs_write_block(freemap, sb.nblocks);
	/* initializing freemap */
	freemap[0] = 0x3; /* 11b, freemap block and sb used*/
	sfs_write_block(freemap, 1);
	memset(&sb, 0, BLOCK_SIZE);
	printf("freemap = %d\n", freemap);
	return 0;
}

/*
 * Load the super block from disk and print the parameters inside
 */
sfs_superblock_t *sfs_print_info()
{
	/* TODO: load the superblock from disk and print*/
	sfs_read_block((char*)&sb, 0);
	printf("super block : nblocks = %d , nfreemap_blocks = %d , first_dir = %d\n", sb.nblocks, sb.nfreemap_blocks, sb.first_dir);
	return &sb;
}

/*
 * Create a new directory and return 0 on success.
 * If the dir already exists, return -1.
 */
int sfs_mkdir(char *dirname)
{
	/* TODO: test if the dir exists */
	/* TODO: insert a new dir to the linked list */
	blkid current;
	sfs_dirblock_t dir;
	blkid bid = sfs_find_dir(dirname);

	if (bid == 0)
	{
		printf("Could not find the directory, so will allocate new block.\n");
		bid = sfs_alloc_block();
		printf("Make new directory with the block id = %d\n", bid);
		// Add the new directory to the directory linked list
		sfs_superblock_t *sb = sfs_print_info();
		current = sb->first_dir;
		while(current != 0) {
			sfs_read_block((char*)&dir, current);
			current = dir.next_dir;
		}
		sb->first_dir = bid;
		sfs_write_block((char*)&dir, bid);
	}	
	return -1;
}

/*
 * Remove an existing empty directory and return 0 on success.
 * If the dir does not exist or still contains files, return -1.
 */
int sfs_rmdir(char *dirname)
{
	/* TODO: check if the dir exists */
	/* TODO: check if no files */
	/* TODO: go thru the linked list and delete the dir*/
	return 0;
}

/*
 * Print all directories. Return the number of directories.
 */
int sfs_lsdir()
{
	/* TODO: go thru the linked list */
	int count;
	sfs_dirblock_t dir;
	blkid bid = sb.first_dir;
	printf("inside the ls bid = %d\n", bid);
	while (bid != 0)
	{
		sfs_read_block((char*)&dir, bid);
		bid = dir.next_dir;
		printf("WHHHHHHATTTT\n");
		printf("next_dir = %d , dir_name = ");
		int i;
		for(i=0; i<sizeof(dir.dir_name); i++)
		{
			printf("%c", ((char *)dir.dir_name)[i]);
		}
		printf("\n");

		count++;
	}
	return 0;
}

/*
 * Open a file. If it does not exist, create a new one.
 * Allocate a file desriptor for the opened file and return the fd.
 */
int sfs_open(char *dirname, char *name)
{
	blkid dir_bid = 0, inode_bid = 0;
	sfs_inode_t *inode;
	sfs_dirblock_t dir;
	int fd;
	int i;

	/* TODO: find a free fd number */
	
	/* TODO: find the dir first */

	/* TODO: traverse the inodes to see if the file exists.
	   If it exists, load its inode. Otherwise, create a new file.
	*/

	/* TODO: create a new file */
	return fd;
}

/*
 * Close a file. Just mark the valid field to be zero.
 */
int sfs_close(int fd)
{
	/* TODO: mark the valid field */
	return 0;
}

/*
 * Remove/delete an existing file
 *
 * This function returns zero on success.
 */
int sfs_remove(int fd)
{
	blkid frame_bid;
	sfs_dirblock_t dir;
	int i;

	/* TODO: update dir */

	/* TODO: free inode and all its frames */

	/* TODO: close the file */
	return 0;
}

/*
 * List all the files in all directories. Return the number of files.
 */
int sfs_ls()
{
	/* TODO: nested loop: traverse all dirs and all containing files*/
	return 0;
}

/*
 * Write to a file. This function can potentially enlarge the file if the 
 * cur+length exceeds the size of file. Also you should be aware that the
 * cur may already be larger than the size (due to sfs_seek). In such
 * case, you will need to expand the file as well.
 * 
 * This function returns number of bytes written.
 */
int sfs_write(int fd, void *buf, int length)
{
	int remaining, offset, to_copy;
	blkid *bids;
	int i, n;
	char *p = (char *)buf;
	char tmp[BLOCK_SIZE];
	u32 cur = fdtable[fd].cur;

	/* TODO: check if we need to resize */
	
	/* TODO: get the block ids of all contents (using sfs_get_file_content() */

	/* TODO: main loop, go through every block, copy the necessary parts
	   to the buffer, consult the hint in the document. Do not forget to 
	   flush to the disk.
	*/
	/* TODO: update the cursor and free the temp buffer
	   for sfs_get_file_content()
	*/
	return 0;
}

/*
 * Read from an opend file. 
 * Read can not enlarge file. So you should not read outside the size of 
 * the file. If the read exceeds the file size, its result will be truncated.
 *
 * This function returns the number of bytes read.
 */
int sfs_read(int fd, void *buf, int length)
{
	int remaining, to_copy, offset;
	blkid *bids;
	int i, n;
	char *p = (char *)buf;
	char tmp[BLOCK_SIZE];
	u32 cur = fdtable[fd].cur;

	/* TODO: check if we need to truncate */
	/* TODO: similar to the sfs_write() */
	return 0;
}

/* 
 * Seek inside the file.
 * Loc is the starting point of the seek, which can be:
 * - SFS_SEEK_SET represents the beginning of the file.
 * - SFS_SEEK_CUR represents the current cursor.
 * - SFS_SEEK_END represents the end of the file.
 * Relative tells whether to seek forwards (positive) or backwards (negative).
 * 
 * This function returns 0 on success.
 */
int sfs_seek(int fd, int relative, int loc)
{
	/* TODO: get the old cursor, change it as specified by the parameters */
	return 0;
}

/*
 * Check if we reach the EOF(end-of-file).
 * 
 * This function returns 1 if it is EOF, otherwise 0.
 */
int sfs_eof(int fd)
{
	/* TODO: check if the cursor has gone out of bound */
	return 0;
}
