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

===============================================================================

File name      : $HeadURL: http://svn.miranda.im/mainrepo/popup/trunk/src/actions.cpp $
Revision       : $Revision: 1622 $
Last change on : $Date: 2010-06-23 23:32:21 +0300 (Ð¡Ñ€, 23 Ð¸ÑŽÐ½ 2010) $
Last change by : $Author: Merlin_de $

===============================================================================
*/

#include "headers.h"

static int ActionsSortFunc(const POPUPACTION *p1, const POPUPACTION *p2)
{
	return lstrcmpA(p1->lpzTitle, p2->lpzTitle);
}

static LIST<POPUPACTION> gActions(3, ActionsSortFunc);

// interface
void LoadActions()
{
	POPUPACTION actions[] =
	{
		{ sizeof(POPUPACTION), IcoLib_GetIcon(ICO_ACT_REPLY,0),		"General/Quick reply",			0},
		{ sizeof(POPUPACTION), IcoLib_GetIcon(ICO_ACT_MESS,0),		"General/Send message",			0},
		{ sizeof(POPUPACTION), IcoLib_GetIcon(ICO_ACT_INFO,0),		"General/User details",			0},
		{ sizeof(POPUPACTION), IcoLib_GetIcon(ICO_ACT_MENU,0),		"General/Contact menu",			0},
		{ sizeof(POPUPACTION), IcoLib_GetIcon(ICO_ACT_ADD,0),		"General/Add permanently",		0},
		{ sizeof(POPUPACTION), IcoLib_GetIcon(ICO_ACT_PIN,0),		"General/Pin popup",			0},
		{ sizeof(POPUPACTION), IcoLib_GetIcon(ICO_ACT_CLOSE,0),		"General/Dismiss popup",		0},
		{ sizeof(POPUPACTION), IcoLib_GetIcon(ICO_ACT_COPY,0),		"General/Copy to clipboard",	0},

//remove popup action
	#if defined(_DEBUG)
		{ sizeof(POPUPACTION), IcoLib_GetIcon(ICO_POPUP_ON,0),		"Popup Plus/Test action",			PAF_ENABLED},
		{ sizeof(POPUPACTION), IcoLib_GetIcon(ICO_ACT_CLOSE,0),		"Popup Plus/Second test action",	0},
		{ sizeof(POPUPACTION), LoadSkinnedIcon(SKINICON_OTHER_MIRANDA),		"Popup Plus/One more action",		PAF_ENABLED},
	#endif

	};

	for (int i = 0; i < SIZEOF(actions); ++i)
		RegisterAction(&actions[i]);
}

void UnloadActions()
{
//	for (int i = 0; i < gActions.getCount(); ++i)
//		delete gActions[i];
	gActions.destroy();
}

