/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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
#include "profilemanager.h"
#include "clc.h"

#define TM_AUTOALPHA  1
#define MENU_MIRANDAMENU         0xFFFF1234

extern HANDLE hEventExtraClick;

static HMODULE hUserDll;
static HANDLE hContactDraggingEvent, hContactDroppedEvent, hContactDragStopEvent;
static int transparentFocus = 1;
UINT uMsgProcessProfile;

#define M_RESTORESTATUS  (WM_USER+7)

typedef struct
{
	int showsbar;
	int showgrip;
	int transparent;
	int alpha;
}
CluiOpts;

static CluiOpts cluiopt = { 0 };

void fnLoadCluiGlobalOpts()
{
	cluiopt.showsbar = db_get_b(0, "CLUI", "ShowSBar", 1);
	cluiopt.showgrip = db_get_b(0, "CLUI", "ShowGrip", 1);
	cluiopt.transparent = db_get_b(0, "CList", "Transparent", SETTING_TRANSPARENT_DEFAULT);
	cluiopt.alpha = db_get_b(0, "CList", "Alpha", SETTING_ALPHA_DEFAULT);
}

// Disconnect all protocols.
// Happens on shutdown and standby.
static void DisconnectAll()
{
	for (auto &it : g_arAccounts)
		CallProtoServiceInt(0, it->szModuleName, PS_SETSTATUS, ID_STATUS_OFFLINE, 0);
}

static int CluiIconsChanged(WPARAM, LPARAM)
{
	DrawMenuBar(g_clistApi.hwndContactList);
	return 0;
}

static int CluiLangpackChanged(WPARAM, LPARAM)
{
	if (g_clistApi.hMenuMain) {
		RemoveMenu(g_clistApi.hMenuMain, 0, MF_BYPOSITION);
		RemoveMenu(g_clistApi.hMenuMain, 0, MF_BYPOSITION);
		DestroyMenu(g_clistApi.hMenuMain);
	}

	g_clistApi.hMenuMain = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_CLISTMENU));
	TranslateMenu(g_clistApi.hMenuMain);

	// make menu bar owner-drawn and set it on
	if (g_clistApi.bOwnerDrawMenu) {
		MENUITEMINFO mii;
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_TYPE | MIIM_DATA;
		mii.dwItemData = MENU_MIRANDAMENU;
		mii.fType = MFT_OWNERDRAW;
		SetMenuItemInfo(g_clistApi.hMenuMain, 0, TRUE, &mii);
	}

	if (GetMenu(g_clistApi.hwndContactList))
		SetMenu(g_clistApi.hwndContactList, g_clistApi.hMenuMain);
	return 0;
}

static HGENMENU hRenameMenuItem;

static int MenuItem_PreBuild(WPARAM, LPARAM)
{
	wchar_t cls[128];
	HWND hwndClist = GetFocus();
	GetClassName(hwndClist, cls, _countof(cls));
	hwndClist = (!mir_wstrcmp(CLISTCONTROL_CLASSW, cls)) ? hwndClist : g_clistApi.hwndContactList;
	HANDLE hItem = (HANDLE)SendMessage(hwndClist, CLM_GETSELECTION, 0, 0);
	Menu_ShowItem(hRenameMenuItem, hItem != nullptr);
	return 0;
}

static INT_PTR MenuItem_RenameContact(WPARAM, LPARAM)
{
	wchar_t cls[128];
	HWND hwndClist = GetFocus();
	GetClassName(hwndClist, cls, _countof(cls));
	// worst case scenario, the rename is sent to the main contact list
	hwndClist = (!mir_wstrcmp(CLISTCONTROL_CLASSW, cls)) ? hwndClist : g_clistApi.hwndContactList;
	HANDLE hItem = (HANDLE)SendMessage(hwndClist, CLM_GETSELECTION, 0, 0);
	if (hItem) {
		SetFocus(hwndClist);
		SendMessage(hwndClist, CLM_EDITLABEL, (WPARAM)hItem, 0);
	}
	return 0;
}

static INT_PTR CALLBACK AskForConfirmationDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWnd);
		{
			LOGFONT lf;
			HFONT hFont = (HFONT)SendDlgItemMessage(hWnd, IDYES, WM_GETFONT, 0, 0);
			GetObject(hFont, sizeof(lf), &lf);
			lf.lfWeight = FW_BOLD;
			SendDlgItemMessage(hWnd, IDC_TOPLINE, WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), 0);

			wchar_t szFormat[256], szFinal[256];
			GetDlgItemText(hWnd, IDC_TOPLINE, szFormat, _countof(szFormat));
			mir_snwprintf(szFinal, szFormat, Clist_GetContactDisplayName(lParam));
			SetDlgItemText(hWnd, IDC_TOPLINE, szFinal);
		}
		SetFocus(GetDlgItem(hWnd, IDNO));
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDYES:
			if (IsDlgButtonChecked(hWnd, IDC_HIDE)) {
				EndDialog(hWnd, IDC_HIDE);
				break;
			}
			__fallthrough;

		case IDCANCEL:
		case IDNO:
			EndDialog(hWnd, LOWORD(wParam));
			break;
		}
		break;

	case WM_CLOSE:
		SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDNO, BN_CLICKED), 0);
		break;

	case WM_DESTROY:
		DeleteObject((HFONT)SendDlgItemMessage(hWnd, IDC_TOPLINE, WM_GETFONT, 0, 0));
		break;
	}

	return FALSE;
}

