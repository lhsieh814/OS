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
	size_t i;
	blkid bid = 1;
	char *p = (char *)freemap;
	/* TODO: write freemap block one by one */
	i = sb.nfreemap_blocks;
	int j;
	for (j = 1; j <= i; j++)
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
	u32 i, j, k;
	/* TODO: find a freemap entry that has a free block */
	/* TODO: find out which bit in the entry is zero,
	   set the bit, flush and return the bid
	*/
	for (i=0; i<size; i++)
	{
		for (k = 0; k < (sizeof(u32) * 8); k++) 
		{
			j = (freemap[i] & (1 << k));
			j = j >> k;
			if (j == 0)
			{
				// Found a free block
				freemap[i] |= (1 << k);
				sfs_flush_freemap();

				return (i * sizeof(u32) + k);
			}
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
	entry_loc = bid / SFS_NBITS_IN_FREEMAP_ENTRY;
	bit_loc = bid % SFS_NBITS_IN_FREEMAP_ENTRY;

	/* TODO unset the bit and flush the freemap */
	freemap[entry_loc] = (freemap[entry_loc] & ~(1 << bit_loc));
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
	blkid bid, content_bid;

	/* TODO: check if new frames are required */
	if (new_nframe - old_nframe > 0)
	{
		/* TODO: allocate a full frame */
		frame_bid = sfs_alloc_block();
		for (j = 0; j < SFS_FRAME_COUNT; j++)
		{
			content_bid = sfs_alloc_block();
			frame.content[j] = content_bid;

			// Empty content block
			char tmp[BLOCK_SIZE];
			memset(tmp, 0, BLOCK_SIZE);
			sfs_write_block((char*)&tmp, content_bid);
		}

		/* TODO: add the new frame to the inode frame list
		   Note that if the inode is changed, you need to write it to the disk
		*/
		bid = fdtable[fd].inode.first_frame;
		if (bid == 0)
		{
			fdtable[fd].inode.first_frame = frame_bid;
			sfs_write_block((char*)&fdtable[fd].inode, fdtable[fd].inode_bid);
		} 
		else 
		{
			for (i = 0; i < SFS_FRAME_COUNT; i++)
			{
				if (bid == 0)
				{
					frame.next = frame_bid;
					break;
				}
				sfs_read_block((char*)&frame, bid);
				bid = frame.next;

			}
		}
		// Write the new frame block
		sfs_write_block((char*)&frame, frame_bid);
	}

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
	u32 start = cur / BLOCK_SIZE;
	/* the ending block of the content */
	u32 end = (cur + length) / BLOCK_SIZE;
	u32 i;
	sfs_inode_frame_t frame;
	blkid bid;

	/* TODO: find blocks between start and end.
	   Transverse the frame list if needed
	*/
	bid = fdtable[fd].inode.first_frame;
	sfs_read_block((char*)&frame, bid);

	for (i = start; i <= end; i++)
	{
		bids[i-start] = frame.content[i];
	}

	return (end - start + 1);
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

	dir_bid = sb.first_dir;
	while (dir_bid != 0) 
	{
		sfs_read_block((char*)&dir, dir_bid);
		// Check if the dirnames are equal
		if (!strncmp(dir.dir_name, dirname, sizeof(dirname)))
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
	blkid nextBlkid;
	sfs_dirblock_t dir, new_dir;

	// Try finding the directory if it exists
	blkid bid = sfs_find_dir(dirname);

	if (bid == 0)
	{
		// Directory does not exists, create a new one
		bid = sfs_alloc_block();
		// Add the new directory to the directory linked list
		nextBlkid = sb.first_dir;

		if (nextBlkid == 0) {
			sb.first_dir = bid;
			sfs_write_block((char*)&sb, 0);
		} 
		else 
		{
			sfs_read_block((char*)&dir, nextBlkid);

			while (dir.next_dir != 0) 
			{
				nextBlkid = dir.next_dir;
				sfs_read_block((char*)&dir, nextBlkid);
			}

			dir.next_dir = bid;
			sfs_write_block((char*)&dir, nextBlkid);
		}
		// Create new directory with next_dir=0 and dir_name
		new_dir.next_dir = 0;
		memset(new_dir.dir_name, 0, sizeof(new_dir.dir_name));	// Empty dir_name
		memset(new_dir.inodes, 0, sizeof(new_dir.inodes));	// Empty inodes
		strcpy(new_dir.dir_name, dirname);	// Copy new dir_name
		sfs_write_block((char*)&new_dir, bid);
		
		return 0;
	}	

	// Directory already exists
	return -1;
}

/*
 * Remove an existing empty directory and return 0 on success.
 * If the dir does not exist or still contains files, return -1.
 */
int sfs_rmdir(char *dirname)
{
	sfs_dirblock_t currentDir, prevDir, nextDir;
	blkid bid, prevBlkid, nextBlkid;

	/* TODO: check if the dir exists */
	bid = sfs_find_dir(dirname);
	if (bid != 0) {
		// Directory exists
		sfs_read_block((char*)&currentDir, bid);

		/* TODO: check if no files */
		int i;
		for (i = 0; i < SFS_DB_NINODES; i++) 
		{
			if (currentDir.inodes[i] != 0) {
				// directory is not empty
				printf("ERROR: inodes not empty = %d\n", currentDir.inodes[i]);
				return -1;
			}
		}

		/* TODO: go thru the linked list to delete the dir*/
		// Need to change the next_dir pointer of the previous dir and dir to be deleted
		nextBlkid = sb.first_dir;
		if (nextBlkid == bid) {
			// Deleting the root directory
			sb.first_dir = 0;
			sfs_read_block((char*)&currentDir, nextBlkid);						
			currentDir.next_dir = 0;
			memset(currentDir.dir_name, 0, sizeof(currentDir.dir_name));
			return 0;
		} 
		else 
		{
			while (nextBlkid != bid)
			{
				prevBlkid = nextBlkid;
				sfs_read_block((char*)&prevDir, nextBlkid);			
				nextBlkid = prevDir.next_dir;
			}
			nextBlkid = currentDir.next_dir;
		}

		// Remove the directory: update the linked list next_dir variable
		prevDir.next_dir = nextBlkid;
		currentDir.next_dir = 0;
		// Remove the dir_name of the dir to be deleted
		memset(currentDir.dir_name, 0, sizeof(currentDir.dir_name));

		// Flush the removed directory and prevDir
		sfs_write_block((char*)&currentDir, bid);
		sfs_write_block((char*)&prevDir, prevBlkid);
		// Update the freemap so that deleted dir is empty
		sfs_free_block(bid);

		return 0;
	}
	return -1;
}

/*
 * Print all directories. Return the number of directories.
 */
int sfs_lsdir()
{
	/* TODO: go thru the linked list */
	int count = 0;
	int i;
	sfs_dirblock_t dir;
	blkid bid = sb.first_dir;

	while (bid != 0)
	{
		sfs_read_block((char*)&dir, bid);
		printf("\tdir_name = ");
		
		for (i = 0; i < sizeof(dir.dir_name); i++)
		{
			printf("%c", ((char *)dir.dir_name)[i]);
		}
		printf("\t\tdir_bid = %d\tnext_dir = %d", bid, dir.next_dir);
		printf("\n");
		bid = dir.next_dir;
		count++;
	}
	return count;
}

/*
 * Open a file. If it does not exist, create a new one.
 * Allocate a file desriptor for the opened file and return the fd.
 */
int sfs_open(char *dirname, char *name)
{
	blkid dir_bid = 0, inode_bid = 0;
	sfs_inode_t *inode, new_inode;
	sfs_dirblock_t dir;
	int fd;
	int i;

	/* TODO: find a free fd number */
	for (i = 0; i < SFS_MAX_OPENED_FILES; i++)
	{
		if (fdtable[i].valid == 0) 
		{
			// Found a free fd
			fd = i;
			break;
		}
	}

	/* TODO: find the dir first */
	dir_bid = sfs_find_dir(dirname);
	if (dir_bid == 0)
	{
		printf("ERROR: the directory does not exists\n");
		return -1;
	}
	sfs_read_block((char*)&dir, dir_bid);

	/* TODO: traverse the inodes to see if the file exists.
	   If it exists, load its inode. Otherwise, create a new file.
	*/
	for (i = 0; i < SFS_DB_NINODES; i++) 
	{
		if (dir.inodes[i] != 0)
		{
			sfs_read_block((char*)&new_inode, dir.inodes[i]);
			
			if (strcmp(new_inode.file_name, name) == 0) {
				inode_bid = dir.inodes[i];
				break;
			}
		}
	}

	if (inode_bid == 0)
	{
		// Allocate a block to store new inode
		inode_bid = sfs_alloc_block();
		// Find empty space in dir.inodes[] and set inode_bid
		for (i=0; i<SFS_DB_NINODES; i++)
		{
			if (dir.inodes[i] == 0) 
			{
				dir.inodes[i] = inode_bid;
				break;
			}
		}
		// Did not find the inode, create a new one
		new_inode.first_frame = 0;
		new_inode.size = 0;
		memset(new_inode.file_name, 0, sizeof(new_inode.file_name));
		strcpy(new_inode.file_name, name);
		sfs_write_block((char*)&new_inode, inode_bid);
		sfs_write_block((char*)&dir, dir_bid);
	}

	/* TODO: create a new file */
	fdtable[fd].dir_bid = dir_bid;
	fdtable[fd].inode_bid = inode_bid;
	fdtable[fd].inode = new_inode;
	fdtable[fd].cur = 0;
	fdtable[fd].valid = 1;

	return fd;
}

/*
 * Close a file. Just mark the valid field to be zero.
 */
int sfs_close(int fd)
{
	/* TODO: mark the valid field */
	fdtable[fd].valid = 0;
	return 0;
}

/*
 * Remove/delete an existing file
 *
 * This function returns zero on success.
 */
int sfs_remove(int fd)
{
	blkid frame_bid, next_frame_bid;
	sfs_dirblock_t dir;
	sfs_inode_frame_t frame;
	int i, j;
	char *tmp;

	/* TODO: update dir */
	sfs_read_block((char*)&dir, fdtable[fd].dir_bid);
	for (i = 0; i < SFS_DB_NINODES; i++)
	{
		if (dir.inodes[i] == fdtable[fd].inode_bid)
		{
			dir.inodes[i] = 0;
		}
	}
	// Write dir block to update inode values
	sfs_write_block((char*)&dir, fdtable[fd].dir_bid);

	/* TODO: free inode and all its frames */
	frame_bid = fdtable[fd].inode.first_frame;
	
	while (frame_bid != 0)
	{
		sfs_read_block((char*)&frame, frame_bid);

		// Free all the content blocks for each frame
		for (j = 0; j < SFS_FRAME_COUNT; j++)
		{
			tmp = (char*)malloc(BLOCK_SIZE);
			memset(tmp, 0, BLOCK_SIZE);
			sfs_write_block(tmp, frame.content[j]);
			sfs_free_block(frame.content[j]);
			free(tmp);
		}

		next_frame_bid = frame.next;
		memset(&frame, 0, BLOCK_SIZE);
		sfs_write_block((char*)&frame, frame_bid);
		sfs_free_block(frame_bid);
		frame_bid = next_frame_bid;
	}
	
	// Free the inode
	memset((char*)&fdtable[fd].inode, 0, BLOCK_SIZE);

	sfs_write_block((char*)&fdtable[fd].inode, fdtable[fd].inode_bid);
	sfs_free_block(fdtable[fd].inode_bid);

	/* TODO: close the file */
	sfs_close(fd);

	return 0;
}

/*
 * List all the files in all directories. Return the number of files.
 */
int sfs_ls()
{
	/* TODO: nested loop: traverse all dirs and all containing files */
	int count = 0;
	int i, j, k;
	sfs_dirblock_t dir;
	sfs_inode_t inode;
	blkid bid;

	bid = sb.first_dir;
	while (bid != 0) {
		sfs_read_block((char*)&dir, bid);
		
		for (j = 0; j<sizeof(dir.dir_name); j++)
		{
			printf("%c", dir.dir_name[j]);
		}
		printf("\n");
		
		for (i = 0; i < SFS_DB_NINODES; i++)
		{
			if (dir.inodes[i] != 0)
			{
				sfs_read_block((char*)&inode, dir.inodes[i]);
				
				printf("\t");
				for (k = 0; k < sizeof(inode.file_name); k++)
				{
					printf("%c", inode.file_name[k]);
				}
				printf("\tsize = %d\n", inode.size);
				count++;
			}
		}
		bid = dir.next_dir;
	}
	printf("count = %d\n", count);
	return count;
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
	int frame_size, new_nframe, old_nframe;

	/* TODO: check if we need to resize */
	frame_size = BLOCK_SIZE * SFS_FRAME_COUNT;
	old_nframe =  (fdtable[fd].inode.size + frame_size-1)/frame_size;
	new_nframe = ((cur+length) + frame_size-1)/frame_size;

	if (new_nframe > old_nframe) 
	{
		// Need to allocate a new frame
		sfs_resize_file(fd, fdtable[fd].inode.size + length);
	}
	/* TODO: get the block ids of all contents (using sfs_get_file_content() */
	bids = malloc(length/BLOCK_SIZE + 1);
	n = sfs_get_file_content(bids, fd, cur, length);

	/* TODO: main loop, go through every block, copy the necessary parts
	   to the buffer, consult the hint in the document. Do not forget to 
	   flush to the disk.
	*/
	remaining = length;
	offset = cur % BLOCK_SIZE;
	if (BLOCK_SIZE - offset > remaining)
	{
		to_copy = remaining;
	}
	else
	{
		to_copy = BLOCK_SIZE - offset;
	}
	for (i = 0; i < n; i++)
	{
		memcpy(tmp + offset , buf, to_copy);
		
		sfs_write_block((char*)&tmp, bids[i]);
		remaining -= to_copy;
		offset = 0;
		
		if (remaining > BLOCK_SIZE)
		{
			to_copy = BLOCK_SIZE - offset;
		} 
		else 
		{
			to_copy = remaining;
		}
	}
	/* TODO: update the cursor and free the temp buffer
	   for sfs_get_file_content()
	*/
	fdtable[fd].cur += length;
	if (fdtable[fd].inode.size < fdtable[fd].cur)
	{
		fdtable[fd].inode.size = fdtable[fd].cur;
	} 

	sfs_write_block((char*)&(fdtable[fd].inode), fdtable[fd].inode_bid);

	free(bids);

	return length;
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
	if ((cur + length) > fdtable[fd].inode.size)
	{
		// Need to truncate, remove the last portion
		length = fdtable[fd].inode.size - length;
	}

	/* TODO: similar to the sfs_write() */
	// Get the block ids of content
	bids = malloc(length/BLOCK_SIZE + 1);
	n = sfs_get_file_content(bids, fd, cur, length);
	// Read operation
	remaining = length;
	offset = cur % BLOCK_SIZE;
	if (BLOCK_SIZE - offset > remaining)
	{
		to_copy = remaining;
	}
	else
	{
		to_copy = BLOCK_SIZE - offset;
	}

	for (i = 0; i < n; i++)
	{
		sfs_read_block((char*)&tmp, bids[i]);
		memcpy(buf, tmp+offset, to_copy);
		remaining -= to_copy;
		offset = 0;
		if (remaining > BLOCK_SIZE)
		{
			to_copy = BLOCK_SIZE - offset;
		} else {
			to_copy = remaining;
		}
	}
 	// Update variables
	fdtable[fd].cur += length;

	free(bids);

	return length;
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
	u32 cur = fdtable[fd].cur;

	switch (loc) {
		case SFS_SEEK_SET:
			cur = 0;
			break;
		case SFS_SEEK_CUR:
			cur = fdtable[fd].cur;
			break;
		case SFS_SEEK_END:
			cur = fdtable[fd].inode.size;
			break;
		default:
			// Invalid input for loc
			printf("ERROR: invalid input for loc\n");
			return 1;
	}
	cur = cur + relative;
	fdtable[fd].cur = cur;

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
	if (fdtable[fd].cur == fdtable[fd].inode.size)
	{
		// Cursor is out of bound
		return 1;
	}

	return 0;
}
