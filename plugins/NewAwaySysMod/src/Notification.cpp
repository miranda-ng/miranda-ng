/*
	New Away System - plugin for Miranda IM
	Copyright (C) 2005-2007 Chervov Dmitry

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

#include "stdafx.h"
#include "Path.h"
#include "Properties.h"

void ShowMsg(wchar_t *FirstLine, wchar_t *SecondLine, bool IsErrorMsg, int Timeout)
{
	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		POPUPDATAT ppd = { 0 };
		ppd.lchIcon = LoadIcon(nullptr, IsErrorMsg ? IDI_EXCLAMATION : IDI_INFORMATION);
		mir_wstrncpy(ppd.lptzContactName, FirstLine, MAX_CONTACTNAME);
		mir_wstrncpy(ppd.lptzText, SecondLine, MAX_SECONDLINE);
		ppd.colorBack = IsErrorMsg ? 0x0202E3 : 0xE8F1FD;
		ppd.colorText = IsErrorMsg ? 0xE8F1FD : 0x000000;
		ppd.iSeconds = Timeout;
		PUAddPopupT(&ppd);
	}
	else MessageBox(nullptr, SecondLine, FirstLine, MB_OK | (IsErrorMsg ? MB_ICONEXCLAMATION : MB_ICONINFORMATION));
}

void ShowLog(TCString &LogFilePath)
{
	INT_PTR Result = (INT_PTR)ShellExecute(nullptr, L"open", LogFilePath, nullptr, nullptr, SW_SHOW);
	if (Result <= 32) {
		wchar_t szError[64];
		mir_snwprintf(szError, TranslateT("Error #%d"), Result);
		ShowMsg(szError, TranslateT("Can't open log file ") + LogFilePath, true);
	}
}
