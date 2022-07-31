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

#define IGNOREEVENT_MAX 5

static int masks[IGNOREEVENT_MAX] = { 0x0001, 0x0004, 0x0008, 0x0010, 0x0040 };

static uint32_t ignoreIdToPf1[IGNOREEVENT_MAX] = { PF1_IMRECV, PF1_FILERECV, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
static uint32_t ignoreIdToPf4[IGNOREEVENT_MAX] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, PF4_SUPPORTTYPING };

static uint32_t GetMask(MCONTACT hContact)
{
	uint32_t mask = db_get_dw(hContact, "Ignore", "Mask1", (uint32_t)-1);
	if (mask == (uint32_t)-1) {
		if (hContact == 0)
			mask = 0;
		else {
			if ((Contact::IsHidden(hContact) && !db_mc_isSub(hContact)) || !Contact::OnList(hContact))
				mask = db_get_dw(0, "Ignore", "Mask1", 0);
			else
				mask = db_get_dw(0, "Ignore", "Default1", 0);
		}
	}
	return mask;
}

static void SetListGroupIcons(HWND hwndList, HANDLE hFirstItem, HANDLE hParentItem, int *groupChildCount)
{
	int iconOn[IGNOREEVENT_MAX] = { 1, 1, 1, 1, 1 };
	int childCount[IGNOREEVENT_MAX] = { 0, 0, 0, 0, 0 };
	HANDLE hItem, hChildItem;

	int typeOfFirst = SendMessage(hwndList, CLM_GETITEMTYPE, (WPARAM)hFirstItem, 0);
	
	// check groups
	if (typeOfFirst == CLCIT_GROUP) hItem = hFirstItem;
	else hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hFirstItem);
	while (hItem) {
		hChildItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
		if (hChildItem)
			SetListGroupIcons(hwndList, hChildItem, hItem, childCount);
		
		for (int i = 0; i < _countof(iconOn); i++)
			if (iconOn[i] && SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, i) == 0)
				iconOn[i] = 0;
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hItem);
	}
	
	// check contacts
	if (typeOfFirst == CLCIT_CONTACT) hItem = hFirstItem;
	else hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, (LPARAM)hFirstItem);
	while (hItem) {
		for (int i = 0; i < _countof(iconOn); i++) {
			int iImage = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, i);
			if (iconOn[i] && iImage == 0) iconOn[i] = 0;
			if (iImage != EMPTY_EXTRA_ICON)
				childCount[i]++;
		}
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, (LPARAM)hItem);
	}
	
	// set icons
	for (int i = 0; i < _countof(iconOn); i++) {
		SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hParentItem, MAKELPARAM(i, childCount[i] ? (iconOn[i] ? i + 3 : 0) : EMPTY_EXTRA_ICON));
		if (groupChildCount)
			groupChildCount[i] += childCount[i];
	}
	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hParentItem, MAKELPARAM(IGNOREEVENT_MAX, 1));
	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hParentItem, MAKELPARAM(IGNOREEVENT_MAX + 1, 2));
}

static void SetAllChildIcons(HWND hwndList, HANDLE hFirstItem, int iColumn, int iImage)
{
	HANDLE hItem;

	int typeOfFirst = SendMessage(hwndList, CLM_GETITEMTYPE, (WPARAM)hFirstItem, 0);

	// check groups
	if (typeOfFirst == CLCIT_GROUP) hItem = hFirstItem;
	else hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hFirstItem);
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

static void ResetListOptions(HWND hwndList)
{
	SendMessage(hwndList, CLM_SETHIDEEMPTYGROUPS, 1, 0);
}

static void SetIconsForColumn(HWND hwndList, HANDLE hItem, HANDLE hItemAll, int iColumn, int iImage)
{
	switch (SendMessage(hwndList, CLM_GETITEMTYPE, (WPARAM)hItem, 0)) {
	case CLCIT_INFO:
		if (hItem == hItemAll)
			SetAllChildIcons(hwndList, hItem, iColumn, iImage);
		else
			SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(iColumn, iImage)); //hItemUnknown
		break;

	case CLCIT_GROUP:
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
		if (hItem)
			SetAllChildIcons(hwndList, hItem, iColumn, iImage);
		break;

	case CLCIT_CONTACT:
		int oldiImage = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, iColumn);
		if (oldiImage != EMPTY_EXTRA_ICON && oldiImage != iImage)
			SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(iColumn, iImage));
		break;
	}
}

