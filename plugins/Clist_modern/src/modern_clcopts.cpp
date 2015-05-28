/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (с) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "hdr/modern_commonheaders.h"
#include "hdr/modern_clc.h"
#include "hdr/modern_commonprototypes.h"
#include "hdr/modern_defsettings.h"
#include "hdr/modern_effectenum.h"
#include "hdr/modern_sync.h"
#include <m_modernopt.h>
#include "m_fontservice.h"

INT_PTR CALLBACK DlgProcSBarOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#define FONTF_NORMAL	0
#define FONTF_BOLD		1
#define FONTF_ITALIC	2
#define FONTF_UNDERLINE	4

struct FontOptionsList {
	DWORD dwFlags;
	int fontID;
	TCHAR *szGroup;
	TCHAR *szDescr;
	COLORREF defColour;
	TCHAR *szDefFace;
	BYTE defCharset, defStyle;
	char defSize;
	FONTEFFECT defeffect;

	COLORREF colour;
	TCHAR szFace[LF_FACESIZE];
	BYTE charset, style;
	char size;
};

#define CLCGROUP			LPGENT("Contact list")_T("/")LPGENT("Contact names")
#define CLCLINESGROUP		LPGENT("Contact list")_T("/")LPGENT("Row items")
#define CLCFRAMESGROUP		LPGENT("Contact list")_T("/")LPGENT("Frame texts")
#define CLCCOLOURSGROUP		LPGENT("Contact list")_T("/")LPGENT("Special colors")

#define DEFAULT_COLOUR		RGB(0, 0, 0)
#define DEFAULT_GREYCOLOUR	RGB(128, 128, 128)
#define DEFAULT_BACKCOLOUR	RGB(255, 255, 255)

#define DEFAULT_FAMILY		_T("Arial")
#define DEFAULT_EFFECT		{ 0, 0x00000000, 0x00000000 }

#define DEFAULT_SIZE		-11
#define DEFAULT_SMALLSIZE	-8

