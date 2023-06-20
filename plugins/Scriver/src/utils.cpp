/*
Scriver

Copyright (c) 2000-12 Miranda ICQ/IM project,

all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#ifndef TTI_NONE
#define TTI_NONE 0
#endif

const char *filename = "scriver.log";

void logInfo(const char *fmt, ...)
{
	SYSTEMTIME time;
	char *str;
	va_list vararg;
	int strsize;
	FILE *flog = fopen(filename, "at");
	if (flog != nullptr) {
		GetLocalTime(&time);
		va_start(vararg, fmt);
		str = (char*)malloc(strsize = 2048);
		while (mir_vsnprintf(str, strsize, fmt, vararg) == -1)
			str = (char*)realloc(str, strsize += 2048);
		va_end(vararg);
		fprintf(flog, "%04d-%02d-%02d %02d:%02d:%02d,%03d [%s]", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds, "INFO");
		fprintf(flog, "  %s\n", str);
		free(str);
		fclose(flog);
	}
}

void rtrimText(wchar_t *text)
{
	static wchar_t szTrimString[] = L":;,.!?\'\"><()[]- \r\n";
	size_t iLen = mir_wstrlen(text) - 1;
	while (wcschr(szTrimString, text[iLen])) {
		text[iLen] = '\0';
		iLen--;
	}
}

wchar_t* limitText(wchar_t *text, int limit)
{
	size_t len = mir_wstrlen(text);
	if (len > g_dat.limitNamesLength) {
		wchar_t *ptszTemp = (wchar_t*)mir_alloc(sizeof(wchar_t) * (limit + 4));
		wcsncpy(ptszTemp, text, limit + 1);
		wcsncpy(ptszTemp + limit, L"...", 4);
		return ptszTemp;
	}
	return text;
}

// Code taken from http://www.geekhideout.com/urlcode.shtml

/* Converts an integer value to its hex character*/
char to_hex(char code)
{
	static char hex[] = "0123456789abcdef";
	return hex[code & 15];
}

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_encode(char *str)
{
	char *pstr = str, *buf = (char*)mir_alloc(mir_strlen(str) * 3 + 1), *pbuf = buf;
	while (*pstr) {
		if ((48 <= *pstr && *pstr <= 57) ||//0-9
			(65 <= *pstr && *pstr <= 90) ||//ABC...XYZ
			(97 <= *pstr && *pstr <= 122) ||//abc...xyz
			*pstr == '-' || *pstr == '_' || *pstr == '.')
			*pbuf++ = *pstr;
		else if (*pstr == ' ')
			*pbuf++ = '+';
		else
			*pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
		pstr++;
	}
	*pbuf = '\0';
	return buf;
}

void SearchWord(wchar_t *word, int engine)
{
	char szURL[4096];
	if (word && word[0]) {
		T2Utf wordUTF(word);
		CMStringA wordURL(mir_urlEncode(wordUTF));
		switch (engine) {
		case SEARCHENGINE_WIKIPEDIA:
			mir_snprintf(szURL, "https://en.wikipedia.org/wiki/%s", wordURL.c_str());
			break;
		case SEARCHENGINE_YAHOO:
			mir_snprintf(szURL, "https://search.yahoo.com/search?p=%s&ei=UTF-8", wordURL.c_str());
			break;
		case SEARCHENGINE_FOODNETWORK:
			mir_snprintf(szURL, "https://www.foodnetwork.com/search/%s-", wordURL.c_str());
			break;
		case SEARCHENGINE_BING:
			mir_snprintf(szURL, "https://www.bing.com/search?q=%s&form=OSDSRC", wordURL.c_str());
			break;
		case SEARCHENGINE_GOOGLE_MAPS:
			mir_snprintf(szURL, "https://maps.google.com/maps?q=%s&ie=utf-8&oe=utf-8", wordURL.c_str());
			break;
		case SEARCHENGINE_GOOGLE_TRANSLATE:
			mir_snprintf(szURL, "https://translate.google.com/?q=%s&ie=utf-8&oe=utf-8", wordURL.c_str());
			break;
		case SEARCHENGINE_YANDEX:
			mir_snprintf(szURL, "https://yandex.ru/yandsearch?text=%s", wordURL.c_str());
			break;
		case SEARCHENGINE_GOOGLE:
		default:
			mir_snprintf(szURL, "https://www.google.com/search?q=%s&ie=utf-8&oe=utf-8", wordURL.c_str());
			break;
		}

		Utils_OpenUrl(szURL);
	}
}

