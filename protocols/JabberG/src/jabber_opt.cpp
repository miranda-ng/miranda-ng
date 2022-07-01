/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (C) 2012-22 Miranda NG team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "stdafx.h"
#include "jabber_list.h"
#include "jabber_caps.h"

static BOOL(WINAPI *pfnEnableThemeDialogTexture)(HANDLE, uint32_t) = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////
// JabberRegisterDlgProc - the dialog proc for registering new account

struct { char *szCode; wchar_t *szDescription; } g_LanguageCodes[] = {
	{ "aa", LPGENW("Afar") },
	{ "ab", LPGENW("Abkhazian") },
	{ "af", LPGENW("Afrikaans") },
	{ "ak", LPGENW("Akan") },
	{ "sq", LPGENW("Albanian") },
	{ "am", LPGENW("Amharic") },
	{ "ar", LPGENW("Arabic") },
	{ "an", LPGENW("Aragonese") },
	{ "hy", LPGENW("Armenian") },
	{ "as", LPGENW("Assamese") },
	{ "av", LPGENW("Avaric") },
	{ "ae", LPGENW("Avestan") },
	{ "ay", LPGENW("Aymara") },
	{ "az", LPGENW("Azerbaijani") },
	{ "ba", LPGENW("Bashkir") },
	{ "bm", LPGENW("Bambara") },
	{ "eu", LPGENW("Basque") },
	{ "be", LPGENW("Belarusian") },
	{ "bn", LPGENW("Bengali") },
	{ "bh", LPGENW("Bihari") },
	{ "bi", LPGENW("Bislama") },
	{ "bs", LPGENW("Bosnian") },
	{ "br", LPGENW("Breton") },
	{ "bg", LPGENW("Bulgarian") },
	{ "my", LPGENW("Burmese") },
	{ "ca", LPGENW("Catalan; Valencian") },
	{ "ch", LPGENW("Chamorro") },
	{ "ce", LPGENW("Chechen") },
	{ "zh", LPGENW("Chinese") },
	{ "cu", LPGENW("Church Slavic; Old Slavonic") },
	{ "cv", LPGENW("Chuvash") },
	{ "kw", LPGENW("Cornish") },
	{ "co", LPGENW("Corsican") },
	{ "cr", LPGENW("Cree") },
	{ "cs", LPGENW("Czech") },
	{ "da", LPGENW("Danish") },
	{ "dv", LPGENW("Divehi; Dhivehi; Maldivian") },
	{ "nl", LPGENW("Dutch; Flemish") },
	{ "dz", LPGENW("Dzongkha") },
	{ "en", LPGENW("English") },
	{ "eo", LPGENW("Esperanto") },
	{ "et", LPGENW("Estonian") },
	{ "ee", LPGENW("Ewe") },
	{ "fo", LPGENW("Faroese") },
	{ "fj", LPGENW("Fijian") },
	{ "fi", LPGENW("Finnish") },
	{ "fr", LPGENW("French") },
	{ "fy", LPGENW("Western Frisian") },
	{ "ff", LPGENW("Fulah") },
	{ "ka", LPGENW("Georgian") },
	{ "de", LPGENW("German") },
	{ "gd", LPGENW("Gaelic; Scottish Gaelic") },
	{ "ga", LPGENW("Irish") },
	{ "gl", LPGENW("Galician") },
	{ "gv", LPGENW("Manx") },
	{ "el", LPGENW("Greek, Modern (1453-)") },
	{ "gn", LPGENW("Guarani") },
	{ "gu", LPGENW("Gujarati") },
	{ "ht", LPGENW("Haitian; Haitian Creole") },
	{ "ha", LPGENW("Hausa") },
	{ "he", LPGENW("Hebrew") },
	{ "hz", LPGENW("Herero") },
	{ "hi", LPGENW("Hindi") },
	{ "ho", LPGENW("Hiri Motu") },
	{ "hu", LPGENW("Hungarian") },
	{ "ig", LPGENW("Igbo") },
	{ "is", LPGENW("Icelandic") },
	{ "io", LPGENW("Ido") },
	{ "ii", LPGENW("Sichuan Yi") },
	{ "iu", LPGENW("Inuktitut") },
	{ "ie", LPGENW("Interlingue") },
	{ "ia", LPGENW("Interlingua (International Auxiliary Language Association)") },
	{ "id", LPGENW("Indonesian") },
	{ "ik", LPGENW("Inupiaq") },
	{ "it", LPGENW("Italian") },
	{ "jv", LPGENW("Javanese") },
	{ "ja", LPGENW("Japanese") },
	{ "kl", LPGENW("Kalaallisut; Greenlandic") },
	{ "kn", LPGENW("Kannada") },
	{ "ks", LPGENW("Kashmiri") },
	{ "kr", LPGENW("Kanuri") },
	{ "kk", LPGENW("Kazakh") },
	{ "km", LPGENW("Central Khmer") },
	{ "ki", LPGENW("Kikuyu; Gikuyu") },
	{ "rw", LPGENW("Kinyarwanda") },
	{ "ky", LPGENW("Kirghiz; Kyrgyz") },
	{ "kv", LPGENW("Komi") },
	{ "kg", LPGENW("Kongo") },
	{ "ko", LPGENW("Korean") },
	{ "kj", LPGENW("Kuanyama; Kwanyama") },
	{ "ku", LPGENW("Kurdish") },
	{ "lo", LPGENW("Lao") },
	{ "la", LPGENW("Latin") },
	{ "lv", LPGENW("Latvian") },
	{ "li", LPGENW("Limburgan; Limburger; Limburgish") },
	{ "ln", LPGENW("Lingala") },
	{ "lt", LPGENW("Lithuanian") },
	{ "lb", LPGENW("Luxembourgish; Letzeburgesch") },
	{ "lu", LPGENW("Luba-Katanga") },
	{ "lg", LPGENW("Ganda") },
	{ "mk", LPGENW("Macedonian") },
	{ "mh", LPGENW("Marshallese") },
	{ "ml", LPGENW("Malayalam") },
	{ "mi", LPGENW("Maori") },
	{ "mr", LPGENW("Marathi") },
	{ "ms", LPGENW("Malay") },
	{ "mg", LPGENW("Malagasy") },
	{ "mt", LPGENW("Maltese") },
	{ "mo", LPGENW("Moldavian") },
	{ "mn", LPGENW("Mongolian") },
	{ "na", LPGENW("Nauru") },
	{ "nv", LPGENW("Navajo; Navaho") },
	{ "nr", LPGENW("Ndebele, South; South Ndebele") },
	{ "nd", LPGENW("Ndebele, North; North Ndebele") },
	{ "ng", LPGENW("Ndonga") },
	{ "ne", LPGENW("Nepali") },
	{ "nn", LPGENW("Norwegian Nynorsk; Nynorsk, Norwegian") },
	{ "nb", LPGENW("Bokmaal, Norwegian; Norwegian Bokmaal") },
	{ "no", LPGENW("Norwegian") },
	{ "ny", LPGENW("Chichewa; Chewa; Nyanja") },
	{ "oc", LPGENW("Occitan (post 1500); Provencal") },
	{ "oj", LPGENW("Ojibwa") },
	{ "or", LPGENW("Oriya") },
	{ "om", LPGENW("Oromo") },
	{ "os", LPGENW("Ossetian; Ossetic") },
	{ "pa", LPGENW("Panjabi; Punjabi") },
	{ "fa", LPGENW("Persian") },
	{ "pi", LPGENW("Pali") },
	{ "pl", LPGENW("Polish") },
	{ "pt", LPGENW("Portuguese") },
	{ "ps", LPGENW("Pushto") },
	{ "qu", LPGENW("Quechua") },
	{ "rm", LPGENW("Romansh") },
	{ "ro", LPGENW("Romanian") },
	{ "rn", LPGENW("Rundi") },
	{ "ru", LPGENW("Russian") },
	{ "sg", LPGENW("Sango") },
	{ "sa", LPGENW("Sanskrit") },
	{ "sr", LPGENW("Serbian") },
	{ "hr", LPGENW("Croatian") },
	{ "si", LPGENW("Sinhala; Sinhalese") },
	{ "sk", LPGENW("Slovak") },
	{ "sl", LPGENW("Slovenian") },
	{ "se", LPGENW("Northern Sami") },
	{ "sm", LPGENW("Samoan") },
	{ "sn", LPGENW("Shona") },
	{ "sd", LPGENW("Sindhi") },
	{ "so", LPGENW("Somali") },
	{ "st", LPGENW("Sotho, Southern") },
	{ "es", LPGENW("Spanish; Castilian") },
	{ "sc", LPGENW("Sardinian") },
	{ "ss", LPGENW("Swati") },
	{ "su", LPGENW("Sundanese") },
	{ "sw", LPGENW("Swahili") },
	{ "sv", LPGENW("Swedish") },
	{ "ty", LPGENW("Tahitian") },
	{ "ta", LPGENW("Tamil") },
	{ "tt", LPGENW("Tatar") },
	{ "te", LPGENW("Telugu") },
	{ "tg", LPGENW("Tajik") },
	{ "tl", LPGENW("Tagalog") },
	{ "th", LPGENW("Thai") },
	{ "bo", LPGENW("Tibetan") },
	{ "ti", LPGENW("Tigrinya") },
	{ "to", LPGENW("Tonga (Tonga Islands)") },
	{ "tn", LPGENW("Tswana") },
	{ "ts", LPGENW("Tsonga") },
	{ "tk", LPGENW("Turkmen") },
	{ "tr", LPGENW("Turkish") },
	{ "tw", LPGENW("Twi") },
	{ "ug", LPGENW("Uighur; Uyghur") },
	{ "uk", LPGENW("Ukrainian") },
	{ "ur", LPGENW("Urdu") },
	{ "uz", LPGENW("Uzbek") },
	{ "ve", LPGENW("Venda") },
	{ "vi", LPGENW("Vietnamese") },
	{ "vo", LPGENW("Volapuk") },
	{ "cy", LPGENW("Welsh") },
	{ "wa", LPGENW("Walloon") },
	{ "wo", LPGENW("Wolof") },
	{ "xh", LPGENW("Xhosa") },
	{ "yi", LPGENW("Yiddish") },
	{ "yo", LPGENW("Yoruba") },
	{ "za", LPGENW("Zhuang; Chuang") },
	{ "zu", LPGENW("Zulu") },
	{ nullptr, nullptr }
};

