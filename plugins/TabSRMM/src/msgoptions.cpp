/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-17 Miranda NG project,
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

#include "stdafx.h"
#include "templates.h"

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

void LoadLogfont(int section, int i, LOGFONTA * lf, COLORREF * colour, char *szModule)
{
	LOGFONT lfResult;
	LoadMsgDlgFont(section, i, &lfResult, colour, szModule);
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
		mir_snprintf(lf->lfFaceName, "%S", lfResult.lfFaceName);
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

static int TSAPI ScanSkinDir(const wchar_t* tszFolder, HWND hwndCombobox)
{
	bool fValid = false;
	wchar_t tszMask[MAX_PATH];
	mir_snwprintf(tszMask, L"%s*.*", tszFolder);

	WIN32_FIND_DATA fd = { 0 };
	HANDLE h = FindFirstFile(tszMask, &fd);
	while (h != INVALID_HANDLE_VALUE) {
		if (mir_wstrlen(fd.cFileName) >= 5 && !wcsnicmp(fd.cFileName + mir_wstrlen(fd.cFileName) - 4, L".tsk", 4)) {
			fValid = true;
			break;
		}
		if (FindNextFile(h, &fd) == 0)
			break;
	}
	if (h != INVALID_HANDLE_VALUE)
		FindClose(h);

	if (fValid) {
		wchar_t	tszFinalName[MAX_PATH], tszRel[MAX_PATH];
		LRESULT lr;
		wchar_t	szBuf[255];

		mir_snwprintf(tszFinalName, L"%s%s", tszFolder, fd.cFileName);

		GetPrivateProfileString(L"Global", L"Name", L"None", szBuf, _countof(szBuf), tszFinalName);
		if (!mir_wstrcmp(szBuf, L"None")) {
			fd.cFileName[mir_wstrlen(fd.cFileName) - 4] = 0;
			wcsncpy_s(szBuf, fd.cFileName, _TRUNCATE);
		}

		PathToRelativeW(tszFinalName, tszRel, M.getSkinPath());
		if ((lr = SendMessage(hwndCombobox, CB_INSERTSTRING, -1, (LPARAM)szBuf)) != CB_ERR) {
			wchar_t *idata = (wchar_t*)mir_alloc((mir_wstrlen(tszRel) + 1) * sizeof(wchar_t));

			mir_wstrcpy(idata, tszRel);
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

	wchar_t tszSkinRoot[MAX_PATH], tszFindMask[MAX_PATH];
	wcsncpy_s(tszSkinRoot, M.getSkinPath(), _TRUNCATE);

	SetDlgItemText(GetParent(hwndCombobox), IDC_SKINROOTFOLDER, tszSkinRoot);
	mir_snwprintf(tszFindMask, L"%s*.*", tszSkinRoot);

	SendMessage(hwndCombobox, CB_RESETCONTENT, 0, 0);
	SendMessage(hwndCombobox, CB_INSERTSTRING, -1, (LPARAM)TranslateT("<no skin>"));

	WIN32_FIND_DATA fd = { 0 };
	HANDLE h = FindFirstFile(tszFindMask, &fd);
	while (h != INVALID_HANDLE_VALUE) {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && fd.cFileName[0] != '.') {
			wchar_t	tszSubDir[MAX_PATH];
			mir_snwprintf(tszSubDir, L"%s%s\\", tszSkinRoot, fd.cFileName);
			ScanSkinDir(tszSubDir, hwndCombobox);
		}
		if (FindNextFile(h, &fd) == 0)
			break;
	}
	if (h != INVALID_HANDLE_VALUE)
		FindClose(h);

	SendMessage(hwndCombobox, CB_SETCURSEL, 0, 0);
	if (0 == db_get_ws(0, SRMSGMOD_T, "ContainerSkin", &dbv)) {
		LRESULT lr = SendMessage(hwndCombobox, CB_GETCOUNT, 0, 0);
		for (int i = 1; i < lr; i++) {
			wchar_t *idata = (wchar_t*)SendMessage(hwndCombobox, CB_GETITEMDATA, i, 0);
			if (idata && idata != (wchar_t*)CB_ERR) {
				if (!mir_wstrcmpi(dbv.ptszVal, idata)) {
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
				const wchar_t *szFilename = GetThemeFileName(1);
				if (szFilename != nullptr)
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

				if (szFilename != nullptr) {
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
					Srmm_Broadcast(DM_OPTIONSAPPLIED, 1, 0);
					Srmm_Broadcast(DM_FORCEDREMAKELOG, 0, 0);
					SendMessage(GetParent(hwndDlg), WM_COMMAND, IDCANCEL, 0);
				}
			}
			break;

		case IDC_HELP_GENERAL:
			Utils_OpenUrl("https://wiki.miranda-ng.org/index.php?title=Plugin:TabSRMM/en/Using_skins");
			break;

		case IDC_SKIN_CLOSENOW:
			CloseAllContainers();
			break;

		case IDC_SKINNAME:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				LRESULT lr = SendDlgItemMessage(hwndDlg, IDC_SKINNAME, CB_GETCURSEL, 0, 0);
				if (lr != CB_ERR && lr > 0) {
					wchar_t	*tszRelPath = (wchar_t*)SendDlgItemMessage(hwndDlg, IDC_SKINNAME, CB_GETITEMDATA, lr, 0);
					if (tszRelPath && tszRelPath != (wchar_t*)CB_ERR)
						db_set_ws(0, SRMSGMOD_T, "ContainerSkin", tszRelPath);
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
	for (int i = 0; lvGroups[i].szName != nullptr; i++) {
		tvi.hParent = 0;
		tvi.hInsertAfter = TVI_LAST;
		tvi.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.item.pszText = TranslateW(lvGroups[i].szName);
		tvi.item.stateMask = TVIS_EXPANDED | TVIS_BOLD;
		tvi.item.state = TVIS_EXPANDED | TVIS_BOLD;
		tvi.item.iImage = tvi.item.iSelectedImage = IMG_GRPOPEN;
		lvGroups[i].handle = (LRESULT)TreeView_InsertItem(hwndTree, &tvi);
	}

	for (int i = 0; lvItems[i].szName != nullptr; i++) {
		tvi.hParent = (HTREEITEM)lvGroups[lvItems[i].uGroup].handle;
		tvi.hInsertAfter = TVI_LAST;
		tvi.item.pszText = TranslateW(lvItems[i].szName);
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

	for (int i = 0; lvItems[i].szName != nullptr; i++) {
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

	for (int i = 0; lvItems[i].szName != nullptr; i++) {
		item.mask = TVIF_HANDLE | TVIF_IMAGE;
		item.hItem = (HTREEITEM)lvItems[i].handle;
		TreeView_GetItem(hwndTree, &item);

		switch (lvItems[i].uType) {
		case LOI_TYPE_FLAG:
			if (dwFlags != nullptr)
				(*dwFlags) |= (item.iImage == IMG_CHECK) ? lvItems[i].lParam : 0;
			if (DBPath == nullptr) {
				UINT *tm = (UINT*)lvItems[i].lParam;
				(*tm) = (item.iImage == IMG_CHECK) ? ((*tm) | lvItems[i].id) : ((*tm) & ~lvItems[i].id);
			}
			break;
		case LOI_TYPE_SETTING:
			if (DBPath != nullptr) {
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
		RedrawWindow(hwndTree, nullptr, nullptr, RDW_INVALIDATE | RDW_NOFRAME | RDW_ERASENOW | RDW_ALLCHILDREN);
		InvalidateRect(hwndTree, nullptr, TRUE);
	}
	else {
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Main options dialog

class COptMainDlg : public CDlgBase
{
	CCtrlSpin   spnAvatar;
	CCtrlButton btnHelp, btnReset;
	CCtrlEdit   edtAvaSize;
	CCtrlCheck  chkAvaPreserve;

public:
	COptMainDlg()
		: CDlgBase(g_hInst, IDD_OPT_MSGDLG),
		btnHelp(this, IDC_HELP_GENERAL),
		btnReset(this, IDC_RESETWARNINGS),
		spnAvatar(this, IDC_AVATARSPIN),
		edtAvaSize(this, IDC_MAXAVATARHEIGHT),
		chkAvaPreserve(this, IDC_PRESERVEAVATARSIZE)
	{
		btnHelp.OnClick = Callback(this, &COptMainDlg::onClick_Help);
		btnReset.OnClick = Callback(this, &COptMainDlg::onClick_Reset);
	}

	virtual void OnInitDialog() override
	{
		TreeViewInit(GetDlgItem(m_hwnd, IDC_WINDOWOPTIONS), CTranslator::TREE_MSG, 0, FALSE);

		int iAvaHeight = M.GetDword("avatarheight", 100);
		edtAvaSize.SetInt(iAvaHeight);
		
		chkAvaPreserve.SetState(M.GetByte("dontscaleavatars", 0));

		spnAvatar.SetRange(150);
		spnAvatar.SetPosition(iAvaHeight);
	}

	virtual void OnApply() override
	{
		db_set_dw(0, SRMSGMOD_T, "avatarheight", edtAvaSize.GetInt());

		db_set_b(0, SRMSGMOD_T, "dontscaleavatars", chkAvaPreserve.GetState());

		// scan the tree view and obtain the options...
		TreeViewToDB(GetDlgItem(m_hwnd, IDC_WINDOWOPTIONS), CTranslator::TREE_MSG, SRMSGMOD_T, nullptr);
		PluginConfig.reloadSettings();
		Srmm_Broadcast(DM_OPTIONSAPPLIED, 1, 0);
	}

	virtual void OnDestroy() override
	{
		TreeViewDestroy(GetDlgItem(m_hwnd, IDC_WINDOWOPTIONS));
	}

	void onClick_Help(CCtrlButton*)
	{
		Utils_OpenUrl("https://wiki.miranda-ng.org/index.php?title=Plugin:TabSRMM/en/General_settings");
	}

	void onClick_Reset(CCtrlButton*)
	{
		db_set_dw(0, SRMSGMOD_T, "cWarningsL", 0);
		db_set_dw(0, SRMSGMOD_T, "cWarningsH", 0);
	}

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_NOTIFY && ((LPNMHDR)lParam)->idFrom == IDC_WINDOWOPTIONS)
			return TreeViewHandleClick(m_hwnd, ((LPNMHDR)lParam)->hwndFrom, wParam, lParam);

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

static UINT __ctrls[] = { IDC_INDENTSPIN, IDC_RINDENTSPIN, IDC_INDENTAMOUNT, IDC_RIGHTINDENT, IDC_MODIFY, IDC_RTLMODIFY };

class COptLogDlg : public CDlgBase
{
	CCtrlButton btnModify, btnRtlModify;
	CCtrlCheck  chkAlwaysTrim, chkLoadUnread, chkLoadCount, chkLoadTime;
	CCtrlSpin   spnLeft, spnRight, spnLoadCount, spnLoadTime, spnTrim;
	CCtrlCombo 	cmbLogDisplay;

	bool have_ieview, have_hpp;

	// configure the option page - hide most of the settings here when either IEView
	// or H++ is set as the global message log viewer. Showing these options may confuse
	// the user, because they are not working and the user needs to configure the 3rd
	// party plugin.

	void ShowHide()
	{
		LRESULT r = cmbLogDisplay.GetCurSel();
		Utils::showDlgControl(m_hwnd, IDC_EXPLAINMSGLOGSETTINGS, r == 0 ? SW_HIDE : SW_SHOW);
		Utils::showDlgControl(m_hwnd, IDC_LOGOPTIONS, r == 0 ? SW_SHOW : SW_HIDE);

		for (int i = 0; i < _countof(__ctrls); i++)
			Utils::enableDlgControl(m_hwnd, __ctrls[i], r == 0);
	}

public:
	COptLogDlg()
		: CDlgBase(g_hInst, IDD_OPT_MSGLOG),
		btnModify(this, IDC_MODIFY),
		btnRtlModify(this, IDC_RTLMODIFY),
		spnTrim(this, IDC_TRIMSPIN),
		spnLeft(this, IDC_INDENTSPIN),
		spnRight(this, IDC_RINDENTSPIN),
		spnLoadTime(this, IDC_LOADTIMESPIN),
		spnLoadCount(this, IDC_LOADCOUNTSPIN),
		chkLoadTime(this, IDC_LOADTIME),
		chkLoadCount(this, IDC_LOADCOUNT),
		chkAlwaysTrim(this, IDC_ALWAYSTRIM),
		chkLoadUnread(this, IDC_LOADUNREAD),
		cmbLogDisplay(this, IDC_MSGLOGDIDSPLAY)
	{
		btnModify.OnClick = Callback(this, &COptLogDlg::onClick_Modify);
		btnRtlModify.OnClick = Callback(this, &COptLogDlg::onClick_RtlModify);

		cmbLogDisplay.OnChange = Callback(this, &COptLogDlg::onChange_Combo);

		chkAlwaysTrim.OnChange = Callback(this, &COptLogDlg::onChange_Trim);
		chkLoadTime.OnChange = chkLoadCount.OnChange = chkLoadUnread.OnChange = Callback(this, &COptLogDlg::onChange_Load);

		have_ieview = ServiceExists(MS_IEVIEW_WINDOW) != 0;
		have_hpp = ServiceExists("History++/ExtGrid/NewWindow") != 0;
	}

	virtual void OnInitDialog() override
	{
		DWORD dwFlags = M.GetDword("mwflags", MWF_LOG_DEFAULT);

		switch (M.GetByte(SRMSGMOD, SRMSGSET_LOADHISTORY, SRMSGDEFSET_LOADHISTORY)) {
		case LOADHISTORY_UNREAD:
			chkLoadUnread.SetState(true);
			break;
		case LOADHISTORY_COUNT:
			chkLoadCount.SetState(true);
			Utils::enableDlgControl(m_hwnd, IDC_LOADCOUNTN, true);
			spnLoadCount.Enable(true);
			break;
		case LOADHISTORY_TIME:
			chkLoadTime.SetState(true);
			Utils::enableDlgControl(m_hwnd, IDC_LOADTIMEN, true);
			spnLoadTime.Enable(true);
			Utils::enableDlgControl(m_hwnd, IDC_STMINSOLD, true);
			break;
		}

		TreeViewInit(GetDlgItem(m_hwnd, IDC_LOGOPTIONS), CTranslator::TREE_LOG, dwFlags, FALSE);

		spnLeft.SetRange(1000);
		spnLeft.SetPosition(M.GetDword("IndentAmount", 20));

		spnRight.SetRange(1000);
		spnRight.SetPosition(M.GetDword("RightIndent", 20));

		spnLoadCount.SetRange(100);
		spnLoadCount.SetPosition(db_get_w(0, SRMSGMOD, SRMSGSET_LOADCOUNT, SRMSGDEFSET_LOADCOUNT));
		
		spnLoadTime.SetRange(24 * 60);
		spnLoadTime.SetPosition(db_get_w(0, SRMSGMOD, SRMSGSET_LOADTIME, SRMSGDEFSET_LOADTIME));

		DWORD maxhist = M.GetDword("maxhist", 0);
		spnTrim.SetRange(1000, 5);
		spnTrim.SetPosition(maxhist);
		spnTrim.Enable(maxhist != 0);
		Utils::enableDlgControl(m_hwnd, IDC_TRIM, maxhist != 0);
		chkAlwaysTrim.SetState(maxhist != 0);

		cmbLogDisplay.AddString(TranslateT("Internal message log"));
		cmbLogDisplay.SetCurSel(0);
		if (have_ieview || have_hpp) {
			if (have_ieview) {
				cmbLogDisplay.AddString(TranslateT("IEView plugin"));
				if (M.GetByte("default_ieview", 0))
					cmbLogDisplay.SetCurSel(1);
			}
			if (have_hpp) {
				cmbLogDisplay.AddString(TranslateT("History++ plugin"));
				if (M.GetByte("default_ieview", 0))
					cmbLogDisplay.SetCurSel(1);
				else if (M.GetByte("default_hpp", 0))
					cmbLogDisplay.SetCurSel(have_ieview ? 2 : 1);
			}
		}
		else cmbLogDisplay.Disable();

		SetDlgItemText(m_hwnd, IDC_EXPLAINMSGLOGSETTINGS, TranslateT("You have chosen to use an external plugin for displaying the message history in the chat window. Most of the settings on this page are for the standard message log viewer only and will have no effect. To change the appearance of the message log, you must configure either IEView or History++."));
		ShowHide();
	}

	virtual void OnApply() override
	{
		LRESULT msglogmode = cmbLogDisplay.GetCurSel();
		DWORD dwFlags = M.GetDword("mwflags", MWF_LOG_DEFAULT);

		dwFlags &= ~(MWF_LOG_ALL);

		if (chkLoadCount.GetState())
			db_set_b(0, SRMSGMOD, SRMSGSET_LOADHISTORY, LOADHISTORY_COUNT);
		else if (chkLoadTime.GetState())
			db_set_b(0, SRMSGMOD, SRMSGSET_LOADHISTORY, LOADHISTORY_TIME);
		else
			db_set_b(0, SRMSGMOD, SRMSGSET_LOADHISTORY, LOADHISTORY_UNREAD);
		db_set_w(0, SRMSGMOD, SRMSGSET_LOADCOUNT, spnLoadCount.GetPosition());
		db_set_w(0, SRMSGMOD, SRMSGSET_LOADTIME, spnLoadTime.GetPosition());

		db_set_dw(0, SRMSGMOD_T, "IndentAmount", spnLeft.GetPosition());
		db_set_dw(0, SRMSGMOD_T, "RightIndent", spnRight.GetPosition());

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
		TreeViewToDB(GetDlgItem(m_hwnd, IDC_LOGOPTIONS), CTranslator::TREE_LOG, SRMSGMOD_T, &dwFlags);
		db_set_dw(0, SRMSGMOD_T, "mwflags", dwFlags);
		if (chkAlwaysTrim.GetState())
			db_set_dw(0, SRMSGMOD_T, "maxhist", spnTrim.GetPosition());
		else
			db_set_dw(0, SRMSGMOD_T, "maxhist", 0);
		PluginConfig.reloadSettings();
		Srmm_Broadcast(DM_OPTIONSAPPLIED, 1, 0);
	}

	virtual void OnDestroy() override
	{
		TreeViewDestroy(GetDlgItem(m_hwnd, IDC_LOGOPTIONS));
	}

	void onChange_Trim(CCtrlCheck*)
	{
		bool bEnabled = chkAlwaysTrim.GetState();
		spnTrim.Enable(bEnabled);
		Utils::enableDlgControl(m_hwnd, IDC_TRIM, bEnabled);
	}

	void onChange_Load(CCtrlCheck*)
	{
		bool bEnabled = chkLoadCount.GetState();
		Utils::enableDlgControl(m_hwnd, IDC_LOADCOUNTN, bEnabled);
		Utils::enableDlgControl(m_hwnd, IDC_LOADCOUNTSPIN, bEnabled);

		bEnabled = chkLoadTime.GetState();
		Utils::enableDlgControl(m_hwnd, IDC_LOADTIMEN, bEnabled);
		spnLoadTime.Enable(bEnabled);
		Utils::enableDlgControl(m_hwnd, IDC_STMINSOLD, bEnabled);
	}

	void onClick_Modify(CCtrlButton*)
	{
		CTemplateEditDlg *pDlg = new CTemplateEditDlg(FALSE, m_hwnd);
		pDlg->Show();
	}

	void onClick_RtlModify(CCtrlButton*)
	{
		CTemplateEditDlg *pDlg = new CTemplateEditDlg(TRUE, m_hwnd);
		pDlg->Show();
	}

	void onChange_Combo(CCtrlCombo*)
	{
		ShowHide();
	}

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_NOTIFY && ((LPNMHDR)lParam)->idFrom == IDC_LOGOPTIONS)
			return TreeViewHandleClick(m_hwnd, ((LPNMHDR)lParam)->hwndFrom, wParam, lParam);

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// typing notify options

class COptTypingDlg : public CDlgBase
{
	HANDLE hItemNew, hItemUnknown;

	CCtrlCheck chkWin, chkNoWin;
	CCtrlCheck chkNotifyPopup, chkNotifyTray, chkShowNotify;
	CCtrlButton btnHelp;

	void ResetCList()
	{
		if (!M.GetByte("CList", "UseGroups", SETTING_USEGROUPS_DEFAULT))
			SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_SETUSEGROUPS, FALSE, 0);
		else
			SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_SETUSEGROUPS, TRUE, 0);
		SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_SETHIDEEMPTYGROUPS, 1, 0);
	}

	void RebuildList()
	{
		BYTE defType = M.GetByte(SRMSGMOD, SRMSGSET_TYPINGNEW, SRMSGDEFSET_TYPINGNEW);
		if (hItemNew && defType)
			SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItemNew, 1);

		if (hItemUnknown && M.GetByte(SRMSGMOD, SRMSGSET_TYPINGUNKNOWN, SRMSGDEFSET_TYPINGUNKNOWN))
			SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItemUnknown, 1);

		for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
			HANDLE hItem = (HANDLE)SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
			if (hItem && db_get_b(hContact, SRMSGMOD, SRMSGSET_TYPING, defType))
				SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItem, 1);
		}
	}

	void SaveList()
	{
		if (hItemNew)
			db_set_b(0, SRMSGMOD, SRMSGSET_TYPINGNEW, (BYTE)(SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItemNew, 0) ? 1 : 0));

		if (hItemUnknown)
			db_set_b(0, SRMSGMOD, SRMSGSET_TYPINGUNKNOWN, (BYTE)(SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItemUnknown, 0) ? 1 : 0));

		for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
			HANDLE hItem = (HANDLE)SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
			if (hItem)
				db_set_b(hContact, SRMSGMOD, SRMSGSET_TYPING, (BYTE)(SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItem, 0) ? 1 : 0));
		}
	}

public:
	COptTypingDlg()
		: CDlgBase(g_hInst, IDD_OPT_MSGTYPE),
		btnHelp(this, IDC_MTN_HELP),
		chkWin(this, IDC_TYPEWIN),
		chkNoWin(this, IDC_TYPENOWIN),
		chkNotifyTray(this, IDC_NOTIFYTRAY),
		chkShowNotify(this, IDC_SHOWNOTIFY),
		chkNotifyPopup(this, IDC_NOTIFYPOPUP)
	{
		btnHelp.OnClick = Callback(this, &COptTypingDlg::onClick_Help);

		chkWin.OnChange = chkNoWin.OnChange = Callback(this, &COptTypingDlg::onCheck_Win);

		chkNotifyTray.OnChange = Callback(this, &COptTypingDlg::onCheck_NotifyTray);
		chkShowNotify.OnChange = Callback(this, &COptTypingDlg::onCheck_ShowNotify);
		chkNotifyPopup.OnChange = Callback(this, &COptTypingDlg::onCheck_NotifyPopup);
	}

	virtual void OnInitDialog() override
	{
		CLCINFOITEM cii = { sizeof(cii) };
		cii.flags = CLCIIF_GROUPFONT | CLCIIF_CHECKBOX;
		cii.pszText = TranslateT("** New contacts **");
		hItemNew = (HANDLE)SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
		cii.pszText = TranslateT("** Unknown contacts **");
		hItemUnknown = (HANDLE)SendDlgItemMessage(m_hwnd, IDC_CLIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);

		SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_CLIST), GWL_STYLE, GetWindowLongPtr(GetDlgItem(m_hwnd, IDC_CLIST), GWL_STYLE) | (CLS_SHOWHIDDEN));
		ResetCList();

		CheckDlgButton(m_hwnd, IDC_SHOWNOTIFY, M.GetByte(SRMSGMOD, SRMSGSET_SHOWTYPING, SRMSGDEFSET_SHOWTYPING) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_TYPEFLASHWIN, M.GetByte(SRMSGMOD, SRMSGSET_SHOWTYPINGWINFLASH, SRMSGDEFSET_SHOWTYPINGWINFLASH) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(m_hwnd, IDC_TYPENOWIN, M.GetByte(SRMSGMOD, SRMSGSET_SHOWTYPINGNOWINOPEN, 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_TYPEWIN, M.GetByte(SRMSGMOD, SRMSGSET_SHOWTYPINGWINOPEN, 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_NOTIFYTRAY, M.GetByte(SRMSGMOD, SRMSGSET_SHOWTYPINGCLIST, SRMSGDEFSET_SHOWTYPINGCLIST) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_NOTIFYBALLOON, M.GetByte(SRMSGMOD, "ShowTypingBalloon", 0));

		CheckDlgButton(m_hwnd, IDC_NOTIFYPOPUP, M.GetByte(SRMSGMOD, "ShowTypingPopup", 0) ? BST_CHECKED : BST_UNCHECKED);

		Utils::enableDlgControl(m_hwnd, IDC_TYPEWIN, IsDlgButtonChecked(m_hwnd, IDC_NOTIFYTRAY) != 0);
		Utils::enableDlgControl(m_hwnd, IDC_TYPENOWIN, IsDlgButtonChecked(m_hwnd, IDC_NOTIFYTRAY) != 0);
		Utils::enableDlgControl(m_hwnd, IDC_NOTIFYBALLOON, IsDlgButtonChecked(m_hwnd, IDC_NOTIFYTRAY) &&
			(IsDlgButtonChecked(m_hwnd, IDC_TYPEWIN) || IsDlgButtonChecked(m_hwnd, IDC_TYPENOWIN)));

		Utils::enableDlgControl(m_hwnd, IDC_TYPEFLASHWIN, IsDlgButtonChecked(m_hwnd, IDC_SHOWNOTIFY) != 0);
		Utils::enableDlgControl(m_hwnd, IDC_MTN_POPUPMODE, IsDlgButtonChecked(m_hwnd, IDC_NOTIFYPOPUP) != 0);

		SendDlgItemMessage(m_hwnd, IDC_MTN_POPUPMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Always"));
		SendDlgItemMessage(m_hwnd, IDC_MTN_POPUPMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Always, but no popup when window is focused"));
		SendDlgItemMessage(m_hwnd, IDC_MTN_POPUPMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Only when no message window is open"));

		SendDlgItemMessage(m_hwnd, IDC_MTN_POPUPMODE, CB_SETCURSEL, (WPARAM)M.GetByte("MTN_PopupMode", 0), 0);

		if (!PluginConfig.g_bPopupAvail) {
			Utils::showDlgControl(m_hwnd, IDC_NOTIFYPOPUP, SW_HIDE);
			Utils::showDlgControl(m_hwnd, IDC_STATIC111, SW_HIDE);
			Utils::showDlgControl(m_hwnd, IDC_MTN_POPUPMODE, SW_HIDE);
		}
	}

	virtual void OnApply() override
	{
		SaveList();
		db_set_b(0, SRMSGMOD, SRMSGSET_SHOWTYPING, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_SHOWNOTIFY));
		db_set_b(0, SRMSGMOD, SRMSGSET_SHOWTYPINGWINFLASH, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_TYPEFLASHWIN));
		db_set_b(0, SRMSGMOD, SRMSGSET_SHOWTYPINGNOWINOPEN, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_TYPENOWIN));
		db_set_b(0, SRMSGMOD, SRMSGSET_SHOWTYPINGWINOPEN, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_TYPEWIN));
		db_set_b(0, SRMSGMOD, SRMSGSET_SHOWTYPINGCLIST, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_NOTIFYTRAY));
		db_set_b(0, SRMSGMOD, "ShowTypingBalloon", (BYTE)IsDlgButtonChecked(m_hwnd, IDC_NOTIFYBALLOON));
		db_set_b(0, SRMSGMOD, "ShowTypingPopup", (BYTE)IsDlgButtonChecked(m_hwnd, IDC_NOTIFYPOPUP));
		db_set_b(0, SRMSGMOD_T, "MTN_PopupMode", (BYTE)SendDlgItemMessage(m_hwnd, IDC_MTN_POPUPMODE, CB_GETCURSEL, 0, 0));
		PluginConfig.reloadSettings();
	}

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_NOTIFY && ((NMHDR*)lParam)->idFrom == IDC_CLIST) {
			switch (((NMHDR*)lParam)->code) {
			case CLN_OPTIONSCHANGED:
				ResetCList();
				break;
			case CLN_CHECKCHANGED:
				SendMessage(m_hwndParent, PSM_CHANGED, 0, 0);
				break;
			case CLN_LISTREBUILT:
				RebuildList();
				break;
			}
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

	void onCheck_NotifyPopup(CCtrlCheck*)
	{
		Utils::enableDlgControl(m_hwnd, IDC_MTN_POPUPMODE, IsDlgButtonChecked(m_hwnd, IDC_NOTIFYPOPUP) != 0);
	}

	void onCheck_NotifyTray(CCtrlCheck*)
	{
		Utils::enableDlgControl(m_hwnd, IDC_TYPEWIN, IsDlgButtonChecked(m_hwnd, IDC_NOTIFYTRAY) != 0);
		Utils::enableDlgControl(m_hwnd, IDC_TYPENOWIN, IsDlgButtonChecked(m_hwnd, IDC_NOTIFYTRAY) != 0);
		Utils::enableDlgControl(m_hwnd, IDC_NOTIFYBALLOON, IsDlgButtonChecked(m_hwnd, IDC_NOTIFYTRAY) != 0);
	}

	void onCheck_ShowNotify(CCtrlCheck*)
	{
		Utils::enableDlgControl(m_hwnd, IDC_TYPEFLASHWIN, IsDlgButtonChecked(m_hwnd, IDC_SHOWNOTIFY) != 0);
	}

	void onCheck_Win(CCtrlCheck*)
	{
		Utils::enableDlgControl(m_hwnd, IDC_NOTIFYBALLOON, IsDlgButtonChecked(m_hwnd, IDC_NOTIFYTRAY) &&
			(IsDlgButtonChecked(m_hwnd, IDC_TYPEWIN) || IsDlgButtonChecked(m_hwnd, IDC_TYPENOWIN)));
	}

	void onClick_Help(CCtrlButton*)
	{
		Utils_OpenUrl("https://wiki.miranda-ng.org/index.php?title=Plugin:TabSRMM/en/Typing_notifications");
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// options for tabbed messaging got their own page.. finally :)

class COptTabbedDlg : public CDlgBase
{
	CCtrlEdit  	edtLimit;
	CCtrlCheck  chkLimit;
	CCtrlSpin   spnLimit;
	CCtrlCombo  cmbEscMode;
	CCtrlButton btnSetup;

public:
	COptTabbedDlg()
		: CDlgBase(g_hInst, IDD_OPT_TABBEDMSG),
		chkLimit(this, IDC_CUT_TABTITLE),
		edtLimit(this, IDC_CUT_TITLEMAX),
		spnLimit(this, IDC_CUT_TITLEMAXSPIN),
		btnSetup(this, IDC_SETUPAUTOCREATEMODES),
		cmbEscMode(this, IDC_ESCMODE)
	{
		btnSetup.OnClick = Callback(this, &COptTabbedDlg::onClick_Setup);

		chkLimit.OnChange = Callback(this, &COptTabbedDlg::onChange_Cut);
	}

	virtual void OnInitDialog() override
	{
		TreeViewInit(GetDlgItem(m_hwnd, IDC_TABMSGOPTIONS), CTranslator::TREE_TAB, 0, FALSE);

		chkLimit.SetState(M.GetByte("cuttitle", 0));
		spnLimit.SetRange(20, 5);
		spnLimit.SetPosition(db_get_w(0, SRMSGMOD_T, "cut_at", 15));
		onChange_Cut(&chkLimit);
		
		cmbEscMode.AddString(TranslateT("Normal - close tab, if last tab is closed also close the window"));
		cmbEscMode.AddString(TranslateT("Minimize the window to the task bar"));
		cmbEscMode.AddString(TranslateT("Close or hide window, depends on the close button setting above"));
		cmbEscMode.SetCurSel(PluginConfig.m_EscapeCloses);
	}

	virtual void OnApply() override
	{
		db_set_w(0, SRMSGMOD_T, "cut_at", spnLimit.GetPosition());
		db_set_b(0, SRMSGMOD_T, "cuttitle", chkLimit.GetState());
		db_set_b(0, SRMSGMOD_T, "escmode", cmbEscMode.GetCurSel());

		TreeViewToDB(GetDlgItem(m_hwnd, IDC_TABMSGOPTIONS), CTranslator::TREE_TAB, SRMSGMOD_T, nullptr);
		
		PluginConfig.reloadSettings();
		Srmm_Broadcast(DM_OPTIONSAPPLIED, 0, 0);
	}

	virtual void OnDestroy() override
	{
		TreeViewDestroy(GetDlgItem(m_hwnd, IDC_TABMSGOPTIONS));
	}

	void onClick_Setup(CCtrlButton*)
	{
		HWND hwndNew = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_CHOOSESTATUSMODES), m_hwnd, DlgProcSetupStatusModes, M.GetDword("autopopupmask", -1));
		SendMessage(hwndNew, DM_SETPARENTDIALOG, 0, (LPARAM)m_hwnd);
	}

	void onChange_Cut(CCtrlCheck*)
	{
		bool bEnabled = chkLimit.GetState() != 0;
		edtLimit.Enable(bEnabled);
		spnLimit.Enable(bEnabled);	}

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_COMMAND && wParam == DM_STATUSMASKSET)
			db_set_dw(0, SRMSGMOD_T, "autopopupmask", (DWORD)lParam);

		if (msg == WM_NOTIFY && ((LPNMHDR)lParam)->idFrom == IDC_TABMSGOPTIONS)
			return TreeViewHandleClick(m_hwnd, ((LPNMHDR)lParam)->hwndFrom, wParam, lParam);

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// container options

