/* 
Copyright (C) 2010 Unsane

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#include "common.h"

static LRESULT CALLBACK MessageEditSubclassProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)  {
	case WM_CHAR:
		if (wParam == 1 && GetKeyState(VK_CONTROL) & 0x8000) 
		{	// ctrl-a
			SendMessage(hwnd, EM_SETSEL, 0, -1);
			return 0;
		}

		if (wParam == 26 && GetKeyState(VK_CONTROL) & 0x8000) 
		{	// ctrl-z
			SendMessage(hwnd, EM_UNDO, 0, 0);
			return 0;
		}

		if (wParam == 127 && GetKeyState(VK_CONTROL) & 0x8000) 
		{	// ctrl-backspace
			DWORD start, end;
			WCHAR text[1024];

			SendMessage(hwnd, EM_GETSEL, (WPARAM) & end, (LPARAM) (PDWORD) NULL);
			SendMessage(hwnd, WM_KEYDOWN, VK_LEFT, 0);
			SendMessage(hwnd, EM_GETSEL, (WPARAM) & start, (LPARAM) (PDWORD) NULL);
			GetWindowText(hwnd, text, SIZEOF(text));
			memmove(text + start, text + end, sizeof(WCHAR) * (wcslen(text) + 1 - end));
			SetWindowText(hwnd, text);
			SendMessage(hwnd, EM_SETSEL, start, start);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), EN_CHANGE), (LPARAM) hwnd);
			return 0;
		}
		break;
	}

	return mir_callNextSubclass(hwnd, MessageEditSubclassProc, msg, wParam, lParam);
}

INT_PTR CALLBACK DlgProcOptionsPage(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
	case WM_INITDIALOG:
		{
			char key[64];
			int count = 0;
			CMString replies;

			TranslateDialogDefault(hwndDlg);
			variables_skin_helpbutton(hwndDlg, IDC_VARIABLES);
			ShowWindow(GetDlgItem(hwndDlg, IDC_VARIABLES_HINT), ServiceExists(MS_VARS_FORMATSTRING));

			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_REPLIES), MessageEditSubclassProc);

			mir_snprintf(key, SIZEOF(key), "ImmediatelySend_%x", iNumber);
			CheckDlgButton(hwndDlg, IDC_IMMEDIATELY, db_get_w(NULL, MODULE, key, 1) ? BST_CHECKED : BST_UNCHECKED);

			mir_snprintf(key, SIZEOF(key), "RepliesCount_%x", iNumber);
			count = db_get_w(NULL, MODULE, key, 0);

			for (int i = 0; i < count; i++)
			{
				mir_snprintf(key, SIZEOF(key), "Reply_%x_%x", iNumber, i);
				wchar_t *value = db_get_wsa(NULL, MODULE, key);
				if (value)
				{
					replies.Append(value);
					replies.Append(_T("\r\n"));
				}
				mir_free(value);
			}
			SetDlgItemText(hwndDlg, IDC_REPLIES, replies.GetBuffer());
		}
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) {
			switch(LOWORD(wParam)) {
			case IDC_VARIABLES:
				variables_showhelp(hwndDlg, IDC_REPLIES, VHF_SIMPLEDLG, NULL, NULL);
				break;

			case IDC_IMMEDIATELY:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
		}
		break;

	case WM_NOTIFY:
		{
			NMHDR *p = ((LPNMHDR)lParam);

			switch (p->code) {
			case PSN_APPLY:
				{
					char key[64];
					int count = 0;
					wchar_t *tszReplies;

					mir_snprintf(key, SIZEOF(key), "RepliesCount_%x", iNumber);
					count = db_get_b(NULL, MODULE, key, 0);

					for (int i = 0; i < count; i++)
					{
						mir_snprintf(key, SIZEOF(key), "Reply_%x_%x", iNumber, i);
						db_unset(NULL, MODULE, key);
					}

					int length = SendDlgItemMessage(hwndDlg, IDC_REPLIES, WM_GETTEXTLENGTH, 0, 0);
					tszReplies = (wchar_t*)mir_alloc(sizeof(wchar_t)* (length + 1));
					GetDlgItemText(hwndDlg, IDC_REPLIES, tszReplies, length + 1);
					tszReplies[length] = '\0';
					{
						CMString replies = tszReplies;
						if (replies.Right(2) != _T("\r\n"))
							replies.Append(_T("\r\n"));

						count = 0;
						int pos = -1, prev = 0;
						while ((pos = replies.Find(_T("\r\n"), prev)) != -1)
						{
							mir_snprintf(key, SIZEOF(key), "Reply_%x_%x", iNumber, count++);
							db_set_ws(NULL, MODULE, key, replies.Mid(prev, pos - prev).GetBuffer());
							prev = pos + 2;
						}
					}
					mir_free(tszReplies);

					mir_snprintf(key, SIZEOF(key), "RepliesCount_%x", iNumber);
					db_set_w(NULL, MODULE, key, count);

					mir_snprintf(key, SIZEOF(key), "ImmediatelySend_%x", iNumber);
					db_set_b(NULL, MODULE, key, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_IMMEDIATELY));

					return TRUE;
				}
				break;
			}
		}
		break;
	}

	if (HIWORD(wParam) == EN_CHANGE && GetFocus() == (HWND)lParam)
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

	return FALSE;
}

int OnOptInitialized(WPARAM wParam, LPARAM lParam)
{
	char tabName[32];
	mir_snprintf(tabName, SIZEOF(tabName), "%s %x", Translate("Button"), iNumber + 1);

	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.pszGroup = LPGEN("Message Sessions");
	odp.pszTitle = LPGEN("Quick Replies");
	odp.pszTab = tabName;
	odp.position = iNumber;
	odp.hInstance = hInstance;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_PAGE);
	odp.pfnDlgProc = DlgProcOptionsPage;
	Options_AddPage(wParam, &odp);
	return 0;
}