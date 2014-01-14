/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-14 Miranda NG project,
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

TCHAR* RemoveFormatting(const TCHAR* pszWord)
{
	static TCHAR szTemp[10000];
	int i = 0;
	int j = 0;

	if (pszWord == 0 || lstrlen(pszWord) == 0)
		return NULL;

	while (j < 9999 && i <= lstrlen(pszWord)) {
		if (pszWord[i] == '%') {
			switch (pszWord[i + 1]) {
			case '%':
				szTemp[j] = '%';
				j++;
				i++; i++;
				break;
			case 'b':
			case 'u':
			case 'i':
			case 'B':
			case 'U':
			case 'I':
			case 'r':
			case 'C':
			case 'F':
				i++; i++;
				break;

			case 'c':
			case 'f':
				i += 4;
				break;

			default:
				szTemp[j] = pszWord[i];
				j++;
				i++;
				break;
			}
		}
		else {
			szTemp[j] = pszWord[i];
			j++;
			i++;
		}
	}

	return (TCHAR*)&szTemp;
}

static void __stdcall ShowRoomFromPopup(void * pi)
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
			CallService(MS_CLIST_REMOVEEVENT, (WPARAM)si->hContact, (LPARAM)"chaticon");

		if (si->hWnd && KillTimer(si->hWnd, TIMERID_FLASHWND))
			FlashWindow(si->hWnd, FALSE);

		PUDeletePopup(hWnd);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

static int ShowPopup(HANDLE hContact, SESSION_INFO *si, HICON hIcon, char* pszProtoName, TCHAR* pszRoomName, COLORREF crBkg, const TCHAR* fmt, ...)
{
	static TCHAR szBuf[4 * 1024];

	if (!fmt || lstrlen(fmt) == 0 || lstrlen(fmt) > 2000)
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
	mir_sntprintf(pd.lptzContactName, MAX_CONTACTNAME - 1, _T("%s - %s"),
		(pa == NULL) ? _A2T(pszProtoName) : pa->tszAccountName,
		cli.pfnGetContactDisplayName(hContact, 0));

	lstrcpyn(pd.lptzText, TranslateTS(szBuf), MAX_SECONDLINE);
	pd.iSeconds = ci.pSettings->iPopupTimeout;

	if (ci.pSettings->iPopupStyle == 2) {
		pd.colorBack = 0;
		pd.colorText = 0;
	}
	else if (ci.pSettings->iPopupStyle == 3) {
		pd.colorBack = ci.pSettings->crPUBkgColour;
		pd.colorText = ci.pSettings->crPUTextColour;
	}
	else {
		pd.colorBack = ci.pSettings->crLogBackground;
		pd.colorText = crBkg;
	}

	pd.PluginWindowProc = PopupDlgProc;
	pd.PluginData = si;
	return PUAddPopupT(&pd);
}

