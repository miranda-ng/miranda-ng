#include "stdafx.h"

CToxOptionsMain::CToxOptionsMain(CToxProto *proto, int idDialog)
	: CToxDlgBase(proto, idDialog),
	m_toxAddress(this, IDC_TOXID),
	m_toxAddressCopy(this, IDC_CLIPBOARD),
	m_profileCreate(this, IDC_PROFILE_NEW),
	m_profileImport(this, IDC_PROFILE_IMPORT),
	m_profileExport(this, IDC_PROFILE_EXPORT),
	m_nickname(this, IDC_NAME),
	m_passwordCreate(this, IDC_PASSWORD_CREATE),
	m_passwordChange(this, IDC_PASSWORD_CHANGE),
	m_passwordRemove(this, IDC_PASSWORD_REMOVE),
	m_group(this, IDC_GROUP),
	m_enableUdp(this, IDC_ENABLE_UDP), m_enableUdpHolePunching(this, IDC_ENABLE_HOLEPUNCHING),
	m_enableIPv6(this, IDC_ENABLE_IPV6),
	m_enableLocalDiscovery(this, IDC_ENABLE_LOCALDISCOVERY),
	m_maxConnectRetries(this, IDC_MAXCONNECTRETRIESSPIN, 255, 1),
	m_maxReconnectRetries(this, IDC_MAXRECONNECTRETRIESSPIN, 255, 1)
{
	CreateLink(m_toxAddress, TOX_SETTINGS_ID, L"");
	CreateLink(m_nickname, "Nick", L"");
	CreateLink(m_group, TOX_SETTINGS_GROUP, L"Tox");
	CreateLink(m_enableUdp, "EnableUDP", DBVT_BYTE, TRUE);
	CreateLink(m_enableUdpHolePunching, "EnableUDPHolePunching", DBVT_BYTE, TRUE);
	CreateLink(m_enableIPv6, "EnableIPv6", DBVT_BYTE, FALSE);
	CreateLink(m_enableLocalDiscovery, "EnableLocalDiscovery", DBVT_BYTE, FALSE);

	if (idDialog == IDD_OPTIONS_MAIN) {
		CreateLink(m_maxConnectRetries, "MaxConnectRetries", DBVT_BYTE, TOX_MAX_CONNECT_RETRIES);
		CreateLink(m_maxReconnectRetries, "MaxReconnectRetries", DBVT_BYTE, TOX_MAX_RECONNECT_RETRIES);
	}

	m_passwordCreate.OnClick = Callback(this, &CToxOptionsMain::PasswordCreate_OnClick);
	m_passwordChange.OnClick = Callback(this, &CToxOptionsMain::PasswordChange_OnClick);
	m_passwordRemove.OnClick = Callback(this, &CToxOptionsMain::PasswordRemove_OnClick);

	m_enableUdp.OnChange = Callback(this, &CToxOptionsMain::EnableUdp_OnClick);
	m_enableUdpHolePunching.Enable(m_enableUdp.GetState());

	m_toxAddressCopy.OnClick = Callback(this, &CToxOptionsMain::ToxAddressCopy_OnClick);
	m_profileCreate.OnClick = Callback(this, &CToxOptionsMain::ProfileCreate_OnClick);
	m_profileImport.OnClick = Callback(this, &CToxOptionsMain::ProfileImport_OnClick);
	m_profileExport.OnClick = Callback(this, &CToxOptionsMain::ProfileExport_OnClick);
}

bool CToxOptionsMain::OnInitDialog()
{
	CToxDlgBase::OnInitDialog();

	ptrW profilePath(m_proto->GetToxProfilePath());
	if (CToxProto::IsFileExists(profilePath)) {
		m_toxAddress.Enable();

		m_profileCreate.Hide();
		m_profileImport.Hide();

		m_toxAddressCopy.Show();
		m_profileExport.Show();
	}

	m_passwordCreate.Enable(m_proto->IsOnline());
	m_passwordChange.Enable(m_proto->IsOnline());
	m_passwordRemove.Enable(m_proto->IsOnline());

	pass_ptrW password(m_proto->getWStringA(TOX_SETTINGS_PASSWORD));
	bool passwordExists = mir_wstrlen(password) > 0;
	m_passwordCreate.Show(!passwordExists);
	m_passwordChange.Show(passwordExists);
	m_passwordRemove.Show(passwordExists);

	m_toxAddress.SetMaxLength(TOX_ADDRESS_SIZE * 2);
	m_nickname.SetMaxLength(TOX_MAX_NAME_LENGTH);
	m_group.SetMaxLength(64);
	return true;
}

