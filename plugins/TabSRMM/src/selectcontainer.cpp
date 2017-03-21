/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-17 Miranda NG project,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
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
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// dialog to manage containers (attaching sessions to containers etc.)

#include "stdafx.h"

INT_PTR CALLBACK SelectContainerDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndMsgDlg = 0;

	hwndMsgDlg = (HWND)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG: {
		wchar_t szNewTitle[128];
		RECT rc, rcParent;

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
		hwndMsgDlg = (HWND)lParam;

		TranslateDialogDefault(hwndDlg);

		if (lParam) {
			CSrmmWindow *dat = (CSrmmWindow*)GetWindowLongPtr((HWND)lParam, GWLP_USERDATA);
			if (dat) {
				mir_snwprintf(szNewTitle, TranslateT("Select container for %s"), dat->m_cache->getNick());
				SetWindowText(hwndDlg, szNewTitle);
			}
		}

		SendMessage(hwndDlg, DM_SC_BUILDLIST, 0, 0);
		SendDlgItemMessage(hwndDlg, IDC_NEWCONTAINERNAME, EM_LIMITTEXT, (WPARAM)CONTAINER_NAMELEN, 0);
		SendDlgItemMessage(hwndDlg, IDC_NEWCONTAINER, EM_LIMITTEXT, (WPARAM)CONTAINER_NAMELEN, 0);

		GetWindowRect(hwndDlg, &rc);
		GetWindowRect(GetParent(hwndDlg), &rcParent);
		SetWindowPos(hwndDlg, 0, (rcParent.left + rcParent.right - (rc.right - rc.left)) / 2, (rcParent.top + rcParent.bottom - (rc.bottom - rc.top)) / 2, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		return TRUE;
	}

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK: {
			wchar_t szName[CONTAINER_NAMELEN];
			LRESULT iItem;

			if ((iItem = SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETCURSEL, 0, 0)) != LB_ERR) {
				SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETTEXT, (WPARAM)iItem, (LPARAM)szName);
				if (IsWindow(hwndMsgDlg))
					SendMessage(hwndMsgDlg, DM_CONTAINERSELECTED, 0, (LPARAM)szName);
			}
			if (IsWindow(hwndDlg))
				DestroyWindow(hwndDlg);
			break;
		}
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;
		case IDC_DELETECONTAINER: {
			wchar_t szName[CONTAINER_NAMELEN + 1];
			LRESULT iItem;

			if ((iItem = SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETCURSEL, 0, 0)) != LB_ERR) {
				SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETTEXT, (WPARAM)iItem, (LPARAM)szName);
				if (!wcsncmp(szName, L"default", CONTAINER_NAMELEN) || !wcsncmp(szName, TranslateT("Default container"), CONTAINER_NAMELEN))
					MessageBox(hwndDlg, TranslateT("You cannot delete the default container"), TranslateT("Error"), MB_OK | MB_ICONERROR);
				else {
					int iIndex = SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETITEMDATA, (WPARAM)iItem, 0);
					DeleteContainer(iIndex);
					SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_RESETCONTENT, 0, 0);
					SendMessage(hwndDlg, DM_SC_BUILDLIST, 0, 0);
					BuildContainerMenu();
				}
			}
			break;
		}
		case IDC_RENAMECONTAINER: {
			wchar_t szNewName[CONTAINER_NAMELEN], szName[CONTAINER_NAMELEN + 1];
			int iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_NEWCONTAINERNAME));
			if (iLen) {
				GetDlgItemText(hwndDlg, IDC_NEWCONTAINERNAME, szNewName, _countof(szNewName));
				if (!wcsncmp(szNewName, CGlobals::m_default_container_name, CONTAINER_NAMELEN) || !wcsncmp(szNewName, TranslateT("Default container"), CONTAINER_NAMELEN)) {
					MessageBox(hwndDlg, TranslateT("You cannot rename the default container"), TranslateT("Error"), MB_OK | MB_ICONERROR);
					break;
				}

				int iItem = SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_FINDSTRING, (WPARAM)-1, (LPARAM)szNewName);
				if (iItem != LB_ERR) {
					wchar_t szOldName[CONTAINER_NAMELEN + 1];
					SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETTEXT, (WPARAM)iItem, (LPARAM)szOldName);
					if (mir_wstrlen(szOldName) == mir_wstrlen(szNewName)) {
						MessageBox(0, TranslateT("This name is already in use"), TranslateT("Error"), MB_OK | MB_ICONERROR);
						SetFocus(GetDlgItem(hwndDlg, IDC_NEWCONTAINERNAME));
						break;
					}
				}
				if ((iItem = SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETCURSEL, 0, 0)) != LB_ERR) {
					SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETTEXT, (WPARAM)iItem, (LPARAM)szName);
					if (!wcsncmp(szName, L"default", CONTAINER_NAMELEN) || !wcsncmp(szName, TranslateT("Default container"), CONTAINER_NAMELEN))
						MessageBox(hwndDlg, TranslateT("You cannot rename the default container"), TranslateT("Error"), MB_OK | MB_ICONERROR);
					else {
						int iIndex = SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETITEMDATA, (WPARAM)iItem, 0);
						RenameContainer(iIndex, szNewName);
						SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_RESETCONTENT, 0, 0);
						for (TContainerData *p = pFirstContainer; p; p = p->pNext) {
							if (!wcsncmp(p->m_wszName, szName, CONTAINER_NAMELEN) && mir_wstrlen(p->m_wszName) == mir_wstrlen(szName)) {
								wcsncpy(p->m_wszName, szNewName, CONTAINER_NAMELEN);
								SendMessage(p->m_hwnd, DM_CONFIGURECONTAINER, 0, 0);
							}
						}
						SendMessage(hwndDlg, DM_SC_BUILDLIST, 0, 0);
						BuildContainerMenu();
					}
				}
			}
			break;
		}
		case IDC_CREATENEW: {
			wchar_t szNewName[CONTAINER_NAMELEN], szName[CONTAINER_NAMELEN + 1];

			int iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_NEWCONTAINER));
			if (iLen) {
				GetDlgItemText(hwndDlg, IDC_NEWCONTAINER, szNewName, _countof(szNewName));
				int iItem = SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_FINDSTRING, (WPARAM)-1, (LPARAM)szNewName);
				if (iItem != LB_ERR || !wcsncmp(szNewName, CGlobals::m_default_container_name, CONTAINER_NAMELEN)) {
					SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETTEXT, (WPARAM)iItem, (LPARAM)szName);
					if (mir_wstrlen(szName) == mir_wstrlen(szNewName) || !wcsncmp(szNewName, CGlobals::m_default_container_name, CONTAINER_NAMELEN)) {
						MessageBox(0, TranslateT("This name is already in use"), TranslateT("Error"), MB_OK | MB_ICONERROR);
						SetFocus(GetDlgItem(hwndDlg, IDC_NEWCONTAINER));
						break;
					}
				}
				if (IsWindow(hwndMsgDlg)) {
					SendMessage(hwndMsgDlg, DM_CONTAINERSELECTED, 0, (LPARAM)szNewName);
					if (IsWindow(hwndDlg))
						DestroyWindow(hwndDlg);
				}
			}
			break;
		}
		case IDC_CNTLIST:
			if (HIWORD(wParam) == LBN_DBLCLK)
				SendMessage(hwndDlg, WM_COMMAND, IDOK, 0);
			break;
		}
		break;
		
	case DM_SC_BUILDLIST: // fill the list box...
		for (int i = 0;; i++) {
			char szValue[10];
			mir_snprintf(szValue, "%d", i);
			ptrW wszName(db_get_wsa(0, "TAB_ContainersW", szValue));
			if (wszName == nullptr) // end of list
				break;

			if (wcsncmp(wszName, L"**free**", CONTAINER_NAMELEN)) {
				int iItemNew = SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_ADDSTRING, 0, (LPARAM)(!mir_wstrcmp(wszName, L"default") ? TranslateT("Default container") : wszName));
				if (iItemNew != LB_ERR)
					SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_SETITEMDATA, iItemNew, i);
			}
		}

		// highlight the name of the container to which the message window currently is assigned
		TContainerData *pContainer = nullptr;
		SendMessage(hwndMsgDlg, DM_QUERYCONTAINER, 0, (LPARAM)&pContainer);
		if (pContainer) {
			LRESULT iItem = SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_FINDSTRING, -1, 
				(LPARAM)(!mir_wstrcmp(pContainer->m_wszName, L"default") ? TranslateT("Default container") : pContainer->m_wszName));
			if (iItem != LB_ERR)
				SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_SETCURSEL, (WPARAM)iItem, 0);
		}
		break;
	}
	return FALSE;
}
