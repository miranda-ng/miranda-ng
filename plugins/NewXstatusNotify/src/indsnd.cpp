/*
	NewXstatusNotify YM - Plugin for Miranda IM
	Copyright (c) 2001-2004 Luca Santarelli
	Copyright (c) 2005-2007 Vasilich
	Copyright (c) 2007-2011 yaho

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
	*/

#include "common.h"

void PreviewSound(HWND hList)
{
	TCHAR buff[MAX_PATH], stzSoundPath[MAX_PATH];

	LVITEM lvi = { 0 };
	lvi.mask = LVIF_PARAM;
	lvi.iItem = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
	ListView_GetItem(hList, &lvi);

	int hlpStatus = lvi.lParam;

	ListView_GetItemText(hList, lvi.iItem, 1, buff, SIZEOF(buff));
	if (!_tcscmp(buff, TranslateT(DEFAULT_SOUND))) {
		if (hlpStatus < ID_STATUS_MIN)
			SkinPlaySound(StatusListEx[hlpStatus].lpzSkinSoundName);
		else
			SkinPlaySound(StatusList[Index(hlpStatus)].lpzSkinSoundName);
	}
	else {
		PathToAbsoluteT(buff, stzSoundPath);
		SkinPlaySoundFile(stzSoundPath);
	}
}

BOOL RemoveSoundFromList(HWND hList)
{
	int iSel = ListView_GetSelectionMark(hList);
	if (iSel != -1) {
		iSel = -1;
		while ((iSel = ListView_GetNextItem(hList, iSel, LVNI_SELECTED)) != -1)
			ListView_SetItemText(hList, iSel, 1, TranslateT(DEFAULT_SOUND));
		return TRUE;
	}

	return FALSE;
}

TCHAR *SelectSound(HWND hwndDlg, TCHAR *buff, size_t bufflen)
{
	OPENFILENAME ofn = { 0 };

	HWND hList = GetDlgItem(hwndDlg, IDC_INDSNDLIST);
	ListView_GetItemText(hList, ListView_GetNextItem(hList, -1, LVNI_SELECTED), 1, buff, (DWORD)bufflen);
	if (!mir_tstrcmp(buff, TranslateT(DEFAULT_SOUND)))
		buff = NULL;

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetParent(hwndDlg);
	ofn.hInstance = hInst;
	TCHAR filter[MAX_PATH];
	if (GetModuleHandle(_T("bass_interface.dll")))
		mir_sntprintf(filter, SIZEOF(filter), _T("%s (*.wav, *.mp3, *.ogg)%c*.wav;*.mp3;*.ogg%c%s (*.*)%c*%c"), TranslateT("Sound files"), 0, 0, TranslateT("All files"), 0, 0);
	else
		mir_sntprintf(filter, SIZEOF(filter), _T("%s (*.wav)%c*.wav%c%s (*.*)%c*%c"), TranslateT("Wave files"), 0, 0, TranslateT("All files"), 0, 0);
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = buff;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR;
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrDefExt = _T("");
	if (GetOpenFileName(&ofn))
		return buff;

	return NULL;
}

HIMAGELIST GetStatusIconsImgList(char *szProto)
{
	HIMAGELIST hList = NULL;
	if (szProto) {
		hList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, STATUS_COUNT - 1, 0);
		if (hList != NULL) {
			for (int i = ID_STATUS_MIN; i <= ID_STATUS_MAX; i++)
				ImageList_AddIcon(hList, LoadSkinnedProtoIcon(szProto, i));
			ImageList_AddIcon(hList, LoadSkinnedIcon(SKINICON_OTHER_USERONLINE));
		}
	}

	return hList;
}