void CToxOptionsMain::PasswordCreate_OnClick(CCtrlButton*)
{
	m_proto->OnCreatePassword(0, 0);
}

void CToxOptionsMain::PasswordChange_OnClick(CCtrlButton*)
{
	m_proto->OnChangePassword(0, 0);
}

void CToxOptionsMain::PasswordRemove_OnClick(CCtrlButton*)
{
	m_proto->OnRemovePassword(0, 0);
	pass_ptrW password(m_proto->getWStringA(TOX_SETTINGS_PASSWORD));
	bool passwordExists = mir_wstrlen(password) > 0;
	m_passwordCreate.Show(!passwordExists);
	m_passwordChange.Show(passwordExists);
	m_passwordRemove.Show(passwordExists);
}

void CToxOptionsMain::EnableUdp_OnClick(CCtrlBase*)
{
	m_enableUdpHolePunching.Enable(m_enableUdp.GetState());
}

void CToxOptionsMain::ToxAddressCopy_OnClick(CCtrlButton*)
{
	Utils_ClipboardCopy(ptrW(m_toxAddress.GetText()));
}

void CToxOptionsMain::ProfileCreate_OnClick(CCtrlButton*)
{
	ptrW profilePath(m_proto->GetToxProfilePath());
	if (!m_proto->IsFileExists(profilePath)) {
		HANDLE hProfile = CreateFile(profilePath, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hProfile == nullptr) {
			m_proto->debugLogA(__FUNCTION__": failed to create tox profile");
			return;
		}
		CloseHandle(hProfile);
	}

	Tox_Options *options = nullptr;
	tox_options_default(options);
	Tox *tox = tox_new(options, nullptr);
	m_proto->InitToxCore(tox);
	m_proto->UninitToxCore(tox);
	tox_kill(tox);

	m_toxAddress.Enable();
	m_toxAddress.SetTextA(ptrA(m_proto->getStringA(TOX_SETTINGS_ID)));

	m_nickname.SetText(ptrW(m_proto->getWStringA("Nick")));

	ShowWindow(m_profileCreate.GetHwnd(), FALSE);
	ShowWindow(m_profileImport.GetHwnd(), FALSE);

	ShowWindow(m_toxAddressCopy.GetHwnd(), TRUE);
	ShowWindow(m_profileExport.GetHwnd(), TRUE);
}

void CToxOptionsMain::ProfileImport_OnClick(CCtrlButton*)
{
	wchar_t filter[MAX_PATH];
	mir_snwprintf(filter, L"%s(*.tox)%c*.tox%c%s(*.*)%c*.*%c%c",
		TranslateT("Tox profile"), 0, 0, TranslateT("All files"), 0, 0, 0);

	wchar_t profilePath[MAX_PATH] = { 0 };

	OPENFILENAME ofn = { sizeof(ofn) };
	ofn.hwndOwner = m_hwnd;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = profilePath;
	ofn.lpstrTitle = TranslateT("Select Tox profile");
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;
	ofn.lpstrInitialDir = L"%APPDATA%\\Tox";

	if (!GetOpenFileName(&ofn))
		return;

	ptrW defaultProfilePath(m_proto->GetToxProfilePath());
	if (mir_wstrcmpi(profilePath, defaultProfilePath) != 0)
		CopyFile(profilePath, defaultProfilePath, FALSE);

	Tox_Options *options = tox_options_new(nullptr);
	if (m_proto->LoadToxProfile(options)) {
		Tox *tox = tox_new(options, nullptr);
		tox_options_free(options);

		uint8_t data[TOX_ADDRESS_SIZE];
		tox_self_get_address(tox, data);
		ToxHexAddress address(data);
		m_proto->setString(TOX_SETTINGS_ID, address);

		m_toxAddress.Enable();
		m_toxAddress.SetTextA(address);

		uint8_t nick[TOX_MAX_NAME_LENGTH] = { 0 };
		tox_self_get_name(tox, nick);
		ptrW nickname(mir_utf8decodeW((char*)nick));
		m_proto->setWString("Nick", nickname);
		m_nickname.SetText(nickname);

		uint8_t statusMessage[TOX_MAX_STATUS_MESSAGE_LENGTH] = { 0 };
		tox_self_get_status_message(tox, statusMessage);
		m_proto->setWString("StatusMsg", ptrW(mir_utf8decodeW((char*)statusMessage)));

		ShowWindow(m_profileCreate.GetHwnd(), FALSE);
		ShowWindow(m_profileImport.GetHwnd(), FALSE);

		ShowWindow(m_toxAddressCopy.GetHwnd(), TRUE);
		ShowWindow(m_profileExport.GetHwnd(), TRUE);

		tox_kill(tox);
	}
	tox_options_free(options);
}