void RegisterAction(POPUPACTION *action)
{
	int index;
	if ((index = gActions.getIndex(action)) >= 0)
	{
		DWORD flags = gActions[index]->flags;
		*gActions[index] = *action;
		gActions[index]->flags = flags;
	} else
	{
		POPUPACTION *actionCopy = (POPUPACTION *)mir_alloc(sizeof(POPUPACTION));
		*actionCopy = *action;
		actionCopy->flags =
			DBGetContactSettingByte(NULL, "PopUpActions", actionCopy->lpzTitle, actionCopy->flags & PAF_ENABLED) ?
				PAF_ENABLED : 0;
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
	POPUPACTION action = {0};
	action.flags = PAF_ENABLED;
	lstrcpyA(action.lpzTitle, name);
	return IsActionEnabled(&action);
}

DWORD MouseOverride(HWND hCombo, int number)
{
	DWORD dwItem = 0;
	DWORD ItemActive = 0;
	if(number<0 || number >7)
		number = 0;
	dwItem = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TranslateT("no overwrite"));
	SendMessage(hCombo, CB_SETITEMDATA, dwItem, 0);
	if(number == 0)
		ItemActive = dwItem;
	dwItem = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TranslateT("Send message"));
	SendMessage(hCombo, CB_SETITEMDATA, dwItem, 1);
	if(number == 1)
		ItemActive = dwItem;
	dwItem = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TranslateT("Quick reply"));
	SendMessage(hCombo, CB_SETITEMDATA, dwItem, 2);
	if(number == 2)
		ItemActive = dwItem;
	dwItem = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TranslateT("User details"));
	SendMessage(hCombo, CB_SETITEMDATA, dwItem, 3);
	if(number == 3)
		ItemActive = dwItem;
	dwItem = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TranslateT("Contact menu"));
	SendMessage(hCombo, CB_SETITEMDATA, dwItem, 4);
	if(number == 4)
		ItemActive = dwItem;
	dwItem = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TranslateT("Dismiss popup"));
	SendMessage(hCombo, CB_SETITEMDATA, dwItem, 5);
	if(number == 5)
		ItemActive = dwItem;
	dwItem = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TranslateT("Pin popup"));
	SendMessage(hCombo, CB_SETITEMDATA, dwItem, 6);
	if(number == 6)
		ItemActive = dwItem;
	dwItem = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TranslateT("Copy to clipboard"));
	SendMessage(hCombo, CB_SETITEMDATA, dwItem, 7);
	if(number == 7)
		ItemActive = dwItem;
	return ItemActive;
}

// options
#if defined(_UNICODE)
#define ListView_InsertItemW(hwnd, pitem)   \
	(int)SendMessageW((hwnd), LVM_INSERTITEMW, 0, (LPARAM)(const LVITEMW *)(pitem))
#else
#define ListView_InsertItemW(hwnd, pitem)   \
	(int)MySendMessageW((hwnd), LVM_INSERTITEMW, 0, (LPARAM)(const LVITEMW *)(pitem))
#endif

void LoadOption_Actions() {
	PopUpOptions.actions			= DBGetContactSettingDword(NULL, MODULNAME, "Actions",
									  ACT_ENABLE|ACT_RIGHTICONS|ACT_DEF_KEEPWND|ACT_DEF_IMONLY|
									  ACT_DEF_NOGLOBAL|ACT_DEF_MESSAGE|ACT_DEF_DETAILS|ACT_DEF_MENU|
									  ACT_DEF_ADD|ACT_DEF_DISMISS|ACT_DEF_PIN);
	PopUpOptions.overrideLeft		= DBGetContactSettingDword(NULL,MODULNAME, "OverrideLeft", 0);
	PopUpOptions.overrideMiddle		= DBGetContactSettingDword(NULL,MODULNAME, "OverrideMiddle", 0);
	PopUpOptions.overrideRight		= DBGetContactSettingDword(NULL,MODULNAME, "OverrideRight", 0);
}

