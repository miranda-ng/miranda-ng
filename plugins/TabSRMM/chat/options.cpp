/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2009 Miranda ICQ/IM project,
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
 * original chat module for Miranda IM, written and copyrighted
 * by Joergen Persson in 2005.
 *
 * (C) 2005-2009 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: options.cpp 13412 2011-03-08 19:13:11Z george.hazan $
 *
 * group chat options and generic font handling
 *
 */

#include "../src/commonheaders.h"
#undef Translate

#define TranslateA(s)   ((char*)CallService(MS_LANGPACK_TRANSLATESTRING,0,(LPARAM)(s)))
#include <shlobj.h>
#include <shlwapi.h>

extern HBRUSH 			hListBkgBrush;
extern HICON			hIcons[30];
extern FONTINFO			aFonts[OPTIONS_FONTCOUNT];
extern HMODULE          g_hIconDLL;

extern HIMAGELIST       CreateStateImageList();

HANDLE					g_hOptions = NULL;


#define FONTF_BOLD   1
#define FONTF_ITALIC 2

struct FontOptionsList {
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

struct ColorOptionsList {
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
	0, LPGENT("TabSRMM/Group Chats"), LPGENT("Group chat log background"), SRMSGSET_BKGCOLOUR_MUC, SRMSGDEFSET_BKGCOLOUR,
	1, LPGENT("TabSRMM"), LPGENT("Input area background"), "inputbg", SRMSGDEFSET_BKGCOLOUR,
	2, LPGENT("TabSRMM"), LPGENT("Log background"), SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR,
	0, LPGENT("TabSRMM/Single Messaging"), LPGENT("Outgoing background"), "outbg", SRMSGDEFSET_BKGOUTCOLOUR,
	1, LPGENT("TabSRMM/Single Messaging"), LPGENT("Incoming background"), "inbg", SRMSGDEFSET_BKGINCOLOUR,
	2, LPGENT("TabSRMM/Single Messaging"), LPGENT("Status background"), "statbg", SRMSGDEFSET_BKGCOLOUR,
	3, LPGENT("TabSRMM/Single Messaging"), LPGENT("Incoming background(old)"), "oldinbg", SRMSGDEFSET_BKGINCOLOUR,
	4, LPGENT("TabSRMM/Single Messaging"), LPGENT("Outgoing background(old)"), "oldoutbg", SRMSGDEFSET_BKGOUTCOLOUR,
	5, LPGENT("TabSRMM/Single Messaging"), LPGENT("Horizontal Grid Lines"), "hgrid", RGB(224, 224, 224),
	0, LPGENT("TabSRMM/Info Panel"), LPGENT("Panel background low"), "ipfieldsbg", 0x62caff,
	1, LPGENT("TabSRMM/Info Panel"), LPGENT("Panel background high"), "ipfieldsbgHigh", 0xf0f0f0,
	0, LPGENT("TabSRMM/Common colors"), LPGENT("Toolbar background high"), "tbBgHigh", 0,
	1, LPGENT("TabSRMM/Common colors"), LPGENT("Toolbar background low"), "tbBgLow", 0,
	2, LPGENT("TabSRMM/Common colors"), LPGENT("Window fill color"), "fillColor", 0,
	3, LPGENT("TabSRMM/Common colors"), LPGENT("Text area borders"), "cRichBorders", 0,
	4, LPGENT("TabSRMM/Common colors"), LPGENT("Aero glow effect"), "aeroGlow", RGB(40, 40, 255),
	4, LPGENT("TabSRMM/Common colors"), LPGENT("Generic text color (only when fill color is set)"), "genericTxtClr", 0xff000000 | COLOR_BTNTEXT,
};

static ColorOptionsList _tabclrs[] = {
	0, LPGENT("TabSRMM/Tabs"), LPGENT("Normal text"), "tab_txt_normal", 0xff000000 | COLOR_BTNTEXT,
	1, LPGENT("TabSRMM/Tabs"), LPGENT("Active text"), "tab_txt_active", 0xff000000 | COLOR_BTNTEXT,
	2, LPGENT("TabSRMM/Tabs"), LPGENT("Hovered text"), "tab_txt_hottrack", 0xff000000 | COLOR_HOTLIGHT,
	3, LPGENT("TabSRMM/Tabs"), LPGENT("Unread text"), "tab_txt_unread", 0xff000000 | COLOR_HOTLIGHT,

	4, LPGENT("TabSRMM/Tabs"), LPGENT("Normal background"), "tab_bg_normal", 0xff000000 | COLOR_3DFACE,
	5, LPGENT("TabSRMM/Tabs"), LPGENT("Active background"), "tab_bg_active", 0xff000000 | COLOR_3DFACE,
	6, LPGENT("TabSRMM/Tabs"), LPGENT("Hovered background"), "tab_bg_hottrack", 0xff000000 | COLOR_3DFACE,
	7, LPGENT("TabSRMM/Tabs"), LPGENT("Unread background"), "tab_bg_unread", 0xff000000 | COLOR_3DFACE
};

extern LOGFONT lfDefault;

//remember to put these in the Translate( ) template file too
static FontOptionsList CHAT_fontOptionsList[] = {
	{LPGENT("Timestamp"), RGB(50, 50, 240), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("Others nicknames"), RGB(0, 0, 192), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12},
	{LPGENT("Your nickname"), RGB(0, 0, 192), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12},
	{LPGENT("User has joined"), RGB(90, 160, 90), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("User has left"), RGB(160, 160, 90), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("User has disconnected"), RGB(160, 90, 90), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("User kicked ..."), RGB(100, 100, 100), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("User is now known as ..."), RGB(90, 90, 160), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("Notice from user"), RGB(160, 130, 60), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("Incoming message"), RGB(90, 90, 90), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("Outgoing message"), RGB(90, 90, 90), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("The topic is ..."), RGB(70, 70, 160), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("Information messages"), RGB(130, 130, 195), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("User enables status for ..."), RGB(70, 150, 70), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("User disables status for ..."), RGB(150, 70, 70), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("Action message"), RGB(160, 90, 160), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("Highlighted message"), RGB(180, 150, 80), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("Chat log symbols (Webdings)"), RGB(170, 170, 170), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("User list members (Online)"), RGB(0, 0, 0), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("User list members (away)"), RGB(170, 170, 170), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
};

const int msgDlgFontCount = SIZEOF(CHAT_fontOptionsList);

static FontOptionsList IM_fontOptionsList[] = {
	{LPGENT(">> Outgoing messages"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT(">> Outgoing misc events"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("<< Incoming messages"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("<< Incoming misc events"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT(">> Outgoing name"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12},
	{LPGENT(">> Outgoing timestamp"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12},
	{LPGENT("<< Incoming name"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12},
	{LPGENT("<< Incoming timestamp"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12},
	{LPGENT(">> Outgoing messages (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT(">> Outgoing misc events (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("<< Incoming messages (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("<< Incoming misc events (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT(">> Outgoing name (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12},
	{LPGENT(">> Outgoing timestamp (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12},
	{LPGENT("<< Incoming name (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12},
	{LPGENT("<< Incoming timestamp (old)"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, FONTF_BOLD, -12},
	{LPGENT("* Message Input Area"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("* Status changes"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("* Dividers"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("* Error and warning Messages"), RGB(50, 50, 50), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("* Symbols (incoming)"), RGB(50, 50, 50), _T("Webdings"), SYMBOL_CHARSET, 0, -12},
	{LPGENT("* Symbols (outgoing)"), RGB(50, 50, 50), _T("Webdings"), SYMBOL_CHARSET, 0, -12},
};

static FontOptionsList IP_fontOptionsList[] = {
	{LPGENT("Nickname"), RGB(0, 0, 0), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("UIN"), RGB(0, 0, 0), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("Status"), RGB(0, 0, 0), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("Protocol"), RGB(0, 0, 0), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("Contacts local time"), RGB(0, 0, 0), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
	{LPGENT("Window caption (skinned mode)"), RGB(255, 255, 255), lfDefault.lfFaceName, DEFAULT_CHARSET, 0, -12},
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
static struct branch_t branch1[] = {
	{LPGENT("Open new chat rooms in the default container"), "DefaultContainer", 0, 1, NULL},
	{LPGENT("Flash window when someone speaks"), "FlashWindow", 0, 0, NULL},
	{LPGENT("Flash window when a word is highlighted"), "FlashWindowHighlight", 0, 1, NULL},
//TODO Fix for 3.0 !!!

#if !defined(__DELAYED_FOR_3_1)
	{LPGENT("Create tabs or windows for highlight events"), "CreateWindowOnHighlight", 0,0, NULL},
	{LPGENT("Activate chat window on highlight"), "AnnoyingHighlight", 0,0, NULL},
#endif

	{LPGENT("Show list of users in the chat room"), "ShowNicklist", 0, 1, NULL},
	{LPGENT("Colorize nicknames in member list"), "ColorizeNicks", 0, 1, NULL},
	{LPGENT("Show button menus when right clicking the buttons"), "RightClickFilter", 0, 0, NULL},
	{LPGENT("Show topic as status message on the contact list"), "TopicOnClist", 0, 1, NULL},
	{LPGENT("Do not pop up the window when joining a chat room"), "PopupOnJoin", 0, 0, NULL},
	{LPGENT("Hide or show the window by double click in the contact list"), "ToggleVisibility", 0, 0, NULL},
	{LPGENT("Sync splitter position with standard IM sessions"), "SyncSplitter", 0, 0, NULL},
	{LPGENT("Show contact's status modes if supported by the protocol"), "ShowContactStatus", 0, 1, NULL},
	{LPGENT("Display contact's status icon before user role icon"), "ContactStatusFirst", 0, 0, NULL},
	//MAD: simple customization improvement
	{LPGENT("Use IRC style status indicators in the user list"), "ClassicIndicators", 0, 0, NULL},
	{LPGENT("Use alternative sorting method in member list"), "AlternativeSorting", 0, 1, NULL},

};
static struct branch_t branch2[] = {
	{LPGENT("Prefix all events with a timestamp"), "ShowTimeStamp", 0, 1, NULL},
	{LPGENT("Timestamp only when event time differs"), "ShowTimeStampIfChanged", 0, 0, NULL},
	{LPGENT("Timestamp has same color as the event"), "TimeStampEventColour", 0, 0, NULL},
	{LPGENT("Indent the second line of a message"), "LogIndentEnabled", 0, 1, NULL},
	{LPGENT("Limit user names in the message log to 20 characters"), "LogLimitNames", 0, 1, NULL},
	{LPGENT("Add a colon (:) to auto-completed user names"), "AddColonToAutoComplete", 0, 1, NULL},
	{LPGENT("Start private conversation on doubleclick in nick list (insert nick if unchecked)"), "DoubleClick4Privat", 0, 0, NULL},
	{LPGENT("Strip colors from messages in the log"), "StripFormatting", 0, 0, NULL},
	{LPGENT("Enable the \'event filter\' for new rooms"), "FilterEnabled", 0, 0, NULL},
//MAD: simple customization improvement
	{LPGENT("Use IRC style status indicators in the log"), "LogClassicIndicators", 0,0, NULL},
//
	{LPGENT("Allow clickable user names in the message log"), "ClickableNicks", 0, 1, NULL},
	{LPGENT("Colorize user names in message log"), "ColorizeNicksInLog", 0, 1, NULL},
	{LPGENT("Scale down icons to 10x10 pixels in the chat log"), "ScaleIcons", 0, 1, NULL},
	{LPGENT("Place a separator in the log after a window lost its foreground status"), "UseDividers", 0, 1, NULL},
	{LPGENT("Only place a separator when an incoming event is announced with a popup"), "DividersUsePopupConfig", 0, 1, NULL},
	{LPGENT("Support the math module plugin"), "MathModSupport", 0, 0, NULL}
};

static HWND hPathTip = 0;

void LoadMsgDlgFont(int section, int i, LOGFONT *lf, COLORREF* colour, char *szMod)
{
	char str[32];
	int style;
	DBVARIANT dbv;
	int j = (i >= 100 ? i - 100 : i);

	struct FontOptionsList *fol = fontOptionsList;
	switch (section)
	{
		case FONTSECTION_CHAT:	fol = CHAT_fontOptionsList;	break;
		case FONTSECTION_IM:	fol = IM_fontOptionsList;	break;
		case FONTSECTION_IP:	fol = IP_fontOptionsList;	break;
	}

	if (colour) {
		wsprintfA(str, "Font%dCol", i);
		*colour = M->GetDword(szMod, str, fol[j].defColour);
	}
	if (lf) {
		wsprintfA(str, "Font%dSize", i);
		lf->lfHeight = (char) M->GetByte(szMod, str, fol[j].defSize);
		lf->lfWidth = 0;
		lf->lfEscapement = 0;
		lf->lfOrientation = 0;
		wsprintfA(str, "Font%dSty", i);
		style = M->GetByte(szMod, str, fol[j].defStyle);
		if(i == MSGFONTID_MESSAGEAREA && section == FONTSECTION_IM && M->GetByte(0, SRMSGMOD_T, "inputFontFix", 1) == 1) {
			lf->lfWeight = FW_NORMAL;
			lf->lfItalic = 0;
			lf->lfUnderline = 0;
			lf->lfStrikeOut = 0;
		}
		else {
			lf->lfWeight = style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
			lf->lfItalic = style & FONTF_ITALIC ? 1 : 0;
			lf->lfUnderline = style & FONTF_UNDERLINE ? 1 : 0;
			lf->lfStrikeOut =  style & FONTF_STRIKEOUT ? 1 : 0;
		}
		wsprintfA(str, "Font%dSet", i);
		lf->lfCharSet = M->GetByte(szMod, str, fol[j].defCharset);
		lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf->lfQuality = DEFAULT_QUALITY;
		lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		wsprintfA(str, "Font%d", i);
		if ((i == 17 && !strcmp(szMod, CHAT_FONTMODULE)) || ((i == 20 || i == 21) && !strcmp(szMod, FONTMODULE))) {
			lf->lfCharSet = SYMBOL_CHARSET;
			lstrcpyn(lf->lfFaceName, _T("Webdings"), SIZEOF(lf->lfFaceName));
		} else {
			if (M->GetTString(NULL, szMod, str, &dbv)) {
				lstrcpy(lf->lfFaceName, fol[j].szDefFace);
			} else {
				lstrcpyn(lf->lfFaceName, dbv.ptszVal, SIZEOF(lf->lfFaceName));
				DBFreeVariant(&dbv);
			}
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

static void FillBranch(HWND hwndTree, HTREEITEM hParent, struct branch_t *branch, int nValues, DWORD defaultval)
{
	TVINSERTSTRUCT tvis;
	int i;
	int iState;

	if (hParent == 0)
		return;

	tvis.hParent = hParent;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT | TVIF_STATE;
	for (i = 0;i < nValues;i++) {
		tvis.item.pszText = TranslateTS(branch[i].szDescr);
		tvis.item.stateMask = TVIS_STATEIMAGEMASK;
		if (branch[i].iMode)
			iState = ((M->GetDword("Chat", branch[i].szDBName, defaultval) & branch[i].iMode) & branch[i].iMode) != 0 ? 3 : 2;
		else
			iState = M->GetByte("Chat", branch[i].szDBName, branch[i].bDefault) != 0 ? 3 : 2;
		tvis.item.state = INDEXTOSTATEIMAGEMASK(iState);
		branch[i].hItem = TreeView_InsertItem(hwndTree, &tvis);
	}
}

static void SaveBranch(HWND hwndTree, struct branch_t *branch, int nValues)
{
	TVITEM tvi;
	BYTE bChecked;
	int i;
	int iState = 0;

	tvi.mask = TVIF_HANDLE | TVIF_STATE;
	for (i = 0;i < nValues;i++) {
		tvi.hItem = branch[i].hItem;
		TreeView_GetItem(hwndTree, &tvi);
		bChecked = ((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 2) ? 0 : 1;
		if (branch[i].iMode) {
			if (bChecked)
				iState |= branch[i].iMode;
			if (iState & GC_EVENT_ADDSTATUS)
				iState |= GC_EVENT_REMOVESTATUS;
			M->WriteDword("Chat", branch[i].szDBName, (DWORD)iState);
		} else M->WriteByte("Chat", branch[i].szDBName, bChecked);
	}
}

static void CheckHeading(HWND hwndTree, HTREEITEM hHeading)
{
	BOOL bChecked = TRUE;
	TVITEM tvi;

	if (hHeading == 0)
		return;

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
	BOOL bChecked = TRUE;
	TVITEM tvi;

	if (hHeading == 0)
		return;

	tvi.mask = TVIF_HANDLE | TVIF_STATE;
	tvi.hItem = hHeading;
	TreeView_GetItem(hwndTree, &tvi);
	if (((tvi.state&TVIS_STATEIMAGEMASK) >> 12 == 3)||((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 1))
		bChecked = FALSE;

	tvi.stateMask = TVIS_STATEIMAGEMASK;
	tvi.state = INDEXTOSTATEIMAGEMASK(bChecked ? 2 : 1);
	TreeView_SetItem(hwndTree, &tvi);
	tvi.hItem = TreeView_GetNextItem(hwndTree, hHeading, TVGN_CHILD);
	while (tvi.hItem) {
		tvi.state = INDEXTOSTATEIMAGEMASK(bChecked ? 3:2);
		if (tvi.hItem != branch1[0].hItem && tvi.hItem != branch1[1].hItem)
			TreeView_SetItem(hwndTree, &tvi);
		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
}

static INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
	char szDir[MAX_PATH];
	switch (uMsg) {
		case BFFM_INITIALIZED: {
			const TCHAR *szData = M->getUserDir();
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)szData);
			break;
		}
		case BFFM_SELCHANGED:
			if (SHGetPathFromIDListA((LPITEMIDLIST) lp , szDir))
				SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)szDir);
			break;
	}
	return 0;
}

static void LoadLogFonts(void)
{
	int i;

	for (i = 0; i < OPTIONS_FONTCOUNT; i++)
		LoadMsgDlgFont(FONTSECTION_CHAT, i, &aFonts[i].lf, &aFonts[i].color, CHAT_FONTMODULE);
}

static struct _tagicons { char *szDesc; char *szName; int id; UINT size;} _icons[] = {
	LPGEN("Window Icon"), "chat_window", IDI_CHANMGR, 16,
	LPGEN("Icon overlay"), "chat_overlay", IDI_OVERLAY, 16,

	LPGEN("Status 1 (10x10)"), "chat_status0", IDI_STATUS0, 16,
	LPGEN("Status 2 (10x10)"), "chat_status1", IDI_STATUS1, 16,
	LPGEN("Status 3 (10x10)"), "chat_status2", IDI_STATUS2, 16,
	LPGEN("Status 4 (10x10)"), "chat_status3", IDI_STATUS3, 16,
	LPGEN("Status 5 (10x10)"), "chat_status4", IDI_STATUS4, 16,
	LPGEN("Status 6 (10x10)"), "chat_status5", IDI_STATUS5, 16,
	NULL, NULL, -1, 0
};

static struct _tag1icons { char *szDesc; char *szName; int id; UINT size;} _logicons[] = {
	LPGEN("Message in (10x10)"), "chat_log_message_in", IDI_MESSAGE, 16,
	LPGEN("Message out (10x10)"), "chat_log_message_out", IDI_MESSAGEOUT, 16,
	LPGEN("Action (10x10)"), "chat_log_action", IDI_ACTION, 16,
	LPGEN("Add Status (10x10)"), "chat_log_addstatus", IDI_ADDSTATUS, 16,
	LPGEN("Remove Status (10x10)"), "chat_log_removestatus", IDI_REMSTATUS, 16,
	LPGEN("Join (10x10)"), "chat_log_join", IDI_JOIN, 16,
	LPGEN("Leave (10x10)"), "chat_log_part", IDI_PART, 16,
	LPGEN("Quit (10x10)"), "chat_log_quit", IDI_QUIT, 16,
	LPGEN("Kick (10x10)"), "chat_log_kick", IDI_KICK, 16,
	LPGEN("Notice (10x10)"), "chat_log_notice", IDI_NOTICE, 16,
	LPGEN("Nickchange (10x10)"), "chat_log_nick", IDI_NICK, 16,
	LPGEN("Topic (10x10)"), "chat_log_topic", IDI_TOPIC, 16,
	LPGEN("Highlight (10x10)"), "chat_log_highlight", IDI_HIGHLIGHT, 16,
	LPGEN("Information (10x10)"), "chat_log_info", IDI_INFO, 16,
	NULL, NULL, 0, 0
};

// add icons to the skinning module
void Chat_AddIcons(void)
{
	if (ServiceExists(MS_SKIN2_ADDICON)) {
		SKINICONDESC sid = {0};
		char szFile[MAX_PATH];
		int i = 0;

		// 16x16 icons
		sid.cbSize = sizeof(SKINICONDESC);
		sid.pszSection = TranslateA("TabSRMM/Group chat windows");
		GetModuleFileNameA(g_hIconDLL, szFile, MAX_PATH);
		sid.pszDefaultFile = szFile;

		while (_icons[i].szDesc != NULL) {
			sid.cx = sid.cy = _icons[i].size;
			sid.pszDescription = _icons[i].szDesc;
			sid.pszName = _icons[i].szName;
			sid.iDefaultIndex = -_icons[i].id;
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
			i++;
		}
		i = 0;
		sid.pszSection = TranslateA("TabSRMM/Group chat log");
		while (_logicons[i].szDesc != NULL) {
			sid.cx = sid.cy = _logicons[i].size;
			sid.pszDescription = _logicons[i].szDesc;
			sid.pszName = _logicons[i].szName;
			sid.iDefaultIndex = -_logicons[i].id;
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
			i++;
		}
	}
	return;
}

/*
 * get icon by name from the core icon library service
 */

HICON LoadIconEx(int iIndex, char * pszIcoLibName, int iX, int iY)
{
	char szTemp[256];
	mir_snprintf(szTemp, sizeof(szTemp), "chat_%s", pszIcoLibName);
	return (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)szTemp);
}

static void InitSetting(TCHAR** ppPointer, char* pszSetting, TCHAR* pszDefault)
{
	DBVARIANT dbv;
	if (!M->GetTString(NULL, "Chat", pszSetting, &dbv)) {
		replaceStr(ppPointer, dbv.ptszVal);
		DBFreeVariant(&dbv);
	} else
		replaceStr(ppPointer, pszDefault);
}

#define OPT_FIXHEADINGS (WM_USER+1)

static UINT _o1controls[] = {IDC_CHECKBOXES, IDC_GROUP, IDC_STATIC_ADD, 0};

HWND CreateToolTip(HWND hwndParent, LPTSTR ptszText, LPTSTR ptszTitle)
{
	TOOLINFO ti = { 0 };
	HWND hwndTT;
	hwndTT = CreateWindowEx(WS_EX_TOPMOST,
		TOOLTIPS_CLASS, NULL,
		WS_POPUP | TTS_NOPREFIX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		hwndParent, NULL, g_hInst, NULL);

	SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS | TTF_CENTERTIP;
	ti.hwnd = hwndParent;
	ti.hinst = g_hInst;
	ti.lpszText = ptszText;
	GetClientRect (hwndParent, &ti.rect);
	ti.rect.left =- 85;

	SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
	SendMessage(hwndTT, TTM_SETTITLE, 1, (LPARAM)ptszTitle);
	return hwndTT;
}

INT_PTR CALLBACK DlgProcOptions1(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HTREEITEM hListHeading1 = 0;
	static HTREEITEM hListHeading2 = 0;

	switch (uMsg) {
		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);
			if (PluginConfig.m_chat_enabled) {
				HIMAGELIST himlOptions;

				SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CHECKBOXES), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CHECKBOXES), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);

				himlOptions = (HIMAGELIST)SendDlgItemMessage(hwndDlg, IDC_CHECKBOXES, TVM_SETIMAGELIST, TVSIL_STATE, (LPARAM)CreateStateImageList());
				ImageList_Destroy(himlOptions);

				hListHeading1 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), const_cast<TCHAR *>(CTranslator::getOpt(CTranslator::OPT_MUC_OPTHEADER1)), TRUE);
				hListHeading2 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), const_cast<TCHAR *>(CTranslator::getOpt(CTranslator::OPT_MUC_OPTHEADER2)), TRUE);

				FillBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading1, branch1, SIZEOF(branch1), 0x0000);
				FillBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading2, branch2, SIZEOF(branch2), 0x0000);

				{
					TCHAR* pszGroup = NULL;
					InitSetting(&pszGroup, "AddToGroup", _T("Chat rooms"));
					SetWindowText(GetDlgItem(hwndDlg, IDC_GROUP), pszGroup);
					mir_free(pszGroup);
					Utils::showDlgControl(hwndDlg, IDC_STATIC_MESSAGE, SW_HIDE);
				}
			} else {
				int i = 0;

				while (_o1controls[i])
					Utils::showDlgControl(hwndDlg, _o1controls[i++], SW_HIDE);
			}
			break;

		case WM_COMMAND:
			if ((LOWORD(wParam) == IDC_GROUP)
					&& (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))	return 0;

			if (lParam != (LPARAM)NULL)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case WM_NOTIFY: {
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
								TVITEM tvi = {0};

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
								} else if (hti.flags&TVHT_ONITEMSTATEICON) {
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
						case PSN_APPLY: {
							if (PluginConfig.m_chat_enabled) {
								int iLen;
								TCHAR *pszText = NULL;
								BYTE b;

								iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_GROUP));
								if (iLen > 0) {
									pszText = (TCHAR *)realloc(pszText, (iLen + 2) * sizeof(TCHAR));
									GetDlgItemText(hwndDlg, IDC_GROUP, pszText, iLen + 1);
									M->WriteTString(NULL, "Chat", "AddToGroup", pszText);
								} else
									M->WriteTString(NULL, "Chat", "AddToGroup", _T(""));

								g_Settings.hGroup = 0;

								if (pszText)
									free(pszText);

								b = M->GetByte("Chat", "Tabs", 1);
								SaveBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), branch1, sizeof(branch1) / sizeof(branch1[0]));
								SaveBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), branch2, sizeof(branch2) / sizeof(branch2[0]));

								LoadGlobalSettings();
								MM_FontsChanged();
								FreeMsgLogBitmaps();
								LoadMsgLogBitmaps();
								SM_BroadcastMessage(NULL, GC_SETWNDPROPS, 0, 0, TRUE);
								SM_ReconfigureFilters();
							}
						}
						return TRUE;
					}
			}
		}
		break;
		case WM_DESTROY: {
			BYTE b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading1, TVIS_EXPANDED) & TVIS_EXPANDED ? 1 : 0;
			M->WriteByte("Chat", "Branch1Exp", b);
			b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading2, TVIS_EXPANDED) & TVIS_EXPANDED ? 1 : 0;
			M->WriteByte("Chat", "Branch2Exp", b);
		}
		break;

		default:
			break;
	}
	return FALSE;
}

static TCHAR* chatcolorsnames[] ={
//	LPGENT("Your nickname"),
	LPGENT("Channel operators"),
	LPGENT("Half operators"),
	LPGENT("Voiced"),
//	LPGENT("Others nicknames"),
	LPGENT("Extended mode 1"),
	LPGENT("Extended mode 2"),
	LPGENT("Selection background"),
	LPGENT("Selected text"),
	LPGENT("Incremental search highlight")
	};

void RegisterFontServiceFonts() {
	int i;
	char szTemp[100];
	LOGFONT lf;
	FontIDT fid = {0};
	ColourIDT cid = {0};

	fid.cbSize = sizeof(FontIDT);
	cid.cbSize = sizeof(ColourIDT);

	strncpy(fid.dbSettingsGroup, FONTMODULE, SIZEOF(fid.dbSettingsGroup));

	for (i = 0; i < SIZEOF(IM_fontOptionsList); i++) {
		fid.flags = FIDF_DEFAULTVALID|FIDF_ALLOWEFFECTS;
		LoadMsgDlgFont(FONTSECTION_IM, i , &lf, &fontOptionsList[i].colour, FONTMODULE);
		mir_snprintf(szTemp, SIZEOF(szTemp), "Font%d", i);
		strncpy(fid.prefix, szTemp, SIZEOF(fid.prefix));
		fid.order = i;
		_tcsncpy(fid.name, fontOptionsList[i].szDescr, SIZEOF(fid.name));
		fid.deffontsettings.colour = fontOptionsList[i].colour;
		fid.deffontsettings.size = (char) lf.lfHeight;
		fid.deffontsettings.style = (lf.lfWeight >= FW_BOLD ? FONTF_BOLD : 0) | (lf.lfItalic ? FONTF_ITALIC : 0);
		fid.deffontsettings.charset = lf.lfCharSet;
		fid.flags = fid.flags & ~FIDF_CLASSMASK | (fid.deffontsettings.style&FONTF_BOLD ? FIDF_CLASSHEADER : FIDF_CLASSGENERAL);
		_tcsncpy(fid.deffontsettings.szFace, lf.lfFaceName, LF_FACESIZE);
		_tcsncpy(fid.backgroundGroup, _T("TabSRMM/Single Messaging"), SIZEOF(fid.backgroundGroup));
		_tcsncpy(fid.group, _T("TabSRMM/Single Messaging"), SIZEOF(fid.group));
		switch (i) {
			case MSGFONTID_MYMSG:
			case 1:
			case MSGFONTID_MYNAME:
			case MSGFONTID_MYTIME:
			case 21:
				_tcsncpy(fid.backgroundName, _T("Outgoing background"), SIZEOF(fid.backgroundName));
				break;
			case 8:
			case 9:
			case 12:
			case 13:
				_tcsncpy(fid.backgroundName, _T("Outgoing background(old)"), SIZEOF(fid.backgroundName));
				break;
			case 10:
			case 11:
			case 14:
			case 15:
				_tcsncpy(fid.backgroundName, _T("Incoming background(old)"), SIZEOF(fid.backgroundName));
				break;
			case MSGFONTID_MESSAGEAREA:
				_tcsncpy(fid.group, _T("TabSRMM"), SIZEOF(fid.group));
				_tcsncpy(fid.backgroundGroup, _T("TabSRMM"), SIZEOF(fid.backgroundGroup));
				_tcsncpy(fid.backgroundName, _T("Input area background"), SIZEOF(fid.backgroundName));
				fid.flags |= FIDF_DISABLESTYLES;
				fid.flags &= ~FIDF_ALLOWEFFECTS;
				break;
			case 17:
				_tcsncpy(fid.backgroundName, _T("Status background"), SIZEOF(fid.backgroundName));
				break;
			case 18:
				_tcsncpy(fid.backgroundGroup, _T("TabSRMM"), SIZEOF(fid.backgroundGroup));
				_tcsncpy(fid.backgroundName, _T("Log background"), SIZEOF(fid.backgroundName));
				break;
 			case 19:
 				_tcsncpy(fid.backgroundName, _T(""), SIZEOF(fid.backgroundName));
 				break;
			default:
				_tcsncpy(fid.backgroundName, _T("Incoming background"), SIZEOF(fid.backgroundName));
				break;
			}
		CallService(MS_FONT_REGISTERT, (WPARAM)&fid, 0);
		}

	fontOptionsList = IP_fontOptionsList;
	fid.flags = FIDF_DEFAULTVALID|FIDF_ALLOWEFFECTS;
	//fid.flags|=FIDF_SAVEPOINTSIZE;
	_tcsncpy(fid.group, _T("TabSRMM/Info Panel"), SIZEOF(fid.group));
	_tcsncpy(fid.backgroundGroup, _T("TabSRMM/Info Panel"), SIZEOF(fid.backgroundGroup));
	_tcsncpy(fid.backgroundName, _T("Fields background"), SIZEOF(fid.backgroundName));
	for (i =0; i < IPFONTCOUNT; i++) {
		LoadMsgDlgFont(FONTSECTION_IP, i + 100 , &lf, &fontOptionsList[i].colour, FONTMODULE);
		mir_snprintf(szTemp, SIZEOF(szTemp), "Font%d", i + 100);
		strncpy(fid.prefix, szTemp, SIZEOF(fid.prefix));
		fid.order = i + 100 ;
		_tcsncpy(fid.name, fontOptionsList[i].szDescr, SIZEOF(fid.name));
		fid.deffontsettings.colour = fontOptionsList[i].colour;
		fid.deffontsettings.size = (char) lf.lfHeight;
		fid.deffontsettings.style = (lf.lfWeight >= FW_BOLD ? FONTF_BOLD : 0) | (lf.lfItalic ? FONTF_ITALIC : 0);
		fid.deffontsettings.charset = lf.lfCharSet;
		fid.flags = fid.flags & ~FIDF_CLASSMASK | (fid.deffontsettings.style&FONTF_BOLD ? FIDF_CLASSHEADER : FIDF_CLASSGENERAL);
		fid.deffontsettings.charset = lf.lfCharSet;
		_tcsncpy(fid.deffontsettings.szFace, lf.lfFaceName, LF_FACESIZE);
		if(i==IPFONTCOUNT-1){
			_tcsncpy(fid.backgroundGroup, _T(""), SIZEOF(fid.backgroundGroup));
			_tcsncpy(fid.backgroundName, _T(""), SIZEOF(fid.backgroundName));
			_tcsncpy(fid.group, _T("TabSRMM"), SIZEOF(fid.group));
			}
		CallService(MS_FONT_REGISTERT, (WPARAM)&fid, 0);
		}

	fontOptionsList = CHAT_fontOptionsList;
	fid.flags = FIDF_DEFAULTVALID|FIDF_ALLOWEFFECTS;
	fid.flags&=~FIDF_SAVEPOINTSIZE;
	_tcsncpy(fid.group, _T("TabSRMM/Group Chats"), SIZEOF(fid.group));
	strncpy(fid.dbSettingsGroup, CHAT_FONTMODULE, SIZEOF(fid.dbSettingsGroup));
	for (i = 0; i < msgDlgFontCount; i++) {
		LoadMsgDlgFont(FONTSECTION_CHAT, i , &lf, &fontOptionsList[i].colour, CHAT_FONTMODULE);
		mir_snprintf(szTemp, SIZEOF(szTemp), "Font%d", i);
		strncpy(fid.prefix, szTemp, SIZEOF(fid.prefix));
		fid.order = i;
		_tcsncpy(fid.name, fontOptionsList[i].szDescr, SIZEOF(fid.name));
		fid.deffontsettings.colour = fontOptionsList[i].colour;
		fid.deffontsettings.size = (char) lf.lfHeight;
		fid.deffontsettings.style = (lf.lfWeight >= FW_BOLD ? FONTF_BOLD : 0) | (lf.lfItalic ? FONTF_ITALIC : 0);
		fid.flags = fid.flags & ~FIDF_CLASSMASK | (fid.deffontsettings.style&FONTF_BOLD ? FIDF_CLASSHEADER : FIDF_CLASSGENERAL);
		fid.deffontsettings.charset = lf.lfCharSet;
		_tcsncpy(fid.deffontsettings.szFace, lf.lfFaceName, LF_FACESIZE);
		_tcsncpy(fid.backgroundGroup, _T("TabSRMM/Group Chats"), SIZEOF(fid.backgroundGroup));
		_tcsncpy(fid.backgroundName, _T("Group chat log background"), SIZEOF(fid.backgroundName));
		if(i == 18 || i == 19)
			_tcsncpy(fid.backgroundName, _T("Userlist background"), SIZEOF(fid.backgroundName));
		CallService(MS_FONT_REGISTERT, (WPARAM)&fid, 0);
	}

	_tcsncpy(cid.group, _T("TabSRMM/Group Chats"), SIZEOF(cid.group));
	strncpy(cid.dbSettingsGroup, "Chat", SIZEOF(cid.dbSettingsGroup));
	for (i = 0; i <= 7; i++) {
		mir_snprintf(szTemp, SIZEOF(szTemp), "NickColor%d", i);
		_tcsncpy(cid.name, chatcolorsnames[i], SIZEOF(cid.name));
		cid.order=i+1;
		strncpy(cid.setting, szTemp, SIZEOF(cid.setting));
		switch (i) {
			case 5:
				cid.defcolour = GetSysColor(COLOR_HIGHLIGHT);
				break;
			case 6:
				cid.defcolour = GetSysColor(COLOR_HIGHLIGHTTEXT);
				break;
			default:
				cid.defcolour =RGB(0, 0, 0);
				break;
			}
		CallService(MS_COLOUR_REGISTERT, (WPARAM)&cid, 0);
		}
	cid.order=8;
	_tcsncpy(cid.name, _T("Userlist background"), SIZEOF(cid.name));
	strncpy(cid.setting, "ColorNicklistBG", SIZEOF(cid.setting));
	cid.defcolour = SRMSGDEFSET_BKGCOLOUR;
	CallService(MS_COLOUR_REGISTERT, (WPARAM)&cid, 0);

	/*
	 * static colors (info panel, tool bar background etc...)
	 */
	strncpy(fid.dbSettingsGroup, FONTMODULE, SIZEOF(fid.dbSettingsGroup));
	strncpy(cid.dbSettingsGroup, FONTMODULE, SIZEOF(fid.dbSettingsGroup));

	for(i = 0; i < (sizeof(_clrs) / sizeof(_clrs[0])); i++) {
		cid.order = _clrs[i].order;
		_tcsncpy(cid.group, _clrs[i].tszGroup, SIZEOF(fid.group));
	 	_tcsncpy(cid.name, _clrs[i].tszName, SIZEOF(cid.name));
	 	strncpy(cid.setting, _clrs[i].szSetting, SIZEOF(cid.setting));
	 	if(_clrs[i].def & 0xff000000)
	 		cid.defcolour = GetSysColor(_clrs[i].def & 0x000000ff);
	 	else
	 		cid.defcolour = _clrs[i].def;
	 	CallService(MS_COLOUR_REGISTERT, (WPARAM)&cid, 0);
	}

	strncpy(cid.dbSettingsGroup, SRMSGMOD_T, SIZEOF(fid.dbSettingsGroup));

	/*
	 * text and background colors for tabs
	 */
	for(i = 0; i < (sizeof(_tabclrs) / sizeof(_tabclrs[0])); i++) {
		cid.order = _tabclrs[i].order;
		_tcsncpy(cid.group, _tabclrs[i].tszGroup, SIZEOF(fid.group));
	 	_tcsncpy(cid.name, _tabclrs[i].tszName, SIZEOF(cid.name));
	 	strncpy(cid.setting, _tabclrs[i].szSetting, SIZEOF(cid.setting));
	 	if(_tabclrs[i].def & 0xff000000)
	 		cid.defcolour = GetSysColor(_tabclrs[i].def & 0x000000ff);
	 	else
	 		cid.defcolour = _tabclrs[i].def;

	 	CallService(MS_COLOUR_REGISTERT, (WPARAM)&cid, 0);
	}
}

int FontServiceFontsChanged(WPARAM wParam, LPARAM lParam)
{
	if (PluginConfig.m_chat_enabled) {
		LOGFONT lf;
		HFONT hFont;
		int iText;

		LoadLogFonts();
		FreeMsgLogBitmaps();
		LoadMsgLogBitmaps();

		LoadMsgDlgFont(FONTSECTION_CHAT, 0, &lf, NULL, CHAT_FONTMODULE);
		hFont = CreateFontIndirect(&lf);
		iText = GetTextPixelSize(MakeTimeStamp(g_Settings.pszTimeStamp, time(NULL)), hFont, TRUE);
		DeleteObject(hFont);
		g_Settings.LogTextIndent = iText;
		g_Settings.LogTextIndent = g_Settings.LogTextIndent * 12 / 10;
		g_Settings.LogIndentEnabled = (M->GetByte("Chat", "LogIndentEnabled", 1) != 0) ? TRUE : FALSE;

		LoadGlobalSettings();
		MM_FontsChanged();
		MM_FixColors();
		SM_BroadcastMessage(NULL, GC_SETWNDPROPS, 0, 0, TRUE);
	}

	PluginConfig.reloadSettings();
	CSkin::initAeroEffect();
	CacheMsgLogIcons();
	CacheLogFonts();
	FreeTabConfig();
	ReloadTabConfig();
	Skin->setupAeroSkins();
	M->BroadcastMessage(DM_OPTIONSAPPLIED, 1, 0);
	return 0;
}


static UINT _o2chatcontrols[] = { IDC_CHAT_SPIN2, IDC_LIMIT, IDC_CHAT_SPIN4, IDC_LOGTIMESTAMP, IDC_TIMESTAMP,
								  IDC_OUTSTAMP, IDC_FONTCHOOSE, IDC_LOGGING, IDC_LOGDIRECTORY, IDC_INSTAMP, IDC_CHAT_SPIN2, IDC_CHAT_SPIN3, IDC_NICKROW2, IDC_LOGLIMIT,
								  IDC_STATIC110, IDC_STATIC112, 0};

static UINT _o3chatcontrols[] = {0};

INT_PTR CALLBACK DlgProcOptions2(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_INITDIALOG: {

			TranslateDialogDefault(hwndDlg);

			if (PluginConfig.m_chat_enabled) {

				SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN2, UDM_SETRANGE, 0, MAKELONG(5000, 0));
				SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN2, UDM_SETPOS, 0, MAKELONG(DBGetContactSettingWord(NULL, "Chat", "LogLimit", 100), 0));
				SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN3, UDM_SETRANGE, 0, MAKELONG(255, 10));
				SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN3, UDM_SETPOS, 0, MAKELONG(M->GetByte("Chat", "NicklistRowDist", 12), 0));
				SetDlgItemText(hwndDlg, IDC_LOGTIMESTAMP, g_Settings.pszTimeStampLog);
				SetDlgItemText(hwndDlg, IDC_TIMESTAMP, g_Settings.pszTimeStamp);
				SetDlgItemText(hwndDlg, IDC_OUTSTAMP, g_Settings.pszOutgoingNick);
				SetDlgItemText(hwndDlg, IDC_INSTAMP, g_Settings.pszIncomingNick);
				CheckDlgButton(hwndDlg, IDC_LOGGING, g_Settings.LoggingEnabled);
				SetDlgItemText(hwndDlg, IDC_LOGDIRECTORY, g_Settings.pszLogDir);
				Utils::enableDlgControl(hwndDlg, IDC_LOGDIRECTORY, g_Settings.LoggingEnabled ? TRUE : FALSE);
				Utils::enableDlgControl(hwndDlg, IDC_FONTCHOOSE, g_Settings.LoggingEnabled ? TRUE : FALSE);
				SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN4, UDM_SETRANGE, 0, MAKELONG(10000, 0));
				SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN4, UDM_SETPOS, 0, MAKELONG(DBGetContactSettingWord(NULL, "Chat", "LoggingLimit", 100), 0));
				Utils::enableDlgControl(hwndDlg, IDC_LIMIT, g_Settings.LoggingEnabled ? TRUE : FALSE);

				if (ServiceExists(MS_UTILS_REPLACEVARS)) {
					TCHAR tszTooltipText[2048];

					mir_sntprintf(tszTooltipText, SIZEOF(tszTooltipText),
						_T("%s - %s\n%s - %s\n%s - %s\n\n")
						_T("%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n\n")
						_T("%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s"),
						// contact vars
						_T("%nick%"),					CTranslator::getOpt(CTranslator::OPT_MUC_LOGTIP1),
						_T("%proto%"),					CTranslator::getOpt(CTranslator::OPT_MUC_LOGTIP2),
						_T("%userid%"),					CTranslator::getOpt(CTranslator::OPT_MUC_LOGTIP3),
						// global vars
						_T("%miranda_path%"),			CTranslator::getOpt(CTranslator::OPT_MUC_LOGTIP4),
						_T("%miranda_profile%"),		CTranslator::getOpt(CTranslator::OPT_MUC_LOGTIP5),
						_T("%miranda_profilename%"),	CTranslator::getOpt(CTranslator::OPT_MUC_LOGTIP6),
						_T("%miranda_userdata%"),		CTranslator::getOpt(CTranslator::OPT_MUC_LOGTIP7),
						_T("%appdata%"),				CTranslator::getOpt(CTranslator::OPT_MUC_LOGTIP8),
						_T("%username%"),				CTranslator::getOpt(CTranslator::OPT_MUC_LOGTIP9),
						_T("%mydocuments%"),			CTranslator::getOpt(CTranslator::OPT_MUC_LOGTIP10),
						_T("%desktop%"),				CTranslator::getOpt(CTranslator::OPT_MUC_LOGTIP11),
						_T("%xxxxxxx%"),				CTranslator::getOpt(CTranslator::OPT_MUC_LOGTIP12),
						// date/time vars
						_T("%d%"),			CTranslator::getOpt(CTranslator::OPT_MUC_LOGTIP13),
						_T("%dd%"),			CTranslator::getOpt(CTranslator::OPT_MUC_LOGTIP14),
						_T("%m%"),			CTranslator::getOpt(CTranslator::OPT_MUC_LOGTIP15),
						_T("%mm%"),			CTranslator::getOpt(CTranslator::OPT_MUC_LOGTIP16),
						_T("%mon%"),		CTranslator::getOpt(CTranslator::OPT_MUC_LOGTIP17),
						_T("%month%"),		CTranslator::getOpt(CTranslator::OPT_MUC_LOGTIP18),
						_T("%yy%"),			CTranslator::getOpt(CTranslator::OPT_MUC_LOGTIP19),
						_T("%yyyy%"),		CTranslator::getOpt(CTranslator::OPT_MUC_LOGTIP20),
						_T("%wday%"),		CTranslator::getOpt(CTranslator::OPT_MUC_LOGTIP21),
						_T("%weekday%"),	CTranslator::getOpt(CTranslator::OPT_MUC_LOGTIP22));
					hPathTip = CreateToolTip(GetDlgItem(hwndDlg, IDC_LOGDIRECTORY), tszTooltipText, const_cast<TCHAR *>(CTranslator::getOpt(CTranslator::OPT_MUC_VARIABLES)));
				}

			} else {
				int i = 0;

				while (_o2chatcontrols[i])
					Utils::enableDlgControl(hwndDlg, _o2chatcontrols[i++], FALSE);
			}
			if (hPathTip)
				SetTimer(hwndDlg, 0, 3000, NULL);
			break;
		}
		case WM_COMMAND:
			if ((LOWORD(wParam)		  == IDC_INSTAMP
					|| LOWORD(wParam) == IDC_OUTSTAMP
					|| LOWORD(wParam) == IDC_TIMESTAMP
					|| LOWORD(wParam) == IDC_LOGLIMIT
					|| LOWORD(wParam) == IDC_NICKROW2
					|| LOWORD(wParam) == IDC_LOGDIRECTORY
					|| LOWORD(wParam) == IDC_LIMIT
					|| LOWORD(wParam) == IDC_LOGTIMESTAMP)
					&& (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))	return 0;