static INT_PTR MenuItem_DeleteContact(WPARAM wParam, LPARAM lParam)
{
	//see notes about deleting contacts on PF1_SERVERCLIST servers in m_protosvc.h
	UINT_PTR action;

	if (Clist::ConfirmDelete && !(GetKeyState(VK_SHIFT) & 0x8000))
		// Ask user for confirmation, and if the contact should be archived (hidden, not deleted)
		action = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_DELETECONTACT), (HWND)lParam, AskForConfirmationDlgProc, wParam);
	else
		action = IDYES;

	switch (action) {
	case IDC_HIDE: // Archive contact
		Contact::Hide(wParam);
		break;

	case IDYES: // Delete contact
		char *szProto = Proto_GetBaseAccountName(wParam);
		if (szProto != nullptr) {
			// Check if protocol uses server side lists
			uint32_t caps = CallProtoServiceInt(0, szProto, PS_GETCAPS, PFLAGNUM_1, 0);
			if (caps & PF1_SERVERCLIST) {
				int status = Proto_GetStatus(szProto);
				if (status == ID_STATUS_OFFLINE || IsStatusConnecting(status)) {
					// Set a flag so we remember to delete the contact when the protocol goes online the next time
					db_set_b(wParam, "CList", "Delete", 1);
					MessageBoxW(nullptr,
								  TranslateT("This contact is on an instant messaging system which stores its contact list on a central server. The contact will be removed from the server and from your contact list when you next connect to that network."),
								  TranslateT("Delete contact"), MB_ICONINFORMATION | MB_OK);
					return 0;
				}
			}
		}

		db_delete_contact(wParam);
		break;
	}

	return 0;
}

static INT_PTR MenuItem_AddContactToList(WPARAM hContact, LPARAM)
{
	Contact::Add(hContact);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// this is the smallest available window procedure

#ifndef CS_DROPSHADOW
#define CS_DROPSHADOW 0x00020000
#endif

LRESULT CALLBACK ContactListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result;
	MSG m;
	m.hwnd = hwnd;
	m.message = msg;
	m.wParam = wParam;
	m.lParam = lParam;
	if (g_clistApi.pfnDocking_ProcessWindowMessage((WPARAM)&m, (LPARAM)&result))
		return result;
	if (g_clistApi.pfnTrayIconProcessMessage((WPARAM)&m, (LPARAM)&result))
		return result;

	return g_clistApi.pfnContactListWndProc(hwnd, msg, wParam, lParam);
}

int LoadCLUIModule(void)
{
	DBVARIANT dbv;
	wchar_t titleText[256];

	uMsgProcessProfile = RegisterWindowMessage(L"Miranda::ProcessProfile");
	g_clistApi.pfnLoadCluiGlobalOpts();

	HookEvent(ME_SKIN_ICONSCHANGED, CluiIconsChanged);

	hContactDraggingEvent = CreateHookableEvent(ME_CLUI_CONTACTDRAGGING);
	hContactDroppedEvent = CreateHookableEvent(ME_CLUI_CONTACTDROPPED);
	hContactDragStopEvent = CreateHookableEvent(ME_CLUI_CONTACTDRAGSTOP);

	WNDCLASSEX wndclass = { 0 };
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_GLOBALCLASS;
	wndclass.lpfnWndProc = g_clistApi.pfnContactListControlWndProc;
	wndclass.cbWndExtra = sizeof(void *);
	wndclass.hInstance = g_clistApi.hInst;
	wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndclass.lpszClassName = CLISTCONTROL_CLASSW;
	RegisterClassEx(&wndclass);

	memset(&wndclass, 0, sizeof(wndclass));
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | ((db_get_b(0, "CList", "WindowShadow", 0) == 1) ? CS_DROPSHADOW : 0);
	wndclass.lpfnWndProc = ContactListWndProc;
	wndclass.hInstance = g_clistApi.hInst;
	wndclass.hIcon = Skin_LoadIcon(SKINICON_OTHER_MIRANDA, true);
	wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
	wndclass.lpszClassName = _A2W(MIRANDACLASS);
	wndclass.hIconSm = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);
	RegisterClassEx(&wndclass);

	if (db_get_ws(0, "CList", "TitleText", &dbv))
		mir_wstrncpy(titleText, _A2W(MIRANDANAME), _countof(titleText));
	else {
		mir_wstrncpy(titleText, dbv.pwszVal, _countof(titleText));
		db_free(&dbv);
	}

	RECT pos;
	pos.left = (int)db_get_dw(0, "CList", "x", 700);
	pos.top = (int)db_get_dw(0, "CList", "y", 221);
	pos.right = pos.left + (int)db_get_dw(0, "CList", "Width", 150);
	pos.bottom = pos.top + (int)db_get_dw(0, "CList", "Height", 310);

	Utils_AssertInsideScreen(&pos);

	g_clistApi.hwndContactList = CreateWindowEx(
		(db_get_b(0, "CList", "ToolWindow", SETTING_TOOLWINDOW_DEFAULT) ? WS_EX_TOOLWINDOW : WS_EX_APPWINDOW),
		_A2W(MIRANDACLASS),
		titleText,
		WS_POPUPWINDOW | WS_THICKFRAME | WS_CLIPCHILDREN |
		(db_get_b(0, "CLUI", "ShowCaption", SETTING_SHOWCAPTION_DEFAULT) ? WS_CAPTION | WS_SYSMENU |
		(db_get_b(0, "CList", "Min2Tray", SETTING_MIN2TRAY_DEFAULT) ? 0 : WS_MINIMIZEBOX) : 0),
		pos.left, pos.top, pos.right - pos.left, pos.bottom - pos.top,
		nullptr, nullptr, g_clistApi.hInst, nullptr);

	if (db_get_b(0, "CList", "OnDesktop", 0)) {
		HWND hProgMan = FindWindow(L"Progman", nullptr);
		if (IsWindow(hProgMan))
			SetParent(g_clistApi.hwndContactList, hProgMan);
	}

	HookEvent(ME_LANGPACK_CHANGED, CluiLangpackChanged);
	CluiLangpackChanged(0, 0);

	SetMenu(g_clistApi.hwndContactList, g_clistApi.hMenuMain);

	g_clistApi.pfnOnCreateClc();

	PostMessage(g_clistApi.hwndContactList, M_RESTORESTATUS, 0, 0);

	int state = db_get_b(0, "CList", "State", SETTING_STATE_NORMAL);
	
	if (!db_get_b(0, "CLUI", "ShowMainMenu", SETTING_SHOWMAINMENU_DEFAULT))
		SetMenu(g_clistApi.hwndContactList, nullptr);

	if (state == SETTING_STATE_NORMAL)
		ShowWindow(g_clistApi.hwndContactList, SW_SHOW);
	else if (state == SETTING_STATE_MINIMIZED)
		ShowWindow(g_clistApi.hwndContactList, SW_SHOWMINIMIZED);
	
	SetWindowPos(g_clistApi.hwndContactList,
					 db_get_b(0, "CList", "OnTop", SETTING_ONTOP_DEFAULT) ? HWND_TOPMOST : HWND_NOTOPMOST,
					 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

	CMenuItem mi(&g_plugin);
	
	SET_UID(mi, 0x66aada45, 0x8bcb, 0x49bb, 0x85, 0xb0, 0xae, 0x1, 0xf8, 0xbe, 0x78, 0x7e);
	mi.position = 2000070000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_DELETE);
	mi.name.a = LPGEN("De&lete");
	mi.pszService = MS_CLIST_DELETECONTACT;
	Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, MenuItem_DeleteContact);

	SET_UID(mi, 0xd5ff6500, 0xd8f4, 0x46c6, 0x87, 0xde, 0x76, 0x26, 0x2a, 0x63, 0xae, 0xed);
	mi.position = 2000050000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_RENAME);
	mi.name.a = LPGEN("&Rename");
	mi.pszService = "CList/RenameContactCommand";
	hRenameMenuItem = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, MenuItem_RenameContact);

	SET_UID(mi, 0x417bc580, 0x7975, 0x43b4, 0x86, 0xf1, 0x5d, 0x18, 0xaf, 0x36, 0xca, 0x37);
	mi.position = -2050000000;
	mi.flags |= CMIF_NOTONLIST;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_ADDCONTACT);
	mi.name.a = LPGEN("&Add permanently to list");
	mi.pszService = "CList/AddToListContactCommand";
	Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, MenuItem_AddContactToList);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, MenuItem_PreBuild);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// default contact list window procedure

