/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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
*/

#include "stdafx.h"
#include "cluiframes.h"

#define TIMERID_VIEWMODEEXPIRE 100

extern HPEN g_hPenCLUIFrames;
extern FRAMEWND *wndFrameViewMode;

typedef int(__cdecl *pfnEnumCallback)(char *szName);
static HWND sttClvmHwnd = nullptr;
static int sttClvm_curItem = 0;
HMENU hViewModeMenu = nullptr;

static int nullImage;
static HWND hwndSelector = nullptr;
static HANDLE hInfoItem = nullptr;
static HIMAGELIST himlViewModes = nullptr;
static uint32_t sttStickyStatusMask = 0;
static char sttModeName[2048];

static int g_ViewModeOptDlg = FALSE;

static UINT _page1Controls[] = { IDC_STATIC1, IDC_STATIC2, IDC_STATIC3, IDC_STATIC5, IDC_STATIC4,
IDC_STATIC8, IDC_ADDVIEWMODE, IDC_DELETEVIEWMODE, IDC_NEWVIEMODE, IDC_GROUPS, IDC_PROTOCOLS,
IDC_VIEWMODES, IDC_STATUSMODES, IDC_STATIC12, IDC_STATIC13, IDC_STATIC14, IDC_PROTOGROUPOP, IDC_GROUPSTATUSOP,
IDC_AUTOCLEAR, IDC_AUTOCLEARVAL, IDC_AUTOCLEARSPIN, IDC_STATIC15, IDC_STATIC16,
IDC_LASTMESSAGEOP, IDC_LASTMESSAGEUNIT, IDC_LASTMSG, IDC_LASTMSGVALUE, 0 };

static UINT _page2Controls[] = { IDC_CLIST, IDC_STATIC9, IDC_STATIC8, IDC_CLEARALL, IDC_CURVIEWMODE2, 0 };

/*
 * enumerate all view modes, call the callback function with the mode name
 * useful for filling lists, menus and so on..
 */

int CLVM_EnumProc(const char *szSetting, void *lParam)
{
	pfnEnumCallback EnumCallback = (pfnEnumCallback)lParam;
	if (szSetting != nullptr)
		EnumCallback((char *)szSetting);
	return 0;
}

void CLVM_EnumModes(pfnEnumCallback EnumCallback)
{
	db_enum_settings(NULL, CLVM_EnumProc, CLVM_MODULE, EnumCallback);
}

int FillModes(char *szsetting)
{
	if (szsetting[0] != 'ö')
		SendDlgItemMessageA(sttClvmHwnd, IDC_VIEWMODES, LB_INSERTSTRING, -1, (LPARAM)szsetting);
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
		RedrawWindow(hwnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE);
	}
}

static int UpdateClistItem(MCONTACT hContact, uint32_t mask)
{
	for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_MAX; i++)
		SendDlgItemMessage(sttClvmHwnd, IDC_CLIST, CLM_SETEXTRAIMAGE, hContact, MAKELONG(i - ID_STATUS_OFFLINE,
		(1 << (i - ID_STATUS_OFFLINE)) & mask ? i - ID_STATUS_OFFLINE : nullImage));

	return 0;
}

static uint32_t GetMaskForItem(HANDLE hItem)
{
	uint32_t dwMask = 0;

	for (int i = 0; i <= ID_STATUS_MAX - ID_STATUS_OFFLINE; i++)
		dwMask |= (SendDlgItemMessage(sttClvmHwnd, IDC_CLIST, CLM_GETEXTRAIMAGE, (WPARAM)hItem, i) == nullImage ? 0 : 1 << i);

	return dwMask;
}

static void UpdateStickies()
{
	for (auto &hContact : Contacts()) {
		MCONTACT hItem = (MCONTACT)SendDlgItemMessage(sttClvmHwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
		if (hItem)
			SendDlgItemMessage(sttClvmHwnd, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItem, db_get_b(hContact, "CLVM", sttModeName, 0) ? 1 : 0);
		uint32_t localMask = HIWORD(db_get_dw(hContact, "CLVM", sttModeName, 0));
		UpdateClistItem(hItem, (localMask == 0 || localMask == sttStickyStatusMask) ? sttStickyStatusMask : localMask);
	}

	for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_MAX; i++)
		SendDlgItemMessage(sttClvmHwnd, IDC_CLIST, CLM_SETEXTRAIMAGE, (WPARAM)hInfoItem, 
			MAKELONG(i - ID_STATUS_OFFLINE, (1 << (i - ID_STATUS_OFFLINE)) & sttStickyStatusMask ? i - ID_STATUS_OFFLINE : MAX_STATUS_COUNT));

	HANDLE hItem = (HANDLE)SendDlgItemMessage(sttClvmHwnd, IDC_CLIST, CLM_GETNEXTITEM, CLGN_ROOT, 0);
	hItem = (HANDLE)SendDlgItemMessage(sttClvmHwnd, IDC_CLIST, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hItem);
	while (hItem) {
		for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_MAX; i++)
			SendDlgItemMessage(sttClvmHwnd, IDC_CLIST, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELONG(i - ID_STATUS_OFFLINE, nullImage));
		hItem = (HANDLE)SendDlgItemMessage(sttClvmHwnd, IDC_CLIST, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hItem);
	}
	ShowPage(sttClvmHwnd, 0);
}

