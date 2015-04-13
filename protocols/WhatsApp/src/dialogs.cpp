#include "common.h"

#define szAskSendSms  LPGEN("An SMS with registration code will be sent to your mobile phone.\nNotice that you are not able to use the real WhatsApp and this plugin simultaneously!\nContinue?")
#define szPasswordSet LPGEN("Your password has been set automatically. You can proceed with login now")

class COptionsDlg : public CProtoDlgBase<WhatsAppProto>
{
	CCtrlEdit m_pw1, m_pw2, m_cc, m_login, m_nick, m_group;
	CCtrlCheck m_ssl, m_autorun;
	CCtrlButton m_request, m_register;

public:
	COptionsDlg(WhatsAppProto *pThis, int dlgId) :
		CProtoDlgBase<WhatsAppProto>(pThis, dlgId, false),
		m_cc(this, IDC_CC),
		m_pw1(this, IDC_PW),
		m_pw2(this, IDC_PW2),
		m_ssl(this, IDC_SSL),
		m_nick(this, IDC_NICK),
		m_group(this, IDC_DEFGROUP),
		m_login(this, IDC_LOGIN),
		m_autorun(this, IDC_AUTORUN),
		m_request(this, IDC_BUTTON_REQUEST_CODE),
		m_register(this, IDC_BUTTON_REGISTER)
	{
		CreateLink(m_ssl, WHATSAPP_KEY_SSL, DBVT_BYTE, false);
		CreateLink(m_autorun, WHATSAPP_KEY_AUTORUNCHATS, DBVT_BYTE, true);

		CreateLink(m_cc, WHATSAPP_KEY_CC, _T(""));
		CreateLink(m_nick, WHATSAPP_KEY_NICK, _T(""));
		CreateLink(m_login, WHATSAPP_KEY_LOGIN, _T(""));
		CreateLink(m_group, WHATSAPP_KEY_DEF_GROUP, _T(""));

		m_request.OnClick = Callback(this, &COptionsDlg::OnRequestClick);
		m_register.OnClick = Callback(this, &COptionsDlg::OnRegisterClick);
	}

	virtual void OnInitDialog()
	{
		m_pw1.SendMsg(EM_LIMITTEXT, 3, 0); m_pw1.Enable(false);
		m_pw2.SendMsg(EM_LIMITTEXT, 3, 0); m_pw2.Enable(false);
	}

	void OnRequestClick(CCtrlButton*)
	{
		if (IDYES != MessageBox(NULL, TranslateT(szAskSendSms), PRODUCT_NAME, MB_YESNO))
			return;

		ptrA cc(m_cc.GetTextA()), number(m_login.GetTextA());
		string password;
		if (m_proto->Register(REG_STATE_REQ_CODE, string(cc), string(number), string(), password)) {
			if (!password.empty()) {
				MessageBox(NULL, TranslateT(szPasswordSet), PRODUCT_NAME, MB_ICONWARNING);
				m_proto->setString(WHATSAPP_KEY_PASS, password.c_str());
			}
			else {
				m_pw1.Enable(); // unblock sms code entry field
				m_pw2.Enable();
			}
		}
	}

	void OnRegisterClick(CCtrlButton*)
	{
		if (GetWindowTextLength(m_pw1.GetHwnd()) != 3 || GetWindowTextLength(m_pw2.GetHwnd()) != 3) {
			MessageBox(NULL, TranslateT("Please correctly specify your registration code received by SMS"), PRODUCT_NAME, MB_ICONEXCLAMATION);
			return;
		}

		char code[10];
		GetWindowTextA(m_pw1.GetHwnd(), code, 4);
		GetWindowTextA(m_pw2.GetHwnd(), code + 3, 4);

		string password;
		ptrA cc(m_cc.GetTextA()), number(m_login.GetTextA());
		if (m_proto->Register(REG_STATE_REG_CODE, string(cc), string(number), string(code), password)) {
			m_proto->setString(WHATSAPP_KEY_PASS, password.c_str());
			MessageBox(NULL, TranslateT(szPasswordSet), PRODUCT_NAME, MB_ICONWARNING);
		}
	}

	virtual void OnApply()
	{
		ptrT tszGroup(m_group.GetText());
		if (mir_tstrcmp(m_proto->m_tszDefaultGroup, tszGroup))
			m_proto->m_tszDefaultGroup = tszGroup.detouch();

		if (m_proto->isOnline())
			MessageBox(NULL, TranslateT("Changes will be applied after protocol restart"), m_proto->m_tszUserName, MB_OK);
	}
};

