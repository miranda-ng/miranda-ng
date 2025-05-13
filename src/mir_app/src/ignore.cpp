/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org),
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
#include "clc.h"

#define IGNOREEVENT_MAX 6

static int masks[IGNOREEVENT_MAX] = { 0x0001, 0x0020, 0x0004, 0x0008, 0x0010, 0x0040 };

static uint32_t ignoreIdToPf1[IGNOREEVENT_MAX] = { PF1_IMRECV, 0xFFFFFFFF, PF1_FILERECV, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
static uint32_t ignoreIdToPf4[IGNOREEVENT_MAX] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, PF4_SUPPORTTYPING };

static uint32_t GetMask(MCONTACT hContact)
{
	uint32_t mask = db_get_dw(hContact, "Ignore", "Mask1", (uint32_t)-1);
	if (mask != (uint32_t)-1)
		return mask;

	if (hContact == 0)
		return 0;

	ptrW pwszGroup(Clist_GetGroup(hContact));
	if (pwszGroup) {
		while (true) {
			if (auto *pGroup = FindGroup(pwszGroup))
				if (pGroup->ignore)
					return pGroup->ignore;

			auto *p = wcsrchr(pwszGroup, '\\');
			if (p)
				*p = 0;
			else
				break;
		}
	}

	if ((Contact::IsHidden(hContact) && !db_mc_isSub(hContact)) || !Contact::OnList(hContact))
		return db_get_dw(0, "Ignore", "Mask1", 0);
	return db_get_dw(0, "Ignore", "Default1", 0);	
}

static void SaveItemValue(MCONTACT hContact, const char *pszSetting, uint32_t dwValue)
{
	db_set_dw(hContact, "Ignore", pszSetting, dwValue);

	// assign the same value to all subs, if any
	for (int i = db_mc_getSubCount(hContact) - 1; i >= 0; i--)
		db_set_dw(db_mc_getSub(hContact, i), "Ignore", pszSetting, dwValue);
}

class IgnoreOptsDlg : public CDlgBase
{
	HICON hIcons[IGNOREEVENT_MAX + 2];
	HANDLE hItemAll, hItemUnknown;

	CCtrlClc m_clist;

	void SaveGroupValue(HANDLE hFirstItem)
	{
		int typeOfFirst = m_clist.GetItemType(hFirstItem);

		HANDLE hItem = (typeOfFirst == CLCIT_GROUP) ? hFirstItem : m_clist.GetNextItem(hFirstItem, CLGN_NEXTGROUP);
		while (hItem) {
			if (HANDLE hChildItem = m_clist.GetNextItem(hItem, CLGN_CHILD))
				SaveGroupValue(hChildItem);

			MGROUP hGroup = UINT_PTR(hItem) & ~HCONTACT_ISGROUP;
			Clist_GroupSetIgnore(hGroup, GetItemMask(hItem), !m_clist.GetCheck(hItem));

			hItem = m_clist.GetNextItem(hItem, CLGN_NEXTGROUP);
		}
	}

	void SetListGroupIcons(HANDLE hFirstItem, HANDLE hParentItem)
	{
		int typeOfFirst = m_clist.GetItemType(hFirstItem);

		// check groups
		HANDLE hItem = (typeOfFirst == CLCIT_GROUP) ? hFirstItem : m_clist.GetNextItem(hFirstItem, CLGN_NEXTGROUP);
		while (hItem) {
			if (HANDLE hChildItem = m_clist.GetNextItem(hItem, CLGN_CHILD))
				SetListGroupIcons(hChildItem, hItem);

			bool isChecked;
			MGROUP hGroup = UINT_PTR(hItem) & ~HCONTACT_ISGROUP;
			InitialiseItem(Clist_GroupGetIgnore(hGroup, &isChecked), hItem, 0xFFFFFFFF, 0xFFFFFFFF);

			m_clist.SetCheck(hItem, isChecked);

			hItem = m_clist.GetNextItem(hItem, CLGN_NEXTGROUP);
		}
		m_clist.SetExtraImage(hParentItem, IGNOREEVENT_MAX, 1);
		m_clist.SetExtraImage(hParentItem, IGNOREEVENT_MAX + 1, 2);
	}

	void SetAllChildIcons(HANDLE hFirstItem, int iColumn, int iImage)
	{
		int typeOfFirst = m_clist.GetItemType(hFirstItem);

		// check groups
		HANDLE hItem = (typeOfFirst == CLCIT_GROUP) ? hFirstItem : m_clist.GetNextItem(hFirstItem, CLGN_NEXTGROUP);
		while (hItem) {
			if (HANDLE hChildItem = m_clist.GetNextItem(hItem, CLGN_CHILD))
				SetAllChildIcons(hChildItem, iColumn, iImage);

			int iOldIcon = m_clist.GetExtraImage(hItem, iColumn);
			if (iOldIcon != EMPTY_EXTRA_ICON && iOldIcon != iImage)
				m_clist.SetExtraImage(hItem, iColumn, iImage);

			hItem = m_clist.GetNextItem(hItem, CLGN_NEXTGROUP);
		}

		// check contacts
		hItem = (typeOfFirst == CLCIT_CONTACT) ? hFirstItem : m_clist.GetNextItem(hFirstItem, CLGN_NEXTCONTACT);
		while (hItem) {
			int iOldIcon = m_clist.GetExtraImage(hItem, iColumn);
			if (iOldIcon != EMPTY_EXTRA_ICON && iOldIcon != iImage)
				m_clist.SetExtraImage(hItem, iColumn, iImage);
			hItem = m_clist.GetNextItem(hItem, CLGN_NEXTCONTACT);
		}
	}

