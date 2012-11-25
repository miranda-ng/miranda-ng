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

static INT_PTR CALLBACK gg_genoptsdlgproc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK gg_confoptsdlgproc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK gg_advoptsdlgproc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK gg_userutildlgproc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

////////////////////////////////////////////////////////////////////////////////
// SetValue

#define SVS_NORMAL       0
#define SVS_GENDER       1
#define SVS_ZEROISUNSPEC 2
#define SVS_IP           3
#define SVS_COUNTRY      4
#define SVS_MONTH        5
#define SVS_SIGNED       6
#define SVS_TIMEZONE     7
#define SVS_GGVERSION    9

static void SetValue(HWND hwndDlg, int idCtrl, HANDLE hContact, char *szModule, char *szSetting, int special, int disableIfUndef)
{
	DBVARIANT dbv = {0};
	char str[80], *pstr = NULL;
	int unspecified = 0;

	dbv.type = DBVT_DELETED;
	if (szModule == NULL) unspecified = 1;
	else unspecified = DBGetContactSettingW(hContact, szModule, szSetting, &dbv);
	if (!unspecified) {
		switch (dbv.type) {
			case DBVT_BYTE:
				if (special == SVS_GENDER) {
					if (dbv.cVal == 'M') pstr = Translate("Male");
					else if (dbv.cVal == 'F') pstr = Translate("Female");
					else unspecified = 1;
				}
				else if (special == SVS_MONTH) {
					if (dbv.bVal > 0 && dbv.bVal <= 12) {
						pstr = str;
						GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SABBREVMONTHNAME1 - 1 + dbv.bVal, str, SIZEOF(str));
					}
					else unspecified = 1;
				}
				else if (special == SVS_TIMEZONE) {
					if (dbv.cVal == -100) unspecified = 1;
					else {
						pstr = str;
						mir_snprintf(str, SIZEOF(str), dbv.cVal ? "GMT%+d:%02d" : "GMT", -dbv.cVal / 2, (dbv.cVal & 1) * 30);
					}
				}
				else {
					unspecified = (special == SVS_ZEROISUNSPEC && dbv.bVal == 0);
					pstr = _itoa(special == SVS_SIGNED ? dbv.cVal : dbv.bVal, str, 10);
				}
				break;
			case DBVT_WORD:
				if (special == SVS_COUNTRY) {
					pstr = (char*)CallService(MS_UTILS_GETCOUNTRYBYNUMBER, dbv.wVal, 0);
					unspecified = pstr == NULL;
				}
				else {
					unspecified = (special == SVS_ZEROISUNSPEC && dbv.wVal == 0);
					pstr = _itoa(special == SVS_SIGNED ? dbv.sVal : dbv.wVal, str, 10);
				}
				break;
			case DBVT_DWORD:
				unspecified = (special == SVS_ZEROISUNSPEC && dbv.dVal == 0);
				if (special == SVS_IP) {
					struct in_addr ia;
					ia.S_un.S_addr = htonl(dbv.dVal);
					pstr = inet_ntoa(ia);
					if (dbv.dVal == 0) unspecified = 1;
				}
				else if (special == SVS_GGVERSION)
					pstr = (char *)gg_version2string(dbv.dVal);
				else
					pstr = _itoa(special == SVS_SIGNED ? dbv.lVal : dbv.dVal, str, 10);
				break;
			case DBVT_ASCIIZ:
				unspecified = (special == SVS_ZEROISUNSPEC && dbv.pszVal[0] == '\0');
				pstr = dbv.pszVal;
				break;
			default: pstr = str; lstrcpyA(str, "???"); break;
		}
	}

	if (disableIfUndef) {
		EnableWindow(GetDlgItem(hwndDlg, idCtrl), !unspecified);
		if (unspecified)
			SetDlgItemText(hwndDlg, idCtrl, TranslateT("<not specified>"));
		else
			SetDlgItemTextA(hwndDlg, idCtrl, pstr);
	}
	else {
		EnableWindow(GetDlgItem(hwndDlg, idCtrl), TRUE);
		if (!unspecified)
			SetDlgItemTextA(hwndDlg, idCtrl, pstr);
	}
	DBFreeVariant(&dbv);
}

////////////////////////////////////////////////////////////////////////////////
// Options Page : Init

int GGPROTO::options_init(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize = sizeof(odp);
	odp.flags = ODPF_TCHAR;
	odp.position = 1003000;
	odp.hInstance = hInstance;
	odp.ptszGroup = LPGENT("Network");
	odp.ptszTitle = m_tszUserName;
	odp.dwInitParam = (LPARAM)this;

	odp.ptszTab = LPGENT("General");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_GG_GENERAL);
	odp.pfnDlgProc = gg_genoptsdlgproc;
	odp.flags = ODPF_TCHAR | ODPF_BOLDGROUPS | ODPF_DONTTRANSLATE;
	Options_AddPage(wParam, &odp);

	odp.ptszTab = LPGENT("Conference");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_GG_CONFERENCE);
	odp.pfnDlgProc = gg_confoptsdlgproc;
	Options_AddPage(wParam, &odp);

	odp.ptszTab = LPGENT("Advanced");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_GG_ADVANCED);
	odp.pfnDlgProc = gg_advoptsdlgproc;
	Options_AddPage(wParam, &odp);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Check if new user data has been filled in for specified account
void GGPROTO::checknewuser(uin_t uin, const char* passwd)
{
	char oldpasswd[128];
	DBVARIANT dbv;
	uin_t olduin = (uin_t)db_get_dw(NULL, m_szModuleName, GG_KEY_UIN, 0);

	oldpasswd[0] = '\0';
	if (!db_get_s(NULL, m_szModuleName, GG_KEY_PASSWORD, &dbv, DBVT_ASCIIZ))
	{
		if (dbv.pszVal) strcpy(oldpasswd, dbv.pszVal);
		DBFreeVariant(&dbv);
	}

	if (uin > 0 && strlen(passwd) > 0 && (uin != olduin || strcmp(oldpasswd, passwd)))
		check_first_conn = 1;
}

////////////////////////////////////////////////////////////////////////////////
// Options Page : Proc