MIR_APP_DLL(void) Clist_DrawMenuItem(DRAWITEMSTRUCT *dis, HICON hIcon, HICON eventIcon)
{
	HBRUSH hBr;
	BOOL bfm = FALSE;
	SystemParametersInfo(SPI_GETFLATMENU, 0, &bfm, 0);
	if (bfm) {
		/* flat menus: fill with COLOR_MENUHILIGHT and outline with COLOR_HIGHLIGHT, otherwise use COLOR_MENUBAR */
		if (dis->itemState & ODS_SELECTED || dis->itemState & ODS_HOTLIGHT) {
			/* selected or hot lighted, no difference */
			hBr = GetSysColorBrush(COLOR_MENUHILIGHT);
			FillRect(dis->hDC, &dis->rcItem, hBr);
			DeleteObject(hBr);
			/* draw the frame */
			hBr = GetSysColorBrush(COLOR_HIGHLIGHT);
			FrameRect(dis->hDC, &dis->rcItem, hBr);
			DeleteObject(hBr);
		}
		else {
			/* flush the DC with the menu bar colour (only supported on XP) and then draw the icon */
			hBr = GetSysColorBrush(COLOR_MENUBAR);
			FillRect(dis->hDC, &dis->rcItem, hBr);
			DeleteObject(hBr);
		} //if
		/* draw the icon */
		if (eventIcon != nullptr) {
			DrawState(dis->hDC, nullptr, nullptr, (LPARAM)eventIcon, 0, 2, (dis->rcItem.bottom + dis->rcItem.top - g_IconHeight) / 2 + (dis->itemState & ODS_SELECTED ? 1 : 0), 0, 0, DST_ICON | (dis->itemState & ODS_INACTIVE ? DSS_DISABLED : DSS_NORMAL));
			DrawState(dis->hDC, nullptr, nullptr, (LPARAM)hIcon, 0, 4 + g_IconWidth, (dis->rcItem.bottom + dis->rcItem.top - g_IconHeight) / 2 + (dis->itemState & ODS_SELECTED ? 1 : 0), 0, 0, DST_ICON | (dis->itemState & ODS_INACTIVE ? DSS_DISABLED : DSS_NORMAL));
		}
		else DrawState(dis->hDC, nullptr, nullptr, (LPARAM)hIcon, 0, (dis->rcItem.right + dis->rcItem.left - g_IconWidth) / 2 + (dis->itemState & ODS_SELECTED ? 1 : 0), (dis->rcItem.bottom + dis->rcItem.top - g_IconHeight) / 2 + (dis->itemState & ODS_SELECTED ? 1 : 0), 0, 0, DST_ICON | (dis->itemState & ODS_INACTIVE ? DSS_DISABLED : DSS_NORMAL));
	}
	else {
		/* non-flat menus, flush the DC with a normal menu colour */
		FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_MENU));
		if (dis->itemState & ODS_HOTLIGHT)
			DrawEdge(dis->hDC, &dis->rcItem, BDR_RAISEDINNER, BF_RECT);
		else if (dis->itemState & ODS_SELECTED)
			DrawEdge(dis->hDC, &dis->rcItem, BDR_SUNKENOUTER, BF_RECT);

		if (eventIcon != nullptr) {
			DrawState(dis->hDC, nullptr, nullptr, (LPARAM)eventIcon, 0, 2, (dis->rcItem.bottom + dis->rcItem.top - g_IconHeight) / 2 + (dis->itemState & ODS_SELECTED ? 1 : 0), 0, 0, DST_ICON | (dis->itemState & ODS_INACTIVE ? DSS_DISABLED : DSS_NORMAL));
			DrawState(dis->hDC, nullptr, nullptr, (LPARAM)hIcon, 0, 4 + g_IconWidth, (dis->rcItem.bottom + dis->rcItem.top - g_IconHeight) / 2 + (dis->itemState & ODS_SELECTED ? 1 : 0), 0, 0, DST_ICON | (dis->itemState & ODS_INACTIVE ? DSS_DISABLED : DSS_NORMAL));
		}
		else DrawState(dis->hDC, nullptr, nullptr, (LPARAM)hIcon, 0, (dis->rcItem.right + dis->rcItem.left - g_IconWidth) / 2 + (dis->itemState & ODS_SELECTED ? 1 : 0), (dis->rcItem.bottom + dis->rcItem.top - g_IconHeight) / 2 + (dis->itemState & ODS_SELECTED ? 1 : 0), 0, 0, DST_ICON | (dis->itemState & ODS_INACTIVE ? DSS_DISABLED : DSS_NORMAL));
	}

	DestroyIcon(hIcon);
	return;
}