class COptContainersDlg : public CDlgBase
{
	CCtrlButton btnHelp;
	CCtrlCombo  cmbAeroEffect;
	CCtrlCheck  chkUseAero, chkUseAeroPeek, chkLimits, chkSingle, chkGroup, chkDefault;
	CCtrlSpin   spnNumFlash, spnTabLimit, spnFlashDelay;

	void onHelp(CCtrlButton*)
	{
		Utils_OpenUrl("https://wiki.miranda-ng.org/index.php?title=Plugin:TabSRMM/en/Containers");
	}

	void onChangeAero(CCtrlCheck*)
	{
		Utils::enableDlgControl(m_hwnd, IDC_AEROEFFECT, chkUseAero.GetState() != 0);
	}

	void onChangeLimits(CCtrlCheck*)
	{
		Utils::enableDlgControl(m_hwnd, IDC_TABLIMIT, chkLimits.GetState() != 0);
	}

public:
	COptContainersDlg()
		: CDlgBase(g_hInst, IDD_OPT_CONTAINERS),
		btnHelp(this, IDC_HELP_CONTAINERS),
		spnNumFlash(this, IDC_NRFLASHSPIN),
		spnTabLimit(this, IDC_TABLIMITSPIN),
		spnFlashDelay(this, IDC_FLASHINTERVALSPIN),
		chkUseAero(this, IDC_USEAERO),
		chkUseAeroPeek(this, IDC_USEAEROPEEK),
		cmbAeroEffect(this, IDC_AEROEFFECT),
		chkLimits(this, IDC_LIMITTABS),
		chkSingle(this, IDC_SINGLEWINDOWMODE),
		chkGroup(this, IDC_CONTAINERGROUPMODE),
		chkDefault(this, IDC_DEFAULTCONTAINERMODE)
	{
		btnHelp.OnClick = Callback(this, &COptContainersDlg::onHelp);

		chkUseAero.OnChange = Callback(this, &COptContainersDlg::onChangeAero);
		chkLimits.OnChange = chkSingle.OnChange = chkGroup.OnChange = chkDefault.OnChange = Callback(this, &COptContainersDlg::onChangeLimits);
	}

