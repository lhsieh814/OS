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

	}
	free(buffer);
	
	return 0;
}

void mydisk_close()
{
	/* TODO: clean up whatever done in mydisk_init()*/
	fclose(thefile);

}

int mydisk_read_block(int block_id, void *buffer)
{
	printf("\n*** Start read block ***\n");
	// printf("block id = %d\n", block_id);

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
		printf("cache enabled for read\n");
		int i;

		// char *x = get_cached_block(block_id);
		if (get_cached_block(block_id) == NULL) {
			printf("Cache miss\n");

			fseek(thefile, block_id * BLOCK_SIZE, SEEK_SET);
			fread(buffer, BLOCK_SIZE, 1, thefile);
printf("before-->\n");
print();
			memcpy(create_cached_block(block_id), buffer, BLOCK_SIZE);
printf("EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n");
print();
// char *x = get_cached_block(block_id);
// for (i=0; i<BLOCK_SIZE; i++) {
// 	printf("%c", x[i]);
// }
// printf("\n");
			// Cache miss
			return 0;
		} else {
			printf("Cache hit\n");
char *x = get_cached_block(block_id);
for(i=0;i<BLOCK_SIZE;i++){
	printf("%c", x[i]);
}
printf("\n");
			memcpy(buffer, get_cached_block(block_id), BLOCK_SIZE);

			// Cache hit
			return -1;
		}
	} else {
		/* TODO: use standard C functiosn to read from disk
		 */

		fseek(thefile, block_id * BLOCK_SIZE, SEEK_SET);
		fread(buffer, BLOCK_SIZE, 1, thefile);

		// int i;
		// for(i=0; i<BLOCK_SIZE; i++) {
		// 	printf("%c", ((char*)buffer)[i]);
		// 	if(i%BLOCK_SIZE == 0 && i!=0) {
		// 		printf("\n");
		// 	}
		// }

		// printf("\n*** End read block ***\n");
		return 0;
	}
}

int mydisk_write_block(int block_id, void *buffer)
{
	/* TODO: this one is similar to read_block() except that
	 * you need to mark it dirty
	 */
	printf("\n--- Start write block ---\n");
	// printf("block id = %d\n", block_id);
	
	if (block_id >= max_blocks) {
		printf("ERROR: block id greater than max blocks");
		return 1;
	}

	if (cache_enabled) {
		printf("cache enabled for write\n");
		int i;
		// char tmp[BLOCK_SIZE];
		// char *x;
		// x = &tmp;

		// char *x = malloc(BLOCK_SIZE);
		// print();
		// *x = get_cached_block(block_id);

		char *y[BLOCK_SIZE];
		*y = get_cached_block(block_id);

		if (*y == NULL) {
			printf("cache miss for write\n");
			
			mark_dirty(block_id);
			memcpy(create_cached_block(block_id), buffer, BLOCK_SIZE);
printf("_______________\n");
char *z = get_cached_block(block_id);
for(i=0; i<BLOCK_SIZE; i++) {
	printf("%c", z[i]);
}
printf("________________\n");
			return 0;
		} else {
printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
			printf("cache hit for write\n");
printf("+++++++++ write this ++++++++\n");
for(i=0; i<BLOCK_SIZE; i++) {
	printf("%c", ((char*)buffer)[i]);
}
printf("\n+++++++++++++++++\n");
			memcpy(get_cached_block(block_id), buffer, BLOCK_SIZE);
printf("_______ should be same as top ________\n");
get_cached_block(block_id);
printf("\n________________\n");
			return -1;
		}

	} else {
		int i;
		char ch;

		fseek(thefile, block_id * BLOCK_SIZE, SEEK_SET);
		if(fwrite(buffer, BLOCK_SIZE, 1, thefile) != 1) {
			printf("ERROR\n");
			return 1;
		} else {
			return 0;
		}
	}
	// fseek(thefile, 0, SEEK_SET);
	// i = 0;
	// while( ( ch = fgetc(thefile) ) != EOF ) {
 //    	printf("%c",ch);
 //    	if (i % BLOCK_SIZE == 0 && (i!=0)) {
 //    		printf("\n");
 //    	}
 //    	i++;
 //  	}

	// printf("\n--- End write block ---\n");
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
	// printf("\n### Start read ###\n");

	if ( !(start_address >= 0) || 
		!(start_address <= (start_address + nbytes)) ||
	 	!(start_address + nbytes < max_blocks * BLOCK_SIZE)) {
	 	printf("ERROR: invalid parameters");
		return 1;
	}

	char tmp[BLOCK_SIZE];
	int start_block = start_address / BLOCK_SIZE;
	int end_block = (start_address + nbytes) / BLOCK_SIZE;
	// printf("start_block = %d\nend_block = %d\n", start_block, end_block);

	remaining = nbytes;
	offset = start_address % BLOCK_SIZE;
	if ((BLOCK_SIZE - offset) > remaining) {
	 	amount = remaining;
	} else {
 		amount = BLOCK_SIZE - offset;
	}

	int i;
	for (i=start_block; i<=end_block; i++) {
	 	
	 	mydisk_read_block(i, tmp);
	 	memcpy(buffer + (nbytes-remaining), tmp + offset, amount);

	 	remaining = remaining - amount;
	 	offset = 0;
	 	if (remaining - BLOCK_SIZE < 0) {
	 		amount = remaining;
	 	} else {
	 		amount = BLOCK_SIZE;
	 	}
	 	// printf("remaining = %d , offset = %d , amount = %d\n", remaining, offset, amount);
	}

	// printf("### End read ###\n");
	return 0;
}

