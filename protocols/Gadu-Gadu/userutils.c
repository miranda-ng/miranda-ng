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

////////////////////////////////////////////////////////////////////////////////
// Create New Account : Proc
void *gg_doregister(GGPROTO *gg, char *newPass, char *newEmail)
{
	// Connection handles
	struct gg_http *h = NULL;
	struct gg_pubdir *s = NULL;
	GGTOKEN token;

#ifdef DEBUGMODE
	gg_netlog(gg, "gg_doregister(): Starting.");
#endif
	if(!newPass || !newEmail) return NULL;

	// Load token
	if(!gg_gettoken(gg, &token)) return NULL;

	if (!(h = gg_register3(newEmail, newPass, token.id, token.val, 0)) || !(s = h->data) || !s->success || !s->uin)
	{
		char error[128];
		mir_snprintf(error, sizeof(error), Translate("Cannot register new account because of error:\n\t%s"),
			(h && !s) ? http_error_string(h ? h->error : 0) :
			(s ? Translate("Registration rejected") : strerror(errno)));
		MessageBox(
			NULL,
			error,
			GG_PROTONAME,
			MB_OK | MB_ICONSTOP
		);
		gg_netlog(gg, "gg_doregister(): Cannot register because of \"%s\".", strerror(errno));
	}
	else
	{
		DBWriteContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, s->uin);
		CallService(MS_DB_CRYPT_ENCODESTRING, strlen(newPass) + 1, (LPARAM) newPass);
		gg_checknewuser(gg, s->uin, newPass);
		DBWriteContactSettingString(NULL, GG_PROTO, GG_KEY_PASSWORD, newPass);
		DBWriteContactSettingString(NULL, GG_PROTO, GG_KEY_EMAIL, newEmail);
		gg_pubdir_free(h);
		gg_netlog(gg, "gg_doregister(): Account registration succesful.");
		MessageBox(
			NULL,
			Translate("You have registered new account.\nPlease fill up your personal details in \"M->View/Change My Details...\""),
			GG_PROTONAME,
			MB_OK | MB_ICONINFORMATION
		);
	}

#ifdef DEBUGMODE
	gg_netlog(gg, "gg_doregister(): End.");
