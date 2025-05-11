#ifndef _HEAP_H
#define _HEAP_H

typedef struct
{
	uint32_t size;			// 4 bytes
	uint8_t is_free;		// 1 byte
	uint8_t padding[7];		// 7 bytes
	uint32_t magic;			// 4 bytes
} __attribute__((packed)) block;	// 16 bytes total

extern void init_heap();
extern void* kmalloc(uint32_t size);
extern void* krealloc(void* ptr, uint32_t size);
extern void kfree(void* ptr);
extern void heap_status();

#endif