static void InitialiseItem(HWND hwndList, MCONTACT hContact, HANDLE hItem, uint32_t proto1Caps, uint32_t proto4Caps)
{
	uint32_t mask = GetMask(hContact);
	for (int i = 0; i < IGNOREEVENT_MAX; i++)
		if ((ignoreIdToPf1[i] == 0xFFFFFFFF && ignoreIdToPf4[i] == 0xFFFFFFFF) || (proto1Caps & ignoreIdToPf1[i] || proto4Caps & ignoreIdToPf4[i]))
			SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(i, (mask & masks[i]) ? i + 3 : 0));

	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(IGNOREEVENT_MAX, 1));
	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(IGNOREEVENT_MAX + 1, 2));
}

static void SaveItemValue(MCONTACT hContact, const char *pszSetting, uint32_t dwValue)
{
	db_set_dw(hContact, "Ignore", pszSetting, dwValue);

	// assign the same value to all subs, if any
	for (int i = db_mc_getSubCount(hContact) - 1; i >= 0; i--)
		db_set_dw(db_mc_getSub(hContact, i), "Ignore", pszSetting, dwValue);
}

static void SaveItemMask(HWND hwndList, MCONTACT hContact, HANDLE hItem, const char *pszSetting)
{
	uint32_t mask = 0;
	for (int i = 0; i < IGNOREEVENT_MAX; i++) {
		int iImage = SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(i, 0));
		if (iImage && iImage != EMPTY_EXTRA_ICON)
			mask |= masks[i];
	}
	SaveItemValue(hContact, pszSetting, mask);
}

static void SetAllContactIcons(HWND hwndList)
{
	for (auto &hContact : Contacts()) {
		HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0);
		if (hItem && SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(IGNOREEVENT_MAX, 0)) == EMPTY_EXTRA_ICON) {
			uint32_t proto1Caps, proto4Caps;
			char *szProto = Proto_GetBaseAccountName(hContact);
			if (szProto) {
				proto1Caps = CallProtoServiceInt(0, szProto, PS_GETCAPS, PFLAGNUM_1, 0);
				proto4Caps = CallProtoServiceInt(0, szProto, PS_GETCAPS, PFLAGNUM_4, 0);
			}
			else proto1Caps = proto4Caps = 0;
			InitialiseItem(hwndList, hContact, hItem, proto1Caps, proto4Caps);
			if (!Contact::IsHidden(hContact))
				SendMessage(hwndList, CLM_SETCHECKMARK, (WPARAM)hItem, 1);
		}
	}
}

