/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-15 Miranda NG project,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// Helper functions for the group chat module.

#include "..\commonheaders.h"

int GetRichTextLength(HWND hwnd)
{
	GETTEXTLENGTHEX gtl;
	gtl.flags = GTL_PRECISE;
	gtl.codepage = CP_ACP;
	return (int)SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
}

static void __stdcall ShowRoomFromPopup(void *pi)
{
	SESSION_INFO *si = (SESSION_INFO*)pi;
	ShowRoom(si, WINDOW_VISIBLE, TRUE);
}

static void __stdcall Chat_DismissPopup(void *pi)
{
	SESSION_INFO *si = (SESSION_INFO*)pi;
	if (si->hContact)
		if (CallService(MS_CLIST_GETEVENT, si->hContact, 0))
			CallServiceSync(MS_CLIST_REMOVEEVENT, si->hContact, (LPARAM)GC_FAKE_EVENT);

	if (si->hWnd && KillTimer(si->hWnd, TIMERID_FLASHWND))
		FlashWindow(si->hWnd, FALSE);
}

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	SESSION_INFO *si = (SESSION_INFO*)PUGetPluginData(hWnd);

	switch (message) {
	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED) {
			if (si) {
				if (nen_options.maskActL & MASK_OPEN)
					CallFunctionAsync(ShowRoomFromPopup, si);
				else
					CallFunctionAsync(Chat_DismissPopup, si);
				PUDeletePopup(hWnd);
			}
			return TRUE;
		}
		break;

	case WM_CONTEXTMENU:
		if (si && si->hContact) {
			if (nen_options.maskActR & MASK_OPEN)
				CallFunctionAsync(ShowRoomFromPopup, si);
			else
				CallFunctionAsync(Chat_DismissPopup, si);
			PUDeletePopup(hWnd);
		}
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

BOOL DoTrayIcon(SESSION_INFO *si, GCEVENT *gce)
{
	int iEvent = gce->pDest->iType;
	if (si && (iEvent & si->iLogTrayFlags))
		return saveCI.DoTrayIcon(si, gce);
	return TRUE;
}

int ShowPopup(MCONTACT hContact, SESSION_INFO *si, HICON hIcon, char* pszProtoName, TCHAR*, COLORREF crBkg, const TCHAR* fmt, ...)
{
	POPUPDATAT pd = { 0 };
	va_list marker;
	static TCHAR szBuf[4 * 1024];

	if (!fmt || mir_tstrlen(fmt) == 0 || mir_tstrlen(fmt) > 2000)
		return 0;

	va_start(marker, fmt);
	mir_vsntprintf(szBuf, SIZEOF(szBuf), fmt, marker);
	va_end(marker);

	pd.lchContact = hContact;

	if (hIcon)
		pd.lchIcon = hIcon;
	else
		pd.lchIcon = LoadIconEx("window");

	PROTOACCOUNT *pa = ProtoGetAccount(pszProtoName);
	mir_sntprintf(pd.lptzContactName, SIZEOF(pd.lptzContactName), _T("%s - %s"),
		(pa == NULL) ? _A2T(pszProtoName) : pa->tszAccountName,
		pcli->pfnGetContactDisplayName(hContact, 0));

	_tcsncpy_s(pd.lptzText, TranslateTS(szBuf), _TRUNCATE);
	pd.iSeconds = g_Settings.iPopupTimeout;

	if (g_Settings.iPopupStyle == 2) {
		pd.colorBack = 0;
		pd.colorText = 0;
	}
	else if (g_Settings.iPopupStyle == 3) {
		pd.colorBack = g_Settings.crPUBkgColour;
		pd.colorText = g_Settings.crPUTextColour;
	}
	else {
		pd.colorBack = g_Settings.crLogBackground;
		pd.colorText = crBkg;
	}

	pd.PluginWindowProc = PopupDlgProc;
	pd.PluginData = si;
	return PUAddPopupT(&pd);
}

