/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This file is part of Msg_Export a Miranda IM plugin
// Copyright (C)2002 Kennet Nielsen ( http://sourceforge.net/projects/msg-export/ )
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"

const int nUINColWitdh = 80;    // width in pixels of the UIN column in the List Ctrl
const int nProtoColWitdh = 100; // width in pixels of the UIN column in the List Ctrl

/////////////////////////////////////////////////////////////////////////////////////////
// Class           :  CLDBEvent
// Superclass      :  
// Project         :  Mes_export
// Designer        :  Kennet Nielsen
// Version         :  1.0.0
// Date            :  020422, 22 April 2002
//
//
// Description: This class is used to store one DB event dyring the export 
//              All history function
//
// Version History:
//   Ver:     Initials:    Date:     Text:
//   1.0.0    KN           020422    First edition

class CLDBEvent
{
	DWORD time;
public:
	MCONTACT hUser;
	MEVENT   hDbEvent;

	CLDBEvent(MCONTACT hU, MEVENT hDBE)
	{
		hUser = hU;
		hDbEvent = hDBE;

		DBEVENTINFO dbei = {};
		db_event_get(hDbEvent, &dbei);
		time = dbei.timestamp;
	}

	bool operator <(const CLDBEvent& rOther) const
	{	return time < rOther.time;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Member Function : CompareFunc
// Type            : Global
// Parameters      : lParam1    - ?
//                   lParam2    - ?
//                   lParamSort - ?
// Returns         : int CALLBACK
// Description     : Used to sort list view by Nick
//                   
// References      : -
// Remarks         : -
// Created         : 020422, 22 April 2002
// Developer       : KN   

int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	if (lParamSort == 1)
		return mir_wstrcmpi(Clist_GetContactDisplayName(lParam1), Clist_GetContactDisplayName(lParam2));

	if (lParamSort == 2)
		return mir_strcmp(GetContactProto((MCONTACT)lParam1), GetContactProto((MCONTACT)lParam2));

	if (lParamSort == 3) {
		DWORD dwUin1 = db_get_dw((MCONTACT)lParam1, GetContactProto((MCONTACT)lParam1), "UIN", 0);
		DWORD dwUin2 = db_get_dw((MCONTACT)lParam2, GetContactProto((MCONTACT)lParam2), "UIN", 0);
		if (dwUin1 == dwUin2)
			return 0;
		return (dwUin1 > dwUin2) ? -1 : 1;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Member Function : DialogProc
// Type            : Global
// Parameters      : hwndDlg   - handle to dialog box
//                   uMsg      - message
//                   wParam    - first message parameter
//                   parameter - second message parameter
// Returns         : INT_PTR CALLBACK
// Description     : Progress bar window function
//                   
// References      : -
// Remarks         : -
// Created         : 020422, 22 April 2002
// Developer       : KN   

INT_PTR CALLBACK __stdcall DialogProc(HWND hwndDlg, UINT uMsg, WPARAM, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		return TRUE;
	case WM_CLOSE:
		if (lParam > 0)
			DestroyWindow(hwndDlg);
		return TRUE;
	}
	return FALSE;
}

struct ExportDialogData {
	list<MCONTACT> contacts;
	HWND hDialog;
};

void __cdecl exportContactsMessages(ExportDialogData *data)
{
	HWND hDlg = data->hDialog;
	HWND hProg = GetDlgItem(hDlg, IDC_EXPORT_PROGRESS);
	HWND hStatus = GetDlgItem(hDlg, IDC_EXP_ALL_STATUS);

	SendMessage(hProg, PBM_SETRANGE, 0, MAKELPARAM(0, data->contacts.size() - 1));
	SetWindowText(hStatus, TranslateT("Reading database information (Phase 1 of 2)"));

	// map with list to stored all DB history before it is exported 
	map<wstring, list< CLDBEvent >, less<wstring> > AllEvents;
	{
		// reading from the database !!! 
		int nCur = 0;
		list< MCONTACT >::const_iterator iterator;
		for (iterator = data->contacts.begin(); iterator != data->contacts.end(); ++iterator) {
			MCONTACT hContact = (*iterator);

			// Check if we should ignore this contact/protocol
			if (!bIsExportEnabled(hContact))
				continue;

			list<CLDBEvent> &rclCurList = AllEvents[GetFilePathFromUser(hContact)];

			MEVENT hDbEvent = db_event_first(hContact);
			while (hDbEvent) {
				rclCurList.push_back(CLDBEvent(hContact, hDbEvent));
				hDbEvent = db_event_next(hContact, hDbEvent);
			}

			SendMessage(hProg, PBM_SETPOS, nCur, 0);
			RedrawWindow(hDlg, nullptr, nullptr, RDW_ALLCHILDREN | RDW_UPDATENOW);

			nCur++;
		}
		// Free the list of contacts
		data->contacts.clear();
		delete data;
	}

	// window text update 
	SetWindowText(hStatus, TranslateT("Sorting and writing database information (Phase 2 of 2)"));
	SendMessage(hProg, PBM_SETRANGE, 0, MAKELPARAM(0, AllEvents.size() - 1));
	SendMessage(hProg, PBM_SETPOS, 0, 0);

	// time to write to files !!!
	map<wstring, list< CLDBEvent >, less<wstring> >::iterator FileIterator;

	int nCur = 0;
	for (FileIterator = AllEvents.begin(); FileIterator != AllEvents.end(); ++FileIterator) {
		(FileIterator->second).sort(); // Sort is preformed here !!
		// events with same time will not be swaped, they will 
		// remain in there original order

		// Open/create file for writing
		wstring sFilePath = FileIterator->first;
		HANDLE hFile = openCreateFile(sFilePath);
		if (hFile == INVALID_HANDLE_VALUE) {
			DisplayErrorDialog(LPGENW("Failed to open or create file :\n"), sFilePath, nullptr);
			continue;
		}

		// At first write we need to have this false (to write file header, etc.), for each next write to same file use true
		bool bAppendOnly = false;

		list< CLDBEvent >::const_iterator iterator;
		for (iterator = FileIterator->second.begin(); iterator != FileIterator->second.end(); ++iterator) {
			MEVENT hDbEvent = (*iterator).hDbEvent;
			MCONTACT hContact = (*iterator).hUser;
			if (!bExportEvent(hContact, hDbEvent, hFile, sFilePath, bAppendOnly))
				break; // serious error, we should close the file and don't continue with it

			// Set this flag, because we're appending to same file now
			bAppendOnly = true;
		}

		// Close the file
		CloseHandle(hFile);

		SendMessage(hProg, PBM_SETPOS, ++nCur, 0);
		RedrawWindow(hDlg, nullptr, nullptr, RDW_ALLCHILDREN | RDW_UPDATENOW);
	}

	SendMessage(hDlg, WM_CLOSE, 0, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////
// CBasicOptDlg : basic options dialog

class CBasicOptDlg : public CDlgBase
{
	CCtrlButton btnExportAll, btnExportSel, btnUserDetails, btnAutoFileName, btnClearAll, btnSetDefault;
	CCtrlButton btnBrowseDir, btnBrowseFile;
	CCtrlListView listUsers;
	CCtrlHyperlink urlHelp;

	bool bUnapplyedChanges = false;

public:
	CBasicOptDlg() :
		CDlgBase(g_plugin, IDD_OPT_MSGEXPORT),
		urlHelp(this, IDC_OPEN_HELP, "https://miranda-ng.org/p/Msg_Export/"),
		listUsers(this, IDC_MAP_USER_LIST),
		btnClearAll(this, IDC_CLEAR_ALL),
		btnExportAll(this, IDC_EXPORTALL),
		btnExportSel(this, ID_EXPORTSELECTED),
		btnBrowseDir(this, IDC_EXPORT_DIR_BROWSE),
		btnBrowseFile(this, IDC_FILE_VIEWER_BROWSE),
		btnSetDefault(this, ID_SET_TO_DEFAULT),
		btnUserDetails(this, ID_USERLIST_USERDETAILS),
		btnAutoFileName(this, IDC_AUTO_FILENAME)
	{
		listUsers.OnKeyDown = Callback(this, &CBasicOptDlg::list_KeyDown);
		listUsers.OnBuildMenu = Callback(this, &CBasicOptDlg::list_OnMenu);
		listUsers.OnColumnClick = Callback(this, &CBasicOptDlg::list_ColumnClick);
		listUsers.OnDoubleClick = Callback(this, &CBasicOptDlg::list_DoubleClick);
		listUsers.OnEndLabelEdit = Callback(this, &CBasicOptDlg::list_LabelEdit);

		btnClearAll.OnClick = Callback(this, &CBasicOptDlg::onClick_ClearAll);
		btnExportAll.OnClick = btnExportSel.OnClick = Callback(this, &CBasicOptDlg::onClick_Export);
		btnBrowseDir.OnClick = Callback(this, &CBasicOptDlg::onClick_BrowseDir);
		btnBrowseFile.OnClick = Callback(this, &CBasicOptDlg::onClick_BrowseFile);
		btnSetDefault.OnClick = Callback(this, &CBasicOptDlg::onClick_SetDefault);
		btnUserDetails.OnClick = Callback(this, &CBasicOptDlg::onClick_Details);
		btnAutoFileName.OnClick = Callback(this, &CBasicOptDlg::onClick_AutoFileName);
	}

	bool OnInitDialog() override
	{
		DWORD dw = listUsers.GetExtendedListViewStyle();
		dw |= LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES;
		listUsers.SetExtendedListViewStyle(dw);

		int nColumnWidth = 100;
		RECT rListSize;
		if (GetWindowRect(listUsers.GetHwnd(), &rListSize)) {
			nColumnWidth = (rListSize.right - rListSize.left - GetSystemMetrics(SM_CXVSCROLL) - 5 - nUINColWitdh - nProtoColWitdh) / 2;
			if (nColumnWidth < 10)
				nColumnWidth = 10;
		}

		// header setup 
		LVCOLUMN cCol = { 0 };
		cCol.mask = LVCF_TEXT | LVCF_WIDTH;
		cCol.cx = nColumnWidth;
		cCol.pszText = TranslateT("File");
		listUsers.InsertColumn(0, &cCol);

		cCol.pszText = TranslateT("Nick");
		listUsers.InsertColumn(1, &cCol);

		cCol.cx = nProtoColWitdh;
		cCol.pszText = TranslateT("Proto");
		listUsers.InsertColumn(2, &cCol);

		cCol.cx = nUINColWitdh;
		cCol.mask |= LVCF_FMT;
		cCol.fmt = LVCFMT_RIGHT;
		cCol.pszText = TranslateT("UIN");
		listUsers.InsertColumn(3, &cCol);

		int nUser = 0;
		wstring sTmp;
		LVITEM sItem = { 0 };
		for (auto &hContact : Contacts()) {
			PROTOACCOUNT *pa = Proto_GetAccount(GetContactProto(hContact));
			if (pa == nullptr)
				continue;

			sItem.mask = LVIF_TEXT | LVIF_PARAM;
			sItem.iItem = nUser++;
			sItem.iSubItem = 0;
			sItem.lParam = hContact;

			sTmp = _DBGetStringW(hContact, MODULENAME, "FileName", L"");
			sItem.pszText = (wchar_t*)sTmp.c_str();
			listUsers.InsertItem(&sItem);

			sItem.mask = LVIF_TEXT;
			sItem.iSubItem = 1;
			sItem.pszText = Clist_GetContactDisplayName(hContact);
			listUsers.SetItem(&sItem);

			sItem.iSubItem = 2;
			sItem.pszText = pa->tszAccountName;
			listUsers.SetItem(&sItem);

			DWORD dwUIN = db_get_dw(hContact, pa->szModuleName, "UIN", 0);
			wchar_t szTmp[50];
			mir_snwprintf(szTmp, L"%d", dwUIN);
			sItem.iSubItem = 3;
			sItem.pszText = szTmp;
			listUsers.SetItem(&sItem);

			listUsers.SetCheckState(sItem.iItem, db_get_b(hContact, MODULENAME, "EnableLog", 1));
		}
		listUsers.SortItems(CompareFunc, 1);

		sItem.mask = LVIF_STATE;
		sItem.iItem = 0;
		sItem.iSubItem = 0;
		sItem.state = LVIS_FOCUSED;
		sItem.stateMask = LVIS_FOCUSED;
		listUsers.SetItem(&sItem);

		SetDlgItemInt(m_hwnd, IDC_MAX_CLOUMN_WIDTH, nMaxLineWidth, TRUE);

		// Export dir
		SetDlgItemText(m_hwnd, IDC_EXPORT_DIR, sExportDir.c_str());
		HWND hComboBox = GetDlgItem(m_hwnd, IDC_EXPORT_DIR);
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"%dbpath%\\MsgExport\\");
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"C:\\Backup\\MsgExport\\");
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"%dbpath%\\MsgExport\\%group% - ");
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"%dbpath%\\MsgExport\\%group%\\");

		// default file
		SetDlgItemText(m_hwnd, IDC_DEFAULT_FILE, sDefaultFile.c_str());
		hComboBox = GetDlgItem(m_hwnd, IDC_DEFAULT_FILE);
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"%nick%.txt");
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"%UIN%.txt");
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"%group%.txt");
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"%e-mail%.txt");
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"%identifier%.txt");
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"%year%-%month%-%day%.txt");
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"%group%\\%nick%.txt");
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"%group%\\%UIN%.txt");
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"%group%\\%identifier%.txt");
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"%protocol%\\%nick%.txt");
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"History.txt");

		// time format
		SetDlgItemText(m_hwnd, IDC_EXPORT_TIMEFORMAT, sTimeFormat.c_str());
		hComboBox = GetDlgItem(m_hwnd, IDC_EXPORT_TIMEFORMAT);
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"d t");
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"d s");
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"d m");
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"D s");
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"D m :");

		// File viewer
		SetDlgItemText(m_hwnd, IDC_FILE_VIEWER, sFileViewerPrg.c_str());
		hComboBox = GetDlgItem(m_hwnd, IDC_FILE_VIEWER);
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"");
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"C:\\Windows\\Notepad.exe");
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"C:\\WinNT\\Notepad.exe");
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)L"C:\\Program Files\\Notepad++\\notepad++.exe");

		CheckDlgButton(m_hwnd, IDC_USE_INTERNAL_VIEWER, bUseInternalViewer() ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_REPLACE_MIRANDA_HISTORY, bReplaceHistory ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_APPEND_NEWLINE, bAppendNewLine ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_USE_UTF8_IN_NEW_FILES, bUseUtf8InNewFiles ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_USE_LESS_AND_GREATER_IN_EXPORT, bUseLessAndGreaterInExport ? BST_CHECKED : BST_UNCHECKED);
		return true;
	}

	bool OnApply() override
	{
		BOOL bTrans;
		wchar_t szTemp[500];

		int nTmp = GetDlgItemInt(m_hwnd, IDC_MAX_CLOUMN_WIDTH, &bTrans, TRUE);
		if (!bTrans || (nTmp > 0 && nTmp < 5)) {
			mir_snwprintf(szTemp, TranslateT("Max line width must be at least %d. Or use 0 for no limit."), 5);
			MessageBox(m_hwnd, szTemp, MSG_BOX_TITEL, MB_OK);
			return false;
		}
		else nMaxLineWidth = nTmp;

		GetDlgItemText(m_hwnd, IDC_EXPORT_TIMEFORMAT, szTemp, _countof(szTemp));
		sTimeFormat = szTemp;

		GetDlgItemText(m_hwnd, IDC_EXPORT_DIR, szTemp, _countof(szTemp));
		sExportDir = szTemp;

		GetDlgItemText(m_hwnd, IDC_DEFAULT_FILE, szTemp, _countof(szTemp));
		sDefaultFile = szTemp;

		GetDlgItemText(m_hwnd, IDC_FILE_VIEWER, szTemp, _countof(szTemp));
		sFileViewerPrg = szTemp;

		bUseInternalViewer(IsDlgButtonChecked(m_hwnd, IDC_USE_INTERNAL_VIEWER) == BST_CHECKED);

		bool bNewRp = IsDlgButtonChecked(m_hwnd, IDC_REPLACE_MIRANDA_HISTORY) == BST_CHECKED;
		if (bReplaceHistory != bNewRp) {
			bReplaceHistory = bNewRp;
			MessageBox(m_hwnd, TranslateT("You need to restart Miranda to change the history function"), MSG_BOX_TITEL, MB_OK);
		}

		bAppendNewLine = IsDlgButtonChecked(m_hwnd, IDC_APPEND_NEWLINE) == BST_CHECKED;
		bUseUtf8InNewFiles = IsDlgButtonChecked(m_hwnd, IDC_USE_UTF8_IN_NEW_FILES) == BST_CHECKED;
		bUseLessAndGreaterInExport = IsDlgButtonChecked(m_hwnd, IDC_USE_LESS_AND_GREATER_IN_EXPORT) == BST_CHECKED;

		int nCount = listUsers.GetItemCount();
		for (int nCur = 0; nCur < nCount; nCur++) {
			LVITEM sItem = { 0 };
			sItem.iItem = nCur;
			sItem.mask = LVIF_TEXT | LVIF_PARAM;
			sItem.pszText = szTemp;
			sItem.cchTextMax = _countof(szTemp);

			if (listUsers.GetItem(&sItem)) {
				MCONTACT hUser = (MCONTACT)sItem.lParam;
				if (mir_wstrlen(szTemp) > 0)
					db_set_ws(hUser, MODULENAME, "FileName", szTemp);
				else
					db_unset(hUser, MODULENAME, "FileName");

				if (listUsers.GetCheckState(nCur))
					db_unset(hUser, MODULENAME, "EnableLog");
				else
					db_set_b(hUser, MODULENAME, "EnableLog", 0);
			}
		}
		UpdateFileToColWidth();

		SaveSettings();

		bUnapplyedChanges = false;
		return true;
	}

	void OnChange() override
	{
		bUnapplyedChanges = true;
	}

	void onClick_Export(CCtrlButton *pButton)
	{
		if (bUnapplyedChanges) {
			DWORD res = MessageBox(m_hwnd, TranslateT("You have not applied the changes, do you wish to apply them first?"), MSG_BOX_TITEL, MB_YESNOCANCEL);
			if (res == IDCANCEL)
				return;

			if (res == IDYES)
				if (!OnApply())
					return;
		}

		bool bOnlySelected = pButton->GetCtrlId() != IDC_EXPORTALL;
		int nTotalContacts = listUsers.GetItemCount();

		int nContacts;
		if (bOnlySelected)
			nContacts = listUsers.GetSelectedCount();
		else
			nContacts = nTotalContacts;

		if (nContacts <= 0) {
			MessageBox(m_hwnd, TranslateT("No contacts found to export"), MSG_BOX_TITEL, MB_OK);
			return;
		}

		ExportDialogData *data = new ExportDialogData();

		// List all contacts to export
		LVITEM sItem = { 0 };
		sItem.mask = LVIF_PARAM;

		for (int nCur = 0; nCur < nTotalContacts; nCur++) {
			if (bOnlySelected)
				if (!(listUsers.GetItemState(nCur, LVIS_SELECTED) & LVIS_SELECTED))
					continue;

			sItem.iItem = nCur;
			if (!listUsers.GetItem(&sItem)) {
				MessageBox(m_hwnd, TranslateT("Failed to export at least one contact"), MSG_BOX_TITEL, MB_OK);
				continue;
			}

			MCONTACT hContact = (MCONTACT)sItem.lParam;
			data->contacts.push_back(hContact);
		}

		// Create progress dialog
		HWND hDlg = data->hDialog = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_EXPORT_ALL_DLG), nullptr, DialogProc);
		ShowWindow(hDlg, SW_SHOWNORMAL);

		// Process the export in other thread
		mir_forkThread<ExportDialogData>(&exportContactsMessages, data);
	}

	void onClick_Details(CCtrlButton*)
	{
		LVITEM sItem;
		sItem.mask = LVIF_PARAM;
		sItem.iItem = listUsers.GetNextItem(-1, LVIS_SELECTED);
		if (sItem.iItem >= 0 && listUsers.GetItem(&sItem))
			CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)sItem.lParam, 0);
	}

	void onClick_AutoFileName(CCtrlButton*)
	{
		wchar_t szDefaultFile[500];
		GetDlgItemText(m_hwnd, IDC_DEFAULT_FILE, szDefaultFile, _countof(szDefaultFile));

		LVITEM sItem = { 0 };

		int nCount = listUsers.GetItemCount();
		for (int nCur = 0; nCur < nCount; nCur++) {
			wchar_t szSearch[100];
			sItem.pszText = szSearch;
			sItem.mask = LVIF_TEXT;
			sItem.iItem = nCur;
			sItem.iSubItem = 1;
			sItem.cchTextMax = _countof(szSearch);
			if (!listUsers.GetItem(&sItem))
				continue;

			int nShortestMatch = 0xFFFF;
			MCONTACT hStortest = 0;
			int nStortestIndex = -1;
			for (int nSubCur = 0; nSubCur < nCount; nSubCur++) {
				if (nSubCur == nCur)
					continue;
				wchar_t szSubCur[100];
				sItem.mask = LVIF_TEXT | LVIF_PARAM;
				sItem.iItem = nSubCur;
				sItem.iSubItem = 1;
				sItem.pszText = szSubCur;
				sItem.cchTextMax = _countof(szSubCur);
				if (listUsers.GetItem(&sItem)) {
					size_t nLen = mir_wstrlen(szSubCur);
					if (wcsnicmp(szSubCur, szSearch, nLen) == 0) {
						if (nLen < (size_t)nShortestMatch) {
							nShortestMatch = (int)nLen;
							nStortestIndex = nSubCur;
							hStortest = (MCONTACT)sItem.lParam;
						}
					}
				}
			}

			if (nShortestMatch != 0xFFFF) {
				wstring sFileName;
				szSearch[0] = 0;
				listUsers.GetItemText(nCur, 0, szSearch, _countof(szSearch));
				bool bPriHasFileName = szSearch[0] != 0;
				if (bPriHasFileName)
					sFileName = szSearch;

				szSearch[0] = 0;
				listUsers.GetItemText(nStortestIndex, 0, szSearch, _countof(szSearch));
				bool bSubHasFileName = szSearch[0] != 0;
				if (bSubHasFileName)
					sFileName = szSearch;

				if (sFileName.empty()) {
					sFileName = szDefaultFile;
					ReplaceDefines(hStortest, sFileName);
					ReplaceTimeVariables(sFileName);
				}

				if (!bPriHasFileName)
					listUsers.SetItemText(nCur, 0, (wchar_t*)sFileName.c_str());

				if (!bSubHasFileName)
					listUsers.SetItemText(nStortestIndex, 0, (wchar_t*)sFileName.c_str());

				NotifyChange();
			}
		}
	}

	void onClick_ClearAll(CCtrlButton*)
	{
		LVITEM sItem = { 0 };
		sItem.mask = LVIF_TEXT;
		sItem.pszText = L"";

		int nCount = listUsers.GetItemCount();
		for (int nCur = 0; nCur < nCount; nCur++) {
			sItem.iItem = nCur;
			listUsers.SetItem(&sItem);
		}

		NotifyChange();
	}

	void onClick_SetDefault(CCtrlButton*)
	{
		int nContacts = listUsers.GetItemCount();
		if (nContacts <= 0)
			return;

		wchar_t szTemp[500];
		if (!GetDlgItemText(m_hwnd, IDC_DEFAULT_FILE, szTemp, _countof(szTemp)))
			return;

		LVITEM sItem = { 0 };

		for (int nCur = 0; nCur < nContacts; nCur++) {
			if (!(listUsers.GetItemState(nCur, LVIS_SELECTED) & LVIS_SELECTED))
				continue;

			sItem.iItem = nCur;
			sItem.mask = LVIF_PARAM;
			if (!listUsers.GetItem(&sItem))
				continue;

			wstring sFileName = szTemp;
			ReplaceDefines((MCONTACT)sItem.lParam, sFileName);
			ReplaceTimeVariables(sFileName);

			sItem.mask = LVIF_TEXT;
			sItem.pszText = (wchar_t*)sFileName.c_str();
			listUsers.SetItem(&sItem);

			NotifyChange();
		}
	}

	void onClick_BrowseFile(CCtrlButton*)
	{
		wchar_t szFile[260];       // buffer for file name
		GetDlgItemText(m_hwnd, IDC_FILE_VIEWER, szFile, _countof(szFile));

		wchar_t buf[MAX_PATH];
		mir_snwprintf(buf, L"%s (*.exe;*.com;*.bat;*.cmd)%c*.exe;*.com;*.bat;*.cmd%c%s (*.*)%c*.*%c%c", TranslateT("Executable files"), 0, 0, TranslateT("All files"), 0, 0, 0);
		{
			OPENFILENAME ofn = {};       // common dialog box structure
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = m_hwnd;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = _countof(szFile);
			ofn.lpstrFilter = buf;
			ofn.nFilterIndex = 1;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

			// Display the Open dialog box. 
			if (GetOpenFileName(&ofn)) {
				SetDlgItemText(m_hwnd, IDC_FILE_VIEWER, szFile);
				NotifyChange();
			}
		}
	}

	void onClick_BrowseDir(CCtrlButton*)
	{
		// Get the shells allocator
		LPMALLOC pMalloc;
		if (FAILED(SHGetMalloc(&pMalloc))) { // we need to use this to support old Windows versions 
			MessageBox(m_hwnd, TranslateT("Failed to get the shells allocator!"), MSG_BOX_TITEL, MB_OK);
			return;
		}

		// Allocate the Dest Dir buffer to receive browse info
		wchar_t *lpDestDir = (wchar_t *)pMalloc->Alloc(MAX_PATH + 100);
		if (!lpDestDir) {
			pMalloc->Release();
			MessageBox(m_hwnd, TranslateT("Failed to Allocate buffer space"), MSG_BOX_TITEL, MB_OK);
			return;
		}

		BROWSEINFO sBrowseInfo;
		sBrowseInfo.hwndOwner = m_hwnd;
		sBrowseInfo.pidlRoot = nullptr;
		sBrowseInfo.pszDisplayName = lpDestDir;
		sBrowseInfo.lpszTitle = TranslateT("Select Destination Directory");
		sBrowseInfo.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
		sBrowseInfo.lpfn = nullptr;
		sBrowseInfo.lParam = 0;
		sBrowseInfo.iImage = 0;

		LPITEMIDLIST psItemIDList = SHBrowseForFolder(&sBrowseInfo);
		if (psItemIDList) {
			SHGetPathFromIDList(psItemIDList, lpDestDir);
			size_t n = mir_wstrlen(lpDestDir);
			if (n > 0 && lpDestDir[n] != '\\') {
				lpDestDir[n] = '\\';
				lpDestDir[n + 1] = 0;
			}
			SetDlgItemText(m_hwnd, IDC_EXPORT_DIR, lpDestDir);
			NotifyChange();

			// Clean up
			pMalloc->Free(psItemIDList);
		}
		pMalloc->Free(lpDestDir);
		pMalloc->Release();
	}

	void list_OnMenu(CCtrlBase*)
	{
		HMENU hMainMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_MSG_EXPORT));
		if (hMainMenu) {
			HMENU hMenu = GetSubMenu(hMainMenu, 0);

			POINT pt;
			GetCursorPos(&pt);

			TranslateMenu(hMenu);
			TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hwnd, nullptr);

			DestroyMenu(hMainMenu);
		}
	}

	void list_LabelEdit(CCtrlListView::TEventInfo *evt)
	{
		NMLVDISPINFO *pdi = evt->nmlvdi;
		if (pdi->item.mask & LVIF_TEXT) {
			pdi->item.mask &= LVIF_TEXT;
			listUsers.SetItem(&pdi->item);
			NotifyChange();
		}
	}

	void list_KeyDown(CCtrlListView::TEventInfo *evt)
	{
		NMLVKEYDOWN *lpnmk = evt->nmlvkey;
		if (lpnmk->wVKey == 'A' && (GetKeyState(VK_CONTROL) & 0x8000)) {
			// select all 
			int nCount = listUsers.GetItemCount();
			for (int nCur = 0; nCur < nCount; nCur++)
				listUsers.SetItemState(nCur, LVIS_SELECTED, LVIS_SELECTED);
			return;
		}

		if (lpnmk->wVKey == VK_F2 || (lpnmk->wVKey >= 'A' &&  lpnmk->wVKey <= 'Z') || (lpnmk->wVKey >= '1' &&  lpnmk->wVKey <= '9') || lpnmk->wVKey == VK_BACK) {
			HWND hEdit = listUsers.EditLabel(listUsers.GetSelectionMark());
			if (hEdit && lpnmk->wVKey != VK_F2) {
				if (isupper(lpnmk->wVKey))
					SendMessage(hEdit, WM_CHAR, tolower(lpnmk->wVKey), 0);
				else
					SendMessage(hEdit, WM_CHAR, lpnmk->wVKey, 0);
			}
		}
	}

	void list_ColumnClick(CCtrlListView::TEventInfo *evt)
	{
		listUsers.SortItems(CompareFunc, evt->nmlv->iSubItem);
	}		

	void list_DoubleClick(CCtrlListView::TEventInfo *evt)
	{
		NMITEMACTIVATE *pdi = evt->nmlvia;
		if (pdi->iItem >= 0)
			listUsers.EditLabel(pdi->iItem);
	}

	void list_CustomDraw(CCtrlListView::TEventInfo *evt)
	{
		LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)evt->nmlv;
		switch (lplvcd->nmcd.dwDrawStage) {
		case CDDS_PREPAINT:
			SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW);
			break;

		case CDDS_ITEMPREPAINT:
			if (lplvcd->nmcd.lItemlParam == 0)
				lplvcd->clrText = RGB(0, 0, 255);

			SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, CDRF_NEWFONT);
			break;
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// CAdvancedOptDlg - class of advanced options

