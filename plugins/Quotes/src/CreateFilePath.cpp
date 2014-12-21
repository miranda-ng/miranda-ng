#include "StdAfx.h"

namespace
{
	TCHAR replace_invalid_symbol(TCHAR chr)
	{
		TCHAR InvaliSymbols[] = { _T('\\'), _T('/'), _T(':'), _T('*'), _T('?'), _T('"'), _T('<'), _T('>'), _T('|') };
		for (int i = 0; i < sizeof(InvaliSymbols) / sizeof(InvaliSymbols[0]); ++i)
		{
			if (chr == InvaliSymbols[i])
			{
				return _T('_');
			}
		}

		return chr;
	}

	void prepare_name(tstring& rsName)
	{
		std::transform(rsName.begin(), rsName.end(), rsName.begin(), boost::bind(replace_invalid_symbol, _1));
	}
}

tstring CreateFilePath(const tstring& rsName)
{
	TCHAR szPath[_MAX_PATH];
	::GetModuleFileName(g_hInstance, szPath, _MAX_PATH);

	TCHAR* p = _tcsrchr(szPath, _T('\\'));
	if (p)
	{
		*p = 0;
	}

	tstring s(rsName);
	prepare_name(s);
	tostringstream o;
	o << szPath << _T("\\Quotes\\") << s;
	return o.str();
}