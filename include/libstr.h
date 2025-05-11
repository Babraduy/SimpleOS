#ifndef _LIBSTR_H
#define _LIBSTR_H

extern int strlen(const char* string);
extern int atoi(const char* string);
extern char* reverse(char* string);
extern char* itoa(int val, char* str, int base);
extern char* utoa(uint32_t val, char* str, int base);
extern void* memset(void* data, uint8_t val, int count);
extern void* memcpy(void* dest, void* src, int count);
extern char* strcat(char* dest, const char* src);
extern char* strcpy(char* dest, const char* src);
extern char* strncpy(char* dest, const char* src, int len);
extern char* strchr(char* str, char find);
extern char* strrchr(char* str, char find);
extern char* strupper(char* str);
extern int strcmp(char* str1, char* str2);
extern int strncmp(char* str1, char* str2, int len);

#endif
