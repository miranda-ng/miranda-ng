/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-22 Miranda NG team,
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

#define DM_GETSTATUSMASK (WM_USER + 10)

/////////////////////////////////////////////////////////////////////////////////////////
// options dialog for setting up tab options

/////////////////////////////////////////////////////////////////////////////////////////
// controls to disable when loading or unloading a skin is not possible (because
// of at least one message window being open).

static UINT _ctrls[] = { IDC_SKINNAME, IDC_RESCANSKIN, IDC_RESCANSKIN, IDC_RELOADSKIN, 0 };

class CSkinOptsDlg : public CDlgBase
{
	// mir_free the item extra data (used to store the skin filenames for each entry).
	void FreeComboData()
	{
		LRESULT lr = cmbSkins.GetCount();
		for (int i = 1; i < lr; i++) {
			void *idata = (void *)cmbSkins.GetItemData(i);
			if (idata && idata != (void *)CB_ERR)
				mir_free(idata);
		}
	}

	// scan the skin root folder for subfolder(s).Each folder is supposed to contain a single
	// skin. This function won't dive deeper into the folder structure, so the folder
	// structure for any VALID skin should be:
	// $SKINS_ROOT/skin_folder/skin_name.tsk
	//
	// By default, $SKINS_ROOT is set to %miranda_userdata% or custom folder
	// selected by the folders plugin.

	void RescanSkins()
	{
		wchar_t tszSkinRoot[MAX_PATH], tszFindMask[MAX_PATH];
		wcsncpy_s(tszSkinRoot, M.getSkinPath(), _TRUNCATE);

		SetDlgItemTextW(m_hwnd, IDC_SKINROOTFOLDER, tszSkinRoot);
		mir_snwprintf(tszFindMask, L"%s*.*", tszSkinRoot);

		cmbSkins.ResetContent();
		cmbSkins.AddString(TranslateT("<no skin>"));

		WIN32_FIND_DATA fd = {};
		HANDLE h = FindFirstFile(tszFindMask, &fd);
		while (h != INVALID_HANDLE_VALUE) {
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && fd.cFileName[0] != '.') {
				wchar_t	tszSubDir[MAX_PATH];
				mir_snwprintf(tszSubDir, L"%s%s\\", tszSkinRoot, fd.cFileName);
				ScanSkinDir(tszSubDir);
			}
			if (FindNextFile(h, &fd) == 0)
				break;
		}
		if (h != INVALID_HANDLE_VALUE)
			FindClose(h);

		ptrW wszCurrSkin(db_get_wsa(0, SRMSGMOD_T, "ContainerSkin"));
		LRESULT lr = cmbSkins.GetCount();
		for (int i = 1; i < lr; i++) {
			wchar_t *idata = (wchar_t *)cmbSkins.GetItemData(i);
			if (idata && idata != (wchar_t *)CB_ERR) {
				if (!mir_wstrcmpi(wszCurrSkin, idata)) {
					cmbSkins.SetCurSel(i);
					return;
				}
			}
		}

		// if no active skin present, set the focus to the first one
		cmbSkins.SetCurSel(0);
	}

	// scan a single skin directory and find the.TSK file.Fill the combobox and set the
	// relative path name as item extra data.
	//
	// If available, read the Name property from the [Global] section and use it in the
	// combo box. If such property is not found, the base filename (without .tsk extension)
	// will be used as the name of the skin.

	void ScanSkinDir(const wchar_t *tszFolder)
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

		if (!fValid)
			return;

		wchar_t tszFinalName[MAX_PATH], tszRel[MAX_PATH], szBuf[255];
		mir_snwprintf(tszFinalName, L"%s%s", tszFolder, fd.cFileName);
		GetPrivateProfileString(L"Global", L"Name", L"None", szBuf, _countof(szBuf), tszFinalName);

		if (!mir_wstrcmp(szBuf, L"None")) {
			fd.cFileName[mir_wstrlen(fd.cFileName) - 4] = 0;
			wcsncpy_s(szBuf, fd.cFileName, _TRUNCATE);
		}

		PathToRelativeW(tszFinalName, tszRel, M.getSkinPath());
		cmbSkins.AddString(szBuf, (LPARAM)mir_wstrdup(tszRel));
	}

	// self - configure the dialog, don't let the user load or unload
	// a skin while a message window is open. Show the warning that all
	// windows must be closed.
	void UpdateControls(CTimer* = nullptr)
	{
		bool fWindowsOpen = (pFirstContainer != nullptr);
		for (auto &it : _ctrls)
			Utils::enableDlgControl(m_hwnd, it, !fWindowsOpen);

		Utils::showDlgControl(m_hwnd, IDC_SKIN_WARN, fWindowsOpen ? SW_SHOW : SW_HIDE);
		Utils::showDlgControl(m_hwnd, IDC_SKIN_CLOSENOW, fWindowsOpen ? SW_SHOW : SW_HIDE);
	}

	CTimer m_timer;
	CCtrlCheck chkUseSkin, chkLoadFonts, chkLoadTempl;
	CCtrlCombo cmbSkins;
	CCtrlButton btnClose, btnReload, btnRescan, btnExport, btnImport;
	CCtrlHyperlink m_link1, m_link2;

