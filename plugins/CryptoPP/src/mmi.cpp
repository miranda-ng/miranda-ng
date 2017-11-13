#include "commonheaders.h"

void __fastcall safe_free(void** p)
{
	if (*p) {
		free(*p);
		*p = nullptr;
	}
}


void __fastcall safe_delete(void** p)
{
	if (*p) {
		delete(*p);
		*p = nullptr;
	}
}