LRESULT CALLBACK fnContactListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int noRecurse = 0;

	if (msg == uMsgProcessProfile) {
		wchar_t profile[MAX_PATH];
		if (GlobalGetAtomName((ATOM)wParam, profile, _countof(profile))) {
			int rc = mir_wstrcmpi(profile, VARSW(L"%miranda_userdata%\\%miranda_profilename%.dat")) == 0;
			ReplyMessage(rc);
			if (rc) {
				ShowWindow(hwnd, SW_RESTORE);
				ShowWindow(hwnd, SW_SHOW);
				SetForegroundWindow(hwnd);
				SetFocus(hwnd);
			}
		}
		return 0;
	}

	switch (msg) {
	case WM_CREATE:
		//create the status wnd
		{
			int flags = WS_CHILD | CCS_BOTTOM;
			flags |= cluiopt.showsbar ? WS_VISIBLE : 0;
			flags |= cluiopt.showgrip ? SBARS_SIZEGRIP : 0;
			g_clistApi.hwndStatus = CreateWindow(STATUSCLASSNAME, nullptr, flags, 0, 0, 0, 0, hwnd, nullptr, g_clistApi.hInst, nullptr);
		}
		g_clistApi.pfnCluiProtocolStatusChanged(0, nullptr);

		//delay creation of CLC so that it can get the status icons right the first time (needs protocol modules loaded)
		PostMessage(hwnd, M_CREATECLC, 0, 0);

		if (cluiopt.transparent) {
			SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
			SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (uint8_t)cluiopt.alpha, LWA_ALPHA);
		}
		transparentFocus = 1;
		return FALSE;

	case M_CREATECLC:
		g_clistApi.hwndContactTree = CreateWindow(CLISTCONTROL_CLASSW, L"",
			WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | CLS_CONTACTLIST | (Clist::UseGroups ? CLS_USEGROUPS : 0) | (Clist::HideOffline ? CLS_HIDEOFFLINE : 0) | (Clist::HideEmptyGroups ? CLS_HIDEEMPTYGROUPS : 0),
			0, 0, 0, 0, hwnd, nullptr, g_clistApi.hInst, nullptr);
		SendMessage(hwnd, WM_SIZE, 0, 0);
		break;

	case M_RESTORESTATUS:
#ifndef _DEBUG
	{
		int nStatus = db_get_w(0, "CList", "Status", ID_STATUS_OFFLINE);
		if (nStatus != ID_STATUS_OFFLINE)
			Clist_SetStatusMode(nStatus);
	}
