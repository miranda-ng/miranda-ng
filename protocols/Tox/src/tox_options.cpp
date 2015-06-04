#include "stdafx.h"

CToxOptionsMain::CToxOptionsMain(CToxProto *proto, int idDialog)
	: CToxDlgBase(proto, idDialog, false),
	m_toxAddress(this, IDC_TOXID), m_toxAddressCopy(this, IDC_CLIPBOARD),
	m_profileCreate(this, IDC_PROFILE_NEW), m_profileImport(this, IDC_PROFILE_IMPORT),
	m_profileExport(this, IDC_PROFILE_EXPORT), m_nickname(this, IDC_NAME),
	m_password(this, IDC_PASSWORD), m_group(this, IDC_GROUP),
	m_enableUdp(this, IDC_ENABLE_UDP), m_enableIPv6(this, IDC_ENABLE_IPV6)
{

	CreateLink(m_toxAddress, TOX_SETTINGS_ID, _T(""));
	CreateLink(m_nickname, "Nick", _T(""));
	CreateLink(m_password, "Password", _T(""));
	CreateLink(m_group, TOX_SETTINGS_GROUP, _T("Tox"));
	CreateLink(m_enableUdp, "EnableUDP", DBVT_BYTE, TRUE);
	CreateLink(m_enableIPv6, "EnableIPv6", DBVT_BYTE, FALSE);

	m_toxAddressCopy.OnClick = Callback(this, &CToxOptionsMain::ToxAddressCopy_OnClick);
	m_profileCreate.OnClick = Callback(this, &CToxOptionsMain::ProfileCreate_OnClick);
	m_profileImport.OnClick = Callback(this, &CToxOptionsMain::ProfileImport_OnClick);
	m_profileExport.OnClick = Callback(this, &CToxOptionsMain::ProfileExport_OnClick);
}

void CToxOptionsMain::OnInitDialog()
{
	CToxDlgBase::OnInitDialog();

	std::tstring profilePath = m_proto->GetToxProfilePath();
	if (CToxProto::IsFileExists(profilePath))
	{
		m_toxAddress.Enable();

		ShowWindow(m_profileCreate.GetHwnd(), FALSE);
		ShowWindow(m_profileImport.GetHwnd(), FALSE);

		ShowWindow(m_toxAddressCopy.GetHwnd(), TRUE);
		ShowWindow(m_profileExport.GetHwnd(), TRUE);
	}

	SendMessage(m_toxAddress.GetHwnd(), EM_LIMITTEXT, TOX_ADDRESS_SIZE * 2, 0);
	SendMessage(m_nickname.GetHwnd(), EM_LIMITTEXT, TOX_MAX_NAME_LENGTH, 0);
	SendMessage(m_password.GetHwnd(), EM_LIMITTEXT, 32, 0);
	SendMessage(m_group.GetHwnd(), EM_LIMITTEXT, 64, 0);
}

void CToxOptionsMain::ToxAddressCopy_OnClick(CCtrlButton*)
{
	char *toxAddress = m_toxAddress.GetTextA();
	size_t toxAddressLength = mir_strlen(toxAddress) + 1;
	if (OpenClipboard(m_toxAddress.GetHwnd()))
	{
		EmptyClipboard();
		HGLOBAL hMemory = GlobalAlloc(GMEM_FIXED, toxAddressLength);
		memcpy(GlobalLock(hMemory), toxAddress, toxAddressLength);
		GlobalUnlock(hMemory);
		SetClipboardData(CF_TEXT, hMemory);
		CloseClipboard();
	}
	mir_free(toxAddress);
}