static BOOL DoTrayIcon(SESSION_INFO *si, GCEVENT *gce)
{
	int iEvent = gce->pDest->iType;

	if (iEvent&ci.pSettings->dwTrayIconFlags) {
		switch (iEvent) {
		case GC_EVENT_MESSAGE | GC_EVENT_HIGHLIGHT:
		case GC_EVENT_ACTION | GC_EVENT_HIGHLIGHT:
			ci.AddEvent(si->hContact, LoadSkinnedIcon(SKINICON_EVENT_MESSAGE), "chaticon", 0, TranslateT("%s wants your attention in %s"), gce->ptszNick, si->ptszName);
			break;
		case GC_EVENT_MESSAGE:
			ci.AddEvent(si->hContact, ci.hIcons[ICON_MESSAGE], "chaticon", CLEF_ONLYAFEW, TranslateT("%s speaks in %s"), gce->ptszNick, si->ptszName);
			break;
		case GC_EVENT_ACTION:
			ci.AddEvent(si->hContact, ci.hIcons[ICON_ACTION], "chaticon", CLEF_ONLYAFEW, TranslateT("%s speaks in %s"), gce->ptszNick, si->ptszName);
			break;
		case GC_EVENT_JOIN:
			ci.AddEvent(si->hContact, ci.hIcons[ICON_JOIN], "chaticon", CLEF_ONLYAFEW, TranslateT("%s has joined %s"), gce->ptszNick, si->ptszName);
			break;
		case GC_EVENT_PART:
			ci.AddEvent(si->hContact, ci.hIcons[ICON_PART], "chaticon", CLEF_ONLYAFEW, TranslateT("%s has left %s"), gce->ptszNick, si->ptszName);
			break;
		case GC_EVENT_QUIT:
			ci.AddEvent(si->hContact, ci.hIcons[ICON_QUIT], "chaticon", CLEF_ONLYAFEW, TranslateT("%s has disconnected"), gce->ptszNick);
			break;
		case GC_EVENT_NICK:
			ci.AddEvent(si->hContact, ci.hIcons[ICON_NICK], "chaticon", CLEF_ONLYAFEW, TranslateT("%s is now known as %s"), gce->ptszNick, gce->ptszText);
			break;
		case GC_EVENT_KICK:
			ci.AddEvent(si->hContact, ci.hIcons[ICON_KICK], "chaticon", CLEF_ONLYAFEW, TranslateT("%s kicked %s from %s"), gce->ptszStatus, gce->ptszNick, si->ptszName);
			break;
		case GC_EVENT_NOTICE:
			ci.AddEvent(si->hContact, ci.hIcons[ICON_NOTICE], "chaticon", CLEF_ONLYAFEW, TranslateT("Notice from %s"), gce->ptszNick);
			break;
		case GC_EVENT_TOPIC:
			ci.AddEvent(si->hContact, ci.hIcons[ICON_TOPIC], "chaticon", CLEF_ONLYAFEW, TranslateT("Topic change in %s"), si->ptszName);
			break;
		case GC_EVENT_INFORMATION:
			ci.AddEvent(si->hContact, ci.hIcons[ICON_INFO], "chaticon", CLEF_ONLYAFEW, TranslateT("Information in %s"), si->ptszName);
			break;
		case GC_EVENT_ADDSTATUS:
			ci.AddEvent(si->hContact, ci.hIcons[ICON_ADDSTATUS], "chaticon", CLEF_ONLYAFEW, TranslateT("%s enables \'%s\' status for %s in %s"), gce->ptszText, gce->ptszStatus, gce->ptszNick, si->ptszName);
			break;
		case GC_EVENT_REMOVESTATUS:
			ci.AddEvent(si->hContact, ci.hIcons[ICON_REMSTATUS], "chaticon", CLEF_ONLYAFEW, TranslateT("%s disables \'%s\' status for %s in %s"), gce->ptszText, gce->ptszStatus, gce->ptszNick, si->ptszName);
			break;
		}
	}

	return TRUE;
}