#endif
	break;

	// Power management
	case WM_POWERBROADCAST:
		switch ((uint32_t)wParam) {
		case PBT_APMSUSPEND:
			// Computer is suspending, disconnect all protocols
			DisconnectAll();
			break;

		case PBT_APMRESUMEAUTOMATIC:
		case PBT_APMRESUMESUSPEND:
			// Computer is resuming, restore all protocols
			PostMessage(hwnd, M_RESTORESTATUS, 0, 0);
			break;
		}
		break;

	case WM_SYSCOLORCHANGE:
		SendMessage(g_clistApi.hwndContactTree, msg, wParam, lParam);
		SendMessage(g_clistApi.hwndStatus, msg, wParam, lParam);
		// XXX: only works with 4.71 with 95, IE4.
		SendMessage(g_clistApi.hwndStatus, SB_SETBKCOLOR, 0, GetSysColor(COLOR_3DFACE));
		break;

	case WM_SIZE:
		if (IsZoomed(hwnd))
			ShowWindow(hwnd, SW_SHOWNORMAL);
		{
			RECT rect, rcStatus;
			GetClientRect(hwnd, &rect);
			if (cluiopt.showsbar) {
				SetWindowPos(g_clistApi.hwndStatus, nullptr, 0, rect.bottom - 20, rect.right - rect.left, 20, SWP_NOZORDER);
				GetWindowRect(g_clistApi.hwndStatus, &rcStatus);
				g_clistApi.pfnCluiProtocolStatusChanged(0, nullptr);
			}
			else
				rcStatus.top = rcStatus.bottom = 0;
			SetWindowPos(g_clistApi.hwndContactTree, nullptr, 0, 0, rect.right, rect.bottom - (rcStatus.bottom - rcStatus.top), SWP_NOZORDER);
		}
		if (wParam == SIZE_MINIMIZED) {
			if ((GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) || db_get_b(0, "CList", "Min2Tray", SETTING_MIN2TRAY_DEFAULT)) {
				ShowWindow(hwnd, SW_HIDE);
				db_set_b(0, "CList", "State", SETTING_STATE_HIDDEN);
			}
			else db_set_b(0, "CList", "State", SETTING_STATE_MINIMIZED);

			if (db_get_b(0, "CList", "DisableWorkingSet", 1))
				SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
		}
		// drop thru
	case WM_MOVE:
		if (!IsIconic(hwnd)) {
			RECT rc;
			GetWindowRect(hwnd, &rc);

			//if docked, dont remember pos (except for width)
			if (!Clist_IsDocked()) {
				db_set_dw(0, "CList", "Height", (uint32_t)(rc.bottom - rc.top));
				db_set_dw(0, "CList", "x", (uint32_t)rc.left);
				db_set_dw(0, "CList", "y", (uint32_t)rc.top);
			}
			db_set_dw(0, "CList", "Width", (uint32_t)(rc.right - rc.left));
		}
		return FALSE;

	case WM_SETFOCUS:
		SetFocus(g_clistApi.hwndContactTree);
		return 0;

	case WM_ACTIVATE:
		if (wParam == WA_INACTIVE) {
			if ((HWND)wParam != hwnd)
				if (cluiopt.transparent)
					if (transparentFocus)
						SetTimer(hwnd, TM_AUTOALPHA, 250, nullptr);
		}
		else {
			if (cluiopt.transparent) {
				KillTimer(hwnd, TM_AUTOALPHA);
				SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (uint8_t)cluiopt.alpha, LWA_ALPHA);
				transparentFocus = 1;
			}
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_SETCURSOR:
		if (cluiopt.transparent) {
			if (!transparentFocus && GetForegroundWindow() != hwnd) {
				SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (uint8_t)cluiopt.alpha, LWA_ALPHA);
				transparentFocus = 1;
				SetTimer(hwnd, TM_AUTOALPHA, 250, nullptr);
			}
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_NCHITTEST:
		{
			LRESULT result;
			result = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
			if (result == HTSIZE || result == HTTOP || result == HTTOPLEFT || result == HTTOPRIGHT ||
				result == HTBOTTOM || result == HTBOTTOMRIGHT || result == HTBOTTOMLEFT)
				if (db_get_b(0, "CLUI", "AutoSize", 0))
					return HTCLIENT;
			return result;
		}

	case WM_TIMER:
		if (wParam == TM_AUTOALPHA) {
			int inwnd;

			if (GetForegroundWindow() == hwnd) {
				KillTimer(hwnd, TM_AUTOALPHA);
				inwnd = 1;
			}
			else {
				POINT pt;
				HWND hwndPt;
				pt.x = (short)LOWORD(GetMessagePos());
				pt.y = (short)HIWORD(GetMessagePos());
				hwndPt = WindowFromPoint(pt);
				inwnd = (hwndPt == hwnd || GetParent(hwndPt) == hwnd);
			}
			if (inwnd != transparentFocus) {        //change
				transparentFocus = inwnd;
				if (transparentFocus)
					SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (uint8_t)cluiopt.alpha, LWA_ALPHA);
				else
					SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (uint8_t)db_get_b(0, "CList", "AutoAlpha", SETTING_AUTOALPHA_DEFAULT), LWA_ALPHA);
			}
			if (!transparentFocus)
				KillTimer(hwnd, TM_AUTOALPHA);
		}
		return TRUE;

	case WM_SHOWWINDOW:
		if (lParam)
			break;
		if (noRecurse)
			break;
		if (!db_get_b(0, "CLUI", "FadeInOut", 0))
			break;
		if (GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_LAYERED) {
			uint32_t thisTick, startTick;
			int sourceAlpha, destAlpha;
			if (wParam) {
				sourceAlpha = 0;
				destAlpha = (uint8_t)cluiopt.alpha;
				SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_ALPHA);
				noRecurse = 1;
				ShowWindow(hwnd, SW_SHOW);
				noRecurse = 0;
			}
			else {
				sourceAlpha = (uint8_t)cluiopt.alpha;
				destAlpha = 0;
			}
			for (startTick = GetTickCount();;) {
				thisTick = GetTickCount();
				if (thisTick >= startTick + 200)
					break;
				SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0),
					(uint8_t)(sourceAlpha + (destAlpha - sourceAlpha) * (int)(thisTick - startTick) / 200), LWA_ALPHA);
			}
			SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (uint8_t)destAlpha, LWA_ALPHA);
		}
		else {
			if (wParam)
				SetForegroundWindow(hwnd);
			AnimateWindow(hwnd, 200, AW_BLEND | (wParam ? 0 : AW_HIDE));
			SetWindowPos(g_clistApi.hwndContactTree, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		}
		break;

	case WM_MENURBUTTONUP: /* this API is so badly documented at MSDN!! */
		{
			UINT id = 0;

			id = GetMenuItemID((HMENU)lParam, LOWORD(wParam)); /* LOWORD(wParam) contains the menu pos in its parent menu */
			if (id != (-1))
				SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(id, 0), 0);
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_SYSCOMMAND:
		switch (wParam) {
		case SC_MAXIMIZE:
			return 0;

		case SC_MINIMIZE:
		case SC_CLOSE:
			if ((GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) ||
				db_get_b(0, "CList", "Min2Tray", SETTING_MIN2TRAY_DEFAULT)) {
				ShowWindow(hwnd, SW_HIDE);
				db_set_b(0, "CList", "State", SETTING_STATE_HIDDEN);

				if (db_get_b(0, "CList", "DisableWorkingSet", 1))
					SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);

				return 0;
			}
			else if (wParam == SC_CLOSE)
				wParam = SC_MINIMIZE;
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_COMMAND:
		if (Clist_MenuProcessCommand(LOWORD(wParam), MPCF_MAINMENU, 0))
			break;

		switch (LOWORD(wParam)) {
		case ID_TRAY_EXIT:
		case ID_ICQ_EXIT:
			if (Miranda_OkToExit())
				DestroyWindow(hwnd);
			break;

		case ID_TRAY_HIDE:
			g_clistApi.pfnShowHide();
			break;

		case POPUP_NEWGROUP:
			SendMessage(g_clistApi.hwndContactTree, CLM_SETHIDEEMPTYGROUPS, 0, 0);
			Clist_GroupCreate(0, nullptr);
			break;

		case POPUP_HIDEOFFLINE:
			g_clistApi.pfnSetHideOffline(-1);
			break;

		case POPUP_HIDEOFFLINEROOT:
			CallService(MS_CLIST_TOGGLEHIDEOFFLINEROOT, 0, 0);
			break;

		case POPUP_HIDEEMPTYGROUPS:
			CallService(MS_CLIST_TOGGLEEMPTYGROUPS);
			break;

		case POPUP_DISABLEGROUPS:
			CallService(MS_CLIST_TOGGLEGROUPS);
			break;

		case POPUP_HIDEMIRANDA:
			g_clistApi.pfnShowHide();
			break;
		}
		return FALSE;

	case WM_KEYDOWN:
		Clist_MenuProcessHotkey(wParam);
		break;

	case WM_GETMINMAXINFO:
		DefWindowProc(hwnd, msg, wParam, lParam);
		((LPMINMAXINFO)lParam)->ptMinTrackSize.x = 16 + GetSystemMetrics(SM_CXHTHUMB);
		((LPMINMAXINFO)lParam)->ptMinTrackSize.y = 16;
		return 0;

	case WM_SETTINGCHANGE:
		if (wParam == SPI_SETWORKAREA && (GetWindowLongPtr(hwnd, GWL_STYLE) & (WS_VISIBLE | WS_MINIMIZE)) == WS_VISIBLE && !Clist_IsDocked()) {
			RECT rc;
			GetWindowRect(hwnd, &rc);
			if (Utils_AssertInsideScreen(&rc) == 1)
				MoveWindow(hwnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_DISPLAYCHANGE:
		DefWindowProc(hwnd, msg, wParam, lParam);
		SendMessage(g_clistApi.hwndContactTree, WM_SIZE, 0, 0); //forces it to send a cln_listsizechanged
		break;

		//MSG FROM CHILD CONTROL
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->hwndFrom == g_clistApi.hwndContactTree) {
			NMCLISTCONTROL *nmc = (NMCLISTCONTROL*)lParam;
			switch (((LPNMHDR)lParam)->code) {
			case CLN_EXPANDED:
				Clist_GroupSetExpanded(UINT_PTR(nmc->hItem), nmc->action);
				return FALSE;

			case CLN_DRAGGING:
				ClientToScreen(hwnd, &nmc->pt);
				if (!(nmc->flags & CLNF_ISGROUP))
					if (NotifyEventHooks(hContactDraggingEvent, (WPARAM)nmc->hItem, MAKELPARAM(nmc->pt.x, nmc->pt.y))) {
						SetCursor(LoadCursor(g_clistApi.hInst, MAKEINTRESOURCE(IDC_DROPUSER)));
						return TRUE;
					}
				break;

			case CLN_DRAGSTOP:
				if (!(nmc->flags & CLNF_ISGROUP))
					NotifyEventHooks(hContactDragStopEvent, (WPARAM)nmc->hItem, 0);
				break;

			case CLN_DROPPED:
				ClientToScreen(hwnd, &nmc->pt);
				if (!(nmc->flags & CLNF_ISGROUP))
					if (NotifyEventHooks(hContactDroppedEvent, (WPARAM)nmc->hItem, MAKELPARAM(nmc->pt.x, nmc->pt.y))) {
						SetCursor(LoadCursor(g_clistApi.hInst, MAKEINTRESOURCE(IDC_DROPUSER)));
						return TRUE;
					}
				break;

			case CLN_NEWCONTACT:
				if (nmc != nullptr)
					ExtraIcon_SetAll((UINT_PTR)nmc->hItem);
				return TRUE;

			case CLN_LISTREBUILT:
				ExtraIcon_SetAll();
				return FALSE;

			case NM_KEYDOWN:
				return Clist_MenuProcessHotkey(((NMKEY*)lParam)->nVKey);

			case CLN_LISTSIZECHANGE:
				{
					RECT rcWindow, rcTree, rcWorkArea;
					int maxHeight, newHeight;

					if (!db_get_b(0, "CLUI", "AutoSize", 0))
						break;
					if (Clist_IsDocked())
						break;
					maxHeight = db_get_b(0, "CLUI", "MaxSizeHeight", 75);
					GetWindowRect(hwnd, &rcWindow);
					GetWindowRect(g_clistApi.hwndContactTree, &rcTree);

					SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, FALSE);
					HMONITOR hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
					MONITORINFO mi;
					mi.cbSize = sizeof(mi);
					if (GetMonitorInfo(hMon, &mi))
						rcWorkArea = mi.rcWork;

					newHeight = max(nmc->pt.y, LONG(9)) + 1 + (rcWindow.bottom - rcWindow.top) - (rcTree.bottom - rcTree.top);
					if (newHeight > (rcWorkArea.bottom - rcWorkArea.top) * maxHeight / 100)
						newHeight = (rcWorkArea.bottom - rcWorkArea.top) * maxHeight / 100;
					if (db_get_b(0, "CLUI", "AutoSizeUpward", 0)) {
						rcWindow.top = rcWindow.bottom - newHeight;
						if (rcWindow.top < rcWorkArea.top)
							rcWindow.top = rcWorkArea.top;
					}
					else {
						rcWindow.bottom = rcWindow.top + newHeight;
						if (rcWindow.bottom > rcWorkArea.bottom)
							rcWindow.bottom = rcWorkArea.bottom;
					}
					SetWindowPos(hwnd, nullptr, rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top,
						SWP_NOZORDER | SWP_NOACTIVATE);
					break;
				}
			case NM_CLICK:
				{
					uint32_t hitFlags;
					HANDLE hItem = (HANDLE)SendMessage(g_clistApi.hwndContactTree, CLM_HITTEST, (WPARAM)&hitFlags, MAKELPARAM(nmc->pt.x, nmc->pt.y));
					if (hItem) {
						if (hitFlags & CLCHT_ONITEMEXTRA) {
							if (!IsHContactGroup((UINT_PTR)hItem) && !IsHContactInfo((UINT_PTR)hItem))
								if (Clist_GetCacheEntry((UINT_PTR)nmc->hItem))
									NotifyEventHooks(hEventExtraClick, (WPARAM)nmc->hItem, nmc->iColumn + 1);
						}
						break;
					}

					if ((hitFlags & (CLCHT_NOWHERE | CLCHT_INLEFTMARGIN | CLCHT_BELOWITEMS)) == 0)
						break;

					if (db_get_b(0, "CLUI", "ClientAreaDrag", SETTING_CLIENTDRAG_DEFAULT)) {
						POINT pt = nmc->pt;
						ClientToScreen(g_clistApi.hwndContactTree, &pt);
						return SendMessage(hwnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, MAKELPARAM(pt.x, pt.y));
					}
				}
				break;
			}
		}
		else if (((LPNMHDR)lParam)->hwndFrom == g_clistApi.hwndStatus) {
			if (((LPNMHDR)lParam)->code == NM_CLICK) {
				unsigned int nParts, nPanel;
				NMMOUSE *nm = (NMMOUSE *)lParam;
				HMENU hMenu;
				RECT rc;
				POINT pt;

				hMenu = Menu_GetStatusMenu();
				nParts = SendMessage(g_clistApi.hwndStatus, SB_GETPARTS, 0, 0);
				if (nm->dwItemSpec == 0xFFFFFFFE) {
					nPanel = nParts - 1;
					SendMessage(g_clistApi.hwndStatus, SB_GETRECT, nPanel, (LPARAM)& rc);
					if (nm->pt.x < rc.left)
						return FALSE;
				}
				else nPanel = nm->dwItemSpec;

				if (nParts > 0) {
					unsigned int cpnl = 0;
					int mcnt = GetMenuItemCount(hMenu);
					for (int i = 0; i < mcnt; i++) {
						HMENU hMenus = GetSubMenu(hMenu, i);
						if (hMenus && cpnl++ == nPanel) {
							hMenu = hMenus;
							break;
						}
					}
				}
				SendMessage(g_clistApi.hwndStatus, SB_GETRECT, nPanel, (LPARAM)& rc);
				pt.x = rc.left;
				pt.y = rc.top;
				ClientToScreen(g_clistApi.hwndStatus, &pt);
				TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, nullptr);
			}
		}
		return FALSE;

	case WM_MENUSELECT:
		if (lParam && (HMENU)lParam == g_clistApi.hMenuMain) {
			int pos = LOWORD(wParam);
			POINT pt;
			GetCursorPos(&pt);
			if ((pos == 0 || pos == 1) && (HIWORD(wParam) & MF_POPUP) && (!(HIWORD(wParam) & MF_MOUSESELECT) || MenuItemFromPoint(hwnd, g_clistApi.hMenuMain, pt) != -1)) {
				MENUITEMINFO mii = { 0 };
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_SUBMENU;
				mii.hSubMenu = (pos == 0) ? Menu_GetMainMenu() : Menu_GetStatusMenu();
				SetMenuItemInfo(g_clistApi.hMenuMain, pos, TRUE, &mii);
			}
		}
		break;

	case WM_CONTEXTMENU:
		{
			// x/y might be -1 if it was generated by a kb click
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

			RECT rc;
			GetWindowRect(g_clistApi.hwndContactTree, &rc);
			if (pt.x == -1 && pt.y == -1) {
				// all this is done in screen-coords!
				GetCursorPos(&pt);
				// the mouse isnt near the window, so put it in the middle of the window
				if (!PtInRect(&rc, pt)) {
					pt.x = rc.left + (rc.right - rc.left) / 2;
					pt.y = rc.top + (rc.bottom - rc.top) / 2;
				}
			}
			if (PtInRect(&rc, pt)) {
				HMENU hMenu = Menu_BuildGroupMenu();
				TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
				Menu_DestroyNestedMenu(hMenu);
				return 0;
			}
			GetWindowRect(g_clistApi.hwndStatus, &rc);
			if (PtInRect(&rc, pt)) {
				HMENU hMenu;
				if (db_get_b(0, "CLUI", "SBarRightClk", 0))
					hMenu = Menu_GetMainMenu();
				else
					hMenu = Menu_GetStatusMenu();
				TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
				return 0;
			}
		}
		break;

	case WM_MEASUREITEM:
		if (((LPMEASUREITEMSTRUCT)lParam)->itemData == MENU_MIRANDAMENU) {
			((LPMEASUREITEMSTRUCT)lParam)->itemWidth = g_IconWidth * 4 / 3;
			((LPMEASUREITEMSTRUCT)lParam)->itemHeight = 0;
			return TRUE;
		}
		return Menu_MeasureItem(lParam);

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			if (dis->hwndItem == g_clistApi.hwndStatus) {
				char *szProto = (char *)dis->itemData;
				if (szProto == nullptr) return 0;
				int status, x;
				SIZE textSize;
				uint8_t showOpts = db_get_b(0, "CLUI", "SBarShow", 1);
				status = Proto_GetStatus(szProto);
				SetBkMode(dis->hDC, TRANSPARENT);
				x = dis->rcItem.left;
				if (showOpts & 1) {
					HICON hIcon = Skin_LoadProtoIcon(szProto, status);
					DrawIconEx(dis->hDC, x, (dis->rcItem.top + dis->rcItem.bottom - g_IconHeight) >> 1, hIcon,
						g_IconWidth, g_IconHeight, 0, nullptr, DI_NORMAL);
					IcoLib_ReleaseIcon(hIcon);
					if (Proto_GetAccount(szProto)->IsLocked()) {
						hIcon = Skin_LoadIcon(SKINICON_OTHER_STATUS_LOCKED);
						if (hIcon != nullptr) {
							DrawIconEx(dis->hDC, x, (dis->rcItem.top + dis->rcItem.bottom - g_IconHeight) >> 1, hIcon,
								g_IconWidth, g_IconHeight, 0, nullptr, DI_NORMAL);
							IcoLib_ReleaseIcon(hIcon);
						}

					}
					x += g_IconWidth + 2;
				}
				else
					x += 2;
				if (showOpts & 2) {
					PROTOACCOUNT *pa;
					wchar_t tszName[64];
					if ((pa = Proto_GetAccount(szProto)) != nullptr)
						mir_snwprintf(tszName, L"%s ", pa->tszAccountName);
					else
						tszName[0] = 0;

					GetTextExtentPoint32(dis->hDC, tszName, (int)mir_wstrlen(tszName), &textSize);
					TextOut(dis->hDC, x, (dis->rcItem.top + dis->rcItem.bottom - textSize.cy) >> 1, tszName, (int)mir_wstrlen(tszName));
					x += textSize.cx;
				}
				if (showOpts & 4) {
					wchar_t* szStatus = Clist_GetStatusModeDescription(status, 0);
					if (!szStatus)
						szStatus = L"";
					GetTextExtentPoint32(dis->hDC, szStatus, (int)mir_wstrlen(szStatus), &textSize);
					TextOut(dis->hDC, x, (dis->rcItem.top + dis->rcItem.bottom - textSize.cy) >> 1, szStatus, (int)mir_wstrlen(szStatus));
				}
			}
			else if (dis->CtlType == ODT_MENU) {
				if (dis->itemData == MENU_MIRANDAMENU) {
					HICON hIcon = Skin_LoadIcon(SKINICON_OTHER_MAINMENU);
					Clist_DrawMenuItem(dis, CopyIcon(hIcon), nullptr);
					IcoLib_ReleaseIcon(hIcon);
					return TRUE;
				}
				return Menu_DrawItem(lParam);
			}
		}
		return 0;

	case WM_CLOSE:
		if (Miranda_OkToExit())
			DestroyWindow(hwnd);
		return FALSE;

	case WM_DESTROY:
		if (!IsIconic(hwnd)) {
			RECT rc;
			GetWindowRect(hwnd, &rc);

			//if docked, dont remember pos (except for width)
			if (!Clist_IsDocked()) {
				db_set_dw(0, "CList", "Height", (uint32_t)(rc.bottom - rc.top));
				db_set_dw(0, "CList", "x", (uint32_t)rc.left);
				db_set_dw(0, "CList", "y", (uint32_t)rc.top);
			}
			db_set_dw(0, "CList", "Width", (uint32_t)(rc.right - rc.left));
		}

		RemoveMenu(g_clistApi.hMenuMain, 0, MF_BYPOSITION);
		RemoveMenu(g_clistApi.hMenuMain, 0, MF_BYPOSITION);
		DestroyMenu(g_clistApi.hMenuMain);
		g_clistApi.hMenuMain = nullptr;

		if (g_clistApi.hwndStatus) {
			DestroyWindow(g_clistApi.hwndStatus);
			g_clistApi.hwndStatus = nullptr;
		}

		// Disconnect all protocols
		DisconnectAll();

		ShowWindow(hwnd, SW_HIDE);
		DestroyWindow(g_clistApi.hwndContactTree);
		FreeLibrary(hUserDll);
		PostQuitMessage(0);

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return TRUE;
}
