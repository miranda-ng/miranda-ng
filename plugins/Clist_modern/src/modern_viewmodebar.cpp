/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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

contact list view modes (CLVM)

$Id: viewmodes.c 2998 2006-06-01 07:11:52Z nightwish2004 $

*/

#include "hdr/modern_commonheaders.h"
#include "CLUIFrames/cluiframes.h"
#include "m_skinbutton.h"

#define TIMERID_VIEWMODEEXPIRE 100

typedef int(__cdecl *pfnEnumCallback)(char *szName);
static HWND clvmHwnd = 0;
static int clvm_curItem = 0;
HMENU hViewModeMenu = 0;

static HWND hwndSelector = 0;
static HIMAGELIST himlViewModes = 0;
static HANDLE hInfoItem = 0;
static int nullImage;
static DWORD stickyStatusMask = 0;
static char g_szModename[2048];

static int g_ViewModeOptDlg = FALSE;

static UINT _page1Controls[] = { IDC_STATIC1, IDC_STATIC2, IDC_STATIC3, IDC_STATIC5, IDC_STATIC4,
IDC_STATIC8, IDC_ADDVIEWMODE, IDC_DELETEVIEWMODE, IDC_NEWVIEMODE, IDC_GROUPS, IDC_PROTOCOLS,
IDC_VIEWMODES, IDC_STATUSMODES, IDC_STATIC12, IDC_STATIC13, IDC_STATIC14, IDC_PROTOGROUPOP, IDC_GROUPSTATUSOP,
IDC_AUTOCLEAR, IDC_AUTOCLEARVAL, IDC_AUTOCLEARSPIN, IDC_STATIC15, IDC_STATIC16,
IDC_LASTMESSAGEOP, IDC_LASTMESSAGEUNIT, IDC_LASTMSG, IDC_LASTMSGVALUE, IDC_USEGROUPS, 0 };

static UINT _page2Controls[] = { IDC_CLIST, IDC_STATIC9, IDC_STATIC8, IDC_CLEARALL, IDC_CURVIEWMODE2, 0 };

static UINT _buttons[] = { IDC_RESETMODES, IDC_SELECTMODE, IDC_CONFIGUREMODES, 0 };

static BOOL sttDrawViewModeBackground(HWND hwnd, HDC hdc, RECT *rect);
static void DeleteViewMode(char * szName);

static int DrawViewModeBar(HWND hWnd, HDC hDC)
{
	RECT rc;
	GetClientRect(hWnd, &rc);
	SkinDrawGlyph(hDC, &rc, &rc, "ViewMode,ID=Background");
	return 0;
}

static int ViewModePaintCallbackProc(HWND hWnd, HDC hDC, RECT *, HRGN, DWORD, void *)
{
	RECT MyRect = { 0 };
	GetWindowRect(hWnd, &MyRect);
	DrawViewModeBar(hWnd, hDC);
	for (int i = 0; _buttons[i] != 0; i++) {
		RECT childRect;
		GetWindowRect(GetDlgItem(hWnd, _buttons[i]), &childRect);

		POINT Offset;
		Offset.x = childRect.left - MyRect.left;
		Offset.y = childRect.top - MyRect.top;
		SendDlgItemMessage(hWnd, _buttons[i], BUTTONDRAWINPARENT, (WPARAM)hDC, (LPARAM)&Offset);

	}
	return 0;
}

/*
 * enumerate all view modes, call the callback function with the mode name
 * useful for filling lists, menus and so on..
 */

int CLVM_EnumProc(const char *szSetting, LPARAM lParam)
{
	pfnEnumCallback EnumCallback = (pfnEnumCallback)lParam;
	if (szSetting != NULL)
		EnumCallback((char *)szSetting);
	return 0;
}

void CLVM_EnumModes(pfnEnumCallback EnumCallback)
{
	DBCONTACTENUMSETTINGS dbces;
	dbces.pfnEnumProc = CLVM_EnumProc;
	dbces.szModule = CLVM_MODULE;
	dbces.ofsSettings = 0;
	dbces.lParam = (LPARAM)EnumCallback;
	CallService(MS_DB_CONTACT_ENUMSETTINGS, 0, (LPARAM)&dbces);
}

int FillModes(char *szsetting)
{
	if (BYTE(szsetting[0]) == 246)
		return 1;
	if (szsetting[0] == 13)
		return 1;

	ptrT temp(mir_utf8decodeT(szsetting));
	if (temp != NULL)
		SendDlgItemMessage(clvmHwnd, IDC_VIEWMODES, LB_INSERTSTRING, -1, (LPARAM)temp);
	return 1;
}

static void ShowPage(HWND hwnd, int page)
{
	int i = 0;
	int pageChange = 0;

	if (page == 0 && IsWindowVisible(GetDlgItem(hwnd, _page2Controls[0])))
		pageChange = 1;

	if (page == 1 && IsWindowVisible(GetDlgItem(hwnd, _page1Controls[0])))
		pageChange = 1;

	if (pageChange)
		SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);

	switch (page) {
	case 0:
		while (_page1Controls[i] != 0)
			ShowWindow(GetDlgItem(hwnd, _page1Controls[i++]), SW_SHOW);
		i = 0;
		while (_page2Controls[i] != 0)
			ShowWindow(GetDlgItem(hwnd, _page2Controls[i++]), SW_HIDE);
		break;
	case 1:
		while (_page1Controls[i] != 0)
			ShowWindow(GetDlgItem(hwnd, _page1Controls[i++]), SW_HIDE);
		i = 0;
		while (_page2Controls[i] != 0)
			ShowWindow(GetDlgItem(hwnd, _page2Controls[i++]), SW_SHOW);
		break;
	}
	if (pageChange) {
		SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
		RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
	}
}

static int UpdateClistItem(MCONTACT hContact, DWORD mask)
{
	for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++)
		SendDlgItemMessage(clvmHwnd, IDC_CLIST, CLM_SETEXTRAIMAGE, hContact, MAKELONG(i - ID_STATUS_OFFLINE,
		(1 << (i - ID_STATUS_OFFLINE)) & mask ? i - ID_STATUS_OFFLINE : nullImage));

	return 0;
}

static DWORD GetMaskForItem(HANDLE hItem)
{
	DWORD dwMask = 0;

	for (int i = 0; i <= ID_STATUS_OUTTOLUNCH - ID_STATUS_OFFLINE; i++)
		dwMask |= (SendDlgItemMessage(clvmHwnd, IDC_CLIST, CLM_GETEXTRAIMAGE, (WPARAM)hItem, i) == nullImage ? 0 : 1 << i);

	return dwMask;
}

static void UpdateStickies()
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		MCONTACT hItem = (MCONTACT)SendDlgItemMessage(clvmHwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
		if (hItem)
			SendDlgItemMessage(clvmHwnd, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItem, (BYTE)db_get_dw(hContact, CLVM_MODULE, g_szModename, 0) ? 1 : 0);

		DWORD localMask = HIWORD(db_get_dw(hContact, CLVM_MODULE, g_szModename, 0));
		UpdateClistItem(hItem, (localMask == 0 || localMask == stickyStatusMask) ? stickyStatusMask : localMask);
	}

	for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++)
		SendDlgItemMessage(clvmHwnd, IDC_CLIST, CLM_SETEXTRAIMAGE, (WPARAM)hInfoItem, MAKELONG(i - ID_STATUS_OFFLINE, (1 << (i - ID_STATUS_OFFLINE)) & stickyStatusMask ? i - ID_STATUS_OFFLINE : ID_STATUS_OUTTOLUNCH - ID_STATUS_OFFLINE + 1));

	HANDLE hItem = (HANDLE)SendDlgItemMessage(clvmHwnd, IDC_CLIST, CLM_GETNEXTITEM, CLGN_ROOT, 0);
	hItem = (HANDLE)SendDlgItemMessage(clvmHwnd, IDC_CLIST, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hItem);
	while (hItem) {
		for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++)
			SendDlgItemMessage(clvmHwnd, IDC_CLIST, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELONG(i - ID_STATUS_OFFLINE, nullImage));
		hItem = (HANDLE)SendDlgItemMessage(clvmHwnd, IDC_CLIST, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hItem);
	}
	ShowPage(clvmHwnd, 0);
}

