/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
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
	char*  name;
	wchar_t* pwszSection;
	wchar_t* pwszDescription;
	wchar_t* ptszTempFile;
	int    hLangpack;

	__inline wchar_t* getSection() const { return TranslateW_LP(pwszSection, hLangpack); }
	__inline wchar_t* getDescr() const { return TranslateW_LP(pwszDescription, hLangpack); }

	__inline void clear(void)
	{
		mir_free(name);
		mir_free(pwszSection);
		mir_free(pwszDescription);
		mir_free(ptszTempFile);
	}
};

static int CompareSounds(const SoundItem* p1, const SoundItem* p2)
{
	return mir_strcmp(p1->name, p2->name);
}

static OBJLIST<SoundItem> arSounds(10, CompareSounds);

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(void) KillModuleSounds(int _hLang)
{
	for (int i = arSounds.getCount()-1; i >= 0; i--) {
		SoundItem &p = arSounds[i];
		if (p.hLangpack == _hLang) {
			p.clear();
			arSounds.remove(i);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static HANDLE hPlayEvent = nullptr;

MIR_APP_DLL(int) Skin_AddSound(const char *pszName, const wchar_t *pwszSection, const wchar_t *pwszDescription, const wchar_t *pwszDefaultFile, int _hLang)
{
	if (pszName == nullptr || pwszDescription == nullptr)
		return 1;

	SoundItem *item = new SoundItem; // due to OBJLIST
	item->name = mir_strdup(pszName);
	item->ptszTempFile = nullptr;
	item->hLangpack = _hLang;
	arSounds.insert(item);

	item->pwszDescription = mir_wstrdup(pwszDescription);
	item->pwszSection = mir_wstrdup((pwszSection != nullptr) ? pwszSection : L"Other");

	if (pwszDefaultFile) {
		ptrW wszSavedValue(db_get_wsa(0, "SkinSounds", item->name));
		if (wszSavedValue == nullptr)
			db_set_ws(0, "SkinSounds", item->name, pwszDefaultFile);
	}

	return 0;
}

static int Skin_PlaySoundDefault(WPARAM wParam, LPARAM lParam)
{
	wchar_t *pszFile = (wchar_t*) lParam;
	if (pszFile && (db_get_b(0, "Skin", "UseSound", 0) || (int)wParam == 1))
		PlaySound(pszFile, nullptr, SND_ASYNC | SND_FILENAME | SND_NOSTOP);

	return 0;
}

MIR_APP_DLL(int) Skin_PlaySoundFile(const wchar_t *pwszFileName)
{
	if (pwszFileName == nullptr)
		return 1;

	wchar_t tszFull[MAX_PATH];
	PathToAbsoluteW(pwszFileName, tszFull);
	NotifyEventHooks(hPlayEvent, 0, (LPARAM)tszFull);
	return 0;
}

MIR_APP_DLL(int) Skin_PlaySound(const char *pszSoundName)
{
	if (pszSoundName == nullptr)
		return 1;

	SoundItem tmp = { (char*)pszSoundName };
	int idx = arSounds.getIndex(&tmp);
	if (idx == -1)
		return 1;

	if (db_get_b(0, "SkinSoundsOff", pszSoundName, 0))
		return 1;

	ptrW wszFilePath(db_get_wsa(0, "SkinSounds", pszSoundName));
	if (wszFilePath == nullptr)
		return 1;

	Skin_PlaySoundFile(wszFilePath);
	return 0;
}

#define DM_REBUILD_STREE (WM_USER+1)
#define DM_HIDEPANE      (WM_USER+2)
#define DM_SHOWPANE      (WM_USER+3)
#define DM_CHECKENABLED  (WM_USER+4)

static HTREEITEM FindNamedTreeItemAtRoot(HWND hwndTree, const wchar_t* name)
{
	wchar_t str[128];
	TVITEM tvi;
	tvi.mask = TVIF_TEXT;
	tvi.pszText = str;
	tvi.cchTextMax = _countof(str);
	tvi.hItem = TreeView_GetRoot(hwndTree);
	while (tvi.hItem != nullptr) {
		SendMessage(hwndTree, TVM_GETITEM, 0, (LPARAM)&tvi);
		if (!mir_wstrcmpi(str, name))
			return tvi.hItem;

		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
	return nullptr;
}

INT_PTR CALLBACK DlgProcSoundOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hwndTree = nullptr;
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		hwndTree = GetDlgItem(hwndDlg, IDC_SOUNDTREE);
		SetWindowLongPtr(hwndTree, GWL_STYLE, GetWindowLongPtr(hwndTree, GWL_STYLE)|TVS_NOHSCROLL|TVS_CHECKBOXES);
		SendMessage(hwndDlg, DM_HIDEPANE, 0, 0);
		SendMessage(hwndDlg, DM_REBUILD_STREE, 0, 0);
		TreeView_SetItemState(hwndTree, 0, TVIS_SELECTED, TVIS_SELECTED);
		CheckDlgButton(hwndDlg, IDC_ENABLESOUNDS, db_get_b(0, "Skin", "UseSound", 0) ? BST_CHECKED : BST_UNCHECKED);
		SendMessage(hwndDlg, DM_CHECKENABLED, 0, 0);
		return TRUE;

	case DM_REBUILD_STREE:
		TreeView_SelectItem(hwndTree, nullptr);
		ShowWindow(hwndTree, SW_HIDE);
		TreeView_DeleteAllItems(hwndTree);
		{
			TVINSERTSTRUCT tvis;
			tvis.hParent = nullptr;
			tvis.hInsertAfter = TVI_SORT;
			tvis.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
			tvis.item.state = tvis.item.stateMask = TVIS_EXPANDED;
			for (int i=0; i < arSounds.getCount(); i++) {
				tvis.item.stateMask = TVIS_EXPANDED;
				tvis.item.state = TVIS_EXPANDED;
				tvis.hParent = FindNamedTreeItemAtRoot(hwndTree, arSounds[i].getSection());
				if (tvis.hParent == nullptr) {
					tvis.item.lParam = -1;
					tvis.item.pszText = arSounds[i].getSection();
					tvis.hParent = tvis.item.hItem = TreeView_InsertItem(hwndTree, &tvis);
					tvis.item.stateMask = TVIS_STATEIMAGEMASK;
					tvis.item.state = INDEXTOSTATEIMAGEMASK(0);
					TreeView_SetItem(hwndTree, &tvis.item);
				}
				tvis.item.stateMask = TVIS_STATEIMAGEMASK;
				tvis.item.state = INDEXTOSTATEIMAGEMASK(!db_get_b(0, "SkinSoundsOff", arSounds[i].name, 0)?2:1);
				tvis.item.lParam = i;
				tvis.item.pszText = arSounds[i].getDescr();
				TreeView_InsertItem(hwndTree, &tvis);
		}	}
		{
			TVITEM tvi;
			tvi.hItem = TreeView_GetRoot(hwndTree);
			while (tvi.hItem != nullptr) {
				tvi.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_STATE;
				TreeView_GetItem(hwndTree, &tvi);
				if (tvi.lParam == -1)
					TreeView_SetItemState(hwndTree, tvi.hItem, INDEXTOSTATEIMAGEMASK(0), TVIS_STATEIMAGEMASK);

				tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
		}	}

		ShowWindow(hwndTree, SW_SHOW);
		break;

	case DM_HIDEPANE:
		ShowWindow( GetDlgItem(hwndDlg, IDC_SGROUP), SW_HIDE);
		ShowWindow( GetDlgItem(hwndDlg, IDC_NAME), SW_HIDE);
		ShowWindow( GetDlgItem(hwndDlg, IDC_NAMEVAL), SW_HIDE);
		ShowWindow( GetDlgItem(hwndDlg, IDC_SLOC), SW_HIDE);
		ShowWindow( GetDlgItem(hwndDlg, IDC_LOCATION), SW_HIDE);
		ShowWindow( GetDlgItem(hwndDlg, IDC_CHANGE), SW_HIDE);
		ShowWindow( GetDlgItem(hwndDlg, IDC_PREVIEW), SW_HIDE);
		ShowWindow( GetDlgItem(hwndDlg, IDC_GETMORE), SW_HIDE);
		break;

	case DM_SHOWPANE:
		ShowWindow( GetDlgItem(hwndDlg, IDC_SGROUP), SW_SHOW);
		ShowWindow( GetDlgItem(hwndDlg, IDC_NAME), SW_SHOW);
		ShowWindow( GetDlgItem(hwndDlg, IDC_NAMEVAL), SW_SHOW);
		ShowWindow( GetDlgItem(hwndDlg, IDC_SLOC), SW_SHOW);
		ShowWindow( GetDlgItem(hwndDlg, IDC_LOCATION), SW_SHOW);
		ShowWindow( GetDlgItem(hwndDlg, IDC_CHANGE), SW_SHOW);
		ShowWindow( GetDlgItem(hwndDlg, IDC_PREVIEW), SW_SHOW);
		ShowWindow( GetDlgItem(hwndDlg, IDC_GETMORE), SW_SHOW);
		break;

	case DM_CHECKENABLED:
		EnableWindow( GetDlgItem(hwndDlg, IDC_SOUNDTREE), IsDlgButtonChecked(hwndDlg, IDC_ENABLESOUNDS));
		if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_ENABLESOUNDS))
			SendMessage(hwndDlg, DM_HIDEPANE, 0, 0);
		else if (TreeView_GetSelection(hwndTree) && TreeView_GetParent(hwndTree, TreeView_GetSelection(hwndTree)))
			SendMessage(hwndDlg, DM_SHOWPANE, 0, 0);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_ENABLESOUNDS)
			SendMessage(hwndDlg, DM_CHECKENABLED, 0, 0);

		if (LOWORD(wParam) == IDC_PREVIEW) {
			HTREEITEM hti = TreeView_GetSelection(hwndTree);
			if (hti == nullptr)
				break;

			TVITEM tvi = { 0 };
			tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT;
			tvi.hItem = hti;
			if (TreeView_GetItem(hwndTree, &tvi) == FALSE)
				break;
			if (tvi.lParam == -1)
				break;

			if (arSounds[tvi.lParam].ptszTempFile)
				NotifyEventHooks(hPlayEvent, 1, (LPARAM)arSounds[tvi.lParam].ptszTempFile);
			else {
				DBVARIANT dbv;
				if (!db_get_ws(0, "SkinSounds", arSounds[tvi.lParam].name, &dbv)) {
					wchar_t szPathFull[MAX_PATH];
					PathToAbsoluteW(dbv.ptszVal, szPathFull);
					NotifyEventHooks(hPlayEvent, 1, (LPARAM)szPathFull);
					db_free(&dbv);
				}
			}
			break;
		}

		if (LOWORD(wParam) == IDC_CHANGE) {
			HTREEITEM hti = TreeView_GetSelection(hwndTree);
			if (hti == nullptr)
				break;

			TVITEM tvi = { 0 };
			tvi.mask = TVIF_HANDLE|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM|TVIF_TEXT;
			tvi.hItem = hti;
			if (TreeView_GetItem(hwndTree, &tvi) == FALSE)
				break;
			if (tvi.lParam == -1)
				break;

			SoundItem &snd = arSounds[tvi.lParam];

			wchar_t str[MAX_PATH], strFull[MAX_PATH], strdir[MAX_PATH], filter[MAX_PATH];
			if (snd.ptszTempFile)
				wcsncpy_s(strFull, snd.ptszTempFile, _TRUNCATE);
			else {
				if (db_get_b(0, "SkinSoundsOff", snd.name, 0) == 0) {
					DBVARIANT dbv;
					if (db_get_ws(0, "SkinSounds", snd.name, &dbv) == 0) {
						PathToAbsoluteW(dbv.ptszVal, strdir);
						db_free(&dbv);
			}	}	}

			wcsncpy_s(strFull, (snd.ptszTempFile ? snd.ptszTempFile : L""), _TRUNCATE);
			PathToAbsoluteW(strFull, strdir);

			OPENFILENAME ofn;
			memset(&ofn, 0, sizeof(ofn));
			if (GetModuleHandle(L"bass_interface.dll"))
				mir_snwprintf(filter, L"%s (*.wav, *.mp3, *.ogg)%c*.wav;*.mp3;*.ogg%c%s (*)%c*%c", TranslateT("Sound files"), 0, 0, TranslateT("All files"), 0, 0);
			else
				mir_snwprintf(filter, L"%s (*.wav)%c*.wav%c%s (*)%c*%c", TranslateT("WAV files"), 0, 0, TranslateT("All files"), 0, 0);
			ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
			ofn.hwndOwner = GetParent(hwndDlg);
			ofn.hInstance = nullptr;
			ofn.lpstrFilter = filter;

			wchar_t* slash = wcsrchr(strdir, '\\');
			if (slash) {
				*slash = 0;
				ofn.lpstrInitialDir = strdir;
			}

			str[0] = 0;
			ofn.lpstrFile = str;
			ofn.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_EXPLORER|OFN_LONGNAMES|OFN_NOCHANGEDIR;
			ofn.nMaxFile = _countof(str);
			ofn.nMaxFileTitle = MAX_PATH;
			ofn.lpstrDefExt = L"wav";
			if (!GetOpenFileName(&ofn))
				break;

			PathToRelativeW(str, strFull);
			snd.ptszTempFile = mir_wstrdup(strFull);
			SetDlgItemText(hwndDlg, IDC_LOCATION, strFull);
		}
		if (LOWORD(wParam) == IDC_GETMORE) {
			Utils_OpenUrl("https://miranda-ng.org/addons/category/14");
			break;
		}
		if (LOWORD(wParam) == IDC_LOCATION)
			break;

		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			if (((LPNMHDR)lParam)->code == PSN_APPLY) {
				db_set_b(0, "Skin", "UseSound", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ENABLESOUNDS));

				for (int i = 0; i < arSounds.getCount(); i++)
					if (arSounds[i].ptszTempFile)
						db_set_ws(0, "SkinSounds", arSounds[i].name, arSounds[i].ptszTempFile);

				TVITEM tvi, tvic;
				tvi.hItem = TreeView_GetRoot(hwndTree);
				while (tvi.hItem != nullptr) {
					tvi.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_STATE;
					TreeView_GetItem(hwndTree, &tvi);
					if (tvi.lParam == -1) {
						tvic.hItem = TreeView_GetChild(hwndTree, tvi.hItem);
						while (tvic.hItem != nullptr) {
							tvic.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_STATE;
							TreeView_GetItem(hwndTree, &tvic);
							if (((tvic.state & TVIS_STATEIMAGEMASK) >> 12 == 2))
								db_unset(0, "SkinSoundsOff", arSounds[tvic.lParam].name);
							else
								db_set_b(0, "SkinSoundsOff", arSounds[tvic.lParam].name, 1);
							tvic.hItem = TreeView_GetNextSibling(hwndTree, tvic.hItem);
						}
					}

					tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
				}
				return TRUE;
			}
			break;

		case IDC_SOUNDTREE:
			switch (((NMHDR*)lParam)->code) {
			case TVN_SELCHANGED:
				{
					NMTREEVIEW *pnmtv = (NMTREEVIEW*)lParam;
					TVITEM tvi = pnmtv->itemNew;

					if (tvi.lParam == -1)
						SendMessage(hwndDlg, DM_HIDEPANE, 0, 0);
					else {
						wchar_t buf[256];
						mir_snwprintf(buf, L"%s: %s", arSounds[tvi.lParam].getSection(), arSounds[tvi.lParam].getDescr());
						SetDlgItemText(hwndDlg, IDC_NAMEVAL, buf);
						if (arSounds[tvi.lParam].ptszTempFile)
							SetDlgItemText(hwndDlg, IDC_LOCATION, arSounds[tvi.lParam].ptszTempFile);
						else {
							DBVARIANT dbv;
							if (!db_get_ws(0, "SkinSounds", arSounds[tvi.lParam].name, &dbv)) {
								SetDlgItemText(hwndDlg, IDC_LOCATION, dbv.ptszVal);
								db_free(&dbv);
							}
							else SetDlgItemText(hwndDlg, IDC_LOCATION, TranslateT("<not specified>"));
						}
						SendMessage(hwndDlg, DM_SHOWPANE, 0, 0);
					}
				}
				break;
			case TVN_KEYDOWN:
				{
					NMTVKEYDOWN* ptkd = (NMTVKEYDOWN*)lParam;
					if (ptkd && ptkd->wVKey == VK_SPACE && TreeView_GetSelection(ptkd->hdr.hwndFrom))
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				break;
			case NM_CLICK:
				{
					TVHITTESTINFO hti;
					hti.pt.x = (short)LOWORD(GetMessagePos());
					hti.pt.y = (short)HIWORD(GetMessagePos());
					ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
					if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti))
						if (hti.flags & (TVHT_ONITEM | TVHT_ONITEMSTATEICON))
							if (TreeView_GetParent(hwndTree, hti.hItem) != nullptr)
								SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				break;
			}
		}
		break;

	case WM_DESTROY:
		ImageList_Destroy(TreeView_GetImageList(hwndTree, TVSIL_STATE));
		break;
	}
	return FALSE;
}

static int SkinOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = -200000000;
	odp.hInstance = g_hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SOUND);
	odp.szTitle.a = LPGEN("Sounds");
	odp.pfnDlgProc = DlgProcSoundOpts;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}

static int SkinSystemModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_OPT_INITIALISE, SkinOptionsInit);
	return 0;
}

int LoadSkinSounds(void)
{
	HookEvent(ME_SYSTEM_MODULESLOADED, SkinSystemModulesLoaded);
	hPlayEvent = CreateHookableEvent(ME_SKIN_PLAYINGSOUND);
	SetHookDefaultForHookableEvent(hPlayEvent, Skin_PlaySoundDefault);
	return 0;
}

void UnloadSkinSounds(void)
{
	for (int i = 0; i < arSounds.getCount(); i++)
		arSounds[i].clear();
}