static int FillDialog(HWND hwnd)
{
	LVCOLUMN lvc = { 0 };
	HWND hwndList = GetDlgItem(hwnd, IDC_PROTOCOLS);
	LVITEM item = { 0 };

	CLVM_EnumModes(FillModes);
	ListView_SetExtendedListViewStyle(hwndList, LVS_EX_CHECKBOXES);
	lvc.mask = LVCF_FMT;
	lvc.fmt = LVCFMT_IMAGE | LVCFMT_LEFT;
	ListView_InsertColumn(hwndList, 0, &lvc);

	// fill protocols...
	item.mask = LVIF_TEXT | LVIF_PARAM;
	item.iItem = 1000;
	for (auto &pa : Accounts()) {
		item.lParam = (LPARAM)pa->szModuleName;
		item.pszText = pa->tszAccountName;
		ListView_InsertItem(hwndList, &item);
	}

	ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);
	ListView_Arrange(hwndList, LVA_ALIGNLEFT | LVA_ALIGNTOP);

	// fill groups
	hwndList = GetDlgItem(hwnd, IDC_GROUPS);

	ListView_SetExtendedListViewStyle(hwndList, LVS_EX_CHECKBOXES);
	lvc.mask = LVCF_FMT;
	lvc.fmt = LVCFMT_IMAGE | LVCFMT_LEFT;
	ListView_InsertColumn(hwndList, 0, &lvc);

	item.pszText = TranslateT("Ungrouped contacts");
	SendMessage(hwndList, LVM_INSERTITEM, 0, (LPARAM)&item);

	wchar_t *grpName;
	for (int i = 1; (grpName = Clist_GroupGetName(i, nullptr)) != nullptr; i++) {
		item.pszText = grpName;
		SendMessage(hwndList, LVM_INSERTITEM, 0, (LPARAM)&item);
	}
	ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);
	ListView_Arrange(hwndList, LVA_ALIGNLEFT | LVA_ALIGNTOP);

	hwndList = GetDlgItem(hwnd, IDC_STATUSMODES);
	ListView_SetExtendedListViewStyle(hwndList, LVS_EX_CHECKBOXES);

	lvc.mask = LVCF_FMT;
	lvc.fmt = LVCFMT_IMAGE | LVCFMT_LEFT;
	ListView_InsertColumn(hwndList, 0, &lvc);

	for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_MAX; i++) {
		item.pszText = TranslateW(Clist_GetStatusModeDescription(i, 0));
		item.iItem = i - ID_STATUS_OFFLINE;
		ListView_InsertItem(hwndList, &item);
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
	int iOldIcon;
	HANDLE hItem, hChildItem;

	int typeOfFirst = SendMessage(hwndList, CLM_GETITEMTYPE, (WPARAM)hFirstItem, 0);
	//check groups
	if (typeOfFirst == CLCIT_GROUP)
		hItem = hFirstItem;
	else
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hFirstItem);
	while (hItem) {
		hChildItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
		if (hChildItem)
			SetAllChildIcons(hwndList, hChildItem, iColumn, iImage);
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hItem);
	}
	//check contacts
	if (typeOfFirst == CLCIT_CONTACT)
		hItem = hFirstItem;
	else
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, (LPARAM)hFirstItem);
	while (hItem) {
		iOldIcon = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, iColumn);
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

void SaveViewMode(const char *name, const wchar_t *szGroupFilter, const char *szProtoFilter, uint32_t statusMask, uint32_t stickyStatusMask, unsigned int options,
	unsigned int stickies, unsigned int operators, unsigned int lmdat)
{
	char szSetting[512];

	mir_snprintf(szSetting, "%c%s_PF", 246, name);
	db_set_s(0, CLVM_MODULE, szSetting, szProtoFilter);
	mir_snprintf(szSetting, "%c%s_GF", 246, name);
	db_set_ws(0, CLVM_MODULE, szSetting, szGroupFilter);
	mir_snprintf(szSetting, "%c%s_SM", 246, name);
	db_set_dw(0, CLVM_MODULE, szSetting, statusMask);
	mir_snprintf(szSetting, "%c%s_SSM", 246, name);
	db_set_dw(0, CLVM_MODULE, szSetting, stickyStatusMask);
	mir_snprintf(szSetting, "%c%s_OPT", 246, name);
	db_set_dw(0, CLVM_MODULE, szSetting, options);
	mir_snprintf(szSetting, "%c%s_LM", 246, name);
	db_set_dw(0, CLVM_MODULE, szSetting, lmdat);

	db_set_dw(0, CLVM_MODULE, name, MAKELONG((unsigned short)operators, (unsigned short)stickies));
}