class CJabberDlgRegister : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	bool m_bProcessStarted;
	JABBER_CONN_DATA *m_regInfo;

	CCtrlButton m_btnOk;

public:
	CJabberDlgRegister(CJabberProto *proto, HWND hwndParent, JABBER_CONN_DATA *regInfo) :
		CJabberDlgBase(proto, IDD_OPT_REGISTER),
		m_bProcessStarted(false),
		m_regInfo(regInfo),
		m_btnOk(this, IDOK)
	{
		SetParent(hwndParent);
	}

	bool OnInitDialog() override
	{
		CMStringA text(FORMAT, "%s %s@%s:%d?", TranslateU("Register"), m_regInfo->username, m_regInfo->server, m_regInfo->port);
		SetDlgItemTextUtf(m_hwnd, IDC_REG_STATUS, text);
		return true;
	}

	bool OnApply() override
	{
		if (m_bProcessStarted) {
			Close();
			return true;
		}

		ShowWindow(GetDlgItem(m_hwnd, IDC_PROGRESS_REG), SW_SHOW);

		m_regInfo->pDlg = this;
		m_proto->ForkThread((CJabberProto::MyThreadFunc) & CJabberProto::ServerThread, m_regInfo);

		m_btnOk.SetText(TranslateT("Cancel"));
		m_bProcessStarted = true;
		return false;
	}

	void OnDestroy() override
	{
		m_regInfo->pDlg = nullptr;
	}

	void Update(int progress, const wchar_t *pwszText)
	{
		SetDlgItemTextW(m_hwnd, IDC_REG_STATUS, pwszText);
		SendDlgItemMessageW(m_hwnd, IDC_PROGRESS_REG, PBM_SETPOS, progress, 0);

		if (progress >= 100)
			m_btnOk.SetText(TranslateT("Close"));
		else
			SetFocus(GetDlgItem(m_hwnd, IDC_PROGRESS_REG));
	}
};

void JABBER_CONN_DATA::SetProgress(int progress, const wchar_t *pwszText)
{
	if (pDlg)
		pDlg->Update(progress, pwszText);
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberOptDlgProc - main options dialog procedure

class CCtrlEditJid : public CCtrlEdit
{
	typedef CCtrlEdit CSuper;

public:
	CCtrlEditJid(CDlgBase *dlg, int ctrlId);

	void OnInit()
	{
		CCtrlEdit::OnInit();
		Subclass();
	}

protected:
	virtual LRESULT CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (msg == WM_CHAR) {
			switch (wParam) {
			case '\"':  case '&':	case '\'':	case '/':
			case ':':	case '<':	case '>':	case '@':
				MessageBeep(MB_ICONASTERISK);
				return 0;
			}
		}
		return CCtrlEdit::CustomWndProc(msg, wParam, lParam);
	}
};

CCtrlEditJid::CCtrlEditJid(CDlgBase* dlg, int ctrlId):
	CCtrlEdit(dlg, ctrlId)
{
}

static void sttStoreJidFromUI(CJabberProto *ppro, CCtrlEdit &txtUsername, CCtrlEdit &cbServer)
{
	ppro->setWString("jid", CMStringW(FORMAT, L"%s@%s", ptrW(txtUsername.GetText()).get(), ptrW(cbServer.GetText()).get()));
}

class CDlgOptAccount : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	CCtrlEditJid	m_txtUsername;
	CCtrlEdit		m_txtPassword;
	CCtrlEdit		m_txtPriority;
	CCtrlCheck		m_chkSavePassword;
	CCtrlCombo		m_cbResource;
	CCtrlCheck		m_chkUseHostnameAsResource;
	CCtrlCheck		m_chkUseDomainLogin;
	CCtrlEdit		m_txtServer;
	CCtrlEdit		m_txtPort;
	CCtrlCheck		m_chkUseSsl;
	CCtrlCheck		m_chkUseTls;
	CCtrlCheck		m_chkManualHost;
	CCtrlEdit		m_txtManualHost;
	CCtrlEdit		m_txtManualPort;
	CCtrlCheck		m_chkKeepAlive;
	CCtrlCheck		m_chkAutoDeleteContacts;
	CCtrlCombo		m_cbLocale, m_cbMam;
	CCtrlButton		m_btnRegister;
	CCtrlButton		m_btnUnregister;
	CCtrlButton		m_btnChangePassword;

