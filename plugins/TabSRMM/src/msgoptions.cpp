/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-15 Miranda NG project,
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
// Implementation of the option pages

#include "commonheaders.h"

#define DM_GETSTATUSMASK (WM_USER + 10)

struct FontOptionsList
{
	COLORREF defColour;
	char *szDefFace;
	BYTE defStyle;
	char defSize;
}

static fontOptionsList[] = {
	{ RGB(0, 0, 0), "Tahoma", 0, -10 }
};



HIMAGELIST CreateStateImageList()
{
	HIMAGELIST himlStates;

	himlStates = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 4, 0);
	ImageList_AddIcon(himlStates, PluginConfig.g_IconUnchecked); /* IMG_NOCHECK */
	ImageList_AddIcon(himlStates, PluginConfig.g_IconChecked); /* IMG_CHECK */
	ImageList_AddIcon(himlStates, PluginConfig.g_IconGroupOpen); /* IMG_GRPOPEN */
	ImageList_AddIcon(himlStates, PluginConfig.g_IconGroupClose); /* IMG_GRPCLOSED */

	return himlStates;
}

void TSAPI LoadLogfont(int i, LOGFONTA * lf, COLORREF * colour, char *szModule)
{
	LOGFONT lfResult;
	LoadMsgDlgFont((i < 100) ? FONTSECTION_IM : FONTSECTION_IP, i, &lfResult, colour, szModule);
	if (lf) {
		lf->lfHeight = lfResult.lfHeight;
		lf->lfWidth = lfResult.lfWidth;
		lf->lfEscapement = lfResult.lfEscapement;
		lf->lfOrientation = lfResult.lfOrientation;
		lf->lfWeight = lfResult.lfWeight;
		lf->lfItalic = lfResult.lfItalic;
		lf->lfUnderline = lfResult.lfUnderline;
		lf->lfStrikeOut = lfResult.lfStrikeOut;
		lf->lfCharSet = lfResult.lfCharSet;
		lf->lfOutPrecision = lfResult.lfOutPrecision;
		lf->lfClipPrecision = lfResult.lfClipPrecision;
		lf->lfQuality = lfResult.lfQuality;
		lf->lfPitchAndFamily = lfResult.lfPitchAndFamily;
		mir_snprintf(lf->lfFaceName, SIZEOF(lf->lfFaceName), "%S", lfResult.lfFaceName);
	}
}

HIMAGELIST g_himlOptions;

static HWND hwndTabConfig = 0;

/////////////////////////////////////////////////////////////////////////////////////////
// scan a single skin directory and find the.TSK file.Fill the combobox and set the
// relative path name as item extra data.
//
// If available, read the Name property from the [Global] section and use it in the
// combo box. If such property is not found, the base filename (without .tsk extension)
// will be used as the name of the skin.
//
// [Global]/Name property is new in TabSRMM version 3.

