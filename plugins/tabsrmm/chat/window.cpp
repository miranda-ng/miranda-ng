/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
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
 * original chat module for Miranda IM, written and copyrighted
 * by Joergen Persson in 2005.
 *
 * (C) 2005-2009 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: window.cpp 13631 2011-04-24 08:44:57Z silvercircle $
 *
 * This implements the group chat dialog window
 *
 */

#include "../src/commonheaders.h"

#include <tom.h>

//#include "../m_MathModule.h"

// externs...
extern LRESULT CALLBACK SplitterSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern HRESULT(WINAPI *MyCloseThemeData)(HANDLE);
extern REOLECallback *mREOLECallback;

int g_cLinesPerPage=0;
int g_iWheelCarryover=0;
//

extern HBRUSH		hListBkgBrush;
extern HANDLE		hSendEvent;
extern HICON		hIcons[30];
extern HMENU		g_hMenu;
extern WNDPROC 		OldSplitterProc;

static WNDPROC OldMessageProc;
static WNDPROC OldNicklistProc;
static WNDPROC OldFilterButtonProc;
static WNDPROC OldLogProc;
static HKL hkl = NULL;
static HCURSOR hCurHyperlinkHand;

typedef struct
{
	time_t lastEnterTime;
	TCHAR*  szSearchQuery;
	TCHAR*  szSearchResult;
	SESSION_INFO *lastSession;
	TCHAR  szTabSave[20];
	BOOL   iSavedSpaces;
} MESSAGESUBDATA;

static const CLSID IID_ITextDocument= { 0x8CC497C0,0xA1DF,0x11CE, { 0x80,0x98, 0x00,0xAA, 0x00,0x47,0xBE,0x5D} };
extern WNDPROC OldIEViewProc;

static void Chat_SetMessageLog(TWindowData *dat)
{
	unsigned int iLogMode = M->GetByte("Chat", "useIEView", 0);

	if (iLogMode == WANT_IEVIEW_LOG && dat->hwndIEView == 0) {
		IEVIEWWINDOW ieWindow;
		IEVIEWEVENT iee;

		//CheckAndDestroyHPP(dat);
		ZeroMemory(&ieWindow, sizeof(ieWindow));
		ZeroMemory(&iee, sizeof(iee));
		ieWindow.cbSize = sizeof(ieWindow);
		ieWindow.iType = IEW_CREATE;
		ieWindow.dwFlags = 0;
		ieWindow.dwMode = IEWM_TABSRMM;
		ieWindow.parent = dat->hwnd;
		ieWindow.x = 0;
		ieWindow.y = 0;
		ieWindow.cx = 200;
		ieWindow.cy = 300;
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
		dat->hwndIEView = ieWindow.hwnd;
		ZeroMemory(&iee, sizeof(iee));
		iee.cbSize = sizeof(iee);
		iee.iType = IEE_CLEAR_LOG;
		iee.hwnd = dat->hwndIEView;
		iee.hContact = dat->hContact;
		iee.codepage = dat->codePage;

		SESSION_INFO *si = (SESSION_INFO *)dat->si;

		iee.pszProto = si->pszModule;
		CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&iee);

		Utils::showDlgControl(dat->hwnd, IDC_CHAT_LOG, SW_HIDE);
		Utils::enableDlgControl(dat->hwnd, IDC_CHAT_LOG, FALSE);
	} else if (iLogMode == WANT_HPP_LOG && dat->hwndHPP == 0) {
		IEVIEWWINDOW ieWindow;

		ZeroMemory(&ieWindow, sizeof(ieWindow));
		//CheckAndDestroyIEView(dat);
		ieWindow.cbSize = sizeof(IEVIEWWINDOW);
		ieWindow.iType = IEW_CREATE;
		ieWindow.dwFlags = 0;
		ieWindow.dwMode = IEWM_MUCC;
		ieWindow.parent = dat->hwnd;
		ieWindow.x = 0;
		ieWindow.y = 0;
		ieWindow.cx = 10;
		ieWindow.cy = 10;
		CallService(MS_HPP_EG_WINDOW, 0, (LPARAM)&ieWindow);
		dat->hwndHPP = ieWindow.hwnd;
		Utils::showDlgControl(dat->hwnd, IDC_CHAT_LOG, SW_HIDE);
		Utils::enableDlgControl(dat->hwnd, IDC_CHAT_LOG, FALSE);
	} else {
		if (iLogMode != WANT_IEVIEW_LOG)
			CheckAndDestroyIEView(dat);
		Utils::showDlgControl(dat->hwnd, IDC_CHAT_LOG, SW_SHOW);
		Utils::enableDlgControl(dat->hwnd, IDC_CHAT_LOG, TRUE);
		dat->hwndIEView = 0;
		dat->hwndIWebBrowserControl = 0;
		dat->hwndHPP = 0;
	}
}


/*
 * checking if theres's protected text at the point
 * emulates EN_LINK WM_NOTIFY to parent to process links
 */
static BOOL CheckCustomLink(HWND hwndDlg, POINT* ptClient, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL bUrlNeeded)
{
	long res = 0, cnt = 0;
	long cpMin = 0, cpMax = 0;
	POINT ptEnd = {0};
	IRichEditOle* RichEditOle = NULL;
	ITextDocument* TextDocument = NULL;
	ITextRange* TextRange = NULL;
	ITextFont* TextFont = NULL;
	BOOL bIsCustomLink = FALSE;

	POINT pt = *ptClient;
	ClientToScreen(hwndDlg, &pt);

	do  {
		if (!SendMessage(hwndDlg, EM_GETOLEINTERFACE, 0, (LPARAM)&RichEditOle)) break;
		if (RichEditOle->QueryInterface(IID_ITextDocument, (void**)&TextDocument) != S_OK) break;
		if (TextDocument->RangeFromPoint(pt.x, pt.y, &TextRange) != S_OK) break;

		TextRange->GetStart(&cpMin);
		cpMax = cpMin+1;
		TextRange->SetEnd(cpMax);

		if (TextRange->GetFont(&TextFont) != S_OK)
			break;

		TextFont->GetProtected(&res);
		if (res != tomTrue)
			break;

		TextRange->GetPoint(tomEnd+TA_BOTTOM+TA_RIGHT, &ptEnd.x, &ptEnd.y);
		if (pt.x > ptEnd.x || pt.y > ptEnd.y)
			break;

		if (bUrlNeeded) {
			TextRange->GetStoryLength(&cnt);
			for (; cpMin > 0; cpMin--) {
				res = tomTrue;
				TextRange->SetIndex(tomCharacter, cpMin+1, tomTrue);
				TextFont->GetProtected(&res);
				if (res != tomTrue) { cpMin++; break; }
			}
			for (cpMax--; cpMax < cnt; cpMax++) {
				res = tomTrue;
				TextRange->SetIndex(tomCharacter, cpMax+1, tomTrue);
				TextFont->GetProtected(&res);
				if (res != tomTrue)
					break;
			}
		}

		bIsCustomLink = (cpMin < cpMax);
		} while(FALSE);

	if (TextFont) TextFont->Release();
	if (TextRange) TextRange->Release();
	if (TextDocument) TextDocument->Release();
	if (RichEditOle) RichEditOle->Release();

	if (bIsCustomLink) {
		ENLINK enlink = {0};
		enlink.nmhdr.hwndFrom = hwndDlg;
		enlink.nmhdr.idFrom = IDC_CHAT_LOG;
		enlink.nmhdr.code = EN_LINK;
		enlink.msg = uMsg;
		enlink.wParam = wParam;
		enlink.lParam = lParam;
		enlink.chrg.cpMin = cpMin;
		enlink.chrg.cpMax = cpMax;
		SendMessage(GetParent(hwndDlg), WM_NOTIFY, (WPARAM)IDC_CHAT_LOG, (LPARAM)&enlink);
	}
	return bIsCustomLink;
}

static BOOL IsStringValidLink(TCHAR* pszText)
{
	TCHAR *p = pszText;

	if (pszText == NULL)
		return FALSE;
	if (lstrlen(pszText) < 5)
		return FALSE;

	while (*p) {
		if (*p == (TCHAR)'"')
			return FALSE;
		p++;
	}
	if (_totlower(pszText[0]) == 'w' && _totlower(pszText[1]) == 'w' && _totlower(pszText[2]) == 'w' && pszText[3] == '.' && _istalnum(pszText[4]))
		return TRUE;

	return(_tcsstr(pszText, _T("://")) == NULL ? FALSE : TRUE);
}

/*
 * called whenever a group chat tab becomes active (either by switching tabs or activating a
 * container window
 */

static void Chat_UpdateWindowState(TWindowData *dat, UINT msg)
{
	if (dat == NULL)
		return;

	HWND hwndDlg = dat->hwnd;
	HWND hwndTab = GetParent(hwndDlg);
	SESSION_INFO *si = (SESSION_INFO *)dat->si;

	if (msg == WM_ACTIVATE) {
		if (dat->pContainer->dwFlags & CNT_TRANSPARENCY && CMimAPI::m_pSetLayeredWindowAttributes != NULL) {
			DWORD trans = LOWORD(dat->pContainer->settings->dwTransparency);
			CMimAPI::m_pSetLayeredWindowAttributes(dat->pContainer->hwnd, CSkin::m_ContainerColorKey, (BYTE)trans, (dat->pContainer->dwFlags & CNT_TRANSPARENCY ? LWA_ALPHA : 0));
		}
	}

	if(si->hwndFilter) {
		POINT pt;
		RECT  rcFilter;

		GetCursorPos(&pt);
		GetWindowRect(si->hwndFilter, &rcFilter);
		if(!PtInRect(&rcFilter, pt)) {
			SendMessage(si->hwndFilter, WM_CLOSE, 1, 1);
			si->hwndFilter = 0;
		}
	}

#if defined(__FEAT_EXP_AUTOSPLITTER)
	if(dat->fIsAutosizingInput && dat->iInputAreaHeight == -1) {
		dat->iInputAreaHeight = 0;
		SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_REQUESTRESIZE, 0, 0);
	}
#endif
	dat->Panel->dismissConfig();
	dat->dwUnread = 0;
	if(dat->pWnd) {
		dat->pWnd->activateTab();
		dat->pWnd->setOverlayIcon(0, true);
	}

	if (dat->pContainer->hwndSaved == hwndDlg || dat->bWasDeleted)
		return;

	dat->pContainer->hwndSaved = hwndDlg;

	SetActiveSession(si->ptszID, si->pszModule);
	dat->hTabIcon = dat->hTabStatusIcon;

	if (dat->iTabID >= 0) {
		if (DBGetContactSettingWord(si->hContact, si->pszModule , "ApparentMode", 0) != 0)
			DBWriteContactSettingWord(si->hContact, si->pszModule , "ApparentMode", (LPARAM) 0);
		if (CallService(MS_CLIST_GETEVENT, (WPARAM)si->hContact, (LPARAM)0))
			CallService(MS_CLIST_REMOVEEVENT, (WPARAM)si->hContact, (LPARAM)szChatIconString);

		SendMessage(hwndDlg, GC_UPDATETITLE, 0, 1);
		dat->dwTickLastEvent = 0;
		dat->dwFlags &= ~MWF_DIVIDERSET;
		if (KillTimer(hwndDlg, TIMERID_FLASHWND) || dat->iFlashIcon) {
			FlashTab(dat, hwndTab, dat->iTabID, &dat->bTabFlash, FALSE, dat->hTabIcon);
			dat->mayFlashTab = FALSE;
			dat->iFlashIcon = 0;
		}
		if (dat->pContainer->dwFlashingStarted != 0) {
			FlashContainer(dat->pContainer, 0, 0);
			dat->pContainer->dwFlashingStarted = 0;
		}
		dat->pContainer->dwFlags &= ~CNT_NEED_UPDATETITLE;

		if (dat->dwFlags & MWF_NEEDCHECKSIZE)
			PostMessage(hwndDlg, DM_SAVESIZE, 0, 0);

		if (PluginConfig.m_AutoLocaleSupport) {
			if (dat->hkl == 0)
				DM_LoadLocale(dat);
			else
				SendMessage(hwndDlg, DM_SETLOCALE, 0, 0);
		}
		SetFocus(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));
		dat->dwLastActivity = GetTickCount();
		dat->pContainer->dwLastActivity = dat->dwLastActivity;
		dat->pContainer->MenuBar->configureMenu();
		UpdateTrayMenuState(dat, FALSE);
		DM_SetDBButtonStates(hwndDlg, dat);

		if (g_Settings.MathMod) {
			CallService(MTH_Set_ToolboxEditHwnd, 0, (LPARAM)GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));
			MTH_updateMathWindow(dat);
		}

		if (dat->dwFlagsEx & MWF_EX_DELAYEDSPLITTER) {
			dat->dwFlagsEx &= ~MWF_EX_DELAYEDSPLITTER;
			ShowWindow(dat->pContainer->hwnd, SW_RESTORE);
			PostMessage(hwndDlg, DM_SPLITTERGLOBALEVENT, dat->wParam, dat->lParam);
			PostMessage(hwndDlg, WM_SIZE, 0, 0);
			dat->wParam = dat->lParam = 0;
		}
		/*
		if (dat->hwndIEView) {
			RECT rcRTF;
			POINT pt;

			GetWindowRect(GetDlgItem(hwndDlg, IDC_CHAT_LOG), &rcRTF);
			rcRTF.left += 20;
			rcRTF.top += 20;
			pt.x = rcRTF.left;
			pt.y = rcRTF.top;
			if (dat->hwndIEView) {
				if (M->GetByte("subclassIEView", 0) && dat->oldIEViewProc == 0) {
					WNDPROC wndProc = (WNDPROC)SetWindowLongPtr(dat->hwndIEView, GWLP_WNDPROC, (LONG_PTR)IEViewSubclassProc);
					if (OldIEViewProc == 0)
						OldIEViewProc = wndProc;
					dat->oldIEViewProc = wndProc;
				}
			}
			dat->hwndIWebBrowserControl = WindowFromPoint(pt);
		}
		*/
	}
	if(M->isAero())
		InvalidateRect(hwndTab, NULL, FALSE);
	if(dat->pContainer->dwFlags & CNT_SIDEBAR)
		dat->pContainer->SideBar->setActiveItem(dat);
	BB_SetButtonsPos(dat);

	if(dat->pWnd)
		dat->pWnd->Invalidate();
}

/*
 * initialize button bar, set all the icons and ensure proper button state
 */

static void	InitButtons(HWND hwndDlg, SESSION_INFO* si)
{
	BOOL isFlat = M->GetByte("tbflat", 1);
	BOOL isThemed = PluginConfig.m_bIsXP;
	MODULEINFO *pInfo = si ? MM_FindModule(si->pszModule) : NULL;
	BOOL bFilterEnabled = si ? si->bFilterEnabled : FALSE;

	int i = 0;

	if (pInfo) {
		Utils::enableDlgControl(hwndDlg, IDC_CHAT_BOLD, pInfo->bBold);
		Utils::enableDlgControl(hwndDlg, IDC_ITALICS, pInfo->bItalics);
		Utils::enableDlgControl(hwndDlg, IDC_CHAT_UNDERLINE, pInfo->bUnderline);
		Utils::enableDlgControl(hwndDlg, IDC_COLOR, pInfo->bColor);
		Utils::enableDlgControl(hwndDlg, IDC_BKGCOLOR, pInfo->bBkgColor);
		if (si->iType == GCW_CHATROOM)
			Utils::enableDlgControl(hwndDlg, IDC_CHANMGR, pInfo->bChanMgr);
	}
}

static void Chat_ResizeIeView(const TWindowData *dat)
{
	RECT 			rcRichEdit;
	POINT 			pt;
	IEVIEWWINDOW 	ieWindow;
	int 			iMode = dat->hwndIEView ? 1 : 2;
	HWND			hwndDlg = dat->hwnd;

	ZeroMemory(&ieWindow, sizeof(ieWindow));
	GetWindowRect(GetDlgItem(hwndDlg, IDC_CHAT_LOG), &rcRichEdit);
	pt.x = rcRichEdit.left;
	pt.y = rcRichEdit.top;
	ScreenToClient(hwndDlg, &pt);
	ieWindow.cbSize = sizeof(IEVIEWWINDOW);
	ieWindow.iType = IEW_SETPOS;
	ieWindow.parent = hwndDlg;
	ieWindow.hwnd = iMode == 1 ? dat->hwndIEView : dat->hwndHPP;
	ieWindow.x = pt.x;
	ieWindow.y = pt.y;
	ieWindow.cx = rcRichEdit.right - rcRichEdit.left;
	ieWindow.cy = rcRichEdit.bottom - rcRichEdit.top;
	if (ieWindow.cx != 0 && ieWindow.cy != 0) {
		CallService(iMode == 1 ? MS_IEVIEW_WINDOW : MS_HPP_EG_WINDOW, 0, (LPARAM)&ieWindow);
	}
}

/*
 * resizer callback for the group chat session window. Called from Mirandas dialog
 * resizing service
 */

