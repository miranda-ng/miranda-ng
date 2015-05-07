/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\core\commonheaders.h"

class CAddContactDlg : public CDlgBase
{
	ADDCONTACTSTRUCT m_acs;

	CCtrlEdit   m_authReq, m_myHandle;
	CCtrlCheck  m_chkAdded, m_chkAuth, m_chkOpen;
	CCtrlButton m_btnOk;
	CCtrlCombo  m_group;

public:
	CAddContactDlg(ADDCONTACTSTRUCT *acs) :
		CDlgBase(hInst, IDD_ADDCONTACT),
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
		if (m_acs.psr) {
			PROTOSEARCHRESULT *psr = (PROTOSEARCHRESULT*)mir_alloc(m_acs.psr->cbSize);
			memcpy(psr, m_acs.psr, m_acs.psr->cbSize);
			psr->nick = (psr->flags & PSR_UNICODE) ? mir_u2t((wchar_t*)psr->nick) : mir_a2t((char*)psr->nick);
			psr->firstName = (psr->flags & PSR_UNICODE) ? mir_u2t((wchar_t*)psr->firstName) : mir_a2t((char*)psr->firstName);
			psr->lastName = (psr->flags & PSR_UNICODE) ? mir_u2t((wchar_t*)psr->lastName) : mir_a2t((char*)psr->lastName);
			psr->email = (psr->flags & PSR_UNICODE) ? mir_u2t((wchar_t*)psr->email) : mir_a2t((char*)psr->email);
			psr->id = (psr->flags & PSR_UNICODE) ? mir_u2t((wchar_t*)psr->id) : mir_a2t((char*)psr->id);
			psr->flags = psr->flags & ~PSR_UNICODE | PSR_TCHAR;
			m_acs.psr = psr;
		}
	}

	void OnInitDialog()
	{
		char szUin[10];
		Window_SetIcon_IcoLib(m_hwnd, SKINICON_OTHER_ADDCONTACT);
		if (m_acs.handleType == HANDLE_EVENT) {
			DWORD dwUin;
			DBEVENTINFO dbei = { sizeof(dbei) };
			dbei.cbBlob = sizeof(DWORD);
			dbei.pBlob = (PBYTE)&dwUin;
			db_event_get(m_acs.hDbEvent, &dbei);
			_ltoa(dwUin, szUin, 10);
			m_acs.szProto = dbei.szModule;
		}

		MCONTACT hContact;
		TCHAR *szName = NULL, *tmpStr = NULL;
		if (m_acs.handleType == HANDLE_CONTACT)
			szName = cli.pfnGetContactDisplayName(hContact = m_acs.hContact, 0);
		else {
			int isSet = 0;
			hContact = 0;

			if (m_acs.handleType == HANDLE_EVENT) {
				DBEVENTINFO dbei = { sizeof(dbei) };
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
				szName = (m_acs.handleType == HANDLE_EVENT) ? (tmpStr = mir_a2t(szUin)) :
					(m_acs.psr->id ? m_acs.psr->id : m_acs.psr->nick);
			}
		}

		if (szName && szName[0])
			SetCaption(CMString(FORMAT, TranslateT("Add %s"), szName));
		else
			SetCaption(TranslateT("Add contact"));
		mir_free(tmpStr);

		if (m_acs.handleType == HANDLE_CONTACT && m_acs.hContact)
			if (m_acs.szProto == NULL || (m_acs.szProto != NULL && *m_acs.szProto == 0))
				m_acs.szProto = GetContactProto(m_acs.hContact);

		int groupSel = 0;
		ptrT tszGroup(db_get_tsa(hContact, "CList", "Group"));
		TCHAR *grpName;
		for (int groupId = 1; (grpName = cli.pfnGetGroupName(groupId, NULL)) != NULL; groupId++) {
			int id = m_group.AddString(grpName, groupId);
			if (!mir_tstrcmpi(tszGroup, grpName))
				groupSel = id;
		}

		m_group.InsertString(TranslateT("None"), 0);
		m_group.SetCurSel(groupSel);

		// By default check both checkboxes
		m_chkAdded.SetState(true);
		m_chkAuth.SetState(true);

		// Set last choice
		if (db_get_b(NULL, "Miranda", "AuthOpenWindow", 1))
			m_chkOpen.SetState(true);

		DWORD flags = (m_acs.szProto) ? CallProtoServiceInt(NULL, m_acs.szProto, PS_GETCAPS, PFLAGNUM_4, 0) : 0;
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

		if (m_acs.psr) {
			mir_free(m_acs.psr->nick);
			mir_free(m_acs.psr->firstName);
			mir_free(m_acs.psr->lastName);
			mir_free(m_acs.psr->email);
			mir_free(m_acs.psr);
		}
	}

	void OnAuthClicked(CCtrlButton*)
	{
		DWORD flags = CallProtoServiceInt(NULL, m_acs.szProto, PS_GETCAPS, PFLAGNUM_4, 0);
		if (flags & PF4_NOCUSTOMAUTH)
			m_authReq.Enable(false);
		else
			m_authReq.Enable(m_chkAuth.Enabled());
	}

	void OnOpenClicked(CCtrlButton*)
	{
		// Remember this choice
		db_set_b(NULL, "Miranda", "AuthOpenWindow", m_chkOpen.Enabled());
	}

	void OnOk(CCtrlButton*)
	{
		MCONTACT hContact = INVALID_CONTACT_ID;
		switch (m_acs.handleType) {
		case HANDLE_EVENT:
			{
				DBEVENTINFO dbei = { sizeof(dbei) };
				db_event_get(m_acs.hDbEvent, &dbei);
				hContact = (MCONTACT)CallProtoServiceInt(NULL, dbei.szModule, PS_ADDTOLISTBYEVENT, 0, (LPARAM)m_acs.hDbEvent);
			}
			break;

		case HANDLE_SEARCHRESULT:
			hContact = (MCONTACT)CallProtoServiceInt(NULL, m_acs.szProto, PS_ADDTOLIST, 0, (LPARAM)m_acs.psr);
			break;

		case HANDLE_CONTACT:
			hContact = m_acs.hContact;
			break;
		}

		if (hContact == NULL)
			return;

		ptrT szHandle(m_myHandle.GetText());
		if (mir_tstrlen(szHandle))
			db_set_ts(hContact, "CList", "MyHandle", szHandle);

		int item = m_group.GetCurSel();
		if (item > 0)
			CallService(MS_CLIST_CONTACTCHANGEGROUP, hContact, m_group.GetItemData(item));

		db_unset(hContact, "CList", "NotOnList");

		if (m_chkAdded.GetState())
			CallContactService(hContact, PSS_ADDED, 0, 0);

		if (m_chkAuth.GetState()) {
			DWORD flags = CallProtoServiceInt(NULL, m_acs.szProto, PS_GETCAPS, PFLAGNUM_4, 0);
			if (flags & PF4_NOCUSTOMAUTH)
				CallContactService(hContact, PSS_AUTHREQUESTT, 0, 0);
			else
				CallContactService(hContact, PSS_AUTHREQUESTT, 0, ptrT(m_authReq.GetText()));
		}

		if (m_chkOpen.GetState())
			CallService(MS_CLIST_CONTACTDOUBLECLICKED, hContact, 0);
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