			switch (LOWORD(wParam)) {
				/*
				 * open the base directory for MUC logs, using a standard file selector
				 * dialog. Simply allows the user to view what log files are there
				 * and possibly delete archived logs.
				 */
				case IDC_MUC_OPENLOGBASEDIR: {
					OPENFILENAME ofn = {0};
					SESSION_INFO si = {0};
					TCHAR	tszReturnName[MAX_PATH];
					TCHAR	tszInitialDir[_MAX_DRIVE + _MAX_PATH + 10];
					TCHAR	tszTemp[MAX_PATH + 20], *p = 0, *p1 = 0;

					mir_sntprintf(tszTemp, MAX_PATH + 20, _T("%s"), g_Settings.pszLogDir);

					p = tszTemp;
					while(*p && (*p == '\\' || *p == '.'))
						p++;

					if(*p) {
						if((p1 = _tcschr(p, '\\')))
							*p1 = 0;
					}

					mir_sntprintf(tszInitialDir, MAX_PATH, _T("%s%s"), M->getChatLogPath(), p);
					if(PathFileExists(tszInitialDir))
						ofn.lpstrInitialDir = tszInitialDir;
					else {
						mir_sntprintf(tszInitialDir, MAX_PATH, _T("%s"), M->getChatLogPath());
						ofn.lpstrInitialDir = tszInitialDir;
					}

					tszReturnName[0] = 0;
					mir_sntprintf(tszTemp, SIZEOF(tszTemp), _T("%s%c*.*%c%c"), TranslateT("All Files"), 0, 0, 0);

					ofn.lpstrFilter = tszTemp;
					ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
					ofn.hwndOwner = 0;
					ofn.lpstrFile = tszReturnName;
					ofn.nMaxFile = MAX_PATH;
					ofn.nMaxFileTitle = MAX_PATH;
					ofn.Flags = OFN_HIDEREADONLY | OFN_DONTADDTORECENT;
					ofn.lpstrDefExt = _T("log");
					GetOpenFileName(&ofn);
					break;
				}

				case  IDC_FONTCHOOSE: {
					TCHAR tszDirectory[MAX_PATH];
					LPITEMIDLIST idList;
					LPMALLOC psMalloc;
					BROWSEINFO bi = {0};

					if (SUCCEEDED(CoGetMalloc(1, &psMalloc))) {
						TCHAR tszTemp[MAX_PATH];
						bi.hwndOwner = hwndDlg;
						bi.pszDisplayName = tszDirectory;
						bi.lpszTitle = const_cast<TCHAR *>(CTranslator::getOpt(CTranslator::OPT_MUC_SELECTFOLDER));
						bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX | BIF_RETURNONLYFSDIRS;
						bi.lpfn = BrowseCallbackProc;
						bi.lParam = (LPARAM)tszDirectory;


						idList = SHBrowseForFolder(&bi);
						if (idList) {
							const TCHAR *szUserDir = M->getUserDir();
							SHGetPathFromIDList(idList, tszDirectory);
							lstrcat(tszDirectory, _T("\\"));
							M->pathToRelative(tszDirectory, tszTemp, const_cast<TCHAR *>(szUserDir));
							SetWindowText(GetDlgItem(hwndDlg, IDC_LOGDIRECTORY), lstrlen(tszTemp) > 1 ? tszTemp : DEFLOGFILENAME);
						}
						psMalloc->Free(idList);
						psMalloc->Release();
					}
					break;
				}

				case IDC_LOGGING:
					if (PluginConfig.m_chat_enabled) {
						Utils::enableDlgControl(hwndDlg, IDC_LOGDIRECTORY, IsDlgButtonChecked(hwndDlg, IDC_LOGGING) == BST_CHECKED ? TRUE : FALSE);
						Utils::enableDlgControl(hwndDlg, IDC_FONTCHOOSE, IsDlgButtonChecked(hwndDlg, IDC_LOGGING) == BST_CHECKED ? TRUE : FALSE);
						Utils::enableDlgControl(hwndDlg, IDC_LIMIT, IsDlgButtonChecked(hwndDlg, IDC_LOGGING) == BST_CHECKED ? TRUE : FALSE);
					}
					break;
			}

