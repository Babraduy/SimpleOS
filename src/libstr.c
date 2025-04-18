#include <stdint.h>
#include "libstr.h"

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

void* memset(void* data, uint8_t val, int count)
{
	uint8_t* ret = data;

	for (; count != 0; count--) *ret++ = val;

	return data;
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