static struct FontOptionsList fontOptionsList[] = {
	{ FIDF_CLASSGENERAL, FONTID_CONTACTS, CLCGROUP, LPGENT("Standard contacts"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_AWAY, CLCGROUP, LPGENT("Away contacts"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_DND, CLCGROUP, LPGENT("DND contacts"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_NA, CLCGROUP, LPGENT("NA contacts"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_OCCUPIED, CLCGROUP, LPGENT("Occupied contacts"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_CHAT, CLCGROUP, LPGENT("Free for chat contacts"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_INVISIBLE, CLCGROUP, LPGENT("Invisible contacts"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_PHONE, CLCGROUP, LPGENT("On the phone contacts"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_LUNCH, CLCGROUP, LPGENT("Out to lunch contacts"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_OFFLINE, CLCGROUP, LPGENT("Offline contacts"), DEFAULT_GREYCOLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_INVIS, CLCGROUP, LPGENT("Online contacts to whom you have a different visibility"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_OFFINVIS, CLCGROUP, LPGENT("Offline contacts to whom you have a different visibility"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_NOTONLIST, CLCGROUP, LPGENT("Contacts who are 'not on list'"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSHEADER, FONTID_OPENGROUPS, CLCGROUP, LPGENT("Open groups"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_BOLD, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSHEADER, FONTID_OPENGROUPCOUNTS, CLCGROUP, LPGENT("Open group member counts"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSHEADER, FONTID_CLOSEDGROUPS, CLCGROUP, LPGENT("Closed groups"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_BOLD, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSHEADER, FONTID_CLOSEDGROUPCOUNTS, CLCGROUP, LPGENT("Closed group member counts"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSSMALL, FONTID_DIVIDERS, CLCGROUP, LPGENT("Dividers"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },

	{ FIDF_CLASSSMALL, FONTID_SECONDLINE, CLCLINESGROUP, LPGENT("Second line"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SMALLSIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSSMALL, FONTID_THIRDLINE, CLCLINESGROUP, LPGENT("Third line"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SMALLSIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSSMALL, FONTID_CONTACT_TIME, CLCLINESGROUP, LPGENT("Contact time"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SMALLSIZE, DEFAULT_EFFECT },

	{ FIDF_CLASSGENERAL, FONTID_STATUSBAR_PROTONAME, CLCFRAMESGROUP, LPGENT("Status bar text"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_EVENTAREA, CLCFRAMESGROUP, LPGENT("Event area text"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
	{ FIDF_CLASSGENERAL, FONTID_VIEMODES, CLCFRAMESGROUP, LPGENT("Current view mode text"), DEFAULT_COLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
};

struct ColourOptionsList {
	char *chGroup;
	char *chName;
	TCHAR *szGroup;
	TCHAR *szDescr;
	COLORREF defColour;
};

static struct ColourOptionsList colourOptionsList[] = {
	{ "CLC", "BkColour", CLCGROUP, LPGENT("Background"), CLCDEFAULT_BKCOLOUR },
	{ "CLC", "Rows_BkColour", CLCLINESGROUP, LPGENT("Background"), CLCDEFAULT_BKCOLOUR },
	{ "CLC", "Frames_BkColour", CLCFRAMESGROUP, LPGENT("Background"), CLCDEFAULT_BKCOLOUR },

	{ "CLC", "HotTextColour", CLCCOLOURSGROUP, LPGENT("Hot text"), CLCDEFAULT_MODERN_HOTTEXTCOLOUR },
	{ "CLC", "SelTextColour", CLCCOLOURSGROUP, LPGENT("Selected text"), CLCDEFAULT_MODERN_SELTEXTCOLOUR },
	{ "CLC", "QuickSearchColour", CLCCOLOURSGROUP, LPGENT("Quick search text"), CLCDEFAULT_MODERN_QUICKSEARCHCOLOUR },

	{ "Menu", "TextColour", CLCCOLOURSGROUP, LPGENT("Menu text"), CLCDEFAULT_TEXTCOLOUR },
	{ "Menu", "SelTextColour", CLCCOLOURSGROUP, LPGENT("Selected menu text"), CLCDEFAULT_MODERN_SELTEXTCOLOUR },
	{ "FrameTitleBar", "TextColour", CLCCOLOURSGROUP, LPGENT("Frame title text"), CLCDEFAULT_TEXTCOLOUR },
	{ "StatusBar", "TextColour", CLCCOLOURSGROUP, LPGENT("Status bar text"), CLCDEFAULT_TEXTCOLOUR },
	{ "ModernSettings", "KeyColor", CLCCOLOURSGROUP, LPGENT("3rd party frames transparent back color"), SETTING_KEYCOLOR_DEFAULT }
};

void RegisterCLUIFonts(void)
{
	static bool registered = false;
	if (registered)
		return;

	FontIDT fontid = { 0 };
	EffectIDT effectid = { 0 };
	char idstr[10];
	int index = 0;

	fontid.cbSize = sizeof(fontid);
	mir_strncpy(fontid.dbSettingsGroup, "CLC", SIZEOF(fontid.dbSettingsGroup));

	effectid.cbSize = sizeof(effectid);
	mir_strncpy(effectid.dbSettingsGroup, "CLC", SIZEOF(effectid.dbSettingsGroup));

	for (int i = 0; i < SIZEOF(fontOptionsList); i++, index++) {
		fontid.flags = FIDF_DEFAULTVALID | FIDF_APPENDNAME | FIDF_SAVEPOINTSIZE | FIDF_ALLOWEFFECTS | FIDF_ALLOWREREGISTER | FIDF_NOAS;
		fontid.flags |= fontOptionsList[i].dwFlags;

		mir_tstrncpy(fontid.group, fontOptionsList[i].szGroup, SIZEOF(fontid.group));
		mir_tstrncpy(fontid.name, fontOptionsList[i].szDescr, SIZEOF(fontid.name));
		mir_snprintf(idstr, SIZEOF(idstr), "Font%d", fontOptionsList[i].fontID);
		mir_strncpy(fontid.prefix, idstr, SIZEOF(fontid.prefix));
		fontid.order = i + 1;

		fontid.deffontsettings.charset = fontOptionsList[i].defCharset;
		fontid.deffontsettings.colour = fontOptionsList[i].defColour;
		fontid.deffontsettings.size = fontOptionsList[i].defSize;
		fontid.deffontsettings.style = fontOptionsList[i].defStyle;
		mir_tstrncpy(fontid.deffontsettings.szFace, fontOptionsList[i].szDefFace, SIZEOF(fontid.deffontsettings.szFace));

		FontRegisterT(&fontid);

		mir_tstrncpy(effectid.group, fontOptionsList[i].szGroup, SIZEOF(effectid.group));
		mir_tstrncpy(effectid.name, fontOptionsList[i].szDescr, SIZEOF(effectid.name));
		mir_snprintf(idstr, SIZEOF(idstr), "Font%d", fontOptionsList[i].fontID);
		mir_strncpy(effectid.setting, idstr, SIZEOF(effectid.setting));
		effectid.order = i + 1;

		effectid.defeffect.effectIndex = fontOptionsList[i].defeffect.effectIndex;
		effectid.defeffect.baseColour = fontOptionsList[i].defeffect.baseColour;
		effectid.defeffect.secondaryColour = fontOptionsList[i].defeffect.secondaryColour;

		EffectRegisterT(&effectid);
	}

	ColourIDT colourid = { 0 };
	colourid.cbSize = sizeof(colourid);

	for (int i = 0; i < SIZEOF(colourOptionsList); i++) {
		mir_tstrncpy(colourid.group, colourOptionsList[i].szGroup, SIZEOF(colourid.group));
		mir_tstrncpy(colourid.name, colourOptionsList[i].szDescr, SIZEOF(colourid.group));
		mir_strncpy(colourid.setting, colourOptionsList[i].chName, SIZEOF(colourid.setting));
		mir_strncpy(colourid.dbSettingsGroup, colourOptionsList[i].chGroup, SIZEOF(colourid.dbSettingsGroup));
		colourid.defcolour = colourOptionsList[i].defColour;
		colourid.order = i + 1;
		ColourRegisterT(&colourid);
	}
	registered = true;
}

DWORD GetDefaultExStyle(void)
{
	BOOL param;
	DWORD ret = CLCDEFAULT_EXSTYLE;
	if (SystemParametersInfo(SPI_GETLISTBOXSMOOTHSCROLLING, 0, &param, FALSE) && !param)
		ret |= CLS_EX_NOSMOOTHSCROLLING;
	if (SystemParametersInfo(SPI_GETHOTTRACKING, 0, &param, FALSE) && !param)
		ret &= ~CLS_EX_TRACKSELECT;
	return ret;
}

void GetFontSetting(int i, LOGFONT *lf, COLORREF *colour, BYTE *effect, COLORREF *eColour1, COLORREF *eColour2)
{
	char idstr[32];

	int index;
	for (index = 0; index < SIZEOF(fontOptionsList); index++)
		if (fontOptionsList[index].fontID == i)
			break;

	if (index == SIZEOF(fontOptionsList))
		return;

	FontIDT fontid = { 0 };
	fontid.cbSize = sizeof(fontid);
	mir_tstrncpy(fontid.group, fontOptionsList[index].szGroup, SIZEOF(fontid.group));
	mir_tstrncpy(fontid.name, fontOptionsList[index].szDescr, SIZEOF(fontid.name));

	COLORREF col = CallService(MS_FONT_GETT, (WPARAM)&fontid, (LPARAM)lf);

	if (colour)
		*colour = col;

	if (effect) {
		mir_snprintf(idstr, SIZEOF(idstr), "Font%dEffect", i);
		*effect = db_get_b(NULL, "CLC", idstr, 0);
		mir_snprintf(idstr, SIZEOF(idstr), "Font%dEffectCol1", i);
		*eColour1 = db_get_dw(NULL, "CLC", idstr, 0);
		mir_snprintf(idstr, SIZEOF(idstr), "Font%dEffectCol2", i);
		*eColour2 = db_get_dw(NULL, "CLC", idstr, 0);
	}
}

struct CheckBoxToStyleEx_t {
	int id;
	DWORD flag;
	int neg;
};

static const struct CheckBoxToStyleEx_t checkBoxToStyleEx[] = {
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

struct CheckBoxValues_t {
	DWORD style;
	TCHAR *szDescr;
};

static const struct CheckBoxValues_t greyoutValues[] = {
	{ GREYF_UNFOCUS, LPGENT("Not focused") },
	{ MODEF_OFFLINE, LPGENT("Offline") },
	{ PF2_ONLINE, LPGENT("Online") },
	{ PF2_SHORTAWAY, LPGENT("Away") },
	{ PF2_LONGAWAY, LPGENT("NA") },
	{ PF2_LIGHTDND, LPGENT("Occupied") },
	{ PF2_HEAVYDND, LPGENT("DND") },
	{ PF2_FREECHAT, LPGENT("Free for chat") },
	{ PF2_INVISIBLE, LPGENT("Invisible") },
	{ PF2_OUTTOLUNCH, LPGENT("Out to lunch") },
	{ PF2_ONTHEPHONE, LPGENT("On the phone") }
};

static const struct CheckBoxValues_t offlineValues[] = {
	{ MODEF_OFFLINE, LPGENT("Offline") },
	{ PF2_ONLINE, LPGENT("Online") },
	{ PF2_SHORTAWAY, LPGENT("Away") },
	{ PF2_LONGAWAY, LPGENT("NA") },
	{ PF2_LIGHTDND, LPGENT("Occupied") },
	{ PF2_HEAVYDND, LPGENT("DND") },
	{ PF2_FREECHAT, LPGENT("Free for chat") },
	{ PF2_INVISIBLE, LPGENT("Invisible") },
	{ PF2_OUTTOLUNCH, LPGENT("Out to lunch") },
	{ PF2_ONTHEPHONE, LPGENT("On the phone") }
};

static void FillCheckBoxTree(HWND hwndTree, const struct CheckBoxValues_t *values, int nValues, DWORD style)
{
	TVINSERTSTRUCT tvis;
	tvis.hParent = NULL;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
	for (int i = 0; i < nValues; i++) {
		tvis.item.lParam = values[i].style;
		tvis.item.pszText = TranslateTS(values[i].szDescr);
		tvis.item.stateMask = TVIS_STATEIMAGEMASK;
		tvis.item.state = INDEXTOSTATEIMAGEMASK((style & tvis.item.lParam) != 0 ? 2 : 1);
		TreeView_InsertItem(hwndTree, &tvis);
	}
}

static DWORD MakeCheckBoxTreeFlags(HWND hwndTree)
{
	DWORD flags = 0;
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
	BYTE t = IsDlgButtonChecked(hwndDlg, IDC_METAEXPAND);
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
		CheckDlgButton(hwndDlg, IDC_META, db_get_b(NULL, "CLC", "Meta", SETTING_USEMETAICON_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR
		CheckDlgButton(hwndDlg, IDC_METADBLCLK, db_get_b(NULL, "CLC", "MetaDoubleClick", SETTING_METAAVOIDDBLCLICK_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR
		CheckDlgButton(hwndDlg, IDC_METASUBEXTRA, db_get_b(NULL, "CLC", "MetaHideExtra", SETTING_METAHIDEEXTRA_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR
		CheckDlgButton(hwndDlg, IDC_METASUBEXTRA_IGN, db_get_b(NULL, "CLC", "MetaIgnoreEmptyExtra", SETTING_METAAVOIDDBLCLICK_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR
		CheckDlgButton(hwndDlg, IDC_METAEXPAND, db_get_b(NULL, "CLC", "MetaExpanding", SETTING_METAEXPANDING_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR
		CheckDlgButton(hwndDlg, IDC_DISCOVER_AWAYMSG, db_get_b(NULL, "ModernData", "InternalAwayMsgDiscovery", SETTING_INTERNALAWAYMSGREQUEST_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR
		CheckDlgButton(hwndDlg, IDC_REMOVE_OFFLINE_AWAYMSG, db_get_b(NULL, "ModernData", "RemoveAwayMessageForOffline", SETTING_REMOVEAWAYMSGFOROFFLINE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR
		CheckDlgButton(hwndDlg, IDC_FILTER_SEARCH, db_get_b(NULL, "CLC", "FilterSearch", SETTING_FILTERSEARCH_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by Robyer

		SendDlgItemMessage(hwndDlg, IDC_SUBINDENTSPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_SUBINDENT), 0);
		SendDlgItemMessage(hwndDlg, IDC_SUBINDENTSPIN, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_SUBINDENTSPIN, UDM_SETPOS, 0, MAKELONG(db_get_b(NULL, "CLC", "SubIndent", CLCDEFAULT_GROUPINDENT), 0));
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
				db_set_b(NULL, "CLC", "Meta", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_META));
				db_set_b(NULL, "CLC", "MetaDoubleClick", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_METADBLCLK));
				db_set_b(NULL, "CLC", "MetaHideExtra", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_METASUBEXTRA));
				db_set_b(NULL, "CLC", "MetaIgnoreEmptyExtra", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_METASUBEXTRA_IGN));
				db_set_b(NULL, "CLC", "MetaExpanding", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_METAEXPAND));
				db_set_b(NULL, "ModernData", "InternalAwayMsgDiscovery", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DISCOVER_AWAYMSG));
				db_set_b(NULL, "ModernData", "RemoveAwayMessageForOffline", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_REMOVE_OFFLINE_AWAYMSG));
				db_set_b(NULL, "CLC", "FilterSearch", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_FILTER_SEARCH));

				db_set_b(NULL, "CLC", "SubIndent", (BYTE)SendDlgItemMessage(hwndDlg, IDC_SUBINDENTSPIN, UDM_GETPOS, 0, 0));
				ClcOptionsChanged();
				CLUI_ReloadCLUIOptions();
				PostMessage(pcli->hwndContactList, WM_SIZE, 0, 0);
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
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_HIDEOFFLINEOPTS), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_HIDEOFFLINEOPTS), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		{
			DWORD exStyle = db_get_dw(NULL, "CLC", "ExStyle", GetDefaultExStyle());
			for (int i = 0; i < SIZEOF(checkBoxToStyleEx); i++)
				CheckDlgButton(hwndDlg, checkBoxToStyleEx[i].id, (exStyle&checkBoxToStyleEx[i].flag) ^ (checkBoxToStyleEx[i].flag*checkBoxToStyleEx[i].neg) ? BST_CHECKED : BST_UNCHECKED);

			UDACCEL accel[2] = { { 0, 10 }, { 2, 50 } };
			SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_SETRANGE, 0, MAKELONG(999, 0));
			SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_SETACCEL, SIZEOF(accel), (LPARAM)&accel);
			SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_SETPOS, 0, MAKELONG(db_get_w(NULL, "CLC", "ScrollTime", CLCDEFAULT_SCROLLTIME), 0));
		}
		CheckDlgButton(hwndDlg, IDC_IDLE, db_get_b(NULL, "CLC", "ShowIdle", CLCDEFAULT_SHOWIDLE) ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg, IDC_GROUPINDENTSPIN, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_GROUPINDENTSPIN, UDM_SETPOS, 0, MAKELONG(db_get_b(NULL, "CLC", "GroupIndent", CLCDEFAULT_GROUPINDENT), 0));
		CheckDlgButton(hwndDlg, IDC_GREYOUT, db_get_dw(NULL, "CLC", "GreyoutFlags", CLCDEFAULT_GREYOUTFLAGS) ? BST_CHECKED : BST_UNCHECKED);

		EnableWindow(GetDlgItem(hwndDlg, IDC_SMOOTHTIME), IsDlgButtonChecked(hwndDlg, IDC_NOTNOSMOOTHSCROLLING));
		EnableWindow(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), IsDlgButtonChecked(hwndDlg, IDC_GREYOUT));
		FillCheckBoxTree(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), greyoutValues, SIZEOF(greyoutValues), db_get_dw(NULL, "CLC", "FullGreyoutFlags", CLCDEFAULT_FULLGREYOUTFLAGS));
		FillCheckBoxTree(GetDlgItem(hwndDlg, IDC_HIDEOFFLINEOPTS), offlineValues, SIZEOF(offlineValues), db_get_dw(NULL, "CLC", "OfflineModes", CLCDEFAULT_OFFLINEMODES));
		CheckDlgButton(hwndDlg, IDC_NOSCROLLBAR, db_get_b(NULL, "CLC", "NoVScrollBar", CLCDEFAULT_NOVSCROLL) ? BST_CHECKED : BST_UNCHECKED);
		return TRUE;

	case WM_VSCROLL:
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_NOTNOSMOOTHSCROLLING)
			EnableWindow(GetDlgItem(hwndDlg, IDC_SMOOTHTIME), IsDlgButtonChecked(hwndDlg, IDC_NOTNOSMOOTHSCROLLING));
		if (LOWORD(wParam) == IDC_GREYOUT)
			EnableWindow(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS), IsDlgButtonChecked(hwndDlg, IDC_GREYOUT));
		if ((/*LOWORD(wParam) == IDC_LEFTMARGIN  || */ LOWORD(wParam) == IDC_SMOOTHTIME || LOWORD(wParam) == IDC_GROUPINDENT) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())) return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_GREYOUTOPTS:
		case IDC_HIDEOFFLINEOPTS:
			if (((LPNMHDR)lParam)->code == NM_CLICK) {
				TVHITTESTINFO hti;
				hti.pt.x = (short)LOWORD(GetMessagePos());
				hti.pt.y = (short)HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
				if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti))
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
			break;

		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				DWORD exStyle = 0;
				for (int i = 0; i < SIZEOF(checkBoxToStyleEx); i++)
					if ((IsDlgButtonChecked(hwndDlg, checkBoxToStyleEx[i].id) == 0) == checkBoxToStyleEx[i].neg)
						exStyle |= checkBoxToStyleEx[i].flag;
				db_set_dw(NULL, "CLC", "ExStyle", exStyle);

				DWORD fullGreyoutFlags = MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg, IDC_GREYOUTOPTS));
				db_set_dw(NULL, "CLC", "FullGreyoutFlags", fullGreyoutFlags);
				if (IsDlgButtonChecked(hwndDlg, IDC_GREYOUT))
					db_set_dw(NULL, "CLC", "GreyoutFlags", fullGreyoutFlags);
				else
					db_set_dw(NULL, "CLC", "GreyoutFlags", 0);

				db_set_b(NULL, "CLC", "ShowIdle", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_IDLE) ? 1 : 0));
				db_set_dw(NULL, "CLC", "OfflineModes", MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg, IDC_HIDEOFFLINEOPTS)));
				db_set_w(NULL, "CLC", "ScrollTime", (WORD)SendDlgItemMessage(hwndDlg, IDC_SMOOTHTIMESPIN, UDM_GETPOS, 0, 0));
				db_set_b(NULL, "CLC", "GroupIndent", (BYTE)SendDlgItemMessage(hwndDlg, IDC_GROUPINDENTSPIN, UDM_GETPOS, 0, 0));
				db_set_b(NULL, "CLC", "NoVScrollBar", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_NOSCROLLBAR) ? 1 : 0));

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
	int i, count, netProtoCount;
	PROTOACCOUNT **accs;
	ProtoEnumAccounts(&count, &accs);
	for (i = 0, netProtoCount = 0; i < count; i++) {
		if (pcli->pfnGetProtocolVisibility(accs[i]->szModuleName) == 0)
			continue;
		netProtoCount++;
	}
	return netProtoCount;
}

TCHAR *sortby[] = { LPGENT("Name"), LPGENT("Name (use locale settings)"), LPGENT("Status"), LPGENT("Last message time"), LPGENT("Account name"), LPGENT("Rate"), LPGENT("-Nothing-") };
int sortbyValue[] = { SORTBY_NAME, SORTBY_NAME_LOCALE, SORTBY_STATUS, SORTBY_LASTMSG, SORTBY_PROTO, SORTBY_RATE, SORTBY_NOTHING };
static INT_PTR CALLBACK DlgProcClistOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		CheckDlgButton(hwndDlg, IDC_HIDEOFFLINE, db_get_b(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_HIDEEMPTYGROUPS, db_get_b(NULL, "CList", "HideEmptyGroups", SETTING_HIDEEMPTYGROUPS_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DISABLEGROUPS, db_get_b(NULL, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT) ? BST_UNCHECKED : BST_CHECKED);
		CheckDlgButton(hwndDlg, IDC_CONFIRMDELETE, db_get_b(NULL, "CList", "ConfirmDelete", SETTING_CONFIRMDELETE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_GAMMACORRECT, db_get_b(NULL, "CLC", "GammaCorrect", CLCDEFAULT_GAMMACORRECT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_HILIGHTMODE, db_get_b(NULL, "CLC", "HiLightMode", SETTING_HILIGHTMODE_DEFAULT) == 0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_HILIGHTMODE1, db_get_b(NULL, "CLC", "HiLightMode", SETTING_HILIGHTMODE_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_HILIGHTMODE2, db_get_b(NULL, "CLC", "HiLightMode", SETTING_HILIGHTMODE_DEFAULT) == 2 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_HILIGHTMODE3, db_get_b(NULL, "CLC", "HiLightMode", SETTING_HILIGHTMODE_DEFAULT) == 3 ? BST_CHECKED : BST_UNCHECKED);
		{
			int i, item;
			int s1, s2, s3;
			for (i = 0; i < SIZEOF(sortby); i++) {
				item = SendDlgItemMessage(hwndDlg, IDC_CLSORT1, CB_ADDSTRING, 0, (LPARAM)TranslateTS(sortby[i]));
				SendDlgItemMessage(hwndDlg, IDC_CLSORT1, CB_SETITEMDATA, item, 0);
				item = SendDlgItemMessage(hwndDlg, IDC_CLSORT2, CB_ADDSTRING, 0, (LPARAM)TranslateTS(sortby[i]));
				SendDlgItemMessage(hwndDlg, IDC_CLSORT2, CB_SETITEMDATA, item, 0);
				item = SendDlgItemMessage(hwndDlg, IDC_CLSORT3, CB_ADDSTRING, 0, (LPARAM)TranslateTS(sortby[i]));
				SendDlgItemMessage(hwndDlg, IDC_CLSORT3, CB_SETITEMDATA, item, 0);

			}
			s1 = db_get_b(NULL, "CList", "SortBy1", SETTING_SORTBY1_DEFAULT);
			s2 = db_get_b(NULL, "CList", "SortBy2", SETTING_SORTBY2_DEFAULT);
			s3 = db_get_b(NULL, "CList", "SortBy3", SETTING_SORTBY3_DEFAULT);

			for (i = 0; i < SIZEOF(sortby); i++) {
				if (s1 == sortbyValue[i])
					SendDlgItemMessage(hwndDlg, IDC_CLSORT1, CB_SETCURSEL, i, 0);
				if (s2 == sortbyValue[i])
					SendDlgItemMessage(hwndDlg, IDC_CLSORT2, CB_SETCURSEL, i, 0);
				if (s3 == sortbyValue[i])
					SendDlgItemMessage(hwndDlg, IDC_CLSORT3, CB_SETCURSEL, i, 0);
			}

			CheckDlgButton(hwndDlg, IDC_NOOFFLINEMOVE, db_get_b(NULL, "CList", "NoOfflineBottom", SETTING_NOOFFLINEBOTTOM_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_OFFLINETOROOT, db_get_b(NULL, "CList", "PlaceOfflineToRoot", SETTING_PLACEOFFLINETOROOT_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
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
				db_set_b(NULL, "CList", "HideOffline", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_HIDEOFFLINE));
				db_set_b(NULL, "CList", "HideEmptyGroups", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_HIDEEMPTYGROUPS));
				db_set_b(NULL, "CList", "UseGroups", (BYTE)BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DISABLEGROUPS));
				db_set_b(NULL, "CList", "ConfirmDelete", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CONFIRMDELETE));

				db_set_b(NULL, "CLC", "GammaCorrect", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_GAMMACORRECT));
				int hil = 0;
				if (IsDlgButtonChecked(hwndDlg, IDC_HILIGHTMODE1))  hil = 1;
				if (IsDlgButtonChecked(hwndDlg, IDC_HILIGHTMODE2))  hil = 2;
				if (IsDlgButtonChecked(hwndDlg, IDC_HILIGHTMODE3))  hil = 3;
				db_set_b(NULL, "CLC", "HiLightMode", (BYTE)hil);

				int s1 = SendDlgItemMessage(hwndDlg, IDC_CLSORT1, CB_GETCURSEL, 0, 0);
				int s2 = SendDlgItemMessage(hwndDlg, IDC_CLSORT2, CB_GETCURSEL, 0, 0);
				int s3 = SendDlgItemMessage(hwndDlg, IDC_CLSORT3, CB_GETCURSEL, 0, 0);
				if (s1 >= 0) s1 = sortbyValue[s1];
				if (s2 >= 0) s2 = sortbyValue[s2];
				if (s3 >= 0) s3 = sortbyValue[s3];
				db_set_b(NULL, "CList", "SortBy1", (BYTE)s1);
				db_set_b(NULL, "CList", "SortBy2", (BYTE)s2);
				db_set_b(NULL, "CList", "SortBy3", (BYTE)s3);

				db_set_b(NULL, "CList", "NoOfflineBottom", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_NOOFFLINEMOVE));
				db_set_b(NULL, "CList", "PlaceOfflineToRoot", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_OFFLINETOROOT));

				pcli->pfnLoadContactTree(); /* this won't do job properly since it only really works when changes happen */
				pcli->pfnInitAutoRebuild(pcli->hwndContactTree); /* force reshuffle */
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

		CheckDlgButton(hwndDlg, IDC_NOOFFLINEMOVE, db_get_b(NULL, "CList", "NoOfflineBottom", SETTING_NOOFFLINEBOTTOM_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_OFFLINETOROOT, db_get_b(NULL, "CList", "PlaceOfflineToRoot", SETTING_PLACEOFFLINETOROOT_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ONECLK, db_get_b(NULL, "CList", "Tray1Click", SETTING_TRAY1CLICK_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		{
			BYTE trayOption = db_get_b(NULL, "CLUI", "XStatusTray", SETTING_TRAYOPTION_DEFAULT);
			CheckDlgButton(hwndDlg, IDC_SHOWXSTATUS, (trayOption & 3) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWNORMAL, (trayOption & 2) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TRANSPARENTOVERLAY, (trayOption & 4) ? BST_CHECKED : BST_UNCHECKED);

			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWNORMAL), IsDlgButtonChecked(hwndDlg, IDC_SHOWXSTATUS));
			EnableWindow(GetDlgItem(hwndDlg, IDC_TRANSPARENTOVERLAY), IsDlgButtonChecked(hwndDlg, IDC_SHOWXSTATUS) && IsDlgButtonChecked(hwndDlg, IDC_SHOWNORMAL));
		}
		CheckDlgButton(hwndDlg, IDC_ALWAYSSTATUS, db_get_b(NULL, "CList", "AlwaysStatus", SETTING_ALWAYSSTATUS_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_DISABLEBLINK, db_get_b(NULL, "CList", "DisableTrayFlash", SETTING_DISABLETRAYFLASH_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_BLINKTIME), 0);		// set buddy
		SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_SETRANGE, 0, MAKELONG(0x3FFF, 250));
		SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_SETPOS, 0, MAKELONG(db_get_w(NULL, "CList", "IconFlashTime", SETTING_ICONFLASHTIME_DEFAULT), 0));

		// == Tray icon mode ==
		// Готовим список аккаунтов.
		{
			int AccNum, i, siS, siV, item;
			PROTOACCOUNT **acc;

			ProtoEnumAccounts(&AccNum, &acc);

			for (siS = siV = -1, i = 0; i < AccNum; i++)
			{
				if (!acc[i]->bIsVirtual && acc[i]->bIsVisible && !acc[i]->bDynDisabled && acc[i]->bIsEnabled)
				{
					item = SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS_S, CB_ADDSTRING, 0, (LPARAM)acc[i]->tszAccountName);
					SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS_S, CB_SETITEMDATA, item, (LPARAM)acc[i]);

					if (!mir_strcmp(acc[i]->szModuleName, db_get_sa(NULL, "CList", "tiAccS")))
						siS = item;

					item = SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS_V, CB_ADDSTRING, 0, (LPARAM)acc[i]->tszAccountName);
					SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS_V, CB_SETITEMDATA, item, (LPARAM)acc[i]);

					if (!mir_strcmp(acc[i]->szModuleName, db_get_sa(NULL, "CList", "tiAccV")))
						siV = item;

				}
			}
			if (siS < 0) siS = 0; if (siV < 0) siV = 0; // Пустой элемент в качестве выбранного оставлять нельзя.
			SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS_S, CB_SETCURSEL, siS, 0);
			SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS_V, CB_SETCURSEL, siV, 0);
		}
		// Какой период смены иконок?
		SendDlgItemMessage(hwndDlg, IDC_CYCLETIMESPIN, UDM_SETRANGE, 0, MAKELONG(120, 1));
		SendDlgItemMessage(hwndDlg, IDC_CYCLETIMESPIN, UDM_SETPOS, 0, MAKELONG(db_get_w(NULL, "CList", "CycleTime", SETTING_CYCLETIME_DEFAULT), 0));
		// Какой режим иконок?
		switch (db_get_b(NULL, "CList", "tiModeS", TRAY_ICON_MODE_GLOBAL))
		{
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
		switch (db_get_b(NULL, "CList", "tiModeV", TRAY_ICON_MODE_GLOBAL))
		{
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

		if (   LOWORD(wParam) == IDC_ICON_GLOBAL_S
			|| LOWORD(wParam) == IDC_ICON_ACC_S
			|| LOWORD(wParam) == IDC_ICON_CYCLE_S
			|| LOWORD(wParam) == IDC_ICON_ALL_S)
			EnableWindow(GetDlgItem(hwndDlg, IDC_PRIMARYSTATUS_S), IsDlgButtonChecked(hwndDlg, IDC_ICON_ACC_S));
		if (   LOWORD(wParam) == IDC_ICON_GLOBAL_V
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
				db_set_b(NULL, "CList", "Tray1Click", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ONECLK));
				db_set_b(NULL, "CList", "AlwaysStatus", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ALWAYSSTATUS));

				db_set_w(NULL, "CList", "IconFlashTime", (WORD)SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_GETPOS, 0, 0));
				db_set_b(NULL, "CList", "DisableTrayFlash", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DISABLEBLINK));

				BYTE xOptions = 0;
				xOptions = IsDlgButtonChecked(hwndDlg, IDC_SHOWXSTATUS) ? 1 : 0;
				xOptions |= (xOptions && IsDlgButtonChecked(hwndDlg, IDC_SHOWNORMAL)) ? 2 : 0;
				xOptions |= (xOptions && IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENTOVERLAY)) ? 4 : 0;
				db_set_b(NULL, "CLUI", "XStatusTray", xOptions);

				// == Tray icon mode ==
				// Имя выбранного аккаунта.
				{
					PROTOACCOUNT *pa;
					pa = (PROTOACCOUNT*)SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS_S, CB_GETITEMDATA,
							SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS_S, CB_GETCURSEL, 0, 0), 0);
					db_set_s(NULL, "CList", "tiAccS", pa->szModuleName);
					pa = (PROTOACCOUNT*)SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS_V, CB_GETITEMDATA,
							SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS_V, CB_GETCURSEL, 0, 0), 0);
					db_set_s(NULL, "CList", "tiAccV", pa->szModuleName);
				}
				// Период смены иконок.
				db_set_w(NULL, "CList", "CycleTime", (WORD)SendDlgItemMessage(hwndDlg, IDC_CYCLETIMESPIN, UDM_GETPOS, 0, 0));
				// Режим иконок.
				db_set_b(NULL, "CList", "tiModeS",
					  IsDlgButtonChecked(hwndDlg, IDC_ICON_GLOBAL_S) << 0
					| IsDlgButtonChecked(hwndDlg, IDC_ICON_ACC_S) << 1
					| IsDlgButtonChecked(hwndDlg, IDC_ICON_CYCLE_S) << 2
					| IsDlgButtonChecked(hwndDlg, IDC_ICON_ALL_S) << 3);
				db_set_b(NULL, "CList", "tiModeV",
					  IsDlgButtonChecked(hwndDlg, IDC_ICON_GLOBAL_V) << 0
					| IsDlgButtonChecked(hwndDlg, IDC_ICON_ACC_V) << 1
					| IsDlgButtonChecked(hwndDlg, IDC_ICON_CYCLE_V) << 2
					| IsDlgButtonChecked(hwndDlg, IDC_ICON_ALL_V) << 3);

				pcli->pfnTrayIconIconsChanged();
				pcli->pfnLoadContactTree(); /* this won't do job properly since it only really works when changes happen */
				pcli->pfnInitAutoRebuild(pcli->hwndContactTree); /* force reshuffle */
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
	pcli->pfnClcBroadcast(INTM_RELOADOPTIONS, 0, 0);
	pcli->pfnClcBroadcast(INTM_INVALIDATE, 0, 0);
}

HWND g_hCLUIOptionsWnd = NULL;

static INT_PTR CALLBACK DlgProcClistBehaviourOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_CLIENTDRAG, db_get_b(NULL, "CLUI", "ClientAreaDrag", SETTING_CLIENTDRAG_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DRAGTOSCROLL, (db_get_b(NULL, "CLUI", "DragToScroll", SETTING_DRAGTOSCROLL_DEFAULT) && !db_get_b(NULL, "CLUI", "ClientAreaDrag", SETTING_CLIENTDRAG_DEFAULT)) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AUTOSIZE, g_CluiData.fAutoSize ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOCKSIZING, db_get_b(NULL, "CLUI", "LockSize", SETTING_LOCKSIZE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_BRINGTOFRONT, db_get_b(NULL, "CList", "BringToFront", SETTING_BRINGTOFRONT_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg, IDC_MAXSIZESPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
		SendDlgItemMessage(hwndDlg, IDC_MAXSIZESPIN, UDM_SETPOS, 0, db_get_b(NULL, "CLUI", "MaxSizeHeight", SETTING_MAXSIZEHEIGHT_DEFAULT));
		SendDlgItemMessage(hwndDlg, IDC_MINSIZESPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
		SendDlgItemMessage(hwndDlg, IDC_MINSIZESPIN, UDM_SETPOS, 0, db_get_b(NULL, "CLUI", "MinSizeHeight", SETTING_MINSIZEHEIGHT_DEFAULT));
		CheckDlgButton(hwndDlg, IDC_AUTOSIZEUPWARD, db_get_b(NULL, "CLUI", "AutoSizeUpward", SETTING_AUTOSIZEUPWARD_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SNAPTOEDGES, db_get_b(NULL, "CLUI", "SnapToEdges", SETTING_SNAPTOEDGES_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DOCKTOSIDES, db_get_b(NULL, "CLUI", "DockToSides", SETTING_DOCKTOSIDES_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_EVENTAREA_NONE, db_get_b(NULL, "CLUI", "EventArea", SETTING_EVENTAREAMODE_DEFAULT) == 0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_EVENTAREA, db_get_b(NULL, "CLUI", "EventArea", SETTING_EVENTAREAMODE_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_EVENTAREA_ALWAYS, db_get_b(NULL, "CLUI", "EventArea", SETTING_EVENTAREAMODE_DEFAULT) == 2 ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_AUTOHIDE, db_get_b(NULL, "CList", "AutoHide", SETTING_AUTOHIDE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_SETRANGE, 0, MAKELONG(900, 1));
		SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_SETPOS, 0, MAKELONG(db_get_w(NULL, "CList", "HideTime", SETTING_HIDETIME_DEFAULT), 0));
		EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIME), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
		EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIMESPIN), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
		EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC01), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
		{
			TCHAR *hidemode[] = { TranslateT("Hide to tray"), TranslateT("Behind left edge"), TranslateT("Behind right edge") };
			for (int i = 0; i < SIZEOF(hidemode); i++) {
				int item = SendDlgItemMessage(hwndDlg, IDC_HIDEMETHOD, CB_ADDSTRING, 0, (LPARAM)hidemode[i]);
				SendDlgItemMessage(hwndDlg, IDC_HIDEMETHOD, CB_SETITEMDATA, item, 0);
				SendDlgItemMessage(hwndDlg, IDC_HIDEMETHOD, CB_SETCURSEL, db_get_b(NULL, "ModernData", "HideBehind", SETTING_HIDEBEHIND_DEFAULT), 0);
			}
		}
		SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN2, UDM_SETRANGE, 0, MAKELONG(600, 0));
		SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN2, UDM_SETPOS, 0, MAKELONG(db_get_w(NULL, "ModernData", "ShowDelay", SETTING_SHOWDELAY_DEFAULT), 0));
		SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN3, UDM_SETRANGE, 0, MAKELONG(600, 0));
		SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN3, UDM_SETPOS, 0, MAKELONG(db_get_w(NULL, "ModernData", "HideDelay", SETTING_HIDEDELAY_DEFAULT), 0));
		SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN4, UDM_SETRANGE, 0, MAKELONG(50, 1));
		SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN4, UDM_SETPOS, 0, MAKELONG(db_get_w(NULL, "ModernData", "HideBehindBorderSize", SETTING_HIDEBEHINDBORDERSIZE_DEFAULT), 0));
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
			db_set_b(NULL, "ModernData", "HideBehind", (BYTE)SendDlgItemMessage(hwndDlg, IDC_HIDEMETHOD, CB_GETCURSEL, 0, 0));
			db_set_w(NULL, "ModernData", "ShowDelay", (WORD)SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN2, UDM_GETPOS, 0, 0));
			db_set_w(NULL, "ModernData", "HideDelay", (WORD)SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN3, UDM_GETPOS, 0, 0));
			db_set_w(NULL, "ModernData", "HideBehindBorderSize", (WORD)SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN4, UDM_GETPOS, 0, 0));

			db_set_b(NULL, "CLUI", "DragToScroll", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DRAGTOSCROLL));
			db_set_b(NULL, "CList", "BringToFront", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_BRINGTOFRONT));
			g_mutex_bChangingMode = TRUE;
			db_set_b(NULL, "CLUI", "ClientAreaDrag", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CLIENTDRAG));
			db_set_b(NULL, "CLUI", "AutoSize", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
			db_set_b(NULL, "CLUI", "LockSize", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_LOCKSIZING));
			db_set_b(NULL, "CLUI", "MaxSizeHeight", (BYTE)GetDlgItemInt(hwndDlg, IDC_MAXSIZEHEIGHT, NULL, FALSE));
			db_set_b(NULL, "CLUI", "MinSizeHeight", (BYTE)GetDlgItemInt(hwndDlg, IDC_MINSIZEHEIGHT, NULL, FALSE));
			db_set_b(NULL, "CLUI", "AutoSizeUpward", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZEUPWARD));
			db_set_b(NULL, "CLUI", "SnapToEdges", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SNAPTOEDGES));

			db_set_b(NULL, "CLUI", "DockToSides", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DOCKTOSIDES));

			db_set_b(NULL, "CLUI", "EventArea",
				(BYTE)(IsDlgButtonChecked(hwndDlg, IDC_EVENTAREA_ALWAYS) ? 2 : (BYTE)IsDlgButtonChecked(hwndDlg, IDC_EVENTAREA) ? 1 : 0));

			db_set_b(NULL, "CList", "AutoHide", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			db_set_w(NULL, "CList", "HideTime", (WORD)SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_GETPOS, 0, 0));
			CLUI_ChangeWindowMode();
			SendMessage(pcli->hwndContactTree, WM_SIZE, 0, 0);	//forces it to send a cln_listsizechanged
			CLUI_ReloadCLUIOptions();
			EventArea_ConfigureEventArea();
			cliShowHide(0, 1);
			g_mutex_bChangingMode = FALSE;
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
		CheckDlgButton(hwndDlg, IDC_ONTOP, db_get_b(NULL, "CList", "OnTop", SETTING_ONTOP_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
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
			EnableWindow(GetDlgItem(hwndDlg, IDC_AEROGLASS), !fEnabled && (g_proc_DWMEnableBlurBehindWindow != NULL));
			EnableWindow(GetDlgItem(hwndDlg, IDC_TITLEBAR_STATIC), fEnabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ROUNDCORNERS), fEnabled);
		}
		{
			//====== Non-Layered Mode =====
			CheckDlgButton(hwndDlg, IDC_TOOLWND, db_get_b(NULL, "CList", "ToolWindow", SETTING_TOOLWINDOW_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_MIN2TRAY, db_get_b(NULL, "CList", "Min2Tray", SETTING_MIN2TRAY_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_BORDER, db_get_b(NULL, "CList", "ThinBorder", SETTING_THINBORDER_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOBORDERWND, db_get_b(NULL, "CList", "NoBorder", SETTING_NOBORDER_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			if (IsDlgButtonChecked(hwndDlg, IDC_TOOLWND))
				EnableWindow(GetDlgItem(hwndDlg, IDC_MIN2TRAY), FALSE);
			CheckDlgButton(hwndDlg, IDC_SHOWCAPTION, db_get_b(NULL, "CLUI", "ShowCaption", SETTING_SHOWCAPTION_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWMAINMENU, db_get_b(NULL, "CLUI", "ShowMainMenu", SETTING_SHOWMAINMENU_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
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
			CheckDlgButton(hwndDlg, IDC_DROPSHADOW, db_get_b(NULL, "CList", "WindowShadow", SETTING_WINDOWSHADOW_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ROUNDCORNERS, db_get_b(NULL, "CLC", "RoundCorners", SETTING_ROUNDCORNERS_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		}   //====== End of Non-Layered Mode =====

		CheckDlgButton(hwndDlg, IDC_FADEINOUT, db_get_b(NULL, "CLUI", "FadeInOut", SETTING_FADEIN_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ONDESKTOP, db_get_b(NULL, "CList", "OnDesktop", SETTING_ONDESKTOP_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_FRAMESSPIN, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_CAPTIONSSPIN, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_FRAMESSPIN, UDM_SETPOS, 0, db_get_dw(NULL, "CLUIFrames", "GapBetweenFrames", SETTING_GAPFRAMES_DEFAULT));
		SendDlgItemMessage(hwndDlg, IDC_CAPTIONSSPIN, UDM_SETPOS, 0, db_get_dw(NULL, "CLUIFrames", "GapBetweenTitleBar", SETTING_GAPTITLEBAR_DEFAULT));
		SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_SETRANGE, 0, MAKELONG(250, 0));
		SendDlgItemMessage(hwndDlg, IDC_RIGHTMARGINSPIN, UDM_SETRANGE, 0, MAKELONG(250, 0));
		SendDlgItemMessage(hwndDlg, IDC_TOPMARGINSPIN, UDM_SETRANGE, 0, MAKELONG(250, 0));
		SendDlgItemMessage(hwndDlg, IDC_BOTTOMMARGINSPIN, UDM_SETRANGE, 0, MAKELONG(250, 0));
		SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_SETPOS, 0, db_get_b(NULL, "CLUI", "LeftClientMargin", SETTING_LEFTCLIENTMARIGN_DEFAULT));
		SendDlgItemMessage(hwndDlg, IDC_RIGHTMARGINSPIN, UDM_SETPOS, 0, db_get_b(NULL, "CLUI", "RightClientMargin", SETTING_RIGHTCLIENTMARIGN_DEFAULT));
		SendDlgItemMessage(hwndDlg, IDC_TOPMARGINSPIN, UDM_SETPOS, 0, db_get_b(NULL, "CLUI", "TopClientMargin", SETTING_TOPCLIENTMARIGN_DEFAULT));
		SendDlgItemMessage(hwndDlg, IDC_BOTTOMMARGINSPIN, UDM_SETPOS, 0, db_get_b(NULL, "CLUI", "BottomClientMargin", SETTING_BOTTOMCLIENTMARIGN_DEFAULT));

		CheckDlgButton(hwndDlg, IDC_DISABLEENGINE, db_get_b(NULL, "ModernData", "DisableEngine", SETTING_DISABLESKIN_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AEROGLASS, db_get_b(NULL, "ModernData", "AeroGlass", SETTING_AEROGLASS_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);

		EnableWindow(GetDlgItem(hwndDlg, IDC_LAYERENGINE), !db_get_b(NULL, "ModernData", "DisableEngine", SETTING_DISABLESKIN_DEFAULT) ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_LAYERENGINE, (db_get_b(NULL, "ModernData", "EnableLayering", SETTING_ENABLELAYERING_DEFAULT) && !db_get_b(NULL, "ModernData", "DisableEngine", SETTING_DISABLESKIN_DEFAULT)) ? BST_UNCHECKED : BST_CHECKED);

		{
			DBVARIANT dbv = { 0 };
			TCHAR *s;
			if (!db_get_ts(NULL, "CList", "TitleText", &dbv))
				s = dbv.ptszVal;
			else
				s = _T(MIRANDANAME);
			SetDlgItemText(hwndDlg, IDC_TITLETEXT, s);
			db_free(&dbv);

			SendDlgItemMessage(hwndDlg, IDC_TITLETEXT, CB_ADDSTRING, 0, (LPARAM)MIRANDANAME);

			char szUin[20];
			mir_snprintf(szUin, SIZEOF(szUin), "%u", db_get_dw(NULL, "ICQ", "UIN", 0));
			SendDlgItemMessage(hwndDlg, IDC_TITLETEXT, CB_ADDSTRING, 0, (LPARAM)szUin);

			if (!db_get_s(NULL, "ICQ", "Nick", &dbv)) {
				SendDlgItemMessage(hwndDlg, IDC_TITLETEXT, CB_ADDSTRING, 0, (LPARAM)dbv.pszVal);
				db_free(&dbv);
				dbv.pszVal = NULL;
			}
			if (!db_get_s(NULL, "ICQ", "FirstName", &dbv)) {
				SendDlgItemMessage(hwndDlg, IDC_TITLETEXT, CB_ADDSTRING, 0, (LPARAM)dbv.pszVal);
				db_free(&dbv);
				dbv.pszVal = NULL;
			}
			if (!db_get_s(NULL, "ICQ", "e-mail", &dbv)) {
				SendDlgItemMessage(hwndDlg, IDC_TITLETEXT, CB_ADDSTRING, 0, (LPARAM)dbv.pszVal);
				db_free(&dbv);
				dbv.pszVal = NULL;
			}
		}
		CheckDlgButton(hwndDlg, IDC_TRANSPARENT, db_get_b(NULL, "CList", "Transparent", SETTING_TRANSPARENT_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
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
		SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_SETPOS, TRUE, db_get_b(NULL, "CList", "Alpha", SETTING_ALPHA_DEFAULT));
		SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_SETPOS, TRUE, db_get_b(NULL, "CList", "AutoAlpha", SETTING_AUTOALPHA_DEFAULT));
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
			EnableWindow(GetDlgItem(hwndDlg, IDC_AEROGLASS), !fEnabled && (g_proc_DWMEnableBlurBehindWindow != NULL));
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
		mir_snprintf(str, SIZEOF(str), "%d%%", 100 * SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_GETPOS, 0, 0) / 255);
		SetDlgItemTextA(hwndDlg, IDC_INACTIVEPERC, str);
		mir_snprintf(str, SIZEOF(str), "%d%%", 100 * SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_GETPOS, 0, 0) / 255);
		SetDlgItemTextA(hwndDlg, IDC_ACTIVEPERC, str);
	}
	if (wParam != 0x12345678)
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
	break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			db_set_b(NULL, "CLUI", "LeftClientMargin", (BYTE)SendDlgItemMessage(hwndDlg, IDC_LEFTMARGINSPIN, UDM_GETPOS, 0, 0));
			db_set_b(NULL, "CLUI", "RightClientMargin", (BYTE)SendDlgItemMessage(hwndDlg, IDC_RIGHTMARGINSPIN, UDM_GETPOS, 0, 0));
			db_set_b(NULL, "CLUI", "TopClientMargin", (BYTE)SendDlgItemMessage(hwndDlg, IDC_TOPMARGINSPIN, UDM_GETPOS, 0, 0));
			db_set_b(NULL, "CLUI", "BottomClientMargin", (BYTE)SendDlgItemMessage(hwndDlg, IDC_BOTTOMMARGINSPIN, UDM_GETPOS, 0, 0));
			db_set_b(NULL, "ModernData", "DisableEngine", IsDlgButtonChecked(hwndDlg, IDC_DISABLEENGINE));
			db_set_b(NULL, "ModernData", "AeroGlass", IsDlgButtonChecked(hwndDlg, IDC_AEROGLASS));
			if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DISABLEENGINE)) {
				if (IsDlgButtonChecked(hwndDlg, IDC_LAYERENGINE))
					db_set_b(NULL, "ModernData", "EnableLayering", 0);
				else
					db_unset(NULL, "ModernData", "EnableLayering");
			}
			g_CluiData.dwKeyColor = db_get_dw(NULL, "ModernSettings", "KeyColor", (DWORD)SETTING_KEYCOLOR_DEFAULT);
			db_set_b(NULL, "CList", "OnDesktop", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ONDESKTOP));
			db_set_b(NULL, "CList", "OnTop", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ONTOP));
			SetWindowPos(pcli->hwndContactList, IsDlgButtonChecked(hwndDlg, IDC_ONTOP) ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			db_set_b(NULL, "CLUI", "DragToScroll", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DRAGTOSCROLL));

			{ //======  Non-Layered Mode ======
				db_set_b(NULL, "CList", "ToolWindow", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_TOOLWND));
				db_set_b(NULL, "CLUI", "ShowCaption", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWCAPTION));
				db_set_b(NULL, "CLUI", "ShowMainMenu", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWMAINMENU));
				db_set_b(NULL, "CList", "ThinBorder", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_BORDER));
				db_set_b(NULL, "CList", "NoBorder", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_NOBORDERWND));
				{
					TCHAR title[256];
					GetDlgItemText(hwndDlg, IDC_TITLETEXT, title, SIZEOF(title));
					db_set_ws(NULL, "CList", "TitleText", title);
				}
				db_set_b(NULL, "CList", "Min2Tray", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_MIN2TRAY));
				db_set_b(NULL, "CList", "WindowShadow", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DROPSHADOW));
				db_set_b(NULL, "CLC", "RoundCorners", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ROUNDCORNERS));
			} //======  End of Non-Layered Mode ======
			g_mutex_bChangingMode = TRUE;

			if (IsDlgButtonChecked(hwndDlg, IDC_ONDESKTOP)) {
				HWND hProgMan = FindWindow(_T("Progman"), NULL);
				if (IsWindow(hProgMan)) {
					SetParent(pcli->hwndContactList, hProgMan);
					Sync(CLUIFrames_SetParentForContainers, (HWND)hProgMan);
					g_CluiData.fOnDesktop = 1;
				}
			}
			else {
				if (GetParent(pcli->hwndContactList)) {
					SetParent(pcli->hwndContactList, NULL);
					Sync(CLUIFrames_SetParentForContainers, (HWND)NULL);
				}
				g_CluiData.fOnDesktop = 0;
			}
			AniAva_UpdateParent();
			db_set_b(NULL, "CLUI", "FadeInOut", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_FADEINOUT));
			g_CluiData.fSmoothAnimation = (BYTE)IsDlgButtonChecked(hwndDlg, IDC_FADEINOUT);
			{
				int i1 = SendDlgItemMessage(hwndDlg, IDC_FRAMESSPIN, UDM_GETPOS, 0, 0);
				int i2 = SendDlgItemMessage(hwndDlg, IDC_CAPTIONSSPIN, UDM_GETPOS, 0, 0);

				db_set_dw(NULL, "CLUIFrames", "GapBetweenFrames", (DWORD)i1);
				db_set_dw(NULL, "CLUIFrames", "GapBetweenTitleBar", (DWORD)i2);
				Sync(CLUIFramesOnClistResize, (WPARAM)pcli->hwndContactList, 0);
			}
			db_set_b(NULL, "CList", "Transparent", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			db_set_b(NULL, "CList", "Alpha", (BYTE)SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_GETPOS, 0, 0));
			db_set_b(NULL, "CList", "AutoAlpha", (BYTE)SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_GETPOS, 0, 0));
			db_set_b(NULL, "CList", "OnDesktop", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ONDESKTOP));

			ske_LoadSkinFromDB();
			CLUI_UpdateLayeredMode();
			CLUI_ChangeWindowMode();
			SendMessage(pcli->hwndContactTree, WM_SIZE, 0, 0);	//forces it to send a cln_listsizechanged
			CLUI_ReloadCLUIOptions();
			cliShowHide(0, 1);
			g_mutex_bChangingMode = FALSE;
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