// saves the state of the filter definitions for the current item
void SaveState()
{
	CMStringW newGroupFilter(L"|");
	CMStringA newProtoFilter("|");
	uint32_t statusMask = 0;
	uint32_t operators = 0;

	if (sttClvm_curItem == -1)
		return;

	{
		LVITEM item = { 0 };
		item.mask = LVIF_PARAM;

		HWND hwndList = GetDlgItem(sttClvmHwnd, IDC_PROTOCOLS);
		for (int i = 0; i < ListView_GetItemCount(hwndList); i++) {
			if (ListView_GetCheckState(hwndList, i)) {
				item.iItem = i;
				ListView_GetItem(hwndList, &item);

				newProtoFilter.Append((char*)item.lParam);
				newProtoFilter.AppendChar('|');
			}
		}
	}

	{
		LVITEM item = { 0 };
		wchar_t szTemp[256];

		HWND hwndList = GetDlgItem(sttClvmHwnd, IDC_GROUPS);

		operators |= ListView_GetCheckState(hwndList, 0) ? CLVM_INCLUDED_UNGROUPED : 0;

		for (int i = 0; i < ListView_GetItemCount(hwndList); i++) {
			if (ListView_GetCheckState(hwndList, i)) {
				item.mask = LVIF_TEXT;
				item.pszText = szTemp;
				item.cchTextMax = _countof(szTemp);
				item.iItem = i;
				SendMessage(hwndList, LVM_GETITEM, 0, (LPARAM)&item);

				newGroupFilter.Append(szTemp);
				newGroupFilter.AppendChar('|');
			}
		}
	}

	HWND hwndList = GetDlgItem(sttClvmHwnd, IDC_STATUSMODES);
	for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_MAX; i++)
		if (ListView_GetCheckState(hwndList, i - ID_STATUS_OFFLINE))
			statusMask |= (1 << (i - ID_STATUS_OFFLINE));

	int iLen = SendDlgItemMessageA(sttClvmHwnd, IDC_VIEWMODES, LB_GETTEXTLEN, sttClvm_curItem, 0);
	if (iLen) {
		char *szModeName = (char*)malloc(iLen + 1);
		if (szModeName) {
			SendDlgItemMessageA(sttClvmHwnd, IDC_VIEWMODES, LB_GETTEXT, sttClvm_curItem, (LPARAM)szModeName);
			uint32_t dwGlobalMask = GetMaskForItem(hInfoItem);

			unsigned int stickies = 0;
			for (auto &hContact : Contacts()) {
				HANDLE hItem = (HANDLE)SendDlgItemMessage(sttClvmHwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
				if (hItem) {
					if (SendDlgItemMessage(sttClvmHwnd, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItem, 0)) {
						uint32_t dwLocalMask = GetMaskForItem(hItem);
						db_set_dw(hContact, "CLVM", szModeName, MAKELONG(1, (unsigned short)dwLocalMask));
						stickies++;
					}
					else if (db_get_dw(hContact, "CLVM", szModeName, 0))
						db_set_dw(hContact, "CLVM", szModeName, 0);
				}
			}

			operators |= ((SendDlgItemMessage(sttClvmHwnd, IDC_PROTOGROUPOP, CB_GETCURSEL, 0, 0) == 1 ? CLVM_PROTOGROUP_OP : 0) |
				(SendDlgItemMessage(sttClvmHwnd, IDC_GROUPSTATUSOP, CB_GETCURSEL, 0, 0) == 1 ? CLVM_GROUPSTATUS_OP : 0) |
				(IsDlgButtonChecked(sttClvmHwnd, IDC_AUTOCLEAR) ? CLVM_AUTOCLEAR : 0) |
				(IsDlgButtonChecked(sttClvmHwnd, IDC_LASTMSG) ? CLVM_USELASTMSG : 0));

			uint32_t options = SendDlgItemMessage(sttClvmHwnd, IDC_AUTOCLEARSPIN, UDM_GETPOS, 0, 0);

			BOOL translated;
			uint32_t lmdat = MAKELONG(GetDlgItemInt(sttClvmHwnd, IDC_LASTMSGVALUE, &translated, FALSE),
				MAKEWORD(SendDlgItemMessage(sttClvmHwnd, IDC_LASTMESSAGEOP, CB_GETCURSEL, 0, 0),
					SendDlgItemMessage(sttClvmHwnd, IDC_LASTMESSAGEUNIT, CB_GETCURSEL, 0, 0)));

			SaveViewMode(szModeName, newGroupFilter, newProtoFilter, statusMask, dwGlobalMask, options, stickies, operators, lmdat);
			free(szModeName);
		}
	}
	Utils::enableDlgControl(sttClvmHwnd, IDC_APPLY, FALSE);
}


