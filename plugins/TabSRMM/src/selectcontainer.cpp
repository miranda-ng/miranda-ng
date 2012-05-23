/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2009 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of tabSRMM messaging plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: selectcontainer.cpp 12351 2010-08-21 21:44:54Z Michael.Kunz@s2005.TU-Chemnitz.de $
 *
 * dialog to manage containers (attaching sessions to containers etc.)
 *
 */

#include "commonheaders.h"
#pragma hdrstop

INT_PTR CALLBACK SelectContainerDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndMsgDlg = 0;

	hwndMsgDlg = (HWND) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG: {
			TCHAR szNewTitle[128];
			RECT rc, rcParent;
			struct TContainerData *pContainer = 0;

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) lParam);
			hwndMsgDlg = (HWND) lParam;

			TranslateDialogDefault(hwndDlg);

			if (lParam) {
				struct TWindowData *dat = (struct TWindowData *)GetWindowLongPtr((HWND)lParam, GWLP_USERDATA);
				if (dat) {
					mir_sntprintf(szNewTitle, safe_sizeof(szNewTitle), CTranslator::get(CTranslator::CNT_SELECT_FOR), dat->cache->getNick());
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
					TCHAR szName[CONTAINER_NAMELEN];
					LRESULT iItem;

					if ((iItem = SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETCURSEL, 0, 0)) != LB_ERR) {
						SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETTEXT, (WPARAM) iItem, (LPARAM) szName);
						if (IsWindow(hwndMsgDlg))
							SendMessage(hwndMsgDlg, DM_CONTAINERSELECTED, 0, (LPARAM) szName);
					}
					if (IsWindow(hwndDlg))
						DestroyWindow(hwndDlg);
					break;
				}
				case IDCANCEL:
					DestroyWindow(hwndDlg);
					break;
				case IDC_DELETECONTAINER: {
					TCHAR szName[CONTAINER_NAMELEN + 1];
					LRESULT iItem;

					if ((iItem = SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETCURSEL, 0, 0)) != LB_ERR) {
						SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETTEXT, (WPARAM) iItem, (LPARAM) szName);
						if (!_tcsncmp(szName, _T("default"), CONTAINER_NAMELEN) || !_tcsncmp(szName, CTranslator::get(CTranslator::GEN_DEFAULT_CONTAINER_NAME), CONTAINER_NAMELEN))
							MessageBox(hwndDlg, CTranslator::get(CTranslator::CNT_SELECT_DELETEERROR), _T("Error"), MB_OK | MB_ICONERROR);
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
					TCHAR szNewName[CONTAINER_NAMELEN], szName[CONTAINER_NAMELEN + 1];
					int iLen, iItem;
					struct TContainerData *pCurrent = pFirstContainer;

					iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_NEWCONTAINERNAME));
					if (iLen) {
						GetWindowText(GetDlgItem(hwndDlg, IDC_NEWCONTAINERNAME), szNewName, CONTAINER_NAMELEN);
						if(!_tcsncmp(szNewName, CGlobals::m_default_container_name, CONTAINER_NAMELEN) || !_tcsncmp(szNewName, CTranslator::get(CTranslator::GEN_DEFAULT_CONTAINER_NAME), CONTAINER_NAMELEN)) {
							MessageBox(hwndDlg, CTranslator::get(CTranslator::CNT_SELECT_RENAMEERROR), _T("Error"), MB_OK | MB_ICONERROR);
							break;
						}

						iItem = SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_FINDSTRING, (WPARAM) - 1, (LPARAM) szNewName);
						if (iItem != LB_ERR) {
							TCHAR szOldName[CONTAINER_NAMELEN + 1];
							SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETTEXT, (WPARAM) iItem, (LPARAM) szOldName);
							if (lstrlen(szOldName) == lstrlen(szNewName)) {
								MessageBox(0, CTranslator::get(CTranslator::CNT_SELECT_INUSE), _T("Error"), MB_OK | MB_ICONERROR);
								SetFocus(GetDlgItem(hwndDlg, IDC_NEWCONTAINERNAME));
								break;
							}
						}
						if ((iItem = SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETCURSEL, 0, 0)) != LB_ERR) {
							SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETTEXT, (WPARAM) iItem, (LPARAM) szName);
							if (!_tcsncmp(szName, _T("default"), CONTAINER_NAMELEN) || !_tcsncmp(szName, CTranslator::get(CTranslator::GEN_DEFAULT_CONTAINER_NAME), CONTAINER_NAMELEN))
								MessageBox(hwndDlg, CTranslator::get(CTranslator::CNT_SELECT_RENAMEERROR), _T("Error"), MB_OK | MB_ICONERROR);
							else {
								int iIndex = SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETITEMDATA, (WPARAM)iItem, 0);
								RenameContainer(iIndex, szNewName);
								SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_RESETCONTENT, 0, 0);
								while (pCurrent) {
									if (!_tcsncmp(pCurrent->szName, szName, CONTAINER_NAMELEN) && lstrlen(pCurrent->szName) == lstrlen(szName)) {
										_tcsncpy(pCurrent->szName, szNewName, CONTAINER_NAMELEN);
										SendMessage(pCurrent->hwnd, DM_CONFIGURECONTAINER, 0, 0);
									}
									pCurrent = pCurrent->pNextContainer;
								}
								SendMessage(hwndDlg, DM_SC_BUILDLIST, 0, 0);
								BuildContainerMenu();
							}
						}
					}
					break;
				}
				case IDC_CREATENEW: {
					int iLen, iItem;
					TCHAR szNewName[CONTAINER_NAMELEN], szName[CONTAINER_NAMELEN + 1];

					iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_NEWCONTAINER));
					if (iLen) {
						GetWindowText(GetDlgItem(hwndDlg, IDC_NEWCONTAINER), szNewName, CONTAINER_NAMELEN);
						iItem = SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_FINDSTRING, (WPARAM) - 1, (LPARAM) szNewName);
						if (iItem != LB_ERR || !_tcsncmp(szNewName, CGlobals::m_default_container_name, CONTAINER_NAMELEN)) {
							SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_GETTEXT, (WPARAM)iItem, (LPARAM)szName);
							if (lstrlen(szName) == lstrlen(szNewName) || !_tcsncmp(szNewName, CGlobals::m_default_container_name, CONTAINER_NAMELEN)) {
								MessageBox(0, CTranslator::get(CTranslator::CNT_SELECT_INUSE), _T("Error"), MB_OK | MB_ICONERROR);
								SetFocus(GetDlgItem(hwndDlg, IDC_NEWCONTAINER));
								break;
							}
						}
						if (IsWindow(hwndMsgDlg)) {
							SendMessage(hwndMsgDlg, DM_CONTAINERSELECTED, 0, (LPARAM) szNewName);
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
			/*
			 * fill the list box...
			 */
		case DM_SC_BUILDLIST: {
			DBVARIANT dbv;
			int iCounter = 0, iItemNew;
			char *szKey = "TAB_ContainersW";
			char szValue[10];
			struct TContainerData *pContainer = 0;
			do {
				_snprintf(szValue, 8, "%d", iCounter);
				if (M->GetTString(NULL, szKey, szValue, &dbv))
					break;          // end of list
				if (dbv.type == DBVT_ASCIIZ || dbv.type == DBVT_WCHAR) {
					if (_tcsncmp(dbv.ptszVal, _T("**free**"), CONTAINER_NAMELEN)) {
						iItemNew = SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_ADDSTRING, 0, (LPARAM)(!_tcscmp(dbv.ptszVal, _T("default")) ?
													  CTranslator::get(CTranslator::GEN_DEFAULT_CONTAINER_NAME) : dbv.ptszVal));
						if (iItemNew != LB_ERR)
							SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_SETITEMDATA, (WPARAM)iItemNew, (LPARAM)iCounter);
					}
					DBFreeVariant(&dbv);
				}
			} while (++iCounter);

			/*
			 * highlight the name of the container to which the message window currently is assigned
			 */

			SendMessage(hwndMsgDlg, DM_QUERYCONTAINER, 0, (LPARAM)&pContainer);
			if (pContainer) {
				LRESULT iItem;

				iItem = SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_FINDSTRING, (WPARAM) - 1, (LPARAM)(!_tcscmp(pContainer->szName, _T("default")) ?
										   CTranslator::get(CTranslator::GEN_DEFAULT_CONTAINER_NAME) : pContainer->szName));
				if (iItem != LB_ERR)
					SendDlgItemMessage(hwndDlg, IDC_CNTLIST, LB_SETCURSEL, (WPARAM) iItem, 0);
			}
		}
		break;
	}
	return FALSE;
}

