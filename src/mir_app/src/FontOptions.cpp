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
#include "FontService.h"
#include <m_skin_eng.h>

// *_w2 is working copy of list
// *_w3 is stores initial configuration

HWND hwndFontOptions = nullptr;

static int sttCompareFont(const FontInternal *p1, const FontInternal *p2)
{
	int result = mir_wstrcmp(p1->group, p2->group);
	if (result != 0)
		return result;

	result = p1->order - p2->order;
	if (result != 0)
		return result;

	return mir_wstrcmp(p1->getName(), p2->getName());
}

OBJLIST<FontInternal> font_id_list(20, sttCompareFont), font_id_list_w2(20, sttCompareFont), font_id_list_w3(20, sttCompareFont);

static int sttCompareColour(const ColourInternal *p1, const ColourInternal *p2)
{
	int result = mir_wstrcmp(p1->group, p2->group);
	if (result != 0)
		return result;

	result = p1->order - p2->order;
	if (result != 0)
		return result;

	return mir_wstrcmp(p1->getName(), p2->getName());
}

OBJLIST<ColourInternal> colour_id_list(10, sttCompareColour), colour_id_list_w2(10, sttCompareColour), colour_id_list_w3(10, sttCompareColour);

static int sttCompareEffect(const EffectInternal *p1, const EffectInternal *p2)
{
	int result = mir_wstrcmp(p1->group, p2->group);
	if (result != 0)
		return result;

	result = p1->order - p2->order;
	if (result != 0)
		return result;

	return mir_wstrcmp(p1->getName(), p2->getName());
}

OBJLIST<EffectInternal> effect_id_list(10, sttCompareEffect), effect_id_list_w2(10, sttCompareEffect), effect_id_list_w3(10, sttCompareEffect);

/////////////////////////////////////////////////////////////////////////////////////////

void notifyOptions()
{
	if (hwndFontOptions)
		SetTimer(hwndFontOptions, 1, 100, nullptr);
}

void KillModuleFonts(CMPluginBase *pPlugin)
{
	// kill fonts
	for (auto &it : font_id_list.rev_iter())
		if (it->pPlugin == pPlugin) {
			font_id_list.removeItem(&it);
			notifyOptions();
		}

	for (auto &it : font_id_list_w2.rev_iter())
		if (it->pPlugin == pPlugin)
			font_id_list_w2.removeItem(&it);

	for (auto &it : font_id_list_w3.rev_iter())
		if (it->pPlugin == pPlugin)
			font_id_list_w3.removeItem(&it);

	// kill colors
	for (auto &it : colour_id_list.rev_iter())
		if (it->pPlugin == pPlugin) {
			colour_id_list.removeItem(&it);
			notifyOptions();
		}

	for (auto &it : colour_id_list_w2.rev_iter())
		if (it->pPlugin == pPlugin)
			colour_id_list_w2.removeItem(&it);

	for (auto &it : colour_id_list_w3.rev_iter())
		if (it->pPlugin == pPlugin)
			colour_id_list_w3.removeItem(&it);

	// kill effects
	for (auto &it : effect_id_list.rev_iter())
		if (it->pPlugin == pPlugin) {
			effect_id_list.removeItem(&it);
			notifyOptions();
		}

	for (auto &it : effect_id_list_w2.rev_iter())
		if (it->pPlugin == pPlugin)
			effect_id_list_w2.removeItem(&it);

	for (auto &it : effect_id_list_w3.rev_iter())
		if (it->pPlugin == pPlugin)
			effect_id_list_w3.removeItem(&it);
}

/////////////////////////////////////////////////////////////////////////////////////////

#define UM_SETFONTGROUP		(WM_USER + 101)

#define FSUI_COLORBOXWIDTH		50
#define FSUI_COLORBOXLEFT		5
#define FSUI_FONTFRAMEHORZ		5
#define FSUI_FONTFRAMEVERT		4
#define FSUI_FONTLEFT			(FSUI_COLORBOXLEFT+FSUI_COLORBOXWIDTH+5)

void UpdateFontSettings(FontIDW *font_id, FontSettingsW *fontsettings);
void UpdateColourSettings(ColourIDW *colour_id, COLORREF *colour);
void UpdateEffectSettings(EffectIDW *effect_id, FONTEFFECT *effectsettings);

static int CompareObj(const MBaseFontObjectW *p1, const MBaseFontObjectW *p2)
{
	return strcmp(p1->dbSettingsGroup, p2->dbSettingsGroup);
}

static BOOL ExportSettings(HWND hwndDlg, const wchar_t *filename, OBJLIST<FontInternal> &flist, OBJLIST<ColourInternal> &clist, OBJLIST<EffectInternal> &elist)
{
	FILE *out = _wfopen(filename, L"w");
	if (out == nullptr) {
		MessageBoxW(hwndDlg, filename, TranslateT("Failed to create file"), MB_ICONWARNING | MB_OK);
		return FALSE;
	}

	char header[512], buff[1024];
	header[0] = 0;

	fputs("SETTINGS:\n\n", out);

	LIST<MBaseFontObjectW> arObjects(100, CompareObj);
	for (auto &it : flist)
		arObjects.insert(it);
	for (auto &it : clist)
		arObjects.insert(it);
	for (auto &it : elist)
		arObjects.insert(it);

	for (auto &it : arObjects) {
		mir_snprintf(buff, "\n[%s]", it->dbSettingsGroup);
		if (mir_strcmp(buff, header) != 0) {
			strncpy(header, buff, _countof(header));
			fputs(buff, out);
			fputc('\n', out);
		}

		FontInternal *F = (FontInternal*)it;
		if (flist.indexOf(F) != -1) {
			fprintf(out, (F->flags & FIDF_APPENDNAME) ? "%sName=s%S\n" : "%s=s%S\n", F->setting, F->value.szFace);

			int iFontSize;
			if (F->flags & FIDF_SAVEACTUALHEIGHT) {
				LOGFONT lf;
				CreateFromFontSettings(&F->value, &lf);
				HFONT hFont = CreateFontIndirect(&lf);

				HDC hdc = GetDC(hwndDlg);
				HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

				SIZE size;
				GetTextExtentPoint32(hdc, L"_W", 2, &size);
				SelectObject(hdc, hOldFont);
				ReleaseDC(hwndDlg, hdc);
				DeleteObject(hFont);

				iFontSize = size.cy;
			}
			else if (F->flags & FIDF_SAVEPOINTSIZE) {
				HDC hdc = GetDC(hwndDlg);
				iFontSize = (uint8_t)-MulDiv(F->value.size, 72, GetDeviceCaps(hdc, LOGPIXELSY));
				ReleaseDC(hwndDlg, hdc);
			}
			else iFontSize = F->value.size;
			fprintf(out, "%sSize=b%d\n", F->setting, iFontSize);

			fprintf(out, "%sSty=b%d\n", F->setting, F->value.style);
			fprintf(out, "%sSet=b%d\n", F->setting, F->value.charset);
			fprintf(out, "%sCol=d%d\n", F->setting, F->value.colour);

			if (F->flags & FIDF_NOAS)
				fprintf(out, "%sAs=w%d\n", F->setting, 0x00FF);

			fprintf(out, "%sFlags=w%d\n", F->setting, F->flags);
			continue;
		}

		ColourInternal *C = (ColourInternal*)it;
		if (clist.indexOf(C) != -1) {
			fprintf(out, "%s=d%d\n", C->setting, (uint32_t)C->value);
			continue;
		}

		EffectInternal *E = (EffectInternal*)it;
		fprintf(out, "%sEffect=b%d\n", E->setting, E->value.effectIndex);
		fprintf(out, "%sEffectCol1=d%d\n", E->setting, E->value.baseColour);
		fprintf(out, "%sEffectCol2=d%d\n", E->setting, E->value.secondaryColour);
	}

	fclose(out);
	return TRUE;
}