	virtual void OnInitDialog() override
	{
		chkGroup.SetState(M.GetByte("useclistgroups", 0));
		chkLimits.SetState(M.GetByte("limittabs", 0));

		spnTabLimit.SetRange(1000, 1);
		spnTabLimit.SetPosition(M.GetDword("maxtabs", 1));
		onChangeLimits(nullptr);
		
		chkSingle.SetState(M.GetByte("singlewinmode", 0));
		chkDefault.SetState(!(chkGroup.GetState() || chkLimits.GetState() || chkSingle.GetState()));

		spnNumFlash.SetRange(255);
		spnNumFlash.SetPosition(M.GetByte("nrflash", 4));

		spnFlashDelay.SetRange(10000, 500);
		spnFlashDelay.SetPosition(M.GetDword("flashinterval", 1000));

		chkUseAero.SetState(M.GetByte("useAero", 1));
		chkUseAeroPeek.SetState(M.GetByte("useAeroPeek", 1));

		for (int i = 0; i < CSkin::AERO_EFFECT_LAST; i++)
			cmbAeroEffect.InsertString(TranslateW(CSkin::m_aeroEffects[i].tszName), -1);
		cmbAeroEffect.SetCurSel(CSkin::m_aeroEffect);
		cmbAeroEffect.Enable(PluginConfig.m_bIsVista);
		
		chkUseAero.Enable(PluginConfig.m_bIsVista);
		chkUseAeroPeek.Enable(PluginConfig.m_bIsWin7);
		if (PluginConfig.m_bIsVista)
			Utils::enableDlgControl(m_hwnd, IDC_AEROEFFECT, chkUseAero.GetState() != 0);
	}

