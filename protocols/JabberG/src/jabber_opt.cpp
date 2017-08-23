/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (ñ) 2012-17 Miranda NG project

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
#include "jabber_opttree.h"

static BOOL(WINAPI *pfnEnableThemeDialogTexture)(HANDLE, DWORD) = 0;

/////////////////////////////////////////////////////////////////////////////////////////
// JabberRegisterDlgProc - the dialog proc for registering new account

#define STR_FORMAT L"%s %s@%S:%d?"

struct { wchar_t *szCode; wchar_t *szDescription; } g_LanguageCodes[] = {
	{ L"aa", LPGENW("Afar") },
	{ L"ab", LPGENW("Abkhazian") },
	{ L"af", LPGENW("Afrikaans") },
	{ L"ak", LPGENW("Akan") },
	{ L"sq", LPGENW("Albanian") },
	{ L"am", LPGENW("Amharic") },
	{ L"ar", LPGENW("Arabic") },
	{ L"an", LPGENW("Aragonese") },
	{ L"hy", LPGENW("Armenian") },
	{ L"as", LPGENW("Assamese") },
	{ L"av", LPGENW("Avaric") },
	{ L"ae", LPGENW("Avestan") },
	{ L"ay", LPGENW("Aymara") },
	{ L"az", LPGENW("Azerbaijani") },
	{ L"ba", LPGENW("Bashkir") },
	{ L"bm", LPGENW("Bambara") },
	{ L"eu", LPGENW("Basque") },
	{ L"be", LPGENW("Belarusian") },
	{ L"bn", LPGENW("Bengali") },
	{ L"bh", LPGENW("Bihari") },
	{ L"bi", LPGENW("Bislama") },
	{ L"bs", LPGENW("Bosnian") },
	{ L"br", LPGENW("Breton") },
	{ L"bg", LPGENW("Bulgarian") },
	{ L"my", LPGENW("Burmese") },
	{ L"ca", LPGENW("Catalan; Valencian") },
	{ L"ch", LPGENW("Chamorro") },
	{ L"ce", LPGENW("Chechen") },
	{ L"zh", LPGENW("Chinese") },
	{ L"cu", LPGENW("Church Slavic; Old Slavonic") },
	{ L"cv", LPGENW("Chuvash") },
	{ L"kw", LPGENW("Cornish") },
	{ L"co", LPGENW("Corsican") },
	{ L"cr", LPGENW("Cree") },
	{ L"cs", LPGENW("Czech") },
	{ L"da", LPGENW("Danish") },
	{ L"dv", LPGENW("Divehi; Dhivehi; Maldivian") },
	{ L"nl", LPGENW("Dutch; Flemish") },
	{ L"dz", LPGENW("Dzongkha") },
	{ L"en", LPGENW("English") },
	{ L"eo", LPGENW("Esperanto") },
	{ L"et", LPGENW("Estonian") },
	{ L"ee", LPGENW("Ewe") },
	{ L"fo", LPGENW("Faroese") },
	{ L"fj", LPGENW("Fijian") },
	{ L"fi", LPGENW("Finnish") },
	{ L"fr", LPGENW("French") },
	{ L"fy", LPGENW("Western Frisian") },
	{ L"ff", LPGENW("Fulah") },
	{ L"ka", LPGENW("Georgian") },
	{ L"de", LPGENW("German") },
	{ L"gd", LPGENW("Gaelic; Scottish Gaelic") },
	{ L"ga", LPGENW("Irish") },
	{ L"gl", LPGENW("Galician") },
	{ L"gv", LPGENW("Manx") },
	{ L"el", LPGENW("Greek, Modern (1453-)") },
	{ L"gn", LPGENW("Guarani") },
	{ L"gu", LPGENW("Gujarati") },
	{ L"ht", LPGENW("Haitian; Haitian Creole") },
	{ L"ha", LPGENW("Hausa") },
	{ L"he", LPGENW("Hebrew") },
	{ L"hz", LPGENW("Herero") },
	{ L"hi", LPGENW("Hindi") },
	{ L"ho", LPGENW("Hiri Motu") },
	{ L"hu", LPGENW("Hungarian") },
	{ L"ig", LPGENW("Igbo") },
	{ L"is", LPGENW("Icelandic") },
	{ L"io", LPGENW("Ido") },
	{ L"ii", LPGENW("Sichuan Yi") },
	{ L"iu", LPGENW("Inuktitut") },
	{ L"ie", LPGENW("Interlingue") },
	{ L"ia", LPGENW("Interlingua (International Auxiliary Language Association)") },
	{ L"id", LPGENW("Indonesian") },
	{ L"ik", LPGENW("Inupiaq") },
	{ L"it", LPGENW("Italian") },
	{ L"jv", LPGENW("Javanese") },
	{ L"ja", LPGENW("Japanese") },
	{ L"kl", LPGENW("Kalaallisut; Greenlandic") },
	{ L"kn", LPGENW("Kannada") },
	{ L"ks", LPGENW("Kashmiri") },
	{ L"kr", LPGENW("Kanuri") },
	{ L"kk", LPGENW("Kazakh") },
	{ L"km", LPGENW("Central Khmer") },
	{ L"ki", LPGENW("Kikuyu; Gikuyu") },
	{ L"rw", LPGENW("Kinyarwanda") },
	{ L"ky", LPGENW("Kirghiz; Kyrgyz") },
	{ L"kv", LPGENW("Komi") },
	{ L"kg", LPGENW("Kongo") },
	{ L"ko", LPGENW("Korean") },
	{ L"kj", LPGENW("Kuanyama; Kwanyama") },
	{ L"ku", LPGENW("Kurdish") },
	{ L"lo", LPGENW("Lao") },
	{ L"la", LPGENW("Latin") },
	{ L"lv", LPGENW("Latvian") },
	{ L"li", LPGENW("Limburgan; Limburger; Limburgish") },
	{ L"ln", LPGENW("Lingala") },
	{ L"lt", LPGENW("Lithuanian") },
	{ L"lb", LPGENW("Luxembourgish; Letzeburgesch") },
	{ L"lu", LPGENW("Luba-Katanga") },
	{ L"lg", LPGENW("Ganda") },
	{ L"mk", LPGENW("Macedonian") },
	{ L"mh", LPGENW("Marshallese") },
	{ L"ml", LPGENW("Malayalam") },
	{ L"mi", LPGENW("Maori") },
	{ L"mr", LPGENW("Marathi") },
	{ L"ms", LPGENW("Malay") },
	{ L"mg", LPGENW("Malagasy") },
	{ L"mt", LPGENW("Maltese") },
	{ L"mo", LPGENW("Moldavian") },
	{ L"mn", LPGENW("Mongolian") },
	{ L"na", LPGENW("Nauru") },
	{ L"nv", LPGENW("Navajo; Navaho") },
	{ L"nr", LPGENW("Ndebele, South; South Ndebele") },
	{ L"nd", LPGENW("Ndebele, North; North Ndebele") },
	{ L"ng", LPGENW("Ndonga") },
	{ L"ne", LPGENW("Nepali") },
	{ L"nn", LPGENW("Norwegian Nynorsk; Nynorsk, Norwegian") },
	{ L"nb", LPGENW("Bokmaal, Norwegian; Norwegian Bokmaal") },
	{ L"no", LPGENW("Norwegian") },
	{ L"ny", LPGENW("Chichewa; Chewa; Nyanja") },
	{ L"oc", LPGENW("Occitan (post 1500); Provencal") },
	{ L"oj", LPGENW("Ojibwa") },
	{ L"or", LPGENW("Oriya") },
	{ L"om", LPGENW("Oromo") },
	{ L"os", LPGENW("Ossetian; Ossetic") },
	{ L"pa", LPGENW("Panjabi; Punjabi") },
	{ L"fa", LPGENW("Persian") },
	{ L"pi", LPGENW("Pali") },
	{ L"pl", LPGENW("Polish") },
	{ L"pt", LPGENW("Portuguese") },
	{ L"ps", LPGENW("Pushto") },
	{ L"qu", LPGENW("Quechua") },
	{ L"rm", LPGENW("Romansh") },
	{ L"ro", LPGENW("Romanian") },
	{ L"rn", LPGENW("Rundi") },
	{ L"ru", LPGENW("Russian") },
	{ L"sg", LPGENW("Sango") },
	{ L"sa", LPGENW("Sanskrit") },
	{ L"sr", LPGENW("Serbian") },
	{ L"hr", LPGENW("Croatian") },
	{ L"si", LPGENW("Sinhala; Sinhalese") },
	{ L"sk", LPGENW("Slovak") },
	{ L"sl", LPGENW("Slovenian") },
	{ L"se", LPGENW("Northern Sami") },
	{ L"sm", LPGENW("Samoan") },
	{ L"sn", LPGENW("Shona") },
	{ L"sd", LPGENW("Sindhi") },
	{ L"so", LPGENW("Somali") },
	{ L"st", LPGENW("Sotho, Southern") },
	{ L"es", LPGENW("Spanish; Castilian") },
	{ L"sc", LPGENW("Sardinian") },
	{ L"ss", LPGENW("Swati") },
	{ L"su", LPGENW("Sundanese") },
	{ L"sw", LPGENW("Swahili") },
	{ L"sv", LPGENW("Swedish") },
	{ L"ty", LPGENW("Tahitian") },
	{ L"ta", LPGENW("Tamil") },
	{ L"tt", LPGENW("Tatar") },
	{ L"te", LPGENW("Telugu") },
	{ L"tg", LPGENW("Tajik") },
	{ L"tl", LPGENW("Tagalog") },
	{ L"th", LPGENW("Thai") },
	{ L"bo", LPGENW("Tibetan") },
	{ L"ti", LPGENW("Tigrinya") },
	{ L"to", LPGENW("Tonga (Tonga Islands)") },
	{ L"tn", LPGENW("Tswana") },
	{ L"ts", LPGENW("Tsonga") },
	{ L"tk", LPGENW("Turkmen") },
	{ L"tr", LPGENW("Turkish") },
	{ L"tw", LPGENW("Twi") },
	{ L"ug", LPGENW("Uighur; Uyghur") },
	{ L"uk", LPGENW("Ukrainian") },
	{ L"ur", LPGENW("Urdu") },
	{ L"uz", LPGENW("Uzbek") },
	{ L"ve", LPGENW("Venda") },
	{ L"vi", LPGENW("Vietnamese") },
	{ L"vo", LPGENW("Volapuk") },
	{ L"cy", LPGENW("Welsh") },
	{ L"wa", LPGENW("Walloon") },
	{ L"wo", LPGENW("Wolof") },
	{ L"xh", LPGENW("Xhosa") },
	{ L"yi", LPGENW("Yiddish") },
	{ L"yo", LPGENW("Yoruba") },
	{ L"za", LPGENW("Zhuang; Chuang") },
	{ L"zu", LPGENW("Zulu") },
	{ nullptr, nullptr }
};

