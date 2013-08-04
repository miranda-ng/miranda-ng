/*
 * Miranda NG: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2009 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of tabSRMM messaging plugin for Miranda.
 *
 * This code is based on and still contains large parts of the the
 * original chat module for Miranda NG, written and copyrighted
 * by Joergen Persson in 2005.
 *
 * (C) 2005-2009 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * Helper functions for the group chat module.
 *
 */

#include "..\commonheaders.h"

int GetRichTextLength(HWND hwnd)
{
	GETTEXTLENGTHEX gtl;
	gtl.flags = GTL_PRECISE;
	gtl.codepage = CP_ACP ;
	return (int) SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
}

/**
 * @param pszWord 		string to strip of any IRC-style
 *  					formatting
 * @param fToLower  	convert to lowercase
 * @param fStripCR		strip cr/lf sequences (only for highlighter, defaults to false)
 * @param buf 			caller-provided buffer, use a static one
 *  					when the caller does not provide a
 *  					buffer
 *  					caller provided buffer is NEEDED to make
 *  					this thread-safe.
 *
 * @return TCHAR*		the stripped string
 */
TCHAR* RemoveFormatting(const TCHAR* pszWord, bool fToLower, bool fStripCR, TCHAR* buf, const size_t len)
{
	static TCHAR 	_szTemp[20000];
	TCHAR*			szTemp = 0;
	size_t			_buflen = 0;

	if (0 == buf) {
		szTemp = _szTemp;
		_buflen = 20000;
	}
	else {
		szTemp = buf;
		_buflen = len;
		szTemp[len - 1] = 0;
	}

	size_t i = 0;
	size_t j = 0;

	if (pszWord == 0)
		return NULL;

	size_t wordlen = lstrlen(pszWord);

	while (j < _buflen && i <= wordlen) {
		if (pszWord[i] == '%') {
			switch (pszWord[i+1]) {
			case '%':
				szTemp[j] = '%';
				j++;
				i++;
				i++;
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
				i++;
				i++;
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
			if (fStripCR) {
				if (0x0a == pszWord[i] || 0x0c == pszWord[i]) {
					szTemp[j++] = ' ';
					i++;
					continue;
				}
			}
			szTemp[j] = pszWord[i];
			j++;
			i++;
		}
	}
	if (fToLower) {
		_wsetlocale(LC_ALL, L"");
		wcslwr(szTemp);
	}
	return(szTemp);
}

static void __stdcall ShowRoomFromPopup(void * pi)
{
	SESSION_INFO *si = (SESSION_INFO*) pi;
	ShowRoom(si, WINDOW_VISIBLE, TRUE);
}

static void TSAPI Chat_OpenPopup(SESSION_INFO *si, HWND hwndPopup)
{
	CallFunctionAsync(ShowRoomFromPopup, si);
	PUDeletePopup(hwndPopup);
}

static void TSAPI Chat_DismissPopup(const SESSION_INFO *si, HWND hwndPopup)
{
	if (si->hContact)
		if (CallService(MS_CLIST_GETEVENT, (WPARAM)si->hContact, 0))
			CallService(MS_CLIST_REMOVEEVENT, (WPARAM)si->hContact, (LPARAM)szChatIconString);

	if (si->hWnd && KillTimer(si->hWnd, TIMERID_FLASHWND))
		FlashWindow(si->hWnd, FALSE);

	PUDeletePopup(hwndPopup);
}

static INT_PTR CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	SESSION_INFO *si = (SESSION_INFO*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd, 0);;

	switch (message) {
	case WM_COMMAND:
		if ( HIWORD(wParam) == STN_CLICKED) {
			if (si) {
				if (nen_options.maskActL & MASK_OPEN)
					Chat_OpenPopup(si, hWnd);
				else
					Chat_DismissPopup(si, hWnd);
			}
			return TRUE;
		}
		break;

	case WM_CONTEXTMENU:
		if (si && si->hContact) {
			if (nen_options.maskActR & MASK_OPEN)
				Chat_OpenPopup(si, hWnd);
			else
				Chat_DismissPopup(si, hWnd);
		}
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

static int ShowPopup(HANDLE hContact, SESSION_INFO *si, HICON hIcon,  char* pszProtoName,  TCHAR* pszRoomName, COLORREF crBkg, const TCHAR* fmt, ...)
{
	POPUPDATAT pd = {0};
	va_list marker;
	static TCHAR szBuf[4*1024];

	if (!fmt || lstrlen(fmt) == 0 || lstrlen(fmt) > 2000)
		return 0;

	va_start(marker, fmt);
	mir_vsntprintf(szBuf, SIZEOF(szBuf), fmt, marker);
	va_end(marker);

	pd.lchContact = hContact;

	if (hIcon)
		pd.lchIcon = hIcon ;
	else
		pd.lchIcon = LoadIconEx(IDI_CHANMGR, "window", 0, 0);

	mir_sntprintf(pd.lptzContactName, MAX_CONTACTNAME - 1, _T("%S - %s"), pszProtoName, pcli->pfnGetContactDisplayName(hContact, 0));
	lstrcpyn(pd.lptzText, TranslateTS(szBuf), MAX_SECONDLINE - 1);
	pd.iSeconds = g_Settings.iPopupTimeout;

	if (g_Settings.iPopupStyle == 2) {
		pd.colorBack = 0;
		pd.colorText = 0;
	} else if (g_Settings.iPopupStyle == 3) {
		pd.colorBack = g_Settings.crPUBkgColour;
		pd.colorText = g_Settings.crPUTextColour;
	} else {
		pd.colorBack = M.GetDword(FONTMODULE, SRMSGSET_BKGCOLOUR_MUC, SRMSGDEFSET_BKGCOLOUR);
		pd.colorText = crBkg;
	}

	pd.PluginWindowProc = (WNDPROC)PopupDlgProc;
	pd.PluginData = si;
	return PUAddPopupT(&pd);
}

static BOOL DoTrayIcon(SESSION_INFO *si, GCEVENT *gce)
{
	int iEvent = gce->pDest->iType;

	if (si && (iEvent & si->iLogTrayFlags)) {
		switch (iEvent) {
		case GC_EVENT_MESSAGE | GC_EVENT_HIGHLIGHT :
		case GC_EVENT_ACTION | GC_EVENT_HIGHLIGHT :
			CList_AddEvent(si->hContact, PluginConfig.g_IconMsgEvent, szChatIconString, 0,
				TranslateT("%s wants your attention in %s"), gce->ptszNick, si->ptszName);
			break;
		case GC_EVENT_MESSAGE :
			CList_AddEvent(si->hContact, hIcons[ICON_MESSAGE], szChatIconString, CLEF_ONLYAFEW,
				TranslateT("%s speaks in %s"), gce->ptszNick, si->ptszName);
			break;
		case GC_EVENT_ACTION:
			CList_AddEvent(si->hContact, hIcons[ICON_ACTION], szChatIconString, CLEF_ONLYAFEW,
				TranslateT("%s speaks in %s"), gce->ptszNick, si->ptszName);
			break;
		case GC_EVENT_JOIN:
			CList_AddEvent(si->hContact, hIcons[ICON_JOIN], szChatIconString, CLEF_ONLYAFEW,
				TranslateT("%s has joined %s"), gce->ptszNick, si->ptszName);
			break;
		case GC_EVENT_PART:
			CList_AddEvent(si->hContact, hIcons[ICON_PART], szChatIconString, CLEF_ONLYAFEW,
				TranslateT("%s has left %s"), gce->ptszNick, si->ptszName);
			break;
		case GC_EVENT_QUIT:
			CList_AddEvent(si->hContact, hIcons[ICON_QUIT], szChatIconString, CLEF_ONLYAFEW,
				TranslateT("%s has disconnected"), gce->ptszNick);
			break;
		case GC_EVENT_NICK:
			CList_AddEvent(si->hContact, hIcons[ICON_NICK], szChatIconString, CLEF_ONLYAFEW,
				TranslateT("%s is now known as %s"), gce->ptszNick, gce->pszText);
			break;
		case GC_EVENT_KICK:
			CList_AddEvent(si->hContact, hIcons[ICON_KICK], szChatIconString, CLEF_ONLYAFEW,
				TranslateT("%s kicked %s from %s"), gce->pszStatus, gce->ptszNick, si->ptszName);
			break;
		case GC_EVENT_NOTICE:
			CList_AddEvent(si->hContact, hIcons[ICON_NOTICE], szChatIconString, CLEF_ONLYAFEW,
				TranslateT("Notice from %s"), gce->ptszNick);
			break;
		case GC_EVENT_TOPIC:
			CList_AddEvent(si->hContact, hIcons[ICON_TOPIC], szChatIconString, CLEF_ONLYAFEW,
				TranslateT("Topic change in %s"), si->ptszName);
			break;
		case GC_EVENT_INFORMATION:
			CList_AddEvent(si->hContact, hIcons[ICON_INFO], szChatIconString, CLEF_ONLYAFEW,
				TranslateT("Information in %s"), si->ptszName);
			break;
		case GC_EVENT_ADDSTATUS:
			CList_AddEvent(si->hContact, hIcons[ICON_ADDSTATUS], szChatIconString, CLEF_ONLYAFEW,
				TranslateT("%s enables \'%s\' status for %s in %s"), gce->pszText, gce->pszStatus, gce->ptszNick, si->ptszName);
			break;
		case GC_EVENT_REMOVESTATUS:
			CList_AddEvent(si->hContact, hIcons[ICON_REMSTATUS], szChatIconString, CLEF_ONLYAFEW,
				TranslateT("%s disables \'%s\' status for %s in %s"), gce->pszText, gce->pszStatus, gce->ptszNick, si->ptszName);
			break;
		}
	}
	return TRUE;
}

static BOOL DoPopup(SESSION_INFO *si, GCEVENT* gce, TWindowData *dat)
{
	int iEvent = gce->pDest->iType;
	TContainerData *pContainer = dat ? dat->pContainer : NULL;
	char *szProto = dat ? dat->szProto : si->pszModule;

	TCHAR *bbStart, *bbEnd;
	if (g_Settings.bBBCodeInPopups) {
		bbStart = _T("[b]");
		bbEnd = _T("[/b]");
	}
	else bbStart = bbEnd = _T("");

	if (si && (iEvent & si->iLogPopupFlags)) {
		if (nen_options.iMUCDisable)                          // no popups at all. Period
			return 0;
		/*
		* check the status mode against the status mask
		*/

		if (nen_options.dwStatusMask != -1) {
			DWORD dwStatus = 0;
			if (szProto != NULL) {
				dwStatus = (DWORD)CallProtoService(szProto, PS_GETSTATUS, 0, 0);
				if (!(dwStatus == 0 || dwStatus <= ID_STATUS_OFFLINE || ((1 << (dwStatus - ID_STATUS_ONLINE)) & nen_options.dwStatusMask)))            // should never happen, but...
					return 0;
			}
		}
		if (dat && pContainer != 0) {                // message window is open, need to check the container config if we want to see a popup nonetheless
			if (nen_options.bWindowCheck) {                  // no popups at all for open windows... no exceptions
				if (!PluginConfig.m_HideOnClose)
					return 0;
				if (pContainer->fHidden)
					goto passed;
				return 0;
			}
			if (pContainer->dwFlags & CNT_DONTREPORT && IsIconic(pContainer->hwnd))        // in tray counts as "minimised"
				goto passed;
			if (pContainer->dwFlags & CNT_DONTREPORTUNFOCUSED) {
				if (!IsIconic(pContainer->hwnd) && GetForegroundWindow() != pContainer->hwnd && GetActiveWindow() != pContainer->hwnd)
					goto passed;
			}
			if (pContainer->dwFlags & CNT_ALWAYSREPORTINACTIVE) {
				if (pContainer->dwFlags & CNT_DONTREPORTFOCUSED)
					goto passed;

				if (pContainer->hwndActive == si->hWnd)
					return 0;

				goto passed;
			}
			return 0;
		}
passed:
		int iNewEvent = iEvent;
		COLORREF clr = 0;

		if ((iNewEvent & GC_EVENT_HIGHLIGHT)) {
			clr = aFonts[16].color;
			iNewEvent &= ~GC_EVENT_HIGHLIGHT;
		}

		switch (iNewEvent) {
		case GC_EVENT_MESSAGE :
			ShowPopup(si->hContact, si, hIcons[ICON_MESSAGE], si->pszModule, si->ptszName, clr ? clr : aFonts[9].color,
				TranslateT("%s%s says:%s %s"), bbStart, gce->ptszNick, bbEnd, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_ACTION:
			ShowPopup(si->hContact, si, hIcons[ICON_ACTION], si->pszModule, si->ptszName, clr ? clr : aFonts[15].color, _T("%s %s"), gce->ptszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_JOIN:
			ShowPopup(si->hContact, si, hIcons[ICON_JOIN], si->pszModule, si->ptszName, clr ? clr : aFonts[3].color,
				TranslateT("%s has joined"), gce->ptszNick);
			break;
		case GC_EVENT_PART:
			if (!gce->pszText)
				ShowPopup(si->hContact, si, hIcons[ICON_PART], si->pszModule, si->ptszName, clr ? clr : aFonts[4].color,
					TranslateT("%s has left"), gce->ptszNick);
			else
				ShowPopup(si->hContact, si, hIcons[ICON_PART], si->pszModule, si->ptszName, clr ? clr : aFonts[4].color,
					TranslateT("%s has left (%s)"), gce->ptszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_QUIT:
			if (!gce->pszText)
				ShowPopup(si->hContact, si, hIcons[ICON_QUIT], si->pszModule, si->ptszName, clr ? clr : aFonts[5].color,
					TranslateT("%s has disconnected"), gce->ptszNick);
			else
				ShowPopup(si->hContact, si, hIcons[ICON_QUIT], si->pszModule, si->ptszName, clr ? clr : aFonts[5].color,
					TranslateT("%s has disconnected (%s)"), gce->ptszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_NICK:
			ShowPopup(si->hContact, si, hIcons[ICON_NICK], si->pszModule, si->ptszName, clr ? clr : aFonts[7].color,
				TranslateT("%s is now known as %s"), gce->ptszNick, gce->ptszText);
			break;
		case GC_EVENT_KICK:
			if (!gce->pszText)
				ShowPopup(si->hContact, si, hIcons[ICON_KICK], si->pszModule, si->ptszName, clr ? clr : aFonts[6].color,
					TranslateT("%s kicked %s"), (char *)gce->pszStatus, gce->ptszNick);
			else
				ShowPopup(si->hContact, si, hIcons[ICON_KICK], si->pszModule, si->ptszName, clr ? clr : aFonts[6].color,
					TranslateT("%s kicked %s (%s)"), (char *)gce->pszStatus, gce->ptszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_NOTICE:
			ShowPopup(si->hContact, si, hIcons[ICON_NOTICE], si->pszModule, si->ptszName, clr ? clr : aFonts[8].color,
				TranslateT("Notice from %s: %s"), gce->ptszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_TOPIC:
			if (!gce->ptszNick)
				ShowPopup(si->hContact, si, hIcons[ICON_TOPIC], si->pszModule, si->ptszName, clr ? clr : aFonts[11].color,
					TranslateT("The topic is \'%s\'"), RemoveFormatting(gce->ptszText));
			else
				ShowPopup(si->hContact, si, hIcons[ICON_TOPIC], si->pszModule, si->ptszName, clr ? clr : aFonts[11].color,
					TranslateT("The topic is \'%s\' (set by %s)"), RemoveFormatting(gce->ptszText), gce->ptszNick);
			break;
		case GC_EVENT_INFORMATION:
			ShowPopup(si->hContact, si, hIcons[ICON_INFO], si->pszModule, si->ptszName, clr ? clr : aFonts[12].color, _T("%s"), RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_ADDSTATUS:
			ShowPopup(si->hContact, si, hIcons[ICON_ADDSTATUS], si->pszModule, si->ptszName, clr ? clr : aFonts[13].color,
				TranslateT("%s enables \'%s\' status for %s"), gce->ptszText, (char *)gce->pszStatus, gce->ptszNick);
			break;
		case GC_EVENT_REMOVESTATUS:
			ShowPopup(si->hContact, si, hIcons[ICON_REMSTATUS], si->pszModule, si->ptszName, clr ? clr : aFonts[14].color,
				TranslateT("%s disables \'%s\' status for %s"), gce->ptszText, (char *)gce->pszStatus, gce->ptszNick);
			break;
		}
	}

	return TRUE;
}

void TSAPI DoFlashAndSoundWorker(FLASH_PARAMS* p)
{
	SESSION_INFO *si = SM_FindSessionByHCONTACT(p->hContact);
	if (si == 0)
		return;

	TWindowData *dat = 0;
	if (si->hWnd) {
		dat = si->dat;
		if (dat) {
			p->bInactive = dat->pContainer->hwnd != GetForegroundWindow();
			p->bActiveTab = (dat->pContainer->hwndActive == si->hWnd);
		}
		if (p->sound && Utils::mustPlaySound(si->dat))
			SkinPlaySound(p->sound);
	}
	else if (p->sound)
		SkinPlaySound(p->sound);

	if (dat) {
		HWND hwndTab = GetParent(si->hWnd);
		BOOL bForcedIcon = (p->hNotifyIcon == hIcons[ICON_HIGHLIGHT] || p->hNotifyIcon == hIcons[ICON_MESSAGE]);

		if ((p->iEvent & si->iLogTrayFlags) || bForcedIcon) {
			if (!p->bActiveTab) {
				if (p->hNotifyIcon == hIcons[ICON_HIGHLIGHT])
					dat->iFlashIcon = p->hNotifyIcon;
				else {
					if (dat->iFlashIcon != hIcons[ICON_HIGHLIGHT] && dat->iFlashIcon != hIcons[ICON_MESSAGE])
						dat->iFlashIcon = p->hNotifyIcon;
				}
				dat->mayFlashTab = TRUE;
				SetTimer(si->hWnd, TIMERID_FLASHWND, TIMEOUT_FLASHWND, NULL);
			}
		}
		if (dat->pWnd) {
			dat->pWnd->updateIcon(p->hNotifyIcon);
			dat->pWnd->setOverlayIcon(p->hNotifyIcon, true);
		}

		// autoswitch tab..
		if (p->bMustAutoswitch) {
			if ((IsIconic(dat->pContainer->hwnd)) && !IsZoomed(dat->pContainer->hwnd) && PluginConfig.haveAutoSwitch() && dat->pContainer->hwndActive != si->hWnd) {
				int iItem = GetTabIndexFromHWND(hwndTab, si->hWnd);
				if (iItem >= 0) {
					TabCtrl_SetCurSel(hwndTab, iItem);
					ShowWindow(dat->pContainer->hwndActive, SW_HIDE);
					dat->pContainer->hwndActive = si->hWnd;
					SendMessage(dat->pContainer->hwnd, DM_UPDATETITLE, (WPARAM)dat->hContact, 0);
					dat->pContainer->dwFlags |= CNT_DEFERREDTABSELECT;
				}
			}
		}

		/*
		* flash window if it is not focused
		*/
		if (p->bMustFlash && p->bInactive)
			if (!(dat->pContainer->dwFlags & CNT_NOFLASH))
				FlashContainer(dat->pContainer, 1, 0);

		if (p->hNotifyIcon && p->bInactive && ((p->iEvent & si->iLogTrayFlags) || bForcedIcon)) {
			if (p->bMustFlash)
				dat->hTabIcon = p->hNotifyIcon;
			else if (dat->iFlashIcon) {
				dat->hTabIcon = dat->iFlashIcon;

				TCITEM item = {0};
				item.mask = TCIF_IMAGE;
				item.iImage = 0;
				TabCtrl_SetItem(GetParent(si->hWnd), dat->iTabID, &item);
			}

			HICON hIcon = (HICON)SendMessage(dat->pContainer->hwnd, WM_GETICON, ICON_BIG, 0);
			if (p->hNotifyIcon == hIcons[ICON_HIGHLIGHT] || (hIcon != hIcons[ICON_MESSAGE] && hIcon != hIcons[ICON_HIGHLIGHT])) {
				SendMessage(dat->pContainer->hwnd, DM_SETICON, (WPARAM)dat, (LPARAM)p->hNotifyIcon);
				dat->pContainer->dwFlags |= CNT_NEED_UPDATETITLE;
			}
		}

		if (p->bMustFlash && p->bInactive)
			UpdateTrayMenu(dat, si->wStatus, si->pszModule, dat ? dat->szStatus : NULL, si->hContact, p->bHighlight ? 1 : 1);
	}

	free(p);
}

BOOL DoSoundsFlashPopupTrayStuff(SESSION_INFO *si, GCEVENT *gce, BOOL bHighlight, int bManyFix)
{
	if (gce == 0 || si == 0 || gce->bIsMe || si->iType == GCW_SERVER)
		return FALSE;

	TWindowData *dat = NULL;
	FLASH_PARAMS *params = (FLASH_PARAMS*)calloc(1, sizeof(FLASH_PARAMS));
	params->hContact = si->hContact;
	params->bInactive = TRUE;
	if (si->hWnd && si->dat) {
		dat = si->dat;
		if ((si->hWnd == si->dat->pContainer->hwndActive) && GetForegroundWindow() == si->dat->pContainer->hwnd)
			params->bInactive = FALSE;
	}
	params->bActiveTab = params->bMustFlash = params->bMustAutoswitch = FALSE;
	params->iEvent = gce->pDest->iType;

	WPARAM wParamForHighLight = 0;
	bool bFlagUnread = false;
	if (bHighlight) {
		gce->pDest->iType |= GC_EVENT_HIGHLIGHT;
		params->sound = "ChatHighlight";
		if (db_get_b(si->hContact, "CList", "Hidden", 0) != 0)
			db_unset(si->hContact, "CList", "Hidden");
		if (params->bInactive) {
			bFlagUnread = true;
			DoTrayIcon(si, gce);
		}

		/* TODO fix for 3.0 final !!! */
#if !defined(__DELAYED_FOR_3_1)
		if (g_Settings.bCreateWindowOnHighlight && 0 == dat)
			wParamForHighLight = 1;

		if (dat && g_Settings.bAnnoyingHighlight && params->bInactive && dat->pContainer->hwnd != GetForegroundWindow()) {
			wParamForHighLight = 2;
			params->hWnd = dat->hwnd;
		}
#endif
		if (dat || !nen_options.iMUCDisable)
			DoPopup(si, gce, dat);
		if (params->bInactive && si && si->hWnd)
			SendMessage(si->hWnd, GC_SETMESSAGEHIGHLIGHT, 0, (LPARAM)si);
		if (g_Settings.bFlashWindowHightlight && params->bInactive)
			params->bMustFlash = TRUE;
		params->bMustAutoswitch = TRUE;
		params->hNotifyIcon = hIcons[ICON_HIGHLIGHT];
	}
	else {
		// do blinking icons in tray
		if (params->bInactive || !g_Settings.bTrayIconInactiveOnly) {
			DoTrayIcon(si, gce);
			if (params->iEvent == GC_EVENT_MESSAGE)
				bFlagUnread = true;
		}
		// stupid thing to not create multiple popups for a QUIT event for instance
		if (bManyFix == 0) {
			// do popups
			if (dat || !nen_options.iMUCDisable)
				DoPopup(si, gce, dat);

			// do sounds and flashing
			switch (params->iEvent) {
			case GC_EVENT_JOIN:
				params->sound = "ChatJoin";
				if (params->bInactive)
					params->hNotifyIcon = hIcons[ICON_JOIN];
				break;
			case GC_EVENT_PART:
				params->sound = "ChatPart";
				if (params->bInactive)
					params->hNotifyIcon = hIcons[ICON_PART];
				break;
			case GC_EVENT_QUIT:
				params->sound = "ChatQuit";
				if (params->bInactive)
					params->hNotifyIcon = hIcons[ICON_QUIT];
				break;
			case GC_EVENT_ADDSTATUS:
			case GC_EVENT_REMOVESTATUS:
				params->sound = "ChatMode";
				if (params->bInactive)
					params->hNotifyIcon = hIcons[params->iEvent == GC_EVENT_ADDSTATUS ? ICON_ADDSTATUS : ICON_REMSTATUS];
				break;
			case GC_EVENT_KICK:
				params->sound = "ChatKick";
				if (params->bInactive)
					params->hNotifyIcon = hIcons[ICON_KICK];
				break;
			case GC_EVENT_MESSAGE:
				params->sound = "ChatMessage";
				if (params->bInactive && !(si->wState & STATE_TALK)) {
					si->wState |= STATE_TALK;
					db_set_w(si->hContact, si->pszModule, "ApparentMode", (LPARAM)(WORD) 40071);
				}
				break;
			case GC_EVENT_ACTION:
				params->sound = "ChatAction";
				if (params->bInactive)
					params->hNotifyIcon = hIcons[ICON_ACTION];
				break;
			case GC_EVENT_NICK:
				params->sound = "ChatNick";
				if (params->bInactive)
					params->hNotifyIcon = hIcons[ICON_NICK];
				break;
			case GC_EVENT_NOTICE:
				params->sound = "ChatNotice";
				if (params->bInactive)
					params->hNotifyIcon = hIcons[ICON_NOTICE];
				break;
			case GC_EVENT_TOPIC:
				params->sound = "ChatTopic";
				if (params->bInactive)
					params->hNotifyIcon = hIcons[ICON_TOPIC];
				break;
			}
		}
		else {
			switch (params->iEvent) {
			case GC_EVENT_JOIN:
				params->hNotifyIcon = hIcons[ICON_JOIN];
				break;
			case GC_EVENT_PART:
				params->hNotifyIcon = hIcons[ICON_PART];
				break;
			case GC_EVENT_QUIT:
				params->hNotifyIcon = hIcons[ICON_QUIT];
				break;
			case GC_EVENT_KICK:
				params->hNotifyIcon = hIcons[ICON_KICK];
				break;
			case GC_EVENT_ACTION:
				params->hNotifyIcon = hIcons[ICON_ACTION];
				break;
			case GC_EVENT_NICK:
				params->hNotifyIcon = hIcons[ICON_NICK];
				break;
			case GC_EVENT_NOTICE:
				params->hNotifyIcon = hIcons[ICON_NOTICE];
				break;
			case GC_EVENT_TOPIC:
				params->hNotifyIcon = hIcons[ICON_TOPIC];
				break;
			case GC_EVENT_ADDSTATUS:
				params->hNotifyIcon = hIcons[ICON_ADDSTATUS];
				break;
			case GC_EVENT_REMOVESTATUS:
				params->hNotifyIcon = hIcons[ICON_REMSTATUS];
				break;
			}
		}

		if (params->iEvent == GC_EVENT_MESSAGE) {
			params->bMustAutoswitch = TRUE;
			if (g_Settings.bFlashWindow)
				params->bMustFlash = TRUE;
			params->hNotifyIcon = hIcons[ICON_MESSAGE];
		}
	}
	if (dat && bFlagUnread) {
		dat->dwUnread++;
		if (dat->pWnd)
			dat->pWnd->Invalidate();
	}
	PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_MUCFLASHWORKER, wParamForHighLight, (LPARAM)params);
	return TRUE;
}

int Chat_GetColorIndex(const char* pszModule, COLORREF cr)
{
	MODULEINFO *pMod = MM_FindModule(pszModule);
	if (!pMod || pMod->nColorCount == 0)
		return -1;

	for (int i=0; i < pMod->nColorCount; i++)
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
	MODULEINFO * pMod = MM_FindModule(pszModule);
	COLORREF crFG;
	COLORREF crBG = (COLORREF)M.GetDword(FONTMODULE, "inputbg", SRMSGDEFSET_BKGCOLOUR);

	LoadLogfont(MSGFONTID_MESSAGEAREA, NULL, &crFG, FONTMODULE);

	if (!pMod)
		return;

	for (int i=0; i < pMod->nColorCount; i++) {
		if (pMod->crColors[i] == crFG || pMod->crColors[i] == crBG) {
			if (pMod->crColors[i] == RGB(255, 255, 255))
				pMod->crColors[i]--;
			else
				pMod->crColors[i]++;
		}
	}
}

TCHAR* my_strstri(const TCHAR* s1, const TCHAR* s2)
{
	int i, j, k;

	_tsetlocale(LC_ALL, _T(""));
	for (i=0;s1[i];i++)
		for (j = i, k = 0; _totlower(s1[j]) == _totlower(s2[k]);j++, k++)
			if (!s2[k+1])
				return (TCHAR*)(s1 + i);

	return NULL;
}

/*
 * log the event to the log file
 * allows selective logging of wanted events
 */
BOOL LogToFile(SESSION_INFO *si, GCEVENT *gce)
{
	TCHAR szBuffer[4096];
	TCHAR szLine[4096];
	TCHAR szTime[100];
	FILE *hFile = NULL;
	TCHAR tszFolder[MAX_PATH];
	TCHAR p = '\0';
	BOOL bFileJustCreated = TRUE;

	if (!si || !gce)
		return FALSE;

	MODULEINFO *mi = MM_FindModule(si->pszModule);
	if (!mi)
		return FALSE;

	/*
     * check whether we have to log this event
    */
	if (!(gce->pDest->iType & si->iDiskLogFlags))
		return FALSE;

	szBuffer[0] = '\0';

	GetChatLogsFilename(si, gce->time);
	bFileJustCreated = !PathFileExists(si->pszLogFileName);
	_tcscpy(tszFolder, si->pszLogFileName);
	PathRemoveFileSpec(tszFolder);
	if (!PathIsDirectory(tszFolder))
		CreateDirectoryTreeT(tszFolder);

	lstrcpyn(szTime, MakeTimeStamp(g_Settings.pszTimeStampLog, gce->time), 99);

	hFile = _tfopen(si->pszLogFileName, _T("ab+"));
	if (hFile) {
		TCHAR szTemp[512], szTemp2[512];
		TCHAR* pszNick = NULL;
		if (bFileJustCreated)
			fputws((const wchar_t*)"\377\376", hFile);		//UTF-16 LE BOM == FF FE
		if (gce->ptszNick) {
			if (g_Settings.bLogLimitNames && lstrlen(gce->ptszNick) > 20) {
				lstrcpyn(szTemp2, gce->ptszNick, 20);
				lstrcpyn(szTemp2 + 20, _T("..."), 4);
			}
			else lstrcpyn(szTemp2, gce->ptszNick, 511);

			if (gce->pszUserInfo)
				mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%s (%s)"), szTemp2, gce->pszUserInfo);
			else
				mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%s"), szTemp2);
			pszNick = szTemp;
		}

		switch (gce->pDest->iType) {
		case GC_EVENT_MESSAGE:
		case GC_EVENT_MESSAGE | GC_EVENT_HIGHLIGHT:
			p = '*';
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), _T("%s: %s"), gce->ptszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_ACTION:
		case GC_EVENT_ACTION | GC_EVENT_HIGHLIGHT:
			p = '*';
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), _T("%s %s"), gce->ptszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_JOIN:
			p = '>';
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s has joined"), (char *)pszNick);
			break;
		case GC_EVENT_PART:
			p = '<';
			if (!gce->pszText)
				mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s has left"), (char *)pszNick);
			else
				mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s has left (%s)"), (char *)pszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_QUIT:
			p = '<';
			if (!gce->pszText)
				mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s has disconnected"), (char *)pszNick);
			else
				mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s has disconnected (%s)"), (char *)pszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_NICK:
			p = '^';
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s is now known as %s"), gce->ptszNick, gce->ptszText);
			break;
		case GC_EVENT_KICK:
			p = '~';
			if (!gce->pszText)
				mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s kicked %s"), (char *)gce->pszStatus, gce->ptszNick);
			else
				mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s kicked %s (%s)"), (char *)gce->pszStatus, gce->ptszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_NOTICE:
			p = 'o';
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("Notice from %s: %s"), gce->ptszNick, RemoveFormatting(gce->ptszText));
			break;
		case GC_EVENT_TOPIC:
			p = '#';
			if (!gce->pszNick)
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
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s enables \'%s\' status for %s"), gce->ptszText, (char *)gce->pszStatus, gce->ptszNick);
			break;
		case GC_EVENT_REMOVESTATUS:
			p = '-';
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("%s disables \'%s\' status for %s"), gce->ptszText, (char *)gce->pszStatus, gce->ptszNick);
			break;
		}

		/*
		 * formatting strings don't need to be translatable - changing them via language pack would
		 * only screw up the log format.
		 */
		if (p)
			mir_sntprintf(szLine, SIZEOF(szLine), _T("%s %c %s\r\n"), szTime, p, szBuffer);
		else
			mir_sntprintf(szLine, SIZEOF(szLine), _T("%s %s\r\n"), szTime, szBuffer);

		if (szLine[0]) {
			_fputts(szLine, hFile);

			if (g_Settings.LoggingLimit > 0) {
				long  dwSize;
				long  trimlimit;

				fseek(hFile, 0, SEEK_END);
				dwSize = ftell(hFile);
				rewind(hFile);

				trimlimit = g_Settings.LoggingLimit * 1024;
				if (dwSize > trimlimit) {
					TCHAR tszDrive[_MAX_DRIVE];
					TCHAR tszDir[_MAX_DIR];
					TCHAR tszName[_MAX_FNAME];
					TCHAR tszExt[_MAX_EXT];
					TCHAR tszNewName[_MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT + 20];
					TCHAR tszNewPath[_MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT + 20];
					TCHAR tszTimestamp[20];
					time_t now = time(0);

					_tcsftime(tszTimestamp, 20, _T("%Y%m%d-%H%M%S"), _localtime32((__time32_t *)&now));
					tszTimestamp[19] = 0;
					/*
					 * max size reached, rotate the log
					 * move old logs to /archived sub folder just inside the log root folder.
					 * add a time stamp to the file name.
					 */
					_tsplitpath(si->pszLogFileName, tszDrive, tszDir, tszName, tszExt);

					mir_sntprintf(tszNewPath, _MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT + 20, _T("%s%sarchived\\"),
							tszDrive, tszDir);

					CreateDirectoryTreeT(tszNewPath);
					mir_sntprintf(tszNewName, _MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT + 20, _T("%s%s-%s%s"), tszNewPath, tszName, tszTimestamp, tszExt);
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
		hFile = NULL;
		return TRUE;
	}
	return FALSE;
}

UINT CreateGCMenu(HWND hwndDlg, HMENU *hMenu, int iIndex, POINT pt, SESSION_INFO *si, TCHAR* pszUID, TCHAR* pszWordText)
{
	HMENU hSubMenu = 0;
	DWORD codepage = M.GetDword(si->hContact, "ANSIcodepage", 0);

	*hMenu = GetSubMenu(g_hMenu, iIndex);
	TranslateMenu(*hMenu);

	GCMENUITEMS gcmi = {0};
	gcmi.pszID = si->ptszID;
	gcmi.pszModule = si->pszModule;
	gcmi.pszUID = pszUID;

	if (iIndex == 1) {
		int i = GetRichTextLength(GetDlgItem(hwndDlg, IDC_CHAT_LOG));

		EnableMenuItem(*hMenu, ID_CLEARLOG, MF_ENABLED);
		EnableMenuItem(*hMenu, ID_COPYALL, MF_ENABLED);
		ModifyMenu(*hMenu, 4, MF_GRAYED | MF_BYPOSITION, 4, NULL);
		if (!i) {
			EnableMenuItem(*hMenu, ID_COPYALL, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(*hMenu, ID_CLEARLOG, MF_BYCOMMAND | MF_GRAYED);
			if (pszWordText && pszWordText[0])
				ModifyMenu(*hMenu, 4, MF_ENABLED | MF_BYPOSITION, 4, NULL);
		}

		if (pszWordText && pszWordText[0]) {
			TCHAR szMenuText[4096];
			mir_sntprintf(szMenuText, 4096, TranslateT("Look up \'%s\':"), pszWordText);
			ModifyMenu(*hMenu, 4, MF_STRING | MF_BYPOSITION, 4, szMenuText);
		}
		else ModifyMenu(*hMenu, 4, MF_STRING | MF_GRAYED | MF_BYPOSITION, 4, TranslateT("No word to look up"));
		gcmi.Type = MENU_ON_LOG;
	}
	else if (iIndex == 0) {
		TCHAR szTemp[30];
		if (pszUID)
			mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("&Message %s"), pszUID);
		else
			lstrcpyn(szTemp, TranslateT("&Message"), 24);

		if (lstrlen(szTemp) > 22)
			lstrcpyn(szTemp + 22, _T("..."), 4);
		ModifyMenu(*hMenu, ID_MESS, MF_STRING | MF_BYCOMMAND, ID_MESS, szTemp);
		gcmi.Type = MENU_ON_NICKLIST;
	}

	NotifyEventHooks(hBuildMenuEvent, 0, (WPARAM)&gcmi);

	if (gcmi.nItems > 0)
		AppendMenu(*hMenu, MF_SEPARATOR, 0, 0);

	for (int i=0; i < gcmi.nItems; i++) {
		TCHAR *ptszDescr = a2tf(gcmi.Item[i].pszDesc, si->dwFlags);
		TCHAR *ptszText = TranslateTS(ptszDescr);
		DWORD dwState = gcmi.Item[i].bDisabled ? MF_GRAYED : 0;

		if (gcmi.Item[i].uType == MENU_NEWPOPUP) {
			hSubMenu = CreateMenu();
			AppendMenu(*hMenu, dwState | MF_POPUP, (UINT_PTR)hSubMenu, ptszText);
		}
		else if (gcmi.Item[i].uType == MENU_POPUPHMENU)
			AppendMenu(hSubMenu == 0 ? *hMenu : hSubMenu, dwState | MF_POPUP, gcmi.Item[i].dwID, ptszText);
		else if (gcmi.Item[i].uType == MENU_POPUPITEM)
			AppendMenu(hSubMenu == 0 ? *hMenu : hSubMenu, dwState | MF_STRING, gcmi.Item[i].dwID, ptszText);
		else if (gcmi.Item[i].uType == MENU_POPUPCHECK)
			AppendMenu(hSubMenu == 0 ? *hMenu : hSubMenu, dwState | MF_CHECKED | MF_STRING, gcmi.Item[i].dwID, ptszText);
		else if (gcmi.Item[i].uType == MENU_POPUPSEPARATOR)
			AppendMenu(hSubMenu == 0 ? *hMenu : hSubMenu, MF_SEPARATOR, 0, ptszText);
		else if (gcmi.Item[i].uType == MENU_SEPARATOR)
			AppendMenu(*hMenu, MF_SEPARATOR, 0, ptszText);
		else if (gcmi.Item[i].uType == MENU_HMENU)
			AppendMenu(*hMenu, dwState | MF_POPUP, gcmi.Item[i].dwID, ptszText);
		else if (gcmi.Item[i].uType == MENU_ITEM)
			AppendMenu(*hMenu, dwState | MF_STRING, gcmi.Item[i].dwID, ptszText);
		else if (gcmi.Item[i].uType == MENU_CHECK)
			AppendMenu(*hMenu, dwState | MF_CHECKED | MF_STRING, gcmi.Item[i].dwID, ptszText);

		mir_free(ptszDescr);
	}

#if !defined(__DELAYED_FOR_3_1)
	if (iIndex == 0) {
		AppendMenu(*hMenu, MF_SEPARATOR, 0, 0);
		int pos = GetMenuItemCount(*hMenu);
		InsertMenu(*hMenu, pos, MF_BYPOSITION, (UINT_PTR)20020, TranslateT("Highlight User..."));
		InsertMenu(*hMenu, pos, MF_BYPOSITION, (UINT_PTR)20021, TranslateT("Edit Highlight List..."));
	}
#endif
	if (iIndex == 1 && si->iType != GCW_SERVER && !(si->dwFlags & GC_UNICODE)) {
		AppendMenu(*hMenu, MF_SEPARATOR, 0, 0);
		InsertMenu(PluginConfig.g_hMenuEncoding, 1, MF_BYPOSITION | MF_STRING, (UINT_PTR)CP_UTF8, TranslateT("UTF-8"));
		int pos = GetMenuItemCount(*hMenu);
		InsertMenu(*hMenu, pos, MF_BYPOSITION | MF_POPUP, (UINT_PTR) PluginConfig.g_hMenuEncoding, TranslateT("Character Encoding"));
		for (int i=0; i < GetMenuItemCount(PluginConfig.g_hMenuEncoding); i++)
			CheckMenuItem(PluginConfig.g_hMenuEncoding, i, MF_BYPOSITION | MF_UNCHECKED);
		if (codepage == CP_ACP)
			CheckMenuItem(PluginConfig.g_hMenuEncoding, 0, MF_BYPOSITION | MF_CHECKED);
		else
			CheckMenuItem(PluginConfig.g_hMenuEncoding, codepage, MF_BYCOMMAND | MF_CHECKED);

	}

	return TrackPopupMenu(*hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL);
}

void DestroyGCMenu(HMENU *hMenu, int iIndex)
{
	MENUITEMINFO mi = { sizeof(mi) };
	mi.fMask = MIIM_SUBMENU;
	while (GetMenuItemInfo(*hMenu, iIndex, TRUE, &mi)) {
		if (mi.hSubMenu != NULL)
			DestroyMenu(mi.hSubMenu);
		RemoveMenu(*hMenu, iIndex, MF_BYPOSITION);
	}
}

BOOL DoEventHookAsync(HWND hwnd, const TCHAR* pszID, const char* pszModule, int iType, TCHAR* pszUID, TCHAR* pszText, DWORD dwItem)
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return FALSE;

	GCHOOK *gch = (GCHOOK*)mir_calloc(sizeof(GCHOOK));
	GCDEST *gcd = (GCDEST*)mir_calloc(sizeof(GCDEST));
	replaceStr(gcd->pszModule, pszModule);

	if (!(si->dwFlags & GC_UNICODE)) {
		DWORD dwCP = M.GetDword(si->hContact, "ANSIcodepage", 0);
		gcd->pszID = mir_t2a(pszID);
		gch->pszUID = mir_t2a(pszUID);
		gch->pszText = mir_t2a_cp(pszText, dwCP);
	}
	else {
		replaceStrT(gcd->ptszID, pszID);
		replaceStrT(gch->ptszUID, pszUID);
		replaceStrT(gch->ptszText, pszText);
	}
	gcd->iType = iType;
	gch->dwData = dwItem;
	gch->pDest = gcd;
	PostMessage(hwnd, GC_FIREHOOK, 0, (LPARAM)gch);
	return TRUE;
}

BOOL DoEventHook(const TCHAR* pszID, const char* pszModule, int iType, const TCHAR* pszUID, const TCHAR* pszText, DWORD dwItem)
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return FALSE;

	GCHOOK gch = {0};
	GCDEST gcd = {0};
	gcd.pszModule = (char*)pszModule;

	if (!(si->dwFlags & GC_UNICODE)) {
		DWORD dwCP = M.GetDword(si->hContact, "ANSIcodepage", 0);
		gcd.pszID = mir_t2a(pszID);
		gch.pszUID = mir_t2a(pszUID);
		gch.pszText = mir_t2a_cp(pszText, dwCP);
	}
	else {
		gcd.ptszID = mir_tstrdup(pszID);
		gch.ptszUID = mir_tstrdup(pszUID);
		gch.ptszText = mir_tstrdup(pszText);
	}
	gcd.iType = iType;
	gch.dwData = dwItem;
	gch.pDest = &gcd;
	NotifyEventHooks(hSendEvent, 0, (WPARAM)&gch);

	mir_free(gcd.pszID);
	mir_free(gch.ptszUID);
	mir_free(gch.ptszText);
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

TCHAR* a2tf(const TCHAR* str, int flags, DWORD cp)
{
	if (str == NULL)
		return NULL;

	if (flags & GC_UNICODE)
		return mir_tstrdup(str);

	if (cp == CP_UTF8)
		return mir_utf8decodeW((char*)str);

	if (cp == 0)
		cp = PluginConfig.m_LangPackCP;
	int cbLen = MultiByteToWideChar(cp, 0, (char*)str, -1, 0, 0);
	TCHAR *result = (TCHAR*)mir_alloc(sizeof(TCHAR) * (cbLen + 1));
	if (result == NULL)
		return NULL;

	MultiByteToWideChar(cp, 0, (char*)str, -1, result, cbLen);
	result[cbLen] = 0;
	return result;
}

/*
 * set all filters and notification config for a session
 * uses per channel mask + filterbits, default config as backup
 */

void Chat_SetFilters(SESSION_INFO *si)
{
	if (si == NULL)
		return;

	int i;
	DWORD dwFlags_default = M.GetDword("Chat", "FilterFlags", 0x03E0);
	DWORD dwFlags_local = db_get_dw(si->hContact, "Chat", "FilterFlags", 0x03E0);
	DWORD dwMask = db_get_dw(si->hContact, "Chat", "FilterMask", 0);

	si->iLogFilterFlags = dwFlags_default;
	for (int i=0; i < 32; i++)
		if (dwMask & (1 << i))
			si->iLogFilterFlags = (dwFlags_local & (1 << i) ? si->iLogFilterFlags | (1 << i) : si->iLogFilterFlags & ~(1 << i));

	dwFlags_default = M.GetDword("Chat", "PopupFlags", 0x03E0);
	dwFlags_local = db_get_dw(si->hContact, "Chat", "PopupFlags", 0x03E0);
	dwMask = db_get_dw(si->hContact, "Chat", "PopupMask", 0);

	si->iLogPopupFlags = dwFlags_default;
	for (i=0; i < 32; i++)
		if (dwMask & (1 << i))
			si->iLogPopupFlags = (dwFlags_local & (1 << i) ? si->iLogPopupFlags | (1 << i) : si->iLogPopupFlags & ~(1 << i));

	dwFlags_default = M.GetDword("Chat", "TrayIconFlags", 0x03E0);
	dwFlags_local = db_get_dw(si->hContact, "Chat", "TrayIconFlags", 0x03E0);
	dwMask = db_get_dw(si->hContact, "Chat", "TrayIconMask", 0);

	si->iLogTrayFlags = dwFlags_default;
	for (i=0; i < 32; i++)
		if (dwMask & (1 << i))
			si->iLogTrayFlags = (dwFlags_local & (1 << i) ? si->iLogTrayFlags | (1 << i) : si->iLogTrayFlags & ~(1 << i));

	dwFlags_default = M.GetDword("Chat", "DiskLogFlags", 0xFFFF);
	si->iDiskLogFlags = dwFlags_default;

	if (si->iLogFilterFlags == 0)
		si->bFilterEnabled = 0;
}

static TCHAR tszOldTimeStamp[30] = _T("\0");

TCHAR* GetChatLogsFilename(SESSION_INFO *si, time_t tTime)
{
	if (!tTime)
		time(&tTime);

	/*
	 * check whether relevant parts of the timestamp have changed and
	 * we have to reparse the filename
	 */

	TCHAR *tszNow = MakeTimeStamp(_T("%a%d%m%Y"), tTime);

	bool fReparse = false;
	if (_tcscmp(tszOldTimeStamp, tszNow)) {
		_tcsncpy(tszOldTimeStamp, tszNow, 30);
		tszOldTimeStamp[29] = 0;
		fReparse = true;
	}

	if (fReparse || 0 == si->pszLogFileName[0]) {
		REPLACEVARSARRAY rva[11];
		rva[0].lptzKey = _T("d");
		rva[0].lptzValue = mir_tstrdup(MakeTimeStamp(_T("%#d"), tTime));
		// day 01-31
		rva[1].lptzKey = _T("dd");
		rva[1].lptzValue = mir_tstrdup(MakeTimeStamp(_T("%d"), tTime));
		// month 1-12
		rva[2].lptzKey = _T("m");
		rva[2].lptzValue = mir_tstrdup(MakeTimeStamp(_T("%#m"), tTime));
		// month 01-12
		rva[3].lptzKey = _T("mm");
		rva[3].lptzValue = mir_tstrdup(MakeTimeStamp(_T("%m"), tTime));
		// month text short
		rva[4].lptzKey = _T("mon");
		rva[4].lptzValue = mir_tstrdup(MakeTimeStamp(_T("%b"), tTime));
		// month text
		rva[5].lptzKey = _T("month");
		rva[5].lptzValue = mir_tstrdup(MakeTimeStamp(_T("%B"), tTime));
		// year 01-99
		rva[6].lptzKey = _T("yy");
		rva[6].lptzValue = mir_tstrdup(MakeTimeStamp(_T("%y"), tTime));
		// year 1901-9999
		rva[7].lptzKey = _T("yyyy");
		rva[7].lptzValue = mir_tstrdup(MakeTimeStamp(_T("%Y"), tTime));
		// weekday short
		rva[8].lptzKey = _T("wday");
		rva[8].lptzValue = mir_tstrdup(MakeTimeStamp(_T("%a"), tTime));
		// weekday
		rva[9].lptzKey = _T("weekday");
		rva[9].lptzValue = mir_tstrdup(MakeTimeStamp(_T("%A"), tTime));
		// end of array
		rva[10].lptzKey = NULL;
		rva[10].lptzValue = NULL;

		if (g_Settings.pszLogDir[lstrlen(g_Settings.pszLogDir)-1] == '\\')
			_tcscat(g_Settings.pszLogDir, _T("%userid%.log"));

		REPLACEVARSDATA dat = { sizeof(dat) };
		dat.dwFlags = RVF_TCHAR;
		dat.hContact = si->hContact;
		dat.variables = rva;
		TCHAR *tszParsedName = (TCHAR*)CallService(MS_UTILS_REPLACEVARS, (WPARAM)g_Settings.pszLogDir, (LPARAM)&dat);

		if (!M.pathIsAbsolute(tszParsedName))
			mir_sntprintf(si->pszLogFileName, MAX_PATH, _T("%s%s"), M.getChatLogPath(), tszParsedName);
		else
			mir_sntprintf(si->pszLogFileName, MAX_PATH, _T("%s"), tszParsedName);

		mir_free(tszParsedName);

		for (int i=0; i < SIZEOF(rva);i++)
			mir_free(rva[i].lptzValue);

		for (TCHAR *p = si->pszLogFileName + 2; *p; ++p)
			if (*p == ':' || *p == '*' || *p == '?' || *p == '"' || *p == '<' || *p == '>' || *p == '|' )
				*p = _T('_');
	}

	return si->pszLogFileName;
}
