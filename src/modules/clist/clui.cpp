/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\core\commonheaders.h"
#include "../database/profilemanager.h"
#include "clc.h"

#define TM_AUTOALPHA  1
#define MENU_MIRANDAMENU         0xFFFF1234

extern HANDLE hEventExtraClick;

static HMODULE hUserDll;
static HANDLE hContactDraggingEvent, hContactDroppedEvent, hContactDragStopEvent;
static int transparentFocus = 1;
UINT uMsgProcessProfile;

#define M_RESTORESTATUS  (WM_USER+7)

typedef struct {
	int showsbar;
	int showgrip;
	int transparent;
	int alpha;
}
	CluiOpts;

static CluiOpts cluiopt = {0};

void fnLoadCluiGlobalOpts()
{
	cluiopt.showsbar = db_get_b(NULL, "CLUI", "ShowSBar", 1);
	cluiopt.showgrip = db_get_b(NULL, "CLUI", "ShowGrip", 1);
	cluiopt.transparent = db_get_b(NULL, "CList", "Transparent", SETTING_TRANSPARENT_DEFAULT);
	cluiopt.alpha = db_get_b(NULL, "CList", "Alpha", SETTING_ALPHA_DEFAULT);
}

static int CluiModulesLoaded(WPARAM, LPARAM)
{
	if (cli.hMenuMain) {
		MENUITEMINFO mii = { sizeof(mii) };
		mii.fMask = MIIM_SUBMENU;
		mii.hSubMenu = (HMENU) CallService(MS_CLIST_MENUGETMAIN, 0, 0);
		SetMenuItemInfo(cli.hMenuMain, 0, TRUE, &mii);
		mii.hSubMenu = (HMENU) CallService(MS_CLIST_MENUGETSTATUS, 0, 0);
		SetMenuItemInfo(cli.hMenuMain, 1, TRUE, &mii);
	}
	return 0;
}

// Disconnect all protocols.
// Happens on shutdown and standby.
static void DisconnectAll()
{
	for (int i = 0; i < accounts.getCount(); i++)
		CallProtoServiceInt(NULL,accounts[i]->szModuleName, PS_SETSTATUS, ID_STATUS_OFFLINE, 0);
}

static int CluiIconsChanged(WPARAM, LPARAM)
{
	DrawMenuBar(cli.hwndContactList);
	return 0;
}

static HGENMENU hRenameMenuItem;

static int MenuItem_PreBuild(WPARAM, LPARAM)
{
	TCHAR cls[128];
	HWND hwndClist = GetFocus();
	GetClassName(hwndClist, cls, SIZEOF(cls));
	hwndClist = (!lstrcmp( _T(CLISTCONTROL_CLASS), cls)) ? hwndClist : cli.hwndContactList;
	HANDLE hItem = (HANDLE)SendMessage(hwndClist, CLM_GETSELECTION, 0, 0);
	Menu_ShowItem(hRenameMenuItem, hItem != 0);
	return 0;
}

static INT_PTR MenuItem_RenameContact(WPARAM, LPARAM)
{
	TCHAR cls[128];
	HWND hwndClist = GetFocus();
	GetClassName(hwndClist, cls, SIZEOF(cls));
	// worst case scenario, the rename is sent to the main contact list
	hwndClist = (!lstrcmp( _T(CLISTCONTROL_CLASS), cls)) ? hwndClist : cli.hwndContactList;
	HANDLE hItem = (HANDLE)SendMessage(hwndClist, CLM_GETSELECTION, 0, 0);
	if (hItem) {
		SetFocus(hwndClist);
		SendMessage(hwndClist, CLM_EDITLABEL, (WPARAM) hItem, 0);
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
			SendDlgItemMessage(hWnd, IDC_TOPLINE, WM_SETFONT, (WPARAM) CreateFontIndirect(&lf), 0);
		
			TCHAR szFormat[256], szFinal[256];
			GetDlgItemText(hWnd, IDC_TOPLINE, szFormat, SIZEOF(szFormat));
			mir_sntprintf(szFinal, SIZEOF(szFinal), szFormat, cli.pfnGetContactDisplayName(lParam, 0));
			SetDlgItemText(hWnd, IDC_TOPLINE, szFinal);
		}
		SetFocus( GetDlgItem(hWnd, IDNO));
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDYES:
			if (IsDlgButtonChecked(hWnd, IDC_HIDE)) {
				EndDialog(hWnd, IDC_HIDE);
				break;
			}
			//fall through
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
		DeleteObject((HFONT) SendDlgItemMessage(hWnd, IDC_TOPLINE, WM_GETFONT, 0, 0));
		break;
	}

	return FALSE;
}

