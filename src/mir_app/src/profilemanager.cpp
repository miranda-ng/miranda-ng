/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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
#include "plugins.h"
#include "langpack.h"
#include "profilemanager.h"
#include <sys/stat.h>

#pragma warning(disable : 4512)

#define WM_INPUTCHANGED (WM_USER + 0x3000)
#define WM_FOCUSTEXTBOX (WM_USER + 0x3001)

typedef BOOL (__cdecl *ENUMPROFILECALLBACK)(wchar_t *tszFullPath, wchar_t *profile, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////////////////
// Profile creator

static int findProfiles(wchar_t *szProfileDir, ENUMPROFILECALLBACK callback, LPARAM lParam)
{
	// find in Miranda NG profile subfolders
	wchar_t searchspec[MAX_PATH];
	mir_snwprintf(searchspec, L"%s\\*.*", szProfileDir);
	
	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(searchspec, &ffd);
	if (hFind == INVALID_HANDLE_VALUE)
		return 0;

	do {
		// find all subfolders except "." and ".."
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && mir_wstrcmp(ffd.cFileName, L".") && mir_wstrcmp(ffd.cFileName, L"..")) {
			wchar_t buf[MAX_PATH], profile[MAX_PATH];
			mir_snwprintf(buf, L"%s\\%s\\%s.dat", szProfileDir, ffd.cFileName, ffd.cFileName);
			if (_waccess(buf, 0) == 0) {
				mir_snwprintf(profile, L"%s.dat", ffd.cFileName);
				if (!callback(buf, profile, lParam))
					break;
			}
		}
	}
		while (FindNextFile(hFind, &ffd));

	FindClose(hFind);
	return 1;
}

static LRESULT CALLBACK ProfileNameValidate(HWND edit, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_CHAR) {
		if (wcschr(L".?/\\#' ", (wchar_t)wParam) != nullptr)
			return 0;
		PostMessage(GetParent(edit), WM_INPUTCHANGED, 0, 0);
	}
	return mir_callNextSubclass(edit, ProfileNameValidate, msg, wParam, lParam);
}

class CCreateProfileDlg : public CDlgBase
{
	CCtrlButton &m_btnOk;
	PROFILEMANAGERDATA *m_pd;

	int CreateProfile(const wchar_t *profile, DATABASELINK *link)
	{
		// check if the file already exists
		const wchar_t *file = wcsrchr(profile, '\\');
		if (file)
			file++;

		int err = 0;
		wchar_t buf[256];

		if (_waccess(profile, 0) == 0) {
			// file already exists!
			mir_snwprintf(buf,
				TranslateT("The profile '%s' already exists. Do you want to move it to the Recycle Bin?\n\nWARNING: The profile will be deleted if Recycle Bin is disabled.\nWARNING: A profile may contain confidential information and should be properly deleted."),
				file);
			if (MessageBox(m_hwnd, buf, TranslateT("The profile already exists"), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) != IDYES)
				return 0;

			// move the file
			if (DeleteDirectoryTreeW(profile, true) != 0) {
				mir_snwprintf(buf, TranslateT("Couldn't move '%s' to the Recycle Bin. Please select another profile name."), file);
				MessageBox(m_hwnd, buf, TranslateT("Problem moving profile"), MB_ICONINFORMATION | MB_OK);
				return 0;
			}
			// now the file should be gone!
		}
		// ask the database to create the profile
		CreatePathToFileW(profile);
		if ((err = link->makeDatabase(profile)) != ERROR_SUCCESS) {
			mir_snwprintf(buf, TranslateT("Unable to create the profile '%s', the error was %x"), file, err);
			MessageBox(m_hwnd, buf, TranslateT("Problem creating profile"), MB_ICONERROR | MB_OK);
			return 0;
		}

		// the profile has been created!
		g_bDbCreated = true;
		return 1;
	}