static int TSAPI ScanSkinDir(const TCHAR* tszFolder, HWND hwndCombobox)
{
	bool fValid = false;
	TCHAR tszMask[MAX_PATH];
	mir_sntprintf(tszMask, SIZEOF(tszMask), _T("%s*.*"), tszFolder);

	WIN32_FIND_DATA fd = { 0 };
	HANDLE h = FindFirstFile(tszMask, &fd);
	while (h != INVALID_HANDLE_VALUE) {
		if (mir_tstrlen(fd.cFileName) >= 5 && !_tcsnicmp(fd.cFileName + mir_tstrlen(fd.cFileName) - 4, _T(".tsk"), 4)) {
			fValid = true;
			break;
		}
		if (FindNextFile(h, &fd) == 0)
			break;
	}
	if (h != INVALID_HANDLE_VALUE)
		FindClose(h);

	if (fValid) {
		TCHAR	tszFinalName[MAX_PATH], tszRel[MAX_PATH];
		LRESULT lr;
		TCHAR	szBuf[255];

		mir_sntprintf(tszFinalName, SIZEOF(tszFinalName), _T("%s%s"), tszFolder, fd.cFileName);

		GetPrivateProfileString(_T("Global"), _T("Name"), _T("None"), szBuf, SIZEOF(szBuf), tszFinalName);
		if (!_tcscmp(szBuf, _T("None"))) {
			fd.cFileName[mir_tstrlen(fd.cFileName) - 4] = 0;
			_tcsncpy_s(szBuf, fd.cFileName, _TRUNCATE);
		}

		PathToRelativeT(tszFinalName, tszRel, M.getSkinPath());
		if ((lr = SendMessage(hwndCombobox, CB_INSERTSTRING, -1, (LPARAM)szBuf)) != CB_ERR) {
			TCHAR *idata = (TCHAR*)mir_alloc((mir_tstrlen(tszRel) + 1) * sizeof(TCHAR));

			_tcscpy(idata, tszRel);
			SendMessage(hwndCombobox, CB_SETITEMDATA, lr, (LPARAM)idata);
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// scan the skin root folder for subfolder(s).Each folder is supposed to contain a single
// skin. This function won't dive deeper into the folder structure, so the folder
// structure for any VALID skin should be:
// $SKINS_ROOT/skin_folder/skin_name.tsk
//
// By default, $SKINS_ROOT is set to %miranda_userdata% or custom folder
// selected by the folders plugin.

static int TSAPI RescanSkins(HWND hwndCombobox)
{
	DBVARIANT dbv = { 0 };

	TCHAR tszSkinRoot[MAX_PATH], tszFindMask[MAX_PATH];
	_tcsncpy_s(tszSkinRoot, M.getSkinPath(), _TRUNCATE);

	SetDlgItemText(GetParent(hwndCombobox), IDC_SKINROOTFOLDER, tszSkinRoot);
	mir_sntprintf(tszFindMask, SIZEOF(tszFindMask), _T("%s*.*"), tszSkinRoot);

	SendMessage(hwndCombobox, CB_RESETCONTENT, 0, 0);
	SendMessage(hwndCombobox, CB_INSERTSTRING, -1, (LPARAM)TranslateT("<no skin>"));

	WIN32_FIND_DATA fd = { 0 };
	HANDLE h = FindFirstFile(tszFindMask, &fd);
	while (h != INVALID_HANDLE_VALUE) {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && fd.cFileName[0] != '.') {
			TCHAR	tszSubDir[MAX_PATH];
			mir_sntprintf(tszSubDir, SIZEOF(tszSubDir), _T("%s%s\\"), tszSkinRoot, fd.cFileName);
			ScanSkinDir(tszSubDir, hwndCombobox);
		}
		if (FindNextFile(h, &fd) == 0)
			break;
	}
	if (h != INVALID_HANDLE_VALUE)
		FindClose(h);

	SendMessage(hwndCombobox, CB_SETCURSEL, 0, 0);
	if (0 == db_get_ts(0, SRMSGMOD_T, "ContainerSkin", &dbv)) {
		LRESULT lr = SendMessage(hwndCombobox, CB_GETCOUNT, 0, 0);
		for (int i = 1; i < lr; i++) {
			TCHAR *idata = (TCHAR*)SendMessage(hwndCombobox, CB_GETITEMDATA, i, 0);
			if (idata && idata != (TCHAR*)CB_ERR) {
				if (!_tcsicmp(dbv.ptszVal, idata)) {
					SendMessage(hwndCombobox, CB_SETCURSEL, i, 0);
					break;
				}
			}
		}
		db_free(&dbv);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// mir_free the item extra data (used to store the skin filenames for each entry).

static void TSAPI FreeComboData(HWND hwndCombobox)
{
	LRESULT lr = SendMessage(hwndCombobox, CB_GETCOUNT, 0, 0);

	for (int i = 1; i < lr; i++) {
		void *idata = (void*)SendMessage(hwndCombobox, CB_GETITEMDATA, i, 0);

		if (idata && idata != (void*)CB_ERR)
			mir_free(idata);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// controls to disable when loading or unloading a skin is not possible (because
// of at least one message window being open).

static UINT _ctrls[] = { IDC_SKINNAME, IDC_RESCANSKIN, IDC_RESCANSKIN, IDC_RELOADSKIN, 0 };

static INT_PTR CALLBACK DlgProcSkinOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BYTE loadMode;

	switch (msg) {
	case WM_INITDIALOG:
		RescanSkins(GetDlgItem(hwndDlg, IDC_SKINNAME));
		TranslateDialogDefault(hwndDlg);

		CheckDlgButton(hwndDlg, IDC_USESKIN, M.GetByte("useskin", 0) ? BST_CHECKED : BST_UNCHECKED);

		loadMode = M.GetByte("skin_loadmode", 0);
		CheckDlgButton(hwndDlg, IDC_SKIN_LOADFONTS, loadMode & THEME_READ_FONTS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SKIN_LOADTEMPLATES, loadMode & THEME_READ_TEMPLATES ? BST_CHECKED : BST_UNCHECKED);

		SendMessage(hwndDlg, WM_USER + 100, 0, 0);
		SetTimer(hwndDlg, 1000, 100, 0);
		return TRUE;

	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_SKIN_WARN)) {
			SetTextColor((HDC)wParam, RGB(255, 50, 50));
			return 0;
		}
		break;

		// self - configure the dialog, don't let the user load or unload
		// a skin while a message window is open. Show the warning that all
		// windows must be closed.
	case WM_USER + 100:
	{
		bool fWindowsOpen = (pFirstContainer != 0 ? true : false);
		for (int i = 0; _ctrls[i]; i++)
			Utils::enableDlgControl(hwndDlg, _ctrls[i], !fWindowsOpen);

		Utils::showDlgControl(hwndDlg, IDC_SKIN_WARN, fWindowsOpen ? SW_SHOW : SW_HIDE);
		Utils::showDlgControl(hwndDlg, IDC_SKIN_CLOSENOW, fWindowsOpen ? SW_SHOW : SW_HIDE);
	}
	return 0;

	case WM_TIMER:
		if (wParam == 1000)
			SendMessage(hwndDlg, WM_USER + 100, 0, 0);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_USESKIN:
			db_set_b(0, SRMSGMOD_T, "useskin", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_USESKIN) ? 1 : 0));
			break;

		case IDC_SKIN_LOADFONTS:
			loadMode = M.GetByte("skin_loadmode", 0);
			loadMode = IsDlgButtonChecked(hwndDlg, IDC_SKIN_LOADFONTS) ? loadMode | THEME_READ_FONTS : loadMode & ~THEME_READ_FONTS;
			db_set_b(0, SRMSGMOD_T, "skin_loadmode", loadMode);
			break;

		case IDC_SKIN_LOADTEMPLATES:
			loadMode = M.GetByte("skin_loadmode", 0);
			loadMode = IsDlgButtonChecked(hwndDlg, IDC_SKIN_LOADTEMPLATES) ? loadMode | THEME_READ_TEMPLATES : loadMode & ~THEME_READ_TEMPLATES;
			db_set_b(0, SRMSGMOD_T, "skin_loadmode", loadMode);
			break;

		case IDC_UNLOAD:
			Skin->Unload();
			SendMessage(hwndTabConfig, WM_USER + 100, 0, 0);
			break;

		case IDC_RELOADSKIN:
			Skin->setFileName();
			Skin->Load();
			SendMessage(hwndTabConfig, WM_USER + 100, 0, 0);
			break;

		case IDC_RESCANSKIN:
			FreeComboData(GetDlgItem(hwndDlg, IDC_SKINNAME));
			RescanSkins(GetDlgItem(hwndDlg, IDC_SKINNAME));
			break;

		case IDC_THEMEEXPORT:
		{
			const TCHAR *szFilename = GetThemeFileName(1);
			if (szFilename != NULL)
				WriteThemeToINI(szFilename, 0);
		}
		break;

		case IDC_THEMEIMPORT:
			if (CSkin::m_skinEnabled) {
				LRESULT r = CWarning::show(CWarning::WARN_THEME_OVERWRITE, MB_YESNOCANCEL | MB_ICONQUESTION);
				if (r == IDNO || r == IDCANCEL)
					return 0;
			}
			{
				LRESULT r = CWarning::show(CWarning::WARN_OPTION_CLOSE, MB_YESNOCANCEL | MB_ICONQUESTION);
				if (r == IDNO || r == IDCANCEL)
					return 0;

				const wchar_t*	szFilename = GetThemeFileName(0);
				DWORD dwFlags = THEME_READ_FONTS;

				if (szFilename != NULL) {
					int result = MessageBox(0, TranslateT("Do you want to also read message templates from the theme?\nCaution: This will overwrite the stored template set which may affect the look of your message window significantly.\nSelect Cancel to not load anything at all."),
						TranslateT("Load theme"), MB_YESNOCANCEL);
					if (result == IDCANCEL)
						return 1;
					if (result == IDYES)
						dwFlags |= THEME_READ_TEMPLATES;
					ReadThemeFromINI(szFilename, 0, 0, dwFlags);
					CacheLogFonts();
					CacheMsgLogIcons();
					PluginConfig.reloadSettings();
					CSkin::setAeroEffect(-1);
					M.BroadcastMessage(DM_OPTIONSAPPLIED, 1, 0);
					M.BroadcastMessage(DM_FORCEDREMAKELOG, 0, 0);
					SendMessage(GetParent(hwndDlg), WM_COMMAND, IDCANCEL, 0);
				}
			}
			break;

		case IDC_HELP_GENERAL:
			CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW, (LPARAM)"http://wiki.miranda.or.at/TabSRMM/UsingSkins");
			break;

		case IDC_SKIN_CLOSENOW:
			CloseAllContainers();
			break;

		case IDC_SKINNAME:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				LRESULT lr = SendDlgItemMessage(hwndDlg, IDC_SKINNAME, CB_GETCURSEL, 0, 0);
				if (lr != CB_ERR && lr > 0) {
					TCHAR	*tszRelPath = (TCHAR*)SendDlgItemMessage(hwndDlg, IDC_SKINNAME, CB_GETITEMDATA, lr, 0);
					if (tszRelPath && tszRelPath != (TCHAR*)CB_ERR)
						db_set_ts(0, SRMSGMOD_T, "ContainerSkin", tszRelPath);
					SendMessage(hwndDlg, WM_COMMAND, IDC_RELOADSKIN, 0);
				}
				else if (lr == 0) {		// selected the <no skin> entry
					db_unset(0, SRMSGMOD_T, "ContainerSkin");
					Skin->Unload();
					SendMessage(hwndTabConfig, WM_USER + 100, 0, 0);
				}
				return 0;
			}
			break;
		}

		if ((LOWORD(wParam) == IDC_SKINNAME) && (HIWORD(wParam) != CBN_SELCHANGE || (HWND)lParam != GetFocus()))
			return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				return TRUE;
			}
			break;
		}
		break;

	case WM_DESTROY:
		KillTimer(hwndDlg, 1000);
		FreeComboData(GetDlgItem(hwndDlg, IDC_SKINNAME));
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
void TreeViewInit(HWND hwndTree, UINT id, DWORD dwFlags, BOOL bFromMem)
{
	TVINSERTSTRUCT tvi = { 0 };
	TOptionListGroup *lvGroups = CTranslator::getGroupTree(id);
	TOptionListItem *lvItems = CTranslator::getTree(id);

	SetWindowLongPtr(hwndTree, GWL_STYLE, GetWindowLongPtr(hwndTree, GWL_STYLE) | (TVS_NOHSCROLL));
	/* Replace image list, destroy old. */
	ImageList_Destroy(TreeView_SetImageList(hwndTree, CreateStateImageList(), TVSIL_NORMAL));

	// fill the list box, create groups first, then add items
	for (int i = 0; lvGroups[i].szName != NULL; i++) {
		tvi.hParent = 0;
		tvi.hInsertAfter = TVI_LAST;
		tvi.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.item.pszText = TranslateTS(lvGroups[i].szName);
		tvi.item.stateMask = TVIS_EXPANDED | TVIS_BOLD;
		tvi.item.state = TVIS_EXPANDED | TVIS_BOLD;
		tvi.item.iImage = tvi.item.iSelectedImage = IMG_GRPOPEN;
		lvGroups[i].handle = (LRESULT)TreeView_InsertItem(hwndTree, &tvi);
	}

	for (int i = 0; lvItems[i].szName != NULL; i++) {
		tvi.hParent = (HTREEITEM)lvGroups[lvItems[i].uGroup].handle;
		tvi.hInsertAfter = TVI_LAST;
		tvi.item.pszText = TranslateTS(lvItems[i].szName);
		tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.item.lParam = i;
		if (bFromMem == FALSE) {
			switch (lvItems[i].uType) {
			case LOI_TYPE_FLAG:
				tvi.item.iImage = tvi.item.iSelectedImage = ((dwFlags & (UINT)lvItems[i].lParam) ? IMG_CHECK : IMG_NOCHECK);
				break;
			case LOI_TYPE_SETTING:
				tvi.item.iImage = tvi.item.iSelectedImage = (M.GetByte((char *)lvItems[i].lParam, lvItems[i].id) ? IMG_CHECK : IMG_NOCHECK);
				break;
			}
		}
		else {
			switch (lvItems[i].uType) {
			case LOI_TYPE_FLAG:
				tvi.item.iImage = tvi.item.iSelectedImage = (((*((UINT*)lvItems[i].lParam)) & lvItems[i].id) ? IMG_CHECK : IMG_NOCHECK);
				break;
			case LOI_TYPE_SETTING:
				tvi.item.iImage = tvi.item.iSelectedImage = ((*((BOOL*)lvItems[i].lParam)) ? IMG_CHECK : IMG_NOCHECK);
				break;
			}
		}
		lvItems[i].handle = (LRESULT)TreeView_InsertItem(hwndTree, &tvi);
	}

}

void TreeViewDestroy(HWND hwndTree)
{
	ImageList_Destroy(TreeView_GetImageList(hwndTree, TVSIL_NORMAL));
}

void TreeViewSetFromDB(HWND hwndTree, UINT id, DWORD dwFlags)
{
	TVITEM item = { 0 };
	TOptionListItem *lvItems = CTranslator::getTree(id);

	for (int i = 0; lvItems[i].szName != NULL; i++) {
		item.mask = TVIF_HANDLE | TVIF_IMAGE;
		item.hItem = (HTREEITEM)lvItems[i].handle;
		if (lvItems[i].uType == LOI_TYPE_FLAG)
			item.iImage = item.iSelectedImage = ((dwFlags & (UINT)lvItems[i].lParam) ? IMG_CHECK : IMG_NOCHECK);
		else if (lvItems[i].uType == LOI_TYPE_SETTING)
			item.iImage = item.iSelectedImage = (M.GetByte((char *)lvItems[i].lParam, lvItems[i].id) ? IMG_CHECK : IMG_NOCHECK);
		TreeView_SetItem(hwndTree, &item);
	}
}

void TreeViewToDB(HWND hwndTree, UINT id, char *DBPath, DWORD *dwFlags)
{
	TVITEM item = { 0 };
	TOptionListItem *lvItems = CTranslator::getTree(id);

	for (int i = 0; lvItems[i].szName != NULL; i++) {
		item.mask = TVIF_HANDLE | TVIF_IMAGE;
		item.hItem = (HTREEITEM)lvItems[i].handle;
		TreeView_GetItem(hwndTree, &item);

		switch (lvItems[i].uType) {
		case LOI_TYPE_FLAG:
			if (dwFlags != NULL)
				(*dwFlags) |= (item.iImage == IMG_CHECK) ? lvItems[i].lParam : 0;
			if (DBPath == NULL) {
				UINT *tm = (UINT*)lvItems[i].lParam;
				(*tm) = (item.iImage == IMG_CHECK) ? ((*tm) | lvItems[i].id) : ((*tm) & ~lvItems[i].id);
			}
			break;
		case LOI_TYPE_SETTING:
			if (DBPath != NULL) {
				db_set_b(0, DBPath, (char *)lvItems[i].lParam, (BYTE)((item.iImage == IMG_CHECK) ? 1 : 0));
			}
			else {
				(*((BOOL*)lvItems[i].lParam)) = ((item.iImage == IMG_CHECK) ? TRUE : FALSE);
			}
			break;
		}
	}
}

