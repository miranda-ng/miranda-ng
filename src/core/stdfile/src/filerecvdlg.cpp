/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "commonheaders.h"
#include "file.h"

#define MAX_MRU_DIRS    5

static BOOL CALLBACK ClipSiblingsChildEnumProc(HWND hwnd, LPARAM)
{
	SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE)|WS_CLIPSIBLINGS);
	return TRUE;
}

static void GetLowestExistingDirName(const TCHAR *szTestDir, TCHAR *szExistingDir, int cchExistingDir)
{
	DWORD dwAttributes;
	TCHAR *pszLastBackslash;

	mir_tstrncpy(szExistingDir, szTestDir, cchExistingDir);
	while ((dwAttributes = GetFileAttributes(szExistingDir)) != INVALID_FILE_ATTRIBUTES && !(dwAttributes&FILE_ATTRIBUTE_DIRECTORY)) {
		pszLastBackslash = _tcsrchr(szExistingDir, '\\');
		if (pszLastBackslash == NULL) { *szExistingDir = '\0'; break; }
		*pszLastBackslash = '\0';
	}
	if (szExistingDir[0] == '\0')
		GetCurrentDirectory(cchExistingDir, szExistingDir);
}

static const TCHAR InvalidFilenameChars[] = _T("\\/:*?\"<>|");
void RemoveInvalidFilenameChars(TCHAR *tszString)
{
	size_t i;
	if (tszString) {
		for (i = _tcscspn(tszString, InvalidFilenameChars); tszString[i]; i+=_tcscspn(tszString+i+1, InvalidFilenameChars)+1)
			tszString[i] = _T('_');
	}
}

static const TCHAR InvalidPathChars[] = _T("*?\"<>|"); // "\/:" are excluded as they are allowed in file path
void RemoveInvalidPathChars(TCHAR *tszString)
{
	if (tszString)
		for (size_t i = _tcscspn(tszString, InvalidPathChars); tszString[i]; i += _tcscspn(tszString + i + 1, InvalidPathChars) + 1)
			tszString[i] = _T('_');
}

static INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
	TCHAR szDir[MAX_PATH];
	switch (uMsg) {
	case BFFM_INITIALIZED:
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
		break;
	case BFFM_SELCHANGED:
		if (SHGetPathFromIDList((LPITEMIDLIST)lp, szDir))
			SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)szDir);
		break;
	}
	return 0;
}

int BrowseForFolder(HWND hwnd, TCHAR *szPath)
{
	BROWSEINFO bi = { 0 };
	bi.hwndOwner = hwnd;
	bi.pszDisplayName = szPath;
	bi.lpszTitle = TranslateT("Select folder");
	bi.ulFlags = BIF_NEWDIALOGSTYLE|BIF_EDITBOX|BIF_RETURNONLYFSDIRS;				// Use this combo instead of BIF_USENEWUI
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = (LPARAM)szPath;

	LPITEMIDLIST pidlResult = SHBrowseForFolder(&bi);
	if (pidlResult) {
		SHGetPathFromIDList(pidlResult, szPath);
		mir_tstrcat(szPath, _T("\\"));
		CoTaskMemFree(pidlResult);
	}
	return pidlResult != NULL;
}

static REPLACEVARSARRAY sttVarsToReplace[] =
{
	{ (TCHAR*)"///", (TCHAR*)"//" },
	{ (TCHAR*)"//", (TCHAR*)"/" },
	{ (TCHAR*)"()", (TCHAR*)"" },
	{ NULL, NULL }
};

static void patchDir(TCHAR *str, size_t strSize)
{
	REPLACEVARSDATA dat = { 0 };
	dat.cbSize = sizeof(dat);
	dat.dwFlags = RVF_TCHAR;
	dat.variables = sttVarsToReplace;

	TCHAR *result = (TCHAR*)CallService(MS_UTILS_REPLACEVARS, (WPARAM)str, (LPARAM)&dat);
	if (result) {
		_tcsncpy(str, result, strSize);
		mir_free(result);
	}

	size_t len = mir_tstrlen(str);
	if (len + 1 < strSize && str[len - 1] != '\\')
		mir_tstrcpy(str + len, _T("\\"));
}