INT_PTR CALLBACK DlgProcSoundUIPage(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static MCONTACT hContact = NULL;
	HWND hList = GetDlgItem(hwndDlg, IDC_INDSNDLIST);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			hContact = lParam;
			char *szProto = GetContactProto(hContact);

			ListView_SetImageList(hList, GetStatusIconsImgList(szProto), LVSIL_SMALL);
			ListView_SetExtendedListViewStyleEx(hList, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

			RECT rc = { 0 };
			GetClientRect(hList, &rc);

			LV_COLUMN lvc = { 0 };
			lvc.mask = LVCF_WIDTH | LVCF_TEXT;
			lvc.cx = STATUS_COLUMN;
			lvc.pszText = TranslateT("Status");
			ListView_InsertColumn(hList, 0, &lvc);

			lvc.cx = rc.right - STATUS_COLUMN - GetSystemMetrics(SM_CXVSCROLL);
			lvc.pszText = TranslateT("Sound file");
			ListView_InsertColumn(hList, 1, &lvc);

			if (szProto) {
				DBVARIANT dbv;
				TCHAR buff[MAX_PATH];

				for (int i = ID_STATUS_MAX; i >= ID_STATUS_MIN; i--) {
					int flags = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_2, 0);
					if (flags == 0)
						flags = PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT | PF2_OUTTOLUNCH | PF2_ONTHEPHONE;

					if ((flags & Proto_Status2Flag(i)) || i == ID_STATUS_OFFLINE) {
						LV_ITEM lvi = { 0 };
						lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
						lvi.iItem = 0;
						lvi.iSubItem = 0;
						lvi.iImage = Index(i);
						lvi.lParam = (LPARAM)i;
						lvi.pszText = TranslateTS(StatusList[Index(i)].lpzSkinSoundDesc);
						lvi.iItem = ListView_InsertItem(hList, &lvi);

						if (!db_get_ts(hContact, MODULE, StatusList[Index(i)].lpzSkinSoundName, &dbv)) {
							_tcscpy(buff, dbv.ptszVal);
							db_free(&dbv);
						}
						else
							_tcscpy(buff, TranslateT(DEFAULT_SOUND));

						ListView_SetItemText(hList, lvi.iItem, 1, buff);
					}
				}

				for (int i = 0; i <= ID_STATUSEX_MAX; i++) {
					LV_ITEM lvi = { 0 };
					lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
					lvi.iItem = 0;
					lvi.iSubItem = 0;
					lvi.iImage = Index(ID_STATUS_MAX) + 1; // additional icon
					lvi.lParam = (LPARAM)i;
					lvi.pszText = TranslateTS(StatusListEx[i].lpzSkinSoundDesc);
					lvi.iItem = ListView_InsertItem(hList, &lvi);

					if (!db_get_ts(hContact, MODULE, StatusList[i].lpzSkinSoundName, &dbv)) {
						_tcsncpy(buff, dbv.ptszVal, SIZEOF(buff)-1);
						db_free(&dbv);
					}
					else
						_tcsncpy(buff, TranslateT(DEFAULT_SOUND), SIZEOF(buff)-1);

					ListView_SetItemText(hList, lvi.iItem, 1, buff);
				}
			}

			CheckDlgButton(hwndDlg, IDC_CHECK_NOTIFYSOUNDS, db_get_b(hContact, MODULE, "EnableSounds", 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHECK_NOTIFYPOPUPS, db_get_b(hContact, MODULE, "EnablePopups", 1) ? BST_CHECKED : BST_UNCHECKED);

			ShowWindow(GetDlgItem(hwndDlg, IDC_INDSNDLIST), opt.UseIndSnd ? SW_SHOW : SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_TEXT_ENABLE_IS), opt.UseIndSnd ? SW_HIDE : SW_SHOW);
			ShowWindow(GetDlgItem(hwndDlg, IDC_CHANGE), opt.UseIndSnd ? SW_SHOW : SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_PREVIEW), opt.UseIndSnd ? SW_SHOW : SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_DELETE), opt.UseIndSnd ? SW_SHOW : SW_HIDE);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_PREVIEW:
			if (ListView_GetSelectionMark(hList) != -1)
				PreviewSound(hList);
			break;
		case IDC_CHANGE:
		{
			int iSel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_INDSNDLIST), -1, LVNI_SELECTED);
			if (iSel != -1) {
				TCHAR stzFilePath[MAX_PATH];
				if (SelectSound(hwndDlg, stzFilePath, MAX_PATH - 1) != NULL) {
					iSel = -1;
					while ((iSel = ListView_GetNextItem(hList, iSel, LVNI_SELECTED)) != -1)
						ListView_SetItemText(hList, iSel, 1, stzFilePath);

					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
			}
		}
		break;
		case IDC_DELETE:
			if (ListView_GetSelectionMark(hList) != -1)
				if (RemoveSoundFromList(GetDlgItem(hwndDlg, IDC_INDSNDLIST)))
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case IDC_CHECK_NOTIFYSOUNDS:
			db_set_b(hContact, MODULE, "EnableSounds", IsDlgButtonChecked(hwndDlg, IDC_CHECK_NOTIFYSOUNDS) ? 1 : 0);
			break;
		case IDC_CHECK_NOTIFYPOPUPS:
			db_set_b(hContact, MODULE, "EnablePopups", IsDlgButtonChecked(hwndDlg, IDC_CHECK_NOTIFYPOPUPS) ? 1 : 0);
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			TCHAR buff[MAX_PATH];
			HWND hList = GetDlgItem(hwndDlg, IDC_INDSNDLIST);

			LVITEM lvi = { 0 };
			lvi.mask = LVIF_PARAM;
			//Cycle through the list reading the text associated to each status.
			for (lvi.iItem = ListView_GetItemCount(hList) - 1; lvi.iItem >= 0; lvi.iItem--) {
				ListView_GetItem(hList, &lvi);
				ListView_GetItemText(hList, lvi.iItem, 1, buff, SIZEOF(buff));

				if (!_tcscmp(buff, TranslateT(DEFAULT_SOUND))) {
					if (lvi.lParam < ID_STATUS_MIN)
						db_unset(hContact, MODULE, StatusListEx[lvi.lParam].lpzSkinSoundName);
					else
						db_unset(hContact, MODULE, StatusList[Index(lvi.lParam)].lpzSkinSoundName);
				}
				else {
					TCHAR stzSoundPath[MAX_PATH] = { 0 };
					PathToRelativeT(buff, stzSoundPath);
					if (lvi.lParam < ID_STATUS_MIN)
						db_set_ws(hContact, MODULE, StatusListEx[lvi.lParam].lpzSkinSoundName, stzSoundPath);
					else
						db_set_ws(hContact, MODULE, StatusList[Index(lvi.lParam)].lpzSkinSoundName, stzSoundPath);
				}
			}

			return TRUE;
		}

		int hlpControlID = (int)wParam;
		switch (hlpControlID) {
		case IDC_INDSNDLIST:
			if (((LPNMHDR)lParam)->code == NM_DBLCLK) {
				TCHAR stzFilePath[MAX_PATH];
				if (SelectSound(hwndDlg, stzFilePath, MAX_PATH - 1) != NULL) {
					int iSel = -1;
					HWND hList = GetDlgItem(hwndDlg, IDC_INDSNDLIST);
					while ((iSel = ListView_GetNextItem(hList, iSel, LVNI_SELECTED)) != -1)
						ListView_SetItemText(hList, iSel, 1, stzFilePath);

					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				return TRUE;
			}
			else if (((LPNMHDR)lParam)->code == LVN_KEYDOWN) {
				LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN)lParam;
				if (pnkd->wVKey == VK_DELETE)
					RemoveSoundFromList(GetDlgItem(hwndDlg, IDC_INDSNDLIST));
			}

			break;
		}
		break;
	}
	return FALSE;
}

