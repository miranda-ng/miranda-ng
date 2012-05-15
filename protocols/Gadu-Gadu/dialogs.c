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
#define SVS_NORMAL			0
#define SVS_GENDER			1
#define SVS_ZEROISUNSPEC	2
#define SVS_IP				3
#define SVS_COUNTRY 		4
#define SVS_MONTH			5
#define SVS_SIGNED			6
#define SVS_TIMEZONE		7
#define SVS_GGVERSION		9

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
						GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SABBREVMONTHNAME1 - 1 + dbv.bVal, str, SIZEOF(str));
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
			default: pstr = str; lstrcpy(str, "???"); break;
		}
	}

	if (disableIfUndef)
	{
		EnableWindow(GetDlgItem(hwndDlg, idCtrl), !unspecified);
		if (unspecified)
			SetDlgItemText(hwndDlg, idCtrl, Translate("<not specified>"));
		else
			SetDlgItemText(hwndDlg, idCtrl, pstr);
	}
	else
	{
		EnableWindow(GetDlgItem(hwndDlg, idCtrl), TRUE);
		if (!unspecified)
			SetDlgItemText(hwndDlg, idCtrl, pstr);
	}
	DBFreeVariant(&dbv);
}

////////////////////////////////////////////////////////////////////////////////
// Options Page : Init
int gg_options_init(GGPROTO *gg, WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };

	odp.cbSize = sizeof(odp);
	odp.position = 1003000;
	odp.hInstance = hInstance;
	odp.pszGroup = LPGEN("Network");
	odp.pszTitle = GG_PROTONAME;
	odp.dwInitParam = (LPARAM)gg;

	odp.pszTab = LPGEN("General");
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT_GG_GENERAL);
	odp.pfnDlgProc = gg_genoptsdlgproc;
	odp.flags = ODPF_BOLDGROUPS | ODPF_DONTTRANSLATE;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM) & odp);

	odp.pszTab = LPGEN("Conference");
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT_GG_CONFERENCE);
	odp.pfnDlgProc = gg_confoptsdlgproc;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM) & odp);

	odp.pszTab = LPGEN("Advanced");
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT_GG_ADVANCED);
	odp.pfnDlgProc = gg_advoptsdlgproc;
	odp.flags |= ODPF_EXPERTONLY;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM) & odp);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Check if new user data has been filled in for specified account
void gg_checknewuser(GGPROTO* gg, uin_t uin, const char* passwd)
{
	char oldpasswd[128];
	DBVARIANT dbv;
	uin_t olduin = (uin_t)DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0);

	oldpasswd[0] = '\0';
	if (!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_PASSWORD, &dbv))
	{
		if (dbv.pszVal) strcpy(oldpasswd, dbv.pszVal);
		DBFreeVariant(&dbv);
	}

	if (uin > 0 && strlen(passwd) > 0 && (uin != olduin || strcmp(oldpasswd, passwd)))
		gg->check_first_conn = 1;
}