// updates the filter list boxes with the data taken from the filtering string
void UpdateFilters()
{
	DBVARIANT dbv_pf = { 0 };
	DBVARIANT dbv_gf = { 0 };
	char szSetting[128];
	uint32_t statusMask = 0;
	uint32_t dwFlags;
	uint32_t opt;

	if (sttClvm_curItem == LB_ERR)
		return;

	int iLen = SendDlgItemMessageA(sttClvmHwnd, IDC_VIEWMODES, LB_GETTEXTLEN, sttClvm_curItem, 0);
	if (iLen == 0)
		return;

	char *szBuf = (char *)malloc(iLen + 1);
	SendDlgItemMessageA(sttClvmHwnd, IDC_VIEWMODES, LB_GETTEXT, sttClvm_curItem, (LPARAM)szBuf);
	mir_strncpy(sttModeName, szBuf, sizeof(sttModeName));
	{
		wchar_t szTemp[100];
		mir_snwprintf(szTemp, TranslateT("Current view mode: %S"), sttModeName);
		SetDlgItemText(sttClvmHwnd, IDC_CURVIEWMODE2, szTemp);
	}

	mir_snprintf(szSetting, "%c%s_PF", 246, szBuf);
	if (db_get_s(0, CLVM_MODULE, szSetting, &dbv_pf))
		goto cleanup;

	mir_snprintf(szSetting, "%c%s_GF", 246, szBuf);
	if (db_get_ws(0, CLVM_MODULE, szSetting, &dbv_gf))
		goto cleanup;

	mir_snprintf(szSetting, "%c%s_OPT", 246, szBuf);
	if ((opt = db_get_dw(0, CLVM_MODULE, szSetting, -1)) != -1) {
		SendDlgItemMessage(sttClvmHwnd, IDC_AUTOCLEARSPIN, UDM_SETPOS, 0, MAKELONG(LOWORD(opt), 0));
	}

	mir_snprintf(szSetting, "%c%s_SM", 246, szBuf);
	statusMask = db_get_dw(0, CLVM_MODULE, szSetting, -1);

	mir_snprintf(szSetting, "%c%s_SSM", 246, szBuf);
	sttStickyStatusMask = db_get_dw(0, CLVM_MODULE, szSetting, -1);

	dwFlags = db_get_dw(0, CLVM_MODULE, szBuf, 0);
	{
		char szMask[256];
		HWND hwndList = GetDlgItem(sttClvmHwnd, IDC_PROTOCOLS);

		LVITEM item = { 0 };
		item.mask = LVIF_PARAM;

		for (int i = 0; i < ListView_GetItemCount(hwndList); i++) {
			item.iItem = i;
			ListView_GetItem(hwndList, &item);
			mir_snprintf(szMask, "%s|", (char*)item.lParam);
			if (dbv_pf.pszVal && strstr(dbv_pf.pszVal, szMask)) {
				ListView_SetCheckState(hwndList, i, TRUE);
			}
			else {
				ListView_SetCheckState(hwndList, i, FALSE);
			}
		}
	}

	{
		LVITEM item = { 0 };
		wchar_t szTemp[256];
		wchar_t szMask[256];
		HWND hwndList = GetDlgItem(sttClvmHwnd, IDC_GROUPS);

		item.mask = LVIF_TEXT;
		item.pszText = szTemp;
		item.cchTextMax = _countof(szTemp);

		ListView_SetCheckState(hwndList, 0, dwFlags & CLVM_INCLUDED_UNGROUPED ? TRUE : FALSE);

		for (int i = 1; i < ListView_GetItemCount(hwndList); i++) {
			item.iItem = i;
			SendMessage(hwndList, LVM_GETITEM, 0, (LPARAM)&item);
			mir_snwprintf(szMask, L"%s|", szTemp);
			if (dbv_gf.pwszVal && wcsstr(dbv_gf.pwszVal, szMask)) {
				ListView_SetCheckState(hwndList, i, TRUE);
			}
			else {
				ListView_SetCheckState(hwndList, i, FALSE);
			}
		}
	}

	{
		HWND hwndList = GetDlgItem(sttClvmHwnd, IDC_STATUSMODES);
		for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_MAX; i++) {
			if ((1 << (i - ID_STATUS_OFFLINE)) & statusMask)
				ListView_SetCheckState(hwndList, i - ID_STATUS_OFFLINE, TRUE)
			else
				ListView_SetCheckState(hwndList, i - ID_STATUS_OFFLINE, FALSE);
		}
	}

	SendDlgItemMessage(sttClvmHwnd, IDC_PROTOGROUPOP, CB_SETCURSEL, dwFlags & CLVM_PROTOGROUP_OP ? 1 : 0, 0);
	SendDlgItemMessage(sttClvmHwnd, IDC_GROUPSTATUSOP, CB_SETCURSEL, dwFlags & CLVM_GROUPSTATUS_OP ? 1 : 0, 0);
	CheckDlgButton(sttClvmHwnd, IDC_AUTOCLEAR, dwFlags & CLVM_AUTOCLEAR ? BST_CHECKED : BST_UNCHECKED);
	UpdateStickies();

	{
		int useLastMsg = dwFlags & CLVM_USELASTMSG;
		uint32_t lmdat;
		uint8_t bTmp;

		CheckDlgButton(sttClvmHwnd, IDC_LASTMSG, useLastMsg ? BST_CHECKED : BST_UNCHECKED);
		Utils::enableDlgControl(sttClvmHwnd, IDC_LASTMESSAGEOP, useLastMsg);
		Utils::enableDlgControl(sttClvmHwnd, IDC_LASTMSGVALUE, useLastMsg);
		Utils::enableDlgControl(sttClvmHwnd, IDC_LASTMESSAGEUNIT, useLastMsg);

		mir_snprintf(szSetting, "%c%s_LM", 246, szBuf);
		lmdat = db_get_dw(0, CLVM_MODULE, szSetting, 0);

		SetDlgItemInt(sttClvmHwnd, IDC_LASTMSGVALUE, LOWORD(lmdat), FALSE);
		bTmp = LOBYTE(HIWORD(lmdat));
		SendDlgItemMessage(sttClvmHwnd, IDC_LASTMESSAGEOP, CB_SETCURSEL, bTmp, 0);
		bTmp = HIBYTE(HIWORD(lmdat));
		SendDlgItemMessage(sttClvmHwnd, IDC_LASTMESSAGEUNIT, CB_SETCURSEL, bTmp, 0);
	}

	ShowPage(sttClvmHwnd, 0);
cleanup:
	db_free(&dbv_pf);
	db_free(&dbv_gf);
	free(szBuf);
}