static void gg_optsdlgcheck(HWND hwndDlg)
{
	TCHAR text[128];
	GetDlgItemText(hwndDlg, IDC_UIN, text, SIZEOF(text));
	if (text[0]) {
		GetDlgItemText(hwndDlg, IDC_EMAIL, text, SIZEOF(text));
		if (text[0])
			ShowWindow(GetDlgItem(hwndDlg, IDC_CHEMAIL), SW_SHOW);
		else
			ShowWindow(GetDlgItem(hwndDlg, IDC_CHEMAIL), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_CHPASS), SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_LOSTPASS), SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_REMOVEACCOUNT), SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_CREATEACCOUNT), SW_HIDE);
	}
	else {
		ShowWindow(GetDlgItem(hwndDlg, IDC_REMOVEACCOUNT), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_LOSTPASS), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_CHPASS), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_CHEMAIL), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_CREATEACCOUNT), SW_SHOW);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
// Proc: General options dialog
static INT_PTR CALLBACK gg_genoptsdlgproc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	GGPROTO *gg = (GGPROTO *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		{
			DBVARIANT dbv;
			DWORD num;
			GGPROTO *gg = (GGPROTO *)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);

			TranslateDialogDefault(hwndDlg);
			if (num = db_get_dw(NULL, gg->m_szModuleName, GG_KEY_UIN, 0))
			{
				SetDlgItemTextA(hwndDlg, IDC_UIN, ditoa(num));
				ShowWindow(GetDlgItem(hwndDlg, IDC_CREATEACCOUNT), SW_HIDE);
			}
			else
			{
				ShowWindow(GetDlgItem(hwndDlg, IDC_CHPASS), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_REMOVEACCOUNT), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_LOSTPASS), SW_HIDE);
			}
			if (!db_get_s(NULL, gg->m_szModuleName, GG_KEY_PASSWORD, &dbv, DBVT_ASCIIZ)) {
				CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal) + 1, (LPARAM) dbv.pszVal);
				SetDlgItemTextA(hwndDlg, IDC_PASSWORD, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			if (!db_get_s(NULL, gg->m_szModuleName, GG_KEY_EMAIL, &dbv, DBVT_ASCIIZ)) {
				SetDlgItemTextA(hwndDlg, IDC_EMAIL, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			else
			{
				ShowWindow(GetDlgItem(hwndDlg, IDC_LOSTPASS), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_CHPASS), SW_HIDE);
			}

			CheckDlgButton(hwndDlg, IDC_FRIENDSONLY, db_get_b(NULL, gg->m_szModuleName, GG_KEY_FRIENDSONLY, GG_KEYDEF_FRIENDSONLY));
			CheckDlgButton(hwndDlg, IDC_SHOWINVISIBLE, db_get_b(NULL, gg->m_szModuleName, GG_KEY_SHOWINVISIBLE, GG_KEYDEF_SHOWINVISIBLE));
			CheckDlgButton(hwndDlg, IDC_LEAVESTATUSMSG, db_get_b(NULL, gg->m_szModuleName, GG_KEY_LEAVESTATUSMSG, GG_KEYDEF_LEAVESTATUSMSG));
			if (gg->gc_enabled)
				CheckDlgButton(hwndDlg, IDC_IGNORECONF, db_get_b(NULL, gg->m_szModuleName, GG_KEY_IGNORECONF, GG_KEYDEF_IGNORECONF));
			else
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_IGNORECONF), FALSE);
				CheckDlgButton(hwndDlg, IDC_IGNORECONF, TRUE);
			}
			CheckDlgButton(hwndDlg, IDC_IMGRECEIVE, db_get_b(NULL, gg->m_szModuleName, GG_KEY_IMGRECEIVE, GG_KEYDEF_IMGRECEIVE));
			CheckDlgButton(hwndDlg, IDC_SHOWLINKS, db_get_b(NULL, gg->m_szModuleName, GG_KEY_SHOWLINKS, GG_KEYDEF_SHOWLINKS));
			CheckDlgButton(hwndDlg, IDC_ENABLEAVATARS, db_get_b(NULL, gg->m_szModuleName, GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS));

			EnableWindow(GetDlgItem(hwndDlg, IDC_LEAVESTATUS), IsDlgButtonChecked(hwndDlg, IDC_LEAVESTATUSMSG));
			EnableWindow(GetDlgItem(hwndDlg, IDC_IMGMETHOD), IsDlgButtonChecked(hwndDlg, IDC_IMGRECEIVE));
			SendDlgItemMessage(hwndDlg, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)TranslateT("<Last Status>"));	// 0
			SendDlgItemMessage(hwndDlg, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Online")); 		// ID_STATUS_ONLINE
			SendDlgItemMessage(hwndDlg, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Away"));			// ID_STATUS_AWAY
			SendDlgItemMessage(hwndDlg, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)TranslateT("DND"));			// ID_STATUS_DND
			SendDlgItemMessage(hwndDlg, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Free for chat"));	// ID_STATUS_FREECHAT
			SendDlgItemMessage(hwndDlg, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Invisible"));		// ID_STATUS_INVISIBLE
			switch(db_get_w(NULL, gg->m_szModuleName, GG_KEY_LEAVESTATUS, GG_KEYDEF_LEAVESTATUS)) {
			case ID_STATUS_ONLINE:
				SendDlgItemMessage(hwndDlg, IDC_LEAVESTATUS, CB_SETCURSEL, 1, 0);
				break;
			case ID_STATUS_AWAY:
				SendDlgItemMessage(hwndDlg, IDC_LEAVESTATUS, CB_SETCURSEL, 2, 0);
				break;
			case ID_STATUS_DND:
				SendDlgItemMessage(hwndDlg, IDC_LEAVESTATUS, CB_SETCURSEL, 3, 0);
				break;
			case ID_STATUS_FREECHAT:
				SendDlgItemMessage(hwndDlg, IDC_LEAVESTATUS, CB_SETCURSEL, 4, 0);
				break;
			case ID_STATUS_INVISIBLE:
				SendDlgItemMessage(hwndDlg, IDC_LEAVESTATUS, CB_SETCURSEL, 5, 0);
				break;
			default:
				SendDlgItemMessage(hwndDlg, IDC_LEAVESTATUS, CB_SETCURSEL, 0, 0);
			}

			SendDlgItemMessage(hwndDlg, IDC_IMGMETHOD, CB_ADDSTRING, 0, (LPARAM)TranslateT("System tray icon"));
			SendDlgItemMessage(hwndDlg, IDC_IMGMETHOD, CB_ADDSTRING, 0, (LPARAM)TranslateT("Popup window"));
			SendDlgItemMessage(hwndDlg, IDC_IMGMETHOD, CB_ADDSTRING, 0, (LPARAM)TranslateT("Message with [img] BBCode"));
			SendDlgItemMessage(hwndDlg, IDC_IMGMETHOD, CB_SETCURSEL,
				db_get_b(NULL, gg->m_szModuleName, GG_KEY_IMGMETHOD, GG_KEYDEF_IMGMETHOD), 0);
			break;
		}
	case WM_COMMAND:
		{
			if ((LOWORD(wParam) == IDC_UIN || LOWORD(wParam) == IDC_PASSWORD || LOWORD(wParam) == IDC_EMAIL)
				&& (HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus()))
				return 0;

			switch (LOWORD(wParam)) {
			case IDC_EMAIL:
			case IDC_UIN:
				gg_optsdlgcheck(hwndDlg);
				break;

			case IDC_LEAVESTATUSMSG:
				EnableWindow(GetDlgItem(hwndDlg, IDC_LEAVESTATUS), IsDlgButtonChecked(hwndDlg, IDC_LEAVESTATUSMSG));
				break;

			case IDC_IMGRECEIVE:
				EnableWindow(GetDlgItem(hwndDlg, IDC_IMGMETHOD), IsDlgButtonChecked(hwndDlg, IDC_IMGRECEIVE));
				break;

			case IDC_LOSTPASS:
				{
					char email[128];
					uin_t uin;
					GetDlgItemTextA(hwndDlg, IDC_UIN, email, sizeof(email));
					uin = atoi(email);
					GetDlgItemTextA(hwndDlg, IDC_EMAIL, email, sizeof(email));
					if (!strlen(email))
						MessageBox(NULL, TranslateT("You need to specify your registration e-mail first."),
						gg->m_tszUserName, MB_OK | MB_ICONEXCLAMATION);
					else if (MessageBox(NULL,
						TranslateT("Your password will be sent to your registration e-mail.\nDo you want to continue ?"),
						gg->m_tszUserName,
						MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
						gg->remindpassword(uin, email);
					return FALSE;
				}
			case IDC_CREATEACCOUNT:
			case IDC_REMOVEACCOUNT:
				if (gg->isonline())
				{
					if (MessageBox(
						NULL,
						TranslateT("You should disconnect before making any permanent changes with your account.\nDo you want to disconnect now ?"),
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
					GetDlgItemTextA(hwndDlg, IDC_UIN, pass, sizeof(pass));
					dat.uin = atoi(pass);
					GetDlgItemTextA(hwndDlg, IDC_PASSWORD, pass, sizeof(pass));
					GetDlgItemTextA(hwndDlg, IDC_EMAIL, email, sizeof(email));
					dat.pass = pass;
					dat.email = email;
					dat.gg = gg;
					if (LOWORD(wParam) == IDC_CREATEACCOUNT)
					{
						dat.mode = GG_USERUTIL_CREATE;
						ret = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CREATEACCOUNT), hwndDlg, gg_userutildlgproc, (LPARAM)&dat);
					}
					else if (LOWORD(wParam) == IDC_CHPASS)
					{
						dat.mode = GG_USERUTIL_PASS;
						ret = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CHPASS), hwndDlg, gg_userutildlgproc, (LPARAM)&dat);
					}
					else if (LOWORD(wParam) == IDC_CHEMAIL)
					{
						dat.mode = GG_USERUTIL_EMAIL;
						ret = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CHEMAIL), hwndDlg, gg_userutildlgproc, (LPARAM)&dat);
					}
					else
					{
						dat.mode = GG_USERUTIL_REMOVE;
						ret = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_REMOVEACCOUNT), hwndDlg, gg_userutildlgproc, (LPARAM)&dat);
					}

					if (ret == IDOK)
					{
						DBVARIANT dbv;
						DWORD num;
						// Show reload required window
						ShowWindow(GetDlgItem(hwndDlg, IDC_RELOADREQD), SW_SHOW);

						// Update uin
						if (num = db_get_dw(NULL, gg->m_szModuleName, GG_KEY_UIN, 0))
							SetDlgItemTextA(hwndDlg, IDC_UIN, ditoa(num));
						else
							SetDlgItemTextA(hwndDlg, IDC_UIN, "");

						// Update password
						if (!db_get_s(NULL, gg->m_szModuleName, GG_KEY_PASSWORD, &dbv, DBVT_ASCIIZ)) {
							CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal) + 1, (LPARAM) dbv.pszVal);
							SetDlgItemTextA(hwndDlg, IDC_PASSWORD, dbv.pszVal);
							DBFreeVariant(&dbv);
						}
						else SetDlgItemTextA(hwndDlg, IDC_PASSWORD, "");

						// Update e-mail
						if (!db_get_s(NULL, gg->m_szModuleName, GG_KEY_EMAIL, &dbv, DBVT_ASCIIZ)) {
							SetDlgItemTextA(hwndDlg, IDC_EMAIL, dbv.pszVal);
							DBFreeVariant(&dbv);
						}
						else SetDlgItemTextA(hwndDlg, IDC_EMAIL, "");

						// Update links
						gg_optsdlgcheck(hwndDlg);

						// Remove details
						if (LOWORD(wParam) != IDC_CHPASS && LOWORD(wParam) != IDC_CHEMAIL)
						{
							db_unset(NULL, gg->m_szModuleName, GG_KEY_NICK);
							db_unset(NULL, gg->m_szModuleName, "NickName");
							db_unset(NULL, gg->m_szModuleName, "City");
							db_unset(NULL, gg->m_szModuleName, "FirstName");
							db_unset(NULL, gg->m_szModuleName, "LastName");
							db_unset(NULL, gg->m_szModuleName, "FamilyName");
							db_unset(NULL, gg->m_szModuleName, "CityOrigin");
							db_unset(NULL, gg->m_szModuleName, "Age");
							db_unset(NULL, gg->m_szModuleName, "BirthYear");
							db_unset(NULL, gg->m_szModuleName, "Gender");
						}
					}
				}
				break;
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
	case WM_NOTIFY:
		{
			switch (((LPNMHDR) lParam)->code) {
			case PSN_APPLY:
				{
					int status_flags = GG_STATUS_FLAG_UNKNOWN;
					char str[128];
					uin_t uin;

					// Write Gadu-Gadu number & password
					GetDlgItemTextA(hwndDlg, IDC_UIN, str, sizeof(str));
					uin = atoi(str);
					GetDlgItemTextA(hwndDlg, IDC_PASSWORD, str, sizeof(str));
					CallService(MS_DB_CRYPT_ENCODESTRING, sizeof(str), (LPARAM) str);
					gg->checknewuser(uin, str);
					db_set_dw(NULL, gg->m_szModuleName, GG_KEY_UIN, uin);
					db_set_s(NULL, gg->m_szModuleName, GG_KEY_PASSWORD, str);

					// Write Gadu-Gadu email
					GetDlgItemTextA(hwndDlg, IDC_EMAIL, str, sizeof(str));
					db_set_s(NULL, gg->m_szModuleName, GG_KEY_EMAIL, str);

					// Write checkboxes
					db_set_b(NULL, gg->m_szModuleName, GG_KEY_FRIENDSONLY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_FRIENDSONLY));
					db_set_b(NULL, gg->m_szModuleName, GG_KEY_SHOWINVISIBLE, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SHOWINVISIBLE));
					db_set_b(NULL, gg->m_szModuleName, GG_KEY_LEAVESTATUSMSG, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_LEAVESTATUSMSG));
					if (gg->gc_enabled)
						db_set_b(NULL, gg->m_szModuleName, GG_KEY_IGNORECONF, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_IGNORECONF));
					db_set_b(NULL, gg->m_szModuleName, GG_KEY_IMGRECEIVE, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_IMGRECEIVE));
					db_set_b(NULL, gg->m_szModuleName, GG_KEY_SHOWLINKS, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SHOWLINKS));
					if (IsDlgButtonChecked(hwndDlg, IDC_SHOWLINKS))
						status_flags |= GG_STATUS_FLAG_SPAM;
					gg->gg_EnterCriticalSection(&gg->sess_mutex, "gg_genoptsdlgproc", 34, "sess_mutex", 1);
					gg_change_status_flags(gg->sess, status_flags);
					gg->gg_LeaveCriticalSection(&gg->sess_mutex, "gg_genoptsdlgproc", 34, 1, "sess_mutex", 1);
					db_set_b(NULL, gg->m_szModuleName, GG_KEY_ENABLEAVATARS, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ENABLEAVATARS));

					db_set_b(NULL, gg->m_szModuleName, GG_KEY_IMGMETHOD,
						(BYTE)SendDlgItemMessage(hwndDlg, IDC_IMGMETHOD, CB_GETCURSEL, 0, 0));

					// Write leave status
					switch(SendDlgItemMessage(hwndDlg, IDC_LEAVESTATUS, CB_GETCURSEL, 0, 0))
					{
					case 1:
						db_set_w(NULL, gg->m_szModuleName, GG_KEY_LEAVESTATUS, ID_STATUS_ONLINE);
						break;
					case 2:
						db_set_w(NULL, gg->m_szModuleName, GG_KEY_LEAVESTATUS, ID_STATUS_AWAY);
						break;
					case 3:
						db_set_w(NULL, gg->m_szModuleName, GG_KEY_LEAVESTATUS, ID_STATUS_DND);
						break;
					case 4:
						db_set_w(NULL, gg->m_szModuleName, GG_KEY_LEAVESTATUS, ID_STATUS_FREECHAT);
						break;
					case 5:
						db_set_w(NULL, gg->m_szModuleName, GG_KEY_LEAVESTATUS, ID_STATUS_INVISIBLE);
						break;
					default:
						db_set_w(NULL, gg->m_szModuleName, GG_KEY_LEAVESTATUS, GG_KEYDEF_LEAVESTATUS);
					}
					break;
				}
			}
			break;
		}
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////
// Proc: Conference options dialog