#endif

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Remove Account : Proc
void *gg_dounregister(GGPROTO *gg, uin_t uin, char *password)
{
	// Connection handles
	struct gg_http *h;
	struct gg_pubdir *s;
	GGTOKEN token;

#ifdef DEBUGMODE
	gg_netlog(gg, "gg_dounregister(): Starting.");
#endif
	if(!uin || !password) return NULL;

	// Load token
	if(!gg_gettoken(gg, &token)) return NULL;

	if (!(h = gg_unregister3(uin, password, token.id, token.val, 0)) || !(s = h->data) || !s->success || s->uin != uin)
	{
		char error[128];
		mir_snprintf(error, sizeof(error), Translate("Your account cannot be removed because of error:\n\t%s"),
			(h && !s) ? http_error_string(h ? h->error : 0) :
			(s ? Translate("Bad number or password") : strerror(errno)));
		MessageBox(
			NULL,
			error,
			GG_PROTONAME,
			MB_OK | MB_ICONSTOP
		);
		gg_netlog(gg, "gg_dounregister(): Cannot remove account because of \"%s\".", strerror(errno));
	}
	else
	{
		gg_pubdir_free(h);
		DBDeleteContactSetting(NULL, GG_PROTO, GG_KEY_PASSWORD);
		DBDeleteContactSetting(NULL, GG_PROTO, GG_KEY_UIN);
		gg_netlog(gg, "gg_dounregister(): Account %d has been removed.", uin);
		MessageBox(
			NULL,
			Translate("Your account has been removed."),
			GG_PROTONAME,
			MB_OK | MB_ICONINFORMATION
		);
	}

#ifdef DEBUGMODE
	gg_netlog(gg, "gg_dounregister(): End.");
#endif

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Change Password Page : Proc
void *gg_dochpass(GGPROTO *gg, uin_t uin, char *password, char *newPass)
{
	// Readup email
	char email[255] = "\0"; DBVARIANT dbv_email;
	// Connection handles
	struct gg_http *h;
	struct gg_pubdir *s;
	GGTOKEN token;

#ifdef DEBUGMODE
	gg_netlog(gg, "gg_dochpass(): Starting.");
#endif
	if(!uin || !password || !newPass) return NULL;

	if (!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_EMAIL, &dbv_email)) 
    {
		strncpy(email, dbv_email.pszVal, sizeof(email));
        DBFreeVariant(&dbv_email);
    }
    
	// Load token
	if(!gg_gettoken(gg, &token)) return NULL;

	if (!(h = gg_change_passwd4(uin, email, password, newPass, token.id, token.val, 0)) || !(s = h->data) || !s->success)
	{
		char error[128];
		mir_snprintf(error, sizeof(error), Translate("Your password cannot be changed because of error:\n\t%s"),
			(h && !s) ? http_error_string(h ? h->error : 0) :
			(s ? Translate("Invalid data entered") : strerror(errno)));
		MessageBox(
			NULL,
			error,
			GG_PROTONAME,
			MB_OK | MB_ICONSTOP
		);
		gg_netlog(gg, "gg_dochpass(): Cannot change password because of \"%s\".", strerror(errno));
	}
	else
	{
		gg_pubdir_free(h);
		CallService(MS_DB_CRYPT_ENCODESTRING, strlen(newPass) + 1, (LPARAM) newPass);
		DBWriteContactSettingString(NULL, GG_PROTO, GG_KEY_PASSWORD, newPass);
		gg_netlog(gg, "gg_dochpass(): Password change succesful.");
		MessageBox(
			NULL,
			Translate("Your password has been changed."),
			GG_PROTONAME,
			MB_OK | MB_ICONINFORMATION
		);
	}

#ifdef DEBUGMODE
	gg_netlog(gg, "gg_dochpass(): End.");
#endif

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Change E-mail Page : Proc
void *gg_dochemail(GGPROTO *gg, uin_t uin, char *password, char *email, char *newEmail)
{
	// Connection handles
	struct gg_http *h;
	struct gg_pubdir *s;
	GGTOKEN token;

#ifdef DEBUGMODE
	gg_netlog(gg, "gg_doemail(): Starting.");
#endif
	if(!uin || !email || !newEmail) return NULL;

	// Load token
	if(!gg_gettoken(gg, &token)) return NULL;

	if (!(h = gg_change_passwd4(uin, newEmail, password, password, token.id, token.val, 0)) || !(s = h->data) || !s->success)
	{
		char error[128];
		mir_snprintf(error, sizeof(error), Translate("Your e-mail cannot be changed because of error:\n\t%s"),
			(h && !s) ? http_error_string(h ? h->error : 0) :
			(s ? Translate("Bad old e-mail or password") : strerror(errno)));
		MessageBox(
			NULL,
			error,
			GG_PROTONAME,
			MB_OK | MB_ICONSTOP
		);
		gg_netlog(gg, "gg_dochpass(): Cannot change e-mail because of \"%s\".", strerror(errno));
	}
	else
	{
		gg_pubdir_free(h);
		DBWriteContactSettingString(NULL, GG_PROTO, GG_KEY_EMAIL, newEmail);
		gg_netlog(gg, "gg_doemail(): E-mail change succesful.");
		MessageBox(
			NULL,
			Translate("Your e-mail has been changed."),
			GG_PROTONAME,
			MB_OK | MB_ICONINFORMATION
		);
	}

#ifdef DEBUGMODE
	gg_netlog(gg, "gg_doemail(): End.");
#endif

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// User Util Dlg Page : Data
INT_PTR CALLBACK gg_userutildlgproc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	GGUSERUTILDLGDATA *dat = (GGUSERUTILDLGDATA *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg)
	{
		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);
			WindowSetIcon(hwndDlg, "settings");
			dat = (GGUSERUTILDLGDATA *)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
			if (dat) SetDlgItemText(hwndDlg, IDC_EMAIL, dat->email); // Readup email
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_PASSWORD:
				case IDC_CPASSWORD:
				case IDC_CONFIRM:
				{
					char pass[128], cpass[128];
					BOOL enable;
					GetDlgItemText(hwndDlg, IDC_PASSWORD, pass, sizeof(pass));
					GetDlgItemText(hwndDlg, IDC_CPASSWORD, cpass, sizeof(cpass));
					enable = strlen(pass) && strlen(cpass) && !strcmp(cpass, pass);
					if (dat && dat->mode == GG_USERUTIL_REMOVE)
						EnableWindow(GetDlgItem(hwndDlg, IDOK), IsDlgButtonChecked(hwndDlg, IDC_CONFIRM) ? enable : FALSE);
					else
						EnableWindow(GetDlgItem(hwndDlg, IDOK), enable);
					break;
				}

				case IDOK:
				{
					char pass[128], cpass[128], email[128];
					GetDlgItemText(hwndDlg, IDC_PASSWORD, pass, sizeof(pass));
					GetDlgItemText(hwndDlg, IDC_CPASSWORD, cpass, sizeof(cpass));
					GetDlgItemText(hwndDlg, IDC_EMAIL, email, sizeof(email));
					EndDialog(hwndDlg, IDOK);

					// Check dialog box mode
					if (!dat) break;
					switch (dat->mode)
					{
						case GG_USERUTIL_CREATE: gg_doregister(dat->gg, pass, email);							break;
						case GG_USERUTIL_REMOVE: gg_dounregister(dat->gg, dat->uin, pass);						break;
						case GG_USERUTIL_PASS:   gg_dochpass(dat->gg, dat->uin, dat->pass, pass);				break;
						case GG_USERUTIL_EMAIL:  gg_dochemail(dat->gg, dat->uin, dat->pass, dat->email, email);	break;
					}
					break;
				}

				case IDCANCEL:
					EndDialog(hwndDlg, IDCANCEL);
					break;
			}
			break;

		case WM_DESTROY:
			WindowFreeIcon(hwndDlg);
			break;
	}
	return FALSE;
}
