/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

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
	{
		mir_cslock lck(m_GCCreateDialogsLock);
		for (int i = 0; i < m_GCCreateDialogs.getCount(); i++)
			m_GCCreateDialogs[i]->Close();
	}

	{
		mir_cslock lck(m_InviteDialogsLock);
		for (int i = 0; i < m_InviteDialogs.getCount(); i++)
			m_InviteDialogs[i]->Close();
	}
}

//CSkypeInvideDlg
CSkypeInviteDlg::CSkypeInviteDlg(CSkypeProto *proto) :
	CSkypeDlgBase(proto, IDD_GC_INVITE, false), m_ok(this, IDOK), m_cancel(this, IDCANCEL), m_combo(this, IDC_CONTACT)
{
	m_ok.OnClick = Callback(this, &CSkypeInviteDlg::btnOk_OnOk);
}

void CSkypeInviteDlg::OnInitDialog()
{
	for (MCONTACT hContact = db_find_first(m_proto->m_szModuleName); hContact; hContact = db_find_next(hContact, m_proto->m_szModuleName)) 
	{
		if (!m_proto->isChatRoom(hContact))
		{
			TCHAR *ptszNick = pcli->pfnGetContactDisplayName(hContact, 0);
			m_combo.AddString(ptszNick, hContact);
		}
	}
}
void CSkypeInviteDlg::btnOk_OnOk(CCtrlButton*)
{
	m_hContact = m_combo.GetItemData(m_combo.GetCurSel());
	EndDialog(m_hwnd, 1);
}

//CSkypeGCCreateDlg
CSkypeGCCreateDlg::CSkypeGCCreateDlg(CSkypeProto *proto) :
	CSkypeDlgBase(proto, IDD_GC_CREATE, false), m_ok(this, IDOK), m_cancel(this, IDCANCEL), m_clc(this, IDC_CLIST), m_ContactsList(1)
{
	m_ok.OnClick = Callback(this, &CSkypeGCCreateDlg::btnOk_OnOk);
}
CSkypeGCCreateDlg::~CSkypeGCCreateDlg()
{
	CSkypeProto::FreeList(m_ContactsList);
	m_ContactsList.destroy();
}

void CSkypeGCCreateDlg::OnInitDialog()
{
	SetWindowLongPtr(m_clc.GetHwnd(), GWL_STYLE,
		GetWindowLongPtr(m_clc.GetHwnd(), GWL_STYLE) | CLS_CHECKBOXES | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE);
	m_clc.SendMsg(CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP | CLS_EX_TRACKSELECT, 0);

	ResetListOptions(&m_clc);
	FilterList(&m_clc);
}

void CSkypeGCCreateDlg::btnOk_OnOk(CCtrlButton*)
{
	for (MCONTACT hContact = db_find_first(m_proto->m_szModuleName); hContact; hContact = db_find_next(hContact, m_proto->m_szModuleName)) 
	{
		if (!m_proto->isChatRoom(hContact))
		{
			if (HANDLE hItem = m_clc.FindContact(hContact)) 
			{
				if (m_clc.GetCheck(hItem)) 
				{
					char *szName = db_get_sa(hContact, m_proto->m_szModuleName, SKYPE_SETTINGS_ID);
					if (szName != NULL)
						m_ContactsList.insert(szName);
				}
			}
		}
	}
	m_ContactsList.insert(m_proto->li.szSkypename);
	EndDialog(m_hwnd, m_ContactsList.getCount());
}

void CSkypeGCCreateDlg::FilterList(CCtrlClc *)
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		char *proto = GetContactProto(hContact);
		if (mir_strcmp(proto, m_proto->m_szModuleName) || m_proto->isChatRoom(hContact))
			if (HANDLE hItem = m_clc.FindContact(hContact))
				m_clc.DeleteItem(hItem);
	}
}

void CSkypeGCCreateDlg::ResetListOptions(CCtrlClc *)
{
	m_clc.SetBkBitmap(0, NULL);
	m_clc.SetBkColor(GetSysColor(COLOR_WINDOW));
	m_clc.SetGreyoutFlags(0);
	m_clc.SetLeftMargin(4);
	m_clc.SetIndent(10);
	m_clc.SetHideEmptyGroups(true);
	m_clc.SetHideOfflineRoot(true);
	for (int i = 0; i <= FONTID_MAX; i++)
		m_clc.SetTextColor(i, GetSysColor(COLOR_WINDOWTEXT));
}