static INT_PTR CALLBACK gg_confoptsdlgproc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	GGPROTO *gg = (GGPROTO *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		{
			DWORD num;
			GGPROTO *gg = (GGPROTO *)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);

			TranslateDialogDefault(hwndDlg);
			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_TOTAL, CB_ADDSTRING, 0, (LPARAM)TranslateT("Allow"));
			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_TOTAL, CB_ADDSTRING, 0, (LPARAM)TranslateT("Ask"));
			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_TOTAL, CB_ADDSTRING, 0, (LPARAM)TranslateT("Ignore"));
			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_TOTAL, CB_SETCURSEL,
				db_get_w(NULL, gg->m_szModuleName, GG_KEY_GC_POLICY_TOTAL, GG_KEYDEF_GC_POLICY_TOTAL), 0);

			if (num = db_get_w(NULL, gg->m_szModuleName, GG_KEY_GC_COUNT_TOTAL, GG_KEYDEF_GC_COUNT_TOTAL))
				SetDlgItemTextA(hwndDlg, IDC_GC_COUNT_TOTAL, ditoa(num));

			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_UNKNOWN, CB_ADDSTRING, 0, (LPARAM)TranslateT("Allow"));
			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_UNKNOWN, CB_ADDSTRING, 0, (LPARAM)TranslateT("Ask"));
			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_UNKNOWN, CB_ADDSTRING, 0, (LPARAM)TranslateT("Ignore"));
			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_UNKNOWN, CB_SETCURSEL,
				db_get_w(NULL, gg->m_szModuleName, GG_KEY_GC_POLICY_UNKNOWN, GG_KEYDEF_GC_POLICY_UNKNOWN), 0);

			if (num = db_get_w(NULL, gg->m_szModuleName, GG_KEY_GC_COUNT_UNKNOWN, GG_KEYDEF_GC_COUNT_UNKNOWN))
				SetDlgItemTextA(hwndDlg, IDC_GC_COUNT_UNKNOWN, ditoa(num));

			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_DEFAULT, CB_ADDSTRING, 0, (LPARAM)TranslateT("Allow"));
			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_DEFAULT, CB_ADDSTRING, 0, (LPARAM)TranslateT("Ask"));
			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_DEFAULT, CB_ADDSTRING, 0, (LPARAM)TranslateT("Ignore"));
			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_DEFAULT, CB_SETCURSEL,
				db_get_w(NULL, gg->m_szModuleName, GG_KEY_GC_POLICY_DEFAULT, GG_KEYDEF_GC_POLICY_DEFAULT), 0);
			break;
		}
	case WM_COMMAND:
		{
			if ((LOWORD(wParam) == IDC_GC_COUNT_TOTAL || LOWORD(wParam) == IDC_GC_COUNT_UNKNOWN)
				&& (HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus()))
				return 0;
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
	case WM_NOTIFY:
		{
			switch (((LPNMHDR) lParam)->code) {
			case PSN_APPLY:
				{
					char str[128];

					// Write groupchat policy
					db_set_w(NULL, gg->m_szModuleName, GG_KEY_GC_POLICY_TOTAL,
						(WORD)SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_TOTAL, CB_GETCURSEL, 0, 0));
					db_set_w(NULL, gg->m_szModuleName, GG_KEY_GC_POLICY_UNKNOWN,
						(WORD)SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_UNKNOWN, CB_GETCURSEL, 0, 0));
					db_set_w(NULL, gg->m_szModuleName, GG_KEY_GC_POLICY_DEFAULT,
						(WORD)SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_DEFAULT, CB_GETCURSEL, 0, 0));

					GetDlgItemTextA(hwndDlg, IDC_GC_COUNT_TOTAL, str, sizeof(str));
					db_set_w(NULL, gg->m_szModuleName, GG_KEY_GC_COUNT_TOTAL, (WORD)atoi(str));
					GetDlgItemTextA(hwndDlg, IDC_GC_COUNT_UNKNOWN, str, sizeof(str));
					db_set_w(NULL, gg->m_szModuleName, GG_KEY_GC_COUNT_UNKNOWN, (WORD)atoi(str));

					break;
				}
			}
			break;
		}
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////
// Proc: Advanced options dialog
static INT_PTR CALLBACK gg_advoptsdlgproc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	GGPROTO *gg = (GGPROTO *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		{
			DBVARIANT dbv;
			DWORD num;
			GGPROTO *gg = (GGPROTO *)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);

			TranslateDialogDefault(hwndDlg);
			if (!db_get_s(NULL, gg->m_szModuleName, GG_KEY_SERVERHOSTS, &dbv, DBVT_ASCIIZ)) {
				SetDlgItemTextA(hwndDlg, IDC_HOST, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			else SetDlgItemTextA(hwndDlg, IDC_HOST, GG_KEYDEF_SERVERHOSTS);

			CheckDlgButton(hwndDlg, IDC_KEEPALIVE, db_get_b(NULL, gg->m_szModuleName, GG_KEY_KEEPALIVE, GG_KEYDEF_KEEPALIVE));
			CheckDlgButton(hwndDlg, IDC_SHOWCERRORS, db_get_b(NULL, gg->m_szModuleName, GG_KEY_SHOWCERRORS, GG_KEYDEF_SHOWCERRORS));
			CheckDlgButton(hwndDlg, IDC_ARECONNECT, db_get_b(NULL, gg->m_szModuleName, GG_KEY_ARECONNECT, GG_KEYDEF_ARECONNECT));
			CheckDlgButton(hwndDlg, IDC_MSGACK, db_get_b(NULL, gg->m_szModuleName, GG_KEY_MSGACK, GG_KEYDEF_MSGACK));
			CheckDlgButton(hwndDlg, IDC_MANUALHOST, db_get_b(NULL, gg->m_szModuleName, GG_KEY_MANUALHOST, GG_KEYDEF_MANUALHOST));
			CheckDlgButton(hwndDlg, IDC_SSLCONN, db_get_b(NULL, gg->m_szModuleName, GG_KEY_SSLCONN, GG_KEYDEF_SSLCONN));

			EnableWindow(GetDlgItem(hwndDlg, IDC_HOST), IsDlgButtonChecked(hwndDlg, IDC_MANUALHOST));
			EnableWindow(GetDlgItem(hwndDlg, IDC_PORT), IsDlgButtonChecked(hwndDlg, IDC_MANUALHOST));

			CheckDlgButton(hwndDlg, IDC_DIRECTCONNS, db_get_b(NULL, gg->m_szModuleName, GG_KEY_DIRECTCONNS, GG_KEYDEF_DIRECTCONNS));
			if (num = db_get_w(NULL, gg->m_szModuleName, GG_KEY_DIRECTPORT, GG_KEYDEF_DIRECTPORT))
				SetDlgItemTextA(hwndDlg, IDC_DIRECTPORT, ditoa(num));
			CheckDlgButton(hwndDlg, IDC_FORWARDING, db_get_b(NULL, gg->m_szModuleName, GG_KEY_FORWARDING, GG_KEYDEF_FORWARDING));
			if (!db_get_s(NULL, gg->m_szModuleName, GG_KEY_FORWARDHOST, &dbv, DBVT_ASCIIZ)) {
				SetDlgItemTextA(hwndDlg, IDC_FORWARDHOST, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			if (num = db_get_w(NULL, gg->m_szModuleName, GG_KEY_FORWARDPORT, GG_KEYDEF_FORWARDPORT))
				SetDlgItemTextA(hwndDlg, IDC_FORWARDPORT, ditoa(num));

			EnableWindow(GetDlgItem(hwndDlg, IDC_DIRECTPORT), IsDlgButtonChecked(hwndDlg, IDC_DIRECTCONNS));
			EnableWindow(GetDlgItem(hwndDlg, IDC_FORWARDING), IsDlgButtonChecked(hwndDlg, IDC_DIRECTCONNS));
			EnableWindow(GetDlgItem(hwndDlg, IDC_FORWARDPORT), IsDlgButtonChecked(hwndDlg, IDC_FORWARDING) && IsDlgButtonChecked(hwndDlg, IDC_DIRECTCONNS));
			EnableWindow(GetDlgItem(hwndDlg, IDC_FORWARDHOST), IsDlgButtonChecked(hwndDlg, IDC_FORWARDING) && IsDlgButtonChecked(hwndDlg, IDC_DIRECTCONNS));
			break;
		}
	case WM_COMMAND:
		{
			if ((LOWORD(wParam) == IDC_DIRECTPORT || LOWORD(wParam) == IDC_FORWARDHOST || LOWORD(wParam) == IDC_FORWARDPORT)
				&& (HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus()))
				return 0;
			switch (LOWORD(wParam)) {
			case IDC_MANUALHOST:
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_HOST), IsDlgButtonChecked(hwndDlg, IDC_MANUALHOST));
					EnableWindow(GetDlgItem(hwndDlg, IDC_PORT), IsDlgButtonChecked(hwndDlg, IDC_MANUALHOST));
					ShowWindow(GetDlgItem(hwndDlg, IDC_RELOADREQD), SW_SHOW);
					break;
				}
			case IDC_DIRECTCONNS:
			case IDC_FORWARDING:
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_DIRECTPORT), IsDlgButtonChecked(hwndDlg, IDC_DIRECTCONNS));
					EnableWindow(GetDlgItem(hwndDlg, IDC_FORWARDING), IsDlgButtonChecked(hwndDlg, IDC_DIRECTCONNS));
					EnableWindow(GetDlgItem(hwndDlg, IDC_FORWARDPORT), IsDlgButtonChecked(hwndDlg, IDC_FORWARDING) && IsDlgButtonChecked(hwndDlg, IDC_DIRECTCONNS));
					EnableWindow(GetDlgItem(hwndDlg, IDC_FORWARDHOST), IsDlgButtonChecked(hwndDlg, IDC_FORWARDING) && IsDlgButtonChecked(hwndDlg, IDC_DIRECTCONNS));
					ShowWindow(GetDlgItem(hwndDlg, IDC_RELOADREQD), SW_SHOW);
					break;
				}
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
	case WM_NOTIFY:
		{
			switch (((LPNMHDR) lParam)->code) {
			case PSN_APPLY:
				{
					char str[512];
					db_set_b(NULL, gg->m_szModuleName, GG_KEY_KEEPALIVE, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_KEEPALIVE));
					db_set_b(NULL, gg->m_szModuleName, GG_KEY_SHOWCERRORS, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SHOWCERRORS));
					db_set_b(NULL, gg->m_szModuleName, GG_KEY_ARECONNECT, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ARECONNECT));
					db_set_b(NULL, gg->m_szModuleName, GG_KEY_MSGACK, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_MSGACK));
					db_set_b(NULL, gg->m_szModuleName, GG_KEY_MANUALHOST, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_MANUALHOST));
					db_set_b(NULL, gg->m_szModuleName, GG_KEY_SSLCONN, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SSLCONN));

					// Transfer settings
					db_set_b(NULL, gg->m_szModuleName, GG_KEY_DIRECTCONNS, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_DIRECTCONNS));
					db_set_b(NULL, gg->m_szModuleName, GG_KEY_FORWARDING, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_FORWARDING));

					// Write custom servers
					GetDlgItemTextA(hwndDlg, IDC_HOST, str, sizeof(str));
					db_set_s(NULL, gg->m_szModuleName, GG_KEY_SERVERHOSTS, str);

					// Write direct port
					GetDlgItemTextA(hwndDlg, IDC_DIRECTPORT, str, sizeof(str));
					db_set_w(NULL, gg->m_szModuleName, GG_KEY_DIRECTPORT, (WORD)atoi(str));
					// Write forwarding host
					GetDlgItemTextA(hwndDlg, IDC_FORWARDHOST, str, sizeof(str));
					db_set_s(NULL, gg->m_szModuleName, GG_KEY_FORWARDHOST, str);
					GetDlgItemTextA(hwndDlg, IDC_FORWARDPORT, str, sizeof(str));
					db_set_w(NULL, gg->m_szModuleName, GG_KEY_FORWARDPORT, (WORD)atoi(str));
					break;
				}
			}
			break;
		}
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// Info Page : Data
struct GGDETAILSDLGDATA
{
	GGPROTO *gg;
	HANDLE hContact;
	int disableUpdate;
	int updating;
};