INT_PTR CALLBACK DlgProcPopupActions(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool windowInitialized = false;

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

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			int i;
			windowInitialized = false;

			TranslateDialogDefault(hwnd);

			SendMessage(GetDlgItem(hwnd, IDC_ICO_INFO), STM_SETICON, (WPARAM)IcoLib_GetIcon(ICO_MISC_NOTIFY,0), 0);

			CheckDlgButton(hwnd, IDC_CHK_ENABLEACTIONS, PopUpOptions.actions&ACT_ENABLE ? TRUE : FALSE);
			CheckDlgButton(hwnd, IDC_CHK_IMCONTACTSONLY, PopUpOptions.actions&ACT_DEF_IMONLY ? TRUE : FALSE);
			CheckDlgButton(hwnd, IDC_CHK_CONTACTSONLY, PopUpOptions.actions&ACT_DEF_NOGLOBAL ? TRUE : FALSE);
			CheckDlgButton(hwnd, IDC_CHK_DONTCLOSE, PopUpOptions.actions&ACT_DEF_KEEPWND ? TRUE : FALSE);
			CheckDlgButton(hwnd, IDC_CHK_LARGEICONS, PopUpOptions.actions&ACT_LARGE ? TRUE : FALSE);
			CheckDlgButton(hwnd, IDC_RD_TEXT, PopUpOptions.actions&ACT_TEXT ? TRUE : FALSE);
			CheckDlgButton(hwnd, IDC_RD_LEFTICONS, PopUpOptions.actions&ACT_LEFTICONS ? TRUE : FALSE);
			CheckDlgButton(hwnd, IDC_RD_RIGHTICONS, PopUpOptions.actions&ACT_RIGHTICONS ? TRUE : FALSE);

			{
				DWORD dwActiveItem = 0;
				HWND hCombo = GetDlgItem(hwnd, IDC_CB_LEFT);
				dwActiveItem = MouseOverride(hCombo, PopUpOptions.overrideLeft);
				SendDlgItemMessage(hwnd, IDC_CB_LEFT, CB_SETCURSEL, dwActiveItem, 0);

				dwActiveItem = 0;
				hCombo = GetDlgItem(hwnd, IDC_CB_MIDDLE);
				dwActiveItem = MouseOverride(hCombo, PopUpOptions.overrideMiddle);
				SendDlgItemMessage(hwnd, IDC_CB_MIDDLE, CB_SETCURSEL, dwActiveItem, 0);

				dwActiveItem = 0;
				hCombo = GetDlgItem(hwnd, IDC_CB_RIGHT);
				dwActiveItem = MouseOverride(hCombo, PopUpOptions.overrideRight);
				SendDlgItemMessage(hwnd, IDC_CB_RIGHT, CB_SETCURSEL, dwActiveItem, 0);
			}

			HWND hwndList = GetDlgItem(hwnd, IDC_ACTIONS);
			ListView_SetExtendedListViewStyleEx(hwndList, 0, LVS_EX_CHECKBOXES|LVS_EX_LABELTIP);
			HIMAGELIST hImgList = ImageList_Create(16, 16, ILC_MASK | (IsWinVerXPPlus()? ILC_COLOR32 : ILC_COLOR16), 10, 1);
			ListView_SetImageList(hwndList, hImgList, LVSIL_SMALL);
			
			LVCOLUMN column = {0};
			column.mask = LVCF_TEXT|LVCF_WIDTH;
			column.pszText = TranslateT("Action");
			column.cx = 175;
			ListView_InsertColumn(hwndList, 0, &column);

			if (IsWinVerXPPlus())
				ListView_EnableGroupView(hwndList, TRUE);

			LIST<char> groups(1, strcmp);

			for (i = 0; i < gActions.getCount(); ++i)
			{
				char szGroup[64];
				char *szName = strchr(gActions[i]->lpzTitle, '/');
				if (!szName) szName = gActions[i]->lpzTitle;
				else ++szName;
				lstrcpynA(szGroup, gActions[i]->lpzTitle, szName - gActions[i]->lpzTitle);

				int grpId = 0;

				if (IsWinVerXPPlus() && ((grpId = groups.getIndex(szGroup)) < 0))
				{
					LVGROUP group = {0};
					group.cbSize = sizeof(group);
					group.mask = LVGF_HEADER|LVGF_GROUPID;
					LPWSTR wszGroup = mir_a2u(szGroup);
					group.pszHeader = TranslateW(wszGroup);
					group.cchHeader = lstrlenW(wszGroup);
					grpId = group.iGroupId = groups.getCount();
					int grpId = ListView_InsertGroup(hwndList, -1, &group);
					mir_free(wszGroup);
					groups.insert(mir_strdup(szGroup), groups.getCount());
				}

				if (g_popup.isOsUnicode)
				{
					LVITEMW item = {0};
					item.mask = LVIF_IMAGE|LVIF_PARAM|LVIF_TEXT|LVIF_STATE|LVIF_INDENT;
					item.iItem = i;
					LPWSTR wszName = mir_a2u(szName);
					item.pszText = TranslateW(wszName);
					item.iImage = ImageList_AddIcon(hImgList, gActions[i]->lchIcon);
					item.lParam = i;
					if (IsWinVerXPPlus())
					{
						item.mask |= LVIF_GROUPID;
						item.iGroupId = grpId;
					}
					item.iIndent = 0;
					ListView_InsertItemW(hwndList, &item);
					mir_free(wszName);
				} else
				{
					LVITEMA item = {0};
					item.mask = LVIF_IMAGE|LVIF_PARAM|LVIF_TEXT|LVIF_STATE|LVIF_GROUPID|LVIF_INDENT;
					item.iItem = i;
					item.pszText = Translate(szName);
					item.iImage = ImageList_AddIcon(hImgList, gActions[i]->lchIcon);
					item.lParam = i;
					item.iGroupId = grpId;
					item.iIndent = 0;
					ListView_InsertItem(hwndList, &item);
				}

				ListView_SetItemState(hwndList, i, (gActions[i]->flags & PAF_ENABLED) ? 0x2000 : 0x1000, LVIS_STATEIMAGEMASK);
			}

			groups.destroy();

			BOOL enabled = (PopUpOptions.actions&ACT_ENABLE) ? TRUE : FALSE;
			for (i = 0; i < SIZEOF(controls); ++i)
				EnableWindow(GetDlgItem(hwnd, controls[i]), enabled);

			windowInitialized = true;
			break;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_CHK_ENABLEACTIONS:
				{
					PopUpOptions.actions &= ~ACT_ENABLE;
					PopUpOptions.actions |= IsDlgButtonChecked(hwnd, IDC_CHK_ENABLEACTIONS) ? ACT_ENABLE : 0;
					SendMessage(GetParent(hwnd), PSM_CHANGED,0,0);

					BOOL enabled = (PopUpOptions.actions&ACT_ENABLE) ? TRUE : FALSE;
					for (int i = 0; i < SIZEOF(controls); ++i)
						EnableWindow(GetDlgItem(hwnd, controls[i]), enabled);
					break;
				}

				case IDC_CHK_IMCONTACTSONLY:
					PopUpOptions.actions &= ~ACT_DEF_IMONLY;
					PopUpOptions.actions |= IsDlgButtonChecked(hwnd, IDC_CHK_IMCONTACTSONLY) ? ACT_DEF_IMONLY : 0;
					SendMessage(GetParent(hwnd), PSM_CHANGED,0,0);
					break;
				case IDC_CHK_CONTACTSONLY:
					PopUpOptions.actions &= ~ACT_DEF_NOGLOBAL;
					PopUpOptions.actions |= IsDlgButtonChecked(hwnd, IDC_CHK_CONTACTSONLY) ? ACT_DEF_NOGLOBAL : 0;
					SendMessage(GetParent(hwnd), PSM_CHANGED,0,0);
					break;
				case IDC_CHK_DONTCLOSE:
					PopUpOptions.actions &= ~ACT_DEF_KEEPWND;
					PopUpOptions.actions |= IsDlgButtonChecked(hwnd, IDC_CHK_DONTCLOSE) ? ACT_DEF_KEEPWND : 0;
					SendMessage(GetParent(hwnd), PSM_CHANGED,0,0);
					break;
				case IDC_CHK_LARGEICONS:
					PopUpOptions.actions &= ~ACT_LARGE;
					PopUpOptions.actions |= IsDlgButtonChecked(hwnd, IDC_CHK_LARGEICONS) ? ACT_LARGE : 0;
					SendMessage(GetParent(hwnd), PSM_CHANGED,0,0);
					break;
				case IDC_RD_TEXT:
					PopUpOptions.actions &= ~(ACT_TEXT|ACT_LEFTICONS|ACT_RIGHTICONS);
					PopUpOptions.actions |= IsDlgButtonChecked(hwnd, IDC_CHK_ENABLEACTIONS) ? ACT_TEXT : 0;
					SendMessage(GetParent(hwnd), PSM_CHANGED,0,0);
					break;
				case IDC_RD_LEFTICONS:
					PopUpOptions.actions &= ~(ACT_TEXT|ACT_LEFTICONS|ACT_RIGHTICONS);
					PopUpOptions.actions |= IsDlgButtonChecked(hwnd, IDC_RD_LEFTICONS) ? ACT_LEFTICONS : 0;
					SendMessage(GetParent(hwnd), PSM_CHANGED,0,0);
					break;
				case IDC_RD_RIGHTICONS:
					PopUpOptions.actions &= ~(ACT_TEXT|ACT_LEFTICONS|ACT_RIGHTICONS);
					PopUpOptions.actions |= IsDlgButtonChecked(hwnd, IDC_RD_RIGHTICONS) ? ACT_RIGHTICONS : 0;
					SendMessage(GetParent(hwnd), PSM_CHANGED,0,0);
					break;
				case IDC_PREVIEW:
					PopUpPreview();
					break;
				case IDC_CB_LEFT:
				case IDC_CB_MIDDLE:
				case IDC_CB_RIGHT:
					PopUpOptions.overrideLeft = SendDlgItemMessage(hwnd, IDC_CB_LEFT, CB_GETITEMDATA,
						SendDlgItemMessage(hwnd, IDC_CB_LEFT, CB_GETCURSEL,0,0),0);
					PopUpOptions.overrideMiddle = SendDlgItemMessage(hwnd, IDC_CB_MIDDLE, CB_GETITEMDATA,
						SendDlgItemMessage(hwnd, IDC_CB_MIDDLE, CB_GETCURSEL,0,0),0);
					PopUpOptions.overrideRight = SendDlgItemMessage(hwnd, IDC_CB_RIGHT, CB_GETITEMDATA,
						SendDlgItemMessage(hwnd, IDC_CB_RIGHT, CB_GETCURSEL,0,0),0);
					SendMessage(GetParent(hwnd), PSM_CHANGED,0,0);
				break;
			}
			break;
		}

		case WM_NOTIFY:
		{
			switch (((LPNMHDR)lParam)->idFrom)
			{
				case 0:
				{
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_RESET:
							LoadOption_Actions();
							break;
						case PSN_APPLY:
						{
							DBWriteContactSettingDword(NULL, MODULNAME, "Actions", PopUpOptions.actions);
							HWND hwndList = GetDlgItem(hwnd, IDC_ACTIONS);
							for (int i = 0; i < gActions.getCount(); ++i)
							{
								gActions[i]->flags = (ListView_GetItemState(hwndList, i, LVIS_STATEIMAGEMASK) == 0x2000) ? PAF_ENABLED : 0;
								DBWriteContactSettingByte(NULL, "PopUpActions", gActions[i]->lpzTitle, (gActions[i]->flags&PAF_ENABLED) ? 1 : 0);
							}
							//overrideActions
							DBWriteContactSettingDword(NULL, MODULNAME, "OverrideLeft", PopUpOptions.overrideLeft);
							DBWriteContactSettingDword(NULL, MODULNAME, "OverrideMiddle", PopUpOptions.overrideMiddle);
							DBWriteContactSettingDword(NULL, MODULNAME, "OverrideRight", PopUpOptions.overrideRight);
							break;
						}
					}
					break;
				}

				case IDC_ACTIONS:
				{
					NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;
					if (windowInitialized &&
						nmlv && nmlv->hdr.code == LVN_ITEMCHANGED && nmlv->uOldState != 0 &&
						(nmlv->uNewState == 0x1000 || nmlv->uNewState == 0x2000))
					{
						SendMessage(GetParent(hwnd), PSM_CHANGED,0,0);
					}
					break;
				}
			}
			break;
		}

	}
	return FALSE;
}