static INT_PTR MenuItem_DeleteContact(WPARAM wParam, LPARAM lParam)
{
	//see notes about deleting contacts on PF1_SERVERCLIST servers in m_protosvc.h
	UINT_PTR action;

	if (db_get_b(NULL, "CList", "ConfirmDelete", SETTING_CONFIRMDELETE_DEFAULT) && !(GetKeyState(VK_SHIFT) & 0x8000))
		// Ask user for confirmation, and if the contact should be archived (hidden, not deleted)
		action = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DELETECONTACT), (HWND) lParam, AskForConfirmationDlgProc, wParam);
	else
		action = IDYES;

	switch (action) {
	case IDC_HIDE: // Archive contact
		db_set_b(wParam, "CList", "Hidden", 1);
		break;
		
	case IDYES: // Delete contact
		char *szProto = GetContactProto(wParam);
		if (szProto != NULL) {
			// Check if protocol uses server side lists
			DWORD caps = CallProtoServiceInt(NULL, szProto, PS_GETCAPS, PFLAGNUM_1, 0);
			if (caps & PF1_SERVERCLIST) {
				int status = CallProtoServiceInt(NULL, szProto, PS_GETSTATUS, 0, 0);
				if (status == ID_STATUS_OFFLINE || (status >= ID_STATUS_CONNECTING && status < ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES)) {
					// Set a flag so we remember to delete the contact when the protocol goes online the next time
					db_set_b(wParam, "CList", "Delete", 1);
					MessageBox(NULL,
						TranslateT("This contact is on an instant messaging system which stores its contact list on a central server. The contact will be removed from the server and from your contact list when you next connect to that network."),
						TranslateT("Delete contact"), MB_ICONINFORMATION | MB_OK);
					return 0;
				}
			}
		}

		CallService(MS_DB_CONTACT_DELETE, wParam, 0);
		break;
	}

	return 0;
}

static INT_PTR MenuItem_AddContactToList(WPARAM hContact, LPARAM)
{
	ADDCONTACTSTRUCT acs = { 0 };
	acs.hContact = hContact;
	acs.handleType = HANDLE_CONTACT;
	acs.szProto = "";
	CallService(MS_ADDCONTACT_SHOW, NULL, (LPARAM)&acs);
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
	if (cli.pfnDocking_ProcessWindowMessage((WPARAM)&m, (LPARAM)&result))
		return result;
	if (cli.pfnTrayIconProcessMessage((WPARAM)&m, (LPARAM)&result))
		return result;
	if (cli.pfnHotkeysProcessMessage((WPARAM)&m, (LPARAM)&result))
		return result;

	return cli.pfnContactListWndProc(hwnd, msg, wParam, lParam);
}

int LoadCLUIModule(void)
{
	DBVARIANT dbv;
	TCHAR titleText[256];

	uMsgProcessProfile = RegisterWindowMessage(_T("Miranda::ProcessProfile"));
	cli.pfnLoadCluiGlobalOpts();

	HookEvent(ME_SYSTEM_MODULESLOADED, CluiModulesLoaded);
	HookEvent(ME_SKIN_ICONSCHANGED, CluiIconsChanged);

	hContactDraggingEvent = CreateHookableEvent(ME_CLUI_CONTACTDRAGGING);
	hContactDroppedEvent  = CreateHookableEvent(ME_CLUI_CONTACTDROPPED);
	hContactDragStopEvent = CreateHookableEvent(ME_CLUI_CONTACTDRAGSTOP);

	WNDCLASSEX wndclass;
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_GLOBALCLASS;
	wndclass.lpfnWndProc = cli.pfnContactListControlWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = sizeof(void *);
	wndclass.hInstance = cli.hInst;
	wndclass.hIcon = NULL;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = _T(CLISTCONTROL_CLASS);
	wndclass.hIconSm = NULL;
	RegisterClassEx(&wndclass);

	wndclass.style = CS_HREDRAW | CS_VREDRAW | ((db_get_b(NULL, "CList", "WindowShadow", 0) == 1) ? CS_DROPSHADOW : 0);
	wndclass.lpfnWndProc = ContactListWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = cli.hInst;
	wndclass.hIcon = LoadSkinIcon(SKINICON_OTHER_MIRANDA, true);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH) (COLOR_3DFACE + 1);
	wndclass.lpszMenuName = MAKEINTRESOURCE(IDR_CLISTMENU);
	wndclass.lpszClassName = _T(MIRANDACLASS);
	wndclass.hIconSm = LoadSkinIcon(SKINICON_OTHER_MIRANDA);
	RegisterClassEx(&wndclass);

	if (db_get_ts(NULL, "CList", "TitleText", &dbv))
		mir_tstrncpy(titleText, _T(MIRANDANAME), SIZEOF(titleText));
	else {
		mir_tstrncpy(titleText, dbv.ptszVal, SIZEOF(titleText));
		db_free(&dbv);
	}

	RECT pos;
	pos.left = (int)db_get_dw(NULL, "CList", "x", 700);
	pos.top = (int)db_get_dw(NULL, "CList", "y", 221);
	pos.right = pos.left + (int)db_get_dw(NULL, "CList", "Width", 108);
	pos.bottom = pos.top + (int)db_get_dw(NULL, "CList", "Height", 310);

	Utils_AssertInsideScreen(&pos);

	cli.hwndContactList = CreateWindowEx(
		(db_get_b(NULL, "CList", "ToolWindow", SETTING_TOOLWINDOW_DEFAULT) ? WS_EX_TOOLWINDOW : WS_EX_APPWINDOW),
		_T(MIRANDACLASS),
		titleText,
		WS_POPUPWINDOW | WS_THICKFRAME | WS_CLIPCHILDREN |
		(db_get_b(NULL, "CLUI", "ShowCaption", SETTING_SHOWCAPTION_DEFAULT) ?  WS_CAPTION | WS_SYSMENU |
			(db_get_b(NULL, "CList", "Min2Tray", SETTING_MIN2TRAY_DEFAULT) ? 0 : WS_MINIMIZEBOX) : 0),
		pos.left, pos.top, pos.right - pos.left, pos.bottom - pos.top,
		NULL, NULL, cli.hInst, NULL);

	if (db_get_b(NULL, "CList", "OnDesktop", 0)) {
		HWND hProgMan = FindWindow(_T("Progman"), NULL);
		if (IsWindow(hProgMan))
			SetParent(cli.hwndContactList, hProgMan);
	}

	cli.pfnOnCreateClc();

	PostMessage(cli.hwndContactList, M_RESTORESTATUS, 0, 0);

	int state = db_get_b(NULL, "CList", "State", SETTING_STATE_NORMAL);
	cli.hMenuMain = GetMenu(cli.hwndContactList);
	if (!db_get_b(NULL, "CLUI", "ShowMainMenu", SETTING_SHOWMAINMENU_DEFAULT))
		SetMenu(cli.hwndContactList, NULL);
	if (state == SETTING_STATE_NORMAL)
		ShowWindow(cli.hwndContactList, SW_SHOW);
	else if (state == SETTING_STATE_MINIMIZED)
		ShowWindow(cli.hwndContactList, SW_SHOWMINIMIZED);
	SetWindowPos(cli.hwndContactList,
		db_get_b(NULL, "CList", "OnTop", SETTING_ONTOP_DEFAULT) ? HWND_TOPMOST : HWND_NOTOPMOST,
		0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

	CLISTMENUITEM mi = { sizeof(mi) };

	CreateServiceFunction("CList/DeleteContactCommand", MenuItem_DeleteContact);
	mi.position = 2000070000;
	mi.icolibItem = GetSkinIconHandle(SKINICON_OTHER_DELETE);
	mi.pszName = LPGEN("De&lete");
	mi.pszService = "CList/DeleteContactCommand";
	Menu_AddContactMenuItem(&mi);

	CreateServiceFunction("CList/RenameContactCommand", MenuItem_RenameContact);
	mi.position = 2000050000;
	mi.icolibItem = GetSkinIconHandle(SKINICON_OTHER_RENAME);
	mi.pszName = LPGEN("&Rename");
	mi.pszService = "CList/RenameContactCommand";
	hRenameMenuItem = Menu_AddContactMenuItem(&mi);

	CreateServiceFunction("CList/AddToListContactCommand", MenuItem_AddContactToList);
	mi.position = -2050000000;
	mi.flags |= CMIF_NOTONLIST;
	mi.icolibItem = GetSkinIconHandle(SKINICON_OTHER_ADDCONTACT);
	mi.pszName = LPGEN("&Add permanently to list");
	mi.pszService = "CList/AddToListContactCommand";
	Menu_AddContactMenuItem(&mi);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, MenuItem_PreBuild);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// default contact list window procedure