void CToxOptionsMain::ProfileExport_OnClick(CCtrlButton*)
{
	wchar_t filter[MAX_PATH];
	mir_snwprintf(filter, L"%s(*.tox)%c*.tox%c%c",
		TranslateT("Tox profile"), 0, 0, 0);

	wchar_t profilePath[MAX_PATH];
	mir_wstrncpy(profilePath, L"tox_save.tox", _countof(profilePath));

	OPENFILENAME ofn = { sizeof(ofn) };
	ofn.hwndOwner = m_hwnd;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = profilePath;
	ofn.lpstrTitle = TranslateT("Save Tox profile");
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_EXPLORER;
	ofn.lpstrInitialDir = L"%HOMEPATH%";

	if (!GetSaveFileName(&ofn))
		return;

	ptrW defaultProfilePath(m_proto->GetToxProfilePath());
	if (mir_wstrcmpi(profilePath, defaultProfilePath) != 0)
		CopyFile(defaultProfilePath, profilePath, FALSE);
}

bool CToxOptionsMain::OnApply()
{
	ptrW group(m_group.GetText());
	if (mir_wstrcmp(group, m_proto->m_defaultGroup)) {
		m_proto->m_defaultGroup = mir_wstrdup(group);
		Clist_GroupCreate(0, group);
	}

	if (m_proto->IsOnline()) {
		CallProtoService(m_proto->m_szModuleName, PS_SETMYNICKNAME, SMNN_UNICODE, (LPARAM)ptrW(m_nickname.GetText()));

		// todo: add checkbox
		//m_proto->setWString("Password", pass_ptrW(m_password.GetText()));

		m_proto->SaveToxProfile(m_proto->m_tox);
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////

class CToxNodeEditor : public CDlgBase
{
	typedef CDlgBase CSuper;

	int m_iItem;
	CCtrlListView *m_list;

	CCtrlEdit m_ipv4;
	CCtrlEdit m_ipv6;
	CCtrlEdit m_port;
	CCtrlEdit m_pkey;

public:
	CToxNodeEditor(int iItem, CCtrlListView *m_nodes) :
		CSuper(g_plugin, IDD_NODE_EDITOR),
		m_ipv4(this, IDC_IPV4), m_ipv6(this, IDC_IPV6),
		m_port(this, IDC_PORT), m_pkey(this, IDC_PKEY),
		m_iItem(iItem)
	{
		m_autoClose = CLOSE_ON_CANCEL;
		m_list = m_nodes;
	}

	bool OnInitDialog() override
	{
		SetWindowText(m_hwnd, m_iItem == -1 ? TranslateT("Add node") : TranslateT("Change node"));

		if (m_iItem > -1) {
			LVITEM lvi = { 0 };
			lvi.mask = LVIF_TEXT;
			lvi.iItem = m_iItem;
			lvi.cchTextMax = MAX_PATH;
			lvi.pszText = (wchar_t *)alloca(MAX_PATH * sizeof(wchar_t));

			lvi.iSubItem = 0;
			m_list->GetItem(&lvi);
			m_ipv4.SetText(lvi.pszText);

			lvi.iSubItem = 1;
			m_list->GetItem(&lvi);
			m_ipv6.SetText(lvi.pszText);

			lvi.iSubItem = 2;
			m_list->GetItem(&lvi);
			m_port.SetText(lvi.pszText);

			lvi.iSubItem = 3;
			m_list->GetItem(&lvi);
			m_pkey.SetText(lvi.pszText);
		}

		Utils_RestoreWindowPositionNoSize(m_hwnd, NULL, MODULE, "EditNodeDlg");
		return true;
	}

	bool OnApply() override
	{
		ptrW ipv4(m_ipv4.GetText());
		if (!ipv4) {
			MessageBox(m_hwnd, TranslateT("Enter IPv4"), TranslateT("Error"), MB_OK);
			return false;
		}

		ptrW port(m_port.GetText());
		if (!port) {
			MessageBox(m_hwnd, TranslateT("Enter port"), TranslateT("Error"), MB_OK);
			return false;
		}

		ptrW pubKey(m_pkey.GetText());
		if (!pubKey) {
			MessageBox(m_hwnd, TranslateT("Enter public key"), TranslateT("Error"), MB_OK);
			return false;
		}

		if (m_iItem == -1) {
			m_iItem = m_list->AddItem(ipv4, -1, NULL, 1);
			m_list->SetCurSel(m_iItem);
			m_list->EnsureVisible(m_iItem, TRUE);
		}
		else
			m_list->SetItem(m_iItem, 0, ipv4);
		m_list->SetItem(m_iItem, 2, port);
		m_list->SetItem(m_iItem, 3, pubKey);
		m_list->SetItem(m_iItem, 4, L"", 0);
		m_list->SetItem(m_iItem, 5, L"", 1);

		SendMessage(GetParent(GetParent(m_list->GetHwnd())), PSM_CHANGED, 0, 0);
		return true;
	}

	void OnDestroy() override
	{
		Utils_SaveWindowPosition(m_hwnd, NULL, MODULE, "EditNodeDlg");
	}
};

/****************************************/

CToxOptionsNodeList::CToxOptionsNodeList(CToxProto *proto)
	: CSuper(proto, IDD_OPTIONS_NODES),
	m_nodes(this, IDC_NODESLIST), m_addNode(this, IDC_ADDNODE),
	m_updateNodes(this, IDC_UPDATENODES)
{
	m_addNode.OnClick = Callback(this, &CToxOptionsNodeList::OnAddNode);
	m_updateNodes.OnClick = Callback(this, &CToxOptionsNodeList::OnUpdateNodes);
	m_nodes.OnClick = Callback(this, &CToxOptionsNodeList::OnNodeListClick);
	m_nodes.OnDoubleClick = Callback(this, &CToxOptionsNodeList::OnNodeListDoubleClick);
	m_nodes.OnKeyDown = Callback(this, &CToxOptionsNodeList::OnNodeListKeyDown);
}

bool CToxOptionsNodeList::OnInitDialog()
{
	m_nodes.SetExtendedListViewStyle(LVS_EX_SUBITEMIMAGES | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

	HIMAGELIST hImageList = m_nodes.CreateImageList(LVSIL_SMALL);
	HICON icon = Skin_LoadIcon(SKINICON_OTHER_TYPING);
	ImageList_AddIcon(hImageList, icon); IcoLib_ReleaseIcon(icon);
	icon = Skin_LoadIcon(SKINICON_OTHER_DELETE);
	ImageList_AddIcon(hImageList, icon); IcoLib_ReleaseIcon(icon);

	m_nodes.AddColumn(0, L"IPv4", 100);
	m_nodes.AddColumn(1, L"IPv6", 100);
	m_nodes.AddColumn(2, TranslateT("Port"), 50);
	m_nodes.AddColumn(3, TranslateT("Public key"), 130);
	m_nodes.AddColumn(4, L"", 32 - GetSystemMetrics(SM_CXVSCROLL));
	m_nodes.AddColumn(5, L"", 32 - GetSystemMetrics(SM_CXVSCROLL));

	m_nodes.EnableGroupView(TRUE);
	m_nodes.AddGroup(0, TranslateT("Common nodes"));
	m_nodes.AddGroup(1, TranslateT("User nodes"));

	ReloadNodeList();
	return true;
}

void CToxOptionsNodeList::OnAddNode(CCtrlBase*)
{
	CToxNodeEditor nodeEditor(-1, &m_nodes);
	if (nodeEditor.DoModal())
		NotifyChange();
}

void CToxOptionsNodeList::OnUpdateNodes(CCtrlBase*)
{
	m_proto->UpdateNodes();

	ReloadNodeList();
}

void CToxOptionsNodeList::OnNodeListDoubleClick(CCtrlBase*)
{
	int iItem = m_nodes.GetNextItem(-1, LVNI_SELECTED);

	LVITEM lvi = { 0 };
	lvi.iItem = iItem;
	lvi.mask = LVIF_GROUPID;
	m_nodes.GetItem(&lvi);
	if (lvi.iGroupId || (lvi.iGroupId == 0 && lvi.iItem == -1)) {
		CToxNodeEditor nodeEditor(lvi.iItem, &m_nodes);
		if (nodeEditor.DoModal())
			NotifyChange();
	}
}

void CToxOptionsNodeList::OnNodeListClick(CCtrlListView::TEventInfo *evt)
{
	LVITEM lvi = { 0 };
	lvi.iItem = evt->nmlvia->iItem;
	lvi.mask = LVIF_GROUPID;
	m_nodes.GetItem(&lvi);
	lvi.iSubItem = evt->nmlvia->iSubItem;
	if (lvi.iGroupId && lvi.iSubItem == 4) {
		CToxNodeEditor nodeEditor(lvi.iItem, &m_nodes);
		if (nodeEditor.DoModal())
			NotifyChange();
	}
	else if (lvi.iGroupId && lvi.iSubItem == 5) {
		if (MessageBox(m_hwnd, TranslateT("Are you sure?"), TranslateT("Node deleting"), MB_YESNO | MB_ICONWARNING) == IDYES) {
			m_nodes.DeleteItem(lvi.iItem);
			NotifyChange();
		}
	}
}

void CToxOptionsNodeList::OnNodeListKeyDown(CCtrlListView::TEventInfo *evt)
{
	LVITEM lvi = { 0 };
	lvi.iItem = m_nodes.GetSelectionMark();
	lvi.mask = LVIF_GROUPID;
	m_nodes.GetItem(&lvi);

	if (lvi.iGroupId && lvi.iItem != -1 && (evt->nmlvkey)->wVKey == VK_DELETE) {
		if (MessageBox(GetParent(m_hwnd), TranslateT("Are you sure?"), TranslateT("Node deleting"), MB_YESNO | MB_ICONWARNING) == IDYES) {
			m_nodes.DeleteItem(lvi.iItem);
			NotifyChange();
		}
	}
}

void CToxOptionsNodeList::ReloadNodeList()
{
	m_nodes.DeleteAllItems();

	int iItem = -1;

	JSONNode nodes = m_proto->ParseNodes();
	for (const auto &node : nodes) {
		ptrW ipv4(mir_utf8decodeW(node.at("ipv4").as_string().c_str()));
		iItem = m_nodes.AddItem(ipv4, -1, NULL, 0);

		ptrW ipv6(mir_utf8decodeW(node.at("ipv6").as_string().c_str()));
		if (mir_wstrcmp(ipv6, L"-"))
			m_nodes.SetItem(iItem, 1, ipv6);

		ptrW port(mir_utf8decodeW(node.at("port").as_string().c_str()));
		m_nodes.SetItem(iItem, 2, port);

		ptrW pubKey(mir_utf8decodeW(node.at("public_key").as_string().c_str()));
		m_nodes.SetItem(iItem, 3, pubKey);
	}

	char module[MAX_PATH], setting[MAX_PATH];
	mir_snprintf(module, "%s_Nodes", m_proto->m_szModuleName);
	int nodeCount = db_get_w(0, module, TOX_SETTINGS_NODE_COUNT, 0);
	for (int i = 0; i < nodeCount; i++) {
		mir_snprintf(setting, TOX_SETTINGS_NODE_IPV4, i);
		ptrW value(db_get_wsa(0, module, setting));
		iItem = m_nodes.AddItem(value, -1, NULL, 1);

		mir_snprintf(setting, TOX_SETTINGS_NODE_IPV6, i);
		value = db_get_wsa(0, module, setting);
		m_nodes.SetItem(iItem, 1, value);

		mir_snprintf(setting, TOX_SETTINGS_NODE_PORT, i);
		int port = db_get_w(0, module, setting, 0);
		if (port > 0) {
			char portNum[10];
			itoa(port, portNum, 10);
			m_nodes.SetItem(iItem, 2, mir_a2u(portNum));
		}

		mir_snprintf(setting, TOX_SETTINGS_NODE_PKEY, i);
		value = db_get_wsa(0, module, setting);
		m_nodes.SetItem(iItem, 3, value);

		m_nodes.SetItem(iItem, 4, L"", 0);
		m_nodes.SetItem(iItem, 5, L"", 1);
	}
}

bool CToxOptionsNodeList::OnApply()
{
	char setting[MAX_PATH];
	wchar_t tszText[MAX_PATH];

	LVITEM lvi = { 0 };
	lvi.cchTextMax = MAX_PATH;
	lvi.pszText = tszText;

	char module[MAX_PATH];
	mir_snprintf(module, "%s_Nodes", m_proto->m_szModuleName);

	int iItem = 0;
	int itemCount = m_nodes.GetItemCount();
	for (int i = 0; i < itemCount; i++) {
		lvi.iItem = i;
		lvi.iGroupId = 0;
		lvi.mask = LVIF_GROUPID;
		m_nodes.GetItem(&lvi);
		if (lvi.iGroupId == 0) {
			continue;
		}

		lvi.mask = LVIF_TEXT;
		lvi.iSubItem = 0;
		m_nodes.GetItem(&lvi);
		mir_snprintf(setting, TOX_SETTINGS_NODE_IPV4, iItem);
		db_set_s(0, module, setting, _T2A(lvi.pszText));

		lvi.iSubItem = 1;
		m_nodes.GetItem(&lvi);
		mir_snprintf(setting, TOX_SETTINGS_NODE_IPV6, iItem);
		db_set_s(0, module, setting, _T2A(lvi.pszText));

		lvi.iSubItem = 2;
		m_nodes.GetItem(&lvi);
		mir_snprintf(setting, TOX_SETTINGS_NODE_PORT, iItem);
		db_set_w(0, module, setting, _wtoi(lvi.pszText));

		lvi.iSubItem = 3;
		m_nodes.GetItem(&lvi);
		mir_snprintf(setting, TOX_SETTINGS_NODE_PKEY, iItem);
		db_set_s(0, module, setting, _T2A(lvi.pszText));

		iItem++;
	}
	itemCount = iItem;
	int nodeCount = db_get_b(0, module, TOX_SETTINGS_NODE_COUNT, 0);
	for (iItem = itemCount; iItem < nodeCount; iItem++) {
		mir_snprintf(setting, TOX_SETTINGS_NODE_IPV4, iItem);
		db_unset(0, module, setting);
		mir_snprintf(setting, TOX_SETTINGS_NODE_IPV6, iItem);
		db_unset(0, module, setting);
		mir_snprintf(setting, TOX_SETTINGS_NODE_PORT, iItem);
		db_unset(0, module, setting);
		mir_snprintf(setting, TOX_SETTINGS_NODE_PKEY, iItem);
		db_unset(0, module, setting);
	}
	db_set_b(0, module, TOX_SETTINGS_NODE_COUNT, itemCount);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////

int CToxProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szTitle.w = m_tszUserName;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE | ODPF_DONTTRANSLATE;
	odp.szGroup.w = LPGENW("Network");

	odp.szTab.w = LPGENW("Account");
	odp.pDialog = CToxOptionsMain::CreateOptionsPage(this);
	g_plugin.addOptions(wParam, &odp);

	/*odp.szTab.w = LPGENW("Multimedia");
	odp.pDialog = CToxOptionsMultimedia::CreateOptionsPage(this);
	g_plugin.addOptions(wParam, &odp);*/

	odp.szTab.w = LPGENW("Nodes");
	odp.pDialog = CToxOptionsNodeList::CreateOptionsPage(this);
	g_plugin.addOptions(wParam, &odp);

	return 0;
}