BOOL TreeViewHandleClick(HWND hwndDlg, HWND hwndTree, WPARAM, LPARAM lParam)
{
	TVITEM item = { 0 };
	TVHITTESTINFO hti;

	switch (((LPNMHDR)lParam)->code) {
	case TVN_KEYDOWN:
		if (((LPNMTVKEYDOWN)lParam)->wVKey != VK_SPACE)
			return FALSE;
		hti.flags = TVHT_ONITEMSTATEICON;
		item.hItem = TreeView_GetSelection(((LPNMHDR)lParam)->hwndFrom);
		break;
	case NM_CLICK:
		hti.pt.x = (short)LOWORD(GetMessagePos());
		hti.pt.y = (short)HIWORD(GetMessagePos());
		ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
		if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti) == 0)
			return FALSE;
		if ((hti.flags & TVHT_ONITEMICON) == 0)
			return FALSE;
		item.hItem = (HTREEITEM)hti.hItem;
		break;

	case TVN_ITEMEXPANDEDW:
		{
			LPNMTREEVIEWW lpnmtv = (LPNMTREEVIEWW)lParam;

			item.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			item.hItem = lpnmtv->itemNew.hItem;
			item.iImage = item.iSelectedImage =
				(lpnmtv->itemNew.state & TVIS_EXPANDED) ? IMG_GRPOPEN : IMG_GRPCLOSED;
			SendMessageW(((LPNMHDR)lParam)->hwndFrom, TVM_SETITEMW, 0, (LPARAM)&item);
		}
		return TRUE;

	default:
		return FALSE;
	}

	item.mask = TVIF_HANDLE | TVIF_IMAGE;
	item.stateMask = TVIS_BOLD;
	SendMessage(hwndTree, TVM_GETITEM, 0, (LPARAM)&item);
	item.mask |= TVIF_SELECTEDIMAGE;
	switch (item.iImage) {
	case IMG_NOCHECK:
		item.iImage = IMG_CHECK;
		break;
	case IMG_CHECK:
		item.iImage = IMG_NOCHECK;
		break;
	case IMG_GRPOPEN:
		item.mask |= TVIF_STATE;
		item.stateMask |= TVIS_EXPANDED;
		item.state = 0;
		item.iImage = IMG_GRPCLOSED;
		break;
	case IMG_GRPCLOSED:
		item.mask |= TVIF_STATE;
		item.stateMask |= TVIS_EXPANDED;
		item.state |= TVIS_EXPANDED;
		item.iImage = IMG_GRPOPEN;
		break;
	}
	item.iSelectedImage = item.iImage;
	SendMessage(hwndTree, TVM_SETITEM, 0, (LPARAM)&item);
	if (item.mask & TVIF_STATE) {
		RedrawWindow(hwndTree, NULL, NULL, RDW_INVALIDATE | RDW_NOFRAME | RDW_ERASENOW | RDW_ALLCHILDREN);
		InvalidateRect(hwndTree, NULL, TRUE);
	}
	else {
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
	}

	return TRUE;
}



static INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			TreeViewInit(GetDlgItem(hwndDlg, IDC_WINDOWOPTIONS), CTranslator::TREE_MSG, 0, FALSE);

			SetDlgItemInt(hwndDlg, IDC_MAXAVATARHEIGHT, M.GetDword("avatarheight", 100), FALSE);
			CheckDlgButton(hwndDlg, IDC_PRESERVEAVATARSIZE, M.GetByte("dontscaleavatars", 0) ? BST_CHECKED : BST_UNCHECKED);
			SendDlgItemMessage(hwndDlg, IDC_AVATARSPIN, UDM_SETRANGE, 0, MAKELONG(150, 0));

			BOOL translated;
			SendDlgItemMessage(hwndDlg, IDC_AVATARSPIN, UDM_SETPOS, 0, GetDlgItemInt(hwndDlg, IDC_MAXAVATARHEIGHT, &translated, FALSE));
		}
		return TRUE;

	case WM_DESTROY:
		TreeViewDestroy(GetDlgItem(hwndDlg, IDC_WINDOWOPTIONS));
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_MAXAVATARHEIGHT:
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
				return TRUE;
			break;

		case IDC_HELP_GENERAL:
			CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW, (LPARAM)"http://wiki.miranda.or.at/TabSRMM/GeneralSettings");
			break;

		case IDC_RESETWARNINGS:
			db_set_dw(0, SRMSGMOD_T, "cWarningsL", 0);
			db_set_dw(0, SRMSGMOD_T, "cWarningsH", 0);
			break;
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_WINDOWOPTIONS:
			return TreeViewHandleClick(hwndDlg, ((LPNMHDR)lParam)->hwndFrom, wParam, lParam);
			break;

		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				BOOL translated;
				db_set_dw(0, SRMSGMOD_T, "avatarheight", GetDlgItemInt(hwndDlg, IDC_MAXAVATARHEIGHT, &translated, FALSE));

				db_set_b(0, SRMSGMOD_T, "dontscaleavatars", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_PRESERVEAVATARSIZE) ? 1 : 0));

				// scan the tree view and obtain the options...
				TreeViewToDB(GetDlgItem(hwndDlg, IDC_WINDOWOPTIONS), CTranslator::TREE_MSG, SRMSGMOD_T, NULL);
				PluginConfig.reloadSettings();
				M.BroadcastMessage(DM_OPTIONSAPPLIED, 1, 0);
				return TRUE;
			}
			break;
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int have_ieview = 0, have_hpp = 0;

static UINT __ctrls[] = { IDC_INDENTSPIN, IDC_RINDENTSPIN, IDC_INDENTAMOUNT, IDC_RIGHTINDENT,
IDC_MODIFY, IDC_RTLMODIFY };

