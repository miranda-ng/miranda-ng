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
//
void *gg_doregister(GaduProto *gg, char *newPass, char *newEmail)
{
	// Connection handles
	struct gg_http *h = nullptr;
	struct gg_pubdir *s = nullptr;
	GGTOKEN token;

#ifdef DEBUGMODE
	gg->debugLogA("gg_doregister(): Starting.");
#endif
	if (!newPass || !newEmail) return nullptr;

	// Load token
	if (!gg->gettoken(&token)) return nullptr;

	if (!(h = gg_register3(newEmail, newPass, token.id, token.val, 0)) || !(s = (gg_pubdir*)h->data) || !s->success || !s->uin) {
		wchar_t error[128];
		mir_snwprintf(error, TranslateT("Cannot register new account because of error:\n\t%s"),
			(h && !s) ? http_error_string(h->error) :
			(s ? TranslateT("Registration rejected") : ws_strerror(errno)));
		MessageBox(nullptr, error, gg->m_tszUserName, MB_OK | MB_ICONSTOP);
		gg->debugLogW(L"gg_doregister(): Cannot register. errno=%d: %s", errno, ws_strerror(errno));
	}
	else {
		gg->setDword(GG_KEY_UIN, s->uin);
		gg->checknewuser(s->uin, newPass);
		gg->setString(GG_KEY_PASSWORD, newPass);
		gg->setString(GG_KEY_EMAIL, newEmail);
		gg_pubdir_free(h);
		gg->debugLogA("gg_doregister(): Account registration succesful.");
		MessageBox(nullptr,
			TranslateT("You have registered new account.\nPlease fill up your personal details in \"Main menu -> View/change my details...\""),
			gg->m_tszUserName, MB_OK | MB_ICONINFORMATION);
	}

#ifdef DEBUGMODE
	gg->debugLogA("gg_doregister(): End.");
#endif

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// Remove Account : Proc
//
void *gg_dounregister(GaduProto *gg, uin_t uin, char *password)
{
	// Connection handles
	struct gg_http *h;
	struct gg_pubdir *s = nullptr;
	GGTOKEN token;

#ifdef DEBUGMODE
	gg->debugLogA("gg_dounregister(): Starting.");
#endif
	if (!uin || !password)
		return nullptr;

	// Load token
	if (!gg->gettoken(&token))
		return nullptr;

	if (!(h = gg_unregister3(uin, password, token.id, token.val, 0)) || !(s = (gg_pubdir*)h->data) || !s->success || s->uin != uin)
	{
		wchar_t error[128];
		mir_snwprintf(error, TranslateT("Your account cannot be removed because of error:\n\t%s"),
			(h && !s) ? http_error_string(h->error) :
			(s ? TranslateT("Bad number or password") : ws_strerror(errno)));
		MessageBox(nullptr, error, gg->m_tszUserName, MB_OK | MB_ICONSTOP);
		gg->debugLogW(L"gg_dounregister(): Cannot remove account. errno=%d: %s", errno, ws_strerror(errno));
	}
	else
	{
		gg_pubdir_free(h);
		gg->delSetting(GG_KEY_PASSWORD);
		gg->delSetting(GG_KEY_UIN);
		gg->debugLogA("gg_dounregister(): Account %d has been removed.", uin);
		MessageBox(nullptr, TranslateT("Your account has been removed."), gg->m_tszUserName, MB_OK | MB_ICONINFORMATION);
	}

#ifdef DEBUGMODE
	gg->debugLogA("gg_dounregister(): End.");
#endif

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// Change Password Page : Proc
//
void *gg_dochpass(GaduProto *gg, uin_t uin, char *password, char *newPass)
{
#ifdef DEBUGMODE
	gg->debugLogA("gg_dochpass(): Starting.");
#endif
	if (!uin || !password || !newPass)
		return nullptr;

	// Readup email
	char email[255] = "\0";
	DBVARIANT dbv_email;
	if (!gg->getString(GG_KEY_EMAIL, &dbv_email))
	{
		strncpy(email, dbv_email.pszVal, sizeof(email));
		db_free(&dbv_email);
	}

	// Load token
	GGTOKEN token;
	if (!gg->gettoken(&token))
		return nullptr;

	gg_http *h;
	gg_pubdir *s = nullptr;
	if (!(h = gg_change_passwd4(uin, email, password, newPass, token.id, token.val, 0)) || !(s = (gg_pubdir*)h->data) || !s->success)
	{
		wchar_t error[128];
		mir_snwprintf(error, TranslateT("Your password cannot be changed because of error:\n\t%s"),
			(h && !s) ? http_error_string(h->error) :
			(s ? TranslateT("Invalid data entered") : ws_strerror(errno)));
		MessageBox(nullptr, error, gg->m_tszUserName, MB_OK | MB_ICONSTOP);
		gg->debugLogW(L"gg_dochpass(): Cannot change password. errno=%d: %s", errno, ws_strerror(errno));
	}
	else
	{
		gg_pubdir_free(h);
		gg->setString(GG_KEY_PASSWORD, newPass);
		gg->debugLogA("gg_dochpass(): Password change succesful.");
		MessageBox(nullptr, TranslateT("Your password has been changed."), gg->m_tszUserName, MB_OK | MB_ICONINFORMATION);
	}

#ifdef DEBUGMODE
	gg->debugLogA("gg_dochpass(): End.");
#endif

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// Change E-mail Page : Proc
//
void *gg_dochemail(GaduProto *gg, uin_t uin, char *password, char *email, char *newEmail)
{
#ifdef DEBUGMODE
	gg->debugLogA("gg_dochemail(): Starting.");
#endif
	if (!uin || !email || !newEmail) return nullptr;

	// Load token
	GGTOKEN token;
	if (!gg->gettoken(&token))
		return nullptr;

	// Connection handles
	gg_pubdir *s = nullptr;
	gg_http *h = gg_change_passwd4(uin, newEmail, password, password, token.id, token.val, 0);
	if (!h || !(s = (gg_pubdir*)h->data) || !s->success)
	{
		wchar_t error[128];
		mir_snwprintf(error, TranslateT("Your e-mail cannot be changed because of error:\n\t%s"),
			(h && !s) ? http_error_string(h->error) : (s ? TranslateT("Bad old e-mail or password") : ws_strerror(errno)));
		MessageBox(nullptr, error, gg->m_tszUserName, MB_OK | MB_ICONSTOP);
		gg->debugLogW(L"gg_dochemail(): Cannot change e-mail. errno=%d: %s", errno, ws_strerror(errno));
	}
	else
	{
		gg_pubdir_free(h);
		gg->setString(GG_KEY_EMAIL, newEmail);
		gg->debugLogA("gg_dochemail(): E-mail change succesful.");
		MessageBox(nullptr, TranslateT("Your e-mail has been changed."), gg->m_tszUserName, MB_OK | MB_ICONINFORMATION);
	}

#ifdef DEBUGMODE
	gg->debugLogA("gg_dochemail(): End.");
#endif

	return nullptr;
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
		Window_SetIcon_IcoLib(hwndDlg, g_plugin.getIconHandle(IDI_SETTINGS));
		dat = (GGUSERUTILDLGDATA *)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
		if (dat) SetDlgItemTextA(hwndDlg, IDC_EMAIL, dat->email); // Readup email
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
			GetDlgItemTextA(hwndDlg, IDC_PASSWORD, pass, _countof(pass));
			GetDlgItemTextA(hwndDlg, IDC_CPASSWORD, cpass, _countof(cpass));
			enable = mir_strlen(pass) && mir_strlen(cpass) && !mir_strcmp(cpass, pass);
			if (dat && dat->mode == GG_USERUTIL_REMOVE)
				EnableWindow(GetDlgItem(hwndDlg, IDOK), IsDlgButtonChecked(hwndDlg, IDC_CONFIRM) ? enable : FALSE);
			else
				EnableWindow(GetDlgItem(hwndDlg, IDOK), enable);
			break;
		}

		case IDOK:
		{
			char pass[128], cpass[128], email[128];
			GetDlgItemTextA(hwndDlg, IDC_PASSWORD, pass, _countof(pass));
			GetDlgItemTextA(hwndDlg, IDC_CPASSWORD, cpass, _countof(cpass));
			GetDlgItemTextA(hwndDlg, IDC_EMAIL, email, _countof(email));
			EndDialog(hwndDlg, IDOK);

			// Check dialog box mode
			if (!dat)
				break;

			switch (dat->mode)
			{
			case GG_USERUTIL_CREATE:
				gg_doregister(dat->gg, pass, email);
				break;
			case GG_USERUTIL_REMOVE:
				gg_dounregister(dat->gg, dat->uin, pass);
				break;
			case GG_USERUTIL_PASS:
				gg_dochpass(dat->gg, dat->uin, dat->pass, pass);
				break;
			case GG_USERUTIL_EMAIL:
				gg_dochemail(dat->gg, dat->uin, dat->pass, dat->email, email);
				break;
			}
			break;
		}

		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			break;
		}
		break;

	case WM_DESTROY:
		Window_FreeIcon_IcoLib(hwndDlg);
		break;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////
// Wait for thread to stop
//
void GaduProto::threadwait(GGTHREAD *thread)
{
	if (!thread->hThread)
		return;
	while (WaitForSingleObjectEx(thread->hThread, INFINITE, TRUE) != WAIT_OBJECT_0);

	CloseHandle(thread->hThread);
	memset(thread, 0, sizeof(GGTHREAD));
}
