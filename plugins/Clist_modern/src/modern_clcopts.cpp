/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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
#include "modern_effectenum.h"
#include "modern_sync.h"

#pragma comment(lib, "shlwapi.lib")

INT_PTR CALLBACK DlgProcSBarOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#define FONTF_NORMAL       0
#define FONTF_BOLD         1
#define FONTF_ITALIC       2
#define FONTF_UNDERLINE    4
							      
#define CLCGROUP           LPGENW("Contact list") L"/" LPGENW("Contact names")
#define CLCLINESGROUP      LPGENW("Contact list") L"/" LPGENW("Row items")
#define CLCFRAMESGROUP     LPGENW("Contact list") L"/" LPGENW("Frame texts")
#define CLCCOLOURSGROUP    LPGENW("Contact list") L"/" LPGENW("Special colors")

#define DEFAULT_COLOUR		RGB(0, 0, 0)
#define DEFAULT_GREYCOLOUR	RGB(128, 128, 128)
#define DEFAULT_BACKCOLOUR	RGB(255, 255, 255)

#define DEFAULT_FAMILY     L"Arial"
#define DEFAULT_EFFECT     { 0, 0x00000000, 0x00000000 }

#define DEFAULT_SIZE       -11
#define DEFAULT_SMALLSIZE  -8

struct
{
	uint32_t dwFlags;
	int fontID;
	const wchar_t *szGroup;
	const wchar_t *szDescr;
	COLORREF defColour;
	const wchar_t *szDefFace;
	uint8_t defCharset, defStyle;
	char defSize;
	FONTEFFECT defeffect;