static INT_PTR CALLBACK DlgProcLogOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			DWORD maxhist = M.GetDword("maxhist", 0);
			DWORD dwFlags = M.GetDword("mwflags", MWF_LOG_DEFAULT);
			BOOL translated;

			switch (M.GetByte(SRMSGMOD, SRMSGSET_LOADHISTORY, SRMSGDEFSET_LOADHISTORY)) {
			case LOADHISTORY_UNREAD:
				CheckDlgButton(hwndDlg, IDC_LOADUNREAD, BST_CHECKED);
				break;
			case LOADHISTORY_COUNT:
				CheckDlgButton(hwndDlg, IDC_LOADCOUNT, BST_CHECKED);
				Utils::enableDlgControl(hwndDlg, IDC_LOADCOUNTN, true);
				Utils::enableDlgControl(hwndDlg, IDC_LOADCOUNTSPIN, true);
				break;
			case LOADHISTORY_TIME:
				CheckDlgButton(hwndDlg, IDC_LOADTIME, BST_CHECKED);
				Utils::enableDlgControl(hwndDlg, IDC_LOADTIMEN, true);
				Utils::enableDlgControl(hwndDlg, IDC_LOADTIMESPIN, true);
				Utils::enableDlgControl(hwndDlg, IDC_STMINSOLD, true);
				break;
			}

			TreeViewInit(GetDlgItem(hwndDlg, IDC_LOGOPTIONS), CTranslator::TREE_LOG, dwFlags, FALSE);

			SendDlgItemMessage(hwndDlg, IDC_LOADCOUNTSPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
			SendDlgItemMessage(hwndDlg, IDC_LOADCOUNTSPIN, UDM_SETPOS, 0, db_get_w(NULL, SRMSGMOD, SRMSGSET_LOADCOUNT, SRMSGDEFSET_LOADCOUNT));
			SendDlgItemMessage(hwndDlg, IDC_LOADTIMESPIN, UDM_SETRANGE, 0, MAKELONG(24 * 60, 0));
			SendDlgItemMessage(hwndDlg, IDC_LOADTIMESPIN, UDM_SETPOS, 0, db_get_w(NULL, SRMSGMOD, SRMSGSET_LOADTIME, SRMSGDEFSET_LOADTIME));

			SetDlgItemInt(hwndDlg, IDC_INDENTAMOUNT, M.GetDword("IndentAmount", 20), FALSE);
			SendDlgItemMessage(hwndDlg, IDC_INDENTSPIN, UDM_SETRANGE, 0, MAKELONG(1000, 0));
			SendDlgItemMessage(hwndDlg, IDC_INDENTSPIN, UDM_SETPOS, 0, GetDlgItemInt(hwndDlg, IDC_INDENTAMOUNT, &translated, FALSE));

			SetDlgItemInt(hwndDlg, IDC_RIGHTINDENT, M.GetDword("RightIndent", 20), FALSE);
			SendDlgItemMessage(hwndDlg, IDC_RINDENTSPIN, UDM_SETRANGE, 0, MAKELONG(1000, 0));
			SendDlgItemMessage(hwndDlg, IDC_RINDENTSPIN, UDM_SETPOS, 0, GetDlgItemInt(hwndDlg, IDC_RIGHTINDENT, &translated, FALSE));
			SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_INDENT, 0), 0);

			SendDlgItemMessage(hwndDlg, IDC_TRIMSPIN, UDM_SETRANGE, 0, MAKELONG(1000, 5));
			SendDlgItemMessage(hwndDlg, IDC_TRIMSPIN, UDM_SETPOS, 0, maxhist);
			Utils::enableDlgControl(hwndDlg, IDC_TRIMSPIN, maxhist != 0);
			Utils::enableDlgControl(hwndDlg, IDC_TRIM, maxhist != 0);
			CheckDlgButton(hwndDlg, IDC_ALWAYSTRIM, maxhist != 0);

			have_ieview = ServiceExists(MS_IEVIEW_WINDOW);
			have_hpp = ServiceExists("History++/ExtGrid/NewWindow");

			SendDlgItemMessage(hwndDlg, IDC_MSGLOGDIDSPLAY, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Internal message log"));
			SendDlgItemMessage(hwndDlg, IDC_MSGLOGDIDSPLAY, CB_SETCURSEL, 0, 0);
			if (have_ieview || have_hpp) {
				if (have_ieview) {
					SendDlgItemMessage(hwndDlg, IDC_MSGLOGDIDSPLAY, CB_INSERTSTRING, -1, (LPARAM)TranslateT("IEView plugin"));
					if (M.GetByte("default_ieview", 0))
						SendDlgItemMessage(hwndDlg, IDC_MSGLOGDIDSPLAY, CB_SETCURSEL, 1, 0);
				}
				if (have_hpp) {
					SendDlgItemMessage(hwndDlg, IDC_MSGLOGDIDSPLAY, CB_INSERTSTRING, -1, (LPARAM)TranslateT("History++ plugin"));
					if (M.GetByte("default_ieview", 0))
						SendDlgItemMessage(hwndDlg, IDC_MSGLOGDIDSPLAY, CB_SETCURSEL, 1, 0);
					else if (M.GetByte("default_hpp", 0))
						SendDlgItemMessage(hwndDlg, IDC_MSGLOGDIDSPLAY, CB_SETCURSEL, have_ieview ? 2 : 1, 0);
				}
			}
			else EnableWindow(GetDlgItem(hwndDlg, IDC_MSGLOGDIDSPLAY), FALSE);

			SetDlgItemText(hwndDlg, IDC_EXPLAINMSGLOGSETTINGS, TranslateT("You have chosen to use an external plugin for displaying the message history in the chat window. Most of the settings on this page are for the standard message log viewer only and will have no effect. To change the appearance of the message log, you must configure either IEView or History++."));
			SendMessage(hwndDlg, WM_USER + 100, 0, 0);
		}
		return TRUE;

	case WM_DESTROY:
		TreeViewDestroy(GetDlgItem(hwndDlg, IDC_LOGOPTIONS));
		break;

		// configure the option page - hide most of the settings here when either IEView
		// or H++ is set as the global message log viewer. Showing these options may confuse
		// the user, because they are not working and the user needs to configure the 3rd
		// party plugin.
	case WM_USER + 100:
	{
		LRESULT r = SendDlgItemMessage(hwndDlg, IDC_MSGLOGDIDSPLAY, CB_GETCURSEL, 0, 0);
		Utils::showDlgControl(hwndDlg, IDC_EXPLAINMSGLOGSETTINGS, r == 0 ? SW_HIDE : SW_SHOW);
		Utils::showDlgControl(hwndDlg, IDC_LOGOPTIONS, r == 0 ? SW_SHOW : SW_HIDE);
		Utils::enableDlgControl(GetDlgItem(hwndDlg, IDC_MSGLOGDIDSPLAY), r != 0);
		for (int i = 0; i < SIZEOF(__ctrls); i++)
			Utils::enableDlgControl(hwndDlg, __ctrls[i], r == 0 ? TRUE : FALSE);
	}
	return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ALWAYSTRIM:
			Utils::enableDlgControl(hwndDlg, IDC_TRIMSPIN, IsDlgButtonChecked(hwndDlg, IDC_ALWAYSTRIM) != 0);
			Utils::enableDlgControl(hwndDlg, IDC_TRIM, IsDlgButtonChecked(hwndDlg, IDC_ALWAYSTRIM) != 0);
			break;
		case IDC_LOADUNREAD:
		case IDC_LOADCOUNT:
		case IDC_LOADTIME:
			Utils::enableDlgControl(hwndDlg, IDC_LOADCOUNTN, IsDlgButtonChecked(hwndDlg, IDC_LOADCOUNT) != 0);
			Utils::enableDlgControl(hwndDlg, IDC_LOADCOUNTSPIN, IsDlgButtonChecked(hwndDlg, IDC_LOADCOUNT) != 0);
			Utils::enableDlgControl(hwndDlg, IDC_LOADTIMEN, IsDlgButtonChecked(hwndDlg, IDC_LOADTIME) != 0);
			Utils::enableDlgControl(hwndDlg, IDC_LOADTIMESPIN, IsDlgButtonChecked(hwndDlg, IDC_LOADTIME) != 0);
			Utils::enableDlgControl(hwndDlg, IDC_STMINSOLD, IsDlgButtonChecked(hwndDlg, IDC_LOADTIME) != 0);
			break;
		case IDC_INDENTAMOUNT:
		case IDC_LOADCOUNTN:
		case IDC_LOADTIMEN:
		case IDC_RIGHTINDENT:
		case IDC_TRIM:
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
				return TRUE;
			break;
		case IDC_MODIFY:
		{
			TemplateEditorNew teNew = { 0, 0, hwndDlg };
			CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_TEMPLATEEDIT), hwndDlg, DlgProcTemplateEditor, (LPARAM)&teNew);
		}
		break;
		case IDC_RTLMODIFY:
		{
			TemplateEditorNew teNew = { 0, TRUE, hwndDlg };
			CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_TEMPLATEEDIT), hwndDlg, DlgProcTemplateEditor, (LPARAM)&teNew);
		}
		break;
		case IDC_MSGLOGDIDSPLAY:
			SendMessage(hwndDlg, WM_USER + 100, 0, 0);
			break;
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_LOGOPTIONS:
			return TreeViewHandleClick(hwndDlg, ((LPNMHDR)lParam)->hwndFrom, wParam, lParam);
			break;

		default:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				LRESULT msglogmode = SendDlgItemMessage(hwndDlg, IDC_MSGLOGDIDSPLAY, CB_GETCURSEL, 0, 0);
				DWORD dwFlags = M.GetDword("mwflags", MWF_LOG_DEFAULT);
				BOOL translated;

				dwFlags &= ~(MWF_LOG_ALL);

				if (IsDlgButtonChecked(hwndDlg, IDC_LOADCOUNT))
					db_set_b(0, SRMSGMOD, SRMSGSET_LOADHISTORY, LOADHISTORY_COUNT);
				else if (IsDlgButtonChecked(hwndDlg, IDC_LOADTIME))
					db_set_b(0, SRMSGMOD, SRMSGSET_LOADHISTORY, LOADHISTORY_TIME);
				else
					db_set_b(0, SRMSGMOD, SRMSGSET_LOADHISTORY, LOADHISTORY_UNREAD);
				db_set_w(NULL, SRMSGMOD, SRMSGSET_LOADCOUNT, (WORD)SendDlgItemMessage(hwndDlg, IDC_LOADCOUNTSPIN, UDM_GETPOS, 0, 0));
				db_set_w(NULL, SRMSGMOD, SRMSGSET_LOADTIME, (WORD)SendDlgItemMessage(hwndDlg, IDC_LOADTIMESPIN, UDM_GETPOS, 0, 0));

				db_set_dw(0, SRMSGMOD_T, "IndentAmount", (DWORD)GetDlgItemInt(hwndDlg, IDC_INDENTAMOUNT, &translated, FALSE));
				db_set_dw(0, SRMSGMOD_T, "RightIndent", (DWORD)GetDlgItemInt(hwndDlg, IDC_RIGHTINDENT, &translated, FALSE));

				db_set_b(0, SRMSGMOD_T, "default_ieview", 0);
				db_set_b(0, SRMSGMOD_T, "default_hpp", 0);
				switch (msglogmode) {
				case 0:
					break;
				case 1:
					if (have_ieview)
						db_set_b(0, SRMSGMOD_T, "default_ieview", 1);
					else
						db_set_b(0, SRMSGMOD_T, "default_hpp", 1);
					break;
				case 2:
					db_set_b(0, SRMSGMOD_T, "default_hpp", 1);
					break;
				}

				// scan the tree view and obtain the options...
				TreeViewToDB(GetDlgItem(hwndDlg, IDC_LOGOPTIONS), CTranslator::TREE_LOG, SRMSGMOD_T, &dwFlags);
				db_set_dw(0, SRMSGMOD_T, "mwflags", dwFlags);
				if (IsDlgButtonChecked(hwndDlg, IDC_ALWAYSTRIM))
					db_set_dw(0, SRMSGMOD_T, "maxhist", (DWORD)SendDlgItemMessage(hwndDlg, IDC_TRIMSPIN, UDM_GETPOS, 0, 0));
				else
					db_set_dw(0, SRMSGMOD_T, "maxhist", 0);
				PluginConfig.reloadSettings();
				M.BroadcastMessage(DM_OPTIONSAPPLIED, 1, 0);
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// typing notify options

static void ResetCList(HWND hwndDlg)
{
	if (CallService(MS_CLUI_GETCAPS, 0, 0) & CLUIF_DISABLEGROUPS && !M.GetByte("CList", "UseGroups", SETTING_USEGROUPS_DEFAULT))
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETUSEGROUPS, FALSE, 0);
	else
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETUSEGROUPS, TRUE, 0);
	SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETHIDEEMPTYGROUPS, 1, 0);
}

static void RebuildList(HWND hwndDlg, HANDLE hItemNew, HANDLE hItemUnknown)
{
	BYTE defType = M.GetByte(SRMSGMOD, SRMSGSET_TYPINGNEW, SRMSGDEFSET_TYPINGNEW);
	if (hItemNew && defType)
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItemNew, 1);

	if (hItemUnknown && M.GetByte(SRMSGMOD, SRMSGSET_TYPINGUNKNOWN, SRMSGDEFSET_TYPINGUNKNOWN))
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItemUnknown, 1);

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
		if (hItem && db_get_b(hContact, SRMSGMOD, SRMSGSET_TYPING, defType))
			SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItem, 1);
	}
}

static void SaveList(HWND hwndDlg, HANDLE hItemNew, HANDLE hItemUnknown)
{
	if (hItemNew)
		db_set_b(0, SRMSGMOD, SRMSGSET_TYPINGNEW, (BYTE)(SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItemNew, 0) ? 1 : 0));

	if (hItemUnknown)
		db_set_b(0, SRMSGMOD, SRMSGSET_TYPINGUNKNOWN, (BYTE)(SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItemUnknown, 0) ? 1 : 0));

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
		if (hItem)
			db_set_b(hContact, SRMSGMOD, SRMSGSET_TYPING, (BYTE)(SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItem, 0) ? 1 : 0));
	}
}