static int FillDialog(HWND hwnd)
{
	LVCOLUMN lvc = { 0 };
	HWND hwndList = GetDlgItem(hwnd, IDC_PROTOCOLS);
	LVITEMA item = { 0 };
	int protoCount = 0, i, newItem;
	PROTOACCOUNT **accs;

	CLVM_EnumModes(FillModes);
	ListView_SetExtendedListViewStyle(GetDlgItem(hwnd, IDC_PROTOCOLS), LVS_EX_CHECKBOXES);
	lvc.mask = LVCF_FMT;
	lvc.fmt = LVCFMT_IMAGE | LVCFMT_LEFT;
	ListView_InsertColumn(GetDlgItem(hwnd, IDC_PROTOCOLS), 0, &lvc);

	// fill protocols...
	ProtoEnumAccounts(&protoCount, &accs);

	item.mask = LVIF_TEXT;
	item.iItem = 1000;
	for (i = 0; i < protoCount; i++) {
		item.pszText = accs[i]->szModuleName;
		newItem = SendMessageA(hwndList, LVM_INSERTITEMA, 0, (LPARAM)&item);
	}

	ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);
	ListView_Arrange(hwndList, LVA_ALIGNLEFT | LVA_ALIGNTOP);

	// fill groups
	{
		hwndList = GetDlgItem(hwnd, IDC_GROUPS);

		ListView_SetExtendedListViewStyle(hwndList, LVS_EX_CHECKBOXES);
		lvc.mask = LVCF_FMT;
		lvc.fmt = LVCFMT_IMAGE | LVCFMT_LEFT;
		ListView_InsertColumn(hwndList, 0, &lvc);

		LVITEM item = { 0 };
		item.mask = LVIF_TEXT;
		item.iItem = 1000;

		item.pszText = TranslateT("Ungrouped contacts");
		newItem = SendMessage(hwndList, LVM_INSERTITEM, 0, (LPARAM)&item);

		TCHAR *szGroup;
		for (int i = 1; (szGroup = pcli->pfnGetGroupName(i, NULL)) != NULL; i++) {
			item.pszText = szGroup;
			newItem = SendMessage(hwndList, LVM_INSERTITEM, 0, (LPARAM)&item);
		}
		ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);
		ListView_Arrange(hwndList, LVA_ALIGNLEFT | LVA_ALIGNTOP);
	}
	hwndList = GetDlgItem(hwnd, IDC_STATUSMODES);
	ListView_SetExtendedListViewStyle(hwndList, LVS_EX_CHECKBOXES);
	lvc.mask = LVCF_FMT;
	lvc.fmt = LVCFMT_IMAGE | LVCFMT_LEFT;
	ListView_InsertColumn(hwndList, 0, &lvc);
	for (i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
		LVITEM item = { 0 };
		item.mask = LVIF_TEXT;
		item.pszText = pcli->pfnGetStatusModeDescription(i, 0);
		item.iItem = i - ID_STATUS_OFFLINE;
		newItem = SendMessage(hwndList, LVM_INSERTITEM, 0, (LPARAM)&item);
	}
	ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);
	ListView_Arrange(hwndList, LVA_ALIGNLEFT | LVA_ALIGNTOP);

	SendDlgItemMessage(hwnd, IDC_PROTOGROUPOP, CB_INSERTSTRING, -1, (LPARAM)TranslateT("And"));
	SendDlgItemMessage(hwnd, IDC_PROTOGROUPOP, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Or"));
	SendDlgItemMessage(hwnd, IDC_GROUPSTATUSOP, CB_INSERTSTRING, -1, (LPARAM)TranslateT("And"));
	SendDlgItemMessage(hwnd, IDC_GROUPSTATUSOP, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Or"));

	SendDlgItemMessage(hwnd, IDC_LASTMESSAGEOP, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Older than"));
	SendDlgItemMessage(hwnd, IDC_LASTMESSAGEOP, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Newer than"));

	SendDlgItemMessage(hwnd, IDC_LASTMESSAGEUNIT, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Minutes"));
	SendDlgItemMessage(hwnd, IDC_LASTMESSAGEUNIT, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Hours"));
	SendDlgItemMessage(hwnd, IDC_LASTMESSAGEUNIT, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Days"));
	SendDlgItemMessage(hwnd, IDC_LASTMESSAGEOP, CB_SETCURSEL, 0, 0);
	SendDlgItemMessage(hwnd, IDC_LASTMESSAGEUNIT, CB_SETCURSEL, 0, 0);
	SetDlgItemInt(hwnd, IDC_LASTMSGVALUE, 0, 0);
	return 0;
}

static void SetAllChildIcons(HWND hwndList, HANDLE hFirstItem, int iColumn, int iImage)
{
	int typeOfFirst = SendMessage(hwndList, CLM_GETITEMTYPE, (WPARAM)hFirstItem, 0);

	// check groups
	HANDLE hItem = (typeOfFirst == CLCIT_GROUP) ? hFirstItem : (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hFirstItem);
	while (hItem) {
		HANDLE hChildItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
		if (hChildItem)
			SetAllChildIcons(hwndList, hChildItem, iColumn, iImage);
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hItem);
	}

	// check contacts
	if (typeOfFirst == CLCIT_CONTACT) hItem = hFirstItem;
	else hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, (LPARAM)hFirstItem);
	while (hItem) {
		int iOldIcon = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, iColumn);
		if (iOldIcon != EMPTY_EXTRA_ICON && iOldIcon != iImage)
			SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(iColumn, iImage));
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, (LPARAM)hItem);
	}
}

static void SetIconsForColumn(HWND hwndList, HANDLE hItem, HANDLE hItemAll, int iColumn, int iImage)
{
	int itemType = SendMessage(hwndList, CLM_GETITEMTYPE, (WPARAM)hItem, 0);
	if (itemType == CLCIT_CONTACT) {
		int oldiImage = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, iColumn);
		if (oldiImage != EMPTY_EXTRA_ICON && oldiImage != iImage)
			SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(iColumn, iImage));
	}
	else if (itemType == CLCIT_INFO) {
		int oldiImage = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, iColumn);
		if (oldiImage != EMPTY_EXTRA_ICON && oldiImage != iImage)
			SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(iColumn, iImage));
		if (hItem == hItemAll)
			SetAllChildIcons(hwndList, hItem, iColumn, iImage);
		else
			SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(iColumn, iImage)); //hItemUnknown
	}
	else if (itemType == CLCIT_GROUP) {
		int oldiImage = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, iColumn);
		if (oldiImage != EMPTY_EXTRA_ICON && oldiImage != iImage)
			SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(iColumn, iImage));
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
		if (hItem)
			SetAllChildIcons(hwndList, hItem, iColumn, iImage);
	}
}

static int DeleteAutoModesCallback(char *szsetting)
{
	if (szsetting[0] == 13)
		DeleteViewMode(szsetting);
	return 1;
}


void SaveViewMode(const char *name, const TCHAR *szGroupFilter, const char *szProtoFilter, DWORD statusMask, DWORD stickyStatusMask, unsigned int options,
	unsigned int stickies, unsigned int operators, unsigned int lmdat)
{
	CLVM_EnumModes(DeleteAutoModesCallback);

	char szSetting[512];
	mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_PF", 246, name);
	db_set_s(NULL, CLVM_MODULE, szSetting, szProtoFilter);
	mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_GF", 246, name);
	db_set_ws(NULL, CLVM_MODULE, szSetting, szGroupFilter);
	mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_SM", 246, name);
	db_set_dw(NULL, CLVM_MODULE, szSetting, statusMask);
	mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_SSM", 246, name);
	db_set_dw(NULL, CLVM_MODULE, szSetting, stickyStatusMask);
	mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_OPT", 246, name);
	db_set_dw(NULL, CLVM_MODULE, szSetting, options);
	mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_LM", 246, name);
	db_set_dw(NULL, CLVM_MODULE, szSetting, lmdat);

	db_set_dw(NULL, CLVM_MODULE, name, MAKELONG((unsigned short)operators, (unsigned short)stickies));
}

