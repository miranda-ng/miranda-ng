/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org),
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

#include "stdafx.h"
#include "cluiframes.h"
#include "m_skinbutton.h"

#define TIMERID_VIEWMODEEXPIRE 100

typedef int(__cdecl *pfnEnumCallback)(char *szName);
HWND  g_ViewModeOptDlg = nullptr;
HMENU hViewModeMenu = nullptr;

static HWND hwndSelector = nullptr;
static HIMAGELIST himlViewModes = nullptr;
static HANDLE hInfoItem = nullptr;
static int nullImage;
static DWORD stickyStatusMask = 0;
static char g_szModename[2048];

static UINT _page1Controls[] = 
{
	IDC_STATIC1, IDC_STATIC2, IDC_STATIC3, IDC_STATIC4,
	IDC_STATIC8, IDC_ADDVIEWMODE, IDC_RENAMEVIEWMODE, IDC_DELETEVIEWMODE, IDC_GROUPS, IDC_PROTOCOLS,
	IDC_VIEWMODES, IDC_STATUSMODES, IDC_STATIC12, IDC_STATIC13, IDC_STATIC14, IDC_PROTOGROUPOP, IDC_GROUPSTATUSOP,
	IDC_AUTOCLEAR, IDC_AUTOCLEARVAL, IDC_AUTOCLEARSPIN, IDC_STATIC15, IDC_STATIC16,
	IDC_LASTMESSAGEOP, IDC_LASTMESSAGEUNIT, IDC_LASTMSG, IDC_LASTMSGVALUE, IDC_USEGROUPS
};

static UINT _page2Controls[] = { IDC_CLIST, IDC_STATIC9, IDC_STATIC8, IDC_CLEARALL, IDC_CURVIEWMODE2 };

static UINT _buttons[] = { IDC_RESETMODES, IDC_SELECTMODE, IDC_CONFIGUREMODES };

static BOOL sttDrawViewModeBackground(HWND hwnd, HDC hdc, RECT *rect);
static void DeleteViewMode(char * szName);

/////////////////////////////////////////////////////////////////////////////////////////
// enumerate all view modes, call the callback function with the mode name
// useful for filling lists, menus and so on..

static int CLVM_EnumProc(const char *szSetting, void *lParam)
{
	pfnEnumCallback EnumCallback = (pfnEnumCallback)lParam;
	if (szSetting != nullptr)
		EnumCallback((char *)szSetting);
	return 0;
}