void ResetListOptions(HWND hwndList)
{
	SetWindowLongPtr(hwndList, GWL_STYLE, GetWindowLongPtr(hwndList, GWL_STYLE) | CLS_SHOWHIDDEN);
}

__inline int GetExtraImage(HWND hwndList, HANDLE hItem, int column)
{
	return SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(column, 0));
}

__inline void SetExtraImage(HWND hwndList, HANDLE hItem, int column, int imageID)
{
	SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(column, imageID));
}

void SetAllContactsIcons(HWND hwndList)
{
	BYTE EnableSounds, EnablePopups, EnableXStatus, EnableXLogging, EnableStatusMsg, EnableSMsgLogging;

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0);
		if (hItem) {
			char *szProto = GetContactProto(hContact);
			if (szProto) {
				EnableSounds = db_get_b(hContact, MODULE, "EnableSounds", 1);
				EnablePopups = db_get_b(hContact, MODULE, "EnablePopups", 1);
				EnableXStatus = db_get_b(hContact, MODULE, "EnableXStatusNotify", 1);
				EnableXLogging = db_get_b(hContact, MODULE, "EnableXLogging", 1);
				EnableStatusMsg = db_get_b(hContact, MODULE, "EnableSMsgNotify", 1);
				EnableSMsgLogging = db_get_b(hContact, MODULE, "EnableSMsgLogging", 1);
			}
			else
				EnableSounds = EnablePopups = EnableXStatus = EnableXLogging = EnableStatusMsg = EnableSMsgLogging = 0;

			SetExtraImage(hwndList, hItem, EXTRA_IMAGE_SOUND, EnableSounds ? EXTRA_IMAGE_SOUND : EXTRA_IMAGE_DOT);
			SetExtraImage(hwndList, hItem, EXTRA_IMAGE_POPUP, EnablePopups ? EXTRA_IMAGE_POPUP : EXTRA_IMAGE_DOT);
			SetExtraImage(hwndList, hItem, EXTRA_IMAGE_XSTATUS, EnableXStatus ? EXTRA_IMAGE_XSTATUS : EXTRA_IMAGE_DOT);
			SetExtraImage(hwndList, hItem, EXTRA_IMAGE_XLOGGING, EnableXLogging ? EXTRA_IMAGE_XLOGGING : EXTRA_IMAGE_DOT);
			SetExtraImage(hwndList, hItem, EXTRA_IMAGE_STATUSMSG, EnableStatusMsg ? EXTRA_IMAGE_STATUSMSG : EXTRA_IMAGE_DOT);
			SetExtraImage(hwndList, hItem, EXTRA_IMAGE_SMSGLOGGING, EnableSMsgLogging ? EXTRA_IMAGE_SMSGLOGGING : EXTRA_IMAGE_DOT);
			SetExtraImage(hwndList, hItem, EXTRA_IMAGE_DISABLEALL, EXTRA_IMAGE_DISABLEALL);
			SetExtraImage(hwndList, hItem, EXTRA_IMAGE_ENABLEALL, EXTRA_IMAGE_ENABLEALL);
		}
	}
}