BOOL DoPopup(SESSION_INFO *si, GCEVENT* gce)
{
	int iEvent = gce->pDest->iType;
	if (si == NULL || !(iEvent & si->iLogPopupFlags))
		return true;

	TWindowData *dat = si->dat;
	TContainerData *pContainer = dat ? dat->pContainer : NULL;

	TCHAR *bbStart, *bbEnd;
	if (g_Settings.bBBCodeInPopups) {
		bbStart = _T("[b]");
		bbEnd = _T("[/b]");
	}
	else bbStart = bbEnd = _T("");

	if (nen_options.iMUCDisable)                          // no popups at all. Period
		return 0;
	/*
	* check the status mode against the status mask
	*/

	char *szProto = dat ? dat->szProto : si->pszModule;
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
			if (!PluginConfig.m_bHideOnClose)
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
		clr = pci->aFonts[16].color;
		iNewEvent &= ~GC_EVENT_HIGHLIGHT;
	}

	if (iNewEvent == GC_EVENT_MESSAGE) {
		ShowPopup(si->hContact, si, pci->hIcons[ICON_MESSAGE], si->pszModule, si->ptszName, clr ? clr : pci->aFonts[9].color,
			TranslateT("%s%s says:%s %s"), bbStart, gce->ptszNick, bbEnd, pci->RemoveFormatting(gce->ptszText));
	}
	else saveCI.DoPopup(si, gce);

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
		BOOL bForcedIcon = (p->hNotifyIcon == pci->hIcons[ICON_HIGHLIGHT] || p->hNotifyIcon == pci->hIcons[ICON_MESSAGE]);

		if ((p->iEvent & si->iLogTrayFlags) || bForcedIcon) {
			if (!p->bActiveTab) {
				if (p->hNotifyIcon == pci->hIcons[ICON_HIGHLIGHT])
					dat->iFlashIcon = p->hNotifyIcon;
				else {
					if (dat->iFlashIcon != pci->hIcons[ICON_HIGHLIGHT] && dat->iFlashIcon != pci->hIcons[ICON_MESSAGE])
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
					SendMessage(dat->pContainer->hwnd, DM_UPDATETITLE, dat->hContact, 0);
					dat->pContainer->dwFlags |= CNT_DEFERREDTABSELECT;
				}
			}
		}

		// flash window if it is not focused
		if (p->bMustFlash && p->bInactive)
			if (!(dat->pContainer->dwFlags & CNT_NOFLASH))
				FlashContainer(dat->pContainer, 1, 0);

		if (p->hNotifyIcon && p->bInactive && ((p->iEvent & si->iLogTrayFlags) || bForcedIcon)) {
			if (p->bMustFlash)
				dat->hTabIcon = p->hNotifyIcon;
			else if (dat->iFlashIcon) {
				dat->hTabIcon = dat->iFlashIcon;

				TCITEM item = { 0 };
				item.mask = TCIF_IMAGE;
				item.iImage = 0;
				TabCtrl_SetItem(GetParent(si->hWnd), dat->iTabID, &item);
			}

			HICON hIcon = (HICON)SendMessage(dat->pContainer->hwnd, WM_GETICON, ICON_BIG, 0);
			if (p->hNotifyIcon == pci->hIcons[ICON_HIGHLIGHT] || (hIcon != pci->hIcons[ICON_MESSAGE] && hIcon != pci->hIcons[ICON_HIGHLIGHT])) {
				SendMessage(dat->pContainer->hwnd, DM_SETICON, (WPARAM)dat, (LPARAM)p->hNotifyIcon);
				dat->pContainer->dwFlags |= CNT_NEED_UPDATETITLE;
			}
		}

		if (p->bMustFlash && p->bInactive)
			UpdateTrayMenu(dat, si->wStatus, si->pszModule, dat->szStatus, si->hContact, 1);
	}

	mir_free(p);
}