void CLVM_EnumModes(pfnEnumCallback EnumCallback)
{
	db_enum_settings(0, CLVM_EnumProc, CLVM_MODULE, EnumCallback);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int DeleteAutoModesCallback(char *szsetting)
{
	if (szsetting[0] == 13)
		DeleteViewMode(szsetting);
	return 1;
}

void SaveViewMode(const char *name, const wchar_t *szGroupFilter, const char *szProtoFilter, DWORD dwStatusMask, DWORD dwStickyStatusMask,
	unsigned int options, unsigned int stickies, unsigned int operators, unsigned int lmdat)
{
	CLVM_EnumModes(DeleteAutoModesCallback);

	char szSetting[512];
	mir_snprintf(szSetting, "%c%s_PF", 246, name);
	db_set_s(0, CLVM_MODULE, szSetting, szProtoFilter);
	mir_snprintf(szSetting, "%c%s_GF", 246, name);
	db_set_ws(0, CLVM_MODULE, szSetting, szGroupFilter);
	mir_snprintf(szSetting, "%c%s_SM", 246, name);
	db_set_dw(0, CLVM_MODULE, szSetting, dwStatusMask);
	mir_snprintf(szSetting, "%c%s_SSM", 246, name);
	db_set_dw(0, CLVM_MODULE, szSetting, dwStickyStatusMask);
	mir_snprintf(szSetting, "%c%s_OPT", 246, name);
	db_set_dw(0, CLVM_MODULE, szSetting, options);
	mir_snprintf(szSetting, "%c%s_LM", 246, name);
	db_set_dw(0, CLVM_MODULE, szSetting, lmdat);

	db_set_dw(0, CLVM_MODULE, name, MAKELONG((unsigned short)operators, (unsigned short)stickies));
}

/////////////////////////////////////////////////////////////////////////////////////////

void DeleteViewMode(char *szName)
{
	char szSetting[256];

	mir_snprintf(szSetting, "%c%s_PF", 246, szName);
	db_unset(0, CLVM_MODULE, szSetting);
	mir_snprintf(szSetting, "%c%s_GF", 246, szName);
	db_unset(0, CLVM_MODULE, szSetting);
	mir_snprintf(szSetting, "%c%s_SM", 246, szName);
	db_unset(0, CLVM_MODULE, szSetting);
	mir_snprintf(szSetting, "%c%s_VA", 246, szName);
	db_unset(0, CLVM_MODULE, szSetting);
	mir_snprintf(szSetting, "%c%s_SSM", 246, szName);
	db_unset(0, CLVM_MODULE, szSetting);
	mir_snprintf(szSetting, "%c%s_OPT", 246, szName);
	db_unset(0, CLVM_MODULE, szSetting);
	mir_snprintf(szSetting, "%c%s_LM", 246, szName);
	db_unset(0, CLVM_MODULE, szSetting);
	db_unset(0, CLVM_MODULE, szName);

	if (!mir_strcmp(g_CluiData.current_viewmode, szName) && mir_strlen(szName) == mir_strlen(g_CluiData.current_viewmode)) {
		g_CluiData.bFilterEffective = 0;
		Clist_Broadcast(CLM_AUTOREBUILD, 0, 0);
		SetWindowText(hwndSelector, TranslateT("All contacts"));
	}

	for (auto &hContact : Contacts())
		if (db_get_dw(hContact, CLVM_MODULE, szName, -1) != -1)
			db_set_dw(hContact, CLVM_MODULE, szName, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// View modes setup dialog

class CViewModeSetupDlg : public CDlgBase
{
	int m_iCurrItem = -1;

	CCtrlCheck chkLastMsg;
	CCtrlButton btnClearAll, btnApply;
	CCtrlMButton btnAdd, btnRename, btnDelete;
	CCtrlListBox modes;
	CCtrlListView protocols, groups, statuses;
	CCtrlClc clist;

	static int __cdecl FillModes(const char *szsetting, void *param)
	{
		if (BYTE(szsetting[0]) == 246)
			return 1;
		if (szsetting[0] == 13)
			return 1;

		ptrW temp(mir_utf8decodeW(szsetting));
		if (temp != nullptr) {
			CCtrlListBox *modes = (CCtrlListBox *)param;
			modes->AddString(temp);
		}
		return 1;
	}

	DWORD GetMaskForItem(HANDLE hItem)
	{
		DWORD dwMask = 0;

		for (int i = 0; i <= ID_STATUS_MAX - ID_STATUS_OFFLINE; i++)
			dwMask |= (clist.GetExtraImage(hItem, i) == nullImage ? 0 : 1 << i);

		return dwMask;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// fills dialog with data

	void FillDialog()
	{
		db_enum_settings(0, FillModes, CLVM_MODULE, &modes);

		// fill protocols...
		protocols.SetExtendedListViewStyle(LVS_EX_CHECKBOXES);

		LVCOLUMN lvc = {};
		lvc.mask = LVCF_FMT;
		lvc.fmt = LVCFMT_IMAGE | LVCFMT_LEFT;
		protocols.InsertColumn(0, &lvc);
		{
			LVITEM item = {};
			item.mask = LVIF_TEXT | LVIF_PARAM;
			item.iItem = 1000;
			for (auto &pa : Accounts()) {
				item.lParam = (LPARAM)pa->szModuleName;
				item.pszText = pa->tszAccountName;
				protocols.InsertItem(&item);
			}
		}
		protocols.SetColumnWidth(0, LVSCW_AUTOSIZE);
		protocols.Arrange(LVA_ALIGNLEFT | LVA_ALIGNTOP);

		// fill groups
		groups.SetExtendedListViewStyle(LVS_EX_CHECKBOXES);
		lvc.mask = LVCF_FMT;
		lvc.fmt = LVCFMT_IMAGE | LVCFMT_LEFT;
		groups.InsertColumn(0, &lvc);
		{
			LVITEM item = {};
			item.mask = LVIF_TEXT;
			item.iItem = 1000;
			item.pszText = TranslateT("Ungrouped contacts");
			groups.InsertItem(&item);

			wchar_t *szGroup;
			for (int i = 1; (szGroup = Clist_GroupGetName(i, nullptr)) != nullptr; i++) {
				item.pszText = szGroup;
				groups.InsertItem(&item);
			}
		}
		groups.SetColumnWidth(0, LVSCW_AUTOSIZE);
		groups.Arrange(LVA_ALIGNLEFT | LVA_ALIGNTOP);

		// fill statuses
		statuses.SetExtendedListViewStyle(LVS_EX_CHECKBOXES);

		lvc.mask = LVCF_FMT;
		lvc.fmt = LVCFMT_IMAGE | LVCFMT_LEFT;
		statuses.InsertColumn(0, &lvc);

		for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_MAX; i++) {
			LVITEM item = {};
			item.mask = LVIF_TEXT;
			item.pszText = Clist_GetStatusModeDescription(i, 0);
			item.iItem = i - ID_STATUS_OFFLINE;
			statuses.InsertItem(&item);
		}
		statuses.SetColumnWidth(0, LVSCW_AUTOSIZE);
		statuses.Arrange(LVA_ALIGNLEFT | LVA_ALIGNTOP);

		SendDlgItemMessage(m_hwnd, IDC_PROTOGROUPOP, CB_INSERTSTRING, -1, (LPARAM)TranslateT("And"));
		SendDlgItemMessage(m_hwnd, IDC_PROTOGROUPOP, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Or"));
		SendDlgItemMessage(m_hwnd, IDC_GROUPSTATUSOP, CB_INSERTSTRING, -1, (LPARAM)TranslateT("And"));
		SendDlgItemMessage(m_hwnd, IDC_GROUPSTATUSOP, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Or"));

		SendDlgItemMessage(m_hwnd, IDC_LASTMESSAGEOP, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Older than"));
		SendDlgItemMessage(m_hwnd, IDC_LASTMESSAGEOP, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Newer than"));

		SendDlgItemMessage(m_hwnd, IDC_LASTMESSAGEUNIT, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Minutes"));
		SendDlgItemMessage(m_hwnd, IDC_LASTMESSAGEUNIT, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Hours"));
		SendDlgItemMessage(m_hwnd, IDC_LASTMESSAGEUNIT, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Days"));
		SendDlgItemMessage(m_hwnd, IDC_LASTMESSAGEOP, CB_SETCURSEL, 0, 0);
		SendDlgItemMessage(m_hwnd, IDC_LASTMESSAGEUNIT, CB_SETCURSEL, 0, 0);
		SetDlgItemInt(m_hwnd, IDC_LASTMSGVALUE, 0, 0);
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// saves the state of the filter definitions for the current item

	void SaveState()
	{
		wchar_t newGroupFilter[2048] = L"|";
		char newProtoFilter[2048] = "|";
		DWORD statusMask = 0;
		DWORD operators = 0;

		if (m_iCurrItem == -1)
			return;

		for (int i = 0; i < protocols.GetItemCount(); i++) {
			if (protocols.GetCheckState(i)) {
				LVITEM item = {};
				item.mask = LVIF_PARAM;
				item.iItem = i;
				protocols.GetItem(&item);

				mir_strncat(newProtoFilter, (char *)item.lParam, _countof(newProtoFilter) - mir_strlen(newProtoFilter));
				mir_strncat(newProtoFilter, "|", _countof(newProtoFilter) - mir_strlen(newProtoFilter));
				newProtoFilter[2047] = 0;
			}
		}

		operators |= groups.GetCheckState(0) ? CLVM_INCLUDED_UNGROUPED : 0;

		for (int i = 0; i < groups.GetItemCount(); i++) {
			if (groups.GetCheckState(i)) {
				wchar_t szTemp[256];
				LVITEM item = {};
				item.mask = LVIF_TEXT;
				item.pszText = szTemp;
				item.cchTextMax = _countof(szTemp);
				item.iItem = i;
				groups.GetItem(&item);
				mir_wstrncat(newGroupFilter, szTemp, _countof(newGroupFilter) - mir_wstrlen(newGroupFilter));
				mir_wstrncat(newGroupFilter, L"|", _countof(newGroupFilter) - mir_wstrlen(newGroupFilter));
				newGroupFilter[2047] = 0;
			}
		}

		for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_MAX; i++)
			if (statuses.GetCheckState(i - ID_STATUS_OFFLINE))
				statusMask |= (1 << (i - ID_STATUS_OFFLINE));

		unsigned int stickies = 0;
		ptrW szTempModeName(modes.GetItemText(m_iCurrItem));
		if (szTempModeName) {
			T2Utf szModeName(szTempModeName);

			DWORD dwGlobalMask = GetMaskForItem(hInfoItem);
			for (auto &hContact : Contacts()) {
				HANDLE hItem = clist.FindContact(hContact);
				if (hItem == nullptr)
					continue;

				if (clist.GetCheck(hItem)) {
					DWORD dwLocalMask = GetMaskForItem(hItem);
					db_set_dw(hContact, CLVM_MODULE, szModeName, MAKELONG(1, (unsigned short)dwLocalMask));
					stickies++;
				}
				else {
					if (db_get_dw(hContact, CLVM_MODULE, szModeName, 0))
						db_set_dw(hContact, CLVM_MODULE, szModeName, 0);
				}
			}

			operators |= ((SendDlgItemMessage(m_hwnd, IDC_PROTOGROUPOP, CB_GETCURSEL, 0, 0) == 1 ? CLVM_PROTOGROUP_OP : 0) |
				(SendDlgItemMessage(m_hwnd, IDC_GROUPSTATUSOP, CB_GETCURSEL, 0, 0) == 1 ? CLVM_GROUPSTATUS_OP : 0) |
				(IsDlgButtonChecked(m_hwnd, IDC_AUTOCLEAR) ? CLVM_AUTOCLEAR : 0) |
				(chkLastMsg.GetState() ? CLVM_USELASTMSG : 0) |
				(IsDlgButtonChecked(m_hwnd, IDC_USEGROUPS) == BST_CHECKED ? CLVM_USEGROUPS : 0) |
				(IsDlgButtonChecked(m_hwnd, IDC_USEGROUPS) == BST_UNCHECKED ? CLVM_DONOTUSEGROUPS : 0));

			DWORD options = SendDlgItemMessage(m_hwnd, IDC_AUTOCLEARSPIN, UDM_GETPOS, 0, 0);

			BOOL translated;
			DWORD lmdat = MAKELONG(GetDlgItemInt(m_hwnd, IDC_LASTMSGVALUE, &translated, FALSE),
				MAKEWORD(SendDlgItemMessage(m_hwnd, IDC_LASTMESSAGEOP, CB_GETCURSEL, 0, 0),
				SendDlgItemMessage(m_hwnd, IDC_LASTMESSAGEUNIT, CB_GETCURSEL, 0, 0)));

			SaveViewMode(szModeName, newGroupFilter, newProtoFilter, statusMask, dwGlobalMask, options,
				stickies, operators, lmdat);
		}
		btnApply.Disable();
	}

	////////////////////////////////////////////////////////////////////////////////////////

	void SetAllChildIcons(HANDLE hFirstItem, int iColumn, int iImage)
	{
		int typeOfFirst = clist.GetItemType(hFirstItem);

		// check groups
		HANDLE hItem = (typeOfFirst == CLCIT_GROUP) ? hFirstItem : clist.GetNextItem(hFirstItem, CLGN_NEXTGROUP);
		while (hItem) {
			HANDLE hChildItem = clist.GetNextItem(hItem, CLGN_CHILD);
			if (hChildItem)
				SetAllChildIcons(hChildItem, iColumn, iImage);
			hItem = clist.GetNextItem(hItem, CLGN_NEXTGROUP);
		}

		// check contacts
		if (typeOfFirst == CLCIT_CONTACT)
			hItem = hFirstItem;
		else
			hItem = clist.GetNextItem(hFirstItem, CLGN_NEXTCONTACT);

		while (hItem) {
			int iOldIcon = clist.GetExtraImage(hItem, iColumn);
			if (iOldIcon != EMPTY_EXTRA_ICON && iOldIcon != iImage)
				clist.SetExtraImage(hItem, iColumn, iImage);
			hItem = clist.GetNextItem(hItem, CLGN_NEXTCONTACT);
		}
	}

	void SetIconsForColumn(HANDLE hItem, HANDLE hItemAll, int iColumn, int iImage)
	{
		int itemType = clist.GetItemType(hItem);
		if (itemType == CLCIT_CONTACT) {
			int oldiImage = clist.GetExtraImage(hItem, iColumn);
			if (oldiImage != EMPTY_EXTRA_ICON && oldiImage != iImage)
				clist.SetExtraImage(hItem, iColumn, iImage);
		}
		else if (itemType == CLCIT_INFO) {
			int oldiImage = clist.GetExtraImage(hItem, iColumn);
			if (oldiImage != EMPTY_EXTRA_ICON && oldiImage != iImage)
				clist.SetExtraImage(hItem, iColumn, iImage);
			if (hItem == hItemAll)
				SetAllChildIcons(hItem, iColumn, iImage);
			else
				clist.SetExtraImage(hItem, iColumn, iImage);
		}
		else if (itemType == CLCIT_GROUP) {
			int oldiImage = clist.GetExtraImage(hItem, iColumn);
			if (oldiImage != EMPTY_EXTRA_ICON && oldiImage != iImage)
				clist.SetExtraImage(hItem, iColumn, iImage);

			hItem = clist.GetNextItem(hItem, CLGN_CHILD);
			if (hItem)
				SetAllChildIcons(hItem, iColumn, iImage);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////

	void ShowPage(int page)
	{
		int pageChange = 0;

		if (page == 0 && IsWindowVisible(GetDlgItem(m_hwnd, _page2Controls[0])))
			pageChange = 1;

		if (page == 1 && IsWindowVisible(GetDlgItem(m_hwnd, _page1Controls[0])))
			pageChange = 1;

		if (pageChange)
			SetDraw(false);

		switch (page) {
		case 0:
			for (auto &ctrlId : _page1Controls)
				::ShowWindow(GetDlgItem(m_hwnd, ctrlId), SW_SHOW);

			for (auto &ctrlId : _page2Controls)
				::ShowWindow(GetDlgItem(m_hwnd, ctrlId), SW_HIDE);
			break;

		case 1:
			for (auto &ctrlId : _page1Controls)
				::ShowWindow(GetDlgItem(m_hwnd, ctrlId), SW_HIDE);

			for (auto &ctrlId : _page2Controls)
				::ShowWindow(GetDlgItem(m_hwnd, ctrlId), SW_SHOW);
			break;
		}

		if (pageChange) {
			SetDraw(true);
			::RedrawWindow(m_hwnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////

	void UpdateClistItem(HANDLE hItem, DWORD mask)
	{
		for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_MAX; i++)
			clist.SetExtraImage(hItem, i - ID_STATUS_OFFLINE, (1 << (i - ID_STATUS_OFFLINE)) & mask ? i - ID_STATUS_OFFLINE : nullImage);
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// updates the filter list boxes with the data taken from the filtering string

	void UpdateFilters()
	{
		char szSetting[128];
		DWORD dwFlags;
		DWORD opt;

		if (m_iCurrItem == LB_ERR)
			return;

		ptrW szTempBuf(modes.GetItemText(m_iCurrItem));
		T2Utf szBuf(szTempBuf);
		mir_strncpy(g_szModename, szBuf, _countof(g_szModename));
		{
			wchar_t szTemp[100];
			mir_snwprintf(szTemp, TranslateT("Configuring view mode: %s"), szTempBuf);
			SetDlgItemText(m_hwnd, IDC_CURVIEWMODE2, szTemp);
		}
		mir_snprintf(szSetting, "%c%s_PF", 246, szBuf.get());
		ptrA szPF(db_get_sa(0, CLVM_MODULE, szSetting));
		if (szPF == nullptr)
			return;

		mir_snprintf(szSetting, "%c%s_GF", 246, szBuf.get());
		ptrW szGF(db_get_wsa(0, CLVM_MODULE, szSetting));
		if (szGF == nullptr)
			return;

		mir_snprintf(szSetting, "%c%s_OPT", 246, szBuf.get());
		if ((opt = db_get_dw(0, CLVM_MODULE, szSetting, -1)) != -1)
			SendDlgItemMessage(m_hwnd, IDC_AUTOCLEARSPIN, UDM_SETPOS, 0, MAKELONG(LOWORD(opt), 0));

		mir_snprintf(szSetting, "%c%s_SM", 246, szBuf.get());
		DWORD statusMask = db_get_dw(0, CLVM_MODULE, szSetting, 0);
		mir_snprintf(szSetting, "%c%s_SSM", 246, szBuf.get());
		stickyStatusMask = db_get_dw(0, CLVM_MODULE, szSetting, -1);
		dwFlags = db_get_dw(0, CLVM_MODULE, szBuf, 0);
		{
			char szMask[256];

			LVITEM item = { 0 };
			item.mask = LVIF_PARAM;

			for (int i = 0; i < protocols.GetItemCount(); i++) {
				item.iItem = i;
				protocols.GetItem(&item);
				mir_snprintf(szMask, "%s|", (char *)item.lParam);
				if (szPF && strstr(szPF, szMask))
					protocols.SetCheckState(i, TRUE);
				else
					protocols.SetCheckState(i, FALSE);
			}
		}
		{
			wchar_t szTemp[256];
			wchar_t szMask[256];

			LVITEM item = {};
			item.mask = LVIF_TEXT;
			item.pszText = szTemp;
			item.cchTextMax = _countof(szTemp);

			groups.SetCheckState(0, (dwFlags & CLVM_INCLUDED_UNGROUPED) ? TRUE : FALSE);

			for (int i = 1; i < groups.GetItemCount(); i++) {
				item.iItem = i;
				groups.GetItem(&item);
				mir_snwprintf(szMask, L"%s|", szTemp);
				if (szGF && wcsstr(szGF, szMask))
					groups.SetCheckState(i, TRUE);
				else
					groups.SetCheckState(i, FALSE);
			}
		}

		for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_MAX; i++) {
			if ((1 << (i - ID_STATUS_OFFLINE)) & statusMask)
				statuses.SetCheckState(i - ID_STATUS_OFFLINE, TRUE);
			else
				statuses.SetCheckState(i - ID_STATUS_OFFLINE, FALSE);
		}

		SendDlgItemMessage(m_hwnd, IDC_PROTOGROUPOP, CB_SETCURSEL, dwFlags & CLVM_PROTOGROUP_OP ? 1 : 0, 0);
		SendDlgItemMessage(m_hwnd, IDC_GROUPSTATUSOP, CB_SETCURSEL, dwFlags & CLVM_GROUPSTATUS_OP ? 1 : 0, 0);
		CheckDlgButton(m_hwnd, IDC_AUTOCLEAR, dwFlags & CLVM_AUTOCLEAR ? BST_CHECKED : BST_UNCHECKED);
		onListRebuilt_Clist(0);

		int useLastMsg = dwFlags & CLVM_USELASTMSG;
		int useGroupsState = (dwFlags & CLVM_USEGROUPS) ? BST_CHECKED : (dwFlags & CLVM_DONOTUSEGROUPS) ? BST_UNCHECKED : BST_INDETERMINATE;

		chkLastMsg.SetState(useLastMsg);
		CheckDlgButton(m_hwnd, IDC_USEGROUPS, useGroupsState ? BST_CHECKED : BST_UNCHECKED);

		EnableWindow(GetDlgItem(m_hwnd, IDC_LASTMESSAGEOP), useLastMsg);
		EnableWindow(GetDlgItem(m_hwnd, IDC_LASTMSGVALUE), useLastMsg);
		EnableWindow(GetDlgItem(m_hwnd, IDC_LASTMESSAGEUNIT), useLastMsg);

		mir_snprintf(szSetting, "%c%s_LM", 246, szBuf.get());
		DWORD lmdat = db_get_dw(0, CLVM_MODULE, szSetting, 0);

		SetDlgItemInt(m_hwnd, IDC_LASTMSGVALUE, LOWORD(lmdat), FALSE);
		BYTE bTmp = LOBYTE(HIWORD(lmdat));
		SendDlgItemMessage(m_hwnd, IDC_LASTMESSAGEOP, CB_SETCURSEL, bTmp, 0);
		bTmp = HIBYTE(HIWORD(lmdat));
		SendDlgItemMessage(m_hwnd, IDC_LASTMESSAGEUNIT, CB_SETCURSEL, bTmp, 0);

		ShowPage(0);
	}

public:
	CViewModeSetupDlg() :
		CDlgBase(g_plugin, IDD_OPT_VIEWMODES),
		clist(this, IDC_CLIST),
		modes(this, IDC_VIEWMODES),
		groups(this, IDC_GROUPS),
		statuses(this, IDC_STATUSMODES),
		protocols(this, IDC_PROTOCOLS),
		btnAdd(this, IDC_ADDVIEWMODE, SKINICON_OTHER_ADDCONTACT, LPGEN("Add view mode")),
		btnRename(this, IDC_RENAMEVIEWMODE, SKINICON_OTHER_RENAME, LPGEN("Rename view mode")),
		btnDelete(this, IDC_DELETEVIEWMODE, SKINICON_OTHER_DELETE, LPGEN("Remove view mode")),
		btnApply(this, IDC_APPLY),
		btnClearAll(this, IDC_CLEARALL),
		chkLastMsg(this, IDC_LASTMSG)
	{
		btnAdd.OnClick = Callback(this, &CViewModeSetupDlg::onClick_Add);
		btnApply.OnClick = Callback(this, &CViewModeSetupDlg::onClick_Apply);
		btnRename.OnClick = Callback(this, &CViewModeSetupDlg::onClick_Rename);
		btnDelete.OnClick = Callback(this, &CViewModeSetupDlg::onClick_Delete);
		btnClearAll.OnClick = Callback(this, &CViewModeSetupDlg::onClick_ClearAll);

		chkLastMsg.OnChange = Callback(this, &CViewModeSetupDlg::onChange_LastMsg);

		modes.OnSelChange = Callback(this, &CViewModeSetupDlg::onSelChange_List);

		clist.OnListRebuilt = Callback(this, &CViewModeSetupDlg::onListRebuilt_Clist);
	}

	bool OnInitDialog() override
	{
		g_ViewModeOptDlg = m_hwnd;
		xpt_EnableThemeDialogTexture(m_hwnd, ETDT_ENABLETAB);
		himlViewModes = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 12, 0);

		for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_MAX; i++) {
			HICON hIcon = Skin_LoadProtoIcon(nullptr, i);
			ImageList_AddIcon(himlViewModes, hIcon);
			IcoLib_ReleaseIcon(hIcon);
		}

		HICON hIcon = (HICON)LoadImage(g_hMirApp, MAKEINTRESOURCE(IDI_SMALLDOT), IMAGE_ICON, 16, 16, 0);
		nullImage = ImageList_AddIcon(himlViewModes, hIcon);
		DestroyIcon(hIcon);

		RECT rcClient;
		GetClientRect(m_hwnd, &rcClient);

		TCITEM tci;
		tci.mask = TCIF_PARAM | TCIF_TEXT;
		tci.lParam = 0;
		tci.pszText = TranslateT("Sticky contacts");
		SendDlgItemMessage(m_hwnd, IDC_TAB, TCM_INSERTITEM, 0, (LPARAM)&tci);

		tci.pszText = TranslateT("Filtering");
		SendDlgItemMessage(m_hwnd, IDC_TAB, TCM_INSERTITEM, 0, (LPARAM)&tci);

		TabCtrl_SetCurSel(GetDlgItem(m_hwnd, IDC_TAB), 0);

		FillDialog();

		SetWindowLong(clist.GetHwnd(), GWL_STYLE, GetWindowLong(clist.GetHwnd(), GWL_STYLE) & ~CLS_SHOWHIDDEN);
		clist.SetExtraImageList(himlViewModes);
		clist.SetExtraColumns(ID_STATUS_MAX - ID_STATUS_OFFLINE);
		clist.SetHideEmptyGroups(true);

		CLCINFOITEM cii = { sizeof(cii) };
		cii.hParentGroup = nullptr;
		cii.pszText = TranslateT("*** All contacts ***");
		hInfoItem = clist.AddInfoItem(&cii);

		int index = 0;

		if (g_CluiData.current_viewmode[0] != '\0') {
			wchar_t *temp = mir_utf8decodeW(g_CluiData.current_viewmode);
			if (temp) {
				index = modes.FindString(temp);
				mir_free(temp);
			}
			if (index == -1)
				index = 0;
		}

		if (modes.SetCurSel(0) != LB_ERR) {
			m_iCurrItem = index;
			UpdateFilters();
		}
		else m_iCurrItem = -1;

		for (auto &ctrlId : _page2Controls)
			ShowWindow(GetDlgItem(m_hwnd, ctrlId), SW_HIDE);
		ShowWindow(m_hwnd, SW_SHOWNORMAL);
		btnApply.Disable();
		SendDlgItemMessage(m_hwnd, IDC_AUTOCLEARSPIN, UDM_SETRANGE, 0, MAKELONG(1000, 0));
		SetWindowText(m_hwnd, TranslateT("Configure view modes"));
		return true;
	}

	void OnChange() override
	{
		btnApply.Enable();
	}

	bool OnApply() override
	{
		SaveState();
		if (g_CluiData.bFilterEffective)
			ApplyViewMode(g_CluiData.current_viewmode);
		return true;
	}

	void OnDestroy() override
	{
		ImageList_RemoveAll(himlViewModes);
		ImageList_Destroy(himlViewModes);
		g_ViewModeOptDlg = nullptr;
	}

	void onChange_LastMsg(CCtrlCheck *)
	{
		bool bUseLastMsg = chkLastMsg.GetState();
		EnableWindow(GetDlgItem(m_hwnd, IDC_LASTMESSAGEOP), bUseLastMsg);
		EnableWindow(GetDlgItem(m_hwnd, IDC_LASTMESSAGEUNIT), bUseLastMsg);
		EnableWindow(GetDlgItem(m_hwnd, IDC_LASTMSGVALUE), bUseLastMsg);
		btnApply.Enable();
	}

	void onClick_Add(CCtrlButton *)
	{
		ENTER_STRING es = {};
		es.caption = LPGENW("Enter view mode name");
		if (!EnterString(&es))
			return;

		T2Utf szUTF8Buf(es.ptszResult);

		if (db_get_dw(0, CLVM_MODULE, szUTF8Buf, -1) != -1)
			MessageBox(nullptr, TranslateT("A view mode with this name does already exist"), TranslateT("Duplicate name"), MB_OK);
		else {
			int iNewItem = modes.AddString(es.ptszResult);
			if (iNewItem != LB_ERR) {
				modes.SetCurSel(iNewItem);
				SaveViewMode(szUTF8Buf, L"", "", 0, -1, 0, 0, 0, 0);
				m_iCurrItem = iNewItem;
				onListRebuilt_Clist(0);
				SendDlgItemMessage(m_hwnd, IDC_PROTOGROUPOP, CB_SETCURSEL, 0, 0);
				SendDlgItemMessage(m_hwnd, IDC_GROUPSTATUSOP, CB_SETCURSEL, 0, 0);
			}
		}

		UpdateFilters();
		ShowPage(0);
	}

	void onClick_Rename(CCtrlButton *)
	{
		ENTER_STRING es = {};
		es.caption = LPGENW("Enter new view mode name");
		if (!EnterString(&es))
			return;

		int idx = modes.GetCurSel();
		ptrW szTempBuf(modes.GetItemText(idx));
		if (!mir_wstrlen(szTempBuf))
			return;

		DeleteViewMode(T2Utf(szTempBuf));
		modes.DeleteString(idx);
		modes.InsertString(es.ptszResult, idx);
		modes.SetCurSel(idx);
		SaveState();
	}

	void onClick_Delete(CCtrlButton *)
	{
		if (IDYES != MessageBox(nullptr, TranslateT("Really delete this view mode? This cannot be undone"), TranslateT("Delete a view mode"), MB_YESNO | MB_ICONQUESTION))
			return;

		int idx = modes.GetCurSel();
		ptrW szTempBuf(modes.GetItemText(idx));
		if (!mir_wstrlen(szTempBuf))
			return;

		DeleteViewMode(T2Utf(szTempBuf));

		modes.DeleteString(idx);
		if (modes.SetCurSel(0) != LB_ERR) {
			m_iCurrItem = 0;
			UpdateFilters();
		}
		else m_iCurrItem = -1;
	}

	void onClick_Apply(CCtrlButton *)
	{
		OnApply();
	}

	void onClick_ClearAll(CCtrlButton*)
	{
		for (auto &hContact : Contacts()) {
			HANDLE hItem = clist.FindContact(hContact);
			if (hItem)
				clist.SetCheck(hItem, 0);
		}
	}

	void onSelChange_List(CCtrlListBox*)
	{
		SaveState();
		m_iCurrItem = modes.GetCurSel();
		UpdateFilters();
	}

	void onListRebuilt_Clist(CCtrlClc::TEventInfo *)
	{
		for (auto &hContact : Contacts()) {
			HANDLE hItem = clist.FindContact(hContact);
			if (hItem)
				clist.SetCheck(hItem, db_get_dw(hContact, CLVM_MODULE, g_szModename, 0));

			DWORD localMask = HIWORD(db_get_dw(hContact, CLVM_MODULE, g_szModename, 0));
			UpdateClistItem(hItem, (localMask == 0 || localMask == stickyStatusMask) ? stickyStatusMask : localMask);
		}

		for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_MAX; i++)
			clist.SetExtraImage(hInfoItem, i - ID_STATUS_OFFLINE, (1 << (i - ID_STATUS_OFFLINE)) & stickyStatusMask ? i - ID_STATUS_OFFLINE : MAX_STATUS_COUNT);

		HANDLE hItem = clist.GetNextItem(0, CLGN_ROOT);
		hItem = clist.GetNextItem(hItem, CLGN_NEXTGROUP);
		while (hItem) {
			for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_MAX; i++)
				clist.SetExtraImage(hItem, i - ID_STATUS_OFFLINE, nullImage);
			hItem = clist.GetNextItem(hItem, CLGN_NEXTGROUP);
		}
	}

	void onClick_Clist(CCtrlClc::TEventInfo *ev)
	{
		NMCLISTCONTROL *nm = (NMCLISTCONTROL *)ev->info;
		if (nm->iColumn == -1)
			return;

		DWORD hitFlags;
		HANDLE hItem = clist.HitTest(nm->pt.x, nm->pt.y, &hitFlags);
		if (hItem == nullptr)
			return;

		if (!(hitFlags & CLCHT_ONITEMEXTRA))
			return;

		int iImage = clist.GetExtraImage(hItem, nm->iColumn);
		if (iImage == nullImage)
			iImage = nm->iColumn;
		else if (iImage != EMPTY_EXTRA_ICON)
			iImage = nullImage;
		SetIconsForColumn(hItem, hInfoItem, nm->iColumn, iImage);
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_NOTIFY) {
			LPNMHDR hdr = (LPNMHDR)lParam;
			if (hdr->code == TCN_SELCHANGE && hdr->idFrom == IDC_TAB) {
				int id = TabCtrl_GetCurSel(GetDlgItem(m_hwnd, IDC_TAB));
				if (id == 0)
					ShowPage(0);
				else
					ShowPage(1);
				return 0;
			}
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

static int menuCounter = 0;

static int FillMenuCallback(char *szSetting)
{
	if (BYTE(szSetting[0]) == 246)
		return 1;
	if (szSetting[0] == 13)
		return 1;

	wchar_t *temp = mir_utf8decodeW(szSetting);
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

	AppendMenu(hViewModeMenu, MF_SEPARATOR, 0, nullptr);

	CLVM_EnumModes(FillMenuCallback);

	if (GetMenuItemCount(hViewModeMenu) > 2)
		AppendMenu(hViewModeMenu, MF_SEPARATOR, 0, nullptr);
	AppendMenu(hViewModeMenu, MFT_STRING, 10001, TranslateT("Setup view modes..."));
}

/////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK ViewModeFrameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CREATE:
		{
			RECT rcMargins = { 12, 0, 2, 0 };
			hwndSelector = CreateWindow(MIRANDABUTTONCLASS, L"", BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 20, 20,
				hwnd, (HMENU)IDC_SELECTMODE, g_plugin.getInst(), nullptr);
			MakeButtonSkinned(hwndSelector);
			SendMessage(hwndSelector, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Select a view mode"), BATF_UNICODE);
			SendMessage(hwndSelector, BUTTONSETMARGINS, 0, (LPARAM)&rcMargins);
			SendMessage(hwndSelector, BUTTONSETID, 0, (LPARAM)"ViewMode.Select");
			SendMessage(hwndSelector, WM_SETFONT, 0, (LPARAM)FONTID_VIEMODES + 1);
			SendMessage(hwndSelector, BUTTONSETASFLATBTN, TRUE, 0);
			SendMessage(hwndSelector, MBM_UPDATETRANSPARENTFLAG, 0, 2);
			SendMessage(hwndSelector, BUTTONSETSENDONDOWN, 0, (LPARAM)1);

			// SendMessage(hwndSelector, BM_SETASMENUACTION, 1, 0);
			HWND hwndButton = CreateWindow(MIRANDABUTTONCLASS, L"", BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 20, 20,
				hwnd, (HMENU)IDC_CONFIGUREMODES, g_plugin.getInst(), nullptr);
			MakeButtonSkinned(hwndButton);
			SendMessage(hwndButton, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Setup view modes"), BATF_UNICODE);
			SendMessage(hwndButton, BUTTONSETID, 0, (LPARAM)"ViewMode.Setup");
			SendMessage(hwndButton, BUTTONSETASFLATBTN, TRUE, 0);
			SendMessage(hwndButton, MBM_UPDATETRANSPARENTFLAG, 0, 2);

			hwndButton = CreateWindow(MIRANDABUTTONCLASS, L"", BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 20, 20,
				hwnd, (HMENU)IDC_RESETMODES, g_plugin.getInst(), nullptr);
			MakeButtonSkinned(hwndButton);
			SendMessage(hwndButton, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Clear view mode and return to default display"), BATF_UNICODE);
			SendMessage(hwndButton, BUTTONSETID, 0, (LPARAM)"ViewMode.Clear");
			SendMessage(hwnd, WM_USER + 100, 0, 0);
			SendMessage(hwndButton, BUTTONSETASFLATBTN, TRUE, 0);
			SendMessage(hwndButton, MBM_UPDATETRANSPARENTFLAG, 0, 2);
		}
		return FALSE;

	case WM_NCCALCSIZE:
		return 18; // FrameNCCalcSize(hwnd, DefWindowProc, wParam, lParam, hasTitleBar);

	case WM_SIZE:
		{
			RECT rcCLVMFrame;
			HDWP PosBatch = BeginDeferWindowPos(3);
			GetClientRect(hwnd, &rcCLVMFrame);
			PosBatch = DeferWindowPos(PosBatch, GetDlgItem(hwnd, IDC_RESETMODES), nullptr,
				rcCLVMFrame.right - 23, 1, 22, 18, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
			PosBatch = DeferWindowPos(PosBatch, GetDlgItem(hwnd, IDC_CONFIGUREMODES), nullptr,
				rcCLVMFrame.right - 45, 1, 22, 18, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
			PosBatch = DeferWindowPos(PosBatch, GetDlgItem(hwnd, IDC_SELECTMODE), nullptr,
				1, 1, rcCLVMFrame.right - 46, 18, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
			EndDeferWindowPos(PosBatch);
		}
		break;

	case WM_USER + 100:
		SendDlgItemMessage(hwnd, IDC_RESETMODES, MBM_SETICOLIBHANDLE, 0,
			(LPARAM)RegisterIcolibIconHandle("CLN_CLVM_reset", LPGEN("Contact list"), LPGEN("Reset view mode"), nullptr, 0, g_plugin.getInst(), IDI_RESETVIEW));

		SendDlgItemMessage(hwnd, IDC_CONFIGUREMODES, MBM_SETICOLIBHANDLE, 0,
			(LPARAM)RegisterIcolibIconHandle("CLN_CLVM_set", LPGEN("Contact list"), LPGEN("Setup view modes"), nullptr, 0, g_plugin.getInst(), IDI_SETVIEW));

		for (auto &btn : _buttons) {
			SendDlgItemMessage(hwnd, btn, BUTTONSETASFLATBTN, TRUE, 0);
			SendDlgItemMessage(hwnd, btn, BUTTONSETASFLATBTN + 10, 0, 0);
		}

		if (g_CluiData.bFilterEffective)
			SetDlgItemText(hwnd, IDC_SELECTMODE, ptrW(mir_utf8decodeW(g_CluiData.current_viewmode)));
		else
			SetDlgItemText(hwnd, IDC_SELECTMODE, TranslateT("All contacts"));
		break;

	case WM_ERASEBKGND:
		if (g_CluiData.fDisableSkinEngine)
			return sttDrawViewModeBackground(hwnd, (HDC)wParam, nullptr);
		return 0;

	case WM_NCPAINT:
	case WM_PAINT:
		if (GetParent(hwnd) == g_clistApi.hwndContactList && g_CluiData.fLayered)
			ValidateRect(hwnd, nullptr);

		else if (GetParent(hwnd) != g_clistApi.hwndContactList || !g_CluiData.fLayered) {
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
				if (GetParent(hwnd) != g_clistApi.hwndContactList) {
					HBRUSH br = GetSysColorBrush(COLOR_3DFACE);
					FillRect(hdc2, &rc, br);
				}
				else ske_BltBackImage(hwnd, hdc2, &rc);

				SkinDrawGlyph(hdc, &rc, &rc, "ViewMode,ID=Background");
			}

			for (auto &btn : _buttons) {
				RECT childRect, MyRect;
				GetWindowRect(hwnd, &MyRect);
				GetWindowRect(GetDlgItem(hwnd, btn), &childRect);

				POINT Offset;
				Offset.x = childRect.left - MyRect.left;
				Offset.y = childRect.top - MyRect.top;
				SendDlgItemMessage(hwnd, btn, BUTTONDRAWINPARENT, (WPARAM)hdc2, (LPARAM)&Offset);
			}

			BitBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdc2, rc.left, rc.top, SRCCOPY);
			SelectObject(hdc2, hbmpo);
			DeleteObject(hbmp);
			DeleteDC(hdc2);

			SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));

			ReleaseDC(hwnd, hdc);
			ValidateRect(hwnd, nullptr);
		}
		return 0;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == BUTTONNEEDREDRAW)
			g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
		return 0;

	case WM_TIMER:
		if (wParam == TIMERID_VIEWMODEEXPIRE) {
			RECT rcCLUI;
			GetWindowRect(g_clistApi.hwndContactList, &rcCLUI);

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

			RECT rc;
			GetWindowRect((HWND)lParam, &rc);
			{
				POINT pt = { rc.left, rc.bottom };
				int selection = TrackPopupMenu(hViewModeMenu, TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, GetParent(hwnd), nullptr);
				PostMessage(hwnd, WM_NULL, 0, 0);
				if (selection) {
					if (selection == 10001)
						goto clvm_config_command;
					if (selection == 10002)
						goto clvm_reset_command;

					wchar_t szTemp[256];
					MENUITEMINFO mii = { 0 };
					mii.cbSize = sizeof(mii);
					mii.fMask = MIIM_STRING;
					mii.dwTypeData = szTemp;
					mii.cch = 256;
					GetMenuItemInfo(hViewModeMenu, selection, FALSE, &mii);

					ApplyViewMode(T2Utf(szTemp));
				}
			}
			break;

		case IDC_RESETMODES:
clvm_reset_command:
			ApplyViewMode("");
			break;

		case IDC_CONFIGUREMODES:
clvm_config_command:
			if (!g_ViewModeOptDlg)
				(new CViewModeSetupDlg())->Create();
			break;
		}

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return TRUE;
}

static int hCLVMFrame;
HWND g_hwndViewModeFrame;

struct view_mode_t
{
	HBITMAP  hBmpBackground;
	COLORREF bkColour;
	int      useWinColors;
	int      backgroundBmpUse;

	view_mode_t() :
		hBmpBackground(nullptr),
		bkColour(CLCDEFAULT_BKCOLOUR),
		useWinColors(CLCDEFAULT_USEWINDOWSCOLOURS),
		backgroundBmpUse(CLCDEFAULT_USEBITMAP)
	{}
};

static view_mode_t view_mode;

static BOOL sttDrawViewModeBackground(HWND hwnd, HDC hdc, RECT *rect)
{
	BOOL bFloat = (GetParent(hwnd) != g_clistApi.hwndContactList);
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

static int ehhViewModeBackgroundSettingsChanged(WPARAM, LPARAM)
{
	if (view_mode.hBmpBackground) {
		DeleteObject(view_mode.hBmpBackground);
		view_mode.hBmpBackground = nullptr;
	}

	if (g_CluiData.fDisableSkinEngine) {
		view_mode.bkColour = cliGetColor("ViewMode", "BkColour", CLCDEFAULT_BKCOLOUR);
		if (db_get_b(0, "ViewMode", "UseBitmap", CLCDEFAULT_USEBITMAP)) {
			ptrW tszBitmapName(db_get_wsa(0, "ViewMode", "BkBitmap"));
			if (tszBitmapName)
				view_mode.hBmpBackground = Bitmap_Load(tszBitmapName);
		}
		view_mode.useWinColors = db_get_b(0, "ViewMode", "UseWinColours", CLCDEFAULT_USEWINDOWSCOLOURS);
		view_mode.backgroundBmpUse = db_get_w(0, "ViewMode", "BkBmpUse", CLCDEFAULT_BKBMPUSE);
	}
	PostMessage(g_clistApi.hwndContactList, WM_SIZE, 0, 0);
	return 0;
}

static int ViewModePaintCallbackProc(HWND hWnd, HDC hDC, RECT *, HRGN, DWORD, void *)
{
	RECT MyRect = { 0 };
	GetWindowRect(hWnd, &MyRect);
	SkinDrawGlyph(hDC, &MyRect, &MyRect, "ViewMode,ID=Background");

	for (auto &btn : _buttons) {
		RECT childRect;
		GetWindowRect(GetDlgItem(hWnd, btn), &childRect);

		POINT Offset;
		Offset.x = childRect.left - MyRect.left;
		Offset.y = childRect.top - MyRect.top;
		SendDlgItemMessage(hWnd, btn, BUTTONDRAWINPARENT, (WPARAM)hDC, (LPARAM)&Offset);
	}
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
	wndclass.hInstance = g_plugin.getInst();
	wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_3DFACE);
	wndclass.lpszMenuName = nullptr;
	wndclass.lpszClassName = L"CLVMFrameWindow";
	RegisterClass(&wndclass);

	CLISTFrame frame = { 0 };
	frame.cbSize = sizeof(frame);
	frame.szName.a = frame.szTBname.a = LPGEN("View modes");
	frame.hIcon = Skin_LoadIcon(SKINICON_OTHER_FRAME);
	frame.height = 18;
	frame.Flags = F_VISIBLE | F_SHOWTBTIP | F_NOBORDER | F_NO_SUBCONTAINER;
	frame.align = alBottom;
	frame.hWnd = CreateWindowEx(0, L"CLVMFrameWindow", _A2W(CLVM_MODULE), WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_CLIPCHILDREN, 0, 0, 20, 20, g_clistApi.hwndContactList, (HMENU)nullptr, g_plugin.getInst(), nullptr);
	g_hwndViewModeFrame = frame.hWnd;
	hCLVMFrame = g_plugin.addFrame(&frame);
	CallService(MS_CLIST_FRAMES_UPDATEFRAME, hCLVMFrame, FU_FMPOS);

	CallService(MS_SKINENG_REGISTERPAINTSUB, (WPARAM)frame.hWnd, (LPARAM)ViewModePaintCallbackProc); //$$$$$ register sub for frame

	ApplyViewMode(nullptr); // Apply last selected view mode
}

void ApplyViewMode(const char *szName)
{
	DBVARIANT dbv = { 0 };

	g_CluiData.bFilterEffective = 0;

	char szSetting[256];
	mir_snprintf(szSetting, "%c_LastMode", 246);

	if (szName == nullptr) { // Name is null - apply last stored view mode
		ptrA szStoredName(db_get_sa(0, CLVM_MODULE, szSetting));
		if (szStoredName == nullptr)
			return;

		szName = NEWSTR_ALLOCA(szStoredName);
	}

	if (szName[0] == '\0') {
		// Reset View Mode
		g_CluiData.bFilterEffective = 0;

		// remove last applied view mode
		mir_snprintf(szSetting, "%c_LastMode", 246);
		db_unset(0, CLVM_MODULE, szSetting);

		if (g_CluiData.bOldUseGroups != (BYTE)-1)
			CallService(MS_CLIST_SETUSEGROUPS, (WPARAM)g_CluiData.bOldUseGroups, 0);

		Clist_Broadcast(CLM_AUTOREBUILD, 0, 0);
		KillTimer(g_hwndViewModeFrame, TIMERID_VIEWMODEEXPIRE);

		if (g_CluiData.boldHideOffline != (BYTE)-1)
			g_clistApi.pfnSetHideOffline(g_CluiData.boldHideOffline);
		if (g_CluiData.bOldUseGroups != (BYTE)-1)
			CallService(MS_CLIST_SETUSEGROUPS, (WPARAM)g_CluiData.bOldUseGroups, 0);
		g_CluiData.boldHideOffline = (BYTE)-1;
		g_CluiData.bOldUseGroups = (BYTE)-1;
		g_CluiData.current_viewmode[0] = 0;
		g_CluiData.old_viewmode[0] = 0;

		SetWindowText(hwndSelector, TranslateT("All contacts"));
	}
	else {
		mir_snprintf(szSetting, "%c%s_PF", 246, szName);
		ptrA szFilter(db_get_sa(0, CLVM_MODULE, szSetting));
		if (mir_strlen(szFilter) >= 2) {
			strncpy_s(g_CluiData.protoFilter, szFilter.get(), _TRUNCATE);
			g_CluiData.bFilterEffective |= CLVM_FILTER_PROTOS;
		}

		mir_snprintf(szSetting, "%c%s_GF", 246, szName);
		ptrW wszGroupFilter(db_get_wsa(0, CLVM_MODULE, szSetting));
		if (mir_wstrlen(wszGroupFilter) >= 2) {
			wcsncpy_s(g_CluiData.groupFilter, wszGroupFilter.get(), _TRUNCATE);
			g_CluiData.bFilterEffective |= CLVM_FILTER_GROUPS;
		}
		mir_free(dbv.pwszVal);

		mir_snprintf(szSetting, "%c%s_SM", 246, szName);
		g_CluiData.statusMaskFilter = db_get_dw(0, CLVM_MODULE, szSetting, -1);
		if (g_CluiData.statusMaskFilter >= 1)
			g_CluiData.bFilterEffective |= CLVM_FILTER_STATUS;

		mir_snprintf(szSetting, "%c%s_SSM", 246, szName);
		g_CluiData.stickyMaskFilter = db_get_dw(0, CLVM_MODULE, szSetting, -1);
		if (g_CluiData.stickyMaskFilter != -1)
			g_CluiData.bFilterEffective |= CLVM_FILTER_STICKYSTATUS;

		g_CluiData.filterFlags = db_get_dw(0, CLVM_MODULE, szName, 0);

		KillTimer(g_hwndViewModeFrame, TIMERID_VIEWMODEEXPIRE);

		if (g_CluiData.filterFlags & CLVM_AUTOCLEAR) {
			mir_snprintf(szSetting, "%c%s_OPT", 246, szName);
			DWORD timerexpire = LOWORD(db_get_dw(0, CLVM_MODULE, szSetting, 0));
			strncpy_s(g_CluiData.old_viewmode, g_CluiData.current_viewmode, _TRUNCATE);
			CLUI_SafeSetTimer(g_hwndViewModeFrame, TIMERID_VIEWMODEEXPIRE, timerexpire * 1000, nullptr);
		}
		else { //store last selected view mode only if it is not autoclear
			mir_snprintf(szSetting, "%c_LastMode", 246);
			db_set_s(0, CLVM_MODULE, szSetting, szName);
		}
		strncpy_s(g_CluiData.current_viewmode, szName, _TRUNCATE);

		if (g_CluiData.filterFlags & CLVM_USELASTMSG) {
			g_CluiData.bFilterEffective |= CLVM_FILTER_LASTMSG;
			mir_snprintf(szSetting, "%c%s_LM", 246, szName);
			g_CluiData.lastMsgFilter = db_get_dw(0, CLVM_MODULE, szSetting, 0);
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
				g_CluiData.boldHideOffline = Clist::HideOffline;

			g_clistApi.pfnSetHideOffline(false);
		}
		else if (g_CluiData.boldHideOffline != (BYTE)-1) {
			g_clistApi.pfnSetHideOffline(g_CluiData.boldHideOffline);
			g_CluiData.boldHideOffline = -1;
		}

		int bUseGroups = -1;
		if (g_CluiData.filterFlags & CLVM_USEGROUPS)
			bUseGroups = 1;
		else if (g_CluiData.filterFlags & CLVM_DONOTUSEGROUPS)
			bUseGroups = 0;

		if (bUseGroups != -1) {
			if (g_CluiData.bOldUseGroups == (BYTE)-1)
				g_CluiData.bOldUseGroups = Clist::UseGroups;

			CallService(MS_CLIST_SETUSEGROUPS, bUseGroups, 0);
		}
		else if (g_CluiData.bOldUseGroups != (BYTE)-1) {
			CallService(MS_CLIST_SETUSEGROUPS, g_CluiData.bOldUseGroups, 0);
			g_CluiData.bOldUseGroups = -1;
		}

		SetWindowText(hwndSelector, ptrW(mir_utf8decodeW((szName[0] == 13) ? szName + 1 : szName)));
	}

	InvalidateRect(hwndSelector, NULL, TRUE);
	CallService(MS_CLIST_FRAMES_UPDATEFRAME, hCLVMFrame, -1);

	Clist_Broadcast(CLM_AUTOREBUILD, 0, 0);
	cliInvalidateRect(g_clistApi.hwndStatus, nullptr, FALSE);
}