class CAdvancedOptDlg : public CDlgBase
{
	CCtrlButton btnDebug;	
	CCtrlListView listProtos;
	CCtrlCheck chkFcPrompt, chkFcRename, chkFcNothing;
	CCtrlCheck chkFdPrompt, chkFdDelete, chkFdNothing;

public:
	CAdvancedOptDlg() :
		CDlgBase(g_plugin, IDD_OPT_MSGEXPORT2),
		btnDebug(this, IDC_DEBUG_INFO),
		chkFcPrompt(this, IDC_FC_PROMPT),
		chkFcRename(this, IDC_FC_RENAME),
		chkFcNothing(this, IDC_FC_NOTHING),
		chkFdPrompt(this, IDC_FD_PROMPT),
		chkFdDelete(this, IDC_FD_DELETE),
		chkFdNothing(this, IDC_FD_NOTHING),
		listProtos(this, IDC_EXPORT_PROTOS)
	{
	}

	bool OnInitDialog() override
	{
		switch (enRenameAction) {
		case eDAPromptUser:
			chkFcPrompt.SetState(true);
			break;
		case eDAAutomatic:
			chkFcRename.SetState(true);
			break;
		case eDANothing:
			chkFcNothing.SetState(true);
			break;
		}
		
		switch (enDeleteAction) {
		case eDAPromptUser:
			chkFdPrompt.SetState(true);
			break;
		case eDAAutomatic:
			chkFdDelete.SetState(true);
			break;
		case eDANothing:
			chkFdNothing.SetState(true);
			break;
		}

		int nColumnWidth = 100;

		RECT rListSize;
		if (GetWindowRect(listProtos.GetHwnd(), &rListSize)) {
			nColumnWidth = (rListSize.right - rListSize.left - GetSystemMetrics(SM_CXVSCROLL) - 5);
			if (nColumnWidth < 10)
				nColumnWidth = 10;
		}

		listProtos.SetExtendedListViewStyle(LVS_EX_CHECKBOXES);

		// header setup !!
		LVCOLUMN cCol = { 0 };
		cCol.mask = LVCF_TEXT | LVCF_WIDTH;
		cCol.cx = nColumnWidth;
		cCol.pszText = TranslateT("Export Protocols");
		listProtos.InsertColumn(0, &cCol);

		LVITEMW sItem = { 0 };
		sItem.mask = LVIF_TEXT | LVIF_PARAM;
		char szTemp[500];

		for (auto &pa : Accounts()) {
			mir_snprintf(szTemp, "DisableProt_%s", pa->szModuleName);
			sItem.pszText = pa->tszAccountName;
			sItem.lParam = (LPARAM)pa->szModuleName;
			listProtos.InsertItem(&sItem);

			listProtos.SetCheckState(sItem.iItem, db_get_b(NULL, MODULENAME, szTemp, 1));

			sItem.iItem++;
		}
		return true;
	}

