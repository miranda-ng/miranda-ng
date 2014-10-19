/*
 * Miranda NG: the free IM client for Microsoft* Windows*
 *
 * Copyright (c) 2000-09 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of tabSRMM messaging plugin for Miranda.
 *
 * This code is based on and still contains large parts of the the
 * original chat module for Miranda NG, written and copyrighted
 * by Joergen Persson in 2005.
 *
 * (C) 2005-2009 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * group chat options and generic font handling
 *
 */

#include "..\commonheaders.h"

#define FONTF_BOLD   1
#define FONTF_ITALIC 2

struct FontOptionsList
{
	TCHAR*   szDescr;
	COLORREF defColour;
	TCHAR*   szDefFace;
	BYTE     defCharset, defStyle;
	char     defSize;
	COLORREF colour;
	TCHAR    szFace[LF_FACESIZE];
	BYTE     charset, style;
	char     size;
};

struct ColorOptionsList
{
	int			order;
	TCHAR*		tszGroup;
	TCHAR*		tszName;
	char*		szSetting;
	COLORREF 	def;
};

/*
* note: bits 24-31 in default color indicates that color is a system color index
* (GetSysColor(default_color & 0x00ffffff)), not a rgb value.
*/
static ColorOptionsList _clrs[] = {
	{ 0, LPGENT("Message Sessions"), LPGENT("Input area background"), "inputbg", SRMSGDEFSET_BKGCOLOUR },
	{ 1, LPGENT("Message Sessions"), LPGENT("Log background"), SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR },
	{ 0, LPGENT("Message Sessions")_T("/")LPGENT("Single Messaging"), LPGENT("Outgoing background"), "outbg", SRMSGDEFSET_BKGOUTCOLOUR },
	{ 1, LPGENT("Message Sessions")_T("/")LPGENT("Single Messaging"), LPGENT("Incoming background"), "inbg", SRMSGDEFSET_BKGINCOLOUR },
	{ 2, LPGENT("Message Sessions")_T("/")LPGENT("Single Messaging"), LPGENT("Status background"), "statbg", SRMSGDEFSET_BKGCOLOUR },
	{ 3, LPGENT("Message Sessions")_T("/")LPGENT("Single Messaging"), LPGENT("Incoming background(old)"), "oldinbg", SRMSGDEFSET_BKGINCOLOUR },
	{ 4, LPGENT("Message Sessions")_T("/")LPGENT("Single Messaging"), LPGENT("Outgoing background(old)"), "oldoutbg", SRMSGDEFSET_BKGOUTCOLOUR },
	{ 5, LPGENT("Message Sessions")_T("/")LPGENT("Single Messaging"), LPGENT("Horizontal Grid Lines"), "hgrid", RGB(224, 224, 224) },
	{ 0, LPGENT("Message Sessions")_T("/")LPGENT("Info Panel"), LPGENT("Panel background low"), "ipfieldsbg", 0x62caff },
	{ 1, LPGENT("Message Sessions")_T("/")LPGENT("Info Panel"), LPGENT("Panel background high"), "ipfieldsbgHigh", 0xf0f0f0 },
	{ 0, LPGENT("Message Sessions")_T("/")LPGENT("Common colors"), LPGENT("Toolbar background high"), "tbBgHigh", 0 },
	{ 1, LPGENT("Message Sessions")_T("/")LPGENT("Common colors"), LPGENT("Toolbar background low"), "tbBgLow", 0 },
	{ 2, LPGENT("Message Sessions")_T("/")LPGENT("Common colors"), LPGENT("Window fill color"), "fillColor", 0 },
	{ 3, LPGENT("Message Sessions")_T("/")LPGENT("Common colors"), LPGENT("Text area borders"), "cRichBorders", 0 },
	{ 4, LPGENT("Message Sessions")_T("/")LPGENT("Common colors"), LPGENT("Aero glow effect"), "aeroGlow", RGB(40, 40, 255) },
	{ 4, LPGENT("Message Sessions")_T("/")LPGENT("Common colors"), LPGENT("Generic text color (only when fill color is set)"), "genericTxtClr", 0xff000000 | COLOR_BTNTEXT },
};

static ColorOptionsList _tabclrs[] = {
	{ 0, LPGENT("Message Sessions")_T("/")LPGENT("Tabs"), LPGENT("Normal text"), "tab_txt_normal", 0xff000000 | COLOR_BTNTEXT },
	{ 1, LPGENT("Message Sessions")_T("/")LPGENT("Tabs"), LPGENT("Active text"), "tab_txt_active", 0xff000000 | COLOR_BTNTEXT },
	{ 2, LPGENT("Message Sessions")_T("/")LPGENT("Tabs"), LPGENT("Hovered text"), "tab_txt_hottrack", 0xff000000 | COLOR_HOTLIGHT },
	{ 3, LPGENT("Message Sessions")_T("/")LPGENT("Tabs"), LPGENT("Unread text"), "tab_txt_unread", 0xff000000 | COLOR_HOTLIGHT },

	{ 4, LPGENT("Message Sessions")_T("/")LPGENT("Tabs"), LPGENT("Normal background"), "tab_bg_normal", 0xff000000 | COLOR_3DFACE },
	{ 5, LPGENT("Message Sessions")_T("/")LPGENT("Tabs"), LPGENT("Active background"), "tab_bg_active", 0xff000000 | COLOR_3DFACE },
	{ 6, LPGENT("Message Sessions")_T("/")LPGENT("Tabs"), LPGENT("Hovered background"), "tab_bg_hottrack", 0xff000000 | COLOR_3DFACE },
	{ 7, LPGENT("Message Sessions")_T("/")LPGENT("Tabs"), LPGENT("Unread background"), "tab_bg_unread", 0xff000000 | COLOR_3DFACE }
};

extern LOGFONT lfDefault;

