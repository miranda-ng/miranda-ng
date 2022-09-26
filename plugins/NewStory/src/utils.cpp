#include "stdafx.h"

uint32_t toggleBit(uint32_t dw, uint32_t bit)
{
	if (dw & bit)
		return dw & ~bit;
	return dw | bit;
}

bool CheckFilter(wchar_t *buf, wchar_t *filter)
{
	//	MessageBox(0, buf, filter, MB_OK);
	int l1 = (int)mir_wstrlen(buf);
	int l2 = (int)mir_wstrlen(filter);
	for (int i = 0; i < l1 - l2 + 1; i++)
		if (CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, buf + i, l2, filter, l2) == CSTR_EQUAL)
			return true;
	return false;
}