void CToxOptionsMain::ProfileCreate_OnClick(CCtrlButton*)
{
	std::tstring profilePath = m_proto->GetToxProfilePath();
	if (!m_proto->IsFileExists(profilePath))
	{
		HANDLE hProfile = CreateFile(profilePath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hProfile == NULL)
		{
			m_proto->debugLogA(__FUNCTION__": failed to create tox profile");
			return;
		}
		CloseHandle(hProfile);

		TOX_ERR_NEW initError;
		m_proto->tox = tox_new(NULL, &initError);
		if (initError != TOX_ERR_NEW_OK)
		{
			m_proto->debugLogA(__FUNCTION__": failed to load tox profile (%d)", initError);
			return;
		}
	}
	
	if (m_proto->InitToxCore())
	{
		TCHAR *group = m_group.GetText();
		if (mir_tstrlen(group) > 0 && Clist_GroupExists(group))
			Clist_CreateGroup(0, group);

		m_proto->LoadFriendList(NULL);
		m_proto->UninitToxCore();

		m_toxAddress.Enable();
		m_toxAddress.SetTextA(ptrA(m_proto->getStringA(TOX_SETTINGS_ID)));

		m_nickname.SetText(ptrT(m_proto->getTStringA("Nick")));
		m_password.SetText(ptrT(m_proto->getTStringA("Password")));
		m_group.SetText(ptrT(m_proto->getTStringA(TOX_SETTINGS_GROUP)));

		ShowWindow(m_profileCreate.GetHwnd(), FALSE);
		ShowWindow(m_profileImport.GetHwnd(), FALSE);

		ShowWindow(m_toxAddressCopy.GetHwnd(), TRUE);
		ShowWindow(m_profileExport.GetHwnd(), TRUE);
	}
}

void CToxOptionsMain::ProfileImport_OnClick(CCtrlButton*)
{
	TCHAR filter[MAX_PATH];
	mir_sntprintf(filter, SIZEOF(filter), _T("%s(*.tox)%c*.tox%c%s(*.*)%c*.*%c%c"),
		TranslateT("Tox profile"), 0, 0, TranslateT("All files"), 0, 0, 0);

	TCHAR profilePath[MAX_PATH] = { 0 };

	OPENFILENAME ofn = { sizeof(ofn) };
	ofn.hwndOwner = m_hwnd;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = profilePath;
	ofn.lpstrTitle = TranslateT("Select tox profile");
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;
	ofn.lpstrInitialDir = _T("%APPDATA%\\Tox");

	if (!GetOpenFileName(&ofn))
	{
		return;
	}

	std::tstring defaultProfilePath = m_proto->GetToxProfilePath();
	if (mir_tstrcmpi(profilePath, defaultProfilePath.c_str()) != 0)
	{
		CopyFile(profilePath, defaultProfilePath.c_str(), FALSE);
	}

	m_profileCreate.OnClick(&m_profileCreate);
}

void CToxOptionsMain::ProfileExport_OnClick(CCtrlButton*)
{
	TCHAR filter[MAX_PATH];
	mir_sntprintf(filter, SIZEOF(filter), _T("%s(*.tox)%c*.tox%c%c"),
		TranslateT("Tox profile"), 0, 0, 0);

	TCHAR profilePath[MAX_PATH];
	mir_tstrncpy(profilePath, _T("tox_save.tox"), SIZEOF(profilePath));

	OPENFILENAME ofn = { sizeof(ofn) };
	ofn.hwndOwner = m_hwnd;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = profilePath;
	ofn.lpstrTitle = TranslateT("Save tox profile");\
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_EXPLORER;
	ofn.lpstrInitialDir = _T("%HOMEPATH%");

	if (!GetSaveFileName(&ofn))
		return;

	std::tstring defaultProfilePath = m_proto->GetToxProfilePath();
	if (mir_tstrcmpi(profilePath, defaultProfilePath.c_str()) != 0)
		CopyFile(defaultProfilePath.c_str(), profilePath, FALSE);
}

void CToxOptionsMain::OnApply()
{
	ptrT group(m_group.GetText());
	if (mir_tstrlen(group) > 0 && Clist_GroupExists(group))
		Clist_CreateGroup(0, group);

	if (m_proto->IsOnline())
	{
		CallProtoService(m_proto->m_szModuleName, PS_SETMYNICKNAME, SMNN_TCHAR, (LPARAM)ptrT(m_nickname.GetText()));

		if (m_proto->password != NULL)
			mir_free(m_proto->password);
		m_proto->password = mir_utf8encodeW(ptrT(m_password.GetText()));

		m_proto->SaveToxProfile();
	}
}

/////////////////////////////////////////////////////////////////////////////////

CToxOptionsMultimedia::CToxOptionsMultimedia(CToxProto *proto)
	: CToxDlgBase(proto, IDD_OPTIONS_MULTIMEDIA, false),
	m_audioInput(this, IDC_AUDIOINPUT),
	m_audioOutput(this, IDC_AUDIOOUTPUT)
{
}