INT_PTR CALLBACK DlgProcViewModesSetup(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	sttClvmHwnd = hwndDlg;

	switch (msg) {
	case WM_INITDIALOG:
		if (IS_THEMED)
			EnableThemeDialogTexture(hwndDlg, ETDT_ENABLETAB);

		himlViewModes = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 12, 0);
		for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_MAX; i++)
			ImageList_AddIcon(himlViewModes, Skin_LoadProtoIcon(nullptr, i));
		{
			HICON hIcon = (HICON)LoadImage(g_plugin.getInst(), MAKEINTRESOURCE(IDI_MINIMIZE), IMAGE_ICON, 16, 16, 0);
			nullImage = ImageList_AddIcon(himlViewModes, hIcon);
			DestroyIcon(hIcon);
		}

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
		Utils::enableDlgControl(hwndDlg, IDC_ADDVIEWMODE, FALSE);

		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETEXTRAIMAGELIST, 0, (LPARAM)himlViewModes);
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETEXTRACOLUMNS, ID_STATUS_MAX - ID_STATUS_OFFLINE, 0);
		{
			CLCINFOITEM cii = { 0 };
			cii.cbSize = sizeof(cii);
			cii.hParentGroup = nullptr;
			cii.pszText = LPGENW("*** All contacts ***");
			hInfoItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
		}
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETHIDEEMPTYGROUPS, 1, 0);
		if (SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_SETCURSEL, 0, 0) != LB_ERR) {
			sttClvm_curItem = 0;
			UpdateFilters();
		}
		else sttClvm_curItem = -1;

		g_ViewModeOptDlg = TRUE;

		for (int i = 0; _page2Controls[i] != 0; i++)
			ShowWindow(GetDlgItem(hwndDlg, _page2Controls[i]), SW_HIDE);
		ShowWindow(hwndDlg, SW_SHOWNORMAL);
		Utils::enableDlgControl(hwndDlg, IDC_APPLY, FALSE);
		SendDlgItemMessage(hwndDlg, IDC_AUTOCLEARSPIN, UDM_SETRANGE, 0, MAKELONG(1000, 0));
		SetWindowText(hwndDlg, TranslateT("Configure view modes"));
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_PROTOGROUPOP:
		case IDC_GROUPSTATUSOP:
		case IDC_LASTMESSAGEUNIT:
		case IDC_LASTMESSAGEOP:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				Utils::enableDlgControl(hwndDlg, IDC_APPLY, TRUE);
			break;
		case IDC_AUTOCLEAR:
			Utils::enableDlgControl(hwndDlg, IDC_APPLY, TRUE);
			break;
		case IDC_LASTMSG:
			{
				int bUseLastMsg = IsDlgButtonChecked(hwndDlg, IDC_LASTMSG);
				Utils::enableDlgControl(hwndDlg, IDC_LASTMESSAGEOP, bUseLastMsg);
				Utils::enableDlgControl(hwndDlg, IDC_LASTMESSAGEUNIT, bUseLastMsg);
				Utils::enableDlgControl(hwndDlg, IDC_LASTMSGVALUE, bUseLastMsg);
				Utils::enableDlgControl(hwndDlg, IDC_APPLY, TRUE);
			}
			break;
		case IDC_AUTOCLEARVAL:
		case IDC_LASTMSGVALUE:
			if (HIWORD(wParam) == EN_CHANGE && GetFocus() == (HWND)lParam)
				Utils::enableDlgControl(hwndDlg, IDC_APPLY, TRUE);
			break;
		case IDC_DELETEVIEWMODE:
			if (MessageBox(nullptr, TranslateT("Really delete this view mode? This cannot be undone"), TranslateT("Delete a view mode"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
				char szSetting[256];
				int iLen = SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_GETTEXTLEN, SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_GETCURSEL, 0, 0), 0);
				if (iLen) {
					char *szBuf = (char*)malloc(iLen + 1);
					if (szBuf) {
						SendDlgItemMessageA(hwndDlg, IDC_VIEWMODES, LB_GETTEXT, SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_GETCURSEL, 0, 0), (LPARAM)szBuf);
						mir_snprintf(szSetting, "%c%s_PF", 246, szBuf);
						db_unset(0, CLVM_MODULE, szSetting);
						mir_snprintf(szSetting, "%c%s_GF", 246, szBuf);
						db_unset(0, CLVM_MODULE, szSetting);
						mir_snprintf(szSetting, "%c%s_SM", 246, szBuf);
						db_unset(0, CLVM_MODULE, szSetting);
						mir_snprintf(szSetting, "%c%s_VA", 246, szBuf);
						db_unset(0, CLVM_MODULE, szSetting);
						mir_snprintf(szSetting, "%c%s_SSM", 246, szBuf);
						db_unset(0, CLVM_MODULE, szSetting);
						db_unset(0, CLVM_MODULE, szBuf);
						if (!mir_strcmp(cfg::dat.current_viewmode, szBuf) && mir_strlen(szBuf) == mir_strlen(cfg::dat.current_viewmode)) {
							cfg::dat.bFilterEffective = 0;
							Clist_Broadcast(CLM_AUTOREBUILD, 0, 0);
							SetWindowText(hwndSelector, TranslateT("No view mode"));
						}
						for (auto &hContact : Contacts())
							if (db_get_dw(hContact, "CLVM", szBuf, -1) != -1)
								db_set_dw(hContact, "CLVM", szBuf, 0);

						SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_DELETESTRING, SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_GETCURSEL, 0, 0), 0);
						if (SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_SETCURSEL, 0, 0) != LB_ERR) {
							sttClvm_curItem = 0;
							UpdateFilters();
						}
						else sttClvm_curItem = -1;
						free(szBuf);
					}
				}
			}
			break;

		case IDC_ADDVIEWMODE:
			char szBuf[256];

			szBuf[0] = 0;
			GetDlgItemTextA(hwndDlg, IDC_NEWVIEMODE, szBuf, _countof(szBuf));
			szBuf[255] = 0;

			if (mir_strlen(szBuf) > 2) {
				if (db_get_dw(0, CLVM_MODULE, szBuf, -1) != -1)
					MessageBox(nullptr, TranslateT("A view mode with this name does already exist"), TranslateT("Duplicate name"), MB_OK);
				else {
					int iNewItem = SendDlgItemMessageA(hwndDlg, IDC_VIEWMODES, LB_INSERTSTRING, -1, (LPARAM)szBuf);
					if (iNewItem != LB_ERR) {
						SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_SETCURSEL, (WPARAM)iNewItem, 0);
						SaveViewMode(szBuf, L"", "", -1, -1, 0, 0, 0, 0);
						sttClvm_curItem = iNewItem;
						UpdateStickies();
						SendDlgItemMessage(hwndDlg, IDC_PROTOGROUPOP, CB_SETCURSEL, 0, 0);
						SendDlgItemMessage(hwndDlg, IDC_GROUPSTATUSOP, CB_SETCURSEL, 0, 0);
					}
				}
				SetDlgItemTextA(hwndDlg, IDC_NEWVIEMODE, "");
			}
			Utils::enableDlgControl(hwndDlg, IDC_ADDVIEWMODE, FALSE);
			break;

		case IDC_CLEARALL:
			for (auto &hContact : Contacts()) {
				HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
				if (hItem)
					SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItem, 0);
			}

		case IDOK:
		case IDC_APPLY:
			SaveState();
			if (cfg::dat.bFilterEffective)
				ApplyViewMode(cfg::dat.current_viewmode);
			if (LOWORD(wParam) == IDOK)
				DestroyWindow(hwndDlg);
			break;

		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;
		}

		if (LOWORD(wParam) == IDC_NEWVIEMODE && HIWORD(wParam) == EN_CHANGE)
			Utils::enableDlgControl(hwndDlg, IDC_ADDVIEWMODE, TRUE);
		if (LOWORD(wParam) == IDC_VIEWMODES && HIWORD(wParam) == LBN_SELCHANGE) {
			SaveState();
			sttClvm_curItem = SendDlgItemMessage(hwndDlg, IDC_VIEWMODES, LB_GETCURSEL, 0, 0);
			UpdateFilters();
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_GROUPS:
		case IDC_STATUSMODES:
		case IDC_PROTOCOLS:
		case IDC_CLIST:
			if (((LPNMHDR)lParam)->code == NM_CLICK || ((LPNMHDR)lParam)->code == CLN_CHECKCHANGED)
				Utils::enableDlgControl(hwndDlg, IDC_APPLY, TRUE);

			switch (((LPNMHDR)lParam)->code) {
			case NM_CLICK:
				{
					NMCLISTCONTROL *nm = (NMCLISTCONTROL*)lParam;
					if (nm->iColumn == -1)
						break;

					uint32_t hitFlags;
					HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_HITTEST, (WPARAM)&hitFlags, MAKELPARAM(nm->pt.x, nm->pt.y));
					if (hItem == nullptr || !(hitFlags & CLCHT_ONITEMEXTRA))
						break;

					int iImage = SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn, 0));
					if (iImage == nullImage)
						iImage = nm->iColumn;
					else if (iImage != EMPTY_EXTRA_ICON)
						iImage = nullImage;
					SetIconsForColumn(GetDlgItem(hwndDlg, IDC_CLIST), hItem, hInfoItem, nm->iColumn, iImage);
					break;
				}
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
	if ((uint8_t)szSetting[0] == 246)
		return 1;

	AppendMenuA(hViewModeMenu, MF_STRING, menuCounter++, szSetting);
	return 1;
}

