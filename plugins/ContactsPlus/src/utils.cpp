// ---------------------------------------------------------------------------
//                Contacts+ for Miranda Instant Messenger
//                _______________________________________
//
// Copyright © 2002 Dominus Procellarum
// Copyright © 2004-2008 Joe Kucera
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
//
// ---------------------------------------------------------------------------

#include "contacts.h"

/* a strlennull() that likes NULL */
size_t __fastcall strlennull(const char *string)
{
	if (string)
		return strlen(string);

	return 0;
}

int __fastcall strcmpnull(const char *str1, const char *str2)
{
	if (!str1 || !str2) return 1;

	return strcmp(str1, str2);
}

void __fastcall strcpynull(char *str1, const char *str2)
{
	if (!str2)
		str2 = "";

	strcpy(str1, str2);
}

char* __fastcall null_strdup(const char *string)
{
	if (string)
		return strdup(string);

	return NULL;
}

TCHAR* GetContactUID(MCONTACT hContact)
{
	char *szProto = GetContactProto(hContact);
	char *uid = (char*)CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
	if (INT_PTR(uid) == CALLSERVICE_NOTFOUND || uid == 0)
		return NULL;

	DBVARIANT vrUid;
	if (db_get_s(hContact, szProto, uid, &vrUid, 0))
		return NULL;

	if (vrUid.type == DBVT_DWORD) {
		TCHAR tmp[100];
		_itot(vrUid.dVal, tmp, 10);
		return mir_tstrdup(tmp);
	}

	if (vrUid.type == DBVT_ASCIIZ) {
		TCHAR *res = mir_a2t(vrUid.pszVal);
		mir_free(vrUid.pszVal);
		return res;
	}

	if (vrUid.type == DBVT_UTF8) {
		TCHAR *res = mir_utf8decodeT(vrUid.pszVal);
		mir_free(vrUid.pszVal);
		return res;
	}

	return NULL;
}

void DialogAddContactExecute(HWND hwndDlg, MCONTACT hNewContact)
{
	ADDCONTACTSTRUCT acs = { 0 };
	acs.hContact = hNewContact;
	acs.handleType = HANDLE_CONTACT;
	CallService(MS_ADDCONTACT_SHOW, (WPARAM)hwndDlg, (LPARAM)&acs);
}


void DrawProtocolIcon(HWND hwndDlg, LPARAM lParam, MCONTACT hContact)
{
	LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;

	if (dis->hwndItem == GetDlgItem(hwndDlg, IDC_PROTOCOL))
	{
		HICON hIcon = LoadContactProtoIcon(hContact);
		if (hIcon)
		{
			DrawIconEx(dis->hDC, dis->rcItem.left, dis->rcItem.top, hIcon, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0, NULL, DI_NORMAL);
			DestroyIcon(hIcon);
		}
	}
}


void UpdateDialogTitle(HWND hwndDlg, MCONTACT hContact, TCHAR *pszTitleStart)
{
	TCHAR newtitle[512];
	mir_tstrncpy(newtitle, TranslateTS(pszTitleStart), SIZEOF(newtitle));
	
	if (hContact) {
		char *szProto = GetContactProto(hContact);
		if (szProto) {
			ptrT uid(GetContactUID(hContact));
			TCHAR *contactName = pcli->pfnGetContactDisplayName(hContact, 0);

			TCHAR oldTitle[MAX_PATH];
			GetDlgItemText(hwndDlg, IDC_NAME, oldTitle, SIZEOF(oldTitle));

			if (mir_tstrcmp(uid ? uid : contactName, oldTitle))
				SetDlgItemText(hwndDlg, IDC_NAME, uid ? uid : contactName);

			TCHAR *szStatus = pcli->pfnGetStatusModeDescription(szProto == NULL ? ID_STATUS_OFFLINE : db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE), 0);
			mir_sntprintf(newtitle, 256, _T("%s %s (%s)"), TranslateTS(pszTitleStart), contactName, szStatus);
		}
	}

	SetWindowText(hwndDlg, newtitle);
}


void UpdateDialogAddButton(HWND hwndDlg, MCONTACT hContact)
{
	int bVisible = db_get_b(hContact, "CList", "NotOnList", 0);
	ShowWindow(GetDlgItem(hwndDlg, IDC_ADD), bVisible ? SW_SHOW : SW_HIDE);
}


HICON InitMButton(HWND hDlg, int idButton, LPCSTR szIcon, char *szTip)
{
	HWND hButton = GetDlgItem(hDlg, idButton);
	HICON hIcon = (HICON)LoadImageA(GetModuleHandle(NULL), szIcon, IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);

	SendMessage(hButton, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
	SendMessage(hButton, BUTTONSETASFLATBTN, TRUE, 0);
	SendMessage(hButton, BUTTONADDTOOLTIP, (WPARAM)szTip, 0);
	return hIcon;
}


HICON LoadContactProtoIcon(MCONTACT hContact)
{
	char *szProto = GetContactProto(hContact);
	if (szProto)
		return (HICON)CallProtoService(szProto, PS_LOADICON, PLI_PROTOCOL | PLIF_SMALL, 0);
	return NULL;
}


void EnableDlgItem(HWND hwndDlg, UINT control, int state)
{
	EnableWindow(GetDlgItem(hwndDlg, control), state);
}


TCHAR* GetWindowTextT(HWND hWnd)
{
	int len = GetWindowTextLength(hWnd) + 1;
	TCHAR* txt = (TCHAR*)mir_alloc(len * sizeof(TCHAR));
	if (txt) {
		txt[0] = 0;
		GetWindowText(hWnd, txt, len);
	}
	return txt;
}

TCHAR* __fastcall strdupT(const TCHAR *string)
{
	if (string)
		return (TCHAR*)wcsdup((TCHAR*)string);
	return NULL;
}