	bool OnApply() override
	{
		if (chkFcPrompt.GetState())
			enRenameAction = eDAPromptUser;
		else if (chkFcRename.GetState() == BST_CHECKED)
			enRenameAction = eDAAutomatic;
		else if (chkFcNothing.GetState() == BST_CHECKED)
			enRenameAction = eDANothing;

		if (chkFdPrompt.GetState() == BST_CHECKED)
			enDeleteAction = eDAPromptUser;
		else if (chkFdDelete.GetState() == BST_CHECKED)
			enDeleteAction = eDAAutomatic;
		else if (chkFdNothing.GetState() == BST_CHECKED)
			enDeleteAction = eDANothing;

		int nCount = listProtos.GetItemCount();
		for (int nCur = 0; nCur < nCount; nCur++) {
			LVITEM sItem = { 0 };
			sItem.iItem = nCur;
			sItem.mask = LVIF_PARAM;
			if (listProtos.GetItem(&sItem)) {
				char szTemp[200];
				mir_snprintf(szTemp, "DisableProt_%s", (char*)sItem.lParam);
				if (listProtos.GetCheckState(nCur))
					db_unset(NULL, MODULENAME, szTemp); // default is Enabled !!
				else
					db_set_b(NULL, MODULENAME, szTemp, 0);
			}
		}
		SaveSettings();
		return true;
	}