HWND CreateToolTip(HWND hwndParent, LPTSTR ptszText, LPTSTR ptszTitle, RECT *rect)
{
	TOOLINFO ti = { 0 };
	HWND hwndTT;
	hwndTT = CreateWindowEx(WS_EX_TOPMOST,
		TOOLTIPS_CLASS, nullptr,
		WS_POPUP | TTS_NOPREFIX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		hwndParent, nullptr, g_plugin.getInst(), nullptr);

	SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS | TTF_CENTERTIP;
	ti.hwnd = hwndParent;
	ti.hinst = g_plugin.getInst();
	ti.lpszText = ptszText;
	ti.rect = *rect;
	SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)&ti);
	SendMessage(hwndTT, TTM_SETTITLE, TTI_NONE, (LPARAM)ptszTitle);
	return hwndTT;
}

void SetToolTipText(HWND hwndParent, HWND hwndTT, LPTSTR ptszText, LPTSTR ptszTitle)
{
	TOOLINFO ti = { sizeof(ti) };
	ti.hinst = g_plugin.getInst();
	ti.hwnd = hwndParent;
	ti.lpszText = ptszText;
	SendMessage(hwndTT, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
	SendMessage(hwndTT, TTM_SETTITLE, TTI_NONE, (LPARAM)ptszTitle);
}

void SetToolTipRect(HWND hwndParent, HWND hwndTT, RECT *rect)
{
	TOOLINFO ti = { sizeof(ti) };
	ti.hinst = g_plugin.getInst();
	ti.hwnd = hwndParent;
	ti.rect = *rect;
	SendMessage(hwndTT, TTM_NEWTOOLRECT, 0, (LPARAM)&ti);
}

void SetButtonsPos(HWND hwndDlg, MCONTACT hContact, bool bShow)
{
	HDWP hdwp = BeginDeferWindowPos(Srmm_GetButtonCount());

	RECT rc;
	GetWindowRect(GetDlgItem(hwndDlg, IDC_SPLITTERY), &rc);
	POINT pt = { 0, rc.top };
	ScreenToClient(hwndDlg, &pt);
	pt.y -= 20;

	int iLeftX = 2, iRightX = rc.right - rc.left - 2;
	int iGap = Srmm_GetButtonGap();

	CustomButtonData *cbd;
	for (int i = 0; cbd = Srmm_GetNthButton(i); i++) {
		HWND hwndButton = GetDlgItem(hwndDlg, cbd->m_dwButtonCID);
		if (hwndButton == nullptr)
			continue;

		if (cbd->m_dwButtonCID == IDC_ADD)
			if (Contact::OnList(hContact)) {
				ShowWindow(hwndButton, SW_HIDE);
				continue;
			}

		ShowWindow(hwndButton, bShow ? SW_SHOW : SW_HIDE);

		int width = iGap + cbd->m_iButtonWidth;
		if (cbd->m_bRSided) {
			iRightX -= width;
			hdwp = DeferWindowPos(hdwp, hwndButton, nullptr, iRightX, pt.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}
		else {
			hdwp = DeferWindowPos(hdwp, hwndButton, nullptr, iLeftX, pt.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			iLeftX += width;
		}
	}

	EndDeferWindowPos(hdwp);
}