////////////////////////////////////////////////////////////////////////////////
// Info Page : Proc
static INT_PTR CALLBACK gg_detailsdlgproc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct GGDETAILSDLGDATA *dat = (struct GGDETAILSDLGDATA *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		dat = (struct GGDETAILSDLGDATA *)mir_alloc(sizeof(struct GGDETAILSDLGDATA));
		dat->hContact=(HANDLE)lParam;
		dat->disableUpdate = FALSE;
		dat->updating = FALSE;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
		// Add genders
		if (!dat->hContact)
		{
			SendDlgItemMessage(hwndDlg, IDC_GENDER, CB_ADDSTRING, 0, (LPARAM)_T(""));				// 0
			SendDlgItemMessage(hwndDlg, IDC_GENDER, CB_ADDSTRING, 0, (LPARAM)TranslateT("Female"));	// 1
			SendDlgItemMessage(hwndDlg, IDC_GENDER, CB_ADDSTRING, 0, (LPARAM)TranslateT("Male"));	// 2
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_PARAMCHANGED:
				dat->gg = (GGPROTO *)((LPPSHNOTIFY)lParam)->lParam;
				break;

			case PSN_INFOCHANGED:
				{
					char *szProto;
					HANDLE hContact = (HANDLE)((LPPSHNOTIFY)lParam)->lParam;
					GGPROTO *gg = dat->gg;

					// Show updated message
					if (dat && dat->updating)
					{
						MessageBox(NULL, TranslateT("Your details has been uploaded to the public directory."),
							gg->m_tszUserName, MB_OK | MB_ICONINFORMATION);
						dat->updating = FALSE;
						break;
					}

					if (hContact == NULL)
						szProto = gg->m_szModuleName;
					else
						szProto = GetContactProto(hContact);
					if (szProto == NULL)
						break;

					// Disable when updating
					if (dat) dat->disableUpdate = TRUE;

					SetValue(hwndDlg, IDC_UIN, hContact, szProto, GG_KEY_UIN, 0, hContact != NULL);
					SetValue(hwndDlg, IDC_REALIP, hContact, szProto, GG_KEY_CLIENTIP, SVS_IP, hContact != NULL);
					SetValue(hwndDlg, IDC_PORT, hContact, szProto, GG_KEY_CLIENTPORT, SVS_ZEROISUNSPEC, hContact != NULL);
					SetValue(hwndDlg, IDC_VERSION, hContact, szProto, GG_KEY_CLIENTVERSION, SVS_GGVERSION, hContact != NULL);

					SetValue(hwndDlg, IDC_FIRSTNAME, hContact, szProto, "FirstName", SVS_NORMAL, hContact != NULL);
					SetValue(hwndDlg, IDC_LASTNAME, hContact, szProto, "LastName", SVS_NORMAL, hContact != NULL);
					SetValue(hwndDlg, IDC_NICKNAME, hContact, szProto, "NickName", SVS_NORMAL, hContact != NULL);
					SetValue(hwndDlg, IDC_BIRTHYEAR, hContact, szProto, "BirthYear", SVS_ZEROISUNSPEC, hContact != NULL);
					SetValue(hwndDlg, IDC_CITY, hContact, szProto, "City", SVS_NORMAL, hContact != NULL);
					SetValue(hwndDlg, IDC_FAMILYNAME, hContact, szProto, "FamilyName", SVS_NORMAL, hContact != NULL);
					SetValue(hwndDlg, IDC_CITYORIGIN, hContact, szProto, "CityOrigin", SVS_NORMAL, hContact != NULL);

					if (hContact)
					{
						SetValue(hwndDlg, IDC_GENDER, hContact, szProto, "Gender", SVS_GENDER, hContact != NULL);
						SetValue(hwndDlg, IDC_STATUSDESCR, hContact, "CList", GG_KEY_STATUSDESCR, SVS_NORMAL, hContact != NULL);
					}
					else switch((char)db_get_b(hContact, gg->m_szModuleName, "Gender", (BYTE)'?'))
					{
					case 'F':
						SendDlgItemMessage(hwndDlg, IDC_GENDER, CB_SETCURSEL, 1, 0);
						break;
					case 'M':
						SendDlgItemMessage(hwndDlg, IDC_GENDER, CB_SETCURSEL, 2, 0);
						break;
					default:
						SendDlgItemMessage(hwndDlg, IDC_GENDER, CB_SETCURSEL, 0, 0);
					}

					// Disable when updating
					if (dat) dat->disableUpdate = FALSE;
					break;
				}
			}
			break;
		}
		break;
	case WM_COMMAND:
		if (dat && !dat->hContact && LOWORD(wParam) == IDC_SAVE && HIWORD(wParam) == BN_CLICKED)
		{
			// Save user data
			char text[256];
			gg_pubdir50_t req;
			GGPROTO *gg = dat->gg;

			if (!gg->isonline())
			{
				MessageBox(NULL,
					TranslateT("You have to be logged in before you can change your details."),
					gg->m_tszUserName, MB_OK | MB_ICONSTOP);
				break;
			}

			EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE), FALSE);

			req = gg_pubdir50_new(GG_PUBDIR50_WRITE);

			GetDlgItemTextA(hwndDlg, IDC_FIRSTNAME, text, sizeof(text));
			if (strlen(text)) gg_pubdir50_add(req, GG_PUBDIR50_FIRSTNAME, text);

			GetDlgItemTextA(hwndDlg, IDC_LASTNAME, text, sizeof(text));
			if (strlen(text)) gg_pubdir50_add(req, GG_PUBDIR50_LASTNAME, text);

			GetDlgItemTextA(hwndDlg, IDC_NICKNAME, text, sizeof(text));
			if (strlen(text)) gg_pubdir50_add(req, GG_PUBDIR50_NICKNAME, text);

			GetDlgItemTextA(hwndDlg, IDC_CITY, text, sizeof(text));
			if (strlen(text)) gg_pubdir50_add(req, GG_PUBDIR50_CITY, text);

			// Gadu-Gadu Female <-> Male
			switch(SendDlgItemMessage(hwndDlg, IDC_GENDER, CB_GETCURSEL, 0, 0))
			{
			case 1:
				gg_pubdir50_add(req, GG_PUBDIR50_GENDER, GG_PUBDIR50_GENDER_SET_FEMALE);
				break;
			case 2:
				gg_pubdir50_add(req, GG_PUBDIR50_GENDER, GG_PUBDIR50_GENDER_SET_MALE);
				break;
			default:
				gg_pubdir50_add(req, GG_PUBDIR50_GENDER, "");
			}

			GetDlgItemTextA(hwndDlg, IDC_BIRTHYEAR, text, sizeof(text));
			if (strlen(text)) gg_pubdir50_add(req, GG_PUBDIR50_BIRTHYEAR, text);

			GetDlgItemTextA(hwndDlg, IDC_FAMILYNAME, text, sizeof(text));
			if (strlen(text)) gg_pubdir50_add(req, GG_PUBDIR50_FAMILYNAME, text);

			GetDlgItemTextA(hwndDlg, IDC_CITYORIGIN, text, sizeof(text));
			if (strlen(text)) gg_pubdir50_add(req, GG_PUBDIR50_FAMILYCITY, text);

			// Run update
			gg_pubdir50_seq_set(req, GG_SEQ_CHINFO);
			gg->gg_EnterCriticalSection(&gg->sess_mutex, "gg_detailsdlgproc", 35, "sess_mutex", 1);
			gg_pubdir50(gg->sess, req);
			gg->gg_LeaveCriticalSection(&gg->sess_mutex, "gg_genoptsdlgproc", 35, 1, "sess_mutex", 1);
			dat->updating = TRUE;

			gg_pubdir50_free(req);
		}

		if (dat && !dat->hContact && !dat->disableUpdate && (HIWORD(wParam) == EN_CHANGE && (
			LOWORD(wParam) == IDC_NICKNAME || LOWORD(wParam) == IDC_FIRSTNAME || LOWORD(wParam) == IDC_LASTNAME || LOWORD(wParam) == IDC_FAMILYNAME ||
			LOWORD(wParam) == IDC_CITY || LOWORD(wParam) == IDC_CITYORIGIN || LOWORD(wParam) == IDC_BIRTHYEAR) ||
			HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_GENDER))
			EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE), TRUE);

		switch(LOWORD(wParam)) {
		case IDCANCEL:
			SendMessage(GetParent(hwndDlg),msg,wParam,lParam);
			break;
		}
		break;

	case WM_DESTROY:
		if (dat) mir_free(dat);
		break;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// Info Page : Init

