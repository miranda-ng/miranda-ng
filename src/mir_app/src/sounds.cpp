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

#pragma comment(lib, "winmm.lib")

struct SoundItem
{
	ptrA name;
	ptrW pwszSection;
	ptrW pwszDescription;
	ptrW ptszTempFile;
	HPLUGIN pPlugin;

	__inline wchar_t* getSection() const { return TranslateW_LP(pwszSection, pPlugin); }
	__inline wchar_t* getDescr() const { return TranslateW_LP(pwszDescription, pPlugin); }
};

static int CompareSounds(const SoundItem* p1, const SoundItem* p2)
{
	return mir_strcmp(p1->name, p2->name);
}

static OBJLIST<SoundItem> arSounds(10, CompareSounds);
static HANDLE hPlayEvent = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////
// sound options

static class CSoundOptionsDlg *g_pDialog = nullptr;

class CSoundOptionsDlg : public CDlgBase
{
	void onChange_Sounds(CCtrlCheck*)
	{
		m_tree.Enable(IsDlgButtonChecked(m_hwnd, IDC_ENABLESOUNDS));

		if (BST_UNCHECKED == IsDlgButtonChecked(m_hwnd, IDC_ENABLESOUNDS))
			ShowHidePane(false);
		else if (m_tree.GetSelection() && m_tree.GetParent(m_tree.GetSelection()))
			ShowHidePane(true);
	}

	HTREEITEM FindNamedTreeItemAtRoot(const wchar_t *name)
	{
		wchar_t str[128];
		TVITEMEX tvi;
		tvi.mask = TVIF_TEXT;
		tvi.pszText = str;
		tvi.cchTextMax = _countof(str);
		tvi.hItem = m_tree.GetRoot();
		while (tvi.hItem != nullptr) {
			m_tree.GetItem(&tvi);
			if (!mir_wstrcmpi(str, name))
				return tvi.hItem;

			tvi.hItem = m_tree.GetNextSibling(tvi.hItem);
		}
		return nullptr;
	}

	void SelectAll(bool bSelect)
	{
		for (HTREEITEM hItem = m_tree.GetRoot(); hItem; hItem = m_tree.GetNextSibling(hItem))
			for (HTREEITEM hItem2 = m_tree.GetChild(hItem); hItem2; hItem2 = m_tree.GetNextSibling(hItem2))
				m_tree.SetCheckState(hItem2, bSelect);
	}

	void ShowHidePane(bool bShow)
	{
		int iCommand = (bShow) ? SW_SHOW : SW_HIDE;
		ShowWindow(GetDlgItem(m_hwnd, IDC_SGROUP), iCommand);
		ShowWindow(GetDlgItem(m_hwnd, IDC_NAME), iCommand);
		ShowWindow(GetDlgItem(m_hwnd, IDC_NAMEVAL), iCommand);
		ShowWindow(GetDlgItem(m_hwnd, IDC_SLOC), iCommand);
		ShowWindow(GetDlgItem(m_hwnd, IDC_LOCATION), iCommand);
		ShowWindow(GetDlgItem(m_hwnd, IDC_CHANGE), iCommand);
		ShowWindow(GetDlgItem(m_hwnd, IDC_PREVIEW), iCommand);
	}

	CCtrlCheck chkSounds;
	CCtrlButton btnPreview, btnChange;
	CCtrlTreeView m_tree;
	CCtrlHyperlink linkGetMore;

public:
	CSoundOptionsDlg() :
		CDlgBase(g_plugin, IDD_OPT_SOUND),
		m_tree(this, IDC_SOUNDTREE),
		chkSounds(this, IDC_ENABLESOUNDS),
		btnChange(this, IDC_CHANGE),
		btnPreview(this, IDC_PREVIEW),
		linkGetMore(this, IDC_GETMORE, "https://miranda-ng.org/tags/sounds")
	{
		btnChange.OnClick = Callback(this, &CSoundOptionsDlg::onClick_Change);
		btnPreview.OnClick = Callback(this, &CSoundOptionsDlg::onClick_Preview);

		chkSounds.OnChange = Callback(this, &CSoundOptionsDlg::onChange_Sounds);

		m_tree.OnBuildMenu = Callback(this, &CSoundOptionsDlg::onMenu_Tree);
		m_tree.OnSelChanged = Callback(this, &CSoundOptionsDlg::onChanged_Tree);
	}