static int RoomWndResize(HWND hwndDlg, LPARAM lParam, UTILRESIZECONTROL *urc)
{
	RECT rc, rcTabs;
	SESSION_INFO* si = (SESSION_INFO*)lParam;
	struct      TWindowData *dat = (struct TWindowData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	int			TabHeight;
	BOOL		bToolbar = !(dat->pContainer->dwFlags & CNT_HIDETOOLBAR);
	BOOL		bBottomToolbar = dat->pContainer->dwFlags & CNT_BOTTOMTOOLBAR ? 1 : 0;
	int 		panelHeight = dat->Panel->getHeight() + 1;

	BOOL		bNick = si->iType != GCW_SERVER && si->bNicklistEnabled;
	int         i = 0;
	static      int msgBottom = 0, msgTop = 0;
	bool		fInfoPanel = dat->Panel->isActive();

	rc.bottom = rc.top = rc.left = rc.right = 0;

	GetClientRect(hwndDlg, &rcTabs);
	TabHeight = rcTabs.bottom - rcTabs.top;

	if(dat->fIsAutosizingInput)
		Utils::showDlgControl(hwndDlg, IDC_SPLITTERY, SW_HIDE);

	if (si->iType != GCW_SERVER) {
		Utils::showDlgControl(hwndDlg, IDC_LIST, si->bNicklistEnabled ? SW_SHOW : SW_HIDE);
		Utils::showDlgControl(hwndDlg, IDC_SPLITTERX, si->bNicklistEnabled ? SW_SHOW : SW_HIDE);

		Utils::enableDlgControl(hwndDlg, IDC_SHOWNICKLIST, TRUE);
		Utils::enableDlgControl(hwndDlg, IDC_FILTER, TRUE);
		if (si->iType == GCW_CHATROOM)	{
			MODULEINFO* tmp = MM_FindModule(si->pszModule);
			if (tmp)
				Utils::enableDlgControl(hwndDlg, IDC_CHANMGR, tmp->bChanMgr);
		}
	} else {
		Utils::showDlgControl(hwndDlg, IDC_LIST, SW_HIDE);
		Utils::showDlgControl(hwndDlg, IDC_SPLITTERX, SW_HIDE);
	}

	if (si->iType == GCW_SERVER) {
		Utils::enableDlgControl(hwndDlg, IDC_SHOWNICKLIST, FALSE);
		Utils::enableDlgControl(hwndDlg, IDC_FILTER, FALSE);
		Utils::enableDlgControl(hwndDlg, IDC_CHANMGR, FALSE);
	}
	//ShowWindow(GetDlgItem(hwndDlg, IDC_CHAT_TOGGLESIDEBAR), dat->pContainer->dwFlags & CNT_SIDEBAR ? SW_SHOW : SW_HIDE);

	switch (urc->wId) {
		case IDC_PANELSPLITTER:
			urc->rcItem.bottom = panelHeight;
			urc->rcItem.top = panelHeight - 2;
			return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;
		case IDC_CHAT_LOG:
			urc->rcItem.top = 0;
			urc->rcItem.left = 0;
			urc->rcItem.right = bNick ? urc->dlgNewSize.cx - si->iSplitterX : urc->dlgNewSize.cx;
			urc->rcItem.bottom = (bToolbar&&!bBottomToolbar) ? (urc->dlgNewSize.cy - si->iSplitterY - (PluginConfig.g_DPIscaleY > 1.0 ? DPISCALEY_S(24) : DPISCALEY_S(23))) : (urc->dlgNewSize.cy - si->iSplitterY - DPISCALEY_S(2));
			if (fInfoPanel)
				urc->rcItem.top += panelHeight;
			if (CSkin::m_skinEnabled) {
				CSkinItem *item = &SkinItems[ID_EXTBKHISTORY];
				if (!item->IGNORED) {
					urc->rcItem.left += item->MARGIN_LEFT;
					urc->rcItem.right -= item->MARGIN_RIGHT;
					urc->rcItem.top += item->MARGIN_TOP;
					urc->rcItem.bottom -= item->MARGIN_BOTTOM;
				}
			}
			return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

		case IDC_LIST:
			urc->rcItem.top = 0;
			urc->rcItem.right = urc->dlgNewSize.cx ;
			urc->rcItem.left = urc->dlgNewSize.cx - si->iSplitterX + 2;
			urc->rcItem.bottom = (bToolbar&&!bBottomToolbar) ? (urc->dlgNewSize.cy - si->iSplitterY - DPISCALEY_S(23)) : (urc->dlgNewSize.cy - si->iSplitterY - DPISCALEY_S(2));
			if (fInfoPanel)
				urc->rcItem.top += panelHeight;
			if (CSkin::m_skinEnabled) {
				CSkinItem *item = &SkinItems[ID_EXTBKUSERLIST];
				if (!item->IGNORED) {
					urc->rcItem.left += item->MARGIN_LEFT;
					urc->rcItem.right -= item->MARGIN_RIGHT;
					urc->rcItem.top += item->MARGIN_TOP;
					urc->rcItem.bottom -= item->MARGIN_BOTTOM;
				}
			}
			return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

		case IDC_SPLITTERX:
			urc->rcItem.right = urc->dlgNewSize.cx - si->iSplitterX + 2;
			urc->rcItem.left = urc->dlgNewSize.cx - si->iSplitterX;
			urc->rcItem.bottom = (bToolbar&&!bBottomToolbar) ? (urc->dlgNewSize.cy - si->iSplitterY - DPISCALEY_S(23)) : (urc->dlgNewSize.cy - si->iSplitterY - DPISCALEY_S(2));
			urc->rcItem.top = 0;
			if (fInfoPanel)
				urc->rcItem.top += panelHeight;
			return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

		case IDC_SPLITTERY:
			urc->rcItem.right = urc->dlgNewSize.cx;
			urc->rcItem.top = (bToolbar&&!bBottomToolbar) ? urc->dlgNewSize.cy - si->iSplitterY : urc->dlgNewSize.cy - si->iSplitterY;
			urc->rcItem.bottom = (bToolbar&&!bBottomToolbar) ? (urc->dlgNewSize.cy - si->iSplitterY + DPISCALEY_S(2)) : (urc->dlgNewSize.cy - si->iSplitterY + DPISCALEY_S(2));
			urc->rcItem.left = 0;
			urc->rcItem.bottom++;
			urc->rcItem.top++;
			return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

		case IDC_CHAT_MESSAGE:
			urc->rcItem.right = urc->dlgNewSize.cx ;
			urc->rcItem.top = urc->dlgNewSize.cy - si->iSplitterY + 3;
			urc->rcItem.bottom = urc->dlgNewSize.cy; // - 1 ;
			msgBottom = urc->rcItem.bottom;

			if(dat->fIsAutosizingInput)
				urc->rcItem.top -= DPISCALEY_S(1);

			msgTop = urc->rcItem.top;
			if (bBottomToolbar&&bToolbar)
				urc->rcItem.bottom -= DPISCALEY_S(22);
			if (CSkin::m_skinEnabled) {
				CSkinItem *item = &SkinItems[ID_EXTBKINPUTAREA];
				if (!item->IGNORED) {
					urc->rcItem.left += item->MARGIN_LEFT;
					urc->rcItem.right -= item->MARGIN_RIGHT;
					urc->rcItem.top += item->MARGIN_TOP;
					urc->rcItem.bottom -= item->MARGIN_BOTTOM;
				}
			}
			return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}


/*
 * subclassing for the message input control (a richedit text control)
 */

static LRESULT CALLBACK MessageSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MESSAGESUBDATA *dat;
	SESSION_INFO* Parentsi;
	struct TWindowData *mwdat;
	HWND hwndParent = GetParent(hwnd);

	mwdat = (struct TWindowData *)GetWindowLongPtr(hwndParent, GWLP_USERDATA);
	Parentsi = (SESSION_INFO *)mwdat->si;

	dat = (MESSAGESUBDATA *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if(mwdat->fkeyProcessed && (msg == WM_KEYUP)) {
		GetKeyboardState(mwdat->kstate);
		if(mwdat->kstate[VK_CONTROL] & 0x80 || mwdat->kstate[VK_SHIFT] & 0x80)
			return(0);
		else {
			mwdat->fkeyProcessed = false;
			return(0);
		}
	}
	switch (msg) {
		case WM_NCCALCSIZE:
			return(CSkin::NcCalcRichEditFrame(hwnd, mwdat, ID_EXTBKINPUTAREA, msg, wParam, lParam, OldMessageProc));

		case WM_NCPAINT:
			return(CSkin::DrawRichEditFrame(hwnd, mwdat, ID_EXTBKINPUTAREA, msg, wParam, lParam, OldMessageProc));

		case EM_SUBCLASSED:
			dat = (MESSAGESUBDATA *) mir_calloc(sizeof(MESSAGESUBDATA));
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) dat);
			return 0;

		case WM_CONTEXTMENU: {
			MODULEINFO* mi = MM_FindModule(Parentsi->pszModule);
			HMENU hMenu, hSubMenu;
			CHARRANGE sel, all = { 0, -1};
			int iSelection;
			int iPrivateBG = M->GetByte(mwdat->hContact, "private_bg", 0);
			MessageWindowPopupData mwpd;
			POINT pt;
			int idFrom = IDC_CHAT_MESSAGE;

			GetCursorPos(&pt);
			hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));
			hSubMenu = GetSubMenu(hMenu, 2);
			RemoveMenu(hSubMenu, 9, MF_BYPOSITION);
			RemoveMenu(hSubMenu, 8, MF_BYPOSITION);
			RemoveMenu(hSubMenu, 4, MF_BYPOSITION);
			EnableMenuItem(hSubMenu, IDM_PASTEFORMATTED, MF_BYCOMMAND | ((mi && mi->bBold) ? MF_ENABLED : MF_GRAYED));
			CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM) hSubMenu, 0);

			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM) & sel);
			if (sel.cpMin == sel.cpMax) {
				EnableMenuItem(hSubMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);
				if (idFrom == IDC_CHAT_MESSAGE)
					EnableMenuItem(hSubMenu, IDM_CUT, MF_BYCOMMAND | MF_GRAYED);
			}
			mwpd.cbSize = sizeof(mwpd);
			mwpd.uType = MSG_WINDOWPOPUP_SHOWING;
			mwpd.uFlags = (idFrom == IDC_LOG ? MSG_WINDOWPOPUP_LOG : MSG_WINDOWPOPUP_INPUT);
			mwpd.hContact = mwdat->hContact;
			mwpd.hwnd = hwnd;
			mwpd.hMenu = hSubMenu;
			mwpd.selection = 0;
			mwpd.pt = pt;
			NotifyEventHooks(PluginConfig.m_event_MsgPopup, 0, (LPARAM)&mwpd);

			iSelection = TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, GetParent(hwnd), NULL);

			mwpd.selection = iSelection;
			mwpd.uType = MSG_WINDOWPOPUP_SELECTED;
			NotifyEventHooks(PluginConfig.m_event_MsgPopup, 0, (LPARAM)&mwpd);

			switch (iSelection) {
				case IDM_COPY:
					SendMessage(hwnd, WM_COPY, 0, 0);
					break;
				case IDM_CUT:
					SendMessage(hwnd, WM_CUT, 0, 0);
					break;
				case IDM_PASTE:
				case IDM_PASTEFORMATTED:
					if (idFrom == IDC_CHAT_MESSAGE)
						SendMessage(hwnd, EM_PASTESPECIAL, (iSelection == IDM_PASTE) ? CF_TEXTT : 0, 0);
					break;
				case IDM_COPYALL:
					SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) & all);
					SendMessage(hwnd, WM_COPY, 0, 0);
					SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) & sel);
					break;
				case IDM_SELECTALL:
					SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) & all);
					break;
			}
			DestroyMenu(hMenu);
			return TRUE;
		}

		case WM_MOUSEWHEEL: {
			LRESULT result = DM_MouseWheelHandler(hwnd, hwndParent, mwdat, wParam, lParam);
			if (result == 0)
				return 0;

			dat->lastEnterTime = 0;
			break;
		}

		case WM_SYSKEYUP:
			if(wParam == VK_MENU) {
				ProcessHotkeysByMsgFilter(hwnd, msg, wParam, lParam, IDC_CHAT_MESSAGE);
				return(0);
			}
			break;

		case WM_SYSKEYDOWN:
			mwdat->fkeyProcessed = false;
			if(ProcessHotkeysByMsgFilter(hwnd, msg, wParam, lParam, IDC_CHAT_MESSAGE)) {
				mwdat->fkeyProcessed = true;
				return(0);
			}
			break;

		case WM_SYSCHAR: {
			if(mwdat->fkeyProcessed) {
				mwdat->fkeyProcessed = false;						// preceeding key event has been processed by miranda hotkey service
				return(0);
			}
			BOOL isMenu = GetKeyState(VK_MENU) & 0x8000;

			if ((wParam >= '0' && wParam <= '9') && isMenu) {       // ALT-1 -> ALT-0 direct tab selection
				BYTE bChar = (BYTE)wParam;
				int iIndex;

				if (bChar == '0')
					iIndex = 10;
				else
					iIndex = bChar - (BYTE)'0';
				SendMessage(mwdat->pContainer->hwnd, DM_SELECTTAB, DM_SELECT_BY_INDEX, (LPARAM)iIndex);
				return 0;
			}
			break;
		}

		case WM_CHAR: {
			BOOL isShift, isAlt, isCtrl;
			KbdState(mwdat, isShift, isCtrl, isAlt);

			//MAD: sound on typing..
			if(PluginConfig.g_bSoundOnTyping && !isAlt &&!isCtrl&&!(mwdat->pContainer->dwFlags&CNT_NOSOUND)&&wParam!=VK_ESCAPE&&!(wParam==VK_TAB&&PluginConfig.m_AllowTab))
				SkinPlaySound("SoundOnTyping");
			//MAD

			if (wParam == 0x0d && isCtrl && PluginConfig.m_MathModAvail) {
				TCHAR toInsert[100];
				BYTE keyState[256];
				size_t i;
				size_t iLen = lstrlen(PluginConfig.m_MathModStartDelimiter);
				ZeroMemory(keyState, 256);
				_tcsncpy(toInsert, PluginConfig.m_MathModStartDelimiter, 30);
				_tcsncat(toInsert, PluginConfig.m_MathModStartDelimiter, 30);
				SendMessage(hwnd, EM_REPLACESEL, TRUE, (LPARAM)toInsert);
				SetKeyboardState(keyState);
				for (i = 0; i < iLen; i++)
					SendMessage(hwnd, WM_KEYDOWN, mwdat->dwFlags & MWF_LOG_RTL ? VK_RIGHT : VK_LEFT, 0);
				return 0;
			}
			if(isCtrl && !isAlt && !isShift) {
				MODULEINFO* mi = MM_FindModule(Parentsi->pszModule);

				switch(wParam) {
					case 0x09: 		// ctrl-i (italics)
						if(mi && mi->bItalics) {
							CheckDlgButton(hwndParent, IDC_ITALICS, IsDlgButtonChecked(hwndParent, IDC_ITALICS) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
							SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(IDC_ITALICS, 0), 0);
						}
						return(0);
					case 0x02:		// ctrl-b (bold)
						if(mi && mi->bBold) {
							CheckDlgButton(hwndParent, IDC_CHAT_BOLD, IsDlgButtonChecked(hwndParent, IDC_CHAT_BOLD) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
							SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(IDC_CHAT_BOLD, 0), 0);
						}
						return 0;
					case 0x20:		// ctrl-space clear formatting
						if(mi && mi->bBold && mi->bItalics && mi->bUnderline) {
							CheckDlgButton(hwndParent, IDC_BKGCOLOR, BST_UNCHECKED);
							CheckDlgButton(hwndParent, IDC_COLOR, BST_UNCHECKED);
							CheckDlgButton(hwndParent, IDC_CHAT_BOLD, BST_UNCHECKED);
							CheckDlgButton(hwndParent, IDC_CHAT_UNDERLINE, BST_UNCHECKED);
							CheckDlgButton(hwndParent, IDC_ITALICS, BST_UNCHECKED);
							SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(IDC_BKGCOLOR, 0), 0);
							SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(IDC_COLOR, 0), 0);
							SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(IDC_CHAT_BOLD, 0), 0);
							SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(IDC_CHAT_UNDERLINE, 0), 0);
							SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(IDC_ITALICS, 0), 0);
						}
						return 0;
					case 0x0c:		// ctrl-l background color
						if(mi && mi->bBkgColor) {
							CheckDlgButton(hwndParent, IDC_BKGCOLOR, IsDlgButtonChecked(hwndParent, IDC_BKGCOLOR) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
							SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(IDC_BKGCOLOR, 0), 0);
						}
						return 0;
					case 0x15:		// ctrl-u underlined
						if(mi && mi->bUnderline) {
							CheckDlgButton(hwndParent, IDC_CHAT_UNDERLINE, IsDlgButtonChecked(hwndParent, IDC_CHAT_UNDERLINE) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
							SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(IDC_CHAT_UNDERLINE, 0), 0);
						}
						return 0;	// ctrl-k color
					case 0x0b:
						if(mi && mi->bColor) {
							CheckDlgButton(hwndParent, IDC_COLOR, IsDlgButtonChecked(hwndParent, IDC_COLOR) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
							SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(IDC_COLOR, 0), 0);
						}
						return 0;
					case 0x17:
						PostMessage(hwndParent, WM_CLOSE, 0, 1);
						return 0;
					default:
						break;
				}
			}
			break;
		}
		case WM_KEYDOWN: {
			static size_t start, end;
			BOOL isShift, isCtrl, isAlt;
			KbdState(mwdat, isShift, isCtrl, isAlt);

			//MAD: sound on typing..
			if(PluginConfig.g_bSoundOnTyping&&!isAlt&&wParam == VK_DELETE)
				SkinPlaySound("SoundOnTyping");
			//
			if (wParam == VK_INSERT && !isShift && !isCtrl && !isAlt) {
				mwdat->fInsertMode = !mwdat->fInsertMode;
				SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), EN_CHANGE), (LPARAM) hwnd);
			}
			if (wParam == VK_CAPITAL || wParam == VK_NUMLOCK)
				SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), EN_CHANGE), (LPARAM) hwnd);

			if (isCtrl && isAlt && !isShift) {
				switch (wParam) {
					case VK_UP:
					case VK_DOWN:
					case VK_PRIOR:
					case VK_NEXT:
					case VK_HOME:
					case VK_END: {
						WPARAM wp = 0;

						if (wParam == VK_UP)
							wp = MAKEWPARAM(SB_LINEUP, 0);
						else if (wParam == VK_PRIOR)
							wp = MAKEWPARAM(SB_PAGEUP, 0);
						else if (wParam == VK_NEXT)
							wp = MAKEWPARAM(SB_PAGEDOWN, 0);
						else if (wParam == VK_HOME)
							wp = MAKEWPARAM(SB_TOP, 0);
						else if (wParam == VK_END) {
							DM_ScrollToBottom(mwdat, 0, 0);
							return 0;
						} else if (wParam == VK_DOWN)
							wp = MAKEWPARAM(SB_LINEDOWN, 0);

						SendMessage(GetDlgItem(hwndParent, IDC_CHAT_LOG), WM_VSCROLL, wp, 0);
						return 0;
					}
				}
			}

			if (wParam == VK_RETURN) {
				if (isShift) {
					if (PluginConfig.m_SendOnShiftEnter) {
						PostMessage(hwndParent, WM_COMMAND, IDOK, 0);
						return 0;
					} else
						break;
				}
				if ((isCtrl && !isShift) ^(0 != PluginConfig.m_SendOnEnter)) {
					PostMessage(hwndParent, WM_COMMAND, IDOK, 0);
					return 0;
				}
				if (PluginConfig.m_SendOnEnter || PluginConfig.m_SendOnDblEnter) {
					if (isCtrl)
						break;
					else {
						if (PluginConfig.m_SendOnDblEnter) {
							if (dat->lastEnterTime + 2 < time(NULL)) {
								dat->lastEnterTime = time(NULL);
								break;
							} else {
								SendMessage(hwnd, WM_KEYDOWN, VK_BACK, 0);
								SendMessage(hwnd, WM_KEYUP, VK_BACK, 0);
								PostMessage(hwndParent, WM_COMMAND, IDOK, 0);
								return 0;
							}
						}
						PostMessage(hwndParent, WM_COMMAND, IDOK, 0);
						return 0;
					}
				} else
					break;
			} else
				dat->lastEnterTime = 0;

			if ((wParam == VK_NEXT && isCtrl && !isShift) || (wParam == VK_TAB && isCtrl && !isShift)) { // CTRL-TAB (switch tab/window)
				SendMessage(mwdat->pContainer->hwnd, DM_SELECTTAB, DM_SELECT_NEXT, 0);
				return TRUE;
			}

			if ((wParam == VK_PRIOR && isCtrl && !isShift) || (wParam == VK_TAB && isCtrl && isShift)) { // CTRL_SHIFT-TAB (switch tab/window)
				SendMessage(mwdat->pContainer->hwnd, DM_SELECTTAB, DM_SELECT_PREV, 0);
				return TRUE;
			}
			if (wParam == VK_TAB && !isCtrl && !isShift) {    //tab-autocomplete
				int 		iLen, end, topicStart;
				BOOL 		isTopic = FALSE;
				BOOL 		isRoom = FALSE;
				wchar_t*	pszText = NULL;
				GETTEXTEX	gt = {0};
				LRESULT		lResult = (LRESULT)SendMessage(hwnd, EM_GETSEL, (WPARAM)NULL, (LPARAM)NULL);
				bool		fCompleted = false;

				SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);
				start = LOWORD(lResult);
				end = HIWORD(lResult);
				SendMessage(hwnd, EM_SETSEL, end, end);

				gt.codepage = 1200;
				gt.flags = GTL_DEFAULT | GTL_PRECISE;

				iLen = SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gt, (LPARAM)0);
				if (iLen > 0) {
					wchar_t*	pszName = NULL;
					pszText = reinterpret_cast<wchar_t*>(Utils::safeMirCalloc((iLen + 10) * sizeof(wchar_t)));
					gt.flags = GT_DEFAULT;
					gt.cb = (iLen + 9) * sizeof(wchar_t);

					SendMessage(hwnd, EM_GETTEXTEX, (WPARAM)&gt, (LPARAM)pszText);

					if(start > 1 && pszText[start-1] == ' ' && pszText[start-2] == ':')
						start--;

					while( start > 0 && pszText[start-1] != ' ' && pszText[start-1] != 13 && pszText[start-1] != VK_TAB)
						start--;

					while (end < iLen && pszText[end] != ' ' && pszText[end] != 13 && pszText[end-1] != VK_TAB)
						end ++;

					if(pszText[start] == '#')
						isRoom = TRUE;
					else {
						topicStart = (int)start;
						while ( topicStart >0 && (pszText[topicStart-1] == ' ' || pszText[topicStart-1] == 13 || pszText[topicStart-1] == VK_TAB))
							topicStart--;
						if (topicStart > 5 && _tcsstr(&pszText[topicStart-6], _T("/topic")) == &pszText[topicStart-6])
							isTopic = TRUE;
					}
					if (dat->szSearchQuery == NULL) {
						size_t len = (end - start) + 1;
						dat->szSearchQuery = reinterpret_cast<wchar_t*>(Utils::safeMirAlloc(sizeof(wchar_t) * len));
						wcsncpy( dat->szSearchQuery, pszText + start, len);
						dat->szSearchQuery[len - 1] = 0;
						dat->szSearchResult = mir_tstrdup(dat->szSearchQuery);
						dat->lastSession = NULL;
					}
					if (isTopic) {
						pszName = Parentsi->ptszTopic;
					} else if (isRoom) {
						dat->lastSession = SM_FindSessionAutoComplete(Parentsi->pszModule, Parentsi, dat->lastSession, dat->szSearchQuery, dat->szSearchResult);
						if (dat->lastSession != NULL)
							pszName = dat->lastSession->ptszName;
					} else
						pszName = UM_FindUserAutoComplete(Parentsi->pUsers, dat->szSearchQuery, dat->szSearchResult);

					mir_free(pszText);
					pszText = NULL;
					mir_free(dat->szSearchResult);
					dat->szSearchResult = 0;
					if (pszName == 0) {
						if ((int)end != (int)start) {
							SendMessage(hwnd, EM_SETSEL, start, end);
							SendMessage(hwnd, EM_REPLACESEL, TRUE, (LPARAM) dat->szSearchQuery);
						}
						mir_free(dat->szSearchQuery);
						dat->szSearchQuery = NULL;
					} else {
						pszText = 0;
						dat->szSearchResult = mir_tstrdup(pszName);
						if ((int)end != (int)start) {
							if (!isRoom && !isTopic && g_Settings.AddColonToAutoComplete && start == 0) {
								pszText = reinterpret_cast<wchar_t*>(Utils::safeMirAlloc((wcslen(pszName) + 4) * sizeof(wchar_t)));
								wcscpy(pszText, pszName);
								wcscat(pszText, L": ");
								pszName = pszText;
							}
							SendMessage(hwnd, EM_SETSEL, start, end);
							SendMessage(hwnd, EM_REPLACESEL, TRUE, (LPARAM)pszName);
						}
						if(pszText)
							mir_free(pszText);
						fCompleted = true;
					}
				}
				SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
				if(!fCompleted && !PluginConfig.m_AllowTab) {
					if((GetSendButtonState(mwdat->hwnd) != PBS_DISABLED))
						SetFocus(GetDlgItem(mwdat->hwnd, IDOK));
					else
						SetFocus(GetDlgItem(mwdat->hwnd, IDC_CHAT_LOG));
				}
				return 0;
			} else if (wParam != VK_RIGHT && wParam != VK_LEFT) {
				mir_free(dat->szSearchQuery);
				dat->szSearchQuery = NULL;
				mir_free(dat->szSearchResult);
				dat->szSearchResult = NULL;
			}

			if (wParam == VK_F4 && isCtrl && !isAlt) { // ctrl-F4 (close tab)
				SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(IDC_CHAT_CLOSE, BN_CLICKED), 0);
				return 0;
			}

			if (wParam == VK_NEXT || wParam == VK_PRIOR) {
				HWND htemp = hwndParent;
				SendDlgItemMessage(htemp, IDC_CHAT_LOG, msg, wParam, lParam);
				dat->lastEnterTime = 0;
				return 0;
			}

			if (wParam == VK_UP && isCtrl && !isAlt) {
				int iLen;
				GETTEXTLENGTHEX gtl = {0};
				SETTEXTEX ste;
				LOGFONTA lf;
				char* lpPrevCmd = SM_GetPrevCommand(Parentsi->ptszID, Parentsi->pszModule);

				SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);

				LoadLogfont(MSGFONTID_MESSAGEAREA, &lf, NULL, FONTMODULE);
				ste.flags = ST_DEFAULT;
				ste.codepage = CP_ACP;
				if (lpPrevCmd)
					SendMessage(hwnd, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)lpPrevCmd);
				else
					SetWindowText(hwnd, _T(""));

				gtl.flags = GTL_PRECISE;
				gtl.codepage = CP_ACP;
				iLen = SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM) & gtl, (LPARAM)NULL);
				SendMessage(hwnd, EM_SCROLLCARET, 0, 0);
				SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
				SendMessage(hwnd, EM_SETSEL, iLen, iLen);
				dat->lastEnterTime = 0;
				return 0;
			}

			if (wParam == VK_DOWN && isCtrl && !isAlt) {
				int iLen;
				GETTEXTLENGTHEX gtl = {0};
				SETTEXTEX ste;

				char* lpPrevCmd = SM_GetNextCommand(Parentsi->ptszID, Parentsi->pszModule);
				SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);

				ste.flags = ST_DEFAULT;
				ste.codepage = CP_ACP;
				if (lpPrevCmd)
					SendMessage(hwnd, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM) lpPrevCmd);
				else
					SetWindowText(hwnd, _T(""));

				gtl.flags = GTL_PRECISE;
				gtl.codepage = CP_ACP;
				iLen = SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM) & gtl, (LPARAM)NULL);
				SendMessage(hwnd, EM_SCROLLCARET, 0, 0);
				SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
				SendMessage(hwnd, EM_SETSEL, iLen, iLen);
				dat->lastEnterTime = 0;
				return 0;
			}
			if (wParam == VK_RETURN)
				break;
			//fall through
		}

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_KILLFOCUS:
			dat->lastEnterTime = 0;
			break;

		case WM_KEYUP:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP: {
			CHARFORMAT2 cf;
			UINT u = 0;
			UINT u2 = 0;
			COLORREF cr;
			MODULEINFO* mi = MM_FindModule(Parentsi->pszModule);

			LoadLogfont(MSGFONTID_MESSAGEAREA, NULL, &cr, FONTMODULE);

			cf.cbSize = sizeof(CHARFORMAT2);
			cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_BACKCOLOR | CFM_COLOR | CFM_UNDERLINETYPE;
			cf.dwEffects = 0;
			SendMessage(hwnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

			if (mi && mi->bColor) {
				int index = Chat_GetColorIndex(Parentsi->pszModule, cf.crTextColor);
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_COLOR);

				if (index >= 0) {
					Parentsi->bFGSet = TRUE;
					Parentsi->iFG = index;
				}

				if (u == BST_UNCHECKED && cf.crTextColor != cr)
					CheckDlgButton(hwndParent, IDC_COLOR, BST_CHECKED);
				else if (u == BST_CHECKED && cf.crTextColor == cr)
					CheckDlgButton(hwndParent, IDC_COLOR, BST_UNCHECKED);
			}

			if (mi && mi->bBkgColor) {
				int index = Chat_GetColorIndex(Parentsi->pszModule, cf.crBackColor);
				COLORREF crB = (COLORREF)M->GetDword(FONTMODULE, "inputbg", SRMSGDEFSET_BKGCOLOUR);
				u = IsDlgButtonChecked(hwndParent, IDC_BKGCOLOR);

				if (index >= 0) {
					Parentsi->bBGSet = TRUE;
					Parentsi->iBG = index;
				}

				if (u == BST_UNCHECKED && cf.crBackColor != crB)
					CheckDlgButton(hwndParent, IDC_BKGCOLOR, BST_CHECKED);
				else if (u == BST_CHECKED && cf.crBackColor == crB)
					CheckDlgButton(hwndParent, IDC_BKGCOLOR, BST_UNCHECKED);
			}

			if (mi && mi->bBold) {
				u = IsDlgButtonChecked(hwndParent, IDC_CHAT_BOLD);
				u2 = cf.dwEffects;
				u2 &= CFE_BOLD;
				if (u == BST_UNCHECKED && u2)
					CheckDlgButton(hwndParent, IDC_CHAT_BOLD, BST_CHECKED);
				else if (u == BST_CHECKED && u2 == 0)
					CheckDlgButton(hwndParent, IDC_CHAT_BOLD, BST_UNCHECKED);
			}

			if (mi && mi->bItalics) {
				u = IsDlgButtonChecked(hwndParent, IDC_ITALICS);
				u2 = cf.dwEffects;
				u2 &= CFE_ITALIC;
				if (u == BST_UNCHECKED && u2)
					CheckDlgButton(hwndParent, IDC_ITALICS, BST_CHECKED);
				else if (u == BST_CHECKED && u2 == 0)
					CheckDlgButton(hwndParent, IDC_ITALICS, BST_UNCHECKED);
			}

			if (mi && mi->bUnderline) {
				u = IsDlgButtonChecked(hwndParent, IDC_CHAT_UNDERLINE);
				if(cf.dwEffects & CFE_UNDERLINE && (cf.bUnderlineType & CFU_UNDERLINE || cf.bUnderlineType & CFU_UNDERLINEWORD)) {
					if (u == BST_UNCHECKED )
						CheckDlgButton(hwndParent, IDC_CHAT_UNDERLINE, BST_CHECKED);
				}
				else {
					if (u == BST_CHECKED)
						CheckDlgButton(hwndParent, IDC_CHAT_UNDERLINE, BST_UNCHECKED);
				}
			}
		}
		break;

		case WM_INPUTLANGCHANGEREQUEST:
			return DefWindowProc(hwnd, WM_INPUTLANGCHANGEREQUEST, wParam, lParam);

		case WM_INPUTLANGCHANGE:
			if (PluginConfig.m_AutoLocaleSupport && GetFocus() == hwnd && mwdat->pContainer->hwndActive == hwndParent && GetForegroundWindow() == mwdat->pContainer->hwnd && GetActiveWindow() == mwdat->pContainer->hwnd) {
				DM_SaveLocale(mwdat, wParam, lParam);
				SendMessage(hwnd, EM_SETLANGOPTIONS, 0, (LPARAM) SendMessage(hwnd, EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOKEYBOARD);
				return 1;
			}
			break;

		case WM_ERASEBKGND: {
			return(CSkin::m_skinEnabled ? 0 : 1);
		}

		case EM_UNSUBCLASSED:
			mir_free(dat);
			return 0;
	}

	return CallWindowProc(OldMessageProc, hwnd, msg, wParam, lParam);
}