////////////////////////////////////////////////////////////////////////////////
// Options Page : Proc
static void gg_optsdlgcheck(HWND hwndDlg)
{
	char text[128];
	GetDlgItemText(hwndDlg, IDC_UIN, text, sizeof(text));
	if(strlen(text))
	{
		GetDlgItemText(hwndDlg, IDC_EMAIL, text, sizeof(text));
		if(strlen(text))
			ShowWindow(GetDlgItem(hwndDlg, IDC_CHEMAIL), SW_SHOW);
		else
			ShowWindow(GetDlgItem(hwndDlg, IDC_CHEMAIL), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_CHPASS), SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_LOSTPASS), SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_REMOVEACCOUNT), SW_SHOW);
		ShowWindow(GetDlgItem(hwndDlg, IDC_CREATEACCOUNT), SW_HIDE);
	}
	else
	{
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
	switch (msg) {
		case WM_INITDIALOG:
		{
			DBVARIANT dbv;
			DWORD num;
			GGPROTO *gg = (GGPROTO *)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);

			TranslateDialogDefault(hwndDlg);
			if (num = DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0))
			{
				SetDlgItemText(hwndDlg, IDC_UIN, ditoa(num));
				ShowWindow(GetDlgItem(hwndDlg, IDC_CREATEACCOUNT), SW_HIDE);
			}
			else
			{
				ShowWindow(GetDlgItem(hwndDlg, IDC_CHPASS), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_REMOVEACCOUNT), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_LOSTPASS), SW_HIDE);
			}
			if (!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_PASSWORD, &dbv)) {
				CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal) + 1, (LPARAM) dbv.pszVal);
				SetDlgItemText(hwndDlg, IDC_PASSWORD, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			if (!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_EMAIL, &dbv)) {
				SetDlgItemText(hwndDlg, IDC_EMAIL, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			else
			{
				ShowWindow(GetDlgItem(hwndDlg, IDC_LOSTPASS), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_CHPASS), SW_HIDE);
			}

			CheckDlgButton(hwndDlg, IDC_FRIENDSONLY, DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_FRIENDSONLY, GG_KEYDEF_FRIENDSONLY));
			CheckDlgButton(hwndDlg, IDC_SHOWINVISIBLE, DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_SHOWINVISIBLE, GG_KEYDEF_SHOWINVISIBLE));
			CheckDlgButton(hwndDlg, IDC_LEAVESTATUSMSG, DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_LEAVESTATUSMSG, GG_KEYDEF_LEAVESTATUSMSG));
			if(gg->gc_enabled)
				CheckDlgButton(hwndDlg, IDC_IGNORECONF, DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_IGNORECONF, GG_KEYDEF_IGNORECONF));
			else
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_IGNORECONF), FALSE);
				CheckDlgButton(hwndDlg, IDC_IGNORECONF, TRUE);
			}
			CheckDlgButton(hwndDlg, IDC_IMGRECEIVE, DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_IMGRECEIVE, GG_KEYDEF_IMGRECEIVE));
			CheckDlgButton(hwndDlg, IDC_SHOWLINKS, DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_SHOWLINKS, GG_KEYDEF_SHOWLINKS));
			CheckDlgButton(hwndDlg, IDC_ENABLEAVATARS, DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS));

			EnableWindow(GetDlgItem(hwndDlg, IDC_LEAVESTATUS), IsDlgButtonChecked(hwndDlg, IDC_LEAVESTATUSMSG));
			EnableWindow(GetDlgItem(hwndDlg, IDC_IMGMETHOD), IsDlgButtonChecked(hwndDlg, IDC_IMGRECEIVE));
			SendDlgItemMessage(hwndDlg, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)Translate("<Last Status>"));	// 0
			SendDlgItemMessage(hwndDlg, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)Translate("Online")); 		// ID_STATUS_ONLINE
			SendDlgItemMessage(hwndDlg, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)Translate("Away"));			// ID_STATUS_AWAY
			SendDlgItemMessage(hwndDlg, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)Translate("DND"));			// ID_STATUS_DND
			SendDlgItemMessage(hwndDlg, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)Translate("Free for chat"));	// ID_STATUS_FREECHAT
			SendDlgItemMessage(hwndDlg, IDC_LEAVESTATUS, CB_ADDSTRING, 0, (LPARAM)Translate("Invisible"));		// ID_STATUS_INVISIBLE
			switch(DBGetContactSettingWord(NULL, GG_PROTO, GG_KEY_LEAVESTATUS, GG_KEYDEF_LEAVESTATUS))
			{
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

			SendDlgItemMessage(hwndDlg, IDC_IMGMETHOD, CB_ADDSTRING, 0, (LPARAM)Translate("System tray icon"));
			SendDlgItemMessage(hwndDlg, IDC_IMGMETHOD, CB_ADDSTRING, 0, (LPARAM)Translate("Popup window"));
			SendDlgItemMessage(hwndDlg, IDC_IMGMETHOD, CB_ADDSTRING, 0, (LPARAM)Translate("Message with [img] BBCode"));
			SendDlgItemMessage(hwndDlg, IDC_IMGMETHOD, CB_SETCURSEL,
				DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_IMGMETHOD, GG_KEYDEF_IMGMETHOD), 0);
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
				{
					gg_optsdlgcheck(hwndDlg);
					break;
				}
				case IDC_LEAVESTATUSMSG:
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_LEAVESTATUS), IsDlgButtonChecked(hwndDlg, IDC_LEAVESTATUSMSG));
					break;
				}
				case IDC_IMGRECEIVE:
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_IMGMETHOD), IsDlgButtonChecked(hwndDlg, IDC_IMGRECEIVE));
					break;
				}
				case IDC_LOSTPASS:
				{
					char email[128];
					uin_t uin;
					GGPROTO *gg = (GGPROTO *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					GetDlgItemText(hwndDlg, IDC_UIN, email, sizeof(email));
					uin = atoi(email);
					GetDlgItemText(hwndDlg, IDC_EMAIL, email, sizeof(email));
					if(!strlen(email))
						MessageBox(
							NULL,
							Translate("You need to specify your registration e-mail first."),
							GG_PROTONAME,
							MB_OK | MB_ICONEXCLAMATION);
					else if(MessageBox(
						NULL,
						Translate("Your password will be sent to your registration e-mail.\nDo you want to continue ?"),
						GG_PROTONAME,
						MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
							gg_remindpassword(gg, uin, email);
					return FALSE;
				}
				case IDC_CREATEACCOUNT:
				case IDC_REMOVEACCOUNT:
					if(gg_isonline((GGPROTO *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA)))
					{
						if(MessageBox(
							NULL,
							Translate("You should disconnect before making any permanent changes with your account.\nDo you want to disconnect now ?"),
							((GGPROTO *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA))->proto.m_szModuleName,
							MB_OKCANCEL | MB_ICONEXCLAMATION) == IDCANCEL)
							break;
						else
							gg_disconnect((GGPROTO *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
					}
				case IDC_CHPASS:
				case IDC_CHEMAIL:
					{
						// Readup data
						GGUSERUTILDLGDATA dat;
						int ret;
						char pass[128], email[128];
						GGPROTO *gg = (GGPROTO *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
						GetDlgItemText(hwndDlg, IDC_UIN, pass, sizeof(pass));
						dat.uin = atoi(pass);
						GetDlgItemText(hwndDlg, IDC_PASSWORD, pass, sizeof(pass));
						GetDlgItemText(hwndDlg, IDC_EMAIL, email, sizeof(email));
						dat.pass = pass;
						dat.email = email;
						dat.gg = gg;
						if(LOWORD(wParam) == IDC_CREATEACCOUNT)
						{
							dat.mode = GG_USERUTIL_CREATE;
							ret = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CREATEACCOUNT), hwndDlg, gg_userutildlgproc, (LPARAM)&dat);
						}
						else if(LOWORD(wParam) == IDC_CHPASS)
						{
							dat.mode = GG_USERUTIL_PASS;
							ret = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CHPASS), hwndDlg, gg_userutildlgproc, (LPARAM)&dat);
						}
						else if(LOWORD(wParam) == IDC_CHEMAIL)
						{
							dat.mode = GG_USERUTIL_EMAIL;
							ret = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CHEMAIL), hwndDlg, gg_userutildlgproc, (LPARAM)&dat);
						}
						else
						{
							dat.mode = GG_USERUTIL_REMOVE;
							ret = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_REMOVEACCOUNT), hwndDlg, gg_userutildlgproc, (LPARAM)&dat);
						}

						if(ret == IDOK)
						{
							DBVARIANT dbv;
							DWORD num;
							GGPROTO *gg = (GGPROTO *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
							// Show reload required window
							ShowWindow(GetDlgItem(hwndDlg, IDC_RELOADREQD), SW_SHOW);

							// Update uin
							if (num = DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0))
								SetDlgItemText(hwndDlg, IDC_UIN, ditoa(num));
							else
								SetDlgItemText(hwndDlg, IDC_UIN, "");

							// Update password
							if (!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_PASSWORD, &dbv)) {
								CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal) + 1, (LPARAM) dbv.pszVal);
								SetDlgItemText(hwndDlg, IDC_PASSWORD, dbv.pszVal);
								DBFreeVariant(&dbv);
							}
							else
								SetDlgItemText(hwndDlg, IDC_PASSWORD, "");

							// Update e-mail
							if (!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_EMAIL, &dbv)) {
								SetDlgItemText(hwndDlg, IDC_EMAIL, dbv.pszVal);
								DBFreeVariant(&dbv);
							}
							else
								SetDlgItemText(hwndDlg, IDC_EMAIL, "");

							// Update links
							gg_optsdlgcheck(hwndDlg);

							// Remove details
							if(LOWORD(wParam) != IDC_CHPASS && LOWORD(wParam) != IDC_CHEMAIL)
							{
								DBDeleteContactSetting(NULL, GG_PROTO, GG_KEY_NICK);
								DBDeleteContactSetting(NULL, GG_PROTO, "NickName");
								DBDeleteContactSetting(NULL, GG_PROTO, "City");
								DBDeleteContactSetting(NULL, GG_PROTO, "FirstName");
								DBDeleteContactSetting(NULL, GG_PROTO, "LastName");
								DBDeleteContactSetting(NULL, GG_PROTO, "FamilyName");
								DBDeleteContactSetting(NULL, GG_PROTO, "CityOrigin");
								DBDeleteContactSetting(NULL, GG_PROTO, "Age");
								DBDeleteContactSetting(NULL, GG_PROTO, "BirthYear");
								DBDeleteContactSetting(NULL, GG_PROTO, "Gender");
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
					GGPROTO *gg = (GGPROTO *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					int status_flags = GG_STATUS_FLAG_UNKNOWN;
					char str[128];
					uin_t uin;

					// Write Gadu-Gadu number & password
					GetDlgItemText(hwndDlg, IDC_UIN, str, sizeof(str));
					uin = atoi(str);
					GetDlgItemText(hwndDlg, IDC_PASSWORD, str, sizeof(str));
					CallService(MS_DB_CRYPT_ENCODESTRING, sizeof(str), (LPARAM) str);
					gg_checknewuser(gg, uin, str);
					DBWriteContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, uin);
					DBWriteContactSettingString(NULL, GG_PROTO, GG_KEY_PASSWORD, str);

					// Write Gadu-Gadu email
					GetDlgItemText(hwndDlg, IDC_EMAIL, str, sizeof(str));
					DBWriteContactSettingString(NULL, GG_PROTO, GG_KEY_EMAIL, str);

					// Write checkboxes
					DBWriteContactSettingByte(NULL, GG_PROTO, GG_KEY_FRIENDSONLY, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_FRIENDSONLY));
					DBWriteContactSettingByte(NULL, GG_PROTO, GG_KEY_SHOWINVISIBLE, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SHOWINVISIBLE));
					DBWriteContactSettingByte(NULL, GG_PROTO, GG_KEY_LEAVESTATUSMSG, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_LEAVESTATUSMSG));
					if (gg->gc_enabled)
						DBWriteContactSettingByte(NULL, GG_PROTO, GG_KEY_IGNORECONF, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_IGNORECONF));
					DBWriteContactSettingByte(NULL, GG_PROTO, GG_KEY_IMGRECEIVE, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_IMGRECEIVE));
					DBWriteContactSettingByte(NULL, GG_PROTO, GG_KEY_SHOWLINKS, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SHOWLINKS));
					if (IsDlgButtonChecked(hwndDlg, IDC_SHOWLINKS))
						status_flags |= GG_STATUS_FLAG_SPAM;
					EnterCriticalSection(&gg->sess_mutex);
					gg_change_status_flags(gg->sess, status_flags);
					LeaveCriticalSection(&gg->sess_mutex);
					DBWriteContactSettingByte(NULL, GG_PROTO, GG_KEY_ENABLEAVATARS, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ENABLEAVATARS));

					DBWriteContactSettingByte(NULL, GG_PROTO, GG_KEY_IMGMETHOD,
						(BYTE)SendDlgItemMessage(hwndDlg, IDC_IMGMETHOD, CB_GETCURSEL, 0, 0));

					// Write leave status
					switch(SendDlgItemMessage(hwndDlg, IDC_LEAVESTATUS, CB_GETCURSEL, 0, 0))
					{
						case 1:
							DBWriteContactSettingWord(NULL, GG_PROTO, GG_KEY_LEAVESTATUS, ID_STATUS_ONLINE);
							break;
						case 2:
							DBWriteContactSettingWord(NULL, GG_PROTO, GG_KEY_LEAVESTATUS, ID_STATUS_AWAY);
							break;
						case 3:
							DBWriteContactSettingWord(NULL, GG_PROTO, GG_KEY_LEAVESTATUS, ID_STATUS_DND);
							break;
						case 4:
							DBWriteContactSettingWord(NULL, GG_PROTO, GG_KEY_LEAVESTATUS, ID_STATUS_FREECHAT);
							break;
						case 5:
							DBWriteContactSettingWord(NULL, GG_PROTO, GG_KEY_LEAVESTATUS, ID_STATUS_INVISIBLE);
							break;
						default:
							DBWriteContactSettingWord(NULL, GG_PROTO, GG_KEY_LEAVESTATUS, GG_KEYDEF_LEAVESTATUS);
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
	switch (msg) {
		case WM_INITDIALOG:
		{
			DWORD num;
			GGPROTO *gg = (GGPROTO *)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);

			TranslateDialogDefault(hwndDlg);
			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_TOTAL, CB_ADDSTRING, 0, (LPARAM)Translate("Allow"));
			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_TOTAL, CB_ADDSTRING, 0, (LPARAM)Translate("Ask"));
			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_TOTAL, CB_ADDSTRING, 0, (LPARAM)Translate("Ignore"));
			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_TOTAL, CB_SETCURSEL,
				DBGetContactSettingWord(NULL, GG_PROTO, GG_KEY_GC_POLICY_TOTAL, GG_KEYDEF_GC_POLICY_TOTAL), 0);

			if (num = DBGetContactSettingWord(NULL, GG_PROTO, GG_KEY_GC_COUNT_TOTAL, GG_KEYDEF_GC_COUNT_TOTAL))
				SetDlgItemText(hwndDlg, IDC_GC_COUNT_TOTAL, ditoa(num));

			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_UNKNOWN, CB_ADDSTRING, 0, (LPARAM)Translate("Allow"));
			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_UNKNOWN, CB_ADDSTRING, 0, (LPARAM)Translate("Ask"));
			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_UNKNOWN, CB_ADDSTRING, 0, (LPARAM)Translate("Ignore"));
			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_UNKNOWN, CB_SETCURSEL,
				DBGetContactSettingWord(NULL, GG_PROTO, GG_KEY_GC_POLICY_UNKNOWN, GG_KEYDEF_GC_POLICY_UNKNOWN), 0);

			if (num = DBGetContactSettingWord(NULL, GG_PROTO, GG_KEY_GC_COUNT_UNKNOWN, GG_KEYDEF_GC_COUNT_UNKNOWN))
				SetDlgItemText(hwndDlg, IDC_GC_COUNT_UNKNOWN, ditoa(num));

			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_DEFAULT, CB_ADDSTRING, 0, (LPARAM)Translate("Allow"));
			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_DEFAULT, CB_ADDSTRING, 0, (LPARAM)Translate("Ask"));
			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_DEFAULT, CB_ADDSTRING, 0, (LPARAM)Translate("Ignore"));
			SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_DEFAULT, CB_SETCURSEL,
				DBGetContactSettingWord(NULL, GG_PROTO, GG_KEY_GC_POLICY_DEFAULT, GG_KEYDEF_GC_POLICY_DEFAULT), 0);
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
					GGPROTO *gg = (GGPROTO *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

					// Write groupchat policy
					DBWriteContactSettingWord(NULL, GG_PROTO, GG_KEY_GC_POLICY_TOTAL,
						(WORD)SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_TOTAL, CB_GETCURSEL, 0, 0));
					DBWriteContactSettingWord(NULL, GG_PROTO, GG_KEY_GC_POLICY_UNKNOWN,
						(WORD)SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_UNKNOWN, CB_GETCURSEL, 0, 0));
					DBWriteContactSettingWord(NULL, GG_PROTO, GG_KEY_GC_POLICY_DEFAULT,
						(WORD)SendDlgItemMessage(hwndDlg, IDC_GC_POLICY_DEFAULT, CB_GETCURSEL, 0, 0));

					GetDlgItemText(hwndDlg, IDC_GC_COUNT_TOTAL, str, sizeof(str));
					DBWriteContactSettingWord(NULL, GG_PROTO, GG_KEY_GC_COUNT_TOTAL, (WORD)atoi(str));
					GetDlgItemText(hwndDlg, IDC_GC_COUNT_UNKNOWN, str, sizeof(str));
					DBWriteContactSettingWord(NULL, GG_PROTO, GG_KEY_GC_COUNT_UNKNOWN, (WORD)atoi(str));

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
	switch (msg) {
		case WM_INITDIALOG:
		{
			DBVARIANT dbv;
			DWORD num;
			GGPROTO *gg = (GGPROTO *)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);

			TranslateDialogDefault(hwndDlg);
			if (!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_SERVERHOSTS, &dbv)) {
				SetDlgItemText(hwndDlg, IDC_HOST, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			else
				SetDlgItemText(hwndDlg, IDC_HOST, GG_KEYDEF_SERVERHOSTS);

			CheckDlgButton(hwndDlg, IDC_KEEPALIVE, DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_KEEPALIVE, GG_KEYDEF_KEEPALIVE));
			CheckDlgButton(hwndDlg, IDC_SHOWCERRORS, DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_SHOWCERRORS, GG_KEYDEF_SHOWCERRORS));
			CheckDlgButton(hwndDlg, IDC_ARECONNECT, DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_ARECONNECT, GG_KEYDEF_ARECONNECT));
			CheckDlgButton(hwndDlg, IDC_MSGACK, DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_MSGACK, GG_KEYDEF_MSGACK));
			CheckDlgButton(hwndDlg, IDC_MANUALHOST, DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_MANUALHOST, GG_KEYDEF_MANUALHOST));
			CheckDlgButton(hwndDlg, IDC_SSLCONN, DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_SSLCONN, GG_KEYDEF_SSLCONN));

			EnableWindow(GetDlgItem(hwndDlg, IDC_HOST), IsDlgButtonChecked(hwndDlg, IDC_MANUALHOST));
			EnableWindow(GetDlgItem(hwndDlg, IDC_PORT), IsDlgButtonChecked(hwndDlg, IDC_MANUALHOST));

			CheckDlgButton(hwndDlg, IDC_DIRECTCONNS, DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_DIRECTCONNS, GG_KEYDEF_DIRECTCONNS));
			if (num = DBGetContactSettingWord(NULL, GG_PROTO, GG_KEY_DIRECTPORT, GG_KEYDEF_DIRECTPORT))
				SetDlgItemText(hwndDlg, IDC_DIRECTPORT, ditoa(num));
			CheckDlgButton(hwndDlg, IDC_FORWARDING, DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_FORWARDING, GG_KEYDEF_FORWARDING));
			if (!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_FORWARDHOST, &dbv)) {
				SetDlgItemText(hwndDlg, IDC_FORWARDHOST, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			if (num = DBGetContactSettingWord(NULL, GG_PROTO, GG_KEY_FORWARDPORT, GG_KEYDEF_FORWARDPORT))
				SetDlgItemText(hwndDlg, IDC_FORWARDPORT, ditoa(num));

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
					GGPROTO *gg = (GGPROTO *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					DBWriteContactSettingByte(NULL, GG_PROTO, GG_KEY_KEEPALIVE, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_KEEPALIVE));
					DBWriteContactSettingByte(NULL, GG_PROTO, GG_KEY_SHOWCERRORS, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SHOWCERRORS));
					DBWriteContactSettingByte(NULL, GG_PROTO, GG_KEY_ARECONNECT, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ARECONNECT));
					DBWriteContactSettingByte(NULL, GG_PROTO, GG_KEY_MSGACK, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_MSGACK));
					DBWriteContactSettingByte(NULL, GG_PROTO, GG_KEY_MANUALHOST, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_MANUALHOST));
					DBWriteContactSettingByte(NULL, GG_PROTO, GG_KEY_SSLCONN, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SSLCONN));

					// Transfer settings
					DBWriteContactSettingByte(NULL, GG_PROTO, GG_KEY_DIRECTCONNS, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_DIRECTCONNS));
					DBWriteContactSettingByte(NULL, GG_PROTO, GG_KEY_FORWARDING, (BYTE) IsDlgButtonChecked(hwndDlg, IDC_FORWARDING));

					// Write custom servers
					GetDlgItemText(hwndDlg, IDC_HOST, str, sizeof(str));
					DBWriteContactSettingString(NULL, GG_PROTO, GG_KEY_SERVERHOSTS, str);

					// Write direct port
					GetDlgItemText(hwndDlg, IDC_DIRECTPORT, str, sizeof(str));
					DBWriteContactSettingWord(NULL, GG_PROTO, GG_KEY_DIRECTPORT, (WORD)atoi(str));
					// Write forwarding host
					GetDlgItemText(hwndDlg, IDC_FORWARDHOST, str, sizeof(str));
					DBWriteContactSettingString(NULL, GG_PROTO, GG_KEY_FORWARDHOST, str);
					GetDlgItemText(hwndDlg, IDC_FORWARDPORT, str, sizeof(str));
					DBWriteContactSettingWord(NULL, GG_PROTO, GG_KEY_FORWARDPORT, (WORD)atoi(str));
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

	switch(msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			dat = (struct GGDETAILSDLGDATA *)mir_alloc(sizeof(struct GGDETAILSDLGDATA));
			dat->hContact=(HANDLE)lParam;
			dat->disableUpdate = FALSE;
			dat->updating = FALSE;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			// Add genders
			if(!dat->hContact)
			{
				SendDlgItemMessage(hwndDlg, IDC_GENDER, CB_ADDSTRING, 0, (LPARAM)_T(""));				// 0
				SendDlgItemMessage(hwndDlg, IDC_GENDER, CB_ADDSTRING, 0, (LPARAM)Translate("Female"));	// 1
				SendDlgItemMessage(hwndDlg, IDC_GENDER, CB_ADDSTRING, 0, (LPARAM)Translate("Male"));	// 2
			}
			break;
		}

		case WM_NOTIFY:
			switch (((LPNMHDR)lParam)->idFrom)
			{
				case 0:
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_PARAMCHANGED:
						{
							dat->gg = (GGPROTO *)((LPPSHNOTIFY)lParam)->lParam;
							break;
						}
						case PSN_INFOCHANGED:
						{
							char *szProto;
							HANDLE hContact = (HANDLE)((LPPSHNOTIFY)lParam)->lParam;
							GGPROTO *gg = dat->gg;

							// Show updated message
							if(dat && dat->updating)
							{
								MessageBox(
									NULL,
									Translate("Your details has been uploaded to the public directory."),
									GG_PROTONAME,
									MB_OK | MB_ICONINFORMATION
								);
								dat->updating = FALSE;
								break;
							}

							if (hContact == NULL)
								szProto = GG_PROTO;
							else
								szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
							if (szProto == NULL)
								break;

							// Disable when updating
							if(dat) dat->disableUpdate = TRUE;

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
							else switch((char)DBGetContactSettingByte(hContact, GG_PROTO, "Gender", (BYTE)'?'))
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
							if(dat) dat->disableUpdate = FALSE;
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

				if (!gg_isonline(gg))
				{
					MessageBox(NULL,
						Translate("You have to be logged in before you can change your details."),
						GG_PROTONAME, MB_OK | MB_ICONSTOP
					);
					break;
				}

				EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE), FALSE);

				req = gg_pubdir50_new(GG_PUBDIR50_WRITE);

				GetDlgItemText(hwndDlg, IDC_FIRSTNAME, text, sizeof(text));
				if (strlen(text)) gg_pubdir50_add(req, GG_PUBDIR50_FIRSTNAME, text);

				GetDlgItemText(hwndDlg, IDC_LASTNAME, text, sizeof(text));
				if (strlen(text)) gg_pubdir50_add(req, GG_PUBDIR50_LASTNAME, text);

				GetDlgItemText(hwndDlg, IDC_NICKNAME, text, sizeof(text));
				if (strlen(text)) gg_pubdir50_add(req, GG_PUBDIR50_NICKNAME, text);

				GetDlgItemText(hwndDlg, IDC_CITY, text, sizeof(text));
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

				GetDlgItemText(hwndDlg, IDC_BIRTHYEAR, text, sizeof(text));
				if (strlen(text)) gg_pubdir50_add(req, GG_PUBDIR50_BIRTHYEAR, text);

				GetDlgItemText(hwndDlg, IDC_FAMILYNAME, text, sizeof(text));
				if (strlen(text)) gg_pubdir50_add(req, GG_PUBDIR50_FAMILYNAME, text);

				GetDlgItemText(hwndDlg, IDC_CITYORIGIN, text, sizeof(text));
				if (strlen(text)) gg_pubdir50_add(req, GG_PUBDIR50_FAMILYCITY, text);

				// Run update
				gg_pubdir50_seq_set(req, GG_SEQ_CHINFO);
				EnterCriticalSection(&gg->sess_mutex);
				gg_pubdir50(gg->sess, req);
				LeaveCriticalSection(&gg->sess_mutex);
				dat->updating = TRUE;

				gg_pubdir50_free(req);
			}

			if(dat && !dat->hContact && !dat->disableUpdate && (HIWORD(wParam) == EN_CHANGE && (
				LOWORD(wParam) == IDC_NICKNAME || LOWORD(wParam) == IDC_FIRSTNAME || LOWORD(wParam) == IDC_LASTNAME || LOWORD(wParam) == IDC_FAMILYNAME ||
				LOWORD(wParam) == IDC_CITY || LOWORD(wParam) == IDC_CITYORIGIN || LOWORD(wParam) == IDC_BIRTHYEAR) ||
				HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_GENDER))
				EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE), TRUE);

			switch(LOWORD(wParam))
			{
				case IDCANCEL:
					SendMessage(GetParent(hwndDlg),msg,wParam,lParam);
					break;
			}
			break;
		case WM_DESTROY:
			if(dat) mir_free(dat);
			break;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// Info Page : Init
