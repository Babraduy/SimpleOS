#include <stdint.h>
#include <stddef.h>
#include "heap.h"
#include "libio.h"
#include "libstr.h"

void* heap_start = (void*) 0x10000;
void* heap_limit = (void*) 0x40000;

void init_heap()
{
	block* current = (block*) heap_start;
	current->size = (uintptr_t) heap_limit - (uintptr_t) heap_start - sizeof(block);
	current->is_free = 1;
	current->magic = 0xdeadbeef;
}

void* kmalloc(uint32_t size)
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
				split->magic = 0xdeadbeef;
			}

			next->is_free = 0;

			return (void*) (next + 1);
		}
		next = (block*) ((uint8_t*) next + sizeof(block) + next->size);
		if ( (void*) next >= heap_limit) return NULL;
	}
}

void* krealloc(void* ptr, uint32_t size)
{
	if (ptr != NULL) kfree(ptr);
	ptr = kmalloc(size);
	return ptr;
}

void kfree(void* ptr)
{
	block* b = ((block*) ptr) - 1;

	if (b->magic != 0xdeadbeef)
	{
		kprint("HEAP IS CORRUPTED!\n", 0x47);
		return;
	}

	b->is_free = 1;

	block* current = (block*) ((uint8_t*) heap_start);
	while ((uint8_t*) current < (uint8_t*) b)
	{
		current = (block*) ((uint8_t*) current + sizeof(block) + current->size);
		if ((uint8_t*) current + sizeof(block) + current->size == (uint8_t*) b && current->is_free)
		{
			current->size = current->size + b->size + sizeof(block);
			break;
		}
	}

	block* next = (block*) ((uint8_t*) b + sizeof(block) + b->size);
	if (next->is_free && (void*) next < heap_limit)
	{
		b->size = b->size + next->size + sizeof(block);
	}
}

void heap_status()
{
	uint32_t free = 0, used = 0;
	uint32_t total = (uintptr_t) heap_limit - (uintptr_t) heap_start;

	block* current = (block*) ((uint8_t*) heap_start);
	while (1)
	{
		if (current->is_free)
		{
			free += current->size;
		}

		else if (!current->is_free)
		{
			used += current->size;
		}

		current = (block*) ((uint8_t*) current + sizeof(block) + current->size);
		if ( (void*) current >= heap_limit) break;
	}

	int name = 0;
	while (total >= 1024)
	{
		total /= 1024;
		name += 1;
	}

	char prefix[3];

	if (name == 0)
	{
		strncpy(prefix, "B\0", 2);
	}
	if (name == 1)
	{
		strncpy(prefix, "KB\0", 3);
	}
	if (name == 2)
	{
		strncpy(prefix, "MB\0", 3);
	}
	if (name == 3)
	{
		strncpy(prefix, "GB\0", 3);
	}

	uint32_t used_percent = (used * 100) / (used + free);
	uint32_t free_percent = 100 - used_percent;

	set_stringf(VGA_WIDTH - 20, 0, "TOTAL: %d%s", 0x47, total, prefix);
	set_stringf(VGA_WIDTH - 20, 1, "USED: %u%", 0x47, used_percent);
	set_stringf(VGA_WIDTH - 20, 2, "FREE: %u%", 0x47, free_percent);
}