	bool OnInitDialog() override
	{
		g_pDialog = this;
		SetWindowLongPtr(m_tree.GetHwnd(), GWL_STYLE, GetWindowLongPtr(m_tree.GetHwnd(), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		ShowHidePane(false);
		RebuildTree();
		m_tree.SetItemState(0, TVIS_SELECTED, TVIS_SELECTED);
		chkSounds.SetState(db_get_b(0, "Skin", "UseSound", 0) != 0);
		return true;
	}

	bool OnApply() override
	{
		db_set_b(0, "Skin", "UseSound", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_ENABLESOUNDS));

		for (auto &p : arSounds)
			if (p->ptszTempFile)
				db_set_ws(0, "SkinSounds", p->name, p->ptszTempFile);

		TVITEMEX tvi, tvic;
		tvi.hItem = m_tree.GetRoot();
		while (tvi.hItem != nullptr) {
			tvi.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_STATE;
			m_tree.GetItem(&tvi);
			if (tvi.lParam == 0) {
				tvic.hItem = m_tree.GetChild(tvi.hItem);
				while (tvic.hItem != nullptr) {
					tvic.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_STATE;
					m_tree.GetItem(&tvic);
					SoundItem *p = (SoundItem *)tvic.lParam;
					if (((tvic.state & TVIS_STATEIMAGEMASK) >> 12 == 2))
						db_unset(0, "SkinSoundsOff", p->name);
					else
						db_set_b(0, "SkinSoundsOff", p->name, 1);
					tvic.hItem = m_tree.GetNextSibling(tvic.hItem);
				}
			}

			tvi.hItem = m_tree.GetNextSibling(tvi.hItem);
		}
		return true;
	}

	void OnDestroy() override
	{
		g_pDialog = nullptr;
		ImageList_Destroy(m_tree.GetImageList(TVSIL_STATE));
	}

	void onClick_Preview(CCtrlButton*)
	{
		HTREEITEM hti = m_tree.GetSelection();
		if (hti == nullptr)
			return;

		TVITEMEX tvi = { 0 };
		tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT;
		tvi.hItem = hti;
		if (m_tree.GetItem(&tvi) == FALSE)
			return;
		if (tvi.lParam == 0)
			return;

		SoundItem *p = (SoundItem*)tvi.lParam;
		if (p->ptszTempFile)
			NotifyEventHooks(hPlayEvent, 1, (LPARAM)p->ptszTempFile);
		else {
			DBVARIANT dbv;
			if (!db_get_ws(0, "SkinSounds", p->name, &dbv)) {
				wchar_t szPathFull[MAX_PATH];
				PathToAbsoluteW(dbv.pwszVal, szPathFull);
				NotifyEventHooks(hPlayEvent, 1, (LPARAM)szPathFull);
				db_free(&dbv);
			}
		}
	}

	void onClick_Change(CCtrlButton*)
	{
		HTREEITEM hti = m_tree.GetSelection();
		if (hti == nullptr)
			return;

		TVITEMEX tvi = {};
		tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT;
		tvi.hItem = hti;
		if (m_tree.GetItem(&tvi) == FALSE)
			return;
		if (tvi.lParam == 0)
			return;

		SoundItem *p = (SoundItem *)tvi.lParam;

		wchar_t str[MAX_PATH], strFull[MAX_PATH], strdir[MAX_PATH], filter[MAX_PATH];
		if (p->ptszTempFile)
			wcsncpy_s(strFull, p->ptszTempFile, _TRUNCATE);
		else {
			if (db_get_b(0, "SkinSoundsOff", p->name, 0) == 0) {
				DBVARIANT dbv;
				if (db_get_ws(0, "SkinSounds", p->name, &dbv) == 0) {
					PathToAbsoluteW(dbv.pwszVal, strdir);
					db_free(&dbv);
				}
			}
		}

		wcsncpy_s(strFull, (p->ptszTempFile ? p->ptszTempFile : L""), _TRUNCATE);
		PathToAbsoluteW(strFull, strdir);

		OPENFILENAME ofn;
		memset(&ofn, 0, sizeof(ofn));
		if (GetModuleHandle(L"bass_interface.dll"))
			mir_snwprintf(filter, L"%s (*.wav,*.mp3,*.ogg)%c*.wav;*.mp3;*.ogg%c%s (*)%c*%c", TranslateT("Sound files"), 0, 0, TranslateT("All files"), 0, 0);
		else
			mir_snwprintf(filter, L"%s (*.wav)%c*.wav%c%s (*)%c*%c", TranslateT("WAV files"), 0, 0, TranslateT("All files"), 0, 0);
		ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
		ofn.hwndOwner = GetParent(m_hwnd);
		ofn.hInstance = nullptr;
		ofn.lpstrFilter = filter;

		wchar_t *slash = wcsrchr(strdir, '\\');
		if (slash) {
			*slash = 0;
			ofn.lpstrInitialDir = strdir;
		}

		str[0] = 0;
		ofn.lpstrFile = str;
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_LONGNAMES | OFN_NOCHANGEDIR;
		ofn.nMaxFile = _countof(str);
		ofn.nMaxFileTitle = MAX_PATH;
		ofn.lpstrDefExt = L"wav";
		if (GetOpenFileName(&ofn)) {
			PathToRelativeW(str, strFull);
			p->ptszTempFile = mir_wstrdup(strFull);
			SetDlgItemText(m_hwnd, IDC_LOCATION, strFull);
			NotifyChange();
		}
	}

	void onChanged_Tree(CCtrlTreeView::TEventInfo *evt)
	{
		TVITEM tvi = evt->nmtv->itemNew;

		if (tvi.lParam == 0)
			ShowHidePane(false);
		else {
			SoundItem *p = (SoundItem *)tvi.lParam;

			wchar_t buf[256];
			mir_snwprintf(buf, L"%s: %s", p->getSection(), p->getDescr());
			SetDlgItemText(m_hwnd, IDC_NAMEVAL, buf);
			if (p->ptszTempFile)
				SetDlgItemText(m_hwnd, IDC_LOCATION, p->ptszTempFile);
			else {
				DBVARIANT dbv;
				if (!db_get_ws(0, "SkinSounds", p->name, &dbv)) {
					SetDlgItemText(m_hwnd, IDC_LOCATION, dbv.pwszVal);
					db_free(&dbv);
				}
				else SetDlgItemText(m_hwnd, IDC_LOCATION, TranslateT("<not specified>"));
			}
			ShowHidePane(true);
		}
	}

	void onMenu_Tree(CCtrlBase*)
	{
		POINT pt;
		GetCursorPos(&pt);

		HMENU hMenu = CreatePopupMenu();
		AppendMenu(hMenu, MF_STRING, 1, TranslateT("Select all"));
		AppendMenu(hMenu, MF_STRING, 2, TranslateT("Unselect all"));

		switch (TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, nullptr)) {
			case 1: SelectAll(true); break;
			case 2: SelectAll(false); break;
		}
		DestroyMenu(hMenu);
	}

