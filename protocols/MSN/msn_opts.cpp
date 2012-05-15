/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.
Copyright (c) 2006-2012 Boris Krasnovskiy.
Copyright (c) 2003-2005 George Hazan.
Copyright (c) 2002-2003 Richard Hughes (original version).

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "msn_global.h"
#include "msn_proto.h"

#include <commdlg.h>


/////////////////////////////////////////////////////////////////////////////////////////
// Icons init

struct _tag_iconList
{
	const char*  szDescr;
	const char*  szName;
	int			 defIconID;
}
static const iconList[] =
{
	{	LPGEN("Protocol icon"),          "main",        IDI_MSN        },
	{	LPGEN("Hotmail Inbox"),          "inbox",       IDI_INBOX      },
	{	LPGEN("Profile"),                "profile",     IDI_PROFILE    },
	{	LPGEN("MSN Services"),           "services",    IDI_SERVICES   },
	{	LPGEN("Block user"),             "block",       IDI_MSNBLOCK   },
	{	LPGEN("Invite to chat"),         "invite",      IDI_INVITE     },
	{	LPGEN("Start Netmeeting"),       "netmeeting",  IDI_NETMEETING },
	{	LPGEN("Contact list"),           "list_fl",     IDI_LIST_FL    },
	{	LPGEN("Allowed list"),           "list_al",     IDI_LIST_AL    },
	{	LPGEN("Blocked list"),           "list_bl",     IDI_LIST_BL    },
	{	LPGEN("Relative list"),          "list_rl",     IDI_LIST_RL    },
	{	LPGEN("Local list"),             "list_lc",     IDI_LIST_LC    },
};

HANDLE hIconLibItem[SIZEOF(iconList)];


void MsnInitIcons(void)
{
	TCHAR szFile[MAX_PATH];
	char szSectionName[100];

	mir_snprintf(szSectionName, sizeof(szSectionName), "%s/%s", LPGEN("Protocols"), LPGEN("MSN"));
	GetModuleFileName(hInst, szFile, SIZEOF(szFile));

	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(SKINICONDESC);
	sid.ptszDefaultFile = szFile;
	sid.pszSection = szSectionName;
	sid.flags = SIDF_PATH_TCHAR;

	for (unsigned i = 0; i < SIZEOF(iconList); i++) 
	{
		char szSettingName[100];
		mir_snprintf(szSettingName, sizeof(szSettingName), "MSN_%s", iconList[i].szName);
		sid.pszName = szSettingName;
		sid.pszDescription = (char*)iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].defIconID;
		hIconLibItem[i] = (HANDLE)CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
	}
}

HICON LoadIconEx(const char* name, bool big)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, sizeof(szSettingName), "MSN_%s", name);
	return (HICON)MSN_CallService(MS_SKIN2_GETICON, big, (LPARAM)szSettingName);
}

HANDLE GetIconHandle(int iconId)
{
	for (unsigned i=0; i < SIZEOF(iconList); i++)
		if (iconList[i].defIconID == iconId)
			return hIconLibItem[i];

	return NULL;
}

void  ReleaseIconEx(const char* name, bool big)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, sizeof(szSettingName), "MSN_%s", name);
	MSN_CallService(big ? MS_SKIN2_RELEASEICONBIG : MS_SKIN2_RELEASEICON, 0, (LPARAM)szSettingName);
}