static FontOptionsList IM_fontOptionsList[] = {
	{ LPGENT(">> Outgoing messages"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENT(">> Outgoing misc events"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("<< Incoming messages"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("<< Incoming misc events"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENT(">> Outgoing name"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENT(">> Outgoing timestamp"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENT("<< Incoming name"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENT("<< Incoming timestamp"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENT(">> Outgoing messages (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENT(">> Outgoing misc events (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("<< Incoming messages (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("<< Incoming misc events (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENT(">> Outgoing name (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENT(">> Outgoing timestamp (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENT("<< Incoming name (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENT("<< Incoming timestamp (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12 },
	{ LPGENT("* Message Input Area"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("* Status changes"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("* Dividers"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("* Error and warning messages"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("* Symbols (incoming)"), RGB(50, 50, 50), _T("Webdings"), SYMBOL_CHARSET, 0, -12 },
	{ LPGENT("* Symbols (outgoing)"), RGB(50, 50, 50), _T("Webdings"), SYMBOL_CHARSET, 0, -12 },
};

static FontOptionsList IP_fontOptionsList[] = {
	{ LPGENT("Nickname"), RGB(0, 0, 0), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("UIN"), RGB(0, 0, 0), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("Status"), RGB(0, 0, 0), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("Protocol"), RGB(0, 0, 0), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("Contacts local time"), RGB(0, 0, 0), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
	{ LPGENT("Window caption (skinned mode)"), RGB(255, 255, 255), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12 },
};

static FontOptionsList *fontOptionsList = IM_fontOptionsList;
static int fontCount = MSGDLGFONTCOUNT;

struct branch_t {
	TCHAR*    szDescr;
	char*     szDBName;
	int       iMode;
	BYTE      bDefault;
	HTREEITEM hItem;
};
static branch_t branch1[] = {
	{ LPGENT("Open new chat rooms in the default container"), "DefaultContainer", 0, 1, NULL },
	{ LPGENT("Flash window when someone speaks"), "FlashWindow", 0, 0, NULL },
	{ LPGENT("Flash window when a word is highlighted"), "FlashWindowHighlight", 0, 1, NULL },
	{ LPGENT("Create tabs or windows for highlight events"), "CreateWindowOnHighlight", 0, 0, NULL },
	{ LPGENT("Activate chat window on highlight"), "AnnoyingHighlight", 0, 0, NULL },
	{ LPGENT("Show list of users in the chat room"), "ShowNicklist", 0, 1, NULL },
	{ LPGENT("Colorize nicknames in member list"), "ColorizeNicks", 0, 1, NULL },
	{ LPGENT("Show button menus when right clicking the buttons"), "RightClickFilter", 0, 1, NULL },
	{ LPGENT("Show topic as status message on the contact list"), "TopicOnClist", 0, 1, NULL },
	{ LPGENT("Do not pop up the window when joining a chat room"), "PopupOnJoin", 0, 0, NULL },
	{ LPGENT("Hide or show the window by double click in the contact list"), "ToggleVisibility", 0, 0, NULL },
	{ LPGENT("Sync splitter position with standard IM sessions"), "SyncSplitter", 0, 0, NULL },
	{ LPGENT("Show contact's status modes if supported by the protocol"), "ShowContactStatus", 0, 1, NULL },
	{ LPGENT("Display contact's status icon before user role icon"), "ContactStatusFirst", 0, 0, NULL },
	{ LPGENT("Use IRC style status indicators in the nick list"), "ClassicIndicators", 0, 0, NULL },
	{ LPGENT("Use alternative sorting method in member list"), "AlternativeSorting", 0, 1, NULL },

};
static branch_t branch2[] = {
	{ LPGENT("Prefix all events with a timestamp"), "ShowTimeStamp", 0, 1, NULL },
	{ LPGENT("Timestamp only when event time differs"), "ShowTimeStampIfChanged", 0, 0, NULL },
	{ LPGENT("Timestamp has same color as the event"), "TimeStampEventColour", 0, 0, NULL },
	{ LPGENT("Indent the second line of a message"), "LogIndentEnabled", 0, 1, NULL },
	{ LPGENT("Limit user names in the message log to 20 characters"), "LogLimitNames", 0, 1, NULL },
	{ LPGENT("Add a colon (:) to auto-completed user names"), "AddColonToAutoComplete", 0, 1, NULL },
	{ LPGENT("Start private conversation on double click in nick list (insert nick if unchecked)"), "DoubleClick4Privat", 0, 0, NULL },
	{ LPGENT("Strip colors from messages in the log"), "StripFormatting", 0, 0, NULL },
	{ LPGENT("Enable the 'event filter' for new rooms"), "FilterEnabled", 0, 0, NULL },
	{ LPGENT("Use IRC style status indicators in the log"), "LogClassicIndicators", 0, 0, NULL },
	{ LPGENT("Allow clickable user names in the message log"), "ClickableNicks", 0, 1, NULL },
	{ LPGENT("Colorize user names in message log"), "ColorizeNicksInLog", 0, 1, NULL },
	{ LPGENT("Scale down icons to 10x10 pixels in the chat log"), "ScaleIcons", 0, 1, NULL },
	{ LPGENT("Place a separator in the log after a window lost its foreground status"), "UseDividers", 0, 1, NULL },
	{ LPGENT("Only place a separator when an incoming event is announced with a popup"), "DividersUsePopupConfig", 0, 1, NULL }
};

static HWND hPathTip = 0;

void LoadMsgDlgFont(int section, int i, LOGFONT *lf, COLORREF* colour, char *szMod)
{
	char str[32];
	int style;
	int j = (i >= 100 ? i - 100 : i);

	FontOptionsList *fol = fontOptionsList;
	switch (section) {
		case FONTSECTION_IM: fol = IM_fontOptionsList; break;
		case FONTSECTION_IP: fol = IP_fontOptionsList; break;
	}

	if (colour) {
		mir_snprintf(str, SIZEOF(str), "Font%dCol", i);
		*colour = M.GetDword(szMod, str, fol[j].defColour);
	}

	if (lf) {
		mir_snprintf(str, SIZEOF(str), "Font%dSize", i);
		lf->lfHeight = (char)M.GetByte(szMod, str, fol[j].defSize);
		lf->lfWidth = 0;
		lf->lfEscapement = 0;
		lf->lfOrientation = 0;
		mir_snprintf(str, SIZEOF(str), "Font%dSty", i);
		style = M.GetByte(szMod, str, fol[j].defStyle);
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
		mir_snprintf(str, SIZEOF(str), "Font%dSet", i);
		lf->lfCharSet = M.GetByte(szMod, str, fol[j].defCharset);
		lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf->lfQuality = DEFAULT_QUALITY;
		lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		mir_snprintf(str, SIZEOF(str), "Font%d", i);
		if ((i == 17 && !strcmp(szMod, CHATFONT_MODULE)) || ((i == 20 || i == 21) && !strcmp(szMod, FONTMODULE))) {
			lf->lfCharSet = SYMBOL_CHARSET;
			lstrcpyn(lf->lfFaceName, _T("Webdings"), SIZEOF(lf->lfFaceName));
		}
		else {
			ptrT tszDefFace(db_get_tsa(NULL, szMod, str));
			if (tszDefFace == NULL)
				lstrcpy(lf->lfFaceName, fol[j].szDefFace);
			else
				lstrcpyn(lf->lfFaceName, tszDefFace, SIZEOF(lf->lfFaceName));
		}
	}
}

static HTREEITEM InsertBranch(HWND hwndTree, TCHAR* pszDescr, BOOL bExpanded)
{
	TVINSERTSTRUCT tvis;
	tvis.hParent = NULL;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT | TVIF_STATE;
	tvis.item.pszText = TranslateTS(pszDescr);
	tvis.item.stateMask = (bExpanded ? TVIS_STATEIMAGEMASK | TVIS_EXPANDED : TVIS_STATEIMAGEMASK) | TVIS_BOLD;
	tvis.item.state = (bExpanded ? INDEXTOSTATEIMAGEMASK(0) | TVIS_EXPANDED : INDEXTOSTATEIMAGEMASK(0)) | TVIS_BOLD;
	return TreeView_InsertItem(hwndTree, &tvis);
}

static void FillBranch(HWND hwndTree, HTREEITEM hParent, branch_t *branch, int nValues, DWORD defaultval)
{
	if (hParent == 0)
		return;

	TVINSERTSTRUCT tvis;
	int iState;

	tvis.hParent = hParent;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT | TVIF_STATE;
	for (int i = 0; i < nValues; i++) {
		tvis.item.pszText = TranslateTS(branch[i].szDescr);
		tvis.item.stateMask = TVIS_STATEIMAGEMASK;
		if (branch[i].iMode)
			iState = ((M.GetDword(CHAT_MODULE, branch[i].szDBName, defaultval) & branch[i].iMode) & branch[i].iMode) != 0 ? 3 : 2;
		else
			iState = M.GetByte(CHAT_MODULE, branch[i].szDBName, branch[i].bDefault) != 0 ? 3 : 2;
		tvis.item.state = INDEXTOSTATEIMAGEMASK(iState);
		branch[i].hItem = TreeView_InsertItem(hwndTree, &tvis);
	}
}

static void SaveBranch(HWND hwndTree, branch_t *branch, int nValues)
{
	TVITEM tvi;
	BYTE bChecked;
	int iState = 0;

	tvi.mask = TVIF_HANDLE | TVIF_STATE;
	for (int i = 0; i < nValues; i++) {
		tvi.hItem = branch[i].hItem;
		TreeView_GetItem(hwndTree, &tvi);
		bChecked = ((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 2) ? 0 : 1;
		if (branch[i].iMode) {
			if (bChecked)
				iState |= branch[i].iMode;
			if (iState & GC_EVENT_ADDSTATUS)
				iState |= GC_EVENT_REMOVESTATUS;
			db_set_dw(0, CHAT_MODULE, branch[i].szDBName, (DWORD)iState);
		}
		else db_set_b(0, CHAT_MODULE, branch[i].szDBName, bChecked);
	}
}

static void CheckHeading(HWND hwndTree, HTREEITEM hHeading)
{
	if (hHeading == 0)
		return;

	BOOL bChecked = TRUE;

	TVITEM tvi;
	tvi.mask = TVIF_HANDLE | TVIF_STATE;
	tvi.hItem = TreeView_GetNextItem(hwndTree, hHeading, TVGN_CHILD);
	while (tvi.hItem && bChecked) {
		if (tvi.hItem != branch1[0].hItem && tvi.hItem != branch1[1].hItem) {
			TreeView_GetItem(hwndTree, &tvi);
			if (((tvi.state&TVIS_STATEIMAGEMASK) >> 12 == 2))
				bChecked = FALSE;
		}
		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
	tvi.stateMask = TVIS_STATEIMAGEMASK;
	tvi.state = INDEXTOSTATEIMAGEMASK(1); //bChecked ? 3 : 2);
	tvi.hItem = hHeading;
	TreeView_SetItem(hwndTree, &tvi);
}

static void CheckBranches(HWND hwndTree, HTREEITEM hHeading)
{
	if (hHeading == 0)
		return;

	BOOL bChecked = TRUE;

	TVITEM tvi;
	tvi.mask = TVIF_HANDLE | TVIF_STATE;
	tvi.hItem = hHeading;
	TreeView_GetItem(hwndTree, &tvi);
	if (((tvi.state&TVIS_STATEIMAGEMASK) >> 12 == 3) || ((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 1))
		bChecked = FALSE;

	tvi.stateMask = TVIS_STATEIMAGEMASK;
	tvi.state = INDEXTOSTATEIMAGEMASK(bChecked ? 2 : 1);
	TreeView_SetItem(hwndTree, &tvi);
	tvi.hItem = TreeView_GetNextItem(hwndTree, hHeading, TVGN_CHILD);
	while (tvi.hItem) {
		tvi.state = INDEXTOSTATEIMAGEMASK(bChecked ? 3 : 2);
		if (tvi.hItem != branch1[0].hItem && tvi.hItem != branch1[1].hItem)
			TreeView_SetItem(hwndTree, &tvi);
		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
}

static INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
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
	Icon_Register(g_hIconDLL, LPGEN("Message Sessions")"/"LPGEN("Group chat windows"), _icons, SIZEOF(_icons));
	Icon_Register(g_hIconDLL, LPGEN("Message Sessions")"/"LPGEN("Group chat log"), _logicons, SIZEOF(_logicons));
	pci->MM_IconsChanged();
}

/*
 * get icon by name from the core icon library service
 */

HICON LoadIconEx(int iIndex, char * pszIcoLibName, int iX, int iY)
{
	char szTemp[256];
	mir_snprintf(szTemp, sizeof(szTemp), "chat_%s", pszIcoLibName);
	return Skin_GetIcon(szTemp);
}

static void InitSetting(TCHAR* &ppPointer, const char *pszSetting, const TCHAR *pszDefault)
{
	ptrT val( db_get_tsa(NULL, CHAT_MODULE, pszSetting));
	replaceStrT(ppPointer, (val != NULL) ? val : pszDefault);
}

#define OPT_FIXHEADINGS (WM_USER+1)

static UINT _o1controls[] = {IDC_CHECKBOXES, IDC_GROUP, IDC_STATIC_ADD};

HWND CreateToolTip(HWND hwndParent, LPTSTR ptszText, LPTSTR ptszTitle)
{
	HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST,
		TOOLTIPS_CLASS, NULL,
		WS_POPUP | TTS_NOPREFIX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		hwndParent, NULL, g_hInst, NULL);

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

	SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
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
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CHECKBOXES), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CHECKBOXES), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		{
			HIMAGELIST himlOptions = (HIMAGELIST)SendDlgItemMessage(hwndDlg, IDC_CHECKBOXES, TVM_SETIMAGELIST, TVSIL_STATE, (LPARAM)CreateStateImageList());
			ImageList_Destroy(himlOptions);

			hListHeading1 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), TranslateT("Appearance and functionality of chat room windows"), TRUE);
			hListHeading2 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), TranslateT("Appearance of the message log"), TRUE);

			FillBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading1, branch1, SIZEOF(branch1), 0x0000);
			FillBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading2, branch2, SIZEOF(branch2), 0x0000);

			TCHAR* pszGroup = NULL;
			InitSetting(pszGroup, "AddToGroup", _T("Chat rooms"));
			SetWindowText(GetDlgItem(hwndDlg, IDC_GROUP), pszGroup);
			mir_free(pszGroup);
			Utils::showDlgControl(hwndDlg, IDC_STATIC_MESSAGE, SW_HIDE);
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
			if (((LPNMHDR)lParam)->code == NM_CLICK || (((LPNMHDR)lParam)->code == TVN_KEYDOWN && ((LPNMTVKEYDOWN)lParam)->wVKey == VK_SPACE)) {
				TVHITTESTINFO hti;
				hti.pt.x = (short)LOWORD(GetMessagePos());
				hti.pt.y = (short)HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
				if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti) || ((LPNMHDR)lParam)->code == TVN_KEYDOWN) {
					if (((LPNMHDR)lParam)->code == TVN_KEYDOWN)
						hti.flags |= TVHT_ONITEMSTATEICON;
					if (hti.flags&TVHT_ONITEMSTATEICON) {
						TVITEM tvi = { 0 };

						tvi.mask = TVIF_HANDLE | TVIF_STATE;
						tvi.stateMask = TVIS_STATEIMAGEMASK | TVIS_BOLD;

						if (((LPNMHDR)lParam)->code == TVN_KEYDOWN)
							tvi.hItem = TreeView_GetSelection(((LPNMHDR)lParam)->hwndFrom);
						else
							tvi.hItem = (HTREEITEM)hti.hItem;

						TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);

						if (tvi.state & TVIS_BOLD && hti.flags & TVHT_ONITEMSTATEICON) {
							tvi.state = INDEXTOSTATEIMAGEMASK(0) | TVIS_BOLD;
							SendDlgItemMessageA(hwndDlg, IDC_CHECKBOXES, TVM_SETITEMA, 0, (LPARAM)&tvi);
						}
						else if (hti.flags&TVHT_ONITEMSTATEICON) {
							if (((tvi.state & TVIS_STATEIMAGEMASK) >> 12) == 3) {
								tvi.state = INDEXTOSTATEIMAGEMASK(1);
								SendDlgItemMessageA(hwndDlg, IDC_CHECKBOXES, TVM_SETITEMA, 0, (LPARAM)&tvi);
							}
						}
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
				}
			}
			break;

		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				int iLen;
				TCHAR *pszText = NULL;
				BYTE b;

				iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_GROUP));
				if (iLen > 0) {
					pszText = (TCHAR*)mir_realloc(pszText, (iLen + 2) * sizeof(TCHAR));
					GetDlgItemText(hwndDlg, IDC_GROUP, pszText, iLen + 1);
					db_set_ts(NULL, CHAT_MODULE, "AddToGroup", pszText);
				}
				else db_set_ts(NULL, CHAT_MODULE, "AddToGroup", _T(""));

				mir_free(pszText);

				b = M.GetByte(CHAT_MODULE, "Tabs", 1);
				SaveBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), branch1, SIZEOF(branch1));
				SaveBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), branch2, SIZEOF(branch2));

				pci->ReloadSettings();
				pci->MM_IconsChanged();
				pci->MM_FontsChanged();
				pci->SM_BroadcastMessage(NULL, GC_SETWNDPROPS, 0, 0, TRUE);
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
	}
	return FALSE;
}

static TCHAR* chatcolorsnames[] =
{
	LPGENT("Voiced"),
	LPGENT("Half operators"),
	LPGENT("Channel operators"),
	LPGENT("Extended mode 1"),
	LPGENT("Extended mode 2"),
	LPGENT("Selection background"),
	LPGENT("Selected text"),
	LPGENT("Incremental search highlight")
};

void RegisterFontServiceFonts() {
	char szTemp[100];
	LOGFONT lf;
	FontIDT fid = {0};
	ColourIDT cid = {0};

	fid.cbSize = sizeof(FontIDT);
	cid.cbSize = sizeof(ColourIDT);

	strncpy(fid.dbSettingsGroup, FONTMODULE, SIZEOF(fid.dbSettingsGroup));

	for (int i = 0; i < SIZEOF(IM_fontOptionsList); i++) {
		fid.flags = FIDF_DEFAULTVALID | FIDF_ALLOWEFFECTS;
		LoadMsgDlgFont(FONTSECTION_IM, i, &lf, &fontOptionsList[i].colour, FONTMODULE);
		mir_snprintf(szTemp, SIZEOF(szTemp), "Font%d", i);
		strncpy(fid.prefix, szTemp, SIZEOF(fid.prefix));
		fid.order = i;
		_tcsncpy(fid.name, fontOptionsList[i].szDescr, SIZEOF(fid.name));
		fid.deffontsettings.colour = fontOptionsList[i].colour;
		fid.deffontsettings.size = (char)lf.lfHeight;
		fid.deffontsettings.style = (lf.lfWeight >= FW_BOLD ? FONTF_BOLD : 0) | (lf.lfItalic ? FONTF_ITALIC : 0);
		fid.deffontsettings.charset = lf.lfCharSet;
		fid.flags = fid.flags & ~FIDF_CLASSMASK | (fid.deffontsettings.style&FONTF_BOLD ? FIDF_CLASSHEADER : FIDF_CLASSGENERAL);
		_tcsncpy(fid.deffontsettings.szFace, lf.lfFaceName, LF_FACESIZE);
		_tcsncpy(fid.backgroundGroup, LPGENT("Message Sessions")_T("/")LPGENT("Single Messaging"), SIZEOF(fid.backgroundGroup));
		_tcsncpy(fid.group, LPGENT("Message Sessions")_T("/")LPGENT("Single Messaging"), SIZEOF(fid.group));
		switch (i) {
		case MSGFONTID_MYMSG:
		case 1:
		case MSGFONTID_MYNAME:
		case MSGFONTID_MYTIME:
		case 21:
			_tcsncpy(fid.backgroundName, LPGENT("Outgoing background"), SIZEOF(fid.backgroundName));
			break;
		case 8:
		case 9:
		case 12:
		case 13:
			_tcsncpy(fid.backgroundName, LPGENT("Outgoing background(old)"), SIZEOF(fid.backgroundName));
			break;
		case 10:
		case 11:
		case 14:
		case 15:
			_tcsncpy(fid.backgroundName, LPGENT("Incoming background(old)"), SIZEOF(fid.backgroundName));
			break;
		case MSGFONTID_MESSAGEAREA:
			_tcsncpy(fid.group, LPGENT("Message Sessions"), SIZEOF(fid.group));
			_tcsncpy(fid.backgroundGroup, LPGENT("Message Sessions"), SIZEOF(fid.backgroundGroup));
			_tcsncpy(fid.backgroundName, LPGENT("Input area background"), SIZEOF(fid.backgroundName));
			fid.flags |= FIDF_DISABLESTYLES;
			fid.flags &= ~FIDF_ALLOWEFFECTS;
			break;
		case 17:
			_tcsncpy(fid.backgroundName, LPGENT("Status background"), SIZEOF(fid.backgroundName));
			break;
		case 18:
			_tcsncpy(fid.backgroundGroup, LPGENT("Message Sessions"), SIZEOF(fid.backgroundGroup));
			_tcsncpy(fid.backgroundName, LPGENT("Log background"), SIZEOF(fid.backgroundName));
			break;
		case 19:
			_tcsncpy(fid.backgroundName, _T(""), SIZEOF(fid.backgroundName));
			break;
		default:
			_tcsncpy(fid.backgroundName, LPGENT("Incoming background"), SIZEOF(fid.backgroundName));
			break;
		}
		FontRegisterT(&fid);
	}

	fontOptionsList = IP_fontOptionsList;
	fid.flags = FIDF_DEFAULTVALID | FIDF_ALLOWEFFECTS;
	_tcsncpy(fid.group, LPGENT("Message Sessions")_T("/")LPGENT("Info Panel"), SIZEOF(fid.group));
	_tcsncpy(fid.backgroundGroup, LPGENT("Message Sessions")_T("/")LPGENT("Info Panel"), SIZEOF(fid.backgroundGroup));
	_tcsncpy(fid.backgroundName, LPGENT("Fields background"), SIZEOF(fid.backgroundName));
	for (int i = 0; i < IPFONTCOUNT; i++) {
		LoadMsgDlgFont(FONTSECTION_IP, i + 100, &lf, &fontOptionsList[i].colour, FONTMODULE);
		mir_snprintf(szTemp, SIZEOF(szTemp), "Font%d", i + 100);
		strncpy(fid.prefix, szTemp, SIZEOF(fid.prefix));
		fid.order = i + 100;
		_tcsncpy(fid.name, fontOptionsList[i].szDescr, SIZEOF(fid.name));
		fid.deffontsettings.colour = fontOptionsList[i].colour;
		fid.deffontsettings.size = (char)lf.lfHeight;
		fid.deffontsettings.style = (lf.lfWeight >= FW_BOLD ? FONTF_BOLD : 0) | (lf.lfItalic ? FONTF_ITALIC : 0);
		fid.deffontsettings.charset = lf.lfCharSet;
		fid.flags = fid.flags & ~FIDF_CLASSMASK | (fid.deffontsettings.style&FONTF_BOLD ? FIDF_CLASSHEADER : FIDF_CLASSGENERAL);
		fid.deffontsettings.charset = lf.lfCharSet;
		_tcsncpy(fid.deffontsettings.szFace, lf.lfFaceName, LF_FACESIZE);
		if (i == IPFONTCOUNT - 1) {
			_tcsncpy(fid.backgroundGroup, _T(""), SIZEOF(fid.backgroundGroup));
			_tcsncpy(fid.backgroundName, _T(""), SIZEOF(fid.backgroundName));
			_tcsncpy(fid.group, LPGENT("Message Sessions"), SIZEOF(fid.group));
		}
		FontRegisterT(&fid);
	}

	_tcsncpy(cid.group, LPGENT("Message Sessions")_T("/")LPGENT("Group chats"), SIZEOF(cid.group));
	strncpy(cid.dbSettingsGroup, CHAT_MODULE, SIZEOF(cid.dbSettingsGroup));
	for (int i = 0; i <= 7; i++) {
		mir_snprintf(szTemp, SIZEOF(szTemp), "NickColor%d", i);
		_tcsncpy(cid.name, chatcolorsnames[i], SIZEOF(cid.name));
		cid.order = i + 1;
		strncpy(cid.setting, szTemp, SIZEOF(cid.setting));
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
		ColourRegisterT(&cid);
	}
	cid.order++;
	_tcsncpy_s(cid.name, LPGENT("Nick list background"), _TRUNCATE);
	strncpy_s(cid.setting, "ColorNicklistBG", _TRUNCATE);
	cid.defcolour = SRMSGDEFSET_BKGCOLOUR;
	ColourRegisterT(&cid);

	cid.order++;
	_tcsncpy_s(cid.name, LPGENT("Group chat log background"), _TRUNCATE);
	strncpy_s(cid.setting, "ColorLogBG", _TRUNCATE);
	ColourRegisterT(&cid);

	// static colors (info panel, tool bar background etc...)
	strncpy(fid.dbSettingsGroup, FONTMODULE, SIZEOF(fid.dbSettingsGroup));
	strncpy(cid.dbSettingsGroup, FONTMODULE, SIZEOF(fid.dbSettingsGroup));

	for (int i = 0; i < SIZEOF(_clrs); i++) {
		cid.order = _clrs[i].order;
		_tcsncpy(cid.group, _clrs[i].tszGroup, SIZEOF(fid.group));
		_tcsncpy(cid.name, _clrs[i].tszName, SIZEOF(cid.name));
		strncpy(cid.setting, _clrs[i].szSetting, SIZEOF(cid.setting));
		if (_clrs[i].def & 0xff000000)
			cid.defcolour = GetSysColor(_clrs[i].def & 0x000000ff);
		else
			cid.defcolour = _clrs[i].def;
		ColourRegisterT(&cid);
	}

	strncpy(cid.dbSettingsGroup, SRMSGMOD_T, SIZEOF(fid.dbSettingsGroup));

	// text and background colors for tabs
	for (int i = 0; i < SIZEOF(_tabclrs); i++) {
		cid.order = _tabclrs[i].order;
		_tcsncpy(cid.group, _tabclrs[i].tszGroup, SIZEOF(fid.group));
		_tcsncpy(cid.name, _tabclrs[i].tszName, SIZEOF(cid.name));
		strncpy(cid.setting, _tabclrs[i].szSetting, SIZEOF(cid.setting));
		if (_tabclrs[i].def & 0xff000000)
			cid.defcolour = GetSysColor(_tabclrs[i].def & 0x000000ff);
		else
			cid.defcolour = _tabclrs[i].def;

		ColourRegisterT(&cid);
	}
}

int FontServiceFontsChanged(WPARAM,LPARAM)
{
	PluginConfig.reloadSettings();
	CSkin::initAeroEffect();
	CacheMsgLogIcons();
	CacheLogFonts();
	FreeTabConfig();
	ReloadTabConfig();
	Skin->setupAeroSkins();
	M.BroadcastMessage(DM_OPTIONSAPPLIED, 1, 0);
	return 0;
}

static UINT _o2chatcontrols[] = 
{
	IDC_CHAT_SPIN2, IDC_LIMIT, IDC_CHAT_SPIN4, IDC_LOGTIMESTAMP, IDC_TIMESTAMP,
	IDC_OUTSTAMP, IDC_FONTCHOOSE, IDC_LOGGING, IDC_LOGDIRECTORY, IDC_INSTAMP, IDC_CHAT_SPIN2, IDC_CHAT_SPIN3, IDC_NICKROW2, IDC_LOGLIMIT,
	IDC_STATIC110, IDC_STATIC112, 0
};

static UINT _o3chatcontrols[] = {0};

INT_PTR CALLBACK DlgProcOptions2(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN2, UDM_SETRANGE, 0, MAKELONG(5000, 0));
			SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN2, UDM_SETPOS, 0, MAKELONG(db_get_w(NULL, CHAT_MODULE, "LogLimit", 100), 0));
			SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN3, UDM_SETRANGE, 0, MAKELONG(255, 10));
			SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN3, UDM_SETPOS, 0, MAKELONG(M.GetByte(CHAT_MODULE, "NicklistRowDist", 12), 0));
			SetDlgItemText(hwndDlg, IDC_LOGTIMESTAMP, g_Settings.pszTimeStampLog);
			SetDlgItemText(hwndDlg, IDC_TIMESTAMP, g_Settings.pszTimeStamp);
			SetDlgItemText(hwndDlg, IDC_OUTSTAMP, g_Settings.pszOutgoingNick);
			SetDlgItemText(hwndDlg, IDC_INSTAMP, g_Settings.pszIncomingNick);
			CheckDlgButton(hwndDlg, IDC_LOGGING, g_Settings.bLoggingEnabled);
			SetDlgItemText(hwndDlg, IDC_LOGDIRECTORY, g_Settings.pszLogDir);
			Utils::enableDlgControl(hwndDlg, IDC_LOGDIRECTORY, g_Settings.bLoggingEnabled);
			Utils::enableDlgControl(hwndDlg, IDC_FONTCHOOSE, g_Settings.bLoggingEnabled);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN4, UDM_SETRANGE, 0, MAKELONG(10000, 0));
			SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN4, UDM_SETPOS, 0, MAKELONG(db_get_w(NULL, CHAT_MODULE, "LoggingLimit", 100), 0));
			Utils::enableDlgControl(hwndDlg, IDC_LIMIT, g_Settings.bLoggingEnabled);

			if (ServiceExists(MS_UTILS_REPLACEVARS)) {
				TCHAR tszTooltipText[2048];

				mir_sntprintf(tszTooltipText, SIZEOF(tszTooltipText),
					_T("%s - %s\n%s - %s\n%s - %s\n%s - %s\n\n")
					_T("%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n\n")
					_T("%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s"),
					// contact vars
					_T("%nick%"),					TranslateT("nick of current contact (if defined)"),
					_T("%proto%"),					TranslateT("protocol name of current contact (if defined). Account name is used when protocol supports multiple accounts"),
					_T("%accountname%"),			TranslateT("user-defined account name of current contact (if defined)."),
					_T("%userid%"),					TranslateT("user ID of current contact (if defined). It is like UIN Number for ICQ, JID for Jabber, etc."),
					// global vars
					_T("%miranda_path%"),			TranslateT("path to Miranda root folder"),
					_T("%miranda_profilesdir%"),		TranslateT("path to folder containing Miranda profiles"),
					_T("%miranda_profilename%"),	TranslateT("name of current Miranda profile (filename, without extension)"),
					_T("%miranda_userdata%"),		TranslateT("will return parsed string %miranda_profilesdir%\\%miranda_profilename%"),
					_T("%miranda_logpath%"),		TranslateT("will return parsed string %miranda_userdata%\\Logs"),
					_T("%appdata%"),				TranslateT("same as environment variable %APPDATA% for currently logged-on Windows user"),
					_T("%username%"),				TranslateT("username for currently logged-on Windows user"),
					_T("%mydocuments%"),			TranslateT("\"My Documents\" folder for currently logged-on Windows user"),
					_T("%desktop%"),				TranslateT("\"Desktop\" folder for currently logged-on Windows user"),
					_T("%xxxxxxx%"),				TranslateT("any environment variable defined in current Windows session (like %systemroot%, %allusersprofile%, etc.)"),
					// date/time vars
					_T("%d%"),			TranslateT("day of month, 1-31"),
					_T("%dd%"),			TranslateT("day of month, 01-31"),
					_T("%m%"),			TranslateT("month number, 1-12"),
					_T("%mm%"),			TranslateT("month number, 01-12"),
					_T("%mon%"),		TranslateT("abbreviated month name"),
					_T("%month%"),		TranslateT("full month name"),
					_T("%yy%"),			TranslateT("year without century, 01-99"),
					_T("%yyyy%"),		TranslateT("year with century, 1901-9999"),
					_T("%wday%"),		TranslateT("abbreviated weekday name"),
					_T("%weekday%"),	TranslateT("full weekday name"));
				hPathTip = CreateToolTip(GetDlgItem(hwndDlg, IDC_LOGDIRECTORY), tszTooltipText, TranslateT("Variables"));
			}
		}
		if (hPathTip)
			SetTimer(hwndDlg, 0, 3000, NULL);
		break;

	case WM_COMMAND:
		if ((LOWORD(wParam)	== IDC_INSTAMP
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
				TCHAR	tszTemp[MAX_PATH + 20];
				mir_sntprintf(tszTemp, MAX_PATH + 20, _T("%s"), g_Settings.pszLogDir);

				TCHAR *p = tszTemp;
				while(*p && (*p == '\\' || *p == '.'))
					p++;

				if (*p)
					if (TCHAR *p1 = _tcschr(p, '\\'))
						*p1 = 0;

				TCHAR	tszInitialDir[_MAX_DRIVE + _MAX_PATH + 10];
				mir_sntprintf(tszInitialDir, MAX_PATH, _T("%s%s"), M.getChatLogPath(), p);
				if (!PathFileExists(tszInitialDir))
					mir_sntprintf(tszInitialDir, MAX_PATH, _T("%s"), M.getChatLogPath());

				TCHAR	tszReturnName[MAX_PATH]; tszReturnName[0] = 0;
				mir_sntprintf(tszTemp, SIZEOF(tszTemp), _T("%s%c*.*%c%c"), TranslateT("All Files"), 0, 0, 0);

				OPENFILENAME ofn = { 0 };
				ofn.lpstrInitialDir = tszInitialDir;
				ofn.lpstrFilter = tszTemp;
				ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
				ofn.lpstrFile = tszReturnName;
				ofn.nMaxFile = MAX_PATH;
				ofn.nMaxFileTitle = MAX_PATH;
				ofn.Flags = OFN_HIDEREADONLY | OFN_DONTADDTORECENT;
				ofn.lpstrDefExt = _T("log");
				GetOpenFileName(&ofn);
			}
			break;

		case IDC_FONTCHOOSE:
			{
				TCHAR tszDirectory[MAX_PATH];
				LPMALLOC psMalloc;

				if (SUCCEEDED(CoGetMalloc(1, &psMalloc))) {
					BROWSEINFO bi = { 0 };
					bi.hwndOwner = hwndDlg;
					bi.pszDisplayName = tszDirectory;
					bi.lpszTitle = TranslateT("Select Folder");
					bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX | BIF_RETURNONLYFSDIRS;
					bi.lpfn = BrowseCallbackProc;
					bi.lParam = (LPARAM)tszDirectory;

					LPITEMIDLIST idList = SHBrowseForFolder(&bi);
					if (idList) {
						const TCHAR *szUserDir = M.getUserDir();
						SHGetPathFromIDList(idList, tszDirectory);
						lstrcat(tszDirectory, _T("\\"));

						TCHAR tszTemp[MAX_PATH];
						PathToRelativeT(tszDirectory, tszTemp, szUserDir);
						SetWindowText(GetDlgItem(hwndDlg, IDC_LOGDIRECTORY), lstrlen(tszTemp) > 1 ? tszTemp : DEFLOGFILENAME);
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
			TCHAR *p2 = NULL;
			char  *pszText = NULL;
			TCHAR *ptszPath = NULL;

			int iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_LOGDIRECTORY));
			if (iLen > 0) {
				TCHAR *pszText1 = (TCHAR*)mir_alloc(iLen*sizeof(TCHAR) + 2);
				GetDlgItemText(hwndDlg, IDC_LOGDIRECTORY, pszText1, iLen + 1);
				db_set_ts(NULL, CHAT_MODULE, "LogDirectory", pszText1);
				mir_free(pszText1);
				g_Settings.bLoggingEnabled = IsDlgButtonChecked(hwndDlg, IDC_LOGGING) == BST_CHECKED;
				db_set_b(0, CHAT_MODULE, "LoggingEnabled", g_Settings.bLoggingEnabled);
			}
			else {
				db_unset(NULL, CHAT_MODULE, "LogDirectory");
				db_set_b(0, CHAT_MODULE, "LoggingEnabled", 0);
			}
			pci->SM_InvalidateLogDirectories();

			iLen = SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN4, UDM_GETPOS, 0, 0);
			db_set_w(NULL, CHAT_MODULE, "LoggingLimit", (WORD)iLen);

			iLen = SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN3, UDM_GETPOS, 0, 0);
			if (iLen > 0)
				db_set_b(0, CHAT_MODULE, "NicklistRowDist", (BYTE)iLen);
			else
				db_unset(NULL, CHAT_MODULE, "NicklistRowDist");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_LOGTIMESTAMP));
			if (iLen > 0) {
				pszText = (char *)mir_realloc(pszText, iLen + 1);
				GetDlgItemTextA(hwndDlg, IDC_LOGTIMESTAMP, pszText, iLen + 1);
				db_set_s(NULL, CHAT_MODULE, "LogTimestamp", pszText);
			}
			else db_unset(NULL, CHAT_MODULE, "LogTimestamp");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_TIMESTAMP));
			if (iLen > 0) {
				pszText = (char *)mir_realloc(pszText, iLen + 1);
				GetDlgItemTextA(hwndDlg, IDC_TIMESTAMP, pszText, iLen + 1);
				db_set_s(NULL, CHAT_MODULE, "HeaderTime", pszText);
			}
			else db_unset(NULL, CHAT_MODULE, "HeaderTime");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_INSTAMP));
			if (iLen > 0) {
				pszText = (char *)mir_realloc(pszText, iLen + 1);
				GetDlgItemTextA(hwndDlg, IDC_INSTAMP, pszText, iLen + 1);
				db_set_s(NULL, CHAT_MODULE, "HeaderIncoming", pszText);
			}
			else db_unset(NULL, CHAT_MODULE, "HeaderIncoming");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_OUTSTAMP));
			if (iLen > 0) {
				pszText = (char *)mir_realloc(pszText, iLen + 1);
				GetDlgItemTextA(hwndDlg, IDC_OUTSTAMP, pszText, iLen + 1);
				db_set_s(NULL, CHAT_MODULE, "HeaderOutgoing", pszText);
			}
			else db_unset(NULL, CHAT_MODULE, "HeaderOutgoing");

			iLen = SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN2, UDM_GETPOS, 0, 0);
			db_set_w(NULL, CHAT_MODULE, "LogLimit", (WORD)iLen);
			mir_free(pszText);

			if (pci->hListBkgBrush)
				DeleteObject(pci->hListBkgBrush);
			pci->hListBkgBrush = CreateSolidBrush(M.GetDword(CHAT_MODULE, "ColorNicklistBG", SRMSGDEFSET_BKGCOLOUR));

			pci->ReloadSettings();
			pci->MM_FontsChanged();
			pci->MM_FixColors();
			pci->SM_BroadcastMessage(NULL, GC_SETWNDPROPS, 0, 0, TRUE);

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

