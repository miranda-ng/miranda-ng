/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
© 2004-2007 Victor Pavlychko
© 2010 MPK
© 2010 Merlin_de

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

static int ActionsSortFunc(const POPUPACTION *p1, const POPUPACTION *p2)
{
	return mir_strcmp(p1->lpzTitle, p2->lpzTitle);
}

static LIST<POPUPACTION> gActions(3, ActionsSortFunc);

// interface
void LoadActions()
{
	POPUPACTION actions[] =
	{
		{ sizeof(POPUPACTION), g_plugin.getIcon(IDI_ACT_REPLY), LPGEN("General") "/" LPGEN("Quick reply"), 0 },
		{ sizeof(POPUPACTION), g_plugin.getIcon(IDI_ACT_MESSAGE), LPGEN("General") "/" LPGEN("Send message"), 0 },
		{ sizeof(POPUPACTION), g_plugin.getIcon(IDI_ACT_INFO), LPGEN("General") "/" LPGEN("User details"), 0 },
		{ sizeof(POPUPACTION), g_plugin.getIcon(IDI_ACT_MENU), LPGEN("General") "/" LPGEN("Contact menu"), 0 },
		{ sizeof(POPUPACTION), g_plugin.getIcon(IDI_ACT_ADD), LPGEN("General") "/" LPGEN("Add permanently"), 0 },
		{ sizeof(POPUPACTION), g_plugin.getIcon(IDI_ACT_PIN), LPGEN("General") "/" LPGEN("Pin popup"), 0 },
		{ sizeof(POPUPACTION), g_plugin.getIcon(IDI_ACT_CLOSE), LPGEN("General") "/" LPGEN("Dismiss popup"), 0 },
		{ sizeof(POPUPACTION), g_plugin.getIcon(IDI_ACT_COPY), LPGEN("General") "/" LPGEN("Copy to clipboard"), 0 },
	};

	for (auto &it : actions)
		RegisterAction(&it);
}

void UnloadActions()
{
	for (auto &it : gActions)
		delete it;
	gActions.destroy();
}

void RegisterAction(POPUPACTION *action)
{
	int index;
	if ((index = gActions.getIndex(action)) >= 0) {
		uint32_t flags = gActions[index]->flags;
		*gActions[index] = *action;
		gActions[index]->flags = flags;
	}
	else {
		POPUPACTION *actionCopy = new POPUPACTION;
		*actionCopy = *action;
		actionCopy->flags = db_get_b(0, "PopupActions", actionCopy->lpzTitle, actionCopy->flags & PAF_ENABLED) ? PAF_ENABLED : 0;
		gActions.insert(actionCopy);
	}
}

bool IsActionEnabled(POPUPACTION *action)
{
	if (!(action->flags & PAF_ENABLED))
		return false;

	int index;
	if ((index = gActions.getIndex(action)) >= 0)
		if (!(gActions[index]->flags & PAF_ENABLED))
			return false;

	return true;
}

bool IsActionEnabled(char *name)
{
	POPUPACTION action = { 0 };
	action.flags = PAF_ENABLED;
	mir_strcpy(action.lpzTitle, name);
	return IsActionEnabled(&action);
}

uint32_t MouseOverride(HWND hCombo, int number)
{
	uint32_t dwItem = 0;
	uint32_t ItemActive = 0;
	if (number < 0 || number > 7)
		number = 0;
	dwItem = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TranslateT("No overwrite"));
	SendMessage(hCombo, CB_SETITEMDATA, dwItem, 0);
	if (number == 0)
		ItemActive = dwItem;
	dwItem = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TranslateT("Send message"));
	SendMessage(hCombo, CB_SETITEMDATA, dwItem, 1);
	if (number == 1)
		ItemActive = dwItem;
	dwItem = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TranslateT("Quick reply"));
	SendMessage(hCombo, CB_SETITEMDATA, dwItem, 2);
	if (number == 2)
		ItemActive = dwItem;
	dwItem = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TranslateT("User details"));
	SendMessage(hCombo, CB_SETITEMDATA, dwItem, 3);
	if (number == 3)
		ItemActive = dwItem;
	dwItem = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TranslateT("Contact menu"));
	SendMessage(hCombo, CB_SETITEMDATA, dwItem, 4);
	if (number == 4)
		ItemActive = dwItem;
	dwItem = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TranslateT("Dismiss popup"));
	SendMessage(hCombo, CB_SETITEMDATA, dwItem, 5);
	if (number == 5)
		ItemActive = dwItem;
	dwItem = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TranslateT("Pin popup"));
	SendMessage(hCombo, CB_SETITEMDATA, dwItem, 6);
	if (number == 6)
		ItemActive = dwItem;
	dwItem = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TranslateT("Copy to clipboard"));
	SendMessage(hCombo, CB_SETITEMDATA, dwItem, 7);
	if (number == 7)
		ItemActive = dwItem;
	return ItemActive;
}

