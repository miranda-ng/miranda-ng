/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
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

static wchar_t* sttDecodeString(DWORD dwFlags, MAllStrings &src)
{
	if (dwFlags & PSR_UNICODE)
		return mir_wstrdup(src.w);

	if (dwFlags & PSR_UTF8)
		return mir_utf8decodeW(src.a);

	return mir_a2u(src.a);
}

class CAddContactDlg : public CDlgBase
{
	ADDCONTACTSTRUCT m_acs;

	CCtrlEdit   m_authReq, m_myHandle;
	CCtrlCheck  m_chkAdded, m_chkAuth, m_chkOpen;
	CCtrlButton m_btnOk;
	CCtrlCombo  m_group;

public:
	CAddContactDlg(ADDCONTACTSTRUCT *acs) :
		CDlgBase(g_hInst, IDD_ADDCONTACT),
		m_chkAdded(this, IDC_ADDED),
		m_chkAuth(this, IDC_AUTH),
		m_chkOpen(this, IDC_OPEN_WINDOW),
		m_btnOk(this, IDOK),
		m_group(this, IDC_GROUP),
		m_authReq(this, IDC_AUTHREQ),
		m_myHandle(this, IDC_MYHANDLE)
	{
		m_chkAuth.OnChange = Callback(this, &CAddContactDlg::OnAuthClicked);
		m_chkOpen.OnChange = Callback(this, &CAddContactDlg::OnOpenClicked);
		m_btnOk.OnClick = Callback(this, &CAddContactDlg::OnOk);

		m_acs = *acs;
	}

	void OnInitDialog()
	{
		char szUin[10];
		Window_SetSkinIcon_IcoLib(m_hwnd, SKINICON_OTHER_ADDCONTACT);
		if (m_acs.handleType == HANDLE_EVENT) {
			DWORD dwUin;
			DBEVENTINFO dbei = {};
			dbei.cbBlob = sizeof(DWORD);
			dbei.pBlob = (PBYTE)&dwUin;
			db_event_get(m_acs.hDbEvent, &dbei);
			_ltoa(dwUin, szUin, 10);
			m_acs.szProto = dbei.szModule;
		}

		MCONTACT hContact;
		wchar_t *szName = nullptr, *tmpStr = nullptr;
		if (m_acs.handleType == HANDLE_CONTACT)
			szName = cli.pfnGetContactDisplayName(hContact = m_acs.hContact, 0);
		else {
			int isSet = 0;
			hContact = 0;

			if (m_acs.handleType == HANDLE_EVENT) {
				DBEVENTINFO dbei = {};
				dbei.cbBlob = db_event_getBlobSize(m_acs.hDbEvent);
				dbei.pBlob = (PBYTE)mir_alloc(dbei.cbBlob);
				db_event_get(m_acs.hDbEvent, &dbei);
				hContact = *(MCONTACT*)(dbei.pBlob + sizeof(DWORD));
				mir_free(dbei.pBlob);
				if (hContact != INVALID_CONTACT_ID) {
					szName = cli.pfnGetContactDisplayName(hContact, 0);
					isSet = 1;
				}
			}
			if (!isSet) {
				if (m_acs.handleType == HANDLE_EVENT)
					szName = mir_a2u(szUin);
				else {
					szName = sttDecodeString(m_acs.psr->flags, m_acs.psr->id);
					if (szName == nullptr)
						szName = sttDecodeString(m_acs.psr->flags, m_acs.psr->nick);
				}
			}
		}

		if (szName && szName[0])
			SetCaption(CMStringW(FORMAT, TranslateT("Add %s"), szName));
		else
			SetCaption(TranslateT("Add contact"));
		mir_free(tmpStr);

		if (m_acs.handleType == HANDLE_CONTACT && m_acs.hContact)
			if (m_acs.szProto == nullptr || (m_acs.szProto != nullptr && *m_acs.szProto == 0))
				m_acs.szProto = GetContactProto(m_acs.hContact);

		int groupSel = 0;
		ptrW tszGroup(db_get_wsa(hContact, "CList", "Group"));
		wchar_t *grpName;
		for (int groupId = 1; (grpName = Clist_GroupGetName(groupId, nullptr)) != nullptr; groupId++) {
			int id = m_group.AddString(grpName, groupId);
			if (!mir_wstrcmpi(tszGroup, grpName))
				groupSel = id;
		}

		m_group.InsertString(TranslateT("None"), 0);
		m_group.SetCurSel(groupSel);

		// By default check both checkboxes
		m_chkAdded.SetState(true);
		m_chkAuth.SetState(true);

		// Set last choice
		if (db_get_b(0, "Miranda", "AuthOpenWindow", 1))
			m_chkOpen.SetState(true);

		DWORD flags = (m_acs.szProto) ? CallProtoServiceInt(0, m_acs.szProto, PS_GETCAPS, PFLAGNUM_4, 0) : 0;
		if (flags & PF4_FORCEADDED)  // force you were added requests for this protocol
			m_chkAdded.Enable(false);

		if (flags & PF4_FORCEAUTH)  // force auth requests for this protocol
			m_chkAuth.Enable(false);

		if (flags & PF4_NOCUSTOMAUTH)
			m_authReq.Enable(false);
		else {
			m_authReq.Enable(m_chkAuth.Enabled());
			m_authReq.SetText(TranslateT("Please authorize my request and add me to your contact list."));
		}
	}