			if (lParam != (LPARAM)NULL)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case WM_NOTIFY:
			if (((LPNMHDR)lParam)->idFrom == 0 && ((LPNMHDR)lParam)->code == PSN_APPLY) {
				int iLen;
				TCHAR *p2 = NULL;
				char  *pszText = NULL;
				TCHAR *ptszPath = NULL;

				if (PluginConfig.m_chat_enabled) {

					iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_LOGDIRECTORY));
					if (iLen > 0) {
						TCHAR *pszText1 = (TCHAR *)malloc(iLen*sizeof(TCHAR) + 2);
						GetDlgItemText(hwndDlg, IDC_LOGDIRECTORY, pszText1, iLen + 1);
						M->WriteTString(NULL, "Chat", "LogDirectory", pszText1);
						free(pszText1);
						g_Settings.LoggingEnabled = IsDlgButtonChecked(hwndDlg, IDC_LOGGING) == BST_CHECKED ? TRUE : FALSE;
						M->WriteByte("Chat", "LoggingEnabled", (BYTE)g_Settings.LoggingEnabled);
					} else {
						DBDeleteContactSetting(NULL, "Chat", "LogDirectory");
						M->WriteByte("Chat", "LoggingEnabled", 0);
					}
					SM_InvalidateLogDirectories();