int gg_details_init(GGPROTO *gg, WPARAM wParam, LPARAM lParam)
{
	char* szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, lParam, 0);
	if ((szProto == NULL || strcmp(szProto, GG_PROTO)) && lParam || lParam && DBGetContactSettingByte((HANDLE)lParam, GG_PROTO, "ChatRoom", 0))
			return 0;

	// Here goes init
	{
		OPTIONSDIALOGPAGE odp = {0};

		odp.cbSize = sizeof(odp);
		odp.flags = ODPF_DONTTRANSLATE;
		odp.hInstance = hInstance;
		odp.pfnDlgProc = gg_detailsdlgproc;
		odp.position = -1900000000;
		odp.pszTemplate = ((HANDLE)lParam != NULL) ? MAKEINTRESOURCE(IDD_INFO_GG) : MAKEINTRESOURCE(IDD_CHINFO_GG);
		odp.ptszTitle = GG_PROTONAME;
		odp.dwInitParam = (LPARAM)gg;
		CallService(MS_USERINFO_ADDPAGE, wParam, (LPARAM)&odp);
	}

	// Start search for user data
	if((HANDLE)lParam == NULL)
		gg_getinfo((PROTO_INTERFACE *)gg, NULL, 0);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
// Proc: Account manager options dialog
INT_PTR CALLBACK gg_acc_mgr_guidlgproc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
////////////////////////////////////////////////////////////////////////////////////////////
{
	switch (msg) {
		case WM_INITDIALOG:
		{
			DBVARIANT dbv;
			DWORD num;
			GGPROTO *gg = (GGPROTO *)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);

			TranslateDialogDefault(hwndDlg);
			if (num = DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0))
				SetDlgItemText(hwndDlg, IDC_UIN, ditoa(num));
			if (!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_PASSWORD, &dbv)) {
				CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal) + 1, (LPARAM) dbv.pszVal);
				SetDlgItemText(hwndDlg, IDC_PASSWORD, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			if (!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_EMAIL, &dbv)) {
				SetDlgItemText(hwndDlg, IDC_EMAIL, dbv.pszVal);
				DBFreeVariant(&dbv);
			}
			break;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
				case IDC_CREATEACCOUNT:
				{
					// Readup data
					GGUSERUTILDLGDATA dat;
					int ret;
					char pass[128], email[128];
					GGPROTO *gg = (GGPROTO *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					GetDlgItemText(hwndDlg, IDC_UIN, pass, sizeof(pass));
					dat.uin = atoi(pass);
					GetDlgItemText(hwndDlg, IDC_PASSWORD, pass, sizeof(pass));
					GetDlgItemText(hwndDlg, IDC_EMAIL, email, sizeof(email));
					dat.pass = pass;
					dat.email = email;
					dat.gg = gg;
					dat.mode = GG_USERUTIL_CREATE;
					ret = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CREATEACCOUNT), hwndDlg, gg_userutildlgproc, (LPARAM)&dat);

					if(ret == IDOK)
					{
						DBVARIANT dbv;
						DWORD num;
						GGPROTO *gg = (GGPROTO *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
						// Show reload required window
						ShowWindow(GetDlgItem(hwndDlg, IDC_RELOADREQD), SW_SHOW);

						// Update uin
						if (num = DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0))
							SetDlgItemText(hwndDlg, IDC_UIN, ditoa(num));
						else
							SetDlgItemText(hwndDlg, IDC_UIN, "");

						// Update password
						if (!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_PASSWORD, &dbv)) {
							CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal) + 1, (LPARAM) dbv.pszVal);
							SetDlgItemText(hwndDlg, IDC_PASSWORD, dbv.pszVal);
							DBFreeVariant(&dbv);
						}
						else
							SetDlgItemText(hwndDlg, IDC_PASSWORD, "");

						// Update e-mail
						if (!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_EMAIL, &dbv)) {
							SetDlgItemText(hwndDlg, IDC_EMAIL, dbv.pszVal);
							DBFreeVariant(&dbv);
						}
						else
							SetDlgItemText(hwndDlg, IDC_EMAIL, "");
					}
				}

			}
			break;
		}
		case WM_NOTIFY:
		{
			switch(((LPNMHDR)lParam)->idFrom)
			{
				case 0:
				switch (((LPNMHDR) lParam)->code) {
					case PSN_APPLY:
					{
						GGPROTO *gg = (GGPROTO *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
						char str[128];
						uin_t uin;

						// Write Gadu-Gadu number & password
						GetDlgItemText(hwndDlg, IDC_UIN, str, sizeof(str));
						uin = atoi(str);
						GetDlgItemText(hwndDlg, IDC_PASSWORD, str, sizeof(str));
						CallService(MS_DB_CRYPT_ENCODESTRING, sizeof(str), (LPARAM) str);
						gg_checknewuser(gg, uin, str);
						DBWriteContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, uin);
						DBWriteContactSettingString(NULL, GG_PROTO, GG_KEY_PASSWORD, str);

						// Write Gadu-Gadu email
						GetDlgItemText(hwndDlg, IDC_EMAIL, str, sizeof(str));
						DBWriteContactSettingString(NULL, GG_PROTO, GG_KEY_EMAIL, str);
					}
				}
			}
			break;
		}
	}
	return FALSE;
}