INT_PTR CALLBACK DlgProcMsnServLists(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////////////////
// MSN Options dialog procedure

static INT_PTR CALLBACK DlgProcMsnOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
	case WM_INITDIALOG: 
		{
			TranslateDialogDefault(hwndDlg);

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			CMsnProto* proto = (CMsnProto*)lParam;

			SetDlgItemTextA(hwndDlg, IDC_HANDLE, proto->MyOptions.szEmail);

			char tBuffer[MAX_PATH];
			if (!proto->getStaticString(NULL, "Password", tBuffer, sizeof(tBuffer))) 
			{
				MSN_CallService(MS_DB_CRYPT_DECODESTRING, strlen(tBuffer)+1, (LPARAM)tBuffer);
				tBuffer[16] = 0;
				SetDlgItemTextA(hwndDlg, IDC_PASSWORD, tBuffer);
			}
			SendDlgItemMessage(hwndDlg, IDC_PASSWORD, EM_SETLIMITTEXT, 16, 0);

			HWND wnd = GetDlgItem(hwndDlg, IDC_HANDLE2);
			DBVARIANT dbv;
			if (!proto->getTString("Nick", &dbv)) 
			{
				SetWindowText(wnd, dbv.ptszVal);
				MSN_FreeVariant(&dbv);
			}
			EnableWindow(wnd, proto->msnLoggedIn);
			EnableWindow(GetDlgItem(hwndDlg, IDC_MOBILESEND), proto->msnLoggedIn && 
				proto->getByte("MobileEnabled", 0) && proto->getByte("MobileAllowed", 0));

			CheckDlgButton(hwndDlg, IDC_MOBILESEND,        proto->getByte("MobileAllowed", 0));
			CheckDlgButton(hwndDlg, IDC_SENDFONTINFO,      proto->getByte("SendFontInfo", 1));
			CheckDlgButton(hwndDlg, IDC_MANAGEGROUPS,      proto->getByte("ManageServer", 1));

			int tValue = proto->getByte("RunMailerOnHotmail", 0);
			CheckDlgButton(hwndDlg, IDC_RUN_APP_ON_HOTMAIL, tValue);
			EnableWindow(GetDlgItem(hwndDlg, IDC_MAILER_APP), tValue);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ENTER_MAILER_APP), tValue);

			if (!proto->getStaticString(NULL, "MailerPath", tBuffer, sizeof(tBuffer)))
				SetDlgItemTextA(hwndDlg, IDC_MAILER_APP, tBuffer);

			if (!proto->msnLoggedIn) 
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_MANAGEGROUPS), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_DISABLE_ANOTHER_CONTACTS), FALSE);
			}
			else CheckDlgButton(hwndDlg, IDC_DISABLE_ANOTHER_CONTACTS, proto->msnOtherContactsBlocked);
			return TRUE;
		}

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_NEWMSNACCOUNTLINK) 
		{
			MSN_CallService(MS_UTILS_OPENURL, 1, (LPARAM)"https://signup.live.com");
			return TRUE;
		}

		if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus()) 
		{
			switch(LOWORD(wParam)) 
			{
			case IDC_HANDLE:			case IDC_PASSWORD:			case IDC_HANDLE2:
			case IDC_GATEWAYSERVER: 	case IDC_YOURHOST:			case IDC_DIRECTSERVER:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}	
		}

		if (HIWORD(wParam) == BN_CLICKED)
			switch(LOWORD(wParam)) 
		{
			case IDC_SENDFONTINFO:
			case IDC_DISABLE_ANOTHER_CONTACTS:
			case IDC_MOBILESEND:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_MANAGEGROUPS:
				if (IsDlgButtonChecked(hwndDlg, IDC_MANAGEGROUPS))
				{
					if (IDYES == MessageBox(hwndDlg,
						TranslateT("Server groups import may change your contact list layout after next login. Do you want to upload your groups to the server?"),
						TranslateT("MSN Protocol"), MB_YESNOCANCEL))
					{
						CMsnProto* proto = (CMsnProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
						proto->MSN_UploadServerGroups(NULL);
					}
				}
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_RUN_APP_ON_HOTMAIL: 
				{
					BOOL tIsChosen = IsDlgButtonChecked(hwndDlg, IDC_RUN_APP_ON_HOTMAIL);
					EnableWindow(GetDlgItem(hwndDlg, IDC_MAILER_APP), tIsChosen);
					EnableWindow(GetDlgItem(hwndDlg, IDC_ENTER_MAILER_APP), tIsChosen);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				}

			case IDC_ENTER_MAILER_APP: 
				{
					HWND tEditField = GetDlgItem(hwndDlg, IDC_MAILER_APP);

					char szFile[MAX_PATH + 2];
					GetWindowTextA(tEditField, szFile, sizeof(szFile));

					size_t tSelectLen = 0;

					if (szFile[0] == '\"') 
					{
						char* p = strchr(szFile+1, '\"');
						if (p != NULL) 
						{
							*p = '\0';
							memmove(szFile, szFile+1, strlen(szFile));
							tSelectLen += 2;
							goto LBL_Continue;
						}	
					}

					{	
						char* p = strchr(szFile, ' ');
						if (p != NULL) *p = '\0';
					}
LBL_Continue:
					tSelectLen += strlen(szFile);

					OPENFILENAMEA ofn = {0};
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = hwndDlg;
					ofn.nMaxFile = sizeof(szFile);
					ofn.lpstrFile = szFile;
					ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
					if (GetOpenFileNameA(&ofn) != TRUE)
						break;

					if (strchr(szFile, ' ') != NULL) 
					{
						char tmpBuf[MAX_PATH + 2];
						mir_snprintf(tmpBuf, sizeof(tmpBuf), "\"%s\"", szFile);
						strcpy(szFile, tmpBuf);
					}

					SendMessage(tEditField, EM_SETSEL, 0, tSelectLen);
					SendMessageA(tEditField, EM_REPLACESEL, TRUE, LPARAM(szFile));
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}	
		}

		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == (UINT)PSN_APPLY) 
		{
			bool reconnectRequired = false;
			TCHAR screenStr[MAX_PATH];
			char  password[100], szEmail[MSN_MAX_EMAIL_LEN];
			DBVARIANT dbv;

			CMsnProto* proto = (CMsnProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

			GetDlgItemTextA(hwndDlg, IDC_HANDLE, szEmail, sizeof(szEmail));
			if (strcmp(_strlwr(szEmail), proto->MyOptions.szEmail)) 
			{
				reconnectRequired = true;
				strcpy(proto->MyOptions.szEmail, szEmail);
				proto->setString("e-mail", szEmail);
			}

			GetDlgItemTextA(hwndDlg, IDC_PASSWORD, password, sizeof(password));
			MSN_CallService(MS_DB_CRYPT_ENCODESTRING, sizeof(password),(LPARAM)password);
			if (!proto->getString("Password", &dbv)) 
			{
				if (strcmp(password, dbv.pszVal)) 
				{
					reconnectRequired = true;
					proto->setString("Password", password);
				}
				MSN_FreeVariant(&dbv);
			}
			else 
			{
				reconnectRequired = true;
				proto->setString("Password", password);
			}

			GetDlgItemText(hwndDlg, IDC_HANDLE2, screenStr, SIZEOF(screenStr));
			if	(!proto->getTString("Nick", &dbv)) 
			{
				if (_tcscmp(dbv.ptszVal, screenStr))
					proto->MSN_SendNickname(screenStr);
				MSN_FreeVariant(&dbv);
			}
			else
			{
				proto->MSN_SendNickname(screenStr);
			}

			BYTE mblsnd = IsDlgButtonChecked(hwndDlg, IDC_MOBILESEND) == BST_CHECKED;
			if (mblsnd != proto->getByte("MobileAllowed", 0))
			{
				proto->msnNsThread->sendPacket("PRP", "MOB %c", mblsnd ? 'Y' : 'N');
				proto->MSN_SetServerStatus(proto->m_iStatus);
			}

			unsigned tValue = IsDlgButtonChecked(hwndDlg, IDC_DISABLE_ANOTHER_CONTACTS);
			if (tValue != proto->msnOtherContactsBlocked && proto->msnLoggedIn) 
			{
				proto->msnOtherContactsBlocked = tValue;
				proto->msnNsThread->sendPacket("BLP", tValue ? "BL" : "AL");
				proto->MSN_ABUpdateAttr(NULL, "MSN.IM.BLP", tValue ? "0" : "1");
				break;
			}

			proto->setByte("SendFontInfo",       (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SENDFONTINFO));
			proto->setByte("RunMailerOnHotmail", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_RUN_APP_ON_HOTMAIL));
			proto->setByte("ManageServer",       (BYTE)IsDlgButtonChecked(hwndDlg, IDC_MANAGEGROUPS));

			GetDlgItemText(hwndDlg, IDC_MAILER_APP, screenStr, SIZEOF(screenStr));
			proto->setTString("MailerPath", screenStr);

			if (reconnectRequired && proto->msnLoggedIn)
				MessageBox(hwndDlg, TranslateT("The changes you have made require you to reconnect to the MSN Messenger network before they take effect"), 
				TranslateT("MSN Options"), MB_OK);

			proto->LoadOptions();
			return TRUE;
		}
		break;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MSN Connection Options dialog procedure