static INT_PTR CALLBACK DlgProcTypeOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HANDLE hItemNew, hItemUnknown;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			CLCINFOITEM cii = { sizeof(cii) };
			cii.flags = CLCIIF_GROUPFONT | CLCIIF_CHECKBOX;
			cii.pszText = TranslateT("** New contacts **");
			hItemNew = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
			cii.pszText = TranslateT("** Unknown contacts **");
			hItemUnknown = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
		}
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CLIST), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CLIST), GWL_STYLE) | (CLS_SHOWHIDDEN));
		ResetCList(hwndDlg);
		RebuildList(hwndDlg, hItemNew, hItemUnknown);

		CheckDlgButton(hwndDlg, IDC_SHOWNOTIFY, M.GetByte(SRMSGMOD, SRMSGSET_SHOWTYPING, SRMSGDEFSET_SHOWTYPING) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_TYPEFLASHWIN, M.GetByte(SRMSGMOD, SRMSGSET_SHOWTYPINGWINFLASH, SRMSGDEFSET_SHOWTYPINGWINFLASH) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_TYPENOWIN, M.GetByte(SRMSGMOD, SRMSGSET_SHOWTYPINGNOWINOPEN, 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_TYPEWIN, M.GetByte(SRMSGMOD, SRMSGSET_SHOWTYPINGWINOPEN, 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_NOTIFYTRAY, M.GetByte(SRMSGMOD, SRMSGSET_SHOWTYPINGCLIST, SRMSGDEFSET_SHOWTYPINGCLIST) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_NOTIFYBALLOON, M.GetByte(SRMSGMOD, "ShowTypingBalloon", 0));

		CheckDlgButton(hwndDlg, IDC_NOTIFYPOPUP, M.GetByte(SRMSGMOD, "ShowTypingPopup", 0) ? BST_CHECKED : BST_UNCHECKED);

		Utils::enableDlgControl(hwndDlg, IDC_TYPEWIN, IsDlgButtonChecked(hwndDlg, IDC_NOTIFYTRAY) != 0);
		Utils::enableDlgControl(hwndDlg, IDC_TYPENOWIN, IsDlgButtonChecked(hwndDlg, IDC_NOTIFYTRAY) != 0);
		Utils::enableDlgControl(hwndDlg, IDC_NOTIFYBALLOON, IsDlgButtonChecked(hwndDlg, IDC_NOTIFYTRAY) &&
			(IsDlgButtonChecked(hwndDlg, IDC_TYPEWIN) || IsDlgButtonChecked(hwndDlg, IDC_TYPENOWIN)));

		Utils::enableDlgControl(hwndDlg, IDC_TYPEFLASHWIN, IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY) != 0);
		Utils::enableDlgControl(hwndDlg, IDC_MTN_POPUPMODE, IsDlgButtonChecked(hwndDlg, IDC_NOTIFYPOPUP) != 0);

		if (!ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) {
			Utils::enableDlgControl(hwndDlg, IDC_NOTIFYBALLOON, false);
			SetDlgItemText(hwndDlg, IDC_NOTIFYBALLOON, TranslateT("Show balloon popup (unsupported system)"));
		}

		SendDlgItemMessage(hwndDlg, IDC_MTN_POPUPMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Always"));
		SendDlgItemMessage(hwndDlg, IDC_MTN_POPUPMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Always, but no popup when window is focused"));
		SendDlgItemMessage(hwndDlg, IDC_MTN_POPUPMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Only when no message window is open"));

		SendDlgItemMessage(hwndDlg, IDC_MTN_POPUPMODE, CB_SETCURSEL, (WPARAM)M.GetByte("MTN_PopupMode", 0), 0);

		if (!PluginConfig.g_bPopupAvail) {
			Utils::showDlgControl(hwndDlg, IDC_NOTIFYPOPUP, SW_HIDE);
			Utils::showDlgControl(hwndDlg, IDC_STATIC111, SW_HIDE);
			Utils::showDlgControl(hwndDlg, IDC_MTN_POPUPMODE, SW_HIDE);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_NOTIFYTRAY:
			Utils::enableDlgControl(hwndDlg, IDC_TYPEWIN, IsDlgButtonChecked(hwndDlg, IDC_NOTIFYTRAY) != 0);
			Utils::enableDlgControl(hwndDlg, IDC_TYPENOWIN, IsDlgButtonChecked(hwndDlg, IDC_NOTIFYTRAY) != 0);
			Utils::enableDlgControl(hwndDlg, IDC_NOTIFYBALLOON, IsDlgButtonChecked(hwndDlg, IDC_NOTIFYTRAY) != 0);
			break;
		case IDC_SHOWNOTIFY:
			Utils::enableDlgControl(hwndDlg, IDC_TYPEFLASHWIN, IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY) != 0);
			break;
		case IDC_NOTIFYPOPUP:
			Utils::enableDlgControl(hwndDlg, IDC_MTN_POPUPMODE, IsDlgButtonChecked(hwndDlg, IDC_NOTIFYPOPUP) != 0);
			break;
		case IDC_TYPEWIN:
		case IDC_TYPENOWIN:
			Utils::enableDlgControl(hwndDlg, IDC_NOTIFYBALLOON, IsDlgButtonChecked(hwndDlg, IDC_NOTIFYTRAY) &&
				(IsDlgButtonChecked(hwndDlg, IDC_TYPEWIN) || IsDlgButtonChecked(hwndDlg, IDC_TYPENOWIN)));
			break;
		case IDC_MTN_HELP:
			CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW, (LPARAM)"http://wiki.miranda.or.at/TabSRMM/TypingNotifications");
			return 0;
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((NMHDR*)lParam)->idFrom) {
		case IDC_CLIST:
			switch (((NMHDR*)lParam)->code) {
			case CLN_OPTIONSCHANGED:
				ResetCList(hwndDlg);
				break;
			case CLN_CHECKCHANGED:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
			break;
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				SaveList(hwndDlg, hItemNew, hItemUnknown);
				db_set_b(0, SRMSGMOD, SRMSGSET_SHOWTYPING, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY));
				db_set_b(0, SRMSGMOD, SRMSGSET_SHOWTYPINGWINFLASH, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_TYPEFLASHWIN));
				db_set_b(0, SRMSGMOD, SRMSGSET_SHOWTYPINGNOWINOPEN, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_TYPENOWIN));
				db_set_b(0, SRMSGMOD, SRMSGSET_SHOWTYPINGWINOPEN, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_TYPEWIN));
				db_set_b(0, SRMSGMOD, SRMSGSET_SHOWTYPINGCLIST, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_NOTIFYTRAY));
				db_set_b(0, SRMSGMOD, "ShowTypingBalloon", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_NOTIFYBALLOON));
				db_set_b(0, SRMSGMOD, "ShowTypingPopup", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_NOTIFYPOPUP));
				db_set_b(0, SRMSGMOD_T, "MTN_PopupMode", (BYTE)SendDlgItemMessage(hwndDlg, IDC_MTN_POPUPMODE, CB_GETCURSEL, 0, 0));
				PluginConfig.reloadSettings();
			}
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// options for tabbed messaging got their own page.. finally :)

static INT_PTR CALLBACK DlgProcTabbedOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		TreeViewInit(GetDlgItem(hwndDlg, IDC_TABMSGOPTIONS), CTranslator::TREE_TAB, 0, FALSE);

		CheckDlgButton(hwndDlg, IDC_CUT_TABTITLE, M.GetByte("cuttitle", 0) ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_CUT_TITLEMAXSPIN, UDM_SETRANGE, 0, MAKELONG(20, 5));
		SendDlgItemMessage(hwndDlg, IDC_CUT_TITLEMAXSPIN, UDM_SETPOS, 0, (WPARAM)db_get_w(NULL, SRMSGMOD_T, "cut_at", 15));

		Utils::enableDlgControl(hwndDlg, IDC_CUT_TITLEMAX, IsDlgButtonChecked(hwndDlg, IDC_CUT_TABTITLE) != 0);
		Utils::enableDlgControl(hwndDlg, IDC_CUT_TITLEMAXSPIN, IsDlgButtonChecked(hwndDlg, IDC_CUT_TABTITLE) != 0);

		SendDlgItemMessage(hwndDlg, IDC_ESCMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Normal - close tab, if last tab is closed also close the window"));
		SendDlgItemMessage(hwndDlg, IDC_ESCMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Minimize the window to the task bar"));
		SendDlgItemMessage(hwndDlg, IDC_ESCMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Close or hide window, depends on the close button setting above"));
		SendDlgItemMessage(hwndDlg, IDC_ESCMODE, CB_SETCURSEL, (WPARAM)PluginConfig.m_EscapeCloses, 0);
		break;

	case WM_DESTROY:
		TreeViewDestroy(GetDlgItem(hwndDlg, IDC_TABMSGOPTIONS));
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CUT_TITLEMAX:
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
				return TRUE;
			break;
		case IDC_SETUPAUTOCREATEMODES: {
			HWND hwndNew = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_CHOOSESTATUSMODES), hwndDlg, DlgProcSetupStatusModes, M.GetDword("autopopupmask", -1));
			SendMessage(hwndNew, DM_SETPARENTDIALOG, 0, (LPARAM)hwndDlg);
			break;
		}
		case IDC_CUT_TABTITLE:
			Utils::enableDlgControl(hwndDlg, IDC_CUT_TITLEMAX, IsDlgButtonChecked(hwndDlg, IDC_CUT_TABTITLE) != 0);
			Utils::enableDlgControl(hwndDlg, IDC_CUT_TITLEMAXSPIN, IsDlgButtonChecked(hwndDlg, IDC_CUT_TABTITLE) != 0);
			break;
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case DM_STATUSMASKSET:
		db_set_dw(0, SRMSGMOD_T, "autopopupmask", (DWORD)lParam);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_TABMSGOPTIONS:
			return TreeViewHandleClick(hwndDlg, ((LPNMHDR)lParam)->hwndFrom, wParam, lParam);
			break;

		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				db_set_b(0, SRMSGMOD_T, "cuttitle", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CUT_TABTITLE));
				db_set_w(NULL, SRMSGMOD_T, "cut_at", (WORD)SendDlgItemMessage(hwndDlg, IDC_CUT_TITLEMAXSPIN, UDM_GETPOS, 0, 0));

				// scan the tree view and obtain the options...
				TreeViewToDB(GetDlgItem(hwndDlg, IDC_TABMSGOPTIONS), CTranslator::TREE_TAB, SRMSGMOD_T, NULL);

				PluginConfig.m_EscapeCloses = (int)SendDlgItemMessage(hwndDlg, IDC_ESCMODE, CB_GETCURSEL, 0, 0);
				db_set_b(0, SRMSGMOD_T, "escmode", (BYTE)PluginConfig.m_EscapeCloses);
				PluginConfig.reloadSettings();
				M.BroadcastMessage(DM_OPTIONSAPPLIED, 0, 0);
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// container options

