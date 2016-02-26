#include "stdafx.h"

CDropboxOptionsMain::CDropboxOptionsMain(CDropbox *instance)
	: CDropboxDlgBase(instance, IDD_OPTIONS_MAIN),
	m_auth(this, IDC_GETAUTH, DROPBOX_WWW_URL "/oauth2/authorize?response_type=code&client_id=" DROPBOX_APP_KEY),
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

CDropboxOptionsInterception::CDropboxOptionsInterception(CDropbox *instance)
	: CDropboxDlgBase(instance, IDD_OPTIONS_INTERCEPTION),
	m_accounts(this, IDC_ACCOUNTS), isAccountListInit(false)
{
}

void CDropboxOptionsInterception::OnInitDialog()
{
	CDropboxDlgBase::OnInitDialog();

	m_accounts.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_INFOTIP);

	m_accounts.AddColumn(0, _T("Account name"), 50);
	m_accounts.AddColumn(1, _T("Protocol"), 50);

	int count;
	PROTOACCOUNT** accounts;
	Proto_EnumAccounts(&count, &accounts);
	const char* interceptedProtos = db_get_sa(NULL, MODULE, "InterceptedProtos");
	for (int i = 0; i < count; i++) {
		PROTOACCOUNT *acc = accounts[i];
		if (strstr(acc->szProtoName, MODULE) || strstr(acc->szProtoName, "Meta"))
			continue;
		int iItem = m_accounts.AddItem(mir_tstrdup(acc->tszAccountName), -1, (LPARAM)acc);
		m_accounts.SetItem(iItem, 1, mir_a2t(acc->szProtoName));
		if (interceptedProtos && strstr(interceptedProtos, acc->szModuleName))
			m_accounts.SetCheckState(iItem, TRUE);
	}

	m_accounts.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
	m_accounts.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);

	isAccountListInit = true;
}

INT_PTR CDropboxOptionsInterception::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_NOTIFY:
	{
		LPNMHDR lpnmHdr = (LPNMHDR)lParam;
		if (lpnmHdr->idFrom == (UINT_PTR)m_accounts.GetCtrlId() && lpnmHdr->code == LVN_ITEMCHANGED)
		{
			LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
			if (pnmv->uChanged & LVIF_STATE && pnmv->uNewState & LVIS_STATEIMAGEMASK)
			{
				if (isAccountListInit)
					NotifyChange();
			}
		}
	}
	break;
	}

	return CDlgBase::DlgProc(msg, wParam, lParam);
}

void CDropboxOptionsInterception::OnApply()
{
	CMStringA interceptedProtos;
	int count = m_accounts.GetItemCount();
	for (int iItem = 0; iItem < count; iItem++)
	{
		TCHAR proto[MAX_PATH];
		PROTOACCOUNT *acc = (PROTOACCOUNT*)m_accounts.GetItemData(iItem);
		if (m_accounts.GetCheckState(iItem))
			interceptedProtos.AppendFormat("%s\t", acc->szModuleName);
		interceptedProtos.TrimRight();
	}
	db_set_s(NULL, MODULE, "InterceptedProtos", interceptedProtos);
}

/////////////////////////////////////////////////////////////////////////////////

int CDropbox::OnOptionsInitialized(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.ptszTitle = _T(MODULE);
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_DONTTRANSLATE;
	odp.ptszGroup = LPGENT("Network");

	odp.ptszTab = _T("General");
	odp.pDialog = CDropboxOptionsMain::CreateOptionsPage(this);
	Options_AddPage(wParam, &odp);

	odp.ptszTab = _T("Interception");
	odp.pDialog = CDropboxOptionsInterception::CreateOptionsPage(this);
	Options_AddPage(wParam, &odp);

	return 0;
}
