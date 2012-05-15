#include "../headers.h"


tstring &GetDlgItemString(HWND hwnd, int id)
{
	HWND h = GetDlgItem(hwnd, id);
	int len = GetWindowTextLength(h);
	TCHAR * buf = new TCHAR[len + 1];
	GetWindowText(h, buf, len + 1);
	static tstring s;
	s = buf;
	delete []buf;
	return s;
}					

bool IsExistMyMessage(HANDLE hContact)
{
	DBEVENTINFO dbei = { 0 };
	HANDLE		hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDFIRST, (WPARAM)hContact, 0);
	while(hDbEvent){
		ZeroMemory(&dbei, sizeof(dbei));
		dbei.cbSize = sizeof(dbei);

		if (CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&dbei)) break;
		if(dbei.flags & DBEF_SENT){
			// mark contact as Answered
			DBWriteContactSettingByte(hContact, pluginName, answeredSetting, 1);
			// ...let the event go its way
			return true;
		}
		hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDNEXT, (WPARAM)hDbEvent, 0);
	}
	return false;
}

void SetDlgItemString(HWND hwndDlg, UINT idItem, std::string const &str)
{
	SetDlgItemTextA(hwndDlg, idItem, str.c_str());
}

void SetDlgItemString(HWND hwndDlg, UINT idItem, std::wstring const &str)
{
	SetDlgItemTextW(hwndDlg, idItem, str.c_str());
}

tstring variables_parse(tstring const &tstrFormat, HANDLE hContact){
	if (ServiceExists(MS_VARS_FORMATSTRING)) {
		FORMATINFO fi;
		TCHAR *tszParsed;
		tstring tstrResult;

		ZeroMemory(&fi, sizeof(fi));
		fi.cbSize = sizeof(fi);
		fi.tszFormat = _tcsdup(tstrFormat.c_str());
		fi.hContact = hContact;
		fi.flags |= FIF_TCHAR;
		tszParsed = (TCHAR *)CallService(MS_VARS_FORMATSTRING, (WPARAM)&fi, 0);
		free(fi.tszFormat);
		if (tszParsed) {
			tstrResult = tszParsed;
			CallService(MS_VARS_FREEMEMORY, (WPARAM)tszParsed, 0);
			return tstrResult;
		}
	}
	return tstrFormat;
}

tstring trim(const tstring &tstr, const tstring& trimChars)
{
    size_t s = tstr.find_first_not_of(trimChars);
    size_t e = tstr.find_last_not_of (trimChars);

    if ((tstring::npos == s) || ( tstring::npos == e))
        return _T("");
    else
        return tstr.substr(s, e - s + 1);
}