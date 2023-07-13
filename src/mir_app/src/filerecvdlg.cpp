/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org),
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

#include "stdafx.h"
#include "file.h"

#define MAX_MRU_DIRS    5

static BOOL CALLBACK ClipSiblingsChildEnumProc(HWND hwnd, LPARAM)
{
	SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE)|WS_CLIPSIBLINGS);
	return TRUE;
}

static void GetLowestExistingDirName(const wchar_t *szTestDir, wchar_t *szExistingDir, int cchExistingDir)
{
	uint32_t dwAttributes;
	wchar_t *pszLastBackslash;

	mir_wstrncpy(szExistingDir, szTestDir, cchExistingDir);
	while ((dwAttributes = GetFileAttributes(szExistingDir)) != INVALID_FILE_ATTRIBUTES && !(dwAttributes&FILE_ATTRIBUTE_DIRECTORY)) {
		pszLastBackslash = wcsrchr(szExistingDir, '\\');
		if (pszLastBackslash == nullptr) { *szExistingDir = '\0'; break; }
		*pszLastBackslash = '\0';
	}
	if (szExistingDir[0] == '\0')
		GetCurrentDirectory(cchExistingDir, szExistingDir);
}

static const wchar_t InvalidFilenameChars[] = L"\\/:*?\"<>|";
void RemoveInvalidFilenameChars(wchar_t *tszString)
{
	size_t i;
	if (tszString) {
		for (i = wcscspn(tszString, InvalidFilenameChars); tszString[i]; i+=wcscspn(tszString+i+1, InvalidFilenameChars)+1)
			tszString[i] = '_';
	}
}

static const wchar_t InvalidPathChars[] = L"*?\"<>|"; // "\/:" are excluded as they are allowed in file path
void RemoveInvalidPathChars(wchar_t *tszString)
{
	if (tszString)
		for (size_t i = wcscspn(tszString, InvalidPathChars); tszString[i]; i += wcscspn(tszString + i + 1, InvalidPathChars) + 1)
			tszString[i] = '_';
}

static INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
	wchar_t szDir[MAX_PATH];
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

int BrowseForFolder(HWND hwnd, wchar_t *szPath)
{
	BROWSEINFO bi = {};
	bi.hwndOwner = hwnd;
	bi.pszDisplayName = szPath;
	bi.lpszTitle = TranslateT("Select folder");
	bi.ulFlags = BIF_NEWDIALOGSTYLE|BIF_EDITBOX|BIF_RETURNONLYFSDIRS;				// Use this combo instead of BIF_USENEWUI
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = (LPARAM)szPath;

	LPITEMIDLIST pidlResult = SHBrowseForFolder(&bi);
	if (pidlResult) {
		SHGetPathFromIDList(pidlResult, szPath);
		mir_wstrcat(szPath, L"\\");
		CoTaskMemFree(pidlResult);
	}
	return pidlResult != nullptr;
}

static REPLACEVARSARRAY sttVarsToReplace[] =
{
	{ "///", "//" },
	{ "//", "/" },
	{ "()", "" },
	{ nullptr, nullptr }
};

static void patchDir(wchar_t *str, size_t strSize)
{
	wchar_t *result = Utils_ReplaceVarsW(str, 0, sttVarsToReplace);
	if (result) {
		wcsncpy(str, result, strSize);
		mir_free(result);
	}

	size_t len = mir_wstrlen(str);
	if (len + 1 < strSize && str[len - 1] != '\\')
		mir_wstrcpy(str + len, L"\\");
}

