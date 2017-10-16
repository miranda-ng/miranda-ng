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
// group chat options and generic font handling

#include "stdafx.h"

#define FONTF_BOLD   1
#define FONTF_ITALIC 2

struct FontOptionsList
{
	wchar_t *szDescr;
	COLORREF defColour;
	wchar_t *szDefFace;
	BYTE     defCharset, defStyle;
	char     defSize;
	COLORREF colour;
	wchar_t  szFace[LF_FACESIZE];
	BYTE     charset, style;
	char     size;
};

struct ColorOptionsList
{
	int		order;
	wchar_t *tszGroup;
	wchar_t *tszName;
	char    *szSetting;
	COLORREF def;
};

/*
* note: bits 24-31 in default color indicates that color is a system color index
* (GetSysColor(default_color & 0x00ffffff)), not a rgb value.
*/
static ColorOptionsList _clrs[] = {
	{ 0, LPGENW("Message Sessions"), LPGENW("Input area background"), "inputbg", SRMSGDEFSET_BKGCOLOUR },
	{ 1, LPGENW("Message Sessions"), LPGENW("Log background"), SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR },
	{ 0, LPGENW("Message Sessions") L"/" LPGENW("Single Messaging"), LPGENW("Outgoing background"), "outbg", SRMSGDEFSET_BKGOUTCOLOUR },
	{ 1, LPGENW("Message Sessions") L"/" LPGENW("Single Messaging"), LPGENW("Incoming background"), "inbg", SRMSGDEFSET_BKGINCOLOUR },
	{ 2, LPGENW("Message Sessions") L"/" LPGENW("Single Messaging"), LPGENW("Status background"), "statbg", SRMSGDEFSET_BKGCOLOUR },
	{ 3, LPGENW("Message Sessions") L"/" LPGENW("Single Messaging"), LPGENW("Incoming background(old)"), "oldinbg", SRMSGDEFSET_BKGINCOLOUR },
	{ 4, LPGENW("Message Sessions") L"/" LPGENW("Single Messaging"), LPGENW("Outgoing background(old)"), "oldoutbg", SRMSGDEFSET_BKGOUTCOLOUR },
	{ 5, LPGENW("Message Sessions") L"/" LPGENW("Single Messaging"), LPGENW("Horizontal Grid Lines"), "hgrid", RGB(224, 224, 224) },
	{ 0, LPGENW("Message Sessions") L"/" LPGENW("Info Panel"), LPGENW("Panel background low"), "ipfieldsbg", 0x62caff },
	{ 1, LPGENW("Message Sessions") L"/" LPGENW("Info Panel"), LPGENW("Panel background high"), "ipfieldsbgHigh", 0xf0f0f0 },
	{ 0, LPGENW("Message Sessions") L"/" LPGENW("Common colors"), LPGENW("Toolbar background high"), "tbBgHigh", 0 },
	{ 1, LPGENW("Message Sessions") L"/" LPGENW("Common colors"), LPGENW("Toolbar background low"), "tbBgLow", 0 },
	{ 2, LPGENW("Message Sessions") L"/" LPGENW("Common colors"), LPGENW("Window fill color"), "fillColor", 0 },
	{ 3, LPGENW("Message Sessions") L"/" LPGENW("Common colors"), LPGENW("Text area borders"), "cRichBorders", 0 },
	{ 4, LPGENW("Message Sessions") L"/" LPGENW("Common colors"), LPGENW("Aero glow effect"), "aeroGlow", RGB(40, 40, 255) },
	{ 4, LPGENW("Message Sessions") L"/" LPGENW("Common colors"), LPGENW("Generic text color (only when fill color is set)"), "genericTxtClr", 0xff000000 | COLOR_BTNTEXT },
};

static ColorOptionsList _tabclrs[] = {
	{ 0, LPGENW("Message Sessions") L"/" LPGENW("Tabs"), LPGENW("Normal text"), "tab_txt_normal", 0xff000000 | COLOR_BTNTEXT },
	{ 1, LPGENW("Message Sessions") L"/" LPGENW("Tabs"), LPGENW("Active text"), "tab_txt_active", 0xff000000 | COLOR_BTNTEXT },
	{ 2, LPGENW("Message Sessions") L"/" LPGENW("Tabs"), LPGENW("Hovered text"), "tab_txt_hottrack", 0xff000000 | COLOR_HOTLIGHT },
	{ 3, LPGENW("Message Sessions") L"/" LPGENW("Tabs"), LPGENW("Unread text"), "tab_txt_unread", 0xff000000 | COLOR_HOTLIGHT },

	{ 4, LPGENW("Message Sessions") L"/" LPGENW("Tabs"), LPGENW("Normal background"), "tab_bg_normal", 0xff000000 | COLOR_3DFACE },
	{ 5, LPGENW("Message Sessions") L"/" LPGENW("Tabs"), LPGENW("Active background"), "tab_bg_active", 0xff000000 | COLOR_3DFACE },
	{ 6, LPGENW("Message Sessions") L"/" LPGENW("Tabs"), LPGENW("Hovered background"), "tab_bg_hottrack", 0xff000000 | COLOR_3DFACE },
	{ 7, LPGENW("Message Sessions") L"/" LPGENW("Tabs"), LPGENW("Unread background"), "tab_bg_unread", 0xff000000 | COLOR_3DFACE }
};

extern LOGFONT lfDefault;