	COLORREF colour;
	wchar_t szFace[LF_FACESIZE];
	uint8_t charset, style;
	char size;
}
static fontOptionsList[] =
{
	{ FIDF_CLASSGENERAL, FONTID_CONTACTS, CLCGROUP, LPGENW("Standard contacts"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_AWAY, CLCGROUP, LPGENW("Away contacts"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_DND, CLCGROUP, LPGENW("Do not disturb contacts"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_NA, CLCGROUP, LPGENW("Not available contacts"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_OCCUPIED, CLCGROUP, LPGENW("Occupied contacts"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_CHAT, CLCGROUP, LPGENW("Free for chat contacts"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_INVISIBLE, CLCGROUP, LPGENW("Invisible contacts"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_OFFLINE, CLCGROUP, LPGENW("Offline contacts"), DEFAULT_GREYCOLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_INVIS, CLCGROUP, LPGENW("Online contacts to whom you have a different visibility"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_OFFINVIS, CLCGROUP, LPGENW("Offline contacts to whom you have a different visibility"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_NOTONLIST, CLCGROUP, LPGENW("Contacts who are 'not on list'"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSHEADER, FONTID_OPENGROUPS, CLCGROUP, LPGENW("Open groups"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_BOLD, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSHEADER, FONTID_OPENGROUPCOUNTS, CLCGROUP, LPGENW("Open group member counts"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSHEADER, FONTID_CLOSEDGROUPS, CLCGROUP, LPGENW("Closed groups"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_BOLD, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSHEADER, FONTID_CLOSEDGROUPCOUNTS, CLCGROUP, LPGENW("Closed group member counts"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSSMALL, FONTID_DIVIDERS, CLCGROUP, LPGENW("Dividers"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },

	{ FIDF_CLASSSMALL, FONTID_SECONDLINE, CLCLINESGROUP, LPGENW("Second line"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SMALLSIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSSMALL, FONTID_THIRDLINE, CLCLINESGROUP, LPGENW("Third line"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SMALLSIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSSMALL, FONTID_CONTACT_TIME, CLCLINESGROUP, LPGENW("Contact time"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SMALLSIZE, DEFAULT_EFFECT },

	{ FIDF_CLASSGENERAL, FONTID_STATUSBAR_PROTONAME, CLCFRAMESGROUP, LPGENW("Status bar text"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_EVENTAREA, CLCFRAMESGROUP, LPGENW("Event area text"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_VIEMODES, CLCFRAMESGROUP, LPGENW("Current view mode text"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
};

/////////////////////////////////////////////////////////////////////////////////////////

struct
{
	char *chGroup;
	char *chName;
	wchar_t *szGroup;
	wchar_t *szDescr;
	COLORREF defColour;
}
static colourOptionsList[] =
{
	{ "CLC", "BkColour", CLCGROUP, LPGENW("Background"), CLCDEFAULT_BKCOLOUR },

	{ "CLC", "HotTextColour", CLCCOLOURSGROUP, LPGENW("Hot text"), CLCDEFAULT_MODERN_HOTTEXTCOLOUR },
	{ "CLC", "SelTextColour", CLCCOLOURSGROUP, LPGENW("Selected text"), CLCDEFAULT_MODERN_SELTEXTCOLOUR },
	{ "CLC", "QuickSearchColour", CLCCOLOURSGROUP, LPGENW("Quick search text"), CLCDEFAULT_MODERN_QUICKSEARCHCOLOUR },

	{ "Menu", "TextColour", CLCCOLOURSGROUP, LPGENW("Menu text"), CLCDEFAULT_TEXTCOLOUR },
	{ "Menu", "SelTextColour", CLCCOLOURSGROUP, LPGENW("Selected menu text"), CLCDEFAULT_MODERN_SELTEXTCOLOUR },
	{ "FrameTitleBar", "TextColour", CLCCOLOURSGROUP, LPGENW("Frame title text"), CLCDEFAULT_TEXTCOLOUR },
	{ "ModernSettings", "KeyColor", CLCCOLOURSGROUP, LPGENW("3rd party frames transparent back color"), SETTING_KEYCOLOR_DEFAULT }
};

void RegisterCLUIFonts(void)
{
	static bool registered = false;
	if (registered)
		return;

	FontIDW fontid = {};
	EffectIDW effectid = {};
	char idstr[10];

	strncpy_s(fontid.dbSettingsGroup, "CLC", _TRUNCATE);
	strncpy_s(effectid.dbSettingsGroup, "CLC", _TRUNCATE);

	int index = 1;
	for (auto &p : fontOptionsList) {
		fontid.flags = FIDF_DEFAULTVALID | FIDF_APPENDNAME | FIDF_SAVEPOINTSIZE | FIDF_ALLOWEFFECTS | FIDF_ALLOWREREGISTER | FIDF_NOAS;
		fontid.flags |= p.dwFlags;

		wcsncpy_s(fontid.group, p.szGroup, _TRUNCATE);
		wcsncpy_s(fontid.name, p.szDescr, _TRUNCATE);
		mir_snprintf(idstr, "Font%d", p.fontID);
		strncpy_s(fontid.setting, idstr, _TRUNCATE);
		fontid.order = index;

		fontid.deffontsettings.charset = p.defCharset;
		fontid.deffontsettings.colour = p.defColour;
		fontid.deffontsettings.size = p.defSize;
		fontid.deffontsettings.style = p.defStyle;
		wcsncpy_s(fontid.deffontsettings.szFace, p.szDefFace, _TRUNCATE);

		g_plugin.addFont(&fontid);

		wcsncpy_s(effectid.group, p.szGroup, _TRUNCATE);
		wcsncpy_s(effectid.name, p.szDescr, _TRUNCATE);
		mir_snprintf(idstr, "Font%d", p.fontID);
		strncpy_s(effectid.setting, idstr, _TRUNCATE);
		effectid.order = index;

		effectid.defeffect.effectIndex = p.defeffect.effectIndex;
		effectid.defeffect.baseColour = p.defeffect.baseColour;
		effectid.defeffect.secondaryColour = p.defeffect.secondaryColour;

		g_plugin.addEffect(&effectid);
		index++;
	}

	ColourIDW colourid = {};

	index = 1;
	for (auto &p : colourOptionsList) {
		wcsncpy_s(colourid.group, p.szGroup, _TRUNCATE);
		wcsncpy_s(colourid.name, p.szDescr, _TRUNCATE);
		strncpy_s(colourid.setting, p.chName, _TRUNCATE);
		strncpy_s(colourid.dbSettingsGroup, p.chGroup, _TRUNCATE);
		colourid.defcolour = p.defColour;
		colourid.order = index++;
		g_plugin.addColor(&colourid);
	}
	registered = true;
}

uint32_t GetDefaultExStyle(void)
{
	BOOL param;
	uint32_t ret = CLCDEFAULT_EXSTYLE;
	if (SystemParametersInfo(SPI_GETLISTBOXSMOOTHSCROLLING, 0, &param, FALSE) && !param)
		ret |= CLS_EX_NOSMOOTHSCROLLING;
	if (SystemParametersInfo(SPI_GETHOTTRACKING, 0, &param, FALSE) && !param)
		ret &= ~CLS_EX_TRACKSELECT;
	return ret;
}

void GetFontSetting(int i, LOGFONT *lf, COLORREF *colour, uint8_t *effect, COLORREF *eColour1, COLORREF *eColour2)
{
	for (auto &p : fontOptionsList) {
		if (p.fontID == i) {
			COLORREF col = Font_GetW(p.szGroup, p.szDescr, lf);

			if (colour)
				*colour = col;

			if (effect) {
				char idstr[32];
				mir_snprintf(idstr, "Font%dEffect", i);
				*effect = db_get_b(0, "CLC", idstr, 0);
				mir_snprintf(idstr, "Font%dEffectCol1", i);
				*eColour1 = db_get_dw(0, "CLC", idstr, 0);
				mir_snprintf(idstr, "Font%dEffectCol2", i);
				*eColour2 = db_get_dw(0, "CLC", idstr, 0);
			}
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

struct
{
	int id;
	uint32_t flag;
	int neg;
}
static const checkBoxToStyleEx[] =
{
	{ IDC_DISABLEDRAGDROP, CLS_EX_DISABLEDRAGDROP, 0 },
	{ IDC_NOTEDITLABELS, CLS_EX_EDITLABELS, 1 },
	{ IDC_SHOWSELALWAYS, CLS_EX_SHOWSELALWAYS, 0 },
	{ IDC_TRACKSELECT, CLS_EX_TRACKSELECT, 0 },
	{ IDC_SHOWGROUPCOUNTS, CLS_EX_SHOWGROUPCOUNTS, 0 },
	{ IDC_HIDECOUNTSWHENEMPTY, CLS_EX_HIDECOUNTSWHENEMPTY, 0 },
	{ IDC_DIVIDERONOFF, CLS_EX_DIVIDERONOFF, 0 },
	{ IDC_NOTNOTRANSLUCENTSEL, CLS_EX_NOTRANSLUCENTSEL, 1 },
	{ IDC_LINEWITHGROUPS, CLS_EX_LINEWITHGROUPS, 0 },
	{ IDC_QUICKSEARCHVISONLY, CLS_EX_QUICKSEARCHVISONLY, 0 },
	{ IDC_SORTGROUPSALPHA, CLS_EX_SORTGROUPSALPHA, 0 },
	{ IDC_NOTNOSMOOTHSCROLLING, CLS_EX_NOSMOOTHSCROLLING, 1 }
};

/////////////////////////////////////////////////////////////////////////////////////////

struct CheckBoxValues_t
{
	uint32_t style;
	wchar_t *szDescr;
}
static const greyoutValues[] = {
	{ GREYF_UNFOCUS, LPGENW("Not focused") },
	{ MODEF_OFFLINE, LPGENW("Offline") },
	{ PF2_ONLINE, LPGENW("Online") },
	{ PF2_SHORTAWAY, LPGENW("Away") },
	{ PF2_LONGAWAY, LPGENW("Not available") },
	{ PF2_LIGHTDND, LPGENW("Occupied") },
	{ PF2_HEAVYDND, LPGENW("Do not disturb") },
	{ PF2_FREECHAT, LPGENW("Free for chat") },
	{ PF2_INVISIBLE, LPGENW("Invisible") }
};

static void FillCheckBoxTree(HWND hwndTree, const struct CheckBoxValues_t *values, int nValues, uint32_t style)
{
	TVINSERTSTRUCT tvis;
	tvis.hParent = nullptr;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
	for (int i = 0; i < nValues; i++) {
		tvis.item.lParam = values[i].style;
		tvis.item.pszText = TranslateW(values[i].szDescr);
		tvis.item.stateMask = TVIS_STATEIMAGEMASK;
		tvis.item.state = INDEXTOSTATEIMAGEMASK((style & tvis.item.lParam) != 0 ? 2 : 1);
		TreeView_InsertItem(hwndTree, &tvis);
	}
}

static uint32_t MakeCheckBoxTreeFlags(HWND hwndTree)
{
	uint32_t flags = 0;
	TVITEM tvi;
	tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
	tvi.hItem = TreeView_GetRoot(hwndTree);
	while (tvi.hItem) {
		TreeView_GetItem(hwndTree, &tvi);
		if ((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 2)
			flags |= tvi.lParam;
		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
	return flags;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void CheckButtons(HWND hwndDlg)
{
	uint8_t t = IsDlgButtonChecked(hwndDlg, IDC_METAEXPAND);
	EnableWindow(GetDlgItem(hwndDlg, IDC_METADBLCLK), t);
	EnableWindow(GetDlgItem(hwndDlg, IDC_METASUBEXTRA), t);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SUBINDENTSPIN), t);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SUBINDENT), t);
}

static INT_PTR CALLBACK DlgProcClistAdditionalOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_META, db_get_b(0, "CLC", "Meta", SETTING_USEMETAICON_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR
		CheckDlgButton(hwndDlg, IDC_METADBLCLK, db_get_b(0, "CLC", "MetaDoubleClick", SETTING_METAAVOIDDBLCLICK_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR
		CheckDlgButton(hwndDlg, IDC_METASUBEXTRA, db_get_b(0, "CLC", "MetaHideExtra", SETTING_METAHIDEEXTRA_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR
		CheckDlgButton(hwndDlg, IDC_METASUBEXTRA_IGN, db_get_b(0, "CLC", "MetaIgnoreEmptyExtra", SETTING_METAAVOIDDBLCLICK_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR
		CheckDlgButton(hwndDlg, IDC_METAEXPAND, db_get_b(0, "CLC", "MetaExpanding", SETTING_METAEXPANDING_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR
		CheckDlgButton(hwndDlg, IDC_DISCOVER_AWAYMSG, db_get_b(0, "ModernData", "InternalAwayMsgDiscovery", SETTING_INTERNALAWAYMSGREQUEST_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR
		CheckDlgButton(hwndDlg, IDC_REMOVE_OFFLINE_AWAYMSG, db_get_b(0, "ModernData", "RemoveAwayMessageForOffline", SETTING_REMOVEAWAYMSGFOROFFLINE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR

		SendDlgItemMessage(hwndDlg, IDC_SUBINDENTSPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_SUBINDENT), 0);
		SendDlgItemMessage(hwndDlg, IDC_SUBINDENTSPIN, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_SUBINDENTSPIN, UDM_SETPOS, 0, MAKELONG(db_get_b(0, "CLC", "SubIndent", CLCDEFAULT_GROUPINDENT), 0));
		CheckButtons(hwndDlg);
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_METAEXPAND)
			CheckButtons(hwndDlg);

		if ((LOWORD(wParam) == IDC_SUBINDENT) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		return TRUE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				db_set_b(0, "CLC", "Meta", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_META));
				db_set_b(0, "CLC", "MetaDoubleClick", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_METADBLCLK));
				db_set_b(0, "CLC", "MetaHideExtra", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_METASUBEXTRA));
				db_set_b(0, "CLC", "MetaIgnoreEmptyExtra", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_METASUBEXTRA_IGN));
				db_set_b(0, "CLC", "MetaExpanding", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_METAEXPAND));
				db_set_b(0, "ModernData", "InternalAwayMsgDiscovery", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_DISCOVER_AWAYMSG));
				db_set_b(0, "ModernData", "RemoveAwayMessageForOffline", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_REMOVE_OFFLINE_AWAYMSG));

				db_set_b(0, "CLC", "SubIndent", (uint8_t)SendDlgItemMessage(hwndDlg, IDC_SUBINDENTSPIN, UDM_GETPOS, 0, 0));
				ClcOptionsChanged();
				CLUI_ReloadCLUIOptions();
				PostMessage(g_clistApi.hwndContactList, WM_SIZE, 0, 0);
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK DlgProcClistListOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		{
			uint32_t exStyle = db_get_dw(0, "CLC", "ExStyle", GetDefaultExStyle());
			for (auto &it : checkBoxToStyleEx)
				CheckDlgButton(hwndDlg, it.id, (exStyle & it.flag) ^ (it.flag * it.neg) ? BST_CHECKED : BST_UNCHECKED);

			UDACCEL accel[2] = { { 0, 10 }, { 2, 50 } };
			SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_SETRANGE, 0, MAKELONG(999, 0));
			SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_SETACCEL, _countof(accel), (LPARAM)&accel);
			SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_SETPOS, 0, MAKELONG(db_get_w(0, "CLC", "ScrollTime", CLCDEFAULT_SCROLLTIME), 0));
		}
		CheckDlgButton(hwndDlg, IDC_IDLE, db_get_b(0, "CLC", "ShowIdle", CLCDEFAULT_SHOWIDLE) ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg, IDC_GROUPINDENTSPIN, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_GROUPINDENTSPIN, UDM_SETPOS, 0, MAKELONG(db_get_b(0, "CLC", "GroupIndent", CLCDEFAULT_GROUPINDENT), 0));
		CheckDlgButton(hwndDlg, IDC_GREYOUT, db_get_dw(0, "CLC", "GreyoutFlags", CLCDEFAULT_GREYOUTFLAGS) ? BST_CHECKED : BST_UNCHECKED);

		EnableWindow(GetDlgItem(hwndDlg, IDC_SMOOTHTIME), IsDlgButtonChecked(hwndDlg, IDC_NOTNOSMOOTHSCROLLING));
		EnableWindow(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), IsDlgButtonChecked(hwndDlg, IDC_GREYOUT));
		FillCheckBoxTree(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), greyoutValues, _countof(greyoutValues), db_get_dw(0, "CLC", "FullGreyoutFlags", CLCDEFAULT_FULLGREYOUTFLAGS));
		CheckDlgButton(hwndDlg, IDC_NOSCROLLBAR, db_get_b(0, "CLC", "NoVScrollBar", CLCDEFAULT_NOVSCROLL) ? BST_CHECKED : BST_UNCHECKED);
		return TRUE;

	case WM_VSCROLL:
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_NOTNOSMOOTHSCROLLING)
			EnableWindow(GetDlgItem(hwndDlg, IDC_SMOOTHTIME), IsDlgButtonChecked(hwndDlg, IDC_NOTNOSMOOTHSCROLLING));
		if (LOWORD(wParam) == IDC_GREYOUT)
			EnableWindow(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), IsDlgButtonChecked(hwndDlg, IDC_GREYOUT));
		if ((LOWORD(wParam) == IDC_SMOOTHTIME || LOWORD(wParam) == IDC_GROUPINDENT) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())) return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_GREYOUTOPTS:
			if (((LPNMHDR)lParam)->code == NM_CLICK) {
				TVHITTESTINFO hti;
				hti.pt.x = (short)LOWORD(GetMessagePos());
				hti.pt.y = (short)HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
				if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti)) {
					if (hti.flags & TVHT_ONITEMICON) {
						TVITEMA tvi;
						tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
						tvi.hItem = hti.hItem;
						TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);
						tvi.iSelectedImage = tvi.iImage = !tvi.iImage;
						TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
					}
				}
			}
			break;

		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				uint32_t exStyle = 0;
				for (auto &it : checkBoxToStyleEx)
					if ((IsDlgButtonChecked(hwndDlg, it.id) == 0) == it.neg)
						exStyle |= it.flag;
				db_set_dw(0, "CLC", "ExStyle", exStyle);

				uint32_t fullGreyoutFlags = MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS));
				db_set_dw(0, "CLC", "FullGreyoutFlags", fullGreyoutFlags);
				if (IsDlgButtonChecked(hwndDlg, IDC_GREYOUT))
					db_set_dw(0, "CLC", "GreyoutFlags", fullGreyoutFlags);
				else
					db_set_dw(0, "CLC", "GreyoutFlags", 0);

				db_set_b(0, "CLC", "ShowIdle", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_IDLE) ? 1 : 0));
				db_set_w(0, "CLC", "ScrollTime", (uint16_t)SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_GETPOS, 0, 0));
				db_set_b(0, "CLC", "GroupIndent", (uint8_t)SendDlgItemMessage(hwndDlg, IDC_GROUPINDENTSPIN, UDM_GETPOS, 0, 0));
				db_set_b(0, "CLC", "NoVScrollBar", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_NOSCROLLBAR) ? 1 : 0));

				ClcOptionsChanged();
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int _GetNetVisibleProtoCount()
{
	int netProtoCount = 0;
	for (auto &pa : Accounts())
		if (pa->IsVisible())
			netProtoCount++;

	return netProtoCount;
}

static wchar_t* sortby[] = 
{
	LPGENW("Name"), 
	LPGENW("Name (use locale settings)"), 
	LPGENW("Status"), 
	LPGENW("Last message time"), 
	LPGENW("Account name"), 
	LPGENW("Rate"), 
	LPGENW("Last online"), 
	LPGENW("-Nothing-")
};

static int sortbyValue[] = { SORTBY_NAME, SORTBY_NAME_LOCALE, SORTBY_STATUS, SORTBY_LASTMSG, SORTBY_PROTO, SORTBY_RATE, SORTBY_LAST_ONLINE, SORTBY_NOTHING };

static INT_PTR CALLBACK DlgProcClistOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		CheckDlgButton(hwndDlg, IDC_GAMMACORRECT, db_get_b(0, "CLC", "GammaCorrect", CLCDEFAULT_GAMMACORRECT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_HILIGHTMODE4, db_get_b(0, "CLC", "HiLightMode", SETTING_HILIGHTMODE_DEFAULT) == 0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_HILIGHTMODE1, db_get_b(0, "CLC", "HiLightMode", SETTING_HILIGHTMODE_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_HILIGHTMODE2, db_get_b(0, "CLC", "HiLightMode", SETTING_HILIGHTMODE_DEFAULT) == 2 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_HILIGHTMODE3, db_get_b(0, "CLC", "HiLightMode", SETTING_HILIGHTMODE_DEFAULT) == 3 ? BST_CHECKED : BST_UNCHECKED);
		{
			for (auto &it : sortby) {
				int item = SendDlgItemMessage(hwndDlg, IDC_CLSORT1, CB_ADDSTRING, 0, (LPARAM)TranslateW(it));
				SendDlgItemMessage(hwndDlg, IDC_CLSORT1, CB_SETITEMDATA, item, 0);
				item = SendDlgItemMessage(hwndDlg, IDC_CLSORT2, CB_ADDSTRING, 0, (LPARAM)TranslateW(it));
				SendDlgItemMessage(hwndDlg, IDC_CLSORT2, CB_SETITEMDATA, item, 0);
				item = SendDlgItemMessage(hwndDlg, IDC_CLSORT3, CB_ADDSTRING, 0, (LPARAM)TranslateW(it));
				SendDlgItemMessage(hwndDlg, IDC_CLSORT3, CB_SETITEMDATA, item, 0);

			}
			int s1 = g_plugin.getByte("SortBy1", SETTING_SORTBY1_DEFAULT);
			int s2 = g_plugin.getByte("SortBy2", SETTING_SORTBY2_DEFAULT);
			int s3 = g_plugin.getByte("SortBy3", SETTING_SORTBY3_DEFAULT);

			for (int i = 0; i < _countof(sortby); i++) {
				if (s1 == sortbyValue[i])
					SendDlgItemMessage(hwndDlg, IDC_CLSORT1, CB_SETCURSEL, i, 0);
				if (s2 == sortbyValue[i])
					SendDlgItemMessage(hwndDlg, IDC_CLSORT2, CB_SETCURSEL, i, 0);
				if (s3 == sortbyValue[i])
					SendDlgItemMessage(hwndDlg, IDC_CLSORT3, CB_SETCURSEL, i, 0);
			}

			CheckDlgButton(hwndDlg, IDC_NOOFFLINEMOVE, g_plugin.getByte("NoOfflineBottom", SETTING_NOOFFLINEBOTTOM_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_OFFLINETOROOT, g_plugin.getByte("PlaceOfflineToRoot", SETTING_PLACEOFFLINETOROOT_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		}
		return TRUE;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDC_CLSORT1 || LOWORD(wParam) == IDC_CLSORT2 || LOWORD(wParam) == IDC_CLSORT3) && HIWORD(wParam) != CBN_SELCHANGE)
			break;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				db_set_b(0, "CLC", "GammaCorrect", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_GAMMACORRECT));
				int hil = 0;
				if (IsDlgButtonChecked(hwndDlg, IDC_HILIGHTMODE1))  hil = 1;
				if (IsDlgButtonChecked(hwndDlg, IDC_HILIGHTMODE2))  hil = 2;
				if (IsDlgButtonChecked(hwndDlg, IDC_HILIGHTMODE3))  hil = 3;
				db_set_b(0, "CLC", "HiLightMode", (uint8_t)hil);

				int s1 = SendDlgItemMessage(hwndDlg, IDC_CLSORT1, CB_GETCURSEL, 0, 0);
				int s2 = SendDlgItemMessage(hwndDlg, IDC_CLSORT2, CB_GETCURSEL, 0, 0);
				int s3 = SendDlgItemMessage(hwndDlg, IDC_CLSORT3, CB_GETCURSEL, 0, 0);
				if (s1 >= 0) s1 = sortbyValue[s1];
				if (s2 >= 0) s2 = sortbyValue[s2];
				if (s3 >= 0) s3 = sortbyValue[s3];
				g_plugin.setByte("SortBy1", (uint8_t)s1);
				g_plugin.setByte("SortBy2", (uint8_t)s2);
				g_plugin.setByte("SortBy3", (uint8_t)s3);

				g_plugin.setByte("NoOfflineBottom", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_NOOFFLINEMOVE));
				g_plugin.setByte("PlaceOfflineToRoot", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_OFFLINETOROOT));

				Clist_LoadContactTree(); /* this won't do job properly since it only really works when changes happen */
				Clist_InitAutoRebuild(g_clistApi.hwndContactTree); /* force reshuffle */
				ClcOptionsChanged(); // Used to force loading avatar an list height related options
				return TRUE;
			}
			break;
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK DlgProcTrayOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		CheckDlgButton(hwndDlg, IDC_NOOFFLINEMOVE, g_plugin.getByte("NoOfflineBottom", SETTING_NOOFFLINEBOTTOM_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_OFFLINETOROOT, g_plugin.getByte("PlaceOfflineToRoot", SETTING_PLACEOFFLINETOROOT_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		{
			uint8_t trayOption = db_get_b(0, "CLUI", "XStatusTray", SETTING_TRAYOPTION_DEFAULT);
			CheckDlgButton(hwndDlg, IDC_SHOWXSTATUS, (trayOption & 3) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWNORMAL, (trayOption & 2) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TRANSPARENTOVERLAY, (trayOption & 4) ? BST_CHECKED : BST_UNCHECKED);

			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWNORMAL), IsDlgButtonChecked(hwndDlg, IDC_SHOWXSTATUS));
			EnableWindow(GetDlgItem(hwndDlg, IDC_TRANSPARENTOVERLAY), IsDlgButtonChecked(hwndDlg, IDC_SHOWXSTATUS) && IsDlgButtonChecked(hwndDlg, IDC_SHOWNORMAL));
		}

		CheckDlgButton(hwndDlg, IDC_DISABLEBLINK, g_plugin.getByte("DisableTrayFlash", SETTING_DISABLETRAYFLASH_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_SETRANGE, 0, MAKELONG(0x3FFF, 250));
		SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_SETPOS, 0, MAKELONG(g_plugin.getWord("IconFlashTime", SETTING_ICONFLASHTIME_DEFAULT), 0));

		// == Tray icon mode ==
		// preparing account list.
		{
			int siS = -1, siV = -1;
			for (auto &pa : Accounts()) {
				if (!pa->bIsVirtual && pa->bIsVisible && !pa->bDynDisabled && pa->bIsEnabled) {
					int item = SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS_S, CB_ADDSTRING, 0, (LPARAM)pa->tszAccountName);
					SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS_S, CB_SETITEMDATA, item, (LPARAM)pa);

					if (!mir_strcmp(pa->szModuleName, g_plugin.getStringA("tiAccS")))
						siS = item;

					item = SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS_V, CB_ADDSTRING, 0, (LPARAM)pa->tszAccountName);
					SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS_V, CB_SETITEMDATA, item, (LPARAM)pa);

					if (!mir_strcmp(pa->szModuleName, g_plugin.getStringA("tiAccV")))
						siV = item;
				}
			}

			// the empty list item must not be selected
			if (siS < 0) siS = 0; if (siV < 0) siV = 0;

			SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS_S, CB_SETCURSEL, siS, 0);
			SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS_V, CB_SETCURSEL, siV, 0);
		}

		// setting icon cycling period
		SendDlgItemMessage(hwndDlg, IDC_CYCLETIMESPIN, UDM_SETRANGE, 0, MAKELONG(120, 1));
		SendDlgItemMessage(hwndDlg, IDC_CYCLETIMESPIN, UDM_SETPOS, 0, MAKELONG(g_plugin.getWord("CycleTime", SETTING_CYCLETIME_DEFAULT), 0));

		// setting icon mode
		switch (g_plugin.getByte("tiModeS", TRAY_ICON_MODE_GLOBAL)) {
		case TRAY_ICON_MODE_GLOBAL:
			CheckDlgButton(hwndDlg, IDC_ICON_GLOBAL_S, 1);
			break;
		case TRAY_ICON_MODE_ACC:
			CheckDlgButton(hwndDlg, IDC_ICON_ACC_S, 1);
			EnableWindow(GetDlgItem(hwndDlg, IDC_PRIMARYSTATUS_S), TRUE);
			break;
		case TRAY_ICON_MODE_CYCLE:
			CheckDlgButton(hwndDlg, IDC_ICON_CYCLE_S, 1);
			break;
		case TRAY_ICON_MODE_ALL:
			CheckDlgButton(hwndDlg, IDC_ICON_ALL_S, 1);
			break;
		}

		switch (g_plugin.getByte("tiModeV", TRAY_ICON_MODE_GLOBAL)) {
		case TRAY_ICON_MODE_GLOBAL:
			CheckDlgButton(hwndDlg, IDC_ICON_GLOBAL_V, 1);
			break;
		case TRAY_ICON_MODE_ACC:
			CheckDlgButton(hwndDlg, IDC_ICON_ACC_V, 1);
			EnableWindow(GetDlgItem(hwndDlg, IDC_PRIMARYSTATUS_V), TRUE);
			break;
		case TRAY_ICON_MODE_CYCLE:
			CheckDlgButton(hwndDlg, IDC_ICON_CYCLE_V, 1);
			break;
		case TRAY_ICON_MODE_ALL:
			CheckDlgButton(hwndDlg, IDC_ICON_ALL_V, 1);
			break;
		}

		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_SHOWXSTATUS || LOWORD(wParam) == IDC_SHOWNORMAL) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWNORMAL), IsDlgButtonChecked(hwndDlg, IDC_SHOWXSTATUS));
			EnableWindow(GetDlgItem(hwndDlg, IDC_TRANSPARENTOVERLAY), IsDlgButtonChecked(hwndDlg, IDC_SHOWXSTATUS) && IsDlgButtonChecked(hwndDlg, IDC_SHOWNORMAL));
		}

		if (LOWORD(wParam) == IDC_ICON_GLOBAL_S
			|| LOWORD(wParam) == IDC_ICON_ACC_S
			|| LOWORD(wParam) == IDC_ICON_CYCLE_S
			|| LOWORD(wParam) == IDC_ICON_ALL_S)
			EnableWindow(GetDlgItem(hwndDlg, IDC_PRIMARYSTATUS_S), IsDlgButtonChecked(hwndDlg, IDC_ICON_ACC_S));
		if (LOWORD(wParam) == IDC_ICON_GLOBAL_V
			|| LOWORD(wParam) == IDC_ICON_ACC_V
			|| LOWORD(wParam) == IDC_ICON_CYCLE_V
			|| LOWORD(wParam) == IDC_ICON_ALL_V)
			EnableWindow(GetDlgItem(hwndDlg, IDC_PRIMARYSTATUS_V), IsDlgButtonChecked(hwndDlg, IDC_ICON_ACC_V));

		if (LOWORD(wParam) == IDC_PRIMARYSTATUS_S && HIWORD(wParam) != CBN_SELCHANGE)
			return 0;
		if (LOWORD(wParam) == IDC_PRIMARYSTATUS_V && HIWORD(wParam) != CBN_SELCHANGE)
			return 0;
		if ((LOWORD(wParam) == IDC_BLINKTIME || LOWORD(wParam) == IDC_CYCLETIME) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				g_plugin.setWord("IconFlashTime", (uint16_t)SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_GETPOS, 0, 0));
				g_plugin.setByte("DisableTrayFlash", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_DISABLEBLINK));

				uint8_t xOptions = 0;
				xOptions = IsDlgButtonChecked(hwndDlg, IDC_SHOWXSTATUS) ? 1 : 0;
				xOptions |= (xOptions && IsDlgButtonChecked(hwndDlg, IDC_SHOWNORMAL)) ? 2 : 0;
				xOptions |= (xOptions && IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENTOVERLAY)) ? 4 : 0;
				db_set_b(0, "CLUI", "XStatusTray", xOptions);

				// == Tray icon mode ==
				// chosen account name.
				int idx = SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS_S, CB_GETCURSEL, 0, 0);
				if (idx != CB_ERR) {
					PROTOACCOUNT *pa = (PROTOACCOUNT*)SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS_S, CB_GETITEMDATA, idx, 0);
					g_plugin.setString("tiAccS", pa->szModuleName);
				}

				idx = SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS_V, CB_GETCURSEL, 0, 0);
				if (idx != CB_ERR) {
					PROTOACCOUNT *pa = (PROTOACCOUNT*)SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS_V, CB_GETITEMDATA, idx, 0);
					g_plugin.setString("tiAccV", pa->szModuleName);
				}

				// icon cycling timeout.
				g_plugin.setWord("CycleTime", (uint16_t)SendDlgItemMessage(hwndDlg, IDC_CYCLETIMESPIN, UDM_GETPOS, 0, 0));

				// icon modes
				g_plugin.setByte("tiModeS",
					IsDlgButtonChecked(hwndDlg, IDC_ICON_GLOBAL_S) << 0
					| IsDlgButtonChecked(hwndDlg, IDC_ICON_ACC_S) << 1
					| IsDlgButtonChecked(hwndDlg, IDC_ICON_CYCLE_S) << 2
					| IsDlgButtonChecked(hwndDlg, IDC_ICON_ALL_S) << 3);
				g_plugin.setByte("tiModeV",
					IsDlgButtonChecked(hwndDlg, IDC_ICON_GLOBAL_V) << 0
					| IsDlgButtonChecked(hwndDlg, IDC_ICON_ACC_V) << 1
					| IsDlgButtonChecked(hwndDlg, IDC_ICON_CYCLE_V) << 2
					| IsDlgButtonChecked(hwndDlg, IDC_ICON_ALL_V) << 3);

				Clist_TrayIconIconsChanged();
				Clist_LoadContactTree(); /* this won't do job properly since it only really works when changes happen */
				Clist_InitAutoRebuild(g_clistApi.hwndContactTree); /* force reshuffle */
				ClcOptionsChanged(); // Used to force loading avatar an list height related options
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

