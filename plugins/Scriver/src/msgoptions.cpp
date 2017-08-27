/*
Scriver

Copyright (c) 2000-12 Miranda ICQ/IM project,

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

INT_PTR CALLBACK DlgProcOptions1(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcOptions2(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcOptionsPopup(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#define FONTF_BOLD   1
#define FONTF_ITALIC 2

struct
{
	const wchar_t *szDescr;
	COLORREF defColour;
	const wchar_t *szDefFace;
	BYTE defStyle;
	char defSize;
	const wchar_t *szBkgName;
}
static const fontOptionsList[] =
{
	{ LPGENW("Outgoing messages"), RGB(106, 106, 106), L"Arial", 0, -12, LPGENW("Outgoing background")},
	{ LPGENW("Incoming messages"), RGB(0, 0, 0), L"Arial", 0, -12, LPGENW("Incoming background")},
	{ LPGENW("Outgoing name"), RGB(89, 89, 89), L"Arial", FONTF_BOLD, -12, LPGENW("Outgoing background")},
	{ LPGENW("Outgoing time"), RGB(0, 0, 0), L"Terminal", FONTF_BOLD, -9, LPGENW("Outgoing background")},
	{ LPGENW("Outgoing colon"), RGB(89, 89, 89), L"Arial", 0, -11, LPGENW("Outgoing background")},
	{ LPGENW("Incoming name"), RGB(215, 0, 0), L"Arial", FONTF_BOLD, -12, LPGENW("Incoming background")},
	{ LPGENW("Incoming time"), RGB(0, 0, 0), L"Terminal", FONTF_BOLD, -9, LPGENW("Incoming background")},
	{ LPGENW("Incoming colon"), RGB(215, 0, 0), L"Arial", 0, -11, LPGENW("Incoming background")},
	{ LPGENW("Message area"), RGB(0, 0, 0), L"Arial", 0, -12, LPGENW("Input area background")},
	{ LPGENW("Notices"), RGB(90, 90, 160), L"Arial", 0, -12, LPGENW("Incoming background")},
	{ LPGENW("Outgoing URL"), RGB(0, 0, 255), L"Arial", 0, -12, LPGENW("Outgoing background")},
	{ LPGENW("Incoming URL"), RGB(0, 0, 255), L"Arial", 0, -12, LPGENW("Incoming background")},
	{ LPGENW("Info bar contact name"), RGB(0, 0, 0), L"Arial", FONTF_BOLD, -19, LPGENW("Info bar background")},
	{ LPGENW("Info bar status message"), RGB(50, 50, 50), L"Arial", FONTF_ITALIC, -11, LPGENW("Info bar background")}
};

int fontOptionsListSize = _countof(fontOptionsList);

struct
{
	const wchar_t *szName;
	const char *szSettingName;
	COLORREF defColour;
	int systemColor;
}
static const colourOptionsList[] =
{
	{ LPGENW("Background"), SRMSGSET_BKGCOLOUR, 0, COLOR_WINDOW},
	{ LPGENW("Input area background"), SRMSGSET_INPUTBKGCOLOUR, 0, COLOR_WINDOW},
	{ LPGENW("Incoming background"), SRMSGSET_INCOMINGBKGCOLOUR, 0, COLOR_WINDOW},
	{ LPGENW("Outgoing background"), SRMSGSET_OUTGOINGBKGCOLOUR, 0, COLOR_WINDOW},
	{ LPGENW("Info bar background"), SRMSGSET_INFOBARBKGCOLOUR, 0, COLOR_3DLIGHT},
	{ LPGENW("Line between messages"), SRMSGSET_LINECOLOUR, 0, COLOR_3DLIGHT},
};

int FontServiceFontsChanged(WPARAM, LPARAM)
{
	LoadMsgLogIcons();
	LoadInfobarFonts();
	Srmm_Broadcast(DM_OPTIONSAPPLIED, 0, 0);
	return 0;
}

void RegisterFontServiceFonts()
{
	FontIDW fid = { sizeof(fid) };
	wcsncpy_s(fid.group, LPGENW("Messaging"), _TRUNCATE);
	wcsncpy_s(fid.backgroundGroup, LPGENW("Messaging"), _TRUNCATE);
	strncpy(fid.dbSettingsGroup, SRMM_MODULE, _countof(fid.dbSettingsGroup));
	fid.flags = FIDF_DEFAULTVALID | FIDF_DEFAULTVALID;
	for (int i = 0; i < _countof(fontOptionsList); i++) {
		fid.order = i;

		char szTemp[100];
		mir_snprintf(szTemp, "SRMFont%d", i);
		strncpy(fid.prefix, szTemp, _countof(fid.prefix));
		wcsncpy(fid.name, fontOptionsList[i].szDescr, _countof(fid.name));
		fid.deffontsettings.colour = fontOptionsList[i].defColour;
		fid.deffontsettings.size = fontOptionsList[i].defSize;
		fid.deffontsettings.style = fontOptionsList[i].defStyle;
		fid.deffontsettings.charset = DEFAULT_CHARSET;
		wcsncpy(fid.deffontsettings.szFace, fontOptionsList[i].szDefFace, _countof(fid.deffontsettings.szFace));
		wcsncpy(fid.backgroundName, fontOptionsList[i].szBkgName, _countof(fid.backgroundName));
		Font_RegisterW(&fid);
	}

	ColourIDW cid = { sizeof(cid) };
	wcsncpy_s(cid.group, LPGENW("Messaging"), _TRUNCATE);
	strncpy(cid.dbSettingsGroup, SRMM_MODULE, _countof(fid.dbSettingsGroup));
	cid.flags = 0;
	for (int i = 0; i < _countof(colourOptionsList); i++) {
		cid.order = i;
		wcsncpy(cid.name, colourOptionsList[i].szName, _countof(cid.name));
		if (colourOptionsList[i].systemColor != -1)
			cid.defcolour = GetSysColor(colourOptionsList[i].systemColor);
		else
			cid.defcolour = colourOptionsList[i].defColour;

		strncpy(cid.setting, colourOptionsList[i].szSettingName, _countof(cid.setting));
		Colour_RegisterW(&cid);
	}
}

int IconsChanged(WPARAM, LPARAM)
{
	ReleaseIcons();
	LoadGlobalIcons();
	FreeMsgLogIcons();
	LoadMsgLogIcons();
	ChangeStatusIcons();
	Srmm_Broadcast(DM_REMAKELOG, 0, 0);
	Srmm_Broadcast(DM_CHANGEICONS, 0, 1);
	return 0;
}

int SmileySettingsChanged(WPARAM wParam, LPARAM)
{
	Srmm_Broadcast(DM_REMAKELOG, wParam, 0);
	return 0;
}

void LoadMsgDlgFont(int i, LOGFONT *lf, COLORREF *colour)
{
	char str[32];

	if (colour) {
		mir_snprintf(str, "%s%dCol", "SRMFont", i);
		*colour = db_get_dw(0, SRMM_MODULE, str, fontOptionsList[i].defColour);
	}

	if (lf) {
		mir_snprintf(str, "%s%dSize", "SRMFont", i);
		lf->lfHeight = (char)db_get_b(0, SRMM_MODULE, str, fontOptionsList[i].defSize);
		lf->lfWidth = 0;
		lf->lfEscapement = 0;
		lf->lfOrientation = 0;
		mir_snprintf(str, "%s%dSty", "SRMFont", i);
		int style = db_get_b(0, SRMM_MODULE, str, fontOptionsList[i].defStyle);
		lf->lfWeight = style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
		lf->lfItalic = style & FONTF_ITALIC ? 1 : 0;
		lf->lfUnderline = 0;
		lf->lfStrikeOut = 0;
		lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf->lfQuality = DEFAULT_QUALITY;
		lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		mir_snprintf(str, "%s%d", "SRMFont", i);

		ptrW tszFace(db_get_wsa(0, SRMM_MODULE, str));
		if (tszFace == nullptr)
			mir_wstrcpy(lf->lfFaceName, fontOptionsList[i].szDefFace);
		else
			wcsncpy(lf->lfFaceName, tszFace, _countof(lf->lfFaceName));

		mir_snprintf(str, "%s%dSet", "SRMFont", i);
		lf->lfCharSet = db_get_b(0, SRMM_MODULE, str, DEFAULT_CHARSET);
	}
}

struct CheckBoxValues_t
{
	DWORD style;
	const wchar_t *szDescr;
};

static const struct CheckBoxValues_t statusValues[] =
{
	{ MODEF_OFFLINE, LPGENW("Offline") },
	{ PF2_ONLINE, LPGENW("Online") },
	{ PF2_SHORTAWAY, LPGENW("Away") },
	{ PF2_LONGAWAY, LPGENW("Not available") },
	{ PF2_LIGHTDND, LPGENW("Occupied") },
	{ PF2_HEAVYDND, LPGENW("Do not disturb") },
	{ PF2_FREECHAT, LPGENW("Free for chat") },
	{ PF2_INVISIBLE, LPGENW("Invisible") },
	{ PF2_OUTTOLUNCH, LPGENW("Out to lunch") },
	{ PF2_ONTHEPHONE, LPGENW("On the phone") }
};

static void FillCheckBoxTree(HWND hwndTree, const struct CheckBoxValues_t *values, int nValues, DWORD style)
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

static DWORD MakeCheckBoxTreeFlags(HWND hwndTree)
{
	DWORD flags = 0;
	TVITEM tvi;
	tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
	tvi.hItem = TreeView_GetRoot(hwndTree);
	while (tvi.hItem) {
		TreeView_GetItem(hwndTree, &tvi);
		if (((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 2))
			flags |= tvi.lParam;
		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
	return flags;
}

static int changed = 0;

static void ApplyChanges(int i)
{
	changed &= ~i;
	if (changed == 0) {
		ReloadGlobals();
		WindowList_Broadcast(g_dat.hParentWindowList, DM_OPTIONSAPPLIED, 0, 0);
		Srmm_Broadcast(DM_OPTIONSAPPLIED, 0, 0);
		Chat_UpdateOptions();
	}
}

static void MarkChanges(int i, HWND hWnd)
{
	SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
	changed |= i;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK DlgProcTabsOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int bChecked;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			CheckDlgButton(hwndDlg, IDC_USETABS, db_get_b(0, SRMM_MODULE, SRMSGSET_USETABS, SRMSGDEFSET_USETABS) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ALWAYSSHOWTABS, !db_get_b(0, SRMM_MODULE, SRMSGSET_HIDEONETAB, SRMSGDEFSET_HIDEONETAB) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TABSATBOTTOM, db_get_b(0, SRMM_MODULE, SRMSGSET_TABSATBOTTOM, SRMSGDEFSET_TABSATBOTTOM));
			CheckDlgButton(hwndDlg, IDC_SWITCHTOACTIVE, db_get_b(0, SRMM_MODULE, SRMSGSET_SWITCHTOACTIVE, SRMSGDEFSET_SWITCHTOACTIVE) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TABCLOSEBUTTON, db_get_b(0, SRMM_MODULE, SRMSGSET_TABCLOSEBUTTON, SRMSGDEFSET_TABCLOSEBUTTON) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_LIMITNAMES, db_get_b(0, SRMM_MODULE, SRMSGSET_LIMITNAMES, SRMSGDEFSET_LIMITNAMES) ? BST_CHECKED : BST_UNCHECKED);
			SendDlgItemMessage(hwndDlg, IDC_LIMITNAMESLENSPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
			SendDlgItemMessage(hwndDlg, IDC_LIMITNAMESLENSPIN, UDM_SETPOS, 0, db_get_w(0, SRMM_MODULE, SRMSGSET_LIMITNAMESLEN, SRMSGDEFSET_LIMITNAMESLEN));

			CheckDlgButton(hwndDlg, IDC_LIMITTABS, db_get_b(0, SRMM_MODULE, SRMSGSET_LIMITTABS, SRMSGDEFSET_LIMITTABS) ? BST_CHECKED : BST_UNCHECKED);
			SendDlgItemMessage(hwndDlg, IDC_LIMITTABSNUMSPIN, UDM_SETRANGE, 0, MAKELONG(100, 1));
			SendDlgItemMessage(hwndDlg, IDC_LIMITTABSNUMSPIN, UDM_SETPOS, 0, db_get_w(0, SRMM_MODULE, SRMSGSET_LIMITTABSNUM, SRMSGDEFSET_LIMITTABSNUM));

			CheckDlgButton(hwndDlg, IDC_LIMITCHATSTABS, db_get_b(0, SRMM_MODULE, SRMSGSET_LIMITCHATSTABS, SRMSGDEFSET_LIMITCHATSTABS) ? BST_CHECKED : BST_UNCHECKED);
			SendDlgItemMessage(hwndDlg, IDC_LIMITCHATSTABSNUMSPIN, UDM_SETRANGE, 0, MAKELONG(100, 1));
			SendDlgItemMessage(hwndDlg, IDC_LIMITCHATSTABSNUMSPIN, UDM_SETPOS, 0, db_get_w(0, SRMM_MODULE, SRMSGSET_LIMITCHATSTABSNUM, SRMSGDEFSET_LIMITCHATSTABSNUM));

			CheckDlgButton(hwndDlg, IDC_SEPARATECHATSCONTAINERS, db_get_b(0, SRMM_MODULE, SRMSGSET_SEPARATECHATSCONTAINERS, SRMSGDEFSET_SEPARATECHATSCONTAINERS) ? BST_CHECKED : BST_UNCHECKED);

			bChecked = IsDlgButtonChecked(hwndDlg, IDC_USETABS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ALWAYSSHOWTABS), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TABSATBOTTOM), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SWITCHTOACTIVE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TABCLOSEBUTTON), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMITNAMES), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SEPARATECHATSCONTAINERS), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMITTABS), bChecked);
			bChecked = IsDlgButtonChecked(hwndDlg, IDC_USETABS) && IsDlgButtonChecked(hwndDlg, IDC_LIMITNAMES);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMITNAMESLEN), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMITNAMESLENSPIN), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHARS), bChecked);
			bChecked = IsDlgButtonChecked(hwndDlg, IDC_USETABS) && IsDlgButtonChecked(hwndDlg, IDC_LIMITTABS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMITTABSNUM), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMITTABSNUMSPIN), bChecked);
			bChecked = IsDlgButtonChecked(hwndDlg, IDC_USETABS) && IsDlgButtonChecked(hwndDlg, IDC_SEPARATECHATSCONTAINERS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMITCHATSTABS), bChecked);
			bChecked = bChecked && IsDlgButtonChecked(hwndDlg, IDC_LIMITCHATSTABS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMITCHATSTABSNUM), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMITCHATSTABSNUMSPIN), bChecked);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_USETABS:
			bChecked = IsDlgButtonChecked(hwndDlg, IDC_USETABS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ALWAYSSHOWTABS), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TABSATBOTTOM), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SWITCHTOACTIVE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TABCLOSEBUTTON), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMITNAMES), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SEPARATECHATSCONTAINERS), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMITTABS), bChecked);

		case IDC_LIMITTABS:
			bChecked = IsDlgButtonChecked(hwndDlg, IDC_USETABS) && IsDlgButtonChecked(hwndDlg, IDC_LIMITTABS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMITTABSNUM), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMITTABSNUMSPIN), bChecked);

		case IDC_SEPARATECHATSCONTAINERS:
			bChecked = IsDlgButtonChecked(hwndDlg, IDC_USETABS) && IsDlgButtonChecked(hwndDlg, IDC_SEPARATECHATSCONTAINERS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMITCHATSTABS), bChecked);

		case IDC_LIMITCHATSTABS:
			bChecked = IsDlgButtonChecked(hwndDlg, IDC_USETABS) && IsDlgButtonChecked(hwndDlg, IDC_SEPARATECHATSCONTAINERS) &&
				IsDlgButtonChecked(hwndDlg, IDC_LIMITCHATSTABS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMITCHATSTABSNUM), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMITCHATSTABSNUMSPIN), bChecked);

		case IDC_LIMITNAMES:
			bChecked = IsDlgButtonChecked(hwndDlg, IDC_LIMITNAMES) && IsDlgButtonChecked(hwndDlg, IDC_USETABS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMITNAMESLEN), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMITNAMESLENSPIN), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHARS), bChecked);
			break;
		case IDC_LIMITNAMESLEN:
		case IDC_LIMITTABSNUM:
		case IDC_LIMITCHATSTABSNUM:
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
				return 0;
			break;
		}
		MarkChanges(8, hwndDlg);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				db_set_b(0, SRMM_MODULE, SRMSGSET_USETABS, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_USETABS));
				db_set_b(0, SRMM_MODULE, SRMSGSET_TABSATBOTTOM, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_TABSATBOTTOM));
				db_set_b(0, SRMM_MODULE, SRMSGSET_LIMITNAMES, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_LIMITNAMES));
				db_set_w(0, SRMM_MODULE, SRMSGSET_LIMITNAMESLEN, (WORD)SendDlgItemMessage(hwndDlg, IDC_LIMITNAMESLENSPIN, UDM_GETPOS, 0, 0));

				db_set_b(0, SRMM_MODULE, SRMSGSET_LIMITTABS, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_LIMITTABS));
				db_set_w(0, SRMM_MODULE, SRMSGSET_LIMITTABSNUM, (WORD)SendDlgItemMessage(hwndDlg, IDC_LIMITTABSNUMSPIN, UDM_GETPOS, 0, 0));
				db_set_b(0, SRMM_MODULE, SRMSGSET_LIMITCHATSTABS, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_LIMITCHATSTABS));
				db_set_w(0, SRMM_MODULE, SRMSGSET_LIMITCHATSTABSNUM, (WORD)SendDlgItemMessage(hwndDlg, IDC_LIMITCHATSTABSNUMSPIN, UDM_GETPOS, 0, 0));

				db_set_b(0, SRMM_MODULE, SRMSGSET_HIDEONETAB, (BYTE)BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_ALWAYSSHOWTABS));
				db_set_b(0, SRMM_MODULE, SRMSGSET_SWITCHTOACTIVE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SWITCHTOACTIVE));
				db_set_b(0, SRMM_MODULE, SRMSGSET_TABCLOSEBUTTON, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_TABCLOSEBUTTON));
				db_set_b(0, SRMM_MODULE, SRMSGSET_SEPARATECHATSCONTAINERS, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SEPARATECHATSCONTAINERS));

				ApplyChanges(8);
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK DlgProcLayoutOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int bChecked;
	char str[10];

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_SHOWSTATUSBAR, db_get_b(0, SRMM_MODULE, SRMSGSET_SHOWSTATUSBAR, SRMSGDEFSET_SHOWSTATUSBAR) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWTITLEBAR, db_get_b(0, SRMM_MODULE, SRMSGSET_SHOWTITLEBAR, SRMSGDEFSET_SHOWTITLEBAR) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWTOOLBAR, db_get_b(0, SRMM_MODULE, SRMSGSET_SHOWBUTTONLINE, SRMSGDEFSET_SHOWBUTTONLINE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWINFOBAR, db_get_b(0, SRMM_MODULE, SRMSGSET_SHOWINFOBAR, SRMSGDEFSET_SHOWINFOBAR) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_TRANSPARENCY, db_get_b(0, SRMM_MODULE, SRMSGSET_USETRANSPARENCY, SRMSGDEFSET_USETRANSPARENCY) ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_ATRANSPARENCYVALUE, TBM_SETRANGE, FALSE, MAKELONG(0, 255));
		SendDlgItemMessage(hwndDlg, IDC_ATRANSPARENCYVALUE, TBM_SETPOS, TRUE, db_get_dw(0, SRMM_MODULE, SRMSGSET_ACTIVEALPHA, SRMSGDEFSET_ACTIVEALPHA));
		SendDlgItemMessage(hwndDlg, IDC_ITRANSPARENCYVALUE, TBM_SETRANGE, FALSE, MAKELONG(0, 255));
		SendDlgItemMessage(hwndDlg, IDC_ITRANSPARENCYVALUE, TBM_SETPOS, TRUE, db_get_dw(0, SRMM_MODULE, SRMSGSET_INACTIVEALPHA, SRMSGDEFSET_INACTIVEALPHA));
		mir_snprintf(str, "%d%%", (int)(100 * SendDlgItemMessage(hwndDlg, IDC_ATRANSPARENCYVALUE, TBM_GETPOS, 0, 0) / 255));
		SetDlgItemTextA(hwndDlg, IDC_ATRANSPARENCYPERC, str);
		mir_snprintf(str, "%d%%", (int)(100 * SendDlgItemMessage(hwndDlg, IDC_ITRANSPARENCYVALUE, TBM_GETPOS, 0, 0) / 255));
		SetDlgItemTextA(hwndDlg, IDC_ITRANSPARENCYPERC, str);
		SendDlgItemMessage(hwndDlg, IDC_INPUTLINESSPIN, UDM_SETRANGE, 0, MAKELONG(100, 1));
		SendDlgItemMessage(hwndDlg, IDC_INPUTLINESSPIN, UDM_SETPOS, 0, db_get_w(0, SRMM_MODULE, SRMSGSET_AUTORESIZELINES, SRMSGDEFSET_AUTORESIZELINES));

		bChecked = IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENCY);
		EnableWindow(GetDlgItem(hwndDlg, IDC_ATRANSPARENCYVALUE), bChecked);
		EnableWindow(GetDlgItem(hwndDlg, IDC_ATRANSPARENCYPERC), bChecked);
		EnableWindow(GetDlgItem(hwndDlg, IDC_ITRANSPARENCYVALUE), bChecked);
		EnableWindow(GetDlgItem(hwndDlg, IDC_ITRANSPARENCYPERC), bChecked);
		EnableWindow(GetDlgItem(hwndDlg, IDC_TRANSPARENCYTEXT1), bChecked);
		EnableWindow(GetDlgItem(hwndDlg, IDC_TRANSPARENCYTEXT2), bChecked);

		CheckDlgButton(hwndDlg, IDC_STATUSWIN, db_get_b(0, SRMM_MODULE, SRMSGSET_STATUSICON, SRMSGDEFSET_STATUSICON) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWPROGRESS, db_get_b(0, SRMM_MODULE, SRMSGSET_SHOWPROGRESS, SRMSGDEFSET_SHOWPROGRESS) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AVATARSUPPORT, g_dat.flags & SMF_AVATAR);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_TRANSPARENCY:
			bChecked = IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENCY);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ATRANSPARENCYVALUE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ATRANSPARENCYPERC), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ITRANSPARENCYVALUE), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ITRANSPARENCYPERC), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TRANSPARENCYTEXT1), bChecked);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TRANSPARENCYTEXT2), bChecked);
			break;

		case IDC_INPUTLINES:
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
				return 0;
			break;
		}
		MarkChanges(16, hwndDlg);
		break;

	case WM_HSCROLL:
		mir_snprintf(str, "%d%%", (int)(100 * SendDlgItemMessage(hwndDlg, IDC_ATRANSPARENCYVALUE, TBM_GETPOS, 0, 0) / 256));
		SetDlgItemTextA(hwndDlg, IDC_ATRANSPARENCYPERC, str);
		mir_snprintf(str, "%d%%", (int)(100 * SendDlgItemMessage(hwndDlg, IDC_ITRANSPARENCYVALUE, TBM_GETPOS, 0, 0) / 256));
		SetDlgItemTextA(hwndDlg, IDC_ITRANSPARENCYPERC, str);
		MarkChanges(16, hwndDlg);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				db_set_b(0, SRMM_MODULE, SRMSGSET_SHOWSTATUSBAR, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWSTATUSBAR));
				db_set_b(0, SRMM_MODULE, SRMSGSET_SHOWTITLEBAR, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWTITLEBAR));
				db_set_b(0, SRMM_MODULE, SRMSGSET_SHOWBUTTONLINE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWTOOLBAR));
				db_set_b(0, SRMM_MODULE, SRMSGSET_SHOWINFOBAR, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWINFOBAR));

				db_set_b(0, SRMM_MODULE, SRMSGSET_USETRANSPARENCY, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENCY));
				db_set_dw(0, SRMM_MODULE, SRMSGSET_ACTIVEALPHA, SendDlgItemMessage(hwndDlg, IDC_ATRANSPARENCYVALUE, TBM_GETPOS, 0, 0));
				db_set_dw(0, SRMM_MODULE, SRMSGSET_INACTIVEALPHA, SendDlgItemMessage(hwndDlg, IDC_ITRANSPARENCYVALUE, TBM_GETPOS, 0, 0));

				db_set_b(0, SRMM_MODULE, SRMSGSET_STATUSICON, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_STATUSWIN));
				db_set_b(0, SRMM_MODULE, SRMSGSET_SHOWPROGRESS, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWPROGRESS));

				db_set_b(0, SRMM_MODULE, SRMSGSET_AVATARENABLE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AVATARSUPPORT));

				db_set_w(0, SRMM_MODULE, SRMSGSET_AUTORESIZELINES, (WORD)SendDlgItemMessage(hwndDlg, IDC_INPUTLINESSPIN, UDM_GETPOS, 0, 0));
				LoadInfobarFonts();

				ApplyChanges(16);
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_POPLIST), GWL_STYLE, (GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_POPLIST), GWL_STYLE) & ~WS_BORDER) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		FillCheckBoxTree(GetDlgItem(hwndDlg, IDC_POPLIST), statusValues, sizeof(statusValues) / sizeof(statusValues[0]),
			db_get_dw(0, SRMM_MODULE, SRMSGSET_POPFLAGS, SRMSGDEFSET_POPFLAGS));
		CheckDlgButton(hwndDlg, IDC_AUTOPOPUP, db_get_b(0, SRMM_MODULE, SRMSGSET_AUTOPOPUP, SRMSGDEFSET_AUTOPOPUP) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_STAYMINIMIZED, db_get_b(0, SRMM_MODULE, SRMSGSET_STAYMINIMIZED, SRMSGDEFSET_STAYMINIMIZED) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AUTOMIN, db_get_b(0, SRMM_MODULE, SRMSGSET_AUTOMIN, SRMSGDEFSET_AUTOMIN) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SAVEDRAFTS, db_get_b(0, SRMM_MODULE, SRMSGSET_SAVEDRAFTS, SRMSGDEFSET_SAVEDRAFTS) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_DELTEMP, db_get_b(0, SRMM_MODULE, SRMSGSET_DELTEMP, SRMSGDEFSET_DELTEMP) ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_SECONDSSPIN, UDM_SETRANGE, 0, MAKELONG(60, 4));
		SendDlgItemMessage(hwndDlg, IDC_SECONDSSPIN, UDM_SETPOS, 0, db_get_dw(0, SRMM_MODULE, SRMSGSET_MSGTIMEOUT, SRMSGDEFSET_MSGTIMEOUT) / 1000);

		CheckDlgButton(hwndDlg, IDC_SAVEPERCONTACT, db_get_b(0, SRMM_MODULE, SRMSGSET_SAVEPERCONTACT, SRMSGDEFSET_SAVEPERCONTACT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CASCADE, db_get_b(0, SRMM_MODULE, SRMSGSET_CASCADE, SRMSGDEFSET_CASCADE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SENDONENTER, db_get_b(0, SRMM_MODULE, SRMSGSET_SENDONENTER, SRMSGDEFSET_SENDONENTER) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SENDONDBLENTER, db_get_b(0, SRMM_MODULE, SRMSGSET_SENDONDBLENTER, SRMSGDEFSET_SENDONDBLENTER) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_STATUSWIN, db_get_b(0, SRMM_MODULE, SRMSGSET_STATUSICON, SRMSGDEFSET_STATUSICON) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_HIDECONTAINERS, db_get_b(0, SRMM_MODULE, SRMSGSET_HIDECONTAINERS, SRMSGDEFSET_HIDECONTAINERS) ? BST_CHECKED : BST_UNCHECKED);

		EnableWindow(GetDlgItem(hwndDlg, IDC_STAYMINIMIZED), IsDlgButtonChecked(hwndDlg, IDC_AUTOPOPUP));
		EnableWindow(GetDlgItem(hwndDlg, IDC_POPLIST), IsDlgButtonChecked(hwndDlg, IDC_AUTOPOPUP));
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_AUTOPOPUP:
			EnableWindow(GetDlgItem(hwndDlg, IDC_STAYMINIMIZED), IsDlgButtonChecked(hwndDlg, IDC_AUTOPOPUP));
			EnableWindow(GetDlgItem(hwndDlg, IDC_POPLIST), IsDlgButtonChecked(hwndDlg, IDC_AUTOPOPUP));
			break;
		case IDC_AUTOMIN:
			break;
		case IDC_SECONDS:
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
				return 0;
			break;
		case IDC_SENDONENTER:
			CheckDlgButton(hwndDlg, IDC_SENDONDBLENTER, BST_UNCHECKED);
			break;
		case IDC_SENDONDBLENTER:
			CheckDlgButton(hwndDlg, IDC_SENDONENTER, BST_UNCHECKED);
			break;
		case IDC_CASCADE:
			CheckDlgButton(hwndDlg, IDC_SAVEPERCONTACT, BST_UNCHECKED);
			break;
		case IDC_SAVEPERCONTACT:
			CheckDlgButton(hwndDlg, IDC_CASCADE, BST_UNCHECKED);
			break;
		}
		MarkChanges(2, hwndDlg);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_POPLIST:
			if (((LPNMHDR)lParam)->code == NM_CLICK) {
				TVHITTESTINFO hti;
				hti.pt.x = (short)LOWORD(GetMessagePos());
				hti.pt.y = (short)HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
				if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti)) {
					if (hti.flags & TVHT_ONITEMSTATEICON) {
						TVITEM tvi;
						tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
						tvi.hItem = hti.hItem;
						TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);
						tvi.iImage = tvi.iSelectedImage = tvi.iImage == 1 ? 2 : 1;
						TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);
						MarkChanges(2, hwndDlg);
					}
				}
			}
			else if (((LPNMHDR)lParam)->code == TVN_KEYDOWN) {
				if (((LPNMTVKEYDOWN)lParam)->wVKey == VK_SPACE)
					MarkChanges(2, hwndDlg);
			}
			break;

		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				db_set_dw(0, SRMM_MODULE, SRMSGSET_POPFLAGS, MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg, IDC_POPLIST)));
				db_set_b(0, SRMM_MODULE, SRMSGSET_AUTOPOPUP, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTOPOPUP));
				db_set_b(0, SRMM_MODULE, SRMSGSET_STAYMINIMIZED, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_STAYMINIMIZED));
				db_set_b(0, SRMM_MODULE, SRMSGSET_AUTOMIN, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTOMIN));
				db_set_b(0, SRMM_MODULE, SRMSGSET_SAVEDRAFTS, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SAVEDRAFTS));

				db_set_b(0, SRMM_MODULE, SRMSGSET_DELTEMP, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DELTEMP));
				db_set_dw(0, SRMM_MODULE, SRMSGSET_MSGTIMEOUT, (DWORD)SendDlgItemMessage(hwndDlg, IDC_SECONDSSPIN, UDM_GETPOS, 0, 0) * 1000);

				db_set_b(0, SRMM_MODULE, SRMSGSET_SENDONENTER, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SENDONENTER));
				db_set_b(0, SRMM_MODULE, SRMSGSET_SENDONDBLENTER, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SENDONDBLENTER));
				db_set_b(0, SRMM_MODULE, SRMSGSET_SAVEPERCONTACT, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SAVEPERCONTACT));
				db_set_b(0, SRMM_MODULE, SRMSGSET_CASCADE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CASCADE));

				db_set_b(0, SRMM_MODULE, SRMSGSET_HIDECONTAINERS, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_HIDECONTAINERS));

				ApplyChanges(2);

				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

class CLogOptionsDlg : public CDlgBase
{
	CCtrlRichEdit m_log;

	void ShowPreview()
	{
		m_log.SetText(L"");

		struct GlobalMessageData gdat = { 0 };
		gdat.flags |= IsDlgButtonChecked(m_hwnd, IDC_SHOWLOGICONS) ? SMF_SHOWICONS : 0;
		gdat.flags |= IsDlgButtonChecked(m_hwnd, IDC_SHOWNAMES) ? 0 : SMF_HIDENAMES;
		gdat.flags |= IsDlgButtonChecked(m_hwnd, IDC_SHOWTIMES) ? SMF_SHOWTIME : 0;
		gdat.flags |= IsDlgButtonChecked(m_hwnd, IDC_SHOWSECONDS) ? SMF_SHOWSECONDS : 0;
		gdat.flags |= IsDlgButtonChecked(m_hwnd, IDC_SHOWDATES) ? SMF_SHOWDATE : 0;
		gdat.flags |= IsDlgButtonChecked(m_hwnd, IDC_USELONGDATE) ? SMF_LONGDATE : 0;
		gdat.flags |= IsDlgButtonChecked(m_hwnd, IDC_USERELATIVEDATE) ? SMF_RELATIVEDATE : 0;
		gdat.flags |= IsDlgButtonChecked(m_hwnd, IDC_GROUPMESSAGES) ? SMF_GROUPMESSAGES : 0;
		gdat.flags |= IsDlgButtonChecked(m_hwnd, IDC_MARKFOLLOWUPS) ? SMF_MARKFOLLOWUPS : 0;
		gdat.flags |= IsDlgButtonChecked(m_hwnd, IDC_MESSAGEONNEWLINE) ? SMF_MSGONNEWLINE : 0;
		gdat.flags |= IsDlgButtonChecked(m_hwnd, IDC_DRAWLINES) ? SMF_DRAWLINES : 0;
		gdat.flags |= IsDlgButtonChecked(m_hwnd, IDC_INDENTTEXT) ? SMF_INDENTTEXT : 0;
		gdat.indentSize = (int)SendDlgItemMessage(m_hwnd, IDC_INDENTSPIN, UDM_GETPOS, 0, 0);

		PARAFORMAT2 pf2;
		pf2.cbSize = sizeof(pf2);
		pf2.dwMask = PFM_OFFSET;
		pf2.dxOffset = (gdat.flags & SMF_INDENTTEXT) ? gdat.indentSize * 1440 / g_dat.logPixelSX : 0;
		m_log.SendMsg(EM_SETPARAFORMAT, 0, (LPARAM)&pf2);

		StreamInTestEvents(m_log.GetHwnd(), &gdat);
	}

public:
	CLogOptionsDlg() :
		CDlgBase(g_hInst, IDD_OPT_MSGLOG),
		m_log(this, IDC_SRMM_LOG)
	{}

	virtual void OnInitDialog() override
	{
		switch (db_get_b(0, SRMM_MODULE, SRMSGSET_LOADHISTORY, SRMSGDEFSET_LOADHISTORY)) {
		case LOADHISTORY_UNREAD:
			CheckDlgButton(m_hwnd, IDC_LOADUNREAD, BST_CHECKED);
			break;
		case LOADHISTORY_COUNT:
			CheckDlgButton(m_hwnd, IDC_LOADCOUNT, BST_CHECKED);
			EnableWindow(GetDlgItem(m_hwnd, IDC_LOADCOUNTN), TRUE);
			EnableWindow(GetDlgItem(m_hwnd, IDC_LOADCOUNTSPIN), TRUE);
			EnableWindow(GetDlgItem(m_hwnd, IDC_LOADCOUNTTEXT2), TRUE);
			break;
		case LOADHISTORY_TIME:
			CheckDlgButton(m_hwnd, IDC_LOADTIME, BST_CHECKED);
			EnableWindow(GetDlgItem(m_hwnd, IDC_LOADTIMEN), TRUE);
			EnableWindow(GetDlgItem(m_hwnd, IDC_LOADTIMESPIN), TRUE);
			EnableWindow(GetDlgItem(m_hwnd, IDC_STMINSOLD), TRUE);
			break;
		}
		SendDlgItemMessage(m_hwnd, IDC_LOADCOUNTSPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
		SendDlgItemMessage(m_hwnd, IDC_LOADCOUNTSPIN, UDM_SETPOS, 0, db_get_w(0, SRMM_MODULE, SRMSGSET_LOADCOUNT, SRMSGDEFSET_LOADCOUNT));
		SendDlgItemMessage(m_hwnd, IDC_LOADTIMESPIN, UDM_SETRANGE, 0, MAKELONG(12 * 60, 0));
		SendDlgItemMessage(m_hwnd, IDC_LOADTIMESPIN, UDM_SETPOS, 0, db_get_w(0, SRMM_MODULE, SRMSGSET_LOADTIME, SRMSGDEFSET_LOADTIME));

		CheckDlgButton(m_hwnd, IDC_USEIEVIEW, db_get_b(0, SRMM_MODULE, SRMSGSET_USEIEVIEW, SRMSGDEFSET_USEIEVIEW) ? BST_CHECKED : BST_UNCHECKED);
		if (!g_dat.ieviewInstalled)
			EnableWindow(GetDlgItem(m_hwnd, IDC_USEIEVIEW), FALSE);

		CheckDlgButton(m_hwnd, IDC_SHOWLOGICONS, db_get_b(0, SRMM_MODULE, SRMSGSET_SHOWLOGICONS, SRMSGDEFSET_SHOWLOGICONS) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SHOWNAMES, !db_get_b(0, SRMM_MODULE, SRMSGSET_HIDENAMES, SRMSGDEFSET_HIDENAMES) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(m_hwnd, IDC_SHOWTIMES, db_get_b(0, SRMM_MODULE, SRMSGSET_SHOWTIME, SRMSGDEFSET_SHOWTIME) ? BST_CHECKED : BST_UNCHECKED);
		BOOL bChecked = IsDlgButtonChecked(m_hwnd, IDC_SHOWTIMES);
		EnableWindow(GetDlgItem(m_hwnd, IDC_SHOWSECONDS), bChecked);
		EnableWindow(GetDlgItem(m_hwnd, IDC_SHOWDATES), bChecked);
		CheckDlgButton(m_hwnd, IDC_SHOWSECONDS, db_get_b(0, SRMM_MODULE, SRMSGSET_SHOWSECONDS, SRMSGDEFSET_SHOWSECONDS) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SHOWDATES, db_get_b(0, SRMM_MODULE, SRMSGSET_SHOWDATE, SRMSGDEFSET_SHOWDATE) ? BST_CHECKED : BST_UNCHECKED);
		bChecked = IsDlgButtonChecked(m_hwnd, IDC_SHOWDATES) && IsDlgButtonChecked(m_hwnd, IDC_SHOWTIMES);
		EnableWindow(GetDlgItem(m_hwnd, IDC_USELONGDATE), bChecked);
		EnableWindow(GetDlgItem(m_hwnd, IDC_USERELATIVEDATE), bChecked);
		CheckDlgButton(m_hwnd, IDC_USELONGDATE, db_get_b(0, SRMM_MODULE, SRMSGSET_USELONGDATE, SRMSGDEFSET_USELONGDATE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_USERELATIVEDATE, db_get_b(0, SRMM_MODULE, SRMSGSET_USERELATIVEDATE, SRMSGDEFSET_USERELATIVEDATE) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(m_hwnd, IDC_GROUPMESSAGES, db_get_b(0, SRMM_MODULE, SRMSGSET_GROUPMESSAGES, SRMSGDEFSET_GROUPMESSAGES) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_MARKFOLLOWUPS, db_get_b(0, SRMM_MODULE, SRMSGSET_MARKFOLLOWUPS, SRMSGDEFSET_MARKFOLLOWUPS) ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(m_hwnd, IDC_MARKFOLLOWUPS), IsDlgButtonChecked(m_hwnd, IDC_GROUPMESSAGES));

		CheckDlgButton(m_hwnd, IDC_MESSAGEONNEWLINE, db_get_b(0, SRMM_MODULE, SRMSGSET_MESSAGEONNEWLINE, SRMSGDEFSET_MESSAGEONNEWLINE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_DRAWLINES, db_get_b(0, SRMM_MODULE, SRMSGSET_DRAWLINES, SRMSGDEFSET_DRAWLINES) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(m_hwnd, IDC_INDENTTEXT, db_get_b(0, SRMM_MODULE, SRMSGSET_INDENTTEXT, SRMSGDEFSET_INDENTTEXT) ? BST_CHECKED : BST_UNCHECKED);
		bChecked = IsDlgButtonChecked(m_hwnd, IDC_INDENTTEXT);
		EnableWindow(GetDlgItem(m_hwnd, IDC_INDENTSIZE), bChecked);
		EnableWindow(GetDlgItem(m_hwnd, IDC_INDENTSPIN), bChecked);
		SendDlgItemMessage(m_hwnd, IDC_INDENTSPIN, UDM_SETRANGE, 0, MAKELONG(999, 0));
		SendDlgItemMessage(m_hwnd, IDC_INDENTSPIN, UDM_SETPOS, 0, db_get_w(0, SRMM_MODULE, SRMSGSET_INDENTSIZE, SRMSGDEFSET_INDENTSIZE));

		PARAFORMAT2 pf2;
		memset(&pf2, 0, sizeof(pf2));
		pf2.cbSize = sizeof(pf2);
		pf2.dwMask = PFM_OFFSETINDENT | PFM_RIGHTINDENT;
		pf2.dxStartIndent = 30;
		pf2.dxRightIndent = 30;
		m_log.SendMsg(EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
		m_log.SendMsg(EM_SETEDITSTYLE, SES_EXTENDBACKCOLOR, SES_EXTENDBACKCOLOR);
		m_log.SendMsg(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(0, 0));
		m_log.SendMsg(EM_AUTOURLDETECT, TRUE, 0);
		m_log.SetReadOnly(true);

		ShowPreview();
	}

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		BOOL bChecked;

		switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDC_FONTSCOLORS:
				Options_Open(L"Customize", L"Fonts and colors");
				break;

			case IDC_LOADUNREAD:
			case IDC_LOADCOUNT:
			case IDC_LOADTIME:
				bChecked = IsDlgButtonChecked(m_hwnd, IDC_LOADCOUNT);
				EnableWindow(GetDlgItem(m_hwnd, IDC_LOADCOUNTN), bChecked);
				EnableWindow(GetDlgItem(m_hwnd, IDC_LOADCOUNTSPIN), bChecked);
				EnableWindow(GetDlgItem(m_hwnd, IDC_LOADCOUNTTEXT2), bChecked);
				bChecked = IsDlgButtonChecked(m_hwnd, IDC_LOADTIME);
				EnableWindow(GetDlgItem(m_hwnd, IDC_LOADTIMEN), bChecked);
				EnableWindow(GetDlgItem(m_hwnd, IDC_LOADTIMESPIN), bChecked);
				EnableWindow(GetDlgItem(m_hwnd, IDC_STMINSOLD), bChecked);
				break;

			case IDC_SHOWTIMES:
				bChecked = IsDlgButtonChecked(m_hwnd, IDC_SHOWTIMES);
				EnableWindow(GetDlgItem(m_hwnd, IDC_SHOWSECONDS), bChecked);
				EnableWindow(GetDlgItem(m_hwnd, IDC_SHOWDATES), bChecked);

			case IDC_SHOWDATES:
				bChecked = IsDlgButtonChecked(m_hwnd, IDC_SHOWDATES) && IsDlgButtonChecked(m_hwnd, IDC_SHOWTIMES);
				EnableWindow(GetDlgItem(m_hwnd, IDC_USELONGDATE), bChecked);
				EnableWindow(GetDlgItem(m_hwnd, IDC_USERELATIVEDATE), bChecked);

			case IDC_SHOWNAMES:
			case IDC_SHOWSECONDS:
			case IDC_USELONGDATE:
			case IDC_USERELATIVEDATE:
			case IDC_MARKFOLLOWUPS:
			case IDC_SHOWLOGICONS:
			case IDC_MESSAGEONNEWLINE:
			case IDC_DRAWLINES:
				ShowPreview();
				break;

			case IDC_GROUPMESSAGES:
				EnableWindow(GetDlgItem(m_hwnd, IDC_MARKFOLLOWUPS), IsDlgButtonChecked(m_hwnd, IDC_GROUPMESSAGES));
				ShowPreview();
				break;

			case IDC_INDENTTEXT:
				EnableWindow(GetDlgItem(m_hwnd, IDC_INDENTSIZE), IsDlgButtonChecked(m_hwnd, IDC_INDENTTEXT));
				EnableWindow(GetDlgItem(m_hwnd, IDC_INDENTSPIN), IsDlgButtonChecked(m_hwnd, IDC_INDENTTEXT));
				ShowPreview();
				break;

			case IDC_INDENTSIZE:
				if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
					return TRUE;
				ShowPreview();
				break;

			case IDC_LOADCOUNTN:
			case IDC_LOADTIMEN:
				if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
					return TRUE;
				break;

			case IDC_SRMM_LOG:
				return 0;
			}
			MarkChanges(4, m_hwnd);
			break;

		case WM_NOTIFY:
			switch (((LPNMHDR)lParam)->idFrom) {
			case 0:
				switch (((LPNMHDR)lParam)->code) {
				case PSN_APPLY:
					if (IsDlgButtonChecked(m_hwnd, IDC_LOADCOUNT))
						db_set_b(0, SRMM_MODULE, SRMSGSET_LOADHISTORY, LOADHISTORY_COUNT);
					else if (IsDlgButtonChecked(m_hwnd, IDC_LOADTIME))
						db_set_b(0, SRMM_MODULE, SRMSGSET_LOADHISTORY, LOADHISTORY_TIME);
					else
						db_set_b(0, SRMM_MODULE, SRMSGSET_LOADHISTORY, LOADHISTORY_UNREAD);
					db_set_w(0, SRMM_MODULE, SRMSGSET_LOADCOUNT, (WORD)SendDlgItemMessage(m_hwnd, IDC_LOADCOUNTSPIN, UDM_GETPOS, 0, 0));
					db_set_w(0, SRMM_MODULE, SRMSGSET_LOADTIME, (WORD)SendDlgItemMessage(m_hwnd, IDC_LOADTIMESPIN, UDM_GETPOS, 0, 0));
					db_set_b(0, SRMM_MODULE, SRMSGSET_SHOWLOGICONS, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_SHOWLOGICONS));
					db_set_b(0, SRMM_MODULE, SRMSGSET_HIDENAMES, (BYTE)BST_UNCHECKED == IsDlgButtonChecked(m_hwnd, IDC_SHOWNAMES));
					db_set_b(0, SRMM_MODULE, SRMSGSET_SHOWTIME, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_SHOWTIMES));
					db_set_b(0, SRMM_MODULE, SRMSGSET_SHOWSECONDS, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_SHOWSECONDS));
					db_set_b(0, SRMM_MODULE, SRMSGSET_SHOWDATE, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_SHOWDATES));
					db_set_b(0, SRMM_MODULE, SRMSGSET_USELONGDATE, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_USELONGDATE));
					db_set_b(0, SRMM_MODULE, SRMSGSET_USERELATIVEDATE, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_USERELATIVEDATE));
					db_set_b(0, SRMM_MODULE, SRMSGSET_GROUPMESSAGES, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_GROUPMESSAGES));
					db_set_b(0, SRMM_MODULE, SRMSGSET_MARKFOLLOWUPS, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_MARKFOLLOWUPS));
					db_set_b(0, SRMM_MODULE, SRMSGSET_MESSAGEONNEWLINE, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_MESSAGEONNEWLINE));
					db_set_b(0, SRMM_MODULE, SRMSGSET_DRAWLINES, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_DRAWLINES));
					db_set_b(0, SRMM_MODULE, SRMSGSET_USEIEVIEW, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_USEIEVIEW));
					db_set_b(0, SRMM_MODULE, SRMSGSET_INDENTTEXT, (BYTE)IsDlgButtonChecked(m_hwnd, IDC_INDENTTEXT));
					db_set_w(0, SRMM_MODULE, SRMSGSET_INDENTSIZE, (WORD)SendDlgItemMessage(m_hwnd, IDC_INDENTSPIN, UDM_GETPOS, 0, 0));

					FreeMsgLogIcons();
					LoadMsgLogIcons();
					ApplyChanges(4);
					return TRUE;
				}
			}
			break;
		}
		return CDlgBase::DlgProc(msg, wParam, lParam);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

static void ResetCList(HWND hwndDlg)
{
	if (!db_get_b(0, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT))
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETUSEGROUPS, FALSE, 0);
	else
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETUSEGROUPS, TRUE, 0);
	SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETHIDEEMPTYGROUPS, 1, 0);
}

static void RebuildList(HWND hwndDlg, HANDLE hItemNew, HANDLE hItemUnknown)
{
	BYTE defType = db_get_b(0, SRMM_MODULE, SRMSGSET_TYPINGNEW, SRMSGDEFSET_TYPINGNEW);
	if (hItemNew && defType)
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItemNew, 1);

	if (hItemUnknown && db_get_b(0, SRMM_MODULE, SRMSGSET_TYPINGUNKNOWN, SRMSGDEFSET_TYPINGUNKNOWN))
		SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItemUnknown, 1);

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
		if (hItem && db_get_b(hContact, SRMM_MODULE, SRMSGSET_TYPING, defType))
			SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETCHECKMARK, (WPARAM)hItem, 1);
	}
}

static void SaveList(HWND hwndDlg, HANDLE hItemNew, HANDLE hItemUnknown)
{
	if (hItemNew)
		db_set_b(0, SRMM_MODULE, SRMSGSET_TYPINGNEW, (BYTE)(SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItemNew, 0) ? 1 : 0));

	if (hItemUnknown)
		db_set_b(0, SRMM_MODULE, SRMSGSET_TYPINGUNKNOWN, (BYTE)(SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItemUnknown, 0) ? 1 : 0));

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
		if (hItem)
			db_set_b(hContact, SRMM_MODULE, SRMSGSET_TYPING, (BYTE)(SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_GETCHECKMARK, (WPARAM)hItem, 0) ? 1 : 0));
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
			cii.pszText = (wchar_t*)TranslateT("** New contacts **");
			hItemNew = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
			cii.pszText = (wchar_t*)TranslateT("** Unknown contacts **");
			hItemUnknown = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
		}

		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CLIST), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CLIST), GWL_STYLE) | CLS_SHOWHIDDEN | CLS_NOHIDEOFFLINE);
		ResetCList(hwndDlg);

		CheckDlgButton(hwndDlg, IDC_SHOWNOTIFY, db_get_b(0, SRMM_MODULE, SRMSGSET_SHOWTYPING, SRMSGDEFSET_SHOWTYPING) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_TYPEWIN, db_get_b(0, SRMM_MODULE, SRMSGSET_SHOWTYPINGWIN, SRMSGDEFSET_SHOWTYPINGWIN) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_TYPETRAY, db_get_b(0, SRMM_MODULE, SRMSGSET_SHOWTYPINGNOWIN, SRMSGDEFSET_SHOWTYPINGNOWIN) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_NOTIFYTRAY, db_get_b(0, SRMM_MODULE, SRMSGSET_SHOWTYPINGCLIST, SRMSGDEFSET_SHOWTYPINGCLIST) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_NOTIFYBALLOON, !db_get_b(0, SRMM_MODULE, SRMSGSET_SHOWTYPINGCLIST, SRMSGDEFSET_SHOWTYPINGCLIST) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_TYPINGSWITCH, db_get_b(0, SRMM_MODULE, SRMSGSET_SHOWTYPINGSWITCH, SRMSGDEFSET_SHOWTYPINGSWITCH) ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_TYPEWIN), IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY));
		EnableWindow(GetDlgItem(hwndDlg, IDC_TYPETRAY), IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY));
		EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYTRAY), IsDlgButtonChecked(hwndDlg, IDC_TYPETRAY));
		EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYBALLOON), IsDlgButtonChecked(hwndDlg, IDC_TYPETRAY));
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_TYPETRAY:
			if (IsDlgButtonChecked(hwndDlg, IDC_TYPETRAY)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYTRAY), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYBALLOON), TRUE);
			}
			else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYTRAY), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYBALLOON), FALSE);
			}
			MarkChanges(4, hwndDlg);
			break;

		case IDC_SHOWNOTIFY:
			EnableWindow(GetDlgItem(hwndDlg, IDC_TYPEWIN), IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY));
			EnableWindow(GetDlgItem(hwndDlg, IDC_TYPETRAY), IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY));
			EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYTRAY), IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY));
			EnableWindow(GetDlgItem(hwndDlg, IDC_NOTIFYBALLOON), IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY));

		// fall-thru
		case IDC_TYPEWIN:
		case IDC_NOTIFYTRAY:
		case IDC_NOTIFYBALLOON:
		case IDC_TYPINGSWITCH:
			MarkChanges(4, hwndDlg);
			break;
		}
		break;
	
	case WM_NOTIFY:
		switch (((NMHDR *)lParam)->idFrom) {
		case IDC_CLIST:
			switch (((NMHDR *)lParam)->code) {
			case CLN_OPTIONSCHANGED:
				ResetCList(hwndDlg);
				break;
			case CLN_CHECKCHANGED:
				MarkChanges(4, hwndDlg);
				break;
			case CLN_LISTREBUILT:
				RebuildList(hwndDlg, hItemNew, hItemUnknown);
				break;
			}
			break;
		
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				SaveList(hwndDlg, hItemNew, hItemUnknown);
				db_set_b(0, SRMM_MODULE, SRMSGSET_SHOWTYPING, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWNOTIFY));
				db_set_b(0, SRMM_MODULE, SRMSGSET_SHOWTYPINGWIN, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_TYPEWIN));
				db_set_b(0, SRMM_MODULE, SRMSGSET_SHOWTYPINGNOWIN, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_TYPETRAY));
				db_set_b(0, SRMM_MODULE, SRMSGSET_SHOWTYPINGCLIST, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_NOTIFYTRAY));
				db_set_b(0, SRMM_MODULE, SRMSGSET_SHOWTYPINGSWITCH, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_TYPINGSWITCH));
				ReloadGlobals();
				Srmm_Broadcast(DM_OPTIONSAPPLIED, 0, 0);
			}
			break;
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

int OptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 910000000;
	odp.hInstance = g_hInst;
	odp.szTitle.a = LPGEN("Message sessions");
	odp.flags = ODPF_BOLDGROUPS;

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MSGDLG);
	odp.pfnDlgProc = DlgProcOptions;
	odp.szTab.a = LPGEN("General");
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MSGTABS);
	odp.pfnDlgProc = DlgProcTabsOptions;
	odp.szTab.a = LPGEN("Tabs");
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_LAYOUT);
	odp.pfnDlgProc = DlgProcLayoutOptions;
	odp.szTab.a = LPGEN("Layout");
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = nullptr;
	odp.pfnDlgProc = nullptr;
	odp.pDialog = new CLogOptionsDlg();
	odp.szTab.a = LPGEN("Event log");
	Options_AddPage(wParam, &odp);

	odp.pDialog = nullptr;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS1);
	odp.pfnDlgProc = DlgProcOptions1;
	odp.szTab.a = LPGEN("Group chat");
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS2);
	odp.pfnDlgProc = DlgProcOptions2;
	odp.szTab.a = LPGEN("Group chat log");
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MSGTYPE);
	odp.szGroup.a = LPGEN("Message sessions");
	odp.szTitle.a = LPGEN("Typing notify");
	odp.pfnDlgProc = DlgProcTypeOptions;
	odp.szTab.a = nullptr;
	Options_AddPage(wParam, &odp);

	if (g_dat.popupInstalled) {
		odp.position = 910000002;
		odp.hInstance = g_hInst;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONSPOPUP);
		odp.szTitle.a = LPGEN("Messaging");
		odp.szGroup.a = LPGEN("Popups");
		odp.pfnDlgProc = DlgProcOptionsPopup;
		odp.flags = ODPF_BOLDGROUPS;
		Options_AddPage(wParam, &odp);
	}
	return 0;
}