	bool       m_bFocused;
	CCtrlCombo m_driverList;
	CCtrlEdit  m_profileName;
	CCtrlBase  m_warning;

public:
	CCreateProfileDlg(CCtrlButton &_btn, PROFILEMANAGERDATA *_pd) :
		CDlgBase(g_plugin, IDD_PROFILE_NEW),
		m_btnOk(_btn),
		m_pd(_pd),
		m_bFocused(false),
		m_driverList(this, IDC_PROFILEDRIVERS),
		m_profileName(this, IDC_PROFILENAME),
		m_warning(this, IDC_NODBDRIVERS)
	{}

	bool OnInitDialog() override
	{
		// what, no plugins?!
		if (arDbPlugins.getCount() == 0) {
			m_driverList.Enable(false);
			m_profileName.Enable(false);
			ShowWindow(m_warning.GetHwnd(), TRUE);
		}
		else {
			for (auto &p : arDbPlugins)
				if (p->capabilities & MDB_CAPS_CREATE)
					m_driverList.AddString(TranslateW(p->szFullName), (LPARAM)p);
		}

		// default item
		m_driverList.SetCurSel(0);

		// subclass the profile name box
		mir_subclassWindow(m_profileName.GetHwnd(), ProfileNameValidate);

		// decide if there is a default profile name given in the INI and if it should be used
		if (m_pd->noProfiles || (shouldAutoCreate(m_pd->ptszProfile) && _waccess(m_pd->ptszProfile, 0))) {
			wchar_t *profile = wcsrchr(m_pd->ptszProfile, '\\');
			if (profile) ++profile;
			else profile = m_pd->ptszProfile;

			wchar_t *p = wcsrchr(profile, '.');
			wchar_t c = 0;
			if (p) { c = *p; *p = 0; }

			m_profileName.SetText(profile);
			if (c) *p = c;
		}

		// focus on the textbox
		PostMessage(m_hwnd, WM_FOCUSTEXTBOX, 0, 0);
		return true;
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		switch (msg) {
		case WM_FOCUSTEXTBOX:
			SetFocus(m_profileName.GetHwnd());
			break;

		case WM_INPUTCHANGED: // when input in the edit box changes
			NotifyChange();
			m_btnOk.Enable(GetWindowTextLength(m_profileName.GetHwnd()) > 0);
			break;

		case WM_SHOWWINDOW:
			if (wParam) {
				m_btnOk.SetText(TranslateT("&Create"));
				SendMessage(m_hwnd, WM_INPUTCHANGED, 0, 0);
				m_bFocused = true;
			}
			else m_bFocused = false;
			break;
		}
		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

	bool OnApply() override
	{
		LRESULT curSel = m_driverList.GetCurSel();
		if (curSel == -1 || !m_bFocused)
			return false; // should never happen

		ptrW szName(m_profileName.GetText());
		if (mir_wstrlen(szName) == 0)
			return false;

		// profile placed in "profile_name" subfolder
		mir_snwprintf(m_pd->ptszProfile, MAX_PATH, L"%s\\%s\\%s.dat", m_pd->ptszProfileDir, szName.get(), szName.get());
		m_pd->newProfile = 1;
		m_pd->dblink = (DATABASELINK *)m_driverList.GetItemData(curSel);

		if (CreateProfile(m_pd->ptszProfile, m_pd->dblink) == 0)
			SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
		else
			m_pd->bRun = true;
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Profile selector

static int numMessages[5];

static void stubAddMessage(int iType, const wchar_t *, ...)
{
	if (iType < 5)
		numMessages[iType]++;
}

class CChooseProfileDlg : public CDlgBase
{
	CCtrlButton &m_btnOk;
	PROFILEMANAGERDATA *m_pd;
	HANDLE m_hFileNotify;

	struct ProfileEnumData
	{
		ProfileEnumData(CCtrlListView &_list, wchar_t *_profile) :
			list(_list),
			szProfile(_profile)
		{}

		CCtrlListView &list;
		wchar_t* szProfile;
	};

	static BOOL EnumProfilesForList(wchar_t *tszFullPath, wchar_t *profile, LPARAM lParam)
	{
		ProfileEnumData *ped = (ProfileEnumData*)lParam;
		CCtrlListView &list = ped->list;

		wchar_t sizeBuf[64];
		bool bFileLocked;

		wchar_t *p = wcsrchr(profile, '.');
		mir_wstrcpy(sizeBuf, L"0 KB");
		if (p != nullptr) *p = 0;

		LVITEM item = { 0 };
		item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		item.pszText = profile;
		item.iItem = 0;

		struct _stat statbuf;
		if (_wstat(tszFullPath, &statbuf) == 0) {
			if (statbuf.st_size > 1000000) {
				mir_snwprintf(sizeBuf, L"%.3lf", (double)statbuf.st_size / 1048576.0);
				mir_wstrcpy(sizeBuf + 5, L" MB");
			}
			else {
				mir_snwprintf(sizeBuf, L"%.3lf", (double)statbuf.st_size / 1024.0);
				mir_wstrcpy(sizeBuf + 5, L" KB");
			}
			bFileLocked = Profile_CheckOpened(tszFullPath);
		}
		else bFileLocked = true;

		DATABASELINK *dblink;
		switch (touchDatabase(tszFullPath, &dblink)) {
		case ERROR_SUCCESS:
			item.iImage = (bFileLocked) ? 1 : 0;
			break;

		case EGROKPRF_OBSOLETE:
			item.iImage = 2;
			break;

		case EGROKPRF_CANTREAD:
			item.iImage = (bFileLocked) ? 1 : 3;
			break;

		default:
			item.iImage = 3;
		}

		item.lParam = (LPARAM)dblink;

		int iItem = list.InsertItem(&item);
		if (mir_wstrcmpi(ped->szProfile, tszFullPath) == 0)
			list.SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

		list.SetItemText(iItem, 2, sizeBuf);

		if (dblink != nullptr)
			list.SetItemText(iItem, 1, TranslateW(dblink->szFullName));
		else if (bFileLocked) // file locked
			list.SetItemText(iItem, 1, TranslateT("<In use>"));
		else
			list.SetItemText(iItem, 1, TranslateT("<Unknown format>"));

		return TRUE;
	}

	void DeleteProfile(const LVITEM &item)
	{
		CMStringW wszMessage(FORMAT, TranslateT("Are you sure you want to remove profile \"%s\"?"), item.pszText);
		if (IDYES != MessageBoxW(nullptr, wszMessage, L"Miranda NG", MB_YESNO | MB_TASKMODAL | MB_ICONWARNING))
			return;

		wszMessage.Format(L"%s\\%s", m_pd->ptszProfileDir, item.pszText);
		DeleteDirectoryTreeW(wszMessage, true);
		
		m_profileList.DeleteItem(item.iItem);
	}

	void CheckProfile(const wchar_t *profile)
	{
		CMStringW wszFullName(FORMAT, L"%s\\%s\\%s.dat", m_pd->ptszProfileDir, profile, profile);

		if (TryLoadPlugin(plugin_checker, false))
			CallService(MS_DB_CHECKPROFILE, (WPARAM)wszFullName.c_str(), 0);
		else
			Plugin_Uninit(plugin_checker);
	}

	void CompactProfile(DATABASELINK *dblink, const wchar_t *profile)
	{
		CMStringW wszFullName(FORMAT, L"%s\\%s\\%s.dat", m_pd->ptszProfileDir, profile, profile);

		if (auto *db = dblink->Load(wszFullName, false)) {
			db->Compact();
			delete db;
			
			MessageBoxW(nullptr, TranslateT("Database was compacted successfully"), L"Miranda NG", MB_OK | MB_ICONINFORMATION);
		}
	}

	void CheckRun()
	{
		m_btnOk.Enable(m_profileList.GetSelectedCount() == 1);

		wchar_t profile[MAX_PATH];
		LVITEM item = { 0 };
		item.mask = LVIF_TEXT | LVIF_IMAGE;
		item.iItem = m_profileList.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
		item.pszText = profile;
		item.cchTextMax = _countof(profile);
		if (!m_profileList.GetItem(&item))
			return;

		switch(item.iImage) {
		case 3:
			m_btnOk.Enable(false);
			return;

		case 2:
			m_btnOk.SetText(TranslateT("&Convert"));
			m_pd->bRun = false;
			break;

		default:
			m_btnOk.SetText(TranslateT("&Run"));
			m_pd->bRun = true;
		}

		// profile is placed in "profile_name" subfolder

		wchar_t tmpPath[MAX_PATH];
		mir_snwprintf(tmpPath, L"%s\\%s.dat", m_pd->ptszProfileDir, profile);
		if (_waccess(tmpPath, 2))
			mir_snwprintf(m_pd->ptszProfile, MAX_PATH, L"%s\\%s\\%s.dat", m_pd->ptszProfileDir, profile, profile);
		else
			wcsncpy_s(m_pd->ptszProfile, MAX_PATH, tmpPath, _TRUNCATE);
	}

	void ExecuteMenu(LPARAM lParam)
	{
		LVHITTESTINFO lvht = { 0 };
		lvht.pt.x = GET_X_LPARAM(lParam);
		lvht.pt.y = GET_Y_LPARAM(lParam);
		ScreenToClient(m_profileList.GetHwnd(), &lvht.pt);

		if (m_profileList.HitTest(&lvht) == -1)
			return;

		if (lvht.iItem == -1)
			return;

		wchar_t profile[MAX_PATH];
		LVITEM item = { 0 };
		item.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT;
		item.iItem = lvht.iItem;
		item.pszText = profile;
		item.cchTextMax = _countof(profile);
		if (!m_profileList.GetItem(&item))
			return;

		lvht.pt.x = GET_X_LPARAM(lParam);
		lvht.pt.y = GET_Y_LPARAM(lParam);

		HMENU hMenu = CreatePopupMenu();
		if (item.iImage < 2) {
			AppendMenu(hMenu, MF_STRING, 1, TranslateT("Run"));
			AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
		}

		DATABASELINK *dblink = (DATABASELINK*)item.lParam;
		if (dblink != nullptr) {
			bool bAdded = false;
			if (dblink->capabilities & MDB_CAPS_COMPACT) {
				AppendMenu(hMenu, MF_STRING, 3, TranslateT("Compact"));
				bAdded = true;
			}

			if (plugin_checker && (dblink->capabilities & MDB_CAPS_CHECK)) {
				AppendMenu(hMenu, MF_STRING, 4, TranslateT("Check database"));
				bAdded = true;
			}

			if (bAdded)
				AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
		}

		AppendMenu(hMenu, MF_STRING, 2, TranslateT("Delete"));
		int index = TrackPopupMenu(hMenu, TPM_RETURNCMD, lvht.pt.x, lvht.pt.y, 0, m_hwnd, nullptr);
		switch (index) {
		case 1:
			SendMessage(GetParent(m_hwndParent), WM_COMMAND, IDOK, 0);
			break;

		case 2:
			DeleteProfile(item);
			break;

		case 3:
			CompactProfile(dblink, profile);
			break;

		case 4:
			CheckProfile(profile);
			break;
		}
		DestroyMenu(hMenu);
	}

	CCtrlListView m_profileList;

public:
	CChooseProfileDlg(CCtrlButton &_btn, PROFILEMANAGERDATA *_pd) :
		CDlgBase(g_plugin, IDD_PROFILE_SELECTION),
		m_btnOk(_btn),
		m_pd(_pd),
		m_profileList(this, IDC_PROFILELIST)
	{
		m_profileList.OnItemChanged = Callback(this, &CChooseProfileDlg::list_OnItemChanged);
		m_profileList.OnKeyDown = Callback(this, &CChooseProfileDlg::list_OnKeyDown);
		m_profileList.OnGetInfoTip = Callback(this, &CChooseProfileDlg::list_OnGetTip);
		m_profileList.OnDoubleClick = Callback(this, &CChooseProfileDlg::list_OnDblClick);
	}

	bool OnInitDialog() override
	{
		// set columns
		LVCOLUMN col;
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.pszText = TranslateT("Profile");
		col.cx = 100;
		m_profileList.InsertColumn(0, &col);

		col.pszText = TranslateT("Driver");
		col.cx = 150 - GetSystemMetrics(SM_CXVSCROLL);
		m_profileList.InsertColumn(1, &col);

		col.pszText = TranslateT("Size");
		col.cx = 60;
		m_profileList.InsertColumn(2, &col);

		// icons
		HIMAGELIST hImgList = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 2, 1);
		ImageList_AddIcon_NotShared(hImgList, MAKEINTRESOURCE(IDI_USERDETAILS));
		ImageList_AddIcon_NotShared(hImgList, MAKEINTRESOURCE(IDI_DELETE));
		ImageList_AddIcon_NotShared(hImgList, MAKEINTRESOURCE(IDI_MWARNING));
		ImageList_AddIcon_NotShared(hImgList, MAKEINTRESOURCE(IDI_MFATAL));

		// LV will destroy the image list
		m_profileList.SetImageList(hImgList, LVSIL_SMALL);
		m_profileList.SetExtendedListViewStyle(m_profileList.GetExtendedListViewStyle() | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP | LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT);

		// find all the profiles
		ProfileEnumData ped(m_profileList, m_pd->ptszProfile);
		findProfiles(m_pd->ptszProfileDir, EnumProfilesForList, (LPARAM)&ped);
		PostMessage(m_hwnd, WM_FOCUSTEXTBOX, 0, 0);

		m_hFileNotify = FindFirstChangeNotification(m_pd->ptszProfileDir, TRUE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE);
		if (m_hFileNotify != INVALID_HANDLE_VALUE)
			SetTimer(m_hwnd, 0, 1200, nullptr);
		return true;
	}

	void OnDestroy()
	{
		KillTimer(m_hwnd, 0);
		FindCloseChangeNotification(m_hFileNotify);
	}

	void list_OnItemChanged(CCtrlListView::TEventInfo*)
	{
		CheckRun();
	}

	void list_OnKeyDown(CCtrlListView::TEventInfo *evt)
	{
		if (evt->nmlvkey->wVKey == VK_DELETE) {
			wchar_t profile[MAX_PATH];
			LVITEM item = { 0 };
			item.mask = LVIF_TEXT;
			item.iItem = m_profileList.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
			item.pszText = profile;
			item.cchTextMax = _countof(profile);
			if (m_profileList.GetItem(&item))
				DeleteProfile(item);
		}
	}

	void list_OnGetTip(CCtrlListView::TEventInfo *evt)
	{
		if (auto pTip = evt->nmlvit) {
			wchar_t profilename[MAX_PATH], tszFullPath[MAX_PATH];
			struct _stat statbuf;
			m_profileList.GetItemText(pTip->iItem, 0, profilename, _countof(profilename));
			mir_snwprintf(tszFullPath, L"%s\\%s\\%s.dat", m_pd->ptszProfileDir, profilename, profilename);
			_wstat(tszFullPath, &statbuf);
			mir_snwprintf(pTip->pszText, pTip->cchTextMax, L"%s\n%s: %s\n%s: %s", tszFullPath, TranslateT("Created"), rtrimw(NEWWSTR_ALLOCA(_wctime(&statbuf.st_ctime))), TranslateT("Modified"), rtrimw(NEWWSTR_ALLOCA(_wctime(&statbuf.st_mtime))));
		}
	}

	void list_OnDblClick(CCtrlListView::TEventInfo*)
	{
		CheckRun();
		EndDialog(GetParent(m_hwndParent), 1);
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		switch (msg) {
		case WM_TIMER:
			if (WaitForSingleObject(m_hFileNotify, 0) == WAIT_OBJECT_0) {
				m_profileList.DeleteAllItems();
				ProfileEnumData ped(m_profileList, m_pd->ptszProfile);
				findProfiles(m_pd->ptszProfileDir, EnumProfilesForList, (LPARAM)&ped);
				FindNextChangeNotification(m_hFileNotify);
			}
			break;

		case WM_FOCUSTEXTBOX:
			SetFocus(m_profileList.GetHwnd());
			if (m_pd->ptszProfile[0] == 0 || m_profileList.GetSelectedCount() == 0)
				m_profileList.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			break;

		case WM_SHOWWINDOW:
			if (wParam)
				CheckRun();
			break;

		case WM_CONTEXTMENU:
			ExecuteMenu(lParam);
			break;
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Tab manager + its envelope

class CProfileManager : public CDlgBase
{
	PROFILEMANAGERDATA *m_pd;

	CCtrlPages  m_tab;
	CCtrlButton m_btnOk;
	CCtrlCheck  m_chkSmEnabled;
	CCtrlCombo  m_servicePlugs;
	CCtrlBase   m_warning;

public:
	CProfileManager(PROFILEMANAGERDATA *_pd) :
		CDlgBase(g_plugin, IDD_PROFILEMANAGER),
		m_btnOk(this, IDOK),
		m_pd(_pd),
		m_tab(this, IDC_TABS),
		m_warning(this, IDC_SM_LABEL),
		m_servicePlugs(this, IDC_SM_COMBO),
		m_chkSmEnabled(this, IDC_SM_ENABLED)
	{
		m_chkSmEnabled.OnChange = Callback(this, &CProfileManager::onChanged);

		m_tab.AddPage(LPGENW("My profiles"), nullptr, new CChooseProfileDlg(m_btnOk, m_pd));
		m_tab.AddPage(LPGENW("New profile"), nullptr, new CCreateProfileDlg(m_btnOk, m_pd));
	}
	
	bool OnInitDialog() override
	{
		// MUST NOT be replaced with Window_SetIcon_IcoLib!!!
		SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadImage(g_plugin.getInst(), MAKEINTRESOURCE(IDI_DETAILSLOGO), IMAGE_ICON, g_iIconSX, g_iIconSY, 0));
		SendMessage(m_hwnd, WM_SETICON, ICON_BIG, (LPARAM)LoadImage(g_plugin.getInst(), MAKEINTRESOURCE(IDI_DETAILSLOGO), IMAGE_ICON, g_iIconX, g_iIconY, 0));

		if (m_pd->noProfiles || shouldAutoCreate(m_pd->ptszProfile))
			m_tab.ActivatePage(1);

		// service mode combobox
		if (servicePlugins.getCount() == 0) {
			ShowWindow(m_warning.GetHwnd(), FALSE);
			ShowWindow(m_chkSmEnabled.GetHwnd(), FALSE);
			ShowWindow(m_servicePlugs.GetHwnd(), FALSE);
		}
		else {
			for (int i = 0; i < servicePlugins.getCount(); i++) {
				pluginEntry *p = servicePlugins[i];
				m_servicePlugs.AddStringA(p->pluginname, i);
			}

			m_servicePlugs.Disable();
			m_servicePlugs.SetCurSel(0);
		}
		return true;
	}

	void OnDestroy()
	{
		if (m_chkSmEnabled.GetState()) {
			int idx = m_servicePlugs.GetCurData();
			if (idx != -1)
				plugin_service = servicePlugins[idx];
		}

		DestroyIcon((HICON)SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, 0));
		DestroyIcon((HICON)SendMessage(m_hwnd, WM_SETICON, ICON_BIG, 0));
	}

	void onChanged(CCtrlCheck*)
	{
		m_servicePlugs.Enable(m_chkSmEnabled.GetState());
	}
};

int getProfileManager(PROFILEMANAGERDATA *pd)
{
	return CProfileManager(pd).DoModal();
}