public:
	CSkinOptsDlg() :
		CDlgBase(g_plugin, IDD_OPT_SKIN),
		m_timer(this, 1000),
		m_link1(this, IDC_GETSKINS, "https://miranda-ng.org/tags/tabsrmm/"),
		m_link2(this, IDC_HELP_GENERAL, "https://wiki.miranda-ng.org/index.php?title=Plugin:TabSRMM/Using_skins"),
		cmbSkins(this, IDC_SKINNAME),
		btnClose(this, IDC_SKIN_CLOSENOW),
		btnReload(this, IDC_RELOADSKIN),
		btnRescan(this, IDC_RESCANSKIN),
		btnExport(this, IDC_THEMEEXPORT),
		btnImport(this, IDC_THEMEIMPORT),
		chkUseSkin(this, IDC_USESKIN),
		chkLoadFonts(this, IDC_SKIN_LOADFONTS),
		chkLoadTempl(this, IDC_SKIN_LOADTEMPLATES)
	{
		m_timer.OnEvent = Callback(this, &CSkinOptsDlg::UpdateControls);

		btnClose.OnClick = Callback(this, &CSkinOptsDlg::onClick_Close);
		btnReload.OnClick = Callback(this, &CSkinOptsDlg::onClick_Reload);
		btnRescan.OnClick = Callback(this, &CSkinOptsDlg::onClick_Rescan);
		btnExport.OnClick = Callback(this, &CSkinOptsDlg::onClick_Export);
		btnImport.OnClick = Callback(this, &CSkinOptsDlg::onClick_Import);

		chkUseSkin.OnChange = Callback(this, &CSkinOptsDlg::onChange_UseSkin);
		chkLoadFonts.OnChange = Callback(this, &CSkinOptsDlg::onChange_LoadFonts);
		chkLoadTempl.OnChange = Callback(this, &CSkinOptsDlg::onChange_LoadTemplates);

		cmbSkins.OnSelChanged = Callback(this, &CSkinOptsDlg::onSelChange_Skins);
	}

	bool OnInitDialog() override
	{
		RescanSkins();

		chkUseSkin.SetState(M.GetByte("useskin", 0));

		int loadMode = M.GetByte("skin_loadmode", 0);
		CheckDlgButton(m_hwnd, IDC_SKIN_LOADFONTS, loadMode & THEME_READ_FONTS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SKIN_LOADTEMPLATES, loadMode & THEME_READ_TEMPLATES ? BST_CHECKED : BST_UNCHECKED);

		UpdateControls();
		m_timer.Start(1000);
		return true;
	}

	void OnDestroy() override
	{
		m_timer.Stop();
		FreeComboData();
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_CTLCOLORSTATIC && (HWND)lParam == GetDlgItem(m_hwnd, IDC_SKIN_WARN)) {
			SetTextColor((HDC)wParam, RGB(255, 50, 50));
			return 0;
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

	void onClick_Close(CCtrlButton *)
	{
		CloseAllContainers();
	}

	void onChange_UseSkin(CCtrlCheck *)
	{
		db_set_b(0, SRMSGMOD_T, "useskin", chkUseSkin.GetState());
	}

	void onChange_LoadFonts(CCtrlCheck *)
	{
		int loadMode = M.GetByte("skin_loadmode", 0);
		loadMode = IsDlgButtonChecked(m_hwnd, IDC_SKIN_LOADFONTS) ? loadMode | THEME_READ_FONTS : loadMode & ~THEME_READ_FONTS;
		db_set_b(0, SRMSGMOD_T, "skin_loadmode", loadMode);
	}

	void onChange_LoadTemplates(CCtrlCheck *)
	{
		int loadMode = M.GetByte("skin_loadmode", 0);
		loadMode = IsDlgButtonChecked(m_hwnd, IDC_SKIN_LOADTEMPLATES) ? loadMode | THEME_READ_TEMPLATES : loadMode & ~THEME_READ_TEMPLATES;
		db_set_b(0, SRMSGMOD_T, "skin_loadmode", loadMode);
	}

	void onClick_Reload(CCtrlButton *)
	{
		Skin->setFileName();
		Skin->Load();
		UpdateControls();
	}

	void onClick_Rescan(CCtrlButton *)
	{
		FreeComboData();
		RescanSkins();
	}

	void onClick_Export(CCtrlButton *)
	{
		const wchar_t *szFilename = GetThemeFileName(1);
		if (szFilename != nullptr)
			WriteThemeToINI(szFilename, nullptr);
	}

	void onClick_Import(CCtrlButton *)
	{
		LRESULT r = CWarning::show(CSkin::m_skinEnabled ? CWarning::WARN_THEME_OVERWRITE : CWarning::WARN_OPTION_CLOSE, MB_YESNOCANCEL | MB_ICONQUESTION);
		if (r == IDNO || r == IDCANCEL)
			return;

		const wchar_t *szFilename = GetThemeFileName(0);
		uint32_t dwFlags = THEME_READ_FONTS;

		if (szFilename != nullptr) {
			int result = MessageBox(nullptr, TranslateT("Do you want to also read message templates from the theme?\nCaution: This will overwrite the stored template set which may affect the look of your message window significantly.\nSelect Cancel to not load anything at all."),
				TranslateT("Load theme"), MB_YESNOCANCEL);
			if (result == IDCANCEL)
				return;
			if (result == IDYES)
				dwFlags |= THEME_READ_TEMPLATES;
			ReadThemeFromINI(szFilename, nullptr, 0, dwFlags);
			CacheLogFonts();
			CacheMsgLogIcons();
			PluginConfig.reloadSettings();
			CSkin::setAeroEffect(-1);
			Srmm_Broadcast(DM_OPTIONSAPPLIED, 1, 0);
			Srmm_Broadcast(DM_FORCEDREMAKELOG, 0, 0);
			SendMessage(GetParent(m_hwnd), WM_COMMAND, IDCANCEL, 0);
		}
	}

	void onSelChange_Skins(CCtrlCombo *)
	{
		LRESULT lr = cmbSkins.GetCurSel();
		if (lr != CB_ERR && lr > 0) {
			wchar_t *tszRelPath = (wchar_t *)cmbSkins.GetItemData(lr);
			if (tszRelPath && tszRelPath != (wchar_t *)CB_ERR)
				db_set_ws(0, SRMSGMOD_T, "ContainerSkin", tszRelPath);
			onClick_Reload(0);
		}
		else if (lr == 0) {		// selected the <no skin> entry
			db_unset(0, SRMSGMOD_T, "ContainerSkin");
			Skin->Unload();
			UpdateControls();
		}
	}
};

class CTabConfigDlg : public CDlgBase
{
	CCtrlSpin adjust, border, outerL, outerR, outerT, outerB, width, xpad, ypad;

public:
	CTabConfigDlg() :
		CDlgBase(g_plugin, IDD_TABCONFIG),
		ypad(this, IDC_SPIN1, 10, 1),
		xpad(this, IDC_SPIN3, 10, 1),
		width(this, IDC_TABWIDTHSPIN, 400, 50),
		adjust(this, IDC_BOTTOMTABADJUSTSPIN, 3, -3),
		border(this, IDC_TABBORDERSPIN, 10),
		outerL(this, IDC_TABBORDERSPINOUTER, 50),
		outerR(this, IDC_TABBORDERSPINOUTERRIGHT, 50),
		outerT(this, IDC_TABBORDERSPINOUTERTOP, 40),
		outerB(this, IDC_TABBORDERSPINOUTERBOTTOM, 40)
	{
	}

	bool OnInitDialog() override
	{
		width.SetPosition(PluginConfig.tabConfig.m_fixedwidth);
		adjust.SetPosition(PluginConfig.tabConfig.m_bottomAdjust);

		border.SetPosition(M.GetByte(CSkin::m_skinEnabled ? "S_tborder" : "tborder", 2));
		outerL.SetPosition(M.GetByte(CSkin::m_skinEnabled ? "S_tborder_outer_left" : "tborder_outer_left", 2));
		outerR.SetPosition(M.GetByte(CSkin::m_skinEnabled ? "S_tborder_outer_right" : "tborder_outer_right", 2));
		outerT.SetPosition(M.GetByte(CSkin::m_skinEnabled ? "S_tborder_outer_top" : "tborder_outer_top", 2));
		outerB.SetPosition(M.GetByte(CSkin::m_skinEnabled ? "S_tborder_outer_bottom" : "tborder_outer_bottom", 2));

		xpad.SetPosition(M.GetByte("x-pad", 4));
		ypad.SetPosition(M.GetByte("y-pad", 3));
		return true;
	}

