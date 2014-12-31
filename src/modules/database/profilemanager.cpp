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

struct DetailsPageInit
{
	int pageCount;
	OPTIONSDIALOGPAGE *odp;
};

struct DetailsPageData
{
	DLGTEMPLATE *pTemplate;
	HINSTANCE hInst;
	DLGPROC dlgProc;
	HWND hwnd;
	int changed;
};

struct DlgProfData
{
	PROPSHEETHEADER *psh;
	HWND hwndOK;
	PROFILEMANAGERDATA *pd;
	HANDLE hFileNotify;
};

struct DetailsData
{
	HINSTANCE hInstIcmp;
	HFONT hBoldFont;
	int pageCount;
	int currentPage;
	DetailsPageData *opd;
	RECT rcDisplay;
	DlgProfData *prof;
};

struct ProfileEnumData
{
	HWND hwnd;
	TCHAR* szProfile;
};

void SetServiceModePlugin(pluginEntry *p);

static void ThemeDialogBackground(HWND hwnd)
{
	EnableThemeDialogTexture(hwnd, ETDT_ENABLETAB);
}

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
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && _tcscmp(ffd.cFileName, _T(".")) && _tcscmp(ffd.cFileName, _T(".."))) {
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

static int CreateProfile(TCHAR *profile, DATABASELINK *link, HWND hwndDlg)
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
		if (MessageBox(hwndDlg, buf, TranslateT("The profile already exists"), MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2) != IDYES)
			return 0;

		// move the file
		SHFILEOPSTRUCT sf = {0};
		sf.wFunc = FO_DELETE;
		sf.pFrom = buf;
		sf.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT | FOF_ALLOWUNDO;
		mir_sntprintf(buf, SIZEOF(buf), _T("%s\0"), profile);
		if (SHFileOperation(&sf) != 0) {
			mir_sntprintf(buf, SIZEOF(buf), TranslateT("Couldn't move '%s' to the Recycle Bin. Please select another profile name."), file);
			MessageBox(0, buf, TranslateT("Problem moving profile"), MB_ICONINFORMATION|MB_OK);
			return 0;
		}
		// now the file should be gone!
	}
	// ask the database to create the profile
	CreatePathToFileT(profile);
	if ((err = link->makeDatabase(profile)) != ERROR_SUCCESS) {
		mir_sntprintf(buf, SIZEOF(buf), TranslateT("Unable to create the profile '%s', the error was %x"), file, err);
		MessageBox(hwndDlg, buf, TranslateT("Problem creating profile"), MB_ICONERROR|MB_OK);
		return 0;
	}

	// the profile has been created!
	g_bDbCreated = true;
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