int GGPROTO::details_init(WPARAM wParam, LPARAM lParam)
{
	char* szProto = GetContactProto((HANDLE)lParam);
	if ((szProto == NULL || strcmp(szProto, m_szModuleName)) && lParam || lParam && db_get_b((HANDLE)lParam, m_szModuleName, "ChatRoom", 0))
			return 0;

	// Here goes init
	{
		OPTIONSDIALOGPAGE odp = {0};

		odp.cbSize = sizeof(odp);
		odp.flags = ODPF_DONTTRANSLATE | ODPF_TCHAR;
		odp.hInstance = hInstance;
		odp.pfnDlgProc = gg_detailsdlgproc;
		odp.position = -1900000000;
		odp.pszTemplate = ((HANDLE)lParam != NULL) ? MAKEINTRESOURCEA(IDD_INFO_GG) : MAKEINTRESOURCEA(IDD_CHINFO_GG);
		odp.ptszTitle = m_tszUserName;
		odp.dwInitParam = (LPARAM)this;
		UserInfo_AddPage(wParam, &odp);
	}

	// Start search for user data
	if ((HANDLE)lParam == NULL)
		GetInfo(NULL, 0);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
// Proc: Account manager options dialog
INT_PTR CALLBACK gg_acc_mgr_guidlgproc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
////////////////////////////////////////////////////////////////////////////////////////////
{
	GGPROTO *gg = (GGPROTO *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		{
			DBVARIANT dbv;
			DWORD num;
			GGPROTO *gg = (GGPROTO *)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);

			TranslateDialogDefault(hwndDlg);
			if (num = db_get_dw(NULL, gg->m_szModuleName, GG_KEY_UIN, 0))
				SetDlgItemTextA(hwndDlg, IDC_UIN, ditoa(num));
			if (!db_get_s(NULL, gg->m_szModuleName, GG_KEY_PASSWORD, &dbv, DBVT_ASCIIZ)) {
				CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal) + 1, (LPARAM) dbv.pszVal);
				SetDlgItemTextA(hwndDlg, IDC_PASSWORD, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			if (!db_get_s(NULL, gg->m_szModuleName, GG_KEY_EMAIL, &dbv, DBVT_ASCIIZ)) {
				SetDlgItemTextA(hwndDlg, IDC_EMAIL, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			break;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CREATEACCOUNT:
			{
				// Readup data
				GGUSERUTILDLGDATA dat;
				int ret;
				char pass[128], email[128];
				GetDlgItemTextA(hwndDlg, IDC_UIN, pass, sizeof(pass));
				dat.uin = atoi(pass);
				GetDlgItemTextA(hwndDlg, IDC_PASSWORD, pass, sizeof(pass));
				GetDlgItemTextA(hwndDlg, IDC_EMAIL, email, sizeof(email));
				dat.pass = pass;
				dat.email = email;
				dat.gg = gg;
				dat.mode = GG_USERUTIL_CREATE;
				ret = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CREATEACCOUNT), hwndDlg, gg_userutildlgproc, (LPARAM)&dat);

				if (ret == IDOK)
				{
					DBVARIANT dbv;
					DWORD num;
					// Show reload required window
					ShowWindow(GetDlgItem(hwndDlg, IDC_RELOADREQD), SW_SHOW);

					// Update uin
					if (num = db_get_dw(NULL, gg->m_szModuleName, GG_KEY_UIN, 0))
						SetDlgItemTextA(hwndDlg, IDC_UIN, ditoa(num));
					else
						SetDlgItemTextA(hwndDlg, IDC_UIN, "");

					// Update password
					if (!db_get_s(NULL, gg->m_szModuleName, GG_KEY_PASSWORD, &dbv, DBVT_ASCIIZ)) {
						CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal) + 1, (LPARAM) dbv.pszVal);
						SetDlgItemTextA(hwndDlg, IDC_PASSWORD, dbv.pszVal);
						DBFreeVariant(&dbv);
					}
					else SetDlgItemTextA(hwndDlg, IDC_PASSWORD, "");

					// Update e-mail
					if (!db_get_s(NULL, gg->m_szModuleName, GG_KEY_EMAIL, &dbv, DBVT_ASCIIZ)) {
						SetDlgItemTextA(hwndDlg, IDC_EMAIL, dbv.pszVal);
						DBFreeVariant(&dbv);
					}
					else SetDlgItemTextA(hwndDlg, IDC_EMAIL, "");
				}
			}
		}
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR) lParam)->code) {
			case PSN_APPLY:
				{
					char str[128];
					uin_t uin;

					// Write Gadu-Gadu number & password
					GetDlgItemTextA(hwndDlg, IDC_UIN, str, sizeof(str));
					uin = atoi(str);
					GetDlgItemTextA(hwndDlg, IDC_PASSWORD, str, sizeof(str));
					CallService(MS_DB_CRYPT_ENCODESTRING, sizeof(str), (LPARAM) str);
					gg->checknewuser(uin, str);
					db_set_dw(NULL, gg->m_szModuleName, GG_KEY_UIN, uin);
					db_set_s(NULL, gg->m_szModuleName, GG_KEY_PASSWORD, str);

					// Write Gadu-Gadu email
					GetDlgItemTextA(hwndDlg, IDC_EMAIL, str, sizeof(str));
					db_set_s(NULL, gg->m_szModuleName, GG_KEY_EMAIL, str);
				}
			}
		}
		break;
	}
	return FALSE;
}