void ClcOptionsChanged(void)
{
	g_plugin.ReadSettings();

	Clist_Broadcast(INTM_RELOADOPTIONS, 0, 0);
	Clist_Broadcast(INTM_INVALIDATE, 0, 0);
}

HWND g_hCLUIOptionsWnd = nullptr;

static INT_PTR CALLBACK DlgProcClistBehaviourOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_CLIENTDRAG, db_get_b(0, "CLUI", "ClientAreaDrag", SETTING_CLIENTDRAG_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DRAGTOSCROLL, (db_get_b(0, "CLUI", "DragToScroll", SETTING_DRAGTOSCROLL_DEFAULT) && !db_get_b(0, "CLUI", "ClientAreaDrag", SETTING_CLIENTDRAG_DEFAULT)) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AUTOSIZE, g_CluiData.fAutoSize ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOCKSIZING, db_get_b(0, "CLUI", "LockSize", SETTING_LOCKSIZE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_BRINGTOFRONT, g_plugin.getByte("BringToFront", SETTING_BRINGTOFRONT_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg, IDC_MAXSIZESPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
		SendDlgItemMessage(hwndDlg, IDC_MAXSIZESPIN, UDM_SETPOS, 0, db_get_b(0, "CLUI", "MaxSizeHeight", SETTING_MAXSIZEHEIGHT_DEFAULT));
		SendDlgItemMessage(hwndDlg, IDC_MINSIZESPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
		SendDlgItemMessage(hwndDlg, IDC_MINSIZESPIN, UDM_SETPOS, 0, db_get_b(0, "CLUI", "MinSizeHeight", SETTING_MINSIZEHEIGHT_DEFAULT));
		CheckDlgButton(hwndDlg, IDC_AUTOSIZEUPWARD, db_get_b(0, "CLUI", "AutoSizeUpward", SETTING_AUTOSIZEUPWARD_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SNAPTOEDGES, db_get_b(0, "CLUI", "SnapToEdges", SETTING_SNAPTOEDGES_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DOCKTOSIDES, db_get_b(0, "CLUI", "DockToSides", SETTING_DOCKTOSIDES_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_EVENTAREA_NONE, db_get_b(0, "CLUI", "EventArea", SETTING_EVENTAREAMODE_DEFAULT) == 0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_EVENTAREA, db_get_b(0, "CLUI", "EventArea", SETTING_EVENTAREAMODE_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_EVENTAREA_ALWAYS, db_get_b(0, "CLUI", "EventArea", SETTING_EVENTAREAMODE_DEFAULT) == 2 ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_AUTOHIDE, g_plugin.getByte("AutoHide", SETTING_AUTOHIDE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_SETRANGE, 0, MAKELONG(900, 1));
		SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_SETPOS, 0, MAKELONG(g_plugin.getWord("HideTime", SETTING_HIDETIME_DEFAULT), 0));
		EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIME), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
		EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIMESPIN), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
		EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC01), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
		{
			wchar_t *hidemode[] = { TranslateT("Hide to tray"), TranslateT("Behind left edge"), TranslateT("Behind right edge") };
			for (auto &it : hidemode) {
				int item = SendDlgItemMessage(hwndDlg, IDC_HIDEMETHOD, CB_ADDSTRING, 0, (LPARAM)it);
				SendDlgItemMessage(hwndDlg, IDC_HIDEMETHOD, CB_SETITEMDATA, item, 0);
				SendDlgItemMessage(hwndDlg, IDC_HIDEMETHOD, CB_SETCURSEL, db_get_b(0, "ModernData", "HideBehind", SETTING_HIDEBEHIND_DEFAULT), 0);
			}
		}
		SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN2, UDM_SETRANGE, 0, MAKELONG(600, 0));
		SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN2, UDM_SETPOS, 0, MAKELONG(db_get_w(0, "ModernData", "ShowDelay", SETTING_SHOWDELAY_DEFAULT), 0));
		SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN3, UDM_SETRANGE, 0, MAKELONG(600, 0));
		SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN3, UDM_SETPOS, 0, MAKELONG(db_get_w(0, "ModernData", "HideDelay", SETTING_HIDEDELAY_DEFAULT), 0));
		SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN4, UDM_SETRANGE, 0, MAKELONG(50, 1));
		SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN4, UDM_SETPOS, 0, MAKELONG(db_get_w(0, "ModernData", "HideBehindBorderSize", SETTING_HIDEBEHINDBORDERSIZE_DEFAULT), 0));
		{
			int mode = SendDlgItemMessage(hwndDlg, IDC_HIDEMETHOD, CB_GETCURSEL, 0, 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWDELAY), mode != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HIDEDELAY), mode != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIMESPIN2), mode != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIMESPIN3), mode != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIMESPIN4), mode != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HIDEDELAY2), mode != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC5), mode != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC6), mode != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC7), mode != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC8), mode != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC10), mode != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC13), mode != 0);
		}

		if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC21), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC22), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_MAXSIZEHEIGHT), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_MAXSIZESPIN), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_MINSIZEHEIGHT), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_MINSIZESPIN), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AUTOSIZEUPWARD), FALSE);
		}
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_AUTOHIDE) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIME), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIMESPIN), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC01), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
		}
		else if (LOWORD(wParam) == IDC_DRAGTOSCROLL && IsDlgButtonChecked(hwndDlg, IDC_CLIENTDRAG)) {
			CheckDlgButton(hwndDlg, IDC_CLIENTDRAG, BST_UNCHECKED);
		}
		else if (LOWORD(wParam) == IDC_CLIENTDRAG && IsDlgButtonChecked(hwndDlg, IDC_DRAGTOSCROLL)) {
			CheckDlgButton(hwndDlg, IDC_DRAGTOSCROLL, BST_UNCHECKED);
		}
		else if (LOWORD(wParam) == IDC_AUTOSIZE) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC21), IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC22), IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_MAXSIZEHEIGHT), IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_MAXSIZESPIN), IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_MINSIZEHEIGHT), IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_MINSIZESPIN), IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_AUTOSIZEUPWARD), IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
		}
		else if (LOWORD(wParam) == IDC_HIDEMETHOD) {
			int mode = SendDlgItemMessage(hwndDlg, IDC_HIDEMETHOD, CB_GETCURSEL, 0, 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWDELAY), mode != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HIDEDELAY), mode != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIMESPIN2), mode != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIMESPIN3), mode != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIMESPIN4), mode != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_HIDEDELAY2), mode != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC5), mode != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC6), mode != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC7), mode != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC8), mode != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC10), mode != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC13), mode != 0);
			if (HIWORD(wParam) != CBN_SELCHANGE || (HWND)lParam != GetFocus())
				return 0;
		}
		if ((LOWORD(wParam) == IDC_HIDETIME || LOWORD(wParam) == IDC_HIDEDELAY2 || LOWORD(wParam) == IDC_HIDEDELAY
			|| LOWORD(wParam) == IDC_SHOWDELAY || LOWORD(wParam) == IDC_MAXSIZEHEIGHT || LOWORD(wParam) == IDC_MINSIZEHEIGHT)
			&& (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;

		// Enable apply button
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			db_set_b(0, "ModernData", "HideBehind", (uint8_t)SendDlgItemMessage(hwndDlg, IDC_HIDEMETHOD, CB_GETCURSEL, 0, 0));
			db_set_w(0, "ModernData", "ShowDelay", (uint16_t)SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN2, UDM_GETPOS, 0, 0));
			db_set_w(0, "ModernData", "HideDelay", (uint16_t)SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN3, UDM_GETPOS, 0, 0));
			db_set_w(0, "ModernData", "HideBehindBorderSize", (uint16_t)SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN4, UDM_GETPOS, 0, 0));

			db_set_b(0, "CLUI", "DragToScroll", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_DRAGTOSCROLL));
			g_plugin.setByte("BringToFront", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_BRINGTOFRONT));
			g_bChangingMode = true;
			db_set_b(0, "CLUI", "ClientAreaDrag", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_CLIENTDRAG));
			db_set_b(0, "CLUI", "AutoSize", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
			db_set_b(0, "CLUI", "LockSize", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_LOCKSIZING));
			db_set_b(0, "CLUI", "MaxSizeHeight", (uint8_t)GetDlgItemInt(hwndDlg, IDC_MAXSIZEHEIGHT, nullptr, FALSE));
			db_set_b(0, "CLUI", "MinSizeHeight", (uint8_t)GetDlgItemInt(hwndDlg, IDC_MINSIZEHEIGHT, nullptr, FALSE));
			db_set_b(0, "CLUI", "AutoSizeUpward", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZEUPWARD));
			db_set_b(0, "CLUI", "SnapToEdges", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_SNAPTOEDGES));

			db_set_b(0, "CLUI", "DockToSides", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_DOCKTOSIDES));

			db_set_b(0, "CLUI", "EventArea",
				(uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_EVENTAREA_ALWAYS) ? 2 : (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_EVENTAREA) ? 1 : 0));

			g_plugin.setByte("AutoHide", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			g_plugin.setWord("HideTime", (uint16_t)SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_GETPOS, 0, 0));
			CLUI_ChangeWindowMode();
			SendMessage(g_clistApi.hwndContactTree, WM_SIZE, 0, 0);	//forces it to send a cln_listsizechanged
			CLUI_ReloadCLUIOptions();
			EventArea_ConfigureEventArea();
			cliShowHide(true);
			g_bChangingMode = false;
			return TRUE;
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK DlgProcClistWindowOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL fEnabled = FALSE;
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		g_hCLUIOptionsWnd = hwndDlg;
		CheckDlgButton(hwndDlg, IDC_ONTOP, g_plugin.getByte("OnTop", SETTING_ONTOP_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		{
			//====== Activate/Deactivate Non-Layered items =======
			fEnabled = !g_CluiData.fLayered || g_CluiData.fDisableSkinEngine;
			EnableWindow(GetDlgItem(hwndDlg, IDC_TOOLWND), fEnabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_MIN2TRAY), fEnabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BORDER), fEnabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_NOBORDERWND), fEnabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWCAPTION), fEnabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWMAINMENU), fEnabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TITLETEXT), fEnabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DROPSHADOW), fEnabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AEROGLASS), !fEnabled && (g_proc_DWMEnableBlurBehindWindow != nullptr));
			EnableWindow(GetDlgItem(hwndDlg, IDC_TITLEBAR_STATIC), fEnabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ROUNDCORNERS), fEnabled);
		}
		{
			//====== Non-Layered Mode =====
			CheckDlgButton(hwndDlg, IDC_TOOLWND, g_plugin.getByte("ToolWindow", SETTING_TOOLWINDOW_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_MIN2TRAY, g_plugin.getByte("Min2Tray", SETTING_MIN2TRAY_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_BORDER, g_plugin.getByte("ThinBorder", SETTING_THINBORDER_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOBORDERWND, g_plugin.getByte("NoBorder", SETTING_NOBORDER_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			if (IsDlgButtonChecked(hwndDlg, IDC_TOOLWND))
				EnableWindow(GetDlgItem(hwndDlg, IDC_MIN2TRAY), FALSE);
			CheckDlgButton(hwndDlg, IDC_SHOWCAPTION, db_get_b(0, "CLUI", "ShowCaption", SETTING_SHOWCAPTION_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWMAINMENU, db_get_b(0, "CLUI", "ShowMainMenu", SETTING_SHOWMAINMENU_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SHOWCAPTION)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_MIN2TRAY), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TOOLWND), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TITLETEXT), FALSE);
			}
			if (IsDlgButtonChecked(hwndDlg, IDC_BORDER) || IsDlgButtonChecked(hwndDlg, IDC_NOBORDERWND)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_MIN2TRAY), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TOOLWND), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TITLETEXT), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWCAPTION), FALSE);
			}
			CheckDlgButton(hwndDlg, IDC_DROPSHADOW, g_plugin.getByte("WindowShadow", SETTING_WINDOWSHADOW_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ROUNDCORNERS, db_get_b(0, "CLC", "RoundCorners", SETTING_ROUNDCORNERS_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		}   //====== End of Non-Layered Mode =====

		CheckDlgButton(hwndDlg, IDC_FADEINOUT, db_get_b(0, "CLUI", "FadeInOut", SETTING_FADEIN_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ONDESKTOP, g_plugin.getByte("OnDesktop", SETTING_ONDESKTOP_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_FRAMESSPIN, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_CAPTIONSSPIN, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_FRAMESSPIN, UDM_SETPOS, 0, db_get_dw(0, "CLUIFrames", "GapBetweenFrames", SETTING_GAPFRAMES_DEFAULT));
		SendDlgItemMessage(hwndDlg, IDC_CAPTIONSSPIN, UDM_SETPOS, 0, db_get_dw(0, "CLUIFrames", "GapBetweenTitleBar", SETTING_GAPTITLEBAR_DEFAULT));
		SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_SETRANGE, 0, MAKELONG(250, 0));
		SendDlgItemMessage(hwndDlg, IDC_RIGHTMARGINSPIN, UDM_SETRANGE, 0, MAKELONG(250, 0));
		SendDlgItemMessage(hwndDlg, IDC_TOPMARGINSPIN, UDM_SETRANGE, 0, MAKELONG(250, 0));
		SendDlgItemMessage(hwndDlg, IDC_BOTTOMMARGINSPIN, UDM_SETRANGE, 0, MAKELONG(250, 0));
		SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_SETPOS, 0, db_get_b(0, "CLUI", "LeftClientMargin", SETTING_LEFTCLIENTMARIGN_DEFAULT));
		SendDlgItemMessage(hwndDlg, IDC_RIGHTMARGINSPIN, UDM_SETPOS, 0, db_get_b(0, "CLUI", "RightClientMargin", SETTING_RIGHTCLIENTMARIGN_DEFAULT));
		SendDlgItemMessage(hwndDlg, IDC_TOPMARGINSPIN, UDM_SETPOS, 0, db_get_b(0, "CLUI", "TopClientMargin", SETTING_TOPCLIENTMARIGN_DEFAULT));
		SendDlgItemMessage(hwndDlg, IDC_BOTTOMMARGINSPIN, UDM_SETPOS, 0, db_get_b(0, "CLUI", "BottomClientMargin", SETTING_BOTTOMCLIENTMARIGN_DEFAULT));

		CheckDlgButton(hwndDlg, IDC_DISABLEENGINE, db_get_b(0, "ModernData", "DisableEngine", SETTING_DISABLESKIN_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AEROGLASS, db_get_b(0, "ModernData", "AeroGlass", SETTING_AEROGLASS_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);

		EnableWindow(GetDlgItem(hwndDlg, IDC_LAYERENGINE), !db_get_b(0, "ModernData", "DisableEngine", SETTING_DISABLESKIN_DEFAULT) ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_LAYERENGINE, (db_get_b(0, "ModernData", "EnableLayering", SETTING_ENABLELAYERING_DEFAULT) && !db_get_b(0, "ModernData", "DisableEngine", SETTING_DISABLESKIN_DEFAULT)) ? BST_UNCHECKED : BST_CHECKED);

		{
			DBVARIANT dbv = { 0 };
			wchar_t *s;
			if (!g_plugin.getWString("TitleText", &dbv))
				s = dbv.pwszVal;
			else
				s = _A2W(MIRANDANAME);
			SetDlgItemText(hwndDlg, IDC_TITLETEXT, s);
			db_free(&dbv);

			SendDlgItemMessage(hwndDlg, IDC_TITLETEXT, CB_ADDSTRING, 0, (LPARAM)MIRANDANAME);

			char szUin[20];
			mir_snprintf(szUin, "%u", db_get_dw(0, "ICQ", "UIN", 0));
			SendDlgItemMessage(hwndDlg, IDC_TITLETEXT, CB_ADDSTRING, 0, (LPARAM)szUin);

			if (!db_get_s(0, "ICQ", "Nick", &dbv)) {
				SendDlgItemMessage(hwndDlg, IDC_TITLETEXT, CB_ADDSTRING, 0, (LPARAM)dbv.pszVal);
				db_free(&dbv);
				dbv.pszVal = nullptr;
			}
			if (!db_get_s(0, "ICQ", "FirstName", &dbv)) {
				SendDlgItemMessage(hwndDlg, IDC_TITLETEXT, CB_ADDSTRING, 0, (LPARAM)dbv.pszVal);
				db_free(&dbv);
				dbv.pszVal = nullptr;
			}
			if (!db_get_s(0, "ICQ", "e-mail", &dbv)) {
				SendDlgItemMessage(hwndDlg, IDC_TITLETEXT, CB_ADDSTRING, 0, (LPARAM)dbv.pszVal);
				db_free(&dbv);
				dbv.pszVal = nullptr;
			}
		}
		CheckDlgButton(hwndDlg, IDC_TRANSPARENT, g_plugin.getByte("Transparent", SETTING_TRANSPARENT_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC11), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC12), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TRANSACTIVE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TRANSINACTIVE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ACTIVEPERC), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_INACTIVEPERC), FALSE);
		}
		SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_SETRANGE, FALSE, MAKELONG(1, 255));
		SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_SETRANGE, FALSE, MAKELONG(1, 255));
		SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_SETPOS, TRUE, g_plugin.getByte("Alpha", SETTING_ALPHA_DEFAULT));
		SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_SETPOS, TRUE, g_plugin.getByte("AutoAlpha", SETTING_AUTOALPHA_DEFAULT));
		SendMessage(hwndDlg, WM_HSCROLL, 0x12345678, 0);
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_TRANSPARENT) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC11), IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC12), IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_TRANSACTIVE), IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_TRANSINACTIVE), IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_ACTIVEPERC), IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_INACTIVEPERC), IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
		}
		else if (LOWORD(wParam) == IDC_LAYERENGINE || LOWORD(wParam) == IDC_DISABLEENGINE) {	//====== Activate/Deactivate Non-Layered items =======
			fEnabled = !(IsWindowEnabled(GetDlgItem(hwndDlg, IDC_LAYERENGINE)) && BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_LAYERENGINE) && BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DISABLEENGINE));

			EnableWindow(GetDlgItem(hwndDlg, IDC_TOOLWND), fEnabled && (IsDlgButtonChecked(hwndDlg, IDC_SHOWCAPTION)) && !(IsDlgButtonChecked(hwndDlg, IDC_NOBORDERWND) || IsDlgButtonChecked(hwndDlg, IDC_BORDER)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_MIN2TRAY), fEnabled && (IsDlgButtonChecked(hwndDlg, IDC_TOOLWND) && IsDlgButtonChecked(hwndDlg, IDC_SHOWCAPTION)) && !(IsDlgButtonChecked(hwndDlg, IDC_NOBORDERWND) || IsDlgButtonChecked(hwndDlg, IDC_BORDER)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_TITLETEXT), fEnabled && (IsDlgButtonChecked(hwndDlg, IDC_SHOWCAPTION)) && !(IsDlgButtonChecked(hwndDlg, IDC_NOBORDERWND) || IsDlgButtonChecked(hwndDlg, IDC_BORDER)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWCAPTION), fEnabled && !(IsDlgButtonChecked(hwndDlg, IDC_NOBORDERWND) || IsDlgButtonChecked(hwndDlg, IDC_BORDER)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_BORDER), fEnabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_NOBORDERWND), fEnabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWMAINMENU), fEnabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DROPSHADOW), fEnabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AEROGLASS), !fEnabled && (g_proc_DWMEnableBlurBehindWindow != nullptr));
			EnableWindow(GetDlgItem(hwndDlg, IDC_TITLEBAR_STATIC), fEnabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ROUNDCORNERS), fEnabled);
			if (LOWORD(wParam) == IDC_DISABLEENGINE) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_LAYERENGINE), BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DISABLEENGINE));
				if (IsDlgButtonChecked(hwndDlg, IDC_DISABLEENGINE))
					CheckDlgButton(hwndDlg, IDC_LAYERENGINE, BST_CHECKED);
			}
		}
		else if (LOWORD(wParam) == IDC_ONDESKTOP && IsDlgButtonChecked(hwndDlg, IDC_ONDESKTOP)) {
			CheckDlgButton(hwndDlg, IDC_ONTOP, BST_UNCHECKED);
		}
		else if (LOWORD(wParam) == IDC_ONTOP && IsDlgButtonChecked(hwndDlg, IDC_ONTOP)) {
			CheckDlgButton(hwndDlg, IDC_ONDESKTOP, BST_UNCHECKED);
		}
		else if (LOWORD(wParam) == IDC_TOOLWND) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_MIN2TRAY), BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_TOOLWND));
		}
		else if (LOWORD(wParam) == IDC_SHOWCAPTION) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_TOOLWND), IsDlgButtonChecked(hwndDlg, IDC_SHOWCAPTION));
			EnableWindow(GetDlgItem(hwndDlg, IDC_MIN2TRAY), BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_TOOLWND) && IsDlgButtonChecked(hwndDlg, IDC_SHOWCAPTION));
			EnableWindow(GetDlgItem(hwndDlg, IDC_TITLETEXT), IsDlgButtonChecked(hwndDlg, IDC_SHOWCAPTION));
		}
		else if (LOWORD(wParam) == IDC_NOBORDERWND || LOWORD(wParam) == IDC_BORDER) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_TOOLWND), (IsDlgButtonChecked(hwndDlg, IDC_SHOWCAPTION)) && !(IsDlgButtonChecked(hwndDlg, IDC_NOBORDERWND) || IsDlgButtonChecked(hwndDlg, IDC_BORDER)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_MIN2TRAY), (IsDlgButtonChecked(hwndDlg, IDC_TOOLWND) && IsDlgButtonChecked(hwndDlg, IDC_SHOWCAPTION)) && !(IsDlgButtonChecked(hwndDlg, IDC_NOBORDERWND) || IsDlgButtonChecked(hwndDlg, IDC_BORDER)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_TITLETEXT), (IsDlgButtonChecked(hwndDlg, IDC_SHOWCAPTION)) && !(IsDlgButtonChecked(hwndDlg, IDC_NOBORDERWND) || IsDlgButtonChecked(hwndDlg, IDC_BORDER)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWCAPTION), !(IsDlgButtonChecked(hwndDlg, IDC_NOBORDERWND) || IsDlgButtonChecked(hwndDlg, IDC_BORDER)));
			if (LOWORD(wParam) == IDC_BORDER) CheckDlgButton(hwndDlg, IDC_NOBORDERWND, BST_UNCHECKED);
			else CheckDlgButton(hwndDlg, IDC_BORDER, BST_UNCHECKED);

		}
		if ((LOWORD(wParam) == IDC_TITLETEXT || LOWORD(wParam) == IDC_MAXSIZEHEIGHT || LOWORD(wParam) == IDC_MINSIZEHEIGHT || LOWORD(wParam) == IDC_FRAMESGAP || LOWORD(wParam) == IDC_CAPTIONSGAP ||
			LOWORD(wParam) == IDC_LEFTMARGIN || LOWORD(wParam) == IDC_RIGHTMARGIN || LOWORD(wParam) == IDC_TOPMARGIN || LOWORD(wParam) == IDC_BOTTOMMARGIN)
			&& (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;
		// Enable apply button
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		break;

	case WM_HSCROLL:
		{
			char str[10];
			mir_snprintf(str, "%d%%", 100 * SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_GETPOS, 0, 0) / 255);
			SetDlgItemTextA(hwndDlg, IDC_INACTIVEPERC, str);
			mir_snprintf(str, "%d%%", 100 * SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_GETPOS, 0, 0) / 255);
			SetDlgItemTextA(hwndDlg, IDC_ACTIVEPERC, str);
		}
		if (wParam != 0x12345678)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			db_set_b(0, "CLUI", "LeftClientMargin", (uint8_t)SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_GETPOS, 0, 0));
			db_set_b(0, "CLUI", "RightClientMargin", (uint8_t)SendDlgItemMessage(hwndDlg, IDC_RIGHTMARGINSPIN, UDM_GETPOS, 0, 0));
			db_set_b(0, "CLUI", "TopClientMargin", (uint8_t)SendDlgItemMessage(hwndDlg, IDC_TOPMARGINSPIN, UDM_GETPOS, 0, 0));
			db_set_b(0, "CLUI", "BottomClientMargin", (uint8_t)SendDlgItemMessage(hwndDlg, IDC_BOTTOMMARGINSPIN, UDM_GETPOS, 0, 0));
			db_set_b(0, "ModernData", "DisableEngine", IsDlgButtonChecked(hwndDlg, IDC_DISABLEENGINE));
			db_set_b(0, "ModernData", "AeroGlass", IsDlgButtonChecked(hwndDlg, IDC_AEROGLASS));
			if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DISABLEENGINE)) {
				if (IsDlgButtonChecked(hwndDlg, IDC_LAYERENGINE))
					db_set_b(0, "ModernData", "EnableLayering", 0);
				else
					db_unset(0, "ModernData", "EnableLayering");
			}
			g_CluiData.dwKeyColor = db_get_dw(0, "ModernSettings", "KeyColor", (uint32_t)SETTING_KEYCOLOR_DEFAULT);
			g_plugin.setByte("OnDesktop", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_ONDESKTOP));
			g_plugin.setByte("OnTop", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_ONTOP));
			SetWindowPos(g_clistApi.hwndContactList, IsDlgButtonChecked(hwndDlg, IDC_ONTOP) ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			db_set_b(0, "CLUI", "DragToScroll", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_DRAGTOSCROLL));

			//======  Non-Layered Mode ======
			g_plugin.setByte("ToolWindow", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_TOOLWND));
			db_set_b(0, "CLUI", "ShowCaption", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_SHOWCAPTION));
			db_set_b(0, "CLUI", "ShowMainMenu", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_SHOWMAINMENU));
			g_plugin.setByte("ThinBorder", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_BORDER));
			g_plugin.setByte("NoBorder", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_NOBORDERWND));
			{
				wchar_t title[256];
				GetDlgItemText(hwndDlg, IDC_TITLETEXT, title, _countof(title));
				g_plugin.setWString("TitleText", title);
			}
			g_plugin.setByte("Min2Tray", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_MIN2TRAY));
			g_plugin.setByte("WindowShadow", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_DROPSHADOW));
			db_set_b(0, "CLC", "RoundCorners", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_ROUNDCORNERS));
			//======  End of Non-Layered Mode ======

			g_bChangingMode = true;

			if (IsDlgButtonChecked(hwndDlg, IDC_ONDESKTOP)) {
				HWND hProgMan = FindWindow(L"Progman", nullptr);
				if (IsWindow(hProgMan)) {
					SetParent(g_clistApi.hwndContactList, hProgMan);
					Sync(CLUIFrames_SetParentForContainers, (HWND)hProgMan);
					g_CluiData.fOnDesktop = true;
				}
			}
			else {
				if (GetParent(g_clistApi.hwndContactList)) {
					SetParent(g_clistApi.hwndContactList, nullptr);
					Sync(CLUIFrames_SetParentForContainers, (HWND)nullptr);
				}
				g_CluiData.fOnDesktop = false;
			}
			AniAva_UpdateParent();
			db_set_b(0, "CLUI", "FadeInOut", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_FADEINOUT));
			g_CluiData.fSmoothAnimation = IsDlgButtonChecked(hwndDlg, IDC_FADEINOUT) != 0;
			{
				int i1 = SendDlgItemMessage(hwndDlg, IDC_FRAMESSPIN, UDM_GETPOS, 0, 0);
				int i2 = SendDlgItemMessage(hwndDlg, IDC_CAPTIONSSPIN, UDM_GETPOS, 0, 0);

				db_set_dw(0, "CLUIFrames", "GapBetweenFrames", (uint32_t)i1);
				db_set_dw(0, "CLUIFrames", "GapBetweenTitleBar", (uint32_t)i2);
				Sync(CLUIFramesOnClistResize, (WPARAM)g_clistApi.hwndContactList, 0);
			}
			g_plugin.setByte("Transparent", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			g_plugin.setByte("Alpha", (uint8_t)SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_GETPOS, 0, 0));
			g_plugin.setByte("AutoAlpha", (uint8_t)SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_GETPOS, 0, 0));
			g_plugin.setByte("OnDesktop", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_ONDESKTOP));

			ske_LoadSkinFromDB();
			CLUI_UpdateLayeredMode();
			CLUI_ChangeWindowMode();
			SendMessage(g_clistApi.hwndContactTree, WM_SIZE, 0, 0);	//forces it to send a cln_listsizechanged
			CLUI_ReloadCLUIOptions();
			cliShowHide(true);
			g_bChangingMode = false;
			return TRUE;
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

