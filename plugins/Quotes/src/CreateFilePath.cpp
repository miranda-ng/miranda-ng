#include "StdAfx.h"

static wchar_t InvalidSymbols[] = { '\\', '/', ':', '*', '?', '"', '<', '>', '|' };

static wchar_t replace_invalid_symbol(wchar_t chr)
{
	for (int i = 0; i < _countof(InvalidSymbols); ++i)
		if (chr == InvalidSymbols[i])
			return '_';

	return chr;
}

void prepare_name(tstring& rsName)
{
	std::transform(rsName.begin(), rsName.end(), rsName.begin(), boost::bind(replace_invalid_symbol, _1));
}

tstring CreateFilePath(const tstring& rsName)
{
	wchar_t szPath[_MAX_PATH];
	::GetModuleFileName(g_hInstance, szPath, _MAX_PATH);

	wchar_t* p = wcsrchr(szPath, '\\');
	if (p)
		*p = 0;

	tstring s(rsName);
	prepare_name(s);
	tostringstream o;
	o << szPath << L"\\Quotes\\" << s;
	return o.str();
}