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

#include "stdafx.h"

/* a strlennull() that likes NULL */
size_t __fastcall strlennull(const char *string)
{
	if (string)
		return mir_strlen(string);

	return 0;
}

int __fastcall strcmpnull(const char *str1, const char *str2)
{
	if (!str1 || !str2) return 1;

	return mir_strcmp(str1, str2);
}

void __fastcall strcpynull(char *str1, const char *str2)
{
	if (!str2)
		str2 = "";

	mir_strcpy(str1, str2);
}

char* __fastcall null_strdup(const char *string)
{
	if (string)
		return strdup(string);

	return nullptr;
}

wchar_t* GetContactUID(MCONTACT hContact)
{
	char *szProto = Proto_GetBaseAccountName(hContact);
	const char *uid = Proto_GetUniqueId(szProto);
	if (uid == nullptr)
		return nullptr;

	DBVARIANT vrUid;
	if (db_get_s(hContact, szProto, uid, &vrUid, 0))
		return nullptr;

	if (vrUid.type == DBVT_DWORD) {
		wchar_t tmp[100];
		_itow(vrUid.dVal, tmp, 10);
		return mir_wstrdup(tmp);
	}

	if (vrUid.type == DBVT_ASCIIZ) {
		wchar_t *res = mir_a2u(vrUid.pszVal);
		mir_free(vrUid.pszVal);
		return res;
	}

	if (vrUid.type == DBVT_UTF8) {
		wchar_t *res = mir_utf8decodeW(vrUid.pszVal);
		mir_free(vrUid.pszVal);
		return res;
	}

	return nullptr;
}

void DrawProtocolIcon(HWND hwndDlg, LPARAM lParam, MCONTACT hContact)
{
	LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;

	if (dis->hwndItem == GetDlgItem(hwndDlg, IDC_PROTOCOL))
	{
		HICON hIcon = LoadContactProtoIcon(hContact);
		if (hIcon)
		{
			DrawIconEx(dis->hDC, dis->rcItem.left, dis->rcItem.top, hIcon, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0, nullptr, DI_NORMAL);
			DestroyIcon(hIcon);
		}
	}
}


void UpdateDialogTitle(HWND hwndDlg, MCONTACT hContact, wchar_t *pszTitleStart)
{
	wchar_t newtitle[512];
	mir_wstrncpy(newtitle, TranslateW(pszTitleStart), _countof(newtitle));
	
	if (hContact) {
		char *szProto = Proto_GetBaseAccountName(hContact);
		if (szProto) {
			ptrW uid(GetContactUID(hContact));
			wchar_t *contactName = Clist_GetContactDisplayName(hContact);

			wchar_t oldTitle[MAX_PATH];
			GetDlgItemText(hwndDlg, IDC_NAME, oldTitle, _countof(oldTitle));

			if (mir_wstrcmp(uid ? uid : contactName, oldTitle))
				SetDlgItemText(hwndDlg, IDC_NAME, uid ? uid : contactName);

			wchar_t *szStatus = Clist_GetStatusModeDescription(db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE), 0);
			mir_snwprintf(newtitle, L"%s %s (%s)", TranslateW(pszTitleStart), contactName, szStatus);
		}
	}

	SetWindowText(hwndDlg, newtitle);
}


void UpdateDialogAddButton(HWND hwndDlg, MCONTACT hContact)
{
	int bVisible = !Contact::OnList(hContact);
	ShowWindow(GetDlgItem(hwndDlg, IDC_ADD), bVisible ? SW_SHOW : SW_HIDE);
}


HICON InitMButton(HWND hDlg, int idButton, int idIcon, wchar_t *szTip)
{
	HWND hButton = GetDlgItem(hDlg, idButton);
	HICON hIcon = Skin_LoadIcon(idIcon);

	SendMessage(hButton, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
	SendMessage(hButton, BUTTONSETASFLATBTN, TRUE, 0);
	SendMessage(hButton, BUTTONADDTOOLTIP, (WPARAM)szTip, BATF_UNICODE);
	return hIcon;
}


HICON LoadContactProtoIcon(MCONTACT hContact)
{
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto)
		return (HICON)CallProtoService(szProto, PS_LOADICON, PLI_PROTOCOL | PLIF_SMALL, 0);
	return nullptr;
}


void EnableDlgItem(HWND hwndDlg, UINT control, int state)
{
	EnableWindow(GetDlgItem(hwndDlg, control), state);
}


wchar_t* GetWindowTextT(HWND hWnd)
{
	int len = GetWindowTextLength(hWnd) + 1;
	wchar_t* txt = (wchar_t*)mir_alloc(len * sizeof(wchar_t));
	if (txt) {
		txt[0] = 0;
		GetWindowText(hWnd, txt, len);
	}
	return txt;
}

wchar_t* __fastcall strdupT(const wchar_t *string)
{
	if (string)
		return (wchar_t*)wcsdup((wchar_t*)string);
	return nullptr;
}
