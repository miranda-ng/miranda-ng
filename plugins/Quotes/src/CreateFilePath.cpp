#include "StdAfx.h"

static TCHAR InvalidSymbols[] = { _T('\\'), _T('/'), _T(':'), _T('*'), _T('?'), _T('"'), _T('<'), _T('>'), _T('|') };

static TCHAR replace_invalid_symbol(TCHAR chr)
{
	for (int i = 0; i < _countof(InvalidSymbols); ++i)
		if (chr == InvalidSymbols[i])
			return _T('_');

	return chr;
}

void prepare_name(tstring& rsName)
{
	std::transform(rsName.begin(), rsName.end(), rsName.begin(), boost::bind(replace_invalid_symbol, _1));
}

tstring CreateFilePath(const tstring& rsName)
{
	TCHAR szPath[_MAX_PATH];
	::GetModuleFileName(g_hInstance, szPath, _MAX_PATH);

	TCHAR* p = _tcsrchr(szPath, _T('\\'));
	if (p)
		*p = 0;

	tstring s(rsName);
	prepare_name(s);
	tostringstream o;
	o << szPath << L"\\Quotes\\" << s;
	return o.str();
}