					iLen = SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN4, UDM_GETPOS, 0, 0);
					DBWriteContactSettingWord(NULL, "Chat", "LoggingLimit", (WORD)iLen);

					iLen = SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN3, UDM_GETPOS, 0, 0);
					if (iLen > 0)
						M->WriteByte("Chat", "NicklistRowDist", (BYTE)iLen);
					else
						DBDeleteContactSetting(NULL, "Chat", "NicklistRowDist");

					iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_LOGTIMESTAMP));
					if (iLen > 0) {
						pszText = (char *)realloc(pszText, iLen + 1);
						GetDlgItemTextA(hwndDlg, IDC_LOGTIMESTAMP, pszText, iLen + 1);
						DBWriteContactSettingString(NULL, "Chat", "LogTimestamp", pszText);
					} else DBDeleteContactSetting(NULL, "Chat", "LogTimestamp");

					iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_TIMESTAMP));
					if (iLen > 0) {
						pszText = (char *)realloc(pszText, iLen + 1);
						GetDlgItemTextA(hwndDlg, IDC_TIMESTAMP, pszText, iLen + 1);
						DBWriteContactSettingString(NULL, "Chat", "HeaderTime", pszText);
					} else DBDeleteContactSetting(NULL, "Chat", "HeaderTime");

					iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_INSTAMP));
					if (iLen > 0) {
						pszText = (char *)realloc(pszText, iLen + 1);
						GetDlgItemTextA(hwndDlg, IDC_INSTAMP, pszText, iLen + 1);
						DBWriteContactSettingString(NULL, "Chat", "HeaderIncoming", pszText);
					} else DBDeleteContactSetting(NULL, "Chat", "HeaderIncoming");

					iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_OUTSTAMP));
					if (iLen > 0) {
						pszText = (char *)realloc(pszText, iLen + 1);
						GetDlgItemTextA(hwndDlg, IDC_OUTSTAMP, pszText, iLen + 1);
						DBWriteContactSettingString(NULL, "Chat", "HeaderOutgoing", pszText);
					} else DBDeleteContactSetting(NULL, "Chat", "HeaderOutgoing");

					iLen = SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN2, UDM_GETPOS, 0, 0);
					DBWriteContactSettingWord(NULL, "Chat", "LogLimit", (WORD)iLen);
				}


				if (pszText != NULL)
					free(pszText);
				if (hListBkgBrush)
					DeleteObject(hListBkgBrush);
				hListBkgBrush = CreateSolidBrush(M->GetDword("Chat", "ColorNicklistBG", SRMSGDEFSET_BKGCOLOUR));


				if (PluginConfig.m_chat_enabled) {
					LOGFONT lf;
					HFONT hFont;
					int iText;

					LoadLogFonts();
					FreeMsgLogBitmaps();
					LoadMsgLogBitmaps();

					LoadMsgDlgFont(FONTSECTION_CHAT, 0, &lf, NULL, CHAT_FONTMODULE);
					hFont = CreateFontIndirect(&lf);
					iText = GetTextPixelSize(MakeTimeStamp(g_Settings.pszTimeStamp, time(NULL)), hFont, TRUE);
					DeleteObject(hFont);
					g_Settings.LogTextIndent = iText;
					g_Settings.LogTextIndent = g_Settings.LogTextIndent * 12 / 10;
					g_Settings.LogIndentEnabled = (M->GetByte("Chat", "LogIndentEnabled", 1) != 0) ? TRUE : FALSE;

					LoadGlobalSettings();
					MM_FontsChanged();
					MM_FixColors();
					SM_BroadcastMessage(NULL, GC_SETWNDPROPS, 0, 0, TRUE);
				}

				PluginConfig.reloadSettings();
				CacheMsgLogIcons();
				CacheLogFonts();
				return TRUE;
			}
			break;
		case WM_TIMER:
			if(IsWindow(hPathTip))
				KillTimer(hPathTip, 4); // It will prevent tooltip autoclosing
			break;
		case WM_DESTROY:
			if (hPathTip)
			{
				KillTimer(hwndDlg, 0);
				DestroyWindow(hPathTip);
				hPathTip = 0;
			}
			break;
	}
	return FALSE;
}