static INT_PTR CALLBACK DlgProcContainerSettings(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_CONTAINERGROUPMODE, M.GetByte("useclistgroups", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LIMITTABS, M.GetByte("limittabs", 0) ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg, IDC_TABLIMITSPIN, UDM_SETRANGE, 0, MAKELONG(1000, 1));
		SendDlgItemMessage(hwndDlg, IDC_TABLIMITSPIN, UDM_SETPOS, 0, (int)M.GetDword("maxtabs", 1));
		SetDlgItemInt(hwndDlg, IDC_TABLIMIT, (int)M.GetDword("maxtabs", 1), FALSE);
		Utils::enableDlgControl(hwndDlg, IDC_TABLIMIT, IsDlgButtonChecked(hwndDlg, IDC_LIMITTABS) != 0);
		CheckDlgButton(hwndDlg, IDC_SINGLEWINDOWMODE, M.GetByte("singlewinmode", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DEFAULTCONTAINERMODE, !(IsDlgButtonChecked(hwndDlg, IDC_CONTAINERGROUPMODE) || IsDlgButtonChecked(hwndDlg, IDC_LIMITTABS) || IsDlgButtonChecked(hwndDlg, IDC_SINGLEWINDOWMODE)) ? BST_CHECKED : BST_UNCHECKED);

		SetDlgItemInt(hwndDlg, IDC_NRFLASH, M.GetByte("nrflash", 4), FALSE);
		SendDlgItemMessage(hwndDlg, IDC_NRFLASHSPIN, UDM_SETRANGE, 0, MAKELONG(255, 1));
		SendDlgItemMessage(hwndDlg, IDC_NRFLASHSPIN, UDM_SETPOS, 0, (int)M.GetByte("nrflash", 4));

		SetDlgItemInt(hwndDlg, IDC_FLASHINTERVAL, M.GetDword("flashinterval", 1000), FALSE);
		SendDlgItemMessage(hwndDlg, IDC_FLASHINTERVALSPIN, UDM_SETRANGE, 0, MAKELONG(10000, 500));
		SendDlgItemMessage(hwndDlg, IDC_FLASHINTERVALSPIN, UDM_SETPOS, 0, (int)M.GetDword("flashinterval", 1000));
		SendDlgItemMessage(hwndDlg, IDC_FLASHINTERVALSPIN, UDM_SETACCEL, 0, (int)M.GetDword("flashinterval", 1000));
		CheckDlgButton(hwndDlg, IDC_USEAERO, M.GetByte("useAero", 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_USEAEROPEEK, M.GetByte("useAeroPeek", 1) ? BST_CHECKED : BST_UNCHECKED);
		for (int i = 0; i < CSkin::AERO_EFFECT_LAST; i++)
			SendDlgItemMessage(hwndDlg, IDC_AEROEFFECT, CB_INSERTSTRING, -1, (LPARAM)TranslateTS(CSkin::m_aeroEffects[i].tszName));

		SendDlgItemMessage(hwndDlg, IDC_AEROEFFECT, CB_SETCURSEL, (WPARAM)CSkin::m_aeroEffect, 0);
		Utils::enableDlgControl(hwndDlg, IDC_AEROEFFECT, PluginConfig.m_bIsVista);
		Utils::enableDlgControl(hwndDlg, IDC_USEAERO, PluginConfig.m_bIsVista);
		Utils::enableDlgControl(hwndDlg, IDC_USEAEROPEEK, PluginConfig.m_bIsWin7);
		if (PluginConfig.m_bIsVista)
			Utils::enableDlgControl(hwndDlg, IDC_AEROEFFECT, IsDlgButtonChecked(hwndDlg, IDC_USEAERO) != 0);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_TABLIMIT:
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
				return TRUE;
			break;
		case IDC_USEAERO:
			Utils::enableDlgControl(hwndDlg, IDC_AEROEFFECT, IsDlgButtonChecked(hwndDlg, IDC_USEAERO) != 0);
			break;
		case IDC_LIMITTABS:
		case IDC_SINGLEWINDOWMODE:
		case IDC_CONTAINERGROUPMODE:
		case IDC_DEFAULTCONTAINERMODE:
			Utils::enableDlgControl(hwndDlg, IDC_TABLIMIT, IsDlgButtonChecked(hwndDlg, IDC_LIMITTABS) != 0);
			break;
		case IDC_HELP_CONTAINERS:
			CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW, (LPARAM)"http://wiki.miranda.or.at/TabSRMM/Containers");
			break;
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				BOOL translated;
				bool	fOldAeroState = M.getAeroState();
				db_set_b(0, SRMSGMOD_T, "useclistgroups", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_CONTAINERGROUPMODE)));
				db_set_b(0, SRMSGMOD_T, "limittabs", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_LIMITTABS)));
				db_set_dw(0, SRMSGMOD_T, "maxtabs", GetDlgItemInt(hwndDlg, IDC_TABLIMIT, &translated, FALSE));
				db_set_b(0, SRMSGMOD_T, "singlewinmode", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_SINGLEWINDOWMODE)));
				db_set_dw(0, SRMSGMOD_T, "flashinterval", GetDlgItemInt(hwndDlg, IDC_FLASHINTERVAL, &translated, FALSE));
				db_set_b(0, SRMSGMOD_T, "nrflash", (BYTE)(GetDlgItemInt(hwndDlg, IDC_NRFLASH, &translated, FALSE)));
				db_set_b(0, SRMSGMOD_T, "useAero", IsDlgButtonChecked(hwndDlg, IDC_USEAERO) ? 1 : 0);
				db_set_b(0, SRMSGMOD_T, "useAeroPeek", IsDlgButtonChecked(hwndDlg, IDC_USEAEROPEEK) ? 1 : 0);
				CSkin::setAeroEffect(SendDlgItemMessage(hwndDlg, IDC_AEROEFFECT, CB_GETCURSEL, 0, 0));

				if (M.getAeroState() != fOldAeroState) {
					SendMessage(PluginConfig.g_hwndHotkeyHandler, WM_DWMCOMPOSITIONCHANGED, 0, 0);	// simulate aero state change
					SendMessage(PluginConfig.g_hwndHotkeyHandler, WM_DWMCOLORIZATIONCOLORCHANGED, 0, 0);	// simulate aero state change
				}
				BuildContainerMenu();
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}


INT_PTR CALLBACK PlusOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		TreeViewInit(GetDlgItem(hwndDlg, IDC_PLUS_CHECKTREE), CTranslator::TREE_MODPLUS, 0, FALSE);

		SendDlgItemMessage(hwndDlg, IDC_TIMEOUTSPIN, UDM_SETRANGE, 0, MAKELONG(300, SRMSGSET_MSGTIMEOUT_MIN / 1000));
		SendDlgItemMessage(hwndDlg, IDC_TIMEOUTSPIN, UDM_SETPOS, 0, PluginConfig.m_MsgTimeout / 1000);

		SendDlgItemMessage(hwndDlg, IDC_HISTORYSIZESPIN, UDM_SETRANGE, 0, MAKELONG(255, 15));
		SendDlgItemMessage(hwndDlg, IDC_HISTORYSIZESPIN, UDM_SETPOS, 0, (int)M.GetByte("historysize", 0));
		return TRUE;

	case WM_DESTROY:
		TreeViewDestroy(GetDlgItem(hwndDlg, IDC_PLUS_CHECKTREE));
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_PLUS_CHECKTREE:
			return TreeViewHandleClick(hwndDlg, ((LPNMHDR)lParam)->hwndFrom, wParam, lParam);
			break;

		default:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				TreeViewToDB(GetDlgItem(hwndDlg, IDC_PLUS_CHECKTREE), CTranslator::TREE_MODPLUS, SRMSGMOD_T, NULL);

				int msgTimeout = 1000 * GetDlgItemInt(hwndDlg, IDC_SECONDS, NULL, FALSE);
				PluginConfig.m_MsgTimeout = msgTimeout >= SRMSGSET_MSGTIMEOUT_MIN ? msgTimeout : SRMSGSET_MSGTIMEOUT_MIN;
				db_set_dw(0, SRMSGMOD, SRMSGSET_MSGTIMEOUT, PluginConfig.m_MsgTimeout);

				db_set_b(0, SRMSGMOD_T, "historysize", (BYTE)SendDlgItemMessage(hwndDlg, IDC_HISTORYSIZESPIN, UDM_GETPOS, 0, 0));
				PluginConfig.reloadAdv();
				return TRUE;
			}
		}
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_PLUS_HELP) {
			CallService(MS_UTILS_OPENURL, 0, (LPARAM)"http://wiki.miranda.or.at/TabSRMM/AdvancedTweaks");
			break;
		}
		else if (LOWORD(wParam) == IDC_PLUS_REVERT) {		// revert to defaults...
			TOptionListItem *lvItems = CTranslator::getTree(CTranslator::TREE_MODPLUS);

			for (int i = 0; lvItems[i].szName != NULL; i++)
				if (lvItems[i].uType == LOI_TYPE_SETTING)
					db_set_b(0, SRMSGMOD_T, (char *)lvItems[i].lParam, (BYTE)lvItems[i].id);
			TreeViewSetFromDB(GetDlgItem(hwndDlg, IDC_PLUS_CHECKTREE), CTranslator::TREE_MODPLUS, 0);
			break;
		}
		if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
			return TRUE;

		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		return 0;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