static INT_PTR CALLBACK DlgProcIgnoreOpts(HWND hwndDlg, UINT msg, WPARAM, LPARAM lParam)
{
	static HICON hIcons[IGNOREEVENT_MAX + 2];
	static HANDLE hItemAll, hItemUnknown;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			HIMAGELIST hIml = ImageList_Create(g_iIconSX, g_iIconSY, ILC_COLOR32 | ILC_MASK, 3 + IGNOREEVENT_MAX, 3 + IGNOREEVENT_MAX);
			ImageList_AddSkinIcon(hIml, SKINICON_OTHER_SMALLDOT);
			ImageList_AddSkinIcon(hIml, SKINICON_OTHER_FILLEDBLOB);
			ImageList_AddSkinIcon(hIml, SKINICON_OTHER_EMPTYBLOB);
			ImageList_AddSkinIcon(hIml, SKINICON_EVENT_MESSAGE);
			ImageList_AddSkinIcon(hIml, SKINICON_EVENT_FILE);
			ImageList_AddSkinIcon(hIml, SKINICON_OTHER_USERONLINE);
			ImageList_AddSkinIcon(hIml, SKINICON_AUTH_REQUEST);
			ImageList_AddSkinIcon(hIml, SKINICON_OTHER_TYPING);

			SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_SETEXTRAIMAGELIST, 0, (LPARAM)hIml);
			for (int i = 0; i < _countof(hIcons); i++)
				hIcons[i] = ImageList_GetIcon(hIml, 1 + i, ILD_NORMAL);
		}

		SendDlgItemMessage(hwndDlg, IDC_ALLICON, STM_SETICON, (WPARAM)hIcons[0], 0);
		SendDlgItemMessage(hwndDlg, IDC_NONEICON, STM_SETICON, (WPARAM)hIcons[1], 0);
		SendDlgItemMessage(hwndDlg, IDC_MSGICON, STM_SETICON, (WPARAM)hIcons[2], 0);
		SendDlgItemMessage(hwndDlg, IDC_FILEICON, STM_SETICON, (WPARAM)hIcons[3], 0);
		SendDlgItemMessage(hwndDlg, IDC_ONLINEICON, STM_SETICON, (WPARAM)hIcons[4], 0);
		SendDlgItemMessage(hwndDlg, IDC_AUTHICON, STM_SETICON, (WPARAM)hIcons[5], 0);
		SendDlgItemMessage(hwndDlg, IDC_TYPINGICON, STM_SETICON, (WPARAM)hIcons[6], 0);

		ResetListOptions(GetDlgItem(hwndDlg, IDC_LIST));
		SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_SETEXTRACOLUMNS, IGNOREEVENT_MAX + 2, 0);
		{
			CLCINFOITEM cii = { sizeof(cii) };
			cii.flags = CLCIIF_GROUPFONT;
			cii.pszText = TranslateT("** All contacts **");
			hItemAll = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);

			cii.pszText = TranslateT("** Unknown contacts **");
			hItemUnknown = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
			InitialiseItem(GetDlgItem(hwndDlg, IDC_LIST), 0, hItemUnknown, 0xFFFFFFFF, 0xFFFFFFFF);
		}

		SetAllContactIcons(GetDlgItem(hwndDlg, IDC_LIST));
		SetListGroupIcons(GetDlgItem(hwndDlg, IDC_LIST), (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETNEXTITEM, CLGN_ROOT, 0), hItemAll, nullptr);
		return TRUE;

	case WM_SETFOCUS:
		SetFocus(GetDlgItem(hwndDlg, IDC_LIST));
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_LIST:
			switch (((LPNMHDR)lParam)->code) {
			case CLN_NEWCONTACT:
			case CLN_LISTREBUILT:
				SetAllContactIcons(GetDlgItem(hwndDlg, IDC_LIST));
				__fallthrough;
			case CLN_CONTACTMOVED:
				SetListGroupIcons(GetDlgItem(hwndDlg, IDC_LIST), (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETNEXTITEM, CLGN_ROOT, 0), hItemAll, nullptr);
				break;
			case CLN_OPTIONSCHANGED:
				ResetListOptions(GetDlgItem(hwndDlg, IDC_LIST));
				break;
			case CLN_CHECKCHANGED:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case NM_CLICK:
				NMCLISTCONTROL *nm = (NMCLISTCONTROL*)lParam;
				if (nm->iColumn == -1)
					break;

				uint32_t hitFlags;
				HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_HITTEST, (WPARAM)&hitFlags, MAKELPARAM(nm->pt.x, nm->pt.y));
				if (hItem == nullptr || !(hitFlags & CLCHT_ONITEMEXTRA))
					break;

				if (nm->iColumn == IGNOREEVENT_MAX) { // ignore all
					for (int iImage = 0; iImage < IGNOREEVENT_MAX; iImage++)
						SetIconsForColumn(GetDlgItem(hwndDlg, IDC_LIST), hItem, hItemAll, iImage, iImage + 3);
				}
				else if (nm->iColumn == IGNOREEVENT_MAX + 1) {	// ignore none
					for (int iImage = 0; iImage < IGNOREEVENT_MAX; iImage++)
						SetIconsForColumn(GetDlgItem(hwndDlg, IDC_LIST), hItem, hItemAll, iImage, 0);
				}
				else {
					int iImage = SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn, 0));
					if (iImage == 0)
						iImage = nm->iColumn + 3;
					else if (iImage != EMPTY_EXTRA_ICON)
						iImage = 0;
					SetIconsForColumn(GetDlgItem(hwndDlg, IDC_LIST), hItem, hItemAll, nm->iColumn, iImage);
				}
				SetListGroupIcons(GetDlgItem(hwndDlg, IDC_LIST), (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETNEXTITEM, CLGN_ROOT, 0), hItemAll, nullptr);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;

		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				for (auto &hContact : Contacts()) {
					HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_FINDCONTACT, hContact, 0);
					if (hItem)
						SaveItemMask(GetDlgItem(hwndDlg, IDC_LIST), hContact, hItem, "Mask1");
					Contact::Hide(hContact, !SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETCHECKMARK, (WPARAM)hItem, 0));
				}

				SaveItemMask(GetDlgItem(hwndDlg, IDC_LIST), 0, hItemAll, "Default1");
				SaveItemMask(GetDlgItem(hwndDlg, IDC_LIST), 0, hItemUnknown, "Mask1");
				return TRUE;
			}
		}
		break;

	case WM_DESTROY:
		for (auto &it : hIcons)
			DestroyIcon(it);

		HIMAGELIST hIml = (HIMAGELIST)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETEXTRAIMAGELIST, 0, 0);
		ImageList_Destroy(hIml);
		break;
	}
	return FALSE;
}