#define NR_GC_EVENTS 12

static UINT _eventorder[] = { GC_EVENT_ACTION,
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
	GC_EVENT_HIGHLIGHT,
	0
};

#define GC_EVENT_ALL (GC_EVENT_ACTION | GC_EVENT_MESSAGE | GC_EVENT_NICK | GC_EVENT_JOIN | \
	GC_EVENT_PART | GC_EVENT_TOPIC | GC_EVENT_ADDSTATUS | GC_EVENT_INFORMATION | GC_EVENT_QUIT | \
	GC_EVENT_KICK |	GC_EVENT_NOTICE)

/**
 * Dialog procedure for group chat options tab #3 (event filter configuration)
 *
 * @return
 */
INT_PTR CALLBACK DlgProcOptions3(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_INITDIALOG: {

			TranslateDialogDefault(hwndDlg);

			if (PluginConfig.m_chat_enabled) {
				DWORD dwFilterFlags = M->GetDword("Chat", "FilterFlags", GC_EVENT_ALL);
				DWORD dwTrayFlags = M->GetDword("Chat", "TrayIconFlags", GC_EVENT_ALL);
				DWORD dwPopupFlags = M->GetDword("Chat", "PopupFlags", GC_EVENT_ALL);
				DWORD dwLogFlags = M->GetDword("Chat", "DiskLogFlags", GC_EVENT_ALL);

				for(int i = 0; _eventorder[i]; i++) {
					if(_eventorder[i] != GC_EVENT_HIGHLIGHT) {
						CheckDlgButton(hwndDlg, IDC_1 + i, dwFilterFlags & _eventorder[i] ? BST_CHECKED : BST_UNCHECKED);
						CheckDlgButton(hwndDlg, IDC_L1 + i, dwLogFlags & _eventorder[i] ? BST_CHECKED : BST_UNCHECKED);
					}
					CheckDlgButton(hwndDlg, IDC_P1 + i, dwPopupFlags & _eventorder[i] ? BST_CHECKED : BST_UNCHECKED);
					CheckDlgButton(hwndDlg, IDC_T1 + i, dwTrayFlags & _eventorder[i] ? BST_CHECKED : BST_UNCHECKED);
				}
				SendDlgItemMessage(hwndDlg, IDC_LOGICONTYPE, CB_ADDSTRING, -1, (LPARAM)CTranslator::getOpt(CTranslator::OPT_MUC_NOMARKERS));
				SendDlgItemMessage(hwndDlg, IDC_LOGICONTYPE, CB_ADDSTRING, -1, (LPARAM)CTranslator::getOpt(CTranslator::OPT_MUC_ASICONS));
				SendDlgItemMessage(hwndDlg, IDC_LOGICONTYPE, CB_ADDSTRING, -1, (LPARAM)CTranslator::getOpt(CTranslator::OPT_MUC_ASSYMBOLS));

				SendDlgItemMessage(hwndDlg, IDC_LOGICONTYPE, CB_SETCURSEL, (g_Settings.LogSymbols ? 2 : (g_Settings.dwIconFlags ? 1 : 0)), 0);

				CheckDlgButton(hwndDlg, IDC_NOPOPUPSFORCLOSEDWINDOWS, M->GetByte("Chat", "SkipWhenNoWindow", 0) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_TRAYONLYFORINACTIVE, M->GetByte("Chat", "TrayIconInactiveOnly", 0) ? BST_CHECKED : BST_UNCHECKED);
			}
			break;
		}
		case WM_COMMAND:
			if (lParam != (LPARAM)NULL)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case WM_NOTIFY:
			switch (((LPNMHDR)lParam)->idFrom) {
				case 0:
					switch (((LPNMHDR)lParam)->code) {
						case PSN_APPLY: {
							DWORD dwFilterFlags = 0, dwTrayFlags = 0,
								dwPopupFlags = 0, dwLogFlags = 0;

							for(int i = 0; _eventorder[i]; i++) {
								if(_eventorder[i] != GC_EVENT_HIGHLIGHT) {
									dwFilterFlags |= (IsDlgButtonChecked(hwndDlg, IDC_1 + i) ? _eventorder[i] : 0);
									dwLogFlags |= (IsDlgButtonChecked(hwndDlg, IDC_L1 + i) ? _eventorder[i] : 0);
								}
								dwPopupFlags |= (IsDlgButtonChecked(hwndDlg, IDC_P1 + i) ? _eventorder[i] : 0);
								dwTrayFlags |= (IsDlgButtonChecked(hwndDlg, IDC_T1 + i) ? _eventorder[i] : 0);
							}
							M->WriteDword("Chat", "FilterFlags", dwFilterFlags);
							M->WriteDword("Chat", "PopupFlags", dwPopupFlags);
							M->WriteDword("Chat", "TrayIconFlags", dwTrayFlags);
							M->WriteDword("Chat", "DiskLogFlags", dwLogFlags);

							LRESULT lr = SendDlgItemMessage(hwndDlg, IDC_LOGICONTYPE, CB_GETCURSEL, 0, 0);

							M->WriteDword("Chat", "IconFlags", lr == 1 ? 1 : 0);
							M->WriteByte("Chat", "LogSymbols", lr == 2 ? 1 : 0);

							M->WriteByte("Chat", "SkipWhenNoWindow", IsDlgButtonChecked(hwndDlg, IDC_NOPOPUPSFORCLOSEDWINDOWS) ? 1 : 0);
							M->WriteByte("Chat", "TrayIconInactiveOnly", IsDlgButtonChecked(hwndDlg, IDC_TRAYONLYFORINACTIVE) ? 1 : 0);
							LoadGlobalSettings();
							MM_FontsChanged();
							SM_BroadcastMessage(NULL, GC_SETWNDPROPS, 0, 0, TRUE);
							SM_ReconfigureFilters();
							break;
						}
						return TRUE;
					}
			}
			break;

		case WM_DESTROY:
			break;

	}
	return FALSE;
}