void GetContactReceivedFilesDir(MCONTACT hContact, wchar_t *szDir, int cchDir, BOOL patchVars)
{
	wchar_t tszTemp[MAX_PATH];

	ptrW tszRecvPath(g_plugin.getWStringA("RecvFilesDirAdv"));
	if (tszRecvPath)
		wcsncpy_s(tszTemp, tszRecvPath, _TRUNCATE);
	else
		mir_snwprintf(tszTemp, L"%%mydocuments%%\\%s\\%%userid%%", TranslateT("My received files"));

	if (hContact) {
		hContact = db_mc_tryMeta(hContact);

		REPLACEVARSARRAY rvaVarsToReplace[4];
		rvaVarsToReplace[0].key.w = L"nick";
		rvaVarsToReplace[0].value.w = mir_wstrdup(Clist_GetContactDisplayName(hContact));
		rvaVarsToReplace[1].key.w = L"userid";
		rvaVarsToReplace[1].value.w = Contact::GetInfo(CNF_UNIQUEID, hContact);
		rvaVarsToReplace[2].key.w = L"proto";
		rvaVarsToReplace[2].value.w = mir_a2u(Proto_GetBaseAccountName(hContact));
		rvaVarsToReplace[3].key.w = nullptr;
		rvaVarsToReplace[3].value.w = nullptr;
		for (int i = 0; i < (_countof(rvaVarsToReplace) - 1); i++)
			RemoveInvalidFilenameChars(rvaVarsToReplace[i].value.w);

		wchar_t *result = Utils_ReplaceVarsW(tszTemp, hContact, rvaVarsToReplace);
		if (result) {
			wcsncpy(tszTemp, result, _countof(tszTemp));
			mir_free(result);
			for (int i = 0; i < (_countof(rvaVarsToReplace) - 1); i++)
				mir_free(rvaVarsToReplace[i].value.w);
		}
	}

	if (patchVars)
		patchDir(tszTemp, _countof(tszTemp));
	RemoveInvalidPathChars(tszTemp);
	mir_wstrncpy(szDir, tszTemp, cchDir);
}

void GetReceivedFilesDir(wchar_t *szDir, int cchDir)
{
	wchar_t tszTemp[MAX_PATH];

	ptrW tszRecvPath(g_plugin.getWStringA("RecvFilesDirAdv"));
	if (tszRecvPath)
		wcsncpy_s(tszTemp, tszRecvPath, _TRUNCATE);
	else
		mir_snwprintf(tszTemp, L"%%mydocuments%%\\%s\\%%userid%%", TranslateT("My received files"));

	patchDir(tszTemp, _countof(tszTemp));
	RemoveInvalidPathChars(tszTemp);
	mir_wstrncpy(szDir, tszTemp, cchDir);
}

class CRecvFileDlg : public CDlgBase
{
	FileDlgData *dat;
	LPARAM m_lParam;

	CCtrlButton btnCancel, btnBrowse;
	CCtrlMButton btnAdd, btnUserMenu, btnDetails, btnHistory;

public:
	CRecvFileDlg(CLISTEVENT *cle) :
		CDlgBase(g_plugin, IDD_FILERECV),
		btnAdd(this, IDC_ADD, SKINICON_OTHER_ADDCONTACT, LPGEN("Add contact permanently to list")),
		btnCancel(this, IDCANCEL),
		btnBrowse(this, IDC_FILEDIRBROWSE),
		btnDetails(this, IDC_DETAILS, SKINICON_OTHER_USERDETAILS, LPGEN("View user's details")),
		btnHistory(this, IDC_HISTORY, SKINICON_OTHER_HISTORY, LPGEN("View user's history")),
		btnUserMenu(this, IDC_USERMENU, SKINICON_OTHER_DOWNARROW, LPGEN("User menu"))
	{
		dat = new FileDlgData();
		dat->hContact = cle->hContact;
		dat->hDbEvent = cle->hDbEvent;
		dat->dwTicks = GetTickCount();
		m_lParam = cle->lParam;

		btnAdd.OnClick = Callback(this, &CRecvFileDlg::onClick_Add);
		btnCancel.OnClick = Callback(this, &CRecvFileDlg::onClick_Cancel);
		btnBrowse.OnClick = Callback(this, &CRecvFileDlg::onClick_Browse);
		btnDetails.OnClick = Callback(this, &CRecvFileDlg::onClick_Details);
		btnHistory.OnClick = Callback(this, &CRecvFileDlg::onClick_History);
		btnUserMenu.OnClick = Callback(this, &CRecvFileDlg::onClick_UserMenu);
	}

