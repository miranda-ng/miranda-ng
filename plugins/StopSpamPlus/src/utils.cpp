#include "stdafx.h"

tstring& GetDlgItemString(HWND hwnd, int id)
{
	HWND h = GetDlgItem(hwnd, id);
	int len = GetWindowTextLength(h);
	wchar_t * buf = new wchar_t[len + 1];
	GetWindowText(h, buf, len + 1);
	static tstring s;
	s = buf;
	delete[]buf;
	return s;
}

bool IsExistMyMessage(MCONTACT hContact)
{
	DB::ECPTR pCursor(DB::Events(hContact));
	while (MEVENT hDbEvent = pCursor.FetchNext()) {
		DBEVENTINFO dbei = {};
		if (db_event_get(hDbEvent, &dbei))
			break;

		if (dbei.flags & DBEF_SENT){
			// mark contact as Answered
			g_plugin.setByte(hContact, DB_KEY_ANSWERED, 1);
			// ...let the event go its way
			return true;
		}
	}
	return false;
}

tstring variables_parse(const wchar_t *tstrFormat, MCONTACT hContact)
{
	if (ServiceExists(MS_VARS_FORMATSTRING)) {
		FORMATINFO fi = {};
		fi.cbSize = sizeof(fi);
		fi.szFormat.w = wcsdup(tstrFormat);
		fi.hContact = hContact;
		fi.flags = FIF_UNICODE;
		wchar_t *tszParsed = (wchar_t*)CallService(MS_VARS_FORMATSTRING, (WPARAM)&fi, 0);
		free(fi.szFormat.w);
		if (tszParsed) {
			tstring tstrResult = tszParsed;
			mir_free(tszParsed);
			return tstrResult;
		}
	}
	return tstrFormat;
}

tstring trim(const tstring &tstr, const tstring& trimChars)
{
	size_t s = tstr.find_first_not_of(trimChars);
	size_t e = tstr.find_last_not_of(trimChars);

	if ((tstring::npos == s) || (tstring::npos == e))
		return L"";

	return tstr.substr(s, e - s + 1);
}
