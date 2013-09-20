#include "mydisk.h"
#include <string.h>

FILE *thefile;     /* the file that stores all blocks */
int max_blocks;    /* max number of blocks given at initialization */
int disk_type;     /* disk type, 0 for HDD and 1 for SSD */
int cache_enabled = 0; /* is cache enabled? 0 no, 1 yes */

int mydisk_init(char const *file_name, int nblocks, int type)
{
	/* TODO: 1. use the proper mode to open the disk file
	 * 2. fill zeros 
	 */
	printf("Starting ... \n");
	thefile = fopen(file_name, "r+");

	//TODO: check that the parameters are correct, otherwise return 1
	printf("max blocks = %d\n", nblocks);
	max_blocks = nblocks;
	printf("disk type = %d\n", type);
	disk_type = type;
	printf("BLOCK_SIZE = %d\n", BLOCK_SIZE);

	if (thefile == NULL) {
		printf("file does not exist\n");
		thefile = fopen(file_name, "w+");
	} else {
		printf("file does exist YAY!!!\n");
	}

	char *buffer = (char*)malloc(max_blocks * BLOCK_SIZE); 
	memset(buffer, '0', max_blocks * BLOCK_SIZE);
	if(fwrite(buffer, BLOCK_SIZE, max_blocks, thefile) != max_blocks) {
		printf("ERROR\n");
		return 1;
	} else {
		printf("Good for now ...\n");
	}

	int i;
	for(i=0; i< (max_blocks * BLOCK_SIZE); i++) {
		printf("%c", buffer[i]);
		if(i%BLOCK_SIZE == 0 && i!=0) {
			printf("\n");
		}
	}
	printf("\n");

	char *b = (char *)malloc(BLOCK_SIZE * max_blocks);
	fread(b, BLOCK_SIZE, 1, thefile);

	return 0;
}

void mydisk_close()
{
	/* TODO: clean up whatever done in mydisk_init()*/
	fclose(thefile);

}

int mydisk_read_block(int block_id, void *buffer)
{
	printf("\n*** Read block ***\n");
	printf("block id = %d\n", block_id);

	if (block_id >= max_blocks) {
		printf("ERROR: block id greater than max blocks");
		return 1;
	}

	if (cache_enabled) {
		/* TODO: 1. check if the block is cached
		 * 2. if not create a new entry for the block and read from disk
		 * 3. fill the requested buffer with the data in the entry 
		 * 4. return proper return code
		 */
		 printf("cache enabled");
		 
		return 0;
	} else {
		/* TODO: use standard C functiosn to read from disk
		 */

		fseek(thefile, block_id, SEEK_SET);
		fread(buffer, BLOCK_SIZE, 1, thefile);

		int i;
		for(i=0; i< (13); i++) {
			printf("%c", ((char*)buffer)[i]);
			if(i%BLOCK_SIZE == 0 && i!=0) {
				printf("\n");
			}
		}
		printf("\n");

		printf("\ntesting -> %d\n", memcmp(buffer, "hello world\n", 13));

		printf("--- End read ---\n");
		return 0;
	}
}

int mydisk_write_block(int block_id, void *buffer)
{
	/* TODO: this one is similar to read_block() except that
	 * you need to mark it dirty
	 */
	printf("\n*** Write block ***\n");
	printf("block id = %d\n", block_id);

	int i;
	char ch;

	fseek(thefile, 0, SEEK_SET);
	if(fwrite(buffer, BLOCK_SIZE, 1, thefile) != 1) {
		printf("ERROR\n");
		return 1;
	} else {
		printf("Good for now ...\n");
	}

	fseek(thefile, 0, SEEK_SET);
	// while( ( ch = fgetc(thefile) ) != EOF ) {
 //    	printf("%c",ch);
 //  	}

  	fseek(thefile, 0, SEEK_SET);
	for( i=0; i<26; i++) {
    	ch = fgetc(thefile);
    	printf("%c",ch);
  	}

	printf("\ntesting -> %d\n", memcmp(buffer, "hello world\n", 13));
	printf("--- End write ---\n");
	return 0;
}

int mydisk_read(int start_address, int nbytes, void *buffer)
{
	int offset, remaining, amount, block_id;
	int cache_hit = 0, cache_miss = 0;

	/* TODO: 1. first, always check the parameters
	 * 2. a loop which process one block each time
	 * 2.1 offset means the in-block offset
	 * amount means the number of bytes to be moved from the block
	 * (starting from offset)
	 * remaining means the remaining bytes before final completion
	 * 2.2 get one block, copy the proper portion
	 * 2.3 update offset, amount and remaining
	 * in terms of latency calculation, monitor if cache hit/miss
	 * for each block access
	 */
	return 0;
}

int mydisk_write(int start_address, int nbytes, void *buffer)
{
	/* TODO: similar to read, except the partial write problem
	 * When a block is modified partially, you need to first read the block,
	 * modify the portion and then write the whole block back
	 */
	return 0;
}
