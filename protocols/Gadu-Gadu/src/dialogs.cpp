////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2003-2006 Adam Strzelecki <ono+miranda@java.pl>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
////////////////////////////////////////////////////////////////////////////////

#include "gg.h"

extern INT_PTR CALLBACK gg_userutildlgproc(HWND m_hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define SVS_NORMAL       0
#define SVS_GENDER       1
#define SVS_ZEROISUNSPEC 2
#define SVS_IP           3
#define SVS_COUNTRY      4
#define SVS_MONTH        5
#define SVS_SIGNED       6
#define SVS_TIMEZONE     7
#define SVS_GGVERSION    9

////////////////////////////////////////////////////////////////////////////////
// Check if new user data has been filled in for specified account
//

void GaduProto::checknewuser(uin_t uin, const char* passwd)
{
	char oldpasswd[128];
	DBVARIANT dbv;

	oldpasswd[0] = '\0';
	if (!getString(GG_KEY_PASSWORD, &dbv)) {
		if (dbv.pszVal)
			mir_strcpy(oldpasswd, dbv.pszVal);
		db_free(&dbv);
	}

	uin_t olduin = (uin_t)getDword(GG_KEY_UIN, 0);
	if (uin > 0 && mir_strlen(passwd) > 0 && (uin != olduin || mir_strcmp(oldpasswd, passwd)))
		check_first_conn = 1;
}

////////////////////////////////////////////////////////////////////////////////
// Options Page : Proc
//
static void gg_optsdlgcheck(HWND m_hwnd)
{
	wchar_t text[128];
	GetDlgItemText(m_hwnd, IDC_UIN, text, _countof(text));
	if (text[0]) {
		GetDlgItemText(m_hwnd, IDC_EMAIL, text, _countof(text));
		if (text[0])
			ShowWindow(GetDlgItem(m_hwnd, IDC_CHEMAIL), SW_SHOW);
		else
			ShowWindow(GetDlgItem(m_hwnd, IDC_CHEMAIL), SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_CHPASS), SW_SHOW);
		ShowWindow(GetDlgItem(m_hwnd, IDC_LOSTPASS), SW_SHOW);
		ShowWindow(GetDlgItem(m_hwnd, IDC_REMOVEACCOUNT), SW_SHOW);
		ShowWindow(GetDlgItem(m_hwnd, IDC_CREATEACCOUNT), SW_HIDE);
	}
	else {
		ShowWindow(GetDlgItem(m_hwnd, IDC_REMOVEACCOUNT), SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_LOSTPASS), SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_CHPASS), SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_CHEMAIL), SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_CREATEACCOUNT), SW_SHOW);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