#define CLBF_TILEVTOROWHEIGHT        0x0100

#define DEFAULT_BKCOLOUR      GetSysColor(COLOR_3DFACE)
#define DEFAULT_USEBITMAP     0
#define DEFAULT_BKBMPUSE      CLB_STRETCH
#define DEFAULT_SELBKCOLOUR   GetSysColor(COLOR_HIGHLIGHT)

char **bkgrList = nullptr;
int bkgrCount = 0;

#define M_BKGR_UPDATE	(WM_USER+10)
#define M_BKGR_SETSTATE	(WM_USER+11)
#define M_BKGR_GETSTATE	(WM_USER+12)

#define M_BKGR_BACKCOLOR	0x01
#define M_BKGR_SELECTCOLOR	0x02
#define M_BKGR_ALLOWBITMAPS	0x04
#define M_BKGR_STRETCH		0x08
#define M_BKGR_TILE			0x10

static int bitmapRelatedControls[] = {
	IDC_FILENAME, IDC_BROWSE, IDC_STRETCHH, IDC_STRETCHV, IDC_TILEH, IDC_TILEV,
	IDC_SCROLL, IDC_PROPORTIONAL, IDC_TILEVROWH
};

struct BkgrItem
{
	uint8_t changed;
	uint8_t useBitmap;
	COLORREF bkColor, selColor;
	char filename[MAX_PATH];
	uint16_t flags;
	uint8_t useWinColours;
};
struct BkgrData
{
	struct BkgrItem *item;
	int indx;
	int count;
};

