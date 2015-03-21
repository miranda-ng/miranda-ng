#include <string.h>

#include "h_util.h"

char* newStr( const char* s )
{
	return (s == NULL) ? NULL : strcpy(new char[strlen(s) + 1], s);
}