/*
 * saves the state of the filter definitions for the current item
 */

void SaveState()
{
	TCHAR newGroupFilter[2048] = _T("|");
	char newProtoFilter[2048] = "|";
	DWORD statusMask = 0;
	DWORD operators = 0;

	if (clvm_curItem == -1)
		return;

	{
		LVITEMA item = { 0 };
		char szTemp[256];

		HWND hwndList = GetDlgItem(clvmHwnd, IDC_PROTOCOLS);
		for (int i = 0; i < ListView_GetItemCount(hwndList); i++) {
			if (ListView_GetCheckState(hwndList, i)) {
				item.mask = LVIF_TEXT;
				item.pszText = szTemp;
				item.cchTextMax = SIZEOF(szTemp);
				item.iItem = i;
				SendMessageA(hwndList, LVM_GETITEMA, 0, (LPARAM)&item);
				mir_strncat(newProtoFilter, szTemp, _TRUNCATE);
				mir_strncat(newProtoFilter, "|", _TRUNCATE);
				newProtoFilter[2047] = 0;
			}
		}
	}
	{
		LVITEM item = { 0 };
		TCHAR szTemp[256];

		HWND hwndList = GetDlgItem(clvmHwnd, IDC_GROUPS);

		operators |= ListView_GetCheckState(hwndList, 0) ? CLVM_INCLUDED_UNGROUPED : 0;

		for (int i = 0; i < ListView_GetItemCount(hwndList); i++) {
			if (ListView_GetCheckState(hwndList, i)) {
				item.mask = LVIF_TEXT;
				item.pszText = szTemp;
				item.cchTextMax = SIZEOF(szTemp);
				item.iItem = i;
				SendMessage(hwndList, LVM_GETITEM, 0, (LPARAM)&item);
				mir_tstrncat(newGroupFilter, szTemp, _TRUNCATE);
				mir_tstrncat(newGroupFilter, _T("|"), _TRUNCATE);
				newGroupFilter[2047] = 0;
			}
		}
	}
	{
		HWND hwndList = GetDlgItem(clvmHwnd, IDC_STATUSMODES);
		for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++)
			if (ListView_GetCheckState(hwndList, i - ID_STATUS_OFFLINE))
				statusMask |= (1 << (i - ID_STATUS_OFFLINE));
	}

	int iLen = SendDlgItemMessage(clvmHwnd, IDC_VIEWMODES, LB_GETTEXTLEN, clvm_curItem, 0);
	if (iLen) {
		unsigned int stickies = 0;

		TCHAR *szTempModeName = (TCHAR*)mir_alloc((iLen + 1)*sizeof(TCHAR));
		if (szTempModeName) {
			SendDlgItemMessage(clvmHwnd, IDC_VIEWMODES, LB_GETTEXT, clvm_curItem, (LPARAM)szTempModeName);

			ptrA szModeName(mir_utf8encodeT(szTempModeName));

			DWORD dwGlobalMask = GetMaskForItem(hInfoItem);
			for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
				HANDLE hItem = (HANDLE)SendDlgItemMessage(clvmHwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
				if (hItem == NULL)
					continue;

				if (SendDlgItemMessage(clvmHwnd, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItem, 0)) {
					DWORD dwLocalMask = GetMaskForItem(hItem);
					db_set_dw(hContact, CLVM_MODULE, szModeName, MAKELONG(1, (unsigned short)dwLocalMask));
					stickies++;
				}
				else {
					if (db_get_dw(hContact, CLVM_MODULE, szModeName, 0))
						db_set_dw(hContact, CLVM_MODULE, szModeName, 0);
				}
			}

			operators |= ((SendDlgItemMessage(clvmHwnd, IDC_PROTOGROUPOP, CB_GETCURSEL, 0, 0) == 1 ? CLVM_PROTOGROUP_OP : 0) |
				(SendDlgItemMessage(clvmHwnd, IDC_GROUPSTATUSOP, CB_GETCURSEL, 0, 0) == 1 ? CLVM_GROUPSTATUS_OP : 0) |
				(IsDlgButtonChecked(clvmHwnd, IDC_AUTOCLEAR) ? CLVM_AUTOCLEAR : 0) |
				(IsDlgButtonChecked(clvmHwnd, IDC_LASTMSG) ? CLVM_USELASTMSG : 0) |
				(IsDlgButtonChecked(clvmHwnd, IDC_USEGROUPS) == BST_CHECKED ? CLVM_USEGROUPS : 0) |
				(IsDlgButtonChecked(clvmHwnd, IDC_USEGROUPS) == BST_UNCHECKED ? CLVM_DONOTUSEGROUPS : 0)
				);

			DWORD options = SendDlgItemMessage(clvmHwnd, IDC_AUTOCLEARSPIN, UDM_GETPOS, 0, 0);

			BOOL translated;
			DWORD lmdat = MAKELONG(GetDlgItemInt(clvmHwnd, IDC_LASTMSGVALUE, &translated, FALSE),
				MAKEWORD(SendDlgItemMessage(clvmHwnd, IDC_LASTMESSAGEOP, CB_GETCURSEL, 0, 0),
				SendDlgItemMessage(clvmHwnd, IDC_LASTMESSAGEUNIT, CB_GETCURSEL, 0, 0)));

			SaveViewMode(szModeName, newGroupFilter, newProtoFilter, statusMask, dwGlobalMask, options,
				stickies, operators, lmdat);

			mir_free(szTempModeName);
			szTempModeName = NULL;
		}
	}
	EnableWindow(GetDlgItem(clvmHwnd, IDC_APPLY), FALSE);
}

/*
 * updates the filter list boxes with the data taken from the filtering string
 */

