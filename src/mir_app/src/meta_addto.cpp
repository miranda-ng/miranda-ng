/*
former MetaContacts Plugin for Miranda IM.

Copyright © 2014-22 Miranda NG team
Copyright © 2004-07 Scott Ellis
Copyright © 2004 Universite Louis PASTEUR, STRASBOURG.

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

#include "metacontacts.h"

/** Adds all the metacontacts desired in the listview.
*
* Adds all the metacontacts present in the database in the list,
*
* @param list : HANDLE to the list which will contain the columns.
* @param nb_contacts : Number of loaded contacts.
* @param contacts : A list of the contacts' identifiers
*
* @param id : Reference to a list of the MetaContacts IDs loaded in the listview.
*             Since this list is resized, its address must be passed.
*
* @return An integer specifying the number of rows added in the list.
*/

static int FillList(HWND list, BOOL sort)
{
	int i = 0;

	// The DB is searched through, to get all the metacontacts
	for (auto &hMetaUser : Contacts()) {
		// if it's not a MetaContact, go to the next
		DBCachedContact *cc = CheckMeta(hMetaUser);
		if (cc == nullptr)
			continue;

		// get contact display name from clist
		wchar_t *swzContactDisplayName = Clist_GetContactDisplayName(hMetaUser);
		// don't insert huge strings that we have to compare with later
		if (mir_wstrlen(swzContactDisplayName) > 1023)
			swzContactDisplayName[1024] = 0;

		int pos = -1;
		if (sort) {
			for (pos = 0; pos < i; pos++) {
				wchar_t buff[1024];
				SendMessage(list, LB_GETTEXT, pos, (LPARAM)buff);
				if (mir_wstrcmp(buff, swzContactDisplayName) > 0)
					break;
			}
		}

		int index = SendMessage(list, LB_INSERTSTRING, pos, (LPARAM)swzContactDisplayName);
		SendMessage(list, LB_SETITEMDATA, index, hMetaUser);
		i++;
	}
	return i;
}

/** Build or update the list.
*
* @param list : HANDLE to the list which will contain the columns
* @param id : Reference to a list that will contain all the MetaContacts IDs loaded in the listview
*             otherwise the list is only refilled \n (Warning : this value must be
*             set to TRUE only once per Dialog display, otherwise all columns will be doubled)
*
* @returns An integer specifying the number of rows inserted or -1 if there was a problem
*/

static int BuildList(HWND list, BOOL sort)
{
	SendMessage(list, LB_RESETCONTENT, 0, 0);
	return FillList(list, sort);
}

/** Callback function for the <b>'Add To'</b> Dialog.
*
* All the UI is controlled here, from display to functionnalities.
*
* @param hwndDlg : HANDLE to the <b>'Add To'</b> Dialog.
* @param uMsg : Specifies the message received by this dialog.
* @param wParam : Specifies additional message-specific information.
* @param lParam : Specifies additional message-specific information.
*
* @return TRUE if the dialog processed the message, FALSE if it did not.
*/

#define szConvMsg LPGENW("Either there is no metacontact in the database (in this case you should first convert a contact into one)\n\
or there is none that can host this contact.\n\
Another solution could be to convert this contact into a new metacontact.\n\nConvert this contact into a new metacontact?")

class CMetaSelectDlg : public CDlgBase
{
	MCONTACT m_hContact;

	CCtrlListBox m_metaList;
	CCtrlCheck m_sortCheck;

protected:
	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;

	void MetaList_OnDblClick(CCtrlListBox*);
	void SortCheck_OnChange(CCtrlCheck*);

public:
	CMetaSelectDlg(MCONTACT hContact);
};

CMetaSelectDlg::CMetaSelectDlg(MCONTACT hContact)
	: CDlgBase(g_plugin, IDD_METASELECT), m_hContact(hContact),
	m_metaList(this, IDC_METALIST), m_sortCheck(this, IDC_CHK_SRT)
{
	m_metaList.OnDblClick = Callback(this, &CMetaSelectDlg::MetaList_OnDblClick);
	m_sortCheck.OnChange = Callback(this, &CMetaSelectDlg::SortCheck_OnChange);
}

