#ifndef _LIBMEM_H
#define _LIBMEM_H

extern int strlen(const char* string);
extern int atoi(const char* string);
extern char* reverse(char* string);
extern char* itoa(int val, char* str, int base);
extern void* memset(void* data, uint8_t val, int count);
extern char* strcat(char* dest, const char* src);
extern char* strcpy(char* dest, const char* src);

#endif