extern HINSTANCE g_hInst;

char **bkgrList = NULL;
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
	BYTE changed;
	BYTE useBitmap;
	COLORREF bkColor, selColor;
	char filename[MAX_PATH];
	WORD flags;
	BYTE useWinColours;
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
				dat->item[indx].useBitmap = db_get_b(NULL, module, "UseBitmap", DEFAULT_USEBITMAP);
				dat->item[indx].bkColor = db_get_dw(NULL, module, "BkColour", DEFAULT_BKCOLOUR);
				dat->item[indx].selColor = db_get_dw(NULL, module, "SelBkColour", DEFAULT_SELBKCOLOUR);
				dat->item[indx].useWinColours = db_get_b(NULL, module, "UseWinColours", CLCDEFAULT_USEWINDOWSCOLOURS);

				DBVARIANT dbv;
				if (!db_get_s(NULL, module, "BkBitmap", &dbv)) {
					int retval = PathToAbsolute(dbv.pszVal, dat->item[indx].filename);
					if (!retval || retval == CALLSERVICE_NOTFOUND)
						mir_strncpy(dat->item[indx].filename, dbv.pszVal, SIZEOF(dat->item[indx].filename));
					mir_free(dbv.pszVal);
				}
				else *dat->item[indx].filename = 0;

				dat->item[indx].flags = db_get_w(NULL, module, "BkBmpUse", DEFAULT_BKBMPUSE);
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

		GetDlgItemTextA(hwndDlg, IDC_FILENAME, dat->item[indx].filename, SIZEOF(dat->item[indx].filename));

		WORD flags = 0;
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
		int flags;
		int indx = wParam;
		if (indx == -1) break;
		flags = dat->item[indx].flags;
		if (indx == CB_ERR || indx >= dat->count) break;
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
		for (int indx = 0; indx < SIZEOF(bitmapRelatedControls); indx++)
			EnableWindow(GetDlgItem(hwndDlg, bitmapRelatedControls[indx]), isChecked);
	}
	break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_BROWSE) {
			TCHAR str[MAX_PATH], filter[512];
			BmpFilterGetStrings(filter, SIZEOF(filter));
			GetDlgItemText(hwndDlg, IDC_FILENAME, str, SIZEOF(str));

			OPENFILENAME ofn = { 0 };
			ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
			ofn.hwndOwner = hwndDlg;
			ofn.lpstrFilter = filter;
			ofn.lpstrFile = str;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			ofn.nMaxFile = SIZEOF(str);
			ofn.nMaxFileTitle = MAX_PATH;
			ofn.lpstrDefExt = _T("bmp");
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
					db_set_b(NULL, module, "UseBitmap", (BYTE)p.useBitmap);

					COLORREF col;
					if ((col = p.bkColor) == DEFAULT_BKCOLOUR)
						db_unset(NULL, module, "BkColour");
					else
						db_set_dw(NULL, module, "BkColour", col);

					if ((col = p.selColor) == DEFAULT_SELBKCOLOUR)
						db_unset(NULL, module, "SelBkColour");
					else
						db_set_dw(NULL, module, "SelBkColour", col);

					db_set_b(NULL, module, "UseWinColours", (BYTE)p.useWinColours);

					char str[MAX_PATH];
					int retval = PathToAbsolute(p.filename, str);
					if (!retval || retval == CALLSERVICE_NOTFOUND)
						db_set_s(NULL, module, "BkBitmap", p.filename);
					else
						db_set_s(NULL, module, "BkBitmap", str);

					db_set_w(NULL, module, "BkBmpUse", p.flags);
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

	char *value = (char *)mir_alloc(len + 4); // add room for flags (DWORD)
	memcpy(value, szSetting, len);
	char *tok = strchr(value, '/');
	if (tok == NULL) {
		mir_free(value);
		return 1;
	}
	*tok = 0;
	*(DWORD*)(value + len) = lParam;

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
	if (bkgrList != NULL) {
		for (int indx = 0; indx < bkgrCount; indx++)
			mir_free(bkgrList[indx]);
		mir_free(bkgrList);
	}
	DestroyHookableEvent(g_CluiData.hEventBkgrChanged);
	g_CluiData.hEventBkgrChanged = NULL;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

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
			return db_get_b(NULL, cb->dbModule, cb->dbSetting, cb->defValue);
		case DBVT_WORD:
			return db_get_w(NULL, cb->dbModule, cb->dbSetting, cb->defValue);
		case DBVT_DWORD:
			return db_get_dw(NULL, cb->dbModule, cb->dbSetting, cb->defValue);
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

void OptCheckBox_Load(HWND hwnd, struct OptCheckBox *cb)
{
	DWORD value = OptCheckBox_LoadValue(cb);
	if (cb->dwBit) value &= cb->dwBit;
	CheckDlgButton(hwnd, cb->idc, value ? BST_CHECKED : BST_UNCHECKED);
}

void OptCheckBox_Save(HWND hwnd, struct OptCheckBox *cb)
{
	DWORD value = IsDlgButtonChecked(hwnd, cb->idc) == BST_CHECKED;

	if (cb->dwBit) {
		DWORD curValue = OptCheckBox_LoadValue(cb);
		value = value ? (curValue | cb->dwBit) : (curValue & ~cb->dwBit);
	}

	switch (cb->dbType) {
	case DBVT_BYTE:
		db_set_b(NULL, cb->dbModule, cb->dbSetting, (BYTE)value);
		break;
	case DBVT_WORD:
		db_set_w(NULL, cb->dbModule, cb->dbSetting, (WORD)value);
		break;
	case DBVT_DWORD:
		db_set_dw(NULL, cb->dbModule, cb->dbSetting, (DWORD)value);
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

static struct OptCheckBox opts[] =
{
	//{IDC_, def, bit, dbtype, dbmodule, dbsetting, valtype, pval},
	{ IDC_ONTOP, SETTING_ONTOP_DEFAULT, 0, DBVT_BYTE, "CList", "OnTop" },
	{ IDC_SHOWPROTO, SETTING_SBARSHOW_DEFAULT, 2, DBVT_BYTE, "CLUI", "SBarShow" },
	{ IDC_SHOWSTATUS, SETTING_SBARSHOW_DEFAULT, 4, DBVT_BYTE, "CLUI", "SBarShow" },
	{ IDC_AUTOHIDE, SETTING_AUTOHIDE_DEFAULT, 0, DBVT_BYTE, "CList", "AutoHide" },
	{ IDC_FADEINOUT, SETTING_FADEIN_DEFAULT, 0, DBVT_BYTE, "CLUI", "FadeInOut" },
	{ IDC_TRANSPARENT, SETTING_TRANSPARENT_DEFAULT, 0, DBVT_BYTE, "CList", "Transparent" },
	{ IDC_SHOWGROUPCOUNTS, GetDefaultExStyle(), CLS_EX_SHOWGROUPCOUNTS, DBVT_DWORD, "CLC", "ExStyle" },
	{ IDC_HIDECOUNTSWHENEMPTY, GetDefaultExStyle(), CLS_EX_HIDECOUNTSWHENEMPTY, DBVT_DWORD, "CLC", "ExStyle" },
	{ IDC_MINIMODE, SETTING_COMPACTMODE_DEFAULT, 0, DBVT_BYTE, "CLC", "CompactMode" },
	{ IDC_SHOW_AVATARS, SETTINGS_SHOWAVATARS_DEFAULT, 0, DBVT_BYTE, "CList", "AvatarsShow" },
	{ IDC_SHOW_ANIAVATARS, FALSE, 0, DBVT_BYTE, "CList", "AvatarsAnimated" },
	{ IDC_SHOW, SETTING_SHOWTIME_DEFAULT, 0, DBVT_BYTE, "CList", "ContactTimeShow" },
};

static INT_PTR CALLBACK DlgProcModernOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool bInit = true;
	char str[10];

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		bInit = true;
		{
			int i, item;
			for (i = 0; i < SIZEOF(opts); ++i)
				OptCheckBox_Load(hwndDlg, opts + i);

			EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIME), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIMESPIN), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_SETRANGE, 0, MAKELONG(900, 1));
			SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_SETPOS, 0, MAKELONG(db_get_w(NULL, "CList", "HideTime", SETTING_HIDETIME_DEFAULT), 0));

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
			SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_SETPOS, TRUE, db_get_b(NULL, "CList", "Alpha", SETTING_ALPHA_DEFAULT));
			SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_SETPOS, TRUE, db_get_b(NULL, "CList", "AutoAlpha", SETTING_AUTOALPHA_DEFAULT));
			SendMessage(hwndDlg, WM_HSCROLL, 0x12345678, 0);

			for (i = 0; i < SIZEOF(sortby); i++) {
				item = SendDlgItemMessage(hwndDlg, IDC_CLSORT1, CB_ADDSTRING, 0, (LPARAM)TranslateTS(sortby[i]));
				SendDlgItemMessage(hwndDlg, IDC_CLSORT1, CB_SETITEMDATA, item, 0);
				item = SendDlgItemMessage(hwndDlg, IDC_CLSORT2, CB_ADDSTRING, 0, (LPARAM)TranslateTS(sortby[i]));
				SendDlgItemMessage(hwndDlg, IDC_CLSORT2, CB_SETITEMDATA, item, 0);
				item = SendDlgItemMessage(hwndDlg, IDC_CLSORT3, CB_ADDSTRING, 0, (LPARAM)TranslateTS(sortby[i]));
				SendDlgItemMessage(hwndDlg, IDC_CLSORT3, CB_SETITEMDATA, item, 0);

			}
			int s1 = db_get_b(NULL, "CList", "SortBy1", SETTING_SORTBY1_DEFAULT);
			int s2 = db_get_b(NULL, "CList", "SortBy2", SETTING_SORTBY2_DEFAULT);
			int s3 = db_get_b(NULL, "CList", "SortBy3", SETTING_SORTBY3_DEFAULT);

			for (i = 0; i < SIZEOF(sortby); i++) {
				if (s1 == sortbyValue[i])
					SendDlgItemMessage(hwndDlg, IDC_CLSORT1, CB_SETCURSEL, i, 0);
				if (s2 == sortbyValue[i])
					SendDlgItemMessage(hwndDlg, IDC_CLSORT2, CB_SETCURSEL, i, 0);
				if (s3 == sortbyValue[i])
					SendDlgItemMessage(hwndDlg, IDC_CLSORT3, CB_SETCURSEL, i, 0);
			}

			bInit = false;
		}
		return TRUE;

	case WM_DESTROY:
		bInit = true;
		break;

	case WM_HSCROLL:
		mir_snprintf(str, SIZEOF(str), "%d%%", 100 * SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_GETPOS, 0, 0) / 255);
		SetDlgItemTextA(hwndDlg, IDC_INACTIVEPERC, str);
		mir_snprintf(str, SIZEOF(str), "%d%%", 100 * SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_GETPOS, 0, 0) / 255);
		SetDlgItemTextA(hwndDlg, IDC_ACTIVEPERC, str);
		if (wParam != 0x12345678 && !bInit)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_AUTOHIDE:
			EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIME), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIMESPIN), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			break;

		case IDC_TRANSPARENT:
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC11), IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC12), IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_TRANSACTIVE), IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_TRANSINACTIVE), IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_ACTIVEPERC), IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_INACTIVEPERC), IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
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
				g_mutex_bChangingMode = TRUE;

				for (int i = 0; i < SIZEOF(opts); ++i)
					OptCheckBox_Save(hwndDlg, opts + i);

				SetWindowPos(pcli->hwndContactList, IsDlgButtonChecked(hwndDlg, IDC_ONTOP) ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				db_set_w(NULL, "CList", "HideTime", (WORD)SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_GETPOS, 0, 0));

				db_set_b(NULL, "CList", "Alpha", (BYTE)SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_GETPOS, 0, 0));
				db_set_b(NULL, "CList", "AutoAlpha", (BYTE)SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_GETPOS, 0, 0));

				int s1 = SendDlgItemMessage(hwndDlg, IDC_CLSORT1, CB_GETCURSEL, 0, 0);
				int s2 = SendDlgItemMessage(hwndDlg, IDC_CLSORT2, CB_GETCURSEL, 0, 0);
				int s3 = SendDlgItemMessage(hwndDlg, IDC_CLSORT3, CB_GETCURSEL, 0, 0);
				if (s1 >= 0) s1 = sortbyValue[s1];
				if (s2 >= 0) s2 = sortbyValue[s2];
				if (s3 >= 0) s3 = sortbyValue[s3];
				db_set_b(NULL, "CList", "SortBy1", (BYTE)s1);
				db_set_b(NULL, "CList", "SortBy2", (BYTE)s2);
				db_set_b(NULL, "CList", "SortBy3", (BYTE)s3);

				ClcOptionsChanged();
				AniAva_UpdateOptions();
				ske_LoadSkinFromDB();
				CLUI_UpdateLayeredMode();
				CLUI_ChangeWindowMode();
				SendMessage(pcli->hwndContactTree, WM_SIZE, 0, 0);	//forces it to send a cln_listsizechanged
				CLUI_ReloadCLUIOptions();
				cliShowHide(0, 1);
			}
			g_mutex_bChangingMode = FALSE;
			return TRUE;
		}
		break;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

