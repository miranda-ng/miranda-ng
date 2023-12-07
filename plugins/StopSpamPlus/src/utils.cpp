#include "stdafx.h"

tstring variables_parse(const wchar_t *tstrFormat, MCONTACT hContact)
{
	if (ServiceExists(MS_VARS_FORMATSTRING)) {
		FORMATINFO fi = {};
		fi.cbSize = sizeof(fi);
		fi.szFormat.w = wcsdup(tstrFormat);
		fi.hContact = hContact;
		fi.flags = FIF_UNICODE;
		wchar_t *tszParsed = (wchar_t *)CallService(MS_VARS_FORMATSTRING, (WPARAM)&fi, 0);
		free(fi.szFormat.w);
		if (tszParsed) {
			tstring tstrResult = tszParsed;
			mir_free(tszParsed);
			return tstrResult;
		}
	}
	return tstrFormat;
}

tstring trim(const tstring &tstr, const tstring &trimChars)
{
	size_t s = tstr.find_first_not_of(trimChars);
	size_t e = tstr.find_last_not_of(trimChars);

	if ((tstring::npos == s) || (tstring::npos == e))
		return L"";

	return tstr.substr(s, e - s + 1);
}
