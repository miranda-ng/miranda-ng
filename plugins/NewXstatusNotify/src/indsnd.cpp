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

#include "stdafx.h"

wchar_t* SelectSound(HWND hwndDlg, wchar_t *buff, size_t bufflen)
{
	OPENFILENAME ofn = { 0 };

	HWND hList = GetDlgItem(hwndDlg, IDC_INDSNDLIST);
	ListView_GetItemText(hList, ListView_GetNextItem(hList, -1, LVNI_SELECTED), 1, buff, (uint32_t)bufflen);
	if (!mir_wstrcmp(buff, TranslateW(DEFAULT_SOUND)))
		buff = nullptr;

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetParent(hwndDlg);
	ofn.hInstance = g_plugin.getInst();
	wchar_t filter[MAX_PATH];
	if (GetModuleHandle(L"bass_interface.dll"))
		mir_snwprintf(filter, L"%s (*.wav, *.mp3, *.ogg)%c*.wav;*.mp3;*.ogg%c%s (*.*)%c*%c", TranslateT("Sound files"), 0, 0, TranslateT("All files"), 0, 0);
	else
		mir_snwprintf(filter, L"%s (*.wav)%c*.wav%c%s (*.*)%c*%c", TranslateT("Wave files"), 0, 0, TranslateT("All files"), 0, 0);
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = buff;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR;
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrDefExt = L"";
	if (GetOpenFileName(&ofn))
		return buff;

	return nullptr;
}