// Proc: General options dialog
//
static INT_PTR CALLBACK gg_genoptsdlgproc(HWND m_hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	GaduProto *gg = (GaduProto *)GetWindowLongPtr(m_hwnd, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		{
			DBVARIANT dbv;
			uint32_t num;
			gg = (GaduProto *)lParam;
			SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)lParam);

			TranslateDialogDefault(m_hwnd);
			if (num = gg->getDword(GG_KEY_UIN, 0)) {
				SetDlgItemTextA(m_hwnd, IDC_UIN, ditoa(num));
				ShowWindow(GetDlgItem(m_hwnd, IDC_CREATEACCOUNT), SW_HIDE);
			}
			else {
				ShowWindow(GetDlgItem(m_hwnd, IDC_CHPASS), SW_HIDE);
				ShowWindow(GetDlgItem(m_hwnd, IDC_REMOVEACCOUNT), SW_HIDE);
				ShowWindow(GetDlgItem(m_hwnd, IDC_LOSTPASS), SW_HIDE);
			}
			if (!gg->getString(GG_KEY_PASSWORD, &dbv)) {
				SetDlgItemTextA(m_hwnd, IDC_PASSWORD, dbv.pszVal);
				db_free(&dbv);
			}
			if (!gg->getString(GG_KEY_EMAIL, &dbv)) {
				SetDlgItemTextA(m_hwnd, IDC_EMAIL, dbv.pszVal);
				db_free(&dbv);
			}
			else {
				ShowWindow(GetDlgItem(m_hwnd, IDC_LOSTPASS), SW_HIDE);
				ShowWindow(GetDlgItem(m_hwnd, IDC_CHPASS), SW_HIDE);
			}

			CheckDlgButton(m_hwnd, IDC_FRIENDSONLY, gg->getByte(GG_KEY_FRIENDSONLY, GG_KEYDEF_FRIENDSONLY) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_SHOWINVISIBLE, gg->getByte(GG_KEY_SHOWINVISIBLE, GG_KEYDEF_SHOWINVISIBLE) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_LEAVESTATUSMSG, gg->getByte(GG_KEY_LEAVESTATUSMSG, GG_KEYDEF_LEAVESTATUSMSG) ? BST_CHECKED : BST_UNCHECKED);
			if (gg->gc_enabled)
				CheckDlgButton(m_hwnd, IDC_IGNORECONF, gg->getByte(GG_KEY_IGNORECONF, GG_KEYDEF_IGNORECONF) ? BST_CHECKED : BST_UNCHECKED);
			else {
				EnableWindow(GetDlgItem(m_hwnd, IDC_IGNORECONF), FALSE);
				CheckDlgButton(m_hwnd, IDC_IGNORECONF, BST_CHECKED);
			}
			CheckDlgButton(m_hwnd, IDC_IMGRECEIVE, gg->getByte(GG_KEY_IMGRECEIVE, GG_KEYDEF_IMGRECEIVE) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_SHOWLINKS, gg->getByte(GG_KEY_SHOWLINKS, GG_KEYDEF_SHOWLINKS) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(m_hwnd, IDC_ENABLEAVATARS, gg->getByte(GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS) ? BST_CHECKED : BST_UNCHECKED);

			EnableWindow(GetDlgItem(m_hwnd, IDC_LEAVESTATUS), IsDlgButtonChecked(m_hwnd, IDC_LEAVESTATUSMSG));
			EnableWindow(GetDlgItem(m_hwnd, IDC_IMGMETHOD), IsDlgButtonChecked(m_hwnd, IDC_IMGRECEIVE));
			SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)TranslateT("<Last Status>"));	// 0
			SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)Clist_GetStatusModeDescription(ID_STATUS_ONLINE, 0));
			SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)Clist_GetStatusModeDescription(ID_STATUS_AWAY, 0));
			SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)Clist_GetStatusModeDescription(ID_STATUS_DND, 0));
			SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)Clist_GetStatusModeDescription(ID_STATUS_FREECHAT, 0));
			SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)Clist_GetStatusModeDescription(ID_STATUS_INVISIBLE, 0));

			switch (gg->getWord(GG_KEY_LEAVESTATUS, GG_KEYDEF_LEAVESTATUS)) {
			case ID_STATUS_ONLINE:
				SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_SETCURSEL, 1, 0);
				break;
			case ID_STATUS_AWAY:
				SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_SETCURSEL, 2, 0);
				break;
			case ID_STATUS_DND:
				SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_SETCURSEL, 3, 0);
				break;
			case ID_STATUS_FREECHAT:
				SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_SETCURSEL, 4, 0);
				break;
			case ID_STATUS_INVISIBLE:
				SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_SETCURSEL, 5, 0);
				break;
			default:
				SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_SETCURSEL, 0, 0);
			}

			SendDlgItemMessage(m_hwnd, IDC_IMGMETHOD, CB_ADDSTRING, 0, (LPARAM)TranslateT("System tray icon"));
			SendDlgItemMessage(m_hwnd, IDC_IMGMETHOD, CB_ADDSTRING, 0, (LPARAM)TranslateT("Popup window"));
			SendDlgItemMessage(m_hwnd, IDC_IMGMETHOD, CB_ADDSTRING, 0, (LPARAM)TranslateT("Message with [img] BBCode"));
			SendDlgItemMessage(m_hwnd, IDC_IMGMETHOD, CB_SETCURSEL, gg->getByte(GG_KEY_IMGMETHOD, GG_KEYDEF_IMGMETHOD), 0);
		}
		break;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDC_UIN || LOWORD(wParam) == IDC_PASSWORD || LOWORD(wParam) == IDC_EMAIL)
			&& (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;

		switch (LOWORD(wParam)) {
		case IDC_EMAIL:
		case IDC_UIN:
			gg_optsdlgcheck(m_hwnd);
			break;

		case IDC_LEAVESTATUSMSG:
			EnableWindow(GetDlgItem(m_hwnd, IDC_LEAVESTATUS), IsDlgButtonChecked(m_hwnd, IDC_LEAVESTATUSMSG));
			break;

		case IDC_IMGRECEIVE:
			EnableWindow(GetDlgItem(m_hwnd, IDC_IMGMETHOD), IsDlgButtonChecked(m_hwnd, IDC_IMGRECEIVE));
			break;

		case IDC_LOSTPASS:
			{
				char email[128];
				uin_t uin;
				GetDlgItemTextA(m_hwnd, IDC_UIN, email, _countof(email));
				uin = atoi(email);
				GetDlgItemTextA(m_hwnd, IDC_EMAIL, email, _countof(email));
				if (!mir_strlen(email))
					MessageBox(nullptr, TranslateT("You need to specify your registration e-mail first."),
						gg->m_tszUserName, MB_OK | MB_ICONEXCLAMATION);
				else if (MessageBox(nullptr,
					TranslateT("Your password will be sent to your registration e-mail.\nDo you want to continue?"),
					gg->m_tszUserName,
					MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
					gg->remindpassword(uin, email);
				return FALSE;
			}
		case IDC_CREATEACCOUNT:
		case IDC_REMOVEACCOUNT:
			if (gg->isonline()) {
				if (MessageBox(
					nullptr,
					TranslateT("You should disconnect before making any permanent changes with your account.\nDo you want to disconnect now?"),
					gg->m_tszUserName,
					MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
					break;
				else
					gg->disconnect();
			}
		case IDC_CHPASS:
		case IDC_CHEMAIL:
			{
				// Readup data
				GGUSERUTILDLGDATA dat;
				int ret;
				char pass[128], email[128];
				GetDlgItemTextA(m_hwnd, IDC_UIN, pass, _countof(pass));
				dat.uin = atoi(pass);
				GetDlgItemTextA(m_hwnd, IDC_PASSWORD, pass, _countof(pass));
				GetDlgItemTextA(m_hwnd, IDC_EMAIL, email, _countof(email));
				dat.pass = pass;
				dat.email = email;
				dat.gg = gg;
				if (LOWORD(wParam) == IDC_CREATEACCOUNT) {
					dat.mode = GG_USERUTIL_CREATE;
					ret = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CREATEACCOUNT), m_hwnd, gg_userutildlgproc, (LPARAM)&dat);
				}
				else if (LOWORD(wParam) == IDC_CHPASS) {
					dat.mode = GG_USERUTIL_PASS;
					ret = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CHPASS), m_hwnd, gg_userutildlgproc, (LPARAM)&dat);
				}
				else if (LOWORD(wParam) == IDC_CHEMAIL) {
					dat.mode = GG_USERUTIL_EMAIL;
					ret = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CHEMAIL), m_hwnd, gg_userutildlgproc, (LPARAM)&dat);
				}
				else {
					dat.mode = GG_USERUTIL_REMOVE;
					ret = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_REMOVEACCOUNT), m_hwnd, gg_userutildlgproc, (LPARAM)&dat);
				}

				if (ret == IDOK) {
					DBVARIANT dbv;
					uint32_t num;
					// Show reload required window
					ShowWindow(GetDlgItem(m_hwnd, IDC_RELOADREQD), SW_SHOW);

					// Update uin
					if (num = gg->getDword(GG_KEY_UIN, 0))
						SetDlgItemTextA(m_hwnd, IDC_UIN, ditoa(num));
					else
						SetDlgItemTextA(m_hwnd, IDC_UIN, "");

					// Update password
					if (!gg->getString(GG_KEY_PASSWORD, &dbv)) {
						SetDlgItemTextA(m_hwnd, IDC_PASSWORD, dbv.pszVal);
						db_free(&dbv);
					}
					else SetDlgItemTextA(m_hwnd, IDC_PASSWORD, "");

					// Update e-mail
					if (!gg->getString(GG_KEY_EMAIL, &dbv)) {
						SetDlgItemTextA(m_hwnd, IDC_EMAIL, dbv.pszVal);
						db_free(&dbv);
					}
					else SetDlgItemTextA(m_hwnd, IDC_EMAIL, "");

					// Update links
					gg_optsdlgcheck(m_hwnd);

					// Remove details
					if (LOWORD(wParam) != IDC_CHPASS && LOWORD(wParam) != IDC_CHEMAIL) {
						gg->delSetting(GG_KEY_NICK);
						gg->delSetting(GG_KEY_PD_NICKNAME);
						gg->delSetting(GG_KEY_PD_CITY);
						gg->delSetting(GG_KEY_PD_FIRSTNAME);
						gg->delSetting(GG_KEY_PD_LASTNAME);
						gg->delSetting(GG_KEY_PD_FAMILYNAME);
						gg->delSetting(GG_KEY_PD_FAMILYCITY);
						gg->delSetting(GG_KEY_PD_AGE);
						gg->delSetting(GG_KEY_PD_BIRTHYEAR);
						gg->delSetting(GG_KEY_PD_GANDER);
					}
				}
			}
			break;
		}
		SendMessage(GetParent(m_hwnd), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			int status_flags = GG_STATUS_FLAG_UNKNOWN;
			char str[128];

			// Write Gadu-Gadu number & password
			GetDlgItemTextA(m_hwnd, IDC_UIN, str, _countof(str));
			uin_t uin = atoi(str);
			GetDlgItemTextA(m_hwnd, IDC_PASSWORD, str, _countof(str));
			gg->checknewuser(uin, str);
			gg->setDword(GG_KEY_UIN, uin);
			gg->setString(GG_KEY_PASSWORD, str);

			// Write Gadu-Gadu email
			GetDlgItemTextA(m_hwnd, IDC_EMAIL, str, _countof(str));
			gg->setString(GG_KEY_EMAIL, str);

			// Write checkboxes
			gg->setByte(GG_KEY_FRIENDSONLY, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_FRIENDSONLY));
			gg->setByte(GG_KEY_SHOWINVISIBLE, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_SHOWINVISIBLE));
			gg->setByte(GG_KEY_LEAVESTATUSMSG, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_LEAVESTATUSMSG));
			if (gg->gc_enabled)
				gg->setByte(GG_KEY_IGNORECONF, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_IGNORECONF));
			gg->setByte(GG_KEY_IMGRECEIVE, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_IMGRECEIVE));
			gg->setByte(GG_KEY_SHOWLINKS, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_SHOWLINKS));
			if (IsDlgButtonChecked(m_hwnd, IDC_SHOWLINKS))
				status_flags |= GG_STATUS_FLAG_SPAM;
			gg->gg_EnterCriticalSection(&gg->sess_mutex, "gg_genoptsdlgproc", 34, "sess_mutex", 1);
			gg_change_status_flags(gg->m_sess, status_flags);
			gg->gg_LeaveCriticalSection(&gg->sess_mutex, "gg_genoptsdlgproc", 34, 1, "sess_mutex", 1);
			gg->setByte(GG_KEY_ENABLEAVATARS, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_ENABLEAVATARS));

			gg->setByte(GG_KEY_IMGMETHOD, (uint8_t)SendDlgItemMessage(m_hwnd, IDC_IMGMETHOD, CB_GETCURSEL, 0, 0));

			// Write leave status
			switch (SendDlgItemMessage(m_hwnd, IDC_LEAVESTATUS, CB_GETCURSEL, 0, 0)) {
			case 1:
				gg->setWord(GG_KEY_LEAVESTATUS, ID_STATUS_ONLINE);
				break;
			case 2:
				gg->setWord(GG_KEY_LEAVESTATUS, ID_STATUS_AWAY);
				break;
			case 3:
				gg->setWord(GG_KEY_LEAVESTATUS, ID_STATUS_DND);
				break;
			case 4:
				gg->setWord(GG_KEY_LEAVESTATUS, ID_STATUS_FREECHAT);
				break;
			case 5:
				gg->setWord(GG_KEY_LEAVESTATUS, ID_STATUS_INVISIBLE);
				break;
			default:
				gg->setWord(GG_KEY_LEAVESTATUS, GG_KEYDEF_LEAVESTATUS);
			}
		}
		break;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// Options Page : Init
