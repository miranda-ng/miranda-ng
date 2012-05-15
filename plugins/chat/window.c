/*
Chat module plugin for Miranda IM

Copyright 2000-2010 Miranda ICQ/IM project, 
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

#include "chat.h"

extern HBRUSH		hEditBkgBrush;
extern HBRUSH		hListBkgBrush;
extern HBRUSH		hListSelectedBkgBrush;
extern HANDLE		hSendEvent;
extern HINSTANCE	g_hInst;
extern HICON		hIcons[30];
extern struct		CREOleCallback reOleCallback;
extern HIMAGELIST	hImageList;
extern HMENU		g_hMenu;
extern BOOL			SmileyAddInstalled;
extern TABLIST *	g_TabList;
extern HIMAGELIST	hIconsList;

static WNDPROC OldSplitterProc;
static WNDPROC OldMessageProc;
static WNDPROC OldNicklistProc;
static WNDPROC OldTabProc;
static WNDPROC OldFilterButtonProc;
static WNDPROC OldLogProc;
static HKL hkl = NULL;

typedef struct
{
	time_t lastEnterTime;
	TCHAR  szTabSave[20];
} MESSAGESUBDATA;


static LRESULT CALLBACK SplitterSubclassProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg) {
	case WM_NCHITTEST:
		return HTCLIENT;

	case WM_SETCURSOR:
	{	RECT rc;
		GetClientRect(hwnd,&rc);
		SetCursor(rc.right>rc.bottom?LoadCursor(NULL, IDC_SIZENS):LoadCursor(NULL, IDC_SIZEWE));
		return TRUE;
	}
	case WM_LBUTTONDOWN:
		SetCapture(hwnd);
		return 0;

	case WM_MOUSEMOVE:
		if (GetCapture()==hwnd) {
			RECT rc;
			GetClientRect(hwnd,&rc);
			SendMessage(GetParent(hwnd),GC_SPLITTERMOVED,rc.right>rc.bottom?(short)HIWORD(GetMessagePos())+rc.bottom/2:(short)LOWORD(GetMessagePos())+rc.right/2,(LPARAM)hwnd);
		}
		return 0;

	case WM_LBUTTONUP:
		ReleaseCapture();
		PostMessage(GetParent(hwnd),WM_SIZE, 0, 0);
		return 0;
	}
	return CallWindowProc(OldSplitterProc,hwnd,msg,wParam,lParam);
}

static void	InitButtons(HWND hwndDlg, SESSION_INFO* si)
{
	MODULEINFO * pInfo = MM_FindModule(si->pszModule);

	SendDlgItemMessage( hwndDlg, IDC_SMILEY,       BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIconEx( "smiley", FALSE    ));
	SendDlgItemMessage( hwndDlg, IDC_BOLD,         BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIconEx( "bold", FALSE      ));
	SendDlgItemMessage( hwndDlg, IDC_ITALICS,      BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIconEx( "italics", FALSE   ));
	SendDlgItemMessage( hwndDlg, IDC_UNDERLINE,    BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIconEx( "underline", FALSE ));
	SendDlgItemMessage( hwndDlg, IDC_COLOR,        BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIconEx( "fgcol", FALSE     ));
	SendDlgItemMessage( hwndDlg, IDC_BKGCOLOR,     BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIconEx( "bkgcol", FALSE    ));
	SendDlgItemMessage( hwndDlg, IDC_HISTORY,      BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIconEx( "history", FALSE   ));
	SendDlgItemMessage( hwndDlg, IDC_CHANMGR,      BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIconEx( "settings", FALSE  ));
	SendDlgItemMessage( hwndDlg, IDC_CLOSE,        BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIconEx( "close", FALSE     ));
	SendDlgItemMessage( hwndDlg, IDC_SHOWNICKLIST, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIconEx( si->bNicklistEnabled ? "nicklist" : "nicklist2", FALSE ));
	SendDlgItemMessage( hwndDlg, IDC_FILTER,       BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIconEx( si->bFilterEnabled ? "filter" : "filter2", FALSE ));

	SendDlgItemMessage( hwndDlg, IDC_SMILEY,       BUTTONSETASFLATBTN, 0, 0 );
	SendDlgItemMessage( hwndDlg, IDC_BOLD,         BUTTONSETASFLATBTN, 0, 0 );
	SendDlgItemMessage( hwndDlg, IDC_ITALICS,      BUTTONSETASFLATBTN, 0, 0 );
	SendDlgItemMessage( hwndDlg, IDC_UNDERLINE,    BUTTONSETASFLATBTN, 0, 0 );
	SendDlgItemMessage( hwndDlg, IDC_BKGCOLOR,     BUTTONSETASFLATBTN, 0, 0 );
	SendDlgItemMessage( hwndDlg, IDC_COLOR,        BUTTONSETASFLATBTN, 0, 0 );
	SendDlgItemMessage( hwndDlg, IDC_HISTORY,      BUTTONSETASFLATBTN, 0, 0 );
	SendDlgItemMessage( hwndDlg, IDC_SHOWNICKLIST, BUTTONSETASFLATBTN, 0, 0 );
	SendDlgItemMessage( hwndDlg, IDC_CHANMGR,      BUTTONSETASFLATBTN, 0, 0 );
	SendDlgItemMessage( hwndDlg, IDC_FILTER,       BUTTONSETASFLATBTN, 0, 0 );
	SendDlgItemMessage( hwndDlg, IDC_CLOSE,        BUTTONSETASFLATBTN, 0, 0 );

	SendMessage(GetDlgItem(hwndDlg,IDC_SMILEY), BUTTONADDTOOLTIP, (WPARAM)LPGEN("Insert a smiley"), 0);
	SendMessage(GetDlgItem(hwndDlg,IDC_BOLD), BUTTONADDTOOLTIP, (WPARAM)LPGEN("Make the text bold (CTRL+B)"), 0);
	SendMessage(GetDlgItem(hwndDlg,IDC_ITALICS), BUTTONADDTOOLTIP, (WPARAM)LPGEN("Make the text italicized (CTRL+I)"), 0);
	SendMessage(GetDlgItem(hwndDlg,IDC_UNDERLINE), BUTTONADDTOOLTIP, (WPARAM)LPGEN("Make the text underlined (CTRL+U)"), 0);
	SendMessage(GetDlgItem(hwndDlg,IDC_BKGCOLOR), BUTTONADDTOOLTIP, (WPARAM)LPGEN("Select a background color for the text (CTRL+L)"), 0);
	SendMessage(GetDlgItem(hwndDlg,IDC_COLOR), BUTTONADDTOOLTIP, (WPARAM)LPGEN("Select a foreground color for the text (CTRL+K)"), 0);
	SendMessage(GetDlgItem(hwndDlg,IDC_HISTORY), BUTTONADDTOOLTIP, (WPARAM)LPGEN("Show the history (CTRL+H)"), 0);
	SendMessage(GetDlgItem(hwndDlg,IDC_SHOWNICKLIST), BUTTONADDTOOLTIP, (WPARAM)LPGEN("Show/hide the nicklist (CTRL+N)"), 0);
	SendMessage(GetDlgItem(hwndDlg,IDC_CHANMGR), BUTTONADDTOOLTIP, (WPARAM)LPGEN("Control this room (CTRL+O)"), 0);
	SendMessage(GetDlgItem(hwndDlg,IDC_FILTER), BUTTONADDTOOLTIP, (WPARAM)LPGEN("Enable/disable the event filter (CTRL+F)"), 0);
	SendMessage(GetDlgItem(hwndDlg,IDC_CLOSE), BUTTONADDTOOLTIP, (WPARAM)LPGEN("Close current tab (CTRL+F4)"), 0);
	SendDlgItemMessage(hwndDlg, IDC_BOLD, BUTTONSETASPUSHBTN, 0, 0);
	SendDlgItemMessage(hwndDlg, IDC_ITALICS, BUTTONSETASPUSHBTN, 0, 0);
	SendDlgItemMessage(hwndDlg, IDC_UNDERLINE, BUTTONSETASPUSHBTN, 0, 0);
	SendDlgItemMessage(hwndDlg, IDC_COLOR, BUTTONSETASPUSHBTN, 0, 0);
	SendDlgItemMessage(hwndDlg, IDC_BKGCOLOR, BUTTONSETASPUSHBTN, 0, 0);
	//	SendDlgItemMessage(hwndDlg, IDC_SHOWNICKLIST, BUTTONSETASPUSHBTN, 0, 0);
	//	SendDlgItemMessage(hwndDlg, IDC_FILTER, BUTTONSETASPUSHBTN, 0, 0);

	if (pInfo) {
		EnableWindow(GetDlgItem(hwndDlg, IDC_BOLD), pInfo->bBold);
		EnableWindow(GetDlgItem(hwndDlg, IDC_ITALICS), pInfo->bItalics);
		EnableWindow(GetDlgItem(hwndDlg, IDC_UNDERLINE), pInfo->bUnderline);
		EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR), pInfo->bColor);
		EnableWindow(GetDlgItem(hwndDlg, IDC_BKGCOLOR), pInfo->bBkgColor);
		if (si->iType == GCW_CHATROOM)
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHANMGR), pInfo->bChanMgr);
}	}

static int RoomWndResize(HWND hwndDlg,LPARAM lParam,UTILRESIZECONTROL *urc)
{
	RECT rc, rcTabs;
	SESSION_INFO* si = (SESSION_INFO*)lParam;
	int			TabHeight;
	BOOL		bControl = (BOOL)DBGetContactSettingByte(NULL, "Chat", "ShowTopButtons", 1);
	BOOL		bFormat = (BOOL)DBGetContactSettingByte(NULL, "Chat", "ShowFormatButtons", 1);
	BOOL		bToolbar = bFormat || bControl;
	BOOL		bSend = (BOOL)DBGetContactSettingByte(NULL, "Chat", "ShowSend", 0);
	BOOL		bNick = si->iType!=GCW_SERVER && si->bNicklistEnabled;
	BOOL		bTabs = g_Settings.TabsEnable;
	BOOL		bTabBottom = g_Settings.TabsAtBottom;

	GetClientRect(GetDlgItem(hwndDlg, IDC_TAB), &rcTabs);
	TabHeight = rcTabs.bottom - rcTabs.top;
	TabCtrl_AdjustRect(GetDlgItem(hwndDlg, IDC_TAB), FALSE, &rcTabs);
	TabHeight -= (rcTabs.bottom - rcTabs.top);
	ShowWindow(GetDlgItem(hwndDlg, IDC_SMILEY), SmileyAddInstalled&&bFormat?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_BOLD), bFormat?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_UNDERLINE), bFormat?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_ITALICS), bFormat?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_COLOR), bFormat?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_BKGCOLOR), bFormat?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_HISTORY), bControl?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_SHOWNICKLIST), bControl?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_FILTER), bControl?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_CHANMGR), bControl?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDOK), bSend?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_SPLITTERX), bNick?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_CLOSE), g_Settings.TabsEnable?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_TAB), g_Settings.TabsEnable?SW_SHOW:SW_HIDE);
	if (si->iType != GCW_SERVER)
		ShowWindow(GetDlgItem(hwndDlg, IDC_LIST), si->bNicklistEnabled?SW_SHOW:SW_HIDE);
	else
		ShowWindow(GetDlgItem(hwndDlg, IDC_LIST), SW_HIDE);

	if (si->iType == GCW_SERVER) {
		EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWNICKLIST), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_FILTER), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHANMGR), FALSE);
	}
	else {
		EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWNICKLIST), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_FILTER), TRUE);
		if (si->iType == GCW_CHATROOM)
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHANMGR), MM_FindModule(si->pszModule)->bChanMgr);
	}

	switch(urc->wId) {
	case IDOK:
		GetWindowRect(si->hwndStatus, &rc);
		urc->rcItem.left = bSend?315:urc->dlgNewSize.cx ;
		urc->rcItem.top = urc->dlgNewSize.cy - si->iSplitterY+23;
		urc->rcItem.bottom = urc->dlgNewSize.cy - (rc.bottom-rc.top)-1;
		return RD_ANCHORX_RIGHT|RD_ANCHORY_CUSTOM;

	case IDC_TAB:
		urc->rcItem.top = 1;
		urc->rcItem.left = 0;
		urc->rcItem.right = urc->dlgNewSize.cx- 24;
		urc->rcItem.bottom = bToolbar?(urc->dlgNewSize.cy - si->iSplitterY):(urc->dlgNewSize.cy - si->iSplitterY+20);
		return RD_ANCHORX_CUSTOM|RD_ANCHORY_CUSTOM;

	case IDC_LOG:
		urc->rcItem.top = bTabs?(bTabBottom?0:rcTabs.top-1):0;
		urc->rcItem.left = 0;
		urc->rcItem.right = bNick?urc->dlgNewSize.cx - si->iSplitterX:urc->dlgNewSize.cx;
		urc->rcItem.bottom = bToolbar?(bTabs&&bTabBottom?urc->dlgNewSize.cy - si->iSplitterY-TabHeight + 6:urc->dlgNewSize.cy - si->iSplitterY):(bTabs&&bTabBottom?urc->dlgNewSize.cy - si->iSplitterY-TabHeight+26:urc->dlgNewSize.cy - si->iSplitterY+20);
		return RD_ANCHORX_CUSTOM|RD_ANCHORY_CUSTOM;

	case IDC_LIST:
		urc->rcItem.top = bTabs?(bTabBottom?0:rcTabs.top-1):0;
		urc->rcItem.right = urc->dlgNewSize.cx ;
		urc->rcItem.left = urc->dlgNewSize.cx - si->iSplitterX + 2;
		urc->rcItem.bottom = bToolbar?(bTabs&&bTabBottom?urc->dlgNewSize.cy - si->iSplitterY-TabHeight + 6:urc->dlgNewSize.cy - si->iSplitterY):(bTabs&&bTabBottom?urc->dlgNewSize.cy - si->iSplitterY-TabHeight+26:urc->dlgNewSize.cy - si->iSplitterY+20);
		return RD_ANCHORX_CUSTOM|RD_ANCHORY_CUSTOM;

	case IDC_SPLITTERX:
		urc->rcItem.right = urc->dlgNewSize.cx - si->iSplitterX+2;
		urc->rcItem.left = urc->dlgNewSize.cx - si->iSplitterX;
		urc->rcItem.bottom = bToolbar?(bTabs&&bTabBottom?urc->dlgNewSize.cy - si->iSplitterY-TabHeight + 6:urc->dlgNewSize.cy - si->iSplitterY):(bTabs&&bTabBottom?urc->dlgNewSize.cy - si->iSplitterY-TabHeight+26:urc->dlgNewSize.cy - si->iSplitterY+20);
		urc->rcItem.top = bTabs ?rcTabs.top:1;
		return RD_ANCHORX_CUSTOM|RD_ANCHORY_CUSTOM;

	case IDC_SPLITTERY:
		urc->rcItem.top = bToolbar?urc->dlgNewSize.cy - si->iSplitterY:urc->dlgNewSize.cy - si->iSplitterY+20;
		urc->rcItem.bottom = bToolbar?(urc->dlgNewSize.cy - si->iSplitterY+2):(urc->dlgNewSize.cy - si->iSplitterY+22);
		return RD_ANCHORX_WIDTH|RD_ANCHORY_CUSTOM;

	case IDC_MESSAGE:
		GetWindowRect(si->hwndStatus, &rc);
		urc->rcItem.right = bSend?urc->dlgNewSize.cx - 64:urc->dlgNewSize.cx ;
		urc->rcItem.top = urc->dlgNewSize.cy - si->iSplitterY+22;
		urc->rcItem.bottom = urc->dlgNewSize.cy - (rc.bottom-rc.top)-1 ;
		return RD_ANCHORX_LEFT|RD_ANCHORY_CUSTOM;

	case IDC_SMILEY:
	case IDC_ITALICS:
	case IDC_BOLD:
	case IDC_UNDERLINE:
	case IDC_COLOR:
	case IDC_BKGCOLOR:
		urc->rcItem.top = urc->dlgNewSize.cy - si->iSplitterY+3;
		urc->rcItem.bottom = urc->dlgNewSize.cy - si->iSplitterY+19;
		return RD_ANCHORX_LEFT|RD_ANCHORY_CUSTOM;

	case IDC_HISTORY:
	case IDC_CHANMGR:
	case IDC_SHOWNICKLIST:
	case IDC_FILTER:
		urc->rcItem.top = urc->dlgNewSize.cy - si->iSplitterY+3;
		urc->rcItem.bottom = urc->dlgNewSize.cy - si->iSplitterY+19;
		return RD_ANCHORX_RIGHT|RD_ANCHORY_CUSTOM;

	case IDC_CLOSE:
		urc->rcItem.right = urc->dlgNewSize.cx-3;
		urc->rcItem.left = urc->dlgNewSize.cx - 19;
		urc->rcItem.bottom = bTabBottom?(bToolbar?urc->dlgNewSize.cy - si->iSplitterY-2:urc->dlgNewSize.cy - si->iSplitterY-2+20):19;
		urc->rcItem.top = bTabBottom?(bToolbar?urc->dlgNewSize.cy - si->iSplitterY-18:urc->dlgNewSize.cy - si->iSplitterY-18+20):3;
		return RD_ANCHORX_CUSTOM|RD_ANCHORY_CUSTOM;
	}
	return RD_ANCHORX_LEFT|RD_ANCHORY_TOP;
}

static LRESULT CALLBACK MessageSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MESSAGESUBDATA *dat;
	SESSION_INFO* Parentsi;

	Parentsi=(SESSION_INFO*)GetWindowLongPtr(GetParent(hwnd),GWLP_USERDATA);
	dat = (MESSAGESUBDATA *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (msg) {
	case EM_SUBCLASSED:
		dat = (MESSAGESUBDATA *) mir_alloc(sizeof(MESSAGESUBDATA));

		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) dat);
		dat->szTabSave[0] = '\0';
		dat->lastEnterTime = 0;
		return 0;

	case WM_MOUSEWHEEL:
		SendMessage(GetDlgItem(GetParent(hwnd), IDC_LOG), WM_MOUSEWHEEL, wParam, lParam);
		dat->lastEnterTime = 0;
		return TRUE;

	case EM_REPLACESEL:
		PostMessage(hwnd, EM_ACTIVATE, 0, 0);
		break;

	case EM_ACTIVATE:
		SetActiveWindow(GetParent(hwnd));
		break;

	case WM_CHAR:
		{
			BOOL isShift = GetKeyState(VK_SHIFT) & 0x8000;
			BOOL isCtrl = GetKeyState(VK_CONTROL) & 0x8000;
			BOOL isAlt = GetKeyState(VK_MENU) & 0x8000;

			if (GetWindowLongPtr(hwnd, GWL_STYLE) & ES_READONLY)
				break;

			if (wParam == 9 && isCtrl && !isAlt) // ctrl-i (italics)
				return TRUE;

			if (wParam == VK_SPACE && isCtrl && !isAlt) // ctrl-space (paste clean text)
				return TRUE;

    		if (wParam == '\n' || wParam == '\r') {
	       		if ((isCtrl != 0) ^ (0 != DBGetContactSettingByte(NULL, "Chat", "SendOnEnter", 1))) {
		      		PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
			     	return 0;
			    }
			    if (DBGetContactSettingByte(NULL, "Chat", "SendOnDblEnter", 0)) {
				    if (dat->lastEnterTime + 2 < time(NULL))
					   dat->lastEnterTime = time(NULL);
				    else {
						SendMessage(hwnd, WM_KEYDOWN, VK_BACK, 0);
						SendMessage(hwnd, WM_KEYUP, VK_BACK, 0);
						PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
						return 0;
				    }
			    }
		    }
		    else
		        dat->lastEnterTime = 0;

			if (wParam == 1 && isCtrl && !isAlt) {      //ctrl-a
				SendMessage(hwnd, EM_SETSEL, 0, -1);
				return 0;
		}	}
		break;

	case WM_KEYDOWN:
		{
			static int start, end;
			BOOL isShift = GetKeyState(VK_SHIFT) & 0x8000;
			BOOL isCtrl = GetKeyState(VK_CONTROL) & 0x8000;
			BOOL isAlt = GetKeyState(VK_MENU) & 0x8000;
			if (wParam == VK_RETURN) {
				dat->szTabSave[0] = '\0';
				if ((isCtrl != 0) ^ (0 != DBGetContactSettingByte(NULL, "Chat", "SendOnEnter", 1)))
					return 0;

				if (DBGetContactSettingByte(NULL, "Chat", "SendOnDblEnter", 0))
					if (dat->lastEnterTime + 2 >= time(NULL))
						return 0;

				break;
			}

			if (wParam == VK_TAB && isShift && !isCtrl) { // SHIFT-TAB (go to nick list)
				SetFocus(GetDlgItem(GetParent(hwnd), IDC_LIST));
				return TRUE;
			}

			if (wParam == VK_TAB && isCtrl && !isShift) { // CTRL-TAB (switch tab/window)
				if (g_Settings.TabsEnable)
					SendMessage(GetParent(hwnd), GC_SWITCHNEXTTAB, 0, 0);
				else
					ShowRoom(SM_GetNextWindow(Parentsi), WINDOW_VISIBLE, TRUE);
				return TRUE;
			}

			if (wParam == VK_TAB && isCtrl && isShift) { // CTRL_SHIFT-TAB (switch tab/window)
				if (g_Settings.TabsEnable)
					SendMessage(GetParent(hwnd), GC_SWITCHPREVTAB, 0, 0);
				else
					ShowRoom(SM_GetPrevWindow(Parentsi), WINDOW_VISIBLE, TRUE);
				return TRUE;
			}

			if (wParam <= '9' && wParam >= '1' && isCtrl && !isAlt) { // CTRL + 1 -> 9 (switch tab)
				if (g_Settings.TabsEnable)
					SendMessage(GetParent(hwnd), GC_SWITCHTAB, 0, (LPARAM)((int)wParam - (int)'1'));
			}

			if (wParam <= VK_NUMPAD9 && wParam >= VK_NUMPAD1 && isCtrl && !isAlt) // CTRL + 1 -> 9 (switch tab)
				if (g_Settings.TabsEnable)
					SendMessage(GetParent(hwnd), GC_SWITCHTAB, 0, (LPARAM)((int)wParam - (int)VK_NUMPAD1));

			if (wParam == VK_TAB && !isCtrl && !isShift) {    //tab-autocomplete
				TCHAR* pszText = NULL;
				int iLen;
				GETTEXTLENGTHEX gtl = {0};
				GETTEXTEX gt = {0};
				LRESULT lResult = (LRESULT)SendMessage(hwnd, EM_GETSEL, (WPARAM)NULL, (LPARAM)NULL);

				SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);
				start = LOWORD(lResult);
				end = HIWORD(lResult);
				SendMessage(hwnd, EM_SETSEL, end, end);
				gtl.flags = GTL_PRECISE;
				gtl.codepage = CP_ACP;
				iLen = SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, (LPARAM)NULL);
				if (iLen >0) {
					TCHAR *pszName = NULL;
					TCHAR *pszSelName = NULL;
					pszText = mir_alloc(sizeof(TCHAR)*(iLen+100));

					gt.cb = iLen+99;
					gt.flags = GT_DEFAULT;
					#if defined( _UNICODE )
						gt.codepage = 1200;
					#else
						gt.codepage = CP_ACP;
					#endif

					SendMessage(hwnd, EM_GETTEXTEX, (WPARAM)&gt, (LPARAM)pszText);
					while ( start >0 && pszText[start-1] != ' ' && pszText[start-1] != 13 && pszText[start-1] != VK_TAB)
						start--;
					while (end < iLen && pszText[end] != ' ' && pszText[end] != 13 && pszText[end-1] != VK_TAB)
						end ++;

					if ( dat->szTabSave[0] =='\0')
						lstrcpyn( dat->szTabSave, pszText+start, end-start+1 );

					pszSelName = mir_alloc( sizeof(TCHAR)*( end-start+1 ));
					lstrcpyn( pszSelName, pszText+start, end-start+1);
					pszName = UM_FindUserAutoComplete(Parentsi->pUsers, dat->szTabSave, pszSelName);
					if (pszName == NULL) {
						pszName = dat->szTabSave;
						SendMessage(hwnd, EM_SETSEL, start, end);
						if (end !=start)
							SendMessage(hwnd, EM_REPLACESEL, FALSE, (LPARAM) pszName);
						dat->szTabSave[0] = '\0';
					}
					else {
						SendMessage(hwnd, EM_SETSEL, start, end);
						if (end !=start)
							SendMessage(hwnd, EM_REPLACESEL, FALSE, (LPARAM) pszName);
					}
					mir_free(pszText);
					mir_free(pszSelName);
				}

				SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
				return 0;
			}

			if (dat->szTabSave[0] != '\0' && wParam != VK_RIGHT && wParam != VK_LEFT
					&& wParam != VK_SPACE && wParam != VK_RETURN && wParam != VK_BACK
					&& wParam != VK_DELETE ) {
				if (g_Settings.AddColonToAutoComplete && start == 0)
					SendMessageA(hwnd, EM_REPLACESEL, FALSE, (LPARAM) ": ");

				dat->szTabSave[0] = '\0';
			}

			if (wParam == VK_F4 && isCtrl && !isAlt) { // ctrl-F4 (close tab)
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CLOSE, BN_CLICKED), 0);
				return TRUE;
			}

			if (wParam == 0x49 && isCtrl && !isAlt) { // ctrl-i (italics)
				CheckDlgButton(GetParent(hwnd), IDC_ITALICS, IsDlgButtonChecked(GetParent(hwnd), IDC_ITALICS) == BST_UNCHECKED?BST_CHECKED:BST_UNCHECKED);
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_ITALICS, 0), 0);
				return TRUE;
			}

			if (wParam == 0x42 && isCtrl && !isAlt) { // ctrl-b (bold)
				CheckDlgButton(GetParent(hwnd), IDC_BOLD, IsDlgButtonChecked(GetParent(hwnd), IDC_BOLD) == BST_UNCHECKED?BST_CHECKED:BST_UNCHECKED);
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_BOLD, 0), 0);
				return TRUE;
			}

			if (wParam == 0x55 && isCtrl && !isAlt) { // ctrl-u (paste clean text)
				CheckDlgButton(GetParent(hwnd), IDC_UNDERLINE, IsDlgButtonChecked(GetParent(hwnd), IDC_UNDERLINE) == BST_UNCHECKED?BST_CHECKED:BST_UNCHECKED);
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_UNDERLINE, 0), 0);
				return TRUE;
			}

			if (wParam == 0x4b && isCtrl && !isAlt) { // ctrl-k (paste clean text)
				CheckDlgButton(GetParent(hwnd), IDC_COLOR, IsDlgButtonChecked(GetParent(hwnd), IDC_COLOR) == BST_UNCHECKED?BST_CHECKED:BST_UNCHECKED);
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_COLOR, 0), 0);
				return TRUE;
			}

			if (wParam == VK_SPACE && isCtrl && !isAlt) { // ctrl-space (paste clean text)
				CheckDlgButton(GetParent(hwnd), IDC_BKGCOLOR, BST_UNCHECKED);
				CheckDlgButton(GetParent(hwnd), IDC_COLOR, BST_UNCHECKED);
				CheckDlgButton(GetParent(hwnd), IDC_BOLD, BST_UNCHECKED);
				CheckDlgButton(GetParent(hwnd), IDC_UNDERLINE, BST_UNCHECKED);
				CheckDlgButton(GetParent(hwnd), IDC_ITALICS, BST_UNCHECKED);
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_BKGCOLOR, 0), 0);
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_COLOR, 0), 0);
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_BOLD, 0), 0);
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_UNDERLINE, 0), 0);
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_ITALICS, 0), 0);
				return TRUE;
			}

			if (wParam == 0x4c && isCtrl && !isAlt) { // ctrl-l (paste clean text)
				CheckDlgButton(GetParent(hwnd), IDC_BKGCOLOR, IsDlgButtonChecked(GetParent(hwnd), IDC_BKGCOLOR) == BST_UNCHECKED?BST_CHECKED:BST_UNCHECKED);
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_BKGCOLOR, 0), 0);
				return TRUE;
			}

			if (wParam == 0x46 && isCtrl && !isAlt) { // ctrl-f (paste clean text)
				if (IsWindowEnabled(GetDlgItem(GetParent(hwnd), IDC_FILTER)))
					SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_FILTER, 0), 0);
				return TRUE;
			}

			if (wParam == 0x4e && isCtrl && !isAlt) { // ctrl-n (nicklist)
				if (IsWindowEnabled(GetDlgItem(GetParent(hwnd), IDC_SHOWNICKLIST)))
					SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_SHOWNICKLIST, 0), 0);
				return TRUE;
			}

			if (wParam == 0x48 && isCtrl && !isAlt) { // ctrl-h (history)
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_HISTORY, 0), 0);
				return TRUE;
			}

			if (wParam == 0x4f && isCtrl && !isAlt) { // ctrl-o (options)
				if (IsWindowEnabled(GetDlgItem(GetParent(hwnd), IDC_CHANMGR)))
					SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHANMGR, 0), 0);
				return TRUE;
			}

			if ((wParam == 45 && isShift || wParam == 0x56 && isCtrl )&& !isAlt) { // ctrl-v (paste clean text)
				SendMessage(hwnd, EM_PASTESPECIAL, CF_TEXT, 0);
				return TRUE;
			}

			if (wParam == 0x57 && isCtrl && !isAlt) { // ctrl-w (close window)
				PostMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
				return TRUE;
			}

			if (wParam == VK_NEXT || wParam == VK_PRIOR) {
				HWND htemp = GetParent(hwnd);
				SendDlgItemMessage(htemp, IDC_LOG, msg, wParam, lParam);
				dat->lastEnterTime = 0;
				return TRUE;
			}

			if (wParam == VK_UP && isCtrl && !isAlt) {
				int iLen;
				GETTEXTLENGTHEX gtl = {0};
				SETTEXTEX ste;
				LOGFONT lf;
				char* lpPrevCmd = SM_GetPrevCommand(Parentsi->ptszID, Parentsi->pszModule);

				SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);

				LoadMsgDlgFont(17, &lf, NULL);
				ste.flags = ST_DEFAULT;
				ste.codepage = CP_ACP;
				if (lpPrevCmd)
					SendMessage(hwnd, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)lpPrevCmd);
				else
					SetWindowText(hwnd, _T(""));

				gtl.flags = GTL_PRECISE;
				gtl.codepage = CP_ACP;
				iLen = SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, (LPARAM)NULL);
				SendMessage(hwnd, EM_SCROLLCARET, 0,0);
				SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
				SendMessage(hwnd, EM_SETSEL,iLen,iLen);
				dat->lastEnterTime = 0;
				return TRUE;
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
				iLen = SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, (LPARAM)NULL);
				SendMessage(hwnd, EM_SCROLLCARET, 0,0);
				SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
				SendMessage(hwnd, EM_SETSEL,iLen,iLen);
				dat->lastEnterTime = 0;
				return TRUE;
			}

			if (wParam == VK_RETURN)
				break;
		}
		//fall through

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_KILLFOCUS:
		dat->lastEnterTime = 0;
		break;

	case WM_RBUTTONDOWN:
		{
			CHARRANGE sel, all = { 0, -1 };
			POINT pt;
			UINT uID = 0;
			HMENU hSubMenu;

			hSubMenu = GetSubMenu(g_hMenu, 4);
			CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM) hSubMenu, 0);
			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM) & sel);

			EnableMenuItem(hSubMenu, ID_MESSAGE_UNDO, SendMessage(hwnd, EM_CANUNDO, 0,0)?MF_ENABLED:MF_GRAYED);
			EnableMenuItem(hSubMenu, ID_MESSAGE_REDO, SendMessage(hwnd, EM_CANREDO, 0,0)?MF_ENABLED:MF_GRAYED);
			EnableMenuItem(hSubMenu, ID_MESSAGE_COPY, sel.cpMax!=sel.cpMin?MF_ENABLED:MF_GRAYED);
			EnableMenuItem(hSubMenu, ID_MESSAGE_CUT, sel.cpMax!=sel.cpMin?MF_ENABLED:MF_GRAYED);

			dat->lastEnterTime = 0;

			pt.x = (short) LOWORD(lParam);
			pt.y = (short) HIWORD(lParam);
			ClientToScreen(hwnd, &pt);

			uID = TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);
			switch (uID) {
			case 0:
				break;

			case ID_MESSAGE_UNDO:
				SendMessage(hwnd, EM_UNDO, 0, 0);
				break;

			case ID_MESSAGE_REDO:
				SendMessage(hwnd, EM_REDO, 0, 0);
				break;

			case ID_MESSAGE_COPY:
				SendMessage(hwnd, WM_COPY, 0, 0);
				break;

			case ID_MESSAGE_CUT:
				SendMessage(hwnd, WM_CUT, 0, 0);
				break;

			case ID_MESSAGE_PASTE:
				SendMessage(hwnd, EM_PASTESPECIAL, CF_TEXT, 0);
				break;

			case ID_MESSAGE_SELECTALL:
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) & all);
				break;

			case ID_MESSAGE_CLEAR:
				SetWindowText(hwnd, _T( "" ));
				break;
			}
			PostMessage(hwnd, WM_KEYUP, 0, 0 );
		}
		break;

	case WM_KEYUP:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		{
			CHARFORMAT2 cf;
			UINT u = 0;
			UINT u2 = 0;
			COLORREF cr;

			LoadMsgDlgFont(17, NULL, &cr);

			cf.cbSize = sizeof(CHARFORMAT2);
			cf.dwMask = CFM_BOLD|CFM_ITALIC|CFM_UNDERLINE|CFM_BACKCOLOR|CFM_COLOR;
			SendMessage(hwnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

			if (MM_FindModule(Parentsi->pszModule) && MM_FindModule(Parentsi->pszModule)->bColor) {
				int index = GetColorIndex(Parentsi->pszModule, cf.crTextColor);
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_COLOR);

				if (index >= 0) {
					Parentsi->bFGSet = TRUE;
					Parentsi->iFG = index;
				}

				if (u == BST_UNCHECKED && cf.crTextColor != cr)
					CheckDlgButton(GetParent(hwnd), IDC_COLOR, BST_CHECKED);
				else if (u == BST_CHECKED && cf.crTextColor == cr)
					CheckDlgButton(GetParent(hwnd), IDC_COLOR, BST_UNCHECKED);
			}

			if (MM_FindModule(Parentsi->pszModule) && MM_FindModule(Parentsi->pszModule)->bBkgColor) {
				int index = GetColorIndex(Parentsi->pszModule, cf.crBackColor);
				COLORREF crB = (COLORREF)DBGetContactSettingDword(NULL, "Chat", "ColorMessageBG", GetSysColor(COLOR_WINDOW));
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_BKGCOLOR);

				if (index >= 0) {
					Parentsi->bBGSet = TRUE;
					Parentsi->iBG = index;
				}
				if (u == BST_UNCHECKED && cf.crBackColor != crB)
					CheckDlgButton(GetParent(hwnd), IDC_BKGCOLOR, BST_CHECKED);
				else if (u == BST_CHECKED && cf.crBackColor == crB)
					CheckDlgButton(GetParent(hwnd), IDC_BKGCOLOR, BST_UNCHECKED);
			}

			if (MM_FindModule(Parentsi->pszModule) && MM_FindModule(Parentsi->pszModule)->bBold) {
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_BOLD);
				u2 = cf.dwEffects;
				u2 &= CFE_BOLD;
				if (u == BST_UNCHECKED && u2)
					CheckDlgButton(GetParent(hwnd), IDC_BOLD, BST_CHECKED);
				else if (u == BST_CHECKED && u2 == 0)
					CheckDlgButton(GetParent(hwnd), IDC_BOLD, BST_UNCHECKED);
			}

			if (MM_FindModule(Parentsi->pszModule) && MM_FindModule(Parentsi->pszModule)->bItalics) {
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_ITALICS);
				u2 = cf.dwEffects;
				u2 &= CFE_ITALIC;
				if (u == BST_UNCHECKED && u2)
					CheckDlgButton(GetParent(hwnd), IDC_ITALICS, BST_CHECKED);
				else if (u == BST_CHECKED && u2 == 0)
					CheckDlgButton(GetParent(hwnd), IDC_ITALICS, BST_UNCHECKED);
			}

			if (MM_FindModule(Parentsi->pszModule) && MM_FindModule(Parentsi->pszModule)->bUnderline) {
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_UNDERLINE);
				u2 = cf.dwEffects;
				u2 &= CFE_UNDERLINE;
				if (u == BST_UNCHECKED && u2)
					CheckDlgButton(GetParent(hwnd), IDC_UNDERLINE, BST_CHECKED);
				else if (u == BST_CHECKED && u2 == 0)
					CheckDlgButton(GetParent(hwnd), IDC_UNDERLINE, BST_UNCHECKED);
		}	}
		break;

	case EM_UNSUBCLASSED:
		mir_free(dat);
		return 0;
	}

	return CallWindowProc(OldMessageProc, hwnd, msg, wParam, lParam);
}

static INT_PTR CALLBACK FilterWndProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static SESSION_INFO* si = NULL;
	switch (uMsg) {
	case WM_INITDIALOG:
		si = (SESSION_INFO*)lParam;
		CheckDlgButton(hwndDlg, IDC_1, si->iLogFilterFlags&GC_EVENT_ACTION);
		CheckDlgButton(hwndDlg, IDC_2, si->iLogFilterFlags&GC_EVENT_MESSAGE);
		CheckDlgButton(hwndDlg, IDC_3, si->iLogFilterFlags&GC_EVENT_NICK);
		CheckDlgButton(hwndDlg, IDC_4, si->iLogFilterFlags&GC_EVENT_JOIN);
		CheckDlgButton(hwndDlg, IDC_5, si->iLogFilterFlags&GC_EVENT_PART);
		CheckDlgButton(hwndDlg, IDC_6, si->iLogFilterFlags&GC_EVENT_TOPIC);
		CheckDlgButton(hwndDlg, IDC_7, si->iLogFilterFlags&GC_EVENT_ADDSTATUS);
		CheckDlgButton(hwndDlg, IDC_8, si->iLogFilterFlags&GC_EVENT_INFORMATION);
		CheckDlgButton(hwndDlg, IDC_9, si->iLogFilterFlags&GC_EVENT_QUIT);
		CheckDlgButton(hwndDlg, IDC_10, si->iLogFilterFlags&GC_EVENT_KICK);
		CheckDlgButton(hwndDlg, IDC_11, si->iLogFilterFlags&GC_EVENT_NOTICE);
		break;

	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
		SetTextColor((HDC)wParam,RGB(60,60,150));
		SetBkColor((HDC)wParam,GetSysColor(COLOR_WINDOW));
		return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			int iFlags = 0;

			if (IsDlgButtonChecked(hwndDlg, IDC_1) == BST_CHECKED)
				iFlags |= GC_EVENT_ACTION;
			if (IsDlgButtonChecked(hwndDlg, IDC_2) == BST_CHECKED)
				iFlags |= GC_EVENT_MESSAGE;
			if (IsDlgButtonChecked(hwndDlg, IDC_3) == BST_CHECKED)
				iFlags |= GC_EVENT_NICK;
			if (IsDlgButtonChecked(hwndDlg, IDC_4) == BST_CHECKED)
				iFlags |= GC_EVENT_JOIN;
			if (IsDlgButtonChecked(hwndDlg, IDC_5) == BST_CHECKED)
				iFlags |= GC_EVENT_PART;
			if (IsDlgButtonChecked(hwndDlg, IDC_6) == BST_CHECKED)
				iFlags |= GC_EVENT_TOPIC;
			if (IsDlgButtonChecked(hwndDlg, IDC_7) == BST_CHECKED)
				iFlags |= GC_EVENT_ADDSTATUS;
			if (IsDlgButtonChecked(hwndDlg, IDC_8) == BST_CHECKED)
				iFlags |= GC_EVENT_INFORMATION;
			if (IsDlgButtonChecked(hwndDlg, IDC_9) == BST_CHECKED)
				iFlags |= GC_EVENT_QUIT;
			if (IsDlgButtonChecked(hwndDlg, IDC_10) == BST_CHECKED)
				iFlags |= GC_EVENT_KICK;
			if (IsDlgButtonChecked(hwndDlg, IDC_11) == BST_CHECKED)
				iFlags |= GC_EVENT_NOTICE;

			if (iFlags&GC_EVENT_ADDSTATUS)
				iFlags |= GC_EVENT_REMOVESTATUS;

			SendMessage(GetParent(hwndDlg), GC_CHANGEFILTERFLAG, 0, (LPARAM)iFlags);
			if (si->bFilterEnabled)
				SendMessage(GetParent(hwndDlg), GC_REDRAWLOG, 0, 0);
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;
	}

	return(FALSE);
}

static LRESULT CALLBACK ButtonSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_RBUTTONUP:
		{
			HWND hFilter = GetDlgItem(GetParent(hwnd), IDC_FILTER);
			HWND hColor = GetDlgItem(GetParent(hwnd), IDC_COLOR);
			HWND hBGColor = GetDlgItem(GetParent(hwnd), IDC_BKGCOLOR);

			if (DBGetContactSettingByte(NULL, "Chat", "RightClickFilter", 0) != 0) {
				if (hFilter == hwnd)
					SendMessage(GetParent(hwnd), GC_SHOWFILTERMENU, 0, 0);
				if (hColor == hwnd)
					SendMessage(GetParent(hwnd), GC_SHOWCOLORCHOOSER, 0, (LPARAM)IDC_COLOR);
				if (hBGColor == hwnd)
					SendMessage(GetParent(hwnd), GC_SHOWCOLORCHOOSER, 0, (LPARAM)IDC_BKGCOLOR);
		}	}
		break;
	}

	return CallWindowProc(OldFilterButtonProc, hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK LogSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_LBUTTONUP:
		{
			CHARRANGE sel;

			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM) &sel);
			if (sel.cpMin != sel.cpMax)
			{
				SendMessage(hwnd, WM_COPY, 0, 0);
				sel.cpMin = sel.cpMax ;
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) & sel);
			}
			SetFocus(GetDlgItem(GetParent(hwnd), IDC_MESSAGE));
			break;
		}
	case WM_KEYDOWN:
		if (wParam == 0x57 && GetKeyState(VK_CONTROL) & 0x8000) { // ctrl-w (close window)
			PostMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
			return TRUE;
		}
		break;

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			CHARRANGE sel;
			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM) &sel);
			if (sel.cpMin != sel.cpMax) {
				sel.cpMin = sel.cpMax ;
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) & sel);
		}	}
		break;

	case WM_CHAR:
		SetFocus(GetDlgItem(GetParent(hwnd), IDC_MESSAGE));
		SendMessage(GetDlgItem(GetParent(hwnd), IDC_MESSAGE), WM_CHAR, wParam, lParam);
		break;
	}

	return CallWindowProc(OldLogProc, hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK TabSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL bDragging = FALSE;
	static int iBeginIndex = 0;
	switch (msg) {
	case WM_LBUTTONDOWN:
		{
			TCHITTESTINFO tci = {0};
			tci.pt.x=(short)LOWORD(GetMessagePos());
			tci.pt.y=(short)HIWORD(GetMessagePos());
			if (DragDetect(hwnd, tci.pt) && TabCtrl_GetItemCount(hwnd) > 1 ) {
				int i;
				tci.flags = TCHT_ONITEM;

				ScreenToClient(hwnd, &tci.pt);
				i= TabCtrl_HitTest(hwnd, &tci);
				if (i != -1) {
					TCITEM tc;
					SESSION_INFO* s = NULL;

					tc.mask = TCIF_PARAM;
					TabCtrl_GetItem(hwnd, i, &tc);
					s = (SESSION_INFO* ) tc.lParam;
					if (s)
					{
						BOOL bOnline = DBGetContactSettingWord(s->hContact, s->pszModule, "Status", ID_STATUS_OFFLINE) == ID_STATUS_ONLINE?TRUE:FALSE;
						bDragging = TRUE;
						iBeginIndex = i;
						ImageList_BeginDrag(hIconsList, bOnline?(MM_FindModule(s->pszModule))->OnlineIconIndex:(MM_FindModule(s->pszModule))->OfflineIconIndex, 8, 8);
						ImageList_DragEnter(hwnd,tci.pt.x, tci.pt.y);
						SetCapture(hwnd);
					}
					return TRUE;
			}	}
			else PostMessage(GetParent(hwnd), GC_TABCLICKED, 0, 0 );
		}
		break;
	case WM_CAPTURECHANGED:
		bDragging = FALSE;
		ImageList_DragLeave(hwnd);
		ImageList_EndDrag();
		break;

	case WM_MOUSEMOVE:
		if (bDragging) {
			TCHITTESTINFO tci = {0};
			tci.pt.x=(short)LOWORD(GetMessagePos());
			tci.pt.y=(short)HIWORD(GetMessagePos());
			ScreenToClient(hwnd, &tci.pt);
			ImageList_DragMove(tci.pt.x, tci.pt.y);
		}
		break;

	case WM_LBUTTONUP:
		if (bDragging && ReleaseCapture()) {
			TCHITTESTINFO tci = {0};
			int i;
			tci.pt.x=(short)LOWORD(GetMessagePos());
			tci.pt.y=(short)HIWORD(GetMessagePos());
			tci.flags = TCHT_ONITEM;
			bDragging = FALSE;
			ImageList_DragLeave(hwnd);
			ImageList_EndDrag();

			ScreenToClient(hwnd, &tci.pt);
			i= TabCtrl_HitTest(hwnd, &tci);
			if (i != -1 && i != iBeginIndex)
				SendMessage(GetParent(hwnd), GC_DROPPEDTAB, (WPARAM)i, (LPARAM)iBeginIndex);
		}
		break;

	case WM_LBUTTONDBLCLK:
		{
			TCHITTESTINFO tci = {0};
			int i = 0;

			tci.pt.x=(short)LOWORD(GetMessagePos());
			tci.pt.y=(short)HIWORD(GetMessagePos());
			tci.flags = TCHT_ONITEM;

			ScreenToClient(hwnd, &tci.pt);
			i = TabCtrl_HitTest(hwnd, &tci);
			if (i != -1 && g_Settings.TabCloseOnDblClick)
				PostMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CLOSE, BN_CLICKED), 0);
		}
		break;

	case WM_MBUTTONUP:
		{
			TCHITTESTINFO tci = {0};
			int i = 0;

			tci.pt.x=(short)LOWORD(GetMessagePos());
			tci.pt.y=(short)HIWORD(GetMessagePos());
			tci.flags = TCHT_ONITEM;

			ScreenToClient(hwnd, &tci.pt);
			i = TabCtrl_HitTest(hwnd, &tci);
			if (i != -1 ) {
				TCITEM tc;
				SESSION_INFO* si ;

				tc.mask = TCIF_PARAM;
				TabCtrl_GetItem(hwnd, i, &tc);
				si = (SESSION_INFO* ) tc.lParam;
				if (si)
					SendMessage(GetParent(hwnd), GC_REMOVETAB, 1, (LPARAM) si );
		}	}
		break;
	}

	return CallWindowProc(OldTabProc, hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK NicklistSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_ERASEBKGND:
		{
			HDC dc = (HDC)wParam;
			SESSION_INFO* parentdat =(SESSION_INFO*)GetWindowLongPtr(GetParent(hwnd),GWLP_USERDATA);
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

					if (rc.bottom-rc.top > items * height) {
						rc.top = items*height;
						FillRect(dc, &rc, hListBkgBrush);
		}	}	}	}
		return 1;

	case WM_KEYDOWN:
		if (wParam == 0x57 && GetKeyState(VK_CONTROL) & 0x8000) { // ctrl-w (close window)
			PostMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
			return TRUE;
		}
		break;

	case WM_RBUTTONDOWN:
		SendMessage(hwnd, WM_LBUTTONDOWN, wParam, lParam);
		break;

	case WM_RBUTTONUP:
		SendMessage(hwnd, WM_LBUTTONUP, wParam, lParam);
		break;

	case WM_MEASUREITEM:
		{
			MEASUREITEMSTRUCT *mis = (MEASUREITEMSTRUCT *) lParam;
			if (mis->CtlType == ODT_MENU)
				return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);
			return FALSE;
		}
	case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *) lParam;
			if (dis->CtlType == ODT_MENU)
				return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
			return FALSE;
		}
	case WM_CONTEXTMENU:
		{
			TVHITTESTINFO hti;
			int item;
			int height;
			USERINFO * ui;
			SESSION_INFO* parentdat =(SESSION_INFO*)GetWindowLongPtr(GetParent(hwnd),GWLP_USERDATA);

			hti.pt.x = (short) LOWORD(lParam);
			hti.pt.y = (short) HIWORD(lParam);
			if (hti.pt.x == -1 && hti.pt.y == -1) {
				int index = SendMessage(hwnd, LB_GETCURSEL, 0, 0);
				int top = SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
				height = SendMessage(hwnd, LB_GETITEMHEIGHT, 0, 0);
				hti.pt.x = 4;
				hti.pt.y = (index - top)*height + 1;
			}
			else ScreenToClient(hwnd,&hti.pt);

			item = LOWORD(SendMessage(GetDlgItem(GetParent(hwnd), IDC_LIST), LB_ITEMFROMPOINT, 0, MAKELPARAM(hti.pt.x, hti.pt.y)));
			ui = SM_GetUserFromIndex(parentdat->ptszID, parentdat->pszModule, item);
			// ui = (USERINFO *)SendMessage(GetDlgItem(GetParent(hwnd), IDC_LIST), LB_GETITEMDATA, item, 0);
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

				case ID_MESS:
					DoEventHookAsync(GetParent(hwnd), parentdat->ptszID, parentdat->pszModule, GC_USER_PRIVMESS, ui->pszUID, NULL, (LPARAM)NULL);
					break;

				default:
					DoEventHookAsync(GetParent(hwnd), parentdat->ptszID, parentdat->pszModule, GC_USER_NICKLISTMENU, ui->pszUID, NULL, (LPARAM)uID);
					break;
				}
				DestroyGCMenu(&hMenu, 1);
				return TRUE;
		}	}
		break;

	case WM_MOUSEMOVE:
		{
			SESSION_INFO* parentdat =(SESSION_INFO*)GetWindowLongPtr(GetParent(hwnd),GWLP_USERDATA);
			if ( parentdat ) {
				POINT p;
				GetCursorPos(&p);
				SendMessage( parentdat->hwndTooltip,TTM_TRACKPOSITION,0,(LPARAM)MAKELPARAM(p.x + 15,p.y + 15));
//				SendMessage( parentdat->hwndTooltip, TTM_ACTIVATE, TRUE, 0 );
		}	}
		break;

	}

	return CallWindowProc(OldNicklistProc, hwnd, msg, wParam, lParam);
}

static int RestoreWindowPosition(HWND hwnd, HANDLE hContact, char * szModule, char * szNamePrefix, UINT showCmd)
{
	WINDOWPLACEMENT wp;
	char szSettingName[64];
	int x,y, width, height;;

	wp.length=sizeof(wp);
	GetWindowPlacement(hwnd,&wp);
	wsprintfA(szSettingName,"%sx",szNamePrefix);
	x=DBGetContactSettingDword(hContact,szModule,szSettingName,-1);
	wsprintfA(szSettingName,"%sy",szNamePrefix);
	y=(int)DBGetContactSettingDword(hContact,szModule,szSettingName,-1);
	wsprintfA(szSettingName,"%swidth",szNamePrefix);
	width=DBGetContactSettingDword(hContact,szModule,szSettingName,-1);
	wsprintfA(szSettingName,"%sheight",szNamePrefix);
	height=DBGetContactSettingDword(hContact,szModule,szSettingName,-1);

	if (x==-1)
		return 0;
	wp.rcNormalPosition.left=x;
	wp.rcNormalPosition.top=y;
	wp.rcNormalPosition.right=wp.rcNormalPosition.left+width;
	wp.rcNormalPosition.bottom=wp.rcNormalPosition.top+height;
	wp.showCmd = showCmd;
	SetWindowPlacement(hwnd,&wp);
	return 1;
}

int GetTextPixelSize( TCHAR* pszText, HFONT hFont, BOOL bWidth)
{
	HDC hdc;
	HFONT hOldFont;
	RECT rc = {0};
	int i;

	if (!pszText || !hFont)
		return 0;

	hdc = GetDC(NULL);
	hOldFont = SelectObject(hdc, hFont);
	i = DrawText(hdc, pszText , -1, &rc, DT_CALCRECT);
	SelectObject(hdc, hOldFont);
	ReleaseDC(NULL,hdc);
	return bWidth ? rc.right - rc.left : rc.bottom - rc.top;
}

static void __cdecl phase2(void * lParam)
{
	SESSION_INFO* si = (SESSION_INFO*) lParam;
	Sleep(30);
	if (si && si->hWnd)
		PostMessage(si->hWnd, GC_REDRAWLOG3, 0, 0);
}

INT_PTR CALLBACK RoomWndProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	SESSION_INFO* si;

	si = (SESSION_INFO*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			int mask;
			HWND hNickList = GetDlgItem(hwndDlg,IDC_LIST);
			si = (SESSION_INFO*)lParam;
			si->pAccPropServicesForNickList = NULL;
			CoCreateInstance(&CLSID_AccPropServices, NULL, CLSCTX_SERVER, &IID_IAccPropServices, &si->pAccPropServicesForNickList);
			TranslateDialogDefault(hwndDlg);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)si);
			OldSplitterProc=(WNDPROC)SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_SPLITTERX),GWLP_WNDPROC,(LONG_PTR)SplitterSubclassProc);
			SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_SPLITTERY),GWLP_WNDPROC,(LONG_PTR)SplitterSubclassProc);
			OldNicklistProc=(WNDPROC)SetWindowLongPtr(hNickList,GWLP_WNDPROC,(LONG_PTR)NicklistSubclassProc);
			OldTabProc=(WNDPROC)SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_TAB),GWLP_WNDPROC,(LONG_PTR)TabSubclassProc);
			OldLogProc=(WNDPROC)SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_LOG),GWLP_WNDPROC,(LONG_PTR)LogSubclassProc);
			OldFilterButtonProc=(WNDPROC)SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_FILTER),GWLP_WNDPROC,(LONG_PTR)ButtonSubclassProc);
			SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_COLOR),GWLP_WNDPROC,(LONG_PTR)ButtonSubclassProc);
			SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_BKGCOLOR),GWLP_WNDPROC,(LONG_PTR)ButtonSubclassProc);
			OldMessageProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_MESSAGE), GWLP_WNDPROC,(LONG_PTR)MessageSubclassProc);
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SUBCLASSED, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_AUTOURLDETECT, 1, 0);
			mask = (int)SendDlgItemMessage(hwndDlg, IDC_LOG, EM_GETEVENTMASK, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETEVENTMASK, 0, mask | ENM_LINK | ENM_MOUSEEVENTS);
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_LIMITTEXT, (WPARAM)sizeof(TCHAR)*0x7FFFFFFF, 0);
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETOLECALLBACK, 0, (LPARAM) & reOleCallback);

			//			RichUtil_SubClass(GetDlgItem(hwndDlg, IDC_MESSAGE));
			//			RichUtil_SubClass(GetDlgItem(hwndDlg, IDC_LOG));

			si->hwndStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP| SBT_TOOLTIPS , 0, 0, 0, 0, hwndDlg, NULL, g_hInst, NULL);
			SendMessage(si->hwndStatus,SB_SETMINHEIGHT,GetSystemMetrics(SM_CYSMICON),0);
			TabCtrl_SetMinTabWidth(GetDlgItem(hwndDlg, IDC_TAB), 80);
			TabCtrl_SetImageList(GetDlgItem(hwndDlg, IDC_TAB), hIconsList);

			// enable tooltips
			si->iOldItemID = -1;
			si->hwndTooltip = CreateWindow(TOOLTIPS_CLASS,NULL,TTS_ALWAYSTIP,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,hNickList,(HMENU)NULL,g_hInst,NULL);
			SetWindowPos(si->hwndTooltip, HWND_TOPMOST,0, 0, 0, 0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			{
				TOOLINFO ti = {0};
				ti.cbSize = sizeof(TOOLINFO);
				ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS | TTF_TRANSPARENT;
				ti.hwnd   = hwndDlg;
				ti.hinst  = g_hInst;
				ti.uId    = (UINT_PTR)hNickList;
				ti.lpszText  = LPSTR_TEXTCALLBACK;
				//GetClientRect( hNickList, &ti.rect );
				SendMessage( si->hwndTooltip, TTM_ADDTOOL, 0, ( LPARAM )&ti );
				SendMessage( si->hwndTooltip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 20000 );
				SendMessage( si->hwndTooltip, TTM_SETMAXTIPWIDTH, 0, 300);

				//SendMessage( psi->hwndTooltip, TTM_TRACKACTIVATE, TRUE, ( LPARAM )&ti );
			}

			// restore previous tabs
			if (g_Settings.TabsEnable && DBGetContactSettingByte(NULL, "Chat", "TabRestore", 0)) {
				TABLIST * node = g_TabList;
				while (node) {
					SESSION_INFO* s = SM_FindSession(node->pszID, node->pszModule);
					if (s)
						SendMessage(hwndDlg, GC_ADDTAB, -1, (LPARAM)s);

					node = node->next;
			}	}

			TabM_RemoveAll();

			EnableWindow(GetDlgItem(hwndDlg, IDC_SMILEY), TRUE);

			SendMessage(GetDlgItem(hwndDlg, IDC_LOG), EM_HIDESELECTION, TRUE, 0);

			SendMessage(hwndDlg, GC_SETWNDPROPS, 0, 0);
			SendMessage(hwndDlg, GC_UPDATESTATUSBAR, 0, 0);
			SendMessage(hwndDlg, GC_UPDATETITLE, 0, 0);
			SendMessage(hwndDlg, GC_SETWINDOWPOS, 0, 0);
		}
		break;

	case GC_SETWNDPROPS:
		{
			HICON hIcon;
			LoadGlobalSettings();
			InitButtons(hwndDlg, si);

			hIcon = si->wStatus==ID_STATUS_ONLINE?MM_FindModule(si->pszModule)->hOnlineIcon:MM_FindModule(si->pszModule)->hOfflineIcon;
			// stupid hack to make icons show. I dunno why this is needed currently
			if (!hIcon) {
				MM_IconsChanged();
				hIcon = si->wStatus==ID_STATUS_ONLINE?MM_FindModule(si->pszModule)->hOnlineIcon:MM_FindModule(si->pszModule)->hOfflineIcon;
			}

			SendMessage(hwndDlg, GC_FIXTABICONS, 0, 0);
			SendMessage(si->hwndStatus, SB_SETICON, 0,(LPARAM)hIcon);
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIconEx( "window", TRUE ));
			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIconEx( "window", FALSE ));

			SendMessage(GetDlgItem(hwndDlg, IDC_LOG), EM_SETBKGNDCOLOR , 0, g_Settings.crLogBackground);

			if (g_Settings.TabsEnable) {
				int mask = (int)GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TAB), GWL_STYLE);
				if (g_Settings.TabsAtBottom)
					mask |= TCS_BOTTOM;
				else
					mask &= ~TCS_BOTTOM;
				SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TAB), GWL_STYLE, (LONG_PTR)mask);
			}

			{ //messagebox
				COLORREF	crFore;

				CHARFORMAT2 cf;
				LoadMsgDlgFont(17, NULL, &crFore);
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwMask = CFM_COLOR|CFM_BOLD|CFM_UNDERLINE|CFM_BACKCOLOR;
				cf.dwEffects = 0;
				cf.crTextColor = crFore;
				cf.crBackColor = (COLORREF)DBGetContactSettingDword(NULL, "Chat", "ColorMessageBG", GetSysColor(COLOR_WINDOW));
				SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), EM_SETBKGNDCOLOR , 0, DBGetContactSettingDword(NULL, "Chat", "ColorMessageBG", GetSysColor(COLOR_WINDOW)));
				SendDlgItemMessage(hwndDlg, IDC_MESSAGE, WM_SETFONT, (WPARAM) g_Settings.MessageBoxFont, MAKELPARAM(TRUE, 0));
				SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETCHARFORMAT, (WPARAM)SCF_ALL , (LPARAM)&cf);
			}
			{ // nicklist
				int ih;
				int ih2;
				int font;
				int height;

				ih = GetTextPixelSize( _T("AQGglo"), g_Settings.UserListFont,FALSE);
				ih2 = GetTextPixelSize( _T("AQGglo"), g_Settings.UserListHeadingsFont,FALSE);
				height = DBGetContactSettingByte(NULL, "Chat", "NicklistRowDist", 12);
				font = ih > ih2?ih:ih2;

				// make sure we have space for icon!
				if (g_Settings.ShowContactStatus)
					font = font > 16 ? font : 16;

				SendMessage(GetDlgItem(hwndDlg, IDC_LIST), LB_SETITEMHEIGHT, 0, (LPARAM)height > font ? height : font);
				InvalidateRect(GetDlgItem(hwndDlg, IDC_LIST), NULL, TRUE);
			}
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
			SendMessage(hwndDlg, GC_REDRAWLOG2, 0, 0);
		}
		break;

	case GC_UPDATETITLE:
		{
			TCHAR szTemp [100];
			switch(si->iType) {
			case GCW_CHATROOM:
				mir_sntprintf(szTemp, SIZEOF(szTemp),
					(si->nUsersInNicklist ==1) ? TranslateT("%s: Chat Room (%u user)") : TranslateT("%s: Chat Room (%u users)"),
					si->ptszName, si->nUsersInNicklist);
				break;
			case GCW_PRIVMESS:
				mir_sntprintf(szTemp, SIZEOF(szTemp),
					(si->nUsersInNicklist ==1) ? TranslateT("%s: Message Session") : TranslateT("%s: Message Session (%u users)"),
					si->ptszName, si->nUsersInNicklist);
				break;
			case GCW_SERVER:
				mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%s: Server"), si->ptszName);
				break;
			}
			SetWindowText(hwndDlg, szTemp);
		}
		break;

	case GC_UPDATESTATUSBAR:
		{
			HICON hIcon;
			int iStatusbarParts[2];
			TCHAR* ptszDispName = MM_FindModule(si->pszModule)->ptszModDispName;
			int x = 12;

			x += GetTextPixelSize(ptszDispName, (HFONT)SendMessage(si->hwndStatus,WM_GETFONT,0,0), TRUE);
			x += GetSystemMetrics(SM_CXSMICON);
			iStatusbarParts[0] = x; iStatusbarParts[1] = -1;
			SendMessage(si->hwndStatus,SB_SETPARTS,2 ,(LPARAM)&iStatusbarParts);

			hIcon = si->wStatus==ID_STATUS_ONLINE?MM_FindModule(si->pszModule)->hOnlineIcon:MM_FindModule(si->pszModule)->hOfflineIcon;

			// stupid hack to make icons show. I dunno why this is needed currently
			if (!hIcon) {
				MM_IconsChanged();
				hIcon = si->wStatus==ID_STATUS_ONLINE?MM_FindModule(si->pszModule)->hOnlineIcon:MM_FindModule(si->pszModule)->hOfflineIcon;
			}

			SendMessage(si->hwndStatus, SB_SETICON, 0,(LPARAM)hIcon);
			SendMessage(hwndDlg, GC_FIXTABICONS, 0, 0);

			SendMessage(si->hwndStatus, SB_SETTEXT,0,(LPARAM)ptszDispName);

			SendMessage(si->hwndStatus, SB_SETTEXT,1,(LPARAM)(si->ptszStatusbarText ? si->ptszStatusbarText : _T("")));
			SendMessage(si->hwndStatus, SB_SETTIPTEXT,1,(LPARAM)(si->ptszStatusbarText ? si->ptszStatusbarText : _T("")));
			return TRUE;
		}
		break;

	case GC_SETWINDOWPOS:
		{
			SESSION_INFO* pActive = GetActiveSession();
			WINDOWPLACEMENT wp;
			RECT screen;
			int savePerContact = DBGetContactSettingByte(NULL, "Chat", "SavePosition", 0);

			wp.length=sizeof(wp);
			GetWindowPlacement(hwndDlg,&wp);
			SystemParametersInfo(SPI_GETWORKAREA, 0,  &screen, 0);

			if (si->iX) {
				wp.rcNormalPosition.left = si->iX;
				wp.rcNormalPosition.top = si->iY;
				wp.rcNormalPosition.right = wp.rcNormalPosition.left + si->iWidth;
				wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + si->iHeight;
				wp.showCmd = SW_HIDE;
				SetWindowPlacement(hwndDlg,&wp);
				//				SetWindowPos(hwndDlg, 0, si->iX,si->iY, si->iWidth, si->iHeight, SWP_NOZORDER |SWP_HIDEWINDOW|SWP_NOACTIVATE);
				break;
			}
			if (savePerContact) {
				if (RestoreWindowPosition(hwndDlg, g_Settings.TabsEnable?NULL:si->hContact, "Chat", "room", SW_HIDE))
					break;
				SetWindowPos(hwndDlg, 0, (screen.right-screen.left)/2- (550)/2,(screen.bottom-screen.top)/2- (400)/2, (550), (400), SWP_NOZORDER |SWP_HIDEWINDOW|SWP_NOACTIVATE);
			}
			else SetWindowPos(hwndDlg, 0, (screen.right-screen.left)/2- (550)/2,(screen.bottom-screen.top)/2- (400)/2, (550), (400), SWP_NOZORDER |SWP_HIDEWINDOW|SWP_NOACTIVATE);

			if (!g_Settings.TabsEnable && pActive && pActive->hWnd && DBGetContactSettingByte(NULL, "Chat", "CascadeWindows", 1)) {
				RECT rcThis, rcNew;
				int dwFlag = SWP_NOZORDER|SWP_NOACTIVATE;
				if (!IsWindowVisible ((HWND)wParam))
					dwFlag |= SWP_HIDEWINDOW;

				GetWindowRect(hwndDlg, &rcThis);
				GetWindowRect(pActive->hWnd, &rcNew);

				{
					int offset = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME);
					SetWindowPos((HWND) hwndDlg, 0, rcNew.left + offset, rcNew.top + offset, rcNew.right-rcNew.left, rcNew.bottom-rcNew.top, dwFlag);
		}	}	}
		break;

	case GC_SAVEWNDPOS:
		{
			WINDOWPLACEMENT wp = { 0 };

			wp.length = sizeof(wp);
			GetWindowPlacement(hwndDlg, &wp);
			g_Settings.iX = wp.rcNormalPosition.left;
			g_Settings.iY = wp.rcNormalPosition.top;
			g_Settings.iWidth = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
			g_Settings.iHeight = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;

			if (!lParam) {
				si->iX = g_Settings.iX;
				si->iY = g_Settings.iY;
				si->iWidth = g_Settings.iWidth;
				si->iHeight = g_Settings.iHeight;
		}	}
		break;

	case WM_SIZE:
		{
			UTILRESIZEDIALOG urd;

			if (wParam == SIZE_MAXIMIZED)
				PostMessage(hwndDlg, GC_SCROLLTOBOTTOM, 0, 0);

			if (IsIconic(hwndDlg)) break;
			SendMessage(si->hwndStatus, WM_SIZE, 0, 0);
			ZeroMemory(&urd,sizeof(urd));
			urd.cbSize=sizeof(urd);
			urd.hInstance=g_hInst;
			urd.hwndDlg=hwndDlg;
			urd.lParam=(LPARAM)si;
			urd.lpTemplate=MAKEINTRESOURCEA(IDD_CHANNEL);
			urd.pfnResizer=RoomWndResize;
			CallService(MS_UTILS_RESIZEDIALOG,0,(LPARAM)&urd);

			InvalidateRect(si->hwndStatus, NULL, TRUE);
			RedrawWindow(GetDlgItem(hwndDlg,IDC_MESSAGE), NULL, NULL, RDW_INVALIDATE);
			RedrawWindow(GetDlgItem(hwndDlg,IDOK), NULL, NULL, RDW_INVALIDATE);
			SendMessage(hwndDlg,GC_SAVEWNDPOS,0,1);
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
				while ( index < 59) {
					if (pLog->next == NULL)
						break;

					pLog = pLog->next;
					if (si->iType != GCW_CHATROOM || !si->bFilterEnabled || (si->iLogFilterFlags&pLog->iType) != 0)
						index++;
				}
				Log_StreamInEvent(hwndDlg, pLog, si, TRUE, FALSE);
				mir_forkthread(phase2, si);
			}
			else Log_StreamInEvent(hwndDlg, si->pLogEnd, si, TRUE, FALSE);
		}
		else SendMessage(hwndDlg, GC_EVENT_CONTROL + WM_USER+500, WINDOW_CLEARLOG, 0);
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

	case GC_ADDLOG:
		if (si->pLogEnd)
			Log_StreamInEvent(hwndDlg, si->pLog, si, FALSE, FALSE);
		else
			SendMessage(hwndDlg, GC_EVENT_CONTROL + WM_USER+500, WINDOW_CLEARLOG, 0);
		break;

	case GC_SWITCHNEXTTAB:
		{
			int total = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_TAB));
			int i = TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_TAB));
			if (i != -1 && total != -1 && total != 1) {
				if (i < total -1)
					i++;
				else
					i=0;
				TabCtrl_SetCurSel(GetDlgItem(hwndDlg, IDC_TAB), i);
				PostMessage(hwndDlg, GC_TABCLICKED, 0, 0 );
		}	}
		break;

	case GC_SWITCHPREVTAB:
		{
			int total = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_TAB));
			int i = TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_TAB));
			if (i != -1 && total != -1 && total != 1) {
				if (i > 0)
					i--;
				else
					i=total-1;
				TabCtrl_SetCurSel(GetDlgItem(hwndDlg, IDC_TAB), i);
				PostMessage(hwndDlg, GC_TABCLICKED, 0, 0 );
		}	}
		break;

	case GC_SWITCHTAB:
		{
			int total = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_TAB));
			int i = TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_TAB));
			if (i != -1 && total != -1 && total != 1 && i != lParam && total > lParam) {
				TabCtrl_SetCurSel(GetDlgItem(hwndDlg, IDC_TAB), lParam);
				PostMessage(hwndDlg, GC_TABCLICKED, 0, 0 );
		}	}
		break;

	case GC_REMOVETAB:
		{
			SESSION_INFO* s2;
			int i = -1;
			int tabId = 0;
			SESSION_INFO* s1 = (SESSION_INFO*) lParam;

			tabId = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_TAB));

			if (s1) {
				if (tabId) {
					for (i = 0; i < tabId; i++) {
						int ii;
						TCITEM tci = {0};
						tci.mask = TCIF_PARAM ;
						ii = TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_TAB), i, &tci);
						if (ii != -1) {
							s2 = (SESSION_INFO*)tci.lParam;
							if (s1 == s2)
								goto END_REMOVETAB;
				}	}	}
			}
			else i = TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_TAB));

END_REMOVETAB:
			if (i != -1 && i < tabId) {
				TCITEM id = {0};
				SESSION_INFO* s;
				TabCtrl_DeleteItem(GetDlgItem(hwndDlg, IDC_TAB), i);
				id.mask = TCIF_PARAM;
				if (!TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_TAB), i, &id)) {
					if (!TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_TAB), i-1, &id)) {
						SendMessage(hwndDlg, WM_CLOSE, 0, 0);
						break;
				}	}

				s = (SESSION_INFO*)id.lParam;
				if (s)
					ShowRoom(s, (WPARAM)WINDOW_VISIBLE, wParam == 1?FALSE:TRUE);
		}	}
		break;

	case GC_ADDTAB:
		{
			TCITEM tci;
			int tabId;
			WORD w = 0;
			int i = 0;
			int indexfound = -1;
			int lastlocked = -1;
			BOOL bFound = FALSE;
			SESSION_INFO* s2;
			SESSION_INFO* s1 = (SESSION_INFO*) lParam;

			tci.mask = TCIF_PARAM ;
			tabId = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_TAB));

			// does the tab already exist?
			for (i = 0; i < tabId; i++) {
				TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_TAB), i, &tci);
				s2 = (SESSION_INFO*)tci.lParam;
				if (s2) {
					if (s1 == s2 && !bFound) {
						if (!bFound) {
							bFound = TRUE;
							indexfound = i;
					}	}

					w = DBGetContactSettingWord(s2->hContact, s2->pszModule, "TabPosition", 0);
					if (w)
						lastlocked = (int)w;
			}	}

			w = 0;

			if (!bFound) { // create a new tab
				int insertat;
				TCHAR szTemp [30];

				lstrcpyn(szTemp, s1->ptszName, 21);
				if (lstrlen(s1->ptszName) >20)
					lstrcpyn(szTemp+20, _T("..."), 4);

				tci.mask = TCIF_TEXT|TCIF_PARAM ;
				tci.pszText = szTemp;
				tci.lParam = lParam;

				// determine insert position
				w = DBGetContactSettingWord(s1->hContact, s1->pszModule, "TabPosition", 0);
				if (wParam == -1)
					insertat = w == 0?tabId:(int)w-1;
				else
					insertat = (int)wParam;

				w = TabCtrl_InsertItem( GetDlgItem(hwndDlg, IDC_TAB), insertat, &tci );
				SendMessage(hwndDlg, GC_FIXTABICONS, 0, (LPARAM)s1);
			}

			if (wParam == -1) {
				if (bFound)
					TabCtrl_SetCurSel(GetDlgItem(hwndDlg, IDC_TAB), indexfound);
				else
					TabCtrl_SetCurSel(GetDlgItem(hwndDlg, IDC_TAB), w);
		}	}
		break;

	case GC_FIXTABICONS:
		{
			SESSION_INFO* s = (SESSION_INFO*) lParam;
			SESSION_INFO* s2;
			int i;
			if (s) {
				TCITEM tci;
				int tabId;

				tabId = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_TAB));
				for (i = 0; i < tabId; i++) {
					tci.mask = TCIF_PARAM|TCIF_IMAGE ;
					TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_TAB), i, &tci);
					s2 = (SESSION_INFO*)tci.lParam;
					if (s2 && s == s2) {
						int image = 0;
						if (!(s2->wState&GC_EVENT_HIGHLIGHT)) {
							image = s2->wStatus==ID_STATUS_ONLINE?MM_FindModule(s2->pszModule)->OnlineIconIndex:MM_FindModule(s2->pszModule)->OfflineIconIndex;
							if (s2->wState&STATE_TALK)
								image++;
						}

						if (tci.iImage != image) {
							tci.mask = TCIF_IMAGE ;
							tci.iImage = image;
							TabCtrl_SetItem(GetDlgItem(hwndDlg, IDC_TAB), i, &tci);
				}	}	}
			}
			else RedrawWindow(GetDlgItem(hwndDlg, IDC_TAB), NULL, NULL, RDW_INVALIDATE);
		}
		break;

	case GC_SETMESSAGEHIGHLIGHT:
		{
			SESSION_INFO* s = (SESSION_INFO*) lParam;
			SESSION_INFO* s2;
			int i;
			if (s) {
				TCITEM tci;
				int tabId;

				tabId = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_TAB));
				for (i = 0; i < tabId; i++) {
					tci.mask = TCIF_PARAM ;
					TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_TAB), i, &tci);
					s2 = (SESSION_INFO*)tci.lParam;
					if (s2 && s == s2) { // highlight
						s2->wState |= GC_EVENT_HIGHLIGHT;
						if (SM_FindSession(si->ptszID, si->pszModule) == s2)
							si->wState = s2->wState;
						SendMessage(hwndDlg, GC_FIXTABICONS, 0, (LPARAM)s2);
						if (DBGetContactSettingByte(NULL, "Chat", "FlashWindowHighlight", 0) != 0 && GetActiveWindow() != hwndDlg && GetForegroundWindow() != hwndDlg)
							SetTimer(hwndDlg, TIMERID_FLASHWND, 900, NULL);
						break;
				}	}
			}
			else RedrawWindow(GetDlgItem(hwndDlg, IDC_TAB), NULL, NULL, RDW_INVALIDATE);
		}
		break;

	case GC_SETTABHIGHLIGHT:
		{
			SESSION_INFO* s = (SESSION_INFO*) lParam;
			SESSION_INFO* s2;
			int i;
			if (s) {
				TCITEM tci;
				int tabId;

				tabId = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_TAB));
				for (i = 0; i < tabId; i++) {
					tci.mask = TCIF_PARAM ;
					TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_TAB), i, &tci);
					s2 = (SESSION_INFO*)tci.lParam;
					if (s2 && s == s2) { // highlight
						SendMessage(hwndDlg, GC_FIXTABICONS, 0, (LPARAM)s2);
						if (g_Settings.FlashWindow && GetActiveWindow() != hwndDlg && GetForegroundWindow() != hwndDlg)
							SetTimer(hwndDlg, TIMERID_FLASHWND, 900, NULL);
						break;
				}	}
			}
			else RedrawWindow(GetDlgItem(hwndDlg, IDC_TAB), NULL, NULL, RDW_INVALIDATE);
		}
		break;

	case GC_TABCHANGE:
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,(LONG_PTR)lParam);
		PostMessage(hwndDlg, GC_SCROLLTOBOTTOM, 0, 0);
		break;

	case GC_TABCLICKED:
		{
			int i;
			i = TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_TAB));
			if (i != -1) {
				SESSION_INFO* s;
				TCITEM id = {0};

				id.mask = TCIF_PARAM;
				TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_TAB), i, &id);
				s = (SESSION_INFO*)id.lParam;
				if (s) {
					if (s->wState&STATE_TALK) {
						s->wState &= ~STATE_TALK;
						DBWriteContactSettingWord(s->hContact, s->pszModule ,"ApparentMode",(LPARAM) 0);
					}

					if (s->wState&GC_EVENT_HIGHLIGHT) {
						s->wState &= ~GC_EVENT_HIGHLIGHT;

						if (CallService(MS_CLIST_GETEVENT, (WPARAM)s->hContact, (LPARAM)0))
							CallService(MS_CLIST_REMOVEEVENT, (WPARAM)s->hContact, (LPARAM)"chaticon");
					}

					SendMessage(hwndDlg, GC_FIXTABICONS, 0, (LPARAM)s);
					if (!s->hWnd) {
						ShowRoom(s, (WPARAM)WINDOW_VISIBLE, TRUE);
						SendMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0 );
		}	}	}	}
		break;

	case GC_DROPPEDTAB:
		{
			TCITEM tci;
			SESSION_INFO* s;
			int begin = (int)lParam;
			int end = (int) wParam;
			int i, tabId;
			if (begin == end)
				break;

			tci.mask = TCIF_PARAM ;
			TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_TAB),begin,  &tci);
			s = (SESSION_INFO*)tci.lParam;
			if (s) {
				TabCtrl_DeleteItem(GetDlgItem(hwndDlg, IDC_TAB), begin);

				SendMessage(hwndDlg, GC_ADDTAB, end, (LPARAM)s);

				// fix the "fixed" positions
				tabId = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_TAB));
				for (i = 0; i< tabId ; i++) {
					TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_TAB),i,  &tci);
					s = (SESSION_INFO*)tci.lParam;
					if (s && s->hContact && DBGetContactSettingWord(s->hContact, s->pszModule, "TabPosition", 0) != 0)
						DBWriteContactSettingWord(s->hContact, s->pszModule, "TabPosition", (WORD)(i + 1));
		}	}	}
		break;

	case GC_SESSIONNAMECHANGE:
		{
			TCITEM tci;
			int i;
			int tabId;
			SESSION_INFO* s2;
			SESSION_INFO* s1 = (SESSION_INFO* ) lParam;

			tabId = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_TAB));
			for (i = 0; i < tabId; i++) {
				int j;
				tci.mask = TCIF_PARAM ;
				j = TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_TAB), i, &tci);
				if (j != -1) {
					s2 = (SESSION_INFO*)tci.lParam;
					if (s1 == s2) {
						tci.mask = TCIF_TEXT ;
						tci.pszText = s1->ptszName;
						TabCtrl_SetItem(GetDlgItem(hwndDlg, IDC_TAB), i, &tci);
		}	}	}	}
		break;

	case GC_ACKMESSAGE:
		SendDlgItemMessage(hwndDlg,IDC_MESSAGE,EM_SETREADONLY,FALSE,0);
		SendDlgItemMessage(hwndDlg,IDC_MESSAGE,WM_SETTEXT,0, (LPARAM)"");
		return TRUE;

	case WM_CTLCOLORLISTBOX:
		SetBkColor((HDC) wParam, g_Settings.crUserListBGColor);
		return (INT_PTR) hListBkgBrush;

	case WM_MEASUREITEM:
		{
			MEASUREITEMSTRUCT *mis = (MEASUREITEMSTRUCT *) lParam;
			if (mis->CtlType == ODT_MENU)
			{
				return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);
			} else
			{
				int ih = GetTextPixelSize( _T("AQGgl'"), g_Settings.UserListFont,FALSE);
				int ih2 = GetTextPixelSize( _T("AQGg'"), g_Settings.UserListHeadingsFont,FALSE);
				int font = ih > ih2?ih:ih2;
				int height = DBGetContactSettingByte(NULL, "Chat", "NicklistRowDist", 12);

				// make sure we have space for icon!
				if (g_Settings.ShowContactStatus)
					font = font > 16 ? font : 16;

				mis->itemHeight = height > font?height:font;
			}
			return TRUE;
		}

	case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *) lParam;
			if (dis->CtlType == ODT_MENU)
			{
				return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
			} else
			if (dis->CtlID == IDC_LIST) {
				HFONT  hFont, hOldFont;
				HICON  hIcon;
				int offset;
				int height;
				int index = dis->itemID;
				USERINFO * ui = SM_GetUserFromIndex(si->ptszID, si->pszModule, index);
				if (ui) {
					int x_offset = 2;

					height = dis->rcItem.bottom - dis->rcItem.top;

					if (height&1)
						height++;
					if (height == 10)
						offset = 0;
					else
						offset = height/2 - 4;
					hIcon = SM_GetStatusIcon(si, ui);
					hFont = (ui->iStatusEx == 0) ? g_Settings.UserListFont : g_Settings.UserListHeadingsFont;
					hOldFont = (HFONT) SelectObject(dis->hDC, hFont);
					SetBkMode(dis->hDC, TRANSPARENT);

					if (dis->itemAction == ODA_FOCUS && dis->itemState & ODS_SELECTED)
						FillRect(dis->hDC, &dis->rcItem, hListSelectedBkgBrush);
					else //if (dis->itemState & ODS_INACTIVE)
						FillRect(dis->hDC, &dis->rcItem, hListBkgBrush);

					if (g_Settings.ShowContactStatus && g_Settings.ContactStatusFirst && ui->ContactStatus) {
						HICON hIcon = LoadSkinnedProtoIcon(si->pszModule, ui->ContactStatus);
						DrawIconEx(dis->hDC, x_offset, dis->rcItem.top+offset-3,hIcon,16,16,0,NULL, DI_NORMAL);
						x_offset += 18;
					}
					DrawIconEx(dis->hDC,x_offset, dis->rcItem.top + offset,hIcon,10,10,0,NULL, DI_NORMAL);
					x_offset += 12;
					if (g_Settings.ShowContactStatus && !g_Settings.ContactStatusFirst && ui->ContactStatus) {
						HICON hIcon = LoadSkinnedProtoIcon(si->pszModule, ui->ContactStatus);
						DrawIconEx(dis->hDC, x_offset, dis->rcItem.top+offset-3,hIcon,16,16,0,NULL, DI_NORMAL);
						x_offset += 18;
					}

					SetTextColor(dis->hDC, ui->iStatusEx == 0?g_Settings.crUserListColor:g_Settings.crUserListHeadingsColor);
					TextOut(dis->hDC, dis->rcItem.left+x_offset, dis->rcItem.top, ui->pszNick, lstrlen(ui->pszNick));
					SelectObject(dis->hDC, hOldFont);

					if (si->pAccPropServicesForNickList) 
					{
						wchar_t *nick = mir_t2u(ui->pszNick);
						si->pAccPropServicesForNickList->lpVtbl->SetHwndPropStr(si->pAccPropServicesForNickList,
							GetDlgItem(hwndDlg,IDC_LIST), OBJID_CLIENT, dis->itemID+1, PROPID_ACC_NAME, nick);
						mir_free(nick);
					}
				}
				return TRUE;
		}	}

	case GC_UPDATENICKLIST:
		{
			int i = SendMessage(GetDlgItem(hwndDlg, IDC_LIST), LB_GETTOPINDEX, 0, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_LIST), LB_SETCOUNT, si->nUsersInNicklist, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_LIST), LB_SETTOPINDEX, i, 0);
			SendMessage(hwndDlg, GC_UPDATETITLE, 0, 0);
		}
		break;

	case GC_EVENT_CONTROL + WM_USER+500:
		{
			switch(wParam) {
			case SESSION_OFFLINE:
				SendMessage(hwndDlg, GC_UPDATESTATUSBAR, 0, 0);
				SendMessage(si->hWnd, GC_UPDATENICKLIST, (WPARAM)0, (LPARAM)0);
				return TRUE;

			case SESSION_ONLINE:
				SendMessage(hwndDlg, GC_UPDATESTATUSBAR, 0, 0);
				return TRUE;

			case WINDOW_HIDDEN:
				SendMessage(hwndDlg, GC_CLOSEWINDOW, 0, 0);
				return TRUE;

			case WINDOW_CLEARLOG:
				SetDlgItemText(hwndDlg, IDC_LOG, _T(""));
				return TRUE;

			case SESSION_TERMINATE:
				SendMessage(hwndDlg,GC_SAVEWNDPOS,0,0);
				if (DBGetContactSettingByte(NULL, "Chat", "SavePosition", 0)) {
					DBWriteContactSettingDword(si->hContact, "Chat", "roomx", si->iX);
					DBWriteContactSettingDword(si->hContact, "Chat", "roomy", si->iY);
					DBWriteContactSettingDword(si->hContact, "Chat", "roomwidth" , si->iWidth);
					DBWriteContactSettingDword(si->hContact, "Chat", "roomheight", si->iHeight);
				}
				if (CallService(MS_CLIST_GETEVENT, (WPARAM)si->hContact, (LPARAM)0))
					CallService(MS_CLIST_REMOVEEVENT, (WPARAM)si->hContact, (LPARAM)"chaticon");
				si->wState &= ~STATE_TALK;
				DBWriteContactSettingWord(si->hContact, si->pszModule ,"ApparentMode",(LPARAM) 0);
				SendMessage(hwndDlg, GC_CLOSEWINDOW, 0, 0);
				return TRUE;

			case WINDOW_MINIMIZE:
				ShowWindow(hwndDlg, SW_MINIMIZE);
				goto LABEL_SHOWWINDOW;

			case WINDOW_MAXIMIZE:
				ShowWindow(hwndDlg, SW_MAXIMIZE);
				goto LABEL_SHOWWINDOW;

			case SESSION_INITDONE:
				if (DBGetContactSettingByte(NULL, "Chat", "PopupOnJoin", 0)!=0)
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
		}	}
		break;

	case GC_SPLITTERMOVED:
		{	POINT pt;
			RECT rc;
			RECT rcLog;
			BOOL bFormat = IsWindowVisible(GetDlgItem(hwndDlg,IDC_SMILEY));

			static int x = 0;

			GetWindowRect(GetDlgItem(hwndDlg,IDC_LOG),&rcLog);
			if ((HWND)lParam==GetDlgItem(hwndDlg,IDC_SPLITTERX)) {
				int oldSplitterX;
				GetClientRect(hwndDlg,&rc);
				pt.x=wParam; pt.y=0;
				ScreenToClient(hwndDlg,&pt);

				oldSplitterX=si->iSplitterX;
				si->iSplitterX=rc.right-pt.x+1;
				if (si->iSplitterX < 35)
					si->iSplitterX=35;
				if (si->iSplitterX > rc.right-rc.left-35)
					si->iSplitterX = rc.right-rc.left-35;
				g_Settings.iSplitterX = si->iSplitterX;
			}
			else if ((HWND)lParam==GetDlgItem(hwndDlg,IDC_SPLITTERY)) {
				int oldSplitterY;
				GetClientRect(hwndDlg,&rc);
				pt.x=0; pt.y=wParam;
				ScreenToClient(hwndDlg,&pt);

				oldSplitterY=si->iSplitterY;
				si->iSplitterY=bFormat?rc.bottom-pt.y+1:rc.bottom-pt.y+20;
				if (si->iSplitterY<63)
					si->iSplitterY=63;
				if (si->iSplitterY>rc.bottom-rc.top-40)
					si->iSplitterY = rc.bottom-rc.top-40;
				g_Settings.iSplitterY = si->iSplitterY;
			}
			if (x==2) {
				PostMessage(hwndDlg,WM_SIZE,0,0);
				x = 0;
			}
			else x++;
		}
		break;

	case GC_FIREHOOK:
		if (lParam) {
			GCHOOK* gch = (GCHOOK *) lParam;
			NotifyEventHooks(hSendEvent,0,(WPARAM)gch);
			if ( gch->pDest ) {
				mir_free( gch->pDest->pszID );
				mir_free( gch->pDest->pszModule );
				mir_free( gch->pDest );
			}
			mir_free( gch->ptszText );
			mir_free( gch->ptszUID );
			mir_free( gch );
		}
		break;

	case GC_CHANGEFILTERFLAG:
		si->iLogFilterFlags = lParam;
		break;

	case GC_SHOWFILTERMENU:
		{
			RECT rc;
			HWND hwnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_FILTER), hwndDlg, FilterWndProc, (LPARAM)si);
			TranslateDialogDefault(hwnd);
			GetWindowRect(GetDlgItem(hwndDlg, IDC_FILTER), &rc);
			SetWindowPos(hwnd, HWND_TOP, rc.left-85, (IsWindowVisible(GetDlgItem(hwndDlg, IDC_FILTER))||IsWindowVisible(GetDlgItem(hwndDlg, IDC_BOLD)))?rc.top-206:rc.top-186, 0, 0, SWP_NOSIZE|SWP_SHOWWINDOW);
		}
		break;

	case GC_SHOWCOLORCHOOSER:
		{
			HWND ColorWindow;
			RECT rc;
			BOOL bFG = lParam == IDC_COLOR?TRUE:FALSE;
			COLORCHOOSER * pCC = mir_alloc(sizeof(COLORCHOOSER));

			GetWindowRect(GetDlgItem(hwndDlg, bFG?IDC_COLOR:IDC_BKGCOLOR), &rc);
			pCC->hWndTarget = GetDlgItem(hwndDlg, IDC_MESSAGE);
			pCC->pModule = MM_FindModule(si->pszModule);
			pCC->xPosition = rc.left+3;
			pCC->yPosition = IsWindowVisible(GetDlgItem(hwndDlg, IDC_COLOR))?rc.top-1:rc.top+20;
			pCC->bForeground = bFG;
			pCC->si = si;

			ColorWindow= CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_COLORCHOOSER), hwndDlg, DlgProcColorToolWindow, (LPARAM) pCC);
		}
		break;

	case GC_SCROLLTOBOTTOM:
		{
			SCROLLINFO si = { 0 };
			if ((GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_LOG), GWL_STYLE) & WS_VSCROLL) != 0) {
				CHARRANGE sel;
				si.cbSize = sizeof(si);
				si.fMask = SIF_PAGE | SIF_RANGE;
				GetScrollInfo(GetDlgItem(hwndDlg, IDC_LOG), SB_VERT, &si);
				si.fMask = SIF_POS;
				si.nPos = si.nMax - si.nPage + 1;
				SetScrollInfo(GetDlgItem(hwndDlg, IDC_LOG), SB_VERT, &si, TRUE);
				sel.cpMin = sel.cpMax = GetRichTextLength(GetDlgItem(hwndDlg, IDC_LOG));
				SendMessage(GetDlgItem(hwndDlg, IDC_LOG), EM_EXSETSEL, 0, (LPARAM) & sel);
				PostMessage(GetDlgItem(hwndDlg, IDC_LOG), WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
		}	}
		break;

	case WM_TIMER:
		if (wParam == TIMERID_FLASHWND)
			FlashWindow(hwndDlg, TRUE);
		break;

	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_ACTIVE)
			break;

		//fall through
	case WM_MOUSEACTIVATE:
		{
			WINDOWPLACEMENT wp = { 0 };

			wp.length = sizeof(wp);
			GetWindowPlacement(hwndDlg, &wp);
			g_Settings.iX = wp.rcNormalPosition.left;
			g_Settings.iY = wp.rcNormalPosition.top;
			g_Settings.iWidth = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
			g_Settings.iHeight = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;

			if (g_Settings.TabsEnable) {
				int i = TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_TAB));
				if (i != -1) {
					SESSION_INFO* s;
					TCITEM tci;

					tci.mask = TCIF_PARAM;
					TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_TAB), i, &tci);
					s = (SESSION_INFO*) tci.lParam;
					if (s) {
						s->wState &= ~GC_EVENT_HIGHLIGHT;
						s->wState &= ~STATE_TALK;
						SendMessage(hwndDlg, GC_FIXTABICONS, 0, (LPARAM)s);
			}	}	}

			if (uMsg != WM_ACTIVATE)
				SetFocus(GetDlgItem(hwndDlg,IDC_MESSAGE));

			SetActiveSession(si->ptszID, si->pszModule);

			if (KillTimer(hwndDlg, TIMERID_FLASHWND))
				FlashWindow(hwndDlg, FALSE);
			if (DBGetContactSettingWord(si->hContact, si->pszModule ,"ApparentMode", 0) != 0)
				DBWriteContactSettingWord(si->hContact, si->pszModule ,"ApparentMode",(LPARAM) 0);
			if (CallService(MS_CLIST_GETEVENT, (WPARAM)si->hContact, (LPARAM)0))
				CallService(MS_CLIST_REMOVEEVENT, (WPARAM)si->hContact, (LPARAM)"chaticon");
		}
		break;

	case WM_NOTIFY:
		{
			LPNMHDR pNmhdr;

			pNmhdr = (LPNMHDR)lParam;
			switch (pNmhdr->code) {
			case NM_RCLICK:
				if (pNmhdr->idFrom == IDC_TAB  ) {
					int i = TabCtrl_GetCurSel(pNmhdr->hwndFrom);

					if (i != -1) {
						SESSION_INFO* s;
						HMENU hSubMenu;
						TCHITTESTINFO tci = {0};
						TCITEM id = {0};
						int i = 0;
						id.mask = TCIF_PARAM;

						tci.pt.x=(short)LOWORD(GetMessagePos());
						tci.pt.y=(short)HIWORD(GetMessagePos());
						tci.flags = TCHT_ONITEM;

						ScreenToClient(GetDlgItem(hwndDlg, IDC_TAB), &tci.pt);
						i = TabCtrl_HitTest(pNmhdr->hwndFrom, &tci);
						if (i != -1) {
							TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_TAB), i, &id);
							s = (SESSION_INFO*)id.lParam;

							ClientToScreen(GetDlgItem(hwndDlg, IDC_TAB), &tci.pt);
							hSubMenu = GetSubMenu(g_hMenu, 5);
							CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM) hSubMenu, 0);
							if (s) {
								WORD w = DBGetContactSettingWord(s->hContact, s->pszModule, "TabPosition", 0);
								if ( w == 0)
									CheckMenuItem(hSubMenu, ID_LOCKPOSITION, MF_BYCOMMAND|MF_UNCHECKED);
								else
									CheckMenuItem(hSubMenu, ID_LOCKPOSITION, MF_BYCOMMAND|MF_CHECKED);
							}
							else CheckMenuItem(hSubMenu, ID_LOCKPOSITION, MF_BYCOMMAND|MF_UNCHECKED);

							switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, tci.pt.x, tci.pt.y, 0, hwndDlg, NULL)) {
							case ID_CLOSE:
								if (TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_TAB)) == i)
									PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_CLOSE, BN_CLICKED), 0);
								else
									TabCtrl_DeleteItem(GetDlgItem(hwndDlg, IDC_TAB), i);
								break;

							case ID_CLOSEOTHER:
								{
									int tabId = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_TAB)) - 1;
									if (tabId > 0) {
										if (TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_TAB)) != i)
											if (s)
												ShowRoom(s, WINDOW_VISIBLE, TRUE);

										for(tabId; tabId >= 0; tabId --) {
											if (tabId == i)
												continue;

											TabCtrl_DeleteItem(GetDlgItem(hwndDlg, IDC_TAB), tabId);
								}	}	}
								break;

							case ID_LOCKPOSITION:
								TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_TAB), i, &id);
								if (!(GetMenuState(hSubMenu, ID_LOCKPOSITION, MF_BYCOMMAND)&MF_CHECKED)) {
									if (s->hContact)
										DBWriteContactSettingWord(s->hContact, s->pszModule, "TabPosition", (WORD)(i + 1));
								}
								else DBDeleteContactSetting(s->hContact, s->pszModule, "TabPosition");
								break;
				}	}	}	}
				break;

			case EN_MSGFILTER:
				if (pNmhdr->idFrom == IDC_LOG && ((MSGFILTER *) lParam)->msg == WM_RBUTTONUP) {
					CHARRANGE sel, all = { 0, -1 };
					POINT pt;
					UINT uID = 0;
					HMENU hMenu = 0;
					TCHAR pszWord[4096];

					pt.x = (short) LOWORD(((ENLINK *) lParam)->lParam);
					pt.y = (short) HIWORD(((ENLINK *) lParam)->lParam);
					ClientToScreen(pNmhdr->hwndFrom, &pt);

					{ // fixing stuff for searches
						long iCharIndex, iLineIndex, iChars, start, end, iRes;
						POINTL ptl;

						pszWord[0] = '\0';
						ptl.x = (LONG)pt.x;
						ptl.y = (LONG)pt.y;
						ScreenToClient(GetDlgItem(hwndDlg, IDC_LOG), (LPPOINT)&ptl);
						iCharIndex = SendMessage(GetDlgItem(hwndDlg, IDC_LOG), EM_CHARFROMPOS, 0, (LPARAM)&ptl);
						if (iCharIndex < 0)
							break;
						iLineIndex = SendMessage(GetDlgItem(hwndDlg, IDC_LOG), EM_EXLINEFROMCHAR, 0, (LPARAM)iCharIndex);
						iChars = SendMessage(GetDlgItem(hwndDlg, IDC_LOG), EM_LINEINDEX, (WPARAM)iLineIndex, 0 );
						start = SendMessage(GetDlgItem(hwndDlg, IDC_LOG), EM_FINDWORDBREAK, WB_LEFT, iCharIndex);//-iChars;
						end = SendMessage(GetDlgItem(hwndDlg, IDC_LOG), EM_FINDWORDBREAK, WB_RIGHT, iCharIndex);//-iChars;

						if (end - start > 0) {
							TEXTRANGE tr;
							CHARRANGE cr;
							static TCHAR szTrimString[] = _T(":;,.!?\'\"><()[]- \r\n");
							ZeroMemory(&tr, sizeof(TEXTRANGE));

							cr.cpMin = start;
							cr.cpMax = end;
							tr.chrg = cr;
							tr.lpstrText = pszWord;
							iRes = SendMessage( GetDlgItem(hwndDlg, IDC_LOG), EM_GETTEXTRANGE, 0, (LPARAM)&tr);

							if (iRes > 0) {
								int iLen = lstrlen(pszWord)-1;
								while(iLen >= 0 && _tcschr(szTrimString, pszWord[iLen])) {
									pszWord[iLen] = _T('\0');
									iLen--;
					}	}	}	}

					uID = CreateGCMenu(hwndDlg, &hMenu, 1, pt, si, NULL, pszWord);
					switch (uID) {
					case 0:
						PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0 );
						break;

					case ID_COPYALL:
						SendMessage(pNmhdr->hwndFrom, EM_EXGETSEL, 0, (LPARAM) & sel);
						SendMessage(pNmhdr->hwndFrom, EM_EXSETSEL, 0, (LPARAM) & all);
						SendMessage(pNmhdr->hwndFrom, WM_COPY, 0, 0);
						SendMessage(pNmhdr->hwndFrom, EM_EXSETSEL, 0, (LPARAM) & sel);
						PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0 );
						break;

					case ID_CLEARLOG:
						{
							SESSION_INFO* s = SM_FindSession(si->ptszID, si->pszModule);
							if (s)
							{
								SetDlgItemText(hwndDlg, IDC_LOG, _T(""));
								LM_RemoveAll(&s->pLog, &s->pLogEnd);
								s->iEventCount = 0;
								s->LastTime = 0;
								si->iEventCount = 0;
								si->LastTime = 0;
								si->pLog = s->pLog;
								si->pLogEnd = s->pLogEnd;
								PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0 );
						}	}
						break;

					case ID_SEARCH_GOOGLE:
						{
							char szURL[4096];
							if (pszWord[0]) {
								mir_snprintf( szURL, sizeof( szURL ), "http://www.google.com/search?q=" TCHAR_STR_PARAM, pszWord );
								CallService(MS_UTILS_OPENURL, 1, (LPARAM) szURL);
							}
							PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0 );
						}
						break;

					case ID_SEARCH_WIKIPEDIA:
						{
							char szURL[4096];
							if (pszWord[0]) {
								mir_snprintf( szURL, sizeof( szURL ), "http://en.wikipedia.org/wiki/" TCHAR_STR_PARAM, pszWord );
								CallService(MS_UTILS_OPENURL, 1, (LPARAM) szURL);
							}
							PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0 );
						}
						break;

					default:
						PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0 );
						DoEventHookAsync(hwndDlg, si->ptszID, si->pszModule, GC_USER_LOGMENU, NULL, NULL, (LPARAM)uID);
						break;
					}
					DestroyGCMenu(&hMenu, 5);
				}
				break;

			case EN_LINK:
				if (pNmhdr->idFrom == IDC_LOG) {
					switch (((ENLINK *) lParam)->msg) {
					case WM_RBUTTONDOWN:
					case WM_LBUTTONUP:
					case WM_LBUTTONDBLCLK:
						{
							TEXTRANGE tr;
							CHARRANGE sel;
							char* pszUrl;

							SendMessage(pNmhdr->hwndFrom, EM_EXGETSEL, 0, (LPARAM) & sel);
							if (sel.cpMin != sel.cpMax)
								break;
							tr.chrg = ((ENLINK *) lParam)->chrg;
							tr.lpstrText = mir_alloc(sizeof(TCHAR)*(tr.chrg.cpMax - tr.chrg.cpMin + 1));
							SendMessage(pNmhdr->hwndFrom, EM_GETTEXTRANGE, 0, (LPARAM) & tr);
							pszUrl = mir_t2a( tr.lpstrText );

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

								case ID_COPY:
									{
										HGLOBAL hData;
										if (!OpenClipboard(hwndDlg))
											break;
										EmptyClipboard();
										hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(TCHAR)*(lstrlen(tr.lpstrText) + 1));
										lstrcpy(( TCHAR* )GlobalLock(hData), tr.lpstrText);
										GlobalUnlock(hData);
										#if defined( _UNICODE )
											SetClipboardData(CF_UNICODETEXT, hData);
										#else
											SetClipboardData(CF_TEXT, hData);
										#endif
										CloseClipboard();
										SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
										break;
								}	}
								mir_free(tr.lpstrText);
								mir_free(pszUrl);
								return TRUE;
							}

							CallService(MS_UTILS_OPENURL, 1, (LPARAM) pszUrl);
							SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
							mir_free(tr.lpstrText);
							mir_free(pszUrl);
							break;
				}	}	}
				break;

			case TTN_NEEDTEXT:
				if (pNmhdr->idFrom == (UINT_PTR)GetDlgItem(hwndDlg,IDC_LIST))
				{
					LPNMTTDISPINFO lpttd = (LPNMTTDISPINFO)lParam;
					POINT p;
					int item;
					USERINFO * ui;
					SESSION_INFO* parentdat =(SESSION_INFO*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);

					GetCursorPos( &p );
					ScreenToClient(GetDlgItem(hwndDlg, IDC_LIST), &p);
					item = LOWORD(SendMessage(GetDlgItem(hwndDlg, IDC_LIST), LB_ITEMFROMPOINT, 0, MAKELPARAM(p.x, p.y)));
					ui = SM_GetUserFromIndex(parentdat->ptszID, parentdat->pszModule, item);
					if ( ui != NULL ) {
						static TCHAR ptszBuf[ 1024 ];
						mir_sntprintf( ptszBuf, SIZEOF(ptszBuf), _T("%s: %s\r\n%s: %s\r\n%s: %s"),
							TranslateT( "Nick name" ), ui->pszNick,
							TranslateT( "Unique id" ), ui->pszUID,
							TranslateT( "Status" ), TM_WordToString( parentdat->pStatuses, ui->Status ));
						lpttd->lpszText = ptszBuf;
				}	}
				break;
		}	}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_LIST:
			if (HIWORD(wParam) == LBN_DBLCLK) {
				TVHITTESTINFO hti;
				int item;
				USERINFO * ui;

				hti.pt.x=(short)LOWORD(GetMessagePos());
				hti.pt.y=(short)HIWORD(GetMessagePos());
				ScreenToClient(GetDlgItem(hwndDlg, IDC_LIST),&hti.pt);

				item = LOWORD(SendMessage(GetDlgItem(hwndDlg, IDC_LIST), LB_ITEMFROMPOINT, 0, MAKELPARAM(hti.pt.x, hti.pt.y)));
				ui = SM_GetUserFromIndex(si->ptszID, si->pszModule, item);
				if (ui) {
					if (GetKeyState(VK_SHIFT) & 0x8000) {
						LRESULT lResult = (LRESULT)SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), EM_GETSEL, (WPARAM)NULL, (LPARAM)NULL);
						int start = LOWORD(lResult);
						TCHAR* pszName = (TCHAR*)alloca(sizeof(TCHAR)*(lstrlen(ui->pszUID) + 3));
						if (start == 0)
							mir_sntprintf(pszName, lstrlen(ui->pszUID)+3, _T("%s: "), ui->pszUID);
						else
							mir_sntprintf(pszName, lstrlen(ui->pszUID)+2, _T("%s "), ui->pszUID);

						SendMessage( GetDlgItem(hwndDlg, IDC_MESSAGE), EM_REPLACESEL, FALSE, (LPARAM) pszName );
						PostMessage( hwndDlg, WM_MOUSEACTIVATE, 0, 0 );
					}
					else DoEventHookAsync(hwndDlg, si->ptszID, si->pszModule, GC_USER_PRIVMESS, ui->pszUID, NULL, (LPARAM)NULL);
				}

				return TRUE;
			}

			if ( HIWORD(wParam) == LBN_KILLFOCUS )
				RedrawWindow(GetDlgItem(hwndDlg, IDC_LIST), NULL, NULL, RDW_INVALIDATE);
			break;

		case IDOK:
			{
				char*  pszRtf;
				TCHAR* ptszText, *p1;
				if (!IsWindowEnabled(GetDlgItem(hwndDlg,IDOK)))
					break;

				pszRtf = Message_GetFromStream(hwndDlg, si);
				SM_AddCommand(si->ptszID, si->pszModule, pszRtf);
				ptszText = DoRtfToTags(pszRtf, si);
				p1 = _tcschr(ptszText, '\0');

				//remove trailing linebreaks
				while ( p1 > ptszText && (*p1 == '\0' || *p1 == '\r' || *p1 == '\n')) {
					*p1 = '\0';
					p1--;
				}

				if ( MM_FindModule(si->pszModule)->bAckMsg ) {
					EnableWindow(GetDlgItem(hwndDlg,IDC_MESSAGE),FALSE);
					SendDlgItemMessage(hwndDlg,IDC_MESSAGE,EM_SETREADONLY,TRUE,0);
				}
				else SendDlgItemMessage(hwndDlg,IDC_MESSAGE,WM_SETTEXT,0,(LPARAM)_T(""));

				EnableWindow(GetDlgItem(hwndDlg,IDOK),FALSE);

				DoEventHookAsync(hwndDlg, si->ptszID, si->pszModule, GC_USER_MESSAGE, NULL, ptszText, (LPARAM)NULL);
				mir_free(pszRtf);
				#if defined( _UNICODE )
					mir_free(ptszText);
				#endif
				SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
			}
			break;

		case IDC_SHOWNICKLIST:
			if (!IsWindowEnabled(GetDlgItem(hwndDlg,IDC_SHOWNICKLIST)))
				break;
			if (si->iType == GCW_SERVER)
				break;

			si->bNicklistEnabled = !si->bNicklistEnabled;

			SendDlgItemMessage(hwndDlg,IDC_SHOWNICKLIST,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIconEx( si->bNicklistEnabled ? "nicklist" : "nicklist2", FALSE ));
			SendMessage(hwndDlg, GC_SCROLLTOBOTTOM, 0, 0);
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
			break;

		case IDC_MESSAGE:
			EnableWindow(GetDlgItem(hwndDlg, IDOK), GetRichTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE)) != 0);
			break;

		case IDC_SMILEY:
			{
				SMADD_SHOWSEL3 smaddInfo;
				RECT rc;

				GetWindowRect(GetDlgItem(hwndDlg, IDC_SMILEY), &rc);

				smaddInfo.cbSize = sizeof(SMADD_SHOWSEL3);
				smaddInfo.hwndTarget = GetDlgItem(hwndDlg, IDC_MESSAGE);
				smaddInfo.targetMessage = EM_REPLACESEL;
				smaddInfo.targetWParam = TRUE;
				smaddInfo.Protocolname = si->pszModule;
				smaddInfo.Direction = 3;
				smaddInfo.xPosition = rc.left+3;
				smaddInfo.yPosition = rc.top-1;
				smaddInfo.hContact = si->hContact;
				smaddInfo.hwndParent = hwndDlg;

				if (SmileyAddInstalled)
					CallService(MS_SMILEYADD_SHOWSELECTION, 0, (LPARAM) &smaddInfo);
			}
			break;

		case IDC_HISTORY:
			{
				TCHAR szFile[MAX_PATH];
				TCHAR szName[MAX_PATH];
				TCHAR szFolder[MAX_PATH];
				MODULEINFO * pInfo = MM_FindModule(si->pszModule);

				if (!IsWindowEnabled(GetDlgItem(hwndDlg,IDC_HISTORY)))
					break;

				if ( pInfo ) {
					TCHAR *szModName = NULL;
					mir_sntprintf(szName, MAX_PATH, _T("%s"), pInfo->ptszModDispName ? pInfo->ptszModDispName : (szModName = mir_a2t(si->pszModule)));
					mir_free(szModName);
					ValidateFilename(szName);
					mir_sntprintf(szFolder, MAX_PATH, _T("%s\\%s"), g_Settings.pszLogDir, szName);
                    mir_sntprintf(szName, MAX_PATH, _T("%s.log"), si->ptszID);
					ValidateFilename(szName);
					mir_sntprintf(szFile, MAX_PATH, _T("%s\\%s"), szFolder, szName);
					ShellExecute(hwndDlg, _T("open"), szFile, NULL, NULL, SW_SHOW);
			}	}
			break;

		case IDC_CLOSE:
			SendMessage(hwndDlg, GC_REMOVETAB, 0, 0);
			break;

		case IDC_CHANMGR:
			if (!IsWindowEnabled(GetDlgItem(hwndDlg,IDC_CHANMGR)))
				break;
			DoEventHookAsync(hwndDlg, si->ptszID, si->pszModule, GC_USER_CHANMGR, NULL, NULL, (LPARAM)NULL);
			break;

		case IDC_FILTER:
			if (!IsWindowEnabled(GetDlgItem(hwndDlg,IDC_FILTER)))
				break;

			si->bFilterEnabled = !si->bFilterEnabled;
			SendDlgItemMessage(hwndDlg,IDC_FILTER,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIconEx( si->bFilterEnabled ? "filter" : "filter2", FALSE ));
			if (si->bFilterEnabled && DBGetContactSettingByte(NULL, "Chat", "RightClickFilter", 0) == 0) {
				SendMessage(hwndDlg, GC_SHOWFILTERMENU, 0, 0);
				break;
			}
			SendMessage(hwndDlg, GC_REDRAWLOG, 0, 0);
			break;

		case IDC_BKGCOLOR:
			{
				CHARFORMAT2 cf;

				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwEffects = 0;

				if (!IsWindowEnabled(GetDlgItem(hwndDlg,IDC_BKGCOLOR)))
					break;

				if (IsDlgButtonChecked(hwndDlg, IDC_BKGCOLOR )) {
					if (DBGetContactSettingByte(NULL, "Chat", "RightClickFilter", 0) == 0)
						SendMessage(hwndDlg, GC_SHOWCOLORCHOOSER, 0, (LPARAM)IDC_BKGCOLOR);
					else if (si->bBGSet) {
						cf.dwMask = CFM_BACKCOLOR;
						cf.crBackColor = MM_FindModule(si->pszModule)->crColors[si->iBG];
						SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				}	}
				else {
					cf.dwMask = CFM_BACKCOLOR;
					cf.crBackColor = (COLORREF)DBGetContactSettingDword(NULL, "Chat", "ColorMessageBG", GetSysColor(COLOR_WINDOW));
					SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			}	}
			break;

		case IDC_COLOR:
			{
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwEffects = 0;

				if (!IsWindowEnabled(GetDlgItem(hwndDlg,IDC_COLOR)))
					break;

				if (IsDlgButtonChecked(hwndDlg, IDC_COLOR )) {
					if (DBGetContactSettingByte(NULL, "Chat", "RightClickFilter", 0) == 0)
						SendMessage(hwndDlg, GC_SHOWCOLORCHOOSER, 0, (LPARAM)IDC_COLOR);
					else if (si->bFGSet) {
						cf.dwMask = CFM_COLOR;
						cf.crTextColor = MM_FindModule(si->pszModule)->crColors[si->iFG];
						SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				}	}
				else {
					COLORREF cr;

					LoadMsgDlgFont(17, NULL, &cr);
					cf.dwMask = CFM_COLOR;
					cf.crTextColor = cr;
					SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			}	}
			break;

		case IDC_BOLD:
		case IDC_ITALICS:
		case IDC_UNDERLINE:
			{
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwMask = CFM_BOLD|CFM_ITALIC|CFM_UNDERLINE;
				cf.dwEffects = 0;

				if (LOWORD(wParam) == IDC_BOLD && !IsWindowEnabled(GetDlgItem(hwndDlg,IDC_BOLD)))
					break;
				if (LOWORD(wParam) == IDC_ITALICS && !IsWindowEnabled(GetDlgItem(hwndDlg,IDC_ITALICS)))
					break;
				if (LOWORD(wParam) == IDC_UNDERLINE && !IsWindowEnabled(GetDlgItem(hwndDlg,IDC_UNDERLINE)))
					break;
				if (IsDlgButtonChecked(hwndDlg, IDC_BOLD))
					cf.dwEffects |= CFE_BOLD;
				if (IsDlgButtonChecked(hwndDlg, IDC_ITALICS))
					cf.dwEffects |= CFE_ITALIC;
				if (IsDlgButtonChecked(hwndDlg, IDC_UNDERLINE))
					cf.dwEffects |= CFE_UNDERLINE;

				SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		}	}
		break;

	case WM_KEYDOWN:
		SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
		break;

	case WM_MOVE:
		SendMessage(hwndDlg,GC_SAVEWNDPOS,0,1);
		break;

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO* mmi = (MINMAXINFO*)lParam;
			mmi->ptMinTrackSize.x = si->iSplitterX + 43;
			if (mmi->ptMinTrackSize.x < 350)
				mmi->ptMinTrackSize.x = 350;

			mmi->ptMinTrackSize.y = si->iSplitterY + 80;
		}
		break;

	case WM_LBUTTONDBLCLK:
		if (LOWORD(lParam) < 30)
			PostMessage(hwndDlg, GC_SCROLLTOBOTTOM, 0, 0);
		break;

	case WM_CLOSE:
		if (g_Settings.TabsEnable && g_Settings.TabRestore && lParam != 1) {
			SESSION_INFO* s;
			TCITEM id = {0};
			int j = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_TAB)) - 1;
			id.mask = TCIF_PARAM;
			for(j; j >= 0; j--) {
				TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_TAB), j, &id);
				s = (SESSION_INFO*)id.lParam;
				if (s)
					TabM_AddTab(s->ptszID, s->pszModule);
		}	}

		SendMessage(hwndDlg, GC_CLOSEWINDOW, 0, 0);
		break;

	case GC_CLOSEWINDOW:
		if (g_Settings.TabsEnable)
			SM_SetTabbedWindowHwnd(0, 0);
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		SendMessage(hwndDlg,GC_SAVEWNDPOS,0,0);

		si->hWnd = NULL;
		si->wState &= ~STATE_TALK;
		DestroyWindow(si->hwndStatus);
		si->hwndStatus = NULL;

		if (si->hwndTooltip != NULL) {
			HWND hNickList = GetDlgItem(hwndDlg,IDC_LIST);
			TOOLINFO ti = { 0 };
			ti.cbSize = sizeof(TOOLINFO);
			ti.uId = (UINT_PTR)hNickList;
			ti.hwnd = hNickList;
			SendMessage( si->hwndTooltip, TTM_DELTOOL, 0, (LPARAM)(LPTOOLINFO)&ti );
		}
		DestroyWindow( si->hwndTooltip );
		si->hwndTooltip = NULL;
		if (si->pAccPropServicesForNickList) si->pAccPropServicesForNickList->lpVtbl->Release(si->pAccPropServicesForNickList);
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,0);
		SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_SPLITTERX),GWLP_WNDPROC,(LONG_PTR)OldSplitterProc);
		SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_SPLITTERY),GWLP_WNDPROC,(LONG_PTR)OldSplitterProc);
		SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_LIST),GWLP_WNDPROC,(LONG_PTR)OldNicklistProc);
		SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_TAB),GWLP_WNDPROC,(LONG_PTR)OldTabProc);
		SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_UNSUBCLASSED, 0, 0);
		SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_MESSAGE),GWLP_WNDPROC,(LONG_PTR)OldMessageProc);
		SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_LOG),GWLP_WNDPROC,(LONG_PTR)OldLogProc);
		SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_FILTER),GWLP_WNDPROC,(LONG_PTR)OldFilterButtonProc);
		SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_COLOR),GWLP_WNDPROC,(LONG_PTR)OldFilterButtonProc);
		SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_BKGCOLOR),GWLP_WNDPROC,(LONG_PTR)OldFilterButtonProc);
		break;
	}
	return(FALSE);
}
