/*
former MetaContacts Plugin for Miranda IM.

Copyright © 2014 Miranda NG Team
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

#include "..\..\core\commonheaders.h"

#include "metacontacts.h"

MetaOptions options;

int Meta_WriteOptions()
{
	db_set_b(NULL, META_PROTO, "LockHandle", options.bLockHandle);
	db_set_b(NULL, META_PROTO, "SuppressStatus", options.bSuppressStatus);
	db_set_w(NULL, META_PROTO, "MenuContactLabel", (WORD)options.menu_contact_label);
	db_set_w(NULL, META_PROTO, "MenuContactFunction", (WORD)options.menu_function);
	db_set_w(NULL, META_PROTO, "CListContactName", (WORD)options.clist_contact_name);
	db_set_dw(NULL, META_PROTO, "SetStatusFromOfflineDelay", (DWORD)(options.set_status_from_offline_delay));
	return 0;
}

int Meta_ReadOptions()
{
	db_mc_enable(db_get_b(NULL, META_PROTO, "Enabled", true) != 0);
	options.bSuppressStatus = db_get_b(NULL, META_PROTO, "SuppressStatus", true) != 0;
	options.menu_contact_label = (int)db_get_w(NULL, META_PROTO, "MenuContactLabel", DNT_UID);
	options.menu_function = (int)db_get_w(NULL, META_PROTO, "MenuContactFunction", FT_MENU);
	options.clist_contact_name = (int)db_get_w(NULL, META_PROTO, "CListContactName", CNNT_DISPLAYNAME);
	options.set_status_from_offline_delay = (int)db_get_dw(NULL, META_PROTO, "SetStatusFromOfflineDelay", DEFAULT_SET_STATUS_SLEEP_TIME);
	options.bLockHandle = db_get_b(NULL, META_PROTO, "LockHandle", false) != 0;
	return 0;
}

class CMetaOptionsDlg : public CDlgBase
{
	CCtrlCheck m_btnUid, m_btnDid, m_btnCheck;
	CCtrlCheck m_btnMsg, m_btnMenu, m_btnInfo;
	CCtrlCheck m_btnNick, m_btnName, m_btnLock;

public:
	CMetaOptionsDlg() :
		CDlgBase(hInst, IDD_METAOPTIONS),
		m_btnUid(this, IDC_RAD_UID),
		m_btnDid(this, IDC_RAD_DID),
		m_btnMsg(this, IDC_RAD_MSG),
		m_btnMenu(this, IDC_RAD_MENU),
		m_btnInfo(this, IDC_RAD_INFO),
		m_btnNick(this, IDC_RAD_NICK),
		m_btnName(this, IDC_RAD_NAME),
		m_btnLock(this, IDC_CHK_LOCKHANDLE),
		m_btnCheck(this, IDC_CHK_SUPPRESSSTATUS)
	{
	}

	virtual void OnInitDialog()
	{
		m_btnLock.SetState(options.bLockHandle);
		m_btnCheck.SetState(options.bSuppressStatus);

		if (options.menu_contact_label == DNT_UID)
			m_btnUid.SetState(true);
		else
			m_btnDid.SetState(true);

		switch (options.menu_function) {
		case FT_MSG: m_btnMsg.SetState(true); break;
		case FT_MENU: m_btnMenu.SetState(true); break;
		case FT_INFO: m_btnInfo.SetState(true); break;
		}

		if (options.clist_contact_name == CNNT_NICK)
			m_btnNick.SetState(true);
		else
			m_btnName.SetState(true);
	}

	virtual void OnApply()
	{
		options.bLockHandle = m_btnLock.GetState() != 0;
		options.bSuppressStatus = m_btnCheck.GetState() != 0;

		     if (m_btnUid.GetState()) options.menu_contact_label = DNT_UID;
		else if (m_btnDid.GetState()) options.menu_contact_label = DNT_DID;

		     if (m_btnMsg.GetState()) options.menu_function = FT_MSG;
		else if (m_btnMenu.GetState()) options.menu_function = FT_MENU;
		else if (m_btnInfo.GetState()) options.menu_function = FT_INFO;

		     if (m_btnNick.GetState()) options.clist_contact_name = CNNT_NICK;
		else if (m_btnName.GetState()) options.clist_contact_name = CNNT_DISPLAYNAME;

		Meta_WriteOptions();

		Meta_SuppressStatus(options.bSuppressStatus);
		Meta_SetAllNicks();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

int Meta_OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = -790000000;
	odp.flags = ODPF_BOLDGROUPS;
	odp.pszTitle = LPGEN("Metacontacts");
	odp.pszGroup = LPGEN("Contacts");
	odp.pDialog = new CMetaOptionsDlg();
	Options_AddPage(wParam, &odp);
	return 0;
}
