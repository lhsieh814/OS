#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mydisk.h"

/* The cache entry struct */
struct cache_entry
{
	int block_id;
	int is_dirty;
	char content[BLOCK_SIZE];
};

struct cache_entry *ring_buffer;
struct cache_entry *p;

int cache_blocks;  /* number of blocks for the cache buffer */

/* TODO: some helper functions e.g. find_cached_entry(block_id) */

int init_cache(int nblocks)
{
	/* TODO: allocate proper data structure (ring buffer)
	 * initialize entry data so that the the ring buffer is empty
	 */
	 printf("Starting cache ...\n");

	 printf("cache_blocks = %d\n", nblocks);
	 cache_blocks = nblocks;

	ring_buffer = malloc(cache_blocks * sizeof(struct cache_entry));	 
	p = malloc(sizeof(struct cache_entry));
	p = ring_buffer;

	return 0;
}

int close_cache()
{
	/* TODO: release the memory for the ring buffer */

	free(ring_buffer);
	return 0;
}

void *get_cached_block(int block_id)
{
	/* TODO: find the entry, return the content 
	 * or return NULL if nut found 
	 */
	int i;
	for (i=0; i<cache_blocks; i++) {
		if (ring_buffer[i].block_id == block_id) {
			printf("Found entry in the cache");
			return ring_buffer[i].content;
		}
	}

	return NULL;
}

void *create_cached_block(int block_id)
{
	/* TODO: create a new entry, insert it into the ring buffer
	 * It might kick an exisitng entry.
	 * Remember to write dirty block back to disk
	 * Note that: think if you can use mydisk_write_block() to 
	 * flush dirty blocks to disk
	 */
	struct cache_entry *tmp = malloc(sizeof(struct cache_entry));
	*tmp->block_id = block_id;
	*tmp->is_dirty = 0;

	p++;

	return NULL;
}

void mark_dirty(int block_id)
{
	/* TODO: find the entry and mark it dirty */
}