void BuildViewModeMenu()
{
	if (hViewModeMenu)
		DestroyMenu(hViewModeMenu);

	menuCounter = 100;
	hViewModeMenu = CreatePopupMenu();
	CLVM_EnumModes(FillMenuCallback);

	if (GetMenuItemCount(hViewModeMenu) > 0)
		AppendMenu(hViewModeMenu, MF_SEPARATOR, 0, nullptr);

	AppendMenu(hViewModeMenu, MF_STRING, 10001, TranslateT("Setup view modes..."));
	AppendMenu(hViewModeMenu, MF_STRING, 10002, TranslateT("Clear current view mode"));
}

struct
{
	UINT btn_id;
	LPCSTR icon;
}
static _buttons[] =
{
	{ IDC_RESETMODES, "CLN_CLVM_reset" },
	{ IDC_SELECTMODE, "CLN_CLVM_select" },
	{ IDC_CONFIGUREMODES, "CLN_CLVM_options" }
};

LRESULT CALLBACK ViewModeFrameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CREATE:
		hwndSelector = CreateWindowEx(0, MIRANDABUTTONCLASS, L"", BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 20, 20,
			hwnd, (HMENU)IDC_SELECTMODE, g_plugin.getInst(), nullptr);
		CustomizeButton(hwndSelector, false, false, false);
		SendMessage(hwndSelector, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Select a view mode"), BATF_UNICODE);
		SendMessage(hwndSelector, BUTTONSETSENDONDOWN, TRUE, 0);
		{
			HWND hwndButton = CreateWindowEx(0, MIRANDABUTTONCLASS, L"", BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 20, 20,
				hwnd, (HMENU)IDC_CONFIGUREMODES, g_plugin.getInst(), nullptr);
			CustomizeButton(hwndButton, false, false, false);
			SendMessage(hwndButton, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Setup view modes"), BATF_UNICODE);

			hwndButton = CreateWindowEx(0, MIRANDABUTTONCLASS, L"", BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 20, 20,
				hwnd, (HMENU)IDC_RESETMODES, g_plugin.getInst(), nullptr);
			CustomizeButton(hwndButton, false, false, false);
			SendMessage(hwndButton, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Clear view mode and return to default display"), BATF_UNICODE);
		}

		SendMessage(hwnd, WM_USER + 100, 0, 0);
		return FALSE;

	case WM_NCCALCSIZE:
		{
			BOOL hasTitleBar = wndFrameViewMode ? wndFrameViewMode->TitleBar.ShowTitleBar : 0;
			return FrameNCCalcSize(hwnd, DefWindowProc, wParam, lParam, hasTitleBar);
		}
	case WM_NCPAINT:
		{
			BOOL hasTitleBar = wndFrameViewMode ? wndFrameViewMode->TitleBar.ShowTitleBar : 0;
			return FrameNCPaint(hwnd, DefWindowProc, wParam, lParam, hasTitleBar);
		}

	case WM_SIZE:
		RECT rcCLVMFrame;
		{
			HDWP PosBatch = BeginDeferWindowPos(3);
			GetClientRect(hwnd, &rcCLVMFrame);
			PosBatch = DeferWindowPos(PosBatch, GetDlgItem(hwnd, IDC_RESETMODES), nullptr,
				rcCLVMFrame.right - 24, 1, 22, 20, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
			PosBatch = DeferWindowPos(PosBatch, GetDlgItem(hwnd, IDC_CONFIGUREMODES), nullptr,
				rcCLVMFrame.right - 47, 1, 22, 20, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
			PosBatch = DeferWindowPos(PosBatch, GetDlgItem(hwnd, IDC_SELECTMODE), nullptr,
				2, 1, rcCLVMFrame.right - 50, 20, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
			EndDeferWindowPos(PosBatch);
		}
		break;

	case WM_USER + 100:
		{
			bool bSkinned = db_get_b(0, "CLCExt", "bskinned", 0) != 0;
			for (auto &it : _buttons) {
				HWND hwndButton = GetDlgItem(hwnd, it.btn_id);
				SendMessage(hwndButton, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIcon(it.icon));
				CustomizeButton(hwndButton, bSkinned, !bSkinned, bSkinned);
			}
		}

		if (cfg::dat.bFilterEffective)
			SetDlgItemTextA(hwnd, IDC_SELECTMODE, cfg::dat.current_viewmode);
		else
			SetDlgItemText(hwnd, IDC_SELECTMODE, TranslateT("No view mode"));
		break;

	case WM_ERASEBKGND:
		break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		{
			HDC hdc = BeginPaint(hwnd, &ps);
			HDC hdcMem = CreateCompatibleDC(hdc);

			RECT rc;
			GetClientRect(hwnd, &rc);
			HBITMAP hbm = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
			HBITMAP hbmold = (HBITMAP)SelectObject(hdcMem, hbm);

			if (cfg::dat.bWallpaperMode)
				SkinDrawBg(hwnd, hdcMem);
			else
				FillRect(hdcMem, &rc, GetSysColorBrush(COLOR_3DFACE));

			BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);
			SelectObject(hdcMem, hbmold);
			DeleteObject(hbm);
			DeleteDC(hdcMem);
			EndPaint(hwnd, &ps);
		}
		return 0;

	case WM_TIMER:
		switch (wParam) {
		case TIMERID_VIEWMODEEXPIRE:
			{
				POINT pt;
				RECT rcCLUI;

				GetWindowRect(g_clistApi.hwndContactList, &rcCLUI);
				GetCursorPos(&pt);
				if (PtInRect(&rcCLUI, pt))
					break;

				KillTimer(hwnd, wParam);
				if (!cfg::dat.old_viewmode[0])
					SendMessage(hwnd, WM_COMMAND, IDC_RESETMODES, 0);
				else
					ApplyViewMode((const char *)cfg::dat.old_viewmode);
				break;
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SELECTMODE:
			BuildViewModeMenu();

			RECT rc;
			GetWindowRect((HWND)lParam, &rc);
			{
				POINT pt;
				pt.x = rc.left;
				pt.y = rc.bottom;
				int selection = TrackPopupMenu(hViewModeMenu, TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
				if (selection) {
					if (selection == 10001)
						goto clvm_config_command;
					else if (selection == 10002)
						goto clvm_reset_command;

					char szTemp[256];
					MENUITEMINFOA mii = { 0 };
					mii.cbSize = sizeof(mii);
					mii.fMask = MIIM_STRING;
					mii.dwTypeData = szTemp;
					mii.cch = 256;
					if (GetMenuItemInfoA(hViewModeMenu, selection, FALSE, &mii))
						ApplyViewMode(szTemp);
				}
			}
			break;

		case IDC_RESETMODES:
clvm_reset_command:
			cfg::dat.bFilterEffective = 0;
			Clist_Broadcast(CLM_AUTOREBUILD, 0, 0);
			SetDlgItemText(hwnd, IDC_SELECTMODE, TranslateT("No view mode"));
			g_clistApi.pfnSetHideOffline(cfg::dat.boldHideOffline);
			cfg::dat.boldHideOffline = (uint8_t)-1;
			SetButtonStates();
			cfg::dat.current_viewmode[0] = 0;
			cfg::dat.old_viewmode[0] = 0;
			g_plugin.setString("LastViewMode", "");
			break;

		case IDC_CONFIGUREMODES:
clvm_config_command:
			if (!g_ViewModeOptDlg)
				CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_OPT_VIEWMODES), nullptr, DlgProcViewModesSetup, 0);
			break;
		}
		break;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return TRUE;
}

HWND g_hwndViewModeFrame;

void CreateViewModeFrame()
{
	CLISTFrame frame = { 0 };
	WNDCLASS wndclass = { 0 };

	wndclass.style = 0;
	wndclass.lpfnWndProc = ViewModeFrameWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = g_plugin.getInst();
	wndclass.hIcon = nullptr;
	wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE);
	wndclass.lpszMenuName = nullptr;
	wndclass.lpszClassName = L"CLVMFrameWindow";
	RegisterClass(&wndclass);

	memset(&frame, 0, sizeof(frame));
	frame.cbSize = sizeof(frame);
	frame.szName.a = "View modes";
	frame.szTBname.a = LPGEN("View modes");
	frame.hIcon = Skin_LoadIcon(SKINICON_OTHER_FRAME);
	frame.height = 22;
	frame.Flags = F_VISIBLE | F_SHOWTBTIP | F_NOBORDER;
	frame.align = alBottom;
	frame.hWnd = CreateWindowEx(0, L"CLVMFrameWindow", L"CLVM", WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_CLIPCHILDREN, 0, 0, 20, 20, g_clistApi.hwndContactList, (HMENU)nullptr, g_plugin.getInst(), nullptr);
	g_hwndViewModeFrame = frame.hWnd;

	int hCLVMFrame = g_plugin.addFrame(&frame);
	CallService(MS_CLIST_FRAMES_UPDATEFRAME, hCLVMFrame, FU_FMPOS);
}

void ApplyViewMode(const char *name)
{
	cfg::dat.bFilterEffective = 0;

	if (name != nullptr) {
		char szSetting[256];
		mir_snprintf(szSetting, "%c%s_PF", 246, name);
		ptrA szProtos(db_get_sa(0, CLVM_MODULE, szSetting));
		if (mir_strlen(szProtos) >= 2) {
			strncpy_s(cfg::dat.protoFilter, szProtos, _TRUNCATE);
			cfg::dat.bFilterEffective |= CLVM_FILTER_PROTOS;
		}

		mir_snprintf(szSetting, "%c%s_GF", 246, name);
		ptrW tszGroups(db_get_wsa(0, CLVM_MODULE, szSetting));
		if (mir_wstrlen(tszGroups) >= 2) {
			wcsncpy_s(cfg::dat.groupFilter, tszGroups, _TRUNCATE);
			cfg::dat.bFilterEffective |= CLVM_FILTER_GROUPS;
		}

		mir_snprintf(szSetting, "%c%s_SM", 246, name);
		cfg::dat.statusMaskFilter = db_get_dw(0, CLVM_MODULE, szSetting, -1);
		if (cfg::dat.statusMaskFilter >= 1)
			cfg::dat.bFilterEffective |= CLVM_FILTER_STATUS;

		mir_snprintf(szSetting, "%c%s_SSM", 246, name);
		cfg::dat.stickyMaskFilter = db_get_dw(0, CLVM_MODULE, szSetting, -1);
		if (cfg::dat.stickyMaskFilter != -1)
			cfg::dat.bFilterEffective |= CLVM_FILTER_STICKYSTATUS;

		cfg::dat.filterFlags = db_get_dw(0, CLVM_MODULE, name, 0);

		KillTimer(g_hwndViewModeFrame, TIMERID_VIEWMODEEXPIRE);

		if (cfg::dat.filterFlags & CLVM_AUTOCLEAR) {
			uint32_t timerexpire;
			mir_snprintf(szSetting, "%c%s_OPT", 246, name);
			timerexpire = LOWORD(db_get_dw(0, CLVM_MODULE, szSetting, 0));
			strncpy(cfg::dat.old_viewmode, cfg::dat.current_viewmode, 256);
			cfg::dat.old_viewmode[255] = 0;
			SetTimer(g_hwndViewModeFrame, TIMERID_VIEWMODEEXPIRE, timerexpire * 1000, nullptr);
		}
		strncpy_s(cfg::dat.current_viewmode, name, _TRUNCATE);

		if (cfg::dat.filterFlags & CLVM_USELASTMSG) {
			uint8_t bSaved = cfg::dat.sortOrder[0];

			cfg::dat.sortOrder[0] = SORTBY_LASTMSG;
			for (auto &p : cfg::arCache)
				p->dwLastMsgTime = INTSORT_GetLastMsgTime(p->hContact);

			cfg::dat.sortOrder[0] = bSaved;

			cfg::dat.bFilterEffective |= CLVM_FILTER_LASTMSG;
			mir_snprintf(szSetting, "%c%s_LM", 246, name);
			cfg::dat.lastMsgFilter = db_get_dw(0, CLVM_MODULE, szSetting, 0);
			if (LOBYTE(HIWORD(cfg::dat.lastMsgFilter)))
				cfg::dat.bFilterEffective |= CLVM_FILTER_LASTMSG_NEWERTHAN;
			else
				cfg::dat.bFilterEffective |= CLVM_FILTER_LASTMSG_OLDERTHAN;

			uint32_t unit = LOWORD(cfg::dat.lastMsgFilter);
			switch (HIBYTE(HIWORD(cfg::dat.lastMsgFilter))) {
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
			cfg::dat.lastMsgFilter = unit;
		}
	}

	if (HIWORD(cfg::dat.filterFlags) > 0)
		cfg::dat.bFilterEffective |= CLVM_STICKY_CONTACTS;

	if (cfg::dat.boldHideOffline == (uint8_t)-1)
		cfg::dat.boldHideOffline = Clist::HideOffline;

	g_clistApi.pfnSetHideOffline(false);
	if (name == nullptr) {
		if (cfg::dat.current_viewmode[0])
			SetWindowTextA(hwndSelector, cfg::dat.current_viewmode);
		else
			SetWindowTextW(hwndSelector, TranslateT("No view mode"));
	}
	else SetWindowTextA(hwndSelector, name);
	Clist_Broadcast(CLM_AUTOREBUILD, 0, 0);
	SetButtonStates();

	g_plugin.setString("LastViewMode", cfg::dat.current_viewmode);
}
