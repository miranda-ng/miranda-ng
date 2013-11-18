#ifndef _CLIST_UTIL_INC
#define _CLIST_UTIL_INC

#include "common.h"

int GroupNameExists(const char *name, int skipGroup);
int RenameGroupWithMove(int groupId, const wchar_t *szName, int move);

#endif