/*
 * subclassing for the message filter dialog (set and configure event filters for the current
 * session
 */

static UINT _eventorder[] = { GC_EVENT_ACTION,
	GC_EVENT_MESSAGE,
	GC_EVENT_NICK,
	GC_EVENT_JOIN,
	GC_EVENT_PART,
	GC_EVENT_TOPIC,
	GC_EVENT_ADDSTATUS,
	GC_EVENT_INFORMATION,
	GC_EVENT_QUIT,
	GC_EVENT_KICK,
	GC_EVENT_NOTICE,
	0
};

static INT_PTR CALLBACK FilterWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SESSION_INFO * si = (SESSION_INFO *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (uMsg) {
		case WM_INITDIALOG: {

			si = (SESSION_INFO *)lParam;
			DWORD dwMask = M->GetDword(si->hContact, "Chat", "FilterMask", 0);
			DWORD dwFlags = M->GetDword(si->hContact, "Chat", "FilterFlags", 0);

			DWORD dwPopupMask = M->GetDword(si->hContact, "Chat", "PopupMask", 0);
			DWORD dwPopupFlags = M->GetDword(si->hContact, "Chat", "PopupFlags", 0);

			DWORD dwTrayMask = M->GetDword(si->hContact, "Chat", "TrayIconMask", 0);
			DWORD dwTrayFlags = M->GetDword(si->hContact, "Chat", "TrayIconFlags", 0);

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)si);

			for(int i = 0; _eventorder[i]; i++) {
				CheckDlgButton(hwndDlg, IDC_1 + i, dwMask & _eventorder[i] ? (dwFlags & _eventorder[i] ? BST_CHECKED : BST_UNCHECKED) : BST_INDETERMINATE);
				CheckDlgButton(hwndDlg, IDC_P1 + i, dwPopupMask & _eventorder[i] ? (dwPopupFlags & _eventorder[i] ? BST_CHECKED : BST_UNCHECKED) : BST_INDETERMINATE);
				CheckDlgButton(hwndDlg, IDC_T1 + i, dwTrayMask & _eventorder[i] ? (dwTrayFlags & _eventorder[i] ? BST_CHECKED : BST_UNCHECKED) : BST_INDETERMINATE);
			}
			return(FALSE);
		}
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORSTATIC:
			SetTextColor((HDC)wParam, RGB(60, 60, 150));
			SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
			return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);

		case WM_CLOSE:
			if (wParam == 1 && lParam == 1) {
				int iFlags = 0, i;
				UINT result;
				DWORD dwMask = 0, dwFlags = 0;

				for(i = 0; _eventorder[i]; i++) {
					result = IsDlgButtonChecked(hwndDlg, IDC_1 + i);
					dwMask |= (result != BST_INDETERMINATE ? _eventorder[i] : 0);
					iFlags |= (result == BST_CHECKED ? _eventorder[i] : 0);
				}

				if (iFlags&GC_EVENT_ADDSTATUS)
					iFlags |= GC_EVENT_REMOVESTATUS;

				if (si) {
					if (dwMask == 0) {
						DBDeleteContactSetting(si->hContact, "Chat", "FilterFlags");
						DBDeleteContactSetting(si->hContact, "Chat", "FilterMask");
					} else {
						M->WriteDword(si->hContact, "Chat", "FilterFlags", iFlags);
						M->WriteDword(si->hContact, "Chat", "FilterMask", dwMask);
					}
				}

				dwMask = iFlags = 0;

				for(i = 0; _eventorder[i]; i++) {
					result = IsDlgButtonChecked(hwndDlg, IDC_P1 + i);
					dwMask |= (result != BST_INDETERMINATE ? _eventorder[i] : 0);
					iFlags |= (result == BST_CHECKED ? _eventorder[i] : 0);
				}

				if (iFlags&GC_EVENT_ADDSTATUS)
					iFlags |= GC_EVENT_REMOVESTATUS;

				if (si) {
					if (dwMask == 0) {
						DBDeleteContactSetting(si->hContact, "Chat", "PopupFlags");
						DBDeleteContactSetting(si->hContact, "Chat", "PopupMask");
					} else {
						M->WriteDword(si->hContact, "Chat", "PopupFlags", iFlags);
						M->WriteDword(si->hContact, "Chat", "PopupMask", dwMask);
					}
				}

				dwMask = iFlags = 0;

				for(i = 0; _eventorder[i]; i++) {
					result = IsDlgButtonChecked(hwndDlg, IDC_T1 + i);
					dwMask |= (result != BST_INDETERMINATE ? _eventorder[i] : 0);
					iFlags |= (result == BST_CHECKED ? _eventorder[i] : 0);
				}
				if (iFlags&GC_EVENT_ADDSTATUS)
					iFlags |= GC_EVENT_REMOVESTATUS;

				if (si) {
					if (dwMask == 0) {
						DBDeleteContactSetting(si->hContact, "Chat", "TrayIconFlags");
						DBDeleteContactSetting(si->hContact, "Chat", "TrayIconMask");
					} else {
						M->WriteDword(si->hContact, "Chat", "TrayIconFlags", iFlags);
						M->WriteDword(si->hContact, "Chat", "TrayIconMask", dwMask);
					}
					Chat_SetFilters(si);
					SendMessage(si->hWnd, GC_CHANGEFILTERFLAG, 0, (LPARAM)iFlags);
					if (si->bFilterEnabled)
						SendMessage(si->hWnd, GC_REDRAWLOG, 0, 0);
				}
			}
			DestroyWindow(hwndDlg);
			break;
		case WM_DESTROY:
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
			break;
	}
	return(FALSE);
}

/**
 * subclass for some tool bar buttons which must perform special actions
 * on right click.
 */
static LRESULT CALLBACK ButtonSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndParent = GetParent(hwnd);

	switch (msg) {
		case WM_RBUTTONUP: {
			HWND hFilter = GetDlgItem(hwndParent, IDC_FILTER);
			HWND hColor = GetDlgItem(hwndParent, IDC_COLOR);
			HWND hBGColor = GetDlgItem(hwndParent, IDC_BKGCOLOR);

			if (M->GetByte("Chat", "RightClickFilter", 0) != 0) {
				if (hFilter == hwnd)
					SendMessage(hwndParent, GC_SHOWFILTERMENU, 0, 0);
				if (hColor == hwnd)
					SendMessage(hwndParent, GC_SHOWCOLORCHOOSER, 0, (LPARAM)IDC_COLOR);
				if (hBGColor == hwnd)
					SendMessage(hwndParent, GC_SHOWCOLORCHOOSER, 0, (LPARAM)IDC_BKGCOLOR);
			}
		}
		break;
	}

	return CallWindowProc(OldFilterButtonProc, hwnd, msg, wParam, lParam);
}


/*
 * subclassing for the message history display (rich edit control in which the chat history appears)
 */

static LRESULT CALLBACK LogSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndParent = GetParent(hwnd);
	struct TWindowData *mwdat = (struct TWindowData *)GetWindowLongPtr(hwndParent, GWLP_USERDATA);

	switch (msg) {
		case WM_NCCALCSIZE:
			return(CSkin::NcCalcRichEditFrame(hwnd, mwdat, ID_EXTBKHISTORY, msg, wParam, lParam, OldLogProc));

		case WM_NCPAINT:
			return(CSkin::DrawRichEditFrame(hwnd, mwdat, ID_EXTBKHISTORY, msg, wParam, lParam, OldLogProc));

		case WM_COPY:
			return(DM_WMCopyHandler(hwnd, OldLogProc, wParam, lParam));

		case WM_SETCURSOR:
			if (g_Settings.ClickableNicks && (LOWORD(lParam) == HTCLIENT)) {
				POINT pt;
				GetCursorPos(&pt);
				ScreenToClient(hwnd,&pt);
				if (CheckCustomLink(hwnd, &pt, msg, wParam, lParam, FALSE)) return TRUE;
			}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
			if (g_Settings.ClickableNicks) {
				POINT pt={LOWORD(lParam), HIWORD(lParam)};
				CheckCustomLink(hwnd, &pt, msg, wParam, lParam, TRUE);
			}
			break;

		case WM_LBUTTONUP:
			{
			CHARRANGE sel;
			if (g_Settings.ClickableNicks) {
				POINT pt={LOWORD(lParam), HIWORD(lParam)};
				CheckCustomLink(hwnd, &pt, msg, wParam, lParam, TRUE);
			}
			if(true || M->GetByte("autocopy", 0)) {
				SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM) &sel);
				if (sel.cpMin != sel.cpMax) {
					SendMessage(hwnd, WM_COPY, 0, 0);
					sel.cpMin = sel.cpMax ;
					SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) & sel);
					SetFocus(GetDlgItem(hwndParent, IDC_CHAT_MESSAGE));
				}
			}
		}
		break;

		case WM_KEYDOWN:
			if (wParam == 0x57 && GetKeyState(VK_CONTROL) & 0x8000) { // ctrl-w (close window)
				PostMessage(hwndParent, WM_CLOSE, 0, 1);
				return TRUE;
			}
			break;

		case WM_SYSKEYUP:
			if(wParam == VK_MENU) {
				ProcessHotkeysByMsgFilter(hwnd, msg, wParam, lParam, IDC_CHAT_LOG);
				return(0);
			}
			break;

		case WM_SYSKEYDOWN:
			mwdat->fkeyProcessed = false;
			if(ProcessHotkeysByMsgFilter(hwnd, msg, wParam, lParam, IDC_CHAT_LOG)) {
				mwdat->fkeyProcessed = true;
				return(0);
			}
			break;

		case WM_SYSCHAR: {
			if(mwdat->fkeyProcessed) {
				mwdat->fkeyProcessed = false;
				return(0);
			}
			break;
		}

		case WM_ACTIVATE: {
			if (LOWORD(wParam) == WA_INACTIVE) {
				CHARRANGE sel;
				SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM) &sel);
				if (sel.cpMin != sel.cpMax) {
					sel.cpMin = sel.cpMax ;
					SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) & sel);
				}
			}
		}
		break;

		case WM_CHAR:
			SetFocus(GetDlgItem(hwndParent, IDC_CHAT_MESSAGE));
			SendMessage(GetDlgItem(hwndParent, IDC_CHAT_MESSAGE), WM_CHAR, wParam, lParam);
			break;
	}

	return CallWindowProc(OldLogProc, hwnd, msg, wParam, lParam);
}


