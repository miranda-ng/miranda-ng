#include "commonheaders.h"

void __fastcall safe_free(void** p)
{
	if (*p) {
		free(*p);
		*p = NULL;
	}
}


void __fastcall safe_delete(void** p)
{
	if (*p) {
		delete(*p);
		*p = NULL;
	}
}