static BOOL DoPopup(SESSION_INFO *si, GCEVENT *gce)
{
	int iEvent = gce->pDest->iType;

	if (iEvent & ci.pSettings->dwPopupFlags) {
		switch (iEvent) {
		case GC_EVENT_MESSAGE | GC_EVENT_HIGHLIGHT:
			ShowPopup(si->hContact, si, LoadSkinnedIcon(SKINICON_EVENT_MESSAGE), si->pszModule, si->ptszName, ci.aFonts[16].color, TranslateT("%s says: %s"), gce->ptszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_ACTION | GC_EVENT_HIGHLIGHT:
			ShowPopup(si->hContact, si, LoadSkinnedIcon(SKINICON_EVENT_MESSAGE), si->pszModule, si->ptszName, ci.aFonts[16].color, _T("%s %s"), gce->ptszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_MESSAGE:
			ShowPopup(si->hContact, si, ci.hIcons[ICON_MESSAGE], si->pszModule, si->ptszName, ci.aFonts[9].color, TranslateT("%s says: %s"), gce->ptszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_ACTION:
			ShowPopup(si->hContact, si, ci.hIcons[ICON_ACTION], si->pszModule, si->ptszName, ci.aFonts[15].color, _T("%s %s"), gce->ptszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_JOIN:
			ShowPopup(si->hContact, si, ci.hIcons[ICON_JOIN], si->pszModule, si->ptszName, ci.aFonts[3].color, TranslateT("%s has joined"), gce->ptszNick);
			break;
		case GC_EVENT_PART:
			if (!gce->ptszText)
				ShowPopup(si->hContact, si, ci.hIcons[ICON_PART], si->pszModule, si->ptszName, ci.aFonts[4].color, TranslateT("%s has left"), gce->ptszNick);
			else
				ShowPopup(si->hContact, si, ci.hIcons[ICON_PART], si->pszModule, si->ptszName, ci.aFonts[4].color, TranslateT("%s has left (%s)"), gce->ptszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_QUIT:
			if (!gce->ptszText)
				ShowPopup(si->hContact, si, ci.hIcons[ICON_QUIT], si->pszModule, si->ptszName, ci.aFonts[5].color, TranslateT("%s has disconnected"), gce->ptszNick);
			else
				ShowPopup(si->hContact, si, ci.hIcons[ICON_QUIT], si->pszModule, si->ptszName, ci.aFonts[5].color, TranslateT("%s has disconnected (%s)"), gce->ptszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_NICK:
			ShowPopup(si->hContact, si, ci.hIcons[ICON_NICK], si->pszModule, si->ptszName, ci.aFonts[7].color, TranslateT("%s is now known as %s"), gce->ptszNick, gce->ptszText);
			break;
		case GC_EVENT_KICK:
			if (!gce->ptszText)
				ShowPopup(si->hContact, si, ci.hIcons[ICON_KICK], si->pszModule, si->ptszName, ci.aFonts[6].color, TranslateT("%s kicked %s"), (char *)gce->ptszStatus, gce->ptszNick);
			else
				ShowPopup(si->hContact, si, ci.hIcons[ICON_KICK], si->pszModule, si->ptszName, ci.aFonts[6].color, TranslateT("%s kicked %s (%s)"), (char *)gce->ptszStatus, gce->ptszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_NOTICE:
			ShowPopup(si->hContact, si, ci.hIcons[ICON_NOTICE], si->pszModule, si->ptszName, ci.aFonts[8].color, TranslateT("Notice from %s: %s"), gce->ptszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_TOPIC:
			if (!gce->ptszNick)
				ShowPopup(si->hContact, si, ci.hIcons[ICON_TOPIC], si->pszModule, si->ptszName, ci.aFonts[11].color, TranslateT("The topic is \'%s\'"), RemoveFormatting(gce->ptszText));
			else
				ShowPopup(si->hContact, si, ci.hIcons[ICON_TOPIC], si->pszModule, si->ptszName, ci.aFonts[11].color, TranslateT("The topic is \'%s\' (set by %s)"), RemoveFormatting(gce->ptszText), gce->ptszNick);
			break;
		case GC_EVENT_INFORMATION:
			ShowPopup(si->hContact, si, ci.hIcons[ICON_INFO], si->pszModule, si->ptszName, ci.aFonts[12].color, _T("%s"), RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_ADDSTATUS:
			ShowPopup(si->hContact, si, ci.hIcons[ICON_ADDSTATUS], si->pszModule, si->ptszName, ci.aFonts[13].color, TranslateT("%s enables \'%s\' status for %s"), gce->ptszText, (char *)gce->ptszStatus, gce->ptszNick);
			break;
		case GC_EVENT_REMOVESTATUS:
			ShowPopup(si->hContact, si, ci.hIcons[ICON_REMSTATUS], si->pszModule, si->ptszName, ci.aFonts[14].color, TranslateT("%s disables \'%s\' status for %s"), gce->ptszText, (char *)gce->ptszStatus, gce->ptszNick);
			break;
		}
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
		if (bInactive || !ci.pSettings->SoundsFocus)
			SkinPlaySound("ChatHighlight");
		if (db_get_b(si->hContact, "CList", "Hidden", 0) != 0)
			db_unset(si->hContact, "CList", "Hidden");
		if (bInactive)
			DoTrayIcon(si, gce);
		if (bInactive || !ci.pSettings->PopupInactiveOnly)
			DoPopup(si, gce);
		if (ci.OnFlashWindow)
			ci.OnFlashWindow(si, bInactive);
		return TRUE;
	}

	// do blinking icons in tray
	if (bInactive || !ci.pSettings->TrayIconInactiveOnly)
		DoTrayIcon(si, gce);

	// stupid thing to not create multiple popups for a QUIT event for instance
	if (bManyFix == 0) {
		// do popups
		if (bInactive || !ci.pSettings->PopupInactiveOnly)
			DoPopup(si, gce);

		// do sounds and flashing
		switch (iEvent) {
		case GC_EVENT_JOIN:
			if (bInactive || !ci.pSettings->SoundsFocus)
				SkinPlaySound("ChatJoin");
			break;
		case GC_EVENT_PART:
			if (bInactive || !ci.pSettings->SoundsFocus)
				SkinPlaySound("ChatPart");
			break;
		case GC_EVENT_QUIT:
			if (bInactive || !ci.pSettings->SoundsFocus)
				SkinPlaySound("ChatQuit");
			break;
		case GC_EVENT_ADDSTATUS:
		case GC_EVENT_REMOVESTATUS:
			if (bInactive || !ci.pSettings->SoundsFocus)
				SkinPlaySound("ChatMode");
			break;
		case GC_EVENT_KICK:
			if (bInactive || !ci.pSettings->SoundsFocus)
				SkinPlaySound("ChatKick");
			break;
		case GC_EVENT_MESSAGE:
			if (bInactive || !ci.pSettings->SoundsFocus)
				SkinPlaySound("ChatMessage");

			if (bInactive && !(si->wState & STATE_TALK)) {
				si->wState |= STATE_TALK;
				db_set_w(si->hContact, si->pszModule, "ApparentMode", (LPARAM)(WORD)40071);
			}
			if (ci.OnFlashWindow)
				ci.OnFlashWindow(si, bInactive);
			break;
		case GC_EVENT_ACTION:
			if (bInactive || !ci.pSettings->SoundsFocus)
				SkinPlaySound("ChatAction");
			break;
		case GC_EVENT_NICK:
			if (bInactive || !ci.pSettings->SoundsFocus)
				SkinPlaySound("ChatNick");
			break;
		case GC_EVENT_NOTICE:
			if (bInactive || !ci.pSettings->SoundsFocus)
				SkinPlaySound("ChatNotice");
			break;
		case GC_EVENT_TOPIC:
			if (bInactive || !ci.pSettings->SoundsFocus)
				SkinPlaySound("ChatTopic");
			break;
		}
	}

	return TRUE;
}

int GetColorIndex(const char* pszModule, COLORREF cr)
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

void CheckColorsInModule(const char* pszModule)
{
	MODULEINFO *pMod = ci.MM_FindModule(pszModule);
	int i = 0;
	COLORREF crFG;
	COLORREF crBG = (COLORREF)db_get_dw(NULL, "Chat", "ColorMessageBG", GetSysColor(COLOR_WINDOW));

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

BOOL IsHighlighted(SESSION_INFO *si, const TCHAR* pszText)
{
	if (ci.pSettings->HighlightEnabled && ci.pSettings->pszHighlightWords && pszText && si->pMe) {
		TCHAR* p1 = ci.pSettings->pszHighlightWords;
		TCHAR* p2 = NULL;
		const TCHAR* p3 = pszText;
		static TCHAR szWord1[1000];
		static TCHAR szWord2[1000];
		static TCHAR szTrimString[] = _T(":,.!?;\'>)");

		// compare word for word
		while (*p1 != '\0') {
			// find the next/first word in the highlight word string
			// skip 'spaces' be4 the word
			while (*p1 == ' ' && *p1 != '\0')
				p1 += 1;

			//find the end of the word
			p2 = _tcschr(p1, ' ');
			if (!p2)
				p2 = _tcschr(p1, '\0');
			if (p1 == p2)
				return FALSE;

			// copy the word into szWord1
			lstrcpyn(szWord1, p1, p2 - p1 > 998 ? 999 : p2 - p1 + 1);
			p1 = p2;

			// replace %m with the users nickname
			p2 = _tcschr(szWord1, '%');
			if (p2 && p2[1] == 'm') {
				TCHAR szTemp[50];

				p2[1] = 's';
				lstrcpyn(szTemp, szWord1, 999);
				mir_sntprintf(szWord1, SIZEOF(szWord1), szTemp, si->pMe->pszNick);
			}

			// time to get the next/first word in the incoming text string
			while (*p3 != '\0') {
				// skip 'spaces' be4 the word
				while (*p3 == ' ' && *p3 != '\0')
					p3 += 1;

				//find the end of the word
				p2 = (TCHAR *)_tcschr(p3, ' ');
				if (!p2)
					p2 = (TCHAR *)_tcschr(p3, '\0');


				if (p3 != p2) {
					// eliminate ending character if needed
					if (p2 - p3 > 1 && _tcschr(szTrimString, p2[-1]))
						p2 -= 1;

					// copy the word into szWord2 and remove formatting
					lstrcpyn(szWord2, p3, p2 - p3 > 998 ? 999 : p2 - p3 + 1);

					// reset the pointer if it was touched because of an ending character
					if (*p2 != '\0' && *p2 != ' ')
						p2 += 1;
					p3 = p2;

					// compare the words, using wildcards
					if (wildcmpit(szWord1, RemoveFormatting(szWord2)))
						return TRUE;
				}
			}

			p3 = pszText;
		}
	}

	return FALSE;
}

BOOL LogToFile(SESSION_INFO *si, GCEVENT *gce)
{
	TCHAR szBuffer[4096];
	TCHAR szLine[4096];
	TCHAR szTime[100];
	TCHAR szFile[MAX_PATH];
	TCHAR szName[MAX_PATH];
	TCHAR szFolder[MAX_PATH];
	char p = '\0';
	szBuffer[0] = '\0';

	if (!si || !gce)
		return FALSE;

	MODULEINFO *mi = ci.MM_FindModule(si->pszModule);
	if (!mi)
		return FALSE;

	TCHAR *szModName = mir_a2t(si->pszModule);
	mir_sntprintf(szName, MAX_PATH, _T("%s"), mi->ptszModDispName ? mi->ptszModDispName : (szModName = mir_a2t(si->pszModule)));
	mir_free(szModName);
	ValidateFilename(szName);
	mir_sntprintf(szFolder, MAX_PATH, _T("%s\\%s"), ci.pSettings->pszLogDir, szName );

	CreateDirectoryTreeT(szFolder);

	mir_sntprintf( szName, MAX_PATH, _T("%s.log"), si->ptszID );
	ValidateFilename(szName);

	mir_sntprintf(szFile, MAX_PATH, _T("%s\\%s"), szFolder, szName );
	lstrcpyn(szTime, MakeTimeStamp(ci.pSettings->pszTimeStampLog, gce->time), 99);

	FILE *hFile = _tfopen(szFile, _T("at+"));
	if (hFile) {
		TCHAR szTemp[512], szTemp2[512];
		TCHAR* pszNick = NULL;
		if ( gce->ptszNick ) {
			if ( ci.pSettings->LogLimitNames && lstrlen(gce->ptszNick) > 20 ) {
				lstrcpyn(szTemp2, gce->ptszNick, 20);
				lstrcpyn(szTemp2+20, _T("..."), 4);
			}
			else lstrcpyn(szTemp2, gce->ptszNick, 511);

			if (gce->ptszUserInfo)
				mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%s (%s)"), szTemp2, gce->ptszUserInfo);
			else
				mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%s"), szTemp2);
			pszNick = szTemp;
		}
		switch (gce->pDest->iType) {
		case GC_EVENT_MESSAGE:
		case GC_EVENT_MESSAGE|GC_EVENT_HIGHLIGHT:
			p = '*';
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), _T("%s * %s"), gce->ptszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_ACTION:
		case GC_EVENT_ACTION|GC_EVENT_HIGHLIGHT:
			p = '*';
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), _T("%s %s"), gce->ptszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_JOIN:
			p = '>';
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s has joined"), (char *)pszNick);
			break;
		case GC_EVENT_PART:
			p = '<';
			if (!gce->ptszText)
				mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s has left"), (char *)pszNick);
			else
				mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s has left (%s)"), (char *)pszNick, RemoveFormatting(gce->ptszText));
				break;
		case GC_EVENT_QUIT:
			p = '<';
			if (!gce->ptszText)
				mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s has disconnected"), (char *)pszNick);
			else
				mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s has disconnected (%s)"), (char *)pszNick,RemoveFormatting(gce->ptszText));
				break;
		case GC_EVENT_NICK:
			p = '^';
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s is now known as %s"), gce->ptszNick, gce->ptszText);
			break;
		case GC_EVENT_KICK:
			p = '~';
			if (!gce->ptszText)
				mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s kicked %s"), (char *)gce->ptszStatus, gce->ptszNick);
			else
				mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s kicked %s (%s)"), (char *)gce->ptszStatus, gce->ptszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_NOTICE:
			p = '¤';
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("Notice from %s: %s"), gce->ptszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_TOPIC:
			p = '#';
			if (!gce->ptszNick)
				mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("The topic is \'%s\'"), RemoveFormatting(gce->ptszText));
			else
				mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("The topic is \'%s\' (set by %s)"), RemoveFormatting(gce->ptszText), gce->ptszNick);
			break;
		case GC_EVENT_INFORMATION:
			p = '!';
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), _T("%s"), RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_ADDSTATUS:
			p = '+';
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s enables \'%s\' status for %s"), gce->ptszText, (char *)gce->ptszStatus, gce->ptszNick);
			break;
		case GC_EVENT_REMOVESTATUS:
			p = '-';
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s disables \'%s\' status for %s"), gce->ptszText, (char *)gce->ptszStatus, gce->ptszNick);
			break;
		}
		if (p)
			mir_sntprintf(szLine, SIZEOF(szLine), TranslateT("%s %c %s\n"), szTime, p, szBuffer);
		else
			mir_sntprintf(szLine, SIZEOF(szLine), TranslateT("%s %s\n"), szTime, szBuffer);

		if (szLine[0]) {
			char *p = mir_t2a(szLine);
			fputs(p, hFile);
			mir_free(p);

			if (ci.pSettings->LoggingLimit > 0) {
				DWORD dwSize;
				DWORD trimlimit;

				fseek(hFile, 0, SEEK_END);
				dwSize = ftell(hFile);
				rewind(hFile);
				trimlimit = ci.pSettings->LoggingLimit * 1024 + 1024 * 10;
				if (dwSize > trimlimit) {
					BYTE * pBuffer = 0;
					BYTE * pBufferTemp = 0;
					int read = 0;

					pBuffer = (BYTE *)mir_alloc(ci.pSettings->LoggingLimit * 1024 + 1);
					pBuffer[ci.pSettings->LoggingLimit * 1024] = '\0';
					fseek(hFile, -ci.pSettings->LoggingLimit * 1024, SEEK_END);
					read = (int)fread(pBuffer, 1, ci.pSettings->LoggingLimit * 1024, hFile);
					fclose(hFile);
					hFile = NULL;

					// trim to whole lines, should help with broken log files I hope.
					pBufferTemp = (BYTE *)strchr((char *)pBuffer, '\n');
					if (pBufferTemp) {
						pBufferTemp++;
						read -= pBufferTemp - pBuffer;
					}
					else pBufferTemp = pBuffer;

					if (read > 0) {
						hFile = _tfopen(szFile, _T("wt"));
						if (hFile) {
							fwrite(pBufferTemp, 1, read, hFile);
							fclose(hFile); hFile = NULL;
						}
					}

					mir_free(pBuffer);
				}
			}
		}

		if (hFile)
			fclose(hFile); hFile = NULL;
		return TRUE;
	}

	return FALSE;
}

BOOL DoEventHookAsync(HWND hwnd, const TCHAR *pszID, const char* pszModule, int iType, TCHAR* pszUID, TCHAR* pszText, DWORD dwItem)
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

BOOL DoEventHook(const TCHAR *pszID, const char* pszModule, int iType, const TCHAR* pszUID, const TCHAR* pszText, DWORD dwItem)
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
