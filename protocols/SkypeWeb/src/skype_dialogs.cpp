/*
Copyright (c) 2015-20 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

void CSkypeProto::CloseDialogs()
{
	{	mir_cslock lck(m_GCCreateDialogsLock);
		for (auto &it : m_GCCreateDialogs)
			it->Close();
	}

	{	mir_cslock lck(m_InviteDialogsLock);
		for (auto &it : m_InviteDialogs)
			it->Close();
	}
}

//CSkypeInvideDlg
CSkypeInviteDlg::CSkypeInviteDlg(CSkypeProto *proto) :
	CSkypeDlgBase(proto, IDD_GC_INVITE), m_ok(this, IDOK), m_cancel(this, IDCANCEL), m_combo(this, IDC_CONTACT), m_hContact(NULL)
{
	m_ok.OnClick = Callback(this, &CSkypeInviteDlg::btnOk_OnOk);
}

bool CSkypeInviteDlg::OnInitDialog()
{
	for (auto &hContact : m_proto->AccContacts())
		if (!m_proto->isChatRoom(hContact))
			m_combo.AddString(Clist_GetContactDisplayName(hContact), hContact);
	return true;
}

void CSkypeInviteDlg::btnOk_OnOk(CCtrlButton*)
{
	m_hContact = m_combo.GetItemData(m_combo.GetCurSel());
	EndDialog(m_hwnd, 1);
}

//CSkypeGCCreateDlg
CSkypeGCCreateDlg::CSkypeGCCreateDlg(CSkypeProto *proto) :
	CSkypeDlgBase(proto, IDD_GC_CREATE), m_ok(this, IDOK), m_cancel(this, IDCANCEL), m_clc(this, IDC_CLIST), m_ContactsList(1)
{
	m_ok.OnClick = Callback(this, &CSkypeGCCreateDlg::btnOk_OnOk);
	m_clc.OnListRebuilt = Callback(this, &CSkypeGCCreateDlg::FilterList);
}

CSkypeGCCreateDlg::~CSkypeGCCreateDlg()
{
	CSkypeProto::FreeList(m_ContactsList);
	m_ContactsList.destroy();
}

bool CSkypeGCCreateDlg::OnInitDialog()
{
	SetWindowLongPtr(m_clc.GetHwnd(), GWL_STYLE,
		GetWindowLongPtr(m_clc.GetHwnd(), GWL_STYLE) | CLS_CHECKBOXES | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE);
	m_clc.SendMsg(CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP | CLS_EX_TRACKSELECT, 0);

	ResetListOptions(&m_clc);
	return true;
}

void CSkypeGCCreateDlg::btnOk_OnOk(CCtrlButton*)
{
	for (auto &hContact : m_proto->AccContacts()) {
		if (!m_proto->isChatRoom(hContact))
			if (HANDLE hItem = m_clc.FindContact(hContact))
				if (m_clc.GetCheck(hItem)) 
					m_ContactsList.insert(m_proto->getId(hContact).Detach());
	}
	
	m_ContactsList.insert(m_proto->m_szSkypename.GetBuffer());
	EndDialog(m_hwnd, m_ContactsList.getCount());
}

void CSkypeGCCreateDlg::FilterList(CCtrlClc *)
{
	for (auto &hContact : Contacts()) {
		char *proto = Proto_GetBaseAccountName(hContact);
		if (mir_strcmp(proto, m_proto->m_szModuleName) || m_proto->isChatRoom(hContact))
			if (HANDLE hItem = m_clc.FindContact(hContact))
				m_clc.DeleteItem(hItem);
	}
}

void CSkypeGCCreateDlg::ResetListOptions(CCtrlClc *)
{
	m_clc.SetHideEmptyGroups(true);
	m_clc.SetHideOfflineRoot(true);
}
