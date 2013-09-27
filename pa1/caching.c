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
/* Finds the cache entry in the ring buffer. Return NULL if it does not exist*/
struct cache_entry *find_cached_entry(int block_id) {
	int i;
	for (i=0; i<cache_blocks; i++) {
		if (ring_buffer[i]->block_id == block_id) {
			printf("find_cached_entry() : Found entry in the cache\n");
			return ring_buffer[i];
		}
	}
	printf("find_cached_entry() : Did not find entry in cache\n");
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
	ring_buffer = malloc(cache_blocks * sizeof(struct cache_entry *));

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
	int i;
	for (i=0; i<cache_blocks; i++) {
		if (ring_buffer[i]->is_dirty == 1) {
			// flush cache
			mydisk_write_block(ring_buffer[i]->block_id, ring_buffer[i]->content);
		}
		free(ring_buffer[i]);
	}
	free(ring_buffer);
	return 0;
}

void *get_cached_block(int block_id)
{
	/* TODO: find the entry, return the content 
	 * or return NULL if not found 
	 */
	struct cache_entry *entry = find_cached_entry(block_id);

	if (entry != NULL) {
		printf("get_cached_block() : Got the content from cache\n");
int i;
for (i=0; i<BLOCK_SIZE; i++) {
	printf("%c", (entry->content)[i]);
}
printf("\n");
		return entry->content;
	}	
	print();
	printf("get_cached_block() : Did not find in cache\n");
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
	char *tmp;
	int next_block = p->block_id;
	if (next_block == -1) {
		//Found an empty entry
		printf("create_cached_block() : Found an empty entry\n");
		p -> block_id = block_id;
		p -> is_dirty = 0;
		tmp = p->content;
		p++;
		return tmp;
	} else {
		if (p->is_dirty == 1) {
			printf("create_cached_block() : Write back the dirty block back to disk\n");
			mydisk_write_block(block_id, *(p->content));
			p->block_id = block_id;
			p->is_dirty = 0;
			tmp = p->content;
			p++;
			return tmp;
		} else {
			printf("create_cached_block() : Not dirty\n");
			printf("block_id = %d\n", block_id);
			printf("block_id = %d\n", p->block_id);

			p->block_id = &block_id;
			p->is_dirty = 0;
			tmp = p->content;
printf("create-->\n");
print();
			p++;
printf("after p++-->\n");
print();
			return tmp;
		}
	}

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
		printf("------ %d ------ %d\n", i, &(ring_buffer[i]->content));
		printf("\tblock_id = %d\n", ring_buffer[i]->block_id);
		printf("\tis_dirty = %d\n", ring_buffer[i]->is_dirty);
		printf("\tcontent = %c\n", ring_buffer[i]->content);
	}
}