	void RebuildTree()
	{
		m_tree.SelectItem(nullptr);
		m_tree.Hide();
		m_tree.DeleteAllItems();

		TVINSERTSTRUCT tvis;
		tvis.hParent = nullptr;
		tvis.hInsertAfter = TVI_SORT;
		tvis.itemex.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
		tvis.itemex.state = tvis.itemex.stateMask = TVIS_EXPANDED;
		for (auto &p : arSounds) {
			tvis.itemex.stateMask = TVIS_EXPANDED;
			tvis.itemex.state = TVIS_EXPANDED;
			tvis.hParent = FindNamedTreeItemAtRoot(p->getSection());
			if (tvis.hParent == nullptr) {
				tvis.itemex.lParam = 0;
				tvis.itemex.pszText = p->getSection();
				tvis.hParent = tvis.itemex.hItem = m_tree.InsertItem(&tvis);
				tvis.itemex.stateMask = TVIS_STATEIMAGEMASK;
				tvis.itemex.state = INDEXTOSTATEIMAGEMASK(0);
				m_tree.SetItem(&tvis.itemex);
			}
			tvis.itemex.stateMask = TVIS_STATEIMAGEMASK;
			tvis.itemex.state = INDEXTOSTATEIMAGEMASK(!db_get_b(0, "SkinSoundsOff", p->name, 0) ? 2 : 1);
			tvis.itemex.lParam = (LPARAM)p;
			tvis.itemex.pszText = p->getDescr();
			m_tree.InsertItem(&tvis);
		}

		TVITEMEX tvi;
		tvi.hItem = m_tree.GetRoot();
		while (tvi.hItem != nullptr) {
			tvi.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_STATE;
			m_tree.GetItem(&tvi);
			if (tvi.lParam == 0)
				m_tree.SetItemState(tvi.hItem, INDEXTOSTATEIMAGEMASK(0), TVIS_STATEIMAGEMASK);

			tvi.hItem = m_tree.GetNextSibling(tvi.hItem);
		}

		m_tree.Show();
	}
};