void OptionsChanged()
{
	NotifyEventHooks(hFontReloadEvent, 0, 0);
	NotifyEventHooks(hColourReloadEvent, 0, 0);
}

UINT_PTR CALLBACK CFHookProc(HWND hdlg, UINT uiMsg, WPARAM, LPARAM lParam)
{
	if (uiMsg == WM_INITDIALOG) {
		CHOOSEFONT *cf = (CHOOSEFONT *)lParam;
		if (cf != nullptr) {
			if (cf->lCustData & FIDF_DISABLESTYLES) {
				EnableWindow(GetDlgItem(hdlg, 1137), FALSE);
				ShowWindow(GetDlgItem(hdlg, 1137), SW_HIDE);
				ShowWindow(GetDlgItem(hdlg, 1095), SW_SHOW);
			}
			else if (cf->lCustData & FIDF_ALLOWEFFECTS) {
				EnableWindow(GetDlgItem(hdlg, 1139), FALSE);
				ShowWindow(GetDlgItem(hdlg, 1139), SW_HIDE);
				ShowWindow(GetDlgItem(hdlg, 1091), SW_HIDE);
			}
		}
	}

	return 0;
}

struct FSUIListItemData
{
	int font_id;
	int colour_id;
	int effect_id;
};

static BOOL sttFsuiBindColourIdToFonts(HWND hwndList, const wchar_t *name, const wchar_t *backgroundGroup, const wchar_t *backgroundName, int colourId)
{
	BOOL res = FALSE;
	for (int i = SendMessage(hwndList, LB_GETCOUNT, 0, 0); i--;) {
		FSUIListItemData *itemData = (FSUIListItemData *)SendMessage(hwndList, LB_GETITEMDATA, i, 0);
		if (itemData && itemData->font_id >= 0) {
			FontInternal &F = font_id_list_w2[itemData->font_id];

			if (name && !mir_wstrcmp(F.name, name)) {
				itemData->colour_id = colourId;
				res = TRUE;
			}

			if (backgroundGroup && backgroundName && !mir_wstrcmp(F.backgroundGroup, backgroundGroup) && !mir_wstrcmp(F.backgroundName, backgroundName)) {
				itemData->colour_id = colourId;
				res = TRUE;
			}
		}
	}

	return res;
}

static bool sttFsuiBindEffectIdToFonts(HWND hwndList, const wchar_t *name, int effectId)
{
	for (int i = SendMessage(hwndList, LB_GETCOUNT, 0, 0); i--;) {
		FSUIListItemData *itemData = (FSUIListItemData *)SendMessage(hwndList, LB_GETITEMDATA, i, 0);
		if (itemData && itemData->font_id >= 0) {
			FontInternal &F = font_id_list_w2[itemData->font_id];

			if (name && !mir_wstrcmp(F.name, name)) {
				itemData->effect_id = effectId;
				return true;
			}
		}
	}

	return false;
}

