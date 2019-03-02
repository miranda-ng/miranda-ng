#include "StdAfx.h"

tstring CreateFilePath(const tstring &rsName)
{
	wchar_t szPath[_MAX_PATH];
	::GetModuleFileName(g_plugin.getInst(), szPath, _MAX_PATH);

	wchar_t* p = wcsrchr(szPath, '\\');
	if (p)
		*p = 0;

	tstring s(rsName);
	FixInvalidChars(s);
	tostringstream o;
	o << szPath << L"\\CurrencyRates\\" << s;
	return o.str();
}