static INT_PTR CALLBACK DlgProcClcBkgOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct BkgrData *dat = (struct BkgrData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			int indx;
			HWND hList = GetDlgItem(hwndDlg, IDC_BKGRLIST);

			dat = (struct BkgrData*)mir_alloc(sizeof(struct BkgrData));
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			dat->count = bkgrCount;
			dat->item = (struct BkgrItem*)mir_alloc(sizeof(struct BkgrItem)*dat->count);
			dat->indx = CB_ERR;
			for (indx = 0; indx < dat->count; indx++) {
				char *module = bkgrList[indx] + mir_strlen(bkgrList[indx]) + 1;
				int jndx;

				dat->item[indx].changed = FALSE;
				dat->item[indx].useBitmap = db_get_b(0, module, "UseBitmap", DEFAULT_USEBITMAP);
				dat->item[indx].bkColor = db_get_dw(0, module, "BkColour", DEFAULT_BKCOLOUR);
				dat->item[indx].selColor = db_get_dw(0, module, "SelBkColour", DEFAULT_SELBKCOLOUR);
				dat->item[indx].useWinColours = db_get_b(0, module, "UseWinColours", CLCDEFAULT_USEWINDOWSCOLOURS);

				DBVARIANT dbv;
				if (!db_get_s(0, module, "BkBitmap", &dbv)) {
					int retval = PathToAbsolute(dbv.pszVal, dat->item[indx].filename);
					if (!retval || retval == CALLSERVICE_NOTFOUND)
						strncpy_s(dat->item[indx].filename, dbv.pszVal, _TRUNCATE);
					mir_free(dbv.pszVal);
				}
				else *dat->item[indx].filename = 0;

				dat->item[indx].flags = db_get_w(0, module, "BkBmpUse", DEFAULT_BKBMPUSE);
				jndx = SendMessageA(hList, CB_ADDSTRING, 0, (LPARAM)Translate(bkgrList[indx]));
				SendMessage(hList, CB_SETITEMDATA, jndx, indx);
			}
			SendMessage(hList, CB_SETCURSEL, 0, 0);
			PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_BKGRLIST, CBN_SELCHANGE), 0);
			SHAutoComplete(GetDlgItem(hwndDlg, IDC_FILENAME), 1);
		}
		return TRUE;

	case WM_DESTROY:
		if (dat) {
			mir_free(dat->item);
			mir_free(dat);
		}
		return TRUE;

	case M_BKGR_GETSTATE:
		{
			int indx = wParam;
			if (indx == CB_ERR || indx >= dat->count)
				break;

			indx = SendDlgItemMessage(hwndDlg, IDC_BKGRLIST, CB_GETITEMDATA, indx, 0);

			dat->item[indx].useBitmap = IsDlgButtonChecked(hwndDlg, IDC_BITMAP);
			dat->item[indx].useWinColours = IsDlgButtonChecked(hwndDlg, IDC_USEWINCOL);
			dat->item[indx].bkColor = SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_GETCOLOUR, 0, 0);
			dat->item[indx].selColor = SendDlgItemMessage(hwndDlg, IDC_SELCOLOUR, CPM_GETCOLOUR, 0, 0);

			GetDlgItemTextA(hwndDlg, IDC_FILENAME, dat->item[indx].filename, _countof(dat->item[indx].filename));

			uint16_t flags = 0;
			if (IsDlgButtonChecked(hwndDlg, IDC_STRETCHH)) flags |= CLB_STRETCHH;
			if (IsDlgButtonChecked(hwndDlg, IDC_STRETCHV)) flags |= CLB_STRETCHV;
			if (IsDlgButtonChecked(hwndDlg, IDC_TILEH)) flags |= CLBF_TILEH;
			if (IsDlgButtonChecked(hwndDlg, IDC_TILEV)) flags |= CLBF_TILEV;
			if (IsDlgButtonChecked(hwndDlg, IDC_SCROLL)) flags |= CLBF_SCROLL;
			if (IsDlgButtonChecked(hwndDlg, IDC_PROPORTIONAL)) flags |= CLBF_PROPORTIONAL;
			if (IsDlgButtonChecked(hwndDlg, IDC_TILEVROWH)) flags |= CLBF_TILEVTOROWHEIGHT;
			dat->item[indx].flags = flags;
		}
		break;

	case M_BKGR_SETSTATE:
		{
			int indx = wParam;
			if (indx == CB_ERR || indx >= dat->count)
				break;

			int flags = dat->item[indx].flags;
			indx = SendDlgItemMessage(hwndDlg, IDC_BKGRLIST, CB_GETITEMDATA, indx, 0);

			CheckDlgButton(hwndDlg, IDC_BITMAP, dat->item[indx].useBitmap ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_USEWINCOL, dat->item[indx].useWinColours ? BST_CHECKED : BST_UNCHECKED);

			EnableWindow(GetDlgItem(hwndDlg, IDC_BKGCOLOUR), !dat->item[indx].useWinColours);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SELCOLOUR), !dat->item[indx].useWinColours);

			SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETDEFAULTCOLOUR, 0, DEFAULT_BKCOLOUR);
			SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETCOLOUR, 0, dat->item[indx].bkColor);
			SendDlgItemMessage(hwndDlg, IDC_SELCOLOUR, CPM_SETDEFAULTCOLOUR, 0, DEFAULT_SELBKCOLOUR);
			SendDlgItemMessage(hwndDlg, IDC_SELCOLOUR, CPM_SETCOLOUR, 0, dat->item[indx].selColor);
			SetDlgItemTextA(hwndDlg, IDC_FILENAME, dat->item[indx].filename);

			CheckDlgButton(hwndDlg, IDC_STRETCHH, flags&CLB_STRETCHH ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_STRETCHV, flags&CLB_STRETCHV ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TILEH, flags&CLBF_TILEH ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TILEV, flags&CLBF_TILEV ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SCROLL, flags&CLBF_SCROLL ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_PROPORTIONAL, flags&CLBF_PROPORTIONAL ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TILEVROWH, flags&CLBF_TILEVTOROWHEIGHT ? BST_CHECKED : BST_UNCHECKED);

			SendMessage(hwndDlg, M_BKGR_UPDATE, 0, 0);
		}
		break;

	case M_BKGR_UPDATE:
		{
			int isChecked = IsDlgButtonChecked(hwndDlg, IDC_BITMAP);
			for (auto &it : bitmapRelatedControls)
				EnableWindow(GetDlgItem(hwndDlg, it), isChecked);
		}
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_BROWSE) {
			wchar_t str[MAX_PATH], filter[512];
			Bitmap_GetFilter(filter, _countof(filter));
			GetDlgItemText(hwndDlg, IDC_FILENAME, str, _countof(str));

			OPENFILENAME ofn = { 0 };
			ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
			ofn.hwndOwner = hwndDlg;
			ofn.lpstrFilter = filter;
			ofn.lpstrFile = str;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			ofn.nMaxFile = _countof(str);
			ofn.nMaxFileTitle = MAX_PATH;
			ofn.lpstrDefExt = L"bmp";
			if (!GetOpenFileName(&ofn))
				break;

			SetDlgItemText(hwndDlg, IDC_FILENAME, str);
		}
		else if (LOWORD(wParam) == IDC_FILENAME && HIWORD(wParam) != EN_CHANGE)
			break;

		if (LOWORD(wParam) == IDC_BITMAP)
			SendMessage(hwndDlg, M_BKGR_UPDATE, 0, 0);
		if (LOWORD(wParam) == IDC_FILENAME && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;

		if (LOWORD(wParam) == IDC_BKGRLIST) {
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				SendMessage(hwndDlg, M_BKGR_GETSTATE, dat->indx, 0);
				SendMessage(hwndDlg, M_BKGR_SETSTATE, dat->indx = SendDlgItemMessage(hwndDlg, IDC_BKGRLIST, CB_GETCURSEL, 0, 0), 0);
			}
			return 0;
		}
		else {
			int indx = SendDlgItemMessage(hwndDlg, IDC_BKGRLIST, CB_GETCURSEL, 0, 0);
			if (indx != CB_ERR && indx < dat->count) {
				indx = SendDlgItemMessage(hwndDlg, IDC_BKGRLIST, CB_GETITEMDATA, indx, 0);
				dat->item[indx].changed = TRUE;
			}

			BOOL EnableColours = BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_USEWINCOL);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BKGCOLOUR), EnableColours);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SELCOLOUR), EnableColours);

			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				SendMessage(hwndDlg, M_BKGR_GETSTATE, SendDlgItemMessage(hwndDlg, IDC_BKGRLIST, CB_GETCURSEL, 0, 0), 0);

				for (int indx = 0; indx < dat->count; indx++) {
					BkgrItem &p = dat->item[indx];
					if (!p.changed)
						continue;

					char *module = bkgrList[indx] + mir_strlen(bkgrList[indx]) + 1;
					db_set_b(0, module, "UseBitmap", (uint8_t)p.useBitmap);

					COLORREF col;
					if ((col = p.bkColor) == DEFAULT_BKCOLOUR)
						db_unset(0, module, "BkColour");
					else
						db_set_dw(0, module, "BkColour", col);

					if ((col = p.selColor) == DEFAULT_SELBKCOLOUR)
						db_unset(0, module, "SelBkColour");
					else
						db_set_dw(0, module, "SelBkColour", col);

					db_set_b(0, module, "UseWinColours", (uint8_t)p.useWinColours);

					char str[MAX_PATH];
					int retval = PathToAbsolute(p.filename, str);
					if (!retval || retval == CALLSERVICE_NOTFOUND)
						db_set_s(0, module, "BkBitmap", p.filename);
					else
						db_set_s(0, module, "BkBitmap", str);

					db_set_w(0, module, "BkBmpUse", p.flags);
					p.changed = FALSE;
					NotifyEventHooks(g_CluiData.hEventBkgrChanged, (WPARAM)module, 0);
				}
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