static INT_PTR CALLBACK DlgProfileNew(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DlgProfData *dat = (DlgProfData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		dat = (DlgProfData*)lParam;
		{
			HWND hwndCombo = GetDlgItem(hwndDlg, IDC_PROFILEDRIVERS);

			// what, no plugins?!
			if (arDbPlugins.getCount() == 0) {
				EnableWindow(hwndCombo, FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_PROFILENAME), FALSE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_NODBDRIVERS), TRUE);
			}
			else {
				for (int i = 0; i < arDbPlugins.getCount(); i++) {
					DATABASELINK *p = arDbPlugins[i];
					LRESULT index = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)TranslateTS(p->szFullName));
					SendMessage(hwndCombo, CB_SETITEMDATA, index, (LPARAM)p);
				}
			}

			// default item
			SendMessage(hwndCombo, CB_SETCURSEL, 0, 0);

			// subclass the profile name box
			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_PROFILENAME), ProfileNameValidate);
		}

		// decide if there is a default profile name given in the INI and if it should be used
		if (dat->pd->noProfiles || (shouldAutoCreate(dat->pd->ptszProfile) && _taccess(dat->pd->ptszProfile, 0))) {
			TCHAR *profile = _tcsrchr(dat->pd->ptszProfile, '\\');
			if (profile) ++profile;
			else profile = dat->pd->ptszProfile;

			TCHAR *p = _tcsrchr(profile, '.');
			TCHAR c = 0;
			if (p) { c = *p; *p = 0; }

			SetDlgItemText(hwndDlg, IDC_PROFILENAME, profile);
			if (c) *p = c;
		}

		// focus on the textbox
		PostMessage(hwndDlg, WM_FOCUSTEXTBOX, 0, 0);
		return TRUE;

	case WM_FOCUSTEXTBOX:
		SetFocus(GetDlgItem(hwndDlg, IDC_PROFILENAME));
		break;

	case WM_INPUTCHANGED: // when input in the edit box changes
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		EnableWindow(dat->hwndOK, GetWindowTextLength(GetDlgItem(hwndDlg, IDC_PROFILENAME)) > 0);
		break;

	case WM_SHOWWINDOW:
		if (wParam) {
			SetWindowText(dat->hwndOK, TranslateT("&Create"));
			SendMessage(hwndDlg, WM_INPUTCHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		NMHDR *hdr = (NMHDR*)lParam;
		if (hdr && hdr->code == PSN_APPLY && dat && IsWindowVisible(hwndDlg)) {
			TCHAR szName[MAX_PATH];
			LRESULT curSel = SendDlgItemMessage(hwndDlg, IDC_PROFILEDRIVERS, CB_GETCURSEL, 0, 0);
			if (curSel == CB_ERR)
				break; // should never happen

			GetDlgItemText(hwndDlg, IDC_PROFILENAME, szName, SIZEOF(szName));
			if (szName[0] == 0)
				break;

			// profile placed in "profile_name" subfolder
			mir_sntprintf(dat->pd->ptszProfile, MAX_PATH, _T("%s\\%s\\%s.dat"), dat->pd->ptszProfileDir, szName, szName);
			dat->pd->newProfile = 1;
			dat->pd->dblink = (DATABASELINK *)SendDlgItemMessage(hwndDlg, IDC_PROFILEDRIVERS, CB_GETITEMDATA, (WPARAM)curSel, 0);

			if (CreateProfile(dat->pd->ptszProfile, dat->pd->dblink, hwndDlg) == 0)
				SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
			else
				dat->pd->bRun = true;
		}
		break;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Profile selector

BOOL EnumProfilesForList(TCHAR *tszFullPath, TCHAR *profile, LPARAM lParam)
{
	ProfileEnumData *ped = (ProfileEnumData*)lParam;
	HWND hwndList = GetDlgItem(ped->hwnd, IDC_PROFILELIST);

	TCHAR sizeBuf[64];
	bool bFileLocked = true;

	TCHAR *p = _tcsrchr(profile, '.');
	_tcscpy(sizeBuf, _T("0 KB"));
	if (p != NULL) *p = 0;

	LVITEM item = { 0 };
	item.mask = LVIF_TEXT | LVIF_IMAGE;
	item.pszText = profile;
	item.iItem = 0;

	struct _stat statbuf;
	if (_tstat(tszFullPath, &statbuf) == 0) {
		if (statbuf.st_size > 1000000) {
			mir_sntprintf(sizeBuf, SIZEOF(sizeBuf), _T("%.3lf"), (double)statbuf.st_size / 1048576.0);
			_tcscpy(sizeBuf + 5, _T(" MB"));
		}
		else {
			mir_sntprintf(sizeBuf, SIZEOF(sizeBuf), _T("%.3lf"), (double)statbuf.st_size / 1024.0);
			_tcscpy(sizeBuf + 5, _T(" KB"));
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

	int iItem = SendMessage(hwndList, LVM_INSERTITEM, 0, (LPARAM)&item);
	if (mir_tstrcmpi(ped->szProfile, tszFullPath) == 0)
		ListView_SetItemState(hwndList, iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

	item.iItem = iItem;
	item.iSubItem = 2;
	item.pszText = sizeBuf;
	SendMessage(hwndList, LVM_SETITEMTEXT, iItem, (LPARAM)&item);

	if (dblink != NULL) {
		if (bFileLocked) // file locked
			item.pszText = TranslateT("<In use>");
		else
			item.pszText = TranslateTS(dblink->szFullName);
	}
	else item.pszText = TranslateT("<Unknown format>");

	item.iSubItem = 1;
	SendMessage(hwndList, LVM_SETITEMTEXT, iItem, (LPARAM)&item);
	return TRUE;
}

void CheckProfile(HWND hwndList, int iItem, DlgProfData *dat)
{
	if (iItem < 0)
		return;

	TCHAR profile[MAX_PATH], fullName[MAX_PATH];
	LVITEM item = { 0 };
	item.mask = LVIF_TEXT | LVIF_IMAGE;
	item.iItem = iItem;
	item.pszText = profile;
	item.cchTextMax = SIZEOF(profile);
	if (!ListView_GetItem(hwndList, &item))
		return;

	mir_sntprintf(fullName, SIZEOF(fullName), _T("%s\\%s\\%s.dat"), dat->pd->ptszProfileDir, profile, profile);
	CallService(MS_DB_CHECKPROFILE, (WPARAM)fullName, item.iImage == 2);
}

void DeleteProfile(HWND hwndList, int iItem, DlgProfData *dat)
{
	if (iItem < 0)
		return;

	TCHAR profile[MAX_PATH], profilef[MAX_PATH * 2];

	LVITEM item = { 0 };
	item.mask = LVIF_TEXT;
	item.iItem = iItem;
	item.pszText = profile;
	item.cchTextMax = SIZEOF(profile);
	if (!ListView_GetItem(hwndList, &item))
		return;

	mir_sntprintf(profilef, SIZEOF(profilef), TranslateT("Are you sure you want to remove profile \"%s\"?"), profile);
	if (IDYES != MessageBox(NULL, profilef, _T("Miranda NG"), MB_YESNO | MB_TASKMODAL | MB_ICONWARNING))
		return;

	mir_sntprintf(profilef, SIZEOF(profilef), _T("%s\\%s%c"), dat->pd->ptszProfileDir, profile, 0);

	SHFILEOPSTRUCT sf = { 0 };
	sf.wFunc = FO_DELETE;
	sf.pFrom = profilef;
	sf.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_ALLOWUNDO;
	SHFileOperation(&sf);
	ListView_DeleteItem(hwndList, item.iItem);
}

static void CheckRun(HWND hwndDlg, int uMsg)
{
	DlgProfData *dat = (DlgProfData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	if (dat == NULL)
		return;

	HWND hwndList = GetDlgItem(hwndDlg, IDC_PROFILELIST);
	EnableWindow(dat->hwndOK, ListView_GetSelectedCount(hwndList) == 1);

	TCHAR profile[MAX_PATH];
	LVITEM item = { 0 };
	item.mask = LVIF_TEXT | LVIF_IMAGE;
	item.iItem = ListView_GetNextItem(hwndList, -1, LVNI_SELECTED | LVNI_ALL);
	item.pszText = profile;
	item.cchTextMax = SIZEOF(profile);
	if (!ListView_GetItem(hwndList, &item))
		return;

	switch(item.iImage) {
	case 3:
		EnableWindow(dat->hwndOK, false);
		return;

	case 2:
		SetWindowText(dat->hwndOK, TranslateT("&Convert"));
		dat->pd->bRun = false;
		break;

	default:
		SetWindowText(dat->hwndOK, TranslateT("&Run"));
		dat->pd->bRun = true;
	}

	// profile is placed in "profile_name" subfolder

	TCHAR tmpPath[MAX_PATH];
	mir_sntprintf(tmpPath, SIZEOF(tmpPath), _T("%s\\%s.dat"), dat->pd->ptszProfileDir, profile);
	if (_taccess(tmpPath, 2))
		mir_sntprintf(dat->pd->ptszProfile, MAX_PATH, _T("%s\\%s\\%s.dat"), dat->pd->ptszProfileDir, profile, profile);
	else
		_tcsncpy_s(dat->pd->ptszProfile, MAX_PATH, tmpPath, _TRUNCATE);

	if (uMsg == NM_DBLCLK)
		EndDialog(GetParent(hwndDlg), 1);
}

static void ExecuteMenu(HWND hwndDlg, LPARAM lParam)
{
	HWND hwndList = GetDlgItem(hwndDlg, IDC_PROFILELIST);
	DlgProfData *dat = (DlgProfData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	LVHITTESTINFO lvht = { 0 };
	lvht.pt.x = GET_X_LPARAM(lParam);
	lvht.pt.y = GET_Y_LPARAM(lParam);
	ScreenToClient(hwndList, &lvht.pt);

	if (ListView_HitTest(hwndList, &lvht) == -1)
		return;

	if (lvht.iItem == -1)
		return;

	LVITEM tvi = { 0 };
	tvi.mask = LVIF_IMAGE;
	tvi.iItem = lvht.iItem;
	if (!ListView_GetItem(hwndList, &tvi))
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
	int index = TrackPopupMenu(hMenu, TPM_RETURNCMD, lvht.pt.x, lvht.pt.y, 0, hwndDlg, NULL);
	switch (index) {
	case 1:
		SendMessage(GetParent(hwndDlg), WM_COMMAND, IDOK, 0);
		break;

	case 2:
		CheckProfile(hwndList, lvht.iItem, dat);
		break;

	case 3:
		DeleteProfile(hwndList, lvht.iItem, dat);
		break;
	}
	DestroyMenu(hMenu);
}

static INT_PTR CALLBACK DlgProfileSelect(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DlgProfData *dat = (DlgProfData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	HWND hwndList = GetDlgItem(hwndDlg, IDC_PROFILELIST);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			dat = (DlgProfData*)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);

			// set columns
			LVCOLUMN col;
			col.mask = LVCF_TEXT | LVCF_WIDTH;
			col.pszText = TranslateT("Profile");
			col.cx = 100;
			ListView_InsertColumn(hwndList, 0, &col);

			col.pszText = TranslateT("Driver");
			col.cx = 150 - GetSystemMetrics(SM_CXVSCROLL);
			ListView_InsertColumn(hwndList, 1, &col);

			col.pszText = TranslateT("Size");
			col.cx = 60;
			ListView_InsertColumn(hwndList, 2, &col);

			// icons
			HIMAGELIST hImgList = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 2, 1);
			ImageList_AddIcon_NotShared(hImgList, MAKEINTRESOURCE(IDI_USERDETAILS));
			ImageList_AddIcon_NotShared(hImgList, MAKEINTRESOURCE(IDI_DELETE));
			ImageList_AddIcon_NotShared(hImgList, MAKEINTRESOURCE(IDI_MWARNING));
			ImageList_AddIcon_NotShared(hImgList, MAKEINTRESOURCE(IDI_MFATAL));

			// LV will destroy the image list
			SetWindowLongPtr(hwndList, GWL_STYLE, GetWindowLongPtr(hwndList, GWL_STYLE) | LVS_SORTASCENDING);
			ListView_SetImageList(hwndList, hImgList, LVSIL_SMALL);
			ListView_SetExtendedListViewStyle(hwndList,
				ListView_GetExtendedListViewStyle(hwndList) | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP | LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT);

			// find all the profiles
			ProfileEnumData ped = { hwndDlg, dat->pd->ptszProfile };
			findProfiles(dat->pd->ptszProfileDir, EnumProfilesForList, (LPARAM)&ped);
			PostMessage(hwndDlg, WM_FOCUSTEXTBOX, 0, 0);

			dat->hFileNotify = FindFirstChangeNotification(dat->pd->ptszProfileDir, TRUE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE);
			if (dat->hFileNotify != INVALID_HANDLE_VALUE)
				SetTimer(hwndDlg, 0, 1200, NULL);
			return TRUE;
		}

	case WM_DESTROY:
		KillTimer(hwndDlg, 0);
		FindCloseChangeNotification(dat->hFileNotify);
		break;

	case WM_TIMER:
		if (WaitForSingleObject(dat->hFileNotify, 0) == WAIT_OBJECT_0) {
			ListView_DeleteAllItems(hwndList);
			ProfileEnumData ped = { hwndDlg, dat->pd->ptszProfile };
			findProfiles(dat->pd->ptszProfileDir, EnumProfilesForList, (LPARAM)&ped);
			FindNextChangeNotification(dat->hFileNotify);
		}
		break;

	case WM_FOCUSTEXTBOX:
		SetFocus(hwndList);
		if (dat->pd->ptszProfile[0] == 0 || ListView_GetSelectedCount(hwndList) == 0)
			ListView_SetItemState(hwndList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		break;

	case WM_SHOWWINDOW:
		if (wParam)
			CheckRun(hwndDlg, 0);
		break;

	case WM_CONTEXTMENU:
		ExecuteMenu(hwndDlg, lParam);
		break;

	case WM_NOTIFY:
		LPNMHDR hdr = (LPNMHDR)lParam;
		if (hdr == NULL)
			break;

		if (hdr->idFrom == 0) {
			CheckRun(hwndDlg, 0);
			break;
		}

		if (hdr->idFrom == IDC_PROFILELIST) {
			switch (hdr->code) {
			case LVN_ITEMCHANGED:
			case NM_DBLCLK:
				CheckRun(hwndDlg, hdr->code);
				return TRUE;

			case LVN_KEYDOWN:
				if (((LPNMLVKEYDOWN)lParam)->wVKey == VK_DELETE)
					DeleteProfile(hwndList, ListView_GetNextItem(hwndList, -1, LVNI_SELECTED | LVNI_ALL), dat);
				break;

			case LVN_GETINFOTIP:
				NMLVGETINFOTIP *pInfoTip = (NMLVGETINFOTIP *)lParam;
				if (pInfoTip != NULL) {
					TCHAR profilename[MAX_PATH], tszFullPath[MAX_PATH];
					struct _stat statbuf;
					ListView_GetItemText(hwndList, pInfoTip->iItem, 0, profilename, SIZEOF(profilename));
					mir_sntprintf(tszFullPath, SIZEOF(tszFullPath), _T("%s\\%s\\%s.dat"), dat->pd->ptszProfileDir, profilename, profilename);
					_tstat(tszFullPath, &statbuf);
					mir_sntprintf(pInfoTip->pszText, pInfoTip->cchTextMax, _T("%s\n%s: %s\n%s: %s"), tszFullPath, TranslateT("Created"), rtrimt(NEWTSTR_ALLOCA(_tctime(&statbuf.st_ctime))), TranslateT("Modified"), rtrimt(NEWTSTR_ALLOCA(_tctime(&statbuf.st_mtime))));
				}
			}
		}
		break;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Tab manager + its envelope

static INT_PTR CALLBACK DlgProfileManager(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DetailsData *dat = (DetailsData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			DlgProfData *prof = (DlgProfData*)lParam;
			PROPSHEETHEADER *psh = prof->psh;
			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadImage(hInst, MAKEINTRESOURCE(IDI_DETAILSLOGO), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0));
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadImage(hInst, MAKEINTRESOURCE(IDI_DETAILSLOGO), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0));
			dat = (DetailsData*)mir_alloc(sizeof(DetailsData));
			dat->prof = prof;
			prof->hwndOK = GetDlgItem(hwndDlg, IDOK);
			EnableWindow(prof->hwndOK, FALSE);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);

			TCHAR buf[512];
			mir_sntprintf(buf, SIZEOF(buf), _T("%s\n%s"), TranslateT("Miranda NG profile manager"), TranslateT("Manage your Miranda NG profile"));
			SetDlgItemText(hwndDlg, IDC_NAME, buf);

			dat->currentPage = 0;
			dat->pageCount = psh->nPages;
			dat->opd = (DetailsPageData*)mir_calloc(sizeof(DetailsPageData)*dat->pageCount);
			OPTIONSDIALOGPAGE *odp = (OPTIONSDIALOGPAGE*)psh->ppsp;

			TCITEM tci;
			tci.mask = TCIF_TEXT;
			for (int i = 0; i < dat->pageCount; i++) {
				dat->opd[i].pTemplate = (DLGTEMPLATE*)LockResource(LoadResource(odp[i].hInstance, FindResourceA(odp[i].hInstance, odp[i].pszTemplate, MAKEINTRESOURCEA(5))));
				dat->opd[i].dlgProc = odp[i].pfnDlgProc;
				dat->opd[i].hInst = odp[i].hInstance;
				dat->opd[i].hwnd = NULL;
				dat->opd[i].changed = 0;
				tci.pszText = (TCHAR*)odp[i].ptszTitle;
				if (dat->prof->pd->noProfiles || shouldAutoCreate(dat->prof->pd->ptszProfile))
					dat->currentPage = 1;
				TabCtrl_InsertItem(GetDlgItem(hwndDlg, IDC_TABS), i, &tci);
			}

			GetWindowRect(GetDlgItem(hwndDlg, IDC_TABS), &dat->rcDisplay);
			TabCtrl_AdjustRect(GetDlgItem(hwndDlg, IDC_TABS), FALSE, &dat->rcDisplay);

			POINT pt = { 0, 0 };
			ClientToScreen(hwndDlg, &pt);
			OffsetRect(&dat->rcDisplay, -pt.x, -pt.y);

			TabCtrl_SetCurSel(GetDlgItem(hwndDlg, IDC_TABS), dat->currentPage);
			dat->opd[dat->currentPage].hwnd = CreateDialogIndirectParam(dat->opd[dat->currentPage].hInst, dat->opd[dat->currentPage].pTemplate, hwndDlg, dat->opd[dat->currentPage].dlgProc, (LPARAM)dat->prof);
			ThemeDialogBackground(dat->opd[dat->currentPage].hwnd);
			SetWindowPos(dat->opd[dat->currentPage].hwnd, HWND_TOP, dat->rcDisplay.left, dat->rcDisplay.top, 0, 0, SWP_NOSIZE);

			PSHNOTIFY pshn;
			pshn.hdr.code = PSN_INFOCHANGED;
			pshn.hdr.hwndFrom = dat->opd[dat->currentPage].hwnd;
			pshn.hdr.idFrom = 0;
			pshn.lParam = 0;
			SendMessage(dat->opd[dat->currentPage].hwnd, WM_NOTIFY, 0, (LPARAM)&pshn);

			// service mode combobox
			if (servicePlugins.getCount() == 0) {
				ShowWindow(GetDlgItem(hwndDlg, IDC_SM_LABEL), FALSE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_SM_COMBO), FALSE);
			}
			else {
				HWND hwndCombo = GetDlgItem(hwndDlg, IDC_SM_COMBO);
				LRESULT index = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)_T(""));
				SendMessage(hwndCombo, CB_SETITEMDATA, index, (LPARAM)-1);
				SendMessage(hwndCombo, CB_SETCURSEL, 0, 0);
				for (int i = 0; i < servicePlugins.getCount(); i++) {
					pluginEntry *p = servicePlugins[i];
					index = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)TranslateTS(p->pluginname));
					SendMessage(hwndCombo, CB_SETITEMDATA, index, (LPARAM)i);
				}
			}
			ShowWindow(dat->opd[dat->currentPage].hwnd, SW_SHOW);
		}
		return TRUE;

	case WM_CTLCOLORSTATIC:
		switch (GetDlgCtrlID((HWND)lParam)) {
		case IDC_WHITERECT:
			SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
			return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
		}
		break;

	case PSM_CHANGED:
		dat->opd[dat->currentPage].changed = 1;
		return TRUE;

	case PSM_FORCECHANGED:
		{
			PSHNOTIFY pshn;
			pshn.hdr.code = PSN_INFOCHANGED;
			pshn.hdr.idFrom = 0;
			pshn.lParam = 0;
			for (int i = 0; i < dat->pageCount; i++) {
				pshn.hdr.hwndFrom = dat->opd[i].hwnd;
				if (dat->opd[i].hwnd != NULL)
					SendMessage(dat->opd[i].hwnd, WM_NOTIFY, 0, (LPARAM)&pshn);
			}
		}
		break;

	case WM_NOTIFY:
		switch (wParam) {
		case IDC_TABS:
			switch (((LPNMHDR)lParam)->code) {
			case TCN_SELCHANGING:
				if (dat->currentPage != -1 && dat->opd[dat->currentPage].hwnd != NULL) {
					PSHNOTIFY pshn;
					pshn.hdr.code = PSN_KILLACTIVE;
					pshn.hdr.hwndFrom = dat->opd[dat->currentPage].hwnd;
					pshn.hdr.idFrom = 0;
					pshn.lParam = 0;
					if (SendMessage(dat->opd[dat->currentPage].hwnd, WM_NOTIFY, 0, (LPARAM)&pshn)) {
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
						return TRUE;
					}
				}
				break;

			case TCN_SELCHANGE:
				if (dat->currentPage != -1 && dat->opd[dat->currentPage].hwnd != NULL)
					ShowWindow(dat->opd[dat->currentPage].hwnd, SW_HIDE);

				dat->currentPage = TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_TABS));
				if (dat->currentPage != -1) {
					if (dat->opd[dat->currentPage].hwnd == NULL) {
						PSHNOTIFY pshn;
						dat->opd[dat->currentPage].hwnd = CreateDialogIndirectParam(dat->opd[dat->currentPage].hInst, dat->opd[dat->currentPage].pTemplate, hwndDlg, dat->opd[dat->currentPage].dlgProc, (LPARAM)dat->prof);
						ThemeDialogBackground(dat->opd[dat->currentPage].hwnd);
						SetWindowPos(dat->opd[dat->currentPage].hwnd, HWND_TOP, dat->rcDisplay.left, dat->rcDisplay.top, 0, 0, SWP_NOSIZE);
						pshn.hdr.code = PSN_INFOCHANGED;
						pshn.hdr.hwndFrom = dat->opd[dat->currentPage].hwnd;
						pshn.hdr.idFrom = 0;
						pshn.lParam = 0;
						SendMessage(dat->opd[dat->currentPage].hwnd, WM_NOTIFY, 0, (LPARAM)&pshn);
					}
					ShowWindow(dat->opd[dat->currentPage].hwnd, SW_SHOW);
				}
				break;
			}
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			{
				PSHNOTIFY pshn;
				pshn.hdr.idFrom = 0;
				pshn.lParam = 0;
				pshn.hdr.code = PSN_RESET;
				for (int i = 0; i < dat->pageCount; i++) {
					if (dat->opd[i].hwnd == NULL || !dat->opd[i].changed)
						continue;

					pshn.hdr.hwndFrom = dat->opd[i].hwnd;
					SendMessage(dat->opd[i].hwnd, WM_NOTIFY, 0, (LPARAM)&pshn);
				}
				EndDialog(hwndDlg, 0);
			}
			break;

		case IDC_REMOVE:
			if (!dat->prof->pd->noProfiles) {
				HWND hwndList = GetDlgItem(dat->opd[0].hwnd, IDC_PROFILELIST);
				DeleteProfile(hwndList, ListView_GetNextItem(hwndList, -1, LVNI_SELECTED | LVNI_ALL), dat->prof);
			}
			break;

		case IDOK:
			PSHNOTIFY pshn;
			pshn.hdr.idFrom = 0;
			pshn.lParam = 0;
			if (dat->currentPage != -1) {
				pshn.hdr.code = PSN_KILLACTIVE;
				pshn.hdr.hwndFrom = dat->opd[dat->currentPage].hwnd;
				if (SendMessage(dat->opd[dat->currentPage].hwnd, WM_NOTIFY, 0, (LPARAM)&pshn))
					break;
			}

			pshn.hdr.code = PSN_APPLY;
			for (int i = 0; i < dat->pageCount; i++) {
				if (dat->opd[i].hwnd == NULL || !dat->opd[i].changed)
					continue;

				pshn.hdr.hwndFrom = dat->opd[i].hwnd;
				SendMessage(dat->opd[i].hwnd, WM_NOTIFY, 0, (LPARAM)&pshn);
				if (GetWindowLongPtr(dat->opd[i].hwnd, DWLP_MSGRESULT) == PSNRET_INVALID_NOCHANGEPAGE) {
					TabCtrl_SetCurSel(GetDlgItem(hwndDlg, IDC_TABS), i);
					if (dat->currentPage != -1)
						ShowWindow(dat->opd[dat->currentPage].hwnd, SW_HIDE);
					dat->currentPage = i;
					ShowWindow(dat->opd[dat->currentPage].hwnd, SW_SHOW);
					return 0;
				}
			}
			EndDialog(hwndDlg, 1);
		}
		break;

	case WM_DESTROY:
		LRESULT curSel = SendDlgItemMessage(hwndDlg, IDC_SM_COMBO, CB_GETCURSEL, 0, 0);
		if (curSel != CB_ERR) {
			int idx = SendDlgItemMessage(hwndDlg, IDC_SM_COMBO, CB_GETITEMDATA, (WPARAM)curSel, 0);
			if (idx != CB_ERR)
				SetServiceModePlugin(servicePlugins[idx]);
		}

		DestroyIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, 0));
		DestroyIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, 0));
		DeleteObject(dat->hBoldFont);

		for (int i = 0; i < dat->pageCount; i++)
			if (dat->opd[i].hwnd != NULL)
				DestroyWindow(dat->opd[i].hwnd);

		mir_free(dat->opd);
		mir_free(dat);
		break;
	}
	return FALSE;
}