bool CMetaSelectDlg::OnInitDialog()
{
	DBCachedContact *cc = g_pCurrDb->getCache()->GetCachedContact(m_hContact);
	if (cc == nullptr)
		return false;

	if (cc->IsMeta()) {
		MessageBox(GetHwnd(),
			TranslateT("This contact is a metacontact.\nYou can't add a metacontact to another metacontact.\n\nPlease choose another."),
			TranslateT("Metacontact conflict"), MB_ICONERROR);
		return false;
	}

	if (cc->IsSub()) {
		MessageBox(GetHwnd(),
			TranslateT("This contact is already associated to a metacontact.\nYou cannot add a contact to multiple metacontacts."),
			TranslateT("Multiple metacontacts"), MB_ICONERROR);
		return false;
	}

	Window_SetIcon_IcoLib(GetHwnd(), Meta_GetIconHandle(I_ADD));

	// Initialize the graphical part
	CheckDlgButton(GetHwnd(), IDC_ONLYAVAIL, BST_CHECKED); // Initially checked; display all metacontacts is only an option
														 // Besides, we can check if there is at least one metacontact to add the contact to.
	if (BuildList(GetDlgItem(GetHwnd(), IDC_METALIST), FALSE) <= 0) {
		if (MessageBox(GetHwnd(), TranslateW(szConvMsg), TranslateT("No suitable metacontact found"), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1) == IDYES)
			Meta_Convert(m_hContact, 0);
		return false;
	}

	// get contact display name from clist
	wchar_t *ptszCDN = Clist_GetContactDisplayName(m_hContact);
	if (!ptszCDN)
		ptszCDN = TranslateT("a contact");

	// ... and set it to the Window title.
	wchar_t buf[256];
	mir_snwprintf(buf, TranslateT("Adding %s..."), ptszCDN);
	SetWindowText(GetHwnd(), buf);
	ShowWindow(GetHwnd(), SW_SHOWNORMAL);
	return true;
}

bool CMetaSelectDlg::OnApply()
{
	int item = m_metaList.GetCurSel();
	if (item == -1) {
		BOOL result = IDOK == MessageBox(GetHwnd(), TranslateT("Please select a metacontact"), TranslateT("No metacontact selected"), MB_ICONHAND);
		EndModal(result);
	}

	MCONTACT hMeta = (MCONTACT)m_metaList.GetItemData(item);
	if (!Meta_Assign(m_hContact, hMeta, false))
		MessageBox(GetHwnd(), TranslateT("Assignment to the metacontact failed."), TranslateT("Assignment failure"), MB_ICONERROR);
	return true;
}

void CMetaSelectDlg::OnDestroy()
{
	HWND clist = GetParent(GetHwnd());
	Window_FreeIcon_IcoLib(GetHwnd());
	EndModal(TRUE);
	SetFocus(clist);
}

void CMetaSelectDlg::MetaList_OnDblClick(CCtrlListBox*)
{
	OnApply();
	Close();
}

void CMetaSelectDlg::SortCheck_OnChange(CCtrlCheck*)
{
	SetWindowLongPtr(m_metaList.GetHwnd(), GWL_STYLE, GetWindowLongPtr(m_metaList.GetHwnd(), GWL_STYLE) ^ LBS_SORT);
	if (BuildList(m_metaList.GetHwnd(), m_sortCheck.GetState() ? TRUE : FALSE) <= 0) {
		if (MessageBox(GetHwnd(), TranslateW(szConvMsg), TranslateT("No suitable metacontact found"), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1) == IDYES)
			Meta_Convert(m_hContact, 0);
		Close();
	}
}

/** Display the <b>'Add to'</b> Dialog
*
* Present a dialog in which the user can choose to which MetaContact this
* contact will be added
*
* @param wParam : HANDLE to the contact that has been chosen.
* @param lParam :	Allways set to 0.
*/

INT_PTR Meta_AddTo(WPARAM hContact, LPARAM)
{
	CMetaSelectDlg dlg(hContact);
	dlg.SetParent(g_clistApi.hwndContactList);
	dlg.DoModal();
	return 0;
}
