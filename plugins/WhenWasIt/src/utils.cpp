/*
WhenWasIt (birthday reminder) plugin for Miranda IM

Copyright © 2006 Cristian Libotean

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

int LogInit()
{
#ifdef _DEBUG
	FILE *fout = fopen(LOG_FILE, "wt");
	fclose(fout);
#endif
	return 0;
}

int Log(char *format, ...)
{
#ifdef _DEBUG
	char		str[4096];
	va_list	vararg;
	int tBytes;
	FILE *fout = fopen(LOG_FILE, "at");
	if (!fout)
		return -1;

	time_t tNow = time(0);
	struct tm *now = localtime(&tNow);
	strftime(str, sizeof(str), "%d %b %Y @ %H:%M:%S: ", now);
	fputs(str, fout);
	va_start(vararg, format);

	tBytes = mir_vsnprintf(str, sizeof(str), format, vararg);
	if (tBytes > 0)
		str[tBytes] = 0;

	va_end(vararg);
	if (str[mir_strlen(str) - 1] != '\n')
		mir_strcat(str, "\n");
	fputs(str, fout);
	fclose(fout);
#endif
	return 0;
}

int Info(char *title, char *format, ...)
{
	char str[4096];
	va_list vararg;
	int tBytes;
	va_start(vararg, format);
	tBytes = mir_vsnprintf(str, sizeof(str), format, vararg);
	if (tBytes > 0)
		str[tBytes] = 0;
	va_end(vararg);
	return MessageBoxA(nullptr, str, title, MB_OK | MB_ICONINFORMATION);
}

///////////////////////////////////////////////////////////////////////////////

void CheckConvert(void)
{
	if (db_get_b(0, "Compatiblilty", MODULENAME) < 1) {
		for (auto &hContact : Contacts()) {
			int bday = db_get_dw(hContact, "UserInfo", "DOBd", -1);
			if (bday == -1)
				continue;

			int bmonth = db_get_dw(hContact, "UserInfo", "DOBm");
			int byear = db_get_dw(hContact, "UserInfo", "DOBy");

			db_unset(hContact, "UserInfo", "DOBd");
			db_unset(hContact, "UserInfo", "DOBm");
			db_unset(hContact, "UserInfo", "DOBy");

			db_set_dw(hContact, "UserInfo", "BirthDay", bday);
			db_set_dw(hContact, "UserInfo", "BirthMonth", bmonth);
			db_set_dw(hContact, "UserInfo", "BirthYear", byear);
		}
		
		db_set_b(0, "Compatiblilty", MODULENAME, 1);
	}
}

///////////////////////////////////////////////////////////////////////////////

wchar_t* GetContactID(MCONTACT hContact)
{
	return GetContactID(hContact, nullptr);
}

wchar_t* GetContactID(MCONTACT hContact, char *szProto)
{
	ptrW res(Contact::GetInfo(CNF_UNIQUEID, hContact, szProto));
	return (res) ? wcsdup(res) : nullptr;
}

///////////////////////////////////////////////////////////////////////////////

MCONTACT GetContactFromID(wchar_t *szID, char *szProto)
{
	for (auto &hContact : Contacts()) {
		char *m_szProto = Proto_GetBaseAccountName(hContact);
		wchar_t *szHandle = GetContactID(hContact, szProto);
		if (szHandle) {
			bool found = (!mir_wstrcmpi(szHandle, szID) && !_stricmp(szProto, m_szProto));
			free(szHandle);
			if (found)
				return hContact;
		}
	}
	return NULL;
}

MCONTACT GetContactFromID(wchar_t *szID, wchar_t *szProto)
{
	char protocol[1024];
	WideCharToMultiByte(CP_ACP, 0, szProto, -1, protocol, sizeof(protocol), nullptr, nullptr);
	return GetContactFromID(szID, protocol);
}

///////////////////////////////////////////////////////////////////////////////

void CreateToolTip(HWND target, const wchar_t* tooltip, LPARAM width)
{
	HWND hwndToolTip = CreateWindow(TOOLTIPS_CLASS, nullptr, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT, target, nullptr, nullptr, nullptr);
	if (hwndToolTip) {
		TOOLINFO ti = { 0 };
		ti.cbSize = sizeof(ti);
		ti.uFlags = TTF_TRANSPARENT | TTF_SUBCLASS;
		ti.hwnd = target;
		ti.uId = 0;
		ti.hinst = nullptr;
		ti.lpszText = (LPWSTR)tooltip;
		GetClientRect(target, &ti.rect);
		SendMessage(hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
		SendMessage(hwndToolTip, TTM_SETMAXTIPWIDTH, 0, width);
		SendMessage(hwndToolTip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 20000);
	}
}
