/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (ñ) 2012-16 Miranda NG project

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

struct { TCHAR *szCode; TCHAR *szDescription; } g_LanguageCodes[] = {
	{ L"aa", LPGENT("Afar") },
	{ L"ab", LPGENT("Abkhazian") },
	{ L"af", LPGENT("Afrikaans") },
	{ L"ak", LPGENT("Akan") },
	{ L"sq", LPGENT("Albanian") },
	{ L"am", LPGENT("Amharic") },
	{ L"ar", LPGENT("Arabic") },
	{ L"an", LPGENT("Aragonese") },
	{ L"hy", LPGENT("Armenian") },
	{ L"as", LPGENT("Assamese") },
	{ L"av", LPGENT("Avaric") },
	{ L"ae", LPGENT("Avestan") },
	{ L"ay", LPGENT("Aymara") },
	{ L"az", LPGENT("Azerbaijani") },
	{ L"ba", LPGENT("Bashkir") },
	{ L"bm", LPGENT("Bambara") },
	{ L"eu", LPGENT("Basque") },
	{ L"be", LPGENT("Belarusian") },
	{ L"bn", LPGENT("Bengali") },
	{ L"bh", LPGENT("Bihari") },
	{ L"bi", LPGENT("Bislama") },
	{ L"bs", LPGENT("Bosnian") },
	{ L"br", LPGENT("Breton") },
	{ L"bg", LPGENT("Bulgarian") },
	{ L"my", LPGENT("Burmese") },
	{ L"ca", LPGENT("Catalan; Valencian") },
	{ L"ch", LPGENT("Chamorro") },
	{ L"ce", LPGENT("Chechen") },
	{ L"zh", LPGENT("Chinese") },
	{ L"cu", LPGENT("Church Slavic; Old Slavonic") },
	{ L"cv", LPGENT("Chuvash") },
	{ L"kw", LPGENT("Cornish") },
	{ L"co", LPGENT("Corsican") },
	{ L"cr", LPGENT("Cree") },
	{ L"cs", LPGENT("Czech") },
	{ L"da", LPGENT("Danish") },
	{ L"dv", LPGENT("Divehi; Dhivehi; Maldivian") },
	{ L"nl", LPGENT("Dutch; Flemish") },
	{ L"dz", LPGENT("Dzongkha") },
	{ L"en", LPGENT("English") },
	{ L"eo", LPGENT("Esperanto") },
	{ L"et", LPGENT("Estonian") },
	{ L"ee", LPGENT("Ewe") },
	{ L"fo", LPGENT("Faroese") },
	{ L"fj", LPGENT("Fijian") },
	{ L"fi", LPGENT("Finnish") },
	{ L"fr", LPGENT("French") },
	{ L"fy", LPGENT("Western Frisian") },
	{ L"ff", LPGENT("Fulah") },
	{ L"ka", LPGENT("Georgian") },
	{ L"de", LPGENT("German") },
	{ L"gd", LPGENT("Gaelic; Scottish Gaelic") },
	{ L"ga", LPGENT("Irish") },
	{ L"gl", LPGENT("Galician") },
	{ L"gv", LPGENT("Manx") },
	{ L"el", LPGENT("Greek, Modern (1453-)") },
	{ L"gn", LPGENT("Guarani") },
	{ L"gu", LPGENT("Gujarati") },
	{ L"ht", LPGENT("Haitian; Haitian Creole") },
	{ L"ha", LPGENT("Hausa") },
	{ L"he", LPGENT("Hebrew") },
	{ L"hz", LPGENT("Herero") },
	{ L"hi", LPGENT("Hindi") },
	{ L"ho", LPGENT("Hiri Motu") },
	{ L"hu", LPGENT("Hungarian") },
	{ L"ig", LPGENT("Igbo") },
	{ L"is", LPGENT("Icelandic") },
	{ L"io", LPGENT("Ido") },
	{ L"ii", LPGENT("Sichuan Yi") },
	{ L"iu", LPGENT("Inuktitut") },
	{ L"ie", LPGENT("Interlingue") },
	{ L"ia", LPGENT("Interlingua (International Auxiliary Language Association)") },
	{ L"id", LPGENT("Indonesian") },
	{ L"ik", LPGENT("Inupiaq") },
	{ L"it", LPGENT("Italian") },
	{ L"jv", LPGENT("Javanese") },
	{ L"ja", LPGENT("Japanese") },
	{ L"kl", LPGENT("Kalaallisut; Greenlandic") },
	{ L"kn", LPGENT("Kannada") },
	{ L"ks", LPGENT("Kashmiri") },
	{ L"kr", LPGENT("Kanuri") },
	{ L"kk", LPGENT("Kazakh") },
	{ L"km", LPGENT("Central Khmer") },
	{ L"ki", LPGENT("Kikuyu; Gikuyu") },
	{ L"rw", LPGENT("Kinyarwanda") },
	{ L"ky", LPGENT("Kirghiz; Kyrgyz") },
	{ L"kv", LPGENT("Komi") },
	{ L"kg", LPGENT("Kongo") },
	{ L"ko", LPGENT("Korean") },
	{ L"kj", LPGENT("Kuanyama; Kwanyama") },
	{ L"ku", LPGENT("Kurdish") },
	{ L"lo", LPGENT("Lao") },
	{ L"la", LPGENT("Latin") },
	{ L"lv", LPGENT("Latvian") },
	{ L"li", LPGENT("Limburgan; Limburger; Limburgish") },
	{ L"ln", LPGENT("Lingala") },
	{ L"lt", LPGENT("Lithuanian") },
	{ L"lb", LPGENT("Luxembourgish; Letzeburgesch") },
	{ L"lu", LPGENT("Luba-Katanga") },
	{ L"lg", LPGENT("Ganda") },
	{ L"mk", LPGENT("Macedonian") },
	{ L"mh", LPGENT("Marshallese") },
	{ L"ml", LPGENT("Malayalam") },
	{ L"mi", LPGENT("Maori") },
	{ L"mr", LPGENT("Marathi") },
	{ L"ms", LPGENT("Malay") },
	{ L"mg", LPGENT("Malagasy") },
	{ L"mt", LPGENT("Maltese") },
	{ L"mo", LPGENT("Moldavian") },
	{ L"mn", LPGENT("Mongolian") },
	{ L"na", LPGENT("Nauru") },
	{ L"nv", LPGENT("Navajo; Navaho") },
	{ L"nr", LPGENT("Ndebele, South; South Ndebele") },
	{ L"nd", LPGENT("Ndebele, North; North Ndebele") },
	{ L"ng", LPGENT("Ndonga") },
	{ L"ne", LPGENT("Nepali") },
	{ L"nn", LPGENT("Norwegian Nynorsk; Nynorsk, Norwegian") },
	{ L"nb", LPGENT("Bokmaal, Norwegian; Norwegian Bokmaal") },
	{ L"no", LPGENT("Norwegian") },
	{ L"ny", LPGENT("Chichewa; Chewa; Nyanja") },
	{ L"oc", LPGENT("Occitan (post 1500); Provencal") },
	{ L"oj", LPGENT("Ojibwa") },
	{ L"or", LPGENT("Oriya") },
	{ L"om", LPGENT("Oromo") },
	{ L"os", LPGENT("Ossetian; Ossetic") },
	{ L"pa", LPGENT("Panjabi; Punjabi") },
	{ L"fa", LPGENT("Persian") },
	{ L"pi", LPGENT("Pali") },
	{ L"pl", LPGENT("Polish") },
	{ L"pt", LPGENT("Portuguese") },
	{ L"ps", LPGENT("Pushto") },
	{ L"qu", LPGENT("Quechua") },
	{ L"rm", LPGENT("Romansh") },
	{ L"ro", LPGENT("Romanian") },
	{ L"rn", LPGENT("Rundi") },
	{ L"ru", LPGENT("Russian") },
	{ L"sg", LPGENT("Sango") },
	{ L"sa", LPGENT("Sanskrit") },
	{ L"sr", LPGENT("Serbian") },
	{ L"hr", LPGENT("Croatian") },
	{ L"si", LPGENT("Sinhala; Sinhalese") },
	{ L"sk", LPGENT("Slovak") },
	{ L"sl", LPGENT("Slovenian") },
	{ L"se", LPGENT("Northern Sami") },
	{ L"sm", LPGENT("Samoan") },
	{ L"sn", LPGENT("Shona") },
	{ L"sd", LPGENT("Sindhi") },
	{ L"so", LPGENT("Somali") },
	{ L"st", LPGENT("Sotho, Southern") },
	{ L"es", LPGENT("Spanish; Castilian") },
	{ L"sc", LPGENT("Sardinian") },
	{ L"ss", LPGENT("Swati") },
	{ L"su", LPGENT("Sundanese") },
	{ L"sw", LPGENT("Swahili") },
	{ L"sv", LPGENT("Swedish") },
	{ L"ty", LPGENT("Tahitian") },
	{ L"ta", LPGENT("Tamil") },
	{ L"tt", LPGENT("Tatar") },
	{ L"te", LPGENT("Telugu") },
	{ L"tg", LPGENT("Tajik") },
	{ L"tl", LPGENT("Tagalog") },
	{ L"th", LPGENT("Thai") },
	{ L"bo", LPGENT("Tibetan") },
	{ L"ti", LPGENT("Tigrinya") },
	{ L"to", LPGENT("Tonga (Tonga Islands)") },
	{ L"tn", LPGENT("Tswana") },
	{ L"ts", LPGENT("Tsonga") },
	{ L"tk", LPGENT("Turkmen") },
	{ L"tr", LPGENT("Turkish") },
	{ L"tw", LPGENT("Twi") },
	{ L"ug", LPGENT("Uighur; Uyghur") },
	{ L"uk", LPGENT("Ukrainian") },
	{ L"ur", LPGENT("Urdu") },
	{ L"uz", LPGENT("Uzbek") },
	{ L"ve", LPGENT("Venda") },
	{ L"vi", LPGENT("Vietnamese") },
	{ L"vo", LPGENT("Volapuk") },
	{ L"cy", LPGENT("Welsh") },
	{ L"wa", LPGENT("Walloon") },
	{ L"wo", LPGENT("Wolof") },
	{ L"xh", LPGENT("Xhosa") },
	{ L"yi", LPGENT("Yiddish") },
	{ L"yo", LPGENT("Yoruba") },
	{ L"za", LPGENT("Zhuang; Chuang") },
	{ L"zu", LPGENT("Zulu") },
	{ NULL, NULL }
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
		TCHAR text[256];
		mir_sntprintf(text, STR_FORMAT, TranslateT("Register"), m_regInfo->username, m_regInfo->server, m_regInfo->port);
		SetDlgItemText(m_hwnd, IDC_REG_STATUS, text);
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg) {
		case WM_JABBER_REGDLG_UPDATE:	// wParam=progress (0-100), lparam=status string
			if ((TCHAR*)lParam == NULL)
				SetDlgItemText(m_hwnd, IDC_REG_STATUS, TranslateT("No message"));
			else
				SetDlgItemText(m_hwnd, IDC_REG_STATUS, (TCHAR*)lParam);

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

	void btnOk_OnClick(CCtrlButton *)
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
	CCtrlEditJid(CDlgBase* dlg, int ctrlId);

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
	ppro->setTString("jid", CMString(FORMAT, L"%s@%s", ptrT(txtUsername.GetText()), ptrT(cbServer.GetText())));
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

		TCHAR *passw = m_proto->getTStringA(NULL, "Password");
		if (passw) {
			m_txtPassword.SetText(passw);
			mir_free(passw);
		}

		m_cbServer.AddString(TranslateT("Loading..."));

		// fill predefined resources
		TCHAR *szResources[] = { L"Home", L"Work", L"Office", L"Miranda" };
		for (int i = 0; i < _countof(szResources); i++)
			m_cbResource.AddString(szResources[i]);

		// append computer name to the resource list
		TCHAR szCompName[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD dwCompNameLength = MAX_COMPUTERNAME_LENGTH;
		if (GetComputerName(szCompName, &dwCompNameLength))
			m_cbResource.AddString(szCompName);

		ptrT tszResource(m_proto->getTStringA("Resource"));
		if (tszResource != NULL) {
			if (CB_ERR == m_cbResource.FindString(tszResource, -1, true))
				m_cbResource.AddString(tszResource);
			m_cbResource.SetText(tszResource);
		}
		else m_cbResource.SetText(L"Miranda");

		for (int i = 0; g_LanguageCodes[i].szCode; i++) {
			int iItem = m_cbLocale.AddString(TranslateTS(g_LanguageCodes[i].szDescription), (LPARAM)g_LanguageCodes[i].szCode);
			if (!mir_tstrcmp(m_proto->m_tszSelectedLang, g_LanguageCodes[i].szCode))
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
		m_proto->m_savedPassword = NULL;

		if (m_chkSavePassword.GetState() == BST_CHECKED)
			m_proto->setTString("Password", ptrT(m_txtPassword.GetText()));
		else
			m_proto->delSetting("Password");

		int index = m_cbLocale.GetCurSel();
		if (index >= 0) {
			TCHAR *szLanguageCode = (TCHAR *)m_cbLocale.GetItemData(index);
			if (szLanguageCode) {
				m_proto->setTString("XmlLang", szLanguageCode);

				mir_free(m_proto->m_tszSelectedLang);
				m_proto->m_tszSelectedLang = mir_tstrdup(szLanguageCode);
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
		int res = MessageBox(NULL,
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
			MessageBox(NULL,
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
			TCHAR szCompName[MAX_COMPUTERNAME_LENGTH + 1];
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
		m_gotservers = node != NULL;

		TCHAR *server = m_cbServer.GetText();
		bool bDropdown = m_cbServer.GetDroppedState();
		if (bDropdown) m_cbServer.ShowDropdown(false);

		m_cbServer.ResetContent();
		if (node) {
			for (int i = 0;; i++) {
				HXML n = XmlGetChild(node, i);
				if (!n)
					break;

				if (!mir_tstrcmp(XmlGetName(n), L"item"))
					if (const TCHAR *jid = XmlGetAttrValue(n, L"jid"))
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

		NETLIBHTTPREQUEST *result = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)wnd->GetProto()->m_hNetlibUser, (LPARAM)&request);
		if (result) {
			if (result->resultCode == 200 && result->dataLength && result->pData) {
				TCHAR *buf = mir_a2t(result->pData);
				XmlNode node(buf, NULL, NULL);
				if (node) {
					HXML queryNode = XmlGetChild(node, L"query");
					SendMessage(hwnd, WM_JABBER_REFRESH, 0, (LPARAM)queryNode);
					bIsError = false;
				}
				mir_free(buf);
			}
			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)result);
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

		m_otvOptions.AddOption(LPGENT("Messaging") L"/" LPGENT("Send messages slower, but with full acknowledgment"), m_proto->m_options.MsgAck);
		m_otvOptions.AddOption(LPGENT("Messaging") L"/" LPGENT("Enable avatars"), m_proto->m_options.EnableAvatars);
		m_otvOptions.AddOption(LPGENT("Messaging") L"/" LPGENT("Log chat state changes"), m_proto->m_options.LogChatstates);
		m_otvOptions.AddOption(LPGENT("Messaging") L"/" LPGENT("Log presence subscription state changes"), m_proto->m_options.LogPresence);
		m_otvOptions.AddOption(LPGENT("Messaging") L"/" LPGENT("Log presence errors"), m_proto->m_options.LogPresenceErrors);
		m_otvOptions.AddOption(LPGENT("Messaging") L"/" LPGENT("Enable user moods receiving"), m_proto->m_options.EnableUserMood);
		m_otvOptions.AddOption(LPGENT("Messaging") L"/" LPGENT("Enable user tunes receiving"), m_proto->m_options.EnableUserTune);
		m_otvOptions.AddOption(LPGENT("Messaging") L"/" LPGENT("Enable user activity receiving"), m_proto->m_options.EnableUserActivity);
		m_otvOptions.AddOption(LPGENT("Messaging") L"/" LPGENT("Receive notes"), m_proto->m_options.AcceptNotes);
		m_otvOptions.AddOption(LPGENT("Messaging") L"/" LPGENT("Automatically save received notes"), m_proto->m_options.AutosaveNotes);
		m_otvOptions.AddOption(LPGENT("Messaging") L"/" LPGENT("Enable server-side history"), m_proto->m_options.EnableMsgArchive);

		m_otvOptions.AddOption(LPGENT("Server options") L"/" LPGENT("Disable SASL authentication (for old servers)"), m_proto->m_options.Disable3920auth);
		m_otvOptions.AddOption(LPGENT("Server options") L"/" LPGENT("Enable stream compression (if possible)"), m_proto->m_options.EnableZlib);

		m_otvOptions.AddOption(LPGENT("Other") L"/" LPGENT("Enable remote controlling (from another resource of same JID only)"), m_proto->m_options.EnableRemoteControl);
		m_otvOptions.AddOption(LPGENT("Other") L"/" LPGENT("Show transport agents on contact list"), m_proto->m_options.ShowTransport);
		m_otvOptions.AddOption(LPGENT("Other") L"/" LPGENT("Automatically add contact when accept authorization"), m_proto->m_options.AutoAdd);
		m_otvOptions.AddOption(LPGENT("Other") L"/" LPGENT("Automatically accept authorization requests"), m_proto->m_options.AutoAcceptAuthorization);
		m_otvOptions.AddOption(LPGENT("Other") L"/" LPGENT("Fix incorrect timestamps in incoming messages"), m_proto->m_options.FixIncorrectTimestamps);
		m_otvOptions.AddOption(LPGENT("Other") L"/" LPGENT("Disable frame"), m_proto->m_options.DisableFrame);
		m_otvOptions.AddOption(LPGENT("Other") L"/" LPGENT("Enable XMPP link processing (requires AssocMgr)"), m_proto->m_options.ProcessXMPPLinks);
		m_otvOptions.AddOption(LPGENT("Other") L"/" LPGENT("Keep contacts assigned to local groups (ignore roster group)"), m_proto->m_options.IgnoreRosterGroups);

		m_otvOptions.AddOption(LPGENT("Security") L"/" LPGENT("Allow servers to request version (XEP-0092)"), m_proto->m_options.AllowVersionRequests);
		m_otvOptions.AddOption(LPGENT("Security") L"/" LPGENT("Show information about operating system in version replies"), m_proto->m_options.ShowOSVersion);
		m_otvOptions.AddOption(LPGENT("Security") L"/" LPGENT("Accept only in band incoming filetransfers (don't disclose own IP)"), m_proto->m_options.BsOnlyIBB);
		m_otvOptions.AddOption(LPGENT("Security") L"/" LPGENT("Accept HTTP Authentication requests (XEP-0070)"), m_proto->m_options.AcceptHttpAuth);
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
			if (item != NULL) {
				if (_tcschr(item->jid, '@') == NULL) {
					MCONTACT hContact = m_proto->HContactFromJID(item->jid);
					if (hContact != NULL) {
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
		CreateLink(m_txtSlap, "GcMsgSlap", TranslateTS(JABBER_GC_MSG_SLAP));
		CreateLink(m_txtQuit, "GcMsgQuit", TranslateTS(JABBER_GC_MSG_QUIT));

		m_otvOptions.AddOption(LPGENT("General") L"/" LPGENT("Autoaccept multiuser chat invitations"),   m_proto->m_options.AutoAcceptMUC);
		m_otvOptions.AddOption(LPGENT("General") L"/" LPGENT("Automatically join bookmarks on login"),   m_proto->m_options.AutoJoinBookmarks);
		m_otvOptions.AddOption(LPGENT("General") L"/" LPGENT("Automatically join conferences on login"), m_proto->m_options.AutoJoinConferences);
		m_otvOptions.AddOption(LPGENT("General") L"/" LPGENT("Hide conference windows at startup"),      m_proto->m_options.AutoJoinHidden);
		m_otvOptions.AddOption(LPGENT("General") L"/" LPGENT("Do not show multiuser chat invitations"),  m_proto->m_options.IgnoreMUCInvites);
		m_otvOptions.AddOption(LPGENT("Log events") L"/" LPGENT("Ban notifications"),                    m_proto->m_options.GcLogBans);
		m_otvOptions.AddOption(LPGENT("Log events") L"/" LPGENT("Room configuration changes"),           m_proto->m_options.GcLogConfig);
		m_otvOptions.AddOption(LPGENT("Log events") L"/" LPGENT("Affiliation changes"),                  m_proto->m_options.GcLogAffiliations);
		m_otvOptions.AddOption(LPGENT("Log events") L"/" LPGENT("Role changes"),                         m_proto->m_options.GcLogRoles);
		m_otvOptions.AddOption(LPGENT("Log events") L"/" LPGENT("Status changes"),                       m_proto->m_options.GcLogStatuses);
		m_otvOptions.AddOption(LPGENT("Log events") L"/" LPGENT("Don't notify history messages"),        m_proto->m_options.GcLogChatHistory);
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

static int	_RosterInsertListItem(HWND hList, const TCHAR * jid, const TCHAR * nick, const TCHAR * group, const TCHAR * subscr, BOOL bChecked)
{
	LVITEM item = { 0 };
	item.mask = LVIF_TEXT | LVIF_STATE;
	item.iItem = ListView_GetItemCount(hList);
	item.pszText = (TCHAR*)jid;

	int index = ListView_InsertItem(hList, &item);
	if (index < 0)
		return index;

	ListView_SetCheckState(hList, index, bChecked);

	ListView_SetItemText(hList, index, 0, (TCHAR*)jid);
	ListView_SetItemText(hList, index, 1, (TCHAR*)nick);
	ListView_SetItemText(hList, index, 2, (TCHAR*)group);
	ListView_SetItemText(hList, index, 3, TranslateTS(subscr));
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
		if (query == NULL) return;
		int i = 1;
		while (TRUE) {
			HXML item = XmlGetNthChild(query, L"item", i++);
			if (item == NULL)
				break;

			const TCHAR *jid = XmlGetAttrValue(item, L"jid");
			if (jid == NULL)
				continue;

			const TCHAR *name = XmlGetAttrValue(item, L"name");
			const TCHAR *subscription = XmlGetAttrValue(item, L"subscription");
			const TCHAR *group = XmlGetText(XmlGetChild(item, "group"));
			_RosterInsertListItem(hList, jid, name, group, subscription, TRUE);
		}

		// now it is require to process whole contact list to add not in roster contacts
		for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
			ptrT tszJid(getTStringA(hContact, "jid"));
			if (tszJid == NULL)
				continue;

			LVFINDINFO lvfi = { 0 };
			lvfi.flags = LVFI_STRING;
			lvfi.psz = tszJid;
			TCHAR *p = _tcschr(tszJid, _T('@'));
			if (p) {
				p = _tcschr(tszJid, _T('/'));
				if (p) *p = 0;
			}
			if (ListView_FindItem(hList, -1, &lvfi) == -1) {
				ptrT tszName(db_get_tsa(hContact, "CList", "MyHandle"));
				ptrT tszGroup(db_get_tsa(hContact, "CList", "Group"));
				_RosterInsertListItem(hList, tszJid, tszName, tszGroup, NULL, FALSE);
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
			TCHAR jid[JABBER_MAX_JID_LEN] = L"";
			TCHAR name[260] = L"";
			TCHAR group[260] = L"";
			TCHAR subscr[260] = L"";
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
					const TCHAR *rosterName = XmlGetAttrValue(itemRoster, L"name");
					if ((rosterName != NULL || name[0] != 0) && mir_tstrcmpi(rosterName, name))
						bPushed = TRUE;
					if (!bPushed) {
						rosterName = XmlGetAttrValue(itemRoster, L"subscription");
						if ((rosterName != NULL || subscr[0] != 0) && mir_tstrcmpi(rosterName, subscr))
							bPushed = TRUE;
					}
					if (!bPushed) {
						const TCHAR *rosterGroup = XmlGetText(XmlGetChild(itemRoster, "group"));
						if ((rosterGroup != NULL || group[0] != 0) && mir_tstrcmpi(rosterGroup, group))
							bPushed = TRUE;
					}
				}
				if (bPushed) {
					HXML item = query << XCHILD(L"item");
					if (mir_tstrlen(group))
						item << XCHILD(L"group", group);
					if (mir_tstrlen(name))
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
		TCHAR *buff = (TCHAR*)mir_alloc(len*sizeof(TCHAR));
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
	TCHAR filename[MAX_PATH] = { 0 };

	TCHAR filter[MAX_PATH];
	mir_sntprintf(filter, L"%s (*.xml)%c*.xml%c%c", TranslateT("XML for MS Excel (UTF-8 encoded)"), 0, 0, 0);
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.hwndOwner = hwndDlg;
	ofn.hInstance = NULL;
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
		TCHAR jid[JABBER_MAX_JID_LEN] = L"";
		TCHAR name[260] = L"";
		TCHAR group[260] = L"";
		TCHAR subscr[260] = L"";
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

	TCHAR *xtmp = xmlToString(root, NULL);
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
	ofn.hInstance = NULL;
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

	TCHAR *newBuf = mir_utf8decodeT(buffer);
	mir_free(buffer);

	int nBytesProcessed = 0;
	XmlNode node(newBuf, &nBytesProcessed, NULL);
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
						const TCHAR *jid = NULL;
						const TCHAR *name = NULL;
						const TCHAR *group = NULL;
						const TCHAR *subscr = NULL;
						HXML Cell = XmlGetNthChild(Row, L"Cell", 1);
						HXML Data = (Cell) ? XmlGetChild(Cell, "Data") : XmlNode();
						if (Data) {
							if (!mir_tstrcmpi(XmlGetText(Data), L"+")) bAdd = TRUE;
							else if (mir_tstrcmpi(XmlGetText(Data), L"-")) continue;

							Cell = XmlGetNthChild(Row, L"Cell", 2);
							if (Cell) Data = XmlGetChild(Cell, "Data");
							else Data = NULL;
							if (Data) {
								jid = XmlGetText(Data);
								if (!jid || mir_tstrlen(jid) == 0) continue;
							}

							Cell = XmlGetNthChild(Row, L"Cell", 3);
							if (Cell) Data = XmlGetChild(Cell, "Data");
							else Data = NULL;
							if (Data) name = XmlGetText(Data);

							Cell = XmlGetNthChild(Row, L"Cell", 4);
							if (Cell) Data = XmlGetChild(Cell, "Data");
							else Data = NULL;
							if (Data) group = XmlGetText(Data);

							Cell = XmlGetNthChild(Row, L"Cell", 5);
							if (Cell) Data = XmlGetChild(Cell, "Data");
							else Data = NULL;
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
			TCHAR buff[260];
			ListView_GetSubItemRect(hList, lvhti.iItem, lvhti.iSubItem, LVIR_BOUNDS, &rc);
			ListView_GetItemText(hList, lvhti.iItem, lvhti.iSubItem, buff, _countof(buff));
			HWND hEditor = CreateWindow(TEXT("EDIT"), buff, WS_CHILD | ES_AUTOHSCROLL, rc.left + 3, rc.top + 2, rc.right - rc.left - 3, rc.bottom - rc.top - 3, hList, NULL, hInst, NULL);
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

		Utils_RestoreWindowPosition(hwndDlg, NULL, ppro->m_szModuleName, "rosterCtrlWnd_");

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
		Utils_SaveWindowPosition(hwndDlg, NULL, ppro->m_szModuleName, "rosterCtrlWnd_");
		ppro->rrud.hwndDlg = NULL;
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
		CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_OPT_JABBER3), NULL, JabberRosterOptDlgProc, (LPARAM)this);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberOptInit - initializes all options dialogs

int CJabberProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.ptszGroup = LPGENT("Network");
	odp.ptszTitle = m_tszUserName;
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_DONTTRANSLATE;

	odp.ptszTab = LPGENT("Account");
	odp.pDialog = new CDlgOptAccount(this);
	Options_AddPage(wParam, &odp);

	odp.ptszTab = LPGENT("Conferences");
	odp.pDialog = new CDlgOptGc(this);
	Options_AddPage(wParam, &odp);

	odp.ptszTab = LPGENT("Advanced");
	odp.pDialog = new CDlgOptAdvanced(this);
	Options_AddPage(wParam, &odp);
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
	enum { ACC_PUBLIC, ACC_TLS, ACC_SSL, ACC_GTALK, ACC_LJTALK, ACC_LOL_EN, ACC_LOL_EW, ACC_LOL_OC, ACC_LOL_US, ACC_OK, ACC_SMS, ACC_YANDEX };

	void OnInitDialog()
	{
		CSuper::OnInitDialog();

		m_gotservers = false;

		TCHAR *passw = m_proto->getTStringA(NULL, "Password");
		if (passw) {
			m_txtPassword.SetText(passw);
			mir_free(passw);
		}

		m_cbServer.AddString(TranslateT("Loading..."));

		// fill predefined resources
		TCHAR *szResources[] = { L"Home", L"Work", L"Office", L"Miranda" };
		for (int i = 0; i < _countof(szResources); i++)
			m_cbResource.AddString(szResources[i]);

		// append computer name to the resource list
		TCHAR szCompName[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD dwCompNameLength = MAX_COMPUTERNAME_LENGTH;
		if (GetComputerName(szCompName, &dwCompNameLength))
			m_cbResource.AddString(szCompName);

		ptrT tszResource(m_proto->getTStringA("Resource"));
		if (tszResource != NULL) {
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
		m_cbType.AddString(TranslateT("Yandex"), ACC_YANDEX);

		char server[256], manualServer[256] = { 0 };
		m_cbServer.GetTextA(server, _countof(server));
		ptrA dbManualServer(db_get_sa(NULL, m_proto->m_szModuleName, "ManualHost"));
		if (dbManualServer != NULL)
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
		else if (!mir_strcmp(server, "ya.ru")) {
			m_cbType.SetCurSel(ACC_YANDEX);
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

				ptrT dbManualHost(m_proto->getTStringA("ManualHost"));
				if (dbManualHost != NULL)
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
		m_proto->m_savedPassword = NULL;

		BOOL bUseHostnameAsResource = FALSE;
		TCHAR szCompName[MAX_COMPUTERNAME_LENGTH + 1], szResource[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD dwCompNameLength = MAX_COMPUTERNAME_LENGTH;
		if (GetComputerName(szCompName, &dwCompNameLength)) {
			m_cbResource.GetText(szResource, _countof(szResource));
			if (!mir_tstrcmp(szCompName, szResource))
				bUseHostnameAsResource = TRUE;
		}
		m_proto->m_options.HostNameAsResource = bUseHostnameAsResource;

		if (m_chkSavePassword.GetState() == BST_CHECKED) {
			TCHAR *text = m_txtPassword.GetText();
			m_proto->setTString("Password", text);
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
		case ACC_YANDEX:
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
	void setupYA();
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
	case ACC_YANDEX: setupYA(); break;
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

void CJabberDlgAccMgrUI::setupYA()
{
	m_canregister = false;
	m_gotservers = true;
	m_cbServer.ResetContent();
	m_cbServer.SetTextA("ya.ru");
	m_cbServer.AddStringA("ya.ru");
	m_chkManualHost.SetState(BST_UNCHECKED);
	m_txtManualHost.SetTextA("xmpp.yandex.ru");
	m_txtPort.SetInt(5222);

	m_cbServer.Disable();
	m_chkManualHost.Disable();
	m_txtManualHost.Disable();
	m_txtPort.Disable();
	m_btnRegister.Disable();
}

void CJabberDlgAccMgrUI::RefreshServers(HXML node)
{
	m_gotservers = node != NULL;

	TCHAR *server = m_cbServer.GetText();
	bool bDropdown = m_cbServer.GetDroppedState();
	if (bDropdown) m_cbServer.ShowDropdown(false);

	m_cbServer.ResetContent();
	if (node) {
		for (int i = 0;; i++) {
			HXML n = XmlGetChild(node, i);
			if (!n)
				break;

			if (!mir_tstrcmp(XmlGetName(n), L"item"))
				if (const TCHAR *jid = XmlGetAttrValue(n, L"jid"))
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

	NETLIBHTTPREQUEST *result = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)wnd->GetProto()->m_hNetlibUser, (LPARAM)&request);
	if (result && IsWindow(hwnd)) {
		if ((result->resultCode == 200) && result->dataLength && result->pData) {
			TCHAR *ptszText = mir_a2t(result->pData);
			XmlNode node(ptszText, NULL, NULL);
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
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)result);
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
	OPENOPTIONSDIALOG ood = { 0 };
	ood.cbSize = sizeof(ood);
	ood.pszGroup = "Network";
	ood.pszPage = mir_t2a(m_tszUserName);
	ood.pszTab = "Account";
	Options_Open(&ood);

	mir_free((void*)ood.pszPage);
	return 0;
}

int CJabberProto::OnModernOptInit(WPARAM, LPARAM)
{/*
	static int iBoldControls[] =
	{
		IDC_TITLE1, MODERNOPT_CTRL_LAST
	};

	MODERNOPTOBJECT obj = {0};
	obj.cbSize = sizeof(obj);
	obj.dwFlags = MODEROPT_FLG_TCHAR;
	obj.hIcon = LoadIconEx("main");
	obj.hInstance = hInst;
	obj.iSection = MODERNOPT_PAGE_ACCOUNTS;
	obj.iType = MODERNOPT_TYPE_SUBSECTIONPAGE;
	obj.lptzSubsection = mir_a2t(m_szModuleName);	// title!!!!!!!!!!!
	obj.lpzTemplate = MAKEINTRESOURCEA(IDD_MODERNOPT);
	obj.iBoldControls = iBoldControls;
	obj.pfnDlgProc = JabberWizardDlgProc;
	obj.lpszClassicGroup = "Network";
	obj.lpszClassicPage = m_szModuleName;	// title!!!!!!!!!!!
	obj.lpszHelpUrl = "http://miranda-ng.org/p/Jabber";
	CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);
	mir_free(obj.lptzSubsection); */
	return 0;
}