static void UpdateFilters()
{
	DBVARIANT dbv_pf = { 0 };
	DBVARIANT dbv_gf = { 0 };
	char szSetting[128];
	TCHAR *szTempBuf = NULL;
	char *szBuf = NULL;
	int iLen;
	DWORD statusMask = 0;
	DWORD dwFlags;
	DWORD opt;
	TCHAR szTemp[100];

	if (clvm_curItem == LB_ERR)
		return;

	iLen = SendDlgItemMessageA(clvmHwnd, IDC_VIEWMODES, LB_GETTEXTLEN, clvm_curItem, 0);

	if (iLen == 0)
		return;

	szTempBuf = (TCHAR *)mir_alloc((iLen + 1)*sizeof(TCHAR));
	SendDlgItemMessage(clvmHwnd, IDC_VIEWMODES, LB_GETTEXT, clvm_curItem, (LPARAM)szTempBuf);

	szBuf = mir_utf8encodeT(szTempBuf);
	mir_strncpy(g_szModename, szBuf, SIZEOF(g_szModename));
	g_szModename[SIZEOF(g_szModename) - 1] = 0;
	mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("Configuring view mode: %s"), szTempBuf);
	SetDlgItemText(clvmHwnd, IDC_CURVIEWMODE2, szTemp);
	mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_PF", 246, szBuf);
	if (db_get_s(NULL, CLVM_MODULE, szSetting, &dbv_pf))
		goto cleanup;
	mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_GF", 246, szBuf);
	if (db_get_ts(NULL, CLVM_MODULE, szSetting, &dbv_gf))
		goto cleanup;
	mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_OPT", 246, szBuf);
	if ((opt = db_get_dw(NULL, CLVM_MODULE, szSetting, -1)) != -1) {
		SendDlgItemMessage(clvmHwnd, IDC_AUTOCLEARSPIN, UDM_SETPOS, 0, MAKELONG(LOWORD(opt), 0));
	}
	mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_SM", 246, szBuf);
	statusMask = db_get_dw(NULL, CLVM_MODULE, szSetting, 0);
	mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_SSM", 246, szBuf);
	stickyStatusMask = db_get_dw(NULL, CLVM_MODULE, szSetting, -1);
	dwFlags = db_get_dw(NULL, CLVM_MODULE, szBuf, 0);
	{
		LVITEMA item = { 0 };
		char szTemp[256];
		char szMask[256];
		int i;
		HWND hwndList = GetDlgItem(clvmHwnd, IDC_PROTOCOLS);

		item.mask = LVIF_TEXT;
		item.pszText = szTemp;
		item.cchTextMax = SIZEOF(szTemp);

		for (i = 0; i < ListView_GetItemCount(hwndList); i++) {
			item.iItem = i;
			SendMessageA(hwndList, LVM_GETITEMA, 0, (LPARAM)&item);
			mir_snprintf(szMask, SIZEOF(szMask), "%s|", szTemp);
			if (dbv_pf.pszVal && strstr(dbv_pf.pszVal, szMask))
				ListView_SetCheckState(hwndList, i, TRUE)
			else
			ListView_SetCheckState(hwndList, i, FALSE);
		}
	}
	{
		LVITEM item = { 0 };
		TCHAR szTemp[256];
		TCHAR szMask[256];
		int i;
		HWND hwndList = GetDlgItem(clvmHwnd, IDC_GROUPS);

		item.mask = LVIF_TEXT;
		item.pszText = szTemp;
		item.cchTextMax = SIZEOF(szTemp);

		ListView_SetCheckState(hwndList, 0, dwFlags & CLVM_INCLUDED_UNGROUPED ? TRUE : FALSE);

		for (i = 1; i < ListView_GetItemCount(hwndList); i++) {
			item.iItem = i;
			SendMessage(hwndList, LVM_GETITEM, 0, (LPARAM)&item);
			mir_sntprintf(szMask, SIZEOF(szMask), _T("%s|"), szTemp);
			if (dbv_gf.ptszVal && _tcsstr(dbv_gf.ptszVal, szMask))
				ListView_SetCheckState(hwndList, i, TRUE)
			else
			ListView_SetCheckState(hwndList, i, FALSE);
		}
	}
	{
		HWND hwndList = GetDlgItem(clvmHwnd, IDC_STATUSMODES);

		for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
			if ((1 << (i - ID_STATUS_OFFLINE)) & statusMask)
				ListView_SetCheckState(hwndList, i - ID_STATUS_OFFLINE, TRUE)
			else
			ListView_SetCheckState(hwndList, i - ID_STATUS_OFFLINE, FALSE);
		}
	}
	SendDlgItemMessage(clvmHwnd, IDC_PROTOGROUPOP, CB_SETCURSEL, dwFlags & CLVM_PROTOGROUP_OP ? 1 : 0, 0);
	SendDlgItemMessage(clvmHwnd, IDC_GROUPSTATUSOP, CB_SETCURSEL, dwFlags & CLVM_GROUPSTATUS_OP ? 1 : 0, 0);
	CheckDlgButton(clvmHwnd, IDC_AUTOCLEAR, dwFlags & CLVM_AUTOCLEAR ? BST_CHECKED : BST_UNCHECKED);
	UpdateStickies();

	{
		int useLastMsg = dwFlags & CLVM_USELASTMSG;

		int useGroupsState = (dwFlags & CLVM_USEGROUPS) ? BST_CHECKED : (dwFlags & CLVM_DONOTUSEGROUPS) ? BST_UNCHECKED : BST_INDETERMINATE;

		DWORD lmdat;
		BYTE bTmp;

		CheckDlgButton(clvmHwnd, IDC_LASTMSG, useLastMsg ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(clvmHwnd, IDC_USEGROUPS, useGroupsState ? BST_CHECKED : BST_UNCHECKED);

		EnableWindow(GetDlgItem(clvmHwnd, IDC_LASTMESSAGEOP), useLastMsg);
		EnableWindow(GetDlgItem(clvmHwnd, IDC_LASTMSGVALUE), useLastMsg);
		EnableWindow(GetDlgItem(clvmHwnd, IDC_LASTMESSAGEUNIT), useLastMsg);

		mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_LM", 246, szBuf);
		lmdat = db_get_dw(NULL, CLVM_MODULE, szSetting, 0);

		SetDlgItemInt(clvmHwnd, IDC_LASTMSGVALUE, LOWORD(lmdat), FALSE);
		bTmp = LOBYTE(HIWORD(lmdat));
		SendDlgItemMessage(clvmHwnd, IDC_LASTMESSAGEOP, CB_SETCURSEL, bTmp, 0);
		bTmp = HIBYTE(HIWORD(lmdat));
		SendDlgItemMessage(clvmHwnd, IDC_LASTMESSAGEUNIT, CB_SETCURSEL, bTmp, 0);
	}

	ShowPage(clvmHwnd, 0);
cleanup:
	db_free(&dbv_pf);
	db_free(&dbv_gf);
	mir_free(szBuf);
	mir_free(szTempBuf);
}

void DeleteViewMode(char * szName)
{
	char szSetting[256];

	mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_PF", 246, szName);
	db_unset(NULL, CLVM_MODULE, szSetting);
	mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_GF", 246, szName);
	db_unset(NULL, CLVM_MODULE, szSetting);
	mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_SM", 246, szName);
	db_unset(NULL, CLVM_MODULE, szSetting);
	mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_VA", 246, szName);
	db_unset(NULL, CLVM_MODULE, szSetting);
	mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_SSM", 246, szName);
	db_unset(NULL, CLVM_MODULE, szSetting);
	db_unset(NULL, CLVM_MODULE, szName);

	if (!mir_strcmp(g_CluiData.current_viewmode, szName) && mir_strlen(szName) == mir_strlen(g_CluiData.current_viewmode)) {
		g_CluiData.bFilterEffective = 0;
		pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);
		SetWindowText(hwndSelector, TranslateT("All contacts"));
	}

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		if (db_get_dw(hContact, CLVM_MODULE, szName, -1) != -1)
			db_set_dw(hContact, CLVM_MODULE, szName, 0);
}

