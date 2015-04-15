/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (ñ) 2012-15 Miranda NG project

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

#include "jabber.h"
#include "jabber_list.h"
#include "jabber_caps.h"
#include "jabber_opttree.h"

static BOOL (WINAPI *pfnEnableThemeDialogTexture)(HANDLE, DWORD) = 0;

/////////////////////////////////////////////////////////////////////////////////////////
// JabberRegisterDlgProc - the dialog proc for registering new account

#define STR_FORMAT _T("%s %s@%S:%d?")

struct { TCHAR *szCode; TCHAR *szDescription; } g_LanguageCodes[] = {
	{	_T("aa"),	LPGENT("Afar")	},
	{	_T("ab"),	LPGENT("Abkhazian")	},
	{	_T("af"),	LPGENT("Afrikaans")	},
	{	_T("ak"),	LPGENT("Akan")	},
	{	_T("sq"),	LPGENT("Albanian")	},
	{	_T("am"),	LPGENT("Amharic")	},
	{	_T("ar"),	LPGENT("Arabic")	},
	{	_T("an"),	LPGENT("Aragonese")	},
	{	_T("hy"),	LPGENT("Armenian")	},
	{	_T("as"),	LPGENT("Assamese")	},
	{	_T("av"),	LPGENT("Avaric")	},
	{	_T("ae"),	LPGENT("Avestan")	},
	{	_T("ay"),	LPGENT("Aymara")	},
	{	_T("az"),	LPGENT("Azerbaijani")	},
	{	_T("ba"),	LPGENT("Bashkir")	},
	{	_T("bm"),	LPGENT("Bambara")	},
	{	_T("eu"),	LPGENT("Basque")	},
	{	_T("be"),	LPGENT("Belarusian")	},
	{	_T("bn"),	LPGENT("Bengali")	},
	{	_T("bh"),	LPGENT("Bihari")	},
	{	_T("bi"),	LPGENT("Bislama")	},
	{	_T("bs"),	LPGENT("Bosnian")	},
	{	_T("br"),	LPGENT("Breton")	},
	{	_T("bg"),	LPGENT("Bulgarian")	},
	{	_T("my"),	LPGENT("Burmese")	},
	{	_T("ca"),	LPGENT("Catalan; Valencian")	},
	{	_T("ch"),	LPGENT("Chamorro")	},
	{	_T("ce"),	LPGENT("Chechen")	},
	{	_T("zh"),	LPGENT("Chinese")	},
	{	_T("cu"),	LPGENT("Church Slavic; Old Slavonic")	},
	{	_T("cv"),	LPGENT("Chuvash")	},
	{	_T("kw"),	LPGENT("Cornish")	},
	{	_T("co"),	LPGENT("Corsican")	},
	{	_T("cr"),	LPGENT("Cree")	},
	{	_T("cs"),	LPGENT("Czech")	},
	{	_T("da"),	LPGENT("Danish")	},
	{	_T("dv"),	LPGENT("Divehi; Dhivehi; Maldivian")	},
	{	_T("nl"),	LPGENT("Dutch; Flemish")	},
	{	_T("dz"),	LPGENT("Dzongkha")	},
	{	_T("en"),	LPGENT("English")	},
	{	_T("eo"),	LPGENT("Esperanto")	},
	{	_T("et"),	LPGENT("Estonian")	},
	{	_T("ee"),	LPGENT("Ewe")	},
	{	_T("fo"),	LPGENT("Faroese")	},
	{	_T("fj"),	LPGENT("Fijian")	},
	{	_T("fi"),	LPGENT("Finnish")	},
	{	_T("fr"),	LPGENT("French")	},
	{	_T("fy"),	LPGENT("Western Frisian")	},
	{	_T("ff"),	LPGENT("Fulah")	},
	{	_T("ka"),	LPGENT("Georgian")	},
	{	_T("de"),	LPGENT("German")	},
	{	_T("gd"),	LPGENT("Gaelic; Scottish Gaelic")	},
	{	_T("ga"),	LPGENT("Irish")	},
	{	_T("gl"),	LPGENT("Galician")	},
	{	_T("gv"),	LPGENT("Manx")	},
	{	_T("el"),	LPGENT("Greek, Modern (1453-)")	},
	{	_T("gn"),	LPGENT("Guarani")	},
	{	_T("gu"),	LPGENT("Gujarati")	},
	{	_T("ht"),	LPGENT("Haitian; Haitian Creole")	},
	{	_T("ha"),	LPGENT("Hausa")	},
	{	_T("he"),	LPGENT("Hebrew")	},
	{	_T("hz"),	LPGENT("Herero")	},
	{	_T("hi"),	LPGENT("Hindi")	},
	{	_T("ho"),	LPGENT("Hiri Motu")	},
	{	_T("hu"),	LPGENT("Hungarian")	},
	{	_T("ig"),	LPGENT("Igbo")	},
	{	_T("is"),	LPGENT("Icelandic")	},
	{	_T("io"),	LPGENT("Ido")	},
	{	_T("ii"),	LPGENT("Sichuan Yi")	},
	{	_T("iu"),	LPGENT("Inuktitut")	},
	{	_T("ie"),	LPGENT("Interlingue")	},
	{	_T("ia"),	LPGENT("Interlingua (International Auxiliary Language Association)")	},
	{	_T("id"),	LPGENT("Indonesian")	},
	{	_T("ik"),	LPGENT("Inupiaq")	},
	{	_T("it"),	LPGENT("Italian")	},
	{	_T("jv"),	LPGENT("Javanese")	},
	{	_T("ja"),	LPGENT("Japanese")	},
	{	_T("kl"),	LPGENT("Kalaallisut; Greenlandic")	},
	{	_T("kn"),	LPGENT("Kannada")	},
	{	_T("ks"),	LPGENT("Kashmiri")	},
	{	_T("kr"),	LPGENT("Kanuri")	},
	{	_T("kk"),	LPGENT("Kazakh")	},
	{	_T("km"),	LPGENT("Central Khmer")	},
	{	_T("ki"),	LPGENT("Kikuyu; Gikuyu")	},
	{	_T("rw"),	LPGENT("Kinyarwanda")	},
	{	_T("ky"),	LPGENT("Kirghiz; Kyrgyz")	},
	{	_T("kv"),	LPGENT("Komi")	},
	{	_T("kg"),	LPGENT("Kongo")	},
	{	_T("ko"),	LPGENT("Korean")	},
	{	_T("kj"),	LPGENT("Kuanyama; Kwanyama")	},
	{	_T("ku"),	LPGENT("Kurdish")	},
	{	_T("lo"),	LPGENT("Lao")	},
	{	_T("la"),	LPGENT("Latin")	},
	{	_T("lv"),	LPGENT("Latvian")	},
	{	_T("li"),	LPGENT("Limburgan; Limburger; Limburgish")	},
	{	_T("ln"),	LPGENT("Lingala")	},
	{	_T("lt"),	LPGENT("Lithuanian")	},
	{	_T("lb"),	LPGENT("Luxembourgish; Letzeburgesch")	},
	{	_T("lu"),	LPGENT("Luba-Katanga")	},
	{	_T("lg"),	LPGENT("Ganda")	},
	{	_T("mk"),	LPGENT("Macedonian")	},
	{	_T("mh"),	LPGENT("Marshallese")	},
	{	_T("ml"),	LPGENT("Malayalam")	},
	{	_T("mi"),	LPGENT("Maori")	},
	{	_T("mr"),	LPGENT("Marathi")	},
	{	_T("ms"),	LPGENT("Malay")	},
	{	_T("mg"),	LPGENT("Malagasy")	},
	{	_T("mt"),	LPGENT("Maltese")	},
	{	_T("mo"),	LPGENT("Moldavian")	},
	{	_T("mn"),	LPGENT("Mongolian")	},
	{	_T("na"),	LPGENT("Nauru")	},
	{	_T("nv"),	LPGENT("Navajo; Navaho")	},
	{	_T("nr"),	LPGENT("Ndebele, South; South Ndebele")	},
	{	_T("nd"),	LPGENT("Ndebele, North; North Ndebele")	},
	{	_T("ng"),	LPGENT("Ndonga")	},
	{	_T("ne"),	LPGENT("Nepali")	},
	{	_T("nn"),	LPGENT("Norwegian Nynorsk; Nynorsk, Norwegian")	},
	{	_T("nb"),	LPGENT("Bokmaal, Norwegian; Norwegian Bokmaal")	},
	{	_T("no"),	LPGENT("Norwegian")	},
	{	_T("ny"),	LPGENT("Chichewa; Chewa; Nyanja")	},
	{	_T("oc"),	LPGENT("Occitan (post 1500); Provencal")	},
	{	_T("oj"),	LPGENT("Ojibwa")	},
	{	_T("or"),	LPGENT("Oriya")	},
	{	_T("om"),	LPGENT("Oromo")	},
	{	_T("os"),	LPGENT("Ossetian; Ossetic")	},
	{	_T("pa"),	LPGENT("Panjabi; Punjabi")	},
	{	_T("fa"),	LPGENT("Persian")	},
	{	_T("pi"),	LPGENT("Pali")	},
	{	_T("pl"),	LPGENT("Polish")	},
	{	_T("pt"),	LPGENT("Portuguese")	},
	{	_T("ps"),	LPGENT("Pushto")	},
	{	_T("qu"),	LPGENT("Quechua")	},
	{	_T("rm"),	LPGENT("Romansh")	},
	{	_T("ro"),	LPGENT("Romanian")	},
	{	_T("rn"),	LPGENT("Rundi")	},
	{	_T("ru"),	LPGENT("Russian")	},
	{	_T("sg"),	LPGENT("Sango")	},
	{	_T("sa"),	LPGENT("Sanskrit")	},
	{	_T("sr"),	LPGENT("Serbian")	},
	{	_T("hr"),	LPGENT("Croatian")	},
	{	_T("si"),	LPGENT("Sinhala; Sinhalese")	},
	{	_T("sk"),	LPGENT("Slovak")	},
	{	_T("sl"),	LPGENT("Slovenian")	},
	{	_T("se"),	LPGENT("Northern Sami")	},
	{	_T("sm"),	LPGENT("Samoan")	},
	{	_T("sn"),	LPGENT("Shona")	},
	{	_T("sd"),	LPGENT("Sindhi")	},
	{	_T("so"),	LPGENT("Somali")	},
	{	_T("st"),	LPGENT("Sotho, Southern")	},
	{	_T("es"),	LPGENT("Spanish; Castilian")	},
	{	_T("sc"),	LPGENT("Sardinian")	},
	{	_T("ss"),	LPGENT("Swati")	},
	{	_T("su"),	LPGENT("Sundanese")	},
	{	_T("sw"),	LPGENT("Swahili")	},
	{	_T("sv"),	LPGENT("Swedish")	},
	{	_T("ty"),	LPGENT("Tahitian")	},
	{	_T("ta"),	LPGENT("Tamil")	},
	{	_T("tt"),	LPGENT("Tatar")	},
	{	_T("te"),	LPGENT("Telugu")	},
	{	_T("tg"),	LPGENT("Tajik")	},
	{	_T("tl"),	LPGENT("Tagalog")	},
	{	_T("th"),	LPGENT("Thai")	},
	{	_T("bo"),	LPGENT("Tibetan")	},
	{	_T("ti"),	LPGENT("Tigrinya")	},
	{	_T("to"),	LPGENT("Tonga (Tonga Islands)")	},
	{	_T("tn"),	LPGENT("Tswana")	},
	{	_T("ts"),	LPGENT("Tsonga")	},
	{	_T("tk"),	LPGENT("Turkmen")	},
	{	_T("tr"),	LPGENT("Turkish")	},
	{	_T("tw"),	LPGENT("Twi")	},
	{	_T("ug"),	LPGENT("Uighur; Uyghur")	},
	{	_T("uk"),	LPGENT("Ukrainian")	},
	{	_T("ur"),	LPGENT("Urdu")	},
	{	_T("uz"),	LPGENT("Uzbek")	},
	{	_T("ve"),	LPGENT("Venda")	},
	{	_T("vi"),	LPGENT("Vietnamese")	},
	{	_T("vo"),	LPGENT("Volapuk")	},
	{	_T("cy"),	LPGENT("Welsh")	},
	{	_T("wa"),	LPGENT("Walloon")	},
	{	_T("wo"),	LPGENT("Wolof")	},
	{	_T("xh"),	LPGENT("Xhosa")	},
	{	_T("yi"),	LPGENT("Yiddish")	},
	{	_T("yo"),	LPGENT("Yoruba")	},
	{	_T("za"),	LPGENT("Zhuang; Chuang")	},
	{	_T("zu"),	LPGENT("Zulu")	},
	{	NULL,	NULL	}
};