class CJabberDlgRegister : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;
public:
	CJabberDlgRegister(CJabberProto *proto, HWND hwndParent, JABBER_CONN_DATA *regInfo) :
		CJabberDlgBase(proto, IDD_OPT_REGISTER, false),
		m_bProcessStarted(false),
		m_regInfo(regInfo),
		m_btnOk(this, IDOK)
	{
		SetParent(hwndParent);
		m_autoClose = CLOSE_ON_CANCEL;
		m_btnOk.OnClick = Callback(this, &CJabberDlgRegister::btnOk_OnClick);
	}

protected:
	void OnInitDialog()
	{
		wchar_t text[256];
		mir_snwprintf(text, STR_FORMAT, TranslateT("Register"), m_regInfo->username, m_regInfo->server, m_regInfo->port);
		SetDlgItemText(m_hwnd, IDC_REG_STATUS, text);
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg) {
		case WM_JABBER_REGDLG_UPDATE:	// wParam=progress (0-100), lparam=status string
			if ((wchar_t*)lParam == nullptr)
				SetDlgItemText(m_hwnd, IDC_REG_STATUS, TranslateT("No message"));
			else
				SetDlgItemText(m_hwnd, IDC_REG_STATUS, (wchar_t*)lParam);

			SendDlgItemMessage(m_hwnd, IDC_PROGRESS_REG, PBM_SETPOS, wParam, 0);
			if (wParam >= 100)
				m_btnOk.SetText(TranslateT("Close"));
			else
				SetFocus(GetDlgItem(m_hwnd, IDC_PROGRESS_REG));

			return TRUE;
		}

		return CSuper::DlgProc(msg, wParam, lParam);
	}

private:
	bool m_bProcessStarted;
	JABBER_CONN_DATA *m_regInfo;

	CCtrlButton m_btnOk;

	void btnOk_OnClick(CCtrlButton*)
	{
		if (m_bProcessStarted) {
			Close();
			return;
		}

		ShowWindow(GetDlgItem(m_hwnd, IDC_PROGRESS_REG), SW_SHOW);

		m_regInfo->reg_hwndDlg = m_hwnd;
		m_proto->ForkThread((CJabberProto::MyThreadFunc)&CJabberProto::ServerThread, m_regInfo);

		m_btnOk.SetText(TranslateT("Cancel"));
		m_bProcessStarted = true;

		m_lresult = TRUE;
	}
};

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

static void sttStoreJidFromUI(CJabberProto *ppro, CCtrlEdit &txtUsername, CCtrlCombo &cbServer)
{
	ppro->setWString("jid", CMStringW(FORMAT, L"%s@%s", ptrW(txtUsername.GetText()), ptrW(cbServer.GetText())));
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
	CCtrlCombo		m_cbServer;
	CCtrlEdit		m_txtPort;
	CCtrlCheck		m_chkUseSsl;
	CCtrlCheck		m_chkUseTls;
	CCtrlCheck		m_chkManualHost;
	CCtrlEdit		m_txtManualHost;
	CCtrlEdit		m_txtManualPort;
	CCtrlCheck		m_chkKeepAlive;
	CCtrlCheck		m_chkAutoDeleteContacts;
	CCtrlEdit		m_txtUserDirectory;
	CCtrlCombo		m_cbLocale;
	CCtrlButton		m_btnRegister;
	CCtrlButton		m_btnUnregister;
	CCtrlButton		m_btnChangePassword;
	CCtrlHyperlink	m_lnkServers;

public:
	CDlgOptAccount(CJabberProto *proto) :
		CJabberDlgBase(proto, IDD_OPT_JABBER, false),
		m_txtUsername(this, IDC_EDIT_USERNAME),
		m_txtPassword(this, IDC_EDIT_PASSWORD),
		m_txtPriority(this, IDC_PRIORITY),
		m_chkSavePassword(this, IDC_SAVEPASSWORD),
		m_cbResource(this, IDC_COMBO_RESOURCE),
		m_chkUseHostnameAsResource(this, IDC_HOSTNAME_AS_RESOURCE),
		m_chkUseDomainLogin(this, IDC_USEDOMAINLOGIN),
		m_cbServer(this, IDC_EDIT_LOGIN_SERVER),
		m_txtPort(this, IDC_PORT),
		m_chkUseSsl(this, IDC_USE_SSL),
		m_chkUseTls(this, IDC_USE_TLS),
		m_chkManualHost(this, IDC_MANUAL),
		m_txtManualHost(this, IDC_HOST),
		m_txtManualPort(this, IDC_HOSTPORT),
		m_chkKeepAlive(this, IDC_KEEPALIVE),
		m_chkAutoDeleteContacts(this, IDC_ROSTER_SYNC),
		m_txtUserDirectory(this, IDC_JUD),
		m_cbLocale(this, IDC_MSGLANG),
		m_btnRegister(this, IDC_BUTTON_REGISTER),
		m_btnUnregister(this, IDC_UNREGISTER),
		m_btnChangePassword(this, IDC_BUTTON_CHANGE_PASSWORD),
		m_gotservers(false),
		m_lnkServers(this, IDC_LINK_PUBLIC_SERVER, "https://xmpp.net/directory.php")
	{
		CreateLink(m_txtUsername, "LoginName", L"");
		CreateLink(m_txtPriority, "Priority", DBVT_DWORD, 0);
		CreateLink(m_chkSavePassword, proto->m_options.SavePassword);
		CreateLink(m_cbResource, "Resource", L"Miranda");
		CreateLink(m_chkUseHostnameAsResource, proto->m_options.HostNameAsResource);
		CreateLink(m_chkUseDomainLogin, proto->m_options.UseDomainLogin);
		CreateLink(m_cbServer, "LoginServer", L"jabber.org");
		CreateLink(m_txtPort, "Port", DBVT_WORD, 5222);
		CreateLink(m_chkUseSsl, proto->m_options.UseSSL);
		CreateLink(m_chkUseTls, proto->m_options.UseTLS);
		CreateLink(m_chkManualHost, proto->m_options.ManualConnect);
		CreateLink(m_txtManualHost, "ManualHost", L"");
		CreateLink(m_txtManualPort, "ManualPort", DBVT_WORD, 0);
		CreateLink(m_chkKeepAlive, proto->m_options.KeepAlive);
		CreateLink(m_chkAutoDeleteContacts, proto->m_options.RosterSync);
		CreateLink(m_txtUserDirectory, "Jud", L"");

		// Bind events
		m_cbServer.OnDropdown = Callback(this, &CDlgOptAccount::cbServer_OnDropdown);
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
	void OnInitDialog()
	{
		CSuper::OnInitDialog();

		SendDlgItemMessage(m_hwnd, IDC_PRIORITY_SPIN, UDM_SETRANGE, 0, (LPARAM)MAKELONG(127, -128));

		wchar_t *passw = m_proto->getWStringA(0, "Password");
		if (passw) {
			m_txtPassword.SetText(passw);
			mir_free(passw);
		}

		m_cbServer.AddString(TranslateT("Loading..."));

		// fill predefined resources
		wchar_t *szResources[] = { L"Home", L"Work", L"Office", L"Miranda" };
		for (int i = 0; i < _countof(szResources); i++)
			m_cbResource.AddString(szResources[i]);

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
			if (!mir_wstrcmp(m_proto->m_tszSelectedLang, g_LanguageCodes[i].szCode))
				m_cbLocale.SetCurSel(iItem);
		}

		EnableWindow(GetDlgItem(m_hwnd, IDC_COMBO_RESOURCE), m_chkUseHostnameAsResource.GetState() != BST_CHECKED);
		EnableWindow(GetDlgItem(m_hwnd, IDC_UNREGISTER), m_proto->m_bJabberOnline);

		m_chkUseTls.Enable(!m_proto->m_options.Disable3920auth && (m_proto->m_options.UseSSL ? false : true));
		if (m_proto->m_options.Disable3920auth) m_chkUseTls.SetState(BST_UNCHECKED);
		m_chkUseSsl.Enable(m_proto->m_options.Disable3920auth || (m_proto->m_options.UseTLS ? false : true));

		if (m_proto->m_options.ManualConnect) {
			m_txtManualHost.Enable();
			m_txtManualPort.Enable();
			m_txtPort.Disable();
		}

		if (m_proto->m_options.UseDomainLogin)
			chkUseDomainLogin_OnChange(&m_chkUseDomainLogin);

		CheckRegistration();
	}

	void OnApply()
	{
		// clear saved password
		m_proto->m_savedPassword = nullptr;

		if (m_chkSavePassword.GetState() == BST_CHECKED)
			m_proto->setWString("Password", ptrW(m_txtPassword.GetText()));
		else
			m_proto->delSetting("Password");

		int index = m_cbLocale.GetCurSel();
		if (index >= 0) {
			wchar_t *szLanguageCode = (wchar_t *)m_cbLocale.GetItemData(index);
			if (szLanguageCode) {
				m_proto->setWString("XmlLang", szLanguageCode);

				mir_free(m_proto->m_tszSelectedLang);
				m_proto->m_tszSelectedLang = mir_wstrdup(szLanguageCode);
			}
		}

		sttStoreJidFromUI(m_proto, m_txtUsername, m_cbServer);

		if (m_proto->m_bJabberOnline) {
			if (m_txtUsername.IsChanged() || m_txtPassword.IsChanged() || m_cbResource.IsChanged() ||
				m_cbServer.IsChanged() || m_chkUseHostnameAsResource.IsChanged() || m_txtPort.IsChanged() ||
				m_txtManualHost.IsChanged() || m_txtManualPort.IsChanged() || m_cbLocale.IsChanged()) {
				MessageBox(m_hwnd,
					TranslateT("These changes will take effect the next time you connect to the Jabber network."),
					TranslateT("Jabber Protocol Option"), MB_OK | MB_SETFOREGROUND);
			}

			m_proto->SendPresence(m_proto->m_iStatus, true);
		}
	}

	void OnChange(CCtrlBase*)
	{
		if (m_initialized)
			CheckRegistration();
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg) {
		case WM_ACTIVATE:
			m_chkUseTls.Enable(!m_proto->m_options.Disable3920auth && (m_proto->m_options.UseSSL ? false : true));
			if (m_proto->m_options.Disable3920auth) m_chkUseTls.SetState(BST_UNCHECKED);
			break;

		case WM_JABBER_REFRESH:
			RefreshServers((HXML)lParam);
			break;
		}
		return CSuper::DlgProc(msg, wParam, lParam);
	}

