/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-22 Miranda NG team,
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

static INT_PTR CALLBACK SelectContainerDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto *pDlg = (CMsgDialog *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG: {
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);

		TranslateDialogDefault(hwndDlg);

		pDlg = (CMsgDialog*)(lParam);
		if (pDlg) {
			wchar_t szNewTitle[128];
			mir_snwprintf(szNewTitle, TranslateT("Select container for %s"), pDlg->m_cache->getNick());
			SetWindowText(hwndDlg, szNewTitle);
		}

		SendMessage(hwndDlg, DM_SC_BUILDLIST, 0, 0);
		SendDlgItemMessage(hwndDlg, IDC_NEWCONTAINERNAME, EM_LIMITTEXT, (WPARAM)CONTAINER_NAMELEN, 0);
		SendDlgItemMessage(hwndDlg, IDC_NEWCONTAINER, EM_LIMITTEXT, (WPARAM)CONTAINER_NAMELEN, 0);

		RECT rc, rcParent;
		GetWindowRect(hwndDlg, &rc);
		GetWindowRect(GetParent(hwndDlg), &rcParent);
		SetWindowPos(hwndDlg, nullptr, (rcParent.left + rcParent.right - (rc.right - rc.left)) / 2, (rcParent.top + rcParent.bottom - (rc.bottom - rc.top)) / 2, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		return TRUE;
	}

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			wchar_t szName[CONTAINER_NAMELEN + 1];
			LRESULT iItem;

			if ((iItem = SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETCURSEL, 0, 0)) != LB_ERR) {
				SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETTEXT, (WPARAM)iItem, (LPARAM)szName);
				if (IsWindow(pDlg->GetHwnd()))
					pDlg->SwitchToContainer(szName);
			}
			if (IsWindow(hwndDlg))
				DestroyWindow(hwndDlg);
			break;
		
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;
		
		case IDC_DELETECONTAINER:
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

		case IDC_RENAMECONTAINER:
			if (int iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_NEWCONTAINERNAME))) {
				wchar_t szNewName[CONTAINER_NAMELEN];
				GetDlgItemText(hwndDlg, IDC_NEWCONTAINERNAME, szNewName, _countof(szNewName));
				if (!wcsncmp(szNewName, CGlobals::m_default_container_name, CONTAINER_NAMELEN) || !wcsncmp(szNewName, TranslateT("Default container"), CONTAINER_NAMELEN)) {
					MessageBox(hwndDlg, TranslateT("You cannot rename the default container"), TranslateT("Error"), MB_OK | MB_ICONERROR);
					break;
				}

				iItem = SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_FINDSTRING, (WPARAM)-1, (LPARAM)szNewName);
				if (iItem != LB_ERR) {
					wchar_t szOldName[CONTAINER_NAMELEN + 1];
					SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETTEXT, (WPARAM)iItem, (LPARAM)szOldName);
					if (mir_wstrlen(szOldName) == mir_wstrlen(szNewName)) {
						MessageBox(nullptr, TranslateT("This name is already in use"), TranslateT("Error"), MB_OK | MB_ICONERROR);
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
								wcsncpy_s(p->m_wszName, szNewName, _TRUNCATE);
								p->Configure();
							}
						}
						SendMessage(hwndDlg, DM_SC_BUILDLIST, 0, 0);
						BuildContainerMenu();
					}
				}
			}
			break;

		case IDC_CREATENEW:
			if (int iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_NEWCONTAINER))) {
				wchar_t szNewName[CONTAINER_NAMELEN];
				GetDlgItemText(hwndDlg, IDC_NEWCONTAINER, szNewName, _countof(szNewName));
				iItem = SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_FINDSTRING, (WPARAM)-1, (LPARAM)szNewName);
				if (iItem != LB_ERR || !wcsncmp(szNewName, CGlobals::m_default_container_name, CONTAINER_NAMELEN)) {
					SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETTEXT, (WPARAM)iItem, (LPARAM)szName);
					if (mir_wstrlen(szName) == mir_wstrlen(szNewName) || !wcsncmp(szNewName, CGlobals::m_default_container_name, CONTAINER_NAMELEN)) {
						MessageBox(nullptr, TranslateT("This name is already in use"), TranslateT("Error"), MB_OK | MB_ICONERROR);
						SetFocus(GetDlgItem(hwndDlg, IDC_NEWCONTAINER));
						break;
					}
				}

				if (IsWindow(pDlg->GetHwnd())) {
					pDlg->SwitchToContainer(szNewName);
					if (IsWindow(hwndDlg))
						DestroyWindow(hwndDlg);
				}
			}
			break;

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
		TContainerData *pContainer = pDlg->m_pContainer;
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

/////////////////////////////////////////////////////////////////////////////////////////
// chooses a new container for a window

void CMsgDialog::SelectContainer()
{
	CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_SELECTCONTAINER), m_hwnd, SelectContainerDlgProc, (LPARAM)this);
}