class CJabberDlgRegister: public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;
public:
	CJabberDlgRegister(CJabberProto *proto, HWND hwndParent, JABBER_CONN_DATA *regInfo):
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
		mir_sntprintf(text, SIZEOF(text), STR_FORMAT, TranslateT("Register"), m_regInfo->username, m_regInfo->server, m_regInfo->port);
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

class CCtrlEditJid: public CCtrlEdit
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
		if (msg == WM_CHAR)
		{
			switch(wParam)
			{
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
	ppro->setTString("jid", CMString(FORMAT, _T("%s@%s"), ptrT(txtUsername.GetText()), ptrT(cbServer.GetText())));
}

class CDlgOptAccount: public CJabberDlgBase
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
	CDlgOptAccount(CJabberProto *proto):
		CJabberDlgBase(proto, IDD_OPT_JABBER, false),
		m_txtUsername(this, IDC_EDIT_USERNAME),
		m_txtPassword(this, IDC_EDIT_PASSWORD),
		m_txtPriority(this, IDC_PRIORITY),
		m_chkSavePassword(this, IDC_SAVEPASSWORD),
		m_cbResource(this, IDC_COMBO_RESOURCE),
		m_chkUseHostnameAsResource(this,IDC_HOSTNAME_AS_RESOURCE),
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
		m_lnkServers(this, IDC_LINK_PUBLIC_SERVER, "http://xmpp.org/services/")
	{
		CreateLink(m_txtUsername, "LoginName", _T(""));
		CreateLink(m_txtPriority, "Priority", DBVT_WORD, 0);
		CreateLink(m_chkSavePassword, proto->m_options.SavePassword);
		CreateLink(m_cbResource, "Resource", _T("Miranda"));
		CreateLink(m_chkUseHostnameAsResource, proto->m_options.HostNameAsResource);
		CreateLink(m_chkUseDomainLogin, proto->m_options.UseDomainLogin);
		CreateLink(m_cbServer, "LoginServer", _T("jabber.org"));
		CreateLink(m_txtPort, "Port", DBVT_WORD, 5222);
		CreateLink(m_chkUseSsl, proto->m_options.UseSSL);
		CreateLink(m_chkUseTls, proto->m_options.UseTLS);
		CreateLink(m_chkManualHost, proto->m_options.ManualConnect);
		CreateLink(m_txtManualHost, "ManualHost", _T(""));
		CreateLink(m_txtManualPort, "ManualPort", DBVT_WORD, 0);
		CreateLink(m_chkKeepAlive, proto->m_options.KeepAlive);
		CreateLink(m_chkAutoDeleteContacts, proto->m_options.RosterSync);
		CreateLink(m_txtUserDirectory, "Jud", _T(""));

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
		TCHAR *szResources[] = { _T("Home"), _T("Work"), _T("Office"), _T("Miranda") };
		for (int i=0; i < SIZEOF(szResources); i++)
			m_cbResource.AddString(szResources[i]);

		// append computer name to the resource list
		TCHAR szCompName[ MAX_COMPUTERNAME_LENGTH + 1];
		DWORD dwCompNameLength = MAX_COMPUTERNAME_LENGTH;
		if (GetComputerName(szCompName, &dwCompNameLength))
			m_cbResource.AddString(szCompName);

		ptrT tszResource( m_proto->getTStringA("Resource"));
		if (tszResource != NULL) {
			if (CB_ERR == m_cbResource.FindString(tszResource, -1, true))
				m_cbResource.AddString(tszResource);
			m_cbResource.SetText(tszResource);
		}
		else m_cbResource.SetText(_T("Miranda"));

		for (int i=0; g_LanguageCodes[i].szCode; i++) {
			int iItem = m_cbLocale.AddString(TranslateTS(g_LanguageCodes[i].szDescription), (LPARAM)g_LanguageCodes[i].szCode);
			if (!_tcscmp(m_proto->m_tszSelectedLang, g_LanguageCodes[i].szCode))
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
		}	}

		sttStoreJidFromUI(m_proto, m_txtUsername, m_cbServer);

		if (m_proto->m_bJabberOnline) {
			if (m_txtUsername.IsChanged() || m_txtPassword.IsChanged() || m_cbResource.IsChanged() ||
				m_cbServer.IsChanged() || m_chkUseHostnameAsResource.IsChanged() || m_txtPort.IsChanged() ||
				m_txtManualHost.IsChanged() || m_txtManualPort.IsChanged() || m_cbLocale.IsChanged())
			{
				MessageBox(m_hwnd,
					TranslateT("These changes will take effect the next time you connect to the Jabber network."),
					TranslateT("Jabber Protocol Option"), MB_OK|MB_SETFOREGROUND);
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
		CMString buf;
		TCHAR pass[512];
		if (!m_proto->EnterString(buf, TranslateT("Confirm password"), ESF_PASSWORD))
			return;

		m_txtPassword.GetText(pass, SIZEOF(pass));
		if (mir_tstrcmp(buf, pass)) {
			MessageBox(m_hwnd, TranslateT("Passwords do not match."), _T("Miranda NG"), MB_ICONSTOP|MB_OK);
			return;
		}

		PSHNOTIFY pshn = {0};
		pshn.hdr.code = PSN_APPLY;
		pshn.hdr.hwndFrom = m_hwnd;
		SendMessage(m_hwnd, WM_NOTIFY, 0, (LPARAM)&pshn);

		JABBER_CONN_DATA regInfo;
		m_txtUsername.GetText(regInfo.username, SIZEOF(regInfo.username));
		m_txtPassword.GetText(regInfo.password, SIZEOF(regInfo.password));
		m_cbServer.GetTextA(regInfo.server, SIZEOF(regInfo.server));
		if (m_chkManualHost.GetState() == BST_CHECKED) {
			regInfo.port = (WORD)m_txtManualPort.GetInt();
			m_txtManualHost.GetTextA(regInfo.manualHost, SIZEOF(regInfo.manualHost));
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
				XmlNodeIq(_T("set"), m_proto->SerialNext(), m_proto->m_szJabberJID) << XQUERY(JABBER_FEAT_REGISTER)
					<< XCHILD(_T("remove")));
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
			m_txtPassword.SetText(_T(""));
			m_txtUsername.SetText(_T(""));
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
		m_txtUsername.GetText(regInfo.username, SIZEOF(regInfo.username));
		m_txtPassword.GetText(regInfo.password, SIZEOF(regInfo.password));
		m_cbServer.GetTextA(regInfo.server, SIZEOF(regInfo.server));
		if (m_chkManualHost.GetState() == BST_CHECKED) {
			regInfo.port = (WORD)m_txtManualPort.GetInt();
			m_txtManualHost.GetTextA(regInfo.manualHost, SIZEOF(regInfo.manualHost));
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
			for (int i=0; ; i++) {
				HXML n = xmlGetChild(node, i);
				if (!n)
					break;

				if (!mir_tstrcmp(xmlGetName(n), _T("item")))
					if (const TCHAR *jid = xmlGetAttrValue(n, _T("jid")))
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
		CDlgOptAccount *wnd = (CDlgOptAccount *)arg;
		HWND hwnd = wnd->GetHwnd();
		bool bIsError = true;

		if (!IsWindow(hwnd)) return;

		NETLIBHTTPREQUEST request = {0};
		request.cbSize = sizeof(request);
		request.requestType = REQUEST_GET;
		request.flags = NLHRF_REDIRECT | NLHRF_HTTP11;
		request.szUrl = "http://xmpp.org/services/services.xml";

		NETLIBHTTPREQUEST *result = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)wnd->GetProto()->m_hNetlibUser, (LPARAM)&request);
		if (result) {
			if (result->resultCode == 200 && result->dataLength && result->pData) {
				TCHAR *buf = mir_a2t(result->pData);
				XmlNode node(buf, NULL, NULL);
				if (node) {
					HXML queryNode = xmlGetChild(node, _T("query"));
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

class CDlgOptAdvanced: public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	CCtrlCheck		m_chkDirect;
	CCtrlCheck		m_chkDirectManual;
	CCtrlCheck		m_chkProxy;
	CCtrlEdit		m_txtDirect;
	CCtrlEdit		m_txtProxy;
	CCtrlTreeOpts	m_otvOptions;

public:
	CDlgOptAdvanced(CJabberProto *proto):
		CJabberDlgBase(proto, IDD_OPT_JABBER2, false),
		m_chkDirect(this, IDC_DIRECT),
		m_chkDirectManual(this, IDC_DIRECT_MANUAL),
		m_chkProxy(this, IDC_PROXY_MANUAL),
		m_txtDirect(this, IDC_DIRECT_ADDR),
		m_txtProxy(this, IDC_PROXY_ADDR),
		m_otvOptions(this, IDC_OPTTREE)
	{
		CreateLink(m_chkDirect, proto->m_options.BsDirect);
		CreateLink(m_chkDirectManual, proto->m_options.BsDirectManual);
		CreateLink(m_chkProxy, proto->m_options.BsProxyManual);
		CreateLink(m_txtDirect, "BsDirectAddr", _T(""));
		CreateLink(m_txtProxy, "BsProxyServer", _T(""));

		m_chkDirect.OnChange =
		m_chkDirectManual.OnChange = Callback(this, &CDlgOptAdvanced::chkDirect_OnChange);
		m_chkProxy.OnChange = Callback(this, &CDlgOptAdvanced::chkProxy_OnChange);

		m_otvOptions.AddOption(LPGENT("Messaging") _T("/") LPGENT("Send messages slower, but with full acknowledgment"), m_proto->m_options.MsgAck);
		m_otvOptions.AddOption(LPGENT("Messaging") _T("/") LPGENT("Enable avatars"), m_proto->m_options.EnableAvatars);
		m_otvOptions.AddOption(LPGENT("Messaging") _T("/") LPGENT("Log chat state changes"), m_proto->m_options.LogChatstates);
		m_otvOptions.AddOption(LPGENT("Messaging") _T("/") LPGENT("Log presence subscription state changes"), m_proto->m_options.LogPresence);
		m_otvOptions.AddOption(LPGENT("Messaging") _T("/") LPGENT("Log presence errors"), m_proto->m_options.LogPresenceErrors);
		m_otvOptions.AddOption(LPGENT("Messaging") _T("/") LPGENT("Enable user moods receiving"), m_proto->m_options.EnableUserMood);
		m_otvOptions.AddOption(LPGENT("Messaging") _T("/") LPGENT("Enable user tunes receiving"), m_proto->m_options.EnableUserTune);
		m_otvOptions.AddOption(LPGENT("Messaging") _T("/") LPGENT("Enable user activity receiving"), m_proto->m_options.EnableUserActivity);
		m_otvOptions.AddOption(LPGENT("Messaging") _T("/") LPGENT("Receive notes"), m_proto->m_options.AcceptNotes);
		m_otvOptions.AddOption(LPGENT("Messaging") _T("/") LPGENT("Automatically save received notes"), m_proto->m_options.AutosaveNotes);
		m_otvOptions.AddOption(LPGENT("Messaging") _T("/") LPGENT("Enable server-side history"), m_proto->m_options.EnableMsgArchive);

		m_otvOptions.AddOption(LPGENT("Server options") _T("/") LPGENT("Disable SASL authentication (for old servers)"), m_proto->m_options.Disable3920auth);
		m_otvOptions.AddOption(LPGENT("Server options") _T("/") LPGENT("Enable stream compression (if possible)"), m_proto->m_options.EnableZlib);

		m_otvOptions.AddOption(LPGENT("Other") _T("/") LPGENT("Enable remote controlling (from another resource of same JID only)"), m_proto->m_options.EnableRemoteControl);
		m_otvOptions.AddOption(LPGENT("Other") _T("/") LPGENT("Show transport agents on contact list"), m_proto->m_options.ShowTransport);
		m_otvOptions.AddOption(LPGENT("Other") _T("/") LPGENT("Automatically add contact when accept authorization"), m_proto->m_options.AutoAdd);
		m_otvOptions.AddOption(LPGENT("Other") _T("/") LPGENT("Automatically accept authorization requests"), m_proto->m_options.AutoAcceptAuthorization);
		m_otvOptions.AddOption(LPGENT("Other") _T("/") LPGENT("Fix incorrect timestamps in incoming messages"), m_proto->m_options.FixIncorrectTimestamps);
		m_otvOptions.AddOption(LPGENT("Other") _T("/") LPGENT("Disable frame"), m_proto->m_options.DisableFrame);
		m_otvOptions.AddOption(LPGENT("Other") _T("/") LPGENT("Enable XMPP link processing (requires Association Manager)"), m_proto->m_options.ProcessXMPPLinks);
		m_otvOptions.AddOption(LPGENT("Other") _T("/") LPGENT("Keep contacts assigned to local groups (ignore roster group)"), m_proto->m_options.IgnoreRosterGroups);

		m_otvOptions.AddOption(LPGENT("Security") _T("/") LPGENT("Allow servers to request version (XEP-0092)"), m_proto->m_options.AllowVersionRequests);
		m_otvOptions.AddOption(LPGENT("Security") _T("/") LPGENT("Show information about operating system in version replies"), m_proto->m_options.ShowOSVersion);
		m_otvOptions.AddOption(LPGENT("Security") _T("/") LPGENT("Accept only in band incoming filetransfers (don't disclose own IP)"), m_proto->m_options.BsOnlyIBB);
		m_otvOptions.AddOption(LPGENT("Security") _T("/") LPGENT("Accept HTTP Authentication requests (XEP-0070)"), m_proto->m_options.AcceptHttpAuth);
	}

	void OnInitDialog()
	{
		CSuper::OnInitDialog();

		chkDirect_OnChange(&m_chkDirect);
		chkProxy_OnChange(&m_chkProxy);
	}

	void OnApply()
	{
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
						}	}
						else if (m_proto->getWord(hContact, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
							m_proto->setWord(hContact, "Status", ID_STATUS_OFFLINE);
			}	}	}
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

class CDlgOptGc: public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	CCtrlEdit		m_txtAltNick;
	CCtrlEdit		m_txtSlap;
	CCtrlEdit		m_txtQuit;
	CCtrlTreeOpts	m_otvOptions;

public:
	CDlgOptGc(CJabberProto *proto):
		CJabberDlgBase(proto, IDD_OPT_JABBER4, false),
		m_txtAltNick(this, IDC_TXT_ALTNICK),
		m_txtSlap(this, IDC_TXT_SLAP),
		m_txtQuit(this, IDC_TXT_QUIT),
		m_otvOptions(this, IDC_OPTTREE)
	{
		CreateLink(m_txtAltNick, "GcAltNick", _T(""));
		CreateLink(m_txtSlap, "GcMsgSlap", TranslateTS(JABBER_GC_MSG_SLAP));
		CreateLink(m_txtQuit, "GcMsgQuit", TranslateTS(JABBER_GC_MSG_QUIT));

		m_otvOptions.AddOption(LPGENT("General") _T("/") LPGENT("Autoaccept multiuser chat invitations"),   m_proto->m_options.AutoAcceptMUC);
		m_otvOptions.AddOption(LPGENT("General") _T("/") LPGENT("Automatically join bookmarks on login"),   m_proto->m_options.AutoJoinBookmarks);
		m_otvOptions.AddOption(LPGENT("General") _T("/") LPGENT("Automatically join conferences on login"), m_proto->m_options.AutoJoinConferences);
		m_otvOptions.AddOption(LPGENT("General") _T("/") LPGENT("Hide conference windows at startup"),      m_proto->m_options.AutoJoinHidden);
		m_otvOptions.AddOption(LPGENT("General") _T("/") LPGENT("Do not show multiuser chat invitations"),  m_proto->m_options.IgnoreMUCInvites);
		m_otvOptions.AddOption(LPGENT("Log events") _T("/") LPGENT("Ban notifications"),                    m_proto->m_options.GcLogBans);
		m_otvOptions.AddOption(LPGENT("Log events") _T("/") LPGENT("Room configuration changes"),           m_proto->m_options.GcLogConfig);
		m_otvOptions.AddOption(LPGENT("Log events") _T("/") LPGENT("Affiliation changes"),                  m_proto->m_options.GcLogAffiliations);
		m_otvOptions.AddOption(LPGENT("Log events") _T("/") LPGENT("Role changes"),                         m_proto->m_options.GcLogRoles);
		m_otvOptions.AddOption(LPGENT("Log events") _T("/") LPGENT("Status changes"),                       m_proto->m_options.GcLogStatuses);
		m_otvOptions.AddOption(LPGENT("Log events") _T("/") LPGENT("Don't notify history messages"),        m_proto->m_options.GcLogChatHistory);
	}
};

//////////////////////////////////////////////////////////////////////////
// roster editor
//

#include <io.h>
#define JM_STATUSCHANGED WM_USER+0x0001
#define fopent(name, mode) _wfopen(name, mode)

enum {
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
	LVITEM item = {0};
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
	while ( ListView_DeleteColumn(hList, 0));

	LV_COLUMN column = {0};
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
	int width = rc.right-rc.left;

	ListView_SetColumnWidth(hList, 0, width*40/100);
	ListView_SetColumnWidth(hList, 1, width*25/100);
	ListView_SetColumnWidth(hList, 2, width*20/100);
	ListView_SetColumnWidth(hList, 3, width*10/100);
}

void CJabberProto::_RosterHandleGetRequest(HXML node, CJabberIqInfo*)
{
	HWND hList = GetDlgItem(rrud.hwndDlg, IDC_ROSTER);
	if (rrud.bRRAction == RRA_FILLLIST) {
		_RosterListClear(rrud.hwndDlg);
		HXML query = xmlGetChild(node , "query");
		if (query == NULL) return;
		int i = 1;
		while (TRUE) {
			HXML item = xmlGetNthChild(query, _T("item"), i++);
			if (item == NULL)
				break;

			const TCHAR *jid = xmlGetAttrValue(item, _T("jid"));
			if (jid == NULL)
				continue;

			const TCHAR *name = xmlGetAttrValue(item, _T("name"));
			const TCHAR *subscription = xmlGetAttrValue(item, _T("subscription"));
			const TCHAR *group = xmlGetText( xmlGetChild(item, "group"));
			_RosterInsertListItem(hList, jid, name, group, subscription, TRUE);
		}

		// now it is require to process whole contact list to add not in roster contacts
		for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
			ptrT tszJid( getTStringA(hContact, "jid"));
			if (tszJid == NULL)
				continue;

			LVFINDINFO lvfi = {0};
			lvfi.flags = LVFI_STRING;
			lvfi.psz = tszJid;
			TCHAR *p = _tcschr(tszJid, _T('@'));
			if (p) {
				p = _tcschr(tszJid, _T('/'));
				if (p) *p = 0;
			}
			if ( ListView_FindItem(hList, -1, &lvfi) == -1) {
				ptrT tszName( db_get_tsa(hContact, "CList", "MyHandle"));
				ptrT tszGroup( db_get_tsa(hContact, "CList", "Group"));
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
		HXML queryRoster = xmlGetChild(node , "query");
		if (!queryRoster)
			return;

		XmlNodeIq iq( AddIQ(&CJabberProto::_RosterHandleGetRequest, JABBER_IQ_TYPE_SET));

		HXML query = iq << XCHILDNS(_T("query"), JABBER_FEAT_IQ_ROSTER);

		int itemCount=0;
		int ListItemCount=ListView_GetItemCount(hList);
		for (int index=0; index < ListItemCount; index++) {
			TCHAR jid[JABBER_MAX_JID_LEN]=_T("");
			TCHAR name[260]=_T("");
			TCHAR group[260]=_T("");
			TCHAR subscr[260]=_T("");
			ListView_GetItemText(hList, index, 0, jid, SIZEOF(jid));
			ListView_GetItemText(hList, index, 1, name, SIZEOF(name));
			ListView_GetItemText(hList, index, 2, group, SIZEOF(group));
			ListView_GetItemText(hList, index, 3, subscr, SIZEOF(subscr));
			HXML itemRoster = xmlGetChildByTag(queryRoster, "item", "jid", jid);
			BOOL bRemove = !ListView_GetCheckState(hList,index);
			if (itemRoster && bRemove) {
				//delete item
				query << XCHILD(_T("item")) << XATTR(_T("jid"), jid) << XATTR(_T("subscription") ,_T("remove"));
				itemCount++;
			}
			else if (!bRemove) {
				BOOL bPushed = itemRoster ? TRUE : FALSE;
				if (!bPushed) {
					const TCHAR *rosterName = xmlGetAttrValue(itemRoster, _T("name"));
					if ((rosterName != NULL || name[0]!=0) && mir_tstrcmpi(rosterName,name))
						bPushed=TRUE;
					if (!bPushed) {
						rosterName = xmlGetAttrValue(itemRoster, _T("subscription"));
						if ((rosterName != NULL || subscr[0]!=0) && mir_tstrcmpi(rosterName,subscr))
							bPushed=TRUE;
					}
					if (!bPushed) {
						const TCHAR *rosterGroup = xmlGetText( xmlGetChild(itemRoster, "group"));
						if ((rosterGroup != NULL || group[0]!=0) && mir_tstrcmpi(rosterGroup,group))
							bPushed=TRUE;
					}
				}
				if (bPushed) {
					HXML item = query << XCHILD(_T("item"));
					if (_tcslen(group))
						item << XCHILD(_T("group"), group);
					if (_tcslen(name))
						item << XATTR(_T("name"), name);
					item << XATTR(_T("jid"), jid) << XATTR(_T("subscription"), subscr[0] ? subscr : _T("none"));
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
		XmlNodeIq( AddIQ(&CJabberProto::_RosterHandleGetRequest, JABBER_IQ_TYPE_GET))
			<< XCHILDNS(_T("query"), JABBER_FEAT_IQ_ROSTER));
}

static void _RosterItemEditEnd(HWND hEditor, ROSTEREDITDAT * edat, BOOL bCancel)
{
	if (!bCancel)
	{
		int len = GetWindowTextLength(hEditor) + 1;
		TCHAR *buff=(TCHAR*)mir_alloc(len*sizeof(TCHAR));
		if (buff) {
			GetWindowText(hEditor,buff,len);
			ListView_SetItemText(edat->hList,edat->index, edat->subindex,buff);
		}
		mir_free(buff);
	}
	DestroyWindow(hEditor);
}

static LRESULT CALLBACK _RosterItemNewEditProc(HWND hEditor, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ROSTEREDITDAT * edat = (ROSTEREDITDAT *) GetWindowLongPtr(hEditor,GWLP_USERDATA);
	if (!edat) return 0;
	switch(msg) {
	case WM_KEYDOWN:
		switch(wParam) {
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
			if (msg2->message==WM_KEYDOWN && msg2->wParam==VK_TAB) return 0;
			if (msg2->message==WM_CHAR && msg2->wParam=='\t') return 0;
		}
		return DLGC_WANTMESSAGE;

	case WM_KILLFOCUS:
		_RosterItemEditEnd(hEditor, edat, FALSE);
		return 0;

	case WM_DESTROY:
		SetWindowLongPtr(hEditor, GWLP_USERDATA, (LONG_PTR) 0);
		free(edat);
		return 0;
	}
	
	return mir_callNextSubclass(hEditor, _RosterItemNewEditProc, msg, wParam, lParam);
}

void CJabberProto::_RosterExportToFile(HWND hwndDlg)
{
	TCHAR filename[MAX_PATH] = { 0 };

	TCHAR filter[MAX_PATH];
	mir_sntprintf(filter, SIZEOF(filter), _T("%s (*.xml)%c*.xml%c%c"), TranslateT("XML for MS Excel (UTF-8 encoded)"), 0, 0, 0);
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.hwndOwner = hwndDlg;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = filename;
	ofn.Flags = OFN_HIDEREADONLY;
	ofn.nMaxFile = SIZEOF(filename);
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrDefExt = _T("xml");
	if (!GetSaveFileName(&ofn)) return;

	FILE * fp = fopent(filename,_T("w"));
	if (!fp) return;
	HWND hList = GetDlgItem(hwndDlg, IDC_ROSTER);
	int ListItemCount = ListView_GetItemCount(hList);

	XmlNode root(_T("Workbook"));
	root << XATTR(_T("xmlns"), _T("urn:schemas-microsoft-com:office:spreadsheet"))
	     << XATTR(_T("xmlns:o"), _T("urn:schemas-microsoft-com:office:office"))
	     << XATTR(_T("xmlns:x"), _T("urn:schemas-microsoft-com:office:excel"))
	     << XATTR(_T("xmlns:ss"), _T("urn:schemas-microsoft-com:office:spreadsheet"))
	     << XATTR(_T("xmlns:html"), _T("http://www.w3.org/TR/REC-html40"));
	root << XCHILD(_T("ExcelWorkbook"))
	     << XATTR(_T("xmlns"), _T("urn:schemas-microsoft-com:office:excel"));
	HXML table = root << XCHILD(_T("Worksheet")) << XATTR(_T("ss:Name"), _T("Exported roster"))
	                  << XCHILD(_T("Table"));

	for (int index=0; index<ListItemCount; index++)
	{
		TCHAR jid[JABBER_MAX_JID_LEN]=_T("");
		TCHAR name[260]=_T("");
		TCHAR group[260]=_T("");
		TCHAR subscr[260]=_T("");
		ListView_GetItemText(hList, index, 0, jid, SIZEOF(jid));
		ListView_GetItemText(hList, index, 1, name, SIZEOF(name));
		ListView_GetItemText(hList, index, 2, group, SIZEOF(group));
		ListView_GetItemText(hList, index, 3, subscr, SIZEOF(subscr));

		HXML node = table << XCHILD(_T("Row"));
		node << XCHILD(_T("Cell")) << XCHILD(_T("Data"), _T("+")) << XATTR(_T("ss:Type"), _T("String"));
		node << XCHILD(_T("Cell")) << XCHILD(_T("Data"), jid) << XATTR(_T("ss:Type"), _T("String"));
		node << XCHILD(_T("Cell")) << XCHILD(_T("Data"), name) << XATTR(_T("ss:Type"), _T("String"));
		node << XCHILD(_T("Cell")) << XCHILD(_T("Data"), group) << XATTR(_T("ss:Type"), _T("String"));
		node << XCHILD(_T("Cell")) << XCHILD(_T("Data"), subscr) << XATTR(_T("ss:Type"), _T("String"));

	}

	char header[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<?mso-application progid=\"Excel.Sheet\"?>\n";
	fwrite(header, 1, sizeof(header) - 1 /* for zero terminator */, fp);

	TCHAR *xtmp = xi.toString(root, NULL);
	char *tmp = mir_utf8encodeT(xtmp);
	xi.freeMem(xtmp);

	fwrite(tmp, 1, strlen(tmp), fp);
	mir_free(tmp);
	fclose(fp);
}

void CJabberProto::_RosterImportFromFile(HWND hwndDlg)
{
	char filename[MAX_PATH]={0};
	char *filter="XML for MS Excel (UTF-8 encoded)(*.xml)\0*.xml\0\0";
	OPENFILENAMEA ofn={0};
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.hwndOwner = hwndDlg;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = filename;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.nMaxFile = SIZEOF(filename);
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrDefExt = "xml";
	if (!GetOpenFileNameA(&ofn))
		return;

	FILE * fp=fopen(filename,"r");
	if (!fp)
		return;

	DWORD bufsize = _filelength(_fileno(fp));
	if (bufsize <= 0) {
		fclose(fp);
		return;
	}

	char* buffer=(char*)mir_calloc(bufsize+1); // zero-terminate it
	fread(buffer,1,bufsize,fp);
	fclose(fp);
	_RosterListClear(hwndDlg);

	TCHAR *newBuf = mir_utf8decodeT(buffer);
	mir_free(buffer);

	int nBytesProcessed = 0;
	XmlNode node(newBuf, &nBytesProcessed, NULL);
	if (node) {
		HXML Workbook = xmlGetChild(node, _T("Workbook"));
		if (Workbook) {
			HXML Worksheet = xmlGetChild(Workbook , "Worksheet");
			if (Worksheet) {
				HXML Table = xmlGetChild(Worksheet , "Table");
				if (Table) {
					int index=1;
					HWND hList=GetDlgItem(hwndDlg, IDC_ROSTER);
					while (TRUE)
					{
						HXML Row = xmlGetNthChild(Table, _T("Row"), index++);
						if (!Row)
							break;

						BOOL bAdd=FALSE;
						const TCHAR *jid=NULL;
						const TCHAR *name=NULL;
						const TCHAR *group=NULL;
						const TCHAR *subscr=NULL;
						HXML Cell = xmlGetNthChild(Row, _T("Cell"), 1);
						HXML Data = (Cell) ? xmlGetChild(Cell , "Data") : XmlNode();
						if (Data)
						{
							if (!mir_tstrcmpi(xmlGetText(Data),_T("+"))) bAdd=TRUE;
							else if (mir_tstrcmpi(xmlGetText(Data),_T("-"))) continue;

							Cell = xmlGetNthChild(Row, _T("Cell"),2);
							if (Cell) Data=xmlGetChild(Cell , "Data");
							else Data = NULL;
							if (Data)
							{
								jid=xmlGetText(Data);
								if (!jid || mir_tstrlen(jid)==0) continue;
							}

							Cell=xmlGetNthChild(Row,_T("Cell"),3);
							if (Cell) Data=xmlGetChild(Cell , "Data");
							else Data = NULL;
							if (Data) name=xmlGetText(Data);

							Cell=xmlGetNthChild(Row,_T("Cell"),4);
							if (Cell) Data=xmlGetChild(Cell , "Data");
							else Data = NULL;
							if (Data) group=xmlGetText(Data);

							Cell=xmlGetNthChild(Row,_T("Cell"),5);
							if (Cell) Data=xmlGetChild(Cell , "Data");
							else Data = NULL;
							if (Data) subscr=xmlGetText(Data);
						}
						_RosterInsertListItem(hList,jid,name,group,subscr,bAdd);
	}	}	}	}	}

	mir_free(newBuf);
	SendMessage(hwndDlg, JM_STATUSCHANGED, 0, 0);
}

static LRESULT CALLBACK _RosterNewListProc(HWND hList, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg==WM_MOUSEWHEEL || msg==WM_NCLBUTTONDOWN || msg==WM_NCRBUTTONDOWN)
		SetFocus(hList);

	if (msg==WM_LBUTTONDOWN)
	{
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(hList, &pt);

		LVHITTESTINFO lvhti={0};
		lvhti.pt=pt;
		ListView_SubItemHitTest(hList,&lvhti);
		if 	(lvhti.flags&LVHT_ONITEM && lvhti.iSubItem !=0)
		{
			RECT rc;
			TCHAR buff[260];
			ListView_GetSubItemRect(hList, lvhti.iItem, lvhti.iSubItem, LVIR_BOUNDS,&rc);
			ListView_GetItemText(hList, lvhti.iItem, lvhti.iSubItem, buff, SIZEOF(buff));
			HWND hEditor=CreateWindow(TEXT("EDIT"),buff,WS_CHILD|ES_AUTOHSCROLL,rc.left+3, rc.top+2, rc.right-rc.left-3, rc.bottom - rc.top-3,hList, NULL, hInst, NULL);
			SendMessage(hEditor,WM_SETFONT,(WPARAM)SendMessage(hList,WM_GETFONT,0,0),0);
			ShowWindow(hEditor,SW_SHOW);
			SetWindowText(hEditor, buff);
			ClientToScreen(hList, &pt);
			ScreenToClient(hEditor, &pt);
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

			ROSTEREDITDAT * edat=(ROSTEREDITDAT *)malloc(sizeof(ROSTEREDITDAT));
			mir_subclassWindow(hEditor, _RosterItemNewEditProc);
			edat->hList=hList;
			edat->index=lvhti.iItem;
			edat->subindex=lvhti.iSubItem;
			SetWindowLongPtr(hEditor,GWLP_USERDATA,(LONG_PTR)edat);
		}
	}
	return mir_callNextSubclass(hList, _RosterNewListProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberRosterOptDlgProc - advanced options dialog procedure

static int sttRosterEditorResizer(HWND /*hwndDlg*/, LPARAM, UTILRESIZECONTROL *urc)
{
	switch (urc->wId)
	{
	case IDC_HEADERBAR:
		return RD_ANCHORX_LEFT|RD_ANCHORY_TOP|RD_ANCHORX_WIDTH;
	case IDC_ROSTER:
		return RD_ANCHORX_LEFT|RD_ANCHORY_TOP|RD_ANCHORY_HEIGHT|RD_ANCHORX_WIDTH;
	case IDC_DOWNLOAD:
	case IDC_UPLOAD:
		return RD_ANCHORX_LEFT|RD_ANCHORY_BOTTOM;
	case IDC_EXPORT:
	case IDC_IMPORT:
		return RD_ANCHORX_RIGHT|RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT|RD_ANCHORY_TOP;
}

static INT_PTR CALLBACK JabberRosterOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CJabberProto *ppro = (CJabberProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case JM_STATUSCHANGED:
		{
			int count = ListView_GetItemCount(GetDlgItem(hwndDlg,IDC_ROSTER));
			EnableWindow(GetDlgItem(hwndDlg, IDC_DOWNLOAD), ppro->m_bJabberOnline);
			EnableWindow(GetDlgItem(hwndDlg, IDC_UPLOAD), count && ppro->m_bJabberOnline);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EXPORT), count > 0);
			break;
		}
	case WM_CLOSE:
		{
			DestroyWindow(hwndDlg);
			break;
		}
	case WM_DESTROY:
		{
			Utils_SaveWindowPosition(hwndDlg, NULL, ppro->m_szModuleName, "rosterCtrlWnd_");
			ppro->rrud.hwndDlg = NULL;
			WindowFreeIcon(hwndDlg);
			break;
		}
	case WM_INITDIALOG:
		{
			ppro = (CJabberProto*)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			TranslateDialogDefault(hwndDlg);
			WindowSetIcon(hwndDlg, ppro, "Agents");

			Utils_RestoreWindowPosition(hwndDlg, NULL, ppro->m_szModuleName, "rosterCtrlWnd_");

			ListView_SetExtendedListViewStyle(GetDlgItem(hwndDlg,IDC_ROSTER),  LVS_EX_CHECKBOXES | LVS_EX_BORDERSELECT /*| LVS_EX_FULLROWSELECT*/ | LVS_EX_GRIDLINES /*| LVS_EX_HEADERDRAGDROP*/);
			mir_subclassWindow( GetDlgItem(hwndDlg,IDC_ROSTER), _RosterNewListProc);
			_RosterListClear(hwndDlg);
			ppro->rrud.hwndDlg = hwndDlg;
			ppro->rrud.bReadyToDownload = TRUE;
			ppro->rrud.bReadyToUpload = FALSE;
			SendMessage(hwndDlg, JM_STATUSCHANGED, 0, 0);
		}
		return TRUE;

	case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
			lpmmi->ptMinTrackSize.x = 550;
			lpmmi->ptMinTrackSize.y = 390;
			return 0;
		}

	case WM_SIZE:
		{
			UTILRESIZEDIALOG urd = {0};
			urd.cbSize = sizeof(urd);
			urd.hInstance = hInst;
			urd.hwndDlg = hwndDlg;
			urd.lpTemplate = MAKEINTRESOURCEA(IDD_OPT_JABBER3);
			urd.pfnResizer = sttRosterEditorResizer;
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_DOWNLOAD:
			ppro->rrud.bReadyToUpload = FALSE;
			ppro->rrud.bReadyToDownload = FALSE;
			SendMessage(ppro->rrud.hwndDlg, JM_STATUSCHANGED,0,0);
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

class CJabberDlgAccMgrUI: public CJabberDlgBase
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
	CJabberDlgAccMgrUI(CJabberProto *proto, HWND hwndParent):
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

		CreateLink(m_txtUsername, "LoginName", _T(""));
		CreateLink(m_chkSavePassword, proto->m_options.SavePassword);
		CreateLink(m_cbResource, "Resource", _T("Miranda"));
		CreateLink(m_cbServer, "LoginServer", _T("jabber.org"));
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
	enum { ACC_PUBLIC, ACC_TLS, ACC_SSL, ACC_GTALK, ACC_LJTALK, ACC_FBOOK, ACC_OK, ACC_SMS };

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
		TCHAR *szResources[] = { _T("Home"), _T("Work"), _T("Office"), _T("Miranda") };
		for (int i=0; i < SIZEOF(szResources); i++)
			m_cbResource.AddString(szResources[i]);

		// append computer name to the resource list
		TCHAR szCompName[ MAX_COMPUTERNAME_LENGTH + 1];
		DWORD dwCompNameLength = MAX_COMPUTERNAME_LENGTH;
		if (GetComputerName(szCompName, &dwCompNameLength))
			m_cbResource.AddString(szCompName);

		ptrT tszResource( m_proto->getTStringA("Resource"));
		if (tszResource != NULL) {
			if (CB_ERR == m_cbResource.FindString(tszResource, -1, true))
				m_cbResource.AddString(tszResource);
			m_cbResource.SetText(tszResource);
		}
		else m_cbResource.SetText(_T("Miranda"));

		m_cbType.AddString(TranslateT("Public XMPP Network"), ACC_PUBLIC);
		m_cbType.AddString(TranslateT("Secure XMPP Network"), ACC_TLS);
		m_cbType.AddString(TranslateT("Secure XMPP Network (old style)"), ACC_SSL);
		m_cbType.AddString(TranslateT("Google Talk!"), ACC_GTALK);
		m_cbType.AddString(TranslateT("LiveJournal Talk"), ACC_LJTALK);
		m_cbType.AddString(TranslateT("Facebook Chat"), ACC_FBOOK);
		m_cbType.AddString(TranslateT("Odnoklassniki"), ACC_OK);
		m_cbType.AddString(TranslateT("S.ms"), ACC_SMS);

		char server[256], manualServer[256]={0};
		m_cbServer.GetTextA(server, SIZEOF(server));
		ptrA dbManualServer( db_get_sa(NULL, m_proto->m_szModuleName, "ManualHost"));
		if (dbManualServer != NULL)
			mir_strncpy(manualServer, dbManualServer, SIZEOF(manualServer));

		m_canregister = true;
		if (!mir_strcmp(manualServer, "talk.google.com")) {
			m_cbType.SetCurSel(ACC_GTALK);
			m_canregister = false;
		}
		else if (!mir_strcmp(server, "livejournal.com")) {
			m_cbType.SetCurSel(ACC_LJTALK);
			m_canregister = false;
		}
		else if (!mir_strcmp(server, "chat.facebook.com")) {
			m_cbType.SetCurSel(ACC_FBOOK);
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

				ptrT dbManualHost( m_proto->getTStringA("ManualHost"));
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
			m_cbResource.GetText(szResource, SIZEOF(szResource));
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
			m_proto->setWord("Priority", 24);
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

		case ACC_FBOOK:
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

		case ACC_SSL:
			m_proto->m_options.UseSSL = TRUE;
			m_proto->m_options.UseTLS = FALSE;
			break;
		}

		char server[256];
		char manualServer[256];

		m_cbServer.GetTextA(server, SIZEOF(server));
		m_txtManualHost.GetTextA(manualServer, SIZEOF(manualServer));

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
				m_cbServer.IsChanged() || m_txtPort.IsChanged() || m_txtManualHost.IsChanged())
			{
				MessageBox(m_hwnd,
					TranslateT("Some changes will take effect the next time you connect to the Jabber network."),
					TranslateT("Jabber Protocol Option"), MB_OK|MB_SETFOREGROUND);
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
		CMString buf;
		if (!m_proto->EnterString(buf, TranslateT("Confirm password"), ESF_PASSWORD))
			return;

		TCHAR pass[512];
		m_txtPassword.GetText(pass, SIZEOF(pass));
		if (mir_tstrcmp(buf, pass)) {
			MessageBox(m_hwnd, TranslateT("Passwords do not match."), _T("Miranda NG"), MB_ICONSTOP|MB_OK);
			return;
		}

		PSHNOTIFY pshn = {0};
		pshn.hdr.code = PSN_APPLY;
		pshn.hdr.hwndFrom = m_hwnd;
		SendMessage(m_hwnd, WM_NOTIFY, 0, (LPARAM)&pshn);

		JABBER_CONN_DATA regInfo;
		m_txtUsername.GetText(regInfo.username, SIZEOF(regInfo.username));
		m_txtPassword.GetText(regInfo.password, SIZEOF(regInfo.password));
		m_cbServer.GetTextA(regInfo.server, SIZEOF(regInfo.server));
		regInfo.port = (WORD)m_txtPort.GetInt();
		if (m_chkManualHost.GetState() == BST_CHECKED)
			m_txtManualHost.GetTextA(regInfo.manualHost, SIZEOF(regInfo.manualHost));
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
			m_txtPassword.SetText(_T(""));
			m_txtUsername.SetText(_T(""));
			m_chkSavePassword.SetState(BST_CHECKED);
		}
	}

	void chkManualHost_OnChange(CCtrlData *sender)
	{
		CCtrlCheck *chk = (CCtrlCheck *)sender;

		if (chk->GetState() == BST_CHECKED) {
			char buf[256];
			m_cbServer.GetTextA(buf, SIZEOF(buf));
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
	void setupFB();
	void setupVK();
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
	m_txtUsername.GetText(regInfo.username, SIZEOF(regInfo.username));
	m_txtPassword.GetText(regInfo.password, SIZEOF(regInfo.password));
	m_cbServer.GetTextA(regInfo.server, SIZEOF(regInfo.server));
	regInfo.port = m_txtPort.GetInt();
	if (m_chkManualHost.GetState() == BST_CHECKED)
		m_txtManualHost.GetTextA(regInfo.manualHost, SIZEOF(regInfo.manualHost));
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
		case ACC_FBOOK: setupFB(); break;
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

void CJabberDlgAccMgrUI::setupFB()
{
	m_canregister = false;
	m_gotservers = true;
	m_cbServer.ResetContent();
	m_cbServer.SetTextA("chat.facebook.com");
	m_cbServer.AddStringA("chat.facebook.com");
	m_chkManualHost.SetState(BST_UNCHECKED);
	m_txtManualHost.SetTextA("");
	m_txtPort.SetInt(443);

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
	m_gotservers = node != NULL;

	TCHAR *server = m_cbServer.GetText();
	bool bDropdown = m_cbServer.GetDroppedState();
	if (bDropdown) m_cbServer.ShowDropdown(false);

	m_cbServer.ResetContent();
	if (node) {
		for (int i = 0;; i++) {
			HXML n = xmlGetChild(node, i);
			if (!n)
				break;

			if (!mir_tstrcmp(xmlGetName(n), _T("item")))
			if (const TCHAR *jid = xmlGetAttrValue(n, _T("jid")))
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
	request.flags = NLHRF_GENERATEHOST | NLHRF_SMARTREMOVEHOST | NLHRF_SMARTAUTHHEADER | NLHRF_HTTP11;
	request.szUrl = "http://xmpp.org/services/services.xml";

	NETLIBHTTPREQUEST *result = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)wnd->GetProto()->m_hNetlibUser, (LPARAM)&request);
	if (result && IsWindow(hwnd)) {
		if ((result->resultCode == 200) && result->dataLength && result->pData) {
			TCHAR *ptszText = mir_a2t(result->pData);
			XmlNode node(ptszText, NULL, NULL);
			if (node) {
				HXML queryNode = xmlGetChild(node, _T("query"));
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