#define GC_EVENT_ALL (GC_EVENT_ACTION | GC_EVENT_MESSAGE | GC_EVENT_NICK | GC_EVENT_JOIN | \
	GC_EVENT_PART | GC_EVENT_TOPIC | GC_EVENT_ADDSTATUS | GC_EVENT_INFORMATION | GC_EVENT_QUIT | \
	GC_EVENT_KICK | GC_EVENT_NOTICE)

/**
 * Dialog procedure for group chat options tab #3 (event filter configuration)
 *
 * @return
 */

INT_PTR CALLBACK DlgProcOptions3(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			DWORD dwFilterFlags = M.GetDword(CHAT_MODULE, "FilterFlags", GC_EVENT_ALL);
			DWORD dwTrayFlags = M.GetDword(CHAT_MODULE, "TrayIconFlags", GC_EVENT_HIGHLIGHT);
			DWORD dwPopupFlags = M.GetDword(CHAT_MODULE, "PopupFlags", GC_EVENT_HIGHLIGHT);
			DWORD dwLogFlags = M.GetDword(CHAT_MODULE, "DiskLogFlags", GC_EVENT_ALL);

			for (int i=0; i < SIZEOF(_eventorder); i++) {
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

		CheckDlgButton(hwndDlg, IDC_NOPOPUPSFORCLOSEDWINDOWS, M.GetByte(CHAT_MODULE, "SkipWhenNoWindow", 0) ? BST_CHECKED : BST_UNCHECKED);
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

				for (int i=0; i < SIZEOF(_eventorder); i++) {
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

				db_set_b(0, CHAT_MODULE, "SkipWhenNoWindow", IsDlgButtonChecked(hwndDlg, IDC_NOPOPUPSFORCLOSEDWINDOWS) ? 1 : 0);
				db_set_b(0, CHAT_MODULE, "TrayIconInactiveOnly", IsDlgButtonChecked(hwndDlg, IDC_TRAYONLYFORINACTIVE) ? 1 : 0);

				pci->ReloadSettings();
				pci->MM_FontsChanged();
				pci->SM_BroadcastMessage(NULL, GC_SETWNDPROPS, 0, 0, TRUE);
				SM_ReconfigureFilters();
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}