INT_PTR CALLBACK DlgProcViewModesSetup(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	clvmHwnd = hwndDlg;

	switch (msg) {
	case WM_INITDIALOG:
		xpt_EnableThemeDialogTexture(hwndDlg, ETDT_ENABLETAB);
		himlViewModes = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 12, 0);
		{
			int i;
			for (i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
				HICON hIcon = LoadSkinnedProtoIcon(NULL, i);
				ImageList_AddIcon(himlViewModes, hIcon);
				Skin_ReleaseIcon(hIcon);
			}

			HICON hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_SMALLDOT), IMAGE_ICON, 16, 16, 0);
			nullImage = ImageList_AddIcon(himlViewModes, hIcon);
			DestroyIcon(hIcon);

			RECT rcClient;
			GetClientRect(hwndDlg, &rcClient);

			TCITEM tci;
			tci.mask = TCIF_PARAM | TCIF_TEXT;
			tci.lParam = 0;
			tci.pszText = TranslateT("Sticky contacts");
			SendDlgItemMessage(hwndDlg, IDC_TAB, TCM_INSERTITEM, 0, (LPARAM)&tci);

			tci.pszText = TranslateT("Filtering");
			SendDlgItemMessage(hwndDlg, IDC_TAB, TCM_INSERTITEM, 0, (LPARAM)&tci);

			TabCtrl_SetCurSel(GetDlgItem(hwndDlg, IDC_TAB), 0);

			TranslateDialogDefault(hwndDlg);
			FillDialog(hwndDlg);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ADDVIEWMODE), FALSE);
			{
				LONG_PTR style = GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CLIST), GWL_STYLE);
				style &= (~CLS_SHOWHIDDEN);
				SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CLIST), GWL_STYLE, style);
			}

			SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETEXTRAIMAGELIST, 0, (LPARAM)himlViewModes);
			SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETEXTRACOLUMNS, ID_STATUS_OUTTOLUNCH - ID_STATUS_OFFLINE, 0);

			CLCINFOITEM cii = { sizeof(cii) };
			cii.hParentGroup = 0;
			cii.pszText = TranslateT("*** All contacts ***");
			hInfoItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
			SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETHIDEEMPTYGROUPS, 1, 0);

			int index = 0;

			if (g_CluiData.current_viewmode[0] != '\0') {
				TCHAR *temp = mir_utf8decodeW(g_CluiData.current_viewmode);
				if (temp) {
					index = SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_FINDSTRING, -1, (LPARAM)temp);
					mir_free(temp);
				}
				if (index == -1)
					index = 0;
			}

			if (SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_SETCURSEL, index, 0) != LB_ERR) {
				clvm_curItem = index;
				UpdateFilters();
			}
			else
				clvm_curItem = -1;
			g_ViewModeOptDlg = TRUE;
			i = 0;
			while (_page2Controls[i] != 0)
				ShowWindow(GetDlgItem(hwndDlg, _page2Controls[i++]), SW_HIDE);
			ShowWindow(hwndDlg, SW_SHOWNORMAL);
			EnableWindow(GetDlgItem(hwndDlg, IDC_APPLY), FALSE);
			SendDlgItemMessage(hwndDlg, IDC_AUTOCLEARSPIN, UDM_SETRANGE, 0, MAKELONG(1000, 0));
			SetWindowText(hwndDlg, TranslateT("Configure view modes"));
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_PROTOGROUPOP:
		case IDC_GROUPSTATUSOP:
		case IDC_LASTMESSAGEUNIT:
		case IDC_LASTMESSAGEOP:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				EnableWindow(GetDlgItem(hwndDlg, IDC_APPLY), TRUE);
			break;

		case IDC_USEGROUPS:
			EnableWindow(GetDlgItem(hwndDlg, IDC_APPLY), TRUE);
			break;

		case IDC_AUTOCLEAR:
			EnableWindow(GetDlgItem(hwndDlg, IDC_APPLY), TRUE);
			break;

		case IDC_LASTMSG:
		{
			int bUseLastMsg = IsDlgButtonChecked(hwndDlg, IDC_LASTMSG);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LASTMESSAGEOP), bUseLastMsg);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LASTMESSAGEUNIT), bUseLastMsg);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LASTMSGVALUE), bUseLastMsg);
			EnableWindow(GetDlgItem(hwndDlg, IDC_APPLY), TRUE);
		}
		break;

		case IDC_AUTOCLEARVAL:
		case IDC_LASTMSGVALUE:
			if (HIWORD(wParam) == EN_CHANGE && GetFocus() == (HWND)lParam)
				EnableWindow(GetDlgItem(hwndDlg, IDC_APPLY), TRUE);
			break;

		case IDC_DELETEVIEWMODE:
			if (MessageBox(0, TranslateT("Really delete this view mode? This cannot be undone"), TranslateT("Delete a view mode"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
				int iLen = SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_GETTEXTLEN, SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_GETCURSEL, 0, 0), 0);
				if (iLen) {
					//TODO: UNICODE
					TCHAR *szTempBuf = (TCHAR*)malloc((iLen + 1)*sizeof(TCHAR));
					char *szBuf = NULL;
					if (szTempBuf) {
						SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_GETTEXT, SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_GETCURSEL, 0, 0), (LPARAM)szTempBuf);
						szBuf = mir_utf8encodeT(szTempBuf);
						DeleteViewMode(szBuf);

						SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_DELETESTRING, SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_GETCURSEL, 0, 0), 0);
						if (SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_SETCURSEL, 0, 0) != LB_ERR) {
							clvm_curItem = 0;
							UpdateFilters();
						}
						else clvm_curItem = -1;

						if (szBuf && szBuf != (char*)szTempBuf) mir_free(szBuf);
						if (szTempBuf) free(szTempBuf);
						szTempBuf = NULL;
						szBuf = NULL;
					}
				}
			}
			break;

		case IDC_ADDVIEWMODE:
		{
			TCHAR szBuf[256];
			szBuf[0] = 0;
			GetDlgItemText(hwndDlg, IDC_NEWVIEMODE, szBuf, SIZEOF(szBuf));
			szBuf[255] = 0;

			if (szBuf[0] != 0) {
				char *szUTF8Buf = mir_utf8encodeT(szBuf);

				if (db_get_dw(NULL, CLVM_MODULE, szUTF8Buf, -1) != -1)
					MessageBox(0, TranslateT("A view mode with this name does already exist"), TranslateT("Duplicate name"), MB_OK);
				else {
					int iNewItem = SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_INSERTSTRING, -1, (LPARAM)szBuf);
					if (iNewItem != LB_ERR) {
						SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_SETCURSEL, (WPARAM)iNewItem, 0);
						SaveViewMode(szUTF8Buf, _T(""), "", 0, -1, 0, 0, 0, 0);
						clvm_curItem = iNewItem;
						UpdateStickies();
						SendDlgItemMessage(hwndDlg, IDC_PROTOGROUPOP, CB_SETCURSEL, 0, 0);
						SendDlgItemMessage(hwndDlg, IDC_GROUPSTATUSOP, CB_SETCURSEL, 0, 0);
					}
				}
				SetDlgItemText(hwndDlg, IDC_NEWVIEMODE, _T(""));
				mir_free(szUTF8Buf);
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_ADDVIEWMODE), FALSE);
			UpdateFilters();
			break;
		}
		case IDC_CLEARALL:
		{
			for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
				HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
				if (hItem)
					SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItem, 0);
			}
		}
		case IDOK:
		case IDC_APPLY:
			SaveState();
			if (g_CluiData.bFilterEffective)
				ApplyViewMode(g_CluiData.current_viewmode);
			if (LOWORD(wParam) == IDOK)
				DestroyWindow(hwndDlg);
			break;
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;
		}

		if (LOWORD(wParam) == IDC_NEWVIEMODE && HIWORD(wParam) == EN_CHANGE)
			EnableWindow(GetDlgItem(hwndDlg, IDC_ADDVIEWMODE), TRUE);

		if (LOWORD(wParam) == IDC_VIEWMODES && HIWORD(wParam) == LBN_SELCHANGE) {
			SaveState();
			clvm_curItem = SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_GETCURSEL, 0, 0);
			UpdateFilters();
			//EnableWindow(GetDlgItem(hwndDlg, IDC_APPLY), TRUE);
			//SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_GROUPS:
		case IDC_STATUSMODES:
		case IDC_PROTOCOLS:
		case IDC_CLIST:
			if (((LPNMHDR)lParam)->code == NM_CLICK || ((LPNMHDR)lParam)->code == CLN_CHECKCHANGED)
				EnableWindow(GetDlgItem(hwndDlg, IDC_APPLY), TRUE);

			if (((LPNMHDR)lParam)->code == NM_CLICK) {
				NMCLISTCONTROL *nm = (NMCLISTCONTROL*)lParam;
				if (nm->iColumn == -1)
					break;

				DWORD hitFlags;
				HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_HITTEST, (WPARAM)&hitFlags, MAKELPARAM(nm->pt.x, nm->pt.y));
				if (hItem == NULL)
					break;

				if (!(hitFlags & CLCHT_ONITEMEXTRA))
					break;

				int iImage = SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn, 0));
				if (iImage == nullImage)
					iImage = nm->iColumn;
				else if (iImage != EMPTY_EXTRA_ICON)
					iImage = nullImage;
				SetIconsForColumn(GetDlgItem(hwndDlg, IDC_CLIST), hItem, hInfoItem, nm->iColumn, iImage);
			}
			break;

		case IDC_TAB:
			if (((LPNMHDR)lParam)->code == TCN_SELCHANGE) {
				int id = TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_TAB));
				if (id == 0)
					ShowPage(hwndDlg, 0);
				else
					ShowPage(hwndDlg, 1);
				break;
			}
		}
		break;

	case WM_DESTROY:
		ImageList_RemoveAll(himlViewModes);
		ImageList_Destroy(himlViewModes);
		g_ViewModeOptDlg = FALSE;
		break;
	}
	return FALSE;
}

