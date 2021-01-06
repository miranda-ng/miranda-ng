#include "stdafx.h"

class CSteamOptionsMain : public CSteamDlgBase
{
	CCtrlEdit m_username, m_password, m_group;
	CCtrlCheck m_biggerAvatars, m_showChatEvents;
	CCtrlSpin m_pollingErrorLimit;

public:
	CSteamOptionsMain(CSteamProto *proto, int idDialog, HWND hwndParent = NULL) :
		CSteamDlgBase(proto, idDialog),
		m_username(this, IDC_USERNAME),
		m_password(this, IDC_PASSWORD),
		m_group(this, IDC_GROUP),
		m_biggerAvatars(this, IDC_BIGGER_AVATARS),
		m_showChatEvents(this, IDC_SHOW_CHAT_EVENTS),
		m_pollingErrorLimit(this, IDC_POLLINGERRORLIMITSPIN, 255)
	{
		SetParent(hwndParent);

		CreateLink(m_username, "Username", L"");
		CreateLink(m_password, "Password", L"");
		CreateLink(m_group, "DefaultGroup", L"Steam");

		if (idDialog == IDD_OPT_MAIN) {
			CreateLink(m_biggerAvatars, "UseBigAvatars", DBVT_BYTE, FALSE);
			CreateLink(m_showChatEvents, "ShowChatEvents", DBVT_BYTE, TRUE);
			CreateLink(m_pollingErrorLimit, "PollingErrorsLimit", DBVT_BYTE, STEAM_API_POLLING_ERRORS_LIMIT);
		}
	}

	bool OnInitDialog() override
	{
		CSteamDlgBase::OnInitDialog();

		SendMessage(m_username.GetHwnd(), EM_LIMITTEXT, 64, 0);
		SendMessage(m_password.GetHwnd(), EM_LIMITTEXT, 64, 0);
		SendMessage(m_group.GetHwnd(), EM_LIMITTEXT, 64, 0);
		return true;
	}

	bool OnApply() override
	{
		ptrW group(m_group.GetText());
		if (mir_wstrcmp(group, m_proto->m_defaultGroup)) {
			m_proto->m_defaultGroup = mir_wstrdup(group);
			Clist_GroupCreate(0, group);
		}

		if (m_proto->IsOnline()) // may be we should show message box with warning?
			m_proto->SetStatus(ID_STATUS_OFFLINE);

		if (m_username.IsChanged()) {
			m_proto->delSetting("SteamID");
			m_proto->delSetting("TokenSecret");
		}
		if (m_password.IsChanged())
			m_proto->delSetting("TokenSecret");
		return true;
	}
};

INT_PTR CSteamProto::OnAccountManagerInit(WPARAM, LPARAM lParam)
{
	auto *page = new CSteamOptionsMain(this, IDD_ACCMGR, (HWND)lParam);
	page->Show();
	return (INT_PTR)page->GetHwnd();
}

/////////////////////////////////////////////////////////////////////////////////

class CSteamOptionsBlockList : public CSteamDlgBase
{
	typedef CSteamDlgBase CSuper;

	CCtrlListView m_list;
	CCtrlCombo m_contacts;
	CCtrlButton m_add;

public:
	CSteamOptionsBlockList(CSteamProto *proto) :
		CSuper(proto, IDD_OPT_BLOCK_LIST),
		m_list(this, IDC_LIST),
		m_contacts(this, IDC_CONTACTS),
		m_add(this, IDC_BLOCK)
	{
		m_add.OnClick = Callback(this, &CSteamOptionsBlockList::OnBlock);
	}

	bool OnInitDialog() override
	{
		m_list.SetExtendedListViewStyle(LVS_EX_SUBITEMIMAGES | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

		m_list.AddColumn(0, TranslateT("Name"), 220);
		m_list.AddColumn(1, L"", 32 - GetSystemMetrics(SM_CXVSCROLL));
		return true;
	}

	void OnBlock(CCtrlButton *)
	{}
};

/////////////////////////////////////////////////////////////////////////////////

int CSteamProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szTitle.w = m_tszUserName;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.szGroup.w = LPGENW("Network");

	odp.szTab.w = LPGENW("Account");
	odp.pDialog = new CSteamOptionsMain(this, IDD_OPT_MAIN);
	g_plugin.addOptions(wParam, &odp);

	// odp.szTab.w = LPGENW("Blocked contacts");
	// odp.pDialog = new CSteamOptionsBlockList(this);
	// g_plugin.addOptions(wParam, &odp);
	return 0;
}