// options

void LoadOption_Actions()
{
	PopupOptions.actions = g_plugin.getDword("Actions",
		ACT_ENABLE | ACT_RIGHTICONS | ACT_DEF_KEEPWND | ACT_DEF_IMONLY |
		ACT_DEF_NOGLOBAL | ACT_DEF_MESSAGE | ACT_DEF_DETAILS | ACT_DEF_MENU |
		ACT_DEF_ADD | ACT_DEF_DISMISS | ACT_DEF_PIN);
	PopupOptions.overrideLeft = g_plugin.getDword("OverrideLeft", 0);
	PopupOptions.overrideMiddle = g_plugin.getDword("OverrideMiddle", 0);
	PopupOptions.overrideRight = g_plugin.getDword("OverrideRight", 0);
}

static UINT controls[] =
{
	IDC_GRP_CUSTOMACTIONS,
	IDC_TXT_CUSTOMACTIONS,
	IDC_CHK_IMCONTACTSONLY,
	IDC_CHK_CONTACTSONLY,
	IDC_CHK_DONTCLOSE,
	IDC_GRP_SIZEPOSITION,
	IDC_CHK_LARGEICONS,
	IDC_TXT_POSITION,
	IDC_RD_TEXT,
	IDC_RD_LEFTICONS,
	IDC_RD_RIGHTICONS,
	IDC_GRP_ACTIONS,
	IDC_ACTIONS,
	IDC_GRP_SIZEPOSITION2,
	IDC_TXT_POSITION2,
	IDC_CB_LEFT,
	IDC_TXT_MIDDLE,
	IDC_CB_MIDDLE,
	IDC_TXT_RIGHT,
	IDC_CB_RIGHT
};