static int menuCounter = 0;

static int FillMenuCallback(char *szSetting)
{
	if (BYTE(szSetting[0]) == 246)
		return 1;
	if (szSetting[0] == 13)
		return 1;

	TCHAR *temp = mir_utf8decodeT(szSetting);
	if (temp) {
		AppendMenu(hViewModeMenu, MFT_STRING, menuCounter++, temp);
		mir_free(temp);
	}
	return 1;
}

void BuildViewModeMenu()
{
	if (hViewModeMenu)
		DestroyMenu(hViewModeMenu);

	menuCounter = 100;
	hViewModeMenu = CreatePopupMenu();

	AppendMenu(hViewModeMenu, MFT_STRING, 10002, TranslateT("All contacts"));

	AppendMenu(hViewModeMenu, MF_SEPARATOR, 0, NULL);

	CLVM_EnumModes(FillMenuCallback);

	if (GetMenuItemCount(hViewModeMenu) > 2)
		AppendMenu(hViewModeMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hViewModeMenu, MFT_STRING, 10001, TranslateT("Setup view modes..."));
}

LRESULT CALLBACK ViewModeFrameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CREATE:
	{
		RECT rcMargins = { 12, 0, 2, 0 };
		hwndSelector = CreateWindow(MIRANDABUTTONCLASS, _T(""), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 20, 20,
			hwnd, (HMENU)IDC_SELECTMODE, g_hInst, NULL);
		MakeButtonSkinned(hwndSelector);
		SendMessage(hwndSelector, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Select a view mode"), BATF_TCHAR);
		SendMessage(hwndSelector, BUTTONSETMARGINS, 0, (LPARAM)&rcMargins);
		SendMessage(hwndSelector, BUTTONSETID, 0, (LPARAM) "ViewMode.Select");
		SendMessage(hwndSelector, WM_SETFONT, 0, (LPARAM)FONTID_VIEMODES + 1);
		SendMessage(hwndSelector, BUTTONSETASFLATBTN, TRUE, 0);
		SendMessage(hwndSelector, MBM_UPDATETRANSPARENTFLAG, 0, 2);
		SendMessage(hwndSelector, BUTTONSETSENDONDOWN, 0, (LPARAM)1);

		//SendMessage(hwndSelector, BM_SETASMENUACTION, 1, 0);
		HWND hwndButton = CreateWindow(MIRANDABUTTONCLASS, _T(""), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 20, 20,
			hwnd, (HMENU)IDC_CONFIGUREMODES, g_hInst, NULL);
		MakeButtonSkinned(hwndButton);
		SendMessage(hwndButton, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Setup view modes"), BATF_TCHAR);
		SendMessage(hwndButton, BUTTONSETID, 0, (LPARAM) "ViewMode.Setup");
		SendMessage(hwndButton, BUTTONSETASFLATBTN, TRUE, 0);
		SendMessage(hwndButton, MBM_UPDATETRANSPARENTFLAG, 0, 2);

		hwndButton = CreateWindow(MIRANDABUTTONCLASS, _T(""), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 20, 20,
			hwnd, (HMENU)IDC_RESETMODES, g_hInst, NULL);
		MakeButtonSkinned(hwndButton);
		SendMessage(hwndButton, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Clear view mode and return to default display"), BATF_TCHAR);
		SendMessage(hwndButton, BUTTONSETID, 0, (LPARAM) "ViewMode.Clear");
		SendMessage(hwnd, WM_USER + 100, 0, 0);
		SendMessage(hwndButton, BUTTONSETASFLATBTN, TRUE, 0);
		SendMessage(hwndButton, MBM_UPDATETRANSPARENTFLAG, 0, 2);
	}
	return FALSE;

	case WM_NCCALCSIZE:
		return 18;// FrameNCCalcSize(hwnd, DefWindowProc, wParam, lParam, hasTitleBar);

	case WM_SIZE:
	{
		RECT rcCLVMFrame;
		HDWP PosBatch = BeginDeferWindowPos(3);
		GetClientRect(hwnd, &rcCLVMFrame);
		PosBatch = DeferWindowPos(PosBatch, GetDlgItem(hwnd, IDC_RESETMODES), 0,
			rcCLVMFrame.right - 23, 1, 22, 18, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
		PosBatch = DeferWindowPos(PosBatch, GetDlgItem(hwnd, IDC_CONFIGUREMODES), 0,
			rcCLVMFrame.right - 45, 1, 22, 18, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
		PosBatch = DeferWindowPos(PosBatch, GetDlgItem(hwnd, IDC_SELECTMODE), 0,
			1, 1, rcCLVMFrame.right - 46, 18, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
		EndDeferWindowPos(PosBatch);
	}
	break;

	case WM_USER + 100:
		SendDlgItemMessage(hwnd, IDC_RESETMODES, MBM_SETICOLIBHANDLE, 0,
			(LPARAM)RegisterIcolibIconHandle("CLN_CLVM_reset", LPGEN("Contact list"), LPGEN("Reset view mode"), NULL, 0, g_hInst, IDI_RESETVIEW));

		SendDlgItemMessage(hwnd, IDC_CONFIGUREMODES, MBM_SETICOLIBHANDLE, 0,
			(LPARAM)RegisterIcolibIconHandle("CLN_CLVM_set", LPGEN("Contact list"), LPGEN("Setup view modes"), NULL, 0, g_hInst, IDI_SETVIEW));

		{
			for (int i = 0; _buttons[i] != 0; i++) {
				SendDlgItemMessage(hwnd, _buttons[i], BUTTONSETASFLATBTN, TRUE, 0);
				SendDlgItemMessage(hwnd, _buttons[i], BUTTONSETASFLATBTN + 10, 0, 0);
			}
		}

		if (g_CluiData.bFilterEffective)
			SetDlgItemText(hwnd, IDC_SELECTMODE, ptrT(mir_utf8decodeT(g_CluiData.current_viewmode)));
		else
			SetDlgItemText(hwnd, IDC_SELECTMODE, TranslateT("All contacts"));
		break;

	case WM_ERASEBKGND:
		if (g_CluiData.fDisableSkinEngine)
			return sttDrawViewModeBackground(hwnd, (HDC)wParam, NULL);
		return 0;

	case WM_NCPAINT:
	case WM_PAINT:
		if (GetParent(hwnd) == pcli->hwndContactList && g_CluiData.fLayered)
			ValidateRect(hwnd, NULL);

		else if (GetParent(hwnd) != pcli->hwndContactList || !g_CluiData.fLayered) {
			RECT rc = { 0 };
			GetClientRect(hwnd, &rc);
			rc.right++;
			rc.bottom++;
			HDC hdc = GetDC(hwnd);
			HDC hdc2 = CreateCompatibleDC(hdc);
			HBITMAP hbmp = ske_CreateDIB32(rc.right, rc.bottom);
			HBITMAP hbmpo = (HBITMAP)SelectObject(hdc2, hbmp);

			if (g_CluiData.fDisableSkinEngine)
				sttDrawViewModeBackground(hwnd, hdc2, &rc);
			else {
				if (GetParent(hwnd) != pcli->hwndContactList) {
					HBRUSH br = GetSysColorBrush(COLOR_3DFACE);
					FillRect(hdc2, &rc, br);
				}
				else ske_BltBackImage(hwnd, hdc2, &rc);

				DrawViewModeBar(hwnd, hdc2);
			}

			for (int i = 0; _buttons[i] != 0; i++) {
				RECT childRect;
				RECT MyRect;
				POINT Offset;
				GetWindowRect(hwnd, &MyRect);
				GetWindowRect(GetDlgItem(hwnd, _buttons[i]), &childRect);
				Offset.x = childRect.left - MyRect.left;
				Offset.y = childRect.top - MyRect.top;
				SendDlgItemMessage(hwnd, _buttons[i], BUTTONDRAWINPARENT, (WPARAM)hdc2, (LPARAM)&Offset);
			}

			BitBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdc2, rc.left, rc.top, SRCCOPY);
			SelectObject(hdc2, hbmpo);
			DeleteObject(hbmp);
			DeleteDC(hdc2);

			SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));

			ReleaseDC(hwnd, hdc);
			ValidateRect(hwnd, NULL);
		}
		return 0;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == BUTTONNEEDREDRAW)
			pcli->pfnInvalidateRect(hwnd, NULL, FALSE);
		return 0;

	case WM_TIMER:
		if (wParam == TIMERID_VIEWMODEEXPIRE) {
			RECT rcCLUI;
			GetWindowRect(pcli->hwndContactList, &rcCLUI);

			POINT pt;
			GetCursorPos(&pt);
			if (PtInRect(&rcCLUI, pt))
				break;

			KillTimer(hwnd, wParam);
			if (!g_CluiData.old_viewmode[0])
				SendMessage(hwnd, WM_COMMAND, IDC_RESETMODES, 0);
			else
				ApplyViewMode((const char *)g_CluiData.old_viewmode);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SELECTMODE:
			BuildViewModeMenu();
			{
				RECT rc;
				GetWindowRect((HWND)lParam, &rc);
				POINT pt = { rc.left, rc.bottom };
				int selection = TrackPopupMenu(hViewModeMenu, TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, GetParent(hwnd), NULL);
				PostMessage(hwnd, WM_NULL, 0, 0);
				if (selection) {
					if (selection == 10001)
						goto clvm_config_command;
					if (selection == 10002)
						goto clvm_reset_command;

					TCHAR szTemp[256];
					MENUITEMINFO mii = { sizeof(mii) };
					mii.fMask = MIIM_STRING;
					mii.dwTypeData = szTemp;
					mii.cch = 256;
					GetMenuItemInfo(hViewModeMenu, selection, FALSE, &mii);

					ApplyViewMode(ptrA(mir_utf8encodeT(szTemp)));
				}
			}
			break;

		case IDC_RESETMODES:
		clvm_reset_command :
			ApplyViewMode("");
						   break;

		case IDC_CONFIGUREMODES:
		clvm_config_command :
			if (!g_ViewModeOptDlg)
				CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_OPT_VIEWMODES), 0, DlgProcViewModesSetup, 0);
							break;
		}

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return TRUE;
}

