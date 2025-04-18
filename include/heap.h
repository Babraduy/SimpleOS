#ifndef _HEAP_H
#define _HEAP_H

typedef struct
{
	uint32_t size;		// 32 bytes
	uint8_t is_free;	// 1 byte
	uint8_t padding[3];	// 3 bytes
} block;

extern void init_heap();
extern void* malloc(uint32_t size);
extern void free(void* ptr);

#endif
