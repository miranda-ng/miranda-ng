/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "gg.h"

////////////////////////////////////////////////////////////////////////////////
// Info Page UI dialog

class GaduUserInfoDlg : public CUserInfoPageDlg
{
	GaduProto *gg;
	bool updating = false;

	CCtrlCombo cmbGender;
	CCtrlButton btnSave;

	void SetValue(int idCtrl, char *szModule, char *szSetting, int special)
	{
		DBVARIANT dbv = { 0 };
		wchar_t str[256];
		wchar_t *ptstr = nullptr;
		wchar_t* valT = nullptr;
		bool unspecified;

		dbv.type = DBVT_DELETED;
		if (szModule == nullptr)
			unspecified = true;
		else
			unspecified = db_get(m_hContact, szModule, szSetting, &dbv) != 0;

		if (!unspecified) {
			switch (dbv.type) {
			case DBVT_BYTE:
				if (special == SVS_GENDER) {
					if (dbv.cVal == 'M')
						ptstr = TranslateT("Male");
					else if (dbv.cVal == 'F')
						ptstr = TranslateT("Female");
					else
						unspecified = 1;
				}
				else if (special == SVS_MONTH) {
					if (dbv.bVal > 0 && dbv.bVal <= 12) {
						ptstr = str;
						GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SABBREVMONTHNAME1 - 1 + dbv.bVal, str, _countof(str));
					}
					else
						unspecified = 1;
				}
				else if (special == SVS_TIMEZONE) {
					if (dbv.cVal == -100)
						unspecified = 1;
					else {
						ptstr = str;
						mir_snwprintf(str, dbv.cVal ? L"GMT%+d:%02d" : L"GMT", -dbv.cVal / 2, (dbv.cVal & 1) * 30);
					}
				}
				else {
					unspecified = (special == SVS_ZEROISUNSPEC && dbv.bVal == 0);
					ptstr = _itow(special == SVS_SIGNED ? dbv.cVal : dbv.bVal, str, 10);
				}
				break;
			case DBVT_WORD:
				if (special == SVS_COUNTRY) {
					char* pstr = (char*)CallService(MS_UTILS_GETCOUNTRYBYNUMBER, dbv.wVal, 0);
					if (pstr == nullptr) {
						unspecified = 1;
					}
					else {
						ptstr = str;
						mir_snwprintf(str, L"%S", pstr);
					}
				}
				else {
					unspecified = (special == SVS_ZEROISUNSPEC && dbv.wVal == 0);
					ptstr = _itow(special == SVS_SIGNED ? dbv.sVal : dbv.wVal, str, 10);
				}
				break;
			case DBVT_DWORD:
				unspecified = (special == SVS_ZEROISUNSPEC && dbv.dVal == 0);
				if (special == SVS_IP) {
					struct in_addr ia;
					ia.S_un.S_addr = htonl(dbv.dVal);
					char* pstr = inet_ntoa(ia);
					if (pstr == nullptr) {
						unspecified = 1;
					}
					else {
						ptstr = str;
						mir_snwprintf(str, L"%S", pstr);
					}
					if (dbv.dVal == 0)
						unspecified = 1;
				}
				else if (special == SVS_GGVERSION) {
					ptstr = str;
					mir_snwprintf(str, L"%S", (char *)gg_version2string(dbv.dVal));
				}
				else {
					ptstr = _itow(special == SVS_SIGNED ? dbv.lVal : dbv.dVal, str, 10);
				}
				break;
			case DBVT_ASCIIZ:
				unspecified = (special == SVS_ZEROISUNSPEC && dbv.pszVal[0] == '\0');
				ptstr = str;
				mir_snwprintf(str, L"%S", dbv.pszVal);
				break;
			case DBVT_WCHAR:
				unspecified = (special == SVS_ZEROISUNSPEC && dbv.pwszVal[0] == '\0');
				ptstr = dbv.pwszVal;
				break;
			case DBVT_UTF8:
				unspecified = (special == SVS_ZEROISUNSPEC && dbv.pszVal[0] == '\0');
				valT = mir_utf8decodeW(dbv.pszVal);
				ptstr = str;
				wcscpy_s(str, _countof(str), valT);
				mir_free(valT);
				break;
			default:
				ptstr = str;
				mir_wstrcpy(str, L"???");
				break;
			}
		}