	void OnDestroy()
	{
		Window_FreeIcon_IcoLib(m_hwnd);
	}

	void OnAuthClicked(CCtrlButton*)
	{
		DWORD flags = CallProtoServiceInt(0, m_acs.szProto, PS_GETCAPS, PFLAGNUM_4, 0);
		if (flags & PF4_NOCUSTOMAUTH)
			m_authReq.Enable(false);
		else
			m_authReq.Enable(m_chkAuth.Enabled());
	}

	void OnOpenClicked(CCtrlButton*)
	{
		// Remember this choice
		db_set_b(0, "Miranda", "AuthOpenWindow", m_chkOpen.Enabled());
	}

	void OnOk(CCtrlButton*)
	{
		MCONTACT hContact = INVALID_CONTACT_ID;
		switch (m_acs.handleType) {
		case HANDLE_EVENT:
			{
				DBEVENTINFO dbei = {};
				db_event_get(m_acs.hDbEvent, &dbei);
				hContact = (MCONTACT)CallProtoServiceInt(0, dbei.szModule, PS_ADDTOLISTBYEVENT, 0, (LPARAM)m_acs.hDbEvent);
			}
			break;

		case HANDLE_SEARCHRESULT:
			hContact = (MCONTACT)CallProtoServiceInt(0, m_acs.szProto, PS_ADDTOLIST, 0, (LPARAM)m_acs.psr);
			break;

		case HANDLE_CONTACT:
			hContact = m_acs.hContact;
			break;
		}

		if (hContact == 0)
			return;

		ptrW szHandle(m_myHandle.GetText());
		if (mir_wstrlen(szHandle))
			db_set_ws(hContact, "CList", "MyHandle", szHandle);

		int item = m_group.GetCurSel();
		if (item > 0)
			Clist_ContactChangeGroup(hContact, m_group.GetItemData(item));

		db_unset(hContact, "CList", "NotOnList");

		if (m_chkAdded.GetState())
			ProtoChainSend(hContact, PSS_ADDED, 0, 0);

		if (m_chkAuth.GetState()) {
			DWORD flags = CallProtoServiceInt(0, m_acs.szProto, PS_GETCAPS, PFLAGNUM_4, 0);
			if (flags & PF4_NOCUSTOMAUTH)
				ProtoChainSend(hContact, PSS_AUTHREQUEST, 0, 0);
			else
				ProtoChainSend(hContact, PSS_AUTHREQUEST, 0, ptrW(m_authReq.GetText()));
		}

		if (m_chkOpen.GetState())
			Clist_ContactDoubleClicked(hContact);
	}
};

INT_PTR AddContactDialog(WPARAM wParam, LPARAM lParam)
{
	if (lParam == 0)
		return 1;

	ADDCONTACTSTRUCT *acs = (ADDCONTACTSTRUCT*)lParam;
	if (wParam) {
		CAddContactDlg dlg(acs);
		dlg.SetParent((HWND)wParam);
		dlg.DoModal();
	}
	else (new CAddContactDlg(acs))->Show();
	return 0;
}

int LoadAddContactModule(void)
{
	CreateServiceFunction(MS_ADDCONTACT_SHOW, AddContactDialog);
	return 0;
}