static FontOptionsList IM_fontOptionsList[] = {
	{ LPGENW(">> Outgoing messages"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW(">> Outgoing misc events"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("<< Incoming messages"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("<< Incoming misc events"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW(">> Outgoing name"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENW(">> Outgoing timestamp"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENW("<< Incoming name"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENW("<< Incoming timestamp"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENW(">> Outgoing messages (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW(">> Outgoing misc events (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("<< Incoming messages (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("<< Incoming misc events (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW(">> Outgoing name (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENW(">> Outgoing timestamp (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENW("<< Incoming name (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENW("<< Incoming timestamp (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENW("* Message Input Area"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("* Status changes"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("* Dividers"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("* Error and warning messages"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("* Symbols (incoming)"), RGB(50, 50, 50), L"Webdings", SYMBOL_CHARSET, 0, -12 },
	{ LPGENW("* Symbols (outgoing)"), RGB(50, 50, 50), L"Webdings", SYMBOL_CHARSET, 0, -12 },
};

static FontOptionsList IP_fontOptionsList[] = {
	{ LPGENW("Nickname"), RGB(0, 0, 0), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("UIN"), RGB(0, 0, 0), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("Status"), RGB(0, 0, 0), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("Protocol"), RGB(0, 0, 0), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("Contacts local time"), RGB(0, 0, 0), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENW("Window caption (skinned mode)"), RGB(255, 255, 255), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
};

static FontOptionsList *fontOptionsList = IM_fontOptionsList;
static int fontCount = MSGDLGFONTCOUNT;

struct branch_t
{
	wchar_t*    szDescr;
	char*     szDBName;
	int       iMode;
	BYTE      bDefault;
	HTREEITEM hItem;
};
static branch_t branch1[] = {
	{ LPGENW("Open new chat rooms in the default container"), "DefaultContainer", 0, 1, nullptr },
	{ LPGENW("Flash window when someone speaks"), "FlashWindow", 0, 0, nullptr },
	{ LPGENW("Flash window when a word is highlighted"), "FlashWindowHighlight", 0, 1, nullptr },
	{ LPGENW("Create tabs or windows for highlight events"), "CreateWindowOnHighlight", 0, 0, nullptr },
	{ LPGENW("Activate chat window on highlight"), "AnnoyingHighlight", 0, 0, nullptr },
	{ LPGENW("Show list of users in the chat room"), "ShowNicklist", 0, 1, nullptr },
	{ LPGENW("Colorize nicknames in member list"), "ColorizeNicks", 0, 1, nullptr },
	{ LPGENW("Show button menus when right clicking the buttons"), "RightClickFilter", 0, 1, nullptr },
	{ LPGENW("Show topic as status message on the contact list"), "TopicOnClist", 0, 1, nullptr },
	{ LPGENW("Do not pop up the window when joining a chat room"), "PopupOnJoin", 0, 0, nullptr },
	{ LPGENW("Hide or show the window by double click in the contact list"), "ToggleVisibility", 0, 0, nullptr },
	{ LPGENW("Sync splitter position with standard IM sessions"), "SyncSplitter", 0, 0, nullptr },
	{ LPGENW("Show contact's status modes if supported by the protocol"), "ShowContactStatus", 0, 1, nullptr },
	{ LPGENW("Display contact's status icon before user role icon"), "ContactStatusFirst", 0, 0, nullptr },
	{ LPGENW("Use IRC style status indicators in the nick list"), "ClassicIndicators", 0, 0, nullptr },
	{ LPGENW("Use alternative sorting method in member list"), "AlternativeSorting", 0, 1, nullptr }
};

static branch_t branch2[] = {
	{ LPGENW("Prefix all events with a timestamp"), "ShowTimeStamp", 0, 1, nullptr },
	{ LPGENW("Timestamp only when event time differs"), "ShowTimeStampIfChanged", 0, 0, nullptr },
	{ LPGENW("Timestamp has same color as the event"), "TimeStampEventColour", 0, 0, nullptr },
	{ LPGENW("Indent the second line of a message"), "LogIndentEnabled", 0, 1, nullptr },
	{ LPGENW("Limit user names in the message log to 20 characters"), "LogLimitNames", 0, 1, nullptr },
	{ LPGENW("Add a colon (:) to auto-completed user names"), "AddColonToAutoComplete", 0, 1, nullptr },
	{ LPGENW("Add a comma instead of a colon to auto-completed user names"), "UseCommaAsColon", 0, 0, nullptr },
	{ LPGENW("Start private conversation on double click in nick list (insert nick if unchecked)"), "DoubleClick4Privat", 0, 0, nullptr },
	{ LPGENW("Strip colors from messages in the log"), "StripFormatting", 0, 0, nullptr },
	{ LPGENW("Enable the 'event filter' for new rooms"), "FilterEnabled", 0, 0, nullptr },
	{ LPGENW("Use IRC style status indicators in the log"), "LogClassicIndicators", 0, 0, nullptr },
	{ LPGENW("Allow clickable user names in the message log"), "ClickableNicks", 0, 1, nullptr },
	{ LPGENW("Add new line after names"), "NewlineAfterNames", 0, 0, nullptr },
	{ LPGENW("Colorize user names in message log"), "ColorizeNicksInLog", 0, 1, nullptr },
	{ LPGENW("Scale down icons to 10x10 pixels in the chat log"), "ScaleIcons", 0, 1, nullptr }
};

static HWND hPathTip = 0;

void LoadMsgDlgFont(int section, int i, LOGFONT *lf, COLORREF* colour, char *szMod)
{
	char str[32];
	int db_idx = (section == FONTSECTION_IM) ? i : i + 100;

	FontOptionsList *fol = fontOptionsList;
	switch (section) {
	case FONTSECTION_IM: fol = IM_fontOptionsList; break;
	case FONTSECTION_IP: fol = IP_fontOptionsList; break;
	}

	if (colour) {
		mir_snprintf(str, "Font%dCol", db_idx);
		*colour = M.GetDword(szMod, str, fol[i].defColour);
	}

	if (lf) {
		mir_snprintf(str, "Font%dSize", db_idx);
		lf->lfHeight = (char)M.GetByte(szMod, str, fol[i].defSize);
		lf->lfWidth = 0;
		lf->lfEscapement = 0;
		lf->lfOrientation = 0;
		mir_snprintf(str, "Font%dSty", db_idx);
		int style = M.GetByte(szMod, str, fol[i].defStyle);
		if (i == MSGFONTID_MESSAGEAREA && section == FONTSECTION_IM && M.GetByte("inputFontFix", 1) == 1) {
			lf->lfWeight = FW_NORMAL;
			lf->lfItalic = 0;
			lf->lfUnderline = 0;
			lf->lfStrikeOut = 0;
		}
		else {
			lf->lfWeight = style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
			lf->lfItalic = style & FONTF_ITALIC ? 1 : 0;
			lf->lfUnderline = style & FONTF_UNDERLINE ? 1 : 0;
			lf->lfStrikeOut = style & FONTF_STRIKEOUT ? 1 : 0;
		}
		mir_snprintf(str, "Font%dSet", db_idx);
		lf->lfCharSet = M.GetByte(szMod, str, fol[i].defCharset);
		lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf->lfQuality = DEFAULT_QUALITY;
		lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		mir_snprintf(str, "Font%d", db_idx);
		if ((i == 17 && !mir_strcmp(szMod, CHATFONT_MODULE)) || ((i == 20 || i == 21) && !mir_strcmp(szMod, FONTMODULE))) {
			lf->lfCharSet = SYMBOL_CHARSET;
			wcsncpy_s(lf->lfFaceName, L"Webdings", _TRUNCATE);
		}
		else {
			ptrW tszDefFace(db_get_wsa(0, szMod, str));
			if (tszDefFace == nullptr)
				wcsncpy_s(lf->lfFaceName, fol[i].szDefFace, _TRUNCATE);
			else
				wcsncpy_s(lf->lfFaceName, tszDefFace, _TRUNCATE);
		}
	}
}

static HTREEITEM InsertBranch(HWND hwndTree, wchar_t* pszDescr, BOOL bExpanded)
{
	TVINSERTSTRUCT tvis;
	tvis.hParent = nullptr;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvis.item.pszText = TranslateW(pszDescr);
	tvis.item.stateMask = TVIS_EXPANDED | TVIS_BOLD;
	tvis.item.state = (bExpanded ? TVIS_EXPANDED : 0) | TVIS_BOLD;
	tvis.item.iImage = tvis.item.iSelectedImage = (bExpanded ? IMG_GRPOPEN : IMG_GRPCLOSED);
	return TreeView_InsertItem(hwndTree, &tvis);
}

static void FillBranch(HWND hwndTree, HTREEITEM hParent, branch_t *branch, int nValues, DWORD defaultval)
{
	if (hParent == 0)
		return;

	TVINSERTSTRUCT tvis = { 0 };

	for (int i = 0; i < nValues; i++) {
		tvis.hParent = hParent;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvis.item.pszText = TranslateW(branch[i].szDescr);
		if (branch[i].iMode)
			tvis.item.iImage = tvis.item.iSelectedImage = ((((M.GetDword(CHAT_MODULE, branch[i].szDBName, defaultval) & branch[i].iMode) & branch[i].iMode) != 0) ? IMG_CHECK : IMG_NOCHECK);
		else
			tvis.item.iImage = tvis.item.iSelectedImage = ((M.GetByte(CHAT_MODULE, branch[i].szDBName, branch[i].bDefault) != 0) ? IMG_CHECK : IMG_NOCHECK);
		branch[i].hItem = TreeView_InsertItem(hwndTree, &tvis);
	}
}

static void SaveBranch(HWND hwndTree, branch_t *branch, int nValues)
{
	TVITEM tvi = { 0 };
	BYTE bChecked;
	DWORD iState = 0;

	for (int i = 0; i < nValues; i++) {
		tvi.mask = TVIF_HANDLE | TVIF_IMAGE;
		tvi.hItem = branch[i].hItem;
		TreeView_GetItem(hwndTree, &tvi);
		bChecked = ((tvi.iImage == IMG_CHECK) ? 1 : 0);
		if (branch[i].iMode) {
			if (bChecked)
				iState |= branch[i].iMode;
			if (iState & GC_EVENT_ADDSTATUS)
				iState |= GC_EVENT_REMOVESTATUS;
			db_set_dw(0, CHAT_MODULE, branch[i].szDBName, iState);
		}
		else db_set_b(0, CHAT_MODULE, branch[i].szDBName, bChecked);
	}
}

static INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM)
{
	char szDir[MAX_PATH];
	switch (uMsg) {
	case BFFM_INITIALIZED:
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)M.getUserDir());
		break;

	case BFFM_SELCHANGED:
		if (SHGetPathFromIDListA((LPITEMIDLIST)lp, szDir))
			SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)szDir);
		break;
	}
	return 0;
}

static IconItem _icons[] =
{
	{ LPGEN("Window Icon"), "chat_window", IDI_CHANMGR },
	{ LPGEN("Icon overlay"), "chat_overlay", IDI_OVERLAY },

	{ LPGEN("Status 1 (10x10)"), "chat_status0", IDI_STATUS0 },
	{ LPGEN("Status 2 (10x10)"), "chat_status1", IDI_STATUS1 },
	{ LPGEN("Status 3 (10x10)"), "chat_status2", IDI_STATUS2 },
	{ LPGEN("Status 4 (10x10)"), "chat_status3", IDI_STATUS3 },
	{ LPGEN("Status 5 (10x10)"), "chat_status4", IDI_STATUS4 },
	{ LPGEN("Status 6 (10x10)"), "chat_status5", IDI_STATUS5 }
};

static IconItem _logicons[] =
{
	{ LPGEN("Message in (10x10)"), "chat_log_message_in", IDI_MESSAGE },
	{ LPGEN("Message out (10x10)"), "chat_log_message_out", IDI_MESSAGEOUT },
	{ LPGEN("Action (10x10)"), "chat_log_action", IDI_ACTION },
	{ LPGEN("Add Status (10x10)"), "chat_log_addstatus", IDI_ADDSTATUS },
	{ LPGEN("Remove Status (10x10)"), "chat_log_removestatus", IDI_REMSTATUS },
	{ LPGEN("Join (10x10)"), "chat_log_join", IDI_JOIN },
	{ LPGEN("Leave (10x10)"), "chat_log_part", IDI_PART },
	{ LPGEN("Quit (10x10)"), "chat_log_quit", IDI_QUIT },
	{ LPGEN("Kick (10x10)"), "chat_log_kick", IDI_KICK },
	{ LPGEN("Notice (10x10)"), "chat_log_notice", IDI_NOTICE },
	{ LPGEN("Nickchange (10x10)"), "chat_log_nick", IDI_NICK },
	{ LPGEN("Topic (10x10)"), "chat_log_topic", IDI_TOPIC },
	{ LPGEN("Highlight (10x10)"), "chat_log_highlight", IDI_HIGHLIGHT },
	{ LPGEN("Information (10x10)"), "chat_log_info", IDI_INFO }
};

// add icons to the skinning module
void Chat_AddIcons(void)
{
	Icon_Register(g_hIconDLL, LPGEN("Message Sessions") "/" LPGEN("Group chat windows"), _icons, _countof(_icons));
	Icon_Register(g_hIconDLL, LPGEN("Message Sessions") "/" LPGEN("Group chat log"), _logicons, _countof(_logicons));
	pci->MM_IconsChanged();
}

/*
 * get icon by name from the core icon library service
 */
HICON LoadIconEx(char *pszIcoLibName)
{
	char szTemp[256];
	mir_snprintf(szTemp, "chat_%s", pszIcoLibName);
	return IcoLib_GetIcon(szTemp);
}

static void InitSetting(wchar_t* &ppPointer, const char *pszSetting, const wchar_t *pszDefault)
{
	ptrW val(db_get_wsa(0, CHAT_MODULE, pszSetting));
	replaceStrW(ppPointer, (val != nullptr) ? val : pszDefault);
}

#define OPT_FIXHEADINGS (WM_USER+1)

static UINT _o1controls[] = { IDC_CHECKBOXES, IDC_GROUP, IDC_STATIC_ADD };

HWND CreateToolTip(HWND hwndParent, LPTSTR ptszText, LPTSTR ptszTitle)
{
	HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST,
		TOOLTIPS_CLASS, nullptr,
		WS_POPUP | TTS_NOPREFIX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		hwndParent, nullptr, g_hInst, nullptr);

	SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	TOOLINFO ti = { 0 };
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS | TTF_CENTERTIP;
	ti.hwnd = hwndParent;
	ti.hinst = g_hInst;
	ti.lpszText = ptszText;
	GetClientRect(hwndParent, &ti.rect);
	ti.rect.left = -65;

	SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)&ti);
	SendMessage(hwndTT, TTM_SETTITLE, 1, (LPARAM)ptszTitle);
	SendMessage(hwndTT, TTM_SETMAXTIPWIDTH, 0, (LPARAM)640);
	return hwndTT;
}

INT_PTR CALLBACK DlgProcOptions1(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HTREEITEM hListHeading1 = 0;
	static HTREEITEM hListHeading2 = 0;

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CHECKBOXES), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CHECKBOXES), GWL_STYLE) | (TVS_NOHSCROLL));
			/* Replace image list, destroy old. */
			ImageList_Destroy(TreeView_SetImageList(GetDlgItem(hwndDlg, IDC_CHECKBOXES), CreateStateImageList(), TVSIL_NORMAL));

			hListHeading1 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), TranslateT("Appearance and functionality of chat room windows"), TRUE);
			hListHeading2 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), TranslateT("Appearance of the message log"), TRUE);

			FillBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading1, branch1, _countof(branch1), 0x0000);
			FillBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading2, branch2, _countof(branch2), 0x0000);

			wchar_t* pszGroup = nullptr;
			InitSetting(pszGroup, "AddToGroup", L"Chat rooms");
			SetDlgItemText(hwndDlg, IDC_GROUP, pszGroup);
			mir_free(pszGroup);
		}
		break;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDC_GROUP) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;

		if (lParam != 0)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_CHECKBOXES:
			return TreeViewHandleClick(hwndDlg, ((LPNMHDR)lParam)->hwndFrom, wParam, lParam);
			break;

		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				int iLen;
				wchar_t *pszText = nullptr;
				BYTE b;

				iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_GROUP));
				if (iLen > 0) {
					pszText = (wchar_t*)mir_realloc(pszText, (iLen + 2) * sizeof(wchar_t));
					GetDlgItemText(hwndDlg, IDC_GROUP, pszText, iLen + 1);
					db_set_ws(0, CHAT_MODULE, "AddToGroup", pszText);
				}
				else db_set_ws(0, CHAT_MODULE, "AddToGroup", L"");

				mir_free(pszText);

				b = M.GetByte(CHAT_MODULE, "Tabs", 1);
				SaveBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), branch1, _countof(branch1));
				SaveBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), branch2, _countof(branch2));

				pci->ReloadSettings();
				pci->MM_IconsChanged();
				pci->MM_FontsChanged();
				Chat_UpdateOptions();
				SM_ReconfigureFilters();
				return TRUE;
			}
		}
		break;

	case WM_DESTROY:
		BYTE b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading1, TVIS_EXPANDED) & TVIS_EXPANDED ? 1 : 0;
		db_set_b(0, CHAT_MODULE, "Branch1Exp", b);
		b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading2, TVIS_EXPANDED) & TVIS_EXPANDED ? 1 : 0;
		db_set_b(0, CHAT_MODULE, "Branch2Exp", b);
		TreeViewDestroy(GetDlgItem(hwndDlg, IDC_CHECKBOXES));
	}
	return FALSE;
}