	bool OnApply() override
	{
		db_set_b(0, SRMSGMOD_T, "y-pad", ypad.GetPosition());
		db_set_b(0, SRMSGMOD_T, "x-pad", xpad.GetPosition());
		db_set_b(0, SRMSGMOD_T, CSkin::m_skinEnabled ? "S_tborder" : "tborder", border.GetPosition());
		db_set_b(0, SRMSGMOD_T, CSkin::m_skinEnabled ? "S_tborder_outer_left" : "tborder_outer_left", outerL.GetPosition());
		db_set_b(0, SRMSGMOD_T, CSkin::m_skinEnabled ? "S_tborder_outer_right" : "tborder_outer_right", outerR.GetPosition());
		db_set_b(0, SRMSGMOD_T, CSkin::m_skinEnabled ? "S_tborder_outer_top" : "tborder_outer_top", outerT.GetPosition());
		db_set_b(0, SRMSGMOD_T, CSkin::m_skinEnabled ? "S_tborder_outer_bottom" : "tborder_outer_bottom", outerB.GetPosition());
		db_set_dw(0, SRMSGMOD_T, "bottomadjust", adjust.GetPosition());

		int fixedWidth = width.GetPosition();
		fixedWidth = (fixedWidth < 60 ? 60 : fixedWidth);
		db_set_dw(0, SRMSGMOD_T, "fixedwidth", fixedWidth);
		FreeTabConfig();
		ReloadTabConfig();

		for (TContainerData* p = pFirstContainer; p; p = p->pNext) {
			TabCtrl_SetPadding(p->m_hwndTabs, xpad.GetPosition(), ypad.GetPosition());
			::RedrawWindow(p->m_hwndTabs, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE);
		}
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Main options dialog

class COptMainDlg : public CDlgBase
{
	CCtrlSpin spnAvaSize;
	CCtrlCheck chkAvaPreserve;
	CCtrlButton btnReset;
	CCtrlTreeOpts treeOpts;
	CCtrlHyperlink urlHelp;

public:
	COptMainDlg() :
		CDlgBase(g_plugin, IDD_OPT_MSGDLG),
		urlHelp(this, IDC_HELP_GENERAL, "https://wiki.miranda-ng.org/index.php?title=Plugin:TabSRMM/General_settings"),
		btnReset(this, IDC_RESETWARNINGS),
		treeOpts(this, IDC_WINDOWOPTIONS),
		spnAvaSize(this, IDC_AVATARSPIN, 150),
		chkAvaPreserve(this, IDC_PRESERVEAVATARSIZE)
	{
		btnReset.OnClick = Callback(this, &COptMainDlg::onClick_Reset);

		auto *pwszSection = LPGENW("Message window behavior");
		treeOpts.AddOption(pwszSection, LPGENW("Always flash contact list and tray icon for new messages"), g_plugin.bFlashOnClist);
		treeOpts.AddOption(pwszSection, LPGENW("Delete temporary contacts on close"), g_plugin.bDeleteTemp);

		pwszSection = LPGENW("Sending messages");
		treeOpts.AddOption(pwszSection, LPGENW("Minimize the message window on send"), g_plugin.bAutoMin);
		treeOpts.AddOption(pwszSection, LPGENW("Close the message window on send"), g_plugin.bAutoClose);
		treeOpts.AddOption(pwszSection, LPGENW("Enable \"Paste and send\" feature"), g_plugin.bPasteAndSend);
		treeOpts.AddOption(pwszSection, LPGENW("Allow BBCode formatting in outgoing messages"), g_plugin.bSendFormat);

		pwszSection = LPGENW("Other options");
		treeOpts.AddOption(pwszSection, LPGENW("Automatically split long messages (experimental, use with care)"), g_plugin.bAutoSplit);
		treeOpts.AddOption(pwszSection, LPGENW("Use the same splitter height for all sessions"), g_plugin.bUseSameSplitSize);
		treeOpts.AddOption(pwszSection, LPGENW("Automatically copy selected text"), g_plugin.bAutoCopy);
	}

	bool OnInitDialog() override
	{
		chkAvaPreserve.SetState(M.GetByte("dontscaleavatars", 0));

		spnAvaSize.SetPosition(M.GetDword("avatarheight", 100));
		return true;
	}

	bool OnApply() override
	{
		db_set_dw(0, SRMSGMOD_T, "avatarheight", spnAvaSize.GetPosition());
		db_set_b(0, SRMSGMOD_T, "dontscaleavatars", chkAvaPreserve.GetState());

		PluginConfig.reloadSettings();
		Srmm_Broadcast(DM_OPTIONSAPPLIED, 1, 0);
		return true;
	}

	void onClick_Reset(CCtrlButton*)
	{
		db_set_dw(0, SRMSGMOD_T, "cWarningsL", 0);
		db_set_dw(0, SRMSGMOD_T, "cWarningsV", 0);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

class CTemplateEditDlg : public CMsgDialog
{
	typedef CMsgDialog CSuper;

	bool m_bRtl;
	bool m_bChanged;               // template in edit field is m_bChanged
	bool m_bSelChanging;
	bool m_bFirstUse = true;
	int  m_iCurrIdx;               // template currently in editor
	bool m_bUpdateInfo[TMPL_MAX];  // item states...

	TTemplateSet *tSet;

	CCtrlEdit edtText;
	CCtrlColor clr1, clr2, clr3, clr4, clr5;
	CCtrlButton btnResetAll, btnSave, btnForget, btnRevert, btnPreview;
	CCtrlListBox listTemplates;
	CCtrlHyperlink urlHelp;

public:
	CTemplateEditDlg(BOOL _rtl, HWND hwndParent) :
		CSuper(IDD_TEMPLATEEDIT, 0),
		m_bRtl(_rtl),
		clr1(this, IDC_COLOR1),
		clr2(this, IDC_COLOR2),
		clr3(this, IDC_COLOR3),
		clr4(this, IDC_COLOR4),
		clr5(this, IDC_COLOR5),
		edtText(this, IDC_EDITTEMPLATE),
		urlHelp(this, IDC_VARIABLESHELP, "https://wiki.miranda-ng.org/index.php?title=Plugin:TabSRMM/Templates"),
		btnSave(this, IDC_SAVETEMPLATE),
		btnForget(this, IDC_FORGET),
		btnRevert(this, IDC_REVERT),
		btnPreview(this, IDC_UPDATEPREVIEW),
		btnResetAll(this, IDC_RESETALLTEMPLATES),
		listTemplates(this, IDC_TEMPLATELIST)
	{
		SetParent(hwndParent);

		m_hContact = db_add_contact();
		Proto_AddToContact(m_hContact, m_szProto = META_PROTO);
		Contact::Hide(m_hContact);
		Contact::RemoveFromList(m_hContact);
		db_set_ws(m_hContact, META_PROTO, "Nick", TranslateT("Test contact"));

		m_pContainer = new TContainerData();
		m_pContainer->LoadOverrideTheme();
		tSet = m_bRtl ? m_pContainer->m_rtl_templates : m_pContainer->m_ltr_templates;

		listTemplates.OnDblClick = Callback(this, &CTemplateEditDlg::onDblClick_List);
		listTemplates.OnSelChange = Callback(this, &CTemplateEditDlg::onSelChange_List);

		edtText.OnChange = Callback(this, &CTemplateEditDlg::onChange_Text);

		btnSave.OnClick = Callback(this, &CTemplateEditDlg::onClick_Save);
		btnForget.OnClick = Callback(this, &CTemplateEditDlg::onClick_Forget);
		btnRevert.OnClick = Callback(this, &CTemplateEditDlg::onClick_Revert);
		btnPreview.OnClick = Callback(this, &CTemplateEditDlg::onClick_Preview);
		btnResetAll.OnClick = Callback(this, &CTemplateEditDlg::onClick_Reset);
	}

	bool OnInitDialog() override
	{
		m_pLog = new CLogWindow(*this);

		m_dwFlags = m_pContainer->m_theme.dwFlags;

		m_cache = new CContactCache(m_hContact);
		m_cache->updateNick();
		m_cache->updateUIN();
		m_cache->updateStats(TSessionStats::INIT_TIMER);
		GetMYUIN();

		edtText.SendMsg(EM_LIMITTEXT, TEMPLATE_LENGTH - 1, 0);
		SetWindowText(m_hwnd, TranslateT("Template set editor"));
		Utils::enableDlgControl(m_hwnd, IDC_SAVETEMPLATE, false);
		Utils::enableDlgControl(m_hwnd, IDC_REVERT, false);
		Utils::enableDlgControl(m_hwnd, IDC_FORGET, false);

		for (auto &it : TemplateNamesW)
			listTemplates.AddString(TranslateW(it), int(&it - TemplateNamesW));

		Utils::enableDlgControl(m_hwndParent, IDC_MODIFY, false);
		Utils::enableDlgControl(m_hwndParent, IDC_RTLMODIFY, false);

		clr1.SetColor(M.GetDword("cc1", SRMSGDEFSET_BKGCOLOUR));
		clr1.SetColor(M.GetDword("cc2", SRMSGDEFSET_BKGCOLOUR));
		clr1.SetColor(M.GetDword("cc3", SRMSGDEFSET_BKGCOLOUR));
		clr1.SetColor(M.GetDword("cc4", SRMSGDEFSET_BKGCOLOUR));
		clr1.SetColor(M.GetDword("cc5", SRMSGDEFSET_BKGCOLOUR));
		edtText.SendMsg(EM_SETREADONLY, true, 0);
		return true;
	}

	void OnDestroy() override
	{
		db_delete_contact(m_hContact);

		Utils::enableDlgControl(m_hwndParent, IDC_MODIFY, true);
		Utils::enableDlgControl(m_hwndParent, IDC_RTLMODIFY, true);

		delete m_pContainer;
		delete m_cache;

		db_set_dw(0, SRMSGMOD_T, "cc1", clr1.GetColor());
		db_set_dw(0, SRMSGMOD_T, "cc2", clr2.GetColor());
		db_set_dw(0, SRMSGMOD_T, "cc3", clr3.GetColor());
		db_set_dw(0, SRMSGMOD_T, "cc4", clr4.GetColor());
		db_set_dw(0, SRMSGMOD_T, "cc5", clr5.GetColor());
	}

	INT_PTR DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		if (uMsg == WM_DRAWITEM) {
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;
			int iItem = dis->itemData;
			SetBkMode(dis->hDC, TRANSPARENT);
			FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_WINDOW));
			if (dis->itemState & ODS_SELECTED) {
				if (m_bUpdateInfo[iItem] == true) {
					HBRUSH bkg = CreateSolidBrush(RGB(255, 0, 0));
					HBRUSH oldBkg = (HBRUSH)SelectObject(dis->hDC, bkg);
					FillRect(dis->hDC, &dis->rcItem, bkg);
					SelectObject(dis->hDC, oldBkg);
					DeleteObject(bkg);
				}
				else FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));

				SetTextColor(dis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
			}
			else {
				if (m_bUpdateInfo[iItem])
					SetTextColor(dis->hDC, RGB(255, 0, 0));
				else
					SetTextColor(dis->hDC, GetSysColor(COLOR_WINDOWTEXT));
			}
			
			const wchar_t *pwszName = TranslateW(TemplateNamesW[iItem]);
			TextOutW(dis->hDC, dis->rcItem.left, dis->rcItem.top, pwszName, (int)mir_wstrlen(pwszName));
		}

		return CSrmmBaseDialog::DlgProc(uMsg, wParam, lParam);
	}