INT_PTR WhatsAppProto::SvcCreateAccMgrUI(WPARAM wParam, LPARAM lParam)
{
	COptionsDlg *pDlg = new COptionsDlg(this, IDD_ACCMGRUI);
	pDlg->SetParent((HWND)lParam);
	pDlg->Show();
	return (INT_PTR)pDlg->GetHwnd();
}

int WhatsAppProto::OnOptionsInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.ptszTitle = m_tszUserName;
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_DONTTRANSLATE;
	odp.ptszGroup = LPGENT("Network");

	odp.ptszTab = LPGENT("Account");
	odp.pDialog = new COptionsDlg(this, IDD_OPTIONS);
	Options_AddPage(wParam, &odp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Invite dialog

class CInviteDialog : public CProtoDlgBase<WhatsAppProto>
{
	CCtrlClc m_clc;
	CCtrlEdit m_entry;
	CCtrlButton m_btnOk;

	void FilterList(CCtrlClc *)
	{
		for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
			char *proto = GetContactProto(hContact);
			if (mir_strcmp(proto, m_proto->m_szModuleName) || m_proto->isChatRoom(hContact))
				if (HANDLE hItem = m_clc.FindContact(hContact))
					m_clc.DeleteItem(hItem);
		}
	}

	void ResetListOptions(CCtrlClc *)
	{
		m_clc.SetBkBitmap(0, NULL);
		m_clc.SetBkColor(GetSysColor(COLOR_WINDOW));
		m_clc.SetGreyoutFlags(0);
		m_clc.SetLeftMargin(4);
		m_clc.SetIndent(10);
		m_clc.SetHideEmptyGroups(true);
		m_clc.SetHideOfflineRoot(true);
		for (int i = 0; i <= FONTID_MAX; i++)
			m_clc.SetTextColor(i, GetSysColor(COLOR_WINDOWTEXT));
	}

public:
	CInviteDialog(WhatsAppProto *pThis) :
		CProtoDlgBase<WhatsAppProto>(pThis, IDD_GROUPCHAT_INVITE, false),
		m_clc(this, IDC_CLIST),
		m_entry(this, IDC_NEWJID),
		m_btnOk(this, IDOK)
	{
		m_btnOk.OnClick = Callback(this, &CInviteDialog::btnOk_OnClick);
		m_clc.OnNewContact =
			m_clc.OnListRebuilt = Callback(this, &CInviteDialog::FilterList);
		m_clc.OnOptionsChanged = Callback(this, &CInviteDialog::ResetListOptions);
	}

	void OnInitDialog()
	{
		SetWindowLongPtr(m_clc.GetHwnd(), GWL_STYLE,
			GetWindowLongPtr(m_clc.GetHwnd(), GWL_STYLE) | CLS_CHECKBOXES | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE);
		m_clc.SendMsg(CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP | CLS_EX_TRACKSELECT, 0);

		ResetListOptions(&m_clc);
		FilterList(&m_clc);
	}

	void btnOk_OnClick(CCtrlButton*)
	{
		m_proto->m_szInviteJids.clear();

		// invite users from clist
		for (MCONTACT hContact = db_find_first(m_proto->m_szModuleName); hContact; hContact = db_find_next(hContact, m_proto->m_szModuleName)) {
			if (m_proto->isChatRoom(hContact))
				continue;

			if (HANDLE hItem = m_clc.FindContact(hContact)) {
				if (m_clc.GetCheck(hItem)) {
					ptrA jid(m_proto->getStringA(hContact, "ID"));
					if (jid != NULL)
						m_proto->m_szInviteJids.push_back((char*)jid);
				}
			}
		}

		ptrA tszText(m_entry.GetTextA());
		if (tszText != NULL)
			m_proto->m_szInviteJids.push_back(string(tszText));
	}
};

void WhatsAppProto::InviteChatUser(WAChatInfo *pInfo)
{
	CInviteDialog dlg(this);
	if (!dlg.DoModal())
		return;

	if (isOnline()) {
		m_pConnection->sendAddParticipants((char*)_T2A(pInfo->tszJid), m_szInviteJids);
		m_szInviteJids.clear();
	}
}