public:
	CDlgOptAccount(CJabberProto *proto) :
		CJabberDlgBase(proto, IDD_OPT_JABBER),
		m_txtUsername(this, IDC_EDIT_USERNAME),
		m_txtPassword(this, IDC_EDIT_PASSWORD),
		m_txtPriority(this, IDC_PRIORITY),
		m_chkSavePassword(this, IDC_SAVEPASSWORD),
		m_cbResource(this, IDC_COMBO_RESOURCE),
		m_chkUseHostnameAsResource(this, IDC_HOSTNAME_AS_RESOURCE),
		m_chkUseDomainLogin(this, IDC_USEDOMAINLOGIN),
		m_cbMam(this, IDC_MAM_MODE),
		m_txtServer(this, IDC_EDIT_LOGIN_SERVER),
		m_txtPort(this, IDC_PORT),
		m_chkUseSsl(this, IDC_USE_SSL),
		m_chkUseTls(this, IDC_USE_TLS),
		m_chkManualHost(this, IDC_MANUAL),
		m_txtManualHost(this, IDC_HOST),
		m_txtManualPort(this, IDC_HOSTPORT),
		m_chkKeepAlive(this, IDC_KEEPALIVE),
		m_chkAutoDeleteContacts(this, IDC_ROSTER_SYNC),
		m_cbLocale(this, IDC_MSGLANG),
		m_btnRegister(this, IDC_BUTTON_REGISTER),
		m_btnUnregister(this, IDC_UNREGISTER),
		m_btnChangePassword(this, IDC_BUTTON_CHANGE_PASSWORD)
	{
		CreateLink(m_txtUsername, "LoginName", L"");
		CreateLink(m_txtPriority, "Priority", DBVT_DWORD, 0);
		CreateLink(m_chkSavePassword, proto->m_bSavePassword);
		CreateLink(m_cbResource, "Resource", L"Miranda");
		CreateLink(m_chkUseHostnameAsResource, proto->m_bHostNameAsResource);
		CreateLink(m_chkUseDomainLogin, proto->m_bUseDomainLogin);
		CreateLink(m_txtServer, "LoginServer", L"jabber.org");
		CreateLink(m_txtPort, "Port", DBVT_WORD, 5222);
		CreateLink(m_chkUseSsl, proto->m_bUseSSL);
		CreateLink(m_chkUseTls, proto->m_bUseTLS);
		CreateLink(m_chkManualHost, proto->m_bManualConnect);
		CreateLink(m_txtManualHost, "ManualHost", L"");
		CreateLink(m_txtManualPort, "ManualPort", DBVT_WORD, 0);
		CreateLink(m_chkKeepAlive, proto->m_bKeepAlive);
		CreateLink(m_chkAutoDeleteContacts, proto->m_bRosterSync);

		// Bind events
		m_chkManualHost.OnChange = Callback(this, &CDlgOptAccount::chkManualHost_OnChange);
		m_chkUseHostnameAsResource.OnChange = Callback(this, &CDlgOptAccount::chkUseHostnameAsResource_OnChange);
		m_chkUseDomainLogin.OnChange = Callback(this, &CDlgOptAccount::chkUseDomainLogin_OnChange);
		m_chkUseSsl.OnChange = Callback(this, &CDlgOptAccount::chkUseSsl_OnChange);
		m_chkUseTls.OnChange = Callback(this, &CDlgOptAccount::chkUseTls_OnChange);

		m_btnRegister.OnClick = Callback(this, &CDlgOptAccount::btnRegister_OnClick);
		m_btnUnregister.OnClick = Callback(this, &CDlgOptAccount::btnUnregister_OnClick);
		m_btnChangePassword.OnClick = Callback(this, &CDlgOptAccount::btnChangePassword_OnClick);
	}

protected:
	bool OnInitDialog() override
	{
		CSuper::OnInitDialog();

		SendDlgItemMessage(m_hwnd, IDC_PRIORITY_SPIN, UDM_SETRANGE, 0, (LPARAM)MAKELONG(127, -128));

		wchar_t *passw = m_proto->getWStringA(0, "Password");
		if (passw) {
			m_txtPassword.SetText(passw);
			mir_free(passw);
		}

		// fill predefined resources
		wchar_t *szResources[] = { L"Home", L"Work", L"Office", L"Miranda" };
		for (auto &it : szResources)
			m_cbResource.AddString(it);


		// fill MAM modes
		wchar_t *szMamModes[] = { LPGENW("Never"), LPGENW("Roster"), LPGENW("Always") };
		for (auto &it : szMamModes)
			m_cbMam.AddString(TranslateW(it), int(&it - szMamModes));
		m_cbMam.SetCurSel(m_proto->m_iMamMode);
		m_cbMam.Enable(m_proto->m_bMamPrefsAvailable);

		// append computer name to the resource list
		wchar_t szCompName[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD dwCompNameLength = MAX_COMPUTERNAME_LENGTH;
		if (GetComputerName(szCompName, &dwCompNameLength))
			m_cbResource.AddString(szCompName);

		ptrW tszResource(m_proto->getWStringA("Resource"));
		if (tszResource != nullptr) {
			if (CB_ERR == m_cbResource.FindString(tszResource, -1, true))
				m_cbResource.AddString(tszResource);
			m_cbResource.SetText(tszResource);
		}
		else m_cbResource.SetText(L"Miranda");

		for (int i = 0; g_LanguageCodes[i].szCode; i++) {
			int iItem = m_cbLocale.AddString(TranslateW(g_LanguageCodes[i].szDescription), (LPARAM)g_LanguageCodes[i].szCode);
			if (!mir_strcmp(m_proto->m_tszSelectedLang, g_LanguageCodes[i].szCode))
				m_cbLocale.SetCurSel(iItem);
		}

		EnableWindow(GetDlgItem(m_hwnd, IDC_COMBO_RESOURCE), !m_chkUseHostnameAsResource.GetState());
		EnableWindow(GetDlgItem(m_hwnd, IDC_UNREGISTER), m_proto->m_bJabberOnline);

		m_chkUseTls.Enable(!m_proto->m_bDisable3920auth && (m_proto->m_bUseSSL ? false : true));
		if (m_proto->m_bDisable3920auth) m_chkUseTls.SetState(BST_UNCHECKED);
		m_chkUseSsl.Enable(m_proto->m_bDisable3920auth || (m_proto->m_bUseTLS ? false : true));

		if (m_proto->m_bManualConnect) {
			m_txtManualHost.Enable();
			m_txtManualPort.Enable();
			m_txtPort.Disable();
		}

		if (m_proto->m_bUseDomainLogin)
			chkUseDomainLogin_OnChange(&m_chkUseDomainLogin);

		CheckRegistration();
		return true;
	}

	bool OnApply() override
	{
		// clear saved password
		m_proto->m_savedPassword = nullptr;

		if (m_chkSavePassword.GetState())
			m_proto->setWString("Password", ptrW(m_txtPassword.GetText()));
		else
			m_proto->delSetting("Password");

		char *szLanguageCode = (char *)m_cbLocale.GetCurData();
		if (szLanguageCode && szLanguageCode != INVALID_HANDLE_VALUE) {
			m_proto->setString("XmlLang", szLanguageCode);
			replaceStr(m_proto->m_tszSelectedLang, szLanguageCode);
		}

		if (m_cbMam.Enabled() && m_cbMam.GetCurSel() != m_proto->m_iMamMode)
			m_proto->MamSetMode(m_cbMam.GetCurSel());

		sttStoreJidFromUI(m_proto, m_txtUsername, m_txtServer);

		if (m_proto->m_bJabberOnline) {
			if (m_txtUsername.IsChanged() || m_txtPassword.IsChanged() || m_cbResource.IsChanged() ||
				m_txtServer.IsChanged() || m_chkUseHostnameAsResource.IsChanged() || m_txtPort.IsChanged() ||
				m_txtManualHost.IsChanged() || m_txtManualPort.IsChanged() || m_cbLocale.IsChanged()) {
				MessageBox(m_hwnd,
					TranslateT("These changes will take effect the next time you connect to the Jabber network."),
					TranslateT("Jabber Protocol Option"), MB_OK | MB_SETFOREGROUND);
			}

			m_proto->SendPresence(m_proto->m_iStatus, true);
		}
		return true;
	}

	void OnChange() override
	{
		if (m_bInitialized)
			CheckRegistration();
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_ACTIVATE) {
			m_chkUseTls.Enable(!m_proto->m_bDisable3920auth && !m_proto->m_bUseSSL);
			if (m_proto->m_bDisable3920auth)
				m_chkUseTls.SetState(BST_UNCHECKED);
		}

		return CSuper::DlgProc(msg, wParam, lParam);
	}