BOOL DoSoundsFlashPopupTrayStuff(SESSION_INFO *si, GCEVENT *gce, BOOL bHighlight, int bManyFix)
{
	if (gce == 0 || si == 0 || gce->bIsMe || si->iType == GCW_SERVER)
		return FALSE;

	TWindowData *dat = NULL;
	FLASH_PARAMS *params = (FLASH_PARAMS*)mir_calloc(sizeof(FLASH_PARAMS));
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

		if (g_Settings.bCreateWindowOnHighlight && dat == NULL)
			wParamForHighLight = 1;

		if (dat && g_Settings.bAnnoyingHighlight && params->bInactive && dat->pContainer->hwnd != GetForegroundWindow()) {
			wParamForHighLight = 2;
			params->hWnd = dat->hwnd;
		}

		if (dat || !nen_options.iMUCDisable)
			DoPopup(si, gce);
		if (params->bInactive && si && si->hWnd)
			SendMessage(si->hWnd, GC_SETMESSAGEHIGHLIGHT, 0, (LPARAM)si);
		if (g_Settings.bFlashWindowHighlight && params->bInactive)
			params->bMustFlash = TRUE;
		params->bMustAutoswitch = TRUE;
		params->hNotifyIcon = pci->hIcons[ICON_HIGHLIGHT];
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
				DoPopup(si, gce);

			// do sounds and flashing
			switch (params->iEvent) {
			case GC_EVENT_JOIN:
				params->sound = "ChatJoin";
				if (params->bInactive)
					params->hNotifyIcon = pci->hIcons[ICON_JOIN];
				break;
			case GC_EVENT_PART:
				params->sound = "ChatPart";
				if (params->bInactive)
					params->hNotifyIcon = pci->hIcons[ICON_PART];
				break;
			case GC_EVENT_QUIT:
				params->sound = "ChatQuit";
				if (params->bInactive)
					params->hNotifyIcon = pci->hIcons[ICON_QUIT];
				break;
			case GC_EVENT_ADDSTATUS:
			case GC_EVENT_REMOVESTATUS:
				params->sound = "ChatMode";
				if (params->bInactive)
					params->hNotifyIcon = pci->hIcons[params->iEvent == GC_EVENT_ADDSTATUS ? ICON_ADDSTATUS : ICON_REMSTATUS];
				break;
			case GC_EVENT_KICK:
				params->sound = "ChatKick";
				if (params->bInactive)
					params->hNotifyIcon = pci->hIcons[ICON_KICK];
				break;
			case GC_EVENT_MESSAGE:
				params->sound = "ChatMessage";
				if (params->bInactive && !(si->wState & STATE_TALK)) {
					si->wState |= STATE_TALK;
					db_set_w(si->hContact, si->pszModule, "ApparentMode", ID_STATUS_OFFLINE);
				}
				break;
			case GC_EVENT_ACTION:
				params->sound = "ChatAction";
				if (params->bInactive)
					params->hNotifyIcon = pci->hIcons[ICON_ACTION];
				break;
			case GC_EVENT_NICK:
				params->sound = "ChatNick";
				if (params->bInactive)
					params->hNotifyIcon = pci->hIcons[ICON_NICK];
				break;
			case GC_EVENT_NOTICE:
				params->sound = "ChatNotice";
				if (params->bInactive)
					params->hNotifyIcon = pci->hIcons[ICON_NOTICE];
				break;
			case GC_EVENT_TOPIC:
				params->sound = "ChatTopic";
				if (params->bInactive)
					params->hNotifyIcon = pci->hIcons[ICON_TOPIC];
				break;
			}
		}
		else {
			switch (params->iEvent) {
			case GC_EVENT_JOIN:
				params->hNotifyIcon = pci->hIcons[ICON_JOIN];
				break;
			case GC_EVENT_PART:
				params->hNotifyIcon = pci->hIcons[ICON_PART];
				break;
			case GC_EVENT_QUIT:
				params->hNotifyIcon = pci->hIcons[ICON_QUIT];
				break;
			case GC_EVENT_KICK:
				params->hNotifyIcon = pci->hIcons[ICON_KICK];
				break;
			case GC_EVENT_ACTION:
				params->hNotifyIcon = pci->hIcons[ICON_ACTION];
				break;
			case GC_EVENT_NICK:
				params->hNotifyIcon = pci->hIcons[ICON_NICK];
				break;
			case GC_EVENT_NOTICE:
				params->hNotifyIcon = pci->hIcons[ICON_NOTICE];
				break;
			case GC_EVENT_TOPIC:
				params->hNotifyIcon = pci->hIcons[ICON_TOPIC];
				break;
			case GC_EVENT_ADDSTATUS:
				params->hNotifyIcon = pci->hIcons[ICON_ADDSTATUS];
				break;
			case GC_EVENT_REMOVESTATUS:
				params->hNotifyIcon = pci->hIcons[ICON_REMSTATUS];
				break;
			}
		}

		if (params->iEvent == GC_EVENT_MESSAGE) {
			params->bMustAutoswitch = TRUE;
			if (g_Settings.bFlashWindow)
				params->bMustFlash = TRUE;
			params->hNotifyIcon = pci->hIcons[ICON_MESSAGE];
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
	MODULEINFO *pMod = pci->MM_FindModule(pszModule);
	if (!pMod || pMod->nColorCount == 0)
		return -1;

	for (int i = 0; i < pMod->nColorCount; i++)
		if (pMod->crColors[i] == cr)
			return i;

	return -1;
}

