#include "stdafx.h"

CSteamOptionsMain::CSteamOptionsMain(CSteamProto *proto, int idDialog, HWND hwndParent)
	: CSteamDlgBase(proto, idDialog, false),
	m_username(this, IDC_USERNAME), m_password(this, IDC_PASSWORD),
	m_group(this, IDC_GROUP), m_biggerAvatars(this, IDC_BIGGER_AVATARS), m_showChatEvents(this, IDC_SHOW_CHAT_EVENTS)
{
	SetParent(hwndParent);

	CreateLink(m_username, "Username", L"");
	CreateLink(m_password, "Password", L"");
	CreateLink(m_group, "DefaultGroup", L"Steam");
	CreateLink(m_biggerAvatars, "UseBigAvatars", DBVT_BYTE, FALSE);
	CreateLink(m_showChatEvents, "ShowChatEvents", DBVT_BYTE, TRUE);
}

void CSteamOptionsMain::OnInitDialog()
{
	CSteamDlgBase::OnInitDialog();

	SendMessage(m_username.GetHwnd(), EM_LIMITTEXT, 64, 0);
	SendMessage(m_password.GetHwnd(), EM_LIMITTEXT, 64, 0);
	SendMessage(m_group.GetHwnd(), EM_LIMITTEXT, 64, 0);
}

void CSteamOptionsMain::OnApply()
{
	wchar_t *group = m_group.GetText();
	if (mir_wstrlen(group) > 0 && !Clist_GroupExists(group))
		Clist_GroupCreate(0, group);

	if (m_proto->IsOnline())
	{
		// may be we should show message box with warning?
		m_proto->SetStatus(ID_STATUS_OFFLINE);
	}
	if (m_username.IsChanged())
	{
		m_proto->delSetting("SteamID");
		m_proto->delSetting("TokenSecret");
	}
	if (m_password.IsChanged())
	{
		m_proto->delSetting("TokenSecret");
	}
	mir_free(group);
}

/////////////////////////////////////////////////////////////////////////////////

CSteamOptionsBlockList::CSteamOptionsBlockList(CSteamProto *proto)
	: CSuper(proto, IDD_OPT_BLOCK_LIST, false),
	m_list(this, IDC_LIST),
	m_contacts(this, IDC_CONTACTS),
	m_add(this, IDC_BLOCK)
{
	m_add.OnClick = Callback(this, &CSteamOptionsBlockList::OnBlock);
}

void CSteamOptionsBlockList::OnInitDialog()
{
	m_list.SetExtendedListViewStyle(LVS_EX_SUBITEMIMAGES | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

	m_list.AddColumn(0, TranslateT("Name"), 220);
	m_list.AddColumn(1, L"", 32 - GetSystemMetrics(SM_CXVSCROLL));
}

void CSteamOptionsBlockList::OnBlock(CCtrlButton*)
{
}