/*
 * process mouse - hovering for the nickname list. fires events so the protocol can
 * show the userinfo - tooltip.
 */

static void ProcessNickListHovering(HWND hwnd, int hoveredItem, POINT * pt, SESSION_INFO * parentdat)
{
	static int currentHovered = -1;
	static HWND hwndToolTip = NULL;
	static HWND oldParent = NULL;
	TOOLINFO ti = {0};
	RECT clientRect;
	BOOL bNewTip = FALSE;
	USERINFO *ui1 = NULL;

	if (hoveredItem == currentHovered) return;
	currentHovered = hoveredItem;

	if (oldParent != hwnd && hwndToolTip) {
		SendMessage(hwndToolTip, TTM_DELTOOL, 0, 0);
		DestroyWindow(hwndToolTip);
		hwndToolTip = NULL;
	}
	if (hoveredItem == -1) {

		SendMessage(hwndToolTip, TTM_ACTIVATE, 0, 0);

	} else {

		if (!hwndToolTip) {
			hwndToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS,  NULL,
										 WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
										 CW_USEDEFAULT, CW_USEDEFAULT,  CW_USEDEFAULT,  CW_USEDEFAULT,
										 hwnd, NULL, g_hInst,  NULL);
			bNewTip = TRUE;
		}

		GetClientRect(hwnd, &clientRect);
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_SUBCLASS;
		ti.hinst = g_hInst;
		ti.hwnd = hwnd;
		ti.uId = 1;
		ti.rect = clientRect;

		ti.lpszText = NULL;

		ui1 = SM_GetUserFromIndex(parentdat->ptszID, parentdat->pszModule, currentHovered);
		if (ui1) {
			char serviceName[256];
			_snprintf(serviceName, SIZEOF(serviceName), "%s"MS_GC_PROTO_GETTOOLTIPTEXT, parentdat->pszModule);
			if (ServiceExists(serviceName))
				ti.lpszText = (TCHAR*)CallService(serviceName, (WPARAM)parentdat->ptszID, (LPARAM)ui1->pszUID);
			else {
				TCHAR ptszBuf[ 1024 ];
				mir_sntprintf( ptszBuf, SIZEOF(ptszBuf), _T("%s: %s\r\n%s: %s\r\n%s: %s"),
					CTranslator::get(CTranslator::GEN_MUC_NICKNAME), ui1->pszNick,
					CTranslator::get(CTranslator::GEN_MUC_UID), ui1->pszUID,
					CTranslator::get(CTranslator::GEN_MUC_STATUS), TM_WordToString( parentdat->pStatuses, ui1->Status ));
				ti.lpszText = mir_tstrdup( ptszBuf );
			}
		}

		SendMessage(hwndToolTip, bNewTip ? TTM_ADDTOOL : TTM_UPDATETIPTEXT, 0, (LPARAM) &ti);
		SendMessage(hwndToolTip, TTM_ACTIVATE, (ti.lpszText != NULL) , 0);
		SendMessage(hwndToolTip, TTM_SETMAXTIPWIDTH, 0 , 400);
		if (ti.lpszText)
			mir_free(ti.lpszText);
	}
}

/*
 * subclassing for the nickname list control. It is an ownerdrawn listbox
 */

static LRESULT CALLBACK NicklistSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndParent = GetParent(hwnd);
	struct TWindowData *mwdat = (struct TWindowData *)GetWindowLongPtr(hwndParent, GWLP_USERDATA);

	switch (msg) {
		//MAD: attemp to fix weird bug, when combobox with hidden vscroll
		//can't be scrolled with mouse-wheel.
		case WM_NCCALCSIZE: {
			if (CSkin::m_DisableScrollbars) {
				RECT lpRect;
				LONG itemHeight;

				GetClientRect (hwnd, &lpRect);
				itemHeight = SendMessage(hwnd, LB_GETITEMHEIGHT, 0, 0);
				g_cLinesPerPage = (lpRect.bottom - lpRect.top) /itemHeight	;
			}
			return(CSkin::NcCalcRichEditFrame(hwnd, mwdat, ID_EXTBKUSERLIST, msg, wParam, lParam, OldNicklistProc));
		}
		 //
		case WM_NCPAINT:
			return(CSkin::DrawRichEditFrame(hwnd, mwdat, ID_EXTBKUSERLIST, msg, wParam, lParam, OldNicklistProc));

		case WM_ERASEBKGND: {
			HDC dc = (HDC)wParam;
			struct TWindowData *dat = (struct TWindowData *)GetWindowLongPtr(hwndParent, GWLP_USERDATA);
			SESSION_INFO *parentdat = (SESSION_INFO *)dat->si;
			if (dc) {
				int height, index, items = 0;

				index = SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
				if (index == LB_ERR || parentdat->nUsersInNicklist <= 0)
					return 0;

				items = parentdat->nUsersInNicklist - index;
				height = SendMessage(hwnd, LB_GETITEMHEIGHT, 0, 0);

				if (height != LB_ERR) {
					RECT rc = {0};
					GetClientRect(hwnd, &rc);

					if (rc.bottom - rc.top > items * height) {
						rc.top = items * height;
						FillRect(dc, &rc, hListBkgBrush);
					}
				}
			}
		}
		return 1;

		//MAD
		case WM_MOUSEWHEEL: {
			if (CSkin::m_DisableScrollbars)	{
				UINT uScroll;
				int dLines;
				short zDelta=(short)HIWORD(wParam);
				if (!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uScroll, 0))
					uScroll = 3;    /* default value */

				if (uScroll == WHEEL_PAGESCROLL)
					uScroll = g_cLinesPerPage;
				if (uScroll == 0)
					return 0;

				zDelta += g_iWheelCarryover;    /* Accumulate wheel motion */

				dLines = zDelta * (int)uScroll / WHEEL_DELTA;


				//Record the unused portion as the next carryover.
				g_iWheelCarryover = zDelta - dLines * WHEEL_DELTA / (int)uScroll;


				// scrolling.
				while (abs(dLines)) {
					if (dLines > 0) {
						SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0);
						dLines--;
					} else {
						SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0);
						dLines++;
					}
				}
				return 0;
			}
			break;
		}
//MAD_
		case WM_KEYDOWN:
			if (wParam == 0x57 && GetKeyState(VK_CONTROL) & 0x8000) { // ctrl-w (close window)
				PostMessage(hwndParent, WM_CLOSE, 0, 1);
				return TRUE;
			}
			if (wParam == VK_ESCAPE || wParam == VK_UP || wParam == VK_DOWN || wParam == VK_NEXT ||
					wParam == VK_PRIOR || wParam == VK_TAB || wParam == VK_HOME || wParam == VK_END) {
				if (mwdat && mwdat->si) {
					SESSION_INFO *si = (SESSION_INFO *)mwdat->si;
					si->szSearch[0] = 0;
					si->iSearchItem = -1;
				}
			}
			break;

		case WM_SETFOCUS:
		case WM_KILLFOCUS:
			if (mwdat && mwdat->si) {                   // set/kill focus invalidates incremental search status
				SESSION_INFO *si = (SESSION_INFO *)mwdat->si;
				si->szSearch[0] = 0;
				si->iSearchItem = -1;
			}
			break;
		case WM_CHAR:
		case WM_UNICHAR: {
			/*
			* simple incremental search for the user (nick) - list control
			* typing esc or movement keys will clear the current search string
			*/

			if (mwdat && mwdat->si) {
				SESSION_INFO *si = (SESSION_INFO *)mwdat->si;
				if (wParam == 27 && si->szSearch[0]) {						// escape - reset everything
					si->szSearch[0] = 0;
					si->iSearchItem = -1;
					break;
				} else if (wParam == '\b' && si->szSearch[0])					// backspace
					si->szSearch[lstrlen(si->szSearch) - 1] = '\0';
				else if (wParam < ' ')
					break;
				else {
					TCHAR szNew[2];
					szNew[0] = (TCHAR) wParam;
					szNew[1] = '\0';
					if (lstrlen(si->szSearch) >= SIZEOF(si->szSearch) - 2) {
						MessageBeep(MB_OK);
						break;
					}
					_tcscat(si->szSearch, szNew);
				}
				if (si->szSearch[0]) {
					int     iItems = SendMessage(hwnd, LB_GETCOUNT, 0, 0);
					int     i;
					USERINFO *ui;

					/*
					* iterate over the (sorted) list of nicknames and search for the
					* string we have
					*/

					for (i = 0; i < iItems; i++) {
						ui = UM_FindUserFromIndex(si->pUsers, i);
						if (ui) {
							if (!_tcsnicmp(ui->pszNick, si->szSearch, lstrlen(si->szSearch))) {
								SendMessage(hwnd, LB_SETSEL, FALSE, -1);
								SendMessage(hwnd, LB_SETSEL, TRUE, i);
								si->iSearchItem = i;
								InvalidateRect(hwnd, NULL, FALSE);
								return 0;
							}
						}
					}
					if (i == iItems) {
						MessageBeep(MB_OK);
						si->szSearch[lstrlen(si->szSearch) - 1] = '\0';
						return 0;
					}
				}
			}
			break;
		}
		case WM_RBUTTONDOWN: {
			int iCounts = SendMessage(hwnd, LB_GETSELCOUNT, 0, 0);

			if (iCounts != LB_ERR && iCounts > 1)
				return 0;
			SendMessage(hwnd, WM_LBUTTONDOWN, wParam, lParam);
			break;
		}

		case WM_RBUTTONUP:
			SendMessage(hwnd, WM_LBUTTONUP, wParam, lParam);
			break;

		case WM_MEASUREITEM: {
			MEASUREITEMSTRUCT *mis = (MEASUREITEMSTRUCT *) lParam;
			if (mis->CtlType == ODT_MENU)
				return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);
			return FALSE;
		}
		case WM_DRAWITEM: {
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *) lParam;
			if (dis->CtlType == ODT_MENU)
				return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
			return FALSE;
		}
		case WM_CONTEXTMENU: {
			TVHITTESTINFO hti;
			int item;
			int height;
			USERINFO * ui;
			struct TWindowData *dat = (struct TWindowData *)GetWindowLongPtr(hwndParent, GWLP_USERDATA);
			SESSION_INFO *parentdat = (SESSION_INFO *)dat->si;


			hti.pt.x = (short) LOWORD(lParam);
			hti.pt.y = (short) HIWORD(lParam);
			if (hti.pt.x == -1 && hti.pt.y == -1) {
				int index = SendMessage(hwnd, LB_GETCURSEL, 0, 0);
				int top = SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
				height = SendMessage(hwnd, LB_GETITEMHEIGHT, 0, 0);
				hti.pt.x = 4;
				hti.pt.y = (index - top) * height + 1;
			} else
				ScreenToClient(hwnd, &hti.pt);

			item = (DWORD)(SendMessage(hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(hti.pt.x, hti.pt.y)));
			if ( HIWORD( item ) == 1 )
				item = (DWORD)(-1);
			else
				item &= 0xFFFF;

			ui = SM_GetUserFromIndex(parentdat->ptszID, parentdat->pszModule, item);
			//			ui = (USERINFO *)SendMessage(GetDlgItem(hwndParent, IDC_LIST), LB_GETITEMDATA, item, 0);
			if (ui) {
				HMENU hMenu = 0;
				UINT uID;
				USERINFO uinew;

				memcpy(&uinew, ui, sizeof(USERINFO));
				if (hti.pt.x == -1 && hti.pt.y == -1)
					hti.pt.y += height - 4;
				ClientToScreen(hwnd, &hti.pt);
				uID = CreateGCMenu(hwnd, &hMenu, 0, hti.pt, parentdat, uinew.pszUID, NULL);

				switch (uID) {
					case 0:
						break;

					case 20020:	{							// add to highlight...
						RECT	rc, rcWnd;
						THighLightEdit the = {THighLightEdit::CMD_ADD, parentdat, ui};

						if(parentdat && ui) {
							HWND hwnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_ADDHIGHLIGHT), parentdat->dat->pContainer->hwnd, CMUCHighlight::dlgProcAdd, (LPARAM)&the);
							TranslateDialogDefault(hwnd);
							GetClientRect(parentdat->pContainer->hwnd, &rcWnd);
							GetWindowRect(hwnd, &rc);

							SetWindowPos(hwnd, HWND_TOP, (rcWnd.right - (rc.right - rc.left)) / 2, (rcWnd.bottom - (rc.bottom - rc.top)) / 2 , 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
						}
						break;
					}

					case ID_MESS:
						DoEventHookAsync(GetParent(hwnd), parentdat->ptszID, parentdat->pszModule, GC_USER_PRIVMESS, ui->pszUID, NULL, (LPARAM)NULL);
						break;

					default: {
						int iCount = SendMessage(hwnd, LB_GETCOUNT, 0, 0);

						if (iCount != LB_ERR) {
							int iSelectedItems = SendMessage(hwnd, LB_GETSELCOUNT, 0, 0);

							if (iSelectedItems != LB_ERR) {
								int *pItems = (int *)malloc(sizeof(int) * (iSelectedItems + 1));

								if (pItems) {
									if (SendMessage(hwnd, LB_GETSELITEMS, (WPARAM)iSelectedItems, (LPARAM)pItems) != LB_ERR) {
										USERINFO *ui1 = NULL;
										int i;

										for (i = 0; i < iSelectedItems; i++) {
											ui1 = SM_GetUserFromIndex(parentdat->ptszID, parentdat->pszModule, pItems[i]);
											if (ui1)
												DoEventHookAsync(hwndParent, parentdat->ptszID, parentdat->pszModule, GC_USER_NICKLISTMENU, ui1->pszUID, NULL, (LPARAM)uID);
										}
									}
									free(pItems);
								}
							}
						}
						//DoEventHookAsync(hwndParent, parentdat->ptszID, parentdat->pszModule, GC_USER_NICKLISTMENU, ui->pszUID, NULL, (LPARAM)uID);
						break;
					}
				}
				DestroyGCMenu(&hMenu, 1);
				return TRUE;
			}
		}
		break;
		case WM_MOUSEMOVE: {
			POINT pt;
			RECT clientRect;
			BOOL bInClient;
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
			GetClientRect(hwnd, &clientRect);
			bInClient = PtInRect(&clientRect, pt);
 			if (bInClient) {
				//hit test item under mouse
				struct TWindowData *dat = (struct TWindowData *)GetWindowLongPtr(hwndParent, GWLP_USERDATA);
				SESSION_INFO *parentdat = (SESSION_INFO *)dat->si;

				DWORD nItemUnderMouse = (DWORD)SendMessage(hwnd, LB_ITEMFROMPOINT, 0, lParam);
				if (HIWORD(nItemUnderMouse) == 1)
					nItemUnderMouse = (DWORD)(-1);
				else
					nItemUnderMouse &= 0xFFFF;

				ProcessNickListHovering(hwnd, (int)nItemUnderMouse, &pt, parentdat);
			} else
				ProcessNickListHovering(hwnd, -1, &pt, NULL);
		}
		break;
	}
	return CallWindowProc(OldNicklistProc, hwnd, msg, wParam, lParam);
}

/*
 * calculate the required rectangle for a string using the given font. This is more
 * precise than using GetTextExtentPoint...()
 */

int GetTextPixelSize(TCHAR* pszText, HFONT hFont, BOOL bWidth)
{
	HDC hdc;
	HFONT hOldFont;
	RECT rc = {0};
	int i;

	if (!pszText || !hFont)
		return 0;

	hdc = GetDC(NULL);
	hOldFont = (HFONT)SelectObject(hdc, hFont);
	i = DrawText(hdc, pszText , -1, &rc, DT_CALCRECT);
	SelectObject(hdc, hOldFont);
	ReleaseDC(NULL, hdc);
	return bWidth ? rc.right - rc.left : rc.bottom - rc.top;
}

static void __cdecl phase2(void * lParam)
{
	SESSION_INFO* si = (SESSION_INFO*) lParam;
	Sleep(30);
	if (si && si->hWnd)
		PostMessage(si->hWnd, GC_REDRAWLOG3, 0, 0);
}


/*
 * the actual group chat session window procedure. Handles the entire chat session window
 * which is usually a (tabbed) child of a container class window.
 */

INT_PTR CALLBACK RoomWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SESSION_INFO * si = NULL;
	HWND hwndTab = GetParent(hwndDlg);
	struct TWindowData *dat = (struct TWindowData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	if (dat)
		si = (SESSION_INFO *)dat->si;

	if (dat == NULL && (uMsg == WM_ACTIVATE || uMsg == WM_SETFOCUS))
		return 0;

	switch (uMsg) {
		case WM_INITDIALOG: {
			int mask;
			struct TNewWindowData *newData = (struct TNewWindowData *) lParam;
			struct TWindowData *dat;
			SESSION_INFO *psi = (SESSION_INFO*)newData->hdbEvent;
			RECT rc;

			dat = (struct TWindowData *)malloc(sizeof(struct TWindowData));
			ZeroMemory(dat, sizeof(struct TWindowData));
			si = psi;
			dat->si = psi;
			dat->hContact = psi->hContact;
			dat->szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)psi->hContact, 0);
			dat->bType = SESSIONTYPE_CHAT;
			dat->Panel = new CInfoPanel(dat);

			dat->cache = CContactCache::getContactCache(dat->hContact);
			dat->cache->updateState();
			dat->cache->updateUIN();
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			newData->item.lParam = (LPARAM) hwndDlg;
			TabCtrl_SetItem(hwndTab, newData->iTabID, &newData->item);
			dat->iTabID = newData->iTabID;
			dat->pContainer = newData->pContainer;
			psi->pContainer = newData->pContainer;
			dat->hwnd = hwndDlg;
			psi->hWnd = hwndDlg;
			psi->dat = dat;
			dat->fIsAutosizingInput = IsAutoSplitEnabled(dat);
			dat->fLimitedUpdate = false;
			dat->iInputAreaHeight = -1;
			if(!dat->pContainer->settings->fPrivate)
				psi->iSplitterY = g_Settings.iSplitterY;
			else {
				if(M->GetByte("Chat", "SyncSplitter", 0))
					psi->iSplitterY = dat->pContainer->settings->splitterPos - DPISCALEY_S(23);
				else
					psi->iSplitterY = g_Settings.iSplitterY;
			}
#if defined(__FEAT_EXP_AUTOSPLITTER)
			if(dat->fIsAutosizingInput)
				psi->iSplitterY = GetDefaultMinimumInputHeight(dat);
#endif
			dat->pWnd = 0;
			CProxyWindow::add(dat);

			dat->fInsertMode = FALSE;

			dat->codePage = M->GetDword(dat->hContact, "ANSIcodepage", CP_ACP);
			dat->Panel->getVisibility();
			dat->Panel->Configure();
			M->AddWindow(hwndDlg, dat->hContact);
			BroadCastContainer(dat->pContainer, DM_REFRESHTABINDEX, 0, 0);

			SendDlgItemMessage(hwndDlg, IDC_CHAT_LOG, EM_SETOLECALLBACK, 0, (LPARAM) mREOLECallback);

			BB_InitDlgButtons(dat);
			DM_InitTip(dat);

			SendMessage(GetDlgItem(hwndDlg,IDC_COLOR), BUTTONSETASPUSHBTN, 0, 0);

			OldSplitterProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_SPLITTERX), GWLP_WNDPROC, (LONG_PTR)SplitterSubclassProc);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_SPLITTERY), GWLP_WNDPROC, (LONG_PTR)SplitterSubclassProc);
			OldNicklistProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_LIST), GWLP_WNDPROC, (LONG_PTR)NicklistSubclassProc);
			OldLogProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CHAT_LOG), GWLP_WNDPROC, (LONG_PTR)LogSubclassProc);
			OldFilterButtonProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_FILTER), GWLP_WNDPROC, (LONG_PTR)ButtonSubclassProc);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_COLOR), GWLP_WNDPROC, (LONG_PTR)ButtonSubclassProc);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_BKGCOLOR), GWLP_WNDPROC, (LONG_PTR)ButtonSubclassProc);
			OldMessageProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), GWLP_WNDPROC, (LONG_PTR)MessageSubclassProc);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SUBCLASSED, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_LOG, EM_AUTOURLDETECT, 1, 0);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_PANELSPLITTER), GWLP_WNDPROC, (LONG_PTR) SplitterSubclassProc);
			TABSRMM_FireEvent(dat->hContact, hwndDlg, MSG_WINDOW_EVT_OPENING, 0);

			mask = (int)SendDlgItemMessage(hwndDlg, IDC_CHAT_LOG, EM_GETEVENTMASK, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_LOG, EM_SETEVENTMASK, 0, mask | ENM_LINK | ENM_MOUSEEVENTS | ENM_KEYEVENTS);