//
int GaduProto::options_init(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_UNICODE;
	odp.position = 1003000;
	odp.szGroup.w = LPGENW("Network");
	odp.szTitle.w = m_tszUserName;
	odp.dwInitParam = (LPARAM)this;
	odp.flags = ODPF_UNICODE | ODPF_BOLDGROUPS | ODPF_DONTTRANSLATE;

	odp.szTab.w = LPGENW("General");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_GG_GENERAL);
	odp.pfnDlgProc = gg_genoptsdlgproc;
	g_plugin.addOptions(wParam, &odp);

	odp.pszTemplate = nullptr;

	odp.szTab.w = LPGENW("Conference");
	odp.position = 1;
	odp.pDialog = new GaduOptionsDlgConference(this);
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Advanced");
	odp.position = 2;
	odp.pDialog = new GaduOptionsDlgAdvanced(this);
	g_plugin.addOptions(wParam, &odp);

	return 0;
}

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
	uip.flags = ODPF_DONTTRANSLATE | ODPF_UNICODE;
	uip.position = -1900000000;
	uip.pDialog = new GaduUserInfoDlg(this, idDialog);
	uip.szTitle.w = m_tszUserName;
	g_plugin.addUserInfo(wParam, &uip);

	// Start search for user data
	if (hContact == NULL)
		GetInfo(NULL, 0);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