void SetGroupsIcons(HWND hwndList, HANDLE hFirstItem, HANDLE hParentItem, int *groupChildCount)
{
	int iconOn[8] = { 1, 1, 1, 1, 1, 1, 1, 1 };
	int childCount[8] = { 0 };
	HANDLE hItem;

	int typeOfFirst = SendMessage(hwndList, CLM_GETITEMTYPE, (WPARAM)hFirstItem, 0);
	if (typeOfFirst == CLCIT_GROUP)
		hItem = hFirstItem;
	else
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hFirstItem);

	while (hItem) {
		HANDLE hChildItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
		if (hChildItem)
			SetGroupsIcons(hwndList, hChildItem, hItem, childCount);

		for (int i = 0; i < SIZEOF(iconOn); i++) {
			if (iconOn[i] && GetExtraImage(hwndList, hItem, i) == EXTRA_IMAGE_DOT)
				iconOn[i] = 0;
		}

		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hItem);
	}

	//check contacts
	if (typeOfFirst == CLCIT_CONTACT)
		hItem = hFirstItem;
	else
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, (LPARAM)hFirstItem);

	while (hItem) {
		for (int i = 0; i < SIZEOF(iconOn); i++) {
			int image = GetExtraImage(hwndList, hItem, i);
			if (iconOn[i] && image == EXTRA_IMAGE_DOT)
				iconOn[i] = 0;

			if (image != EMPTY_EXTRA_ICON)
				childCount[i]++;
		}

		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, (LPARAM)hItem);
	}

	//set icons
	for (int i = 0; i < SIZEOF(iconOn); i++) {
		SetExtraImage(hwndList, hParentItem, i, childCount[i] ? (iconOn[i] ? i : EXTRA_IMAGE_DOT) : EMPTY_EXTRA_ICON);
		if (groupChildCount)
			groupChildCount[i] += childCount[i];
	}
}

