/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-17 Miranda NG project,
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

#include "chat.h"

#pragma comment(lib, "Shlwapi.lib")

int GetRichTextLength(HWND hwnd)
{
	GETTEXTLENGTHEX gtl;
	gtl.flags = GTL_PRECISE;
	gtl.codepage = CP_ACP;
	return (int)SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

wchar_t* RemoveFormatting(const wchar_t *pszWord)
{
	static wchar_t szTemp[10000];

	if (pszWord == nullptr)
		return nullptr;

	wchar_t *d = szTemp;
	size_t cbLen = mir_wstrlen(pszWord);
	if (cbLen > _countof(szTemp))
		cbLen = _countof(szTemp)-1;

	for (size_t i = 0; i < cbLen;) {
		if (pszWord[i] == '%') {
			switch (pszWord[i+1]) {
			case '%':
				*d++ = '%';

			case 'b':
			case 'u':
			case 'i':
			case 'B':
			case 'U':
			case 'I':
			case 'r':
			case 'C':
			case 'F':
				i += 2;
				continue;

			case 'c':
			case 'f':
				i += 4;
				continue;
			}
		}

		*d++ = pszWord[i++];
	}
	*d = 0;

	return szTemp;
}

BOOL DoTrayIcon(SESSION_INFO *si, GCEVENT *gce)
{
	switch (gce->iType) {
	case GC_EVENT_MESSAGE | GC_EVENT_HIGHLIGHT:
	case GC_EVENT_ACTION | GC_EVENT_HIGHLIGHT:
		chatApi.AddEvent(si->hContact, Skin_LoadIcon(SKINICON_EVENT_MESSAGE), GC_FAKE_EVENT, 0, TranslateT("%s wants your attention in %s"), gce->ptszNick, si->ptszName);
		break;
	case GC_EVENT_MESSAGE:
		chatApi.AddEvent(si->hContact, chatApi.hIcons[ICON_MESSAGE], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s speaks in %s"), gce->ptszNick, si->ptszName);
		break;
	case GC_EVENT_ACTION:
		chatApi.AddEvent(si->hContact, chatApi.hIcons[ICON_ACTION], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s speaks in %s"), gce->ptszNick, si->ptszName);
		break;
	case GC_EVENT_JOIN:
		chatApi.AddEvent(si->hContact, chatApi.hIcons[ICON_JOIN], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s has joined %s"), gce->ptszNick, si->ptszName);
		break;
	case GC_EVENT_PART:
		chatApi.AddEvent(si->hContact, chatApi.hIcons[ICON_PART], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s has left %s"), gce->ptszNick, si->ptszName);
		break;
	case GC_EVENT_QUIT:
		chatApi.AddEvent(si->hContact, chatApi.hIcons[ICON_QUIT], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s has disconnected"), gce->ptszNick);
		break;
	case GC_EVENT_NICK:
		chatApi.AddEvent(si->hContact, chatApi.hIcons[ICON_NICK], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s is now known as %s"), gce->ptszNick, gce->ptszText);
		break;
	case GC_EVENT_KICK:
		chatApi.AddEvent(si->hContact, chatApi.hIcons[ICON_KICK], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s kicked %s from %s"), gce->ptszStatus, gce->ptszNick, si->ptszName);
		break;
	case GC_EVENT_NOTICE:
		chatApi.AddEvent(si->hContact, chatApi.hIcons[ICON_NOTICE], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("Notice from %s"), gce->ptszNick);
		break;
	case GC_EVENT_TOPIC:
		chatApi.AddEvent(si->hContact, chatApi.hIcons[ICON_TOPIC], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("Topic change in %s"), si->ptszName);
		break;
	case GC_EVENT_INFORMATION:
		chatApi.AddEvent(si->hContact, chatApi.hIcons[ICON_INFO], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("Information in %s"), si->ptszName);
		break;
	case GC_EVENT_ADDSTATUS:
		chatApi.AddEvent(si->hContact, chatApi.hIcons[ICON_ADDSTATUS], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s enables '%s' status for %s in %s"), gce->ptszText, gce->ptszStatus, gce->ptszNick, si->ptszName);
		break;
	case GC_EVENT_REMOVESTATUS:
		chatApi.AddEvent(si->hContact, chatApi.hIcons[ICON_REMSTATUS], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s disables '%s' status for %s in %s"), gce->ptszText, gce->ptszStatus, gce->ptszNick, si->ptszName);
		break;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void __stdcall ShowRoomFromPopup(void *pi)
{
	SESSION_INFO *si = (SESSION_INFO*)pi;
	chatApi.ShowRoom(si);
}

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED) {
			SESSION_INFO *si = (SESSION_INFO*)PUGetPluginData(hWnd);
			CallFunctionAsync(ShowRoomFromPopup, si);

			PUDeletePopup(hWnd);
			return TRUE;
		}
		break;
	case WM_CONTEXTMENU:
		SESSION_INFO *si = (SESSION_INFO*)PUGetPluginData(hWnd);
		if (si->hContact)
			if (cli.pfnGetEvent(si->hContact, 0))
				cli.pfnRemoveEvent(si->hContact, GC_FAKE_EVENT);

		if (si->pDlg && KillTimer(si->pDlg->GetHwnd(), TIMERID_FLASHWND))
			FlashWindow(si->pDlg->GetHwnd(), FALSE);

		PUDeletePopup(hWnd);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

int ShowPopup(MCONTACT hContact, SESSION_INFO *si, HICON hIcon, char *pszProtoName, wchar_t*, COLORREF crBkg, const wchar_t *fmt, ...)
{
	static wchar_t szBuf[4 * 1024];

	if (!fmt || fmt[0] == 0 || mir_wstrlen(fmt) > 2000)
		return 0;

	va_list marker;
	va_start(marker, fmt);
	mir_vsnwprintf(szBuf, 4096, fmt, marker);
	va_end(marker);

	POPUPDATAT pd = { 0 };
	pd.lchContact = hContact;

	if (hIcon)
		pd.lchIcon = hIcon;
	else
		pd.lchIcon = LoadIconEx("window", FALSE);

	PROTOACCOUNT *pa = Proto_GetAccount(pszProtoName);
	mir_snwprintf(pd.lptzContactName, L"%s - %s", (pa == nullptr) ? _A2T(pszProtoName) : pa->tszAccountName, cli.pfnGetContactDisplayName(hContact, 0));
	mir_wstrncpy(pd.lptzText, TranslateW(szBuf), _countof(pd.lptzText));
	pd.iSeconds = g_Settings->iPopupTimeout;

	if (g_Settings->iPopupStyle == 2) {
		pd.colorBack = 0;
		pd.colorText = 0;
	}
	else if (g_Settings->iPopupStyle == 3) {
		pd.colorBack = g_Settings->crPUBkgColour;
		pd.colorText = g_Settings->crPUTextColour;
	}
	else {
		pd.colorBack = g_Settings->crLogBackground;
		pd.colorText = crBkg;
	}

	pd.PluginWindowProc = PopupDlgProc;
	pd.PluginData = si;
	return PUAddPopupT(&pd);
}

BOOL DoPopup(SESSION_INFO *si, GCEVENT *gce)
{
	switch (gce->iType) {
	case GC_EVENT_MESSAGE | GC_EVENT_HIGHLIGHT:
		chatApi.ShowPopup(si->hContact, si, Skin_LoadIcon(SKINICON_EVENT_MESSAGE), si->pszModule, si->ptszName, chatApi.aFonts[16].color, TranslateT("%s says: %s"), gce->ptszNick, RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_ACTION | GC_EVENT_HIGHLIGHT:
		chatApi.ShowPopup(si->hContact, si, Skin_LoadIcon(SKINICON_EVENT_MESSAGE), si->pszModule, si->ptszName, chatApi.aFonts[16].color, L"%s %s", gce->ptszNick, RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_MESSAGE:
		chatApi.ShowPopup(si->hContact, si, chatApi.hIcons[ICON_MESSAGE], si->pszModule, si->ptszName, chatApi.aFonts[9].color, TranslateT("%s says: %s"), gce->ptszNick, RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_ACTION:
		chatApi.ShowPopup(si->hContact, si, chatApi.hIcons[ICON_ACTION], si->pszModule, si->ptszName, chatApi.aFonts[15].color, L"%s %s", gce->ptszNick, RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_JOIN:
		chatApi.ShowPopup(si->hContact, si, chatApi.hIcons[ICON_JOIN], si->pszModule, si->ptszName, chatApi.aFonts[3].color, TranslateT("%s has joined"), gce->ptszNick);
		break;
	case GC_EVENT_PART:
		if (!gce->ptszText)
			chatApi.ShowPopup(si->hContact, si, chatApi.hIcons[ICON_PART], si->pszModule, si->ptszName, chatApi.aFonts[4].color, TranslateT("%s has left"), gce->ptszNick);
		else
			chatApi.ShowPopup(si->hContact, si, chatApi.hIcons[ICON_PART], si->pszModule, si->ptszName, chatApi.aFonts[4].color, TranslateT("%s has left (%s)"), gce->ptszNick, RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_QUIT:
		if (!gce->ptszText)
			chatApi.ShowPopup(si->hContact, si, chatApi.hIcons[ICON_QUIT], si->pszModule, si->ptszName, chatApi.aFonts[5].color, TranslateT("%s has disconnected"), gce->ptszNick);
		else
			chatApi.ShowPopup(si->hContact, si, chatApi.hIcons[ICON_QUIT], si->pszModule, si->ptszName, chatApi.aFonts[5].color, TranslateT("%s has disconnected (%s)"), gce->ptszNick, RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_NICK:
		chatApi.ShowPopup(si->hContact, si, chatApi.hIcons[ICON_NICK], si->pszModule, si->ptszName, chatApi.aFonts[7].color, TranslateT("%s is now known as %s"), gce->ptszNick, gce->ptszText);
		break;
	case GC_EVENT_KICK:
		if (!gce->ptszText)
			chatApi.ShowPopup(si->hContact, si, chatApi.hIcons[ICON_KICK], si->pszModule, si->ptszName, chatApi.aFonts[6].color, TranslateT("%s kicked %s"), (char *)gce->ptszStatus, gce->ptszNick);
		else
			chatApi.ShowPopup(si->hContact, si, chatApi.hIcons[ICON_KICK], si->pszModule, si->ptszName, chatApi.aFonts[6].color, TranslateT("%s kicked %s (%s)"), (char *)gce->ptszStatus, gce->ptszNick, RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_NOTICE:
		chatApi.ShowPopup(si->hContact, si, chatApi.hIcons[ICON_NOTICE], si->pszModule, si->ptszName, chatApi.aFonts[8].color, TranslateT("Notice from %s: %s"), gce->ptszNick, RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_TOPIC:
		if (!gce->ptszNick)
			chatApi.ShowPopup(si->hContact, si, chatApi.hIcons[ICON_TOPIC], si->pszModule, si->ptszName, chatApi.aFonts[11].color, TranslateT("The topic is '%s'"), RemoveFormatting(gce->ptszText));
		else
			chatApi.ShowPopup(si->hContact, si, chatApi.hIcons[ICON_TOPIC], si->pszModule, si->ptszName, chatApi.aFonts[11].color, TranslateT("The topic is '%s' (set by %s)"), RemoveFormatting(gce->ptszText), gce->ptszNick);
		break;
	case GC_EVENT_INFORMATION:
		chatApi.ShowPopup(si->hContact, si, chatApi.hIcons[ICON_INFO], si->pszModule, si->ptszName, chatApi.aFonts[12].color, L"%s", RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_ADDSTATUS:
		chatApi.ShowPopup(si->hContact, si, chatApi.hIcons[ICON_ADDSTATUS], si->pszModule, si->ptszName, chatApi.aFonts[13].color, TranslateT("%s enables '%s' status for %s"), gce->ptszText, (char *)gce->ptszStatus, gce->ptszNick);
		break;
	case GC_EVENT_REMOVESTATUS:
		chatApi.ShowPopup(si->hContact, si, chatApi.hIcons[ICON_REMSTATUS], si->pszModule, si->ptszName, chatApi.aFonts[14].color, TranslateT("%s disables '%s' status for %s"), gce->ptszText, (char *)gce->ptszStatus, gce->ptszNick);
		break;
	}

	return TRUE;
}

BOOL DoSoundsFlashPopupTrayStuff(SESSION_INFO *si, GCEVENT *gce, BOOL bHighlight, int bManyFix)
{
	if (!gce || !si || gce->bIsMe || si->iType == GCW_SERVER)
		return FALSE;

	BOOL bInactive = si->pDlg == nullptr || GetForegroundWindow() != si->pDlg->GetHwnd();

	int iEvent = gce->iType;

	if (bHighlight) {
		gce->iType |= GC_EVENT_HIGHLIGHT;
		if (bInactive || !g_Settings->bSoundsFocus)
			Skin_PlaySound("ChatHighlight");
		if (db_get_b(si->hContact, "CList", "Hidden", 0) != 0)
			db_unset(si->hContact, "CList", "Hidden");
		if (bInactive)
			chatApi.DoTrayIcon(si, gce);
		if (bInactive || !g_Settings->bPopupInactiveOnly)
			chatApi.DoPopup(si, gce);
		if (chatApi.OnFlashHighlight)
			chatApi.OnFlashHighlight(si, bInactive);
		return TRUE;
	}

	// do blinking icons in tray
	if (bInactive || !g_Settings->bTrayIconInactiveOnly)
		chatApi.DoTrayIcon(si, gce);

	// stupid thing to not create multiple popups for a QUIT event for instance
	if (bManyFix == 0) {
		// do popups
		if (bInactive || !g_Settings->bPopupInactiveOnly)
			chatApi.DoPopup(si, gce);

		// do sounds and flashing
		switch (iEvent) {
		case GC_EVENT_JOIN:
			if (bInactive || !g_Settings->bSoundsFocus)
				Skin_PlaySound("ChatJoin");
			break;
		case GC_EVENT_PART:
			if (bInactive || !g_Settings->bSoundsFocus)
				Skin_PlaySound("ChatPart");
			break;
		case GC_EVENT_QUIT:
			if (bInactive || !g_Settings->bSoundsFocus)
				Skin_PlaySound("ChatQuit");
			break;
		case GC_EVENT_ADDSTATUS:
		case GC_EVENT_REMOVESTATUS:
			if (bInactive || !g_Settings->bSoundsFocus)
				Skin_PlaySound("ChatMode");
			break;
		case GC_EVENT_KICK:
			if (bInactive || !g_Settings->bSoundsFocus)
				Skin_PlaySound("ChatKick");
			break;
		case GC_EVENT_MESSAGE:
			if (bInactive || !g_Settings->bSoundsFocus)
				Skin_PlaySound("ChatMessage");

			if (bInactive && !(si->wState & STATE_TALK)) {
				si->wState |= STATE_TALK;
				db_set_w(si->hContact, si->pszModule, "ApparentMode", ID_STATUS_OFFLINE);
			}
			if (chatApi.OnFlashWindow)
				chatApi.OnFlashWindow(si, bInactive);
			break;
		case GC_EVENT_ACTION:
			if (bInactive || !g_Settings->bSoundsFocus)
				Skin_PlaySound("ChatAction");
			break;
		case GC_EVENT_NICK:
			if (bInactive || !g_Settings->bSoundsFocus)
				Skin_PlaySound("ChatNick");
			break;
		case GC_EVENT_NOTICE:
			if (bInactive || !g_Settings->bSoundsFocus)
				Skin_PlaySound("ChatNotice");
			break;
		case GC_EVENT_TOPIC:
			if (bInactive || !g_Settings->bSoundsFocus)
				Skin_PlaySound("ChatTopic");
			break;
		}
	}

	return TRUE;
}

const wchar_t* my_strstri(const wchar_t* s1, const wchar_t* s2)
{
	int i, j, k;
	for (i = 0; s1[i]; i++)
		for (j = i, k = 0; towlower(s1[j]) == towlower(s2[k]); j++, k++)
			if (!s2[k + 1])
				return s1 + i;

	return nullptr;
}

static wchar_t szTrimString[] = L":,.!?;\'>)";

bool IsHighlighted(SESSION_INFO *si, GCEVENT *gce)
{
	if (!g_Settings->bHighlightEnabled || !g_Settings->pszHighlightWords || !gce || !si || !si->pMe)
		return FALSE;

	if (gce->ptszText == nullptr)
		return FALSE;

	wchar_t *buf = RemoveFormatting(NEWWSTR_ALLOCA(gce->ptszText));

	int iStart = 0;
	CMStringW tszHighlightWords(g_Settings->pszHighlightWords);

	while (true) {
		CMStringW tszToken = tszHighlightWords.Tokenize(L"\t ", iStart);
		if (iStart == -1)
			break;

		// replace %m with the users nickname
		if (tszToken == L"%m")
			tszToken = si->pMe->pszNick;

		if (tszToken.Find('*') == -1)
			tszToken = '*' + tszToken + '*';

		// time to get the next/first word in the incoming text string
		for (const wchar_t *p = buf; *p != '\0'; p += wcscspn(p, L" ")) {
			p += _tcsspn(p, L" ");

			// compare the words, using wildcards
			if (wildcmpiw(p, tszToken))
				return TRUE;
		}
	}

	return FALSE;
}

BOOL LogToFile(SESSION_INFO *si, GCEVENT *gce)
{
	wchar_t szBuffer[4096];
	wchar_t szLine[4096];
	wchar_t p = '\0';
	szBuffer[0] = '\0';

	GetChatLogsFilename(si, gce->time);
	BOOL bFileJustCreated = !PathFileExists(si->pszLogFileName);

	wchar_t tszFolder[MAX_PATH];
	wcsncpy_s(tszFolder, si->pszLogFileName, _TRUNCATE);
	PathRemoveFileSpec(tszFolder);
	if (!PathIsDirectory(tszFolder))
		CreateDirectoryTreeW(tszFolder);

	wchar_t szTime[100];
	mir_wstrncpy(szTime, chatApi.MakeTimeStamp(g_Settings->pszTimeStampLog, gce->time), 99);

	FILE *hFile = _wfopen(si->pszLogFileName, L"ab+");
	if (hFile == nullptr)
		return FALSE;

	wchar_t szTemp[512], szTemp2[512];
	wchar_t* pszNick = nullptr;
	if (bFileJustCreated)
		fputws((const wchar_t*)"\377\376", hFile);		//UTF-16 LE BOM == FF FE
	if (gce->ptszNick) {
		if (g_Settings->bLogLimitNames && mir_wstrlen(gce->ptszNick) > 20) {
			mir_wstrncpy(szTemp2, gce->ptszNick, 20);
			mir_wstrncpy(szTemp2 + 20, L"...", 4);
		}
		else mir_wstrncpy(szTemp2, gce->ptszNick, 511);

		if (gce->ptszUserInfo)
			mir_snwprintf(szTemp, L"%s (%s)", szTemp2, gce->ptszUserInfo);
		else
			wcsncpy_s(szTemp, szTemp2, _TRUNCATE);
		pszNick = szTemp;
	}

	switch (gce->iType) {
	case GC_EVENT_MESSAGE:
	case GC_EVENT_MESSAGE | GC_EVENT_HIGHLIGHT:
		p = '*';
		mir_snwprintf(szBuffer, L"%s: %s", gce->ptszNick, chatApi.RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_ACTION:
	case GC_EVENT_ACTION | GC_EVENT_HIGHLIGHT:
		p = '*';
		mir_snwprintf(szBuffer, L"%s %s", gce->ptszNick, chatApi.RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_JOIN:
		p = '>';
		mir_snwprintf(szBuffer, TranslateT("%s has joined"), pszNick);
		break;
	case GC_EVENT_PART:
		p = '<';
		if (!gce->ptszText)
			mir_snwprintf(szBuffer, TranslateT("%s has left"), pszNick);
		else
			mir_snwprintf(szBuffer, TranslateT("%s has left (%s)"), pszNick, chatApi.RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_QUIT:
		p = '<';
		if (!gce->ptszText)
			mir_snwprintf(szBuffer, TranslateT("%s has disconnected"), pszNick);
		else
			mir_snwprintf(szBuffer, TranslateT("%s has disconnected (%s)"), pszNick, chatApi.RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_NICK:
		p = '^';
		mir_snwprintf(szBuffer, TranslateT("%s is now known as %s"), gce->ptszNick, gce->ptszText);
		break;
	case GC_EVENT_KICK:
		p = '~';
		if (!gce->ptszText)
			mir_snwprintf(szBuffer, TranslateT("%s kicked %s"), gce->ptszStatus, gce->ptszNick);
		else
			mir_snwprintf(szBuffer, TranslateT("%s kicked %s (%s)"), gce->ptszStatus, gce->ptszNick, chatApi.RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_NOTICE:
		p = 'o';
		mir_snwprintf(szBuffer, TranslateT("Notice from %s: %s"), gce->ptszNick, chatApi.RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_TOPIC:
		p = '#';
		if (!gce->ptszNick)
			mir_snwprintf(szBuffer, TranslateT("The topic is '%s'"), chatApi.RemoveFormatting(gce->ptszText));
		else
			mir_snwprintf(szBuffer, TranslateT("The topic is '%s' (set by %s)"), chatApi.RemoveFormatting(gce->ptszText), gce->ptszNick);
		break;
	case GC_EVENT_INFORMATION:
		p = '!';
		wcsncpy_s(szBuffer, chatApi.RemoveFormatting(gce->ptszText), _TRUNCATE);
		break;
	case GC_EVENT_ADDSTATUS:
		p = '+';
		mir_snwprintf(szBuffer, TranslateT("%s enables '%s' status for %s"), gce->ptszText, gce->ptszStatus, gce->ptszNick);
		break;
	case GC_EVENT_REMOVESTATUS:
		p = '-';
		mir_snwprintf(szBuffer, TranslateT("%s disables '%s' status for %s"), gce->ptszText, gce->ptszStatus, gce->ptszNick);
		break;
	}

	// formatting strings don't need to be translatable - changing them via language pack would
	// only screw up the log format.
	if (p)
		mir_snwprintf(szLine, L"%s %c %s\r\n", szTime, p, szBuffer);
	else
		mir_snwprintf(szLine, L"%s %s\r\n", szTime, szBuffer);

	if (szLine[0]) {
		fputws(szLine, hFile);

		if (g_Settings->LoggingLimit > 0) {
			fseek(hFile, 0, SEEK_END);
			long dwSize = ftell(hFile);
			rewind(hFile);

			long trimlimit = g_Settings->LoggingLimit * 1024;
			if (dwSize > trimlimit) {
				time_t now = time(0);

				wchar_t tszTimestamp[20];
				wcsftime(tszTimestamp, 20, L"%Y%m%d-%H%M%S", _localtime32((__time32_t *)&now));
				tszTimestamp[19] = 0;

				// max size reached, rotate the log
				// move old logs to /archived sub folder just inside the log root folder.
				// add a time stamp to the file name.
				wchar_t tszDrive[_MAX_DRIVE], tszDir[_MAX_DIR], tszName[_MAX_FNAME], tszExt[_MAX_EXT];
				_wsplitpath(si->pszLogFileName, tszDrive, tszDir, tszName, tszExt);

				wchar_t tszNewPath[_MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT + 20];
				mir_snwprintf(tszNewPath, L"%s%sarchived\\", tszDrive, tszDir);
				CreateDirectoryTreeW(tszNewPath);

				wchar_t tszNewName[_MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT + 20];
				mir_snwprintf(tszNewName, L"%s%s-%s%s", tszNewPath, tszName, tszTimestamp, tszExt);
				fclose(hFile);
				hFile = 0;
				if (!PathFileExists(tszNewName))
					CopyFile(si->pszLogFileName, tszNewName, TRUE);
				DeleteFile(si->pszLogFileName);
			}
		}
	}

	if (hFile)
		fclose(hFile);
	return TRUE;
}

MIR_APP_DLL(BOOL) Chat_DoEventHook(SESSION_INFO *si, int iType, const USERINFO *pUser, const wchar_t* pszText, INT_PTR dwItem)
{
	if (si == nullptr)
		return FALSE;

	GCHOOK gch = { si->pszModule, si->ptszID, iType };
	if (pUser != nullptr) {
		gch.ptszUID = pUser->pszUID;
		gch.ptszNick = pUser->pszNick;
	}
	else gch.ptszUID = gch.ptszNick = nullptr;

	gch.ptszText = (LPTSTR)pszText;
	gch.dwData = dwItem;
	NotifyEventHooks(hevSendEvent, 0, (WPARAM)&gch);
	return TRUE;
}

BOOL IsEventSupported(int eventType)
{
	// Supported events
	switch (eventType) {
	case GC_EVENT_JOIN:
	case GC_EVENT_PART:
	case GC_EVENT_QUIT:
	case GC_EVENT_KICK:
	case GC_EVENT_NICK:
	case GC_EVENT_NOTICE:
	case GC_EVENT_MESSAGE:
	case GC_EVENT_TOPIC:
	case GC_EVENT_INFORMATION:
	case GC_EVENT_ACTION:
	case GC_EVENT_ADDSTATUS:
	case GC_EVENT_REMOVESTATUS:
	case GC_EVENT_SETCONTACTSTATUS:
		return TRUE;
	}

	// Other events
	return FALSE;
}

void ValidateFilename(wchar_t *filename)
{
	wchar_t *p1 = filename;
	wchar_t szForbidden[] = L"\\/:*?\"<>|";
	while (*p1 != '\0') {
		if (wcschr(szForbidden, *p1))
			*p1 = '_';
		p1 += 1;
	}
}

static wchar_t tszOldTimeStamp[30];

wchar_t* GetChatLogsFilename(SESSION_INFO *si, time_t tTime)
{
	if (!tTime)
		time(&tTime);

	// check whether relevant parts of the timestamp have changed and
	// we have to reparse the filename
	wchar_t *tszNow = chatApi.MakeTimeStamp(L"%a%d%m%Y", tTime); // once a day
	if (mir_wstrcmp(tszOldTimeStamp, tszNow)) {
		wcsncpy_s(tszOldTimeStamp, tszNow, _TRUNCATE);
		*si->pszLogFileName = 0;
	}

	if (si->pszLogFileName[0] == 0) {
		REPLACEVARSARRAY rva[11];
		rva[0].key.w = L"d";
		rva[0].value.w = mir_wstrdup(chatApi.MakeTimeStamp(L"%#d", tTime));
		// day 01-31
		rva[1].key.w = L"dd";
		rva[1].value.w = mir_wstrdup(chatApi.MakeTimeStamp(L"%d", tTime));
		// month 1-12
		rva[2].key.w = L"m";
		rva[2].value.w = mir_wstrdup(chatApi.MakeTimeStamp(L"%#m", tTime));
		// month 01-12
		rva[3].key.w = L"mm";
		rva[3].value.w = mir_wstrdup(chatApi.MakeTimeStamp(L"%m", tTime));
		// month text short
		rva[4].key.w = L"mon";
		rva[4].value.w = mir_wstrdup(chatApi.MakeTimeStamp(L"%b", tTime));
		// month text
		rva[5].key.w = L"month";
		rva[5].value.w = mir_wstrdup(chatApi.MakeTimeStamp(L"%B", tTime));
		// year 01-99
		rva[6].key.w = L"yy";
		rva[6].value.w = mir_wstrdup(chatApi.MakeTimeStamp(L"%y", tTime));
		// year 1901-9999
		rva[7].key.w = L"yyyy";
		rva[7].value.w = mir_wstrdup(chatApi.MakeTimeStamp(L"%Y", tTime));
		// weekday short
		rva[8].key.w = L"wday";
		rva[8].value.w = mir_wstrdup(chatApi.MakeTimeStamp(L"%a", tTime));
		// weekday
		rva[9].key.w = L"weekday";
		rva[9].value.w = mir_wstrdup(chatApi.MakeTimeStamp(L"%A", tTime));
		// end of array
		rva[10].key.w = nullptr;
		rva[10].value.w = nullptr;

		wchar_t tszTemp[MAX_PATH], *ptszVarPath;
		if (g_Settings->pszLogDir[mir_wstrlen(g_Settings->pszLogDir) - 1] == '\\') {
			mir_snwprintf(tszTemp, L"%s%s", g_Settings->pszLogDir, L"%userid%.log");
			ptszVarPath = tszTemp;
		}
		else ptszVarPath = g_Settings->pszLogDir;

		wchar_t *tszParsedName = Utils_ReplaceVarsW(ptszVarPath, si->hContact, rva);
		if (chatApi.OnGetLogName)
			chatApi.OnGetLogName(si, tszParsedName);
		else
			PathToAbsoluteW(tszParsedName, si->pszLogFileName);
		mir_free(tszParsedName);

		for (int i = 0; i < _countof(rva); i++)
			mir_free(rva[i].value.w);

		for (wchar_t *p = si->pszLogFileName + 2; *p; ++p)
			if (*p == ':' || *p == '*' || *p == '?' || *p == '"' || *p == '<' || *p == '>' || *p == '|')
				*p = '_';
	}

	return si->pszLogFileName;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(wchar_t*) Chat_UnescapeTags(wchar_t *str_in)
{
	wchar_t *s = str_in, *d = str_in;
	while (*s) {
		if (*s == '%' && s[1] == '%')
			s++;
		*d++ = *s++;
	}
	*d = 0;
	return str_in;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(void) Chat_AddMenuItems(HMENU hMenu, int nItems, const gc_item *Item, int _hLang)
{
	if (nItems > 0)
		AppendMenu(hMenu, MF_SEPARATOR, 0, 0);

	HMENU hSubMenu = nullptr;
	for (int i = 0; i < nItems; i++) {
		wchar_t *ptszText = TranslateW_LP(Item[i].pszDesc, _hLang);
		DWORD dwState = Item[i].bDisabled ? MF_GRAYED : 0;

		if (Item[i].uType == MENU_NEWPOPUP) {
			hSubMenu = CreateMenu();
			AppendMenu(hMenu, dwState | MF_POPUP, (UINT_PTR)hSubMenu, ptszText);
		}
		else if (Item[i].uType == MENU_POPUPHMENU)
			AppendMenu(hSubMenu == 0 ? hMenu : hSubMenu, dwState | MF_POPUP, Item[i].dwID, ptszText);
		else if (Item[i].uType == MENU_POPUPITEM)
			AppendMenu(hSubMenu == 0 ? hMenu : hSubMenu, dwState | MF_STRING, Item[i].dwID, ptszText);
		else if (Item[i].uType == MENU_POPUPCHECK)
			AppendMenu(hSubMenu == 0 ? hMenu : hSubMenu, dwState | MF_CHECKED | MF_STRING, Item[i].dwID, ptszText);
		else if (Item[i].uType == MENU_POPUPSEPARATOR)
			AppendMenu(hSubMenu == 0 ? hMenu : hSubMenu, MF_SEPARATOR, 0, ptszText);
		else if (Item[i].uType == MENU_SEPARATOR)
			AppendMenu(hMenu, MF_SEPARATOR, 0, ptszText);
		else if (Item[i].uType == MENU_HMENU)
			AppendMenu(hMenu, dwState | MF_POPUP, Item[i].dwID, ptszText);
		else if (Item[i].uType == MENU_ITEM)
			AppendMenu(hMenu, dwState | MF_STRING, Item[i].dwID, ptszText);
		else if (Item[i].uType == MENU_CHECK)
			AppendMenu(hMenu, dwState | MF_CHECKED | MF_STRING, Item[i].dwID, ptszText);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(UINT) Chat_CreateGCMenu(HWND hwnd, HMENU hMenu, POINT pt, SESSION_INFO *si, const wchar_t *pszUID, const wchar_t *pszWordText)
{
	GCMENUITEMS gcmi = {};
	gcmi.pszID = si->ptszID;
	gcmi.pszModule = si->pszModule;
	gcmi.pszUID = (wchar_t*)pszUID;
	gcmi.hMenu = hMenu;

	if (pszUID == nullptr) {
		int flags = MF_BYPOSITION | (GetRichTextLength(hwnd) == 0 ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem(hMenu, 0, flags);
		EnableMenuItem(hMenu, 2, flags);

		if (pszWordText && pszWordText[0]) {
			wchar_t szMenuText[4096];
			mir_snwprintf(szMenuText, TranslateT("Look up '%s':"), pszWordText);
			ModifyMenu(hMenu, 4, MF_STRING | MF_BYPOSITION, 4, szMenuText);
		}
		else ModifyMenu(hMenu, 4, MF_STRING | MF_GRAYED | MF_BYPOSITION, 4, TranslateT("No word to look up"));
		gcmi.Type = MENU_ON_LOG;
	}
	else {
		wchar_t szTemp[50];
		if (pszWordText)
			mir_snwprintf(szTemp, TranslateT("&Message %s"), pszWordText);
		else
			mir_wstrncpy(szTemp, TranslateT("&Message"), _countof(szTemp) - 1);

		if (mir_wstrlen(szTemp) > 40)
			mir_wstrncpy(szTemp + 40, L"...", 4);
		ModifyMenu(hMenu, 0, MF_STRING | MF_BYPOSITION, IDM_SENDMESSAGE, szTemp);
		gcmi.Type = MENU_ON_NICKLIST;
	}

	NotifyEventHooks(hevBuildMenuEvent, 0, (WPARAM)&gcmi);

	return TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, nullptr);
}

MIR_APP_DLL(void) Chat_DestroyGCMenu(HMENU hMenu, int iIndex)
{
	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_SUBMENU;
	while (GetMenuItemInfo(hMenu, iIndex, TRUE, &mii)) {
		if (mii.hSubMenu != nullptr)
			DestroyMenu(mii.hSubMenu);
		RemoveMenu(hMenu, iIndex, MF_BYPOSITION);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// calculates the required rectangle for a string using the given font. This is more
// precise than using GetTextExtentPoint...()

MIR_APP_DLL(int) Chat_GetTextPixelSize(const wchar_t *pszText, HFONT hFont, bool bWidth)
{
	if (!pszText || !hFont)
		return 0;

	HDC hdc = GetDC(nullptr);
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

	RECT rc = { 0 };
	DrawText(hdc, pszText, -1, &rc, DT_CALCRECT);

	SelectObject(hdc, hOldFont);
	ReleaseDC(nullptr, hdc);
	return bWidth ? rc.right - rc.left : rc.bottom - rc.top;
}
