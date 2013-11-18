#include "clist_util.h"

int GroupNameExists(const char *name, int skipGroup)
{
	char idstr[33];
	DBVARIANT dbv;
	int i;

	for (i = 0;; i++) {
		if (i == skipGroup)
			continue;
		itoa(i, idstr, 10);
		if (DBGetContactSettingStringUtf(NULL, "CListGroups", idstr, &dbv))
			break;
		if (strcmp(dbv.pszVal + 1, name) == 0) {
			DBFreeVariant(&dbv);
			return i + 1;
		}
		DBFreeVariant(&dbv);
	}
	return 0;
}