HIMAGELIST GetStatusIconsImgList(char *szProto)
{
	HIMAGELIST hList = nullptr;
	if (szProto) {
		hList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, STATUS_COUNT - 1, 0);
		if (hList != nullptr) {
			for (int i = ID_STATUS_MIN; i <= ID_STATUS_MAX; i++)
				ImageList_AddIcon(hList, Skin_LoadProtoIcon(szProto, i));
			ImageList_AddSkinIcon(hList, SKINICON_OTHER_USERONLINE);
		}
	}

	return hList;
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
	uint8_t EnableSounds, EnablePopups, EnableXStatus, EnableXLogging, EnableStatusMsg, EnableSMsgLogging;

	for (auto &hContact : Contacts()) {
		HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0);
		if (hItem) {
			char *szProto = Proto_GetBaseAccountName(hContact);
			if (szProto) {
				EnableSounds = g_plugin.getByte(hContact, "EnableSounds", 1);
				EnablePopups = g_plugin.getByte(hContact, "EnablePopups", 1);
				EnableXStatus = g_plugin.getByte(hContact, "EnableXStatusNotify", 1);
				EnableXLogging = g_plugin.getByte(hContact, "EnableXLogging", 1);
				EnableStatusMsg = g_plugin.getByte(hContact, "EnableSMsgNotify", 1);
				EnableSMsgLogging = g_plugin.getByte(hContact, "EnableSMsgLogging", 1);
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

		for (int i = 0; i < _countof(iconOn); i++) {
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
		for (int i = 0; i < _countof(iconOn); i++) {
			int image = GetExtraImage(hwndList, hItem, i);
			if (iconOn[i] && image == EXTRA_IMAGE_DOT)
				iconOn[i] = 0;

			if (image != EMPTY_EXTRA_ICON)
				childCount[i]++;
		}

		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXTCONTACT, (LPARAM)hItem);
	}

	//set icons
	for (int i = 0; i < _countof(iconOn); i++) {
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

		ImageList_AddIcon(hImageList, IcoLib_GetIconByHandle(iconList[ICO_SOUND].hIcolib));
		SendDlgItemMessage(hwndDlg, IDC_SOUNDICON, STM_SETICON, (WPARAM)ImageList_GetIcon(hImageList, EXTRA_IMAGE_SOUND, ILD_NORMAL), 0);
		ImageList_AddIcon(hImageList, Skin_LoadIcon(SKINICON_OTHER_POPUP));
		SendDlgItemMessage(hwndDlg, IDC_POPUPICON, STM_SETICON, (WPARAM)ImageList_GetIcon(hImageList, EXTRA_IMAGE_POPUP, ILD_NORMAL), 0);
		ImageList_AddIcon(hImageList, IcoLib_GetIconByHandle(iconList[ICO_XSTATUS].hIcolib));
		SendDlgItemMessage(hwndDlg, IDC_XSTATUSICON, STM_SETICON, (WPARAM)ImageList_GetIcon(hImageList, EXTRA_IMAGE_XSTATUS, ILD_NORMAL), 0);
		ImageList_AddIcon(hImageList, IcoLib_GetIconByHandle(iconList[ICO_LOGGING_XSTATUS].hIcolib));
		SendDlgItemMessage(hwndDlg, IDC_XLOGGINGICON, STM_SETICON, (WPARAM)ImageList_GetIcon(hImageList, EXTRA_IMAGE_XLOGGING, ILD_NORMAL), 0);
		ImageList_AddIcon(hImageList, IcoLib_GetIconByHandle(iconList[ICO_STATUS_MESSAGE].hIcolib));
		SendDlgItemMessage(hwndDlg, IDC_SMSGICON, STM_SETICON, (WPARAM)ImageList_GetIcon(hImageList, EXTRA_IMAGE_STATUSMSG, ILD_NORMAL), 0);
		ImageList_AddIcon(hImageList, IcoLib_GetIconByHandle(iconList[ICO_LOGGING_SMSG].hIcolib));
		SendDlgItemMessage(hwndDlg, IDC_SMSGLOGGINGICON, STM_SETICON, (WPARAM)ImageList_GetIcon(hImageList, EXTRA_IMAGE_SMSGLOGGING, ILD_NORMAL), 0);
		ImageList_AddIcon(hImageList, IcoLib_GetIconByHandle(iconList[ICO_DISABLEALL].hIcolib));
		SendDlgItemMessage(hwndDlg, IDC_DISABLEALLICON, STM_SETICON, (WPARAM)ImageList_GetIcon(hImageList, EXTRA_IMAGE_DISABLEALL, ILD_NORMAL), 0);
		ImageList_AddIcon(hImageList, IcoLib_GetIconByHandle(iconList[ICO_ENABLEALL].hIcolib));
		SendDlgItemMessage(hwndDlg, IDC_ENABLEALLICON, STM_SETICON, (WPARAM)ImageList_GetIcon(hImageList, EXTRA_IMAGE_ENABLEALL, ILD_NORMAL), 0);

		ImageList_AddSkinIcon(hImageList, SKINICON_OTHER_SMALLDOT);

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
				SetGroupsIcons(hList, (HANDLE)SendMessage(hList, CLM_GETNEXTITEM, CLGN_ROOT, 0), hItemAll, nullptr);
				break;
			case CLN_OPTIONSCHANGED:
				ResetListOptions(hList);
				break;
			case NM_CLICK:
			{
				NMCLISTCONTROL *nm = (NMCLISTCONTROL *)lParam;
				uint32_t hitFlags;

				// Make sure we have an extra column
				if (nm->iColumn == -1)
					break;

				// Find clicked item
				HANDLE hItem = (HANDLE)SendMessage(hList, CLM_HITTEST, (WPARAM)&hitFlags, MAKELPARAM(nm->pt.x, nm->pt.y));
				if (hItem == nullptr)
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
				SetGroupsIcons(hList, (HANDLE)SendMessage(hList, CLM_GETNEXTITEM, CLGN_ROOT, 0), hItemAll, nullptr);
				// Activate Apply button
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
			}
			break;
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				for (auto &hContact : Contacts()) {
					HANDLE hItem = (HANDLE)SendMessage(hList, CLM_FINDCONTACT, hContact, 0);
					if (hItem) {
						if (GetExtraImage(hList, hItem, EXTRA_IMAGE_SOUND) == EXTRA_IMAGE_SOUND)
							g_plugin.delSetting(hContact, "EnableSounds");
						else
							g_plugin.setByte(hContact, "EnableSounds", 0);

						if (GetExtraImage(hList, hItem, EXTRA_IMAGE_POPUP) == EXTRA_IMAGE_POPUP)
							g_plugin.delSetting(hContact, "EnablePopups");
						else
							g_plugin.setByte(hContact, "EnablePopups", 0);

						if (GetExtraImage(hList, hItem, EXTRA_IMAGE_XSTATUS) == EXTRA_IMAGE_XSTATUS)
							g_plugin.delSetting(hContact, "EnableXStatusNotify");
						else
							g_plugin.setByte(hContact, "EnableXStatusNotify", 0);

						if (GetExtraImage(hList, hItem, EXTRA_IMAGE_XLOGGING) == EXTRA_IMAGE_XLOGGING)
							g_plugin.delSetting(hContact, "EnableXLogging");
						else
							g_plugin.setByte(hContact, "EnableXLogging", 0);

						if (GetExtraImage(hList, hItem, EXTRA_IMAGE_STATUSMSG) == EXTRA_IMAGE_STATUSMSG)
							g_plugin.delSetting(hContact, "EnableSMsgNotify");
						else
							g_plugin.setByte(hContact, "EnableSMsgNotify", 0);

						if (GetExtraImage(hList, hItem, EXTRA_IMAGE_SMSGLOGGING) == EXTRA_IMAGE_SMSGLOGGING)
							g_plugin.delSetting(hContact, "EnableSMsgLogging");
						else
							g_plugin.setByte(hContact, "EnableSMsgLogging", 0);
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

/////////////////////////////////////////////////////////////////////////////////////////
// User info dialog

class SoundUIPageDlg : public CUserInfoPageDlg
{
	CCtrlCheck chkSounds, chkPopups;
	CCtrlButton btnPreview, btnChange, btnDelete;
	CCtrlListView m_list;

	void PreviewSound()
	{
		LVITEM lvi = { 0 };
		lvi.mask = LVIF_PARAM;
		lvi.iItem = m_list.GetNextItem(-1, LVNI_SELECTED);
		m_list.GetItem(&lvi);

		int hlpStatus = lvi.lParam;

		wchar_t buff[MAX_PATH], stzSoundPath[MAX_PATH];
		m_list.GetItemText(lvi.iItem, 1, buff, _countof(buff));
		if (!mir_wstrcmp(buff, TranslateW(DEFAULT_SOUND))) {
			if (hlpStatus < ID_STATUS_MIN)
				Skin_PlaySound(StatusListEx[hlpStatus].lpzSkinSoundName);
			else
				Skin_PlaySound(StatusList[Index(hlpStatus)].lpzSkinSoundName);
		}
		else {
			PathToAbsoluteW(buff, stzSoundPath);
			Skin_PlaySoundFile(stzSoundPath);
		}
	}

	BOOL RemoveSoundFromList()
	{
		int iSel = m_list.GetSelectionMark();
		if (iSel != -1) {
			iSel = -1;
			while ((iSel = m_list.GetNextItem(iSel, LVNI_SELECTED)) != -1)
				m_list.SetItemText(iSel, 1, TranslateW(DEFAULT_SOUND));
			return TRUE;
		}

		return FALSE;
	}

public:
	SoundUIPageDlg() :
		CUserInfoPageDlg(g_plugin, IDD_INFO_SOUNDS),
		m_list(this, IDC_INDSNDLIST),
		chkPopups(this, IDC_CHECK_NOTIFYPOPUPS),
		chkSounds(this, IDC_CHECK_NOTIFYSOUNDS),
		btnChange(this, IDC_CHANGE),
		btnDelete(this, IDC_DELETE),
		btnPreview(this, IDC_PREVIEW)
	{
		m_list.OnDoubleClick = Callback(this, &SoundUIPageDlg::onDblClick_List);

		chkPopups.OnChange = Callback(this, &SoundUIPageDlg::onChange_Popups);
		chkSounds.OnChange = Callback(this, &SoundUIPageDlg::onChange_Sounds);

		btnChange.OnClick = Callback(this, &SoundUIPageDlg::onClick_Change);
		btnDelete.OnClick = Callback(this, &SoundUIPageDlg::onClick_Delete);
		btnPreview.OnClick = Callback(this, &SoundUIPageDlg::onClick_Preview);
	}

	bool OnInitDialog() override
	{
		char *szProto = Proto_GetBaseAccountName(m_hContact);

		m_list.SetImageList(GetStatusIconsImgList(szProto), LVSIL_SMALL);
		m_list.SetExtendedListViewStyleEx(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

		RECT rc = { 0 };
		GetClientRect(m_list.GetHwnd(), &rc);

		LV_COLUMN lvc = { 0 };
		lvc.mask = LVCF_WIDTH | LVCF_TEXT;
		lvc.cx = STATUS_COLUMN;
		lvc.pszText = TranslateT("Status");
		m_list.InsertColumn(0, &lvc);

		lvc.cx = rc.right - STATUS_COLUMN - GetSystemMetrics(SM_CXVSCROLL);
		lvc.pszText = TranslateT("Sound file");
		m_list.InsertColumn(1, &lvc);

		if (szProto) {
			DBVARIANT dbv;
			wchar_t buff[MAX_PATH];

			for (int i = ID_STATUS_MAX; i >= ID_STATUS_MIN; i--) {
				int flags = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_2, 0);
				if (flags == 0)
					flags = PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT;

				if ((flags & Proto_Status2Flag(i)) || i == ID_STATUS_OFFLINE) {
					LV_ITEM lvi = { 0 };
					lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
					lvi.iItem = 0;
					lvi.iSubItem = 0;
					lvi.iImage = Index(i);
					lvi.lParam = (LPARAM)i;
					lvi.pszText = TranslateW(StatusList[Index(i)].lpzSkinSoundDesc);
					lvi.iItem = m_list.InsertItem(&lvi);

					if (!g_plugin.getWString(m_hContact, StatusList[Index(i)].lpzSkinSoundName, &dbv)) {
						mir_wstrcpy(buff, dbv.pwszVal);
						db_free(&dbv);
					}
					else mir_wstrcpy(buff, TranslateW(DEFAULT_SOUND));

					m_list.SetItemText(lvi.iItem, 1, buff);
				}
			}

			for (int i = 0; i <= ID_STATUSEX_MAX; i++) {
				LV_ITEM lvi = { 0 };
				lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
				lvi.iItem = 0;
				lvi.iSubItem = 0;
				lvi.iImage = Index(ID_STATUS_MAX) + 1; // additional icon
				lvi.lParam = (LPARAM)i;
				lvi.pszText = TranslateW(StatusListEx[i].lpzSkinSoundDesc);
				lvi.iItem = m_list.InsertItem(&lvi);

				if (!g_plugin.getWString(m_hContact, StatusList[i].lpzSkinSoundName, &dbv)) {
					wcsncpy(buff, dbv.pwszVal, _countof(buff) - 1);
					db_free(&dbv);
				}
				else wcsncpy(buff, TranslateW(DEFAULT_SOUND), _countof(buff) - 1);

				m_list.SetItemText(lvi.iItem, 1, buff);
			}
		}

		chkSounds.SetState(g_plugin.getByte(m_hContact, "EnableSounds", 1));
		chkPopups.SetState(g_plugin.getByte(m_hContact, "EnablePopups", 1));

		ShowWindow(GetDlgItem(m_hwnd, IDC_INDSNDLIST), opt.UseIndSnd ? SW_SHOW : SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_TEXT_ENABLE_IS), opt.UseIndSnd ? SW_HIDE : SW_SHOW);
		ShowWindow(GetDlgItem(m_hwnd, IDC_CHANGE), opt.UseIndSnd ? SW_SHOW : SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_PREVIEW), opt.UseIndSnd ? SW_SHOW : SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_DELETE), opt.UseIndSnd ? SW_SHOW : SW_HIDE);
		return true;
	}

	bool OnApply() override
	{
		wchar_t buff[MAX_PATH];

		LVITEM lvi = { 0 };
		lvi.mask = LVIF_PARAM;
		//Cycle through the list reading the text associated to each status.
		for (lvi.iItem = m_list.GetItemCount() - 1; lvi.iItem >= 0; lvi.iItem--) {
			m_list.GetItem(&lvi);
			m_list.GetItemText(lvi.iItem, 1, buff, _countof(buff));

			if (!mir_wstrcmp(buff, TranslateW(DEFAULT_SOUND))) {
				if (lvi.lParam < ID_STATUS_MIN)
					g_plugin.delSetting(m_hContact, StatusListEx[lvi.lParam].lpzSkinSoundName);
				else
					g_plugin.delSetting(m_hContact, StatusList[Index(lvi.lParam)].lpzSkinSoundName);
			}
			else {
				wchar_t stzSoundPath[MAX_PATH] = { 0 };
				PathToRelativeW(buff, stzSoundPath);
				if (lvi.lParam < ID_STATUS_MIN)
					g_plugin.setWString(m_hContact, StatusListEx[lvi.lParam].lpzSkinSoundName, stzSoundPath);
				else
					g_plugin.setWString(m_hContact, StatusList[Index(lvi.lParam)].lpzSkinSoundName, stzSoundPath);
			}
		}

		return true;
	}

	void onClick_Preview(CCtrlButton*)
	{
		if (m_list.GetSelectionMark() != -1)
			PreviewSound();
	}

	void onClick_Change(CCtrlButton*)
	{
		int iSel = m_list.GetNextItem(-1, LVNI_SELECTED);
		if (iSel != -1) {
			wchar_t stzFilePath[MAX_PATH];
			if (SelectSound(m_hwnd, stzFilePath, MAX_PATH - 1) != nullptr) {
				iSel = -1;
				while ((iSel = m_list.GetNextItem(iSel, LVNI_SELECTED)) != -1)
					m_list.SetItemText(iSel, 1, stzFilePath);

				NotifyChange();
			}
		}
	}

	void onClick_Delete(CCtrlButton*)
	{
		if (m_list.GetSelectionMark() != -1)
			if (RemoveSoundFromList())
				NotifyChange();
	}

	void onChange_Popups(CCtrlCheck*)
	{
		g_plugin.setByte(m_hContact, "EnableSounds", chkPopups.GetState());
	}

	void onChange_Sounds(CCtrlCheck*)
	{
		g_plugin.setByte(m_hContact, "EnablePopups", chkSounds.GetState());
	}

	void onDblClick_List(CCtrlListView::TEventInfo*)
	{
		wchar_t stzFilePath[MAX_PATH];
		if (SelectSound(m_hwnd, stzFilePath, MAX_PATH - 1) != nullptr) {
			int iSel = -1;
			while ((iSel = m_list.GetNextItem(iSel, LVNI_SELECTED)) != -1)
				m_list.SetItemText(iSel, 1, stzFilePath);

			SendMessage(GetParent(m_hwnd), PSM_CHANGED, 0, 0);
		}
	}
};

int UserInfoInitialise(WPARAM wParam, LPARAM lParam)
{
	if (lParam) {
		USERINFOPAGE uip = {};
		uip.position = 100000000;
		uip.szTitle.a = LPGEN("Status Notify");
		uip.pDialog = new SoundUIPageDlg();
		g_plugin.addUserInfo(wParam, &uip);
	}
	return 0;
}
