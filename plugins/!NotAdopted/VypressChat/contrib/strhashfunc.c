#include <string.h>

#include "strhashfunc.h"

unsigned int hashtable_strhashfunc(void * p)
{
	const char * str = (const char *)p;
	unsigned int hash = 5381;
	int c;

	while ((c = *str++) != '\0')
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

int hashtable_strequalfunc(void * str1, void * str2)
{
	return !strcmp(str1, str2);
}