static INT_PTR CALLBACK DlgProcMsnConnOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			CMsnProto* proto = (CMsnProto*)lParam;

			if (!proto->getString(NULL, "DirectServer", &dbv)) 
			{
				SetDlgItemTextA(hwndDlg, IDC_DIRECTSERVER, dbv.pszVal);
				MSN_FreeVariant(&dbv);
			}
			else 
				SetDlgItemTextA(hwndDlg, IDC_DIRECTSERVER,  MSN_DEFAULT_LOGIN_SERVER);

			if (!proto->getString(NULL, "GatewayServer", &dbv)) 
			{
				SetDlgItemTextA(hwndDlg, IDC_GATEWAYSERVER, dbv.pszVal);
				MSN_FreeVariant(&dbv);
			}
			else 
				SetDlgItemTextA(hwndDlg, IDC_GATEWAYSERVER,  MSN_DEFAULT_GATEWAY);

			CheckDlgButton(hwndDlg, IDC_SLOWSEND,    proto->getByte("SlowSend",    0));

			SendDlgItemMessage(hwndDlg, IDC_HOSTOPT, CB_ADDSTRING, 0, (LPARAM)TranslateT("Automatically obtain host/port"));
			SendDlgItemMessage(hwndDlg, IDC_HOSTOPT, CB_ADDSTRING, 0, (LPARAM)TranslateT("Manually specify host/port"));
			SendDlgItemMessage(hwndDlg, IDC_HOSTOPT, CB_ADDSTRING, 0, (LPARAM)TranslateT("Disable"));

			unsigned gethst = proto->getByte("AutoGetHost", 1);
			if (gethst < 2) gethst = !gethst;

			char ipaddr[256] = "";
			if (gethst == 1) 
			{
				if (proto->getStaticString(NULL, "YourHost", ipaddr, sizeof(ipaddr)))
					gethst = 0;
			}
			if (gethst == 0)
			{
				mir_snprintf(ipaddr, sizeof(ipaddr), "%s", proto->msnLoggedIn ? 
					proto->MyConnection.GetMyExtIPStr() : "");
			}
			SendDlgItemMessage(hwndDlg, IDC_HOSTOPT, CB_SETCURSEL, gethst, 0);
			if (ipaddr[0])
				SetDlgItemTextA(hwndDlg, IDC_YOURHOST, ipaddr);
			else
				SetDlgItemText(hwndDlg, IDC_YOURHOST, TranslateT("IP info available only after login"));
			EnableWindow(GetDlgItem(hwndDlg, IDC_YOURHOST), gethst == 1);

			return TRUE;
		}

	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{
		case IDC_RESETSERVER:
			SetDlgItemTextA(hwndDlg, IDC_DIRECTSERVER, MSN_DEFAULT_LOGIN_SERVER);
			SetDlgItemTextA(hwndDlg, IDC_GATEWAYSERVER, MSN_DEFAULT_GATEWAY);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}

		if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus())
			switch(LOWORD(wParam)) {
			case IDC_DIRECTSERVER:
			case IDC_GATEWAYSERVER:
			case IDC_YOURHOST:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}

		if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_HOSTOPT)
		{
			unsigned gethst = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_YOURHOST), gethst == 1);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}

		if (HIWORD(wParam) == BN_CLICKED)
		{
			switch(LOWORD(wParam)) 
			{
			case IDC_SLOWSEND:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}	
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == (UINT)PSN_APPLY) 
		{
			bool reconnectRequired = false;
			char str[MAX_PATH];

			CMsnProto* proto = (CMsnProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

			GetDlgItemTextA(hwndDlg, IDC_DIRECTSERVER, str, sizeof(str));
			if (strcmp(str, MSN_DEFAULT_LOGIN_SERVER))
				proto->setString(NULL, "DirectServer", str);
			else
				proto->deleteSetting(NULL, "DirectServer");

			GetDlgItemTextA(hwndDlg, IDC_GATEWAYSERVER, str, sizeof(str));
			if (strcmp(str, MSN_DEFAULT_GATEWAY))
				proto->setString(NULL, "GatewayServer", str);
			else
				proto->deleteSetting(NULL, "GatewayServer");

			proto->setByte("SlowSend",   (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SLOWSEND ));
			if (proto->getByte("SlowSend", FALSE))
			{
				if (DBGetContactSettingDword(NULL, "SRMsg", "MessageTimeout", 60000) < 60000 ||  
					DBGetContactSettingDword(NULL, "SRMM",  "MessageTimeout", 60000) < 60000) 
				{
					MessageBox(NULL, TranslateT("MSN Protocol requires message timeout to be not less then 60 sec. Correct the timeout value."), 
						TranslateT("MSN Protocol"), MB_OK|MB_ICONINFORMATION);
				}
			}

			unsigned gethst2 = proto->getByte("AutoGetHost", 1);
			unsigned gethst = SendDlgItemMessage(hwndDlg, IDC_HOSTOPT, CB_GETCURSEL, 0, 0);
			if (gethst < 2) gethst = !gethst;
			proto->setByte("AutoGetHost", (BYTE)gethst);

			if (gethst == 0)
			{
				GetDlgItemTextA(hwndDlg, IDC_YOURHOST, str, sizeof(str));
				proto->setString(NULL, "YourHost", str);
			}
			else
				proto->deleteSetting(NULL, "YourHost");

			if (gethst != gethst2)
			{
				proto->ForkThread(&CMsnProto::MSNConnDetectThread, NULL);
			}

			if (reconnectRequired && proto->msnLoggedIn)
				MessageBox(hwndDlg, TranslateT("The changes you have made require you to reconnect to the MSN Messenger network before they take effect"), 
				TranslateT("MSN Options"), MB_OK);

			proto->LoadOptions();
			return TRUE;
		}	
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// PopUp Options Dialog: style, position, color, font...

static INT_PTR CALLBACK DlgProcHotmailPopUpOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool bEnabled;

	switch(msg) 
	{
	case WM_INITDIALOG: 
		{
			TranslateDialogDefault(hwndDlg);
			bEnabled = false;

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			CMsnProto* proto = (CMsnProto*)lParam;

			int disableHotmailPopup = proto->getByte("DisableHotmail", 0);

			CheckDlgButton(hwndDlg, IDC_DISABLEHOTMAILPOPUP, disableHotmailPopup);
			CheckDlgButton(hwndDlg, IDC_DISABLEHOTMAILTRAY,  proto->getByte("DisableHotmailTray", 1));
			CheckDlgButton(hwndDlg, IDC_DISABLEHOTMAILCL,    proto->getByte("DisableHotmailCL", 0));
			CheckDlgButton(hwndDlg, IDC_DISABLEHOTJUNK,      proto->getByte("DisableHotmailJunk", 0));
			CheckDlgButton(hwndDlg, IDC_NOTIFY_ENDSESSION,   proto->getByte("EnableSessionPopup", 0));
			CheckDlgButton(hwndDlg, IDC_NOTIFY_FIRSTMSG,     proto->getByte("EnableDeliveryPopup", 0));
			CheckDlgButton(hwndDlg, IDC_ERRORS_USING_POPUPS, proto->getByte("ShowErrorsAsPopups", 0));

			bEnabled = true;
			return TRUE;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{
		case IDC_DISABLEHOTMAILPOPUP:
		case IDC_DISABLEHOTMAILTRAY: 
		case IDC_DISABLEHOTMAILCL: 
		case IDC_DISABLEHOTJUNK:
		case IDC_NOTIFY_ENDSESSION:
		case IDC_NOTIFY_FIRSTMSG:
		case IDC_ERRORS_USING_POPUPS:
			if (bEnabled)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
		break;

	case WM_NOTIFY: //Here we have pressed either the OK or the APPLY button.
		switch(((LPNMHDR)lParam)->idFrom) 
		{
		case 0:
			switch (((LPNMHDR)lParam)->code) 
			{
			case PSN_RESET: 
				{
					CMsnProto* proto = (CMsnProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					proto->LoadOptions();
					return TRUE;
				}

			case PSN_APPLY: 
				{
					CMsnProto* proto = (CMsnProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

					proto->MyOptions.ShowErrorsAsPopups = IsDlgButtonChecked(hwndDlg, IDC_ERRORS_USING_POPUPS) != 0;
					proto->setByte("ShowErrorsAsPopups", proto->MyOptions.ShowErrorsAsPopups);

					proto->setByte("DisableHotmail", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DISABLEHOTMAILPOPUP));
					proto->setByte("DisableHotmailCL", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DISABLEHOTMAILCL));
					proto->setByte("DisableHotmailTray", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DISABLEHOTMAILTRAY));
					proto->setByte("DisableHotmailJunk",(BYTE)IsDlgButtonChecked(hwndDlg, IDC_DISABLEHOTJUNK));
					proto->setByte("EnableDeliveryPopup", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_NOTIFY_FIRSTMSG));
					proto->setByte("EnableSessionPopup", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_NOTIFY_ENDSESSION));

					HANDLE hContact = proto->MSN_HContactFromEmail(proto->MyOptions.szEmail);
					if (hContact) proto->displayEmailCount(hContact);
				}
				return TRUE;
			}
			break;
		}
		break;
	}

	return FALSE;
}