int ModernOptInit(WPARAM wParam, LPARAM)
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
	obj.iSection = MODERNOPT_PAGE_CLIST;
	obj.iType = MODERNOPT_TYPE_SECTIONPAGE;
	obj.iBoldControls = iBoldControls;
	obj.lptzSubsection = _T("Fonts");
	obj.lpzClassicGroup = NULL;
	obj.lpzClassicPage = "Contact list";
	obj.lpzHelpUrl = "http://wiki.miranda-ng.org/";

	obj.lpzTemplate = MAKEINTRESOURCEA(IDD_MODERNOPTS);
	obj.pfnDlgProc = DlgProcModernOptions;
	CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct
{
	char *name;			// Tab name
	int id;					// Dialog id
	DLGPROC wnd_proc;		// Dialog function
	DWORD flag;				// Expertonly
}
static clist_opt_items[] =
{
	{ LPGEN("General"), IDD_OPT_CLIST, DlgProcClistOpts, 0 },
	{ LPGEN("Tray"), IDD_OPT_TRAY, DlgProcTrayOpts, 0 },
	{ LPGEN("List"), IDD_OPT_CLC, DlgProcClistListOpts, 0 },
	{ LPGEN("Window"), IDD_OPT_CLUI, DlgProcClistWindowOpts, 0 },
	{ LPGEN("Behavior"), IDD_OPT_CLUI_2, DlgProcClistBehaviourOpts, 0 },
	{ LPGEN("Status bar"), IDD_OPT_SBAR, DlgProcSBarOpts, 0 },
	{ LPGEN("Additional stuff"), IDD_OPT_META_CLC, DlgProcClistAdditionalOpts, 0 }
};

int ClcOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = g_hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CLC);
	odp.pszTitle = LPGEN("Contact list");
	odp.pfnDlgProc = DlgProcClistListOpts;
	odp.flags = ODPF_BOLDGROUPS;

	for (int i = 0; i < SIZEOF(clist_opt_items); i++) {
		odp.pszTemplate = MAKEINTRESOURCEA(clist_opt_items[i].id);
		odp.pszTab = clist_opt_items[i].name;
		odp.pfnDlgProc = clist_opt_items[i].wnd_proc;
		odp.flags = ODPF_BOLDGROUPS | clist_opt_items[i].flag;
		Options_AddPage(wParam, &odp);
	}

	if (g_CluiData.fDisableSkinEngine) {
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CLIST_LISTBKG);
		odp.pszGroup = LPGEN("Skins");
		odp.pszTitle = LPGEN("Contact list");
		odp.pszTab = LPGEN("List background");
		odp.pfnDlgProc = DlgProcClcBkgOpts;
		odp.flags = ODPF_BOLDGROUPS;
		Options_AddPage(wParam, &odp);
	}
	return 0;
}