void GetContactReceivedFilesDir(MCONTACT hContact, TCHAR *szDir, int cchDir, BOOL patchVars)
{
	TCHAR tszTemp[MAX_PATH];

	ptrT tszRecvPath(db_get_tsa(NULL, "SRFile", "RecvFilesDirAdv"));
	if (tszRecvPath)
		_tcsncpy_s(tszTemp, tszRecvPath, _TRUNCATE);
	else
		mir_sntprintf(tszTemp, SIZEOF(tszTemp), _T("%%mydocuments%%\\%s\\%%userid%%"), TranslateT("My received files"));

	if (hContact) {
		hContact = db_mc_tryMeta(hContact);

		REPLACEVARSDATA dat = { 0 };
		REPLACEVARSARRAY rvaVarsToReplace[4];
		rvaVarsToReplace[0].lptzKey = _T("nick");
		rvaVarsToReplace[0].lptzValue = mir_tstrdup((TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR));
		rvaVarsToReplace[1].lptzKey = _T("userid");
		rvaVarsToReplace[1].lptzValue = GetContactID(hContact);
		rvaVarsToReplace[2].lptzKey = _T("proto");
		rvaVarsToReplace[2].lptzValue = mir_a2t(GetContactProto(hContact));
		rvaVarsToReplace[3].lptzKey = NULL;
		rvaVarsToReplace[3].lptzValue = NULL;
		for (int i = 0; i < (SIZEOF(rvaVarsToReplace) - 1); i++)
			RemoveInvalidFilenameChars(rvaVarsToReplace[i].lptzValue);

		dat.cbSize = sizeof(dat);
		dat.dwFlags = RVF_TCHAR;
		dat.variables = rvaVarsToReplace;
		dat.hContact = hContact;
		TCHAR *result = (TCHAR*)CallService(MS_UTILS_REPLACEVARS, (WPARAM)tszTemp, (LPARAM)&dat);
		if (result) {
			_tcsncpy(tszTemp, result, SIZEOF(tszTemp));
			mir_free(result);
			for (int i = 0; i < (SIZEOF(rvaVarsToReplace) - 1); i++)
				mir_free(rvaVarsToReplace[i].lptzValue);
		}
	}

	if (patchVars)
		patchDir(tszTemp, SIZEOF(tszTemp));
	RemoveInvalidPathChars(tszTemp);
	mir_tstrncpy(szDir, tszTemp, cchDir);
}

void GetReceivedFilesDir(TCHAR *szDir, int cchDir)
{
	TCHAR tszTemp[MAX_PATH];

	ptrT tszRecvPath(db_get_tsa(NULL, "SRFile", "RecvFilesDirAdv"));
	if (tszRecvPath)
		_tcsncpy_s(tszTemp, tszRecvPath, _TRUNCATE);
	else
		mir_sntprintf(tszTemp, SIZEOF(tszTemp), _T("%%mydocuments%%\\%s\\%%userid%%"), TranslateT("My received files"));

	patchDir(tszTemp, SIZEOF(tszTemp));
	RemoveInvalidPathChars(tszTemp);
	mir_tstrncpy(szDir, tszTemp, cchDir);
}

