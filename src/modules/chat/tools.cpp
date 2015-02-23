/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-15 Miranda NG project,
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

#include "..\..\core\commonheaders.h"

#include "chat.h"

int GetRichTextLength(HWND hwnd)
{
	GETTEXTLENGTHEX gtl;
	gtl.flags = GTL_PRECISE;
	gtl.codepage = CP_ACP;
	return (int)SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

static TCHAR szTemp[10000];

TCHAR* RemoveFormatting(const TCHAR *pszWord)
{
	if (pszWord == NULL)
		return NULL;

	TCHAR *d = szTemp;
	size_t cbLen = mir_tstrlen(pszWord);
	if (cbLen > SIZEOF(szTemp))
		cbLen = SIZEOF(szTemp)-1;

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
	switch (gce->pDest->iType) {
	case GC_EVENT_MESSAGE | GC_EVENT_HIGHLIGHT:
	case GC_EVENT_ACTION | GC_EVENT_HIGHLIGHT:
		ci.AddEvent(si->hContact, LoadSkinnedIcon(SKINICON_EVENT_MESSAGE), GC_FAKE_EVENT, 0, TranslateT("%s wants your attention in %s"), gce->ptszNick, si->ptszName);
		break;
	case GC_EVENT_MESSAGE:
		ci.AddEvent(si->hContact, ci.hIcons[ICON_MESSAGE], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s speaks in %s"), gce->ptszNick, si->ptszName);
		break;
	case GC_EVENT_ACTION:
		ci.AddEvent(si->hContact, ci.hIcons[ICON_ACTION], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s speaks in %s"), gce->ptszNick, si->ptszName);
		break;
	case GC_EVENT_JOIN:
		ci.AddEvent(si->hContact, ci.hIcons[ICON_JOIN], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s has joined %s"), gce->ptszNick, si->ptszName);
		break;
	case GC_EVENT_PART:
		ci.AddEvent(si->hContact, ci.hIcons[ICON_PART], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s has left %s"), gce->ptszNick, si->ptszName);
		break;
	case GC_EVENT_QUIT:
		ci.AddEvent(si->hContact, ci.hIcons[ICON_QUIT], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s has disconnected"), gce->ptszNick);
		break;
	case GC_EVENT_NICK:
		ci.AddEvent(si->hContact, ci.hIcons[ICON_NICK], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s is now known as %s"), gce->ptszNick, gce->ptszText);
		break;
	case GC_EVENT_KICK:
		ci.AddEvent(si->hContact, ci.hIcons[ICON_KICK], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s kicked %s from %s"), gce->ptszStatus, gce->ptszNick, si->ptszName);
		break;
	case GC_EVENT_NOTICE:
		ci.AddEvent(si->hContact, ci.hIcons[ICON_NOTICE], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("Notice from %s"), gce->ptszNick);
		break;
	case GC_EVENT_TOPIC:
		ci.AddEvent(si->hContact, ci.hIcons[ICON_TOPIC], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("Topic change in %s"), si->ptszName);
		break;
	case GC_EVENT_INFORMATION:
		ci.AddEvent(si->hContact, ci.hIcons[ICON_INFO], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("Information in %s"), si->ptszName);
		break;
	case GC_EVENT_ADDSTATUS:
		ci.AddEvent(si->hContact, ci.hIcons[ICON_ADDSTATUS], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s enables '%s' status for %s in %s"), gce->ptszText, gce->ptszStatus, gce->ptszNick, si->ptszName);
		break;
	case GC_EVENT_REMOVESTATUS:
		ci.AddEvent(si->hContact, ci.hIcons[ICON_REMSTATUS], GC_FAKE_EVENT, CLEF_ONLYAFEW, TranslateT("%s disables '%s' status for %s in %s"), gce->ptszText, gce->ptszStatus, gce->ptszNick, si->ptszName);
		break;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void __stdcall ShowRoomFromPopup(void *pi)
{
	SESSION_INFO *si = (SESSION_INFO*)pi;
	ci.ShowRoom(si, WINDOW_VISIBLE, TRUE);
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
			if (CallService(MS_CLIST_GETEVENT, (WPARAM)si->hContact, 0))
				CallService(MS_CLIST_REMOVEEVENT, (WPARAM)si->hContact, (LPARAM)GC_FAKE_EVENT);

		if (si->hWnd && KillTimer(si->hWnd, TIMERID_FLASHWND))
			FlashWindow(si->hWnd, FALSE);

		PUDeletePopup(hWnd);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

int ShowPopup(MCONTACT hContact, SESSION_INFO *si, HICON hIcon, char* pszProtoName, TCHAR* pszRoomName, COLORREF crBkg, const TCHAR* fmt, ...)
{
	static TCHAR szBuf[4 * 1024];

	if (!fmt || fmt[0] == 0 || mir_tstrlen(fmt) > 2000)
		return 0;

	va_list marker;
	va_start(marker, fmt);
	mir_vsntprintf(szBuf, 4096, fmt, marker);
	va_end(marker);

	POPUPDATAT pd = { 0 };
	pd.lchContact = hContact;

	if (hIcon)
		pd.lchIcon = hIcon;
	else
		pd.lchIcon = LoadIconEx("window", FALSE);

	PROTOACCOUNT *pa = ProtoGetAccount(pszProtoName);
	mir_sntprintf(pd.lptzContactName, SIZEOF(pd.lptzContactName), _T("%s - %s"), 
		(pa == NULL) ? _A2T(pszProtoName) : pa->tszAccountName,
		cli.pfnGetContactDisplayName(hContact, 0));

	mir_tstrncpy(pd.lptzText, TranslateTS(szBuf), SIZEOF(pd.lptzText));
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
	switch (gce->pDest->iType) {
	case GC_EVENT_MESSAGE | GC_EVENT_HIGHLIGHT:
		ci.ShowPopup(si->hContact, si, LoadSkinnedIcon(SKINICON_EVENT_MESSAGE), si->pszModule, si->ptszName, ci.aFonts[16].color, TranslateT("%s says: %s"), gce->ptszNick, RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_ACTION | GC_EVENT_HIGHLIGHT:
		ci.ShowPopup(si->hContact, si, LoadSkinnedIcon(SKINICON_EVENT_MESSAGE), si->pszModule, si->ptszName, ci.aFonts[16].color, _T("%s %s"), gce->ptszNick, RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_MESSAGE:
		ci.ShowPopup(si->hContact, si, ci.hIcons[ICON_MESSAGE], si->pszModule, si->ptszName, ci.aFonts[9].color, TranslateT("%s says: %s"), gce->ptszNick, RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_ACTION:
		ci.ShowPopup(si->hContact, si, ci.hIcons[ICON_ACTION], si->pszModule, si->ptszName, ci.aFonts[15].color, _T("%s %s"), gce->ptszNick, RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_JOIN:
		ci.ShowPopup(si->hContact, si, ci.hIcons[ICON_JOIN], si->pszModule, si->ptszName, ci.aFonts[3].color, TranslateT("%s has joined"), gce->ptszNick);
		break;
	case GC_EVENT_PART:
		if (!gce->ptszText)
			ci.ShowPopup(si->hContact, si, ci.hIcons[ICON_PART], si->pszModule, si->ptszName, ci.aFonts[4].color, TranslateT("%s has left"), gce->ptszNick);
		else
			ci.ShowPopup(si->hContact, si, ci.hIcons[ICON_PART], si->pszModule, si->ptszName, ci.aFonts[4].color, TranslateT("%s has left (%s)"), gce->ptszNick, RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_QUIT:
		if (!gce->ptszText)
			ci.ShowPopup(si->hContact, si, ci.hIcons[ICON_QUIT], si->pszModule, si->ptszName, ci.aFonts[5].color, TranslateT("%s has disconnected"), gce->ptszNick);
		else
			ci.ShowPopup(si->hContact, si, ci.hIcons[ICON_QUIT], si->pszModule, si->ptszName, ci.aFonts[5].color, TranslateT("%s has disconnected (%s)"), gce->ptszNick, RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_NICK:
		ci.ShowPopup(si->hContact, si, ci.hIcons[ICON_NICK], si->pszModule, si->ptszName, ci.aFonts[7].color, TranslateT("%s is now known as %s"), gce->ptszNick, gce->ptszText);
		break;
	case GC_EVENT_KICK:
		if (!gce->ptszText)
			ci.ShowPopup(si->hContact, si, ci.hIcons[ICON_KICK], si->pszModule, si->ptszName, ci.aFonts[6].color, TranslateT("%s kicked %s"), (char *)gce->ptszStatus, gce->ptszNick);
		else
			ci.ShowPopup(si->hContact, si, ci.hIcons[ICON_KICK], si->pszModule, si->ptszName, ci.aFonts[6].color, TranslateT("%s kicked %s (%s)"), (char *)gce->ptszStatus, gce->ptszNick, RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_NOTICE:
		ci.ShowPopup(si->hContact, si, ci.hIcons[ICON_NOTICE], si->pszModule, si->ptszName, ci.aFonts[8].color, TranslateT("Notice from %s: %s"), gce->ptszNick, RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_TOPIC:
		if (!gce->ptszNick)
			ci.ShowPopup(si->hContact, si, ci.hIcons[ICON_TOPIC], si->pszModule, si->ptszName, ci.aFonts[11].color, TranslateT("The topic is '%s'"), RemoveFormatting(gce->ptszText));
		else
			ci.ShowPopup(si->hContact, si, ci.hIcons[ICON_TOPIC], si->pszModule, si->ptszName, ci.aFonts[11].color, TranslateT("The topic is '%s' (set by %s)"), RemoveFormatting(gce->ptszText), gce->ptszNick);
		break;
	case GC_EVENT_INFORMATION:
		ci.ShowPopup(si->hContact, si, ci.hIcons[ICON_INFO], si->pszModule, si->ptszName, ci.aFonts[12].color, _T("%s"), RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_ADDSTATUS:
		ci.ShowPopup(si->hContact, si, ci.hIcons[ICON_ADDSTATUS], si->pszModule, si->ptszName, ci.aFonts[13].color, TranslateT("%s enables '%s' status for %s"), gce->ptszText, (char *)gce->ptszStatus, gce->ptszNick);
		break;
	case GC_EVENT_REMOVESTATUS:
		ci.ShowPopup(si->hContact, si, ci.hIcons[ICON_REMSTATUS], si->pszModule, si->ptszName, ci.aFonts[14].color, TranslateT("%s disables '%s' status for %s"), gce->ptszText, (char *)gce->ptszStatus, gce->ptszNick);
		break;
	}

	return TRUE;
}

BOOL DoSoundsFlashPopupTrayStuff(SESSION_INFO *si, GCEVENT *gce, BOOL bHighlight, int bManyFix)
{
	if (!gce || !si || gce->bIsMe || si->iType == GCW_SERVER)
		return FALSE;

	BOOL bInactive = si->hWnd == NULL || GetForegroundWindow() != si->hWnd;

	int iEvent = gce->pDest->iType;

	if (bHighlight) {
		gce->pDest->iType |= GC_EVENT_HIGHLIGHT;
		if (bInactive || !g_Settings->bSoundsFocus)
			SkinPlaySound("ChatHighlight");
		if (db_get_b(si->hContact, "CList", "Hidden", 0) != 0)
			db_unset(si->hContact, "CList", "Hidden");
		if (bInactive)
			ci.DoTrayIcon(si, gce);
		if (bInactive || !g_Settings->bPopupInactiveOnly)
			ci.DoPopup(si, gce);
		if (ci.OnFlashHighlight)
			ci.OnFlashHighlight(si, bInactive);
		return TRUE;
	}

	// do blinking icons in tray
	if (bInactive || !g_Settings->bTrayIconInactiveOnly)
		ci.DoTrayIcon(si, gce);

	// stupid thing to not create multiple popups for a QUIT event for instance
	if (bManyFix == 0) {
		// do popups
		if (bInactive || !g_Settings->bPopupInactiveOnly)
			ci.DoPopup(si, gce);

		// do sounds and flashing
		switch (iEvent) {
		case GC_EVENT_JOIN:
			if (bInactive || !g_Settings->bSoundsFocus)
				SkinPlaySound("ChatJoin");
			break;
		case GC_EVENT_PART:
			if (bInactive || !g_Settings->bSoundsFocus)
				SkinPlaySound("ChatPart");
			break;
		case GC_EVENT_QUIT:
			if (bInactive || !g_Settings->bSoundsFocus)
				SkinPlaySound("ChatQuit");
			break;
		case GC_EVENT_ADDSTATUS:
		case GC_EVENT_REMOVESTATUS:
			if (bInactive || !g_Settings->bSoundsFocus)
				SkinPlaySound("ChatMode");
			break;
		case GC_EVENT_KICK:
			if (bInactive || !g_Settings->bSoundsFocus)
				SkinPlaySound("ChatKick");
			break;
		case GC_EVENT_MESSAGE:
			if (bInactive || !g_Settings->bSoundsFocus)
				SkinPlaySound("ChatMessage");

			if (bInactive && !(si->wState & STATE_TALK)) {
				si->wState |= STATE_TALK;
				db_set_w(si->hContact, si->pszModule, "ApparentMode", (LPARAM)(WORD)40071);
			}
			if (ci.OnFlashWindow)
				ci.OnFlashWindow(si, bInactive);
			break;
		case GC_EVENT_ACTION:
			if (bInactive || !g_Settings->bSoundsFocus)
				SkinPlaySound("ChatAction");
			break;
		case GC_EVENT_NICK:
			if (bInactive || !g_Settings->bSoundsFocus)
				SkinPlaySound("ChatNick");
			break;
		case GC_EVENT_NOTICE:
			if (bInactive || !g_Settings->bSoundsFocus)
				SkinPlaySound("ChatNotice");
			break;
		case GC_EVENT_TOPIC:
			if (bInactive || !g_Settings->bSoundsFocus)
				SkinPlaySound("ChatTopic");
			break;
		}
	}

	return TRUE;
}

int GetColorIndex(const char *pszModule, COLORREF cr)
{
	MODULEINFO *pMod = ci.MM_FindModule(pszModule);
	int i = 0;

	if (!pMod || pMod->nColorCount == 0)
		return -1;

	for (i = 0; i < pMod->nColorCount; i++)
	if (pMod->crColors[i] == cr)
		return i;

	return -1;
}

// obscure function that is used to make sure that any of the colors
// passed by the protocol is used as fore- or background color
// in the messagebox. THis is to vvercome limitations in the richedit
// that I do not know currently how to fix

void CheckColorsInModule(const char *pszModule)
{
	MODULEINFO *pMod = ci.MM_FindModule(pszModule);
	int i = 0;
	COLORREF crFG;
	COLORREF crBG = (COLORREF)db_get_dw(NULL, CHAT_MODULE, "ColorMessageBG", GetSysColor(COLOR_WINDOW));

	LoadMsgDlgFont(17, NULL, &crFG);

	if (!pMod)
		return;

	for (i = 0; i < pMod->nColorCount; i++) {
		if (pMod->crColors[i] == crFG || pMod->crColors[i] == crBG) {
			if (pMod->crColors[i] == RGB(255, 255, 255))
				pMod->crColors[i]--;
			else
				pMod->crColors[i]++;
		}
	}
}

const TCHAR* my_strstri(const TCHAR* s1, const TCHAR* s2)
{
	int i, j, k;
	for (i = 0; s1[i]; i++)
	for (j = i, k = 0; _totlower(s1[j]) == _totlower(s2[k]); j++, k++)
	if (!s2[k + 1])
		return s1 + i;

	return NULL;
}

static TCHAR szTrimString[] = _T(":,.!?;\'>)");

BOOL IsHighlighted(SESSION_INFO *si, GCEVENT *gce)
{
	if (!g_Settings->bHighlightEnabled || !g_Settings->pszHighlightWords || !gce || !si || !si->pMe)
		return FALSE;

	if (gce->ptszText == NULL)
		return FALSE;

	TCHAR *buf = RemoveFormatting(NEWTSTR_ALLOCA(gce->ptszText));

	int iStart = 0;
	CMString tszHighlightWords(g_Settings->pszHighlightWords);

	while (true) {
		CMString tszToken = tszHighlightWords.Tokenize(_T("\t "), iStart);
		if (iStart == -1)
			break;

		// replace %m with the users nickname
		if (tszToken == _T("%m"))
			tszToken = si->pMe->pszNick;

		if (tszToken.Find('*') == -1)
			tszToken = '*' + tszToken + '*';

		// time to get the next/first word in the incoming text string
		for (const TCHAR *p = buf; *p != '\0'; p += _tcscspn(p, _T(" "))) {
			p += _tcsspn(p, _T(" "));

			// compare the words, using wildcards
			if (wildcmpit(RemoveFormatting(p), tszToken))
				return TRUE;
		}
	}

	return FALSE;
}

BOOL LogToFile(SESSION_INFO *si, GCEVENT *gce)
{
	TCHAR szBuffer[4096];
	TCHAR szLine[4096];
	TCHAR p = '\0';
	szBuffer[0] = '\0';

	GetChatLogsFilename(si, gce->time);
	BOOL bFileJustCreated = !PathFileExists(si->pszLogFileName);

	TCHAR tszFolder[MAX_PATH];
	_tcscpy(tszFolder, si->pszLogFileName);
	PathRemoveFileSpec(tszFolder);
	if (!PathIsDirectory(tszFolder))
		CreateDirectoryTreeT(tszFolder);

	TCHAR szTime[100];
	mir_tstrncpy(szTime, ci.MakeTimeStamp(g_Settings->pszTimeStampLog, gce->time), 99);

	FILE *hFile = _tfopen(si->pszLogFileName, _T("ab+"));
	if (hFile == NULL)
		return FALSE;

	TCHAR szTemp[512], szTemp2[512];
	TCHAR* pszNick = NULL;
	if (bFileJustCreated)
		fputws((const wchar_t*)"\377\376", hFile);		//UTF-16 LE BOM == FF FE
	if (gce->ptszNick) {
		if (g_Settings->bLogLimitNames && mir_tstrlen(gce->ptszNick) > 20) {
			mir_tstrncpy(szTemp2, gce->ptszNick, 20);
			mir_tstrncpy(szTemp2 + 20, _T("..."), 4);
		}
		else mir_tstrncpy(szTemp2, gce->ptszNick, 511);

		if (gce->ptszUserInfo)
			mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%s (%s)"), szTemp2, gce->ptszUserInfo);
		else
			_tcsncpy_s(szTemp, szTemp2, _TRUNCATE);
		pszNick = szTemp;
	}

	switch (gce->pDest->iType) {
	case GC_EVENT_MESSAGE:
	case GC_EVENT_MESSAGE | GC_EVENT_HIGHLIGHT:
		p = '*';
		mir_sntprintf(szBuffer, SIZEOF(szBuffer), _T("%s: %s"), gce->ptszNick, ci.RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_ACTION:
	case GC_EVENT_ACTION | GC_EVENT_HIGHLIGHT:
		p = '*';
		mir_sntprintf(szBuffer, SIZEOF(szBuffer), _T("%s %s"), gce->ptszNick, ci.RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_JOIN:
		p = '>';
		mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s has joined"), pszNick);
		break;
	case GC_EVENT_PART:
		p = '<';
		if (!gce->ptszText)
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s has left"), pszNick);
		else
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s has left (%s)"), pszNick, ci.RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_QUIT:
		p = '<';
		if (!gce->ptszText)
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s has disconnected"), pszNick);
		else
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s has disconnected (%s)"), pszNick, ci.RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_NICK:
		p = '^';
		mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s is now known as %s"), gce->ptszNick, gce->ptszText);
		break;
	case GC_EVENT_KICK:
		p = '~';
		if (!gce->ptszText)
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s kicked %s"), gce->ptszStatus, gce->ptszNick);
		else
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s kicked %s (%s)"), gce->ptszStatus, gce->ptszNick, ci.RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_NOTICE:
		p = 'o';
		mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("Notice from %s: %s"), gce->ptszNick, ci.RemoveFormatting(gce->ptszText));
		break;
	case GC_EVENT_TOPIC:
		p = '#';
		if (!gce->ptszNick)
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("The topic is '%s'"), ci.RemoveFormatting(gce->ptszText));
		else
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("The topic is '%s' (set by %s)"), ci.RemoveFormatting(gce->ptszText), gce->ptszNick);
		break;
	case GC_EVENT_INFORMATION:
		p = '!';
		_tcsncpy_s(szBuffer, ci.RemoveFormatting(gce->ptszText), _TRUNCATE);
		break;
	case GC_EVENT_ADDSTATUS:
		p = '+';
		mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s enables '%s' status for %s"), gce->ptszText, gce->ptszStatus, gce->ptszNick);
		break;
	case GC_EVENT_REMOVESTATUS:
		p = '-';
		mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s disables '%s' status for %s"), gce->ptszText, gce->ptszStatus, gce->ptszNick);
		break;
	}

	// formatting strings don't need to be translatable - changing them via language pack would
	// only screw up the log format.
	if (p)
		mir_sntprintf(szLine, SIZEOF(szLine), _T("%s %c %s\r\n"), szTime, p, szBuffer);
	else
		mir_sntprintf(szLine, SIZEOF(szLine), _T("%s %s\r\n"), szTime, szBuffer);

	if (szLine[0]) {
		_fputts(szLine, hFile);

		if (g_Settings->LoggingLimit > 0) {
			fseek(hFile, 0, SEEK_END);
			long dwSize = ftell(hFile);
			rewind(hFile);

			long trimlimit = g_Settings->LoggingLimit * 1024;
			if (dwSize > trimlimit) {
				time_t now = time(0);

				TCHAR tszTimestamp[20];
				_tcsftime(tszTimestamp, 20, _T("%Y%m%d-%H%M%S"), _localtime32((__time32_t *)&now));
				tszTimestamp[19] = 0;

				// max size reached, rotate the log
				// move old logs to /archived sub folder just inside the log root folder.
				// add a time stamp to the file name.
				TCHAR tszDrive[_MAX_DRIVE], tszDir[_MAX_DIR], tszName[_MAX_FNAME], tszExt[_MAX_EXT];
				_tsplitpath(si->pszLogFileName, tszDrive, tszDir, tszName, tszExt);

				TCHAR tszNewPath[_MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT + 20];
				mir_sntprintf(tszNewPath, SIZEOF(tszNewPath), _T("%s%sarchived\\"), tszDrive, tszDir);
				CreateDirectoryTreeT(tszNewPath);

				TCHAR tszNewName[_MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT + 20];
				mir_sntprintf(tszNewName, SIZEOF(tszNewName), _T("%s%s-%s%s"), tszNewPath, tszName, tszTimestamp, tszExt);
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

BOOL DoEventHookAsync(HWND hwnd, const TCHAR *pszID, const char *pszModule, int iType, const TCHAR* pszUID, const TCHAR* pszText, INT_PTR dwItem)
{
	SESSION_INFO *si = ci.SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return FALSE;

	GCDEST *gcd = (GCDEST*)mir_calloc(sizeof(GCDEST));
	gcd->pszModule = mir_strdup(pszModule);
	gcd->ptszID = mir_tstrdup(pszID);
	gcd->iType = iType;

	GCHOOK *gch = (GCHOOK*)mir_calloc(sizeof(GCHOOK));
	gch->ptszUID = mir_tstrdup(pszUID);
	gch->ptszText = mir_tstrdup(pszText);
	gch->dwData = dwItem;
	gch->pDest = gcd;
	PostMessage(hwnd, GC_FIREHOOK, 0, (LPARAM)gch);
	return TRUE;
}

BOOL DoEventHook(const TCHAR *pszID, const char *pszModule, int iType, const TCHAR *pszUID, const TCHAR* pszText, INT_PTR dwItem)
{
	SESSION_INFO *si = ci.SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return FALSE;

	GCDEST gcd = { (char*)pszModule, pszID, iType };
	GCHOOK gch = { 0 };
	gch.ptszUID = (LPTSTR)pszUID;
	gch.ptszText = (LPTSTR)pszText;
	gch.dwData = dwItem;
	gch.pDest = &gcd;
	NotifyEventHooks(ci.hSendEvent, 0, (WPARAM)&gch);
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
	case GC_EVENT_CHUID:
	case GC_EVENT_CHANGESESSIONAME:
	case GC_EVENT_ADDGROUP:
	case GC_EVENT_SETITEMDATA:
	case GC_EVENT_GETITEMDATA:
	case GC_EVENT_SETSBTEXT:
	case GC_EVENT_ACK:
	case GC_EVENT_SENDMESSAGE:
	case GC_EVENT_SETSTATUSEX:
	case GC_EVENT_CONTROL:
	case GC_EVENT_SETCONTACTSTATUS:
		return TRUE;
	}

	// Other events
	return FALSE;
}

void ValidateFilename(TCHAR *filename)
{
	TCHAR *p1 = filename;
	TCHAR szForbidden[] = _T("\\/:*?\"<>|");
	while (*p1 != '\0') {
		if (_tcschr(szForbidden, *p1))
			*p1 = '_';
		p1 += 1;
	}
}

static TCHAR tszOldTimeStamp[30];

TCHAR* GetChatLogsFilename(SESSION_INFO *si, time_t tTime)
{
	if (!tTime)
		time(&tTime);

	// check whether relevant parts of the timestamp have changed and
	// we have to reparse the filename
	TCHAR *tszNow = ci.MakeTimeStamp(_T("%a%d%m%Y"), tTime); // once a day
	if (_tcscmp(tszOldTimeStamp, tszNow)) {
		_tcsncpy_s(tszOldTimeStamp, tszNow, _TRUNCATE);
		*si->pszLogFileName = 0;
	}

	if (si->pszLogFileName[0] == 0) {
		REPLACEVARSARRAY rva[11];
		rva[0].lptzKey = _T("d");
		rva[0].lptzValue = mir_tstrdup(ci.MakeTimeStamp(_T("%#d"), tTime));
		// day 01-31
		rva[1].lptzKey = _T("dd");
		rva[1].lptzValue = mir_tstrdup(ci.MakeTimeStamp(_T("%d"), tTime));
		// month 1-12
		rva[2].lptzKey = _T("m");
		rva[2].lptzValue = mir_tstrdup(ci.MakeTimeStamp(_T("%#m"), tTime));
		// month 01-12
		rva[3].lptzKey = _T("mm");
		rva[3].lptzValue = mir_tstrdup(ci.MakeTimeStamp(_T("%m"), tTime));
		// month text short
		rva[4].lptzKey = _T("mon");
		rva[4].lptzValue = mir_tstrdup(ci.MakeTimeStamp(_T("%b"), tTime));
		// month text
		rva[5].lptzKey = _T("month");
		rva[5].lptzValue = mir_tstrdup(ci.MakeTimeStamp(_T("%B"), tTime));
		// year 01-99
		rva[6].lptzKey = _T("yy");
		rva[6].lptzValue = mir_tstrdup(ci.MakeTimeStamp(_T("%y"), tTime));
		// year 1901-9999
		rva[7].lptzKey = _T("yyyy");
		rva[7].lptzValue = mir_tstrdup(ci.MakeTimeStamp(_T("%Y"), tTime));
		// weekday short
		rva[8].lptzKey = _T("wday");
		rva[8].lptzValue = mir_tstrdup(ci.MakeTimeStamp(_T("%a"), tTime));
		// weekday
		rva[9].lptzKey = _T("weekday");
		rva[9].lptzValue = mir_tstrdup(ci.MakeTimeStamp(_T("%A"), tTime));
		// end of array
		rva[10].lptzKey = NULL;
		rva[10].lptzValue = NULL;

		TCHAR tszTemp[MAX_PATH], *ptszVarPath;
		if (g_Settings->pszLogDir[mir_tstrlen(g_Settings->pszLogDir) - 1] == '\\') {
			mir_sntprintf(tszTemp, SIZEOF(tszTemp), _T("%s%s"), g_Settings->pszLogDir, _T("%userid%.log"));
			ptszVarPath = tszTemp;
		}
		else ptszVarPath = g_Settings->pszLogDir;

		REPLACEVARSDATA dat = { sizeof(dat) };
		dat.dwFlags = RVF_TCHAR;
		dat.hContact = si->hContact;
		dat.variables = rva;
		TCHAR *tszParsedName = (TCHAR*)CallService(MS_UTILS_REPLACEVARS, (WPARAM)ptszVarPath, (LPARAM)&dat);
		if (ci.OnGetLogName)
			ci.OnGetLogName(si, tszParsedName);
		else
			PathToAbsoluteT(tszParsedName, si->pszLogFileName);
		mir_free(tszParsedName);

		for (int i = 0; i < SIZEOF(rva); i++)
			mir_free(rva[i].lptzValue);

		for (TCHAR *p = si->pszLogFileName + 2; *p; ++p)
			if (*p == ':' || *p == '*' || *p == '?' || *p == '"' || *p == '<' || *p == '>' || *p == '|')
				*p = _T('_');
	}

	return si->pszLogFileName;
}