#if defined(__FEAT_EXP_AUTOSPLITTER)
			SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETEVENTMASK, 0, ENM_REQUESTRESIZE | ENM_MOUSEEVENTS | ENM_SCROLL | ENM_KEYEVENTS | ENM_CHANGE);
#else
			SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_SCROLL | ENM_KEYEVENTS | ENM_CHANGE);
#endif

			SendDlgItemMessage(hwndDlg, IDC_CHAT_LOG, EM_LIMITTEXT, (WPARAM)0x7FFFFFFF, 0);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(3, 3));
			SendDlgItemMessage(hwndDlg, IDC_CHAT_LOG, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(3, 3));
			dat->Panel->loadHeight();
			Utils::enableDlgControl(hwndDlg, IDC_SMILEYBTN, TRUE);

			if (PluginConfig.g_hMenuTrayUnread != 0 && dat->hContact != 0 && dat->szProto != NULL)
				UpdateTrayMenu(0, dat->wStatus, dat->szProto, dat->szStatus, dat->hContact, FALSE);

			DM_ThemeChanged(dat);
			SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), EM_HIDESELECTION, TRUE, 0);

			CreateWindowEx(0, _T("TSButtonClass"), _T(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 6, DPISCALEY_S(20),
					hwndDlg, (HMENU)IDC_CHAT_TOGGLESIDEBAR, g_hInst, NULL);

			GetMYUIN(dat);
			GetMyNick(dat);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_TOGGLESIDEBAR, BUTTONSETASFLATBTN + 10, 0, PluginConfig.m_bIsXP);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_TOGGLESIDEBAR, BUTTONSETASFLATBTN + 12, 0, (LPARAM)dat->pContainer);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_TOGGLESIDEBAR, BUTTONSETASFLATBTN, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_TOGGLESIDEBAR, BUTTONSETASTOOLBARBUTTON, 0, 1);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_TOGGLESIDEBAR, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Expand or collapse the side bar"), 1);

			dat->hwndIEView = dat->hwndHPP = 0;

			//Chat_SetMessageLog(dat);

			SendMessage(hwndDlg, GC_SETWNDPROPS, 0, 0);
			SendMessage(hwndDlg, GC_UPDATESTATUSBAR, 0, 0);
			SendMessage(hwndDlg, GC_UPDATETITLE, 0, 1);
			SendMessage(dat->pContainer->hwnd, DM_QUERYCLIENTAREA, 0, (LPARAM)&rc);
			SetWindowPos(hwndDlg, HWND_TOP, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), 0);
			ShowWindow(hwndDlg, SW_SHOW);
			PostMessage(hwndDlg, GC_UPDATENICKLIST, 0, 0);
			dat->pContainer->hwndActive = hwndDlg;
			BB_SetButtonsPos(dat);
			TABSRMM_FireEvent(dat->hContact, hwndDlg, MSG_WINDOW_EVT_OPEN, 0);
		}
		break;

		case WM_SETFOCUS:
			if (CMimAPI::m_shutDown)
				break;

			Chat_UpdateWindowState(dat, WM_SETFOCUS);
			SetFocus(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));
			return 1;

		case WM_TIMECHANGE:
			PostMessage(hwndDlg, GC_REDRAWLOG, 0, 0);
			break;

		case DM_LOADBUTTONBARICONS: {
			BB_UpdateIcons(hwndDlg, dat);
			return 0;
		}

		case GC_SETWNDPROPS: {
			//HICON hIcon;
			COLORREF colour = M->GetDword(FONTMODULE, SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR);
			InitButtons(hwndDlg, si);
			ConfigureSmileyButton(dat);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_LOG, EM_SETBKGNDCOLOR, 0, colour);

			DM_InitRichEdit(dat);
			SendDlgItemMessage(hwndDlg, IDOK, BUTTONSETASFLATBTN + 14, 0, 0);
			{
				SendMessage(GetDlgItem(hwndDlg, IDC_LIST), LB_SETITEMHEIGHT, 0, (LPARAM)g_Settings.iNickListFontHeight);
				InvalidateRect(GetDlgItem(hwndDlg, IDC_LIST), NULL, TRUE);
			}
			SendDlgItemMessage(hwndDlg, IDC_FILTER, BUTTONSETOVERLAYICON, 0,
							   (LPARAM)(si->bFilterEnabled ? PluginConfig.g_iconOverlayEnabled : PluginConfig.g_iconOverlayDisabled));
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
			SendMessage(hwndDlg, GC_REDRAWLOG2, 0, 0);
		}
		break;

		case DM_UPDATETITLE:
			return(SendMessage(hwndDlg, GC_UPDATETITLE, wParam, lParam));

		case GC_UPDATETITLE: {
			TCHAR 			szTemp [100];
			HICON 			hIcon;
			BOOL 			fNoCopy = TRUE;
			const TCHAR*	szNick = dat->cache->getNick();

			if(dat->bWasDeleted)
				return(0);

			dat->wStatus = si->wStatus;

			if (lstrlen(szNick) > 0) {
				if (M->GetByte("cuttitle", 0))
					CutContactName(szNick, dat->newtitle, safe_sizeof(dat->newtitle));
				else {
					lstrcpyn(dat->newtitle, szNick, safe_sizeof(dat->newtitle));
					dat->newtitle[129] = 0;
				}
			}

			switch (si->iType) {
 				case GCW_CHATROOM:
 					hIcon = dat->wStatus <= ID_STATUS_OFFLINE ? LoadSkinnedProtoIcon(si->pszModule, ID_STATUS_OFFLINE) : LoadSkinnedProtoIcon(si->pszModule, dat->wStatus);
 					fNoCopy = FALSE;
 					mir_sntprintf(szTemp, SIZEOF(szTemp),
 								  (si->nUsersInNicklist == 1) ? CTranslator::get(CTranslator::GEN_MUC_ROOM_TITLE_USER) :
								  CTranslator::get(CTranslator::GEN_MUC_ROOM_TITLE_USERS),
 								  si->ptszName, si->nUsersInNicklist, si->bFilterEnabled ? CTranslator::get(CTranslator::GEN_MUC_ROOM_TITLE_FILTER) : _T(""));
 					break;
				case GCW_PRIVMESS:
					mir_sntprintf(szTemp, SIZEOF(szTemp),
								  (si->nUsersInNicklist == 1) ? CTranslator::get(CTranslator::GEN_MUC_PRIVSESSION) :
								  CTranslator::get(CTranslator::GEN_MUC_PRIVSESSION_MULTI), si->ptszName, si->nUsersInNicklist);
					break;
				case GCW_SERVER:
					mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%s: Server"), si->ptszName);
					hIcon = LoadIconEx(IDI_CHANMGR, "window", 16, 16);
					break;
			}

			if(dat->pWnd) {
				dat->pWnd->updateTitle(dat->newtitle);
				dat->pWnd->updateIcon(hIcon);
			}
			dat->hTabStatusIcon = hIcon;

			if (lParam)
				dat->hTabIcon = dat->hTabStatusIcon;

			if (dat->cache->getStatus() != dat->cache->getOldStatus()) {
				TCITEM item;

				ZeroMemory(&item, sizeof(item));
				item.mask = TCIF_TEXT;

				lstrcpyn(dat->szStatus, (TCHAR *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)dat->wStatus, GSMDF_TCHAR), 50);
				dat->szStatus[49] = 0;
				item.pszText = dat->newtitle;
				item.cchTextMax = 120;
				TabCtrl_SetItem(hwndTab, dat->iTabID, &item);
			}
			SetWindowText(hwndDlg, szTemp);
			if (dat->pContainer->hwndActive == hwndDlg) {
				SendMessage(dat->pContainer->hwnd, DM_UPDATETITLE, (WPARAM)hwndDlg, 1);
				SendMessage(hwndDlg, GC_UPDATESTATUSBAR, 0, 0);
			}
		}
		break;

		case GC_UPDATESTATUSBAR:
			if(dat->bWasDeleted)
				return(0);

			if (dat->pContainer->hwndActive != hwndDlg || dat->pContainer->hwndStatus == 0 || CMimAPI::m_shutDown || dat->szStatusBar[0])
				break;
			if (si->pszModule != NULL) {
				TCHAR  szFinalStatusBarText[512];
				MODULEINFO* mi=NULL;
				int    x = 12;

				//Mad: strange rare crash here...
				mi = MM_FindModule(si->pszModule);
				if(!mi)
					break;

				if(!mi->ptszModDispName)
					break;

				x += GetTextPixelSize(mi->ptszModDispName, (HFONT)SendMessage(dat->pContainer->hwndStatus, WM_GETFONT, 0, 0), TRUE);
				x += GetSystemMetrics(SM_CXSMICON);

				if(dat->Panel->isActive()) {
					time_t now = time(0);
					DWORD diff = (now - mi->idleTimeStamp) / 60;

					if((diff >= 1 && diff != mi->lastIdleCheck) || lParam) {
						mi->lastIdleCheck = diff;
						if(diff == 0)
							mi->tszIdleMsg[0] = 0;
						else if(diff > 59) {
							DWORD hours = diff / 60;
							DWORD minutes = diff % 60;
							mir_sntprintf(mi->tszIdleMsg, 60, CTranslator::get(CTranslator::MUC_SBAR_IDLEFORMAT), hours, hours > 1 ?
										  CTranslator::get(CTranslator::GEN_STRING_HOURS) : CTranslator::get(CTranslator::GEN_STRING_HOUR),
										  minutes, minutes > 1 ? CTranslator::get(CTranslator::GEN_STRING_MINUTES) : CTranslator::get(CTranslator::GEN_STRING_MINUTE));
						}
						else
							mir_sntprintf(mi->tszIdleMsg, 60, CTranslator::get(CTranslator::MUC_SBAR_IDLEFORMAT_SHORT),
										  diff, diff > 1 ? CTranslator::get(CTranslator::GEN_STRING_MINUTES) : CTranslator::get(CTranslator::GEN_STRING_MINUTE));
					}
					mir_sntprintf(szFinalStatusBarText, SIZEOF(szFinalStatusBarText), CTranslator::get(CTranslator::MUC_SBAR_ON_SERVER), dat->szMyNickname, mi->ptszModDispName, mi->tszIdleMsg);
				} else {
					if (si->ptszStatusbarText)
						mir_sntprintf(szFinalStatusBarText, SIZEOF(szFinalStatusBarText), _T("%s %s"), mi->ptszModDispName, si->ptszStatusbarText);
					else {
						lstrcpyn(szFinalStatusBarText, mi->ptszModDispName, SIZEOF(szFinalStatusBarText));
						szFinalStatusBarText[511] = 0;
					}
				}
				SendMessage(dat->pContainer->hwndStatus, SB_SETTEXT, 0, (LPARAM)szFinalStatusBarText);
				UpdateStatusBar(dat);
				dat->Panel->Invalidate();
				if(dat->pWnd)
					dat->pWnd->Invalidate();
				return TRUE;
			}
			break;

		case WM_SIZE: {
			UTILRESIZEDIALOG 	urd;
			RECT				rc;
			int 				panelHeight = dat->Panel->getHeight() + 1;
			LONG				cx;

			if (dat->ipFieldHeight == 0)
				dat->ipFieldHeight = CInfoPanel::m_ipConfig.height1;

			if (wParam == SIZE_MAXIMIZED)
				PostMessage(hwndDlg, GC_SCROLLTOBOTTOM, 0, 0);

			if (IsIconic(hwndDlg)) break;
			ZeroMemory(&urd, sizeof(urd));
			urd.cbSize = sizeof(urd);
			urd.hInstance = g_hInst;
			urd.hwndDlg = hwndDlg;
			urd.lParam = (LPARAM)si;
			urd.lpTemplate = MAKEINTRESOURCEA(IDD_CHANNEL);
			urd.pfnResizer = RoomWndResize;
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);
			//mad
			BB_SetButtonsPos(dat);

			GetClientRect(hwndDlg, &rc);
			cx = rc.right;

			rc.left = panelHeight <= CInfoPanel::LEFT_OFFSET_LOGO ? panelHeight : CInfoPanel::LEFT_OFFSET_LOGO;
			rc.right = cx;
			rc.top = 1;
			rc.bottom = (panelHeight > CInfoPanel::DEGRADE_THRESHOLD ? rc.top + dat->ipFieldHeight - 2 : panelHeight - 1);
			dat->rcNick = rc;

			rc.left = panelHeight <= CInfoPanel::LEFT_OFFSET_LOGO ? panelHeight : CInfoPanel::LEFT_OFFSET_LOGO;
			rc.right = cx;
			rc.bottom = panelHeight - 2;
			rc.top = dat->rcNick.bottom + 1;
			dat->rcUIN = rc;

			if (dat->hwndIEView || dat->hwndHPP)
				Chat_ResizeIeView(dat);
			DetermineMinHeight(dat);
		}
		break;

		case GC_REDRAWWINDOW:
			InvalidateRect(hwndDlg, NULL, TRUE);
			break;

		case GC_REDRAWLOG:
			si->LastTime = 0;
			if (si->pLog) {
				LOGINFO * pLog = si->pLog;
				if (si->iEventCount > 60) {
					int index = 0;
					while (index < 59) {
						if (pLog->next == NULL)
							break;
						pLog = pLog->next;
						if (si->iType != GCW_CHATROOM || !si->bFilterEnabled || (si->iLogFilterFlags&pLog->iType) != 0)
							index++;
					}
					Log_StreamInEvent(hwndDlg, pLog, si, TRUE, FALSE);
					mir_forkthread(phase2, si);
				} else Log_StreamInEvent(hwndDlg, si->pLogEnd, si, TRUE, FALSE);
			} else SendMessage(hwndDlg, GC_EVENT_CONTROL + WM_USER + 500, WINDOW_CLEARLOG, 0);
			break;

		case GC_REDRAWLOG2:
			si->LastTime = 0;
			if (si->pLog)
				Log_StreamInEvent(hwndDlg, si->pLogEnd, si, TRUE, FALSE);
			break;

		case GC_REDRAWLOG3:
			si->LastTime = 0;
			if (si->pLog)
				Log_StreamInEvent(hwndDlg, si->pLogEnd, si, TRUE, TRUE);
			break;

		case GC_ADDLOG: {
			BOOL	fInactive = (GetForegroundWindow() != dat->pContainer->hwnd || GetActiveWindow() != dat->pContainer->hwnd);

			if (g_Settings.UseDividers && g_Settings.DividersUsePopupConfig) {
				if (!MessageWindowOpened(0, (LPARAM)hwndDlg))
					SendMessage(hwndDlg, DM_ADDDIVIDER, 0, 0);
			} else if (g_Settings.UseDividers) {
				if (fInactive)
					SendMessage(hwndDlg, DM_ADDDIVIDER, 0, 0);
				else {
					if (dat->pContainer->hwndActive != hwndDlg)
						SendMessage(hwndDlg, DM_ADDDIVIDER, 0, 0);
				}
			}

			if (si->pLogEnd)
				Log_StreamInEvent(hwndDlg, si->pLog, si, FALSE, FALSE);
			else
				SendMessage(hwndDlg, GC_EVENT_CONTROL + WM_USER + 500, WINDOW_CLEARLOG, 0);
			break;
		}

		case GC_ACKMESSAGE:
			SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETREADONLY, FALSE, 0);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, WM_SETTEXT, 0, (LPARAM)_T(""));
			return TRUE;

		case WM_CTLCOLORLISTBOX:
			SetBkColor((HDC) wParam, g_Settings.crUserListBGColor);
			return (INT_PTR) hListBkgBrush;

		case WM_MEASUREITEM: {
			MEASUREITEMSTRUCT *mis = (MEASUREITEMSTRUCT *) lParam;

			if (mis->CtlType == ODT_MENU) {
				if(dat->Panel->isHovered()) {
					mis->itemHeight = 0;
					mis->itemWidth  = 6;
					return(TRUE);
				}
				return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);
			}
			else
				mis->itemHeight = g_Settings.iNickListFontHeight;
			return TRUE;
		}

		case WM_DRAWITEM: {
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *) lParam;

			if (dis->CtlType == ODT_MENU) {
				if(dat->Panel->isHovered()) {
					DrawMenuItem(dis, (HICON)dis->itemData, 0);
					return(TRUE);
				}
				return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
			}
			else {
				if (dis->CtlID == IDC_LIST) {
					HFONT  hFont, hOldFont;
					HICON  hIcon;
					int offset, x_offset = 0;
					int height;
					int index = dis->itemID;
					USERINFO * ui = UM_FindUserFromIndex(si->pUsers, index);
					char szIndicator = 0;

					if (ui) {
						height = dis->rcItem.bottom - dis->rcItem.top;

						if (height&1)
							height++;
						if (height == 10)
							offset = 0;
						else
							offset = height / 2;
						hIcon = SM_GetStatusIcon(si, ui, &szIndicator);
						hFont = (ui->iStatusEx == 0) ? g_Settings.UserListFont : g_Settings.UserListHeadingsFont;
						hOldFont = (HFONT) SelectObject(dis->hDC, hFont);
						SetBkMode(dis->hDC, TRANSPARENT);

						if (dis->itemState & ODS_SELECTED) {
							FillRect(dis->hDC, &dis->rcItem, g_Settings.SelectionBGBrush);
							SetTextColor(dis->hDC, g_Settings.nickColors[6]);
						} else {
							FillRect(dis->hDC, &dis->rcItem, hListBkgBrush);
							if (g_Settings.ColorizeNicks && szIndicator != 0) {
								COLORREF clr;

								switch (szIndicator) {
									case '@':
										clr = g_Settings.nickColors[0];
										break;
									case '%':
										clr = g_Settings.nickColors[1];
										break;
									case '+':
										clr = g_Settings.nickColors[2];
										break;
									case '!':
										clr = g_Settings.nickColors[3];
										break;
									case '*':
										clr = g_Settings.nickColors[4];
										break;
								}
								SetTextColor(dis->hDC, clr);
							} else SetTextColor(dis->hDC, ui->iStatusEx == 0 ? g_Settings.crUserListColor : g_Settings.crUserListHeadingsColor);
						}
						x_offset = 2;

						if (g_Settings.ShowContactStatus && g_Settings.ContactStatusFirst && ui->ContactStatus) {
							HICON hIcon = LoadSkinnedProtoIcon(si->pszModule, ui->ContactStatus);
							DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset - 8, hIcon, 16, 16, 0, NULL, DI_NORMAL);
							CallService(MS_SKIN2_RELEASEICON, (WPARAM)hIcon, 0);
							x_offset += 18;
						}

						if (g_Settings.ClassicIndicators) {
							char szTemp[3];
							SIZE szUmode;

							szTemp[1] = 0;
							szTemp[0] = szIndicator;
							if (szTemp[0]) {
								GetTextExtentPoint32A(dis->hDC, szTemp, 1, &szUmode);
								TextOutA(dis->hDC, x_offset, dis->rcItem.top, szTemp, 1);
								x_offset += szUmode.cx + 2;
							} else x_offset += 8;
						} else {
							DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset - 5, hIcon, 10, 10, 0, NULL, DI_NORMAL);
							x_offset += 12;
						}

						if (g_Settings.ShowContactStatus && !g_Settings.ContactStatusFirst && ui->ContactStatus) {
							HICON hIcon = LoadSkinnedProtoIcon(si->pszModule, ui->ContactStatus);
							DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset - 8, hIcon, 16, 16, 0, NULL, DI_NORMAL);
							CallService(MS_SKIN2_RELEASEICON, (WPARAM)hIcon, 0);
							x_offset += 18;
						}

						{
							SIZE sz;

							if (si->iSearchItem != -1 && si->iSearchItem == index && si->szSearch[0]) {
								COLORREF clr_orig = GetTextColor(dis->hDC);
								GetTextExtentPoint32(dis->hDC, ui->pszNick, lstrlen(si->szSearch), &sz);
								SetTextColor(dis->hDC, RGB(250, 250, 0));
								TextOut(dis->hDC, x_offset, (dis->rcItem.top + dis->rcItem.bottom - sz.cy) / 2, ui->pszNick, lstrlen(si->szSearch));
								SetTextColor(dis->hDC, clr_orig);
								x_offset += sz.cx;
								TextOut(dis->hDC, x_offset, (dis->rcItem.top + dis->rcItem.bottom - sz.cy) / 2, ui->pszNick + lstrlen(si->szSearch), lstrlen(ui->pszNick) - lstrlen(si->szSearch));
							} else {
								GetTextExtentPoint32(dis->hDC, ui->pszNick, lstrlen(ui->pszNick), &sz);
								TextOut(dis->hDC, x_offset, (dis->rcItem.top + dis->rcItem.bottom - sz.cy) / 2, ui->pszNick, lstrlen(ui->pszNick));
								SelectObject(dis->hDC, hOldFont);
							}
						}
					}
					return TRUE;
				}
			}
		}
		break;
		case WM_CONTEXTMENU:{
			//mad
			DWORD idFrom=GetDlgCtrlID((HWND)wParam);
			if(idFrom>=MIN_CBUTTONID&&idFrom<=MAX_CBUTTONID)
				BB_CustomButtonClick(dat,idFrom,(HWND) wParam,1);
			}break;
			//
		case GC_UPDATENICKLIST: {
			int i = SendMessage(GetDlgItem(hwndDlg, IDC_LIST), LB_GETTOPINDEX, 0, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_LIST), LB_SETCOUNT, si->nUsersInNicklist, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_LIST), LB_SETTOPINDEX, i, 0);
			SendMessage(hwndDlg, GC_UPDATETITLE, 0, 0);
		}
		break;

		case GC_EVENT_CONTROL + WM_USER + 500: {
			switch (wParam) {
				case SESSION_OFFLINE:
					SendMessage(hwndDlg, GC_UPDATESTATUSBAR, 0, 0);
					SendMessage(si->hWnd, GC_UPDATENICKLIST, (WPARAM)0, (LPARAM)0);
					return TRUE;

				case SESSION_ONLINE:
					SendMessage(hwndDlg, GC_UPDATESTATUSBAR, 0, 0);
					return TRUE;

				case WINDOW_HIDDEN:
					SendMessage(hwndDlg, GC_CLOSEWINDOW, 0, 1);
					return TRUE;

				case WINDOW_CLEARLOG:
					SetDlgItemText(hwndDlg, IDC_CHAT_LOG, _T(""));
					return TRUE;

				case SESSION_TERMINATE:
					if (CallService(MS_CLIST_GETEVENT, (WPARAM)si->hContact, (LPARAM)0))
						CallService(MS_CLIST_REMOVEEVENT, (WPARAM)si->hContact, (LPARAM)szChatIconString);

					si->wState &= ~STATE_TALK;
					dat->bWasDeleted = 1;
					DBWriteContactSettingWord(si->hContact, si->pszModule , "ApparentMode", (LPARAM) 0);
					SendMessage(hwndDlg, GC_CLOSEWINDOW, 0, lParam == 2 ? lParam : 1);
					return TRUE;

				case WINDOW_MINIMIZE:
					ShowWindow(hwndDlg, SW_MINIMIZE);
					goto LABEL_SHOWWINDOW;

				case WINDOW_MAXIMIZE:
					ShowWindow(hwndDlg, SW_MAXIMIZE);
					goto LABEL_SHOWWINDOW;

				case SESSION_INITDONE:
					if (M->GetByte("Chat", "PopupOnJoin", 0) != 0)
						return TRUE;
					// fall through
				case WINDOW_VISIBLE:
					if (IsIconic(hwndDlg))
						ShowWindow(hwndDlg, SW_NORMAL);
LABEL_SHOWWINDOW:
					SendMessage(hwndDlg, WM_SIZE, 0, 0);
					SendMessage(hwndDlg, GC_REDRAWLOG, 0, 0);
					SendMessage(hwndDlg, GC_UPDATENICKLIST, 0, 0);
					SendMessage(hwndDlg, GC_UPDATESTATUSBAR, 0, 0);
					ShowWindow(hwndDlg, SW_SHOW);
					SendMessage(hwndDlg, WM_SIZE, 0, 0);
					SetForegroundWindow(hwndDlg);
					return TRUE;
			}
		}
		break;

		case DM_SPLITTERMOVED: {
			POINT pt;
			RECT rc;
			RECT rcLog;
			BOOL bFormat = TRUE; //IsWindowVisible(GetDlgItem(hwndDlg,IDC_SMILEY));

			static int x = 0;

			GetWindowRect(GetDlgItem(hwndDlg, IDC_CHAT_LOG), &rcLog);
			if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_SPLITTERX)) {
				int oldSplitterX;
				GetClientRect(hwndDlg, &rc);
				pt.x = wParam;
				pt.y = 0;
				ScreenToClient(hwndDlg, &pt);

				oldSplitterX = si->iSplitterX;
				si->iSplitterX = rc.right - pt.x + 1;
				if (si->iSplitterX < 35)
					si->iSplitterX = 35;
				if (si->iSplitterX > rc.right - rc.left - 35)
					si->iSplitterX = rc.right - rc.left - 35;
				g_Settings.iSplitterX = si->iSplitterX;
				SendMessage(dat->hwnd, WM_SIZE, 0, 0);
			} else if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_SPLITTERY) || lParam == -1) {
				int oldSplitterY;
				GetClientRect(hwndDlg, &rc);
				rc.top += (dat->Panel->isActive() ? dat->Panel->getHeight() + 40 : 30);
				pt.x = 0;
				pt.y = wParam;
				ScreenToClient(hwndDlg, &pt);

				oldSplitterY = si->iSplitterY;
				si->iSplitterY = bFormat ? rc.bottom - pt.y + DPISCALEY_S(1) : rc.bottom - pt.y + DPISCALEY_S(20);
				if (si->iSplitterY < DPISCALEY_S(23))
					si->iSplitterY = DPISCALEY_S(23);
				if (si->iSplitterY > rc.bottom - rc.top - DPISCALEY_S(40))
					si->iSplitterY = rc.bottom - rc.top - DPISCALEY_S(40);
				g_Settings.iSplitterY = si->iSplitterY;
				CSkin::UpdateToolbarBG(dat, RDW_ALLCHILDREN);
				SendMessage(dat->hwnd, WM_SIZE, 0, 0);
			} else if ((HWND) lParam == GetDlgItem(hwndDlg, IDC_PANELSPLITTER)) {
				RECT rc;
				POINT pt;
				pt.x = 0;
				pt.y = wParam;
				ScreenToClient(hwndDlg, &pt);
				GetClientRect(GetDlgItem(hwndDlg, IDC_CHAT_LOG), &rc);
				if ((pt.y + 2 >= MIN_PANELHEIGHT + 2) && (pt.y + 2 < 100) && (pt.y + 2 < rc.bottom - 30))
					dat->Panel->setHeight(pt.y + 2);
				dat->panelWidth = -1;
				RedrawWindow(hwndDlg, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
				if(M->isAero())
					InvalidateRect(GetParent(hwndDlg), NULL, FALSE);
				SendMessage(hwndDlg, WM_SIZE, DM_SPLITTERMOVED, 0);
				break;
			}
		}
		break;

		case GC_FIREHOOK:
			if (lParam) {
				GCHOOK* gch = (GCHOOK *) lParam;
				NotifyEventHooks(hSendEvent, 0, (WPARAM)gch);
				if (gch->pDest) {
					mir_free(gch->pDest->pszID);
					mir_free(gch->pDest->pszModule);
					mir_free(gch->pDest);
				}
				mir_free(gch->ptszText);
				mir_free(gch->ptszUID);
				mir_free(gch);
			}
			break;

		case GC_CHANGEFILTERFLAG:
			if (si->iLogFilterFlags == 0 && si->bFilterEnabled)
				SendMessage(hwndDlg, WM_COMMAND, IDC_FILTER, 0);
			break;

		case GC_SHOWFILTERMENU: {
			RECT  rcFilter, rcLog;
			POINT pt;

			si->hwndFilter = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_FILTER), dat->pContainer->hwnd, FilterWndProc, (LPARAM)si);
			TranslateDialogDefault(si->hwndFilter);

			GetClientRect(si->hwndFilter, &rcFilter);
			GetWindowRect(GetDlgItem(hwndDlg, IDC_CHAT_LOG), &rcLog);
			pt.x = rcLog.right;
			pt.y = rcLog.bottom;
			ScreenToClient(dat->pContainer->hwnd, &pt);

			SetWindowPos(si->hwndFilter, HWND_TOP, pt.x - rcFilter.right, pt.y - rcFilter.bottom, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		}
		break;

		case DM_SPLITTERGLOBALEVENT: {
			DM_SplitterGlobalEvent(dat, wParam, lParam);
			return(0);
		}

		case GC_SHOWCOLORCHOOSER: {
			HWND ColorWindow;
			RECT rc;
			BOOL bFG = lParam == IDC_COLOR ? TRUE : FALSE;
			COLORCHOOSER *pCC = (COLORCHOOSER *)mir_alloc(sizeof(COLORCHOOSER));

			GetWindowRect(GetDlgItem(hwndDlg, bFG ? IDC_COLOR : IDC_BKGCOLOR), &rc);
			pCC->hWndTarget = GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE);
			pCC->pModule = MM_FindModule(si->pszModule);
			pCC->xPosition = rc.left + 3;
			pCC->yPosition = IsWindowVisible(GetDlgItem(hwndDlg, IDC_COLOR)) ? rc.top - 1 : rc.top + 20;
			pCC->bForeground = bFG;
			pCC->si = si;

			ColorWindow = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_COLORCHOOSER), hwndDlg, DlgProcColorToolWindow, (LPARAM) pCC);
		}
		break;

		case GC_SCROLLTOBOTTOM: {
			return(DM_ScrollToBottom(dat, wParam, lParam));
		}

		case WM_TIMER:
			if (wParam == TIMERID_FLASHWND)
				if (dat->mayFlashTab)
					FlashTab(dat, hwndTab, dat->iTabID, &dat->bTabFlash, TRUE, dat->hTabIcon);
			break;

		case WM_ACTIVATE:
			if (LOWORD(wParam) != WA_ACTIVE) {
				dat->pContainer->hwndSaved = 0;
				break;
			}

			//fall through
		case WM_MOUSEACTIVATE:
			Chat_UpdateWindowState(dat, WM_ACTIVATE);
			return 1;

		case WM_NOTIFY: {
			LPNMHDR pNmhdr = (LPNMHDR)lParam;
			switch (pNmhdr->code) {
				case EN_MSGFILTER: {
					UINT  msg = ((MSGFILTER *) lParam)->msg;
					WPARAM wp = ((MSGFILTER *) lParam)->wParam;
					LPARAM lp = ((MSGFILTER *) lParam)->lParam;

					BOOL isShift, isCtrl, isMenu;
					KbdState(dat, isShift, isCtrl, isMenu);

					MSG		message;
					message.hwnd = hwndDlg;
					message.message = msg;
					message.lParam = lp;
					message.wParam = wp;

					if(msg == WM_SYSKEYUP) {
						if(wp == VK_MENU) {
							if(!dat->fkeyProcessed && !(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_SHIFT) & 0x8000) && !(lp & (1 << 24)))
								dat->pContainer->MenuBar->autoShow();
						}
						return(_dlgReturn(hwndDlg, 0));
					}

					if(msg == WM_MOUSEMOVE) {
						POINT	pt;
						GetCursorPos(&pt);
						DM_DismissTip(dat, pt);
						dat->Panel->trackMouse(pt);
						break;
					}
					if(msg == WM_KEYDOWN) {
						if ((wp == VK_INSERT && isShift && !isCtrl && !isMenu) || (wp == 'V' && !isShift && !isMenu && isCtrl)) {
							SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), EM_PASTESPECIAL, CF_TEXTT, 0);
							((MSGFILTER *) lParam)->msg = WM_NULL;
							((MSGFILTER *) lParam)->wParam = 0;
							((MSGFILTER *) lParam)->lParam = 0;
							return(_dlgReturn(hwndDlg, 1));
						}
					}

					if(msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN)
						dat->pContainer->MenuBar->Cancel();

					if ((msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) && !(GetKeyState(VK_RMENU) & 0x8000)) {

						if(DM_GenericHotkeysCheck(&message, dat)) {
							dat->fkeyProcessed = true;
							return(_dlgReturn(hwndDlg, 1));
						}

						LRESULT mim_hotkey_check = CallService(MS_HOTKEY_CHECK, (WPARAM)&message, (LPARAM)(TABSRMM_HK_SECTION_GC));
						if(mim_hotkey_check)
							dat->fkeyProcessed = true;
						switch(mim_hotkey_check) {								// nothing (yet) FIXME
							case TABSRMM_HK_CHANNELMGR:
								SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_CHANMGR, BN_CLICKED), 0);
								return(_dlgReturn(hwndDlg, 1));
							case TABSRMM_HK_FILTERTOGGLE:
								SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_FILTER, BN_CLICKED), 0);
								InvalidateRect(GetDlgItem(hwndDlg, IDC_FILTER), NULL, TRUE);
								return(_dlgReturn(hwndDlg, 1));
							case TABSRMM_HK_LISTTOGGLE:
								SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_SHOWNICKLIST, BN_CLICKED), 0);
								return(_dlgReturn(hwndDlg, 1));
							case TABSRMM_HK_MUC_SHOWSERVER:
								if(si->iType != GCW_SERVER)
									DoEventHookAsync(hwndDlg, si->ptszID, si->pszModule, GC_USER_MESSAGE, NULL, L"/servershow", (LPARAM)NULL);
								return(_dlgReturn(hwndDlg, 1));
							default:
								break;
						}
					}

					if(msg == WM_KEYDOWN && wp == VK_TAB) {
						if(((NMHDR *)lParam)->idFrom == IDC_CHAT_LOG) {
							SetFocus(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));
							return(_dlgReturn(hwndDlg, 1));
						}
					}

					if (pNmhdr->idFrom == IDC_CHAT_LOG && ((MSGFILTER *) lParam)->msg == WM_RBUTTONUP) {
						CHARRANGE sel, all = { 0, -1 };
						POINT pt;
						UINT uID = 0;
						HMENU hMenu = 0;
						TCHAR pszWord[4096];
						int pos;

						pt.x = (short) LOWORD(((ENLINK *) lParam)->lParam);
						pt.y = (short) HIWORD(((ENLINK *) lParam)->lParam);
						ClientToScreen(pNmhdr->hwndFrom, &pt);

						{ // fixing stuff for searches
							long iCharIndex, iLineIndex, iChars, start, end, iRes;
							POINTL ptl;

							pszWord[0] = '\0';
							ptl.x = (LONG)pt.x;
							ptl.y = (LONG)pt.y;
							ScreenToClient(GetDlgItem(hwndDlg, IDC_CHAT_LOG), (LPPOINT)&ptl);
							iCharIndex = SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), EM_CHARFROMPOS, 0, (LPARAM) & ptl);
							if (iCharIndex < 0)
								break;
							iLineIndex = SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), EM_EXLINEFROMCHAR, 0, (LPARAM)iCharIndex);
							iChars = SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), EM_LINEINDEX, (WPARAM)iLineIndex, 0);
							start = SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), EM_FINDWORDBREAK, WB_LEFT, iCharIndex);//-iChars;
							end = SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), EM_FINDWORDBREAK, WB_RIGHT, iCharIndex);//-iChars;

							if (end - start > 0) {
								TEXTRANGE tr;
								CHARRANGE cr;
								static char szTrimString[] = ":;,.!?\'\"><()[]- \r\n";
								ZeroMemory(&tr, sizeof(TEXTRANGE));

								cr.cpMin = start;
								cr.cpMax = end;
								tr.chrg = cr;
								tr.lpstrText = (TCHAR *)pszWord;
								iRes = SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), EM_GETTEXTRANGE, 0, (LPARAM) & tr);

								if (iRes > 0) {
									int iLen = lstrlen(pszWord) - 1;
									while (iLen >= 0 && strchr(szTrimString, pszWord[iLen])) {
										pszWord[iLen] = '\0';
										iLen--;
									}
								}
							}
						}

						uID = CreateGCMenu(hwndDlg, &hMenu, 1, pt, si, NULL, pszWord);

						if ((uID > 800 && uID < 1400) || uID == CP_UTF8 || uID == 20866) {
							dat->codePage = uID;
							M->WriteDword(dat->hContact, SRMSGMOD_T, "ANSIcodepage", dat->codePage);
						} else if (uID == 500) {
							dat->codePage = CP_ACP;
							DBDeleteContactSetting(dat->hContact, SRMSGMOD_T, "ANSIcodepage");
						} else {
							switch (uID) {
								case 0:
									PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0);
									break;

								case ID_COPYALL:
									SendMessage(pNmhdr->hwndFrom, EM_EXGETSEL, 0, (LPARAM) & sel);
									SendMessage(pNmhdr->hwndFrom, EM_EXSETSEL, 0, (LPARAM) & all);
									SendMessage(pNmhdr->hwndFrom, WM_COPY, 0, 0);
									SendMessage(pNmhdr->hwndFrom, EM_EXSETSEL, 0, (LPARAM) & sel);
									PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0);
									break;

								case ID_CLEARLOG:
									ClearLog(dat);
									break;

								case ID_SEARCH_GOOGLE: {
									char szURL[4096];
									if (pszWord[0]) {
										mir_snprintf(szURL, sizeof(szURL), "http://www.google.com/search?q=" TCHAR_STR_PARAM, pszWord);
										CallService(MS_UTILS_OPENURL, 1, (LPARAM) szURL);
									}
									PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0);
								}
								break;

								case ID_SEARCH_WIKIPEDIA: {
									char szURL[4096];
									if (pszWord[0]) {
										mir_snprintf(szURL, sizeof(szURL), "http://en.wikipedia.org/wiki/" TCHAR_STR_PARAM, pszWord);
										CallService(MS_UTILS_OPENURL, 1, (LPARAM) szURL);
									}
									PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0);
								}
								break;

								default:
									PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0);
									DoEventHookAsync(hwndDlg, si->ptszID, si->pszModule, GC_USER_LOGMENU, NULL, NULL, (LPARAM)uID);
									break;
							}
						}
						if (si->iType != GCW_SERVER && !(si->dwFlags & GC_UNICODE)) {
							pos = GetMenuItemCount(hMenu);
							RemoveMenu(hMenu, pos - 1, MF_BYPOSITION);
							RemoveMenu(PluginConfig.g_hMenuEncoding, 1, MF_BYPOSITION);
						}
						DestroyGCMenu(&hMenu, 5);
					}
					break;
				}

				case EN_REQUESTRESIZE: {
					if(pNmhdr->idFrom == IDC_CHAT_MESSAGE) {
						REQRESIZE *rr = (REQRESIZE *)lParam;
						DM_HandleAutoSizeRequest(dat, rr);
					}
					break;
				}

				case EN_LINK:
					if (pNmhdr->idFrom == IDC_CHAT_LOG) {
						switch (((ENLINK *) lParam)->msg) {
							case WM_SETCURSOR:

								if (g_Settings.ClickableNicks) {
									if (!hCurHyperlinkHand)
										hCurHyperlinkHand = LoadCursor(NULL, IDC_HAND);
									if (hCurHyperlinkHand != GetCursor())
										SetCursor(hCurHyperlinkHand);
									return TRUE;
								}
								break;

							case WM_RBUTTONDOWN:
							case WM_LBUTTONUP:
							case WM_LBUTTONDBLCLK: {
								TEXTRANGE tr;
								CHARRANGE sel;
								BOOL isLink = FALSE;
								UINT msg = ((ENLINK *) lParam)->msg;

								dat->pContainer->MenuBar->Cancel();

								tr.lpstrText = NULL;
								SendMessage(pNmhdr->hwndFrom, EM_EXGETSEL, 0, (LPARAM) & sel);
								if (sel.cpMin != sel.cpMax)
									break;
								tr.chrg = ((ENLINK *) lParam)->chrg;
								tr.lpstrText = (TCHAR *)mir_alloc(sizeof(TCHAR) * (tr.chrg.cpMax - tr.chrg.cpMin + 2));
								SendMessage(pNmhdr->hwndFrom, EM_GETTEXTRANGE, 0, (LPARAM) & tr);

								isLink = IsStringValidLink(tr.lpstrText);

								if (isLink) {
									char* pszUrl = t2a(tr.lpstrText, 0);
									if (((ENLINK *) lParam)->msg == WM_RBUTTONDOWN) {
										HMENU hSubMenu;
										POINT pt;

										hSubMenu = GetSubMenu(g_hMenu, 2);
										CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM) hSubMenu, 0);
										pt.x = (short) LOWORD(((ENLINK *) lParam)->lParam);
										pt.y = (short) HIWORD(((ENLINK *) lParam)->lParam);
										ClientToScreen(((NMHDR *) lParam)->hwndFrom, &pt);
										switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL)) {
											case ID_NEW:
												CallService(MS_UTILS_OPENURL, 1, (LPARAM) pszUrl);
												break;
											case ID_CURR:
												CallService(MS_UTILS_OPENURL, 0, (LPARAM) pszUrl);
												break;
											case ID_COPY: {
												HGLOBAL hData;
												if (!OpenClipboard(hwndDlg))
													break;
												EmptyClipboard();
												hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(TCHAR) * (lstrlen(tr.lpstrText) + 1));
												lstrcpy((TCHAR*)GlobalLock(hData), tr.lpstrText);
												GlobalUnlock(hData);
												SetClipboardData(CF_UNICODETEXT, hData);
											}
											CloseClipboard();
											SetFocus(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));
											break;
										}
										mir_free(tr.lpstrText);
										mir_free(pszUrl);
										return TRUE;
									} else if (((ENLINK *) lParam)->msg == WM_LBUTTONUP) {
										CallService(MS_UTILS_OPENURL, 1, (LPARAM) pszUrl);
										SetFocus(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));
										mir_free(tr.lpstrText);
										mir_free(pszUrl);
										return TRUE;
									}
								} else if (g_Settings.ClickableNicks) {                    // clicked a nick name
									CHARRANGE chr;
									TEXTRANGE tr2;
									TCHAR tszAplTmpl[] = _T("%s:"),
														 *tszAppeal, *tszTmp;
									size_t st;

									if (msg == WM_RBUTTONDOWN) {
										USERINFO *ui = si->pUsers;
										HMENU     hMenu = 0;
										USERINFO  uiNew;
										while (ui) {
											if (!lstrcmp(ui->pszNick, tr.lpstrText)) {
												POINT pt;
												UINT  uID;

												pt.x = (short) LOWORD(((ENLINK *) lParam)->lParam);
												pt.y = (short) HIWORD(((ENLINK *) lParam)->lParam);
												ClientToScreen(((NMHDR *) lParam)->hwndFrom, &pt);
												CopyMemory(&uiNew, ui, sizeof(USERINFO));
												uID = CreateGCMenu(hwndDlg, &hMenu, 0, pt, si, uiNew.pszUID, NULL);
												switch (uID) {
													case 0:
														break;

													case ID_MESS:
														DoEventHookAsync(hwndDlg, si->ptszID, si->pszModule, GC_USER_PRIVMESS, ui->pszUID, NULL, (LPARAM)NULL);
														break;

													default:
														DoEventHookAsync(hwndDlg, si->ptszID, si->pszModule, GC_USER_NICKLISTMENU, ui->pszUID, NULL, (LPARAM)uID);
														break;
												}
												DestroyGCMenu(&hMenu, 1);
												return TRUE;
											}
											ui = ui->next;
										}
										return TRUE;
									}
									else if (msg == WM_LBUTTONUP) {
										USERINFO	*ui = si->pUsers;
											SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_EXGETSEL, 0, (LPARAM) &chr);
											tszTmp = tszAppeal = (TCHAR *) malloc((lstrlen(tr.lpstrText) + lstrlen(tszAplTmpl) + 3) * sizeof(TCHAR));
											tr2.lpstrText = (LPTSTR) malloc(sizeof(TCHAR) * 2);
											if (chr.cpMin) {
												/* prepend nick with space if needed */
												tr2.chrg.cpMin = chr.cpMin - 1;
												tr2.chrg.cpMax = chr.cpMin;
												SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_GETTEXTRANGE, 0, (LPARAM) &tr2);
												if (! _istspace(*tr2.lpstrText))
													*tszTmp++ = _T(' ');
												_tcscpy(tszTmp, tr.lpstrText);
											}
											else
												/* in the beginning of the message window */
												_stprintf(tszAppeal, tszAplTmpl, tr.lpstrText);
											st = lstrlen(tszAppeal);
											if (chr.cpMax != -1) {
												tr2.chrg.cpMin = chr.cpMax;
												tr2.chrg.cpMax = chr.cpMax + 1;
												/* if there is no space after selection,
												or there is nothing after selection at all... */
												if (! SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE,	EM_GETTEXTRANGE, 0, (LPARAM) &tr2) || ! _istspace(*tr2.lpstrText)) {
														tszAppeal[st++] = _T(' ');
														tszAppeal[st++] = _T('\0');
												}
											}
											else {
												tszAppeal[st++] = _T(' ');
												tszAppeal[st++] = _T('\0');
											}
											SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_REPLACESEL,  FALSE, (LPARAM)tszAppeal);
											free((void *) tr2.lpstrText);
											free((void *) tszAppeal);
									}
								}
								SetFocus(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));
								mir_free(tr.lpstrText);
								return TRUE;
							}
						}
						return TRUE;
					}
					return TRUE;
			}
		}
		break;

		case WM_LBUTTONDOWN: {
			POINT tmp; //+ Protogenes
			POINTS cur; //+ Protogenes
			GetCursorPos(&tmp); //+ Protogenes
			if(!dat->Panel->isHovered()) {
				cur.x = (SHORT)tmp.x; //+ Protogenes
				cur.y = (SHORT)tmp.y; //+ Protogenes
				SendMessage(dat->pContainer->hwnd, WM_NCLBUTTONDOWN, HTCAPTION, *((LPARAM*)(&cur))); //+ Protogenes
			}
			break;
		}

		case WM_LBUTTONUP: {
			POINT tmp; //+ Protogenes
			POINTS cur; //+ Protogenes
			GetCursorPos(&tmp); //+ Protogenes
			if(dat->Panel->isHovered())
				dat->Panel->handleClick(tmp);
			else {
				cur.x = (SHORT)tmp.x; //+ Protogenes
				cur.y = (SHORT)tmp.y; //+ Protogenes
				SendMessage(dat->pContainer->hwnd, WM_NCLBUTTONUP, HTCAPTION, *((LPARAM*)(&cur))); //+ Protogenes
			}
			break;
		}

		case WM_MOUSEMOVE: {
			POINT pt;
			GetCursorPos(&pt);
			DM_DismissTip(dat, pt);
			dat->Panel->trackMouse(pt);
			break;
		}
		case WM_APPCOMMAND: {
			DWORD cmd = GET_APPCOMMAND_LPARAM(lParam);
			if (cmd == APPCOMMAND_BROWSER_BACKWARD || cmd == APPCOMMAND_BROWSER_FORWARD) {
				SendMessage(dat->pContainer->hwnd, DM_SELECTTAB, cmd == APPCOMMAND_BROWSER_BACKWARD ? DM_SELECT_PREV : DM_SELECT_NEXT, 0);
				return 1;
			}
		}
		break;

		case WM_COMMAND:
			//mad
			if(LOWORD(wParam)>=MIN_CBUTTONID&&LOWORD(wParam)<=MAX_CBUTTONID){
				BB_CustomButtonClick(dat,LOWORD(wParam) ,GetDlgItem(hwndDlg,LOWORD(wParam)),0);
				break;
				}
			//
			switch (LOWORD(wParam)) {
				case IDC_LIST:
					if (HIWORD(wParam) == LBN_DBLCLK) {
						TVHITTESTINFO hti;
						int item;
						USERINFO * ui;

						hti.pt.x = (short)LOWORD(GetMessagePos());
						hti.pt.y = (short)HIWORD(GetMessagePos());
						ScreenToClient(GetDlgItem(hwndDlg, IDC_LIST), &hti.pt);

						item = LOWORD(SendMessage(GetDlgItem(hwndDlg, IDC_LIST), LB_ITEMFROMPOINT, 0, MAKELPARAM(hti.pt.x, hti.pt.y)));
						ui = UM_FindUserFromIndex(si->pUsers, item);
						//ui = SM_GetUserFromIndex(si->pszID, si->pszModule, item);
						if (ui) {
							if (g_Settings.DoubleClick4Privat ? GetKeyState(VK_SHIFT) & 0x8000 : !(GetKeyState(VK_SHIFT) & 0x8000)) {
								LRESULT lResult = (LRESULT)SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), EM_GETSEL, (WPARAM)NULL, (LPARAM)NULL);
								int start = LOWORD(lResult);
								TCHAR* pszName = (TCHAR*)alloca(sizeof(TCHAR) * (lstrlen(ui->pszUID) + 3));
								if (start == 0)
									mir_sntprintf(pszName, lstrlen(ui->pszUID) + 3, _T("%s: "), ui->pszUID);
								else
									mir_sntprintf(pszName, lstrlen(ui->pszUID) + 2, _T("%s "), ui->pszUID);

								SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), EM_REPLACESEL, FALSE, (LPARAM) pszName);
								PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0);
								SetFocus(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));
							} else DoEventHookAsync(hwndDlg, si->ptszID, si->pszModule, GC_USER_PRIVMESS, ui->pszUID, NULL, (LPARAM)NULL);
						}

						return TRUE;
					} else if (HIWORD(wParam) == LBN_KILLFOCUS)
						RedrawWindow(GetDlgItem(hwndDlg, IDC_LIST), NULL, NULL, RDW_INVALIDATE);
					break;

				case IDC_CHAT_TOGGLESIDEBAR:
					SendMessage(dat->pContainer->hwnd, WM_COMMAND, IDC_TOGGLESIDEBAR, 0);
					break;

				case IDCANCEL:
					ShowWindow(dat->pContainer->hwnd, SW_MINIMIZE);
					return FALSE;

				case IDOK: {
					char*  		pszRtf;
					TCHAR* 		ptszText/*, *p1*/;
					MODULEINFO*	mi;
					bool		fSound = true;

					if (GetSendButtonState(hwndDlg) == PBS_DISABLED)
						break;

					mi = MM_FindModule(si->pszModule);

					pszRtf = Chat_Message_GetFromStream(hwndDlg, si);
					SM_AddCommand(si->ptszID, si->pszModule, pszRtf);
					ptszText = Chat_DoRtfToTags(pszRtf, si);
					DoTrimMessage(ptszText);

					if(mi && mi->bAckMsg) {
						Utils::enableDlgControl(hwndDlg, IDC_CHAT_MESSAGE, FALSE);
						SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETREADONLY, TRUE, 0);
					} else SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, WM_SETTEXT, 0, (LPARAM)_T(""));

					Utils::enableDlgControl(hwndDlg, IDOK, FALSE);

					if(ptszText[0] == '/' || si->iType == GCW_SERVER)
						fSound = false;
					DoEventHookAsync(hwndDlg, si->ptszID, si->pszModule, GC_USER_MESSAGE, NULL, ptszText, (LPARAM)NULL);
					mi->idleTimeStamp = time(0);
					mi->lastIdleCheck = 0;
					SM_BroadcastMessage(si->pszModule, GC_UPDATESTATUSBAR, 0, 1, TRUE);
					if (dat && dat->pContainer) {
						if (fSound && !nen_options.iNoSounds && !(dat->pContainer->dwFlags & CNT_NOSOUND))
							SkinPlaySound("ChatSent");
					}
					mir_free(pszRtf);
					mir_free(ptszText);
					SetFocus(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));
				}
				break;

				case IDC_SHOWNICKLIST:
					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_SHOWNICKLIST)))
						break;
					if (si->iType == GCW_SERVER)
						break;

					si->bNicklistEnabled = !si->bNicklistEnabled;

					SendMessage(hwndDlg, WM_SIZE, 0, 0);
					if (CSkin::m_skinEnabled)
						InvalidateRect(hwndDlg, NULL, TRUE);
					PostMessage(hwndDlg, GC_SCROLLTOBOTTOM, 0, 0);
					break;

				case IDC_CHAT_MESSAGE:
					if (g_Settings.MathMod)
						MTH_updateMathWindow(dat);

					if (HIWORD(wParam) == EN_CHANGE) {
						if (dat->pContainer->hwndActive == hwndDlg)
							UpdateReadChars(dat);
						dat->dwLastActivity = GetTickCount();
						dat->pContainer->dwLastActivity = dat->dwLastActivity;
						SendDlgItemMessage(hwndDlg, IDOK, BUTTONSETASFLATBTN + 14, GetRichTextLength(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE)) != 0, 0);
						Utils::enableDlgControl(hwndDlg, IDOK, GetRichTextLength(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE)) != 0);
					}
					break;

				case IDC_SMILEY:
				case IDC_SMILEYBTN: {
					SMADD_SHOWSEL3 smaddInfo = {0};
					RECT rc;

					if (lParam == 0)
						GetWindowRect(GetDlgItem(hwndDlg, IDC_SMILEYBTN), &rc);
					else
						GetWindowRect((HWND)lParam, &rc);
					smaddInfo.cbSize = sizeof(SMADD_SHOWSEL3);
					smaddInfo.hwndTarget = GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE);
					smaddInfo.targetMessage = EM_REPLACESEL;
					smaddInfo.targetWParam = TRUE;
					smaddInfo.Protocolname = si->pszModule;
					smaddInfo.Direction = 0;
					smaddInfo.xPosition = rc.left;
					smaddInfo.yPosition = rc.top + 24;
					smaddInfo.hContact = si->hContact;
					smaddInfo.hwndParent = dat->pContainer->hwnd;
					if (PluginConfig.g_SmileyAddAvail)
						CallService(MS_SMILEYADD_SHOWSELECTION, 0, (LPARAM) &smaddInfo);
				}
				break;

				case IDC_CHAT_HISTORY: {
					MODULEINFO * pInfo = MM_FindModule(si->pszModule);

					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHAT_HISTORY)))
						break;

					if (ServiceExists("MSP/HTMLlog/ViewLog") && strstr(si->pszModule, "IRC")) {
						char szName[MAX_PATH];

						WideCharToMultiByte(CP_ACP, 0, si->ptszName, -1, szName, MAX_PATH, 0, 0);
						szName[MAX_PATH - 1] = 0;
						CallService("MSP/HTMLlog/ViewLog", (WPARAM)si->pszModule, (LPARAM)szName);
					} else if (pInfo)
						ShellExecute(hwndDlg, NULL, GetChatLogsFilename(si, 0), NULL, NULL, SW_SHOW);
				}
				break;

				case IDC_CHAT_CLOSE:
					SendMessage(hwndDlg, WM_CLOSE, 0, 1);
					break;

				case IDC_CHANMGR:
					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHANMGR)))
						break;
					DoEventHookAsync(hwndDlg, si->ptszID, si->pszModule, GC_USER_CHANMGR, NULL, NULL, (LPARAM)NULL);
					break;

				case IDC_FILTER:
					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_FILTER)))
						break;

					if (si->iLogFilterFlags == 0 && !si->bFilterEnabled) {
						MessageBox(0, CTranslator::get(CTranslator::GEN_MUC_FILTER_ERROR), CTranslator::get(CTranslator::GEN_MUC_FILTER_ERROR_TITLE), MB_OK);
						si->bFilterEnabled = 0;
					} else
						si->bFilterEnabled = !si->bFilterEnabled;

					SendDlgItemMessage(hwndDlg, IDC_FILTER, BUTTONSETOVERLAYICON, 0,
									   (LPARAM)(si->bFilterEnabled ? PluginConfig.g_iconOverlayEnabled : PluginConfig.g_iconOverlayDisabled));

					if (si->bFilterEnabled && M->GetByte("Chat", "RightClickFilter", 0) == 0) {
						SendMessage(hwndDlg, GC_SHOWFILTERMENU, 0, 0);
						break;
					}
					SendMessage(hwndDlg, GC_REDRAWLOG, 0, 0);
					SendMessage(hwndDlg, GC_UPDATETITLE, 0, 0);
					M->WriteByte(si->hContact, "Chat", "FilterEnabled", (BYTE)si->bFilterEnabled);
					break;

				case IDC_BKGCOLOR: {
					CHARFORMAT2 cf;

					cf.cbSize = sizeof(CHARFORMAT2);
					cf.dwEffects = 0;

					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BKGCOLOR)))
						break;

					if (IsDlgButtonChecked(hwndDlg, IDC_BKGCOLOR)) {
						if (M->GetByte("Chat", "RightClickFilter", 0) == 0)
							SendMessage(hwndDlg, GC_SHOWCOLORCHOOSER, 0, (LPARAM)IDC_BKGCOLOR);
						else if (si->bBGSet) {
							cf.dwMask = CFM_BACKCOLOR;
							cf.crBackColor = MM_FindModule(si->pszModule)->crColors[si->iBG];
							SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
						}
					} else {
						cf.dwMask = CFM_BACKCOLOR;
						cf.crBackColor = (COLORREF)M->GetDword(FONTMODULE, "inputbg", SRMSGDEFSET_BKGCOLOUR);
						SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
					}
				}
				break;

				case IDC_COLOR: {
					CHARFORMAT2 cf;
					cf.cbSize = sizeof(CHARFORMAT2);
					cf.dwEffects = 0;

					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_COLOR)))
						break;

					if (IsDlgButtonChecked(hwndDlg, IDC_COLOR)) {
						if (M->GetByte("Chat", "RightClickFilter", 0) == 0)
							SendMessage(hwndDlg, GC_SHOWCOLORCHOOSER, 0, (LPARAM)IDC_COLOR);
						else if (si->bFGSet) {
							cf.dwMask = CFM_COLOR;
							cf.crTextColor = MM_FindModule(si->pszModule)->crColors[si->iFG];
							SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
						}
					} else {
						COLORREF cr;

						LoadLogfont(MSGFONTID_MESSAGEAREA, NULL, &cr, FONTMODULE);
						cf.dwMask = CFM_COLOR;
						cf.crTextColor = cr;
						SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
					}
				}
				break;

			case IDC_CHAT_BOLD:
			case IDC_ITALICS:
			case IDC_CHAT_UNDERLINE: {
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE;
				cf.dwEffects = 0;

				if (LOWORD(wParam) == IDC_CHAT_BOLD && !IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHAT_BOLD)))
					break;
				if (LOWORD(wParam) == IDC_ITALICS && !IsWindowEnabled(GetDlgItem(hwndDlg, IDC_ITALICS)))
					break;
				if (LOWORD(wParam) == IDC_CHAT_UNDERLINE && !IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHAT_UNDERLINE)))
					break;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_BOLD))
					cf.dwEffects |= CFE_BOLD;
				if (IsDlgButtonChecked(hwndDlg, IDC_ITALICS))
					cf.dwEffects |= CFE_ITALIC;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_UNDERLINE))
					cf.dwEffects |= CFE_UNDERLINE;

				SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			}
		}
		break;

		case WM_KEYDOWN:
			SetFocus(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));
			break;

		case WM_MOVE:
			break;

		case WM_ERASEBKGND: {
			HDC  hdc = (HDC)wParam;
			RECT rcClient, rcWindow, rc;
			CSkinItem *item;
			POINT pt;
			UINT item_ids[3] = {ID_EXTBKUSERLIST, ID_EXTBKHISTORY, ID_EXTBKINPUTAREA};
			UINT ctl_ids[3] = {IDC_LIST, IDC_CHAT_LOG, IDC_CHAT_MESSAGE};
			int  i;
			bool 	fAero = M->isAero();
			bool 	fInfoPanel = dat->Panel->isActive();
			HANDLE 	hbp = 0;
			HDC 	hdcMem = 0;
			HBITMAP hbm, hbmOld;

			GetClientRect(hwndDlg, &rcClient);
			LONG cx = rcClient.right - rcClient.left;
			LONG cy = rcClient.bottom - rcClient.top;

			if(CMimAPI::m_haveBufferedPaint)
				hbp = CSkin::InitiateBufferedPaint(hdc, rcClient, hdcMem);
			else {
				hdcMem = CreateCompatibleDC(hdc);
				hbm =  CSkin::CreateAeroCompatibleBitmap(rcClient, hdc);
				hbmOld = (HBITMAP)SelectObject(hdcMem, hbm);
			}

			if(CSkin::m_skinEnabled && !fAero) {
				CSkin::SkinDrawBG(hwndDlg, dat->pContainer->hwnd, dat->pContainer, &rcClient, hdcMem);
				for (i = 0; i < 3; i++) {
					item = &SkinItems[item_ids[i]];
					if (!item->IGNORED) {

						GetWindowRect(GetDlgItem(hwndDlg, ctl_ids[i]), &rcWindow);
						pt.x = rcWindow.left;
						pt.y = rcWindow.top;
						ScreenToClient(hwndDlg, &pt);
						rc.left = pt.x - item->MARGIN_LEFT;
						rc.top = pt.y - item->MARGIN_TOP;
						rc.right = rc.left + item->MARGIN_RIGHT + (rcWindow.right - rcWindow.left) + item->MARGIN_LEFT;
						rc.bottom = rc.top + item->MARGIN_BOTTOM + (rcWindow.bottom - rcWindow.top) + item->MARGIN_TOP;
						CSkin::DrawItem(hdcMem, &rc, item);
					}
				}
			}
			else {
				CSkin::FillBack(hdcMem, &rcClient);

				if(M->isAero()) {
					LONG temp = rcClient.bottom;
					rcClient.bottom = dat->Panel->isActive() ? dat->Panel->getHeight() + 5 : 5;
					FillRect(hdcMem, &rcClient, (HBRUSH)GetStockObject(BLACK_BRUSH));
					rcClient.bottom = temp;
				}
			}

			GetClientRect(hwndDlg, &rc);
			dat->Panel->renderBG(hdcMem, rc, &SkinItems[ID_EXTBKINFOPANELBG], fAero);


			dat->Panel->renderContent(hdcMem);

			if(!CSkin::m_skinEnabled)
				CSkin::RenderToolbarBG(dat, hdcMem, rcClient);

			if(hbp)
				CSkin::FinalizeBufferedPaint(hbp, &rcClient);
			else {
				BitBlt(hdc, 0, 0, cx, cy, hdcMem, 0, 0, SRCCOPY);
				SelectObject(hdcMem, hbmOld);
				DeleteObject(hbm);
				DeleteDC(hdcMem);
			}
			if(!dat->fLimitedUpdate)
				SetAeroMargins(dat->pContainer);
			return(1);
		}

		case WM_NCPAINT:
			if (CSkin::m_skinEnabled)
				return 0;
			break;
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC 	hdc = BeginPaint(hwndDlg, &ps);
			EndPaint(hwndDlg, &ps);
			return 0;
		}

		case DM_SETINFOPANEL:
			CInfoPanel::setPanelHandler(dat, wParam, lParam);
			return(0);

		case DM_INVALIDATEPANEL:
			if(dat->Panel)
				dat->Panel->Invalidate(true);
			return(0);

		case WM_RBUTTONUP: {
			POINT pt;
			int iSelection;
			HMENU subMenu;
			int isHandled;
			int menuID = 0;

			GetCursorPos(&pt);

			if(dat->Panel->invokeConfigDialog(pt))
				break;

			subMenu = GetSubMenu(dat->pContainer->hMenuContext, 0);

			MsgWindowUpdateMenu(dat, subMenu, MENU_TABCONTEXT);

			iSelection = TrackPopupMenu(subMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL);
			if (iSelection >= IDM_CONTAINERMENU) {
				DBVARIANT dbv = {0};
				char szIndex[10];
				char *szKey = "TAB_ContainersW";
				_snprintf(szIndex, 8, "%d", iSelection - IDM_CONTAINERMENU);
				if (iSelection - IDM_CONTAINERMENU >= 0) {
					if (!M->GetTString(NULL, szKey, szIndex, &dbv)) {
						SendMessage(hwndDlg, DM_CONTAINERSELECTED, 0, (LPARAM)dbv.ptszVal);
						DBFreeVariant(&dbv);
					}
				}

				break;
			}
			isHandled = MsgWindowMenuHandler(dat, iSelection, MENU_TABCONTEXT);
			break;
		}

		case WM_LBUTTONDBLCLK: {
			if (LOWORD(lParam) < 30)
				PostMessage(hwndDlg, GC_SCROLLTOBOTTOM, 0, 0);
			break;
		}

		case WM_CLOSE:
			if (wParam == 0 && lParam == 0) {
				if(PluginConfig.m_EscapeCloses == 1) {
					SendMessage(dat->pContainer->hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
					return(TRUE);
				} else if(PluginConfig.m_HideOnClose && PluginConfig.m_EscapeCloses == 2) {
					ShowWindow(dat->pContainer->hwnd, SW_HIDE);
					return(TRUE);
				}
				_dlgReturn(hwndDlg, TRUE);
			}
			SendMessage(hwndDlg, GC_CLOSEWINDOW, 0, 1);
			break;

		case DM_CONTAINERSELECTED: {
			struct TContainerData *pNewContainer = 0;
			TCHAR *szNewName = (TCHAR *)lParam;
			if(!_tcscmp(szNewName, CTranslator::get(CTranslator::GEN_DEFAULT_CONTAINER_NAME)))
				szNewName = CGlobals::m_default_container_name;
			int iOldItems = TabCtrl_GetItemCount(hwndTab);
			if (!_tcsncmp(dat->pContainer->szName, szNewName, CONTAINER_NAMELEN))
				break;
			pNewContainer = FindContainerByName(szNewName);
			if (pNewContainer == NULL)
				pNewContainer = CreateContainer(szNewName, FALSE, dat->hContact);
			M->WriteTString(dat->hContact, SRMSGMOD_T, "containerW", szNewName);
			PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_DOCREATETAB_CHAT, (WPARAM)pNewContainer, (LPARAM)hwndDlg);
			if (iOldItems > 1)                // there were more than 1 tab, container is still valid
				SendMessage(dat->pContainer->hwndActive, WM_SIZE, 0, 0);
			SetForegroundWindow(pNewContainer->hwnd);
			SetActiveWindow(pNewContainer->hwnd);
		}
		break;
		// container API support functions

		case DM_QUERYCONTAINER: {
			struct TContainerData **pc = (struct TContainerData **) lParam;
			if (pc)
				*pc = dat->pContainer;
			return 0;
		}

		case DM_QUERYHCONTACT: {
			HANDLE *phContact = (HANDLE *) lParam;
			if (phContact)
				*phContact = dat->hContact;
			return 0;
		}

		case GC_CLOSEWINDOW: {
			int iTabs, i;
			TCITEM item = {0};
			RECT rc;
			struct TContainerData *pContainer = dat->pContainer;
			BOOL   bForced = (lParam == 2);

			iTabs = TabCtrl_GetItemCount(hwndTab);
			if (iTabs == 1) {
				if (/*!bForced && */CMimAPI::m_shutDown == 0) {
					//DestroyWindow(GetParent(GetParent(hwndDlg)));
					//PostMessage(hwndDlg, WM_CLOSE, 0, 1);
					SendMessage(GetParent(GetParent(hwndDlg)), WM_CLOSE, 0, 1);
					return 1;
				}
			}

			dat->pContainer->iChilds--;
			i = GetTabIndexFromHWND(hwndTab, hwndDlg);

			/*
			* after closing a tab, we need to activate the tab to the left side of
			* the previously open tab.
			* normally, this tab has the same index after the deletion of the formerly active tab
			* unless, of course, we closed the last (rightmost) tab.
			*/
			if (!dat->pContainer->bDontSmartClose && iTabs > 1 && !bForced) {
				if (i == iTabs - 1)
					i--;
				else
					i++;
				TabCtrl_SetCurSel(hwndTab, i);
				item.mask = TCIF_PARAM;
				TabCtrl_GetItem(hwndTab, i, &item);         // retrieve dialog hwnd for the now active tab...

				dat->pContainer->hwndActive = (HWND) item.lParam;
				SendMessage(dat->pContainer->hwnd, DM_QUERYCLIENTAREA, 0, (LPARAM)&rc);
				SetWindowPos(dat->pContainer->hwndActive, HWND_TOP, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), SWP_SHOWWINDOW);
				ShowWindow((HWND)item.lParam, SW_SHOW);
				SetForegroundWindow(dat->pContainer->hwndActive);
				SetFocus(dat->pContainer->hwndActive);
				SendMessage(dat->pContainer->hwnd, WM_SIZE, 0, 0);
			}
			//SM_SetTabbedWindowHwnd(0, 0);
			//DestroyWindow(hwndDlg);
			if (iTabs == 1)
				SendMessage(GetParent(GetParent(hwndDlg)), WM_CLOSE, 0, 1);
			else {
				PostMessage(pContainer->hwnd, WM_SIZE, 0, 0);
				DestroyWindow(hwndDlg);
			}
			return 0;
		}

		case DM_SETLOCALE:
			if (dat->dwFlags & MWF_WASBACKGROUNDCREATE)
				break;
			if (dat->pContainer->hwndActive == hwndDlg && PluginConfig.m_AutoLocaleSupport && dat->hContact != 0 && dat->pContainer->hwnd == GetForegroundWindow() && dat->pContainer->hwnd == GetActiveWindow()) {
				if(lParam)
					dat->hkl = (HKL)lParam;

				if (dat->hkl)
					ActivateKeyboardLayout(dat->hkl, 0);
			}
			return 0;

		case DM_SAVESIZE: {
			RECT rcClient;

			if (dat->dwFlags & MWF_NEEDCHECKSIZE)
				lParam = 0;

			dat->dwFlags &= ~MWF_NEEDCHECKSIZE;
			if (dat->dwFlags & MWF_WASBACKGROUNDCREATE)
				dat->dwFlags &= ~MWF_INITMODE;

			SendMessage(dat->pContainer->hwnd, DM_QUERYCLIENTAREA, 0, (LPARAM)&rcClient);
			MoveWindow(hwndDlg, rcClient.left, rcClient.top, (rcClient.right - rcClient.left), (rcClient.bottom - rcClient.top), TRUE);
			if (dat->dwFlags & MWF_WASBACKGROUNDCREATE) {
				POINT pt = {0};;

				dat->dwFlags &= ~MWF_WASBACKGROUNDCREATE;
				SendMessage(hwndDlg, WM_SIZE, 0, 0);
				SendDlgItemMessage(hwndDlg, IDC_CHAT_LOG, EM_SETSCROLLPOS, 0, (LPARAM)&pt);
				if(PluginConfig.m_AutoLocaleSupport) {
					if(dat->hkl == 0)
						DM_LoadLocale(dat);
					else
						PostMessage(hwndDlg, DM_SETLOCALE, 0, 0);
				}
			} else {
				SendMessage(hwndDlg, WM_SIZE, 0, 0);
				if (lParam == 0)
					PostMessage(hwndDlg, GC_SCROLLTOBOTTOM, 1, 1);
			}
			return 0;
		}

		case DM_GETWINDOWSTATE: {
			UINT state = 0;

			state |= MSG_WINDOW_STATE_EXISTS;
			if (IsWindowVisible(hwndDlg))
				state |= MSG_WINDOW_STATE_VISIBLE;
			if (GetForegroundWindow() == dat->pContainer->hwnd)
				state |= MSG_WINDOW_STATE_FOCUS;
			if (IsIconic(dat->pContainer->hwnd))
				state |= MSG_WINDOW_STATE_ICONIC;
			SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, state);
			return TRUE;
		}

		case DM_ADDDIVIDER:
			if (!(dat->dwFlags & MWF_DIVIDERSET) && g_Settings.UseDividers) {
				if (GetWindowTextLengthA(GetDlgItem(hwndDlg, IDC_CHAT_LOG)) > 0) {
					dat->dwFlags |= MWF_DIVIDERWANTED;
					dat->dwFlags |= MWF_DIVIDERSET;
				}
			}
			return 0;

		case DM_CHECKSIZE:
			dat->dwFlags |= MWF_NEEDCHECKSIZE;
			return 0;

		case DM_REFRESHTABINDEX:
			dat->iTabID = GetTabIndexFromHWND(GetParent(hwndDlg), hwndDlg);
			return 0;

		case DM_STATUSBARCHANGED:
			UpdateStatusBar(dat);
			break;

			//mad: bb-api
		case DM_BBNEEDUPDATE:{
			if(lParam)
				CB_ChangeButton(hwndDlg,dat,(CustomButtonData*)lParam);
			else
				BB_InitDlgButtons(dat);

			BB_SetButtonsPos(dat);
			}break;

		case DM_CBDESTROY:{
			if(lParam)
				CB_DestroyButton(hwndDlg,dat,(DWORD)wParam,(DWORD)lParam);
			else
				CB_DestroyAllButtons(hwndDlg,dat);
			}break;
		//

		case DM_CONFIGURETOOLBAR:
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
			break;

		case DM_SMILEYOPTIONSCHANGED:
			ConfigureSmileyButton(dat);
			SendMessage(hwndDlg, GC_REDRAWLOG, 0, 1);
			break;

		case EM_THEMECHANGED:
			DM_FreeTheme(dat);
			return DM_ThemeChanged(dat);

		case WM_DWMCOMPOSITIONCHANGED:
			BB_RefreshTheme(dat);
			memset((void *)&dat->pContainer->mOld, -1000, sizeof(MARGINS));
			CProxyWindow::verify(dat);
			break;

		case DM_ACTIVATEME:
			ActivateExistingTab(dat->pContainer, hwndDlg);
			return 0;

		case DM_ACTIVATETOOLTIP: {
			if (IsIconic(dat->pContainer->hwnd) || dat->pContainer->hwndActive != hwndDlg)
				break;

			dat->Panel->showTip(wParam, lParam);
			break;
		}

		case DM_SAVEMESSAGELOG:
			DM_SaveLogAsRTF(dat);
			return(0);

		case DM_CHECKAUTOHIDE:
			DM_CheckAutoHide(dat, wParam, lParam);
			return(0);

		case WM_NCDESTROY:
			if (dat) {
				memset((void *)&dat->pContainer->mOld, -1000, sizeof(MARGINS));
				PostMessage(dat->pContainer->hwnd, WM_SIZE, 0, 1);
				delete dat->Panel;
				if(dat->pContainer->dwFlags & CNT_SIDEBAR)
					dat->pContainer->SideBar->removeSession(dat);
				free(dat);
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
			}
			break;

		case WM_DESTROY: {
			int i;

			if (CallService(MS_CLIST_GETEVENT, (WPARAM)si->hContact, (LPARAM)0))
				CallService(MS_CLIST_REMOVEEVENT, (WPARAM)si->hContact, (LPARAM)szChatIconString);
			si->wState &= ~STATE_TALK;
			si->hWnd = NULL;
			si->dat = 0;
			si->pContainer = 0;

			//SetWindowLongPtr(hwndDlg,GWLP_USERDATA,0);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_SPLITTERX), GWLP_WNDPROC, (LONG_PTR)OldSplitterProc);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_SPLITTERY), GWLP_WNDPROC, (LONG_PTR)OldSplitterProc);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_LIST), GWLP_WNDPROC, (LONG_PTR)OldNicklistProc);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_UNSUBCLASSED, 0, 0);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), GWLP_WNDPROC, (LONG_PTR)OldMessageProc);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CHAT_LOG), GWLP_WNDPROC, (LONG_PTR)OldLogProc);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_FILTER), GWLP_WNDPROC, (LONG_PTR)OldFilterButtonProc);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_COLOR), GWLP_WNDPROC, (LONG_PTR)OldFilterButtonProc);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_BKGCOLOR), GWLP_WNDPROC, (LONG_PTR)OldFilterButtonProc);

			TABSRMM_FireEvent(dat->hContact, hwndDlg, MSG_WINDOW_EVT_CLOSING, 0);

			if(!dat->fIsAutosizingInput)
				DBWriteContactSettingWord(NULL, "Chat", "SplitterX", (WORD)g_Settings.iSplitterX);

			if(dat->pContainer->settings->fPrivate && !IsAutoSplitEnabled(dat))
				DBWriteContactSettingWord(NULL, "Chat", "splitY", (WORD)g_Settings.iSplitterY);

			DM_FreeTheme(dat);

			UpdateTrayMenuState(dat, FALSE);               // remove me from the tray menu (if still there)
			if (PluginConfig.g_hMenuTrayUnread)
				DeleteMenu(PluginConfig.g_hMenuTrayUnread, (UINT_PTR)dat->hContact, MF_BYCOMMAND);

			if (dat->hSmileyIcon)
				DestroyIcon(dat->hSmileyIcon);

			if (dat->hwndTip)
				DestroyWindow(dat->hwndTip);

			if (hCurHyperlinkHand)
				DestroyCursor(hCurHyperlinkHand);

			i = GetTabIndexFromHWND(hwndTab, hwndDlg);
			if (i >= 0) {
				SendMessage(hwndTab, WM_USER + 100, 0, 0);              // clean up tooltip
				TabCtrl_DeleteItem(hwndTab, i);
				BroadCastContainer(dat->pContainer, DM_REFRESHTABINDEX, 0, 0);
				dat->iTabID = -1;
			}
			if(dat->pWnd) {
				delete dat->pWnd;
				dat->pWnd = 0;
			}
			//MAD
			M->RemoveWindow(hwndDlg);

			TABSRMM_FireEvent(dat->hContact, hwndDlg, MSG_WINDOW_EVT_CLOSE, 0);
			break;
		}
	}
	return(FALSE);
}