static INT_PTR CALLBACK DlgProcAccMgrUI(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) 
	{
	case WM_INITDIALOG: 
		{
			TranslateDialogDefault(hwndDlg);

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			CMsnProto* proto = (CMsnProto*)lParam;

			SetDlgItemTextA(hwndDlg, IDC_HANDLE, proto->MyOptions.szEmail);

			char tBuffer[MAX_PATH];
			if (!proto->getStaticString(NULL, "Password", tBuffer, sizeof(tBuffer))) 
			{
				MSN_CallService(MS_DB_CRYPT_DECODESTRING, strlen(tBuffer)+1, (LPARAM)tBuffer);
				tBuffer[16] = 0;
				SetDlgItemTextA(hwndDlg, IDC_PASSWORD, tBuffer);
			}
			SendDlgItemMessage(hwndDlg, IDC_PASSWORD, EM_SETLIMITTEXT, 16, 0);

			DBVARIANT dbv;
			if (!proto->getTString("Place", &dbv))
			{
				SetDlgItemText(hwndDlg, IDC_PLACE, dbv.ptszVal);
				MSN_FreeVariant(&dbv);
			}
			return TRUE;
		}

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_NEWMSNACCOUNTLINK) 
		{
			MSN_CallService(MS_UTILS_OPENURL, 1, (LPARAM)"https://signup.live.com");
			return TRUE;
		}

		if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus()) 
		{
			switch(LOWORD(wParam)) 
			{
			case IDC_HANDLE:			case IDC_PASSWORD:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == (UINT)PSN_APPLY) 
		{
			char  password[100], szEmail[MSN_MAX_EMAIL_LEN];
			DBVARIANT dbv;

			CMsnProto* proto = (CMsnProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

			GetDlgItemTextA(hwndDlg, IDC_HANDLE, szEmail, sizeof(szEmail));
			if (strcmp(szEmail, proto->MyOptions.szEmail))
			{
				strcpy(proto->MyOptions.szEmail, szEmail);
				proto->setString("e-mail", szEmail);
			}

			GetDlgItemTextA(hwndDlg, IDC_PASSWORD, password, sizeof(password));
			MSN_CallService(MS_DB_CRYPT_ENCODESTRING, sizeof(password),(LPARAM)password);
			if (!proto->getString("Password", &dbv))
			{
				if (strcmp(password, dbv.pszVal))
					proto->setString("Password", password);
				MSN_FreeVariant(&dbv);
			}
			else 
				proto->setString("Password", password);

			TCHAR szPlace[64];
			GetDlgItemText(hwndDlg, IDC_PLACE, szPlace, SIZEOF(szPlace));
			if (szPlace[0])
				proto->setTString("Place", szPlace);
			else
				proto->deleteSetting(NULL, "Place");

			return TRUE;
		}
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK DlgDeleteContactUI(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) 
	{
	case WM_INITDIALOG: 
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		return TRUE;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) 
		{
			int isBlock = IsDlgButtonChecked(hwndDlg, IDC_REMOVEBLOCK);
			int isHot = IsDlgButtonChecked(hwndDlg, IDC_REMOVEHOT);

			DeleteParam *param = (DeleteParam*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

			char szEmail[MSN_MAX_EMAIL_LEN];
			if (!param->proto->getStaticString(param->hContact, "e-mail", szEmail, sizeof(szEmail))) 
			{
				param->proto->MSN_AddUser(param->hContact, szEmail, 0, LIST_FL | (isHot ? LIST_REMOVE : LIST_REMOVENH));

				if (isBlock) 
				{
					param->proto->MSN_AddUser(param->hContact, szEmail, 0, LIST_AL | LIST_REMOVE);
					param->proto->MSN_AddUser(param->hContact, szEmail, 0, LIST_BL);
				}
			}
			EndDialog(hwndDlg, 1);
		}
		break;
	}

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////
// Initialize options pages

int CMsnProto::OnOptionsInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {0};

	odp.cbSize      = sizeof(odp);
	odp.position    = -790000000;
	odp.hInstance   = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MSN);
	odp.ptszTitle   = m_tszUserName;
	odp.ptszGroup   = LPGENT("Network");
	odp.ptszTab     = LPGENT("Account");
	odp.flags       = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_DONTTRANSLATE;
	odp.pfnDlgProc  = DlgProcMsnOpts;
	odp.dwInitParam = (LPARAM)this;
	MSN_CallService(MS_OPT_ADDPAGE, wParam,(LPARAM)&odp);

	odp.ptszTab     = LPGENT("Connection");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MSN_CONN);
	odp.pfnDlgProc  = DlgProcMsnConnOpts;
	MSN_CallService(MS_OPT_ADDPAGE, wParam,(LPARAM)&odp);

	odp.ptszTab      = LPGENT("Server list");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_LISTSMGR);
	odp.pfnDlgProc  = DlgProcMsnServLists;
	MSN_CallService(MS_OPT_ADDPAGE, wParam,(LPARAM)&odp);

	odp.ptszTab     = LPGENT("Notifications");
	odp.pszTemplate	= MAKEINTRESOURCEA(IDD_OPT_NOTIFY);
	odp.pfnDlgProc	= DlgProcHotmailPopUpOpts;
	MSN_CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);

	return 0;
}