TCHAR* my_strstri(const TCHAR* s1, const TCHAR* s2)
{
	int i, j, k;

	_tsetlocale(LC_ALL, _T(""));
	for (i = 0; s1[i]; i++)
		for (j = i, k = 0; _totlower(s1[j]) == _totlower(s2[k]); j++, k++)
			if (!s2[k + 1])
				return (TCHAR*)(s1 + i);

	return NULL;
}

/*
* log the event to the log file
* allows selective logging of wanted events
*/

BOOL LogToFile(SESSION_INFO *si, GCEVENT *gce)
{
	if (!si || !gce)
		return FALSE;

	/*
	* check whether we have to log this event
	*/
	if (!(gce->pDest->iType & si->iDiskLogFlags))
		return FALSE;

	return saveCI.LogToFile(si, gce); // call kernel method
}

UINT CreateGCMenu(HWND hwndDlg, HMENU *hMenu, int iIndex, POINT pt, SESSION_INFO *si, TCHAR* pszUID, TCHAR* pszWordText)
{
	HMENU hSubMenu = 0;

	*hMenu = GetSubMenu(g_hMenu, iIndex);
	TranslateMenu(*hMenu);

	GCMENUITEMS gcmi = { 0 };
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
			mir_sntprintf(szMenuText, SIZEOF(szMenuText), TranslateT("Look up '%s':"), pszWordText);
			ModifyMenu(*hMenu, 4, MF_STRING | MF_BYPOSITION, 4, szMenuText);
		}
		else ModifyMenu(*hMenu, 4, MF_STRING | MF_GRAYED | MF_BYPOSITION, 4, TranslateT("No word to look up"));
		gcmi.Type = MENU_ON_LOG;
	}
	else if (iIndex == 0) {
		TCHAR szTemp[50];
		if (pszWordText)
			mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("&Message %s"), pszWordText);
		else
			_tcsncpy_s(szTemp, TranslateT("&Message"), _TRUNCATE);

		if (mir_tstrlen(szTemp) > 40)
			_tcsncpy_s(szTemp + 40, 4, _T("..."), _TRUNCATE);
		ModifyMenu(*hMenu, ID_MESS, MF_STRING | MF_BYCOMMAND, ID_MESS, szTemp);
		gcmi.Type = MENU_ON_NICKLIST;
	}

	NotifyEventHooks(pci->hBuildMenuEvent, 0, (WPARAM)&gcmi);

	if (gcmi.nItems > 0)
		AppendMenu(*hMenu, MF_SEPARATOR, 0, 0);

	for (int i = 0; i < gcmi.nItems; i++) {
		TCHAR *ptszText = TranslateTS(gcmi.Item[i].pszDesc);
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
	}

	if (iIndex == 0) {
		AppendMenu(*hMenu, MF_SEPARATOR, 0, 0);
		int pos = GetMenuItemCount(*hMenu);
		InsertMenu(*hMenu, pos, MF_BYPOSITION, (UINT_PTR)20020, TranslateT("Highlight user..."));
		InsertMenu(*hMenu, pos, MF_BYPOSITION, (UINT_PTR)20021, TranslateT("Edit highlight list..."));
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

/*
 * set all filters and notification config for a session
 * uses per channel mask + filterbits, default config as backup
 */

void Chat_SetFilters(SESSION_INFO *si)
{
	if (si == NULL)
		return;

	DWORD dwFlags_default = M.GetDword(CHAT_MODULE, "FilterFlags", 0x03E0);
	DWORD dwFlags_local = db_get_dw(si->hContact, CHAT_MODULE, "FilterFlags", 0x03E0);
	DWORD dwMask = db_get_dw(si->hContact, CHAT_MODULE, "FilterMask", 0);

	si->iLogFilterFlags = dwFlags_default;
	for (int i = 0; i < 32; i++) {
		DWORD dwBit = 1 << i;
		if (dwMask & dwBit)
			si->iLogFilterFlags = (dwFlags_local & dwBit) ? si->iLogFilterFlags | dwBit : si->iLogFilterFlags & ~dwBit;
	}

	dwFlags_default = M.GetDword(CHAT_MODULE, "PopupFlags", 0x03E0);
	dwFlags_local = db_get_dw(si->hContact, CHAT_MODULE, "PopupFlags", 0x03E0);
	dwMask = db_get_dw(si->hContact, CHAT_MODULE, "PopupMask", 0);

	si->iLogPopupFlags = dwFlags_default;
	for (int i = 0; i < 32; i++) {
		DWORD dwBit = 1 << i;
		if (dwMask & dwBit)
			si->iLogPopupFlags = (dwFlags_local & dwBit) ? si->iLogPopupFlags | dwBit : si->iLogPopupFlags & ~dwBit;
	}

	dwFlags_default = M.GetDword(CHAT_MODULE, "TrayIconFlags", 0x03E0);
	dwFlags_local = db_get_dw(si->hContact, CHAT_MODULE, "TrayIconFlags", 0x03E0);
	dwMask = db_get_dw(si->hContact, CHAT_MODULE, "TrayIconMask", 0);

	si->iDiskLogFlags = M.GetDword(CHAT_MODULE, "DiskLogFlags", 0xFFFF);

	si->iLogTrayFlags = dwFlags_default;
	for (int i = 0; i < 32; i++) {
		DWORD dwBit = 1 << i;
		if (dwMask & dwBit)
			si->iLogTrayFlags = (dwFlags_local & dwBit) ? si->iLogTrayFlags | dwBit : si->iLogTrayFlags & ~dwBit;
	}

	if (si->iLogFilterFlags == 0)
		si->bFilterEnabled = 0;
}

char GetIndicator(SESSION_INFO *si, LPCTSTR ptszNick, int *iNickIndex)
{
	if (iNickIndex)
		*iNickIndex = 0;

	for (USERINFO *ui = si->pUsers; ui; ui = ui->next) {
		if (!mir_tstrcmp(ui->pszNick, ptszNick)) {
			STATUSINFO *ti = pci->TM_FindStatus(si->pStatuses, pci->TM_WordToString(si->pStatuses, ui->Status));
			if (ti && (int)ti->hIcon < si->iStatusCount) {
				if (iNickIndex)
					*iNickIndex = (int)ti->hIcon; // color table's index is not zero-based
				return szIndicators[(int)ti->hIcon];
			}
			break;
		}
	}
	return 0;
}

BOOL IsHighlighted(SESSION_INFO *si, GCEVENT *gce)
{
	if (!g_Settings.bHighlightEnabled || !gce || gce->bIsMe)
		return FALSE;

	GCEVENT evTmp = *gce;

	int dwMask = 0;
	if (gce->ptszText != NULL)
		dwMask |= CMUCHighlight::MATCH_TEXT;

	if (gce->ptszNick != NULL) {
		dwMask |= CMUCHighlight::MATCH_NICKNAME;
		if (si && g_Settings.bLogClassicIndicators) {
			size_t len = mir_tstrlen(gce->ptszNick) + 1;
			TCHAR *tmp = (TCHAR*)_alloca(sizeof(TCHAR)*(len + 1));
			*tmp = GetIndicator(si, gce->ptszNick, 0);
			_tcscpy(tmp + 1, gce->ptszNick);
			evTmp.ptszNick = tmp;
		}
	}
	return g_Settings.Highlight->match(&evTmp, si, dwMask);
}