INT_PTR CALLBACK DlgProcRecvFile(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	FileDlgData *dat = (FileDlgData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			TCHAR szPath[450];
			CLISTEVENT* cle = (CLISTEVENT*)lParam;

			dat = (FileDlgData*)mir_calloc(sizeof(FileDlgData));
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			dat->hContact = cle->hContact;
			dat->hDbEvent = cle->hDbEvent;
			dat->hNotifyEvent = HookEventMessage(ME_PROTO_ACK, hwndDlg, HM_RECVEVENT);
			dat->hPreshutdownEvent = HookEventMessage(ME_SYSTEM_PRESHUTDOWN, hwndDlg, M_PRESHUTDOWN);
			dat->dwTicks = GetTickCount();

			EnumChildWindows(hwndDlg, ClipSiblingsChildEnumProc, 0);

			Window_SetIcon_IcoLib(hwndDlg, SKINICON_EVENT_FILE);
			Button_SetIcon_IcoLib(hwndDlg, IDC_ADD, SKINICON_OTHER_ADDCONTACT, LPGEN("Add contact permanently to list"));
			Button_SetIcon_IcoLib(hwndDlg, IDC_DETAILS, SKINICON_OTHER_USERDETAILS, LPGEN("View user's details"));
			Button_SetIcon_IcoLib(hwndDlg, IDC_HISTORY, SKINICON_OTHER_HISTORY, LPGEN("View user's history"));
			Button_SetIcon_IcoLib(hwndDlg, IDC_USERMENU, SKINICON_OTHER_DOWNARROW, LPGEN("User menu"));

			TCHAR *contactName = pcli->pfnGetContactDisplayName(dat->hContact, 0);
			SetDlgItemText(hwndDlg, IDC_FROM, contactName);
			GetContactReceivedFilesDir(dat->hContact, szPath, SIZEOF(szPath), TRUE);
			SetDlgItemText(hwndDlg, IDC_FILEDIR, szPath);
			SHAutoComplete(GetWindow(GetDlgItem(hwndDlg, IDC_FILEDIR), GW_CHILD), 1);

			for (int i = 0; i < MAX_MRU_DIRS; i++) {
				char idstr[32];
				mir_snprintf(idstr, SIZEOF(idstr), "MruDir%d", i);

				DBVARIANT dbv;
				if (db_get_ts(NULL, "SRFile", idstr, &dbv))
					break;
				SendDlgItemMessage(hwndDlg, IDC_FILEDIR, CB_ADDSTRING, 0, (LPARAM)dbv.ptszVal);
				db_free(&dbv);
			}

			db_event_markRead(dat->hContact, dat->hDbEvent);

			DBEVENTINFO dbei = { sizeof(dbei) };
			dbei.cbBlob = db_event_getBlobSize(dat->hDbEvent);
			if (dbei.cbBlob > 4 && dbei.cbBlob <= 8196) {
				dbei.pBlob = (PBYTE)alloca(dbei.cbBlob + 1);
				db_event_get(dat->hDbEvent, &dbei);
				dbei.pBlob[dbei.cbBlob] = 0;
				dat->fs = cle->lParam ? (HANDLE)cle->lParam : (HANDLE)*(PDWORD)dbei.pBlob;

				char *str = (char*)dbei.pBlob + 4;
				ptrT ptszFileName(DbGetEventStringT(&dbei, str));
				SetDlgItemText(hwndDlg, IDC_FILENAMES, ptszFileName);

				unsigned len = (unsigned)mir_strlen(str) + 1;
				if (len + 4 < dbei.cbBlob) {
					str += len;
					ptrT ptszDescription(DbGetEventStringT(&dbei, str));
					SetDlgItemText(hwndDlg, IDC_MSG, ptszDescription);
				}
			}
			else DestroyWindow(hwndDlg);

			TCHAR datetimestr[64];
			tmi.printTimeStamp(NULL, dbei.timestamp, _T("t d"), datetimestr, SIZEOF(datetimestr), 0);
			SetDlgItemText(hwndDlg, IDC_DATE, datetimestr);

			char* szProto = GetContactProto(dat->hContact);
			if (szProto) {
				int hasName = 0;
				char buf[128];

				CONTACTINFO ci = { 0 };
				ci.cbSize = sizeof(ci);
				ci.hContact = dat->hContact;
				ci.szProto = szProto;
				ci.dwFlag = CNF_UNIQUEID;
				if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) {
					switch (ci.type) {
					case CNFT_ASCIIZ:
						hasName = 1;
						strncpy_s(buf, (char*)ci.pszVal, _TRUNCATE);
						mir_free(ci.pszVal);
						break;
					case CNFT_DWORD:
						hasName = 1;
						mir_snprintf(buf, SIZEOF(buf), "%u", ci.dVal);
						break;
					}
				}
				if (hasName)
					SetDlgItemTextA(hwndDlg, IDC_NAME, buf);
				else
					SetDlgItemText(hwndDlg, IDC_NAME, contactName);
			}

			if (db_get_b(dat->hContact, "CList", "NotOnList", 0)) {
				RECT rcBtn1, rcBtn2, rcDateCtrl;
				GetWindowRect(GetDlgItem(hwndDlg, IDC_ADD), &rcBtn1);
				GetWindowRect(GetDlgItem(hwndDlg, IDC_USERMENU), &rcBtn2);
				GetWindowRect(GetDlgItem(hwndDlg, IDC_DATE), &rcDateCtrl);
				SetWindowPos(GetDlgItem(hwndDlg, IDC_DATE), 0, 0, 0, rcDateCtrl.right - rcDateCtrl.left - (rcBtn2.left - rcBtn1.left), rcDateCtrl.bottom - rcDateCtrl.top, SWP_NOZORDER | SWP_NOMOVE);
			}
			else if (db_get_b(NULL, "SRFile", "AutoAccept", 0)) {
				//don't check auto-min here to fix BUG#647620
				PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), (LPARAM)GetDlgItem(hwndDlg, IDOK));
			}
			if (!db_get_b(dat->hContact, "CList", "NotOnList", 0))
				ShowWindow(GetDlgItem(hwndDlg, IDC_ADD), SW_HIDE);
		}
		return TRUE;

	case WM_MEASUREITEM:
		return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			if (dis->hwndItem == GetDlgItem(hwndDlg, IDC_PROTOCOL)) {
				char *szProto = GetContactProto(dat->hContact);
				if (szProto) {
					HICON hIcon = (HICON)CallProtoService(szProto, PS_LOADICON, PLI_PROTOCOL|PLIF_SMALL, 0);
					if (hIcon) {
						DrawIconEx(dis->hDC, dis->rcItem.left, dis->rcItem.top, hIcon, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0, NULL, DI_NORMAL);
						DestroyIcon(hIcon);
					}
				}
			}
		}
		return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);

	case WM_COMMAND:
		if (CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM)dat->hContact))
			break;

		switch (LOWORD(wParam)) {
		case IDC_FILEDIRBROWSE:
			{
				TCHAR szDirName[MAX_PATH], szExistingDirName[MAX_PATH];

				GetDlgItemText(hwndDlg, IDC_FILEDIR, szDirName, SIZEOF(szDirName));
				GetLowestExistingDirName(szDirName, szExistingDirName, SIZEOF(szExistingDirName));
				if (BrowseForFolder(hwndDlg, szExistingDirName))
					SetDlgItemText(hwndDlg, IDC_FILEDIR, szExistingDirName);
			}
			break;

		case IDOK:
			{	//most recently used directories
				TCHAR szRecvDir[MAX_PATH], szDefaultRecvDir[MAX_PATH];
				GetDlgItemText(hwndDlg, IDC_FILEDIR, szRecvDir, SIZEOF(szRecvDir));
				RemoveInvalidPathChars(szRecvDir);
				GetContactReceivedFilesDir(NULL, szDefaultRecvDir, SIZEOF(szDefaultRecvDir), TRUE);
				if (_tcsnicmp(szRecvDir, szDefaultRecvDir, mir_tstrlen(szDefaultRecvDir))) {
					char idstr[32];
					int i;
					DBVARIANT dbv;
					for (i = MAX_MRU_DIRS-2;i>=0;i--) {
						mir_snprintf(idstr, SIZEOF(idstr), "MruDir%d", i);
						if (db_get_ts(NULL, "SRFile", idstr, &dbv)) continue;
						mir_snprintf(idstr, SIZEOF(idstr), "MruDir%d", i+1);
						db_set_ts(NULL, "SRFile", idstr, dbv.ptszVal);
						db_free(&dbv);
					}
					db_set_ts(NULL, "SRFile", idstr, szRecvDir);
				}
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAMES), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_MSG), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILEDIR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILEDIRBROWSE), FALSE);

			GetDlgItemText(hwndDlg, IDC_FILEDIR, dat->szSavePath, SIZEOF(dat->szSavePath));
			GetDlgItemText(hwndDlg, IDC_FILE, dat->szFilenames, SIZEOF(dat->szFilenames));
			GetDlgItemText(hwndDlg, IDC_MSG, dat->szMsg, SIZEOF(dat->szMsg));
			dat->hwndTransfer = FtMgr_AddTransfer(dat);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
			//check for auto-minimize here to fix BUG#647620
			if (db_get_b(NULL, "SRFile", "AutoAccept", 0) && db_get_b(NULL, "SRFile", "AutoMin", 0)) {
				ShowWindow(hwndDlg, SW_HIDE);
				ShowWindow(hwndDlg, SW_SHOWMINNOACTIVE);
			}
			DestroyWindow(hwndDlg);
			break;

		case IDCANCEL:
			if (dat->fs) CallContactService(dat->hContact, PSS_FILEDENY, (WPARAM)dat->fs, (LPARAM)TranslateT("Canceled"));
			dat->fs = NULL; /* the protocol will free the handle */
			DestroyWindow(hwndDlg);
			break;

		case IDC_ADD:
			{
				ADDCONTACTSTRUCT acs = { 0 };
				acs.hContact = dat->hContact;
				acs.handleType = HANDLE_CONTACT;
				acs.szProto = "";
				CallService(MS_ADDCONTACT_SHOW, (WPARAM)hwndDlg, (LPARAM)&acs);
				if (!db_get_b(dat->hContact, "CList", "NotOnList", 0))
					ShowWindow(GetDlgItem(hwndDlg, IDC_ADD), SW_HIDE);
			}
			break;

		case IDC_USERMENU:
			{
				RECT rc;
				HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)dat->hContact, 0);
				GetWindowRect((HWND)lParam, &rc);
				TrackPopupMenu(hMenu, 0, rc.left, rc.bottom, 0, hwndDlg, NULL);
				DestroyMenu(hMenu);
			}
			break;

		case IDC_DETAILS:
			CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)dat->hContact, 0);
			break;

		case IDC_HISTORY:
			CallService(MS_HISTORY_SHOWCONTACTHISTORY, (WPARAM)dat->hContact, 0);
			break;
		}
		break;

	case HM_RECVEVENT:
		{
			ACKDATA *ack = (ACKDATA*)lParam;
			if ((ack == NULL) || (ack->hProcess != dat->fs) || (ack->type != ACKTYPE_FILE) || (ack->hContact != dat->hContact))
				break;

			if (ack->result == ACKRESULT_DENIED || ack->result == ACKRESULT_FAILED) {
				EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILEDIR), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILEDIRBROWSE), FALSE);
				SetDlgItemText(hwndDlg, IDC_MSG, TranslateT("This file transfer has been canceled by the other side"));
				SkinPlaySound("FileDenied");
				FlashWindow(hwndDlg, TRUE);
			}
			else if (ack->result != ACKRESULT_FILERESUME)
			{
				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)GetDlgItem(hwndDlg, IDCANCEL));
			}
		}
		break;

	case WM_DESTROY:
		Window_FreeIcon_IcoLib(hwndDlg);
		Button_FreeIcon_IcoLib(hwndDlg, IDC_ADD);
		Button_FreeIcon_IcoLib(hwndDlg, IDC_DETAILS);
		Button_FreeIcon_IcoLib(hwndDlg, IDC_HISTORY);
		Button_FreeIcon_IcoLib(hwndDlg, IDC_USERMENU);

		FreeFileDlgData(dat);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		break;
	}
	return FALSE;
}