INT_PTR CALLBACK DlgProcPopupActions(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool windowInitialized = false;

	switch (msg) {
	case WM_INITDIALOG:
		windowInitialized = false;

		TranslateDialogDefault(hwnd);

		SendDlgItemMessage(hwnd, IDC_ICO_INFO, STM_SETICON, (WPARAM)g_plugin.getIcon(IDI_MB_INFO), 0);

		CheckDlgButton(hwnd, IDC_CHK_ENABLEACTIONS, PopupOptions.actions & ACT_ENABLE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CHK_IMCONTACTSONLY, PopupOptions.actions & ACT_DEF_IMONLY ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CHK_CONTACTSONLY, PopupOptions.actions & ACT_DEF_NOGLOBAL ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CHK_DONTCLOSE, PopupOptions.actions & ACT_DEF_KEEPWND ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CHK_LARGEICONS, PopupOptions.actions & ACT_LARGE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_RD_TEXT, PopupOptions.actions & ACT_TEXT ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_RD_LEFTICONS, PopupOptions.actions & ACT_LEFTICONS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_RD_RIGHTICONS, PopupOptions.actions & ACT_RIGHTICONS ? BST_CHECKED : BST_UNCHECKED);

		{
			uint32_t dwActiveItem = 0;
			HWND hCombo = GetDlgItem(hwnd, IDC_CB_LEFT);
			dwActiveItem = MouseOverride(hCombo, PopupOptions.overrideLeft);
			SendDlgItemMessage(hwnd, IDC_CB_LEFT, CB_SETCURSEL, dwActiveItem, 0);

			dwActiveItem = 0;
			hCombo = GetDlgItem(hwnd, IDC_CB_MIDDLE);
			dwActiveItem = MouseOverride(hCombo, PopupOptions.overrideMiddle);
			SendDlgItemMessage(hwnd, IDC_CB_MIDDLE, CB_SETCURSEL, dwActiveItem, 0);

			dwActiveItem = 0;
			hCombo = GetDlgItem(hwnd, IDC_CB_RIGHT);
			dwActiveItem = MouseOverride(hCombo, PopupOptions.overrideRight);
			SendDlgItemMessage(hwnd, IDC_CB_RIGHT, CB_SETCURSEL, dwActiveItem, 0);

			HWND hwndList = GetDlgItem(hwnd, IDC_ACTIONS);
			ListView_SetExtendedListViewStyleEx(hwndList, 0, LVS_EX_CHECKBOXES | LVS_EX_LABELTIP);
			HIMAGELIST hImgList = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 10, 1);
			ListView_SetImageList(hwndList, hImgList, LVSIL_SMALL);

			LVCOLUMN column = { 0 };
			column.mask = LVCF_TEXT | LVCF_WIDTH;
			column.pszText = TranslateT("Action");
			column.cx = 175;
			ListView_InsertColumn(hwndList, 0, &column);

			ListView_EnableGroupView(hwndList, TRUE);

			LIST<char> groups(1, strcmp);

			for (int i = 0; i < gActions.getCount(); ++i) {
				char szGroup[64];
				char *szName = strchr(gActions[i]->lpzTitle, '/');
				if (!szName) szName = gActions[i]->lpzTitle;
				else ++szName;
				mir_strncpy(szGroup, gActions[i]->lpzTitle, szName - gActions[i]->lpzTitle);

				int grpId = 0;
				if ((grpId = groups.getIndex(szGroup)) < 0) {
					LVGROUP group = { 0 };
					group.cbSize = sizeof(group);
					group.mask = LVGF_HEADER | LVGF_GROUPID;
					LPTSTR wszGroup = mir_a2u(szGroup);
					group.pszHeader = TranslateW(wszGroup);
					group.cchHeader = (int)mir_wstrlen(wszGroup);
					grpId = group.iGroupId = groups.getCount();
					ListView_InsertGroup(hwndList, -1, &group);
					mir_free(wszGroup);
					groups.insert(mir_strdup(szGroup), groups.getCount());
				}

				LVITEM item = { 0 };
				item.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT | LVIF_STATE | LVIF_INDENT;
				item.iItem = i;
				ptrW tszName(mir_a2u(szName));
				item.pszText = TranslateW(tszName);
				item.iImage = ImageList_AddIcon(hImgList, gActions[i]->lchIcon);
				item.lParam = i;
				item.mask |= LVIF_GROUPID;
				item.iGroupId = grpId;
				item.iIndent = 0;
				ListView_InsertItem(hwndList, &item);

				ListView_SetItemState(hwndList, i, (gActions[i]->flags & PAF_ENABLED) ? 0x2000 : 0x1000, LVIS_STATEIMAGEMASK);
			}

			BOOL enabled = (PopupOptions.actions & ACT_ENABLE) ? TRUE : FALSE;
			for (auto &it : controls)
				EnableWindow(GetDlgItem(hwnd, it), enabled);
		}
		windowInitialized = true;
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CHK_ENABLEACTIONS:
			PopupOptions.actions &= ~ACT_ENABLE;
			PopupOptions.actions |= IsDlgButtonChecked(hwnd, IDC_CHK_ENABLEACTIONS) ? ACT_ENABLE : 0;
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			{
				BOOL enabled = (PopupOptions.actions & ACT_ENABLE) ? TRUE : FALSE;
				for (auto &it : controls)
					EnableWindow(GetDlgItem(hwnd, it), enabled);
			}
			break;

		case IDC_CHK_IMCONTACTSONLY:
			PopupOptions.actions &= ~ACT_DEF_IMONLY;
			PopupOptions.actions |= IsDlgButtonChecked(hwnd, IDC_CHK_IMCONTACTSONLY) ? ACT_DEF_IMONLY : 0;
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			break;
		case IDC_CHK_CONTACTSONLY:
			PopupOptions.actions &= ~ACT_DEF_NOGLOBAL;
			PopupOptions.actions |= IsDlgButtonChecked(hwnd, IDC_CHK_CONTACTSONLY) ? ACT_DEF_NOGLOBAL : 0;
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			break;
		case IDC_CHK_DONTCLOSE:
			PopupOptions.actions &= ~ACT_DEF_KEEPWND;
			PopupOptions.actions |= IsDlgButtonChecked(hwnd, IDC_CHK_DONTCLOSE) ? ACT_DEF_KEEPWND : 0;
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			break;
		case IDC_CHK_LARGEICONS:
			PopupOptions.actions &= ~ACT_LARGE;
			PopupOptions.actions |= IsDlgButtonChecked(hwnd, IDC_CHK_LARGEICONS) ? ACT_LARGE : 0;
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			break;
		case IDC_RD_TEXT:
			PopupOptions.actions &= ~(ACT_TEXT | ACT_LEFTICONS | ACT_RIGHTICONS);
			PopupOptions.actions |= IsDlgButtonChecked(hwnd, IDC_CHK_ENABLEACTIONS) ? ACT_TEXT : 0;
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			break;
		case IDC_RD_LEFTICONS:
			PopupOptions.actions &= ~(ACT_TEXT | ACT_LEFTICONS | ACT_RIGHTICONS);
			PopupOptions.actions |= IsDlgButtonChecked(hwnd, IDC_RD_LEFTICONS) ? ACT_LEFTICONS : 0;
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			break;
		case IDC_RD_RIGHTICONS:
			PopupOptions.actions &= ~(ACT_TEXT | ACT_LEFTICONS | ACT_RIGHTICONS);
			PopupOptions.actions |= IsDlgButtonChecked(hwnd, IDC_RD_RIGHTICONS) ? ACT_RIGHTICONS : 0;
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			break;
		case IDC_PREVIEW:
			PopupPreview();
			break;
		case IDC_CB_LEFT:
		case IDC_CB_MIDDLE:
		case IDC_CB_RIGHT:
			PopupOptions.overrideLeft = SendDlgItemMessage(hwnd, IDC_CB_LEFT, CB_GETITEMDATA,
				SendDlgItemMessage(hwnd, IDC_CB_LEFT, CB_GETCURSEL, 0, 0), 0);
			PopupOptions.overrideMiddle = SendDlgItemMessage(hwnd, IDC_CB_MIDDLE, CB_GETITEMDATA,
				SendDlgItemMessage(hwnd, IDC_CB_MIDDLE, CB_GETCURSEL, 0, 0), 0);
			PopupOptions.overrideRight = SendDlgItemMessage(hwnd, IDC_CB_RIGHT, CB_GETITEMDATA,
				SendDlgItemMessage(hwnd, IDC_CB_RIGHT, CB_GETCURSEL, 0, 0), 0);
			if (HIWORD(wParam) == CBN_SELCHANGE)
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_RESET:
				LoadOption_Actions();
				break;

			case PSN_APPLY:
				g_plugin.setDword("Actions", PopupOptions.actions);
				HWND hwndList = GetDlgItem(hwnd, IDC_ACTIONS);

				for (int i = 0; i < gActions.getCount(); ++i) {
					gActions[i]->flags = (ListView_GetItemState(hwndList, i, LVIS_STATEIMAGEMASK) == 0x2000) ? PAF_ENABLED : 0;
					db_set_b(0, "PopupActions", gActions[i]->lpzTitle, (gActions[i]->flags & PAF_ENABLED) ? 1 : 0);
				}

				// overrideActions
				g_plugin.setDword("OverrideLeft", PopupOptions.overrideLeft);
				g_plugin.setDword("OverrideMiddle", PopupOptions.overrideMiddle);
				g_plugin.setDword("OverrideRight", PopupOptions.overrideRight);
			}
			break;

		case IDC_ACTIONS:
			NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;
			if (windowInitialized && nmlv && nmlv->hdr.code == LVN_ITEMCHANGED && nmlv->uOldState != 0 && (nmlv->uNewState == 0x1000 || nmlv->uNewState == 0x2000))
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
		}
	}
	return FALSE;
}