int mydisk_write(int start_address, int nbytes, void *buffer)
{
	/* TODO: similar to read, except the partial write problem
	 * When a block is modified partially, you need to first read the block,
	 * modify the portion and then write the whole block back
	 */
	// printf("\n$$$ Start write $$$\n");

	int offset, remaining, amount, block_id;

	if ( !(start_address >= 0) || 
		!(start_address <= (start_address + nbytes)) ||
	 	!(start_address + nbytes < max_blocks * BLOCK_SIZE)) {
	 	printf("ERROR: invalid parameters");
	 	return 1;
	}
	char tmp[BLOCK_SIZE];
	int start_block = start_address / BLOCK_SIZE;
	int end_block = (start_address + nbytes) / BLOCK_SIZE;
	printf("start_block = %d\nend_block = %d\n", start_block, end_block);

	remaining = nbytes;
	offset = start_address % BLOCK_SIZE;
	if ((BLOCK_SIZE - offset) > remaining) {
	 	amount = remaining;
	} else {
 		amount = BLOCK_SIZE - offset;
	}
	// printf("OLD -> remaining = %d , offset = %d , amount = %d \n", remaining, offset, amount);

	int i;
	for (i=start_block; i<=end_block; i++) {
		printf("round %d\n", i);
		mydisk_read_block(i, tmp);

		// printf("tmp + %d\n", offset);
		// printf("buffer + %d\n", (nbytes - remaining));
		// printf("amount = %d\n", amount);
		memcpy(tmp + offset, buffer + (nbytes - remaining), amount);

		printf("buffer[nbytes-remaining] = %c\n", ((char*)buffer)[nbytes-remaining]);
		printf("Need to write this in the new block : \n");
		int j;
		for(j=0; j< 512; j++) {
			printf("%c", tmp[j]);
		}
		printf("\n");

		mydisk_write_block(i, tmp);

		remaining = remaining - amount;
	 	offset = 0;
	 	if (remaining - BLOCK_SIZE < 0) {
	 		amount = remaining;
	 	} else {
	 		amount = BLOCK_SIZE;
	 	}
	 	// printf("NEW -> remaining = %d , offset = %d , amount = %d \n", remaining, offset, amount);

	}


	// printf("$$$ End write $$$\n");
	return 0;
}

