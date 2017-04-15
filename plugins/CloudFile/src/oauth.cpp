#include "stdafx.h"

COAuthDlg::COAuthDlg(CCloudService *service, const char *authUrl, pThreadFuncOwner requestAccessTokenThread)
	: CDlgBase(hInstance, IDD_OAUTH), m_service(service),
	m_requestAccessTokenThread(requestAccessTokenThread),
	m_auth(this, IDC_OAUTH_AUTHORIZE, authUrl),
	m_code(this, IDC_OAUTH_CODE), m_ok(this, IDOK)
{
	m_autoClose = CLOSE_ON_CANCEL;
	m_code.OnChange = Callback(this, &COAuthDlg::Code_OnChange);
	m_ok.OnClick = Callback(this, &COAuthDlg::Ok_OnClick);
}

void COAuthDlg::Code_OnChange(CCtrlBase*)
{
	ptrA requestToken(m_code.GetTextA());
	m_ok.Enable(mir_strlen(requestToken) != 0);
}

void COAuthDlg::Ok_OnClick(CCtrlButton*)
{
	mir_forkthreadowner(m_requestAccessTokenThread, m_service, m_hwnd);
}