		if (m_hContact != 0) {
			EnableWindow(GetDlgItem(m_hwnd, idCtrl), !unspecified);
			if (unspecified)
				SetDlgItemText(m_hwnd, idCtrl, TranslateT("<not specified>"));
			else
				SetDlgItemText(m_hwnd, idCtrl, ptstr);
		}
		else {
			EnableWindow(GetDlgItem(m_hwnd, idCtrl), TRUE);
			if (!unspecified)
				SetDlgItemText(m_hwnd, idCtrl, ptstr);
		}
		db_free(&dbv);
	}

public:
	GaduUserInfoDlg(GaduProto *_gg, int idDialog) :
		CUserInfoPageDlg(g_plugin, idDialog),
		gg(_gg),
		btnSave(this, IDC_SAVE),
		cmbGender(this, IDC_GENDER)
	{
		btnSave.OnClick = Callback(this, &GaduUserInfoDlg::onClick_Save);
	}

	bool OnInitDialog() override
	{
		// Add genders
		cmbGender.AddString(L"", 0);
		cmbGender.AddString(TranslateT("Female"), 1);
		cmbGender.AddString(TranslateT("Male"), 2);
		return true;
	}

	bool OnRefresh() override
	{
		// Show updated message
		if (updating) {
			MessageBox(nullptr, TranslateT("Your details has been uploaded to the public directory."),
				gg->m_tszUserName, MB_OK | MB_ICONINFORMATION);
			updating = false;
			return false;
		}

		char *szProto = (m_hContact == NULL) ? gg->m_szModuleName : Proto_GetBaseAccountName(m_hContact);
		if (szProto == nullptr)
			return false;

		// Disable when updating
		m_bInitialized = false;

		SetValue(IDC_UIN, szProto, GG_KEY_UIN, 0);
		SetValue(IDC_REALIP, szProto, GG_KEY_CLIENTIP, SVS_IP);
		SetValue(IDC_PORT, szProto, GG_KEY_CLIENTPORT, SVS_ZEROISUNSPEC);
		SetValue(IDC_VERSION, szProto, GG_KEY_CLIENTVERSION, SVS_GGVERSION);

		SetValue(IDC_FIRSTNAME, szProto, GG_KEY_PD_FIRSTNAME, SVS_NORMAL);
		SetValue(IDC_LASTNAME, szProto, GG_KEY_PD_LASTNAME, SVS_NORMAL);
		SetValue(IDC_NICKNAME, szProto, GG_KEY_PD_NICKNAME, SVS_NORMAL);
		SetValue(IDC_BIRTHYEAR, szProto, GG_KEY_PD_BIRTHYEAR, SVS_ZEROISUNSPEC);
		SetValue(IDC_CITY, szProto, GG_KEY_PD_CITY, SVS_NORMAL);
		SetValue(IDC_FAMILYNAME, szProto, GG_KEY_PD_FAMILYNAME, SVS_NORMAL);
		SetValue(IDC_CITYORIGIN, szProto, GG_KEY_PD_FAMILYCITY, SVS_NORMAL);

		if (m_hContact) {
			SetValue(IDC_GENDER, szProto, GG_KEY_PD_GANDER, SVS_GENDER);
			SetValue(IDC_STATUSDESCR, "CList", GG_KEY_STATUSDESCR, SVS_NORMAL);
		}
		else switch ((char)db_get_b(m_hContact, gg->m_szModuleName, GG_KEY_PD_GANDER, (uint8_t)'?')) {
		case 'F':
			SendDlgItemMessage(m_hwnd, IDC_GENDER, CB_SETCURSEL, 1, 0);
			break;
		case 'M':
			SendDlgItemMessage(m_hwnd, IDC_GENDER, CB_SETCURSEL, 2, 0);
			break;
		default:
			SendDlgItemMessage(m_hwnd, IDC_GENDER, CB_SETCURSEL, 0, 0);
		}

		// Disable when updating
		m_bInitialized = true;
		return false;
	}

	void OnChange() override
	{
		EnableWindow(GetDlgItem(m_hwnd, IDC_SAVE), TRUE);
	}

	void onClick_Save(CCtrlButton*) 
	{
		wchar_t text[256];

		if (!gg->isonline()) {
			MessageBox(nullptr,
				TranslateT("You have to be logged in before you can change your details."),
				gg->m_tszUserName, MB_OK | MB_ICONSTOP);
			return;
		}

		EnableWindow(GetDlgItem(m_hwnd, IDC_SAVE), FALSE);

		gg_pubdir50_t req = gg_pubdir50_new(GG_PUBDIR50_WRITE);
		if (req == nullptr)
			return;

		GetDlgItemText(m_hwnd, IDC_FIRSTNAME, text, _countof(text));
		if (mir_wstrlen(text))
			gg_pubdir50_add(req, GG_PUBDIR50_FIRSTNAME, T2Utf(text));

		GetDlgItemText(m_hwnd, IDC_LASTNAME, text, _countof(text));
		if (mir_wstrlen(text))
			gg_pubdir50_add(req, GG_PUBDIR50_LASTNAME, T2Utf(text));

		GetDlgItemText(m_hwnd, IDC_NICKNAME, text, _countof(text));
		if (mir_wstrlen(text))
			gg_pubdir50_add(req, GG_PUBDIR50_NICKNAME, T2Utf(text));

		GetDlgItemText(m_hwnd, IDC_CITY, text, _countof(text));
		if (mir_wstrlen(text))
			gg_pubdir50_add(req, GG_PUBDIR50_CITY, T2Utf(text));

		// Gadu-Gadu Female <-> Male
		switch (SendDlgItemMessage(m_hwnd, IDC_GENDER, CB_GETCURSEL, 0, 0)) {
		case 1:
			gg_pubdir50_add(req, GG_PUBDIR50_GENDER, GG_PUBDIR50_GENDER_SET_FEMALE);
			break;
		case 2:
			gg_pubdir50_add(req, GG_PUBDIR50_GENDER, GG_PUBDIR50_GENDER_SET_MALE);
			break;
		default:
			gg_pubdir50_add(req, GG_PUBDIR50_GENDER, "");
		}

		GetDlgItemText(m_hwnd, IDC_BIRTHYEAR, text, _countof(text));
		if (mir_wstrlen(text))
			gg_pubdir50_add(req, GG_PUBDIR50_BIRTHYEAR, T2Utf(text));

		GetDlgItemText(m_hwnd, IDC_FAMILYNAME, text, _countof(text));
		if (mir_wstrlen(text))
			gg_pubdir50_add(req, GG_PUBDIR50_FAMILYNAME, T2Utf(text));

		GetDlgItemText(m_hwnd, IDC_CITYORIGIN, text, _countof(text));
		if (mir_wstrlen(text))
			gg_pubdir50_add(req, GG_PUBDIR50_FAMILYCITY, T2Utf(text));

		// Run update
		gg_pubdir50_seq_set(req, GG_SEQ_CHINFO);
		gg->gg_EnterCriticalSection(&gg->sess_mutex, "gg_detailsdlgproc", 35, "sess_mutex", 1);
		gg_pubdir50(gg->m_sess, req);
		gg->gg_LeaveCriticalSection(&gg->sess_mutex, "gg_genoptsdlgproc", 35, 1, "sess_mutex", 1);
		updating = true;

		gg_pubdir50_free(req);
	}
};

int GaduProto::details_init(WPARAM wParam, LPARAM hContact)
{
	int idDialog;

	// View/Change My Details
	if (hContact == NULL) {
		idDialog = IDD_CHINFO_GG;
	}
	// Other user details
	else {
		char* szProto = Proto_GetBaseAccountName(hContact);
		if (szProto == nullptr)
			return 0;
		if (mir_strcmp(szProto, m_szModuleName) || isChatRoom(hContact))
			return 0;
		idDialog = IDD_INFO_GG;
	}

	USERINFOPAGE uip = {};
	uip.flags = ODPF_DONTTRANSLATE | ODPF_UNICODE | ODPF_ICON;
	uip.position = -1900000000;
	uip.pDialog = new GaduUserInfoDlg(this, idDialog);
	uip.szTitle.w = m_tszUserName;
	uip.dwInitParam = LPARAM(g_plugin.getIconHandle(IDI_GG));
	g_plugin.addUserInfo(wParam, &uip);

	// Start search for user data
	if (hContact == NULL)
		GetInfo(NULL, 0);

	return 0;
}