static int SkinOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = -200000000;
	odp.szTitle.a = LPGEN("Sounds");
	odp.pDialog = new CSoundOptionsDlg();
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void KillModuleSounds(CMPluginBase *pPlugin)
{
	bool bFound = false;

	auto T = arSounds.rev_iter();
	for (auto &it : T)
		if (it->pPlugin == pPlugin) {
			arSounds.removeItem(&it);
			bFound = true;
		}

	if (bFound && g_pDialog)
		g_pDialog->RebuildTree();
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPluginBase::addSound(const char *pszName, const wchar_t *pwszSection, const wchar_t *pwszDescription, const wchar_t *pwszDefaultFile)
{
	if (pszName == nullptr || pwszDescription == nullptr)
		return 1;

	SoundItem *item = new SoundItem; // due to OBJLIST
	item->name = mir_strdup(pszName);
	item->ptszTempFile = nullptr;
	item->pPlugin = this;
	arSounds.insert(item);

	item->pwszDescription = mir_wstrdup(pwszDescription);
	item->pwszSection = mir_wstrdup((pwszSection != nullptr) ? pwszSection : L"Other");

	if (pwszDefaultFile) {
		ptrW wszSavedValue(db_get_wsa(0, "SkinSounds", item->name));
		if (wszSavedValue == nullptr)
			db_set_ws(0, "SkinSounds", item->name, pwszDefaultFile);
	}

	if (g_pDialog != nullptr)
		g_pDialog->RebuildTree();
	return 0;
}

static int Skin_PlaySoundDefault(WPARAM wParam, LPARAM lParam)
{
	wchar_t *pszFile = (wchar_t*)lParam;
	if (db_get_b(0, "Skin", "UseSound", 0) || (wParam & SPS_FORCEPLAY) != 0) {
		int flags;
		if (pszFile) {
			flags = SND_ASYNC | SND_FILENAME | SND_NOSTOP;
			if (wParam & SPS_LOOP)
				flags |= SND_LOOP;
		}
		else flags = 0;

		if (!PlaySoundW(pszFile, nullptr, flags))
			return 1;
	}

	return 0;
}

MIR_APP_DLL(int) Skin_PlaySoundFile(const wchar_t *pwszFileName, int flags)
{
	if (pwszFileName) {
		wchar_t tszFull[MAX_PATH];
		PathToAbsoluteW(pwszFileName, tszFull);
		return NotifyEventHooks(hPlayEvent, flags, (LPARAM)tszFull);
	}

	return NotifyEventHooks(hPlayEvent, flags, 0);
}

MIR_APP_DLL(int) Skin_PlaySound(const char *pszSoundName, int flags)
{
	if (pszSoundName == nullptr)
		return 1;

	int idx = arSounds.getIndex((SoundItem*)&pszSoundName);
	if (idx == -1)
		return 1;

	if (db_get_b(0, "SkinSoundsOff", pszSoundName, 0))
		return 1;

	ptrW wszFilePath(db_get_wsa(0, "SkinSounds", pszSoundName));
	if (wszFilePath == nullptr)
		return 1;

	return Skin_PlaySoundFile(wszFilePath, flags);
}

int LoadSkinSounds(void)
{
	HookEvent(ME_OPT_INITIALISE, SkinOptionsInit);

	hPlayEvent = CreateHookableEvent(ME_SKIN_PLAYINGSOUND);
	SetHookDefaultForHookableEvent(hPlayEvent, Skin_PlaySoundDefault);
	return 0;
}

void UnloadSkinSounds(void)
{
	arSounds.destroy();
}
