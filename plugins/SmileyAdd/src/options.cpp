/*
Miranda NG SmileyAdd Plugin
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
Copyright (C) 2005-11 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2003-04 Rein-Peter de Boer

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

#include "stdafx.h"

OptionsType opt;

/////////////////////////////////////////////////////////////////////////////////////////

void OptionsType::Save(void)
{
	g_plugin.setByte("EnforceSpaces", EnforceSpaces);
	g_plugin.setByte("ScaleToTextheight", ScaleToTextheight);
	g_plugin.setByte("UseOneForAll", UseOneForAll);
	g_plugin.setByte("UsePhysProto", UsePhysProto);
	g_plugin.setByte("SurroundSmileyWithSpaces", SurroundSmileyWithSpaces);
	g_plugin.setByte("ScaleAllSmileys", ScaleAllSmileys);
	g_plugin.setByte("IEViewStyle", IEViewStyle);
	g_plugin.setByte("AnimateSel", AnimateSel);
	g_plugin.setByte("AnimateDlg", AnimateDlg);
	g_plugin.setByte("InputSmileys", InputSmileys);
	g_plugin.setByte("DCursorSmiley", DCursorSmiley);
	g_plugin.setByte("HQScaling", HQScaling);
	g_plugin.setDword("MaxCustomSmileySize", MaxCustomSmileySize);
	g_plugin.setDword("MinSmileySize", MinSmileySize);
	g_plugin.setByte("HorizontalSorting", HorizontalSorting);
}

void OptionsType::Load(void)
{
	EnforceSpaces = g_plugin.getByte("EnforceSpaces", FALSE) != 0;
	ScaleToTextheight = g_plugin.getByte("ScaleToTextheight", FALSE) != 0;
	UseOneForAll = g_plugin.getByte("UseOneForAll", TRUE) != 0;
	UsePhysProto = g_plugin.getByte("UsePhysProto", FALSE) != 0;
	SurroundSmileyWithSpaces = g_plugin.getByte("SurroundSmileyWithSpaces", FALSE) != 0;
	ScaleAllSmileys = g_plugin.getByte("ScaleAllSmileys", FALSE) != 0;
	IEViewStyle = g_plugin.getByte("IEViewStyle", FALSE) != 0;
	AnimateSel = g_plugin.getByte("AnimateSel", TRUE) != 0;
	AnimateDlg = g_plugin.getByte("AnimateDlg", TRUE) != 0;
	InputSmileys = g_plugin.getByte("InputSmileys", TRUE) != 0;
	DCursorSmiley = g_plugin.getByte("DCursorSmiley", FALSE) != 0;
	HQScaling = g_plugin.getByte("HQScaling", FALSE) != 0;

	SelWndBkgClr = g_plugin.getDword("SelWndBkgClr", GetSysColor(COLOR_WINDOW));
	MaxCustomSmileySize = g_plugin.getDword("MaxCustomSmileySize", 0);
	MinSmileySize = g_plugin.getDword("MinSmileySize", 0);
	HorizontalSorting = g_plugin.getByte("HorizontalSorting", 1) != 0;
}

void OptionsType::ReadPackFileName(CMStringW &filename, const CMStringW &name, const CMStringW &defaultFilename)
{
	CMStringW settingKey = name + L"-filename";

	ptrW tszValue(g_plugin.getWStringA(_T2A(settingKey.c_str())));
	if (tszValue != nullptr)
		filename = tszValue;
	else {
		if (defaultFilename.IsEmpty())
			filename.Format(L"%s\\Smileys\\nova\\default.msl", g_plugin.wszDefaultPath);
		else
			filename = defaultFilename;
	}
}

void OptionsType::WritePackFileName(const CMStringW &filename, const CMStringW &name)
{
	CMStringW settingKey = name + L"-filename";
	g_plugin.setWString(_T2A(settingKey.c_str()), filename.c_str());
}

void OptionsType::ReadCustomCategories(CMStringW &cats)
{
	ptrW tszValue(g_plugin.getWStringA("CustomCategories"));
	if (tszValue != nullptr)
		cats = tszValue;
}

void OptionsType::WriteCustomCategories(const CMStringW &cats)
{
	if (cats.IsEmpty())
		g_plugin.delSetting("CustomCategories");
	else
		g_plugin.setWString("CustomCategories", cats.c_str());
}

void OptionsType::ReadContactCategory(MCONTACT hContact, CMStringW &cats)
{
	ptrW tszValue(g_plugin.getWStringA(hContact, "CustomCategory"));
	if (tszValue != nullptr)
		cats = tszValue;
}

void OptionsType::WriteContactCategory(MCONTACT hContact, const CMStringW &cats)
{
	if (cats.IsEmpty())
		g_plugin.delSetting(hContact, "CustomCategory");
	else
		g_plugin.setWString(hContact, "CustomCategory", cats.c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////
// Basic class for options

class COptionsBaseDialog : public CDlgBase
{
	void OnFinish(CDlgBase*)
	{
		ProcessAllInputAreas(true);
		CloseSmileys();

		opt.Save();

		NotifyEventHooks(hEvent1, 0, 0);
		ProcessAllInputAreas(false);
	}

protected:
	COptionsBaseDialog(int dlgId) :
		CDlgBase(g_plugin, dlgId)
	{
		m_OnFinishWizard = Callback(this, &COptionsBaseDialog::OnFinish);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// General options page

class CGeneralOptions : public COptionsBaseDialog
{
	CCtrlHyperlink linkGetMore;

public:
	CGeneralOptions() :
		COptionsBaseDialog(IDD_OPT_GENERAL),
		linkGetMore(this, IDC_GETMORE, "https://miranda-ng.org/tags/smileyadd/")
	{}

	bool OnInitDialog() override
	{
		CheckDlgButton(m_hwnd, IDC_SPACES, opt.EnforceSpaces ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SCALETOTEXTHEIGHT, opt.ScaleToTextheight ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_APPENDSPACES, opt.SurroundSmileyWithSpaces ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SCALEALLSMILEYS, opt.ScaleAllSmileys ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_IEVIEWSTYLE, opt.IEViewStyle ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_ANIMATESEL, opt.AnimateSel ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_ANIMATEDLG, opt.AnimateDlg ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_INPUTSMILEYS, opt.InputSmileys ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_DCURSORSMILEY, opt.DCursorSmiley ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_HQSCALING, opt.HQScaling ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SORTING_HORIZONTAL, opt.HorizontalSorting ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(m_hwnd, IDC_MAXCUSTSPIN, UDM_SETRANGE32, 0, 99);
		SendDlgItemMessage(m_hwnd, IDC_MAXCUSTSPIN, UDM_SETPOS, 0, opt.MaxCustomSmileySize);
		SendDlgItemMessage(m_hwnd, IDC_MAXCUSTSMSZ, EM_LIMITTEXT, 2, 0);

		SendDlgItemMessage(m_hwnd, IDC_MINSPIN, UDM_SETRANGE32, 0, 99);
		SendDlgItemMessage(m_hwnd, IDC_MINSPIN, UDM_SETPOS, 0, opt.MinSmileySize);
		SendDlgItemMessage(m_hwnd, IDC_MINSMSZ, EM_LIMITTEXT, 2, 0);
		return true;
	}

	bool OnApply() override
	{
		opt.EnforceSpaces = IsDlgButtonChecked(m_hwnd, IDC_SPACES) == BST_CHECKED;
		opt.ScaleToTextheight = IsDlgButtonChecked(m_hwnd, IDC_SCALETOTEXTHEIGHT) == BST_CHECKED;
		opt.SurroundSmileyWithSpaces = IsDlgButtonChecked(m_hwnd, IDC_APPENDSPACES) == BST_CHECKED;
		opt.ScaleAllSmileys = IsDlgButtonChecked(m_hwnd, IDC_SCALEALLSMILEYS) == BST_CHECKED;
		opt.IEViewStyle = IsDlgButtonChecked(m_hwnd, IDC_IEVIEWSTYLE) == BST_CHECKED;
		opt.AnimateSel = IsDlgButtonChecked(m_hwnd, IDC_ANIMATESEL) == BST_CHECKED;
		opt.AnimateDlg = IsDlgButtonChecked(m_hwnd, IDC_ANIMATEDLG) == BST_CHECKED;
		opt.InputSmileys = IsDlgButtonChecked(m_hwnd, IDC_INPUTSMILEYS) == BST_CHECKED;
		opt.DCursorSmiley = IsDlgButtonChecked(m_hwnd, IDC_DCURSORSMILEY) == BST_CHECKED;
		opt.HQScaling = IsDlgButtonChecked(m_hwnd, IDC_HQSCALING) == BST_CHECKED;
		opt.HorizontalSorting = IsDlgButtonChecked(m_hwnd, IDC_SORTING_HORIZONTAL) == BST_CHECKED;

		opt.MaxCustomSmileySize = GetDlgItemInt(m_hwnd, IDC_MAXCUSTSMSZ, nullptr, FALSE);
		opt.MinSmileySize = GetDlgItemInt(m_hwnd, IDC_MINSMSZ, nullptr, FALSE);
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Categories options page

class CGategoriesOptions : public COptionsBaseDialog
{
	SmileyCategoryListType tmpsmcat;
	SmileyPackType smPack;

	bool BrowseForSmileyPacks(int item)
	{
		wchar_t filter[512], filename[MAX_PATH] = L"";
		mir_snwprintf(filter, L"%s (*.msl;*.asl;*.xep)%c*.msl;*.asl;*.xep%c%s (*.*)%c*.*%c", TranslateT("Smiley packs"), 0, 0, TranslateT("All files"), 0, 0);

		CMStringW inidir;
		SmileyCategoryType *smc = tmpsmcat.GetSmileyCategory(item);
		if (smc->GetFilename().IsEmpty())
			inidir = g_plugin.wszDefaultPath;
		else {
			inidir = VARSW(smc->GetFilename());
			inidir.Truncate(inidir.ReverseFind('\\'));
		}

		OPENFILENAME ofn = {};
		ofn.lpstrFile = filename;
		ofn.nMaxFile = _countof(filename);
		ofn.lpstrInitialDir = inidir.c_str();
		ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
		ofn.hwndOwner = m_hwnd;
		ofn.lpstrFilter = filter;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_READONLY | OFN_EXPLORER | OFN_LONGNAMES | OFN_NOCHANGEDIR;
		ofn.lpstrDefExt = L"msl";
		if (!GetOpenFileNameW(&ofn))
			return false;

		wchar_t szOutPath[MAX_PATH];
		PathToRelativeW(filename, szOutPath, g_plugin.wszDefaultPath);
		smc->SetFilename(szOutPath);
		UpdateControls();
		return true;
	}

	long GetSelProto(HTREEITEM hItem = nullptr)
	{
		TVITEMEX tvi = {};
		tvi.mask = TVIF_PARAM;
		tvi.hItem = hItem == nullptr ? categories.GetSelection() : hItem;
		categories.GetItem(&tvi);
		return (long)tvi.lParam;
	}

	void PopulateSmPackList(void)
	{
		categories.SelectItem(nullptr);
		categories.DeleteAllItems();

		UpdateVisibleSmPackList();

		TVINSERTSTRUCT tvi = {};
		tvi.hParent = TVI_ROOT;
		tvi.hInsertAfter = TVI_LAST;
		tvi.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_STATE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
		tvi.item.stateMask = TVIS_STATEIMAGEMASK | TVIS_SELECTED;

		auto &smc = *tmpsmcat.GetSmileyCategoryList();
		for (int i = 0; i < smc.getCount(); i++) {
			if (smc[i].IsVisible()) {
				tvi.item.pszText = (wchar_t *)smc[i].GetDisplayName().c_str();
				if (!smc[i].IsProto()) {
					tvi.item.iImage = 0;
					tvi.item.iSelectedImage = 0;
				}
				else {
					tvi.item.iImage = i;
					tvi.item.iSelectedImage = i;
				}
				tvi.item.lParam = i;
				tvi.item.state = INDEXTOSTATEIMAGEMASK(smPack.LoadSmileyFile(smc[i].GetFilename(), smc[i].GetDisplayName(), true, true) ? 2 : 1);
				categories.InsertItem(&tvi);

				smPack.Clear();
			}
		}
		categories.SelectItem(categories.GetRoot());
	}

	void UpdateControls(bool force = false)
	{
		const SmileyCategoryType *smc = tmpsmcat.GetSmileyCategory(GetSelProto());
		if (smc == nullptr)
			return;

		const CMStringW &smf = smc->GetFilename();
		SetDlgItemText(m_hwnd, IDC_FILENAME, smf);

		if (smPack.GetFilename() != smf || force)
			smPack.LoadSmileyFile(smf, smPack.GetName(), false, true);

		categories.SetCheckState(categories.GetSelection(), smPack.SmileyCount() != 0);

		SetDlgItemText(m_hwnd, IDC_LIBAUTHOR, smPack.GetAuthor().c_str());
		SetDlgItemText(m_hwnd, IDC_LIBVERSION, smPack.GetVersion().c_str());
		SetDlgItemText(m_hwnd, IDC_LIBNAME, TranslateW(smPack.GetName().c_str()));
	}

	void UpdateVisibleSmPackList()
	{
		bool useOne = !chkStdPack.GetState();
		bool usePhysProto = chkUsePhys.GetState();

		auto &smc = *tmpsmcat.GetSmileyCategoryList();
		for (auto &it : smc) {
			bool visiblecat = usePhysProto ? !it->IsAcc() : !it->IsPhysProto();
			bool visible = useOne ? !it->IsProto() : visiblecat;

			if (!visible && it->IsAcc() && !useOne) {
				CMStringW PhysProtoName = L"AllProto";
				CMStringW ProtoName = it->GetName();
				DBVARIANT dbv;
				if (db_get_ws(0, _T2A(ProtoName.GetBuffer()), "AM_BaseProto", &dbv) == 0) {
					ProtoName = dbv.pwszVal;
					db_free(&dbv);
				}
				else
					ProtoName.Empty();

				CMStringW FileName;
				if (!ProtoName.IsEmpty()) {
					PhysProtoName += ProtoName;
					SmileyCategoryType *scm = tmpsmcat.GetSmileyCategory(PhysProtoName);
					if (scm == nullptr)
						visible = false;
					else if (scm->GetFilename().IsEmpty())
						visible = true;
				}
			}

			it->SetVisible(visible);
		}
	}

	void UserAction(HTREEITEM hItem)
	{
		if (categories.GetCheckState(hItem)) {
			if (!BrowseForSmileyPacks(GetSelProto(hItem)))
				categories.SetCheckState(hItem, TRUE);
		}
		else tmpsmcat.GetSmileyCategory(GetSelProto(hItem))->ClearFilename();

		if (hItem == categories.GetSelection())
			UpdateControls();
		else
			categories.SelectItem(hItem);

		NotifyChange();
		PopulateSmPackList();
	}

	CCtrlEdit edtFilename;
	CCtrlCheck chkStdPack, chkUsePhys;
	CCtrlButton btnAdd, btnBrowse, btnDelete, btnPreview;
	CCtrlTreeView categories;

public:
	CGategoriesOptions() :
		COptionsBaseDialog(IDD_OPT_CATEGORIES), 
		btnAdd(this, IDC_ADDCATEGORY),
		btnBrowse(this, IDC_BROWSE),
		btnDelete(this, IDC_DELETECATEGORY),
		btnPreview(this, IDC_SMLOPTBUTTON),
		chkStdPack(this, IDC_USESTDPACK),
		chkUsePhys(this, IDC_USEPHYSPROTO),
		categories(this, IDC_CATEGORYLIST),
		edtFilename(this, IDC_FILENAME)
	{
		btnAdd.OnClick = Callback(this, &CGategoriesOptions::onClick_Add);
		btnBrowse.OnClick = Callback(this, &CGategoriesOptions::onClick_Browse);
		btnDelete.OnClick = Callback(this, &CGategoriesOptions::onClick_Delete);
		btnPreview.OnClick = Callback(this, &CGategoriesOptions::onClick_Preview);

		chkStdPack.OnChange = Callback(this, &CGategoriesOptions::onChange_StdPack);
		chkUsePhys.OnChange = Callback(this, &CGategoriesOptions::onChange_UsePhys);

		categories.OnSelChanged = Callback(this, &CGategoriesOptions::onSelectChange_Tree);
		categories.OnItemChanged = Callback(this, &CGategoriesOptions::onChange_Filename);
	}

	bool OnInitDialog() override
	{
		chkStdPack.SetState(!opt.UseOneForAll);
		chkUsePhys.SetState(opt.UsePhysProto);
		chkUsePhys.Enable(!opt.UseOneForAll);

		// Create and populate image list
		HIMAGELIST hImList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
			ILC_MASK | ILC_COLOR32, g_SmileyCategories.NumberOfSmileyCategories(), 0);

		tmpsmcat = g_SmileyCategories;

		auto &smc = *g_SmileyCategories.GetSmileyCategoryList();
		for (auto &it : smc) {
			HICON hIcon = nullptr;
			if (it->IsProto()) {
				hIcon = (HICON)CallProtoService(_T2A(it->GetName().c_str()), PS_LOADICON, PLI_PROTOCOL | PLIF_SMALL, 0);
				if (hIcon == nullptr || (INT_PTR)hIcon == CALLSERVICE_NOTFOUND)
					hIcon = (HICON)CallProtoService(_T2A(it->GetName().c_str()), PS_LOADICON, PLI_PROTOCOL, 0);
			}
			if (hIcon == nullptr || hIcon == (HICON)CALLSERVICE_NOTFOUND)
				hIcon = GetDefaultIcon();

			ImageList_AddIcon(hImList, hIcon);
			DestroyIcon(hIcon);
		}

		categories.SetImageList(hImList, TVSIL_NORMAL);

		PopulateSmPackList();
		return true;
	}

	bool OnApply() override
	{
		opt.UseOneForAll = !chkStdPack.GetState();
		opt.UsePhysProto = chkUsePhys.GetState();
	
		// Cleanup database
		CMStringW empty;
		auto &smc = *g_SmileyCategories.GetSmileyCategoryList();
		for (auto &it : smc)
			if (tmpsmcat.GetSmileyCategory(it->GetName()) == nullptr)
				opt.WritePackFileName(empty, it->GetName());

		g_SmileyCategories = tmpsmcat;
		g_SmileyCategories.SaveSettings();
		g_SmileyCategories.ClearAndLoadAll();

		smPack.LoadSmileyFile(tmpsmcat.GetSmileyCategory(GetSelProto())->GetFilename(), tmpsmcat.GetSmileyCategory(GetSelProto())->GetDisplayName(), false, true);
		return true;
	}

	void OnDestroy() override
	{
		HIMAGELIST hImList = categories.GetImageList(TVSIL_NORMAL);
		ImageList_Destroy(hImList);
	}

	INT_PTR DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		if (uMsg == WM_COMMAND && LOWORD(wParam) == IDC_FILENAME && HIWORD(wParam) == EN_KILLFOCUS)
			onChange_Filename(0);

		return COptionsBaseDialog::DlgProc(uMsg, wParam, lParam);
	}

	void onClick_Add(CCtrlButton*)
	{
		wchar_t cat[30];
		GetDlgItemText(m_hwnd, IDC_NEWCATEGORY, cat, _countof(cat));
		CMStringW catd = cat;

		if (!catd.IsEmpty()) {
			tmpsmcat.AddCategory(cat, catd, smcCustom);

			PopulateSmPackList();
			NotifyChange();
		}
	}

	void onClick_Browse(CCtrlButton*)
	{
		if (BrowseForSmileyPacks(GetSelProto())) {
			UpdateControls(true);
			NotifyChange();
		}
	}

	void onClick_Delete(CCtrlButton*)
	{
		if (tmpsmcat.DeleteCustomCategory(GetSelProto())) {
			PopulateSmPackList();
			NotifyChange();
		}
	}

	void onClick_Preview(CCtrlButton*)
	{
		RECT rect;
		GetWindowRect(GetDlgItem(m_hwnd, IDC_SMLOPTBUTTON), &rect);

		SmileyToolWindowParam *stwp = new SmileyToolWindowParam;
		stwp->pSmileyPack = &smPack;
		stwp->hWndParent = m_hwnd;
		stwp->hWndTarget = nullptr;
		stwp->targetMessage = 0;
		stwp->targetWParam = 0;
		stwp->xPosition = rect.left;
		stwp->yPosition = rect.bottom + 4;
		stwp->direction = 1;
		mir_forkThread<SmileyToolWindowParam>(SmileyToolThread, stwp);
	}

	void onChange_Filename(CCtrlEdit*)
	{
		wchar_t str[MAX_PATH];
		GetDlgItemTextW(m_hwnd, IDC_FILENAME, str, _countof(str));

		SmileyCategoryType *smc = tmpsmcat.GetSmileyCategory(GetSelProto());
		if (smc->GetFilename() != str) {
			smc->SetFilename(str);
			UpdateControls();
		}
	}

	void onChange_StdPack(CCtrlCheck*)
	{
		chkUsePhys.Enable(chkStdPack.GetState());
		PopulateSmPackList();
		NotifyChange();
	}

	void onChange_UsePhys(CCtrlCheck*)
	{
		PopulateSmPackList();
		NotifyChange();
	}

	void onSelectChange_Tree(CCtrlTreeView::TEventInfo *ev)
	{
		if (ev->nmtv->itemNew.state & TVIS_SELECTED)
			UpdateControls();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

int SmileysOptionsInitialize(WPARAM addInfo, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.position = 910000000;
	odp.szTitle.a = LPGEN("Smileys");
	odp.szGroup.a = LPGEN("Customize");

	odp.szTab.a = LPGEN("General");
	odp.pDialog = new CGeneralOptions();
	g_plugin.addOptions(addInfo, &odp);

	odp.szTab.a = LPGEN("Categories");
	odp.pDialog = new CGategoriesOptions();
	g_plugin.addOptions(addInfo, &odp);
	return 0;
}