#define DBFONTF_BOLD       1
#define DBFONTF_ITALIC     2
#define DBFONTF_UNDERLINE  4

#define FONTS_TO_CONFIG MSGDLGFONTCOUNT

#define SAMEASF_FACE   1
#define SAMEASF_SIZE   2
#define SAMEASF_STYLE  4
#define SAMEASF_COLOUR 8
#include <pshpack1.h>

struct {
	BYTE sameAsFlags, sameAs;
	COLORREF colour;
	char size;
	BYTE style;
	BYTE charset;
	char szFace[LF_FACESIZE];
} static fontSettings[MSGDLGFONTCOUNT + 1];

#include <poppack.h>

#define SRFONTSETTINGMODULE FONTMODULE

static int OptInitialise(WPARAM wParam, LPARAM lParam)
{
	if (PluginConfig.g_bPopupAvail)
		TN_OptionsInitialize(wParam, lParam);

	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 910000000;
	odp.hInstance = g_hInst;
	odp.pszTitle = LPGEN("Message sessions");
	odp.flags = ODPF_BOLDGROUPS;

	odp.pszTab = LPGEN("General");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MSGDLG);
	odp.pfnDlgProc = DlgProcOptions;
	Options_AddPage(wParam, &odp);

	odp.pszTab = LPGEN("Tabs and layout");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_TABBEDMSG);
	odp.pfnDlgProc = DlgProcTabbedOptions;
	Options_AddPage(wParam, &odp);

	odp.pszTab = LPGEN("Containers");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CONTAINERS);
	odp.pfnDlgProc = DlgProcContainerSettings;
	Options_AddPage(wParam, &odp);

	odp.pszTab = LPGEN("Message log");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MSGLOG);
	odp.pfnDlgProc = DlgProcLogOptions;
	Options_AddPage(wParam, &odp);

	odp.pszTab = LPGEN("Toolbar");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_TOOLBAR);
	odp.pfnDlgProc = DlgProcToolBar;
	Options_AddPage(wParam, &odp);

	odp.pszTab = LPGEN("Advanced tweaks");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_PLUS);
	odp.pfnDlgProc = PlusOptionsProc;
	Options_AddPage(wParam, &odp);

	odp.pszGroup = LPGEN("Message sessions");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MSGTYPE);
	odp.pszTitle = LPGEN("Typing notify");
	odp.pfnDlgProc = DlgProcTypeOptions;
	Options_AddPage(wParam, &odp);

	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUP_OPT);
		odp.pszTitle = LPGEN("Event notifications");
		odp.pszGroup = LPGEN("Popups");
		odp.pfnDlgProc = DlgProcPopupOpts;
		Options_AddPage(wParam, &odp);
	}

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SKIN);
	odp.pszTitle = LPGEN("Message window");
	odp.pszTab = LPGEN("Load and apply");
	odp.pfnDlgProc = DlgProcSkinOpts;
	odp.pszGroup = LPGEN("Skins");
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_TABCONFIG);
	odp.pszTab = LPGEN("Window layout tweaks");
	odp.pfnDlgProc = DlgProcTabConfig;
	Options_AddPage(wParam, &odp);

	/* group chats */

	odp.pszGroup = LPGEN("Message sessions");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS1);
	odp.pszTitle = LPGEN("Group chats");
	odp.pszTab = LPGEN("Settings");
	odp.pfnDlgProc = DlgProcOptions1;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS2);
	odp.pszTab = LPGEN("Log formatting");
	odp.pfnDlgProc = DlgProcOptions2;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS3);
	odp.pszTab = LPGEN("Events and filters");
	odp.pfnDlgProc = DlgProcOptions3;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS4);
	odp.pszTab = LPGEN("Highlighting");
	odp.pfnDlgProc = CMUCHighlight::dlgProc;
	Options_AddPage(wParam, &odp);
	return 0;
}

enum
{
	CBVT_NONE,
	CBVT_CHAR,
	CBVT_INT,
	CBVT_BYTE,
	CBVT_DWORD,
	CBVT_BOOL,
};

struct OptCheckBox
{
	UINT idc;

	DWORD defValue;		// should be full combined value for masked items!
	DWORD dwBit;

	BYTE dbType;
	char *dbModule;
	char *dbSetting;

	BYTE valueType;
	union
	{
		void *pValue;

		char *charValue;
		int *intValue;
		BYTE *byteValue;
		DWORD *dwordValue;
		BOOL *boolValue;
	};
};

DWORD OptCheckBox_LoadValue(struct OptCheckBox *cb)
{
	switch (cb->valueType) {
	case CBVT_NONE:
		switch (cb->dbType) {
		case DBVT_BYTE:
			return M.GetByte(cb->dbModule, cb->dbSetting, cb->defValue);
		case DBVT_WORD:
			return db_get_w(NULL, cb->dbModule, cb->dbSetting, cb->defValue);
		case DBVT_DWORD:
			return M.GetDword(cb->dbModule, cb->dbSetting, cb->defValue);
		}
		break;

	case CBVT_CHAR:
		return *cb->charValue;
	case CBVT_INT:
		return *cb->intValue;
	case CBVT_BYTE:
		return *cb->byteValue;
	case CBVT_DWORD:
		return *cb->dwordValue;
	case CBVT_BOOL:
		return *cb->boolValue;
	}

	return cb->defValue;
}

void OptCheckBox_Load(HWND hwnd, OptCheckBox *cb)
{
	DWORD value = OptCheckBox_LoadValue(cb);
	if (cb->dwBit) value &= cb->dwBit;
	CheckDlgButton(hwnd, cb->idc, value ? BST_CHECKED : BST_UNCHECKED);
}

void OptCheckBox_Save(HWND hwnd, OptCheckBox *cb)
{
	DWORD value = IsDlgButtonChecked(hwnd, cb->idc) == BST_CHECKED;

	if (cb->dwBit) {
		DWORD curValue = OptCheckBox_LoadValue(cb);
		value = value ? (curValue | cb->dwBit) : (curValue & ~cb->dwBit);
	}

	switch (cb->dbType) {
	case DBVT_BYTE:
		db_set_b(0, cb->dbModule, cb->dbSetting, (BYTE)value);
		break;
	case DBVT_WORD:
		db_set_w(NULL, cb->dbModule, cb->dbSetting, (WORD)value);
		break;
	case DBVT_DWORD:
		db_set_dw(0, cb->dbModule, cb->dbSetting, (DWORD)value);
		break;
	}

	switch (cb->valueType) {
	case CBVT_CHAR:
		*cb->charValue = (char)value;
		break;
	case CBVT_INT:
		*cb->intValue = (int)value;
		break;
	case CBVT_BYTE:
		*cb->byteValue = (BYTE)value;
		break;
	case CBVT_DWORD:
		*cb->dwordValue = (DWORD)value;
		break;
	case CBVT_BOOL:
		*cb->boolValue = (BOOL)value;
		break;
	}
}

