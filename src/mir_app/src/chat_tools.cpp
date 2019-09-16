/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-19 Miranda NG team,
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
				__fallthrough;

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
		g_chatApi.AddEvent(si->hContact, Skin_LoadIcon(SKINICON_EVENT_MESSAGE), GC_FAKE_EVENT, 0, TranslateT("%s wants your attention in %s"), gce->pszNick.w, si->ptszName);
		break;
	case GC_EVENT_MESSAGE:
		g_chatApi.AddEvent(si->hContact, g_chatApi.hIcons[ICON_MESSAGE], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s speaks in %s"), gce->pszNick.w, si->ptszName);
		break;
	case GC_EVENT_ACTION:
		g_chatApi.AddEvent(si->hContact, g_chatApi.hIcons[ICON_ACTION], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s speaks in %s"), gce->pszNick.w, si->ptszName);
		break;
	case GC_EVENT_JOIN:
		g_chatApi.AddEvent(si->hContact, g_chatApi.hIcons[ICON_JOIN], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s has joined %s"), gce->pszNick.w, si->ptszName);
		break;
	case GC_EVENT_PART:
		g_chatApi.AddEvent(si->hContact, g_chatApi.hIcons[ICON_PART], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s has left %s"), gce->pszNick.w, si->ptszName);
		break;
	case GC_EVENT_QUIT:
		g_chatApi.AddEvent(si->hContact, g_chatApi.hIcons[ICON_QUIT], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s has disconnected"), gce->pszNick.w);
		break;
	case GC_EVENT_NICK:
		g_chatApi.AddEvent(si->hContact, g_chatApi.hIcons[ICON_NICK], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s is now known as %s"), gce->pszNick.w, gce->pszText.w);
		break;
	case GC_EVENT_KICK:
		g_chatApi.AddEvent(si->hContact, g_chatApi.hIcons[ICON_KICK], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s kicked %s from %s"), gce->pszStatus.w, gce->pszNick.w, si->ptszName);
		break;
	case GC_EVENT_NOTICE:
		g_chatApi.AddEvent(si->hContact, g_chatApi.hIcons[ICON_NOTICE], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("Notice from %s"), gce->pszNick.w);
		break;
	case GC_EVENT_TOPIC:
		g_chatApi.AddEvent(si->hContact, g_chatApi.hIcons[ICON_TOPIC], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("Topic change in %s"), si->ptszName);
		break;
	case GC_EVENT_INFORMATION:
		g_chatApi.AddEvent(si->hContact, g_chatApi.hIcons[ICON_INFO], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("Information in %s"), si->ptszName);
		break;
	case GC_EVENT_ADDSTATUS:
		g_chatApi.AddEvent(si->hContact, g_chatApi.hIcons[ICON_ADDSTATUS], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s enables '%s' status for %s in %s"), gce->pszText.w, gce->pszStatus.w, gce->pszNick.w, si->ptszName);
		break;
	case GC_EVENT_REMOVESTATUS:
		g_chatApi.AddEvent(si->hContact, g_chatApi.hIcons[ICON_REMSTATUS], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s disables '%s' status for %s in %s"), gce->pszText.w, gce->pszStatus.w, gce->pszNick.w, si->ptszName);
		break;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void __stdcall ShowRoomFromPopup(void *pi)
{
	SESSION_INFO *si = (SESSION_INFO*)pi;
	g_chatApi.ShowRoom(si);
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
			if (g_clistApi.pfnGetEvent(si->hContact, 0))
				g_clistApi.pfnRemoveEvent(si->hContact, GC_FAKE_EVENT);

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

	POPUPDATAW pd;
	pd.lchContact = hContact;

	if (hIcon)
		pd.lchIcon = hIcon;
	else
		pd.lchIcon = LoadIconEx("window", FALSE);

	PROTOACCOUNT *pa = Proto_GetAccount(pszProtoName);
	mir_snwprintf(pd.lpwzContactName, L"%s - %s", (pa == nullptr) ? _A2T(pszProtoName).get() : pa->tszAccountName, Clist_GetContactDisplayName(hContact));
	mir_wstrncpy(pd.lpwzText, TranslateW(szBuf), _countof(pd.lpwzText));
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
	return (INT_PTR)PUAddPopupW(&pd);
}

BOOL DoPopup(SESSION_INFO *si, GCEVENT *gce)
{
	switch (gce->iType) {
	case GC_EVENT_MESSAGE | GC_EVENT_HIGHLIGHT:
		g_chatApi.ShowPopup(si->hContact, si, Skin_LoadIcon(SKINICON_EVENT_MESSAGE), si->pszModule, si->ptszName, g_chatApi.aFonts[16].color, TranslateT("%s says: %s"), gce->pszNick.w, RemoveFormatting(gce->pszText.w));
		break;
	case GC_EVENT_ACTION | GC_EVENT_HIGHLIGHT:
		g_chatApi.ShowPopup(si->hContact, si, Skin_LoadIcon(SKINICON_EVENT_MESSAGE), si->pszModule, si->ptszName, g_chatApi.aFonts[16].color, L"%s %s", gce->pszNick.w, RemoveFormatting(gce->pszText.w));
		break;
	case GC_EVENT_MESSAGE:
		g_chatApi.ShowPopup(si->hContact, si, g_chatApi.hIcons[ICON_MESSAGE], si->pszModule, si->ptszName, g_chatApi.aFonts[9].color, TranslateT("%s says: %s"), gce->pszNick.w, RemoveFormatting(gce->pszText.w));
		break;
	case GC_EVENT_ACTION:
		g_chatApi.ShowPopup(si->hContact, si, g_chatApi.hIcons[ICON_ACTION], si->pszModule, si->ptszName, g_chatApi.aFonts[15].color, L"%s %s", gce->pszNick.w, RemoveFormatting(gce->pszText.w));
		break;
	case GC_EVENT_JOIN:
		g_chatApi.ShowPopup(si->hContact, si, g_chatApi.hIcons[ICON_JOIN], si->pszModule, si->ptszName, g_chatApi.aFonts[3].color, TranslateT("%s has joined"), gce->pszNick.w);
		break;
	case GC_EVENT_PART:
		if (!gce->pszText.w)
			g_chatApi.ShowPopup(si->hContact, si, g_chatApi.hIcons[ICON_PART], si->pszModule, si->ptszName, g_chatApi.aFonts[4].color, TranslateT("%s has left"), gce->pszNick.w);
		else
			g_chatApi.ShowPopup(si->hContact, si, g_chatApi.hIcons[ICON_PART], si->pszModule, si->ptszName, g_chatApi.aFonts[4].color, TranslateT("%s has left (%s)"), gce->pszNick.w, RemoveFormatting(gce->pszText.w));
		break;
	case GC_EVENT_QUIT:
		if (!gce->pszText.w)
			g_chatApi.ShowPopup(si->hContact, si, g_chatApi.hIcons[ICON_QUIT], si->pszModule, si->ptszName, g_chatApi.aFonts[5].color, TranslateT("%s has disconnected"), gce->pszNick.w);
		else
			g_chatApi.ShowPopup(si->hContact, si, g_chatApi.hIcons[ICON_QUIT], si->pszModule, si->ptszName, g_chatApi.aFonts[5].color, TranslateT("%s has disconnected (%s)"), gce->pszNick.w, RemoveFormatting(gce->pszText.w));
		break;
	case GC_EVENT_NICK:
		g_chatApi.ShowPopup(si->hContact, si, g_chatApi.hIcons[ICON_NICK], si->pszModule, si->ptszName, g_chatApi.aFonts[7].color, TranslateT("%s is now known as %s"), gce->pszNick.w, gce->pszText.w);
		break;
	case GC_EVENT_KICK:
		if (!gce->pszText.w)
			g_chatApi.ShowPopup(si->hContact, si, g_chatApi.hIcons[ICON_KICK], si->pszModule, si->ptszName, g_chatApi.aFonts[6].color, TranslateT("%s kicked %s"), gce->pszStatus.w, gce->pszNick.w);
		else
			g_chatApi.ShowPopup(si->hContact, si, g_chatApi.hIcons[ICON_KICK], si->pszModule, si->ptszName, g_chatApi.aFonts[6].color, TranslateT("%s kicked %s (%s)"), gce->pszStatus.w, gce->pszNick.w, RemoveFormatting(gce->pszText.w));
		break;
	case GC_EVENT_NOTICE:
		g_chatApi.ShowPopup(si->hContact, si, g_chatApi.hIcons[ICON_NOTICE], si->pszModule, si->ptszName, g_chatApi.aFonts[8].color, TranslateT("Notice from %s: %s"), gce->pszNick.w, RemoveFormatting(gce->pszText.w));
		break;
	case GC_EVENT_TOPIC:
		if (!gce->pszNick.w)
			g_chatApi.ShowPopup(si->hContact, si, g_chatApi.hIcons[ICON_TOPIC], si->pszModule, si->ptszName, g_chatApi.aFonts[11].color, TranslateT("The topic is '%s'"), RemoveFormatting(gce->pszText.w));
		else
			g_chatApi.ShowPopup(si->hContact, si, g_chatApi.hIcons[ICON_TOPIC], si->pszModule, si->ptszName, g_chatApi.aFonts[11].color, TranslateT("The topic is '%s' (set by %s)"), RemoveFormatting(gce->pszText.w), gce->pszNick.w);
		break;
	case GC_EVENT_INFORMATION:
		g_chatApi.ShowPopup(si->hContact, si, g_chatApi.hIcons[ICON_INFO], si->pszModule, si->ptszName, g_chatApi.aFonts[12].color, L"%s", RemoveFormatting(gce->pszText.w));
		break;
	case GC_EVENT_ADDSTATUS:
		g_chatApi.ShowPopup(si->hContact, si, g_chatApi.hIcons[ICON_ADDSTATUS], si->pszModule, si->ptszName, g_chatApi.aFonts[13].color, TranslateT("%s enables '%s' status for %s"), gce->pszText.w, (char *)gce->pszStatus.w, gce->pszNick.w);
		break;
	case GC_EVENT_REMOVESTATUS:
		g_chatApi.ShowPopup(si->hContact, si, g_chatApi.hIcons[ICON_REMSTATUS], si->pszModule, si->ptszName, g_chatApi.aFonts[14].color, TranslateT("%s disables '%s' status for %s"), gce->pszText.w, (char *)gce->pszStatus.w, gce->pszNick.w);
		break;
	}

	return TRUE;
}

static bool ContainsWindow(HWND hwndOwner, HWND hwndChild)
{
	while (hwndChild != nullptr) {
		if (hwndChild == hwndOwner)
			return true;

		hwndChild = GetParent(hwndChild);
	}
	return false;
}

BOOL DoSoundsFlashPopupTrayStuff(SESSION_INFO *si, GCEVENT *gce, BOOL bHighlight, int bManyFix)
{
	if (!gce || !si || gce->bIsMe || si->iType == GCW_SERVER)
		return FALSE;

	BOOL bInactive = si->pDlg == nullptr || !si->pDlg->IsActive();

	int iEvent = gce->iType;

	if (bHighlight) {
		gce->iType |= GC_EVENT_HIGHLIGHT;
		if (bInactive || !g_Settings->bSoundsFocus)
			Skin_PlaySound("ChatHighlight");
		if (db_get_b(si->hContact, "CList", "Hidden", 0) != 0)
			db_unset(si->hContact, "CList", "Hidden");
		if (bInactive)
			g_chatApi.DoTrayIcon(si, gce);
		if (bInactive || !g_Settings->bPopupInactiveOnly)
			g_chatApi.DoPopup(si, gce);
		if (g_chatApi.OnFlashHighlight)
			g_chatApi.OnFlashHighlight(si, bInactive);
		return TRUE;
	}

	// do blinking icons in tray
	if (bInactive || !g_Settings->bTrayIconInactiveOnly)
		g_chatApi.DoTrayIcon(si, gce);

	// stupid thing to not create multiple popups for a QUIT event for instance
	if (bManyFix == 0) {
		// do popups
		if (bInactive || !g_Settings->bPopupInactiveOnly)
			g_chatApi.DoPopup(si, gce);

		// do sounds and flashing
		const char *szSound = nullptr;
		switch (iEvent) {
		case GC_EVENT_JOIN:           szSound = "ChatJoin";   break;
		case GC_EVENT_PART:           szSound = "ChatPart";   break;
		case GC_EVENT_QUIT:           szSound = "ChatQuit";   break;
		case GC_EVENT_ADDSTATUS:
		case GC_EVENT_REMOVESTATUS:   szSound = "ChatMode";   break;
		case GC_EVENT_KICK:           szSound = "ChatKick";   break;
		case GC_EVENT_ACTION:         szSound = "ChatAction"; break;
		case GC_EVENT_NICK:           szSound = "ChatNick";   break;
		case GC_EVENT_NOTICE:         szSound = "ChatNotice"; break;
		case GC_EVENT_TOPIC:          szSound = "ChatTopic";  break;
		case GC_EVENT_MESSAGE:
			szSound = "ChatMessage";

			if (bInactive && !(si->wState & STATE_TALK)) {
				si->wState |= STATE_TALK;
				db_set_w(si->hContact, si->pszModule, "ApparentMode", ID_STATUS_OFFLINE);
			}
			if (g_chatApi.OnFlashWindow)
				g_chatApi.OnFlashWindow(si, bInactive);
			break;
		}

		if (szSound && (bInactive || !g_Settings->bSoundsFocus))
			Skin_PlaySound(szSound);
	}

	return TRUE;
}

const wchar_t* my_strstri(const wchar_t *s1, const wchar_t *s2)
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
	if (!g_Settings->bHighlightEnabled || !g_Settings->pszHighlightWords || !gce || !si)
		return FALSE;

	if (gce->pszText.w == nullptr)
		return FALSE;

	USERINFO *pMe = si->getMe();
	if (pMe == nullptr)
		return FALSE;

	wchar_t *buf = RemoveFormatting(NEWWSTR_ALLOCA(gce->pszText.w));

	int iStart = 0;
	CMStringW tszHighlightWords(g_Settings->pszHighlightWords);

	while (true) {
		CMStringW tszToken = tszHighlightWords.Tokenize(L"\t ", iStart);
		if (iStart == -1)
			break;

		// replace %m with the users nickname
		if (tszToken == L"%m")
			tszToken = pMe->pszNick;

		if (tszToken.Find('*') == -1)
			tszToken = '*' + tszToken + '*';

		// time to get the next/first word in the incoming text string
		for (const wchar_t *p = buf; *p != '\0'; p += wcscspn(p, L" ")) {
			p += wcsspn(p, L" ");

			// compare the words, using wildcards
			if (wildcmpiw(p, tszToken))
				return TRUE;
		}
	}

	return FALSE;
}

BOOL LogToFile(SESSION_INFO *si, GCEVENT *gce)
{
	wchar_t p = '\0';

	GetChatLogsFilename(si, gce->time);
	BOOL bFileJustCreated = !PathFileExists(si->pszLogFileName);

	wchar_t tszFolder[MAX_PATH];
	wcsncpy_s(tszFolder, si->pszLogFileName, _TRUNCATE);
	PathRemoveFileSpec(tszFolder);
	if (!PathIsDirectory(tszFolder))
		CreateDirectoryTreeW(tszFolder);

	FILE *hFile = _wfopen(si->pszLogFileName, L"ab+");
	if (hFile == nullptr)
		return FALSE;

	wchar_t szTemp[512], szTemp2[512];
	wchar_t *pszNick = nullptr;
	if (bFileJustCreated)
		fwrite("\377\376", 1, 2, hFile); // UTF-16 LE BOM == FF FE
	if (gce->pszNick.w) {
		if (g_Settings->bLogLimitNames && mir_wstrlen(gce->pszNick.w) > 20) {
			mir_wstrncpy(szTemp2, gce->pszNick.w, 20);
			mir_wstrncpy(szTemp2 + 20, L"...", 4);
		}
		else mir_wstrncpy(szTemp2, gce->pszNick.w, 511);

		if (gce->pszUserInfo.w)
			mir_snwprintf(szTemp, L"%s (%s)", szTemp2, gce->pszUserInfo.w);
		else
			wcsncpy_s(szTemp, szTemp2, _TRUNCATE);
		pszNick = szTemp;
	}

	CMStringW buf;
	switch (gce->iType) {
	case GC_EVENT_MESSAGE:
	case GC_EVENT_MESSAGE | GC_EVENT_HIGHLIGHT:
		p = '*';
		buf.AppendFormat(L"%s: %s", gce->pszNick.w, g_chatApi.RemoveFormatting(gce->pszText.w));
		break;
	case GC_EVENT_ACTION:
	case GC_EVENT_ACTION | GC_EVENT_HIGHLIGHT:
		p = '*';
		buf.AppendFormat(L"%s %s", gce->pszNick.w, g_chatApi.RemoveFormatting(gce->pszText.w));
		break;
	case GC_EVENT_JOIN:
		p = '>';
		buf.AppendFormat(TranslateT("%s has joined"), pszNick);
		break;
	case GC_EVENT_PART:
		p = '<';
		if (!gce->pszText.w)
			buf.AppendFormat(TranslateT("%s has left"), pszNick);
		else
			buf.AppendFormat(TranslateT("%s has left (%s)"), pszNick, g_chatApi.RemoveFormatting(gce->pszText.w));
		break;
	case GC_EVENT_QUIT:
		p = '<';
		if (!gce->pszText.w)
			buf.AppendFormat(TranslateT("%s has disconnected"), pszNick);
		else
			buf.AppendFormat(TranslateT("%s has disconnected (%s)"), pszNick, g_chatApi.RemoveFormatting(gce->pszText.w));
		break;
	case GC_EVENT_NICK:
		p = '^';
		buf.AppendFormat(TranslateT("%s is now known as %s"), gce->pszNick.w, gce->pszText.w);
		break;
	case GC_EVENT_KICK:
		p = '~';
		if (!gce->pszText.w)
			buf.AppendFormat(TranslateT("%s kicked %s"), gce->pszStatus.w, gce->pszNick.w);
		else
			buf.AppendFormat(TranslateT("%s kicked %s (%s)"), gce->pszStatus.w, gce->pszNick.w, g_chatApi.RemoveFormatting(gce->pszText.w));
		break;
	case GC_EVENT_NOTICE:
		p = 'o';
		buf.AppendFormat(TranslateT("Notice from %s: %s"), gce->pszNick.w, g_chatApi.RemoveFormatting(gce->pszText.w));
		break;
	case GC_EVENT_TOPIC:
		p = '#';
		if (!gce->pszNick.w)
			buf.AppendFormat(TranslateT("The topic is '%s'"), g_chatApi.RemoveFormatting(gce->pszText.w));
		else
			buf.AppendFormat(TranslateT("The topic is '%s' (set by %s)"), g_chatApi.RemoveFormatting(gce->pszText.w), gce->pszNick.w);
		break;
	case GC_EVENT_INFORMATION:
		p = '!';
		buf = g_chatApi.RemoveFormatting(gce->pszText.w);
		break;
	case GC_EVENT_ADDSTATUS:
		p = '+';
		buf.AppendFormat(TranslateT("%s enables '%s' status for %s"), gce->pszText.w, gce->pszStatus.w, gce->pszNick.w);
		break;
	case GC_EVENT_REMOVESTATUS:
		p = '-';
		buf.AppendFormat(TranslateT("%s disables '%s' status for %s"), gce->pszText.w, gce->pszStatus.w, gce->pszNick.w);
		break;
	}

	// formatting strings don't need to be translatable - changing them via language pack would
	// only screw up the log format.
	wchar_t *szTime = g_chatApi.MakeTimeStamp(g_Settings->pszTimeStampLog, gce->time);
	CMStringW szLine;
	if (p)
		szLine.Format(L"%s %c %s\r\n", szTime, p, buf.c_str());
	else
		szLine.Format(L"%s %s\r\n", szTime, buf.c_str());

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
				hFile = nullptr;
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

MIR_APP_DLL(BOOL) Chat_DoEventHook(SESSION_INFO *si, int iType, const USERINFO *pUser, const wchar_t *pszText, INT_PTR dwItem)
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
	gch.pDlg = si->pDlg;
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
	wchar_t *tszNow = g_chatApi.MakeTimeStamp(L"%a%d%m%Y", tTime); // once a day
	if (mir_wstrcmp(tszOldTimeStamp, tszNow)) {
		wcsncpy_s(tszOldTimeStamp, tszNow, _TRUNCATE);
		*si->pszLogFileName = 0;
	}

	if (si->pszLogFileName[0] == 0) {
		REPLACEVARSARRAY rva[11];
		rva[0].key.w = L"d";
		rva[0].value.w = mir_wstrdup(g_chatApi.MakeTimeStamp(L"%#d", tTime));
		// day 01-31
		rva[1].key.w = L"dd";
		rva[1].value.w = mir_wstrdup(g_chatApi.MakeTimeStamp(L"%d", tTime));
		// month 1-12
		rva[2].key.w = L"m";
		rva[2].value.w = mir_wstrdup(g_chatApi.MakeTimeStamp(L"%#m", tTime));
		// month 01-12
		rva[3].key.w = L"mm";
		rva[3].value.w = mir_wstrdup(g_chatApi.MakeTimeStamp(L"%m", tTime));
		// month text short
		rva[4].key.w = L"mon";
		rva[4].value.w = mir_wstrdup(g_chatApi.MakeTimeStamp(L"%b", tTime));
		// month text
		rva[5].key.w = L"month";
		rva[5].value.w = mir_wstrdup(g_chatApi.MakeTimeStamp(L"%B", tTime));
		// year 01-99
		rva[6].key.w = L"yy";
		rva[6].value.w = mir_wstrdup(g_chatApi.MakeTimeStamp(L"%y", tTime));
		// year 1901-9999
		rva[7].key.w = L"yyyy";
		rva[7].value.w = mir_wstrdup(g_chatApi.MakeTimeStamp(L"%Y", tTime));
		// weekday short
		rva[8].key.w = L"wday";
		rva[8].value.w = mir_wstrdup(g_chatApi.MakeTimeStamp(L"%a", tTime));
		// weekday
		rva[9].key.w = L"weekday";
		rva[9].value.w = mir_wstrdup(g_chatApi.MakeTimeStamp(L"%A", tTime));
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
		if (g_chatApi.OnGetLogName)
			g_chatApi.OnGetLogName(si, tszParsedName);
		else
			PathToAbsoluteW(tszParsedName, si->pszLogFileName);
		mir_free(tszParsedName);

		for (auto &it : rva)
			mir_free(it.value.w);

		for (wchar_t *p = si->pszLogFileName + 2; *p; ++p)
			if (*p == ':' || *p == '*' || *p == '?' || *p == '"' || *p == '<' || *p == '>' || *p == '|')
				*p = '_';
	}

	return si->pszLogFileName;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(wchar_t*) Chat_GetGroup()
{
	return db_get_wsa(0, CHAT_MODULE, "AddToGroup", TranslateT("Chat rooms"));
}

MIR_APP_DLL(void) Chat_SetGroup(const wchar_t *pwszGroupName)
{
	if (mir_wstrlen(pwszGroupName))
		db_set_ws(0, CHAT_MODULE, "AddToGroup", pwszGroupName);
	else
		db_unset(0, CHAT_MODULE, "AddToGroup");
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

MIR_APP_DLL(void) Chat_AddMenuItems(HMENU hMenu, int nItems, const gc_item *Item, HPLUGIN pPlugin)
{
	if (nItems > 0)
		AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);

	HMENU hSubMenu = nullptr;
	for (int i = 0; i < nItems; i++) {
		wchar_t *ptszText = TranslateW_LP(Item[i].pszDesc, pPlugin);
		DWORD dwState = Item[i].bDisabled ? MF_GRAYED : 0;

		if (Item[i].uType == MENU_NEWPOPUP) {
			hSubMenu = CreateMenu();
			AppendMenu(hMenu, dwState | MF_POPUP, (UINT_PTR)hSubMenu, ptszText);
		}
		else if (Item[i].uType == MENU_POPUPHMENU)
			AppendMenu(hSubMenu == nullptr ? hMenu : hSubMenu, dwState | MF_POPUP, Item[i].dwID, ptszText);
		else if (Item[i].uType == MENU_POPUPITEM)
			AppendMenu(hSubMenu == nullptr ? hMenu : hSubMenu, dwState | MF_STRING, Item[i].dwID, ptszText);
		else if (Item[i].uType == MENU_POPUPCHECK)
			AppendMenu(hSubMenu == nullptr ? hMenu : hSubMenu, dwState | MF_CHECKED | MF_STRING, Item[i].dwID, ptszText);
		else if (Item[i].uType == MENU_POPUPSEPARATOR)
			AppendMenu(hSubMenu == nullptr ? hMenu : hSubMenu, MF_SEPARATOR, 0, ptszText);
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

	return TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
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
