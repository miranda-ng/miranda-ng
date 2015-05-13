#include "stdafx.h"

CDropboxOptionsMain::CDropboxOptionsMain(CDropbox *instance, int idDialog)
	: CDropboxDlgBase(instance, idDialog),
	m_auth(this, IDC_GETAUTH, DROPBOX_WWW_URL DROPBOX_API_VER "/oauth2/authorize?response_type=code&client_id=" DROPBOX_APP_KEY),
	m_requestCode(this, IDC_REQUEST_CODE), m_authorize(this, IDC_AUTHORIZE), m_authStatus(this, IDC_AUTH_STATUS),
	m_useShortUrl(this, IDC_USE_SHORT_LINKS), m_urlAutoSend(this, IDC_URL_AUTOSEND),
	m_urlPasteToMessageInputArea(this, IDC_URL_COPYTOMIA), m_urlCopyToClipboard(this, IDC_URL_COPYTOCB)
{
	CreateLink(m_useShortUrl, "UseSortLinks", DBVT_BYTE, 1);
	CreateLink(m_urlAutoSend, "UrlAutoSend", DBVT_BYTE, 1);
	CreateLink(m_urlPasteToMessageInputArea, "UrlPasteToMessageInputArea", DBVT_BYTE, 0);
	CreateLink(m_urlCopyToClipboard, "UrlCopyToClipboard", DBVT_BYTE, 0);

	//m_auth.OnClick = Callback(this, &CDropboxOptionsMain::Auth_OnClick);
	m_requestCode.OnChange = Callback(this, &CDropboxOptionsMain::RequestCode_OnChange);
	m_authorize.OnClick = Callback(this, &CDropboxOptionsMain::Authorize_OnClick);
}

void CDropboxOptionsMain::OnInitDialog()
{
	CDropboxDlgBase::OnInitDialog();

	LOGFONT lf;
	HFONT hFont = (HFONT)m_authStatus.SendMsg(WM_GETFONT, 0, 0);
	GetObject(hFont, sizeof(lf), &lf);
	lf.lfWeight = FW_BOLD;
	m_authStatus.SendMsg(WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), 0);

	if (m_instance->HasAccessToken())
		m_authStatus.SetText(TranslateT("you are already authorized"));
	else
		m_authStatus.SetText(TranslateT("you are not authorized yet"));
}

void CDropboxOptionsMain::Auth_OnClick(CCtrlBase*)
{
	SetFocus(m_requestCode.GetHwnd());
}

void CDropboxOptionsMain::RequestCode_OnChange(CCtrlBase*)
{
	ptrA requestToken(m_requestCode.GetTextA());
	EnableWindow(m_authorize.GetHwnd(), mir_strlen(requestToken) != 0);
}

void CDropboxOptionsMain::Authorize_OnClick(CCtrlBase*)
{
	mir_forkthreadowner(CDropbox::RequestAccessTokenAsync, m_instance, m_hwnd, 0);
}

/////////////////////////////////////////////////////////////////////////////////

int CDropbox::OnOptionsInitialized(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.ptszTitle = _T(MODULE);
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_DONTTRANSLATE;
	odp.ptszGroup = LPGENT("Network");

	odp.ptszTab = LPGENT("Dropbox");
	odp.pDialog = CDropboxOptionsMain::CreateOptionsPage(this);
	Options_AddPage(wParam, &odp);

	return 0;
}
