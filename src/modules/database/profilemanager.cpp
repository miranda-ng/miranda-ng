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

#include "..\..\core\commonheaders.h"
#include "..\plugins\plugins.h"
#include "..\langpack\langpack.h"
#include "profilemanager.h"
#include <sys/stat.h>

void EnsureCheckerLoaded(bool);

#define WM_INPUTCHANGED (WM_USER + 0x3000)
#define WM_FOCUSTEXTBOX (WM_USER + 0x3001)

typedef BOOL (__cdecl *ENUMPROFILECALLBACK) (TCHAR *tszFullPath, TCHAR *profile, LPARAM lParam);

void SetServiceModePlugin(pluginEntry *p);

/////////////////////////////////////////////////////////////////////////////////////////
// Profile creator

static int findProfiles(TCHAR *szProfileDir, ENUMPROFILECALLBACK callback, LPARAM lParam)
{
	// find in Miranda NG profile subfolders
	TCHAR searchspec[MAX_PATH];
	mir_sntprintf(searchspec, SIZEOF(searchspec), _T("%s\\*.*"), szProfileDir);
	
	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(searchspec, &ffd);
	if (hFind == INVALID_HANDLE_VALUE)
		return 0;

	do {
		// find all subfolders except "." and ".."
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && mir_tstrcmp(ffd.cFileName, _T(".")) && mir_tstrcmp(ffd.cFileName, _T(".."))) {
			TCHAR buf[MAX_PATH], profile[MAX_PATH];
			mir_sntprintf(buf, SIZEOF(buf), _T("%s\\%s\\%s.dat"), szProfileDir, ffd.cFileName, ffd.cFileName);
			if (_taccess(buf, 0) == 0) {
				mir_sntprintf(profile, SIZEOF(profile), _T("%s.dat"), ffd.cFileName);
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
		if (_tcschr(_T(".?/\\#' "), (TCHAR)wParam) != 0)
			return 0;
		PostMessage(GetParent(edit), WM_INPUTCHANGED, 0, 0);
	}
	return mir_callNextSubclass(edit, ProfileNameValidate, msg, wParam, lParam);
}

class CCreateProfileDlg : public CDlgBase
{
	CCtrlButton &m_btnOk;
	PROFILEMANAGERDATA *m_pd;

	int CreateProfile(TCHAR *profile, DATABASELINK *link)
	{
		TCHAR buf[256];
		int err = 0;
		// check if the file already exists
		TCHAR *file = _tcsrchr(profile, '\\');
		if (file) file++;
		if (_taccess(profile, 0) == 0) {
			// file already exists!
			mir_sntprintf(buf, SIZEOF(buf),
				TranslateT("The profile '%s' already exists. Do you want to move it to the Recycle Bin?\n\nWARNING: The profile will be deleted if Recycle Bin is disabled.\nWARNING: A profile may contain confidential information and should be properly deleted."),
				file);
			if (MessageBox(m_hwnd, buf, TranslateT("The profile already exists"), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) != IDYES)
				return 0;

			// move the file
			SHFILEOPSTRUCT sf = { 0 };
			sf.wFunc = FO_DELETE;
			sf.pFrom = buf;
			sf.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT | FOF_ALLOWUNDO;
			mir_sntprintf(buf, SIZEOF(buf), _T("%s\0"), profile);
			if (SHFileOperation(&sf) != 0) {
				mir_sntprintf(buf, SIZEOF(buf), TranslateT("Couldn't move '%s' to the Recycle Bin. Please select another profile name."), file);
				MessageBox(m_hwnd, buf, TranslateT("Problem moving profile"), MB_ICONINFORMATION | MB_OK);
				return 0;
			}
			// now the file should be gone!
		}
		// ask the database to create the profile
		CreatePathToFileT(profile);
		if ((err = link->makeDatabase(profile)) != ERROR_SUCCESS) {
			mir_sntprintf(buf, SIZEOF(buf), TranslateT("Unable to create the profile '%s', the error was %x"), file, err);
			MessageBox(m_hwnd, buf, TranslateT("Problem creating profile"), MB_ICONERROR | MB_OK);
			return 0;
		}

		// the profile has been created!
		g_bDbCreated = true;
		return 1;
	}

	CCtrlCombo m_driverList;
	CCtrlEdit  m_profileName;
	CCtrlBase  m_warning;

public:
	CCreateProfileDlg(CCtrlButton &_btn, PROFILEMANAGERDATA *_pd) :
		CDlgBase(hInst, IDD_PROFILE_NEW),
		m_btnOk(_btn),
		m_pd(_pd),
		m_driverList(this, IDC_PROFILEDRIVERS),
		m_profileName(this, IDC_PROFILENAME),
		m_warning(this, IDC_NODBDRIVERS)
	{}

	virtual void OnInitDialog()
	{
		// what, no plugins?!
		if (arDbPlugins.getCount() == 0) {
			m_driverList.Enable(false);
			m_profileName.Enable(false);
			ShowWindow(m_warning.GetHwnd(), TRUE);
		}
		else {
			for (int i = 0; i < arDbPlugins.getCount(); i++) {
				DATABASELINK *p = arDbPlugins[i];
				m_driverList.AddString(TranslateTS(p->szFullName), (LPARAM)p);
			}
		}

		// default item
		m_driverList.SetCurSel(0);

		// subclass the profile name box
		mir_subclassWindow(m_profileName.GetHwnd(), ProfileNameValidate);

		// decide if there is a default profile name given in the INI and if it should be used
		if (m_pd->noProfiles || (shouldAutoCreate(m_pd->ptszProfile) && _taccess(m_pd->ptszProfile, 0))) {
			TCHAR *profile = _tcsrchr(m_pd->ptszProfile, '\\');
			if (profile) ++profile;
			else profile = m_pd->ptszProfile;

			TCHAR *p = _tcsrchr(profile, '.');
			TCHAR c = 0;
			if (p) { c = *p; *p = 0; }

			m_profileName.SetText(profile);
			if (c) *p = c;
		}

		// focus on the textbox
		PostMessage(m_hwnd, WM_FOCUSTEXTBOX, 0, 0);
	}

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
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
			}
			break;
		}
		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

	virtual void OnApply()
	{
		LRESULT curSel = m_driverList.GetCurSel();
		if (curSel == -1)
			return; // should never happen

		ptrT szName(m_profileName.GetText());
		if (szName == 0)
			return;

		// profile placed in "profile_name" subfolder
		mir_sntprintf(m_pd->ptszProfile, MAX_PATH, _T("%s\\%s\\%s.dat"), m_pd->ptszProfileDir, szName, szName);
		m_pd->newProfile = 1;
		m_pd->dblink = (DATABASELINK *)m_driverList.GetItemData(curSel);

		if (CreateProfile(m_pd->ptszProfile, m_pd->dblink) == 0)
			SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
		else
			m_pd->bRun = true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Profile selector

class CChooseProfileDlg : public CDlgBase
{
	CCtrlButton &m_btnOk;
	PROFILEMANAGERDATA *m_pd;
	HANDLE m_hFileNotify;

	struct ProfileEnumData
	{
		CCtrlListView &list;
		TCHAR* szProfile;
	};

	static BOOL EnumProfilesForList(TCHAR *tszFullPath, TCHAR *profile, LPARAM lParam)
	{
		ProfileEnumData *ped = (ProfileEnumData*)lParam;
		CCtrlListView &list = ped->list;

		TCHAR sizeBuf[64];
		bool bFileLocked = true;

		TCHAR *p = _tcsrchr(profile, '.');
		mir_tstrcpy(sizeBuf, _T("0 KB"));
		if (p != NULL) *p = 0;

		LVITEM item = { 0 };
		item.mask = LVIF_TEXT | LVIF_IMAGE;
		item.pszText = profile;
		item.iItem = 0;

		struct _stat statbuf;
		if (_tstat(tszFullPath, &statbuf) == 0) {
			if (statbuf.st_size > 1000000) {
				mir_sntprintf(sizeBuf, SIZEOF(sizeBuf), _T("%.3lf"), (double)statbuf.st_size / 1048576.0);
				mir_tstrcpy(sizeBuf + 5, _T(" MB"));
			}
			else {
				mir_sntprintf(sizeBuf, SIZEOF(sizeBuf), _T("%.3lf"), (double)statbuf.st_size / 1024.0);
				mir_tstrcpy(sizeBuf + 5, _T(" KB"));
			}
			bFileLocked = !fileExist(tszFullPath);
		}

		DATABASELINK *dblink;
		switch (touchDatabase(tszFullPath, &dblink)) {
		case ERROR_SUCCESS:
			item.iImage = bFileLocked;
			break;

		case EGROKPRF_OBSOLETE:
			item.iImage = 2;
			break;

		default:
			item.iImage = 3;
		}

		int iItem = list.InsertItem(&item);
		if (mir_tstrcmpi(ped->szProfile, tszFullPath) == 0)
			list.SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

		list.SetItemText(iItem, 2, sizeBuf);

		if (dblink != NULL) {
			if (bFileLocked) // file locked
				list.SetItemText(iItem, 1, TranslateT("<In use>"));
			else
				list.SetItemText(iItem, 1, TranslateTS(dblink->szFullName));
		}
		else list.SetItemText(iItem, 1, TranslateT("<Unknown format>"));

		return TRUE;
	}

	void CheckProfile(int iItem)
	{
		if (iItem < 0)
			return;

		TCHAR profile[MAX_PATH], fullName[MAX_PATH];
		LVITEM item = { 0 };
		item.mask = LVIF_TEXT | LVIF_IMAGE;
		item.iItem = iItem;
		item.pszText = profile;
		item.cchTextMax = SIZEOF(profile);
		if (!m_profileList.GetItem(&item))
			return;

		mir_sntprintf(fullName, SIZEOF(fullName), _T("%s\\%s\\%s.dat"), m_pd->ptszProfileDir, profile, profile);
		CallService(MS_DB_CHECKPROFILE, (WPARAM)fullName, item.iImage == 2);
	}

	void DeleteProfile(int iItem)
	{
		if (iItem < 0)
			return;

		TCHAR profile[MAX_PATH], profilef[MAX_PATH * 2];

		LVITEM item = { 0 };
		item.mask = LVIF_TEXT;
		item.iItem = iItem;
		item.pszText = profile;
		item.cchTextMax = SIZEOF(profile);
		if (!m_profileList.GetItem(&item))
			return;

		mir_sntprintf(profilef, SIZEOF(profilef), TranslateT("Are you sure you want to remove profile \"%s\"?"), profile);
		if (IDYES != MessageBox(NULL, profilef, _T("Miranda NG"), MB_YESNO | MB_TASKMODAL | MB_ICONWARNING))
			return;

		mir_sntprintf(profilef, SIZEOF(profilef), _T("%s\\%s%c"), m_pd->ptszProfileDir, profile, 0);

		SHFILEOPSTRUCT sf = { 0 };
		sf.wFunc = FO_DELETE;
		sf.pFrom = profilef;
		sf.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_ALLOWUNDO;
		SHFileOperation(&sf);
		m_profileList.DeleteItem(item.iItem);
	}

	void CheckRun()
	{
		m_btnOk.Enable(m_profileList.GetSelectedCount() == 1);

		TCHAR profile[MAX_PATH];
		LVITEM item = { 0 };
		item.mask = LVIF_TEXT | LVIF_IMAGE;
		item.iItem = m_profileList.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
		item.pszText = profile;
		item.cchTextMax = SIZEOF(profile);
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

		TCHAR tmpPath[MAX_PATH];
		mir_sntprintf(tmpPath, SIZEOF(tmpPath), _T("%s\\%s.dat"), m_pd->ptszProfileDir, profile);
		if (_taccess(tmpPath, 2))
			mir_sntprintf(m_pd->ptszProfile, MAX_PATH, _T("%s\\%s\\%s.dat"), m_pd->ptszProfileDir, profile, profile);
		else
			_tcsncpy_s(m_pd->ptszProfile, MAX_PATH, tmpPath, _TRUNCATE);
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

		LVITEM tvi = { 0 };
		tvi.mask = LVIF_IMAGE;
		tvi.iItem = lvht.iItem;
		if (!m_profileList.GetItem(&tvi))
			return;

		bool bConvert = (tvi.iImage == 2);

		lvht.pt.x = GET_X_LPARAM(lParam);
		lvht.pt.y = GET_Y_LPARAM(lParam);

		HMENU hMenu = CreatePopupMenu();
		if (tvi.iImage < 2) {
			AppendMenu(hMenu, MF_STRING, 1, TranslateT("Run"));
			AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
		}
		if (tvi.iImage != 3 && ServiceExists(MS_DB_CHECKPROFILE)) {
			if (bConvert)
				AppendMenu(hMenu, MF_STRING, 2, TranslateT("Convert database"));
			else
				AppendMenu(hMenu, MF_STRING, 2, TranslateT("Check database"));
			AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
		}
		AppendMenu(hMenu, MF_STRING, 3, TranslateT("Delete"));
		int index = TrackPopupMenu(hMenu, TPM_RETURNCMD, lvht.pt.x, lvht.pt.y, 0, m_hwnd, NULL);
		switch (index) {
		case 1:
			SendMessage(GetParent(m_hwndParent), WM_COMMAND, IDOK, 0);
			break;

		case 2:
			CheckProfile(lvht.iItem);
			break;

		case 3:
			DeleteProfile(lvht.iItem);
			break;
		}
		DestroyMenu(hMenu);
	}

	CCtrlListView m_profileList;

public:
	CChooseProfileDlg(CCtrlButton &_btn, PROFILEMANAGERDATA *_pd) :
		CDlgBase(hInst, IDD_PROFILE_SELECTION),
		m_btnOk(_btn),
		m_pd(_pd),
		m_profileList(this, IDC_PROFILELIST)
	{
		m_profileList.OnItemChanged = Callback(this, &CChooseProfileDlg::list_OnItemChanged);
		m_profileList.OnKeyDown = Callback(this, &CChooseProfileDlg::list_OnKeyDown);
		m_profileList.OnGetInfoTip = Callback(this, &CChooseProfileDlg::list_OnGetTip);
		m_profileList.OnDoubleClick = Callback(this, &CChooseProfileDlg::list_OnDblClick);
	}

	virtual void OnInitDialog()
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
		ProfileEnumData ped = { m_profileList, m_pd->ptszProfile };
		findProfiles(m_pd->ptszProfileDir, EnumProfilesForList, (LPARAM)&ped);
		PostMessage(m_hwnd, WM_FOCUSTEXTBOX, 0, 0);

		m_hFileNotify = FindFirstChangeNotification(m_pd->ptszProfileDir, TRUE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE);
		if (m_hFileNotify != INVALID_HANDLE_VALUE)
			SetTimer(m_hwnd, 0, 1200, NULL);
	}

	virtual void OnDestroy()
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
		if (evt->nmlvkey->wVKey == VK_DELETE)
			DeleteProfile(m_profileList.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL));
	}

	void list_OnGetTip(CCtrlListView::TEventInfo *evt)
	{
		if (auto pTip = evt->nmlvit) {
			TCHAR profilename[MAX_PATH], tszFullPath[MAX_PATH];
			struct _stat statbuf;
			m_profileList.GetItemText(pTip->iItem, 0, profilename, SIZEOF(profilename));
			mir_sntprintf(tszFullPath, SIZEOF(tszFullPath), _T("%s\\%s\\%s.dat"), m_pd->ptszProfileDir, profilename, profilename);
			_tstat(tszFullPath, &statbuf);
			mir_sntprintf(pTip->pszText, pTip->cchTextMax, _T("%s\n%s: %s\n%s: %s"), tszFullPath, TranslateT("Created"), rtrimt(NEWTSTR_ALLOCA(_tctime(&statbuf.st_ctime))), TranslateT("Modified"), rtrimt(NEWTSTR_ALLOCA(_tctime(&statbuf.st_mtime))));
		}
	}

	void list_OnDblClick(CCtrlListView::TEventInfo *evt)
	{
		CheckRun();
		EndDialog(GetParent(m_hwndParent), 1);
	}

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg) {
		case WM_TIMER:
			if (WaitForSingleObject(m_hFileNotify, 0) == WAIT_OBJECT_0) {
				m_profileList.DeleteAllItems();
				ProfileEnumData ped = { m_profileList, m_pd->ptszProfile };
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
	CCtrlCombo  m_servicePlugs;
	CCtrlBase   m_warning;

public:
	CProfileManager(PROFILEMANAGERDATA *_pd) :
		CDlgBase(hInst, IDD_PROFILEMANAGER),
		m_btnOk(this, IDOK),
		m_pd(_pd),
		m_tab(this, IDC_TABS),
		m_servicePlugs(this, IDC_SM_COMBO),
		m_warning(this, IDC_SM_LABEL)
	{
		m_btnOk.OnClick = Callback(this, &CProfileManager::onOk);

		m_tab.AddPage(LPGENT("My profiles"), NULL, new CChooseProfileDlg(m_btnOk, m_pd));
		m_tab.AddPage(LPGENT("New profile"), NULL, new CCreateProfileDlg(m_btnOk, m_pd));
	}
	
	virtual void OnInitDialog()
	{
		SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadImage(hInst, MAKEINTRESOURCE(IDI_DETAILSLOGO), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0));
		SendMessage(m_hwnd, WM_SETICON, ICON_BIG, (LPARAM)LoadImage(hInst, MAKEINTRESOURCE(IDI_DETAILSLOGO), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0));

		if (m_pd->noProfiles || shouldAutoCreate(m_pd->ptszProfile))
			m_tab.ActivatePage(1);

		// service mode combobox
		if (servicePlugins.getCount() == 0) {
			ShowWindow(m_warning.GetHwnd(), FALSE);
			ShowWindow(m_servicePlugs.GetHwnd(), FALSE);
		}
		else {
			m_servicePlugs.AddStringA("", -1);
			m_servicePlugs.SetCurSel(0);

			for (int i = 0; i < servicePlugins.getCount(); i++) {
				pluginEntry *p = servicePlugins[i];
				m_servicePlugs.AddString(TranslateTS(p->pluginname), i);
			}
		}
	}

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg) {
		case WM_CTLCOLORSTATIC:
			switch (GetDlgCtrlID((HWND)lParam)) {
			case IDC_WHITERECT:
				SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
				return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
			}
			break;
		}
		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

	virtual void OnDestroy()
	{
		LRESULT curSel = m_servicePlugs.GetCurSel();
		if (curSel != -1) {
			int idx = m_servicePlugs.GetItemData(curSel);
			if (idx != -1)
				SetServiceModePlugin(servicePlugins[idx]);
		}

		DestroyIcon((HICON)SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, 0));
		DestroyIcon((HICON)SendMessage(m_hwnd, WM_SETICON, ICON_BIG, 0));
	}

	void onOk(CCtrlButton*)
	{
		EndDialog(m_hwnd, 1);
	}
};

int getProfileManager(PROFILEMANAGERDATA *pd)
{
	EnsureCheckerLoaded(true);

	return CProfileManager(pd).DoModal();
}
