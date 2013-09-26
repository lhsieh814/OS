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

int cache_blocks;  /* number of blocks for the cache buffer */
struct cache_entry **ring_buffer;
struct cache_entry *p;

/* TODO: some helper functions e.g. find_cached_entry(block_id) */
struct cache_entry *find_cached_entry(int block_id) {
	printf("{find_cached_entry()\n");
	int i;
	for (i=0; i<cache_blocks; i++) {
		if (ring_buffer[i]->block_id == block_id) {
			printf("Found entry in the cache}\n");
			return ring_buffer[i];
		}
	}
	printf("Did not find entry in cache}\n");
	return NULL;
}

int init_cache(int nblocks)
{
	/* TODO: allocate proper data structure (ring buffer)
	 * initialize entry data so that the the ring buffer is empty
	 */
	printf("Starting cache ...\n");
	cache_enabled = 1;
	printf("cache_blocks = %d\n", nblocks);
	cache_blocks = nblocks;

	// Allocate and initialize the ring buffer
	ring_buffer = malloc(cache_blocks * sizeof(struct cache_entry));

	int i;
	for (i=0; i<cache_blocks; i++) {
		ring_buffer[i] = malloc(sizeof(struct cache_entry));
		ring_buffer[i]->block_id = -1;
		ring_buffer[i]->is_dirty = 0;
	}	 

	// Pointer p points to the first entry in the ring_buffer
	p = *ring_buffer;
	print();
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
	 * or return NULL if not found 
	 */
	printf("{get_cached_block()\n");
	struct cache_entry *entry = find_cached_entry(block_id);

	if (entry != NULL) {
		printf("Got the content from cache}\n");
		return entry->content;
	}	
	printf("Did not find in cache}\n");
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
	printf("{create_cached_block()\n");
	void *content;
	if(p->block_id == -1) {
		//Found an empty entry
		printf("Found an empty entry}\n");
		p -> block_id = block_id;
		p -> is_dirty = 0;
		content = (*p).content;
		p++;
		return content;
	} else {
		printf("Need to kick an existing entry}\n");
		if (p->is_dirty == 1) {
			printf("Write back the dirty block back to disk}\n");
			mydisk_write_block(block_id, *(p->content));
			content = (*p).content;
			p++;
			return content;
		}
	}

	p++;

	return NULL;
}

void mark_dirty(int block_id)
{
	/* TODO: find the entry and mark it dirty */
	printf("mark_dirty()\n");
	struct cache_entry *entry = find_cached_entry(block_id);
	if (entry != NULL) {
		entry->is_dirty = 1;
	} else {
		printf("\tCannot mark entry as dirty because it is not found in cache.\n");
	}
}

void print()
{
	int i;
	for (i=0; i<cache_blocks; i++) {
		printf("------ %d ------\n", i);
		printf("block_id = %d\n", ring_buffer[i]->block_id);
		printf("is_dirty = %d\n", ring_buffer[i]->is_dirty);
		printf("content = %c\n", ring_buffer[i]->content);
	}
}