void fnDrawMenuItem(DRAWITEMSTRUCT *dis, HICON hIcon, HICON eventIcon)
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
		} else {
			/* flush the DC with the menu bar colour (only supported on XP) and then draw the icon */
			hBr = GetSysColorBrush(COLOR_MENUBAR);
			FillRect(dis->hDC, &dis->rcItem, hBr);
			DeleteObject(hBr);
		} //if
		/* draw the icon */
		if (eventIcon != 0) {
			DrawState(dis->hDC, NULL, NULL, (LPARAM) eventIcon, 0, 2, (dis->rcItem.bottom + dis->rcItem.top - g_IconHeight) / 2 + (dis->itemState & ODS_SELECTED ? 1 : 0), 0, 0, DST_ICON | (dis->itemState & ODS_INACTIVE ? DSS_DISABLED : DSS_NORMAL));
			DrawState(dis->hDC, NULL, NULL, (LPARAM) hIcon, 0, 4 + g_IconWidth, (dis->rcItem.bottom + dis->rcItem.top - g_IconHeight) / 2 + (dis->itemState & ODS_SELECTED ? 1 : 0), 0, 0, DST_ICON | (dis->itemState & ODS_INACTIVE ? DSS_DISABLED : DSS_NORMAL));
		}
		else DrawState(dis->hDC, NULL, NULL, (LPARAM) hIcon, 0, (dis->rcItem.right + dis->rcItem.left - g_IconWidth) / 2 + (dis->itemState & ODS_SELECTED ? 1 : 0), (dis->rcItem.bottom + dis->rcItem.top - g_IconHeight) / 2 + (dis->itemState & ODS_SELECTED ? 1 : 0), 0, 0, DST_ICON | (dis->itemState & ODS_INACTIVE ? DSS_DISABLED : DSS_NORMAL));
	}
	else {
		/* non-flat menus, flush the DC with a normal menu colour */
		FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_MENU));
		if (dis->itemState & ODS_HOTLIGHT)
			DrawEdge(dis->hDC, &dis->rcItem, BDR_RAISEDINNER, BF_RECT);
		else if (dis->itemState & ODS_SELECTED)
			DrawEdge(dis->hDC, &dis->rcItem, BDR_SUNKENOUTER, BF_RECT);

		if (eventIcon != 0) {
			DrawState(dis->hDC, NULL, NULL, (LPARAM) eventIcon, 0, 2, (dis->rcItem.bottom + dis->rcItem.top - g_IconHeight) / 2 + (dis->itemState & ODS_SELECTED ? 1 : 0), 0, 0, DST_ICON | (dis->itemState & ODS_INACTIVE ? DSS_DISABLED : DSS_NORMAL));
			DrawState(dis->hDC, NULL, NULL, (LPARAM) hIcon, 0, 4 + g_IconWidth, (dis->rcItem.bottom + dis->rcItem.top - g_IconHeight) / 2 + (dis->itemState & ODS_SELECTED ? 1 : 0), 0, 0, DST_ICON | (dis->itemState & ODS_INACTIVE ? DSS_DISABLED : DSS_NORMAL));
		}
		else DrawState(dis->hDC, NULL, NULL, (LPARAM) hIcon, 0, (dis->rcItem.right + dis->rcItem.left - g_IconWidth) / 2 + (dis->itemState & ODS_SELECTED ? 1 : 0), (dis->rcItem.bottom + dis->rcItem.top - g_IconHeight) / 2 + (dis->itemState & ODS_SELECTED ? 1 : 0), 0, 0, DST_ICON | (dis->itemState & ODS_INACTIVE ? DSS_DISABLED : DSS_NORMAL));
	}

	DestroyIcon(hIcon);
	return;
}