void LoadGlobalSettings(void)
{
	LOGFONT lf;
	int i;
	char szBuf[40];

	g_Settings.LogLimitNames = M->GetByte("Chat", "LogLimitNames", 1);
	g_Settings.ShowTime = M->GetByte("Chat", "ShowTimeStamp", 1);
	g_Settings.ShowTimeIfChanged = (BOOL)M->GetByte("Chat", "ShowTimeStampIfChanged", 0);
	g_Settings.TimeStampEventColour = (BOOL)M->GetByte("Chat", "TimeStampEventColour", 0);
	g_Settings.iEventLimit = DBGetContactSettingWord(NULL, "Chat", "LogLimit", 100);
	g_Settings.iEventLimitThreshold = DBGetContactSettingWord(NULL, "Chat", "LogLimitThreshold", 20);
	g_Settings.dwIconFlags = M->GetDword("Chat", "IconFlags", 0x0000);
	g_Settings.LoggingLimit = (size_t)DBGetContactSettingWord(NULL, "Chat", "LoggingLimit", 100);
	g_Settings.LoggingEnabled = (BOOL)M->GetByte("Chat", "LoggingEnabled", 0);
	g_Settings.OpenInDefault = (BOOL)M->GetByte("Chat", "DefaultContainer", 1);
	g_Settings.FlashWindow = (BOOL)M->GetByte("Chat", "FlashWindow", 0);
	g_Settings.FlashWindowHightlight = (BOOL)M->GetByte("Chat", "FlashWindowHighlight", 0);
	g_Settings.HighlightEnabled = (BOOL)M->GetByte("Chat", "HighlightEnabled", 1);
	g_Settings.crUserListColor = (BOOL)M->GetDword(CHAT_FONTMODULE, "Font18Col", RGB(0, 0, 0));
	g_Settings.crUserListBGColor = (BOOL)M->GetDword("Chat", "ColorNicklistBG", SRMSGDEFSET_BKGCOLOUR);
	g_Settings.crUserListHeadingsColor = (BOOL)M->GetDword(CHAT_FONTMODULE, "Font19Col", RGB(170, 170, 170));
	g_Settings.StripFormat = (BOOL)M->GetByte("Chat", "StripFormatting", 0);
	g_Settings.TrayIconInactiveOnly = (BOOL)M->GetByte("Chat", "TrayIconInactiveOnly", 1);
	g_Settings.BBCodeInPopups = (BOOL)M->GetByte("Chat", "BBCodeInPopups", 0);
	g_Settings.AddColonToAutoComplete = (BOOL)M->GetByte("Chat", "AddColonToAutoComplete", 1);
	g_Settings.iPopupStyle = M->GetByte("Chat", "PopupStyle", 1);
	g_Settings.iPopupTimeout = DBGetContactSettingWord(NULL, "Chat", "PopupTimeout", 3);
	g_Settings.crPUBkgColour = M->GetDword("Chat", "PopupColorBG", GetSysColor(COLOR_WINDOW));
	g_Settings.crPUTextColour = M->GetDword("Chat", "PopupColorText", 0);
	g_Settings.ClassicIndicators = M->GetByte("Chat", "ClassicIndicators", 0);
	//MAD
	g_Settings.LogClassicIndicators = M->GetByte("Chat", "LogClassicIndicators", 0);
	g_Settings.AlternativeSorting   = M->GetByte("Chat", "AlternativeSorting", 1);
	g_Settings.AnnoyingHighlight	= M->GetByte("Chat", "AnnoyingHighlight", 0);
	g_Settings.CreateWindowOnHighlight = M->GetByte("Chat", "CreateWindowOnHighlight", 1);
	//MAD_
	g_Settings.LogSymbols = M->GetByte("Chat", "LogSymbols", 1);
	g_Settings.ClickableNicks = M->GetByte("Chat", "ClickableNicks", 1);
	g_Settings.ColorizeNicks = M->GetByte("Chat", "ColorizeNicks", 1);
	g_Settings.ColorizeNicksInLog = M->GetByte("Chat", "ColorizeNicksInLog", 1);
	g_Settings.ScaleIcons = M->GetByte("Chat", "ScaleIcons", 1);
	g_Settings.UseDividers = M->GetByte("Chat", "UseDividers", 1);
	g_Settings.DividersUsePopupConfig = M->GetByte("Chat", "DividersUsePopupConfig", 1);
	g_Settings.MathMod = ServiceExists(MATH_RTF_REPLACE_FORMULAE) && M->GetByte("Chat", "MathModSupport", 0);

	g_Settings.DoubleClick4Privat = (BOOL)M->GetByte("Chat", "DoubleClick4Privat", 0);
	g_Settings.ShowContactStatus = M->GetByte("Chat", "ShowContactStatus", 1);
	g_Settings.ContactStatusFirst = M->GetByte("Chat", "ContactStatusFirst", 0);


	if (hListBkgBrush)
		DeleteObject(hListBkgBrush);
	hListBkgBrush = CreateSolidBrush(M->GetDword("Chat", "ColorNicklistBG", SRMSGDEFSET_BKGCOLOUR));

	InitSetting(&g_Settings.pszTimeStamp, "HeaderTime", _T("[%H:%M]"));
	InitSetting(&g_Settings.pszTimeStampLog, "LogTimestamp", _T("[%d %b %y %H:%M]"));
	InitSetting(&g_Settings.pszIncomingNick, "HeaderIncoming", _T("%n:"));
	InitSetting(&g_Settings.pszOutgoingNick, "HeaderOutgoing", _T("%n:"));

	DBVARIANT dbv;

	if (!M->GetTString(NULL, "Chat", "LogDirectory", &dbv)) {
		lstrcpyn(g_Settings.pszLogDir, dbv.ptszVal, MAX_PATH);
		DBFreeVariant(&dbv);
	} else
		lstrcpyn(g_Settings.pszLogDir, DEFLOGFILENAME, MAX_PATH);

	g_Settings.pszLogDir[MAX_PATH - 1] = 0;

	g_Settings.LogIndentEnabled = (M->GetByte("Chat", "LogIndentEnabled", 1) != 0) ? TRUE : FALSE;


	// nicklist

	if(g_Settings.UserListFont) {
		DeleteObject(g_Settings.UserListFont);
		DeleteObject(g_Settings.UserListHeadingsFont);
	}

	LoadMsgDlgFont(FONTSECTION_CHAT, 18, &lf, NULL, CHAT_FONTMODULE);
	g_Settings.UserListFont = CreateFontIndirect(&lf);

	LoadMsgDlgFont(FONTSECTION_CHAT, 19, &lf, NULL, CHAT_FONTMODULE);
	g_Settings.UserListHeadingsFont = CreateFontIndirect(&lf);

	int ih;
	int ih2;

	ih = GetTextPixelSize(_T("AQGglo"), g_Settings.UserListFont, FALSE);
	ih2 = GetTextPixelSize(_T("AQGglo"), g_Settings.UserListHeadingsFont, FALSE);
	g_Settings.iNickListFontHeight = max(M->GetByte("Chat", "NicklistRowDist", 12), (ih > ih2 ? ih : ih2));

	for (i = 0; i < 7; i++) {
		mir_snprintf(szBuf, 20, "NickColor%d", i);
		g_Settings.nickColors[i] = M->GetDword("Chat", szBuf, g_Settings.crUserListColor);
	}
	g_Settings.nickColors[5] = M->GetDword("Chat", "NickColor5", GetSysColor(COLOR_HIGHLIGHT));
	g_Settings.nickColors[6] = M->GetDword("Chat", "NickColor6", GetSysColor(COLOR_HIGHLIGHTTEXT));
	if (g_Settings.SelectionBGBrush)
		DeleteObject(g_Settings.SelectionBGBrush);
	g_Settings.SelectionBGBrush = CreateSolidBrush(g_Settings.nickColors[5]);
}