static HTREEITEM sttFindNamedTreeItemAt(HWND hwndTree, HTREEITEM hItem, const wchar_t *name)
{
	TVITEM tvi = { 0 };
	wchar_t str[MAX_PATH];

	if (hItem)
		tvi.hItem = TreeView_GetChild(hwndTree, hItem);
	else
		tvi.hItem = TreeView_GetRoot(hwndTree);

	if (!name)
		return tvi.hItem;

	tvi.mask = TVIF_TEXT;
	tvi.pszText = str;
	tvi.cchTextMax = _countof(str);

	while (tvi.hItem) {
		TreeView_GetItem(hwndTree, &tvi);

		if (!mir_wstrcmp(tvi.pszText, name))
			return tvi.hItem;

		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
	return nullptr;
}

static void sttFsuiCreateSettingsTreeNode(HWND hwndTree, const wchar_t *groupName, HPLUGIN pPlugin)
{
	wchar_t itemName[1024];
	wchar_t *sectionName;
	int sectionLevel = 0;

	HTREEITEM hSection = nullptr;
	mir_wstrcpy(itemName, groupName);
	sectionName = itemName;

	while (sectionName) {
		// allow multi-level tree
		wchar_t *pItemName = sectionName;
		HTREEITEM hItem;

		// one level deeper
		if (sectionName = wcschr(sectionName, '/'))
			*sectionName = 0;

		pItemName = TranslateW_LP(pItemName, pPlugin);

		hItem = sttFindNamedTreeItemAt(hwndTree, hSection, pItemName);
		if (!sectionName || !hItem) {
			if (!hItem) {
				TVINSERTSTRUCT tvis = {};
				TreeItem *treeItem = (TreeItem *)mir_alloc(sizeof(TreeItem));
				treeItem->groupName = sectionName ? nullptr : mir_wstrdup(groupName);
				treeItem->paramName = mir_u2a(itemName);

				tvis.hParent = hSection;
				tvis.hInsertAfter = TVI_SORT;//TVI_LAST;
				tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
				tvis.item.pszText = pItemName;
				tvis.item.lParam = (LPARAM)treeItem;

				hItem = TreeView_InsertItem(hwndTree, &tvis);

				memset(&tvis.item, 0, sizeof(tvis.item));
				tvis.item.hItem = hItem;
				tvis.item.mask = TVIF_HANDLE | TVIF_STATE;
				tvis.item.state = tvis.item.stateMask = db_get_b(0, "FontServiceUI", treeItem->paramName, TVIS_EXPANDED);
				TreeView_SetItem(hwndTree, &tvis.item);
			}
		}

		if (sectionName) {
			*sectionName = '/';
			sectionName++;
		}

		sectionLevel++;

		hSection = hItem;
	}
}

static void sttSaveCollapseState(HWND hwndTree)
{
	HTREEITEM hti;
	TVITEM tvi;

	hti = TreeView_GetRoot(hwndTree);
	while (hti != nullptr) {
		HTREEITEM ht;
		TreeItem *treeItem;

		tvi.mask = TVIF_STATE | TVIF_HANDLE | TVIF_CHILDREN | TVIF_PARAM;
		tvi.hItem = hti;
		tvi.stateMask = (uint32_t)-1;
		TreeView_GetItem(hwndTree, &tvi);

		if (tvi.cChildren > 0) {
			treeItem = (TreeItem *)tvi.lParam;
			if (tvi.state & TVIS_EXPANDED)
				db_set_b(0, "FontServiceUI", treeItem->paramName, TVIS_EXPANDED);
			else
				db_set_b(0, "FontServiceUI", treeItem->paramName, 0);
		}

		ht = TreeView_GetChild(hwndTree, hti);
		if (ht == nullptr) {
			ht = TreeView_GetNextSibling(hwndTree, hti);
			while (ht == nullptr) {
				hti = TreeView_GetParent(hwndTree, hti);
				if (hti == nullptr) break;
				ht = TreeView_GetNextSibling(hwndTree, hti);
			}
		}

		hti = ht;
	}
}

static void sttFreeListItems(HWND hList)
{
	int count = SendMessage(hList, LB_GETCOUNT, 0, 0);
	if (count > 0) {
		for (int idx = 0; idx < count; idx++) {
			LRESULT res = SendMessage(hList, LB_GETITEMDATA, idx, 0);
			FSUIListItemData *itemData = (FSUIListItemData *)res;
			if (itemData && res != LB_ERR)
				mir_free(itemData);
		}
	}
}

static void ShowEffectButton(HWND hwndDlg, BOOL bShow)
{
	ShowWindow(GetDlgItem(hwndDlg, IDC_BKGCOLOUR), bShow ? SW_HIDE : SW_SHOW);
	ShowWindow(GetDlgItem(hwndDlg, IDC_BKGCOLOUR_STATIC), bShow ? SW_HIDE : SW_SHOW);

	ShowWindow(GetDlgItem(hwndDlg, IDC_EFFECT), bShow ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_EFFECT_STATIC), bShow ? SW_SHOW : SW_HIDE);
}

wchar_t *ModernEffectNames[] = { LPGENW("<none>"), LPGENW("Shadow at left"), LPGENW("Shadow at right"), LPGENW("Outline"), LPGENW("Outline smooth"), LPGENW("Smooth bump"), LPGENW("Contour thin"), LPGENW("Contour heavy") };

static INT_PTR CALLBACK ChooseEffectDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static FONTEFFECT *pEffect = nullptr;

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		pEffect = (FONTEFFECT*)lParam;
		{
			for (int i = 0; i < _countof(ModernEffectNames); i++) {
				int itemid = SendDlgItemMessage(hwndDlg, IDC_EFFECT_COMBO, CB_ADDSTRING, 0, (LPARAM)TranslateW(ModernEffectNames[i]));
				SendDlgItemMessage(hwndDlg, IDC_EFFECT_COMBO, CB_SETITEMDATA, itemid, i);
				SendDlgItemMessage(hwndDlg, IDC_EFFECT_COMBO, CB_SETCURSEL, 0, 0);
			}

			int cnt = SendDlgItemMessage(hwndDlg, IDC_EFFECT_COMBO, CB_GETCOUNT, 0, 0);
			for (int i = 0; i < cnt; i++) {
				if (SendDlgItemMessage(hwndDlg, IDC_EFFECT_COMBO, CB_GETITEMDATA, i, 0) == pEffect->effectIndex) {
					SendDlgItemMessage(hwndDlg, IDC_EFFECT_COMBO, CB_SETCURSEL, i, 0);
					break;
				}
			}
		}

		SendDlgItemMessage(hwndDlg, IDC_EFFECT_COLOUR1, CPM_SETCOLOUR, 0, pEffect->baseColour & 0x00FFFFFF);
		SendDlgItemMessage(hwndDlg, IDC_EFFECT_COLOUR2, CPM_SETCOLOUR, 0, pEffect->secondaryColour & 0x00FFFFFF);

		SendDlgItemMessage(hwndDlg, IDC_EFFECT_COLOUR_SPIN1, UDM_SETRANGE, 0, MAKELONG(255, 0));
		SendDlgItemMessage(hwndDlg, IDC_EFFECT_COLOUR_SPIN2, UDM_SETRANGE, 0, MAKELONG(255, 0));
		SendDlgItemMessage(hwndDlg, IDC_EFFECT_COLOUR_SPIN1, UDM_SETPOS, 0, MAKELONG((uint8_t)~((uint8_t)((pEffect->baseColour & 0xFF000000) >> 24)), 0));
		SendDlgItemMessage(hwndDlg, IDC_EFFECT_COLOUR_SPIN2, UDM_SETPOS, 0, MAKELONG((uint8_t)~((uint8_t)((pEffect->secondaryColour & 0xFF000000) >> 24)), 0));
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
				int i = SendDlgItemMessage(hwndDlg, IDC_EFFECT_COMBO, CB_GETCURSEL, 0, 0);
				pEffect->effectIndex = (uint8_t)SendDlgItemMessage(hwndDlg, IDC_EFFECT_COMBO, CB_GETITEMDATA, i, 0);
				pEffect->baseColour = SendDlgItemMessage(hwndDlg, IDC_EFFECT_COLOUR1, CPM_GETCOLOUR, 0, 0) | ((~(uint8_t)SendDlgItemMessage(hwndDlg, IDC_EFFECT_COLOUR_SPIN1, UDM_GETPOS, 0, 0)) << 24);
				pEffect->secondaryColour = SendDlgItemMessage(hwndDlg, IDC_EFFECT_COLOUR2, CPM_GETCOLOUR, 0, 0) | ((~(uint8_t)SendDlgItemMessage(hwndDlg, IDC_EFFECT_COLOUR_SPIN2, UDM_GETPOS, 0, 0)) << 24);
			}
			EndDialog(hwndDlg, IDOK);
			return TRUE;

		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			return TRUE;
		}
		break;
	case WM_DESTROY:
		pEffect = nullptr;
		return TRUE;
	}
	return FALSE;
}