static wchar_t* chatcolorsnames[] =
{
	LPGENW("Voiced"),
	LPGENW("Half operators"),
	LPGENW("Channel operators"),
	LPGENW("Extended mode 1"),
	LPGENW("Extended mode 2"),
	LPGENW("Selection background"),
	LPGENW("Selected text"),
	LPGENW("Incremental search highlight")
};

void RegisterFontServiceFonts()
{
	LOGFONT lf;
	FontIDW fid = { 0 };
	ColourIDW cid = { 0 };

	fid.cbSize = sizeof(FontIDW);
	cid.cbSize = sizeof(ColourIDW);

	strncpy(fid.dbSettingsGroup, FONTMODULE, _countof(fid.dbSettingsGroup));

	for (int i = 0; i < _countof(IM_fontOptionsList); i++) {
		fid.flags = FIDF_DEFAULTVALID | FIDF_ALLOWEFFECTS;
		LoadMsgDlgFont(FONTSECTION_IM, i, &lf, &fontOptionsList[i].colour, FONTMODULE);
		mir_snprintf(fid.prefix, "Font%d", i);
		fid.order = i;
		wcsncpy(fid.name, fontOptionsList[i].szDescr, _countof(fid.name));
		fid.deffontsettings.colour = fontOptionsList[i].colour;
		fid.deffontsettings.size = (char)lf.lfHeight;
		fid.deffontsettings.style = (lf.lfWeight >= FW_BOLD ? FONTF_BOLD : 0) | (lf.lfItalic ? FONTF_ITALIC : 0);
		fid.deffontsettings.charset = lf.lfCharSet;
		fid.flags = fid.flags & ~FIDF_CLASSMASK | (fid.deffontsettings.style&FONTF_BOLD ? FIDF_CLASSHEADER : FIDF_CLASSGENERAL);
		wcsncpy(fid.deffontsettings.szFace, lf.lfFaceName, LF_FACESIZE);
		wcsncpy(fid.backgroundGroup, LPGENW("Message Sessions") L"/" LPGENW("Single Messaging"), _countof(fid.backgroundGroup));
		wcsncpy(fid.group, LPGENW("Message Sessions") L"/" LPGENW("Single Messaging"), _countof(fid.group));
		switch (i) {
		case MSGFONTID_MYMSG:
		case 1:
		case MSGFONTID_MYNAME:
		case MSGFONTID_MYTIME:
		case 21:
			wcsncpy(fid.backgroundName, LPGENW("Outgoing background"), _countof(fid.backgroundName));
			break;
		case 8:
		case 9:
		case 12:
		case 13:
			wcsncpy(fid.backgroundName, LPGENW("Outgoing background(old)"), _countof(fid.backgroundName));
			break;
		case 10:
		case 11:
		case 14:
		case 15:
			wcsncpy(fid.backgroundName, LPGENW("Incoming background(old)"), _countof(fid.backgroundName));
			break;
		case MSGFONTID_MESSAGEAREA:
			wcsncpy(fid.group, LPGENW("Message Sessions"), _countof(fid.group));
			wcsncpy(fid.backgroundGroup, LPGENW("Message Sessions"), _countof(fid.backgroundGroup));
			wcsncpy(fid.backgroundName, LPGENW("Input area background"), _countof(fid.backgroundName));
			fid.flags |= FIDF_DISABLESTYLES;
			fid.flags &= ~FIDF_ALLOWEFFECTS;
			break;
		case 17:
			wcsncpy(fid.backgroundName, LPGENW("Status background"), _countof(fid.backgroundName));
			break;
		case 18:
			wcsncpy(fid.backgroundGroup, LPGENW("Message Sessions"), _countof(fid.backgroundGroup));
			wcsncpy(fid.backgroundName, LPGENW("Log background"), _countof(fid.backgroundName));
			break;
		case 19:
			wcsncpy(fid.backgroundName, L"", _countof(fid.backgroundName));
			break;
		default:
			wcsncpy(fid.backgroundName, LPGENW("Incoming background"), _countof(fid.backgroundName));
			break;
		}
		Font_RegisterW(&fid);
	}

	fontOptionsList = IP_fontOptionsList;
	fid.flags = FIDF_DEFAULTVALID | FIDF_ALLOWEFFECTS;
	wcsncpy(fid.group, LPGENW("Message Sessions") L"/" LPGENW("Info Panel"), _countof(fid.group));
	wcsncpy(fid.backgroundGroup, LPGENW("Message Sessions") L"/" LPGENW("Info Panel"), _countof(fid.backgroundGroup));
	wcsncpy(fid.backgroundName, LPGENW("Fields background"), _countof(fid.backgroundName));
	for (int i = 0; i < IPFONTCOUNT; i++) {
		LoadMsgDlgFont(FONTSECTION_IP, i, &lf, &fontOptionsList[i].colour, FONTMODULE);
		mir_snprintf(fid.prefix, "Font%d", i + 100);
		fid.order = i + 100;
		wcsncpy(fid.name, fontOptionsList[i].szDescr, _countof(fid.name));
		fid.deffontsettings.colour = fontOptionsList[i].colour;
		fid.deffontsettings.size = (char)lf.lfHeight;
		fid.deffontsettings.style = (lf.lfWeight >= FW_BOLD ? FONTF_BOLD : 0) | (lf.lfItalic ? FONTF_ITALIC : 0);
		fid.deffontsettings.charset = lf.lfCharSet;
		fid.flags = fid.flags & ~FIDF_CLASSMASK | (fid.deffontsettings.style&FONTF_BOLD ? FIDF_CLASSHEADER : FIDF_CLASSGENERAL);
		fid.deffontsettings.charset = lf.lfCharSet;
		wcsncpy(fid.deffontsettings.szFace, lf.lfFaceName, LF_FACESIZE);
		if (i == IPFONTCOUNT - 1) {
			wcsncpy(fid.backgroundGroup, L"", _countof(fid.backgroundGroup));
			wcsncpy(fid.backgroundName, L"", _countof(fid.backgroundName));
			wcsncpy(fid.group, LPGENW("Message Sessions"), _countof(fid.group));
		}
		Font_RegisterW(&fid);
	}

	wcsncpy(cid.group, LPGENW("Message Sessions") L"/" LPGENW("Group chats"), _countof(cid.group));
	strncpy(cid.dbSettingsGroup, CHAT_MODULE, _countof(cid.dbSettingsGroup));
	for (int i = 0; i <= 7; i++) {
		mir_snprintf(cid.setting, "NickColor%d", i);
		wcsncpy(cid.name, chatcolorsnames[i], _countof(cid.name));
		cid.order = i + 1;
		switch (i) {
		case 5:
			cid.defcolour = GetSysColor(COLOR_HIGHLIGHT);
			break;
		case 6:
			cid.defcolour = GetSysColor(COLOR_HIGHLIGHTTEXT);
			break;
		default:
			cid.defcolour = RGB(0, 0, 0);
			break;
		}
		Colour_RegisterW(&cid);
	}
	cid.order++;
	wcsncpy_s(cid.name, LPGENW("Nick list background"), _TRUNCATE);
	strncpy_s(cid.setting, "ColorNicklistBG", _TRUNCATE);
	cid.defcolour = SRMSGDEFSET_BKGCOLOUR;
	Colour_RegisterW(&cid);

	cid.order++;
	wcsncpy_s(cid.name, LPGENW("Group chat log background"), _TRUNCATE);
	strncpy_s(cid.setting, "ColorLogBG", _TRUNCATE);
	Colour_RegisterW(&cid);

	// static colors (info panel, tool bar background etc...)
	strncpy(fid.dbSettingsGroup, FONTMODULE, _countof(fid.dbSettingsGroup));
	strncpy(cid.dbSettingsGroup, FONTMODULE, _countof(fid.dbSettingsGroup));

	for (int i = 0; i < _countof(_clrs); i++) {
		cid.order = _clrs[i].order;
		wcsncpy(cid.group, _clrs[i].tszGroup, _countof(fid.group));
		wcsncpy(cid.name, _clrs[i].tszName, _countof(cid.name));
		strncpy(cid.setting, _clrs[i].szSetting, _countof(cid.setting));
		if (_clrs[i].def & 0xff000000)
			cid.defcolour = GetSysColor(_clrs[i].def & 0x000000ff);
		else
			cid.defcolour = _clrs[i].def;
		Colour_RegisterW(&cid);
	}

	strncpy(cid.dbSettingsGroup, SRMSGMOD_T, _countof(fid.dbSettingsGroup));

	// text and background colors for tabs
	for (int i = 0; i < _countof(_tabclrs); i++) {
		cid.order = _tabclrs[i].order;
		wcsncpy(cid.group, _tabclrs[i].tszGroup, _countof(fid.group));
		wcsncpy(cid.name, _tabclrs[i].tszName, _countof(cid.name));
		strncpy(cid.setting, _tabclrs[i].szSetting, _countof(cid.setting));
		if (_tabclrs[i].def & 0xff000000)
			cid.defcolour = GetSysColor(_tabclrs[i].def & 0x000000ff);
		else
			cid.defcolour = _tabclrs[i].def;

		Colour_RegisterW(&cid);
	}
}

