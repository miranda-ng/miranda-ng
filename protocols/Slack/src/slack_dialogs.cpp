#include "stdafx.h"

CSlackOAuth::CSlackOAuth(CSlackProto *proto)
	: CSuper(proto, IDD_OAUTH, false),
	m_authorize(this, IDC_OAUTH_AUTHORIZE, SLACK_URL "/oauth/authorize?scope=identify+read+post&redirect_uri=" SLACK_REDIRECT_URL "&client_id=" SLACK_CLIENT_ID),
	m_code(this, IDC_OAUTH_CODE), m_ok(this, IDOK)
{
	m_ok.OnClick = Callback(this, &CSlackOAuth::OnOk);

	Utils_RestoreWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "OAuthWindow");
}

void CSlackOAuth::OnInitDialog()
{
	CSuper::OnInitDialog();

	Window_SetIcon_IcoLib(m_hwnd, m_proto->GetIconHandle(IDI_SLACK));

	SendMessage(m_code.GetHwnd(), EM_LIMITTEXT, 40, 0);
}

void CSlackOAuth::OnOk(CCtrlButton*)
{
	mir_strncpy(m_authCode, ptrA(m_code.GetTextA()), _countof(m_authCode));
	EndDialog(m_hwnd, 1);
}

void CSlackOAuth::OnClose()
{
	Utils_SaveWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "OAuthWindow");
}

const char* CSlackOAuth::GetAuthCode()
{
	return m_authCode;
}