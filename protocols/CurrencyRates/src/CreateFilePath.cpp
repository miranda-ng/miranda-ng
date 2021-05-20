#include "StdAfx.h"

CMStringW CreateFilePath(const CMStringW &rsName)
{
	wchar_t szPath[_MAX_PATH];
	::GetModuleFileName(g_plugin.getInst(), szPath, _MAX_PATH);

	wchar_t* p = wcsrchr(szPath, '\\');
	if (p)
		*p = 0;

	CMStringW s(rsName);
	FixInvalidChars(s);
	return CMStringW(szPath) + L"\\CurrencyRates\\" + s;
}