	void onChange_Text(CCtrlEdit *)
	{
		if (!m_bSelChanging) {
			m_bChanged = true;
			m_bUpdateInfo[m_iCurrIdx] = true;
			Utils::enableDlgControl(m_hwnd, IDC_SAVETEMPLATE, true);
			Utils::enableDlgControl(m_hwnd, IDC_FORGET, true);
			listTemplates.Disable();
			Utils::enableDlgControl(m_hwnd, IDC_REVERT, true);
		}
		InvalidateRect(listTemplates.GetHwnd(), nullptr, FALSE);
	}

	void onClick_Forget(CCtrlButton *)
	{
		m_bChanged = false;
		m_bUpdateInfo[m_iCurrIdx] = false;
		m_bSelChanging = true;
		edtText.SetText(tSet->szTemplates[m_iCurrIdx]);
		SetFocus(edtText.GetHwnd());
		InvalidateRect(listTemplates.GetHwnd(), nullptr, FALSE);
		Utils::enableDlgControl(m_hwnd, IDC_SAVETEMPLATE, false);
		Utils::enableDlgControl(m_hwnd, IDC_FORGET, false);
		listTemplates.Enable();
		Utils::enableDlgControl(m_hwnd, IDC_REVERT, false);
		m_bSelChanging = false;
		edtText.SendMsg(EM_SETREADONLY, true, 0);
	}

	void onClick_Preview(CCtrlButton *)
	{
		int iIndex = listTemplates.GetCurSel();
		wchar_t szTemp[TEMPLATE_LENGTH + 2];

		if (m_bChanged) {
			memcpy(szTemp, tSet->szTemplates[m_iCurrIdx], (TEMPLATE_LENGTH * sizeof(wchar_t)));
			edtText.GetText(tSet->szTemplates[m_iCurrIdx], TEMPLATE_LENGTH);
		}

		T2Utf szText((iIndex == 6) ? TranslateT("is now offline (was online)") : TranslateT("The quick brown fox jumps over the lazy dog."));

		DBEVENTINFO dbei = {};
		dbei.szModule = m_szProto;
		dbei.timestamp = time(0);
		dbei.eventType = (iIndex == 6) ? EVENTTYPE_STATUSCHANGE : EVENTTYPE_MESSAGE;
		dbei.eventType = (iIndex == 7) ? EVENTTYPE_ERRMSG : dbei.eventType;
		if (dbei.eventType == EVENTTYPE_ERRMSG)
			dbei.szModule = (char *)TranslateT("Sample error message");
		dbei.pBlob = (uint8_t *)szText.get();
		dbei.cbBlob = (int)mir_strlen((char *)dbei.pBlob) + 1;
		dbei.flags = (iIndex == 1 || iIndex == 3 || iIndex == 5) ? DBEF_SENT : 0;
		dbei.flags |= (m_bRtl ? DBEF_RTL : 0);
		m_lastEventTime = (iIndex == 4 || iIndex == 5) ? time(0) - 1 : 0;
		m_iLastEventType = MAKELONG(dbei.flags, dbei.eventType);
		m_dwFlags = MWF_LOG_ALL;
		m_dwFlags = (m_bRtl ? m_dwFlags | MWF_LOG_RTL : m_dwFlags & ~MWF_LOG_RTL);
		m_dwFlags = (iIndex == 0 || iIndex == 1) ? m_dwFlags & ~MWF_LOG_GROUPMODE : m_dwFlags | MWF_LOG_GROUPMODE;
		mir_snwprintf(m_wszMyNickname, TranslateT("My Nickname"));
		LOG()->LogEvents(0, 1, false, &dbei);
		if (m_bFirstUse) {
			if (m_bRtl)
				LOG()->LogEvents(0, 1, false, &dbei);
			m_bFirstUse = false;
		}
		if (m_bChanged)
			memcpy(tSet->szTemplates[m_iCurrIdx], szTemp, TEMPLATE_LENGTH * sizeof(wchar_t));
	}