// Proc: Account manager options dialog
//
INT_PTR CALLBACK gg_acc_mgr_guidlgproc(HWND m_hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	GaduProto *gg = (GaduProto *)GetWindowLongPtr(m_hwnd, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		{
			DBVARIANT dbv;
			gg = (GaduProto *)lParam;
			SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)lParam);

			TranslateDialogDefault(m_hwnd);
			uint32_t num = gg->getDword(GG_KEY_UIN, 0);
			if (num)
				SetDlgItemTextA(m_hwnd, IDC_UIN, ditoa(num));
			if (!gg->getString(GG_KEY_PASSWORD, &dbv)) {
				SetDlgItemTextA(m_hwnd, IDC_PASSWORD, dbv.pszVal);
				db_free(&dbv);
			}
			if (!gg->getString(GG_KEY_EMAIL, &dbv)) {
				SetDlgItemTextA(m_hwnd, IDC_EMAIL, dbv.pszVal);
				db_free(&dbv);
			}
			break;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CREATEACCOUNT:
			{
				// Readup data
				GGUSERUTILDLGDATA dat;
				char pass[128], email[128];
				GetDlgItemTextA(m_hwnd, IDC_UIN, pass, _countof(pass));
				dat.uin = atoi(pass);
				GetDlgItemTextA(m_hwnd, IDC_PASSWORD, pass, _countof(pass));
				GetDlgItemTextA(m_hwnd, IDC_EMAIL, email, _countof(email));
				dat.pass = pass;
				dat.email = email;
				dat.gg = gg;
				dat.mode = GG_USERUTIL_CREATE;
				int ret = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CREATEACCOUNT), m_hwnd, gg_userutildlgproc, (LPARAM)&dat);

				if (ret == IDOK) {
					DBVARIANT dbv;
					uint32_t num;
					// Show reload required window
					ShowWindow(GetDlgItem(m_hwnd, IDC_RELOADREQD), SW_SHOW);

					// Update uin
					if (num = gg->getDword(GG_KEY_UIN, 0))
						SetDlgItemTextA(m_hwnd, IDC_UIN, ditoa(num));
					else
						SetDlgItemTextA(m_hwnd, IDC_UIN, "");

					// Update password
					if (!gg->getString(GG_KEY_PASSWORD, &dbv)) {
						SetDlgItemTextA(m_hwnd, IDC_PASSWORD, dbv.pszVal);
						db_free(&dbv);
					}
					else SetDlgItemTextA(m_hwnd, IDC_PASSWORD, "");

					// Update e-mail
					if (!gg->getString(GG_KEY_EMAIL, &dbv)) {
						SetDlgItemTextA(m_hwnd, IDC_EMAIL, dbv.pszVal);
						db_free(&dbv);
					}
					else SetDlgItemTextA(m_hwnd, IDC_EMAIL, "");
				}
			}
			break;
		
		case IDC_UIN:
		case IDC_PASSWORD:
		case IDC_EMAIL:
			if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus()) {
				SendMessage(GetParent(m_hwnd), PSM_CHANGED, 0, 0);
				break;
			}
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				char str[128];

				// Write Gadu-Gadu number & password
				GetDlgItemTextA(m_hwnd, IDC_UIN, str, _countof(str));
				uin_t uin = atoi(str);
				GetDlgItemTextA(m_hwnd, IDC_PASSWORD, str, _countof(str));
				gg->checknewuser(uin, str);
				gg->setDword(GG_KEY_UIN, uin);
				gg->setString(GG_KEY_PASSWORD, str);

				// Write Gadu-Gadu email
				GetDlgItemTextA(m_hwnd, IDC_EMAIL, str, _countof(str));
				gg->setString(GG_KEY_EMAIL, str);
			}
		}
		break;
	}
	return FALSE;
}