static int AddProfileManagerPage(struct DetailsPageInit *opi, OPTIONSDIALOGPAGE *odp)
{
	if (odp->cbSize != sizeof(OPTIONSDIALOGPAGE))
		return 1;

	opi->odp = (OPTIONSDIALOGPAGE*)mir_realloc(opi->odp, sizeof(OPTIONSDIALOGPAGE)*(opi->pageCount + 1));

	OPTIONSDIALOGPAGE *p = opi->odp + opi->pageCount++;
	p->cbSize = sizeof(OPTIONSDIALOGPAGE);
	p->hInstance = odp->hInstance;
	p->pfnDlgProc = odp->pfnDlgProc;
	p->position = odp->position;
	p->ptszTitle = Langpack_PcharToTchar(odp->pszTitle);
	p->pszGroup = NULL;
	p->groupPosition = odp->groupPosition;
	p->hGroupIcon = odp->hGroupIcon;
	p->hIcon = odp->hIcon;
	if ((DWORD_PTR)odp->pszTemplate & 0xFFFF0000)
		p->pszTemplate = mir_strdup(odp->pszTemplate);
	else
		p->pszTemplate = odp->pszTemplate;
	return 0;
}

int getProfileManager(PROFILEMANAGERDATA *pd)
{
	EnsureCheckerLoaded(true);

	DetailsPageInit opi = { 0 };

	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize = sizeof(odp);

	odp.pszTitle = LPGEN("My profiles");
	odp.pfnDlgProc = DlgProfileSelect;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_PROFILE_SELECTION);
	odp.hInstance = hInst;
	AddProfileManagerPage(&opi, &odp);

	odp.pszTitle = LPGEN("New profile");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_PROFILE_NEW);
	odp.pfnDlgProc = DlgProfileNew;
	AddProfileManagerPage(&opi, &odp);

	PROPSHEETHEADER psh = { 0 };
	psh.dwSize = sizeof(psh);
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
	psh.nPages = opi.pageCount;
	psh.ppsp = (PROPSHEETPAGE*)opi.odp;

	DlgProfData prof;
	prof.pd = pd;
	prof.psh = &psh;
	int rc = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PROFILEMANAGER), NULL, DlgProfileManager, (LPARAM)&prof);
	if (rc != -1) {
		for (int i = 0; i < opi.pageCount; i++) {
			mir_free(opi.odp[i].pszTitle);
			mir_free(opi.odp[i].pszGroup);
			if ((DWORD_PTR)opi.odp[i].pszTemplate & 0xFFFF0000)
				mir_free((char*)opi.odp[i].pszTemplate);
		}
	}
	mir_free(opi.odp);

	return rc;
}