bool CToxOptionsMultimedia::GetDeviceFullName(GUID guid, TCHAR *deviceName, DWORD deviceNameLength)
{
	TCHAR registryKey[MAX_PATH];
	mir_sntprintf(registryKey, SIZEOF(registryKey), _T("System\\CurrentControlSet\\Control\\MediaCategories\\{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
		guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

	HKEY hKey;
	LONG error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, registryKey, 0, KEY_READ, &hKey);
	if (error != ERROR_SUCCESS)
		return false;

	error = RegQueryValueEx(hKey, _T("Name"), 0, NULL, (LPBYTE)deviceName, &deviceNameLength);
	if (error != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return false;
	}

	RegCloseKey(hKey);
	return true;
}

void CToxOptionsMultimedia::OnInitDialog()
{
	CToxDlgBase::OnInitDialog();
	
	DWORD count = 0;
	TCHAR deviceName[MAX_PATH];
	DWORD deviceNameLength = SIZEOF(deviceName);

	WAVEINCAPS2 wic2;
	count = waveInGetNumDevs();
	for (DWORD i = 0; i < count; i++)
	{
		if (!waveInGetDevCaps(i, (LPWAVEINCAPS)&wic2, sizeof(WAVEINCAPS2)))
		{
			if (!GetDeviceFullName(wic2.NameGuid, deviceName, deviceNameLength))
				mir_tstrncpy(deviceName, wic2.szPname, deviceNameLength);

			m_audioInput.InsertString(deviceName, i);
		}
	}
	m_audioInput.SetCurSel(m_proto->getDword("AudioInputDeviceID", 0));

	WAVEOUTCAPS2 woc2;
	count = waveOutGetNumDevs();
	for (DWORD i = 0; i < count; i++)
	{
		if (!waveOutGetDevCaps(i, (LPWAVEOUTCAPS)&woc2, sizeof(WAVEOUTCAPS2)))
		{
			if (!GetDeviceFullName(woc2.NameGuid, deviceName, deviceNameLength))
				mir_tstrncpy(deviceName, woc2.szPname, deviceNameLength);

			m_audioOutput.InsertString(deviceName, i);
		}
	}
	m_audioOutput.SetCurSel(m_proto->getDword("AudioOutputDeviceID", 0));
}

void CToxOptionsMultimedia::OnApply()
{
	m_proto->setDword("AudioInputDeviceID", m_audioInput.GetCurSel());
	m_proto->setDword("AudioOutputDeviceID", m_audioOutput.GetCurSel());
}

/////////////////////////////////////////////////////////////////////////////////

CToxNodeEditor::CToxNodeEditor(int iItem, CCtrlListView *m_nodes)
	: CSuper(g_hInstance, IDD_NODE_EDITOR),
	m_ipv4(this, IDC_IPV4), m_ipv6(this, IDC_IPV6),
	m_port(this, IDC_PORT), m_pkey(this, IDC_PKEY),
	m_ok(this, IDOK), m_iItem(iItem)
{
	m_autoClose = CLOSE_ON_CANCEL;
	m_list = m_nodes;
	m_ok.OnClick = Callback(this, &CToxNodeEditor::OnOk);
}

void CToxNodeEditor::OnInitDialog()
{
	SetWindowText(m_hwnd, m_iItem == -1 ? TranslateT("Add node") : TranslateT("Change node"));

	if (m_iItem > -1)
	{
		LVITEM lvi = { 0 };
		lvi.mask = LVIF_TEXT;
		lvi.iItem = m_iItem;
		lvi.cchTextMax = MAX_PATH;
		lvi.pszText = (TCHAR*)mir_alloc(MAX_PATH * sizeof(TCHAR));

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

		mir_free(lvi.pszText);
	}

	Utils_RestoreWindowPositionNoSize(m_hwnd, NULL, MODULE, "EditNodeDlg");
}

void CToxNodeEditor::OnOk(CCtrlBase*)
{
	if (!m_ipv4.GetInt())
	{
		MessageBox(m_hwnd, TranslateT("Enter IPv4"), TranslateT("Error"), MB_OK);
		return;
	}
	if (!m_pkey.GetInt())
	{
		MessageBox(m_hwnd, TranslateT("Enter public key"), TranslateT("Error"), MB_OK);
		return;
	}

	ptrT ipv4(m_ipv4.GetText());
	if (m_iItem == -1)
	{
		m_iItem = m_list->AddItem(ipv4, -1, NULL, 1);
		m_list->SetItemState(m_iItem, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
		m_list->EnsureVisible(m_iItem, TRUE);
	}
	else
		m_list->SetItem(m_iItem, 0, ipv4);
	m_list->SetItem(m_iItem, 1, ptrT(m_ipv6.GetText()));
	m_list->SetItem(m_iItem, 2, ptrT(m_port.GetText()));
	m_list->SetItem(m_iItem, 3, ptrT(m_pkey.GetText()));
	m_list->SetItem(m_iItem, 4, _T(""), 0);
	m_list->SetItem(m_iItem, 5, _T(""), 1);

	EndDialog(m_hwnd, 1);
}

void CToxNodeEditor::OnClose()
{
	Utils_SaveWindowPosition(m_hwnd, NULL, MODULE, "EditNodeDlg");
}

/****************************************/

CCtrlNodeList::CCtrlNodeList(CDlgBase* dlg, int ctrlId)
	: CCtrlListView(dlg, ctrlId)
{
}

BOOL CCtrlNodeList::OnNotify(int idCtrl, NMHDR *pnmh)
{
	if (pnmh->code == NM_CLICK)
	{
		TEventInfo evt = { this, pnmh };
		OnClick(&evt);
		return TRUE;
	}
	return CCtrlListView::OnNotify(idCtrl, pnmh);
}

/****************************************/

CToxOptionsNodeList::CToxOptionsNodeList(CToxProto *proto)
	: CSuper(proto, IDD_OPTIONS_NODES, false),
	m_nodes(this, IDC_NODESLIST), m_addNode(this, IDC_ADDNODE)
{
	m_addNode.OnClick = Callback(this, &CToxOptionsNodeList::OnAddNode);
	m_nodes.OnClick = Callback(this, &CToxOptionsNodeList::OnNodeListClick);
	m_nodes.OnDoubleClick = Callback(this, &CToxOptionsNodeList::OnNodeListDoubleClick);
	m_nodes.OnKeyDown = Callback(this, &CToxOptionsNodeList::OnNodeListKeyDown);
}

void CToxOptionsNodeList::OnInitDialog()
{
	m_nodes.SetExtendedListViewStyle(LVS_EX_SUBITEMIMAGES | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

	HIMAGELIST hImageList = m_nodes.CreateImageList(LVSIL_SMALL);
	HICON icon = LoadSkinnedIcon(SKINICON_OTHER_TYPING);
	ImageList_AddIcon(hImageList, icon); Skin_ReleaseIcon(icon);
	icon = LoadSkinnedIcon(SKINICON_OTHER_DELETE);
	ImageList_AddIcon(hImageList, icon); Skin_ReleaseIcon(icon);

	m_nodes.AddColumn(0, _T("IPv4"), 100);
	m_nodes.AddColumn(1, _T("IPv6"), 100);
	m_nodes.AddColumn(2, TranslateT("Port"), 50);
	m_nodes.AddColumn(3, TranslateT("Public key"), 130);
	m_nodes.AddColumn(4, _T(""), 32 - GetSystemMetrics(SM_CXVSCROLL));
	m_nodes.AddColumn(5, _T(""), 32 - GetSystemMetrics(SM_CXVSCROLL));

	m_nodes.EnableGroupView(TRUE);
	m_nodes.AddGroup(0, TranslateT("Common nodes"));
	m_nodes.AddGroup(1, TranslateT("User nodes"));

	////////////////////////////////////////

	int iItem = -1;

	if (CToxProto::IsFileExists((TCHAR*)VARST(_T(TOX_INI_PATH))))
	{
		char fileName[MAX_PATH];
		mir_strcpy(fileName, VARS(TOX_INI_PATH));

		char *section, sections[MAX_PATH], value[MAX_PATH];
		GetPrivateProfileSectionNamesA(sections, SIZEOF(sections), fileName);
		section = sections;
		while (*section != NULL)
		{
			if (strstr(section, TOX_SETTINGS_NODE_PREFIX) == section)
			{
				GetPrivateProfileStringA(section, "IPv4", NULL, value, SIZEOF(value), fileName);
				iItem = m_nodes.AddItem(mir_a2t(value), -1, NULL, 0);

				GetPrivateProfileStringA(section, "IPv6", NULL, value, SIZEOF(value), fileName);
				m_nodes.SetItem(iItem, 1, mir_a2t(value));

				GetPrivateProfileStringA(section, "Port", NULL, value, SIZEOF(value), fileName);
				m_nodes.SetItem(iItem, 2, mir_a2t(value));

				GetPrivateProfileStringA(section, "PubKey", NULL, value, SIZEOF(value), fileName);
				m_nodes.SetItem(iItem, 3, mir_a2t(value));
			}
			section += mir_strlen(section) + 1;
		}
	}

	char module[MAX_PATH], setting[MAX_PATH];
	mir_snprintf(module, SIZEOF(module), "%s_Nodes", m_proto->m_szModuleName);
	int nodeCount = db_get_w(NULL, module, TOX_SETTINGS_NODE_COUNT, 0);
	for (int i = 0; i < nodeCount; i++)
	{
		mir_snprintf(setting, TOX_SETTINGS_NODE_IPV4, i);
		ptrT value(db_get_tsa(NULL, module, setting));
		iItem = m_nodes.AddItem(value, -1, NULL, 1);

		mir_snprintf(setting, TOX_SETTINGS_NODE_IPV6, i);
		value = db_get_tsa(NULL, module, setting);
		m_nodes.SetItem(iItem, 1, value);

		mir_snprintf(setting, TOX_SETTINGS_NODE_PORT, i);
		int port = db_get_w(NULL, module, setting, 0);
		if (port > 0)
		{
			char portNum[10];
			itoa(port, portNum, 10);
			m_nodes.SetItem(iItem, 2, mir_a2t(portNum));
		}

		mir_snprintf(setting, TOX_SETTINGS_NODE_PKEY, i);
		value = db_get_tsa(NULL, module, setting);
		m_nodes.SetItem(iItem, 3, value);

		m_nodes.SetItem(iItem, 4, _T(""), 0);
		m_nodes.SetItem(iItem, 5, _T(""), 1);
	}
}

void CToxOptionsNodeList::OnAddNode(CCtrlBase*)
{
	CToxNodeEditor nodeEditor(-1, &m_nodes);
	if (nodeEditor.DoModal())
		SendMessage(GetParent(m_hwnd), PSM_CHANGED, 0, 0);
}

void CToxOptionsNodeList::OnNodeListDoubleClick(CCtrlBase*)
{
	int iItem = m_nodes.GetNextItem(-1, LVNI_SELECTED);

	LVITEM lvi = { 0 };
	lvi.iItem = iItem;
	lvi.mask = LVIF_GROUPID;
	m_nodes.GetItem(&lvi);
	if (lvi.iGroupId || (lvi.iGroupId == 0 && lvi.iItem == -1))
	{
		CToxNodeEditor nodeEditor(lvi.iItem, &m_nodes);
		if (nodeEditor.DoModal())
			SendMessage(GetParent(m_hwnd), PSM_CHANGED, 0, 0);
	}
}

void CToxOptionsNodeList::OnNodeListClick(CCtrlListView::TEventInfo *evt)
{
	LVITEM lvi = { 0 };
	lvi.iItem = evt->nmlvia->iItem;
	lvi.mask = LVIF_GROUPID;
	m_nodes.GetItem(&lvi);
	lvi.iSubItem = evt->nmlvia->iSubItem;
	if (lvi.iGroupId && lvi.iSubItem == 4)
	{
		CToxNodeEditor nodeEditor(lvi.iItem, &m_nodes);
		if (nodeEditor.DoModal())
			SendMessage(GetParent(GetParent(m_hwnd)), PSM_CHANGED, 0, 0);
	}
	else if (lvi.iGroupId && lvi.iSubItem == 5)
	{
		if (MessageBox(m_hwnd, TranslateT("Are you sure?"), TranslateT("Node deleting"), MB_YESNO | MB_ICONWARNING) == IDYES)
		{
			m_nodes.DeleteItem(lvi.iItem);
			SendMessage(GetParent(GetParent(m_hwnd)), PSM_CHANGED, 0, 0);
		}
	}
}

void CToxOptionsNodeList::OnNodeListKeyDown(CCtrlListView::TEventInfo *evt)
{
	LVITEM lvi = { 0 };
	lvi.iItem = m_nodes.GetSelectionMark();
	lvi.mask = LVIF_GROUPID;
	m_nodes.GetItem(&lvi);

	if (lvi.iGroupId && lvi.iItem != -1 && (evt->nmlvkey)->wVKey == VK_DELETE)
	{
		if (MessageBox(
			GetParent(m_hwnd),
			TranslateT("Are you sure?"),
			TranslateT("Node deleting"),
			MB_YESNO | MB_ICONWARNING) == IDYES)
		{
			m_nodes.DeleteItem(lvi.iItem);
			SendMessage(GetParent(GetParent(m_hwnd)), PSM_CHANGED, 0, 0);
		}
	}
}

void CToxOptionsNodeList::OnApply()
{
	char setting[MAX_PATH];

	LVITEM lvi = { 0 };
	lvi.cchTextMax = MAX_PATH;
	lvi.pszText = (TCHAR*)mir_alloc(MAX_PATH * sizeof(TCHAR));

	char module[MAX_PATH];
	mir_snprintf(module, SIZEOF(module), "%s_Nodes", m_proto->m_szModuleName);

	int iItem = 0;
	int itemCount = m_nodes.GetItemCount();
	for (int i = 0; i < itemCount; i++)
	{
		lvi.iItem = i;
		lvi.mask = LVIF_GROUPID;
		m_nodes.GetItem(&lvi);
		if (lvi.iGroupId == 0)
		{
			continue;
		}

		lvi.mask = LVIF_TEXT;
		lvi.iSubItem = 0;
		m_nodes.GetItem(&lvi);
		mir_snprintf(setting, TOX_SETTINGS_NODE_IPV4, iItem);
		db_set_s(NULL, module, setting, _T2A(lvi.pszText));

		lvi.iSubItem = 1;
		m_nodes.GetItem(&lvi);
		mir_snprintf(setting, TOX_SETTINGS_NODE_IPV6, iItem);
		db_set_s(NULL, module, setting, _T2A(lvi.pszText));

		lvi.iSubItem = 2;
		m_nodes.GetItem(&lvi);
		mir_snprintf(setting, TOX_SETTINGS_NODE_PORT, iItem);
		db_set_w(NULL, module, setting, _ttoi(lvi.pszText));

		lvi.iSubItem = 3;
		m_nodes.GetItem(&lvi);
		mir_snprintf(setting, TOX_SETTINGS_NODE_PKEY, iItem);
		db_set_s(NULL, module, setting, _T2A(lvi.pszText));

		iItem++;
	}
	itemCount = iItem;
	int nodeCount = db_get_b(NULL, module, TOX_SETTINGS_NODE_COUNT, 0);
	for (iItem = itemCount; iItem < nodeCount; iItem++)
	{
		mir_snprintf(setting, TOX_SETTINGS_NODE_IPV4, iItem);
		db_unset(NULL, module, setting);
		mir_snprintf(setting, TOX_SETTINGS_NODE_IPV6, iItem);
		db_unset(NULL, module, setting);
		mir_snprintf(setting, TOX_SETTINGS_NODE_PORT, iItem);
		db_unset(NULL, module, setting);
		mir_snprintf(setting, TOX_SETTINGS_NODE_PKEY, iItem);
		db_unset(NULL, module, setting);
	}
	db_set_b(NULL, module, TOX_SETTINGS_NODE_COUNT, itemCount);
}

/////////////////////////////////////////////////////////////////////////////////

int CToxProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.ptszTitle = m_tszUserName;
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_DONTTRANSLATE;
	odp.ptszGroup = LPGENT("Network");

	odp.ptszTab = LPGENT("Account");
	odp.pDialog = CToxOptionsMain::CreateOptionsPage(this);
	Options_AddPage(wParam, &odp);

	odp.ptszTab = LPGENT("Multimedia");
	odp.pDialog = CToxOptionsMultimedia::CreateOptionsPage(this);
	Options_AddPage(wParam, &odp);

	odp.ptszTab = LPGENT("Nodes");
	odp.pDialog = CToxOptionsNodeList::CreateOptionsPage(this);
	Options_AddPage(wParam, &odp);

	return 0;
}
