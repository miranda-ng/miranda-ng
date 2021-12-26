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

void CopyText(HWND hwnd, const wchar_t *text)
{
	OpenClipboard(hwnd);
	EmptyClipboard();
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, sizeof(wchar_t) * (mir_wstrlen(text) + 1));
	wchar_t *s = (wchar_t *)GlobalLock(hMem);
	mir_wstrcpy(s, text);
	GlobalUnlock(hMem);
	SetClipboardData(CF_UNICODETEXT, hMem);
	CloseClipboard();
}