void SetAllChildrenIcons(HWND hwndList, HANDLE hFirstItem, int column, int image)
{
	HANDLE hItem, hChildItem;

	int typeOfFirst = SendMessage(hwndList, CLM_GETITEMTYPE, (WPARAM)hFirstItem, 0);
	if (typeOfFirst == CLCIT_GROUP)
		hItem = hFirstItem;
	else
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hFirstItem);

	while (hItem) {
		hChildItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
		if (hChildItem)
			SetAllChildrenIcons(hwndList, hChildItem, column, image);

		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTGROUP, (LPARAM)hItem);
	}

	if (typeOfFirst == CLCIT_CONTACT)
		hItem = hFirstItem;
	else
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, (LPARAM)hFirstItem);

	while (hItem) {
		int oldIcon = GetExtraImage(hwndList, hItem, column);
		if (oldIcon != EMPTY_EXTRA_ICON && oldIcon != image)
			SetExtraImage(hwndList, hItem, column, image);

		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, (LPARAM)hItem);
	}
}

INT_PTR CALLBACK DlgProcFiltering(HWND hwndDlg, UINT msg, WPARAM, LPARAM lParam)
{
	static HANDLE hItemAll;
	switch (msg) {
	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwndDlg);

		HIMAGELIST hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 3, 3);

		ImageList_AddIcon(hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_SOUND)));
		SendDlgItemMessage(hwndDlg, IDC_SOUNDICON, STM_SETICON, (WPARAM)ImageList_GetIcon(hImageList, EXTRA_IMAGE_SOUND, ILD_NORMAL), 0);
		ImageList_AddIcon(hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_POPUP)));
		SendDlgItemMessage(hwndDlg, IDC_POPUPICON, STM_SETICON, (WPARAM)ImageList_GetIcon(hImageList, EXTRA_IMAGE_POPUP, ILD_NORMAL), 0);
		ImageList_AddIcon(hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_XSTATUS)));
		SendDlgItemMessage(hwndDlg, IDC_XSTATUSICON, STM_SETICON, (WPARAM)ImageList_GetIcon(hImageList, EXTRA_IMAGE_XSTATUS, ILD_NORMAL), 0);
		ImageList_AddIcon(hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_LOGGING_XSTATUS)));
		SendDlgItemMessage(hwndDlg, IDC_XLOGGINGICON, STM_SETICON, (WPARAM)ImageList_GetIcon(hImageList, EXTRA_IMAGE_XLOGGING, ILD_NORMAL), 0);
		ImageList_AddIcon(hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_STATUS_MESSAGE)));
		SendDlgItemMessage(hwndDlg, IDC_SMSGICON, STM_SETICON, (WPARAM)ImageList_GetIcon(hImageList, EXTRA_IMAGE_STATUSMSG, ILD_NORMAL), 0);
		ImageList_AddIcon(hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_LOGGING_SMSG)));
		SendDlgItemMessage(hwndDlg, IDC_SMSGLOGGINGICON, STM_SETICON, (WPARAM)ImageList_GetIcon(hImageList, EXTRA_IMAGE_SMSGLOGGING, ILD_NORMAL), 0);
		ImageList_AddIcon(hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_DISABLEALL)));
		SendDlgItemMessage(hwndDlg, IDC_DISABLEALLICON, STM_SETICON, (WPARAM)ImageList_GetIcon(hImageList, EXTRA_IMAGE_DISABLEALL, ILD_NORMAL), 0);
		ImageList_AddIcon(hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_ENABLEALL)));
		SendDlgItemMessage(hwndDlg, IDC_ENABLEALLICON, STM_SETICON, (WPARAM)ImageList_GetIcon(hImageList, EXTRA_IMAGE_ENABLEALL, ILD_NORMAL), 0);

		ImageList_AddIcon(hImageList, LoadSkinnedIcon(SKINICON_OTHER_SMALLDOT));

		SendDlgItemMessage(hwndDlg, IDC_INDSNDLIST, CLM_SETEXTRAIMAGELIST, 0, (LPARAM)hImageList);
		SendDlgItemMessage(hwndDlg, IDC_INDSNDLIST, CLM_SETEXTRACOLUMNS, 8, 0);

		HWND hList = GetDlgItem(hwndDlg, IDC_INDSNDLIST);
		ResetListOptions(hList);

		CLCINFOITEM cii = { 0 };
		cii.cbSize = sizeof(cii);
		cii.flags = CLCIIF_GROUPFONT;
		cii.pszText = TranslateT("** All contacts **");
		hItemAll = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_INDSNDLIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);

		return TRUE;
	}
	case WM_SETFOCUS:
		SetFocus(GetDlgItem(hwndDlg, IDC_INDSNDLIST));
		break;

	case WM_NOTIFY:
	{
		HWND hList = GetDlgItem(hwndDlg, IDC_INDSNDLIST);
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_INDSNDLIST:
			switch (((LPNMHDR)lParam)->code) {
			case CLN_NEWCONTACT:
			case CLN_LISTREBUILT:
				SetAllContactsIcons(hList);
				//fall through
			case CLN_CONTACTMOVED:
				SetGroupsIcons(hList, (HANDLE)SendMessage(hList, CLM_GETNEXTITEM, CLGN_ROOT, 0), hItemAll, NULL);
				break;
			case CLN_OPTIONSCHANGED:
				ResetListOptions(hList);
				break;
			case NM_CLICK:
			{
				NMCLISTCONTROL *nm = (NMCLISTCONTROL *)lParam;
				DWORD hitFlags;

				// Make sure we have an extra column
				if (nm->iColumn == -1)
					break;

				// Find clicked item
				HANDLE hItem = (HANDLE)SendMessage(hList, CLM_HITTEST, (WPARAM)&hitFlags, MAKELPARAM(nm->pt.x, nm->pt.y));
				if (hItem == NULL)
					break;
				if (!(hitFlags & CLCHT_ONITEMEXTRA))
					break;

				int itemType = SendMessage(hList, CLM_GETITEMTYPE, (WPARAM)hItem, 0);

				// Get image in clicked column 
				int image = GetExtraImage(hList, hItem, nm->iColumn);
				if (image == EXTRA_IMAGE_DOT)
					image = nm->iColumn;
				else if (image >= EXTRA_IMAGE_SOUND && image <= EXTRA_IMAGE_SMSGLOGGING)
					image = EXTRA_IMAGE_DOT;

				// Get item type (contact, group, etc...)
				if (itemType == CLCIT_CONTACT) {
					if (image == EXTRA_IMAGE_DISABLEALL) {
						for (int i = 0; i < 6; i++)
							SetExtraImage(hList, hItem, i, EXTRA_IMAGE_DOT);
					}
					else if (image == EXTRA_IMAGE_ENABLEALL) {
						for (int i = 0; i < 6; i++)
							SetExtraImage(hList, hItem, i, i);
					}
					else
						SetExtraImage(hList, hItem, nm->iColumn, image);
				}
				else if (itemType == CLCIT_INFO || itemType == CLCIT_GROUP) {
					if (itemType == CLCIT_GROUP)
						hItem = (HANDLE)SendMessage(hList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);

					if (hItem) {
						if (image == EXTRA_IMAGE_DISABLEALL) {
							for (int i = 0; i < 6; i++)
								SetAllChildrenIcons(hList, hItem, i, EXTRA_IMAGE_DOT);
						}
						else if (image == EXTRA_IMAGE_ENABLEALL) {
							for (int i = 0; i < 6; i++)
								SetAllChildrenIcons(hList, hItem, i, i);
						}
						else
							SetAllChildrenIcons(hList, hItem, nm->iColumn, image);
					}
				}

				// Update the all/none icons
				SetGroupsIcons(hList, (HANDLE)SendMessage(hList, CLM_GETNEXTITEM, CLGN_ROOT, 0), hItemAll, NULL);
				// Activate Apply button
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
			}
			break;
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
					HANDLE hItem = (HANDLE)SendMessage(hList, CLM_FINDCONTACT, hContact, 0);
					if (hItem) {
						if (GetExtraImage(hList, hItem, EXTRA_IMAGE_SOUND) == EXTRA_IMAGE_SOUND)
							db_unset(hContact, MODULE, "EnableSounds");
						else
							db_set_b(hContact, MODULE, "EnableSounds", 0);

						if (GetExtraImage(hList, hItem, EXTRA_IMAGE_POPUP) == EXTRA_IMAGE_POPUP)
							db_unset(hContact, MODULE, "EnablePopups");
						else
							db_set_b(hContact, MODULE, "EnablePopups", 0);

						if (GetExtraImage(hList, hItem, EXTRA_IMAGE_XSTATUS) == EXTRA_IMAGE_XSTATUS)
							db_unset(hContact, MODULE, "EnableXStatusNotify");
						else
							db_set_b(hContact, MODULE, "EnableXStatusNotify", 0);

						if (GetExtraImage(hList, hItem, EXTRA_IMAGE_XLOGGING) == EXTRA_IMAGE_XLOGGING)
							db_unset(hContact, MODULE, "EnableXLogging");
						else
							db_set_b(hContact, MODULE, "EnableXLogging", 0);

						if (GetExtraImage(hList, hItem, EXTRA_IMAGE_STATUSMSG) == EXTRA_IMAGE_STATUSMSG)
							db_unset(hContact, MODULE, "EnableSMsgNotify");
						else
							db_set_b(hContact, MODULE, "EnableSMsgNotify", 0);

						if (GetExtraImage(hList, hItem, EXTRA_IMAGE_SMSGLOGGING) == EXTRA_IMAGE_SMSGLOGGING)
							db_unset(hContact, MODULE, "EnableSMsgLogging");
						else
							db_set_b(hContact, MODULE, "EnableSMsgLogging", 0);
					}
				}
				return TRUE;
			}
		}
	}
	break;

	case WM_DESTROY:
		HIMAGELIST hImageList = (HIMAGELIST)SendDlgItemMessage(hwndDlg, IDC_INDSNDLIST, CLM_GETEXTRAIMAGELIST, 0, 0);
		for (int i = 0; i < ImageList_GetImageCount(hImageList); i++)
			DestroyIcon(ImageList_GetIcon(hImageList, i, ILD_NORMAL));
		ImageList_Destroy(hImageList);
		break;
	}
	return FALSE;
}

int UserInfoInitialise(WPARAM wParam, LPARAM lParam)
{
	if (lParam) {
		OPTIONSDIALOGPAGE odp = { sizeof(odp) };
		odp.position = 100000000;
		odp.hInstance = hInst;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_SOUNDS);
		odp.pszTitle = LPGEN("Status Notify");
		odp.pfnDlgProc = DlgProcSoundUIPage;
		UserInfo_AddPage(wParam, &odp);
	}
	return 0;
}