	void ResetListOptions()
	{
		m_clist.SetHideEmptyGroups(true);
	}

	void SetIconsForColumn(HANDLE hItem, int iColumn, int iImage)
	{
		switch (m_clist.GetItemType(hItem)) {
		case CLCIT_INFO:
			if (hItem == hItemAll)
				SetAllChildIcons(hItem, iColumn, iImage);
			m_clist.SetExtraImage(hItem, iColumn, iImage); // hItemUnknown
			break;

		case CLCIT_GROUP:
			if (HANDLE hChild = m_clist.GetNextItem(hItem, CLGN_CHILD))
				SetAllChildIcons(hChild, iColumn, iImage);
			__fallthrough;

		case CLCIT_CONTACT:
			int oldiImage = m_clist.GetExtraImage(hItem, iColumn);
			if (oldiImage != EMPTY_EXTRA_ICON && oldiImage != iImage)
				m_clist.SetExtraImage(hItem, iColumn, iImage);
			break;
		}
	}

	void InitialiseItem(uint32_t mask, HANDLE hItem, uint32_t proto1Caps, uint32_t proto4Caps)
	{
		for (int i = 0; i < IGNOREEVENT_MAX; i++)
			if ((ignoreIdToPf1[i] == 0xFFFFFFFF && ignoreIdToPf4[i] == 0xFFFFFFFF) || (proto1Caps & ignoreIdToPf1[i] || proto4Caps & ignoreIdToPf4[i]))
				m_clist.SetExtraImage(hItem, i, (mask & masks[i]) ? i + 3 : 0);

		m_clist.SetExtraImage(hItem, IGNOREEVENT_MAX, 1);
		m_clist.SetExtraImage(hItem, IGNOREEVENT_MAX + 1, 2);
	}

	uint32_t GetItemMask(HANDLE hItem)
	{
		uint32_t mask = 0;
		for (int i = 0; i < IGNOREEVENT_MAX; i++) {
			int iImage = m_clist.GetExtraImage(hItem, i);
			if (iImage && iImage != EMPTY_EXTRA_ICON)
				mask |= masks[i];
		}
		return mask;
	}

	void SetAllContactIcons()
	{
		for (auto &hContact : Contacts()) {
			HANDLE hItem = m_clist.FindContact(hContact);
			if (hItem && m_clist.GetExtraImage(hItem, IGNOREEVENT_MAX) == EMPTY_EXTRA_ICON) {
				uint32_t proto1Caps, proto4Caps;
				char *szProto = Proto_GetBaseAccountName(hContact);
				if (szProto) {
					proto1Caps = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1);
					proto4Caps = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4);
				}
				else proto1Caps = proto4Caps = 0;
				InitialiseItem(GetMask(hContact), hItem, proto1Caps, proto4Caps);
				if (!Contact::IsHidden(hContact))
					m_clist.SetCheck(hItem, 1);
			}
		}
	}

