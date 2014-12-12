/*
	NewXstatusNotify YM - Plugin for Miranda IM
	Copyright (c) 2001-2004 Luca Santarelli
	Copyright (c) 2005-2007 Vasilich
	Copyright (c) 2007-2011 yaho

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "common.h"

// From NewEventNotify :-)
bool CheckMsgWnd(MCONTACT hContact)
{
	MessageWindowInputData mwid;
	mwid.cbSize = sizeof(MessageWindowInputData);
	mwid.hContact = hContact;
	mwid.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;

	MessageWindowData mwd;
	mwd.cbSize = sizeof(MessageWindowData);
	mwd.hContact = hContact;
	if (CallService(MS_MSG_GETWINDOWDATA, (WPARAM)&mwid, (LPARAM)&mwd) != NULL)
		return false;

	if (mwd.hwndWindow != NULL && (mwd.uState & MSG_WINDOW_STATE_EXISTS))
		return true;

	return false;
}

int DBGetStringDefault(MCONTACT hContact, const char *szModule, const char *szSetting, TCHAR *setting, int size, const TCHAR *defaultValue)
{
	DBVARIANT dbv;
	if (!db_get_ts(hContact, szModule, szSetting, &dbv)) {
		_tcsncpy(setting, dbv.ptszVal, size);
		db_free(&dbv);
		return 0;
	}

	_tcsncpy(setting, defaultValue, size);
	return 1;
}

void ShowLog(TCHAR *file)
{
	int res = (int)ShellExecute(NULL, _T("open"), file, NULL, NULL, SW_SHOW);
	if (res <= 32) // error
		MessageBox(0, TranslateT("Can't open the log file!"), TranslateT("NewXstatusNotify"), MB_OK | MB_ICONERROR);
}

BOOL StatusHasAwayMessage(char *szProto, int status)
{
	if (szProto != NULL) {
		unsigned long iSupportsSM = (unsigned long)CallProtoService(szProto, PS_GETCAPS, (WPARAM)PFLAGNUM_3, 0);
		return (iSupportsSM & Proto_Status2Flag(status)) ? TRUE : FALSE;
	}

	switch (status) {
	case ID_STATUS_AWAY:
	case ID_STATUS_NA:
	case ID_STATUS_OCCUPIED:
	case ID_STATUS_DND:
	case ID_STATUS_FREECHAT:
	case ID_STATUS_ONTHEPHONE:
	case ID_STATUS_OUTTOLUNCH:
		return TRUE;

	default:
		return FALSE;
	}
}

void LogToFile(TCHAR *stzText)
{
	FILE *fp = _tfopen(opt.LogFilePath, _T("a+b, ccs=UTF-8"));
	if (fp) {
		char *encodedText = mir_utf8encodeT(stzText);
		if (encodedText) {
			fprintf(fp, encodedText);
			mir_free(encodedText);
		}
		fclose(fp);
	}
}

WCHAR *mir_dupToUnicodeEx(char *ptr, UINT CodePage)
{
	if (ptr == NULL)
		return NULL;

	size_t size = strlen(ptr) + 1;
	WCHAR *tmp = (WCHAR *)mir_alloc(size * sizeof(WCHAR));

	MultiByteToWideChar(CodePage, 0, ptr, -1, tmp, (int)size * sizeof(WCHAR));
	return tmp;
}

TCHAR *AddCR(const TCHAR *stzText)
{
	const TCHAR *found;
	size_t i = 0, len = mir_tstrlen(stzText), j;
	TCHAR *tmp = (TCHAR *)mir_alloc(1024 * sizeof(TCHAR));
	*tmp = _T('\0');
	while ((found = _tcsstr((stzText + i), _T("\n"))) != NULL && _tcslen(tmp) + 1 < 1024) {
		j = (int)(found - stzText);
		if (mir_tstrlen(tmp) + j - i + 2 < 1024)
			tmp = _tcsncat(tmp, stzText + i, j - i);
		else
			break;

		if (j == 0 || *(stzText + j - 1) != _T('\r'))
			tmp = mir_tstrcat(tmp, _T("\r"));

		tmp = mir_tstrcat(tmp, _T("\n"));
		i = j + 1;
	}
	if (mir_tstrlen(tmp) + len - i + 1 < 1024)
		tmp = mir_tstrcat(tmp, stzText + i);

	return tmp;
}