static HWND hCLVMFrame;
HWND g_hwndViewModeFrame;

struct view_mode_t
{
	HBITMAP  hBmpBackground;
	COLORREF bkColour;
	int      useWinColors;
	int      backgroundBmpUse;

	view_mode_t() :
		hBmpBackground(NULL),
		bkColour(CLCDEFAULT_BKCOLOUR),
		useWinColors(CLCDEFAULT_USEWINDOWSCOLOURS),
		backgroundBmpUse(CLCDEFAULT_USEBITMAP)
	{}
};

static view_mode_t view_mode;

static BOOL sttDrawViewModeBackground(HWND hwnd, HDC hdc, RECT *rect)
{
	BOOL bFloat = (GetParent(hwnd) != pcli->hwndContactList);
	if (g_CluiData.fDisableSkinEngine || !g_CluiData.fLayered || bFloat) {
		RECT rc;
		if (rect)
			rc = *rect;
		else
			GetClientRect(hwnd, &rc);

		if (!view_mode.hBmpBackground && !view_mode.useWinColors) {
			HBRUSH hbr = CreateSolidBrush(view_mode.bkColour);
			FillRect(hdc, &rc, hbr);
			DeleteObject(hbr);
		}
		else DrawBackGround(hwnd, hdc, view_mode.hBmpBackground, view_mode.bkColour, view_mode.backgroundBmpUse);
	}
	return TRUE;
}

COLORREF sttGetColor(char * module, char * color, COLORREF defColor); //clcutils

static int  ehhViewModeBackgroundSettingsChanged(WPARAM, LPARAM)
{
	if (view_mode.hBmpBackground) {
		DeleteObject(view_mode.hBmpBackground);
		view_mode.hBmpBackground = NULL;
	}

	if (g_CluiData.fDisableSkinEngine) {
		DBVARIANT dbv;
		view_mode.bkColour = sttGetColor("ViewMode", "BkColour", CLCDEFAULT_BKCOLOUR);
		if (db_get_b(NULL, "ViewMode", "UseBitmap", CLCDEFAULT_USEBITMAP)) {
			if (!db_get_s(NULL, "ViewMode", "BkBitmap", &dbv)) {
				view_mode.hBmpBackground = (HBITMAP)CallService(MS_UTILS_LOADBITMAP, 0, (LPARAM)dbv.pszVal);
				db_free(&dbv);
			}
		}
		view_mode.useWinColors = db_get_b(NULL, "ViewMode", "UseWinColours", CLCDEFAULT_USEWINDOWSCOLOURS);
		view_mode.backgroundBmpUse = db_get_w(NULL, "ViewMode", "BkBmpUse", CLCDEFAULT_BKBMPUSE);
	}
	PostMessage(pcli->hwndContactList, WM_SIZE, 0, 0);
	return 0;
}

void CreateViewModeFrame()
{
	CallService(MS_BACKGROUNDCONFIG_REGISTER, (WPARAM)(LPGEN("View mode background")"/ViewMode"), 0);
	HookEvent(ME_BACKGROUNDCONFIG_CHANGED, ehhViewModeBackgroundSettingsChanged);
	ehhViewModeBackgroundSettingsChanged(0, 0);

	WNDCLASS wndclass = { 0 };
	wndclass.style = 0;
	wndclass.lpfnWndProc = ViewModeFrameWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = g_hInst;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_3DFACE);
	wndclass.lpszMenuName = 0;
	wndclass.lpszClassName = _T("CLVMFrameWindow");
	RegisterClass(&wndclass);

	CLISTFrame frame = { 0 };
	frame.cbSize = sizeof(frame);
	frame.tname = _T("View modes");
	frame.hIcon = LoadSkinnedIcon(SKINICON_OTHER_FRAME);
	frame.height = 18;
	frame.TBtname = TranslateT("View modes");
	frame.Flags = F_VISIBLE | F_SHOWTBTIP | F_NOBORDER | F_NO_SUBCONTAINER | F_TCHAR;
	frame.align = alBottom;
	frame.hWnd = CreateWindowEx(0, _T("CLVMFrameWindow"), _T(CLVM_MODULE), WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_CLIPCHILDREN, 0, 0, 20, 20, pcli->hwndContactList, (HMENU)0, g_hInst, NULL);
	g_hwndViewModeFrame = frame.hWnd;
	hCLVMFrame = (HWND)CallService(MS_CLIST_FRAMES_ADDFRAME, (WPARAM)&frame, 0);
	CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)hCLVMFrame, FU_FMPOS);

	CallService(MS_SKINENG_REGISTERPAINTSUB, (WPARAM)frame.hWnd, (LPARAM)ViewModePaintCallbackProc); //$$$$$ register sub for frame

	ApplyViewMode(NULL); //Apply last selected view mode
}

const char *MakeVariablesString(const char *src, const char *UIN);

