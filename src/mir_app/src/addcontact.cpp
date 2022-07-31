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

static wchar_t* sttDecodeString(uint32_t dwFlags, MAllStrings &src)
{
	if (dwFlags & PSR_UNICODE)
		return mir_wstrdup(src.w);

	if (dwFlags & PSR_UTF8)
		return mir_utf8decodeW(src.a);

	return mir_a2u(src.a);
}

class CAddContactDlg : public CDlgBase
{
	CCtrlEdit   m_authReq, m_myHandle;
	CCtrlCheck  m_chkAuth, m_chkOpen;
	CCtrlCombo  m_group;

protected:
	MEVENT m_hDbEvent = 0;
	MCONTACT m_hContact = 0;
	const char *m_szProto;
	PROTOSEARCHRESULT *m_psr = nullptr;
	CMStringW m_szName;

public:
	CAddContactDlg() :
		CDlgBase(g_plugin, IDD_ADDCONTACT),
		m_chkAuth(this, IDC_AUTH),
		m_chkOpen(this, IDC_OPEN_WINDOW),
		m_group(this, IDC_GROUP),
		m_authReq(this, IDC_AUTHREQ),
		m_myHandle(this, IDC_MYHANDLE)
	{
		m_chkAuth.OnChange = Callback(this, &CAddContactDlg::OnAuthClicked);
		m_chkOpen.OnChange = Callback(this, &CAddContactDlg::OnOpenClicked);
	}

	bool OnInitDialog() override
	{
		Window_SetSkinIcon_IcoLib(m_hwnd, SKINICON_OTHER_ADDCONTACT);

		if (!m_szName.IsEmpty())
			SetCaption(CMStringW(FORMAT, TranslateT("Add %s"), m_szName.c_str()));
		else
			SetCaption(TranslateT("Add contact"));

		int groupSel = 0;
		ptrW tszGroup(Clist_GetGroup(m_hContact));
		wchar_t *grpName;
		for (int groupId = 1; (grpName = Clist_GroupGetName(groupId, nullptr)) != nullptr; groupId++) {
			int id = m_group.AddString(grpName, groupId);
			if (!mir_wstrcmpi(tszGroup, grpName))
				groupSel = id;
		}

		m_group.InsertString(TranslateT("None"), 0);
		m_group.SetCurSel(groupSel);

		// By default check both checkboxes
		m_chkAuth.SetState(true);

		// Set last choice
		if (db_get_b(0, "Miranda", "AuthOpenWindow", 1))
			m_chkOpen.SetState(true);

		uint32_t flags = (m_szProto) ? CallProtoServiceInt(0, m_szProto, PS_GETCAPS, PFLAGNUM_4, 0) : 0;
		if (flags & PF4_FORCEAUTH)  // force auth requests for this protocol
			m_chkAuth.Enable(false);

		if (flags & PF4_NOCUSTOMAUTH)
			m_authReq.Enable(false);
		else {
			m_authReq.Enable(m_chkAuth.Enabled());
			m_authReq.SetText(TranslateT("Please authorize my request and add me to your contact list."));
		}
		return true;
	}

	bool OnApply() override
	{
		MCONTACT hContact = 0;
		if (m_hDbEvent)
			hContact = (MCONTACT)CallProtoServiceInt(0, m_szProto, PS_ADDTOLISTBYEVENT, 0, m_hDbEvent);
		else if (m_psr)
			hContact = (MCONTACT)CallProtoServiceInt(0, m_szProto, PS_ADDTOLIST, 0, (LPARAM)m_psr);
		else
			hContact = m_hContact;

		if (hContact == 0) // something went wrong
			return false;

		ptrW szHandle(m_myHandle.GetText());
		if (mir_wstrlen(szHandle))
			db_set_ws(hContact, "CList", "MyHandle", szHandle);

		MGROUP iGroup = m_group.GetCurData();
		if (iGroup >= 0)
			Clist_ContactChangeGroup(hContact, iGroup);

		Contact::PutOnList(hContact);

		if (m_chkAuth.GetState()) {
			uint32_t flags = CallProtoServiceInt(0, m_szProto, PS_GETCAPS, PFLAGNUM_4, 0);
			if (flags & PF4_NOCUSTOMAUTH)
				ProtoChainSend(hContact, PSS_AUTHREQUEST, 0, 0);
			else
				ProtoChainSend(hContact, PSS_AUTHREQUEST, 0, ptrW(m_authReq.GetText()));
		}

		if (m_chkOpen.GetState())
			Clist_ContactDoubleClicked(hContact);
		return true;
	}

	void OnDestroy()
	{
		Window_FreeIcon_IcoLib(m_hwnd);
	}

	void OnAuthClicked(CCtrlButton*)
	{
		uint32_t flags = CallProtoServiceInt(0, m_szProto, PS_GETCAPS, PFLAGNUM_4, 0);
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
};

MIR_APP_DLL(void) Contact::Add(MCONTACT hContact, HWND hwndParent)
{
	if (hContact == 0)
		return;

	struct CAddByContact : public CAddContactDlg
	{
		CAddByContact(MCONTACT hContact)			
		{	
			m_hContact = hContact;
			m_szName = Clist_GetContactDisplayName(hContact);
			m_szProto = Proto_GetBaseAccountName(hContact);
		}
	};

	if (hwndParent != nullptr) {
		CAddByContact dlg(hContact);
		dlg.SetParent(hwndParent);
		dlg.DoModal();
	}
	else (new CAddByContact(hContact))->Show();
}

MIR_APP_DLL(void) Contact::AddByEvent(MEVENT hEvent, MWindow hwndParent)
{
	struct CAddByEvent : public CAddContactDlg
	{
		CAddByEvent(MEVENT hEvent)
		{
			m_hDbEvent = hEvent;

			uint32_t dwData[2];
			DBEVENTINFO dbei = {};
			dbei.cbBlob = sizeof(dwData);
			dbei.pBlob = (uint8_t*)&dwData;
			db_event_get(hEvent, &dbei);
			if (dwData[0] != 0)
				m_szName.Format(L"%d", dwData[0]);

			m_hContact = dwData[1];
			if (m_hContact != INVALID_CONTACT_ID)
				m_szName = Clist_GetContactDisplayName(m_hContact);

			m_szProto = dbei.szModule;
		}
	};

	if (hwndParent != nullptr) {
		CAddByEvent dlg(hEvent);
		dlg.SetParent(hwndParent);
		dlg.DoModal();
	}
	else (new CAddByEvent(hEvent))->Show();
}

MIR_APP_DLL(void) Contact::AddBySearch(const char *szProto, struct PROTOSEARCHRESULT *psr, HWND hwndParent)
{
	struct CAddBySearch : public CAddContactDlg
	{
		CAddBySearch(const char *szProto, struct PROTOSEARCHRESULT *psr)
		{
			m_szProto = szProto;
			m_psr = psr;

			wchar_t *p = sttDecodeString(psr->flags, psr->id);
			if (p == nullptr)
				p = sttDecodeString(psr->flags, psr->nick);
			
			if (p) {
				m_szName = p;
				mir_free(p);
			}
		}
	};

	if (hwndParent != nullptr) {
		CAddBySearch dlg(szProto, psr);
		dlg.SetParent(hwndParent);
		dlg.DoModal();
	}
	else (new CAddBySearch(szProto, psr))->Show();
}