static INT_PTR CALLBACK DlgProcTabSrmmModernOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	OptCheckBox opts[] =
	{
		//{IDC_, def, bit, dbtype, dbmodule, dbsetting, valtype, pval},
		{ IDC_CLOSEONESC, FALSE, 0, DBVT_BYTE, SRMSGMOD_T, "escmode" },
		{ IDC_ALWAYSPOPUP, FALSE, 0, DBVT_BYTE, SRMSGMOD_T, SRMSGSET_AUTOPOPUP },
		{ IDC_CREATEMIN, TRUE, 0, DBVT_BYTE, SRMSGMOD_T, "autocontainer" },
		//{IDC_USETABS, , 0, DBVT_BYTE, SRMSGMOD_T, },
		{ IDC_CREATENOACTIVATE, TRUE, 0, DBVT_BYTE, SRMSGMOD_T, "autotabs" },
		{ IDC_POPUPONCREATE, FALSE, 0, DBVT_BYTE, SRMSGMOD_T, "cpopup" },
		{ IDC_AUTOSWITCHTABS, TRUE, 0, DBVT_BYTE, SRMSGMOD_T, "autoswitchtabs" },
		//{IDC_SENDCTRLENTER, , 0, DBVT_BYTE, SRMSGMOD_T, },
		{ IDC_SENDSHIFTENTER, FALSE, 0, DBVT_BYTE, SRMSGMOD_T, "sendonshiftenter" },
		{ IDC_SENDENTER, FALSE, 0, DBVT_BYTE, SRMSGMOD_T, SRMSGSET_SENDONENTER },
		{ IDC_SENDDBLENTER, FALSE, 0, DBVT_BYTE, SRMSGMOD_T, "SendOnDblEnter" },
		{ IDC_MINSEND, FALSE, 0, DBVT_BYTE, SRMSGMOD_T, SRMSGSET_AUTOMIN },
		{ IDC_NOOPENNOTIFY, FALSE, 0, DBVT_BYTE, "tabSRMM_NEN", OPT_WINDOWCHECK, CBVT_BOOL, &nen_options.bWindowCheck },
	};

	static BOOL bInit = TRUE;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			DWORD maxhist = M.GetDword("maxhist", 0);

			bInit = TRUE;

			for (int i = 0; i < SIZEOF(opts); ++i)
				OptCheckBox_Load(hwndDlg, opts + i);

			// Always on!
			CheckDlgButton(hwndDlg, IDC_SENDCTRLENTER, BST_CHECKED);

			switch (M.GetByte(SRMSGMOD, SRMSGSET_LOADHISTORY, SRMSGDEFSET_LOADHISTORY)) {
			case LOADHISTORY_UNREAD:
				CheckDlgButton(hwndDlg, IDC_LOADUNREAD, BST_CHECKED);
				break;
			case LOADHISTORY_COUNT:
				CheckDlgButton(hwndDlg, IDC_LOADCOUNT, BST_CHECKED);
				Utils::enableDlgControl(hwndDlg, IDC_LOADCOUNTN, true);
				Utils::enableDlgControl(hwndDlg, IDC_LOADCOUNTSPIN, true);
				break;
			case LOADHISTORY_TIME:
				CheckDlgButton(hwndDlg, IDC_LOADTIME, BST_CHECKED);
				Utils::enableDlgControl(hwndDlg, IDC_LOADTIMEN, true);
				Utils::enableDlgControl(hwndDlg, IDC_LOADTIMESPIN, true);
				Utils::enableDlgControl(hwndDlg, IDC_STMINSOLD, true);
				break;
			}

			SendDlgItemMessage(hwndDlg, IDC_LOADCOUNTSPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
			SendDlgItemMessage(hwndDlg, IDC_LOADCOUNTSPIN, UDM_SETPOS, 0, db_get_w(NULL, SRMSGMOD, SRMSGSET_LOADCOUNT, SRMSGDEFSET_LOADCOUNT));
			SendDlgItemMessage(hwndDlg, IDC_LOADTIMESPIN, UDM_SETRANGE, 0, MAKELONG(24 * 60, 0));
			SendDlgItemMessage(hwndDlg, IDC_LOADTIMESPIN, UDM_SETPOS, 0, db_get_w(NULL, SRMSGMOD, SRMSGSET_LOADTIME, SRMSGDEFSET_LOADTIME));

			SendDlgItemMessage(hwndDlg, IDC_TRIMSPIN, UDM_SETRANGE, 0, MAKELONG(1000, 5));
			SendDlgItemMessage(hwndDlg, IDC_TRIMSPIN, UDM_SETPOS, 0, maxhist);
			Utils::enableDlgControl(hwndDlg, IDC_TRIMSPIN, maxhist != 0);
			Utils::enableDlgControl(hwndDlg, IDC_TRIM, maxhist != 0);
			CheckDlgButton(hwndDlg, IDC_ALWAYSTRIM, maxhist != 0 ? BST_CHECKED : BST_UNCHECKED);

			BOOL bTabOptGroups = M.GetByte("useclistgroups", 0);
			BOOL bTabOptLimit = M.GetByte("limittabs", 0);
			BOOL bTabOptSingle = M.GetByte("singlewinmode", 0);

			if (bTabOptSingle && !bTabOptGroups && !bTabOptLimit)
				CheckDlgButton(hwndDlg, IDC_USETABS, BST_UNCHECKED);
			else if (!bTabOptSingle && !bTabOptGroups && !bTabOptLimit)
				CheckDlgButton(hwndDlg, IDC_USETABS, BST_CHECKED);
			else {
				LONG s = (GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_USETABS), GWL_STYLE) & ~BS_TYPEMASK) | BS_AUTO3STATE;
				SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_USETABS), GWL_STYLE, s);
				CheckDlgButton(hwndDlg, IDC_USETABS, BST_INDETERMINATE);
			}
		}
		bInit = FALSE;
		return TRUE;

	case WM_DESTROY:
		bInit = TRUE;
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_LOADUNREAD:
		case IDC_LOADCOUNT:
		case IDC_LOADTIME:
			Utils::enableDlgControl(hwndDlg, IDC_LOADCOUNTN, IsDlgButtonChecked(hwndDlg, IDC_LOADCOUNT) != 0);
			Utils::enableDlgControl(hwndDlg, IDC_LOADCOUNTSPIN, IsDlgButtonChecked(hwndDlg, IDC_LOADCOUNT) != 0);
			Utils::enableDlgControl(hwndDlg, IDC_LOADTIMEN, IsDlgButtonChecked(hwndDlg, IDC_LOADTIME) != 0);
			Utils::enableDlgControl(hwndDlg, IDC_LOADTIMESPIN, IsDlgButtonChecked(hwndDlg, IDC_LOADTIME) != 0);
			Utils::enableDlgControl(hwndDlg, IDC_STMINSOLD, IsDlgButtonChecked(hwndDlg, IDC_LOADTIME) != 0);
			break;
		case IDC_ALWAYSTRIM:
			Utils::enableDlgControl(hwndDlg, IDC_TRIMSPIN, IsDlgButtonChecked(hwndDlg, IDC_ALWAYSTRIM) != 0);
			Utils::enableDlgControl(hwndDlg, IDC_TRIM, IsDlgButtonChecked(hwndDlg, IDC_ALWAYSTRIM) != 0);
			break;
		case IDC_TRIM:
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
				return TRUE;
			break;
		}
		if (!bInit)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				for (int i = 0; i < SIZEOF(opts); ++i)
					OptCheckBox_Save(hwndDlg, opts + i);

				if (IsDlgButtonChecked(hwndDlg, IDC_LOADCOUNT))
					db_set_b(0, SRMSGMOD, SRMSGSET_LOADHISTORY, LOADHISTORY_COUNT);
				else if (IsDlgButtonChecked(hwndDlg, IDC_LOADTIME))
					db_set_b(0, SRMSGMOD, SRMSGSET_LOADHISTORY, LOADHISTORY_TIME);
				else
					db_set_b(0, SRMSGMOD, SRMSGSET_LOADHISTORY, LOADHISTORY_UNREAD);
				db_set_w(NULL, SRMSGMOD, SRMSGSET_LOADCOUNT, (WORD)SendDlgItemMessage(hwndDlg, IDC_LOADCOUNTSPIN, UDM_GETPOS, 0, 0));
				db_set_w(NULL, SRMSGMOD, SRMSGSET_LOADTIME, (WORD)SendDlgItemMessage(hwndDlg, IDC_LOADTIMESPIN, UDM_GETPOS, 0, 0));

				if (IsDlgButtonChecked(hwndDlg, IDC_ALWAYSTRIM))
					db_set_dw(0, SRMSGMOD_T, "maxhist", (DWORD)SendDlgItemMessage(hwndDlg, IDC_TRIMSPIN, UDM_GETPOS, 0, 0));
				else
					db_set_dw(0, SRMSGMOD_T, "maxhist", 0);

				switch (IsDlgButtonChecked(hwndDlg, IDC_USETABS)) {
				case BST_UNCHECKED:
					db_set_b(0, SRMSGMOD_T, "useclistgroups", 0);
					db_set_b(0, SRMSGMOD_T, "limittabs", 0);
					db_set_b(0, SRMSGMOD_T, "singlewinmode", 1);
					break;
				case BST_CHECKED:
					db_set_b(0, SRMSGMOD_T, "useclistgroups", 0);
					db_set_b(0, SRMSGMOD_T, "limittabs", 0);
					db_set_b(0, SRMSGMOD_T, "singlewinmode", 0);
					break;
				}
				PluginConfig.reloadSettings();
				M.BroadcastMessage(DM_OPTIONSAPPLIED, 1, 0);
				return TRUE;
			}
		}
		break;
	}

	return FALSE;
}

static int ModernOptInitialise(WPARAM wParam, LPARAM)
{
	static int iBoldControls[] =
	{
		IDC_TXT_TITLE1, IDC_TXT_TITLE2,
		IDC_TXT_TITLE3, IDC_TXT_TITLE4,
		IDC_TXT_TITLE5,
		MODERNOPT_CTRL_LAST
	};

	MODERNOPTOBJECT obj = { sizeof(obj) };
	obj.dwFlags = MODEROPT_FLG_TCHAR | MODEROPT_FLG_NORESIZE;
	obj.hIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	obj.hInstance = g_hInst;
	obj.iSection = MODERNOPT_PAGE_MSGS;
	obj.iType = MODERNOPT_TYPE_SECTIONPAGE;
	obj.iBoldControls = iBoldControls;
	obj.lpzClassicGroup = NULL;
	obj.lpzClassicPage = "Message Sessions";
	obj.lpzHelpUrl = "http://wiki.miranda.or.at/";

	obj.lpzTemplate = MAKEINTRESOURCEA(IDD_MODERNOPTS);
	obj.pfnDlgProc = DlgProcTabSrmmModernOptions;
	CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);
	return 0;
}

int TSAPI InitOptions(void)
{
	HookEvent(ME_OPT_INITIALISE, OptInitialise);
	HookEvent(ME_MODERNOPT_INITIALIZE, ModernOptInitialise);
	return 0;
}

INT_PTR CALLBACK DlgProcSetupStatusModes(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DWORD dwStatusMask = GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	static DWORD dwNewStatusMask = 0;
	static HWND hwndParent = 0;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		dwStatusMask = lParam;

		SetWindowText(hwndDlg, TranslateT("Choose status modes"));
		{
			for (int i = ID_STATUS_ONLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
				SetDlgItemText(hwndDlg, i, pcli->pfnGetStatusModeDescription(i, GSMDF_TCHAR));
				if (dwStatusMask != -1 && (dwStatusMask & (1 << (i - ID_STATUS_ONLINE))))
					CheckDlgButton(hwndDlg, i, BST_CHECKED);
				Utils::enableDlgControl(hwndDlg, i, dwStatusMask != -1);
			}
		}
		if (dwStatusMask == -1)
			CheckDlgButton(hwndDlg, IDC_ALWAYS, BST_CHECKED);
		ShowWindow(hwndDlg, SW_SHOWNORMAL);
		return TRUE;

	case DM_SETPARENTDIALOG:
		hwndParent = (HWND)lParam;
		break;
	case DM_GETSTATUSMASK:
		if (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS))
			dwNewStatusMask = -1;
		else {
			dwNewStatusMask = 0;
			for (int i = ID_STATUS_ONLINE; i <= ID_STATUS_OUTTOLUNCH; i++)
				dwNewStatusMask |= (IsDlgButtonChecked(hwndDlg, i) ? (1 << (i - ID_STATUS_ONLINE)) : 0);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			if (LOWORD(wParam) == IDOK) {
				SendMessage(hwndDlg, DM_GETSTATUSMASK, 0, 0);
				SendMessage(hwndParent, DM_STATUSMASKSET, 0, (LPARAM)dwNewStatusMask);
			}
			DestroyWindow(hwndDlg);
			break;
		case IDC_ALWAYS:
			for (int i = ID_STATUS_ONLINE; i <= ID_STATUS_OUTTOLUNCH; i++)
				Utils::enableDlgControl(hwndDlg, i, !IsDlgButtonChecked(hwndDlg, IDC_ALWAYS));
			break;
		}

	case WM_DESTROY:
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		break;
	}
	return FALSE;
}
