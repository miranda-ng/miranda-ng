#include "StdAfx.h"

std::wstring CreateFilePath(const std::wstring &rsName)
{
	wchar_t szPath[_MAX_PATH];
	::GetModuleFileName(g_plugin.getInst(), szPath, _MAX_PATH);

	wchar_t* p = wcsrchr(szPath, '\\');
	if (p)
		*p = 0;

	std::wstring s(rsName);
	FixInvalidChars(s);
	std::wostringstream o;
	o << szPath << L"\\CurrencyRates\\" << s;
	return o.str();
}