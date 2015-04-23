/*
Miranda SmileyAdd Plugin
Copyright (C) 2005 - 2011 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2003 - 2004 Rein-Peter de Boer

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "general.h"

OptionsType opt;

#define UM_CHECKSTATECHANGE (WM_USER + 100)

class OptionsDialogType
{
private:
	HWND m_hwndDialog;
	SmileyCategoryListType tmpsmcat;
	SmileyPackType smPack;

	void InitDialog(void);
	void DestroyDialog(void);
	void AddCategory(void);
	void ApplyChanges(void);
	void UpdateControls(bool force = false);
	void SetChanged(void);
	bool BrowseForSmileyPacks(int item);
	void FilenameChanged(void);
	void ShowSmileyPreview(void);
	void PopulateSmPackList(void);
	void UserAction(HTREEITEM hItem);
	long GetSelProto(HTREEITEM hItem = NULL);

public:
	OptionsDialogType(HWND hWnd) { m_hwndDialog = hWnd; }
	BOOL DialogProcedure(UINT msg, WPARAM wParam, LPARAM lParam);
};

static INT_PTR CALLBACK DlgProcSmileysOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

//Init and de-init functions, called from main
int SmileysOptionsInitialize(WPARAM addInfo, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 910000000;
	odp.hInstance = g_hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SMILEYS);
	odp.pszTitle = LPGEN("Smileys");
	odp.pszGroup = LPGEN("Customize");
	odp.pfnDlgProc = DlgProcSmileysOptions;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(addInfo, &odp);
	return 0;
}


//
// dialog procedure for the options dialog. creates or 
// retrieves the options class and calls it
//
static INT_PTR CALLBACK DlgProcSmileysOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	OptionsDialogType *pOD = (OptionsDialogType*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	if (pOD == NULL) {
		pOD = new OptionsDialogType(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pOD);
	}

	INT_PTR Result = pOD->DialogProcedure(msg, wParam, lParam);
	SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, Result); 

	if (msg == WM_NCDESTROY)
		delete pOD;

	return Result;
}


//OptionsDialog class functions
BOOL OptionsDialogType::DialogProcedure(UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL Result = FALSE;

	switch(msg) {
	case WM_INITDIALOG:
		InitDialog();
		Result = TRUE;
		break;

	case WM_DESTROY:
		DestroyDialog();
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_FILENAME:
			switch(HIWORD(wParam)) {
			case EN_KILLFOCUS:
				FilenameChanged();
				break;

			case EN_CHANGE:
				if (GetFocus() == (HWND)lParam) SetChanged();
				break;
			}
			break;

		case IDC_BROWSE:
			if (HIWORD(wParam) == BN_CLICKED)
				if (BrowseForSmileyPacks(GetSelProto())) {
					UpdateControls(true); 
					SetChanged();
				}
			break;

		case IDC_SMLOPTBUTTON:
			if (HIWORD(wParam) == BN_CLICKED)
				ShowSmileyPreview();
			break;

		case IDC_USESTDPACK:
			if (HIWORD(wParam) == BN_CLICKED) {
				PopulateSmPackList(); 
				SetChanged();
			}
			break;

		case IDC_PLUGENABLED:
			if (HIWORD(wParam) == BN_CLICKED) {
				BOOL en = IsDlgButtonChecked(m_hwndDialog, IDC_PLUGENABLED) == BST_UNCHECKED;
				EnableWindow(GetDlgItem(m_hwndDialog, IDC_SMLBUT), en);
				SetChanged();
			}
			break;

		case IDC_ADDCATEGORY:
			if (HIWORD(wParam) == BN_CLICKED)
				AddCategory();
			break;

		case IDC_DELETECATEGORY:
			if (HIWORD(wParam) == BN_CLICKED)
				if (tmpsmcat.DeleteCustomCategory(GetSelProto())) {
					PopulateSmPackList();
					SetChanged();
				}
			break;

		case IDC_SPACES:
		case IDC_USEPHYSPROTO:
		case IDC_SCALETOTEXTHEIGHT:
		case IDC_APPENDSPACES:
		case IDC_SMLBUT:
		case IDC_SCALEALLSMILEYS:
		case IDC_IEVIEWSTYLE:
		case IDC_ANIMATESEL:
		case IDC_ANIMATEDLG:
		case IDC_INPUTSMILEYS:
		case IDC_DCURSORSMILEY:
		case IDC_DISABLECUSTOM:
		case IDC_HQSCALING:
			if (HIWORD(wParam) == BN_CLICKED)
				SetChanged();
			break;

		case IDC_SELCLR:
			if (HIWORD(wParam) == CPN_COLOURCHANGED)
				SetChanged();
			break;

		case IDC_MAXCUSTSMSZ:
		case IDC_MINSMSZ:
			if (HIWORD(wParam) == EN_CHANGE && GetFocus() == (HWND)lParam)
				SetChanged();
			break;
		}
		break;

	case UM_CHECKSTATECHANGE:
		UserAction((HTREEITEM)lParam);
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				ApplyChanges();
				break;
			}
			break;

		case IDC_CATEGORYLIST:
			switch (((LPNMHDR)lParam)->code) {
			case NM_CLICK:
				{
					TVHITTESTINFO ht = {0};

					DWORD dwpos = GetMessagePos();
					POINTSTOPOINT(ht.pt, MAKEPOINTS(dwpos));
					MapWindowPoints(HWND_DESKTOP, ((LPNMHDR)lParam)->hwndFrom, &ht.pt, 1);

					TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &ht);
					if (TVHT_ONITEM & ht.flags)
						FilenameChanged();
					if (TVHT_ONITEMSTATEICON & ht.flags)
						PostMessage(m_hwndDialog, UM_CHECKSTATECHANGE, 0, (LPARAM)ht.hItem);
				}

			case TVN_KEYDOWN:
				if (((LPNMTVKEYDOWN) lParam)->wVKey == VK_SPACE)
					PostMessage(m_hwndDialog, UM_CHECKSTATECHANGE, 0, 
					(LPARAM)TreeView_GetSelection(((LPNMHDR)lParam)->hwndFrom));
				break;

			case TVN_SELCHANGEDA:
			case TVN_SELCHANGEDW:
				LPNMTREEVIEW pnmtv = (LPNMTREEVIEW) lParam;
				if (pnmtv->itemNew.state & TVIS_SELECTED)
					UpdateControls();
			}
		}
		break;
	}

	return Result;
}

void OptionsDialogType::AddCategory(void)
{
	TCHAR cat[30];

	GetDlgItemText(m_hwndDialog, IDC_NEWCATEGORY, cat, SIZEOF(cat)); 
	CMString catd = cat;

	if (!catd.IsEmpty()) {
		tmpsmcat.AddCategory(cat, catd, smcCustom);

		PopulateSmPackList();
		SetChanged();
	}
}

void OptionsDialogType::UserAction(HTREEITEM hItem)
{
	HWND hLstView = GetDlgItem(m_hwndDialog, IDC_CATEGORYLIST);

	if (TreeView_GetCheckState(hLstView, hItem)) {
		if (!BrowseForSmileyPacks(GetSelProto(hItem)))
			TreeView_SetCheckState(hLstView, hItem, TRUE)
	}
	else tmpsmcat.GetSmileyCategory(GetSelProto(hItem))->ClearFilename();

	if (hItem == TreeView_GetSelection(hLstView))
		UpdateControls();
	else
		TreeView_SelectItem(hLstView, hItem);

	SetChanged();
}


void OptionsDialogType::SetChanged(void)
{
	SendMessage(GetParent(m_hwndDialog), PSM_CHANGED, 0, 0);
}


void OptionsDialogType::UpdateControls(bool force)
{
	const SmileyCategoryType* smc = tmpsmcat.GetSmileyCategory(GetSelProto());
	if (smc == NULL) return;

	const CMString& smf = smc->GetFilename();

	SetDlgItemText(m_hwndDialog, IDC_FILENAME, smf.c_str());

	if (smPack.GetFilename() != smf || force)
		smPack.LoadSmileyFile(smf, false, true);

	HWND hLstView = GetDlgItem(m_hwndDialog, IDC_CATEGORYLIST);
	TreeView_SetCheckState(hLstView, TreeView_GetSelection(hLstView), smPack.SmileyCount() != 0);

	SetDlgItemText(m_hwndDialog, IDC_LIBAUTHOR, smPack.GetAuthor().c_str());
	SetDlgItemText(m_hwndDialog, IDC_LIBVERSION, smPack.GetVersion().c_str());
	SetDlgItemText(m_hwndDialog, IDC_LIBNAME, TranslateTS(smPack.GetName().c_str()));
}


long OptionsDialogType::GetSelProto(HTREEITEM hItem)
{
	HWND hLstView = GetDlgItem(m_hwndDialog, IDC_CATEGORYLIST);
	TVITEM tvi = {0};

	tvi.mask = TVIF_PARAM;
	tvi.hItem = hItem == NULL ? TreeView_GetSelection(hLstView) : hItem;

	TreeView_GetItem(hLstView, &tvi);

	return (long)tvi.lParam;
}


void OptionsDialogType::PopulateSmPackList(void)
{
	bool useOne = IsDlgButtonChecked(m_hwndDialog, IDC_USESTDPACK) == BST_UNCHECKED;

	HWND hLstView = GetDlgItem(m_hwndDialog, IDC_CATEGORYLIST);

	TreeView_SelectItem(hLstView, NULL);
	TreeView_DeleteAllItems(hLstView);

	TVINSERTSTRUCT tvi = {0};
	tvi.hParent = TVI_ROOT;
	tvi.hInsertAfter = TVI_LAST;
	tvi.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_STATE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	tvi.item.stateMask = TVIS_STATEIMAGEMASK | TVIS_SELECTED;

	SmileyCategoryListType::SmileyCategoryVectorType& smc = *tmpsmcat.GetSmileyCategoryList();
	for (int i=0; i < smc.getCount(); i++) {
		if (!useOne || !smc[i].IsProto()) {
			tvi.item.pszText = (TCHAR*)smc[i].GetDisplayName().c_str();
			if (!smc[i].IsProto()) {
				tvi.item.iImage = 0;
				tvi.item.iSelectedImage = 0;
			}
			else {
				tvi.item.iImage = i;
				tvi.item.iSelectedImage = i;
			}
			tvi.item.lParam = i;
			tvi.item.state = INDEXTOSTATEIMAGEMASK(smPack.LoadSmileyFile(smc[i].GetFilename(), true, true) ? 2 : 1);
			TreeView_InsertItem(hLstView, &tvi);

			smPack.Clear();
		}
	}
	TreeView_SelectItem(hLstView, TreeView_GetRoot(hLstView));
}

void OptionsDialogType::InitDialog(void)
{
	TranslateDialogDefault(m_hwndDialog);

	CheckDlgButton(m_hwndDialog, IDC_PLUGENABLED, opt.PluginSupportEnabled ? BST_UNCHECKED : BST_CHECKED);
	CheckDlgButton(m_hwndDialog, IDC_SPACES, opt.EnforceSpaces ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(m_hwndDialog, IDC_SCALETOTEXTHEIGHT, opt.ScaleToTextheight ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(m_hwndDialog, IDC_USESTDPACK, opt.UseOneForAll ? BST_UNCHECKED : BST_CHECKED);
	CheckDlgButton(m_hwndDialog, IDC_USEPHYSPROTO, opt.UsePhysProto ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(m_hwndDialog, IDC_APPENDSPACES, opt.SurroundSmileyWithSpaces ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(m_hwndDialog, IDC_SCALEALLSMILEYS, opt.ScaleAllSmileys ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(m_hwndDialog, IDC_IEVIEWSTYLE, opt.IEViewStyle ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(m_hwndDialog, IDC_ANIMATESEL, opt.AnimateSel ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(m_hwndDialog, IDC_ANIMATEDLG, opt.AnimateDlg ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(m_hwndDialog, IDC_INPUTSMILEYS, opt.InputSmileys ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(m_hwndDialog, IDC_DCURSORSMILEY, opt.DCursorSmiley ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(m_hwndDialog, IDC_DISABLECUSTOM, opt.DisableCustom ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(m_hwndDialog, IDC_HQSCALING, opt.HQScaling ? BST_CHECKED : BST_UNCHECKED);

	SendDlgItemMessage(m_hwndDialog, IDC_SMLBUT, CB_ADDSTRING, 0, (LPARAM) TranslateT("Off"));  
	SendDlgItemMessage(m_hwndDialog, IDC_SMLBUT, CB_ADDSTRING, 0, (LPARAM) TranslateT("Top"));  

	if (IsOldSrmm())
		SendDlgItemMessage(m_hwndDialog, IDC_SMLBUT, CB_ADDSTRING, 0, (LPARAM) TranslateT("Bottom"));  

	SendDlgItemMessage(m_hwndDialog, IDC_SMLBUT, CB_SETCURSEL, opt.ButtonStatus, 0);  
	EnableWindow(GetDlgItem(m_hwndDialog, IDC_SMLBUT), opt.PluginSupportEnabled);

	SendDlgItemMessage(m_hwndDialog, IDC_MAXCUSTSPIN, UDM_SETRANGE32, 0, 99);
	SendDlgItemMessage(m_hwndDialog, IDC_MAXCUSTSPIN, UDM_SETPOS, 0, opt.MaxCustomSmileySize);
	SendDlgItemMessage(m_hwndDialog, IDC_MAXCUSTSMSZ, EM_LIMITTEXT, 2, 0);

	SendDlgItemMessage(m_hwndDialog, IDC_MINSPIN, UDM_SETRANGE32, 0, 99);
	SendDlgItemMessage(m_hwndDialog, IDC_MINSPIN, UDM_SETPOS, 0, opt.MinSmileySize);
	SendDlgItemMessage(m_hwndDialog, IDC_MINSMSZ, EM_LIMITTEXT, 2, 0);

	SendDlgItemMessage(m_hwndDialog, IDC_SELCLR, CPM_SETCOLOUR, 0, opt.SelWndBkgClr);
	SendDlgItemMessage(m_hwndDialog, IDC_SELCLR, CPM_SETDEFAULTCOLOUR, 0, GetSysColor(COLOR_WINDOW));

	// Create and populate image list
	HIMAGELIST hImList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
		ILC_MASK | ILC_COLOR32, g_SmileyCategories.NumberOfSmileyCategories(), 0);

	tmpsmcat = g_SmileyCategories;

	SmileyCategoryListType::SmileyCategoryVectorType& smc = *g_SmileyCategories.GetSmileyCategoryList();
	for (int i=0; i < smc.getCount(); i++) {
		HICON hIcon = NULL;
		if (smc[i].IsProto()) {
			hIcon = (HICON)CallProtoService(T2A_SM(smc[i].GetName().c_str()), PS_LOADICON, PLI_PROTOCOL | PLIF_SMALL, 0);
			if (hIcon == NULL || (INT_PTR)hIcon == CALLSERVICE_NOTFOUND) 
				hIcon = (HICON)CallProtoService(T2A_SM(smc[i].GetName().c_str()), PS_LOADICON, PLI_PROTOCOL, 0);
		} 
		if (hIcon == NULL || hIcon == (HICON)CALLSERVICE_NOTFOUND) 
			hIcon = GetDefaultIcon();

		ImageList_AddIcon(hImList, hIcon);
		DestroyIcon(hIcon);
	}

	HWND hLstView = GetDlgItem(m_hwndDialog, IDC_CATEGORYLIST);
	TreeView_SetImageList(hLstView, hImList, TVSIL_NORMAL);

	PopulateSmPackList();
}

void OptionsDialogType::DestroyDialog(void)
{
	HWND hLstView = GetDlgItem(m_hwndDialog, IDC_CATEGORYLIST);
	HIMAGELIST hImList = TreeView_SetImageList(hLstView, NULL, TVSIL_NORMAL);
	ImageList_Destroy(hImList);
}


void OptionsDialogType::ApplyChanges(void)
{
	ProcessAllInputAreas(true);
	CloseSmileys();

	opt.PluginSupportEnabled = IsDlgButtonChecked(m_hwndDialog, IDC_PLUGENABLED) == BST_UNCHECKED;
	opt.EnforceSpaces = IsDlgButtonChecked(m_hwndDialog, IDC_SPACES) == BST_CHECKED;
	opt.ScaleToTextheight = IsDlgButtonChecked(m_hwndDialog, IDC_SCALETOTEXTHEIGHT) == BST_CHECKED;
	opt.UseOneForAll = IsDlgButtonChecked(m_hwndDialog, IDC_USESTDPACK) == BST_UNCHECKED;
	opt.UsePhysProto = IsDlgButtonChecked(m_hwndDialog, IDC_USEPHYSPROTO) == BST_CHECKED;
	opt.SurroundSmileyWithSpaces = IsDlgButtonChecked(m_hwndDialog, IDC_APPENDSPACES) == BST_CHECKED;
	opt.ScaleAllSmileys = IsDlgButtonChecked(m_hwndDialog, IDC_SCALEALLSMILEYS) == BST_CHECKED;
	opt.IEViewStyle = IsDlgButtonChecked(m_hwndDialog, IDC_IEVIEWSTYLE) == BST_CHECKED;
	opt.AnimateSel = IsDlgButtonChecked(m_hwndDialog, IDC_ANIMATESEL) == BST_CHECKED;
	opt.AnimateDlg = IsDlgButtonChecked(m_hwndDialog, IDC_ANIMATEDLG) == BST_CHECKED;
	opt.InputSmileys = IsDlgButtonChecked(m_hwndDialog, IDC_INPUTSMILEYS) == BST_CHECKED;
	opt.DCursorSmiley = IsDlgButtonChecked(m_hwndDialog, IDC_DCURSORSMILEY) == BST_CHECKED;
	opt.DisableCustom = IsDlgButtonChecked(m_hwndDialog, IDC_DISABLECUSTOM) == BST_CHECKED;
	opt.HQScaling = IsDlgButtonChecked(m_hwndDialog, IDC_HQSCALING) == BST_CHECKED;

	opt.ButtonStatus = (unsigned)SendDlgItemMessage(m_hwndDialog, IDC_SMLBUT, CB_GETCURSEL, 0, 0);  
	opt.SelWndBkgClr = (unsigned)SendDlgItemMessage(m_hwndDialog, IDC_SELCLR, CPM_GETCOLOUR, 0, 0);
	opt.MaxCustomSmileySize = GetDlgItemInt(m_hwndDialog, IDC_MAXCUSTSMSZ, NULL, FALSE);
	opt.MinSmileySize = GetDlgItemInt(m_hwndDialog, IDC_MINSMSZ, NULL, FALSE);

	opt.Save();

	// Cleanup database
	SmileyCategoryListType::SmileyCategoryVectorType& smc = *g_SmileyCategories.GetSmileyCategoryList();
	for (int i=0; i < smc.getCount(); i++) {
		if (tmpsmcat.GetSmileyCategory(smc[i].GetName()) == NULL) {
			CMString empty;
			opt.WritePackFileName(empty, smc[i].GetName());
		}
	}

	g_SmileyCategories = tmpsmcat;
	g_SmileyCategories.SaveSettings();
	g_SmileyCategories.ClearAndLoadAll();

	smPack.LoadSmileyFile(tmpsmcat.GetSmileyCategory(GetSelProto())->GetFilename(), false);

	NotifyEventHooks(hEvent1, 0, 0);
	ProcessAllInputAreas(false);
}

bool OptionsDialogType::BrowseForSmileyPacks(int item)
{
	OPENFILENAME ofn = {0};

	TCHAR filename[MAX_PATH] = _T("");
	ofn.lpstrFile = filename;
	ofn.nMaxFile = SIZEOF(filename);

	CMString inidir;
	SmileyCategoryType* smc = tmpsmcat.GetSmileyCategory(item); 
	if (smc->GetFilename().IsEmpty())
		pathToAbsolute(_T("Smileys"), inidir);
	else {
		pathToAbsolute(smc->GetFilename(), inidir);
		inidir.Truncate(inidir.ReverseFind('\\'));
	}

	ofn.lpstrInitialDir = inidir.c_str();
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.hwndOwner = m_hwndDialog;

	TCHAR filter[512], *pfilter;
	_tcscpy(filter, TranslateT("Smiley packs"));
	mir_tstrcat(filter, _T(" (*.msl;*.asl;*.xep)"));
	pfilter = filter + _tcslen(filter) + 1;
	_tcscpy(pfilter, _T("*.msl;*.asl;*.xep"));
	pfilter = pfilter + _tcslen(pfilter) + 1;
	_tcscpy(pfilter, TranslateT("All files"));
	mir_tstrcat(pfilter, _T(" (*.*)"));
	pfilter = pfilter + _tcslen(pfilter) + 1;
	_tcscpy(pfilter, _T("*.*"));
	pfilter = pfilter + _tcslen(pfilter) + 1;
	*pfilter = '\0';  
	ofn.lpstrFilter = filter;

	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_READONLY |
		OFN_EXPLORER | OFN_LONGNAMES | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = _T("msl");

	if (GetOpenFileName(&ofn)) {
		CMString relpath;
		pathToRelative(filename, relpath);
		smc->SetFilename(relpath);

		return true;
	}

	return false;
}

void OptionsDialogType::FilenameChanged(void)
{
	TCHAR str[MAX_PATH];
	GetDlgItemText(m_hwndDialog, IDC_FILENAME, str, SIZEOF(str));

	SmileyCategoryType* smc = tmpsmcat.GetSmileyCategory(GetSelProto()); 
	if (smc->GetFilename() != str) {
		CMString temp(str);
		smc->SetFilename(temp);
		UpdateControls();
	}
}

void OptionsDialogType::ShowSmileyPreview(void)
{
	RECT rect;
	GetWindowRect(GetDlgItem(m_hwndDialog, IDC_SMLOPTBUTTON), &rect);

	SmileyToolWindowParam *stwp = new SmileyToolWindowParam;
	stwp->pSmileyPack = &smPack;
	stwp->hWndParent = m_hwndDialog;
	stwp->hWndTarget = NULL;
	stwp->targetMessage = 0;
	stwp->targetWParam = 0;
	stwp->xPosition = rect.left;
	stwp->yPosition = rect.bottom + 4;
	stwp->direction = 1;
	stwp->hContact = NULL;

	mir_forkthread(SmileyToolThread, stwp);
}

void OptionsType::Save(void)
{
	db_set_b(NULL, "SmileyAdd", "PluginSupportEnabled", PluginSupportEnabled);
	db_set_b(NULL, "SmileyAdd", "EnforceSpaces", EnforceSpaces);
	db_set_b(NULL, "SmileyAdd", "ScaleToTextheight", ScaleToTextheight);
	db_set_b(NULL, "SmileyAdd", "UseOneForAll", UseOneForAll);
	db_set_b(NULL, "SmileyAdd", "UsePhysProto", UsePhysProto);
	db_set_b(NULL, "SmileyAdd", "SurroundSmileyWithSpaces", SurroundSmileyWithSpaces);
	db_set_b(NULL, "SmileyAdd", "ScaleAllSmileys", ScaleAllSmileys);
	db_set_b(NULL, "SmileyAdd", "IEViewStyle", IEViewStyle);
	db_set_b(NULL, "SmileyAdd", "AnimateSel", AnimateSel);
	db_set_b(NULL, "SmileyAdd", "AnimateDlg", AnimateDlg);
	db_set_b(NULL, "SmileyAdd", "InputSmileys", InputSmileys);
	db_set_b(NULL, "SmileyAdd", "DCursorSmiley", DCursorSmiley);
	db_set_b(NULL, "SmileyAdd", "DisableCustom", DisableCustom);
	db_set_b(NULL, "SmileyAdd", "HQScaling", HQScaling);
	db_set_b(NULL, "SmileyAdd", "ButtonStatus", (BYTE)ButtonStatus);
	db_set_dw(NULL, "SmileyAdd", "SelWndBkgClr", SelWndBkgClr);
	db_set_dw(NULL, "SmileyAdd", "MaxCustomSmileySize", MaxCustomSmileySize);
	db_set_dw(NULL, "SmileyAdd", "MinSmileySize", MinSmileySize);
}

void OptionsType::Load(void)
{
	PluginSupportEnabled = db_get_b(NULL, "SmileyAdd", "PluginSupportEnabled", TRUE) != 0;
	EnforceSpaces = db_get_b(NULL, "SmileyAdd", "EnforceSpaces", FALSE) != 0;
	ScaleToTextheight = db_get_b(NULL, "SmileyAdd", "ScaleToTextheight", FALSE) != 0;
	UseOneForAll = db_get_b(NULL, "SmileyAdd", "UseOneForAll", TRUE) != 0;
	UsePhysProto = db_get_b(NULL, "SmileyAdd", "UsePhysProto", FALSE) != 0;
	SurroundSmileyWithSpaces = 
		db_get_b(NULL, "SmileyAdd", "SurroundSmileyWithSpaces", FALSE) != 0;
	ScaleAllSmileys = db_get_b(NULL, "SmileyAdd", "ScaleAllSmileys", FALSE) != 0;
	IEViewStyle = db_get_b(NULL, "SmileyAdd", "IEViewStyle", FALSE) != 0;
	AnimateSel = db_get_b(NULL, "SmileyAdd", "AnimateSel", TRUE) != 0;
	AnimateDlg = db_get_b(NULL, "SmileyAdd", "AnimateDlg", TRUE) != 0;
	InputSmileys = db_get_b(NULL, "SmileyAdd", "InputSmileys", TRUE) != 0;
	DCursorSmiley = db_get_b(NULL, "SmileyAdd", "DCursorSmiley", FALSE) != 0;
	DisableCustom = db_get_b(NULL, "SmileyAdd", "DisableCustom", FALSE) != 0;
	HQScaling = db_get_b(NULL, "SmileyAdd", "HQScaling", FALSE) != 0;

	ButtonStatus = db_get_b(NULL, "SmileyAdd", "ButtonStatus", 1);
	SelWndBkgClr = db_get_dw(NULL, "SmileyAdd", "SelWndBkgClr", GetSysColor(COLOR_WINDOW));
	MaxCustomSmileySize = db_get_dw(NULL, "SmileyAdd", "MaxCustomSmileySize", 0);
	MinSmileySize = db_get_dw(NULL, "SmileyAdd", "MinSmileySize", 0);
}


void OptionsType::ReadPackFileName(CMString& filename, const CMString& name, const CMString& defaultFilename)
{
	CMString settingKey = name + _T("-filename");

	DBVARIANT dbv;
	if (!db_get_ts(NULL, "SmileyAdd", T2A_SM(settingKey.c_str()), &dbv)) {
		filename = dbv.ptszVal;
		db_free(&dbv);
	}
	else filename = defaultFilename;
}

void OptionsType::WritePackFileName(const CMString& filename, const CMString& name)
{
	CMString settingKey = name + _T("-filename");
	db_set_ts(NULL, "SmileyAdd", T2A_SM(settingKey.c_str()), filename.c_str());
}


void OptionsType::ReadCustomCategories(CMString& cats)
{
	DBVARIANT dbv;
	INT_PTR res = db_get_ts(NULL, "SmileyAdd", "CustomCategories", &dbv);
	if (res == 0) {
		cats = dbv.ptszVal;
		db_free(&dbv);
	}
}


void OptionsType::WriteCustomCategories(const CMString& cats)
{
	if (cats.IsEmpty())
		db_unset(NULL, "SmileyAdd", "CustomCategories");
	else
		db_set_ts(NULL, "SmileyAdd", "CustomCategories",	cats.c_str());
}


void OptionsType::ReadContactCategory(MCONTACT hContact, CMString& cats)
{
	DBVARIANT dbv;
	INT_PTR res = db_get_ts(hContact, "SmileyAdd", "CustomCategory", &dbv);
	if (res == 0) {
		cats = dbv.ptszVal;
		db_free(&dbv);
	}
}


void OptionsType::WriteContactCategory(MCONTACT hContact, const CMString& cats)
{
	if (cats.IsEmpty())
		db_unset(hContact, "SmileyAdd", "CustomCategory");
	else
		db_set_ts(hContact, "SmileyAdd", "CustomCategory",	cats.c_str());
}
