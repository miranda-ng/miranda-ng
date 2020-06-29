#include "stdafx.h"

int CToxProto::OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	if (!Proto_IsProtoOnContact(hContact, m_szModuleName))
		return 0;

	if (!this->IsOnline())
		return 0;

	if (this->isChatRoom(hContact))
		return 0;

	bool isCtrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

	bool isAuthNeed = getByte(hContact, "Auth", 0) > 0;
	Menu_ShowItem(GetMenuItem(PROTO_MENU_REQ_AUTH), isCtrlPressed || isAuthNeed);

	bool isGrantNeed = getByte(hContact, "Grant", 0) > 0;
	Menu_ShowItem(GetMenuItem(PROTO_MENU_GRANT_AUTH), isCtrlPressed || isGrantNeed);

	return 0;
}

int CToxProto::UpdateStatusMenu(WPARAM, LPARAM)
{
	bool isOnline = IsOnline();
	Menu_EnableItem(StatusMenuItems[SMI_PASSWORD], isOnline);
	Menu_EnableItem(StatusMenuItems[SMI_PASSWORD_CREATE], isOnline);
	Menu_EnableItem(StatusMenuItems[SMI_PASSWORD_CHANGE], isOnline);
	Menu_EnableItem(StatusMenuItems[SMI_PASSWORD_REMOVE], isOnline);

	pass_ptrW password(getWStringA(TOX_SETTINGS_PASSWORD));
	bool passwordExists = mir_wstrlen(password) > 0;
	Menu_ShowItem(StatusMenuItems[SMI_PASSWORD_CREATE], !passwordExists);
	Menu_ShowItem(StatusMenuItems[SMI_PASSWORD_CHANGE], passwordExists);
	Menu_ShowItem(StatusMenuItems[SMI_PASSWORD_REMOVE], passwordExists);
	return 0;
}

void CToxProto::OnBuildProtoMenu()
{
	CMenuItem mi(&g_plugin);
	mi.flags = CMIF_UNICODE;
	mi.root = Menu_GetProtocolRoot(this);

	// Create copy tox id command
	mi.pszService = "/CopyToxID";
	CreateProtoService(mi.pszService, &CToxProto::OnCopyToxID);
	mi.name.w = LPGENW("Copy Tox ID");
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_COPY);
	mi.position = SMI_POSITION + SMI_TOXID_COPY;
	StatusMenuItems[SMI_TOXID_COPY] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	// Password
	mi.pszService = nullptr;
	mi.name.w = LPGENW("Password");
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_KEYS);
	StatusMenuItems[SMI_PASSWORD] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	// Create password command
	mi.root = StatusMenuItems[SMI_PASSWORD];
	mi.pszService = "/CreatePassword";
	CreateProtoService(mi.pszService, &CToxProto::OnCreatePassword);
	mi.name.w = LPGENW("Create password");
	mi.position = SMI_PASSWORD_CREATE;
	StatusMenuItems[SMI_PASSWORD_CREATE] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	// Change password command
	mi.pszService = "/ChangePassword";
	CreateProtoService(mi.pszService, &CToxProto::OnChangePassword);
	mi.name.w = LPGENW("Change password");
	mi.position = SMI_PASSWORD_CHANGE;
	StatusMenuItems[SMI_PASSWORD_CHANGE] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	// Remove password command
	mi.pszService = "/RemovePassword";
	CreateProtoService(mi.pszService, &CToxProto::OnRemovePassword);
	mi.name.w = LPGENW("Remove password");
	mi.position = SMI_PASSWORD_REMOVE;
	StatusMenuItems[SMI_PASSWORD_REMOVE] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	UpdateStatusMenu(NULL, NULL);
}