int FontServiceFontsChanged(WPARAM, LPARAM)
{
	PluginConfig.reloadSettings();
	CSkin::initAeroEffect();
	CacheMsgLogIcons();
	CacheLogFonts();
	FreeTabConfig();
	ReloadTabConfig();
	Skin->setupAeroSkins();
	Srmm_Broadcast(DM_OPTIONSAPPLIED, 1, 0);
	return 0;
}

static UINT _o2chatcontrols[] =
{
	IDC_CHAT_SPIN2, IDC_LIMIT, IDC_CHAT_SPIN4, IDC_LOGTIMESTAMP, IDC_TIMESTAMP,
	IDC_OUTSTAMP, IDC_FONTCHOOSE, IDC_LOGGING, IDC_LOGDIRECTORY, IDC_INSTAMP, IDC_CHAT_SPIN2, IDC_CHAT_SPIN3, IDC_NICKROW2, IDC_LOGLIMIT,
	IDC_STATIC110, IDC_STATIC112, 0
};

static UINT _o3chatcontrols[] = { 0 };

INT_PTR CALLBACK DlgProcOptions2(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN2, UDM_SETRANGE, 0, MAKELONG(5000, 0));
			SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN2, UDM_SETPOS, 0, MAKELONG(db_get_w(0, CHAT_MODULE, "LogLimit", 100), 0));
			SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN3, UDM_SETRANGE, 0, MAKELONG(255, 10));
			SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN3, UDM_SETPOS, 0, MAKELONG(M.GetByte(CHAT_MODULE, "NicklistRowDist", 12), 0));
			SetDlgItemText(hwndDlg, IDC_LOGTIMESTAMP, g_Settings.pszTimeStampLog);
			SetDlgItemText(hwndDlg, IDC_TIMESTAMP, g_Settings.pszTimeStamp);
			SetDlgItemText(hwndDlg, IDC_OUTSTAMP, g_Settings.pszOutgoingNick);
			SetDlgItemText(hwndDlg, IDC_INSTAMP, g_Settings.pszIncomingNick);
			CheckDlgButton(hwndDlg, IDC_LOGGING, g_Settings.bLoggingEnabled ? BST_CHECKED : BST_UNCHECKED);
			SetDlgItemText(hwndDlg, IDC_LOGDIRECTORY, g_Settings.pszLogDir);
			Utils::enableDlgControl(hwndDlg, IDC_LOGDIRECTORY, g_Settings.bLoggingEnabled);
			Utils::enableDlgControl(hwndDlg, IDC_FONTCHOOSE, g_Settings.bLoggingEnabled);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN4, UDM_SETRANGE, 0, MAKELONG(10000, 0));
			SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN4, UDM_SETPOS, 0, MAKELONG(db_get_w(0, CHAT_MODULE, "LoggingLimit", 100), 0));
			Utils::enableDlgControl(hwndDlg, IDC_LIMIT, g_Settings.bLoggingEnabled);

			wchar_t tszTooltipText[2048];

			mir_snwprintf(tszTooltipText,
				L"%s - %s\n%s - %s\n%s - %s\n%s - %s\n\n"
				L"%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n\n"
				L"%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s",
				// contact vars
				L"%nick%", TranslateT("nick of current contact (if defined)"),
				L"%proto%", TranslateT("protocol name of current contact (if defined). Account name is used when protocol supports multiple accounts"),
				L"%accountname%", TranslateT("user-defined account name of current contact (if defined)."),
				L"%userid%", TranslateT("user ID of current contact (if defined). It is like UIN for ICQ, JID for Jabber, etc."),
				// global vars
				L"%miranda_path%", TranslateT("path to Miranda root folder"),
				L"%miranda_profilesdir%", TranslateT("path to folder containing Miranda profiles"),
				L"%miranda_profilename%", TranslateT("name of current Miranda profile (filename, without extension)"),
				L"%miranda_userdata%", TranslateT("will return parsed string %miranda_profilesdir%\\%miranda_profilename%"),
				L"%miranda_logpath%", TranslateT("will return parsed string %miranda_userdata%\\Logs"),
				L"%appdata%", TranslateT("same as environment variable %APPDATA% for currently logged-on Windows user"),
				L"%username%", TranslateT("username for currently logged-on Windows user"),
				L"%mydocuments%", TranslateT("\"My Documents\" folder for currently logged-on Windows user"),
				L"%desktop%", TranslateT("\"Desktop\" folder for currently logged-on Windows user"),
				L"%xxxxxxx%", TranslateT("any environment variable defined in current Windows session (like %systemroot%, %allusersprofile%, etc.)"),
				// date/time vars
				L"%d%", TranslateT("day of month, 1-31"),
				L"%dd%", TranslateT("day of month, 01-31"),
				L"%m%", TranslateT("month number, 1-12"),
				L"%mm%", TranslateT("month number, 01-12"),
				L"%mon%", TranslateT("abbreviated month name"),
				L"%month%", TranslateT("full month name"),
				L"%yy%", TranslateT("year without century, 01-99"),
				L"%yyyy%", TranslateT("year with century, 1901-9999"),
				L"%wday%", TranslateT("abbreviated weekday name"),
				L"%weekday%", TranslateT("full weekday name"));
			hPathTip = CreateToolTip(GetDlgItem(hwndDlg, IDC_LOGDIRECTORY), tszTooltipText, TranslateT("Variables"));
		}
		if (hPathTip)
			SetTimer(hwndDlg, 0, 3000, nullptr);
		break;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDC_INSTAMP
			|| LOWORD(wParam) == IDC_OUTSTAMP
			|| LOWORD(wParam) == IDC_TIMESTAMP
			|| LOWORD(wParam) == IDC_LOGLIMIT
			|| LOWORD(wParam) == IDC_NICKROW2
			|| LOWORD(wParam) == IDC_LOGDIRECTORY
			|| LOWORD(wParam) == IDC_LIMIT
			|| LOWORD(wParam) == IDC_LOGTIMESTAMP)
			&& (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))	return 0;

		// open the base directory for MUC logs, using a standard file selector
		// dialog. Simply allows the user to view what log files are there
		// and possibly delete archived logs.
		switch (LOWORD(wParam)) {
		case IDC_MUC_OPENLOGBASEDIR:
			{
				wchar_t	tszTemp[MAX_PATH + 20];
				wcsncpy_s(tszTemp, g_Settings.pszLogDir, _TRUNCATE);

				wchar_t *p = tszTemp;
				while (*p && (*p == '\\' || *p == '.'))
					p++;

				if (*p)
					if (wchar_t *p1 = wcschr(p, '\\'))
						*p1 = 0;

				wchar_t tszInitialDir[_MAX_DRIVE + _MAX_PATH + 10];
				mir_snwprintf(tszInitialDir, L"%s%s", M.getChatLogPath(), p);
				if (!PathFileExists(tszInitialDir))
					wcsncpy_s(tszInitialDir, M.getChatLogPath(), _TRUNCATE);

				wchar_t	tszReturnName[MAX_PATH]; tszReturnName[0] = 0;
				mir_snwprintf(tszTemp, L"%s%c*.*%c%c", TranslateT("All files"), 0, 0, 0);

				OPENFILENAME ofn = { 0 };
				ofn.lpstrInitialDir = tszInitialDir;
				ofn.lpstrFilter = tszTemp;
				ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
				ofn.lpstrFile = tszReturnName;
				ofn.nMaxFile = MAX_PATH;
				ofn.nMaxFileTitle = MAX_PATH;
				ofn.Flags = OFN_HIDEREADONLY | OFN_DONTADDTORECENT;
				ofn.lpstrDefExt = L"log";
				GetOpenFileName(&ofn);
			}
			break;

		case IDC_FONTCHOOSE:
			{
				wchar_t tszDirectory[MAX_PATH];
				LPMALLOC psMalloc;

				if (SUCCEEDED(CoGetMalloc(1, &psMalloc))) {
					BROWSEINFO bi = { 0 };
					bi.hwndOwner = hwndDlg;
					bi.pszDisplayName = tszDirectory;
					bi.lpszTitle = TranslateT("Select folder");
					bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX | BIF_RETURNONLYFSDIRS;
					bi.lpfn = BrowseCallbackProc;
					bi.lParam = (LPARAM)tszDirectory;

					LPITEMIDLIST idList = SHBrowseForFolder(&bi);
					if (idList) {
						const wchar_t *szUserDir = M.getUserDir();
						SHGetPathFromIDList(idList, tszDirectory);
						mir_wstrcat(tszDirectory, L"\\");

						wchar_t tszTemp[MAX_PATH];
						PathToRelativeW(tszDirectory, tszTemp, szUserDir);
						SetDlgItemText(hwndDlg, IDC_LOGDIRECTORY, mir_wstrlen(tszTemp) > 1 ? tszTemp : DEFLOGFILENAME);
					}
					psMalloc->Free(idList);
					psMalloc->Release();
				}
			}
			break;

		case IDC_LOGGING:
			Utils::enableDlgControl(hwndDlg, IDC_LOGDIRECTORY, IsDlgButtonChecked(hwndDlg, IDC_LOGGING) == BST_CHECKED ? TRUE : FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_FONTCHOOSE, IsDlgButtonChecked(hwndDlg, IDC_LOGGING) == BST_CHECKED ? TRUE : FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_LIMIT, IsDlgButtonChecked(hwndDlg, IDC_LOGGING) == BST_CHECKED ? TRUE : FALSE);
			break;
		}

		if (lParam != 0)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0 && ((LPNMHDR)lParam)->code == PSN_APPLY) {
			char *pszText = nullptr;

			int iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_LOGDIRECTORY));
			if (iLen > 0) {
				wchar_t *pszText1 = (wchar_t*)mir_alloc(iLen*sizeof(wchar_t) + 2);
				GetDlgItemText(hwndDlg, IDC_LOGDIRECTORY, pszText1, iLen + 1);
				db_set_ws(0, CHAT_MODULE, "LogDirectory", pszText1);
				mir_free(pszText1);
				g_Settings.bLoggingEnabled = IsDlgButtonChecked(hwndDlg, IDC_LOGGING) == BST_CHECKED;
				db_set_b(0, CHAT_MODULE, "LoggingEnabled", g_Settings.bLoggingEnabled);
			}
			else {
				db_unset(0, CHAT_MODULE, "LogDirectory");
				db_set_b(0, CHAT_MODULE, "LoggingEnabled", 0);
			}
			pci->SM_InvalidateLogDirectories();

			iLen = SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN4, UDM_GETPOS, 0, 0);
			db_set_w(0, CHAT_MODULE, "LoggingLimit", (WORD)iLen);

			iLen = SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN3, UDM_GETPOS, 0, 0);
			if (iLen > 0)
				db_set_b(0, CHAT_MODULE, "NicklistRowDist", (BYTE)iLen);
			else
				db_unset(0, CHAT_MODULE, "NicklistRowDist");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_LOGTIMESTAMP));
			if (iLen > 0) {
				pszText = (char *)mir_realloc(pszText, iLen + 1);
				GetDlgItemTextA(hwndDlg, IDC_LOGTIMESTAMP, pszText, iLen + 1);
				db_set_s(0, CHAT_MODULE, "LogTimestamp", pszText);
			}
			else db_unset(0, CHAT_MODULE, "LogTimestamp");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_TIMESTAMP));
			if (iLen > 0) {
				pszText = (char *)mir_realloc(pszText, iLen + 1);
				GetDlgItemTextA(hwndDlg, IDC_TIMESTAMP, pszText, iLen + 1);
				db_set_s(0, CHAT_MODULE, "HeaderTime", pszText);
			}
			else db_unset(0, CHAT_MODULE, "HeaderTime");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_INSTAMP));
			if (iLen > 0) {
				pszText = (char *)mir_realloc(pszText, iLen + 1);
				GetDlgItemTextA(hwndDlg, IDC_INSTAMP, pszText, iLen + 1);
				db_set_s(0, CHAT_MODULE, "HeaderIncoming", pszText);
			}
			else db_unset(0, CHAT_MODULE, "HeaderIncoming");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_OUTSTAMP));
			if (iLen > 0) {
				pszText = (char *)mir_realloc(pszText, iLen + 1);
				GetDlgItemTextA(hwndDlg, IDC_OUTSTAMP, pszText, iLen + 1);
				db_set_s(0, CHAT_MODULE, "HeaderOutgoing", pszText);
			}
			else db_unset(0, CHAT_MODULE, "HeaderOutgoing");

			iLen = SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN2, UDM_GETPOS, 0, 0);
			db_set_w(0, CHAT_MODULE, "LogLimit", (WORD)iLen);
			mir_free(pszText);

			if (pci->hListBkgBrush)
				DeleteObject(pci->hListBkgBrush);
			pci->hListBkgBrush = CreateSolidBrush(M.GetDword(CHAT_MODULE, "ColorNicklistBG", SRMSGDEFSET_BKGCOLOUR));

			pci->ReloadSettings();
			pci->MM_FontsChanged();
			Chat_UpdateOptions();

			PluginConfig.reloadSettings();
			CacheMsgLogIcons();
			CacheLogFonts();
			return TRUE;
		}
		break;

	case WM_TIMER:
		if (IsWindow(hPathTip))
			KillTimer(hPathTip, 4); // It will prevent tooltip autoclosing
		break;

	case WM_DESTROY:
		if (hPathTip) {
			KillTimer(hwndDlg, 0);
			DestroyWindow(hPathTip);
			hPathTip = 0;
		}
		break;
	}
	return FALSE;
}