LRESULT CALLBACK fnContactListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int noRecurse = 0;

	if (msg == uMsgProcessProfile) {
		TCHAR profile[MAX_PATH];
		int rc;
		// wParam = (ATOM)hProfileAtom, lParam = 0
		if (GlobalGetAtomName((ATOM) wParam, profile, SIZEOF(profile))) {
			rc = lstrcmpi(profile, VARST(_T("%miranda_userdata%\\%miranda_profilename%.dat"))) == 0;
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
	case WM_NCCREATE:
		{
			MENUITEMINFO mii = { sizeof(mii) };
			mii.fMask = MIIM_TYPE | MIIM_DATA;
			mii.dwItemData = MENU_MIRANDAMENU;
			mii.fType = MFT_OWNERDRAW;
			SetMenuItemInfo(GetMenu(hwnd), 0, TRUE, &mii);
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_CREATE:
		TranslateMenu(GetMenu(hwnd));
		DrawMenuBar(hwnd);

		//create the status wnd
		{
			int flags = WS_CHILD | CCS_BOTTOM;
			flags |= cluiopt.showsbar ? WS_VISIBLE : 0;
			flags |= cluiopt.showgrip ? SBARS_SIZEGRIP : 0;
			cli.hwndStatus = CreateWindow(STATUSCLASSNAME, NULL, flags, 0, 0, 0, 0, hwnd, NULL, cli.hInst, NULL);
		}
		cli.pfnCluiProtocolStatusChanged(0, 0);

		//delay creation of CLC so that it can get the status icons right the first time (needs protocol modules loaded)
		PostMessage(hwnd, M_CREATECLC, 0, 0);

		if (cluiopt.transparent) {
			SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
			SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (BYTE) cluiopt.alpha, LWA_ALPHA);
		}
		transparentFocus = 1;
		return FALSE;

	case M_CREATECLC:
		cli.hwndContactTree = CreateWindow( _T(CLISTCONTROL_CLASS), _T(""),
			WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN
			| CLS_CONTACTLIST
			| (db_get_b(NULL, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT) ? CLS_USEGROUPS : 0)
			| (db_get_b(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT) ? CLS_HIDEOFFLINE : 0)
			| (db_get_b(NULL, "CList", "HideEmptyGroups", SETTING_HIDEEMPTYGROUPS_DEFAULT) ?
					CLS_HIDEEMPTYGROUPS : 0), 0, 0, 0, 0, hwnd, NULL, cli.hInst, NULL);
		SendMessage(hwnd, WM_SIZE, 0, 0);
		break;

	case M_RESTORESTATUS:
		#ifndef _DEBUG
		{
			int nStatus = db_get_w(NULL, "CList", "Status", ID_STATUS_OFFLINE);
			if (nStatus != ID_STATUS_OFFLINE) CallService(MS_CLIST_SETSTATUSMODE, nStatus, 0);
		}
		#endif
		break;

	// Power management
	case WM_POWERBROADCAST:
		switch ((DWORD) wParam) {
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
		SendMessage(cli.hwndContactTree, msg, wParam, lParam);
		SendMessage(cli.hwndStatus, msg, wParam, lParam);
		// XXX: only works with 4.71 with 95, IE4.
		SendMessage(cli.hwndStatus, SB_SETBKCOLOR, 0, GetSysColor(COLOR_3DFACE));
		break;

	case WM_SIZE:
		if (IsZoomed(hwnd))
			ShowWindow(hwnd, SW_SHOWNORMAL);
		{
			RECT rect, rcStatus;
			GetClientRect(hwnd, &rect);
			if (cluiopt.showsbar) {
				SetWindowPos(cli.hwndStatus, NULL, 0, rect.bottom - 20, rect.right - rect.left, 20, SWP_NOZORDER);
				GetWindowRect(cli.hwndStatus, &rcStatus);
				cli.pfnCluiProtocolStatusChanged(0, 0);
			}
			else
				rcStatus.top = rcStatus.bottom = 0;
			SetWindowPos(cli.hwndContactTree, NULL, 0, 0, rect.right, rect.bottom - (rcStatus.bottom - rcStatus.top), SWP_NOZORDER);
		}
		if (wParam == SIZE_MINIMIZED) {
			if ((GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) || db_get_b(NULL, "CList", "Min2Tray", SETTING_MIN2TRAY_DEFAULT)) {
				ShowWindow(hwnd, SW_HIDE);
				db_set_b(NULL, "CList", "State", SETTING_STATE_HIDDEN);
			}
			else db_set_b(NULL, "CList", "State", SETTING_STATE_MINIMIZED);

			if (db_get_b(NULL, "CList", "DisableWorkingSet", 1))
				SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
		}
		// drop thru
	case WM_MOVE:
		if (!IsIconic(hwnd)) {
			RECT rc;
			GetWindowRect(hwnd, &rc);

			//if docked, dont remember pos (except for width)
			if (!CallService(MS_CLIST_DOCKINGISDOCKED, 0, 0)) {
				db_set_dw(NULL, "CList", "Height", (DWORD) (rc.bottom - rc.top));
				db_set_dw(NULL, "CList", "x", (DWORD) rc.left);
				db_set_dw(NULL, "CList", "y", (DWORD) rc.top);
			}
			db_set_dw(NULL, "CList", "Width", (DWORD) (rc.right - rc.left));
		}
		return FALSE;

	case WM_SETFOCUS:
		SetFocus(cli.hwndContactTree);
		return 0;

	case WM_ACTIVATE:
		if (wParam == WA_INACTIVE) {
			if ((HWND) wParam != hwnd)
				if (cluiopt.transparent)
					if (transparentFocus)
						SetTimer(hwnd, TM_AUTOALPHA, 250, NULL);
		}
		else {
			if (cluiopt.transparent) {
				KillTimer(hwnd, TM_AUTOALPHA);
				SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (BYTE) cluiopt.alpha, LWA_ALPHA);
				transparentFocus = 1;
			}
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_SETCURSOR:
		if (cluiopt.transparent) {
			if (!transparentFocus && GetForegroundWindow() != hwnd) {
				SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (BYTE)cluiopt.alpha, LWA_ALPHA);
				transparentFocus = 1;
				SetTimer(hwnd, TM_AUTOALPHA, 250, NULL);
			}
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_NCHITTEST:
	{
		LRESULT result;
		result = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
		if (result == HTSIZE || result == HTTOP || result == HTTOPLEFT || result == HTTOPRIGHT  ||
			result == HTBOTTOM || result == HTBOTTOMRIGHT || result == HTBOTTOMLEFT)
			if (db_get_b(NULL, "CLUI", "AutoSize", 0))
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
				pt.x = (short) LOWORD(GetMessagePos());
				pt.y = (short) HIWORD(GetMessagePos());
				hwndPt = WindowFromPoint(pt);
				inwnd = (hwndPt == hwnd || GetParent(hwndPt) == hwnd);
			}
			if (inwnd != transparentFocus) {        //change
				transparentFocus = inwnd;
				if (transparentFocus)
					SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (BYTE) cluiopt.alpha, LWA_ALPHA);
				else
					SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (BYTE) db_get_b(NULL, "CList", "AutoAlpha", SETTING_AUTOALPHA_DEFAULT), LWA_ALPHA);
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
		if (!db_get_b(NULL, "CLUI", "FadeInOut", 0))
			break;
		if (GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_LAYERED) {
			DWORD thisTick, startTick;
			int sourceAlpha, destAlpha;
			if (wParam) {
				sourceAlpha = 0;
				destAlpha = (BYTE) cluiopt.alpha;
				SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_ALPHA);
				noRecurse = 1;
				ShowWindow(hwnd, SW_SHOW);
				noRecurse = 0;
			}
			else {
				sourceAlpha = (BYTE) cluiopt.alpha;
				destAlpha = 0;
			}
			for (startTick = GetTickCount();;) {
				thisTick = GetTickCount();
				if (thisTick >= startTick + 200)
					break;
				SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0),
					(BYTE) (sourceAlpha + (destAlpha - sourceAlpha) * (int)(thisTick - startTick) / 200), LWA_ALPHA);
			}
			SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), (BYTE) destAlpha, LWA_ALPHA);
		}
		else {
			if (wParam)
				SetForegroundWindow(hwnd);
			AnimateWindow(hwnd, 200, AW_BLEND | (wParam ? 0 : AW_HIDE));
			SetWindowPos(cli.hwndContactTree, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		}
		break;

	case WM_MENURBUTTONUP: /* this API is so badly documented at MSDN!! */
		{
			UINT id = 0;

			id = GetMenuItemID((HMENU) lParam, LOWORD(wParam)); /* LOWORD(wParam) contains the menu pos in its parent menu */
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
				db_get_b(NULL, "CList", "Min2Tray", SETTING_MIN2TRAY_DEFAULT))
			{
				ShowWindow(hwnd, SW_HIDE);
				db_set_b(NULL, "CList", "State", SETTING_STATE_HIDDEN);

				if (db_get_b(NULL, "CList", "DisableWorkingSet", 1))
					SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);

				return 0;
			}
			else if (wParam == SC_CLOSE)
				wParam = SC_MINIMIZE;
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_COMMAND:
		if (CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_MAINMENU), (LPARAM) (HANDLE) NULL))
			break;

		switch (LOWORD(wParam)) {
		case ID_TRAY_EXIT:
		case ID_ICQ_EXIT:
			if (CallService(MS_SYSTEM_OKTOEXIT, 0, 0))
				DestroyWindow(hwnd);
			break;

		case ID_TRAY_HIDE:
			CallService(MS_CLIST_SHOWHIDE, 0, 0);
			break;

		case POPUP_NEWGROUP:
			SendMessage(cli.hwndContactTree, CLM_SETHIDEEMPTYGROUPS, 0, 0);
			CallService(MS_CLIST_GROUPCREATE, 0, 0);
			break;

		case POPUP_HIDEOFFLINE:
			CallService(MS_CLIST_SETHIDEOFFLINE, (WPARAM) (-1), 0);
			break;

		case POPUP_HIDEOFFLINEROOT:
			SendMessage(cli.hwndContactTree, CLM_SETHIDEOFFLINEROOT, !SendMessage(cli.hwndContactTree, CLM_GETHIDEOFFLINEROOT, 0, 0), 0);
			break;

		case POPUP_HIDEEMPTYGROUPS:
			{
				int newVal = !(GetWindowLongPtr(cli.hwndContactTree, GWL_STYLE) & CLS_HIDEEMPTYGROUPS);
				db_set_b(NULL, "CList", "HideEmptyGroups", (BYTE) newVal);
				SendMessage(cli.hwndContactTree, CLM_SETHIDEEMPTYGROUPS, newVal, 0);
			}
			break;

		case POPUP_DISABLEGROUPS:
			{
				int newVal = !(GetWindowLongPtr(cli.hwndContactTree, GWL_STYLE) & CLS_USEGROUPS);
				db_set_b(NULL, "CList", "UseGroups", (BYTE) newVal);
				SendMessage(cli.hwndContactTree, CLM_SETUSEGROUPS, newVal, 0);
			}
			break;

		case POPUP_HIDEMIRANDA:
			CallService(MS_CLIST_SHOWHIDE, 0, 0);
			break;
		}
		return FALSE;

	case WM_KEYDOWN:
		CallService(MS_CLIST_MENUPROCESSHOTKEY, wParam, MPCF_MAINMENU | MPCF_CONTACTMENU);
		break;

	case WM_GETMINMAXINFO:
		DefWindowProc(hwnd, msg, wParam, lParam);
		((LPMINMAXINFO) lParam)->ptMinTrackSize.x = 16 + GetSystemMetrics(SM_CXHTHUMB);
		((LPMINMAXINFO) lParam)->ptMinTrackSize.y = 16;
		return 0;

	case WM_SETTINGCHANGE:
		if (wParam == SPI_SETWORKAREA && (GetWindowLongPtr(hwnd, GWL_STYLE) & (WS_VISIBLE | WS_MINIMIZE)) == WS_VISIBLE &&
			!CallService(MS_CLIST_DOCKINGISDOCKED, 0, 0))
		{
			RECT rc;
			GetWindowRect(hwnd, &rc);
			if (Utils_AssertInsideScreen(&rc) == 1)
				MoveWindow(hwnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_DISPLAYCHANGE:
		DefWindowProc(hwnd, msg, wParam, lParam);
		SendMessage(cli.hwndContactTree, WM_SIZE, 0, 0); //forces it to send a cln_listsizechanged
		break;

		//MSG FROM CHILD CONTROL
	case WM_NOTIFY:
		if (((LPNMHDR) lParam)->hwndFrom == cli.hwndContactTree) {
			NMCLISTCONTROL *nmc = (NMCLISTCONTROL*)lParam;
			switch (((LPNMHDR) lParam)->code) {
			case CLN_EXPANDED:
				CallService(MS_CLIST_GROUPSETEXPANDED, (WPARAM) nmc->hItem, nmc->action);
				return FALSE;

			case CLN_DRAGGING:
				ClientToScreen(hwnd, &nmc->pt);
				if (!(nmc->flags & CLNF_ISGROUP))
					if (NotifyEventHooks(hContactDraggingEvent, (WPARAM) nmc->hItem, MAKELPARAM(nmc->pt.x, nmc->pt.y))) {
						SetCursor(LoadCursor(cli.hInst, MAKEINTRESOURCE(IDC_DROPUSER)));
						return TRUE;
					}
				break;

			case CLN_DRAGSTOP:
				if (!(nmc->flags & CLNF_ISGROUP))
					NotifyEventHooks(hContactDragStopEvent, (WPARAM) nmc->hItem, 0);
				break;

			case CLN_DROPPED:
				ClientToScreen(hwnd, &nmc->pt);
				if (!(nmc->flags & CLNF_ISGROUP))
					if (NotifyEventHooks(hContactDroppedEvent, (WPARAM) nmc->hItem, MAKELPARAM(nmc->pt.x, nmc->pt.y))) {
						SetCursor(LoadCursor(cli.hInst, MAKEINTRESOURCE(IDC_DROPUSER)));
						return TRUE;
					}
				break;

			case CLN_NEWCONTACT:
				if (nmc != NULL)
					cli.pfnSetAllExtraIcons((MCONTACT)nmc->hItem);
				return TRUE;

			case CLN_LISTREBUILT:
				cli.pfnSetAllExtraIcons(NULL);
				return(FALSE);

			case NM_KEYDOWN:
				return CallService(MS_CLIST_MENUPROCESSHOTKEY, ((NMKEY*)lParam)->nVKey, MPCF_MAINMENU | MPCF_CONTACTMENU);

			case CLN_LISTSIZECHANGE:
				{
					RECT rcWindow, rcTree, rcWorkArea;
					int maxHeight, newHeight;

					if (!db_get_b(NULL, "CLUI", "AutoSize", 0))
						break;
					if (CallService(MS_CLIST_DOCKINGISDOCKED, 0, 0))
						break;
					maxHeight = db_get_b(NULL, "CLUI", "MaxSizeHeight", 75);
					GetWindowRect(hwnd, &rcWindow);
					GetWindowRect(cli.hwndContactTree, &rcTree);

					SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, FALSE);
					HMONITOR hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
					MONITORINFO mi;
					mi.cbSize = sizeof(mi);
					if (GetMonitorInfo(hMon, &mi))
						rcWorkArea = mi.rcWork;

					newHeight = max(nmc->pt.y, 9) + 1 + (rcWindow.bottom - rcWindow.top) - (rcTree.bottom - rcTree.top);
					if (newHeight > (rcWorkArea.bottom - rcWorkArea.top) * maxHeight / 100)
						newHeight = (rcWorkArea.bottom - rcWorkArea.top) * maxHeight / 100;
					if (db_get_b(NULL, "CLUI", "AutoSizeUpward", 0)) {
						rcWindow.top = rcWindow.bottom - newHeight;
						if (rcWindow.top < rcWorkArea.top)
							rcWindow.top = rcWorkArea.top;
					}
					else {
						rcWindow.bottom = rcWindow.top + newHeight;
						if (rcWindow.bottom > rcWorkArea.bottom)
							rcWindow.bottom = rcWorkArea.bottom;
					}
					SetWindowPos(hwnd, 0, rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top,
						SWP_NOZORDER | SWP_NOACTIVATE);
					break;
				}
			case NM_CLICK:
				{
					DWORD hitFlags;
					HANDLE hItem = (HANDLE)SendMessage(cli.hwndContactTree, CLM_HITTEST, (WPARAM)&hitFlags, MAKELPARAM(nmc->pt.x, nmc->pt.y));
					if (hItem) {
						if (hitFlags & CLCHT_ONITEMEXTRA) {
							if (!IsHContactGroup(hItem) && !IsHContactInfo(hItem))
								if (cli.pfnGetCacheEntry((MCONTACT)nmc->hItem))
									NotifyEventHooks(hEventExtraClick, (WPARAM)nmc->hItem, nmc->iColumn+1);
						}
						break;
					}

					if ((hitFlags & (CLCHT_NOWHERE | CLCHT_INLEFTMARGIN | CLCHT_BELOWITEMS)) == 0)
						break;

					if (db_get_b(NULL, "CLUI", "ClientAreaDrag", SETTING_CLIENTDRAG_DEFAULT)) {
						POINT pt = nmc->pt;
						ClientToScreen(cli.hwndContactTree, &pt);
						return SendMessage(hwnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, MAKELPARAM(pt.x, pt.y));
					}
				}
				break;
			}
		}
		else if (((LPNMHDR) lParam)->hwndFrom == cli.hwndStatus) {
			if (((LPNMHDR) lParam)->code == NM_CLICK) {
				unsigned int nParts, nPanel;
				NMMOUSE *nm = (NMMOUSE *) lParam;
				HMENU hMenu;
				RECT rc;
				POINT pt;

				hMenu = (HMENU) CallService(MS_CLIST_MENUGETSTATUS, 0, 0);
				nParts = SendMessage(cli.hwndStatus, SB_GETPARTS, 0, 0);
				if (nm->dwItemSpec == 0xFFFFFFFE) {
					nPanel = nParts - 1;
					SendMessage(cli.hwndStatus, SB_GETRECT, nPanel, (LPARAM) & rc);
					if (nm->pt.x < rc.left)
						return FALSE;
				}
				else nPanel = nm->dwItemSpec;

				if (nParts > 0) {
					unsigned int cpnl = 0;
					int mcnt = GetMenuItemCount(hMenu);
					for (int i=0; i<mcnt; i++) {
						HMENU hMenus = GetSubMenu(hMenu, i);
						if (hMenus && cpnl++ == nPanel) {
							hMenu = hMenus;
							break;
						}
					}
				}
				SendMessage(cli.hwndStatus, SB_GETRECT, nPanel, (LPARAM) & rc);
				pt.x = rc.left;
				pt.y = rc.top;
				ClientToScreen(cli.hwndStatus, &pt);
				TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
			}
		}
		return FALSE;

	case WM_MENUSELECT:
		if (lParam && (HMENU)lParam == cli.hMenuMain) {
			int pos = LOWORD(wParam);
			POINT pt;
			GetCursorPos(&pt);
			if ((pos == 0 || pos == 1) && (HIWORD(wParam) & MF_POPUP) && (!(HIWORD(wParam) & MF_MOUSESELECT) || MenuItemFromPoint(hwnd, cli.hMenuMain, pt) != -1)) {
				MENUITEMINFO mii = { sizeof(mii) };
				mii.fMask = MIIM_SUBMENU;
				mii.hSubMenu = (HMENU)CallService((pos == 0) ? MS_CLIST_MENUGETMAIN : MS_CLIST_MENUGETSTATUS, 0, 0);
				SetMenuItemInfo(cli.hMenuMain, pos, TRUE, &mii);
			}
		}
		break;

	case WM_CONTEXTMENU:
		{
			// x/y might be -1 if it was generated by a kb click
			POINT pt;
			pt.x = (short) LOWORD(lParam);
			pt.y = (short) HIWORD(lParam);

			RECT rc;
			GetWindowRect(cli.hwndContactTree, &rc);
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
				HMENU hMenu;
				hMenu = GetSubMenu(LoadMenu(cli.hInst, MAKEINTRESOURCE(IDR_CONTEXT)), 1);
				TranslateMenu(hMenu);
				CheckMenuItem(hMenu, POPUP_HIDEOFFLINE,
					db_get_b(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT) ? MF_CHECKED : MF_UNCHECKED);
				CheckMenuItem(hMenu, POPUP_HIDEOFFLINEROOT, SendMessage(cli.hwndContactTree, CLM_GETHIDEOFFLINEROOT, 0, 0) ? MF_CHECKED : MF_UNCHECKED);
				CheckMenuItem(hMenu, POPUP_HIDEEMPTYGROUPS,
					GetWindowLongPtr(cli.hwndContactTree, GWL_STYLE) & CLS_HIDEEMPTYGROUPS ? MF_CHECKED : MF_UNCHECKED);
				CheckMenuItem(hMenu, POPUP_DISABLEGROUPS, GetWindowLongPtr(cli.hwndContactTree, GWL_STYLE) & CLS_USEGROUPS ? MF_UNCHECKED : MF_CHECKED);
				TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
				DestroyMenu(hMenu);
				return 0;
			}
			GetWindowRect(cli.hwndStatus, &rc);
			if (PtInRect(&rc, pt)) {
				HMENU hMenu;
				if (db_get_b(NULL, "CLUI", "SBarRightClk", 0))
					hMenu = (HMENU) CallService(MS_CLIST_MENUGETMAIN, 0, 0);
				else
					hMenu = (HMENU) CallService(MS_CLIST_MENUGETSTATUS, 0, 0);
				TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
				return 0;
			}
		}
		break;

	case WM_MEASUREITEM:
		if (((LPMEASUREITEMSTRUCT) lParam)->itemData == MENU_MIRANDAMENU) {
			((LPMEASUREITEMSTRUCT) lParam)->itemWidth = g_IconWidth * 4 / 3;
			((LPMEASUREITEMSTRUCT) lParam)->itemHeight = 0;
			return TRUE;
		}
		return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT) lParam;
			if (dis->hwndItem == cli.hwndStatus) {
				char *szProto = (char *) dis->itemData;
				if (szProto == NULL) return 0;
				int status, x;
				SIZE textSize;
				BYTE showOpts = db_get_b(NULL, "CLUI", "SBarShow", 1);
				status = CallProtoServiceInt(NULL,szProto, PS_GETSTATUS, 0, 0);
				SetBkMode(dis->hDC, TRANSPARENT);
				x = dis->rcItem.left;
				if (showOpts & 1) {
					HICON hIcon = LoadSkinProtoIcon(szProto, status);
					DrawIconEx(dis->hDC, x, (dis->rcItem.top + dis->rcItem.bottom - g_IconHeight) >> 1, hIcon,
						g_IconWidth, g_IconHeight, 0, NULL, DI_NORMAL);
					IcoLib_ReleaseIcon(hIcon, 0);
					if (Proto_IsAccountLocked(Proto_GetAccount(szProto))) {
						hIcon = LoadSkinnedIcon(SKINICON_OTHER_STATUS_LOCKED);
						if (hIcon != NULL) {
							DrawIconEx(dis->hDC, x, (dis->rcItem.top + dis->rcItem.bottom - g_IconHeight) >> 1, hIcon,
								g_IconWidth, g_IconHeight, 0, NULL, DI_NORMAL);
							IcoLib_ReleaseIcon(hIcon, 0);
						}

					}
					x += g_IconWidth + 2;
				}
				else
					x += 2;
				if (showOpts & 2) {
					PROTOACCOUNT *pa;
					TCHAR tszName[64];
					if ((pa = Proto_GetAccount(szProto)) != NULL)
						mir_sntprintf(tszName, SIZEOF(tszName), _T("%s "), pa->tszAccountName);
					else
						tszName[0] = 0;

					GetTextExtentPoint32(dis->hDC, tszName, mir_tstrlen(tszName), &textSize);
					TextOut(dis->hDC, x, (dis->rcItem.top + dis->rcItem.bottom - textSize.cy) >> 1, tszName, mir_tstrlen(tszName));
					x += textSize.cx;
				}
				if (showOpts & 4) {
					TCHAR* szStatus = cli.pfnGetStatusModeDescription(status, 0);
					if (!szStatus)
						szStatus = _T("");
					GetTextExtentPoint32(dis->hDC, szStatus, mir_tstrlen(szStatus), &textSize);
					TextOut(dis->hDC, x, (dis->rcItem.top + dis->rcItem.bottom - textSize.cy) >> 1, szStatus, mir_tstrlen(szStatus));
				}
			}
			else if (dis->CtlType == ODT_MENU) {
				if (dis->itemData == MENU_MIRANDAMENU) {
					HICON hIcon = LoadSkinnedIcon(SKINICON_OTHER_MAINMENU);
					fnDrawMenuItem(dis, CopyIcon(hIcon), NULL);
					IcoLib_ReleaseIcon(hIcon, NULL);
					return TRUE;
				}
				return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
			}
		}
		return 0;

	case WM_CLOSE:
		if (CallService(MS_SYSTEM_OKTOEXIT, 0, 0))
			DestroyWindow(hwnd);
		return FALSE;

	case WM_DESTROY:
		if (!IsIconic(hwnd)) {
			RECT rc;
			GetWindowRect(hwnd, &rc);

			//if docked, dont remember pos (except for width)
			if (!CallService(MS_CLIST_DOCKINGISDOCKED, 0, 0)) {
				db_set_dw(NULL, "CList", "Height", (DWORD) (rc.bottom - rc.top));
				db_set_dw(NULL, "CList", "x", (DWORD) rc.left);
				db_set_dw(NULL, "CList", "y", (DWORD) rc.top);
			}
			db_set_dw(NULL, "CList", "Width", (DWORD) (rc.right - rc.left));
		}

		RemoveMenu(cli.hMenuMain, 0, MF_BYPOSITION);
		RemoveMenu(cli.hMenuMain, 0, MF_BYPOSITION);

		if (cli.hwndStatus) {
			DestroyWindow(cli.hwndStatus);
			cli.hwndStatus = NULL;
		}

		// Disconnect all protocols
		DisconnectAll();

		ShowWindow(hwnd, SW_HIDE);
		DestroyWindow(cli.hwndContactTree);
		FreeLibrary(hUserDll);
		PostQuitMessage(0);

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return TRUE;
}