	void onClick_Debug(CCtrlButton*)
	{
		wstring sDebug = L"Debug information\r\nsDBPath :";
		sDebug += sDBPath;
		sDebug += L"\r\nsMirandaPath :";
		sDebug += sMirandaPath;
		sDebug += L"\r\nsDefaultFile :";
		sDebug += sDefaultFile;

		sDebug += L"\r\nGetFilePathFromUser(NULL) :";
		sDebug += GetFilePathFromUser(NULL);

		MessageBox(nullptr, sDebug.c_str(), MSG_BOX_TITEL, MB_OK);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Member Function : OptionsInitialize
// Type            : Global
// Parameters      : wParam - ?
//                   lParam - ?
// Returns         : int
// Description     : Called when the user openes the options dialog
//                   I need to add my options page.
// References      : -
// Remarks         : -
// Created         : 020422, 22 April 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////////////////////////

int OptionsInitialize(WPARAM wParam, LPARAM /*lParam*/)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 100000000;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.szTitle.w = LPGENW("Message export");
	odp.szGroup.w = LPGENW("History");

	odp.szTab.w = LPGENW("General");
	odp.pDialog = new CBasicOptDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.position = 100000001;
	odp.szTab.w = LPGENW("Additional");
	odp.pDialog = new CAdvancedOptDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