#define NR_GC_EVENTS 12

static UINT _eventorder[] =
{
	GC_EVENT_ACTION,
	GC_EVENT_MESSAGE,
	GC_EVENT_NICK,
	GC_EVENT_JOIN,
	GC_EVENT_PART,
	GC_EVENT_TOPIC,
	GC_EVENT_ADDSTATUS,
	GC_EVENT_INFORMATION,
	GC_EVENT_QUIT,
	GC_EVENT_KICK,
	GC_EVENT_NOTICE,
	GC_EVENT_HIGHLIGHT
};

// Dialog procedure for group chat options tab #3 (event filter configuration)
INT_PTR CALLBACK DlgProcOptions3(HWND hwndDlg, UINT uMsg, WPARAM, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			DWORD dwFilterFlags = M.GetDword(CHAT_MODULE, "FilterFlags", GC_EVENT_ALL);
			DWORD dwTrayFlags = M.GetDword(CHAT_MODULE, "TrayIconFlags", GC_EVENT_HIGHLIGHT);
			DWORD dwPopupFlags = M.GetDword(CHAT_MODULE, "PopupFlags", GC_EVENT_HIGHLIGHT);
			DWORD dwLogFlags = M.GetDword(CHAT_MODULE, "DiskLogFlags", GC_EVENT_ALL);

			for (int i = 0; i < _countof(_eventorder); i++) {
				if (_eventorder[i] != GC_EVENT_HIGHLIGHT) {
					CheckDlgButton(hwndDlg, IDC_1 + i, dwFilterFlags & _eventorder[i] ? BST_CHECKED : BST_UNCHECKED);
					CheckDlgButton(hwndDlg, IDC_L1 + i, dwLogFlags & _eventorder[i] ? BST_CHECKED : BST_UNCHECKED);
				}
				CheckDlgButton(hwndDlg, IDC_P1 + i, dwPopupFlags & _eventorder[i] ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_T1 + i, dwTrayFlags & _eventorder[i] ? BST_CHECKED : BST_UNCHECKED);
			}
		}
		SendDlgItemMessage(hwndDlg, IDC_LOGICONTYPE, CB_ADDSTRING, -1, (LPARAM)TranslateT("No markers"));
		SendDlgItemMessage(hwndDlg, IDC_LOGICONTYPE, CB_ADDSTRING, -1, (LPARAM)TranslateT("Show as icons"));
		SendDlgItemMessage(hwndDlg, IDC_LOGICONTYPE, CB_ADDSTRING, -1, (LPARAM)TranslateT("Show as text symbols"));

		SendDlgItemMessage(hwndDlg, IDC_LOGICONTYPE, CB_SETCURSEL, (g_Settings.bLogSymbols ? 2 : (g_Settings.dwIconFlags ? 1 : 0)), 0);

		CheckDlgButton(hwndDlg, IDC_TRAYONLYFORINACTIVE, M.GetByte(CHAT_MODULE, "TrayIconInactiveOnly", 0) ? BST_CHECKED : BST_UNCHECKED);
		break;

	case WM_COMMAND:
		if (lParam != 0)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				DWORD dwFilterFlags = 0, dwTrayFlags = 0,
					dwPopupFlags = 0, dwLogFlags = 0;

				for (int i = 0; i < _countof(_eventorder); i++) {
					if (_eventorder[i] != GC_EVENT_HIGHLIGHT) {
						dwFilterFlags |= (IsDlgButtonChecked(hwndDlg, IDC_1 + i) ? _eventorder[i] : 0);
						dwLogFlags |= (IsDlgButtonChecked(hwndDlg, IDC_L1 + i) ? _eventorder[i] : 0);
					}
					dwPopupFlags |= (IsDlgButtonChecked(hwndDlg, IDC_P1 + i) ? _eventorder[i] : 0);
					dwTrayFlags |= (IsDlgButtonChecked(hwndDlg, IDC_T1 + i) ? _eventorder[i] : 0);
				}
				db_set_dw(0, CHAT_MODULE, "FilterFlags", dwFilterFlags);
				db_set_dw(0, CHAT_MODULE, "PopupFlags", dwPopupFlags);
				db_set_dw(0, CHAT_MODULE, "TrayIconFlags", dwTrayFlags);
				db_set_dw(0, CHAT_MODULE, "DiskLogFlags", dwLogFlags);

				LRESULT lr = SendDlgItemMessage(hwndDlg, IDC_LOGICONTYPE, CB_GETCURSEL, 0, 0);

				db_set_dw(0, CHAT_MODULE, "IconFlags", lr == 1 ? 1 : 0);
				db_set_b(0, CHAT_MODULE, "LogSymbols", lr == 2 ? 1 : 0);

				db_set_b(0, CHAT_MODULE, "TrayIconInactiveOnly", IsDlgButtonChecked(hwndDlg, IDC_TRAYONLYFORINACTIVE) ? 1 : 0);

				pci->ReloadSettings();
				pci->MM_FontsChanged();
				Chat_UpdateOptions();
				SM_ReconfigureFilters();
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}
