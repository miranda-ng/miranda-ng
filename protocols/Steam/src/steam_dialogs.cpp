#include "stdafx.h"

CSteamPasswordEditor::CSteamPasswordEditor(CSteamProto *proto) :
	CSteamDlgBase(proto, IDD_PASSWORD_EDITOR),
	m_password(this, IDC_PASSWORD),
	m_savePermanently(this, IDC_SAVEPERMANENTLY)
{
}

bool CSteamPasswordEditor::OnInitDialog()
{
	char iconName[100];
	mir_snprintf(iconName, "%s_%s", MODULE, "main");
	Window_SetIcon_IcoLib(m_hwnd, IcoLib_GetIconHandle(iconName));

	SendMessage(m_password.GetHwnd(), EM_LIMITTEXT, 64, 0);

	Utils_RestoreWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "PasswordWindow");
	return true;
}

bool CSteamPasswordEditor::OnApply()
{
	m_proto->m_password = m_password.GetText();
	if (m_savePermanently.Enabled())
		m_proto->setWString("Password", m_proto->m_password);

	EndModal(DIALOG_RESULT_OK);
	return true;
}

void CSteamPasswordEditor::OnDestroy()
{
	Utils_SaveWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "PasswordWindow");
}