static INT_PTR BkgrCfg_Register(WPARAM wParam, LPARAM lParam)
{
	char *szSetting = (char*)wParam;
	size_t len = mir_strlen(szSetting) + 1;

	char *value = (char *)mir_alloc(len + 4); // add room for flags (uint32_t)
	memcpy(value, szSetting, len);
	char *tok = strchr(value, '/');
	if (tok == nullptr) {
		mir_free(value);
		return 1;
	}
	*tok = 0;
	*(uint32_t*)(value + len) = lParam;

	bkgrList = (char **)mir_realloc(bkgrList, sizeof(char*)*(bkgrCount + 1));
	bkgrList[bkgrCount] = value;
	bkgrCount++;
	return 0;
}

HRESULT BackgroundsLoadModule()
{
	CreateServiceFunction(MS_BACKGROUNDCONFIG_REGISTER, BkgrCfg_Register);
	return S_OK;
}

int BackgroundsUnloadModule(void)
{
	if (bkgrList != nullptr) {
		for (int indx = 0; indx < bkgrCount; indx++)
			mir_free(bkgrList[indx]);
		mir_free(bkgrList);
	}
	DestroyHookableEvent(g_CluiData.hEventBkgrChanged);
	g_CluiData.hEventBkgrChanged = nullptr;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct
{
	char *name;          // Tab name
	int id;              // Dialog id
	DLGPROC wnd_proc;    // Dialog function
}
static clist_opt_items[] =
{
	{ LPGEN("General"), IDD_OPT_CLIST, DlgProcClistOpts },
	{ LPGEN("Tray"), IDD_OPT_TRAY, DlgProcTrayOpts},
	{ LPGEN("List"), IDD_OPT_CLC, DlgProcClistListOpts },
	{ LPGEN("Window"), IDD_OPT_CLUI, DlgProcClistWindowOpts },
	{ LPGEN("Behavior"), IDD_OPT_CLUI_2, DlgProcClistBehaviourOpts },
	{ LPGEN("Status bar"), IDD_OPT_SBAR, DlgProcSBarOpts },
	{ LPGEN("Additional stuff"), IDD_OPT_META_CLC, DlgProcClistAdditionalOpts }
};

int ClcOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CLC);
	odp.szTitle.a = LPGEN("Contact list");
	odp.pfnDlgProc = DlgProcClistListOpts;
	odp.flags = ODPF_BOLDGROUPS;

	for (auto &it : clist_opt_items) {
		odp.pszTemplate = MAKEINTRESOURCEA(it.id);
		odp.szTab.a = it.name;
		odp.pfnDlgProc = it.wnd_proc;
		g_plugin.addOptions(wParam, &odp);
	}

	if (g_CluiData.fDisableSkinEngine) {
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CLIST_LISTBKG);
		odp.szGroup.a = LPGEN("Skins");
		odp.szTitle.a = LPGEN("Contact list");
		odp.szTab.a = LPGEN("List background");
		odp.pfnDlgProc = DlgProcClcBkgOpts;
		odp.flags = ODPF_BOLDGROUPS;
		g_plugin.addOptions(wParam, &odp);
	}
	return 0;
}