static void sttSaveFontData(HWND hwndDlg, FontInternal &F)
{
	LOGFONT lf;
	char str[128];

	if (F.flags & FIDF_APPENDNAME)
		mir_snprintf(str, "%sName", F.setting);
	else
		strncpy_s(str, F.setting, _TRUNCATE);

	if (db_set_ws(0, F.dbSettingsGroup, str, F.value.szFace)) {
		char buff[1024];
		WideCharToMultiByte(code_page, 0, F.value.szFace, -1, buff, 1024, nullptr, nullptr);
		db_set_s(0, F.dbSettingsGroup, str, buff);
	}

	mir_snprintf(str, "%sSize", F.setting);
	if (F.flags & FIDF_SAVEACTUALHEIGHT) {
		SIZE size;
		CreateFromFontSettings(&F.value, &lf);
		HFONT hFont = CreateFontIndirect(&lf);
		HDC hdc = GetDC(hwndDlg);
		HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
		GetTextExtentPoint32(hdc, L"_W", 2, &size);
		SelectObject(hdc, hOldFont);
		ReleaseDC(hwndDlg, hdc);
		DeleteObject(hFont);

		db_set_b(0, F.dbSettingsGroup, str, (char)size.cy);
	}
	else if (F.flags & FIDF_SAVEPOINTSIZE) {
		HDC hdc = GetDC(hwndDlg);
		db_set_b(0, F.dbSettingsGroup, str, (uint8_t)-MulDiv(F.value.size, 72, GetDeviceCaps(hdc, LOGPIXELSY)));
		ReleaseDC(hwndDlg, hdc);
	}
	else db_set_b(0, F.dbSettingsGroup, str, F.value.size);

	mir_snprintf(str, "%sSty", F.setting);
	db_set_b(0, F.dbSettingsGroup, str, F.value.style);
	mir_snprintf(str, "%sSet", F.setting);
	db_set_b(0, F.dbSettingsGroup, str, F.value.charset);
	mir_snprintf(str, "%sCol", F.setting);
	db_set_dw(0, F.dbSettingsGroup, str, F.value.colour);
	if (F.flags & FIDF_NOAS) {
		mir_snprintf(str, "%sAs", F.setting);
		db_set_w(0, F.dbSettingsGroup, str, (uint16_t)0x00FF);
	}
	mir_snprintf(str, "%sFlags", F.setting);
	db_set_w(0, F.dbSettingsGroup, str, (uint16_t)F.flags);
}

static void RebuildTree(HWND hwndDlg)
{
	font_id_list_w2 = font_id_list;
	font_id_list_w3 = font_id_list;

	colour_id_list_w2 = colour_id_list;
	colour_id_list_w3 = colour_id_list;

	effect_id_list_w2 = effect_id_list;
	effect_id_list_w3 = effect_id_list;

	for (auto &F : font_id_list_w2) {
		// sync settings with database
		UpdateFontSettings(F, &F->value);
		sttFsuiCreateSettingsTreeNode(GetDlgItem(hwndDlg, IDC_FONTGROUP), F->group, F->pPlugin);
	}

	for (auto &C : colour_id_list_w2) {
		// sync settings with database
		UpdateColourSettings(C, &C->value);
		sttFsuiCreateSettingsTreeNode(GetDlgItem(hwndDlg, IDC_FONTGROUP), C->group, C->pPlugin);
	}

	for (auto &E : effect_id_list_w2) {
		// sync settings with database
		UpdateEffectSettings(E, &E->value);
		sttFsuiCreateSettingsTreeNode(GetDlgItem(hwndDlg, IDC_FONTGROUP), E->group, E->pPlugin);
	}
}