private:
	void btnRegister_OnClick(CCtrlButton *)
	{
		PSHNOTIFY pshn = {};
		pshn.hdr.code = PSN_APPLY;
		pshn.hdr.hwndFrom = m_hwnd;
		SendMessage(m_hwnd, WM_NOTIFY, 0, (LPARAM)&pshn);

		JABBER_CONN_DATA regInfo;
		m_txtUsername.GetTextU(regInfo.username, _countof(regInfo.username));
		m_txtPassword.GetTextU(regInfo.password, _countof(regInfo.password));
		m_txtServer.GetTextA(regInfo.server, _countof(regInfo.server));
		if (m_chkManualHost.GetState()) {
			regInfo.port = (uint16_t)m_txtManualPort.GetInt();
			m_txtManualHost.GetTextA(regInfo.manualHost, _countof(regInfo.manualHost));
		}
		else {
			regInfo.port = (uint16_t)m_txtPort.GetInt();
			regInfo.manualHost[0] = '\0';
		}

		if (regInfo.username[0] && regInfo.password[0] && regInfo.server[0] && regInfo.port > 0 && (!m_chkManualHost.GetState() || regInfo.manualHost[0])) {
			CJabberDlgRegister dlg(m_proto, m_hwnd, &regInfo);
			dlg.DoModal();
		}
	}

	void btnUnregister_OnClick(CCtrlButton *)
	{
		int res = MessageBox(nullptr,
			TranslateT("This operation will kill your account, roster and all other information stored at the server. Are you ready to do that?"),
			TranslateT("Account removal warning"), MB_YESNOCANCEL);

		if (res == IDYES)
			m_proto->m_ThreadInfo->send(
				XmlNodeIq("set", m_proto->SerialNext(), m_proto->m_szJabberJID) << XQUERY(JABBER_FEAT_REGISTER)
				<< XCHILD("remove"));
	}

	void btnChangePassword_OnClick(CCtrlButton *)
	{
		if (!m_proto->m_bJabberOnline) {
			MessageBox(nullptr,
				TranslateT("You can change your password only when you are online"),
				TranslateT("You must be online"), MB_OK | MB_ICONSTOP);
			return;
		}

		m_proto->OnMenuHandleChangePassword(0, 0);
	}

	void chkManualHost_OnChange(CCtrlData *sender)
	{
		CCtrlCheck *chk = (CCtrlCheck *)sender;

		if (chk->GetState()) {
			m_txtManualHost.Enable();
			m_txtManualPort.Enable();
			m_txtPort.Disable();
		}
		else {
			m_txtManualHost.Disable();
			m_txtManualPort.Disable();
			m_txtPort.Enable();
		}
	}

	void chkUseHostnameAsResource_OnChange(CCtrlData *sender)
	{
		CCtrlCheck *chk = (CCtrlCheck *)sender;

		m_cbResource.Enable(!chk->GetState());
		if (chk->GetState()) {
			wchar_t szCompName[MAX_COMPUTERNAME_LENGTH + 1];
			DWORD dwCompNameLength = MAX_COMPUTERNAME_LENGTH;
			if (GetComputerName(szCompName, &dwCompNameLength))
				m_cbResource.SetText(szCompName);
		}
	}

	void chkUseDomainLogin_OnChange(CCtrlData *sender)
	{
		CCtrlCheck *chk = (CCtrlCheck *)sender;
		bool bChecked = chk->GetState();

		m_txtPassword.Enable(!bChecked);
		m_txtUsername.Enable(!bChecked);
		m_chkSavePassword.Enable(!bChecked);
		if (bChecked) {
			m_txtPassword.SetText(L"");
			m_txtUsername.SetText(L"");
			m_chkSavePassword.SetState(BST_CHECKED);
		}
	}

	void chkUseSsl_OnChange(CCtrlData *)
	{
		bool bManualHost = m_chkManualHost.GetState();
		if (m_chkUseSsl.GetState()) {
			m_chkUseTls.Disable();
			if (!bManualHost)
				m_txtPort.SetInt(5223);
		}
		else {
			if (!m_proto->m_bDisable3920auth)
				m_chkUseTls.Enable();
			if (!bManualHost)
				m_txtPort.SetInt(5222);
		}
	}

	void chkUseTls_OnChange(CCtrlData *)
	{
		if (m_chkUseTls.GetState())
			m_chkUseSsl.Disable();
		else
			m_chkUseSsl.Enable();
	}

	void CheckRegistration()
	{
		JABBER_CONN_DATA regInfo;
		m_txtUsername.GetTextU(regInfo.username, _countof(regInfo.username));
		m_txtPassword.GetTextU(regInfo.password, _countof(regInfo.password));
		m_txtServer.GetTextA(regInfo.server, _countof(regInfo.server));
		if (m_chkManualHost.GetState()) {
			regInfo.port = (uint16_t)m_txtManualPort.GetInt();
			m_txtManualHost.GetTextA(regInfo.manualHost, _countof(regInfo.manualHost));
		}
		else {
			regInfo.port = (uint16_t)m_txtPort.GetInt();
			regInfo.manualHost[0] = '\0';
		}

		if (regInfo.username[0] && regInfo.password[0] && regInfo.server[0] && regInfo.port > 0 && (!m_chkManualHost.GetState() || regInfo.manualHost[0]))
			m_btnRegister.Enable();
		else
			m_btnRegister.Disable();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// JabberAdvOptDlgProc - advanced options dialog procedure

class CDlgOptAdvanced : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	CCtrlCheck		m_chkDirect;
	CCtrlCheck		m_chkDirectManual;
	CCtrlEdit		m_txtDirect;
	CCtrlTreeOpts	m_options;

	bool m_oldFrameValue;

public:
	CDlgOptAdvanced(CJabberProto *proto) :
		CJabberDlgBase(proto, IDD_OPT_JABBER2),
		m_chkDirect(this, IDC_DIRECT),
		m_chkDirectManual(this, IDC_DIRECT_MANUAL),
		m_txtDirect(this, IDC_DIRECT_ADDR),
		m_options(this, IDC_OPTTREE),
		m_oldFrameValue(proto->m_bDisableFrame)
	{
		CreateLink(m_chkDirect, proto->m_bBsDirect);
		CreateLink(m_chkDirectManual, proto->m_bBsDirectManual);
		CreateLink(m_txtDirect, "BsDirectAddr", L"");

		m_chkDirect.OnChange = m_chkDirectManual.OnChange = Callback(this, &CDlgOptAdvanced::chkDirect_OnChange);

		m_options.AddOption(LPGENW("Messaging"), LPGENW("Use message delivery receipts (XEP-0184)"), m_proto->m_bMsgAck);
		m_options.AddOption(LPGENW("Messaging"), LPGENW("Enable avatars"), m_proto->m_bEnableAvatars);
		m_options.AddOption(LPGENW("Messaging"), LPGENW("Log chat state changes"), m_proto->m_bLogChatstates);
		m_options.AddOption(LPGENW("Messaging"), LPGENW("Log presence subscription state changes"), m_proto->m_bLogPresence);
		m_options.AddOption(LPGENW("Messaging"), LPGENW("Log presence errors"), m_proto->m_bLogPresenceErrors);
		m_options.AddOption(LPGENW("Messaging"), LPGENW("Enable user moods receiving"), m_proto->m_bEnableUserMood);
		m_options.AddOption(LPGENW("Messaging"), LPGENW("Enable user tunes receiving"), m_proto->m_bEnableUserTune);
		m_options.AddOption(LPGENW("Messaging"), LPGENW("Enable user activity receiving"), m_proto->m_bEnableUserActivity);
		m_options.AddOption(LPGENW("Messaging"), LPGENW("Receive notes"), m_proto->m_bAcceptNotes);
		m_options.AddOption(LPGENW("Messaging"), LPGENW("Automatically save received notes"), m_proto->m_bAutosaveNotes);
		m_options.AddOption(LPGENW("Messaging"), LPGENW("Inline pictures in messages (XEP-0231)"), m_proto->m_bInlinePictures);
		m_options.AddOption(LPGENW("Messaging"), LPGENW("Enable chat states sending (XEP-0085)"), m_proto->m_bEnableChatStates);
		m_options.AddOption(LPGENW("Messaging"), LPGENW("Enable server-side history (XEP-0136)"), m_proto->m_bEnableMsgArchive);
		m_options.AddOption(LPGENW("Messaging"), LPGENW("Enable Message Archive Management (XEP-0313)"), m_proto->m_bEnableMam);
		m_options.AddOption(LPGENW("Messaging"), LPGENW("Enable carbon copies (XEP-0280)"), m_proto->m_bEnableCarbons);
		m_options.AddOption(LPGENW("Messaging"), LPGENW("Use Stream Management (XEP-0198) if possible (Testing)"), m_proto->m_bEnableStreamMgmt);

		m_options.AddOption(LPGENW("Server options"), LPGENW("Disable SASL authentication (for old servers)"), m_proto->m_bDisable3920auth);
		m_options.AddOption(LPGENW("Server options"), LPGENW("Enable stream compression (if possible)"), m_proto->m_bEnableZlib);

		m_options.AddOption(LPGENW("Other"), LPGENW("Enable remote controlling (from another resource of same JID only)"), m_proto->m_bEnableRemoteControl);
		m_options.AddOption(LPGENW("Other"), LPGENW("Show transport agents on contact list"), m_proto->m_bShowTransport);
		m_options.AddOption(LPGENW("Other"), LPGENW("Automatically add contact when accept authorization"), m_proto->m_bAutoAdd);
		m_options.AddOption(LPGENW("Other"), LPGENW("Automatically accept authorization requests"), m_proto->m_bAutoAcceptAuthorization);
		m_options.AddOption(LPGENW("Other"), LPGENW("Fix incorrect timestamps in incoming messages"), m_proto->m_bFixIncorrectTimestamps);
		m_options.AddOption(LPGENW("Other"), LPGENW("Disable frame"), m_proto->m_bDisableFrame);
		m_options.AddOption(LPGENW("Other"), LPGENW("Enable XMPP link processing (requires AssocMgr)"), m_proto->m_bProcessXMPPLinks);
		m_options.AddOption(LPGENW("Other"), LPGENW("Embrace picture URLs with [img]"), m_proto->m_bEmbraceUrls);
		m_options.AddOption(LPGENW("Other"), LPGENW("Ignore server roster (groups and nick names)"), m_proto->m_bIgnoreRoster);

		m_options.AddOption(LPGENW("Security"), LPGENW("Allow local time and timezone requests (XEP-0202)"), m_proto->m_bAllowTimeReplies); 
		m_options.AddOption(LPGENW("Security"), LPGENW("Allow servers to request version (XEP-0092)"), m_proto->m_bAllowVersionRequests);
		m_options.AddOption(LPGENW("Security"), LPGENW("Show information about operating system in version replies"), m_proto->m_bShowOSVersion);
		m_options.AddOption(LPGENW("Security"), LPGENW("Accept HTTP Authentication requests (XEP-0070)"), m_proto->m_bAcceptHttpAuth);
		m_options.AddOption(LPGENW("Security"), LPGENW("Use OMEMO encryption for messages if possible (XEP-0384) (Basic support without GUI)"), m_proto->m_bUseOMEMO);
	}

	bool OnInitDialog() override
	{
		CSuper::OnInitDialog();

		chkDirect_OnChange(&m_chkDirect);
		return true;
	}

	bool OnApply() override
	{
		if (m_proto->m_bDisableFrame != m_oldFrameValue) {
			m_proto->InitInfoFrame(); // create or destroy a frame
			m_oldFrameValue = m_proto->m_bDisableFrame;
		}

		BOOL bChecked = m_proto->m_bShowTransport;
		LISTFOREACH(index, m_proto, LIST_ROSTER)
		{
			JABBER_LIST_ITEM *item = m_proto->ListGetItemPtrFromIndex(index);
			if (item != nullptr) {
				if (strchr(item->jid, '@') == nullptr) {
					MCONTACT hContact = m_proto->HContactFromJID(item->jid);
					if (hContact != 0) {
						if (bChecked) {
							if (item->getTemp()->m_iStatus != m_proto->getWord(hContact, "Status", ID_STATUS_OFFLINE)) {
								m_proto->setWord(hContact, "Status", (uint16_t)item->getTemp()->m_iStatus);
							}
						}
						else if (m_proto->getWord(hContact, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
							m_proto->setWord(hContact, "Status", ID_STATUS_OFFLINE);
					}
				}
			}
		}

		if (m_proto->m_bUseOMEMO)
			m_proto->m_omemo.init();
		else
			m_proto->m_omemo.deinit();
		m_proto->UpdateFeatHash();
		m_proto->SendPresence(m_proto->m_iStatus, true);
		return true;
	}

	void chkDirect_OnChange(CCtrlData *)
	{
		if (m_chkDirect.GetState()) {
			if (m_chkDirectManual.GetState())
				m_txtDirect.Enable();
			else
				m_txtDirect.Disable();

			m_chkDirectManual.Enable();
		}
		else {
			m_txtDirect.Disable();
			m_chkDirectManual.Disable();
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// JabberGcOptDlgProc - chat options dialog procedure

class CDlgOptGc : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	CCtrlEdit		m_txtAltNick;
	CCtrlEdit		m_txtSlap;
	CCtrlEdit		m_txtQuit;
	CCtrlTreeOpts	m_options;

public:
	CDlgOptGc(CJabberProto *proto) :
		CJabberDlgBase(proto, IDD_OPT_JABBER4),
		m_txtAltNick(this, IDC_TXT_ALTNICK),
		m_txtSlap(this, IDC_TXT_SLAP),
		m_txtQuit(this, IDC_TXT_QUIT),
		m_options(this, IDC_OPTTREE)
	{
		CreateLink(m_txtAltNick, "GcAltNick", L"");
		CreateLink(m_txtSlap, "GcMsgSlap", TranslateW(_T(JABBER_GC_MSG_SLAP)));
		CreateLink(m_txtQuit, "GcMsgQuit", TranslateW(_T(JABBER_GC_MSG_QUIT)));

		m_options.AddOption(LPGENW("General"), LPGENW("Autoaccept multiuser chat invitations"),   m_proto->m_bAutoAcceptMUC);
		m_options.AddOption(LPGENW("General"), LPGENW("Automatically join bookmarks on login"),   m_proto->m_bAutoJoinBookmarks);
		m_options.AddOption(LPGENW("General"), LPGENW("Automatically join conferences on login"), m_proto->m_bAutoJoinConferences);
		m_options.AddOption(LPGENW("General"), LPGENW("Do not open chat windows on creation"),    m_proto->m_bAutoJoinHidden);
		m_options.AddOption(LPGENW("General"), LPGENW("Do not show multiuser chat invitations"),  m_proto->m_bIgnoreMUCInvites);
		
		m_options.AddOption(LPGENW("Log events"), LPGENW("Ban notifications"),                    m_proto->m_bGcLogBans);
		m_options.AddOption(LPGENW("Log events"), LPGENW("Room configuration changes"),           m_proto->m_bGcLogConfig);
		m_options.AddOption(LPGENW("Log events"), LPGENW("Affiliation changes"),                  m_proto->m_bGcLogAffiliations);
		m_options.AddOption(LPGENW("Log events"), LPGENW("Role changes"),                         m_proto->m_bGcLogRoles);
		m_options.AddOption(LPGENW("Log events"), LPGENW("Status changes"),                       m_proto->m_bGcLogStatuses);
		m_options.AddOption(LPGENW("Log events"), LPGENW("Don't notify history messages"),        m_proto->m_bGcLogChatHistory);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// JabberOptInit - initializes all options dialogs

int CJabberProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.w = LPGENW("Network");
	odp.szTitle.w = m_tszUserName;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE | ODPF_DONTTRANSLATE;

	odp.szTab.w = LPGENW("Account");
	odp.pDialog = new CDlgOptAccount(this);
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Conferences");
	odp.pDialog = new CDlgOptGc(this);
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Advanced");
	odp.pDialog = new CDlgOptAdvanced(this);
	g_plugin.addOptions(wParam, &odp);
	//TODO: add omemo options
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Account manager UI

class CJabberDlgAccMgrUI : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	CCtrlCombo		m_cbType;
	CCtrlEditJid	m_txtUsername;
	CCtrlEdit		m_txtServer;
	CCtrlEdit		m_txtPassword;
	CCtrlCheck		m_chkSavePassword;
	CCtrlCheck		m_chkUseDomainLogin;
	CCtrlCombo		m_cbResource;
	CCtrlCheck		m_chkManualHost;
	CCtrlEdit		m_txtManualHost;
	CCtrlEdit		m_txtPort;
	CCtrlButton		m_btnRegister;

public:
	CJabberDlgAccMgrUI(CJabberProto *proto, HWND hwndParent) :
		CJabberDlgBase(proto, IDD_ACCMGRUI),
		m_cbType(this, IDC_CB_TYPE),
		m_txtUsername(this, IDC_EDIT_USERNAME),
		m_txtPassword(this, IDC_EDIT_PASSWORD),
		m_chkUseDomainLogin(this, IDC_USEDOMAINLOGIN),
		m_chkSavePassword(this, IDC_SAVEPASSWORD),
		m_cbResource(this, IDC_COMBO_RESOURCE),
		m_txtServer(this, IDC_EDIT_LOGIN_SERVER),
		m_txtPort(this, IDC_PORT),
		m_chkManualHost(this, IDC_MANUAL),
		m_txtManualHost(this, IDC_HOST),
		m_btnRegister(this, IDC_BUTTON_REGISTER)
	{
		SetParent(hwndParent);

		CreateLink(m_txtUsername, "LoginName", L"");
		CreateLink(m_chkSavePassword, proto->m_bSavePassword);
		CreateLink(m_cbResource, "Resource", L"Miranda");
		CreateLink(m_txtServer, "LoginServer", L"jabber.org");
		CreateLink(m_txtPort, "Port", DBVT_WORD, 5222);
		CreateLink(m_chkUseDomainLogin, proto->m_bUseDomainLogin);

		// Bind events
		m_cbType.OnChange = Callback(this, &CJabberDlgAccMgrUI::cbType_OnChange);
		m_chkManualHost.OnChange = Callback(this, &CJabberDlgAccMgrUI::chkManualHost_OnChange);
		m_chkUseDomainLogin.OnChange = Callback(this, &CJabberDlgAccMgrUI::chkUseDomainLogin_OnChange);

		m_btnRegister.OnClick = Callback(this, &CJabberDlgAccMgrUI::btnRegister_OnClick);
	}

protected:
	enum { ACC_PUBLIC, ACC_TLS, ACC_SSL, ACC_HIPCHAT, ACC_LJTALK, ACC_LOL_EN, ACC_LOL_EW, ACC_LOL_OC, ACC_LOL_US, ACC_OK, ACC_SMS };

	bool OnInitDialog() override
	{
		CSuper::OnInitDialog();

		wchar_t *passw = m_proto->getWStringA(0, "Password");
		if (passw) {
			m_txtPassword.SetText(passw);
			mir_free(passw);
		}

		// fill predefined resources
		wchar_t *szResources[] = { L"Home", L"Work", L"Office", L"Miranda" };
		for (auto &it : szResources)
			m_cbResource.AddString(it);

		// append computer name to the resource list
		wchar_t szCompName[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD dwCompNameLength = MAX_COMPUTERNAME_LENGTH;
		if (GetComputerName(szCompName, &dwCompNameLength))
			m_cbResource.AddString(szCompName);

		ptrW tszResource(m_proto->getWStringA("Resource"));
		if (tszResource != nullptr) {
			if (CB_ERR == m_cbResource.FindString(tszResource, -1, true))
				m_cbResource.AddString(tszResource);
			m_cbResource.SetText(tszResource);
		}
		else m_cbResource.SetText(L"Miranda");

		m_cbType.AddString(TranslateT("Public XMPP Network"), ACC_PUBLIC);
		m_cbType.AddString(TranslateT("Secure XMPP Network"), ACC_TLS);
		m_cbType.AddString(TranslateT("Secure XMPP Network (old style)"), ACC_SSL);
		m_cbType.AddString(TranslateT("Hipchat"), ACC_HIPCHAT);
		m_cbType.AddString(TranslateT("LiveJournal Talk"), ACC_LJTALK);
		m_cbType.AddString(TranslateT("League Of Legends (EU Nordic)"), ACC_LOL_EN);
		m_cbType.AddString(TranslateT("League Of Legends (EU West)"), ACC_LOL_EW);
		m_cbType.AddString(TranslateT("League Of Legends (Oceania)"), ACC_LOL_OC);
		m_cbType.AddString(TranslateT("League Of Legends (US)"), ACC_LOL_US);
		m_cbType.AddString(TranslateT("Odnoklassniki"), ACC_OK);
		m_cbType.AddString(TranslateT("S.ms"), ACC_SMS);

		char server[256], manualServer[256] = { 0 };
		m_txtServer.GetTextA(server, _countof(server));
		ptrA dbManualServer(db_get_sa(0, m_proto->m_szModuleName, "ManualHost"));
		if (dbManualServer != nullptr)
			mir_strncpy(manualServer, dbManualServer, _countof(manualServer));

		m_canregister = true;
		if (!mir_strcmp(server, "chat.hipchat.com")) {
			m_cbType.SetCurSel(ACC_HIPCHAT);
			m_canregister = false;
		}
		else if (!mir_strcmp(server, "livejournal.com")) {
			m_cbType.SetCurSel(ACC_LJTALK);
			m_canregister = false;
		}
		else if (!mir_strcmp(server, "chat.eun1.lol.riotgames.com")) {
			m_cbType.SetCurSel(ACC_LOL_EN);
			m_canregister = false;
		}
		else if (!mir_strcmp(server, "chat.euw1.lol.riotgames.com")) {
			m_cbType.SetCurSel(ACC_LOL_EW);
			m_canregister = false;
		}
		else if (!mir_strcmp(server, "chat.oc1.lol.riotgames.com")) {
			m_cbType.SetCurSel(ACC_LOL_OC);
			m_canregister = false;
		}
		else if (!mir_strcmp(server, "chat.na2.lol.riotgames.com")) {
			m_cbType.SetCurSel(ACC_LOL_US);
			m_canregister = false;
		}
		else if (!mir_strcmp(server, "xmpp.odnoklassniki.ru")) {
			m_cbType.SetCurSel(ACC_OK);
			m_canregister = false;
		}
		else if (!mir_strcmp(server, "S.ms")) {
			m_cbType.SetCurSel(ACC_SMS);
			m_canregister = false;
		}
		else if (m_proto->m_bUseSSL)
			m_cbType.SetCurSel(ACC_SSL);
		else if (m_proto->m_bUseTLS) {
			m_cbType.SetCurSel(ACC_TLS);
			m_txtPort.SetInt(5222);
		}
		else m_cbType.SetCurSel(ACC_PUBLIC);

		if (m_chkManualHost.Enabled()) {
			if (m_proto->m_bManualConnect) {
				m_chkManualHost.SetState(BST_CHECKED);
				m_txtManualHost.Enable();
				m_txtPort.Enable();

				ptrW dbManualHost(m_proto->getWStringA("ManualHost"));
				if (dbManualHost != nullptr)
					m_txtManualHost.SetText(dbManualHost);

				m_txtPort.SetInt(m_proto->getWord("ManualPort", m_txtPort.GetInt()));
			}
			else {
				int defPort = m_txtPort.GetInt();
				int port = m_proto->getWord("Port", defPort);

				if (port != defPort) {
					m_chkManualHost.SetState(BST_CHECKED);
					m_txtManualHost.Enable();
					m_txtPort.Enable();

					m_txtManualHost.SetTextA(server);
					m_txtPort.SetInt(port);
				}
				else {
					m_chkManualHost.SetState(BST_UNCHECKED);
					m_txtManualHost.Disable();
					m_txtPort.Disable();
				}
			}
		}

		if (m_proto->m_bUseDomainLogin)
			chkUseDomainLogin_OnChange(&m_chkUseDomainLogin);

		CheckRegistration();
		return true;
	}

	bool OnApply() override
	{
		// clear saved password
		m_proto->m_savedPassword = nullptr;

		bool bUseHostnameAsResource = false;
		wchar_t szCompName[MAX_COMPUTERNAME_LENGTH + 1], szResource[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD dwCompNameLength = MAX_COMPUTERNAME_LENGTH;
		if (GetComputerName(szCompName, &dwCompNameLength)) {
			m_cbResource.GetText(szResource, _countof(szResource));
			if (!mir_wstrcmp(szCompName, szResource))
				bUseHostnameAsResource = true;
		}
		m_proto->m_bHostNameAsResource = bUseHostnameAsResource;

		if (m_chkSavePassword.GetState()) {
			wchar_t *text = m_txtPassword.GetText();
			m_proto->setWString("Password", text);
			mir_free(text);
		}
		else m_proto->delSetting("Password");

		switch (m_cbType.GetItemData(m_cbType.GetCurSel())) {
		case ACC_PUBLIC:
			m_proto->m_bUseSSL = m_proto->m_bUseTLS = false;
			break;

		case ACC_OK:
			m_proto->m_bIgnoreRoster = true;
			m_proto->m_bUseSSL = false;
			m_proto->m_bUseTLS = true;

		case ACC_TLS:
		case ACC_HIPCHAT:
		case ACC_LJTALK:
		case ACC_SMS:
			m_proto->m_bUseSSL = false;
			m_proto->m_bUseTLS = true;
			break;

		case ACC_LOL_EN:
		case ACC_LOL_EW:
		case ACC_LOL_OC:
		case ACC_LOL_US:
			m_proto->setDword("Priority", -2);
			m_proto->m_bUseSSL = true;
			m_proto->m_bUseTLS = false;
			break;

		case ACC_SSL:
			m_proto->m_bUseSSL = true;
			m_proto->m_bUseTLS = false;
			break;
		}

		char server[256];
		char manualServer[256];

		m_txtServer.GetTextA(server, _countof(server));
		m_txtManualHost.GetTextA(manualServer, _countof(manualServer));

		if ((m_chkManualHost.GetState()) && mir_strcmp(server, manualServer)) {
			m_proto->m_bManualConnect = true;
			m_proto->setString("ManualHost", manualServer);
			m_proto->setWord("ManualPort", m_txtPort.GetInt());
			m_proto->setWord("Port", m_txtPort.GetInt());
		}
		else {
			m_proto->m_bManualConnect = false;
			m_proto->delSetting("ManualHost");
			m_proto->delSetting("ManualPort");
			m_proto->setWord("Port", m_txtPort.GetInt());
		}

		sttStoreJidFromUI(m_proto, m_txtUsername, m_txtServer);

		if (m_proto->m_bJabberOnline) {
			if (m_cbType.IsChanged() || m_txtPassword.IsChanged() || m_cbResource.IsChanged() ||
				m_txtServer.IsChanged() || m_txtPort.IsChanged() || m_txtManualHost.IsChanged()) {
				MessageBox(m_hwnd,
					TranslateT("Some changes will take effect the next time you connect to the Jabber network."),
					TranslateT("Jabber Protocol Option"), MB_OK | MB_SETFOREGROUND);
			}

			m_proto->SendPresence(m_proto->m_iStatus, true);
		}
		return true;
	}

	void OnChange() override
	{
		if (m_bInitialized)
			CheckRegistration();
	}

private:
	bool m_canregister;

	void btnRegister_OnClick(CCtrlButton*)
	{
		PSHNOTIFY pshn = {};
		pshn.hdr.code = PSN_APPLY;
		pshn.hdr.hwndFrom = m_hwnd;
		SendMessage(m_hwnd, WM_NOTIFY, 0, (LPARAM)&pshn);

		JABBER_CONN_DATA regInfo;
		m_txtUsername.GetTextU(regInfo.username, _countof(regInfo.username));
		m_txtPassword.GetTextU(regInfo.password, _countof(regInfo.password));
		m_txtServer.GetTextA(regInfo.server, _countof(regInfo.server));
		regInfo.port = (uint16_t)m_txtPort.GetInt();
		if (m_chkManualHost.GetState())
			m_txtManualHost.GetTextA(regInfo.manualHost, _countof(regInfo.manualHost));
		else
			regInfo.manualHost[0] = '\0';

		if (regInfo.username[0] && regInfo.password[0] && regInfo.server[0] && regInfo.port > 0 && (!m_chkManualHost.GetState() || regInfo.manualHost[0]))
			CJabberDlgRegister(m_proto, m_hwnd, &regInfo).DoModal();
	}

	void cbType_OnChange(CCtrlData *sender)
	{
		CCtrlCombo *chk = (CCtrlCombo *)sender;
		setupConnection(chk->GetCurData());
		CheckRegistration();
	}

	void chkUseDomainLogin_OnChange(CCtrlData *sender)
	{
		CCtrlCheck *chk = (CCtrlCheck *)sender;
		bool bChecked = chk->GetState();

		m_txtPassword.Enable(!bChecked);
		m_txtUsername.Enable(!bChecked);
		m_chkSavePassword.Enable(!bChecked);
		if (bChecked) {
			m_txtPassword.SetText(L"");
			m_txtUsername.SetText(L"");
			m_chkSavePassword.SetState(BST_CHECKED);
		}
	}

	void chkManualHost_OnChange(CCtrlData *sender)
	{
		CCtrlCheck *chk = (CCtrlCheck *)sender;

		if (chk->GetState()) {
			char buf[256];
			m_txtServer.GetTextA(buf, _countof(buf));
			m_txtManualHost.SetTextA(buf);
			m_txtPort.SetInt(5222);

			m_txtManualHost.Enable();
			m_txtPort.Enable();
		}
		else {
			m_txtManualHost.Disable();
			m_txtPort.Disable();
		}
	}

	void CheckRegistration()
	{
		if (!m_canregister) {
			m_btnRegister.Disable();
			return;
		}

		JABBER_CONN_DATA regInfo;
		m_txtUsername.GetTextU(regInfo.username, _countof(regInfo.username));
		m_txtPassword.GetTextU(regInfo.password, _countof(regInfo.password));
		m_txtServer.GetTextA(regInfo.server, _countof(regInfo.server));
		regInfo.port = m_txtPort.GetInt();
		if (m_chkManualHost.GetState())
			m_txtManualHost.GetTextA(regInfo.manualHost, _countof(regInfo.manualHost));
		else
			regInfo.manualHost[0] = '\0';

		if (regInfo.username[0] && regInfo.password[0] && regInfo.server[0] && regInfo.port > 0 && (!m_chkManualHost.GetState() || regInfo.manualHost[0]))
			m_btnRegister.Enable();
		else
			m_btnRegister.Disable();
	}

	void setupConnection(int type)
	{
		switch (type) {
		case ACC_PUBLIC: setupPublic(); break;
		case ACC_TLS: setupSecure(); break;
		case ACC_SSL: setupSecureSSL(); break;
		case ACC_HIPCHAT: setupHipchat(); break;
		case ACC_LJTALK: setupLJ(); break;
		case ACC_LOL_EN: setupLOLEN(); break;
		case ACC_LOL_EW: setupLOLEW(); break;
		case ACC_LOL_OC: setupLOLOC(); break;
		case ACC_LOL_US: setupLOLUS(); break;
		case ACC_OK: setupOK(); break;
		case ACC_SMS: setupSMS(); break;
		}
	}

	void setupPublic()
	{
		m_canregister = true;
		m_chkManualHost.SetState(BST_UNCHECKED);
		m_txtManualHost.SetTextA("");
		m_txtPort.SetInt(5222);

		m_txtServer.Enable();
		m_chkManualHost.Enable();
		m_txtManualHost.Disable();
		m_txtPort.Disable();
		m_btnRegister.Enable();
	}

	void setupSecure()
	{
		m_canregister = true;
		m_chkManualHost.SetState(BST_UNCHECKED);
		m_txtManualHost.SetTextA("");
		m_txtPort.SetInt(5222);

		m_txtServer.Enable();
		m_chkManualHost.Enable();
		m_txtManualHost.Disable();
		m_txtPort.Disable();
		m_btnRegister.Enable();
	}

	void setupSecureSSL()
	{
		m_canregister = true;
		m_chkManualHost.SetState(BST_UNCHECKED);
		m_txtManualHost.SetTextA("");
		m_txtPort.SetInt(5223);

		m_txtServer.Enable();
		m_chkManualHost.Enable();
		m_txtManualHost.Disable();
		m_txtPort.Disable();
		m_btnRegister.Enable();
	}

	void setupHipchat()
	{
		m_canregister = false;
		m_txtServer.SetTextA("chat.hipchat.com");
		m_chkManualHost.SetState(BST_UNCHECKED);
		m_txtManualHost.SetTextA("");
		m_txtPort.SetInt(5222);

		m_txtServer.Disable();
		m_chkManualHost.Disable();
		m_txtManualHost.Disable();
		m_txtPort.Disable();
		m_btnRegister.Disable();
	}

	void setupLJ()
	{
		m_canregister = false;
		m_txtServer.SetTextA("livejournal.com");
		m_chkManualHost.SetState(BST_UNCHECKED);
		m_txtManualHost.SetTextA("");
		m_txtPort.SetInt(5222);

		m_txtServer.Disable();
		m_chkManualHost.Disable();
		m_txtManualHost.Disable();
		m_txtPort.Disable();
		m_btnRegister.Disable();
	}

	void setupLOLEN()
	{
		m_canregister = false;
		m_txtServer.SetTextA("pvp.net");
		m_chkManualHost.SetState(BST_UNCHECKED);
		m_txtManualHost.SetTextA("chat.eun1.lol.riotgames.com");
		m_txtPort.SetInt(5223);

		m_txtServer.Disable();
		m_chkManualHost.Disable();
		m_txtManualHost.Disable();
		m_txtPort.Disable();
		m_btnRegister.Disable();
	}

	void setupLOLEW()
	{
		m_canregister = false;
		m_txtServer.SetTextA("pvp.net");
		m_chkManualHost.SetState(BST_UNCHECKED);
		m_txtManualHost.SetTextA("chat.euw1.lol.riotgames.com");
		m_txtPort.SetInt(5223);

		m_txtServer.Disable();
		m_chkManualHost.Disable();
		m_txtManualHost.Disable();
		m_txtPort.Disable();
		m_btnRegister.Disable();
	}

	void setupLOLOC()
	{
		m_canregister = false;
		m_txtServer.SetTextA("pvp.net");
		m_chkManualHost.SetState(BST_UNCHECKED);
		m_txtManualHost.SetTextA("chat.oc1.lol.riotgames.com");
		m_txtPort.SetInt(5223);

		m_txtServer.Disable();
		m_chkManualHost.Disable();
		m_txtManualHost.Disable();
		m_txtPort.Disable();
		m_btnRegister.Disable();
	}

	void setupLOLUS()
	{
		m_canregister = false;
		m_txtServer.SetTextA("pvp.net");
		m_chkManualHost.SetState(BST_UNCHECKED);
		m_txtManualHost.SetTextA("chat.na2.lol.riotgames.com");
		m_txtPort.SetInt(5223);

		m_txtServer.Disable();
		m_chkManualHost.Disable();
		m_txtManualHost.Disable();
		m_txtPort.Disable();
		m_btnRegister.Disable();
	}

	void setupOK()
	{
		m_canregister = false;
		m_txtServer.SetTextA("xmpp.odnoklassniki.ru");
		m_chkManualHost.SetState(BST_UNCHECKED);
		m_txtManualHost.SetTextA("");
		m_txtPort.SetInt(5222);

		m_txtServer.Disable();
		m_chkManualHost.Disable();
		m_txtManualHost.Disable();
		m_txtPort.Disable();
		m_btnRegister.Disable();
	}

	void setupSMS()
	{
		m_canregister = false;
		m_txtServer.SetTextA("S.ms");
		m_chkManualHost.SetState(BST_UNCHECKED);
		m_txtManualHost.SetTextA("");
		m_txtPort.SetInt(5222);

		m_txtServer.Disable();
		m_chkManualHost.Disable();
		m_txtManualHost.Disable();
		m_txtPort.Disable();
		m_btnRegister.Disable();
	}
};

INT_PTR CJabberProto::SvcCreateAccMgrUI(WPARAM, LPARAM lParam)
{
	CJabberDlgAccMgrUI *dlg = new CJabberDlgAccMgrUI(this, (HWND)lParam);
	dlg->Show();
	return (INT_PTR)dlg->GetHwnd();
}

INT_PTR __cdecl CJabberProto::OnMenuOptions(WPARAM, LPARAM)
{
	g_plugin.openOptions(L"Network", m_tszUserName, L"Account");
	return 0;
}