	virtual void OnApply() override
	{
		bool fOldAeroState = M.getAeroState();

		db_set_b(0, SRMSGMOD_T, "useclistgroups", chkGroup.GetState());
		db_set_b(0, SRMSGMOD_T, "limittabs", chkLimits.GetState());
		db_set_dw(0, SRMSGMOD_T, "maxtabs", spnTabLimit.GetPosition());
		db_set_b(0, SRMSGMOD_T, "singlewinmode", chkSingle.GetState());
		db_set_dw(0, SRMSGMOD_T, "flashinterval", spnFlashDelay.GetPosition());
		db_set_b(0, SRMSGMOD_T, "nrflash", spnNumFlash.GetPosition());
		db_set_b(0, SRMSGMOD_T, "useAero", chkUseAero.GetState());
		db_set_b(0, SRMSGMOD_T, "useAeroPeek", chkUseAeroPeek.GetState());
		
		CSkin::setAeroEffect(cmbAeroEffect.GetCurSel());
		if (M.getAeroState() != fOldAeroState) {
			SendMessage(PluginConfig.g_hwndHotkeyHandler, WM_DWMCOMPOSITIONCHANGED, 0, 0);	// simulate aero state change
			SendMessage(PluginConfig.g_hwndHotkeyHandler, WM_DWMCOLORIZATIONCOLORCHANGED, 0, 0);	// simulate aero state change
		}
		BuildContainerMenu();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// TabModPlus options

class COptAdvancedDlg : public CDlgBase
{
	CCtrlSpin spnTimeout, spnHistSize;
	CCtrlButton btnHelp, btnRevert;

public:
	COptAdvancedDlg()
		: CDlgBase(g_hInst, IDD_OPTIONS_PLUS),
		btnHelp(this, IDC_PLUS_HELP),
		btnRevert(this, IDC_PLUS_REVERT),
		spnTimeout(this, IDC_TIMEOUTSPIN),
		spnHistSize(this, IDC_HISTORYSIZESPIN)
	{
		btnHelp.OnClick = Callback(this, &COptAdvancedDlg::onClick_Help);
		btnRevert.OnClick = Callback(this, &COptAdvancedDlg::onClick_Revert);
	}

	virtual void OnInitDialog() override
	{
		TreeViewInit(GetDlgItem(m_hwnd, IDC_PLUS_CHECKTREE), CTranslator::TREE_MODPLUS, 0, FALSE);

		spnTimeout.SetRange(300, SRMSGSET_MSGTIMEOUT_MIN / 1000);
		spnTimeout.SetPosition(PluginConfig.m_MsgTimeout / 1000);

		spnHistSize.SetRange(255, 15);
		spnHistSize.SetPosition(M.GetByte("historysize", 0));
	}

	virtual void OnApply() override
	{
		TreeViewToDB(GetDlgItem(m_hwnd, IDC_PLUS_CHECKTREE), CTranslator::TREE_MODPLUS, SRMSGMOD_T, nullptr);

		int msgTimeout = 1000 * spnTimeout.GetPosition();
		PluginConfig.m_MsgTimeout = msgTimeout >= SRMSGSET_MSGTIMEOUT_MIN ? msgTimeout : SRMSGSET_MSGTIMEOUT_MIN;
		db_set_dw(0, SRMSGMOD, SRMSGSET_MSGTIMEOUT, PluginConfig.m_MsgTimeout);

		db_set_b(0, SRMSGMOD_T, "historysize", spnHistSize.GetPosition());
		PluginConfig.reloadAdv();
	}

	virtual void OnDestroy() override
	{
		TreeViewDestroy(GetDlgItem(m_hwnd, IDC_PLUS_CHECKTREE));
	}

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_NOTIFY && ((LPNMHDR)lParam)->idFrom == IDC_PLUS_CHECKTREE)
			return TreeViewHandleClick(m_hwnd, ((LPNMHDR)lParam)->hwndFrom, wParam, lParam);

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

	void onClick_Help(CCtrlButton*)
	{
		Utils_OpenUrl("https://wiki.miranda-ng.org/index.php?title=Plugin:TabSRMM/en/Advanced_tweaks");
	}

	void onClick_Revert(CCtrlButton*)
	{
		TOptionListItem *lvItems = CTranslator::getTree(CTranslator::TREE_MODPLUS);

		for (int i = 0; lvItems[i].szName != nullptr; i++)
			if (lvItems[i].uType == LOI_TYPE_SETTING)
				db_set_b(0, SRMSGMOD_T, (char *)lvItems[i].lParam, (BYTE)lvItems[i].id);
		TreeViewSetFromDB(GetDlgItem(m_hwnd, IDC_PLUS_CHECKTREE), CTranslator::TREE_MODPLUS, 0);
	}
};

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

struct
{
	BYTE sameAsFlags, sameAs;
	COLORREF colour;
	char size;
	BYTE style;
	BYTE charset;
	char szFace[LF_FACESIZE];
}
static fontSettings[MSGDLGFONTCOUNT + 1];

#include <poppack.h>

#define SRFONTSETTINGMODULE FONTMODULE

static int OptInitialise(WPARAM wParam, LPARAM lParam)
{
	if (PluginConfig.g_bPopupAvail)
		TN_OptionsInitialize(wParam, lParam);

	OPTIONSDIALOGPAGE odpnew = {};
	odpnew.position = 910000000;
	odpnew.hInstance = g_hInst;
	odpnew.flags = ODPF_BOLDGROUPS;
	odpnew.szTitle.a = LPGEN("Message sessions");

	odpnew.szTab.a = LPGEN("General");
	odpnew.pDialog = new COptMainDlg();
	Options_AddPage(wParam, &odpnew);

	odpnew.szTab.a = LPGEN("Tabs and layout");
	odpnew.pDialog = new COptTabbedDlg();
	Options_AddPage(wParam, &odpnew);

	odpnew.szTab.a = LPGEN("Containers");
	odpnew.pDialog = new COptContainersDlg();
	Options_AddPage(wParam, &odpnew);

	odpnew.szTab.a = LPGEN("Message log");
	odpnew.pDialog = new COptLogDlg();
	Options_AddPage(wParam, &odpnew);

	odpnew.szTab.a = LPGEN("Advanced tweaks");
	odpnew.pDialog = new COptAdvancedDlg();
	Options_AddPage(wParam, &odpnew);

	odpnew.szGroup.a = LPGEN("Message sessions");
	odpnew.szTitle.a = LPGEN("Typing notify");
	odpnew.pDialog = new COptTypingDlg();
	Options_AddPage(wParam, &odpnew);

	OPTIONSDIALOGPAGE odp = {};
	odp.hInstance = g_hInst;
	odp.flags = ODPF_BOLDGROUPS;
	odp.position = 910000000;

	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUP_OPT);
		odp.szTitle.a = LPGEN("Event notifications");
		odp.szGroup.a = LPGEN("Popups");
		odp.pfnDlgProc = DlgProcPopupOpts;
		Options_AddPage(wParam, &odp);
	}

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SKIN);
	odp.szTitle.a = LPGEN("Message window");
	odp.szTab.a = LPGEN("Load and apply");
	odp.pfnDlgProc = DlgProcSkinOpts;
	odp.szGroup.a = LPGEN("Skins");
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_TABCONFIG);
	odp.szTab.a = LPGEN("Window layout tweaks");
	odp.pfnDlgProc = DlgProcTabConfig;
	Options_AddPage(wParam, &odp);

	/* group chats */

	odp.szGroup.a = LPGEN("Message sessions");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS1);
	odp.szTitle.a = LPGEN("Group chats");
	odp.szTab.a = LPGEN("Settings");
	odp.pfnDlgProc = DlgProcOptions1;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS2);
	odp.szTab.a = LPGEN("Log formatting");
	odp.pfnDlgProc = DlgProcOptions2;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS3);
	odp.szTab.a = LPGEN("Events and filters");
	odp.pfnDlgProc = DlgProcOptions3;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS4);
	odp.szTab.a = LPGEN("Highlighting");
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
			return db_get_w(0, cb->dbModule, cb->dbSetting, cb->defValue);
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
		db_set_w(0, cb->dbModule, cb->dbSetting, (WORD)value);
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

int TSAPI InitOptions(void)
{
	HookEvent(ME_OPT_INITIALISE, OptInitialise);
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
				SetDlgItemText(hwndDlg, i, pcli->pfnGetStatusModeDescription(i, 0));
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
