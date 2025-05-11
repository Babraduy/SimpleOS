#include <stdint.h>
#include <stddef.h>
#include "libstr.h"
#include "libio.h"

int strlen(const char* string)
{
	int i = 0;

	while (string[i] != '\0') i++;

	return i;
}

int atoi(const char* string)
{
	int res = 0;

	for (int i=0; string[i] != '\0'; i++)
	{
		res += string[i] - '0';
		if (string[i+1] != '\0') res *= 10;
	}

	if (*string == '-') res *= -1;

	return res;
}

char* reverse(char* string)
{
	int start = 0;
	int end = strlen(string) - 1;

	while (start < end)
	{
		char temp = string[start];
		string[start] = string[end];
		string[end] = temp;

		start++;
		end--;
	}

	return string;
}

char* itoa(int num, char* str, int base)
{
	int i = 0;
	int isNegative = 0;

	if (num == 0)
	{
		str[i++] = '0';
		str[i] = '\0';

		return str;
	}

	if (num < 0 && base == 10)
	{
		isNegative = 1;
		num *= -1;
	}

	while (num != 0)
	{
		int rem = num % base;
		str[i++] = rem > 9 ? (rem - 10) + 'a' : rem + '0';
		num = num/base;
	}

	if (isNegative) str[i++] = '-';

	reverse(str);

	str[i] = '\0';

	return str;
}

char* utoa(uint32_t num, char* str, int base)
{
	int i = 0;

	if (num == 0)
	{
		str[i++] = '0';
		str[i] = '\0';

		return str;
	}

	while (num != 0)
	{
		int rem = num % base;
		str[i++] = rem > 9 ? (rem - 10) + 'a' : rem + '0';
		num = num/base;
	}

	reverse(str);

	str[i] = '\0';

	return str;
}

void* memset(void* data, uint8_t val, int count)
{
	uint8_t* ret = data;

	for (; count != 0; count--) *ret++ = val;

	return data;
}

void* memcpy(void* dest, void* src, int count)
{
	uint8_t* p_dest = (uint8_t*) dest;
	uint8_t* p_src = (uint8_t*) src;
	for (int i=0; i<count; i++)
	{
		p_dest[i] = p_src[i];
	}
	return dest;
}

char* strcat(char* dest, const char* src)
{
	while (*dest) dest++;

	return strcpy(dest, src);
}

char* strcpy(char* dest, const char* src)
{
	while (*src) *dest++ = *src++;

	*dest = '\0';

	return dest;
}

char* strncpy(char* dest, const char* src, int len)
{
	for (int i=0;i<len; i++)
	{
		*dest++ = *src++;
	}

	return dest;
}

char* strchr(char* str, char find)
{
	char* ptr;

	for (ptr = str; *ptr != find; ptr++)
	{
		if (*ptr == '\0') return NULL;
	}

	return ptr;
}

char* strrchr(char* str, char find)
{
	char* ptr;

	for (ptr = str + strlen(str); *ptr != find; ptr--)
	{
		if (ptr == str && *ptr != find) return NULL;
	}

	return ptr;
}

char* strupper(char* str)
{
	while (*str)
	{
		if (*str >= 'a' && *str <= 'z') *str += 'A' - 'a';
		str++;
	}

	return str;
}

int strcmp(char* str1, char* str2)
{
	if (strlen(str1) != strlen(str2)) return -1;

	int dif = 0;

	for (int i=0; i<strlen(str1); i++)
	{
		dif += str1[i] - str2[i];
	}

	return dif;
}

int strncmp(char* str1, char* str2, int len)
{
	if (strlen(str1) < len || strlen(str2) < len) return -1;
	int dif = 0;

	for (int i=0; i<len; i++)
	{
		dif += str1[i] - str2[i];
	}

	return dif;
}