INT_PTR CMsnProto::SvcCreateAccMgrUI(WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)CreateDialogParam (hInst, MAKEINTRESOURCE(IDD_ACCMGRUI), 
		(HWND)lParam, DlgProcAccMgrUI, (LPARAM)this);
}


/////////////////////////////////////////////////////////////////////////////////////////
// Load resident option values into memory

void  CMsnProto::LoadOptions(void)
{
	memset(&MyOptions, 0, sizeof(MyOptions));

	//PopUp Options
	MyOptions.ManageServer = getByte("ManageServer", TRUE) != 0;
	MyOptions.ShowErrorsAsPopups = getByte("ShowErrorsAsPopups", TRUE) != 0;
	MyOptions.SlowSend = getByte("SlowSend", FALSE) != 0;
	if (getStaticString(NULL, "e-mail", MyOptions.szEmail, sizeof(MyOptions.szEmail)))
		MyOptions.szEmail[0] = 0;
	_strlwr(MyOptions.szEmail);

	if (getStaticString(NULL, "MachineGuid", MyOptions.szMachineGuid, sizeof(MyOptions.szMachineGuid)))
	{
		char* uuid = getNewUuid();
		strcpy(MyOptions.szMachineGuid, uuid);
		setString("MachineGuid", MyOptions.szMachineGuid);
		mir_free(uuid);
	}
	strcpy(MyOptions.szMachineGuidP2P, MyOptions.szMachineGuid);
	_strlwr(MyOptions.szMachineGuidP2P);
}
