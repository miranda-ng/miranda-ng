#include "stdafx.h"

COAuthDlg::COAuthDlg(CCloudService *service, const char *authUrl, pThreadFuncOwner requestAccessTokenThread)
	: CDlgBase(hInstance, IDD_OAUTH),
	m_service(service), m_authUrl(authUrl),
	m_requestAccessTokenThread(requestAccessTokenThread),
	m_authorize(this, IDC_OAUTH_AUTHORIZE),
	m_code(this, IDC_OAUTH_CODE), m_ok(this, IDOK)
{
	m_autoClose = CLOSE_ON_CANCEL;
	m_authorize.OnClick = Callback(this, &COAuthDlg::Auth_OnClick);
	m_code.OnChange = Callback(this, &COAuthDlg::Code_OnChange);
	m_ok.OnClick = Callback(this, &COAuthDlg::Ok_OnClick);
}

void COAuthDlg::OnInitDialog()
{
	CCtrlLabel &ctrl = *(CCtrlLabel*)FindControl(IDC_AUTH_TEXT);
	ptrW format(ctrl.GetText());
	wchar_t text[MAX_PATH];
	mir_snwprintf(text, (const wchar_t*)format, m_service->GetUserName());
	ctrl.SetText(text);
}

void COAuthDlg::Auth_OnClick(CCtrlHyperlink*)
{
	if (SUCCEEDED(OleInitialize(NULL))) {
		CComPtr<IWebBrowser2> browser;
		if (SUCCEEDED(CoCreateInstance(CLSID_InternetExplorer, NULL, CLSCTX_LOCAL_SERVER, IID_IWebBrowser2, (void**)&browser))) {
			VARIANT empty;
			VariantInit(&empty);
			BSTR url = SysAllocString(_A2T(m_authUrl));
			if (SUCCEEDED(browser->Navigate(url, &empty, &empty, &empty, &empty))) {
				browser->put_AddressBar(VARIANT_FALSE);
				browser->put_MenuBar(VARIANT_FALSE);
				browser->put_ToolBar(VARIANT_FALSE);
				browser->put_StatusBar(VARIANT_FALSE);
				//browser->put_Resizable(VARIANT_FALSE);
				browser->put_Visible(VARIANT_TRUE);
			}
			else
				browser->Quit();
			SysFreeString(url);
			browser.Release();
		}
		OleUninitialize();
	}
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