static int IgnoreOptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 900000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_IGNORE);
	odp.szTitle.a = LPGEN("Ignore");
	odp.szGroup.a = LPGEN("Contacts");
	odp.pfnDlgProc = DlgProcIgnoreOpts;
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(bool) Ignore_IsIgnored(MCONTACT hContact, int idx)
{
	uint32_t mask = GetMask(hContact);
	if (idx < 1 || idx > IGNOREEVENT_MAX)
		return 1;
	return (masks[idx-1] & mask) != 0;
}

MIR_APP_DLL(int) Ignore_Ignore(MCONTACT hContact, int idx)
{
	uint32_t mask = GetMask(hContact);
	if ((idx < 1 || idx > IGNOREEVENT_MAX) && idx != IGNOREEVENT_ALL)
		return 1;
	
	if (idx == IGNOREEVENT_ALL)
		mask = 0xFFFF;
	else
		mask |= masks[idx-1];
	SaveItemValue(hContact, "Mask1", mask);
	return 0;
}

MIR_APP_DLL(int) Ignore_Allow(MCONTACT hContact, int idx)
{
	uint32_t mask = GetMask(hContact);
	if ((idx < 1 || idx > IGNOREEVENT_MAX) && idx != IGNOREEVENT_ALL)
		return 1;

	if (idx == IGNOREEVENT_ALL)
		mask = 0;
	else
		mask &= ~(masks[idx-1]);
	SaveItemValue(hContact, "Mask1", mask);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR IgnoreRecvMessage(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA*)lParam;
	if (Ignore_IsIgnored(ccs->hContact, IGNOREEVENT_MESSAGE))
		return 1;
	return Proto_ChainRecv(wParam, ccs);
}

static INT_PTR IgnoreRecvFile(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA*)lParam;
	if (Ignore_IsIgnored(ccs->hContact, IGNOREEVENT_FILE))
		return 1;
	return Proto_ChainRecv(wParam, ccs);
}

static INT_PTR IgnoreRecvAuth(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA*)lParam;
	if (Ignore_IsIgnored(ccs->hContact, IGNOREEVENT_AUTHORIZATION))
		return 1;
	return Proto_ChainRecv(wParam, ccs);
}

int LoadIgnoreModule(void)
{
	Proto_RegisterModule(PROTOTYPE_IGNORE, "Ignore");

	CreateProtoServiceFunction("Ignore", PSR_MESSAGE, IgnoreRecvMessage);
	CreateProtoServiceFunction("Ignore", PSR_FILE, IgnoreRecvFile);
	CreateProtoServiceFunction("Ignore", PSR_AUTH, IgnoreRecvAuth);

	HookEvent(ME_OPT_INITIALISE, IgnoreOptInitialise);
	return 0;
}