static INT_PTR CALLBACK DlgProcLogOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int selCount;
	LOGFONT lf;

	static HBRUSH hBkgColourBrush = nullptr;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		hwndFontOptions = hwndDlg;

		RebuildTree(hwndDlg);

		SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETDEFAULTCOLOUR, 0, GetSysColor(COLOR_WINDOW));
		return TRUE;

	case UM_SETFONTGROUP:
		TreeItem *treeItem;
		{
			TVITEM tvi = { 0 };
			tvi.hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_FONTGROUP));
			tvi.mask = TVIF_HANDLE | TVIF_PARAM;
			TreeView_GetItem(GetDlgItem(hwndDlg, IDC_FONTGROUP), &tvi);
			treeItem = (TreeItem *)tvi.lParam;
			wchar_t *group_buff = treeItem->groupName;

			sttFreeListItems(GetDlgItem(hwndDlg, IDC_FONTLIST));
			SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_RESETCONTENT, 0, 0);

			if (group_buff) {
				BOOL need_restart = FALSE;

				SendDlgItemMessage(hwndDlg, IDC_FONTLIST, WM_SETREDRAW, FALSE, 0);

				for (auto &it : font_id_list_w2) {
					if (wcsncmp(it->group, group_buff, 64))
						continue;

					FSUIListItemData *itemData = (FSUIListItemData*)mir_alloc(sizeof(FSUIListItemData));
					itemData->colour_id = -1;
					itemData->effect_id = -1;
					itemData->font_id = font_id_list_w2.indexOf(&it);
					SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_ADDSTRING, -1, (LPARAM)itemData);
					need_restart |= (it->flags & FIDF_NEEDRESTART);
				}

				if (hBkgColourBrush) {
					DeleteObject(hBkgColourBrush);
					hBkgColourBrush = nullptr;
				}

				for (auto &it : colour_id_list_w2) {
					if (wcsncmp(it->group, group_buff, 64))
						continue;

					int colourId = colour_id_list_w2.indexOf(&it);
					if (!sttFsuiBindColourIdToFonts(GetDlgItem(hwndDlg, IDC_FONTLIST), it->name, it->group, it->name, colourId)) {
						FSUIListItemData *itemData = (FSUIListItemData*)mir_alloc(sizeof(FSUIListItemData));
						itemData->colour_id = colourId;
						itemData->font_id = -1;
						itemData->effect_id = -1;

						SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_ADDSTRING, -1, (LPARAM)itemData);
					}

					if (mir_wstrcmp(it->name, L"Background") == 0)
						hBkgColourBrush = CreateSolidBrush(it->value);
				}

				if (!hBkgColourBrush)
					hBkgColourBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));

				for (auto &it : effect_id_list_w2) {
					if (wcsncmp(it->group, group_buff, 64))
						continue;

					int effectId = effect_id_list_w2.indexOf(&it);
					if (!sttFsuiBindEffectIdToFonts(GetDlgItem(hwndDlg, IDC_FONTLIST), it->name, effectId)) {
						FSUIListItemData *itemData = (FSUIListItemData*)mir_alloc(sizeof(FSUIListItemData));
						itemData->effect_id = effectId;
						itemData->font_id = -1;
						itemData->colour_id = -1;

						SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_ADDSTRING, -1, (LPARAM)itemData);
					}
				}

				SendDlgItemMessage(hwndDlg, IDC_FONTLIST, WM_SETREDRAW, TRUE, 0);
				UpdateWindow(GetDlgItem(hwndDlg, IDC_FONTLIST));

				SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_SETSEL, TRUE, 0);
				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_FONTLIST, LBN_SELCHANGE), 0);
			}
			else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_BKGCOLOUR), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FONTCOLOUR), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHOOSEFONT), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_RESET), FALSE);
				ShowEffectButton(hwndDlg, FALSE);
			}
		}
		return TRUE;

	case WM_MEASUREITEM:
		{
			MEASUREITEMSTRUCT *mis = (MEASUREITEMSTRUCT *)lParam;
			if ((mis->CtlID != IDC_FONTLIST) || (mis->itemID == -1))
				break;

			FSUIListItemData *itemData = (FSUIListItemData *)mis->itemData;
			if (!itemData)
				return FALSE;

			HFONT hFont = nullptr, hoFont = nullptr;
			BOOL bIsFont = FALSE;
			wchar_t *itemName = nullptr;
			if (itemData->font_id >= 0) {
				int iItem = itemData->font_id;
				bIsFont = TRUE;
				CreateFromFontSettings(&font_id_list_w2[iItem].value, &lf);
				hFont = CreateFontIndirect(&lf);
				itemName = font_id_list_w2[iItem].getName();
			}

			if (itemData->colour_id >= 0) {
				int iItem = itemData->colour_id;
				if (!itemName)
					itemName = colour_id_list_w2[iItem].getName();
			}

			HDC hdc = GetDC(GetDlgItem(hwndDlg, mis->CtlID));
			if (hFont)
				hoFont = (HFONT)SelectObject(hdc, hFont);
			else
				hoFont = (HFONT)SelectObject(hdc, (HFONT)SendDlgItemMessage(hwndDlg, mis->CtlID, WM_GETFONT, 0, 0));

			SIZE fontSize;
			GetTextExtentPoint32(hdc, itemName, (int)mir_wstrlen(itemName), &fontSize);
			if (hoFont) SelectObject(hdc, hoFont);
			if (hFont) DeleteObject(hFont);
			ReleaseDC(GetDlgItem(hwndDlg, mis->CtlID), hdc);
			mis->itemWidth = fontSize.cx + 2 * FSUI_FONTFRAMEHORZ + 4;
			mis->itemHeight = fontSize.cy + 2 * FSUI_FONTFRAMEVERT + 4;
		}
		return TRUE;

	case WM_DRAWITEM:
		FONTEFFECT Effect;
		{
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;
			if (dis->CtlID != IDC_FONTLIST)
				break;

			FSUIListItemData *itemData = (FSUIListItemData *)dis->itemData;
			if (!itemData)
				return FALSE;

			HFONT hFont = nullptr, hoFont = nullptr;
			wchar_t *itemName = nullptr;
			COLORREF clBack = (COLORREF)-1;
			COLORREF clText = GetSysColor(COLOR_WINDOWTEXT);
			FONTEFFECT *pEffect = nullptr;

			if (itemData->font_id >= 0) {
				int iItem = itemData->font_id;
				CreateFromFontSettings(&font_id_list_w2[iItem].value, &lf);
				hFont = CreateFontIndirect(&lf);
				itemName = font_id_list_w2[iItem].getName();
				clText = font_id_list_w2[iItem].value.colour;
			}

			if (itemData->colour_id >= 0) {
				int iItem = itemData->colour_id;
				if (hFont)
					clBack = colour_id_list_w2[iItem].value;
				else {
					clText = colour_id_list_w2[iItem].value;
					itemName = colour_id_list_w2[iItem].getName();
				}
			}

			if (itemData->effect_id >= 0) {
				int iItem = itemData->effect_id;

				Effect.effectIndex = effect_id_list_w2[iItem].value.effectIndex;
				Effect.baseColour = effect_id_list_w2[iItem].value.baseColour;
				Effect.secondaryColour = effect_id_list_w2[iItem].value.secondaryColour;
				pEffect = &Effect;

				if (!hFont)
					itemName = effect_id_list_w2[iItem].getName();
			}

			if (hFont)
				hoFont = (HFONT)SelectObject(dis->hDC, hFont);
			else
				hoFont = (HFONT)SelectObject(dis->hDC, (HFONT)SendDlgItemMessage(hwndDlg, dis->CtlID, WM_GETFONT, 0, 0));

			SetBkMode(dis->hDC, TRANSPARENT);

			if (dis->itemState & ODS_SELECTED) {
				SetTextColor(dis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
				FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
			}
			else {
				SetTextColor(dis->hDC, hFont ? clText : GetSysColor(COLOR_WINDOWTEXT));
				if (hFont && (clBack != (COLORREF)-1)) {
					HBRUSH hbrTmp = CreateSolidBrush(clBack);
					FillRect(dis->hDC, &dis->rcItem, hbrTmp);
					DeleteObject(hbrTmp);
				}
				else FillRect(dis->hDC, &dis->rcItem, hFont ? hBkgColourBrush : GetSysColorBrush(COLOR_WINDOW));
			}

			if (hFont) {
				HBRUSH hbrBack;
				RECT rc;

				if (clBack != (COLORREF)-1)
					hbrBack = CreateSolidBrush(clBack);
				else {
					LOGBRUSH lb;
					GetObject(hBkgColourBrush, sizeof(lf), &lb);
					hbrBack = CreateBrushIndirect(&lb);
				}

				SetRect(&rc,
					dis->rcItem.left + FSUI_COLORBOXLEFT,
					dis->rcItem.top + FSUI_FONTFRAMEVERT,
					dis->rcItem.left + FSUI_COLORBOXLEFT + FSUI_COLORBOXWIDTH,
					dis->rcItem.bottom - FSUI_FONTFRAMEVERT);

				FillRect(dis->hDC, &rc, hbrBack);
				DeleteObject(hbrBack);

				FrameRect(dis->hDC, &rc, GetSysColorBrush(COLOR_HIGHLIGHT));
				rc.left += 1;
				rc.top += 1;
				rc.right -= 1;
				rc.bottom -= 1;
				FrameRect(dis->hDC, &rc, GetSysColorBrush(COLOR_HIGHLIGHTTEXT));

				SetTextColor(dis->hDC, clText);

				DrawTextWithEffect(dis->hDC, L"abc", 3, &rc, DT_CENTER | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS, pEffect);

				SelectObject(dis->hDC, hoFont);
				DeleteObject(hFont);
				hoFont = (HFONT)SelectObject(dis->hDC, (HFONT)SendDlgItemMessage(hwndDlg, dis->CtlID, WM_GETFONT, 0, 0));

				if (dis->itemState & ODS_SELECTED) {
					SetTextColor(dis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
					pEffect = nullptr; // Do not draw effect on selected item name text
				}
				rc = dis->rcItem;
				rc.left += FSUI_FONTLEFT;
				DrawTextWithEffect(dis->hDC, itemName, (int)mir_wstrlen(itemName), &rc, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS, pEffect);
			}
			else {
				RECT rc;
				HBRUSH hbrTmp;
				SetRect(&rc,
					dis->rcItem.left + FSUI_COLORBOXLEFT,
					dis->rcItem.top + FSUI_FONTFRAMEVERT,
					dis->rcItem.left + FSUI_COLORBOXLEFT + FSUI_COLORBOXWIDTH,
					dis->rcItem.bottom - FSUI_FONTFRAMEVERT);

				hbrTmp = CreateSolidBrush(clText);
				FillRect(dis->hDC, &rc, hbrTmp);
				DeleteObject(hbrTmp);

				FrameRect(dis->hDC, &rc, GetSysColorBrush(COLOR_HIGHLIGHT));
				rc.left += 1;
				rc.top += 1;
				rc.right -= 1;
				rc.bottom -= 1;
				FrameRect(dis->hDC, &rc, GetSysColorBrush(COLOR_HIGHLIGHTTEXT));

				rc = dis->rcItem;
				rc.left += FSUI_FONTLEFT;

				DrawTextWithEffect(dis->hDC, itemName, (int)mir_wstrlen(itemName), &rc, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS, pEffect);
			}
			
			if (hoFont)
				SelectObject(dis->hDC, hoFont);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_FONTLIST:
			if (HIWORD(wParam) == LBN_SELCHANGE) {
				bool bEnableFont = true;
				bool bEnableClText = true;
				bool bEnableClBack = true;
				bool bEnableEffect = true;
				bool bEnableReset = true;

				COLORREF clBack = 0xffffffff;
				COLORREF clText = 0xffffffff;

				if (selCount = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELCOUNT, 0, 0)) {
					int *selItems = (int *)mir_alloc(font_id_list_w2.getCount() * sizeof(int));
					SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELITEMS, selCount, (LPARAM)selItems);
					for (int i = 0; i < selCount; i++) {
						FSUIListItemData *itemData = (FSUIListItemData *)SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[i], 0);
						if (IsBadReadPtr(itemData, sizeof(FSUIListItemData))) continue; // prevent possible problems with corrupted itemData

						if (bEnableClBack && (itemData->colour_id < 0))
							bEnableClBack = false;
						if (bEnableEffect && (itemData->effect_id < 0))
							bEnableEffect = false;
						if (bEnableFont && (itemData->font_id < 0))
							bEnableFont = false;
						if (!bEnableFont || bEnableClText && (itemData->font_id < 0))
							bEnableClText = false;
						if (bEnableReset && (itemData->font_id >= 0) && !(font_id_list_w2[itemData->font_id].flags & FIDF_DEFAULTVALID))
							bEnableReset = false;

						if (bEnableClBack && (itemData->colour_id >= 0) && (clBack == 0xffffffff))
							clBack = colour_id_list_w2[itemData->colour_id].value;
						if (bEnableClText && (itemData->font_id >= 0) && (clText == 0xffffffff))
							clText = font_id_list_w2[itemData->font_id].value.colour;
					}
					mir_free(selItems);
				}
				else bEnableFont = bEnableClText = bEnableClBack = bEnableReset = bEnableEffect = false;

				EnableWindow(GetDlgItem(hwndDlg, IDC_BKGCOLOUR), bEnableClBack);
				ShowEffectButton(hwndDlg, bEnableEffect && !bEnableClBack);

				EnableWindow(GetDlgItem(hwndDlg, IDC_FONTCOLOUR), bEnableClText);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHOOSEFONT), bEnableFont);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_RESET), bEnableReset);

				if (bEnableClBack) SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETCOLOUR, 0, clBack);
				if (bEnableClText) SendDlgItemMessage(hwndDlg, IDC_FONTCOLOUR, CPM_SETCOLOUR, 0, clText);

				return TRUE;
			}

			if (HIWORD(wParam) != LBN_DBLCLK)
				return TRUE;
			__fallthrough;

		case IDC_CHOOSEFONT:
			if (selCount = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELCOUNT, 0, 0)) {
				int *selItems = (int *)mir_alloc(selCount * sizeof(int));
				SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELITEMS, selCount, (LPARAM)selItems);

				FSUIListItemData *itemData = (FSUIListItemData *)SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[0], 0);
				if (itemData->font_id < 0) {
					mir_free(selItems);
					if (itemData->colour_id >= 0)
						SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, WM_LBUTTONUP, 0, 0);
					return TRUE;
				}

				FontInternal &F = font_id_list_w2[itemData->font_id];
				CreateFromFontSettings(&F.value, &lf);

				CHOOSEFONT cf = { 0 };
				cf.lStructSize = sizeof(cf);
				cf.hwndOwner = hwndDlg;
				cf.lpLogFont = &lf;
				cf.lCustData = F.flags;
				cf.Flags = CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_ENABLEHOOK;
				cf.lpfnHook = CFHookProc;

				if (F.flags & FIDF_ALLOWEFFECTS) // enable effects section
					cf.Flags |= CF_EFFECTS;
				else if (F.flags & FIDF_DISABLESTYLES) { // mutually exclusive with FIDF_ALLOWEFFECTS
					cf.Flags |= CF_TTONLY | CF_NOOEMFONTS;
					lf.lfWeight = FW_NORMAL;
					lf.lfItalic = lf.lfUnderline = lf.lfStrikeOut = FALSE;
				}

				if (ChooseFontW(&cf)) {
					for (int i = 0; i < selCount; i++) {
						FSUIListItemData *pItem = (FSUIListItemData *)SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[i], 0);
						if (pItem->font_id < 0)
							continue;

						FontInternal &F1 = font_id_list_w2[pItem->font_id];
						F1.value.size = (char)lf.lfHeight;
						F1.value.style = (lf.lfWeight >= FW_BOLD ? DBFONTF_BOLD : 0) | (lf.lfItalic ? DBFONTF_ITALIC : 0) | (lf.lfUnderline ? DBFONTF_UNDERLINE : 0) | (lf.lfStrikeOut ? DBFONTF_STRIKEOUT : 0);
						F1.value.charset = lf.lfCharSet;
						wcsncpy_s(F1.value.szFace, lf.lfFaceName, _TRUNCATE);

						MEASUREITEMSTRUCT mis = { 0 };
						mis.CtlID = IDC_FONTLIST;
						mis.itemID = selItems[i];
						mis.itemData = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[i], 0);
						SendMessage(hwndDlg, WM_MEASUREITEM, 0, (LPARAM)&mis);
						SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_SETITEMHEIGHT, selItems[i], mis.itemHeight);
					}
					InvalidateRect(GetDlgItem(hwndDlg, IDC_FONTLIST), nullptr, TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UNDO), TRUE);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}

				mir_free(selItems);
			}
			return TRUE;

		case IDC_EFFECT:
			if (selCount = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELCOUNT, 0, 0)) {
				int *selItems = (int *)mir_alloc(selCount * sizeof(int));
				SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELITEMS, selCount, (LPARAM)selItems);
				FSUIListItemData *itemData = (FSUIListItemData *)SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[0], 0);
				EffectInternal &E = effect_id_list_w2[itemData->effect_id];

				FONTEFFECT es = E.value;
				if (IDOK == DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CHOOSE_FONT_EFFECT), hwndDlg, ChooseEffectDlgProc, (LPARAM)&es)) {
					for (int i = 0; i < selCount; i++) {
						itemData = (FSUIListItemData *)SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[i], 0);
						if (itemData->effect_id < 0)
							continue;

						EffectInternal &E1 = effect_id_list_w2[itemData->effect_id];
						E1.value = es;
					}
					InvalidateRect(GetDlgItem(hwndDlg, IDC_FONTLIST), nullptr, TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UNDO), TRUE);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}

				mir_free(selItems);
			}
			return TRUE;

		case IDC_FONTCOLOUR:
			if (selCount = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELCOUNT, 0, 0)) {
				int *selItems = (int *)mir_alloc(selCount * sizeof(int));
				SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELITEMS, selCount, (LPARAM)selItems);
				for (int i = 0; i < selCount; i++) {
					FSUIListItemData *itemData = (FSUIListItemData *)SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[i], 0);
					if (itemData->font_id < 0) continue;
					font_id_list_w2[itemData->font_id].value.colour = SendDlgItemMessage(hwndDlg, IDC_FONTCOLOUR, CPM_GETCOLOUR, 0, 0);
				}
				mir_free(selItems);
				InvalidateRect(GetDlgItem(hwndDlg, IDC_FONTLIST), nullptr, FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UNDO), TRUE);
			}
			break;

		case IDC_BKGCOLOUR:
			if (selCount = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELCOUNT, 0, 0)) {
				int *selItems = (int *)mir_alloc(selCount * sizeof(int));
				SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELITEMS, selCount, (LPARAM)selItems);
				for (int i = 0; i < selCount; i++) {
					FSUIListItemData *itemData = (FSUIListItemData *)SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[i], 0);
					if (itemData->colour_id < 0) continue;
					colour_id_list_w2[itemData->colour_id].value = SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_GETCOLOUR, 0, 0);

					if (mir_wstrcmp(colour_id_list_w2[itemData->colour_id].name, L"Background") == 0) {
						if (hBkgColourBrush) DeleteObject(hBkgColourBrush);
						hBkgColourBrush = CreateSolidBrush(colour_id_list_w2[itemData->colour_id].value);
					}
				}
				mir_free(selItems);
				InvalidateRect(GetDlgItem(hwndDlg, IDC_FONTLIST), nullptr, FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UNDO), TRUE);
			}
			break;

		case IDC_BTN_RESET:
			if (font_id_list_w2.getCount() && (selCount = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELCOUNT, 0, 0))) {
				int *selItems = (int *)mir_alloc(font_id_list_w2.getCount() * sizeof(int));
				SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELITEMS, selCount, (LPARAM)selItems);
				for (int i = 0; i < selCount; i++) {
					FSUIListItemData *itemData = (FSUIListItemData *)SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[i], 0);
					if (IsBadReadPtr(itemData, sizeof(FSUIListItemData))) continue; // prevent possible problems with corrupted itemData

					if ((itemData->font_id >= 0) && (font_id_list_w2[itemData->font_id].flags & FIDF_DEFAULTVALID)) {
						font_id_list_w2[itemData->font_id].value = font_id_list_w2[itemData->font_id].deffontsettings;

						MEASUREITEMSTRUCT mis = { 0 };
						mis.CtlID = IDC_FONTLIST;
						mis.itemID = selItems[i];
						mis.itemData = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[i], 0);
						SendMessage(hwndDlg, WM_MEASUREITEM, 0, (LPARAM)&mis);
						SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_SETITEMHEIGHT, selItems[i], mis.itemHeight);
					}

					if (itemData->colour_id >= 0)
						colour_id_list_w2[itemData->colour_id].value = colour_id_list_w2[itemData->colour_id].defcolour;

					if (itemData->effect_id >= 0)
						effect_id_list_w2[itemData->effect_id].value = effect_id_list_w2[itemData->effect_id].defeffect;

				}
				mir_free(selItems);
				InvalidateRect(GetDlgItem(hwndDlg, IDC_FONTLIST), nullptr, TRUE);
				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_FONTLIST, LBN_SELCHANGE), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UNDO), TRUE);
			}
			break;

		case IDC_BTN_EXPORT:
			{
				wchar_t fname_buff[MAX_PATH], filter[MAX_PATH];
				mir_snwprintf(filter, L"%s (*.ini)%c*.ini%c%s (*.txt)%c*.TXT%c%s (*.*)%c*.*%c", TranslateT("Configuration files"), 0, 0, TranslateT("Text files"), 0, 0, TranslateT("All files"), 0, 0);

				OPENFILENAME ofn = { 0 };
				ofn.lStructSize = sizeof(ofn);
				ofn.lpstrFile = fname_buff;
				ofn.lpstrFile[0] = '\0';
				ofn.nMaxFile = MAX_PATH;
				ofn.hwndOwner = hwndDlg;
				ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT;
				ofn.lpstrFilter = filter;
				ofn.nFilterIndex = 1;

				ofn.lpstrDefExt = L"ini";

				if (GetSaveFileName(&ofn) == TRUE)
					if (!ExportSettings(hwndDlg, ofn.lpstrFile, font_id_list, colour_id_list, effect_id_list))
						MessageBoxW(hwndDlg, TranslateT("Error writing file"), TranslateT("Error"), MB_ICONWARNING | MB_OK);
			}
			return TRUE;

		case IDC_BTN_UNDO:
			font_id_list_w2 = font_id_list_w3;
			colour_id_list_w2 = colour_id_list_w3;
			effect_id_list_w2 = effect_id_list_w3;
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UNDO), FALSE);

			SendMessage(hwndDlg, UM_SETFONTGROUP, 0, 0);
			break;
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0 && ((LPNMHDR)lParam)->code == PSN_APPLY) {
			char str[32];

			font_id_list_w3 = font_id_list;
			colour_id_list_w3 = colour_id_list;
			effect_id_list_w3 = effect_id_list;

			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UNDO), TRUE);

			font_id_list = font_id_list_w2;
			colour_id_list = colour_id_list_w2;
			effect_id_list = effect_id_list_w2;

			for (auto &F : font_id_list_w2)
				sttSaveFontData(hwndDlg, *F);

			for (auto &C : colour_id_list_w2)
				db_set_dw(0, C->dbSettingsGroup, C->setting, C->value);

			for (auto &E : effect_id_list_w2) {
				mir_snprintf(str, "%sEffect", E->setting);
				db_set_b(0, E->dbSettingsGroup, str, E->value.effectIndex);

				mir_snprintf(str, "%sEffectCol1", E->setting);
				db_set_dw(0, E->dbSettingsGroup, str, E->value.baseColour);

				mir_snprintf(str, "%sEffectCol2", E->setting);
				db_set_dw(0, E->dbSettingsGroup, str, E->value.secondaryColour);
			}

			OptionsChanged();
			return TRUE;
		}

		if (((LPNMHDR)lParam)->idFrom == IDC_FONTGROUP) {
			switch (((NMHDR*)lParam)->code) {
			case TVN_SELCHANGED:
				SendMessage(hwndDlg, UM_SETFONTGROUP, 0, 0);
				break;

			case TVN_DELETEITEM:
				treeItem = (TreeItem *)(((LPNMTREEVIEW)lParam)->itemOld.lParam);
				if (treeItem) {
					mir_free(treeItem->groupName);
					mir_free(treeItem->paramName);
					mir_free(treeItem);
				}
				break;
			}
		}
		break;

	case WM_TIMER:
		if (wParam == 1) {
			KillTimer(hwndDlg, wParam);
			TreeView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_FONTGROUP));
			RebuildTree(hwndDlg);
		}
		break;

	case WM_DESTROY:
		hwndFontOptions = nullptr;
		sttSaveCollapseState(GetDlgItem(hwndDlg, IDC_FONTGROUP));
		DeleteObject(hBkgColourBrush);
		font_id_list_w2.destroy();
		font_id_list_w3.destroy();
		colour_id_list_w2.destroy();
		colour_id_list_w3.destroy();
		effect_id_list_w2.destroy();
		effect_id_list_w3.destroy();
		sttFreeListItems(GetDlgItem(hwndDlg, IDC_FONTLIST));
		break;
	}
	return FALSE;
}

int OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = -790000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_FONTS);
	odp.szTitle.a = LPGEN("Fonts and colors");
	odp.szGroup.a = LPGEN("Customize");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgProcLogOptions;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