void ApplyViewMode(const char *Name, bool onlySelector)
{
	char szSetting[256];
	DBVARIANT dbv = { 0 };

	g_CluiData.bFilterEffective = 0;

	mir_snprintf(szSetting, SIZEOF(szSetting), "%c_LastMode", 246);

	if (!Name) { // Name is null - apply last stored view mode
		if (!db_get_s(NULL, CLVM_MODULE, szSetting, &dbv)) {
			Name = NEWSTR_ALLOCA(dbv.pszVal);
			db_free(&dbv);
		}
		else return;
	}

	if (Name[0] == '\0') {
		// Reset View Mode
		g_CluiData.bFilterEffective = 0;

		// remove last applied view mode
		mir_snprintf(szSetting, SIZEOF(szSetting), "%c_LastMode", 246);
		db_unset(NULL, CLVM_MODULE, szSetting);

		if (g_CluiData.bOldUseGroups != (BYTE)-1)
			CallService(MS_CLIST_SETUSEGROUPS, (WPARAM)g_CluiData.bOldUseGroups, 0);

		pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);
		KillTimer(g_hwndViewModeFrame, TIMERID_VIEWMODEEXPIRE);
		SetDlgItemText(g_hwndViewModeFrame, IDC_SELECTMODE, TranslateT("All contacts"));
		if (g_CluiData.boldHideOffline != (BYTE)-1)
			CallService(MS_CLIST_SETHIDEOFFLINE, (WPARAM)g_CluiData.boldHideOffline, 0);
		if (g_CluiData.bOldUseGroups != (BYTE)-1)
			CallService(MS_CLIST_SETUSEGROUPS, (WPARAM)g_CluiData.bOldUseGroups, 0);
		g_CluiData.boldHideOffline = (BYTE)-1;
		g_CluiData.bOldUseGroups = (BYTE)-1;
		g_CluiData.current_viewmode[0] = 0;
		g_CluiData.old_viewmode[0] = 0;
		return;
	}

	if (!onlySelector) {
		mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_PF", 246, Name);
		if (!db_get_s(NULL, CLVM_MODULE, szSetting, &dbv)) {
			if (mir_strlen(dbv.pszVal) >= 2) {
				mir_strncpy(g_CluiData.protoFilter, dbv.pszVal, SIZEOF(g_CluiData.protoFilter));
				g_CluiData.protoFilter[SIZEOF(g_CluiData.protoFilter) - 1] = 0;
				g_CluiData.bFilterEffective |= CLVM_FILTER_PROTOS;
			}
			mir_free(dbv.pszVal);
		}
		mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_GF", 246, Name);
		if (!db_get_ts(NULL, CLVM_MODULE, szSetting, &dbv)) {
			if (mir_tstrlen(dbv.ptszVal) >= 2) {
				mir_tstrncpy(g_CluiData.groupFilter, dbv.ptszVal, SIZEOF(g_CluiData.groupFilter));
				g_CluiData.groupFilter[SIZEOF(g_CluiData.groupFilter) - 1] = 0;
				g_CluiData.bFilterEffective |= CLVM_FILTER_GROUPS;
			}
			mir_free(dbv.ptszVal);
		}
		mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_SM", 246, Name);
		g_CluiData.statusMaskFilter = db_get_dw(NULL, CLVM_MODULE, szSetting, -1);
		if (g_CluiData.statusMaskFilter >= 1)
			g_CluiData.bFilterEffective |= CLVM_FILTER_STATUS;

		mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_SSM", 246, Name);
		g_CluiData.stickyMaskFilter = db_get_dw(NULL, CLVM_MODULE, szSetting, -1);
		if (g_CluiData.stickyMaskFilter != -1)
			g_CluiData.bFilterEffective |= CLVM_FILTER_STICKYSTATUS;

		g_CluiData.filterFlags = db_get_dw(NULL, CLVM_MODULE, Name, 0);

		KillTimer(g_hwndViewModeFrame, TIMERID_VIEWMODEEXPIRE);

		if (g_CluiData.filterFlags & CLVM_AUTOCLEAR) {
			mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_OPT", 246, Name);
			DWORD timerexpire = LOWORD(db_get_dw(NULL, CLVM_MODULE, szSetting, 0));
			mir_strncpy(g_CluiData.old_viewmode, g_CluiData.current_viewmode, SIZEOF(g_CluiData.old_viewmode));
			g_CluiData.old_viewmode[255] = 0;
			CLUI_SafeSetTimer(g_hwndViewModeFrame, TIMERID_VIEWMODEEXPIRE, timerexpire * 1000, NULL);
		}
		else { //store last selected view mode only if it is not autoclear
			mir_snprintf(szSetting, SIZEOF(szSetting), "%c_LastMode", 246);
			db_set_s(NULL, CLVM_MODULE, szSetting, Name);
		}
		mir_strncpy(g_CluiData.current_viewmode, Name, SIZEOF(g_CluiData.current_viewmode));
		g_CluiData.current_viewmode[255] = 0;

		if (g_CluiData.filterFlags & CLVM_USELASTMSG) {
			BYTE bSaved = g_CluiData.bSortByOrder[0];

			g_CluiData.bSortByOrder[0] = SORTBY_LASTMSG;
			for (int i = 0; i < clistCache.getCount(); i++) {
				ClcCacheEntry *pdnce = clistCache[i];
				pdnce->dwLastMsgTime = CompareContacts2_getLMTime(pdnce->hContact);
			}
			g_CluiData.bSortByOrder[0] = bSaved;

			g_CluiData.bFilterEffective |= CLVM_FILTER_LASTMSG;
			mir_snprintf(szSetting, SIZEOF(szSetting), "%c%s_LM", 246, Name);
			g_CluiData.lastMsgFilter = db_get_dw(NULL, CLVM_MODULE, szSetting, 0);
			if (LOBYTE(HIWORD(g_CluiData.lastMsgFilter)))
				g_CluiData.bFilterEffective |= CLVM_FILTER_LASTMSG_NEWERTHAN;
			else
				g_CluiData.bFilterEffective |= CLVM_FILTER_LASTMSG_OLDERTHAN;

			DWORD unit = LOWORD(g_CluiData.lastMsgFilter);
			switch (HIBYTE(HIWORD(g_CluiData.lastMsgFilter))) {
			case 0:
				unit *= 60;
				break;
			case 1:
				unit *= 3600;
				break;
			case 2:
				unit *= 86400;
				break;
			}
			g_CluiData.lastMsgFilter = unit;
		}

		if (HIWORD(g_CluiData.filterFlags) > 0)
			g_CluiData.bFilterEffective |= CLVM_STICKY_CONTACTS;

		if (g_CluiData.bFilterEffective & CLVM_FILTER_STATUS) {
			if (g_CluiData.boldHideOffline == (BYTE)-1)
				g_CluiData.boldHideOffline = db_get_b(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT);

			CallService(MS_CLIST_SETHIDEOFFLINE, 0, 0);
		}
		else if (g_CluiData.boldHideOffline != (BYTE)-1) {
			CallService(MS_CLIST_SETHIDEOFFLINE, g_CluiData.boldHideOffline, 0);
			g_CluiData.boldHideOffline = -1;
		}

		int bUseGroups = -1;

		if (g_CluiData.filterFlags & CLVM_USEGROUPS)
			bUseGroups = 1;
		else if (g_CluiData.filterFlags & CLVM_DONOTUSEGROUPS)
			bUseGroups = 0;

		if (bUseGroups != -1) {
			if (g_CluiData.bOldUseGroups == (BYTE)-1)
				g_CluiData.bOldUseGroups = db_get_b(NULL, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT);

			CallService(MS_CLIST_SETUSEGROUPS, bUseGroups, 0);
		}
		else if (g_CluiData.bOldUseGroups != (BYTE)-1) {
			CallService(MS_CLIST_SETUSEGROUPS, g_CluiData.bOldUseGroups, 0);
			g_CluiData.bOldUseGroups = -1;
		}
	}

	SetWindowText(hwndSelector, ptrT(mir_utf8decodeW((Name[0] == 13) ? Name + 1 : Name)));

	pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);
	CLUI__cliInvalidateRect(pcli->hwndStatus, NULL, FALSE);
}

static int SkinSetViewMode(WPARAM wParam /*char * name*/, LPARAM lParam /*int index*/)
{
	if (wParam == 0 && lParam == 0)
		ApplyViewMode(NULL);
	else if (wParam && !IsBadStringPtrA((const char*)wParam, -1))
		ApplyViewMode((const char*)wParam);
	return 0;
}
