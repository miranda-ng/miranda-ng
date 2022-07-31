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
	uint32_t time;
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
		return mir_strcmp(Proto_GetBaseAccountName((MCONTACT)lParam1), Proto_GetBaseAccountName((MCONTACT)lParam2));

	if (lParamSort == 3) {
		uint32_t dwUin1 = db_get_dw((MCONTACT)lParam1, Proto_GetBaseAccountName((MCONTACT)lParam1), "UIN", 0);
		uint32_t dwUin2 = db_get_dw((MCONTACT)lParam2, Proto_GetBaseAccountName((MCONTACT)lParam2), "UIN", 0);
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

void __cdecl exportContactsMessages(struct ExportDialogData *data)
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
		for (auto &hContact : data->contacts) {
			// Check if we should ignore this contact/protocol
			if (!bIsExportEnabled(hContact))
				continue;

			list<CLDBEvent> &rclCurList = AllEvents[GetFilePathFromUser(hContact)];

			DB::ECPTR pCursor(DB::Events(hContact));
			while (MEVENT hDbEvent = pCursor.FetchNext())
				rclCurList.push_back(CLDBEvent(hContact, hDbEvent));

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
	int nCur = 0;
	for (auto &F : AllEvents) {
		F.second.sort(); // Sort is preformed here !!
		// events with same time will not be swaped, they will 
		// remain in there original order

		// Open/create file for writing
		wstring sFilePath = F.first;
		HANDLE hFile = openCreateFile(sFilePath);
		if (hFile == INVALID_HANDLE_VALUE) {
			DisplayErrorDialog(LPGENW("Failed to open or create file:\n"), sFilePath, nullptr);
			continue;
		}

		// At first write we need to have this false (to write file header, etc.), for each next write to same file use true
		bool bAppendOnly = false;

		for (auto &E : F.second) {
			MEVENT hDbEvent = E.hDbEvent;
			MCONTACT hContact = E.hUser;
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
	CCtrlButton btnBrowseDir, btnBrowseFile;
	CCtrlCombo cmbExportDir, cmbDefaultFile, cmbTimeFormat, cmbFileViewer;

public:
	CBasicOptDlg() :
		CDlgBase(g_plugin, IDD_OPT_MSGEXPORT),
		btnBrowseDir(this, IDC_EXPORT_DIR_BROWSE),
		btnBrowseFile(this, IDC_FILE_VIEWER_BROWSE),
		cmbExportDir(this, IDC_EXPORT_DIR),
		cmbTimeFormat(this, IDC_EXPORT_TIMEFORMAT),
		cmbFileViewer(this, IDC_FILE_VIEWER),
		cmbDefaultFile(this, IDC_DEFAULT_FILE)
	{
		btnBrowseDir.OnClick = Callback(this, &CBasicOptDlg::onClick_BrowseDir);
		btnBrowseFile.OnClick = Callback(this, &CBasicOptDlg::onClick_BrowseFile);
	}

	bool OnInitDialog() override
	{
		SetDlgItemInt(m_hwnd, IDC_MAX_CLOUMN_WIDTH, nMaxLineWidth, TRUE);

		// Export dir
		cmbExportDir.SetText(g_sExportDir.c_str());
		
		cmbExportDir.AddString(L"%dbpath%\\MsgExport\\");
		cmbExportDir.AddString(L"C:\\Backup\\MsgExport\\");
		cmbExportDir.AddString(L"%dbpath%\\MsgExport\\%group% - ");
		cmbExportDir.AddString(L"%dbpath%\\MsgExport\\%group%\\");

		// default file
		cmbDefaultFile.SetText(g_sDefaultFile.c_str());
		
		cmbDefaultFile.AddString(L"%nick%.txt");
		cmbDefaultFile.AddString(L"%UIN%.txt");
		cmbDefaultFile.AddString(L"%group%.txt");
		cmbDefaultFile.AddString(L"%e-mail%.txt");
		cmbDefaultFile.AddString(L"%id%.txt");
		cmbDefaultFile.AddString(L"%year%-%month%-%day%.txt");
		cmbDefaultFile.AddString(L"%group%\\%nick%.txt");
		cmbDefaultFile.AddString(L"%group%\\%UIN%.txt");
		cmbDefaultFile.AddString(L"%group%\\%id%.txt");
		cmbDefaultFile.AddString(L"%protocol%\\%nick%.txt");
		cmbDefaultFile.AddString(L"History.txt");

		// time format
		cmbTimeFormat.SetText(g_sTimeFormat.c_str());
		
		cmbTimeFormat.AddString(L"d t");
		cmbTimeFormat.AddString(L"d s");
		cmbTimeFormat.AddString(L"d m");
		cmbTimeFormat.AddString(L"D s");
		cmbTimeFormat.AddString(L"D m :");

		// File viewer
		cmbFileViewer.SetText(sFileViewerPrg.c_str());

		cmbFileViewer.AddString(L"");
		cmbFileViewer.AddString(L"C:\\Windows\\Notepad.exe");
		cmbFileViewer.AddString(L"C:\\WinNT\\Notepad.exe");
		cmbFileViewer.AddString(L"C:\\Program Files\\Notepad++\\notepad++.exe");

		CheckDlgButton(m_hwnd, IDC_USE_JSON, g_bUseJson ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_USE_INTERNAL_VIEWER, g_bUseIntViewer ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_REPLACE_MIRANDA_HISTORY, g_bReplaceHistory ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_APPEND_NEWLINE, g_bAppendNewLine ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_USE_UTF8_IN_NEW_FILES, g_bUseUtf8InNewFiles ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_USE_LESS_AND_GREATER_IN_EXPORT, g_bUseLessAndGreaterInExport ? BST_CHECKED : BST_UNCHECKED);
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
		g_plugin.setWord("MaxLineWidth", (uint16_t)nMaxLineWidth);

		cmbTimeFormat.GetText(szTemp, _countof(szTemp));
		g_sTimeFormat = szTemp;
		g_plugin.setWString("TimeFormat", g_sTimeFormat.c_str());

		cmbExportDir.GetText(szTemp, _countof(szTemp));
		g_sExportDir = szTemp;
		g_plugin.setWString("ExportDir", g_sExportDir.c_str());

		cmbDefaultFile.GetText(szTemp, _countof(szTemp));
		g_sDefaultFile = szTemp;
		g_plugin.setWString("DefaultFile", g_sDefaultFile.c_str());

		cmbFileViewer.GetText(szTemp, _countof(szTemp));
		sFileViewerPrg = szTemp;
		g_plugin.setWString("FileViewerPrg", sFileViewerPrg.c_str());

		bUseInternalViewer(IsDlgButtonChecked(m_hwnd, IDC_USE_INTERNAL_VIEWER) == BST_CHECKED);
		g_plugin.setByte("UseInternalViewer", g_bUseIntViewer);

		bool bNewRp = IsDlgButtonChecked(m_hwnd, IDC_REPLACE_MIRANDA_HISTORY) == BST_CHECKED;
		if (g_bReplaceHistory != bNewRp) {
			g_bReplaceHistory = bNewRp;
			MessageBox(m_hwnd, TranslateT("You need to restart Miranda to change the history function"), MSG_BOX_TITEL, MB_OK);
		}
		g_plugin.setByte("ReplaceHistory", g_bReplaceHistory);

		g_bUseJson = IsDlgButtonChecked(m_hwnd, IDC_USE_JSON) == BST_CHECKED;
		g_plugin.setByte("UseJson", g_bUseJson);

		g_bAppendNewLine = IsDlgButtonChecked(m_hwnd, IDC_APPEND_NEWLINE) == BST_CHECKED;
		g_plugin.setByte("AppendNewLine", g_bAppendNewLine);

		g_bUseUtf8InNewFiles = IsDlgButtonChecked(m_hwnd, IDC_USE_UTF8_IN_NEW_FILES) == BST_CHECKED;
		g_plugin.setByte("UseUtf8InNewFiles", g_bUseUtf8InNewFiles);

		g_bUseLessAndGreaterInExport = IsDlgButtonChecked(m_hwnd, IDC_USE_LESS_AND_GREATER_IN_EXPORT) == BST_CHECKED;
		g_plugin.setByte("UseLessAndGreaterInExport", g_bUseLessAndGreaterInExport);
		return true;
	}

	void onClick_BrowseFile(CCtrlButton*)
	{
		wchar_t szFile[260];       // buffer for file name
		cmbFileViewer.GetText(szFile, _countof(szFile));

		wchar_t buf[MAX_PATH];
		mir_snwprintf(buf, L"%s (*.exe;*.com;*.bat;*.cmd)%c*.exe;*.com;*.bat;*.cmd%c%s (*.*)%c*.*%c%c", TranslateT("Executable files"), 0, 0, TranslateT("All files"), 0, 0, 0);

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

	void onClick_BrowseDir(CCtrlButton*)
	{
		// Get the shells allocator
		LPMALLOC pMalloc;
		if (FAILED(SHGetMalloc(&pMalloc)))
			return;

		// Allocate the Dest Dir buffer to receive browse info
		wchar_t *lpDestDir = (wchar_t *)pMalloc->Alloc(MAX_PATH + 100);
		if (!lpDestDir) {
			pMalloc->Release();
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
			cmbExportDir.SetText(lpDestDir);
			NotifyChange();

			// Clean up
			pMalloc->Free(psItemIDList);
		}
		pMalloc->Free(lpDestDir);
		pMalloc->Release();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// CContactsOptDlg - class of advanced options

class CContactsOptDlg : public CDlgBase
{
	CCtrlListView listUsers;
	CCtrlHyperlink urlHelp;

public:
	CContactsOptDlg() :
		CDlgBase(g_plugin, IDD_OPT_CONTACTS),
		urlHelp(this, IDC_OPEN_HELP, "https://miranda-ng.org/p/Msg_Export/"),
		listUsers(this, IDC_MAP_USER_LIST)
	{
		listUsers.OnKeyDown = Callback(this, &CContactsOptDlg::list_KeyDown);
		listUsers.OnBuildMenu = Callback(this, &CContactsOptDlg::list_OnMenu);
		listUsers.OnColumnClick = Callback(this, &CContactsOptDlg::list_ColumnClick);
		listUsers.OnDoubleClick = Callback(this, &CContactsOptDlg::list_DoubleClick);
		listUsers.OnEndLabelEdit = Callback(this, &CContactsOptDlg::list_LabelEdit);
	}

	bool OnInitDialog() override
	{
		uint32_t dw = listUsers.GetExtendedListViewStyle();
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
		cCol.pszText = TranslateT("Account");
		listUsers.InsertColumn(2, &cCol);

		cCol.cx = nUINColWitdh;
		cCol.mask |= LVCF_FMT;
		cCol.fmt = LVCFMT_RIGHT;
		cCol.pszText = TranslateT("User ID");
		listUsers.InsertColumn(3, &cCol);

		int nUser = 0;
		wstring sTmp;
		LVITEM sItem = { 0 };
		for (auto &hContact : Contacts()) {
			PROTOACCOUNT *pa = Proto_GetAccount(Proto_GetBaseAccountName(hContact));
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

			ptrW wszUin(Contact::GetInfo(CNF_UNIQUEID, hContact, pa->szModuleName));
			if (wszUin) {
				sItem.iSubItem = 3;
				sItem.pszText = wszUin;
				listUsers.SetItem(&sItem);
			}

			listUsers.SetCheckState(sItem.iItem, g_plugin.getByte(hContact, "EnableLog", 1));
		}
		listUsers.SortItems(CompareFunc, 1);

		Select(true);

		sItem.mask = LVIF_STATE;
		sItem.iItem = 0;
		sItem.iSubItem = 0;
		sItem.state = LVIS_FOCUSED;
		sItem.stateMask = LVIS_FOCUSED;
		listUsers.SetItem(&sItem);
		return true;
	}

	bool OnApply() override
	{
		int nCount = listUsers.GetItemCount();
		wchar_t szTemp[500];

		for (int nCur = 0; nCur < nCount; nCur++) {
			LVITEM sItem = { 0 };
			sItem.iItem = nCur;
			sItem.mask = LVIF_TEXT | LVIF_PARAM;
			sItem.pszText = szTemp;
			sItem.cchTextMax = _countof(szTemp);

			if (listUsers.GetItem(&sItem)) {
				MCONTACT hUser = (MCONTACT)sItem.lParam;
				if (mir_wstrlen(szTemp) > 0)
					g_plugin.setWString(hUser, "FileName", szTemp);
				else
					g_plugin.delSetting(hUser, "FileName");

				if (listUsers.GetCheckState(nCur))
					g_plugin.delSetting(hUser, "EnableLog");
				else
					g_plugin.setByte(hUser, "EnableLog", 0);
			}
		}
		UpdateFileToColWidth();

		return true;
	}

	void Export()
	{
		int nContacts = listUsers.GetSelectedCount();
		if (nContacts <= 0) {
			MessageBox(m_hwnd, TranslateT("No contacts found to export"), MSG_BOX_TITEL, MB_OK);
			return;
		}

		ExportDialogData *data = new ExportDialogData();

		// List all contacts to export
		LVITEM sItem = { 0 };
		sItem.mask = LVIF_PARAM;

		int nTotalContacts = listUsers.GetItemCount();
		for (int nCur = 0; nCur < nTotalContacts; nCur++) {
			if (!(listUsers.GetItemState(nCur, LVIS_SELECTED) & LVIS_SELECTED))
				continue;

			sItem.iItem = nCur;
			if (!listUsers.GetItem(&sItem))
				continue;

			MCONTACT hContact = (MCONTACT)sItem.lParam;
			data->contacts.push_back(hContact);
		}

		data->Run();
	}

	void Details()
	{
		LVITEM sItem;
		sItem.mask = LVIF_PARAM;
		sItem.iItem = listUsers.GetNextItem(-1, LVIS_SELECTED);
		if (sItem.iItem >= 0 && listUsers.GetItem(&sItem))
			CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)sItem.lParam, 0);
	}

	void AutoFileName()
	{
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
					sFileName = g_sDefaultFile;
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

	void ClearAll()
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

	void Select(bool bCheck)
	{
		int nContacts = listUsers.GetItemCount();
		if (nContacts <= 0)
			return;

		for (int nCur = 0; nCur < nContacts; nCur++)
			listUsers.SetItemState(nCur, bCheck ? LVIS_SELECTED : 0, LVIS_SELECTED);
	}

	void SetDefault()
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

	void list_OnMenu(CCtrlBase*)
	{
		HMENU hMainMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_MSG_EXPORT));
		if (hMainMenu) {
			HMENU hMenu = GetSubMenu(hMainMenu, 0);

			POINT pt;
			GetCursorPos(&pt);

			TranslateMenu(hMenu);
			int iRet = TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, nullptr);
			DestroyMenu(hMainMenu);

			switch (iRet) {
			case ID_EXPORT:               Export(); break;
			case ID_USERLIST_USERDETAILS: Details(); break;
			case ID_CLEAR_ALL:            ClearAll(); break;
			case ID_SELECT_ALL:           Select(true); break;
			case ID_SELECT_NONE:          Select(false); break;
			case ID_SET_TO_DEFAULT:       SetDefault(); break;
			case ID_AUTO_FILENAME:        AutoFileName(); break;
			}
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
		btnDebug.OnClick = Callback(this, &CAdvancedOptDlg::onClick_Debug);
	}

	bool OnInitDialog() override
	{
		switch (g_enRenameAction) {
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
		
		switch (g_enDeleteAction) {
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
		cCol.pszText = TranslateT("Export accounts");
		listProtos.InsertColumn(0, &cCol);

		LVITEMW sItem = { 0 };
		sItem.mask = LVIF_TEXT | LVIF_PARAM;
		char szTemp[500];

		for (auto &pa : Accounts()) {
			mir_snprintf(szTemp, "DisableProt_%s", pa->szModuleName);
			sItem.pszText = pa->tszAccountName;
			sItem.lParam = (LPARAM)pa->szModuleName;
			listProtos.InsertItem(&sItem);

			listProtos.SetCheckState(sItem.iItem, g_plugin.getByte(szTemp, 1));

			sItem.iItem++;
		}
		return true;
	}

	bool OnApply() override
	{
		if (chkFcPrompt.GetState())
			g_enRenameAction = eDAPromptUser;
		else if (chkFcRename.GetState() == BST_CHECKED)
			g_enRenameAction = eDAAutomatic;
		else if (chkFcNothing.GetState() == BST_CHECKED)
			g_enRenameAction = eDANothing;
		g_plugin.setByte("RenameAction", (uint8_t)g_enRenameAction);

		if (chkFdPrompt.GetState() == BST_CHECKED)
			g_enDeleteAction = eDAPromptUser;
		else if (chkFdDelete.GetState() == BST_CHECKED)
			g_enDeleteAction = eDAAutomatic;
		else if (chkFdNothing.GetState() == BST_CHECKED)
			g_enDeleteAction = eDANothing;
		g_plugin.setByte("DeleteAction", (uint8_t)g_enDeleteAction);

		int nCount = listProtos.GetItemCount();
		for (int nCur = 0; nCur < nCount; nCur++) {
			LVITEM sItem = { 0 };
			sItem.iItem = nCur;
			sItem.mask = LVIF_PARAM;
			if (listProtos.GetItem(&sItem)) {
				char szTemp[200];
				mir_snprintf(szTemp, "DisableProt_%s", (char*)sItem.lParam);
				if (listProtos.GetCheckState(nCur))
					g_plugin.delSetting(szTemp); // default is Enabled !!
				else
					g_plugin.setByte(szTemp, 0);
			}
		}
		return true;
	}

	void onClick_Debug(CCtrlButton*)
	{
		wstring sDebug = L"Debug information\r\nsDBPath :";
		sDebug += g_sDBPath;
		sDebug += L"\r\nsMirandaPath :";
		sDebug += g_sMirandaPath;
		sDebug += L"\r\nsDefaultFile :";
		sDebug += g_sDefaultFile;

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
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.szTitle.w = LPGENW("Message export");
	odp.szGroup.w = LPGENW("History");

	odp.position = 100000000;
	odp.szTab.w = LPGENW("General");
	odp.pDialog = new CBasicOptDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.position = 100000001;
	odp.szTab.w = LPGENW("Contacts");
	odp.pDialog = new CContactsOptDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.position = 100000002;
	odp.szTab.w = LPGENW("Additional");
	odp.pDialog = new CAdvancedOptDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
