#include <stdint.h>
#include <stddef.h>
#include "heap.h"

void* heap_start = (void*) 0x10000;
void* heap_limit = (void*) 0x20000;

block* current;

void init_heap()
{
	current = (block*) heap_start;
	current->size = (int) heap_limit - (int) heap_start - sizeof(block);
	current->is_free = 1;
}

void* malloc(uint32_t size)
{
	block* next = (block*) ((uint8_t*) heap_start);
	while (1)
	{
		if (next->is_free && next->size >= size)
		{
			if (next->size - size >= sizeof(block) + 1)	// split
			{
				int original_size = next->size;
				next->size = size;

				block* split = (block*) ((uint8_t*) next + sizeof(block) + size);
				split->size = original_size - size - sizeof(block);
				split->is_free = 1;
			}

			next->is_free = 0;

			return (void*) (next + 1);
		}
		next = (block*) ((uint8_t*) next + sizeof(block) + next->size);
		if ( (void*) next >= heap_limit) return NULL;
	}
}

void free(void* ptr)
{
	block* b = ((block*) ptr) - 1;
	
	b->is_free = 1;
}