	void onClick_Reset(CCtrlButton *)
	{
		if (MessageBox(m_hwnd, TranslateT("This will reset the template set to the default built-in templates. Are you sure you want to do this?"),
			TranslateT("Template set editor"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
			db_set_b(0, m_bRtl ? RTLTEMPLATES_MODULE : TEMPLATES_MODULE, "setup", 0);
			LoadDefaultTemplates();
			MessageBox(m_hwnd,
				TranslateT("Template set was successfully reset, please close and reopen all message windows. This template editor window will now close."),
				TranslateT("Template set editor"), MB_OK);
			Close();
		}
	}

	void onClick_Revert(CCtrlButton *)
	{
		m_bChanged = false;
		m_bUpdateInfo[m_iCurrIdx] = false;
		m_bSelChanging = true;
		memcpy(tSet->szTemplates[m_iCurrIdx], LTR_Default.szTemplates[m_iCurrIdx], sizeof(wchar_t) * TEMPLATE_LENGTH);
		edtText.SetText(tSet->szTemplates[m_iCurrIdx]);
		db_unset(0, m_bRtl ? RTLTEMPLATES_MODULE : TEMPLATES_MODULE, TemplateNames[m_iCurrIdx]);
		SetFocus(edtText.GetHwnd());
		InvalidateRect(listTemplates.GetHwnd(), nullptr, FALSE);
		m_bSelChanging = false;
		
		Utils::enableDlgControl(m_hwnd, IDC_SAVETEMPLATE, false);
		Utils::enableDlgControl(m_hwnd, IDC_REVERT, false);
		Utils::enableDlgControl(m_hwnd, IDC_FORGET, false);
		listTemplates.Enable();
		edtText.SendMsg(EM_SETREADONLY, true, 0);
	}

	void onClick_Save(CCtrlButton *)
	{
		wchar_t newTemplate[TEMPLATE_LENGTH + 2];
		edtText.GetText(newTemplate, _countof(newTemplate));
		memcpy(tSet->szTemplates[m_iCurrIdx], newTemplate, sizeof(wchar_t) * TEMPLATE_LENGTH);
		m_bChanged = false;
		m_bUpdateInfo[m_iCurrIdx] = false;
		Utils::enableDlgControl(m_hwnd, IDC_SAVETEMPLATE, false);
		Utils::enableDlgControl(m_hwnd, IDC_FORGET, false);
		listTemplates.Enable();
		Utils::enableDlgControl(m_hwnd, IDC_REVERT, false);
		InvalidateRect(listTemplates.GetHwnd(), nullptr, FALSE);
		db_set_ws(0, m_bRtl ? RTLTEMPLATES_MODULE : TEMPLATES_MODULE, TemplateNames[m_iCurrIdx], newTemplate);
		edtText.SendMsg(EM_SETREADONLY, true, 0);
	}

	void onDblClick_List(CCtrlListBox *)
	{
		LRESULT iIndex = listTemplates.GetCurSel();
		if (iIndex != LB_ERR) {
			edtText.SetText(tSet->szTemplates[iIndex]);
			m_iCurrIdx = iIndex;
			m_bChanged = false;
			m_bSelChanging = false;
			SetFocus(edtText.GetHwnd());
			edtText.SendMsg(EM_SETREADONLY, FALSE, 0);
		}
	}

	void onSelChange_List(CCtrlListBox *)
	{
		LRESULT iIndex = listTemplates.GetCurSel();
		m_bSelChanging = true;
		if (iIndex != LB_ERR) {
			edtText.SetText(tSet->szTemplates[iIndex]);
			m_iCurrIdx = iIndex;
			m_bChanged = false;
		}
		edtText.SendMsg(EM_SETREADONLY, TRUE, 0);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

static UINT __ctrls[] = { IDC_INDENTSPIN, IDC_RINDENTSPIN, IDC_INDENTAMOUNT, IDC_RIGHTINDENT, IDC_MODIFY, IDC_RTLMODIFY };

class COptLogDlg : public CDlgBase
{
	CCtrlSpin spnLeft, spnRight, spnLoadCount, spnLoadTime, spnTrim;
	CCtrlCheck chkAlwaysTrim, chkLoadUnread, chkLoadCount, chkLoadTime;
	CCtrlButton btnModify, btnRtlModify;
	CCtrlTreeOpts logOpts;

	uint32_t m_flags;

	// configure the option page - hide most of the settings here when either IEView
	// or H++ is set as the global message log viewer. Showing these options may confuse
	// the user, because they are not working and the user needs to configure the 3rd
	// party plugin.

public:
	COptLogDlg() :
		CDlgBase(g_plugin, IDD_OPT_MSGLOG),
		logOpts(this, IDC_LOGOPTIONS),
		btnModify(this, IDC_MODIFY),
		btnRtlModify(this, IDC_RTLMODIFY),
		spnTrim(this, IDC_TRIMSPIN, 1000, 5),
		spnLeft(this, IDC_INDENTSPIN, 1000),
		spnRight(this, IDC_RINDENTSPIN, 1000),
		spnLoadTime(this, IDC_LOADTIMESPIN, 24 * 60),
		spnLoadCount(this, IDC_LOADCOUNTSPIN, 100),
		chkLoadTime(this, IDC_LOADTIME),
		chkLoadCount(this, IDC_LOADCOUNT),
		chkAlwaysTrim(this, IDC_ALWAYSTRIM),
		chkLoadUnread(this, IDC_LOADUNREAD)
	{
		btnModify.OnClick = Callback(this, &COptLogDlg::onClick_Modify);
		btnRtlModify.OnClick = Callback(this, &COptLogDlg::onClick_RtlModify);

		chkAlwaysTrim.OnChange = Callback(this, &COptLogDlg::onChange_Trim);
		chkLoadTime.OnChange = chkLoadCount.OnChange = chkLoadUnread.OnChange = Callback(this, &COptLogDlg::onChange_Load);

		m_flags = M.GetDword("mwflags", MWF_LOG_DEFAULT) & MWF_LOG_ALL;

		auto *pwszSection = LPGENW("Message log appearance");
		logOpts.AddOption(pwszSection, LPGENW("Draw grid lines"), m_flags, MWF_LOG_GRID);
		logOpts.AddOption(pwszSection, LPGENW("Log status changes"), g_plugin.bLogStatusChanges);
		logOpts.AddOption(pwszSection, LPGENW("Use message grouping"), m_flags, MWF_LOG_GROUPMODE);
		logOpts.AddOption(pwszSection, LPGENW("Indent message body"), m_flags, MWF_LOG_INDENT);
		logOpts.AddOption(pwszSection, LPGENW("Simple text formatting (*bold*, etc.)"), m_flags, MWF_LOG_TEXTFORMAT);
		logOpts.AddOption(pwszSection, LPGENW("Support BBCode formatting"), m_flags, MWF_LOG_BBCODE);
		logOpts.AddOption(pwszSection, LPGENW("Place a separator in the log after a window lost its foreground status"), g_plugin.bUseDividers);
		logOpts.AddOption(pwszSection, LPGENW("Only place a separator when an incoming event is announced with a popup"), g_plugin.bDividersUsePopupConfig);
		logOpts.AddOption(pwszSection, LPGENW("RTL is default text direction"), m_flags, MWF_LOG_RTL);
		logOpts.AddOption(pwszSection, LPGENW("Use normal templates (uncheck to use simple templates if your template set supports them)"), m_flags, MWF_LOG_NORMALTEMPLATES);

		pwszSection = LPGENW("Support for external plugins");
		logOpts.AddOption(pwszSection, LPGENW("Show events at the new line (IEView Compatibility Mode)"), m_flags, MWF_LOG_NEWLINE);
		logOpts.AddOption(pwszSection, LPGENW("Underline timestamp/nickname (IEView Compatibility Mode)"), m_flags, MWF_LOG_UNDERLINE);
		logOpts.AddOption(pwszSection, LPGENW("Show timestamp after nickname (IEView Compatibility Mode)"), m_flags, MWF_LOG_SWAPNICK);

		pwszSection = LPGENW("Timestamp settings (note: timestamps also depend on your templates)");
		logOpts.AddOption(pwszSection, LPGENW("Show timestamps"), m_flags, MWF_LOG_SHOWTIME);
		logOpts.AddOption(pwszSection, LPGENW("Show dates in timestamps"), m_flags, MWF_LOG_SHOWDATES);
		logOpts.AddOption(pwszSection, LPGENW("Show seconds in timestamps"), m_flags, MWF_LOG_SHOWSECONDS);
		logOpts.AddOption(pwszSection, LPGENW("Use contacts local time (if timezone info available)"), m_flags, MWF_LOG_LOCALTIME);

		pwszSection = LPGENW("Message log icons");
		logOpts.AddOption(pwszSection, LPGENW("Event type icons in the message log"), m_flags, MWF_LOG_SHOWICONS);
		logOpts.AddOption(pwszSection, LPGENW("Text symbols as event markers"), m_flags, MWF_LOG_SYMBOLS);
		logOpts.AddOption(pwszSection, LPGENW("Use incoming/outgoing icons"), m_flags, MWF_LOG_INOUTICONS);
	}

	bool OnInitDialog() override
	{
		switch (g_plugin.getByte(SRMSGSET_LOADHISTORY, SRMSGDEFSET_LOADHISTORY)) {
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

		spnLeft.SetPosition(M.GetDword("IndentAmount", 20));
		spnRight.SetPosition(M.GetDword("RightIndent", 20));
		spnLoadCount.SetPosition(g_plugin.getWord(SRMSGSET_LOADCOUNT, SRMSGDEFSET_LOADCOUNT));
		spnLoadTime.SetPosition(g_plugin.getWord(SRMSGSET_LOADTIME, SRMSGDEFSET_LOADTIME));

		uint32_t maxhist = M.GetDword("maxhist", 0);
		spnTrim.SetPosition(maxhist);
		spnTrim.Enable(maxhist != 0);
		Utils::enableDlgControl(m_hwnd, IDC_TRIM, maxhist != 0);
		chkAlwaysTrim.SetState(maxhist != 0);
		return true;
	}

	bool OnApply() override
	{
		uint32_t dwFlags = M.GetDword("mwflags", MWF_LOG_DEFAULT) & ~(MWF_LOG_ALL);
		db_set_dw(0, SRMSGMOD_T, "mwflags", m_flags | dwFlags);

		if (chkLoadCount.GetState())
			g_plugin.setByte(SRMSGSET_LOADHISTORY, LOADHISTORY_COUNT);
		else if (chkLoadTime.GetState())
			g_plugin.setByte(SRMSGSET_LOADHISTORY, LOADHISTORY_TIME);
		else
			g_plugin.setByte(SRMSGSET_LOADHISTORY, LOADHISTORY_UNREAD);
		g_plugin.setWord(SRMSGSET_LOADCOUNT, spnLoadCount.GetPosition());
		g_plugin.setWord(SRMSGSET_LOADTIME, spnLoadTime.GetPosition());

		db_set_dw(0, SRMSGMOD_T, "IndentAmount", spnLeft.GetPosition());
		db_set_dw(0, SRMSGMOD_T, "RightIndent", spnRight.GetPosition());

		// scan the tree view and obtain the options...
		if (chkAlwaysTrim.GetState())
			db_set_dw(0, SRMSGMOD_T, "maxhist", spnTrim.GetPosition());
		else
			db_set_dw(0, SRMSGMOD_T, "maxhist", 0);
		PluginConfig.reloadSettings();
		Srmm_Broadcast(DM_OPTIONSAPPLIED, 1, 0);
		return true;
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
		CTemplateEditDlg *pDlg = new CTemplateEditDlg(false, m_hwnd);
		pDlg->Show();
	}

	void onClick_RtlModify(CCtrlButton*)
	{
		CTemplateEditDlg *pDlg = new CTemplateEditDlg(true, m_hwnd);
		pDlg->Show();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// typing notify options

class COptTypingDlg : public CDlgBase
{
	HANDLE hItemNew, hItemUnknown;

	CCtrlClc m_clist;
	CCtrlCheck chkWin, chkNoWin;
	CCtrlCheck chkNotifyTray, chkShowNotify;
	CCtrlHyperlink urlHelp;

	void ResetCList(CCtrlClc* = nullptr)
	{
		m_clist.SetUseGroups(Clist::UseGroups);
		m_clist.SetHideEmptyGroups(true);
	}

	void RebuildList(CCtrlClc* = nullptr)
	{
		bool defType = g_plugin.bTypingNew;
		if (hItemNew && defType)
			m_clist.SetCheck(hItemNew, true);

		if (hItemUnknown && g_plugin.bTypingUnknown)
			m_clist.SetCheck(hItemUnknown, true);

		for (auto &hContact : Contacts())
			if (HANDLE hItem = m_clist.FindContact(hContact))
				m_clist.SetCheck(hItem, g_plugin.getByte(hContact, SRMSGSET_TYPING, defType));
	}

	void SaveList()
	{
		if (hItemNew)
			g_plugin.bTypingNew = m_clist.GetCheck(hItemNew);

		if (hItemUnknown)
			g_plugin.bTypingUnknown = m_clist.GetCheck(hItemUnknown);

		for (auto &hContact : Contacts())
			if (HANDLE hItem = m_clist.FindContact(hContact))
				g_plugin.setByte(hContact, SRMSGSET_TYPING, m_clist.GetCheck(hItem));
	}

public:
	COptTypingDlg() :
		CDlgBase(g_plugin, IDD_OPT_MSGTYPE),
		urlHelp(this, IDC_MTN_HELP, "https://wiki.miranda-ng.org/index.php?title=Plugin:TabSRMM/Typing_notifications"),
		m_clist(this, IDC_CLIST),
		chkWin(this, IDC_TYPEWIN),
		chkNoWin(this, IDC_TYPENOWIN),
		chkNotifyTray(this, IDC_NOTIFYTRAY),
		chkShowNotify(this, IDC_SHOWNOTIFY)
	{
		m_clist.OnListRebuilt = Callback(this, &COptTypingDlg::RebuildList);
		m_clist.OnOptionsChanged = Callback(this, &COptTypingDlg::ResetCList);

		chkWin.OnChange = chkNoWin.OnChange = Callback(this, &COptTypingDlg::onCheck_Win);

		chkNotifyTray.OnChange = Callback(this, &COptTypingDlg::onCheck_NotifyTray);
		chkShowNotify.OnChange = Callback(this, &COptTypingDlg::onCheck_ShowNotify);
	}

	bool OnInitDialog() override
	{
		CLCINFOITEM cii = { sizeof(cii) };
		cii.flags = CLCIIF_GROUPFONT | CLCIIF_CHECKBOX;
		cii.pszText = TranslateT("** New contacts **");
		hItemNew = m_clist.AddInfoItem(&cii);
		cii.pszText = TranslateT("** Unknown contacts **");
		hItemUnknown = m_clist.AddInfoItem(&cii);

		SetWindowLongPtr(m_clist.GetHwnd(), GWL_STYLE, GetWindowLongPtr(m_clist.GetHwnd(), GWL_STYLE) | (CLS_SHOWHIDDEN));
		ResetCList();

		CheckDlgButton(m_hwnd, IDC_SHOWNOTIFY, g_plugin.getByte(SRMSGSET_SHOWTYPING, SRMSGDEFSET_SHOWTYPING) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_TYPEFLASHWIN, g_plugin.getByte(SRMSGSET_SHOWTYPINGWINFLASH, SRMSGDEFSET_SHOWTYPINGWINFLASH) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(m_hwnd, IDC_TYPENOWIN, g_plugin.getByte(SRMSGSET_SHOWTYPINGNOWINOPEN, 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_TYPEWIN, g_plugin.getByte(SRMSGSET_SHOWTYPINGWINOPEN, 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_NOTIFYTRAY, g_plugin.getByte(SRMSGSET_SHOWTYPINGCLIST, SRMSGDEFSET_SHOWTYPINGCLIST) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_NOTIFYBALLOON, g_plugin.getByte("ShowTypingBalloon", 0));

		Utils::enableDlgControl(m_hwnd, IDC_TYPEWIN, IsDlgButtonChecked(m_hwnd, IDC_NOTIFYTRAY) != 0);
		Utils::enableDlgControl(m_hwnd, IDC_TYPENOWIN, IsDlgButtonChecked(m_hwnd, IDC_NOTIFYTRAY) != 0);
		Utils::enableDlgControl(m_hwnd, IDC_NOTIFYBALLOON, IsDlgButtonChecked(m_hwnd, IDC_NOTIFYTRAY) &&
			(IsDlgButtonChecked(m_hwnd, IDC_TYPEWIN) || IsDlgButtonChecked(m_hwnd, IDC_TYPENOWIN)));

		Utils::enableDlgControl(m_hwnd, IDC_TYPEFLASHWIN, IsDlgButtonChecked(m_hwnd, IDC_SHOWNOTIFY) != 0);
		Utils::enableDlgControl(m_hwnd, IDC_MTN_POPUPMODE, g_plugin.bPopups);

		SendDlgItemMessage(m_hwnd, IDC_MTN_POPUPMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Always"));
		SendDlgItemMessage(m_hwnd, IDC_MTN_POPUPMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Always, but no popup when window is focused"));
		SendDlgItemMessage(m_hwnd, IDC_MTN_POPUPMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Only when no message window is open"));

		SendDlgItemMessage(m_hwnd, IDC_MTN_POPUPMODE, CB_SETCURSEL, (WPARAM)M.GetByte("MTN_PopupMode", 0), 0);
		return true;
	}

	bool OnApply() override
	{
		SaveList();
		g_plugin.setByte(SRMSGSET_SHOWTYPING, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_SHOWNOTIFY));
		g_plugin.setByte(SRMSGSET_SHOWTYPINGWINFLASH, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_TYPEFLASHWIN));
		g_plugin.setByte(SRMSGSET_SHOWTYPINGNOWINOPEN, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_TYPENOWIN));
		g_plugin.setByte(SRMSGSET_SHOWTYPINGWINOPEN, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_TYPEWIN));
		g_plugin.setByte(SRMSGSET_SHOWTYPINGCLIST, (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_NOTIFYTRAY));
		g_plugin.setByte("ShowTypingBalloon", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_NOTIFYBALLOON));
		db_set_b(0, SRMSGMOD_T, "MTN_PopupMode", (uint8_t)SendDlgItemMessage(m_hwnd, IDC_MTN_POPUPMODE, CB_GETCURSEL, 0, 0));
		PluginConfig.reloadSettings();
		return true;
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_SETFOCUS)
			Utils::enableDlgControl(m_hwnd, IDC_MTN_POPUPMODE, g_plugin.bPopups);

		return CDlgBase::DlgProc(msg, wParam, lParam);
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
};

/////////////////////////////////////////////////////////////////////////////////////////
// options for tabbed messaging got their own page.. finally :)

class COptTabbedDlg : public CDlgBase
{
	CCtrlEdit edtLimit;
	CCtrlSpin spnLimit;
	CCtrlCombo cmbEscMode;
	CCtrlCheck chkLimit;
	CCtrlButton btnSetup;
	CCtrlTreeOpts tabOptions;

public:
	COptTabbedDlg() :
		CDlgBase(g_plugin, IDD_OPT_TABBEDMSG),
		chkLimit(this, IDC_CUT_TABTITLE),
		edtLimit(this, IDC_CUT_TITLEMAX),
		spnLimit(this, IDC_CUT_TITLEMAXSPIN, 20, 5),
		btnSetup(this, IDC_SETUPAUTOCREATEMODES),
		tabOptions(this, IDC_TABMSGOPTIONS),
		cmbEscMode(this, IDC_ESCMODE)
	{
		btnSetup.OnClick = Callback(this, &COptTabbedDlg::onClick_Setup);

		chkLimit.OnChange = Callback(this, &COptTabbedDlg::onChange_Cut);

		auto *pwszSection = LPGENW("Tab options");
		tabOptions.AddOption(pwszSection, LPGENW("Show status text on tabs"), g_plugin.bStatusOnTabs);
		tabOptions.AddOption(pwszSection, LPGENW("Prefer xStatus icons when available"), g_plugin.bUseXStatus);
		tabOptions.AddOption(pwszSection, LPGENW("Detailed tooltip on tabs (requires Tipper plugin)"), g_plugin.bDetailedTooltips);

		pwszSection = LPGENW("How to create tabs and windows for incoming messages");
		tabOptions.AddOption(pwszSection, LPGENW("ALWAYS activate new message sessions (has PRIORITY over the options below)"), g_plugin.bAutoPopup);
		tabOptions.AddOption(pwszSection, LPGENW("Automatically create new message sessions without activating them"), g_plugin.bAutoTabs);
		tabOptions.AddOption(pwszSection, LPGENW("New windows are minimized (the option above MUST be active)"), g_plugin.bAutoContainer);
		tabOptions.AddOption(pwszSection, LPGENW("Activate a minimized window when a new tab is created inside it"), g_plugin.bPopupContainer);
		tabOptions.AddOption(pwszSection, LPGENW("Automatically switch existing tabs in minimized windows on incoming messages (ignored when using Aero Peek task bar features)"), g_plugin.bAutoSwitchTabs);

		pwszSection = LPGENW("Miscellaneous options");
		tabOptions.AddOption(pwszSection, LPGENW("Close button only hides message windows"), g_plugin.bHideOnClose);
		tabOptions.AddOption(pwszSection, LPGENW("Allow Tab key in typing area (this will disable focus selection by Tab key)"), g_plugin.bAllowTab);
		tabOptions.AddOption(pwszSection, LPGENW("Add offline contacts to multisend list"), g_plugin.bAllowOfflineMultisend);
	}

	bool OnInitDialog() override
	{
		chkLimit.SetState(M.GetByte("cuttitle", 0));
		spnLimit.SetPosition(db_get_w(0, SRMSGMOD_T, "cut_at", 15));

		cmbEscMode.AddString(TranslateT("Normal - close tab, if last tab is closed also close the window"));
		cmbEscMode.AddString(TranslateT("Minimize the window to the task bar"));
		cmbEscMode.AddString(TranslateT("Close or hide window, depends on the close button setting above"));
		cmbEscMode.AddString(TranslateT("Do nothing (ignore Esc key)"));
		cmbEscMode.SetCurSel(PluginConfig.m_EscapeCloses);
		return true;
	}

	bool OnApply() override
	{
		db_set_w(0, SRMSGMOD_T, "cut_at", spnLimit.GetPosition());
		db_set_b(0, SRMSGMOD_T, "cuttitle", chkLimit.GetState());
		db_set_b(0, SRMSGMOD_T, "escmode", cmbEscMode.GetCurSel());

		PluginConfig.reloadSettings();
		Srmm_Broadcast(DM_OPTIONSAPPLIED, 0, 0);
		return true;
	}

	void onClick_Setup(CCtrlButton*)
	{
		CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CHOOSESTATUSMODES), m_hwnd, DlgProcSetupStatusModes, M.GetDword("autopopupmask", -1));
	}

	void onChange_Cut(CCtrlCheck*)
	{
		bool bEnabled = chkLimit.GetState() != 0;
		edtLimit.Enable(bEnabled);
		spnLimit.Enable(bEnabled);
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_COMMAND && wParam == DM_STATUSMASKSET)
			db_set_dw(0, SRMSGMOD_T, "autopopupmask", (uint32_t)lParam);

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// container options

class COptContainersDlg : public CDlgBase
{
	CCtrlSpin spnNumFlash, spnTabLimit, spnFlashDelay;
	CCtrlCombo cmbAeroEffect;
	CCtrlCheck chkUseAero, chkUseAeroPeek, chkLimits, chkSingle, chkGroup, chkDefault;
	CCtrlHyperlink urlHelp;

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
		: CDlgBase(g_plugin, IDD_OPT_CONTAINERS),
		urlHelp(this, IDC_HELP_CONTAINERS, "https://wiki.miranda-ng.org/index.php?title=Plugin:TabSRMM/Containers"),
		spnNumFlash(this, IDC_NRFLASHSPIN, 255),
		spnTabLimit(this, IDC_TABLIMITSPIN, 1000, 1),
		spnFlashDelay(this, IDC_FLASHINTERVALSPIN, 10000, 500),
		chkUseAero(this, IDC_USEAERO),
		chkUseAeroPeek(this, IDC_USEAEROPEEK),
		cmbAeroEffect(this, IDC_AEROEFFECT),
		chkLimits(this, IDC_LIMITTABS),
		chkSingle(this, IDC_SINGLEWINDOWMODE),
		chkGroup(this, IDC_CONTAINERGROUPMODE),
		chkDefault(this, IDC_DEFAULTCONTAINERMODE)
	{
		chkUseAero.OnChange = Callback(this, &COptContainersDlg::onChangeAero);
		chkLimits.OnChange = chkSingle.OnChange = chkGroup.OnChange = chkDefault.OnChange = Callback(this, &COptContainersDlg::onChangeLimits);
	}

	bool OnInitDialog() override
	{
		chkGroup.SetState(M.GetByte("useclistgroups", 0));
		chkLimits.SetState(M.GetByte("limittabs", 0));

		spnTabLimit.SetPosition(M.GetDword("maxtabs", 1));
		onChangeLimits(nullptr);

		chkSingle.SetState(M.GetByte("singlewinmode", 0));
		chkDefault.SetState(!(chkGroup.GetState() || chkLimits.GetState() || chkSingle.GetState()));

		spnNumFlash.SetPosition(M.GetByte("nrflash", 4));
		spnFlashDelay.SetPosition(M.GetDword("flashinterval", 1000));

		chkUseAero.SetState(M.GetByte("useAero", 1));
		chkUseAeroPeek.SetState(M.GetByte("useAeroPeek", 1));

		for (int i = 0; i < CSkin::AERO_EFFECT_LAST; i++)
			cmbAeroEffect.InsertString(TranslateW(CSkin::m_aeroEffects[i].tszName), -1);
		cmbAeroEffect.SetCurSel(CSkin::m_aeroEffect);
		cmbAeroEffect.Enable(IsWinVerVistaPlus());

		chkUseAero.Enable(IsWinVerVistaPlus());
		chkUseAeroPeek.Enable(IsWinVer7Plus());
		if (IsWinVerVistaPlus())
			Utils::enableDlgControl(m_hwnd, IDC_AEROEFFECT, chkUseAero.GetState() != 0);
		return true;
	}

	bool OnApply() override
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
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// TabModPlus options

class COptAdvancedDlg : public CDlgBase
{
	CCtrlSpin spnTimeout, spnHistSize;
	CCtrlTreeOpts plusOptions;
	CCtrlHyperlink urlHelp;

public:
	COptAdvancedDlg() :
		CDlgBase(g_plugin, IDD_OPTIONS_PLUS),
		urlHelp(this, IDC_PLUS_HELP, "https://wiki.miranda-ng.org/index.php?title=Plugin:TabSRMM/Typing_notifications"),
		spnTimeout(this, IDC_TIMEOUTSPIN, 300, SRMSGSET_MSGTIMEOUT_MIN / 1000),
		spnHistSize(this, IDC_HISTORYSIZESPIN, 255, 15),
		plusOptions(this, IDC_PLUS_CHECKTREE)
	{
		auto *pwszSection = LPGENW("Message window tweaks");
		plusOptions.AddOption(pwszSection, LPGENW("Close current tab on send"), g_plugin.bCloseSend);
		plusOptions.AddOption(pwszSection, LPGENW("Enable unattended send (experimental feature, required for multisend and send later)"), SendLater::Avail);
		plusOptions.AddOption(pwszSection, LPGENW("Show client description in info panel"), g_plugin.bShowDesc);

		pwszSection = LPGENW("Display metacontact icons");
		plusOptions.AddOption(pwszSection, LPGENW("On tab control"), g_plugin.bMetaTab);
		plusOptions.AddOption(pwszSection, LPGENW("On the button bar"), g_plugin.bMetaBar);

		pwszSection = LPGENW("Error feedback");
		plusOptions.AddOption(pwszSection, LPGENW("Enable error popups on sending failures"), g_plugin.bErrorPopup);
	}

	bool OnInitDialog() override
	{
		spnTimeout.SetPosition(PluginConfig.m_MsgTimeout / 1000);
		spnHistSize.SetPosition(M.GetByte("historysize", 0));
		return true;
	}

	bool OnApply() override
	{
		int msgTimeout = 1000 * spnTimeout.GetPosition();
		PluginConfig.m_MsgTimeout = msgTimeout >= SRMSGSET_MSGTIMEOUT_MIN ? msgTimeout : SRMSGSET_MSGTIMEOUT_MIN;
		g_plugin.setDword(SRMSGSET_MSGTIMEOUT, PluginConfig.m_MsgTimeout);

		db_set_b(0, SRMSGMOD_T, "historysize", spnHistSize.GetPosition());
		PluginConfig.reloadAdv();
		return true;
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
	uint8_t sameAsFlags, sameAs;
	COLORREF colour;
	char size;
	uint8_t style;
	uint8_t charset;
	char szFace[LF_FACESIZE];
}
static fontSettings[MSGDLGFONTCOUNT + 1];

#include <poppack.h>

#define SRFONTSETTINGMODULE FONTMODULE

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

	uint32_t defValue;		// should be full combined value for masked items!
	uint32_t dwBit;

	uint8_t dbType;
	char *dbModule;
	char *dbSetting;

	uint8_t valueType;
	union
	{
		void *pValue;

		char *charValue;
		int *intValue;
		uint8_t *byteValue;
		uint32_t *dwordValue;
		BOOL *boolValue;
	};
};

uint32_t OptCheckBox_LoadValue(struct OptCheckBox *cb)
{
	switch (cb->valueType) {
	case CBVT_NONE:
		switch (cb->dbType) {
		case DBVT_BYTE:
			return db_get_b(0, cb->dbModule, cb->dbSetting, cb->defValue);
		case DBVT_WORD:
			return db_get_w(0, cb->dbModule, cb->dbSetting, cb->defValue);
		case DBVT_DWORD:
			return db_get_dw(0, cb->dbModule, cb->dbSetting, cb->defValue);
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
	uint32_t value = OptCheckBox_LoadValue(cb);
	if (cb->dwBit) value &= cb->dwBit;
	CheckDlgButton(hwnd, cb->idc, value ? BST_CHECKED : BST_UNCHECKED);
}

void OptCheckBox_Save(HWND hwnd, OptCheckBox *cb)
{
	uint32_t value = IsDlgButtonChecked(hwnd, cb->idc) == BST_CHECKED;

	if (cb->dwBit) {
		uint32_t curValue = OptCheckBox_LoadValue(cb);
		value = value ? (curValue | cb->dwBit) : (curValue & ~cb->dwBit);
	}

	switch (cb->dbType) {
	case DBVT_BYTE:
		db_set_b(0, cb->dbModule, cb->dbSetting, (uint8_t)value);
		break;
	case DBVT_WORD:
		db_set_w(0, cb->dbModule, cb->dbSetting, (uint16_t)value);
		break;
	case DBVT_DWORD:
		db_set_dw(0, cb->dbModule, cb->dbSetting, (uint32_t)value);
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
		*cb->byteValue = (uint8_t)value;
		break;
	case CBVT_DWORD:
		*cb->dwordValue = (uint32_t)value;
		break;
	case CBVT_BOOL:
		*cb->boolValue = (BOOL)value;
		break;
	}
}

INT_PTR CALLBACK DlgProcSetupStatusModes(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	uint32_t dwStatusMask = GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	static uint32_t dwNewStatusMask = 0;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		dwStatusMask = lParam;

		SetWindowText(hwndDlg, TranslateT("Choose status modes"));
		{
			for (int i = ID_STATUS_ONLINE; i <= ID_STATUS_MAX; i++) {
				SetDlgItemText(hwndDlg, i, Clist_GetStatusModeDescription(i, 0));
				if (dwStatusMask != -1 && (dwStatusMask & (1 << (i - ID_STATUS_ONLINE))))
					CheckDlgButton(hwndDlg, i, BST_CHECKED);
				Utils::enableDlgControl(hwndDlg, i, dwStatusMask != -1);
			}
		}
		if (dwStatusMask == -1)
			CheckDlgButton(hwndDlg, IDC_ALWAYS, BST_CHECKED);
		ShowWindow(hwndDlg, SW_SHOWNORMAL);
		return true;

	case DM_GETSTATUSMASK:
		if (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS))
			dwNewStatusMask = -1;
		else {
			dwNewStatusMask = 0;
			for (int i = ID_STATUS_ONLINE; i <= ID_STATUS_MAX; i++)
				dwNewStatusMask |= (IsDlgButtonChecked(hwndDlg, i) ? (1 << (i - ID_STATUS_ONLINE)) : 0);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			SendMessage(hwndDlg, DM_GETSTATUSMASK, 0, 0);
			SendMessage(GetParent(hwndDlg), DM_STATUSMASKSET, 0, (LPARAM)dwNewStatusMask);
			__fallthrough;

		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;

		case IDC_ALWAYS:
			for (int i = ID_STATUS_ONLINE; i <= ID_STATUS_MAX; i++)
				Utils::enableDlgControl(hwndDlg, i, !IsDlgButtonChecked(hwndDlg, IDC_ALWAYS));
			break;
		}

	case WM_DESTROY:
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

int OptInitialise(WPARAM wParam, LPARAM lParam)
{
	TN_OptionsInitialize(wParam, lParam);

	// message sessions' options
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 910000000;
	odp.flags = ODPF_BOLDGROUPS;
	odp.szTitle.a = LPGEN("Message sessions");

	odp.szTab.a = LPGEN("General");
	odp.pDialog = new COptMainDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Tabs and layout");
	odp.pDialog = new COptTabbedDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Containers");
	odp.pDialog = new COptContainersDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Message log");
	odp.pDialog = new COptLogDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Advanced tweaks");
	odp.pDialog = new COptAdvancedDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szGroup.a = LPGEN("Message sessions");
	odp.szTitle.a = LPGEN("Typing notify");
	odp.pDialog = new COptTypingDlg();
	g_plugin.addOptions(wParam, &odp);

	// skin options
	odp.position = 910000000;
	odp.szGroup.a = LPGEN("Skins");
	odp.szTitle.a = LPGEN("Message window");

	odp.szTab.a = LPGEN("Load and apply");
	odp.pDialog = new CSkinOptsDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Window layout tweaks");
	odp.pDialog = new CTabConfigDlg();
	g_plugin.addOptions(wParam, &odp);

	// group chats
	Chat_Options(wParam);
	return 0;
}