	bool OnInitDialog() override
	{
		char *szProto = Proto_GetBaseAccountName(dat->hContact);

		dat->hNotifyEvent = HookEventMessage(ME_PROTO_ACK, m_hwnd, HM_RECVEVENT);
		dat->hPreshutdownEvent = HookEventMessage(ME_SYSTEM_PRESHUTDOWN, m_hwnd, M_PRESHUTDOWN);

		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)dat);

		EnumChildWindows(m_hwnd, ClipSiblingsChildEnumProc, 0);

		Window_SetSkinIcon_IcoLib(m_hwnd, SKINICON_EVENT_FILE);
		SendDlgItemMessage(m_hwnd, IDC_PROTOCOL, STM_SETICON, LPARAM(Skin_LoadProtoIcon(szProto, ID_STATUS_ONLINE)), 0);

		wchar_t *contactName = Clist_GetContactDisplayName(dat->hContact);
		SetDlgItemText(m_hwnd, IDC_FROM, contactName);

		wchar_t szPath[450];
		GetContactReceivedFilesDir(dat->hContact, szPath, _countof(szPath), TRUE);
		SetDlgItemText(m_hwnd, IDC_FILEDIR, szPath);
		SHAutoComplete(GetWindow(GetDlgItem(m_hwnd, IDC_FILEDIR), GW_CHILD), 1);

		for (int i = 0; i < MAX_MRU_DIRS; i++) {
			char idstr[32];
			mir_snprintf(idstr, "MruDir%d", i);

			DBVARIANT dbv;
			if (g_plugin.getWString(idstr, &dbv))
				break;
			SendDlgItemMessage(m_hwnd, IDC_FILEDIR, CB_ADDSTRING, 0, (LPARAM)dbv.pwszVal);
			db_free(&dbv);
		}

		db_event_markRead(dat->hContact, dat->hDbEvent);

		DB::EventInfo dbei(dat->hDbEvent);
		if (!dbei)
			return false;

		dat->fs = (HANDLE)m_lParam;

		DB::FILE_BLOB blob(dbei);
		SetDlgItemText(m_hwnd, IDC_FILENAMES, blob.getName());
		if (mir_wstrlen(blob.getDescr()))
			SetDlgItemText(m_hwnd, IDC_MSG, blob.getDescr());

		wchar_t datetimestr[64];
		TimeZone_PrintTimeStamp(NULL, dbei.timestamp, L"t d", datetimestr, _countof(datetimestr), 0);
		SetDlgItemText(m_hwnd, IDC_DATE, datetimestr);

		ptrW info(Contact::GetInfo(CNF_UNIQUEID, dat->hContact));
		SetDlgItemText(m_hwnd, IDC_NAME, (info) ? info : contactName);

		if (!Contact::OnList(dat->hContact)) {
			RECT rcBtn1, rcBtn2, rcDateCtrl;
			GetWindowRect(GetDlgItem(m_hwnd, IDC_ADD), &rcBtn1);
			GetWindowRect(GetDlgItem(m_hwnd, IDC_USERMENU), &rcBtn2);
			GetWindowRect(GetDlgItem(m_hwnd, IDC_DATE), &rcDateCtrl);
			SetWindowPos(GetDlgItem(m_hwnd, IDC_DATE), 0, 0, 0, rcDateCtrl.right - rcDateCtrl.left - (rcBtn2.left - rcBtn1.left), rcDateCtrl.bottom - rcDateCtrl.top, SWP_NOZORDER | SWP_NOMOVE);
		}
		else if (File::bAutoAccept) {
			//don't check auto-min here to fix BUG#647620
			PostMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), (LPARAM)GetDlgItem(m_hwnd, IDOK));
		}
		if (Contact::OnList(dat->hContact))
			btnAdd.Hide();
		return true;
	}

	bool OnApply() override
	{
		// most recently used directories
		wchar_t szRecvDir[MAX_PATH], szDefaultRecvDir[MAX_PATH];
		GetDlgItemText(m_hwnd, IDC_FILEDIR, szRecvDir, _countof(szRecvDir));
		RemoveInvalidPathChars(szRecvDir);
		GetContactReceivedFilesDir(NULL, szDefaultRecvDir, _countof(szDefaultRecvDir), TRUE);
		if (wcsnicmp(szRecvDir, szDefaultRecvDir, mir_wstrlen(szDefaultRecvDir))) {
			char idstr[32];
			int i;
			DBVARIANT dbv;
			for (i = MAX_MRU_DIRS - 2; i >= 0; i--) {
				mir_snprintf(idstr, "MruDir%d", i);
				if (g_plugin.getWString(idstr, &dbv)) continue;
				mir_snprintf(idstr, "MruDir%d", i + 1);
				g_plugin.setWString(idstr, dbv.pwszVal);
				db_free(&dbv);
			}
			g_plugin.setWString(idstr, szRecvDir);
		}

		EnableWindow(GetDlgItem(m_hwnd, IDC_FILENAMES), FALSE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_MSG), FALSE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_FILEDIR), FALSE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_FILEDIRBROWSE), FALSE);

		GetDlgItemText(m_hwnd, IDC_FILEDIR, dat->szSavePath, _countof(dat->szSavePath));
		GetDlgItemText(m_hwnd, IDC_FILE, dat->szFilenames, _countof(dat->szFilenames));
		GetDlgItemText(m_hwnd, IDC_MSG, dat->szMsg, _countof(dat->szMsg));
		FtMgr_AddTransfer(dat);
		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, 0);
		dat = nullptr;
		
		// check for auto-minimize here to fix BUG#647620
		if (File::bAutoAccept && File::bAutoMin) {
			ShowWindow(m_hwnd, SW_HIDE);
			ShowWindow(m_hwnd, SW_SHOWMINNOACTIVE);
		}
		return true;
	}

	void OnDestroy() override
	{
		Window_FreeIcon_IcoLib(m_hwnd);

		delete dat; dat = nullptr;
		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, 0);
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		switch (msg) {
		case WM_COMMAND:
			if (Clist_MenuProcessCommand(LOWORD(wParam), MPCF_CONTACTMENU, dat->hContact))
				return 1;
			break;

		case HM_RECVEVENT:
			ACKDATA *ack = (ACKDATA *)lParam;
			if (ack && dat) {
				if (ack->hProcess != dat->fs || ack->type != ACKTYPE_FILE || ack->hContact != dat->hContact)
					break;

				if (ack->result == ACKRESULT_DENIED || ack->result == ACKRESULT_FAILED) {
					EnableWindow(GetDlgItem(m_hwnd, IDOK), FALSE);
					EnableWindow(GetDlgItem(m_hwnd, IDC_FILEDIR), FALSE);
					EnableWindow(GetDlgItem(m_hwnd, IDC_FILEDIRBROWSE), FALSE);
					SetDlgItemText(m_hwnd, IDC_MSG, TranslateT("This file transfer has been canceled by the other side"));
					Skin_PlaySound("FileDenied");
					FlashWindow(m_hwnd, TRUE);
				}
				else if (ack->result != ACKRESULT_FILERESUME) {
					btnCancel.Click();
				}
			}
			break;
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

	void onClick_Browse(CCtrlButton *)
	{
		wchar_t szDirName[MAX_PATH], szExistingDirName[MAX_PATH];

		GetDlgItemText(m_hwnd, IDC_FILEDIR, szDirName, _countof(szDirName));
		GetLowestExistingDirName(szDirName, szExistingDirName, _countof(szExistingDirName));
		if (BrowseForFolder(m_hwnd, szExistingDirName))
			SetDlgItemText(m_hwnd, IDC_FILEDIR, szExistingDirName);
	}

	void onClick_Cancel(CCtrlButton *)
	{
		if (dat->fs) {
			ProtoChainSend(dat->hContact, PSS_FILEDENY, (WPARAM)dat->fs, (LPARAM)TranslateT("Canceled"));
			dat->fs = nullptr; /* the protocol will free the handle */
		}
	}

	void onClick_Add(CCtrlButton *)
	{
		Contact::Add(dat->hContact, m_hwnd);

		if (Contact::OnList(dat->hContact))
			ShowWindow(GetDlgItem(m_hwnd, IDC_ADD), SW_HIDE);
	}

	void onClick_UserMenu(CCtrlButton *pButton)
	{
		RECT rc;
		GetWindowRect(pButton->GetHwnd(), &rc);
		HMENU hMenu = Menu_BuildContactMenu(dat->hContact);
		TrackPopupMenu(hMenu, 0, rc.left, rc.bottom, 0, m_hwnd, NULL);
		DestroyMenu(hMenu);
	}

	void onClick_Details(CCtrlButton *)
	{
		CallService(MS_USERINFO_SHOWDIALOG, dat->hContact, 0);
	}

	void onClick_History(CCtrlButton *)
	{
		CallService(MS_HISTORY_SHOWCONTACTHISTORY, dat->hContact, 0);
	}
};

void LaunchRecvDialog(CLISTEVENT *cle)
{
	auto *pDlg = new CRecvFileDlg(cle);
	pDlg->Show();
}