private:
	bool m_gotservers;

	void btnRegister_OnClick(CCtrlButton *)
	{
		PSHNOTIFY pshn = { 0 };
		pshn.hdr.code = PSN_APPLY;
		pshn.hdr.hwndFrom = m_hwnd;
		SendMessage(m_hwnd, WM_NOTIFY, 0, (LPARAM)&pshn);

		JABBER_CONN_DATA regInfo;
		m_txtUsername.GetText(regInfo.username, _countof(regInfo.username));
		m_txtPassword.GetText(regInfo.password, _countof(regInfo.password));
		m_cbServer.GetTextA(regInfo.server, _countof(regInfo.server));
		if (m_chkManualHost.GetState() == BST_CHECKED) {
			regInfo.port = (WORD)m_txtManualPort.GetInt();
			m_txtManualHost.GetTextA(regInfo.manualHost, _countof(regInfo.manualHost));
		}
		else {
			regInfo.port = (WORD)m_txtPort.GetInt();
			regInfo.manualHost[0] = '\0';
		}

		if (regInfo.username[0] && regInfo.password[0] && regInfo.server[0] && regInfo.port > 0 && ((m_chkManualHost.GetState() != BST_CHECKED) || regInfo.manualHost[0])) {
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
			XmlNodeIq(L"set", m_proto->SerialNext(), m_proto->m_szJabberJID) << XQUERY(JABBER_FEAT_REGISTER)
			<< XCHILD(L"remove"));
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

	void cbServer_OnDropdown(CCtrlCombo*)
	{
		if (!m_gotservers)
			mir_forkthread(QueryServerListThread, this);
	}

	void chkManualHost_OnChange(CCtrlData *sender)
	{
		CCtrlCheck *chk = (CCtrlCheck *)sender;

		if (chk->GetState() == BST_CHECKED) {
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

		m_cbResource.Enable(chk->GetState() != BST_CHECKED);
		if (chk->GetState() == BST_CHECKED) {
			wchar_t szCompName[MAX_COMPUTERNAME_LENGTH + 1];
			DWORD dwCompNameLength = MAX_COMPUTERNAME_LENGTH;
			if (GetComputerName(szCompName, &dwCompNameLength))
				m_cbResource.SetText(szCompName);
		}
	}

	void chkUseDomainLogin_OnChange(CCtrlData *sender)
	{
		CCtrlCheck *chk = (CCtrlCheck *)sender;
		BOOL checked = chk->GetState() == BST_CHECKED;

		m_txtPassword.Enable(!checked);
		m_txtUsername.Enable(!checked);
		m_chkSavePassword.Enable(!checked);
		if (checked) {
			m_txtPassword.SetText(L"");
			m_txtUsername.SetText(L"");
			m_chkSavePassword.SetState(BST_CHECKED);
		}
	}

	void chkUseSsl_OnChange(CCtrlData*)
	{
		BOOL bManualHost = m_chkManualHost.GetState() == BST_CHECKED;
		if (m_chkUseSsl.GetState() == BST_CHECKED) {
			m_chkUseTls.Disable();
			if (!bManualHost)
				m_txtPort.SetInt(5223);
		}
		else {
			if (!m_proto->m_options.Disable3920auth)
				m_chkUseTls.Enable();
			if (!bManualHost)
				m_txtPort.SetInt(5222);
		}
	}

	void chkUseTls_OnChange(CCtrlData*)
	{
		if (m_chkUseTls.GetState() == BST_CHECKED)
			m_chkUseSsl.Disable();
		else
			m_chkUseSsl.Enable();
	}

	void CheckRegistration()
	{
		JABBER_CONN_DATA regInfo;
		m_txtUsername.GetText(regInfo.username, _countof(regInfo.username));
		m_txtPassword.GetText(regInfo.password, _countof(regInfo.password));
		m_cbServer.GetTextA(regInfo.server, _countof(regInfo.server));
		if (m_chkManualHost.GetState() == BST_CHECKED) {
			regInfo.port = (WORD)m_txtManualPort.GetInt();
			m_txtManualHost.GetTextA(regInfo.manualHost, _countof(regInfo.manualHost));
		}
		else {
			regInfo.port = (WORD)m_txtPort.GetInt();
			regInfo.manualHost[0] = '\0';
		}

		if (regInfo.username[0] && regInfo.password[0] && regInfo.server[0] && regInfo.port > 0 && ((m_chkManualHost.GetState() != BST_CHECKED) || regInfo.manualHost[0]))
			EnableWindow(GetDlgItem(m_hwnd, IDC_BUTTON_REGISTER), TRUE);
		else
			EnableWindow(GetDlgItem(m_hwnd, IDC_BUTTON_REGISTER), FALSE);
	}

	void RefreshServers(HXML node)
	{
		m_gotservers = node != nullptr;

		wchar_t *server = m_cbServer.GetText();
		bool bDropdown = m_cbServer.GetDroppedState();
		if (bDropdown) m_cbServer.ShowDropdown(false);

		m_cbServer.ResetContent();
		if (node) {
			for (int i = 0;; i++) {
				HXML n = XmlGetChild(node, i);
				if (!n)
					break;

				if (!mir_wstrcmp(XmlGetName(n), L"item"))
					if (const wchar_t *jid = XmlGetAttrValue(n, L"jid"))
						if (m_cbServer.FindString(jid, -1, true) == CB_ERR)
							m_cbServer.AddString(jid);
			}
		}

		m_cbServer.SetText(server);

		if (bDropdown) m_cbServer.ShowDropdown();
		mir_free(server);
	}

	static void QueryServerListThread(void *arg)
	{
		Thread_SetName("Jabber: QueryServerListThread");

		CDlgOptAccount *wnd = (CDlgOptAccount *)arg;
		HWND hwnd = wnd->GetHwnd();
		bool bIsError = true;

		if (!IsWindow(hwnd)) return;

		NETLIBHTTPREQUEST request = { 0 };
		request.cbSize = sizeof(request);
		request.requestType = REQUEST_GET;
		request.flags = NLHRF_REDIRECT | NLHRF_HTTP11;
		request.szUrl = JABBER_SERVER_URL;

		NETLIBHTTPREQUEST *result = Netlib_HttpTransaction(wnd->GetProto()->m_hNetlibUser, &request);
		if (result) {
			if (result->resultCode == 200 && result->dataLength && result->pData) {
				wchar_t *buf = mir_a2u(result->pData);
				XmlNode node(buf, nullptr, nullptr);
				if (node) {
					HXML queryNode = XmlGetChild(node, L"query");
					SendMessage(hwnd, WM_JABBER_REFRESH, 0, (LPARAM)queryNode);
					bIsError = false;
				}
				mir_free(buf);
			}
			Netlib_FreeHttpRequest(result);
		}

		if (bIsError)
			SendMessage(hwnd, WM_JABBER_REFRESH, 0, 0);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// JabberAdvOptDlgProc - advanced options dialog procedure

class CDlgOptAdvanced : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	CCtrlCheck		m_chkDirect;
	CCtrlCheck		m_chkDirectManual;
	CCtrlCheck		m_chkProxy;
	CCtrlEdit		m_txtDirect;
	CCtrlEdit		m_txtProxy;
	CCtrlTreeOpts	m_otvOptions;

	BYTE m_oldFrameValue;

public:
	CDlgOptAdvanced(CJabberProto *proto) :
		CJabberDlgBase(proto, IDD_OPT_JABBER2, false),
		m_chkDirect(this, IDC_DIRECT),
		m_chkDirectManual(this, IDC_DIRECT_MANUAL),
		m_chkProxy(this, IDC_PROXY_MANUAL),
		m_txtDirect(this, IDC_DIRECT_ADDR),
		m_txtProxy(this, IDC_PROXY_ADDR),
		m_otvOptions(this, IDC_OPTTREE),
		m_oldFrameValue(proto->m_options.DisableFrame)
	{
		CreateLink(m_chkDirect, proto->m_options.BsDirect);
		CreateLink(m_chkDirectManual, proto->m_options.BsDirectManual);
		CreateLink(m_chkProxy, proto->m_options.BsProxyManual);
		CreateLink(m_txtDirect, "BsDirectAddr", L"");
		CreateLink(m_txtProxy, "BsProxyServer", L"");

		m_chkDirect.OnChange =
			m_chkDirectManual.OnChange = Callback(this, &CDlgOptAdvanced::chkDirect_OnChange);
		m_chkProxy.OnChange = Callback(this, &CDlgOptAdvanced::chkProxy_OnChange);

		m_otvOptions.AddOption(LPGENW("Messaging") L"/" LPGENW("Send messages slower, but with full acknowledgment"), m_proto->m_options.MsgAck);
		m_otvOptions.AddOption(LPGENW("Messaging") L"/" LPGENW("Enable avatars"), m_proto->m_options.EnableAvatars);
		m_otvOptions.AddOption(LPGENW("Messaging") L"/" LPGENW("Log chat state changes"), m_proto->m_options.LogChatstates);
		m_otvOptions.AddOption(LPGENW("Messaging") L"/" LPGENW("Log presence subscription state changes"), m_proto->m_options.LogPresence);
		m_otvOptions.AddOption(LPGENW("Messaging") L"/" LPGENW("Log presence errors"), m_proto->m_options.LogPresenceErrors);
		m_otvOptions.AddOption(LPGENW("Messaging") L"/" LPGENW("Enable user moods receiving"), m_proto->m_options.EnableUserMood);
		m_otvOptions.AddOption(LPGENW("Messaging") L"/" LPGENW("Enable user tunes receiving"), m_proto->m_options.EnableUserTune);
		m_otvOptions.AddOption(LPGENW("Messaging") L"/" LPGENW("Enable user activity receiving"), m_proto->m_options.EnableUserActivity);
		m_otvOptions.AddOption(LPGENW("Messaging") L"/" LPGENW("Receive notes"), m_proto->m_options.AcceptNotes);
		m_otvOptions.AddOption(LPGENW("Messaging") L"/" LPGENW("Automatically save received notes"), m_proto->m_options.AutosaveNotes);
		m_otvOptions.AddOption(LPGENW("Messaging") L"/" LPGENW("Enable server-side history"), m_proto->m_options.EnableMsgArchive);

		m_otvOptions.AddOption(LPGENW("Server options") L"/" LPGENW("Disable SASL authentication (for old servers)"), m_proto->m_options.Disable3920auth);
		m_otvOptions.AddOption(LPGENW("Server options") L"/" LPGENW("Enable stream compression (if possible)"), m_proto->m_options.EnableZlib);

		m_otvOptions.AddOption(LPGENW("Other") L"/" LPGENW("Enable remote controlling (from another resource of same JID only)"), m_proto->m_options.EnableRemoteControl);
		m_otvOptions.AddOption(LPGENW("Other") L"/" LPGENW("Show transport agents on contact list"), m_proto->m_options.ShowTransport);
		m_otvOptions.AddOption(LPGENW("Other") L"/" LPGENW("Automatically add contact when accept authorization"), m_proto->m_options.AutoAdd);
		m_otvOptions.AddOption(LPGENW("Other") L"/" LPGENW("Automatically accept authorization requests"), m_proto->m_options.AutoAcceptAuthorization);
		m_otvOptions.AddOption(LPGENW("Other") L"/" LPGENW("Fix incorrect timestamps in incoming messages"), m_proto->m_options.FixIncorrectTimestamps);
		m_otvOptions.AddOption(LPGENW("Other") L"/" LPGENW("Disable frame"), m_proto->m_options.DisableFrame);
		m_otvOptions.AddOption(LPGENW("Other") L"/" LPGENW("Enable XMPP link processing (requires AssocMgr)"), m_proto->m_options.ProcessXMPPLinks);
		m_otvOptions.AddOption(LPGENW("Other") L"/" LPGENW("Keep contacts assigned to local groups (ignore roster group)"), m_proto->m_options.IgnoreRosterGroups);

		m_otvOptions.AddOption(LPGENW("Security") L"/" LPGENW("Allow servers to request version (XEP-0092)"), m_proto->m_options.AllowVersionRequests);
		m_otvOptions.AddOption(LPGENW("Security") L"/" LPGENW("Show information about operating system in version replies"), m_proto->m_options.ShowOSVersion);
		m_otvOptions.AddOption(LPGENW("Security") L"/" LPGENW("Accept only in band incoming filetransfers (don't disclose own IP)"), m_proto->m_options.BsOnlyIBB);
		m_otvOptions.AddOption(LPGENW("Security") L"/" LPGENW("Accept HTTP Authentication requests (XEP-0070)"), m_proto->m_options.AcceptHttpAuth);
		m_otvOptions.AddOption(LPGENW("Security") L"/" LPGENW("Use OMEMO encryption for messages if possible (Experimental! WIP!)"), m_proto->m_options.UseOMEMO);
	}

	void OnInitDialog()
	{
		CSuper::OnInitDialog();

		chkDirect_OnChange(&m_chkDirect);
		chkProxy_OnChange(&m_chkProxy);
	}

	void OnApply()
	{
		if (m_proto->m_options.DisableFrame != m_oldFrameValue) {
			m_proto->InitInfoFrame(); // create or destroy a frame
			m_oldFrameValue = m_proto->m_options.DisableFrame;
		}

		BOOL bChecked = m_proto->m_options.ShowTransport;
		LISTFOREACH(index, m_proto, LIST_ROSTER)
		{
			JABBER_LIST_ITEM *item = m_proto->ListGetItemPtrFromIndex(index);
			if (item != nullptr) {
				if (wcschr(item->jid, '@') == nullptr) {
					MCONTACT hContact = m_proto->HContactFromJID(item->jid);
					if (hContact != 0) {
						if (bChecked) {
							if (item->getTemp()->m_iStatus != m_proto->getWord(hContact, "Status", ID_STATUS_OFFLINE)) {
								m_proto->setWord(hContact, "Status", (WORD)item->getTemp()->m_iStatus);
							}
						}
						else if (m_proto->getWord(hContact, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
							m_proto->setWord(hContact, "Status", ID_STATUS_OFFLINE);
					}
				}
			}
		}
		if (m_proto->m_options.UseOMEMO)
			m_proto->m_omemo.init();
		else
			m_proto->m_omemo.deinit();
		m_proto->m_clientCapsManager.UpdateFeatHash();
		m_proto->SendPresence(m_proto->m_iStatus, true);
	}

	void chkDirect_OnChange(CCtrlData *)
	{
		if (m_chkDirect.GetState() == BST_CHECKED) {
			if (m_chkDirectManual.GetState() == BST_CHECKED)
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

	void chkProxy_OnChange(CCtrlData *)
	{
		if (m_chkProxy.GetState() == BST_CHECKED)
			m_txtProxy.Enable();
		else
			m_txtProxy.Disable();
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
	CCtrlTreeOpts	m_otvOptions;

public:
	CDlgOptGc(CJabberProto *proto) :
		CJabberDlgBase(proto, IDD_OPT_JABBER4, false),
		m_txtAltNick(this, IDC_TXT_ALTNICK),
		m_txtSlap(this, IDC_TXT_SLAP),
		m_txtQuit(this, IDC_TXT_QUIT),
		m_otvOptions(this, IDC_OPTTREE)
	{
		CreateLink(m_txtAltNick, "GcAltNick", L"");
		CreateLink(m_txtSlap, "GcMsgSlap", TranslateW(JABBER_GC_MSG_SLAP));
		CreateLink(m_txtQuit, "GcMsgQuit", TranslateW(JABBER_GC_MSG_QUIT));

		m_otvOptions.AddOption(LPGENW("General") L"/" LPGENW("Autoaccept multiuser chat invitations"),   m_proto->m_options.AutoAcceptMUC);
		m_otvOptions.AddOption(LPGENW("General") L"/" LPGENW("Automatically join bookmarks on login"),   m_proto->m_options.AutoJoinBookmarks);
		m_otvOptions.AddOption(LPGENW("General") L"/" LPGENW("Automatically join conferences on login"), m_proto->m_options.AutoJoinConferences);
		m_otvOptions.AddOption(LPGENW("General") L"/" LPGENW("Hide conference windows at startup"),      m_proto->m_options.AutoJoinHidden);
		m_otvOptions.AddOption(LPGENW("General") L"/" LPGENW("Do not show multiuser chat invitations"),  m_proto->m_options.IgnoreMUCInvites);
		m_otvOptions.AddOption(LPGENW("Log events") L"/" LPGENW("Ban notifications"),                    m_proto->m_options.GcLogBans);
		m_otvOptions.AddOption(LPGENW("Log events") L"/" LPGENW("Room configuration changes"),           m_proto->m_options.GcLogConfig);
		m_otvOptions.AddOption(LPGENW("Log events") L"/" LPGENW("Affiliation changes"),                  m_proto->m_options.GcLogAffiliations);
		m_otvOptions.AddOption(LPGENW("Log events") L"/" LPGENW("Role changes"),                         m_proto->m_options.GcLogRoles);
		m_otvOptions.AddOption(LPGENW("Log events") L"/" LPGENW("Status changes"),                       m_proto->m_options.GcLogStatuses);
		m_otvOptions.AddOption(LPGENW("Log events") L"/" LPGENW("Don't notify history messages"),        m_proto->m_options.GcLogChatHistory);
	}
};

//////////////////////////////////////////////////////////////////////////
// roster editor
//

#include <io.h>
#define JM_STATUSCHANGED WM_USER+0x0001
#define fopent(name, mode) _wfopen(name, mode)

enum
{
	RRA_FILLLIST = 0,
	RRA_SYNCROSTER,
	RRA_SYNCDONE
};

struct ROSTEREDITDAT
{
	HWND hList;
	int index;
	int subindex;
};

static int	_RosterInsertListItem(HWND hList, const wchar_t * jid, const wchar_t * nick, const wchar_t * group, const wchar_t * subscr, BOOL bChecked)
{
	LVITEM item = { 0 };
	item.mask = LVIF_TEXT | LVIF_STATE;
	item.iItem = ListView_GetItemCount(hList);
	item.pszText = (wchar_t*)jid;

	int index = ListView_InsertItem(hList, &item);
	if (index < 0)
		return index;

	ListView_SetCheckState(hList, index, bChecked);

	ListView_SetItemText(hList, index, 0, (wchar_t*)jid);
	ListView_SetItemText(hList, index, 1, (wchar_t*)nick);
	ListView_SetItemText(hList, index, 2, (wchar_t*)group);
	ListView_SetItemText(hList, index, 3, TranslateW(subscr));
	return index;
}

static void _RosterListClear(HWND hwndDlg)
{
	HWND hList = GetDlgItem(hwndDlg, IDC_ROSTER);
	if (!hList)
		return;

	ListView_DeleteAllItems(hList);
	while (ListView_DeleteColumn(hList, 0));

	LV_COLUMN column = { 0 };
	column.mask = LVCF_TEXT;
	column.cx = 500;

	column.pszText = TranslateT("JID");
	ListView_InsertColumn(hList, 1, &column);

	column.pszText = TranslateT("Nickname");
	ListView_InsertColumn(hList, 2, &column);

	column.pszText = TranslateT("Group");
	ListView_InsertColumn(hList, 3, &column);

	column.pszText = TranslateT("Subscription");
	ListView_InsertColumn(hList, 4, &column);

	RECT rc;
	GetClientRect(hList, &rc);
	int width = rc.right - rc.left;

	ListView_SetColumnWidth(hList, 0, width * 40 / 100);
	ListView_SetColumnWidth(hList, 1, width * 25 / 100);
	ListView_SetColumnWidth(hList, 2, width * 20 / 100);
	ListView_SetColumnWidth(hList, 3, width * 10 / 100);
}

void CJabberProto::_RosterHandleGetRequest(HXML node, CJabberIqInfo*)
{
	HWND hList = GetDlgItem(rrud.hwndDlg, IDC_ROSTER);
	if (rrud.bRRAction == RRA_FILLLIST) {
		_RosterListClear(rrud.hwndDlg);
		HXML query = XmlGetChild(node, "query");
		if (query == nullptr) return;
		int i = 1;
		while (TRUE) {
			HXML item = XmlGetNthChild(query, L"item", i++);
			if (item == nullptr)
				break;

			const wchar_t *jid = XmlGetAttrValue(item, L"jid");
			if (jid == nullptr)
				continue;

			const wchar_t *name = XmlGetAttrValue(item, L"name");
			const wchar_t *subscription = XmlGetAttrValue(item, L"subscription");
			const wchar_t *group = XmlGetText(XmlGetChild(item, "group"));
			_RosterInsertListItem(hList, jid, name, group, subscription, TRUE);
		}

		// now it is require to process whole contact list to add not in roster contacts
		for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
			ptrW tszJid(getWStringA(hContact, "jid"));
			if (tszJid == nullptr)
				continue;

			LVFINDINFO lvfi = { 0 };
			lvfi.flags = LVFI_STRING;
			lvfi.psz = tszJid;
			wchar_t *p = wcschr(tszJid, '@');
			if (p) {
				p = wcschr(tszJid, '/');
				if (p) *p = 0;
			}
			if (ListView_FindItem(hList, -1, &lvfi) == -1) {
				ptrW tszName(db_get_wsa(hContact, "CList", "MyHandle"));
				ptrW tszGroup(db_get_wsa(hContact, "CList", "Group"));
				_RosterInsertListItem(hList, tszJid, tszName, tszGroup, nullptr, FALSE);
			}
		}
		rrud.bReadyToDownload = FALSE;
		rrud.bReadyToUpload = TRUE;
		SetDlgItemText(rrud.hwndDlg, IDC_DOWNLOAD, TranslateT("Download"));
		SetDlgItemText(rrud.hwndDlg, IDC_UPLOAD, TranslateT("Upload"));
		SendMessage(rrud.hwndDlg, JM_STATUSCHANGED, 0, 0);
		return;
	}

	if (rrud.bRRAction == RRA_SYNCROSTER) {
		SetDlgItemText(rrud.hwndDlg, IDC_UPLOAD, TranslateT("Uploading..."));
		HXML queryRoster = XmlGetChild(node, "query");
		if (!queryRoster)
			return;

		XmlNodeIq iq(AddIQ(&CJabberProto::_RosterHandleGetRequest, JABBER_IQ_TYPE_SET));

		HXML query = iq << XCHILDNS(L"query", JABBER_FEAT_IQ_ROSTER);

		int itemCount = 0;
		int ListItemCount = ListView_GetItemCount(hList);
		for (int index = 0; index < ListItemCount; index++) {
			wchar_t jid[JABBER_MAX_JID_LEN] = L"";
			wchar_t name[260] = L"";
			wchar_t group[260] = L"";
			wchar_t subscr[260] = L"";
			ListView_GetItemText(hList, index, 0, jid, _countof(jid));
			ListView_GetItemText(hList, index, 1, name, _countof(name));
			ListView_GetItemText(hList, index, 2, group, _countof(group));
			ListView_GetItemText(hList, index, 3, subscr, _countof(subscr));
			HXML itemRoster = XmlGetChildByTag(queryRoster, "item", "jid", jid);
			BOOL bRemove = !ListView_GetCheckState(hList, index);
			if (itemRoster && bRemove) {
				//delete item
				query << XCHILD(L"item") << XATTR(L"jid", jid) << XATTR(L"subscription", L"remove");
				itemCount++;
			}
			else if (!bRemove) {
				BOOL bPushed = itemRoster ? TRUE : FALSE;
				if (!bPushed) {
					const wchar_t *rosterName = XmlGetAttrValue(itemRoster, L"name");
					if ((rosterName != nullptr || name[0] != 0) && mir_wstrcmpi(rosterName, name))
						bPushed = TRUE;
					if (!bPushed) {
						rosterName = XmlGetAttrValue(itemRoster, L"subscription");
						if ((rosterName != nullptr || subscr[0] != 0) && mir_wstrcmpi(rosterName, subscr))
							bPushed = TRUE;
					}
					if (!bPushed) {
						const wchar_t *rosterGroup = XmlGetText(XmlGetChild(itemRoster, "group"));
						if ((rosterGroup != nullptr || group[0] != 0) && mir_wstrcmpi(rosterGroup, group))
							bPushed = TRUE;
					}
				}
				if (bPushed) {
					HXML item = query << XCHILD(L"item");
					if (mir_wstrlen(group))
						item << XCHILD(L"group", group);
					if (mir_wstrlen(name))
						item << XATTR(L"name", name);
					item << XATTR(L"jid", jid) << XATTR(L"subscription", subscr[0] ? subscr : L"none");
					itemCount++;
				}
			}
		}
		rrud.bRRAction = RRA_SYNCDONE;
		if (itemCount)
			m_ThreadInfo->send(iq);
		else
			_RosterSendRequest(rrud.hwndDlg, RRA_FILLLIST);
	}
	else {
		SetDlgItemText(rrud.hwndDlg, IDC_UPLOAD, TranslateT("Upload"));
		rrud.bReadyToUpload = rrud.bReadyToDownload = FALSE;
		SendMessage(rrud.hwndDlg, JM_STATUSCHANGED, 0, 0);
		SetDlgItemText(rrud.hwndDlg, IDC_DOWNLOAD, TranslateT("Downloading..."));
		_RosterSendRequest(rrud.hwndDlg, RRA_FILLLIST);
	}
}

void CJabberProto::_RosterSendRequest(HWND hwndDlg, BYTE rrAction)
{
	rrud.bRRAction = rrAction;
	rrud.hwndDlg = hwndDlg;

	m_ThreadInfo->send(
		XmlNodeIq(AddIQ(&CJabberProto::_RosterHandleGetRequest, JABBER_IQ_TYPE_GET))
		<< XCHILDNS(L"query", JABBER_FEAT_IQ_ROSTER));
}

static void _RosterItemEditEnd(HWND hEditor, ROSTEREDITDAT * edat, BOOL bCancel)
{
	if (!bCancel) {
		int len = GetWindowTextLength(hEditor) + 1;
		wchar_t *buff = (wchar_t*)mir_alloc(len*sizeof(wchar_t));
		if (buff) {
			GetWindowText(hEditor, buff, len);
			ListView_SetItemText(edat->hList, edat->index, edat->subindex, buff);
		}
		mir_free(buff);
	}
	DestroyWindow(hEditor);
}

static LRESULT CALLBACK _RosterItemNewEditProc(HWND hEditor, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ROSTEREDITDAT * edat = (ROSTEREDITDAT *)GetWindowLongPtr(hEditor, GWLP_USERDATA);
	if (!edat) return 0;
	switch (msg) {
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_RETURN:
			_RosterItemEditEnd(hEditor, edat, FALSE);
			return 0;
		case VK_ESCAPE:
			_RosterItemEditEnd(hEditor, edat, TRUE);
			return 0;
		}
		break;

	case WM_GETDLGCODE:
		if (lParam) {
			MSG *msg2 = (MSG*)lParam;
			if (msg2->message == WM_KEYDOWN && msg2->wParam == VK_TAB) return 0;
			if (msg2->message == WM_CHAR && msg2->wParam == '\t') return 0;
		}
		return DLGC_WANTMESSAGE;

	case WM_KILLFOCUS:
		_RosterItemEditEnd(hEditor, edat, FALSE);
		return 0;

	case WM_DESTROY:
		SetWindowLongPtr(hEditor, GWLP_USERDATA, (LONG_PTR)0);
		free(edat);
		return 0;
	}

	return mir_callNextSubclass(hEditor, _RosterItemNewEditProc, msg, wParam, lParam);
}

void CJabberProto::_RosterExportToFile(HWND hwndDlg)
{
	wchar_t filename[MAX_PATH] = { 0 };

	wchar_t filter[MAX_PATH];
	mir_snwprintf(filter, L"%s (*.xml)%c*.xml%c%c", TranslateT("XML for MS Excel (UTF-8 encoded)"), 0, 0, 0);
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.hwndOwner = hwndDlg;
	ofn.hInstance = nullptr;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = filename;
	ofn.Flags = OFN_HIDEREADONLY;
	ofn.nMaxFile = _countof(filename);
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrDefExt = L"xml";
	if (!GetSaveFileName(&ofn)) return;

	FILE * fp = fopent(filename, L"w");
	if (!fp) return;
	HWND hList = GetDlgItem(hwndDlg, IDC_ROSTER);
	int ListItemCount = ListView_GetItemCount(hList);

	XmlNode root(L"Workbook");
	root << XATTR(L"xmlns", L"urn:schemas-microsoft-com:office:spreadsheet")
	     << XATTR(L"xmlns:o", L"urn:schemas-microsoft-com:office:office")
	     << XATTR(L"xmlns:x", L"urn:schemas-microsoft-com:office:excel")
	     << XATTR(L"xmlns:ss", L"urn:schemas-microsoft-com:office:spreadsheet")
	     << XATTR(L"xmlns:html", L"http://www.w3.org/TR/REC-html40");
	root << XCHILD(L"ExcelWorkbook")
	     << XATTR(L"xmlns", L"urn:schemas-microsoft-com:office:excel");
	HXML table = root << XCHILD(L"Worksheet") << XATTR(L"ss:Name", L"Exported roster")
	                  << XCHILD(L"Table");

	for (int index = 0; index < ListItemCount; index++) {
		wchar_t jid[JABBER_MAX_JID_LEN] = L"";
		wchar_t name[260] = L"";
		wchar_t group[260] = L"";
		wchar_t subscr[260] = L"";
		ListView_GetItemText(hList, index, 0, jid, _countof(jid));
		ListView_GetItemText(hList, index, 1, name, _countof(name));
		ListView_GetItemText(hList, index, 2, group, _countof(group));
		ListView_GetItemText(hList, index, 3, subscr, _countof(subscr));

		HXML node = table << XCHILD(L"Row");
		node << XCHILD(L"Cell") << XCHILD(L"Data", L"+") << XATTR(L"ss:Type", L"String");
		node << XCHILD(L"Cell") << XCHILD(L"Data", jid) << XATTR(L"ss:Type", L"String");
		node << XCHILD(L"Cell") << XCHILD(L"Data", name) << XATTR(L"ss:Type", L"String");
		node << XCHILD(L"Cell") << XCHILD(L"Data", group) << XATTR(L"ss:Type", L"String");
		node << XCHILD(L"Cell") << XCHILD(L"Data", subscr) << XATTR(L"ss:Type", L"String");

	}

	char header[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<?mso-application progid=\"Excel.Sheet\"?>\n";
	fwrite(header, 1, sizeof(header) - 1 /* for zero terminator */, fp);

	wchar_t *xtmp = xmlToString(root, nullptr);
	fputs(T2Utf(xtmp), fp);
	xmlFree(xtmp);
	fclose(fp);
}

void CJabberProto::_RosterImportFromFile(HWND hwndDlg)
{
	char filename[MAX_PATH] = { 0 };
	char *filter = "XML for MS Excel (UTF-8 encoded)(*.xml)\0*.xml\0\0";
	OPENFILENAMEA ofn = { 0 };
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.hwndOwner = hwndDlg;
	ofn.hInstance = nullptr;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = filename;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.nMaxFile = _countof(filename);
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrDefExt = "xml";
	if (!GetOpenFileNameA(&ofn))
		return;

	FILE * fp = fopen(filename, "r");
	if (!fp)
		return;

	DWORD bufsize = _filelength(_fileno(fp));
	if (bufsize <= 0) {
		fclose(fp);
		return;
	}

	char* buffer = (char*)mir_calloc(bufsize + 1); // zero-terminate it
	fread(buffer, 1, bufsize, fp);
	fclose(fp);
	_RosterListClear(hwndDlg);

	wchar_t *newBuf = mir_utf8decodeW(buffer);
	mir_free(buffer);

	int nBytesProcessed = 0;
	XmlNode node(newBuf, &nBytesProcessed, nullptr);
	if (node) {
		HXML Workbook = XmlGetChild(node, L"Workbook");
		if (Workbook) {
			HXML Worksheet = XmlGetChild(Workbook, "Worksheet");
			if (Worksheet) {
				HXML Table = XmlGetChild(Worksheet, "Table");
				if (Table) {
					int index = 1;
					HWND hList = GetDlgItem(hwndDlg, IDC_ROSTER);
					while (TRUE) {
						HXML Row = XmlGetNthChild(Table, L"Row", index++);
						if (!Row)
							break;

						BOOL bAdd = FALSE;
						const wchar_t *jid = nullptr;
						const wchar_t *name = nullptr;
						const wchar_t *group = nullptr;
						const wchar_t *subscr = nullptr;
						HXML Cell = XmlGetNthChild(Row, L"Cell", 1);
						HXML Data = (Cell) ? XmlGetChild(Cell, "Data") : XmlNode();
						if (Data) {
							if (!mir_wstrcmpi(XmlGetText(Data), L"+")) bAdd = TRUE;
							else if (mir_wstrcmpi(XmlGetText(Data), L"-")) continue;

							Cell = XmlGetNthChild(Row, L"Cell", 2);
							if (Cell) Data = XmlGetChild(Cell, "Data");
							else Data = nullptr;
							if (Data) {
								jid = XmlGetText(Data);
								if (!jid || mir_wstrlen(jid) == 0) continue;
							}

							Cell = XmlGetNthChild(Row, L"Cell", 3);
							if (Cell) Data = XmlGetChild(Cell, "Data");
							else Data = nullptr;
							if (Data) name = XmlGetText(Data);

							Cell = XmlGetNthChild(Row, L"Cell", 4);
							if (Cell) Data = XmlGetChild(Cell, "Data");
							else Data = nullptr;
							if (Data) group = XmlGetText(Data);

							Cell = XmlGetNthChild(Row, L"Cell", 5);
							if (Cell) Data = XmlGetChild(Cell, "Data");
							else Data = nullptr;
							if (Data) subscr = XmlGetText(Data);
						}
						_RosterInsertListItem(hList, jid, name, group, subscr, bAdd);
					}
				}
			}
		}
	}

	mir_free(newBuf);
	SendMessage(hwndDlg, JM_STATUSCHANGED, 0, 0);
}

static LRESULT CALLBACK _RosterNewListProc(HWND hList, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_MOUSEWHEEL || msg == WM_NCLBUTTONDOWN || msg == WM_NCRBUTTONDOWN)
		SetFocus(hList);

	if (msg == WM_LBUTTONDOWN) {
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(hList, &pt);

		LVHITTESTINFO lvhti = { 0 };
		lvhti.pt = pt;
		ListView_SubItemHitTest(hList, &lvhti);
		if (lvhti.flags&LVHT_ONITEM && lvhti.iSubItem != 0) {
			RECT rc;
			wchar_t buff[260];
			ListView_GetSubItemRect(hList, lvhti.iItem, lvhti.iSubItem, LVIR_BOUNDS, &rc);
			ListView_GetItemText(hList, lvhti.iItem, lvhti.iSubItem, buff, _countof(buff));
			HWND hEditor = CreateWindow(TEXT("EDIT"), buff, WS_CHILD | ES_AUTOHSCROLL, rc.left + 3, rc.top + 2, rc.right - rc.left - 3, rc.bottom - rc.top - 3, hList, nullptr, hInst, nullptr);
			SendMessage(hEditor, WM_SETFONT, (WPARAM)SendMessage(hList, WM_GETFONT, 0, 0), 0);
			ShowWindow(hEditor, SW_SHOW);
			SetWindowText(hEditor, buff);
			ClientToScreen(hList, &pt);
			ScreenToClient(hEditor, &pt);
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

			ROSTEREDITDAT * edat = (ROSTEREDITDAT *)malloc(sizeof(ROSTEREDITDAT));
			mir_subclassWindow(hEditor, _RosterItemNewEditProc);
			edat->hList = hList;
			edat->index = lvhti.iItem;
			edat->subindex = lvhti.iSubItem;
			SetWindowLongPtr(hEditor, GWLP_USERDATA, (LONG_PTR)edat);
		}
	}
	return mir_callNextSubclass(hList, _RosterNewListProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberRosterOptDlgProc - advanced options dialog procedure

static int sttRosterEditorResizer(HWND /*hwndDlg*/, LPARAM, UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_HEADERBAR:
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORX_WIDTH;
	case IDC_ROSTER:
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORY_HEIGHT | RD_ANCHORX_WIDTH;
	case IDC_DOWNLOAD:
	case IDC_UPLOAD:
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;
	case IDC_EXPORT:
	case IDC_IMPORT:
		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

static INT_PTR CALLBACK JabberRosterOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CJabberProto *ppro = (CJabberProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		ppro = (CJabberProto*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		TranslateDialogDefault(hwndDlg);
		Window_SetIcon_IcoLib(hwndDlg, g_GetIconHandle(IDI_AGENTS));

		Utils_RestoreWindowPosition(hwndDlg, 0, ppro->m_szModuleName, "rosterCtrlWnd_");

		ListView_SetExtendedListViewStyle(GetDlgItem(hwndDlg, IDC_ROSTER), LVS_EX_CHECKBOXES | LVS_EX_BORDERSELECT /*| LVS_EX_FULLROWSELECT*/ | LVS_EX_GRIDLINES /*| LVS_EX_HEADERDRAGDROP*/);
		mir_subclassWindow(GetDlgItem(hwndDlg, IDC_ROSTER), _RosterNewListProc);
		_RosterListClear(hwndDlg);
		ppro->rrud.hwndDlg = hwndDlg;
		ppro->rrud.bReadyToDownload = TRUE;
		ppro->rrud.bReadyToUpload = FALSE;
		SendMessage(hwndDlg, JM_STATUSCHANGED, 0, 0);
		return TRUE;

	case JM_STATUSCHANGED:
		{
			int count = ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_ROSTER));
			EnableWindow(GetDlgItem(hwndDlg, IDC_DOWNLOAD), ppro->m_bJabberOnline);
			EnableWindow(GetDlgItem(hwndDlg, IDC_UPLOAD), count && ppro->m_bJabberOnline);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EXPORT), count > 0);
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		Utils_SaveWindowPosition(hwndDlg, 0, ppro->m_szModuleName, "rosterCtrlWnd_");
		ppro->rrud.hwndDlg = nullptr;
		Window_FreeIcon_IcoLib(hwndDlg);
		break;

	case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
			lpmmi->ptMinTrackSize.x = 550;
			lpmmi->ptMinTrackSize.y = 390;
			return 0;
		}

	case WM_SIZE:
		Utils_ResizeDialog(hwndDlg, hInst, MAKEINTRESOURCEA(IDD_OPT_JABBER3), sttRosterEditorResizer);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_DOWNLOAD:
			ppro->rrud.bReadyToUpload = FALSE;
			ppro->rrud.bReadyToDownload = FALSE;
			SendMessage(ppro->rrud.hwndDlg, JM_STATUSCHANGED, 0, 0);
			SetDlgItemText(ppro->rrud.hwndDlg, IDC_DOWNLOAD, TranslateT("Downloading..."));
			ppro->_RosterSendRequest(hwndDlg, RRA_FILLLIST);
			break;

		case IDC_UPLOAD:
			ppro->rrud.bReadyToUpload = FALSE;
			SendMessage(ppro->rrud.hwndDlg, JM_STATUSCHANGED, 0, 0);
			SetDlgItemText(ppro->rrud.hwndDlg, IDC_UPLOAD, TranslateT("Connecting..."));
			ppro->_RosterSendRequest(hwndDlg, RRA_SYNCROSTER);
			break;

		case IDC_EXPORT:
			ppro->_RosterExportToFile(hwndDlg);
			break;

		case IDC_IMPORT:
			ppro->_RosterImportFromFile(hwndDlg);
			break;
		}
		break;
	}
	return FALSE;
}

INT_PTR __cdecl CJabberProto::OnMenuHandleRosterControl(WPARAM, LPARAM)
{
	if (rrud.hwndDlg && IsWindow(rrud.hwndDlg))
		SetForegroundWindow(rrud.hwndDlg);
	else
		CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_OPT_JABBER3), nullptr, JabberRosterOptDlgProc, (LPARAM)this);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberOptInit - initializes all options dialogs

int CJabberProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.szGroup.w = LPGENW("Network");
	odp.szTitle.w = m_tszUserName;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE | ODPF_DONTTRANSLATE;

	odp.szTab.w = LPGENW("Account");
	odp.pDialog = new CDlgOptAccount(this);
	Options_AddPage(wParam, &odp);

	odp.szTab.w = LPGENW("Conferences");
	odp.pDialog = new CDlgOptGc(this);
	Options_AddPage(wParam, &odp);

	odp.szTab.w = LPGENW("Advanced");
	odp.pDialog = new CDlgOptAdvanced(this);
	Options_AddPage(wParam, &odp);
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
	CCtrlCombo		m_cbServer;
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
		CJabberDlgBase(proto, IDD_ACCMGRUI, false),
		m_cbType(this, IDC_CB_TYPE),
		m_txtUsername(this, IDC_EDIT_USERNAME),
		m_txtPassword(this, IDC_EDIT_PASSWORD),
		m_chkUseDomainLogin(this, IDC_USEDOMAINLOGIN),
		m_chkSavePassword(this, IDC_SAVEPASSWORD),
		m_cbResource(this, IDC_COMBO_RESOURCE),
		m_cbServer(this, IDC_EDIT_LOGIN_SERVER),
		m_txtPort(this, IDC_PORT),
		m_chkManualHost(this, IDC_MANUAL),
		m_txtManualHost(this, IDC_HOST),
		m_btnRegister(this, IDC_BUTTON_REGISTER)
	{
		SetParent(hwndParent);

		CreateLink(m_txtUsername, "LoginName", L"");
		CreateLink(m_chkSavePassword, proto->m_options.SavePassword);
		CreateLink(m_cbResource, "Resource", L"Miranda");
		CreateLink(m_cbServer, "LoginServer", L"jabber.org");
		CreateLink(m_txtPort, "Port", DBVT_WORD, 5222);
		CreateLink(m_chkUseDomainLogin, proto->m_options.UseDomainLogin);

		// Bind events
		m_cbType.OnChange = Callback(this, &CJabberDlgAccMgrUI::cbType_OnChange);
		m_cbServer.OnDropdown = Callback(this, &CJabberDlgAccMgrUI::cbServer_OnDropdown);
		m_chkManualHost.OnChange = Callback(this, &CJabberDlgAccMgrUI::chkManualHost_OnChange);
		m_chkUseDomainLogin.OnChange = Callback(this, &CJabberDlgAccMgrUI::chkUseDomainLogin_OnChange);

		m_btnRegister.OnClick = Callback(this, &CJabberDlgAccMgrUI::btnRegister_OnClick);
	}

protected:
	enum { ACC_PUBLIC, ACC_TLS, ACC_SSL, ACC_GTALK, ACC_LJTALK, ACC_LOL_EN, ACC_LOL_EW, ACC_LOL_OC, ACC_LOL_US, ACC_OK, ACC_SMS };

	void OnInitDialog()
	{
		CSuper::OnInitDialog();

		m_gotservers = false;

		wchar_t *passw = m_proto->getWStringA(0, "Password");
		if (passw) {
			m_txtPassword.SetText(passw);
			mir_free(passw);
		}

		m_cbServer.AddString(TranslateT("Loading..."));

		// fill predefined resources
		wchar_t *szResources[] = { L"Home", L"Work", L"Office", L"Miranda" };
		for (int i = 0; i < _countof(szResources); i++)
			m_cbResource.AddString(szResources[i]);

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
		m_cbType.AddString(TranslateT("Google Talk!"), ACC_GTALK);
		m_cbType.AddString(TranslateT("LiveJournal Talk"), ACC_LJTALK);
		m_cbType.AddString(TranslateT("League Of Legends (EU Nordic)"), ACC_LOL_EN);
		m_cbType.AddString(TranslateT("League Of Legends (EU West)"), ACC_LOL_EW);
		m_cbType.AddString(TranslateT("League Of Legends (Oceania)"), ACC_LOL_OC);
		m_cbType.AddString(TranslateT("League Of Legends (US)"), ACC_LOL_US);
		m_cbType.AddString(TranslateT("Odnoklassniki"), ACC_OK);
		m_cbType.AddString(TranslateT("S.ms"), ACC_SMS);

		char server[256], manualServer[256] = { 0 };
		m_cbServer.GetTextA(server, _countof(server));
		ptrA dbManualServer(db_get_sa(0, m_proto->m_szModuleName, "ManualHost"));
		if (dbManualServer != nullptr)
			mir_strncpy(manualServer, dbManualServer, _countof(manualServer));

		m_canregister = true;
		if (!mir_strcmp(manualServer, "talk.google.com")) {
			m_cbType.SetCurSel(ACC_GTALK);
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
		else if (m_proto->m_options.UseSSL)
			m_cbType.SetCurSel(ACC_SSL);
		else if (m_proto->m_options.UseTLS) {
			m_cbType.SetCurSel(ACC_TLS);
			m_txtPort.SetInt(5222);
		}
		else m_cbType.SetCurSel(ACC_PUBLIC);

		if (m_chkManualHost.Enabled()) {
			if (m_proto->m_options.ManualConnect) {
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

		if (m_proto->m_options.UseDomainLogin)
			chkUseDomainLogin_OnChange(&m_chkUseDomainLogin);

		CheckRegistration();
	}

	void OnApply()
	{
		// clear saved password
		m_proto->m_savedPassword = nullptr;

		BOOL bUseHostnameAsResource = FALSE;
		wchar_t szCompName[MAX_COMPUTERNAME_LENGTH + 1], szResource[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD dwCompNameLength = MAX_COMPUTERNAME_LENGTH;
		if (GetComputerName(szCompName, &dwCompNameLength)) {
			m_cbResource.GetText(szResource, _countof(szResource));
			if (!mir_wstrcmp(szCompName, szResource))
				bUseHostnameAsResource = TRUE;
		}
		m_proto->m_options.HostNameAsResource = bUseHostnameAsResource;

		if (m_chkSavePassword.GetState() == BST_CHECKED) {
			wchar_t *text = m_txtPassword.GetText();
			m_proto->setWString("Password", text);
			mir_free(text);
		}
		else m_proto->delSetting("Password");

		switch (m_cbType.GetItemData(m_cbType.GetCurSel())) {
		case ACC_PUBLIC:
			m_proto->m_options.UseSSL = m_proto->m_options.UseTLS = FALSE;
			break;

		case ACC_GTALK:
			m_proto->setDword("Priority", 24);
			{
				int port = m_txtPort.GetInt();
				if (port == 443 || port == 5223) {
					m_proto->m_options.UseSSL = TRUE;
					m_proto->m_options.UseTLS = FALSE;
				}
				else if (port == 5222) {
					m_proto->m_options.UseSSL = FALSE;
					m_proto->m_options.UseTLS = TRUE;
				}
			}
			break;

		case ACC_OK:
			m_proto->m_options.IgnoreRosterGroups = TRUE;
			m_proto->m_options.UseSSL = FALSE;
			m_proto->m_options.UseTLS = TRUE;

		case ACC_TLS:
		case ACC_LJTALK:
		case ACC_SMS:
			m_proto->m_options.UseSSL = FALSE;
			m_proto->m_options.UseTLS = TRUE;
			break;

		case ACC_LOL_EN:
		case ACC_LOL_EW:
		case ACC_LOL_OC:
		case ACC_LOL_US:
			m_proto->setDword("Priority", -2);
			m_proto->m_options.UseSSL = TRUE;
			m_proto->m_options.UseTLS = FALSE;
			break;

		case ACC_SSL:
			m_proto->m_options.UseSSL = TRUE;
			m_proto->m_options.UseTLS = FALSE;
			break;
		}

		char server[256];
		char manualServer[256];

		m_cbServer.GetTextA(server, _countof(server));
		m_txtManualHost.GetTextA(manualServer, _countof(manualServer));

		if ((m_chkManualHost.GetState() == BST_CHECKED) && mir_strcmp(server, manualServer)) {
			m_proto->m_options.ManualConnect = TRUE;
			m_proto->setString("ManualHost", manualServer);
			m_proto->setWord("ManualPort", m_txtPort.GetInt());
			m_proto->setWord("Port", m_txtPort.GetInt());
		}
		else {
			m_proto->m_options.ManualConnect = FALSE;
			m_proto->delSetting("ManualHost");
			m_proto->delSetting("ManualPort");
			m_proto->setWord("Port", m_txtPort.GetInt());
		}

		sttStoreJidFromUI(m_proto, m_txtUsername, m_cbServer);

		if (m_proto->m_bJabberOnline) {
			if (m_cbType.IsChanged() || m_txtPassword.IsChanged() || m_cbResource.IsChanged() ||
				m_cbServer.IsChanged() || m_txtPort.IsChanged() || m_txtManualHost.IsChanged()) {
				MessageBox(m_hwnd,
					TranslateT("Some changes will take effect the next time you connect to the Jabber network."),
					TranslateT("Jabber Protocol Option"), MB_OK | MB_SETFOREGROUND);
			}

			m_proto->SendPresence(m_proto->m_iStatus, true);
		}
	}

	void OnChange(CCtrlBase*)
	{
		if (m_initialized)
			CheckRegistration();
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg) {
		case WM_JABBER_REFRESH:
			RefreshServers((HXML)lParam);
			break;
		}
		return CSuper::DlgProc(msg, wParam, lParam);
	}

private:
	bool m_gotservers;
	bool m_canregister;

	void btnRegister_OnClick(CCtrlButton *)
	{
		PSHNOTIFY pshn = { 0 };
		pshn.hdr.code = PSN_APPLY;
		pshn.hdr.hwndFrom = m_hwnd;
		SendMessage(m_hwnd, WM_NOTIFY, 0, (LPARAM)&pshn);

		JABBER_CONN_DATA regInfo;
		m_txtUsername.GetText(regInfo.username, _countof(regInfo.username));
		m_txtPassword.GetText(regInfo.password, _countof(regInfo.password));
		m_cbServer.GetTextA(regInfo.server, _countof(regInfo.server));
		regInfo.port = (WORD)m_txtPort.GetInt();
		if (m_chkManualHost.GetState() == BST_CHECKED)
			m_txtManualHost.GetTextA(regInfo.manualHost, _countof(regInfo.manualHost));
		else
			regInfo.manualHost[0] = '\0';

		if (regInfo.username[0] && regInfo.password[0] && regInfo.server[0] && regInfo.port > 0 && ((m_chkManualHost.GetState() != BST_CHECKED) || regInfo.manualHost[0]))
			CJabberDlgRegister(m_proto, m_hwnd, &regInfo).DoModal();
	}

	void cbServer_OnDropdown(CCtrlCombo*)
	{
		if (!m_gotservers)
			mir_forkthread(QueryServerListThread, this);
	}

	void cbType_OnChange(CCtrlData *sender)
	{
		CCtrlCombo *chk = (CCtrlCombo *)sender;
		setupConnection(chk->GetItemData(chk->GetCurSel()));
		CheckRegistration();
	}

	void chkUseDomainLogin_OnChange(CCtrlData *sender)
	{
		CCtrlCheck *chk = (CCtrlCheck *)sender;
		BOOL checked = chk->GetState() == BST_CHECKED;

		m_txtPassword.Enable(!checked);
		m_txtUsername.Enable(!checked);
		m_chkSavePassword.Enable(!checked);
		if (checked) {
			m_txtPassword.SetText(L"");
			m_txtUsername.SetText(L"");
			m_chkSavePassword.SetState(BST_CHECKED);
		}
	}

	void chkManualHost_OnChange(CCtrlData *sender)
	{
		CCtrlCheck *chk = (CCtrlCheck *)sender;

		if (chk->GetState() == BST_CHECKED) {
			char buf[256];
			m_cbServer.GetTextA(buf, _countof(buf));
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

	void CheckRegistration();
	void setupConnection(int type);
	void setupPublic();
	void setupSecure();
	void setupSecureSSL();
	void setupGoogle();
	void setupLJ();
	void setupLOLEN();
	void setupLOLEW();
	void setupLOLOC();
	void setupLOLUS();
	void setupOK();
	void setupSMS();
	void RefreshServers(HXML node);
	static void QueryServerListThread(void *arg);
};

void CJabberDlgAccMgrUI::CheckRegistration()
{
	if (!m_canregister) {
		m_btnRegister.Disable();
		return;
	}

	JABBER_CONN_DATA regInfo;
	m_txtUsername.GetText(regInfo.username, _countof(regInfo.username));
	m_txtPassword.GetText(regInfo.password, _countof(regInfo.password));
	m_cbServer.GetTextA(regInfo.server, _countof(regInfo.server));
	regInfo.port = m_txtPort.GetInt();
	if (m_chkManualHost.GetState() == BST_CHECKED)
		m_txtManualHost.GetTextA(regInfo.manualHost, _countof(regInfo.manualHost));
	else
		regInfo.manualHost[0] = '\0';

	if (regInfo.username[0] && regInfo.password[0] && regInfo.server[0] && regInfo.port > 0 && ((m_chkManualHost.GetState() != BST_CHECKED) || regInfo.manualHost[0]))
		m_btnRegister.Enable();
	else
		m_btnRegister.Disable();
}

void CJabberDlgAccMgrUI::setupConnection(int type)
{
	switch (type) {
	case ACC_PUBLIC: setupPublic(); break;
	case ACC_TLS: setupSecure(); break;
	case ACC_SSL: setupSecureSSL(); break;
	case ACC_GTALK: setupGoogle(); break;
	case ACC_LJTALK: setupLJ(); break;
	case ACC_LOL_EN: setupLOLEN(); break;
	case ACC_LOL_EW: setupLOLEW(); break;
	case ACC_LOL_OC: setupLOLOC(); break;
	case ACC_LOL_US: setupLOLUS(); break;
	case ACC_OK: setupOK(); break;
	case ACC_SMS: setupSMS(); break;
	}
}

void CJabberDlgAccMgrUI::setupPublic()
{
	m_canregister = true;
	m_gotservers = false;
	m_chkManualHost.SetState(BST_UNCHECKED);
	m_txtManualHost.SetTextA("");
	m_txtPort.SetInt(5222);

	m_cbServer.Enable();
	m_chkManualHost.Enable();
	m_txtManualHost.Disable();
	m_txtPort.Disable();
	m_btnRegister.Enable();
}

void CJabberDlgAccMgrUI::setupSecure()
{
	m_canregister = true;
	m_gotservers = false;
	m_chkManualHost.SetState(BST_UNCHECKED);
	m_txtManualHost.SetTextA("");
	m_txtPort.SetInt(5222);

	m_cbServer.Enable();
	m_chkManualHost.Enable();
	m_txtManualHost.Disable();
	m_txtPort.Disable();
	m_btnRegister.Enable();
}

void CJabberDlgAccMgrUI::setupSecureSSL()
{
	m_canregister = true;
	m_gotservers = false;
	m_chkManualHost.SetState(BST_UNCHECKED);
	m_txtManualHost.SetTextA("");
	m_txtPort.SetInt(5223);

	m_cbServer.Enable();
	m_chkManualHost.Enable();
	m_txtManualHost.Disable();
	m_txtPort.Disable();
	m_btnRegister.Enable();
}

void CJabberDlgAccMgrUI::setupGoogle()
{
	m_canregister = false;
	m_gotservers = true;
	m_cbServer.ResetContent();
	m_cbServer.AddStringA("gmail.com");
	m_cbServer.AddStringA("googlemail.com");
	m_cbServer.SetTextA("gmail.com");
	m_chkManualHost.SetState(BST_CHECKED);
	m_txtManualHost.SetTextA("talk.google.com");
	m_txtPort.SetInt(443);

	m_cbServer.Enable();
	m_chkManualHost.Disable();
	m_txtManualHost.Disable();
	//m_txtPort.Disable();
	m_btnRegister.Disable();
}

void CJabberDlgAccMgrUI::setupLJ()
{
	m_canregister = false;
	m_gotservers = true;
	m_cbServer.ResetContent();
	m_cbServer.SetTextA("livejournal.com");
	m_cbServer.AddStringA("livejournal.com");
	m_chkManualHost.SetState(BST_UNCHECKED);
	m_txtManualHost.SetTextA("");
	m_txtPort.SetInt(5222);

	m_cbServer.Disable();
	m_chkManualHost.Disable();
	m_txtManualHost.Disable();
	m_txtPort.Disable();
	m_btnRegister.Disable();
}

void CJabberDlgAccMgrUI::setupLOLEN()
{
	m_canregister = false;
	m_gotservers = true;
	m_cbServer.ResetContent();
	m_cbServer.SetTextA("pvp.net");
	m_cbServer.AddStringA("pvp.net");
	m_chkManualHost.SetState(BST_UNCHECKED);
	m_txtManualHost.SetTextA("chat.eun1.lol.riotgames.com");
	m_txtPort.SetInt(5223);

	m_cbServer.Disable();
	m_chkManualHost.Disable();
	m_txtManualHost.Disable();
	m_txtPort.Disable();
	m_btnRegister.Disable();
}

void CJabberDlgAccMgrUI::setupLOLEW()
{
	m_canregister = false;
	m_gotservers = true;
	m_cbServer.ResetContent();
	m_cbServer.SetTextA("pvp.net");
	m_cbServer.AddStringA("pvp.net");
	m_chkManualHost.SetState(BST_UNCHECKED);
	m_txtManualHost.SetTextA("chat.euw1.lol.riotgames.com");
	m_txtPort.SetInt(5223);

	m_cbServer.Disable();
	m_chkManualHost.Disable();
	m_txtManualHost.Disable();
	m_txtPort.Disable();
	m_btnRegister.Disable();
}

void CJabberDlgAccMgrUI::setupLOLOC()
{
	m_canregister = false;
	m_gotservers = true;
	m_cbServer.ResetContent();
	m_cbServer.SetTextA("pvp.net");
	m_cbServer.AddStringA("pvp.net");
	m_chkManualHost.SetState(BST_UNCHECKED);
	m_txtManualHost.SetTextA("chat.oc1.lol.riotgames.com");
	m_txtPort.SetInt(5223);

	m_cbServer.Disable();
	m_chkManualHost.Disable();
	m_txtManualHost.Disable();
	m_txtPort.Disable();
	m_btnRegister.Disable();
}

void CJabberDlgAccMgrUI::setupLOLUS()
{
	m_canregister = false;
	m_gotservers = true;
	m_cbServer.ResetContent();
	m_cbServer.SetTextA("pvp.net");
	m_cbServer.AddStringA("pvp.net");
	m_chkManualHost.SetState(BST_UNCHECKED);
	m_txtManualHost.SetTextA("chat.na2.lol.riotgames.com");
	m_txtPort.SetInt(5223);

	m_cbServer.Disable();
	m_chkManualHost.Disable();
	m_txtManualHost.Disable();
	m_txtPort.Disable();
	m_btnRegister.Disable();
}

void CJabberDlgAccMgrUI::setupOK()
{
	m_canregister = false;
	m_gotservers = true;
	m_cbServer.ResetContent();
	m_cbServer.SetTextA("xmpp.odnoklassniki.ru");
	m_cbServer.AddStringA("xmpp.odnoklassniki.ru");
	m_chkManualHost.SetState(BST_UNCHECKED);
	m_txtManualHost.SetTextA("");
	m_txtPort.SetInt(5222);

	m_cbServer.Disable();
	m_chkManualHost.Disable();
	m_txtManualHost.Disable();
	m_txtPort.Disable();
	m_btnRegister.Disable();
}

void CJabberDlgAccMgrUI::setupSMS()
{
	m_canregister = false;
	m_gotservers = true;
	m_cbServer.ResetContent();
	m_cbServer.SetTextA("S.ms");
	m_cbServer.AddStringA("S.ms");
	m_chkManualHost.SetState(BST_UNCHECKED);
	m_txtManualHost.SetTextA("");
	m_txtPort.SetInt(5222);

	m_cbServer.Disable();
	m_chkManualHost.Disable();
	m_txtManualHost.Disable();
	m_txtPort.Disable();
	m_btnRegister.Disable();
}

void CJabberDlgAccMgrUI::RefreshServers(HXML node)
{
	m_gotservers = node != nullptr;

	wchar_t *server = m_cbServer.GetText();
	bool bDropdown = m_cbServer.GetDroppedState();
	if (bDropdown) m_cbServer.ShowDropdown(false);

	m_cbServer.ResetContent();
	if (node) {
		for (int i = 0;; i++) {
			HXML n = XmlGetChild(node, i);
			if (!n)
				break;

			if (!mir_wstrcmp(XmlGetName(n), L"item"))
				if (const wchar_t *jid = XmlGetAttrValue(n, L"jid"))
					if (m_cbServer.FindString(jid, -1, true) == CB_ERR)
						m_cbServer.AddString(jid);
		}
	}

	m_cbServer.SetText(server);

	if (bDropdown) m_cbServer.ShowDropdown();
	mir_free(server);
}

void CJabberDlgAccMgrUI::QueryServerListThread(void *arg)
{
	CDlgOptAccount *wnd = (CDlgOptAccount *)arg;
	HWND hwnd = wnd->GetHwnd();
	bool bIsError = true;

	NETLIBHTTPREQUEST request = { 0 };
	request.cbSize = sizeof(request);
	request.requestType = REQUEST_GET;
	request.flags = NLHRF_HTTP11;
	request.szUrl = JABBER_SERVER_URL;

	NETLIBHTTPREQUEST *result = Netlib_HttpTransaction(wnd->GetProto()->m_hNetlibUser, &request);
	if (result && IsWindow(hwnd)) {
		if ((result->resultCode == 200) && result->dataLength && result->pData) {
			wchar_t *ptszText = mir_a2u(result->pData);
			XmlNode node(ptszText, nullptr, nullptr);
			if (node) {
				HXML queryNode = XmlGetChild(node, L"query");
				if (queryNode && IsWindow(hwnd)) {
					SendMessage(hwnd, WM_JABBER_REFRESH, 0, (LPARAM)queryNode);
					bIsError = false;
				}
			}
			mir_free(ptszText);
		}
	}

	if (result)
		Netlib_FreeHttpRequest(result);
	if (bIsError)
		SendMessage(hwnd, WM_JABBER_REFRESH, 0, 0);
}

INT_PTR CJabberProto::SvcCreateAccMgrUI(WPARAM, LPARAM lParam)
{
	CJabberDlgAccMgrUI *dlg = new CJabberDlgAccMgrUI(this, (HWND)lParam);
	dlg->Show();
	return (INT_PTR)dlg->GetHwnd();
}

void CJabberProto::JabberUpdateDialogs(BOOL)
{
	if (rrud.hwndDlg)
		SendMessage(rrud.hwndDlg, JM_STATUSCHANGED, 0, 0);
}

INT_PTR __cdecl CJabberProto::OnMenuOptions(WPARAM, LPARAM)
{
	Options_Open(L"Network", m_tszUserName, L"Account");
	return 0;
}