static void FreeGlobalSettings(void)
{
	mir_free(g_Settings.pszTimeStamp);
	mir_free(g_Settings.pszTimeStampLog);
	mir_free(g_Settings.pszIncomingNick);
	mir_free(g_Settings.pszOutgoingNick);
	if(g_Settings.UserListFont) {
		DeleteObject(g_Settings.UserListFont);
		DeleteObject(g_Settings.UserListHeadingsFont);
	}
	if (g_Settings.SelectionBGBrush)
		DeleteObject(g_Settings.SelectionBGBrush);

	delete g_Settings.Highlight;
}

int OptionsInit(void)
{
	LOGFONT lf;
	HFONT hFont;
	int iText;

	LoadLogFonts();
	LoadMsgDlgFont(FONTSECTION_CHAT, 17, &lf, NULL, CHAT_FONTMODULE);
	lstrcpy(lf.lfFaceName, _T("MS Shell Dlg"));
	lf.lfUnderline = lf.lfItalic = lf.lfStrikeOut = 0;
	lf.lfHeight = -17;
	lf.lfWeight = FW_BOLD;
	ZeroMemory(&g_Settings, sizeof(TMUCSettings));
	g_Settings.NameFont = CreateFontIndirect(&lf);
	g_Settings.iSplitterX = DBGetContactSettingWord(NULL, "Chat", "SplitterX", 105);
	if(g_Settings.iSplitterX <= 50)
		g_Settings.iSplitterX = 105;
	g_Settings.iSplitterY = DBGetContactSettingWord(NULL, "Chat", "splitY", 50);
	if(g_Settings.iSplitterY <= 20)
		g_Settings.iSplitterY = 50;
	g_Settings.hGroup = 0;
	LoadGlobalSettings();
	g_Settings.Highlight = new CMUCHighlight();
	SkinAddNewSoundEx("ChatMessage", "Group chats", "Incoming message");
	SkinAddNewSoundEx("ChatSent", "Group chats", "Outgoing message");
	SkinAddNewSoundEx("ChatHighlight", "Group chats", "Message is highlighted");
	SkinAddNewSoundEx("ChatAction", "Group chats", "User has performed an action");
	SkinAddNewSoundEx("ChatJoin", "Group chats", "User has joined");
	SkinAddNewSoundEx("ChatPart", "Group chats", "User has left");
	SkinAddNewSoundEx("ChatKick", "Group chats", "User has kicked some other user");
	SkinAddNewSoundEx("ChatMode", "Group chats", "User's status was changed");
	SkinAddNewSoundEx("ChatNick", "Group chats", "User has changed name");
	SkinAddNewSoundEx("ChatNotice", "Group chats", "User has sent a notice");
	SkinAddNewSoundEx("ChatQuit", "Group chats", "User has disconnected");
	SkinAddNewSoundEx("ChatTopic", "Group chats", "The topic has been changed");

	LoadMsgDlgFont(FONTSECTION_CHAT, 0, &lf, NULL, CHAT_FONTMODULE);
	hFont = CreateFontIndirect(&lf);
	iText = GetTextPixelSize(MakeTimeStamp(g_Settings.pszTimeStamp, time(NULL)), hFont, TRUE);
	DeleteObject(hFont);
	g_Settings.LogTextIndent = iText;
	g_Settings.LogTextIndent = g_Settings.LogTextIndent * 12 / 10;
	return 0;
}


int OptionsUnInit(void)
{
	FreeGlobalSettings();
	UnhookEvent(g_hOptions);
	DeleteObject(hListBkgBrush);
	DeleteObject(g_Settings.NameFont);
	return 0;
}