public:
	IgnoreOptsDlg() :
		CDlgBase(g_plugin, IDD_OPT_IGNORE),
		m_clist(this, IDC_LIST)
	{
		m_clist.OnClick = Callback(this, &IgnoreOptsDlg::onClistClicked);
		m_clist.OnOptionsChanged = Callback(this, &IgnoreOptsDlg::onClistOptionsChanged);
		m_clist.OnListRebuilt = m_clist.OnNewContact = Callback(this, &IgnoreOptsDlg::onClistRebuilt);
	}

	bool OnInitDialog() override
	{
		HIMAGELIST hIml = ImageList_Create(g_iIconSX, g_iIconSY, ILC_COLOR32 | ILC_MASK, 3 + IGNOREEVENT_MAX, 3 + IGNOREEVENT_MAX);
		ImageList_AddSkinIcon(hIml, SKINICON_OTHER_SMALLDOT);
		ImageList_AddSkinIcon(hIml, SKINICON_OTHER_FILLEDBLOB);
		ImageList_AddSkinIcon(hIml, SKINICON_OTHER_EMPTYBLOB);
		ImageList_AddSkinIcon(hIml, SKINICON_EVENT_MESSAGE);
		ImageList_AddSkinIcon(hIml, SKINICON_EVENT_FILE);
		ImageList_AddIcon(hIml, g_plugin.getIcon(IDI_CLOUD));
		ImageList_AddSkinIcon(hIml, SKINICON_OTHER_USERONLINE);
		ImageList_AddSkinIcon(hIml, SKINICON_AUTH_REQUEST);
		ImageList_AddSkinIcon(hIml, SKINICON_OTHER_TYPING);

		m_clist.SetExtraImageList(hIml);
		for (int i = 0; i < _countof(hIcons); i++)
			hIcons[i] = ImageList_GetIcon(hIml, 1 + i, ILD_NORMAL);

		SendDlgItemMessage(m_hwnd, IDC_ALLICON, STM_SETICON, (WPARAM)hIcons[0], 0);
		SendDlgItemMessage(m_hwnd, IDC_NONEICON, STM_SETICON, (WPARAM)hIcons[1], 0);
		SendDlgItemMessage(m_hwnd, IDC_MSGICON, STM_SETICON, (WPARAM)hIcons[2], 0);
		SendDlgItemMessage(m_hwnd, IDC_FILEICON, STM_SETICON, (WPARAM)hIcons[3], 0);
		SendDlgItemMessage(m_hwnd, IDC_OFFLINEICON, STM_SETICON, (WPARAM)hIcons[4], 0);
		SendDlgItemMessage(m_hwnd, IDC_ONLINEICON, STM_SETICON, (WPARAM)hIcons[5], 0);
		SendDlgItemMessage(m_hwnd, IDC_AUTHICON, STM_SETICON, (WPARAM)hIcons[6], 0);
		SendDlgItemMessage(m_hwnd, IDC_TYPINGICON, STM_SETICON, (WPARAM)hIcons[7], 0);

		ResetListOptions();
		m_clist.SetExtraColumns(IGNOREEVENT_MAX + 2);

		CLCINFOITEM cii = { sizeof(cii) };
		cii.flags = CLCIIF_GROUPFONT;
		cii.pszText = TranslateT("** All contacts **");
		hItemAll = m_clist.AddInfoItem(&cii);
		InitialiseItem(0, hItemAll, 0xFFFFFFFF, 0xFFFFFFFF);

		cii.pszText = TranslateT("** Unknown contacts **");
		hItemUnknown = m_clist.AddInfoItem(&cii);
		InitialiseItem(GetMask(0), hItemUnknown, 0xFFFFFFFF, 0xFFFFFFFF);

		SetFocus(m_clist.GetHwnd());
		SetAllContactIcons();
		SetListGroupIcons(m_clist.GetNextItem(0, CLGN_ROOT), hItemAll);
		return true;
	}

	bool OnApply() override
	{
		for (auto &hContact : Contacts()) {
			HANDLE hItem = m_clist.FindContact(hContact);
			if (hItem)
				SaveItemValue(hContact, "Mask1", GetItemMask(hItem));
			Contact::Hide(hContact, !m_clist.GetCheck(hItem));
		}

		SaveGroupValue(m_clist.GetNextItem(0, CLGN_ROOT));

		SaveItemValue(0, "Default1", GetItemMask(hItemAll));
		SaveItemValue(0, "Mask1", GetItemMask(hItemUnknown));
		return true;
	}

	void OnDestroy() override
	{
		for (auto &it : hIcons)
			DestroyIcon(it);

		ImageList_Destroy(m_clist.GetExtraImageList());
	}

	void onClistRebuilt(CCtrlClc *)
	{
		SetAllContactIcons();
		SetListGroupIcons(m_clist.GetNextItem(0, CLGN_ROOT), hItemAll);
	}

	void onClistOptionsChanged(CCtrlClc *)
	{
		ResetListOptions();
	}

	void onClistClicked(CCtrlClc::TEventInfo *ev)
	{
		auto *nm = ev->info;

		uint32_t hitFlags;
		HANDLE hItem = m_clist.HitTest(nm->pt.x, nm->pt.y, &hitFlags);
		if (hItem == nullptr || !(hitFlags & CLCHT_ONITEMEXTRA))
			return;

		if (nm->iColumn == IGNOREEVENT_MAX) { // ignore all
			for (int iImage = 0; iImage < IGNOREEVENT_MAX; iImage++)
				SetIconsForColumn(hItem, iImage, iImage + 3);
		}
		else if (nm->iColumn == IGNOREEVENT_MAX + 1) {	// ignore none
			for (int iImage = 0; iImage < IGNOREEVENT_MAX; iImage++)
				SetIconsForColumn(hItem, iImage, 0);
		}
		else {
			int iImage = m_clist.GetExtraImage(hItem, nm->iColumn);
			if (iImage == 0)
				iImage = nm->iColumn + 3;
			else if (iImage != EMPTY_EXTRA_ICON)
				iImage = 0;
			SetIconsForColumn(hItem, nm->iColumn, iImage);
		}

		NotifyChange();
	}
};

static int IgnoreOptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 900000000;
	odp.szTitle.a = LPGEN("Ignore");
	odp.szGroup.a = LPGEN("Contacts");
	odp.pDialog = new IgnoreOptsDlg();
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
