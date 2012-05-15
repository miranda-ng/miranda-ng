/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson

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

#include "resource.h"
#include "chat.h"
#include <shlobj.h>

#include <m_fontservice.h>

extern HANDLE       g_hInst;
extern HBRUSH       hEditBkgBrush;
extern HBRUSH       hListBkgBrush;
extern HBRUSH       hListSelectedBkgBrush;
extern HICON        hIcons[30];
extern FONTINFO     aFonts[OPTIONS_FONTCOUNT];
extern BOOL         PopUpInstalled;
extern SESSION_INFO g_TabSession;

HANDLE g_hOptions = NULL;

#define FONTF_BOLD   1
#define FONTF_ITALIC 2
struct FontOptionsList
{
	const TCHAR*  szDescr;
	COLORREF      defColour;
	const TCHAR*  szDefFace;
	BYTE          defCharset, defStyle; 
	char          defSize;
	COLORREF      colour;
	const TCHAR*  szFace;
	BYTE          charset, style;
	char          size;
}

//remeber to put these in the Translate( ) template file too
static const fontOptionsList[] = {
	{ LPGENT("Timestamp"),                    RGB(50, 50, 240),   _T("Terminal"), DEFAULT_CHARSET, 0, -8},
	{ LPGENT("Others nicknames"),             RGB(0, 0, 0),       _T("Verdana"), DEFAULT_CHARSET, FONTF_BOLD, -12},
	{ LPGENT("Your nickname"),                RGB(0, 0, 0),       _T("Verdana"), DEFAULT_CHARSET, FONTF_BOLD, -12},
	{ LPGENT("User has joined"),              RGB(90, 160, 90),   _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("User has left"),                RGB(160, 160, 90),  _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("User has disconnected"),        RGB(160, 90, 90),   _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("User kicked ..."),              RGB(100, 100, 100), _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("User is now known as ..."),     RGB(90, 90, 160),   _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("Notice from user"),             RGB(160, 130, 60),  _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("Incoming message"),             RGB(90, 90, 90),    _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("Outgoing message"),             RGB(90, 90, 90),    _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("The topic is ..."),             RGB(70, 70, 160),   _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("Information messages"),         RGB(130, 130, 195), _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("User enables status for ..."),  RGB(70, 150, 70),   _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("User disables status for ..."), RGB(150, 70, 70),   _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("Action message"),               RGB(160, 90, 160),  _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("Highlighted message"),          RGB(180, 150, 80),  _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("Message typing area"),          RGB(0, 0, 40),      _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("User list members (Online)"),   RGB(0,0, 0),        _T("Verdana"), DEFAULT_CHARSET, 0, -12},
	{ LPGENT("User list members (away)"),     RGB(170, 170, 170), _T("Verdana"), DEFAULT_CHARSET, 0, -12},
};

const int msgDlgFontCount = SIZEOF(fontOptionsList);

struct branch_t
{
	const TCHAR* szDescr;
	const char*  szDBName;
	int          iMode;
	BYTE         bDefault;
};

static const struct branch_t branch0[] = {
	{ LPGENT("Use a tabbed interface"), "Tabs", 0, 1},
	{ LPGENT("Close tab on doubleclick"), "TabCloseOnDblClick", 0, 0},
	{ LPGENT("Restore previously open tabs when showing the window"), "TabRestore", 0, 0},
	{ LPGENT("Show tabs at the bottom"), "TabBottom", 0, 0},
};

static const struct branch_t branch1[] = {
	{ LPGENT("Send message by pressing the Enter key"), "SendOnEnter", 0, 1},
	{ LPGENT("Send message by pressing the Enter key twice"), "SendOnDblEnter", 0,0},
	{ LPGENT("Flash window when someone speaks"), "FlashWindow", 0,0},
	{ LPGENT("Flash window when a word is highlighted"), "FlashWindowHighlight", 0,1},
	{ LPGENT("Show list of users in the chat room"), "ShowNicklist", 0,1},
	{ LPGENT("Show button for sending messages"), "ShowSend", 0, 0},
	{ LPGENT("Show buttons for controlling the chat room"), "ShowTopButtons", 0,1},
	{ LPGENT("Show buttons for formatting the text you are typing"), "ShowFormatButtons", 0,1},
	{ LPGENT("Show button menus when right clicking the buttons"), "RightClickFilter", 0,0},
	{ LPGENT("Show new windows cascaded"), "CascadeWindows", 0,1},
	{ LPGENT("Save the size and position of chat rooms"), "SavePosition", 0,0},
	{ LPGENT("Show the topic of the room on your contact list (if supported)"), "TopicOnClist", 0, 0},
	{ LPGENT("Do not play sounds when the chat room is focused"), "SoundsFocus", 0, 0},
	{ LPGENT("Do not pop up the window when joining a chat room"), "PopupOnJoin", 0,0},
	{ LPGENT("Toggle the visible state when double clicking in the contact list"), "ToggleVisibility", 0,0},
    {LPGENT("Show contact statuses if protocol supports them"), "ShowContactStatus", 0,0},
    {LPGENT("Display contact status icon before user role icon"), "ContactStatusFirst", 0,0},
};

static const struct branch_t branch2[] = {
	{ LPGENT("Prefix all events with a timestamp"), "ShowTimeStamp", 0,1},
	{ LPGENT("Only prefix with timestamp if it has changed"), "ShowTimeStampIfChanged", 0,0},
	{ LPGENT("Timestamp has same colour as the event"), "TimeStampEventColour", 0,0},
	{ LPGENT("Indent the second line of a message"), "LogIndentEnabled", 0,1},
	{ LPGENT("Limit user names in the message log to 20 characters"), "LogLimitNames", 0,1},
	{ LPGENT("Add \':\' to auto-completed user names"), "AddColonToAutoComplete", 0, 1},
	{ LPGENT("Strip colors from messages in the log"), "StripFormatting", 0, 0},
	{ LPGENT("Enable the \'event filter\' for new rooms"), "FilterEnabled", 0,0}
};

static const struct branch_t branch3[] = {
	{ LPGENT("Show topic changes"), "FilterFlags", GC_EVENT_TOPIC, 0},
	{ LPGENT("Show users joining"), "FilterFlags", GC_EVENT_JOIN, 0},
	{ LPGENT("Show users disconnecting"), "FilterFlags", GC_EVENT_QUIT, 0},
	{ LPGENT("Show messages"), "FilterFlags", GC_EVENT_MESSAGE, 1},
	{ LPGENT("Show actions"), "FilterFlags", GC_EVENT_ACTION, 1},
	{ LPGENT("Show users leaving"), "FilterFlags", GC_EVENT_PART, 0},
	{ LPGENT("Show users being kicked"), "FilterFlags", GC_EVENT_KICK, 1},
	{ LPGENT("Show notices"), "FilterFlags", GC_EVENT_NOTICE, 1},
	{ LPGENT("Show users changing name"), "FilterFlags", GC_EVENT_NICK, 0},
	{ LPGENT("Show information messages"), "FilterFlags", GC_EVENT_INFORMATION, 1},
	{ LPGENT("Show status changes of users"), "FilterFlags", GC_EVENT_ADDSTATUS, 0},
};

static const struct branch_t branch4[] = {
	{ LPGENT("Show icon for topic changes"), "IconFlags", GC_EVENT_TOPIC, 0},
	{ LPGENT("Show icon for users joining"), "IconFlags", GC_EVENT_JOIN, 1},
	{ LPGENT("Show icon for users disconnecting"), "IconFlags", GC_EVENT_QUIT, 0},
	{ LPGENT("Show icon for messages"), "IconFlags", GC_EVENT_MESSAGE, 0},
	{ LPGENT("Show icon for actions"), "IconFlags", GC_EVENT_ACTION, 0},
	{ LPGENT("Show icon for highlights"), "IconFlags", GC_EVENT_HIGHLIGHT, 0},
	{ LPGENT("Show icon for users leaving"), "IconFlags", GC_EVENT_PART, 0},
	{ LPGENT("Show icon for users kicking other user"), "IconFlags", GC_EVENT_KICK, 0},
	{ LPGENT("Show icon for notices "), "IconFlags", GC_EVENT_NOTICE, 0},
	{ LPGENT("Show icon for name changes"), "IconFlags", GC_EVENT_NICK, 0},
	{ LPGENT("Show icon for information messages"), "IconFlags", GC_EVENT_INFORMATION, 0},
	{ LPGENT("Show icon for status changes"), "IconFlags", GC_EVENT_ADDSTATUS, 0},
};

static const struct branch_t branch5[] = {
	{ LPGENT("Show icons in tray only when the chat room is not active"), "TrayIconInactiveOnly", 0, 1},
	{ LPGENT("Show icon in tray for topic changes"), "TrayIconFlags", GC_EVENT_TOPIC, 0},
	{ LPGENT("Show icon in tray for users joining"), "TrayIconFlags", GC_EVENT_JOIN, 0},
	{ LPGENT("Show icon in tray for users disconnecting"), "TrayIconFlags", GC_EVENT_QUIT, 0},
	{ LPGENT("Show icon in tray for messages"), "TrayIconFlags", GC_EVENT_MESSAGE, 0},
	{ LPGENT("Show icon in tray for actions"), "TrayIconFlags", GC_EVENT_ACTION, 0},
	{ LPGENT("Show icon in tray for highlights"), "TrayIconFlags", GC_EVENT_HIGHLIGHT, 1},
	{ LPGENT("Show icon in tray for users leaving"), "TrayIconFlags", GC_EVENT_PART, 0},
	{ LPGENT("Show icon in tray for users kicking other user"), "TrayIconFlags", GC_EVENT_KICK, 0},
	{ LPGENT("Show icon in tray for notices "), "TrayIconFlags", GC_EVENT_NOTICE, 0},
	{ LPGENT("Show icon in tray for name changes"), "TrayIconFlags", GC_EVENT_NICK, 0},
	{ LPGENT("Show icon in tray for information messages"), "TrayIconFlags", GC_EVENT_INFORMATION, 0},
	{ LPGENT("Show icon in tray for status changes"), "TrayIconFlags", GC_EVENT_ADDSTATUS, 0},
};

static const struct branch_t branch6[] = {
	{ LPGENT("Show pop-ups only when the chat room is not active"), "PopUpInactiveOnly", 0, 1},
	{ LPGENT("Show pop-up for topic changes"), "PopupFlags", GC_EVENT_TOPIC, 0},
	{ LPGENT("Show pop-up for users joining"), "PopupFlags", GC_EVENT_JOIN, 0},
	{ LPGENT("Show pop-up for users disconnecting"), "PopupFlags", GC_EVENT_QUIT, 0},
	{ LPGENT("Show pop-up for messages"), "PopupFlags", GC_EVENT_MESSAGE, 0},
	{ LPGENT("Show pop-up for actions"), "PopupFlags", GC_EVENT_ACTION, 0},
	{ LPGENT("Show pop-up for highlights"), "PopupFlags", GC_EVENT_HIGHLIGHT, 0},
	{ LPGENT("Show pop-up for users leaving"), "PopupFlags", GC_EVENT_PART, 0},
	{ LPGENT("Show pop-up for users kicking other user"), "PopupFlags", GC_EVENT_KICK, 0},
	{ LPGENT("Show pop-up for notices "), "PopupFlags", GC_EVENT_NOTICE, 0},
	{ LPGENT("Show pop-up for name changes"), "PopupFlags", GC_EVENT_NICK, 0},
	{ LPGENT("Show pop-up for information messages"), "PopupFlags", GC_EVENT_INFORMATION, 0},
	{ LPGENT("Show pop-up for status changes"), "PopupFlags", GC_EVENT_ADDSTATUS, 0},
};

HTREEITEM hItemB0[SIZEOF(branch0)];
HTREEITEM hItemB1[SIZEOF(branch1)];
HTREEITEM hItemB2[SIZEOF(branch2)];
HTREEITEM hItemB3[SIZEOF(branch3)];
HTREEITEM hItemB4[SIZEOF(branch4)];
HTREEITEM hItemB5[SIZEOF(branch5)];
HTREEITEM hItemB6[SIZEOF(branch6)];

static HTREEITEM InsertBranch(HWND hwndTree, char* pszDescr, BOOL bExpanded)
{
	HTREEITEM res;
	TVINSERTSTRUCT tvis = {0};

	tvis.hInsertAfter   = TVI_LAST;
	tvis.item.mask      = TVIF_TEXT|TVIF_STATE;
	tvis.item.pszText   = (TCHAR*)CallService(MS_LANGPACK_PCHARTOTCHAR, 0, (LPARAM)pszDescr);
	tvis.item.stateMask = bExpanded ? TVIS_STATEIMAGEMASK | TVIS_EXPANDED : TVIS_STATEIMAGEMASK;
	tvis.item.state     = bExpanded ? INDEXTOSTATEIMAGEMASK(1) | TVIS_EXPANDED : INDEXTOSTATEIMAGEMASK(1);
	res = TreeView_InsertItem(hwndTree, &tvis);
	mir_free(tvis.item.pszText);
	return res;
}

static void FillBranch(HWND hwndTree, HTREEITEM hParent, const struct branch_t *branch, HTREEITEM *hItemB, int nValues, DWORD defaultval)
{
	TVINSERTSTRUCT tvis;
	int i;
	int iState;

	if (hParent == 0)
		return;

	tvis.hParent=hParent;
	tvis.hInsertAfter=TVI_LAST;
	tvis.item.mask=TVIF_TEXT|TVIF_STATE;
	for (i=0;i<nValues;i++) {
		tvis.item.pszText = TranslateTS(branch[i].szDescr);
		tvis.item.stateMask = TVIS_STATEIMAGEMASK;
		if (branch[i].iMode)
			iState = ((DBGetContactSettingDword(NULL, "Chat", branch[i].szDBName, defaultval)&branch[i].iMode)&branch[i].iMode)!=0?2:1;
		else
			iState = DBGetContactSettingByte(NULL, "Chat", branch[i].szDBName, branch[i].bDefault)!=0?2:1;
		tvis.item.state=INDEXTOSTATEIMAGEMASK(iState);
		hItemB[i] = TreeView_InsertItem(hwndTree, &tvis);
}	}

static void SaveBranch(HWND hwndTree, const struct branch_t *branch, HTREEITEM *hItemB, int nValues)
{
	TVITEM tvi;
	BYTE bChecked;
	int i;
	int iState = 0;

	tvi.mask=TVIF_HANDLE|TVIF_STATE;
	for (i=0;i<nValues;i++) {
		tvi.hItem = hItemB[i];
		TreeView_GetItem(hwndTree,&tvi);
		bChecked = ((tvi.state&TVIS_STATEIMAGEMASK)>>12==1)?0:1;
		if(branch[i].iMode) {
			if (bChecked)
				iState |= branch[i].iMode;
			if (iState&GC_EVENT_ADDSTATUS)
				iState |= GC_EVENT_REMOVESTATUS;
			DBWriteContactSettingDword(NULL, "Chat", branch[i].szDBName, (DWORD)iState);
		}
		else DBWriteContactSettingByte(NULL, "Chat", branch[i].szDBName, bChecked);
}	}

static void CheckHeading(HWND hwndTree, HTREEITEM hHeading)
{
	BOOL bChecked = TRUE;
	TVITEM tvi;

	if (hHeading == 0)
		return;

	tvi.mask=TVIF_HANDLE|TVIF_STATE;
	tvi.hItem=TreeView_GetNextItem(hwndTree, hHeading, TVGN_CHILD);
	while(tvi.hItem && bChecked) {
		if (tvi.hItem != hItemB1[0] && tvi.hItem != hItemB1[1] ) {
			TreeView_GetItem(hwndTree,&tvi);
			if (((tvi.state & TVIS_STATEIMAGEMASK)>>12 == 1))
				bChecked = FALSE;
		}
		tvi.hItem=TreeView_GetNextSibling(hwndTree,tvi.hItem);
	}
	tvi.stateMask = TVIS_STATEIMAGEMASK;
	tvi.state = INDEXTOSTATEIMAGEMASK(bChecked?2:1);
	tvi.hItem = hHeading;
	TreeView_SetItem(hwndTree,&tvi);
}

static void CheckBranches(HWND hwndTree, HTREEITEM hHeading)
{
	BOOL bChecked = TRUE;
	TVITEM tvi;

	if (hHeading == 0)
		return;

	tvi.mask=TVIF_HANDLE|TVIF_STATE;
	tvi.hItem = hHeading;
	TreeView_GetItem(hwndTree,&tvi);
	if (((tvi.state&TVIS_STATEIMAGEMASK)>>12==2))
		bChecked = FALSE;
	tvi.hItem=TreeView_GetNextItem(hwndTree, hHeading, TVGN_CHILD);
	tvi.stateMask = TVIS_STATEIMAGEMASK;
	while(tvi.hItem) {
		tvi.state=INDEXTOSTATEIMAGEMASK(bChecked?2:1);
		if (tvi.hItem !=hItemB1[0] && tvi.hItem != hItemB1[1] )
			TreeView_SetItem(hwndTree,&tvi);
		tvi.hItem=TreeView_GetNextSibling(hwndTree,tvi.hItem);
}	}

static INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
	TCHAR szDir[MAX_PATH];
	switch(uMsg) {
	case BFFM_INITIALIZED:
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
		break;

	case BFFM_SELCHANGED:
		if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir))
			SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
		break;
	}
	return 0;
}

void LoadLogFonts(void)
{
	int i;

	for ( i = 0; i<OPTIONS_FONTCOUNT; i++)
		LoadMsgDlgFont(i, &aFonts[i].lf, &aFonts[i].color);
}

void LoadMsgDlgFont(int i, LOGFONT* lf, COLORREF* colour)
{
	char str[32];
	int style;
	DBVARIANT dbv;

	if (colour) {
		wsprintfA(str, "Font%dCol", i);
		*colour = DBGetContactSettingDword(NULL, "ChatFonts", str, fontOptionsList[i].defColour);
	}
	if (lf) {
		wsprintfA(str, "Font%dSize", i);
		lf->lfHeight = (char) DBGetContactSettingByte(NULL, "ChatFonts", str, fontOptionsList[i].defSize);
		lf->lfWidth = 0;
		lf->lfEscapement = 0;
		lf->lfOrientation = 0;
		wsprintfA(str, "Font%dSty", i);
		style = DBGetContactSettingByte(NULL, "ChatFonts", str, fontOptionsList[i].defStyle);
		lf->lfWeight = style & FONTF_BOLD ? FW_BOLD : FW_NORMAL;
		lf->lfItalic = style & FONTF_ITALIC ? 1 : 0;
		lf->lfUnderline = 0;
		lf->lfStrikeOut = 0;
		wsprintfA(str, "Font%dSet", i);
		lf->lfCharSet = DBGetContactSettingByte(NULL, "ChatFonts", str, fontOptionsList[i].defCharset);
		lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf->lfQuality = DEFAULT_QUALITY;
		lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		wsprintfA(str, "Font%d", i);
		if (DBGetContactSettingTString(NULL, "ChatFonts", str, &dbv))
			lstrcpy(lf->lfFaceName, fontOptionsList[i].szDefFace);
		else {
			lstrcpyn(lf->lfFaceName, dbv.ptszVal, SIZEOF(lf->lfFaceName));
			DBFreeVariant(&dbv);
}	}	}

void RegisterFonts( void )
{
	FontIDT fontid = {0};
	ColourIDT colourid;
	char idstr[10];
	int index = 0, i;

	fontid.cbSize = sizeof(FontIDT);
	fontid.flags = FIDF_ALLOWREREGISTER | FIDF_DEFAULTVALID | FIDF_NEEDRESTART;
	for (i = 0; i < msgDlgFontCount; i++, index++) {
		strncpy(fontid.dbSettingsGroup, "ChatFonts", sizeof(fontid.dbSettingsGroup));
		_tcsncpy(fontid.group, _T("Chat Module"), SIZEOF(fontid.group));
		_tcsncpy(fontid.name, fontOptionsList[i].szDescr, SIZEOF(fontid.name));
		sprintf(idstr, "Font%d", index);
		strncpy(fontid.prefix, idstr, sizeof(fontid.prefix));
		fontid.order = index;

		fontid.deffontsettings.charset = fontOptionsList[i].defCharset;
		fontid.deffontsettings.colour = fontOptionsList[i].defColour;
		fontid.deffontsettings.size = fontOptionsList[i].defSize;
		fontid.deffontsettings.style = fontOptionsList[i].defStyle;
		_tcsncpy(fontid.deffontsettings.szFace, fontOptionsList[i].szDefFace, SIZEOF(fontid.deffontsettings.szFace));
		_tcsncpy(fontid.backgroundGroup, _T("Chat Module"), SIZEOF(fontid.backgroundGroup));
		switch (i) {
		case 17:
			_tcsncpy(fontid.backgroundName, _T("Message Background"), SIZEOF(fontid.backgroundName));
			break;
		case 18:
		case 19:
			_tcsncpy(fontid.backgroundName, _T("Userlist Background"), SIZEOF(fontid.backgroundName));
			break;
		default:
			_tcsncpy(fontid.backgroundName, _T("Background"), SIZEOF(fontid.backgroundName));
			break;
		}
		CallService(MS_FONT_REGISTERT, (WPARAM)&fontid, 0);
	}

	colourid.cbSize = sizeof(ColourIDT);
	colourid.order = 0;
	strncpy(colourid.dbSettingsGroup, "Chat", sizeof(colourid.dbSettingsGroup));

	strncpy(colourid.setting, "ColorLogBG", SIZEOF(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("Background"), SIZEOF(colourid.name));
	_tcsncpy(colourid.group, LPGENT("Chat Module"), SIZEOF(colourid.group));
	colourid.defcolour = GetSysColor(COLOR_WINDOW);
	CallService(MS_COLOUR_REGISTERT, (WPARAM)&colourid, 0);

	strncpy(colourid.setting, "ColorMessageBG", SIZEOF(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("Message Background"), SIZEOF(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_WINDOW);
	CallService(MS_COLOUR_REGISTERT, (WPARAM)&colourid, 0);

	strncpy(colourid.setting, "ColorNicklistBG", SIZEOF(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("Userlist Background"), SIZEOF(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_WINDOW);
	CallService(MS_COLOUR_REGISTERT, (WPARAM)&colourid, 0);

	strncpy(colourid.setting, "ColorNicklistLines", SIZEOF(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("Userlist Lines"), SIZEOF(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_INACTIVEBORDER);
	CallService(MS_COLOUR_REGISTERT, (WPARAM)&colourid, 0);

	strncpy(colourid.setting, "ColorNicklistSelectedBG", SIZEOF(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("Userlist Background (selected)"), SIZEOF(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_HIGHLIGHT);
	CallService(MS_COLOUR_REGISTERT, (WPARAM)&colourid, 0);
}

// add icons to the skinning module

struct
{
	int	size;
	const char* szSection;
	const char* szDescr;
	const char* szName;
	int   defIconID;
}
static const iconList[] =
{
	{	 0, LPGEN("Messaging") "/" LPGEN("Group Chats"), LPGEN("Window Icon"),           "chat_window",    IDI_CHANMGR    },
	{	 0, LPGEN("Messaging") "/" LPGEN("Group Chats"), LPGEN("Text colour"),           "chat_fgcol",     IDI_COLOR      },
	{	 0, LPGEN("Messaging") "/" LPGEN("Group Chats"), LPGEN("Background colour"),     "chat_bkgcol",    IDI_BKGCOLOR   },
	{	 0, LPGEN("Messaging") "/" LPGEN("Group Chats"), LPGEN("Bold"),                  "chat_bold",      IDI_BBOLD      },
	{	 0, LPGEN("Messaging") "/" LPGEN("Group Chats"), LPGEN("Italics"),               "chat_italics",   IDI_BITALICS   },
	{	 0, LPGEN("Messaging") "/" LPGEN("Group Chats"), LPGEN("Underlined"),            "chat_underline", IDI_BUNDERLINE },
	{	 0, LPGEN("Messaging") "/" LPGEN("Group Chats"), LPGEN("Smiley button"),         "chat_smiley",    IDI_BSMILEY    },
	{	 0, LPGEN("Messaging") "/" LPGEN("Group Chats"), LPGEN("Room history"),          "chat_history",   IDI_HISTORY    },
	{	 0, LPGEN("Messaging") "/" LPGEN("Group Chats"), LPGEN("Room settings"),         "chat_settings",  IDI_TOPICBUT   },
	{	 0, LPGEN("Messaging") "/" LPGEN("Group Chats"), LPGEN("Event filter disabled"), "chat_filter",    IDI_FILTER     },
	{	 0, LPGEN("Messaging") "/" LPGEN("Group Chats"), LPGEN("Event filter enabled"),  "chat_filter2",   IDI_FILTER2    },
	{	 0, LPGEN("Messaging") "/" LPGEN("Group Chats"), LPGEN("Hide userlist"),         "chat_nicklist",  IDI_NICKLIST   },
	{	 0, LPGEN("Messaging") "/" LPGEN("Group Chats"), LPGEN("Show userlist"),         "chat_nicklist2", IDI_NICKLIST2  },
	{	 0, LPGEN("Messaging") "/" LPGEN("Group Chats"), LPGEN("Icon overlay"),          "chat_overlay",   IDI_OVERLAY    },
	{	 0, LPGEN("Messaging") "/" LPGEN("Group Chats"), LPGEN("Close"),                 "chat_close",     IDI_CLOSE      },

	{	10, LPGEN("Messaging") "/" LPGEN("Group Chats"), LPGEN("Status 1 (10x10)"),      "chat_status0",   IDI_STATUS0    },
	{	10, LPGEN("Messaging") "/" LPGEN("Group Chats"), LPGEN("Status 2 (10x10)"),      "chat_status1",   IDI_STATUS1    },
	{	10, LPGEN("Messaging") "/" LPGEN("Group Chats"), LPGEN("Status 3 (10x10)"),      "chat_status2",   IDI_STATUS2    },
	{	10, LPGEN("Messaging") "/" LPGEN("Group Chats"), LPGEN("Status 4 (10x10)"),      "chat_status3",   IDI_STATUS3    },
	{	10, LPGEN("Messaging") "/" LPGEN("Group Chats"), LPGEN("Status 5 (10x10)"),      "chat_status4",   IDI_STATUS4    },
	{	10, LPGEN("Messaging") "/" LPGEN("Group Chats"), LPGEN("Status 6 (10x10)"),      "chat_status5",   IDI_STATUS5    },

	{	10, LPGEN("Messaging") "/" LPGEN("Group Chats Log"), LPGEN("Message in (10x10)"),    "chat_log_message_in",   IDI_MESSAGE    },
	{	10, LPGEN("Messaging") "/" LPGEN("Group Chats Log"), LPGEN("Message out (10x10)"),   "chat_log_message_out",  IDI_MESSAGEOUT },
	{	10, LPGEN("Messaging") "/" LPGEN("Group Chats Log"), LPGEN("Action (10x10)"),        "chat_log_action",       IDI_ACTION     },
	{	10, LPGEN("Messaging") "/" LPGEN("Group Chats Log"), LPGEN("Add Status (10x10)"),    "chat_log_addstatus",    IDI_ADDSTATUS  },
	{	10, LPGEN("Messaging") "/" LPGEN("Group Chats Log"), LPGEN("Remove status (10x10)"), "chat_log_removestatus", IDI_REMSTATUS  },
	{	10, LPGEN("Messaging") "/" LPGEN("Group Chats Log"), LPGEN("Join (10x10)"),          "chat_log_join",         IDI_JOIN       },
	{	10, LPGEN("Messaging") "/" LPGEN("Group Chats Log"), LPGEN("Leave (10x10)"),         "chat_log_part",         IDI_PART       },
	{	10, LPGEN("Messaging") "/" LPGEN("Group Chats Log"), LPGEN("Quit (10x10)"),          "chat_log_quit",         IDI_QUIT       },
	{	10, LPGEN("Messaging") "/" LPGEN("Group Chats Log"), LPGEN("Kick (10x10)"),          "chat_log_kick",         IDI_KICK       },
	{	10, LPGEN("Messaging") "/" LPGEN("Group Chats Log"), LPGEN("Nickchange (10x10)"),    "chat_log_nick",         IDI_NICK       },
	{	10, LPGEN("Messaging") "/" LPGEN("Group Chats Log"), LPGEN("Notice (10x10)"),        "chat_log_notice",       IDI_NOTICE     },
	{	10, LPGEN("Messaging") "/" LPGEN("Group Chats Log"), LPGEN("Topic (10x10)"),         "chat_log_topic",        IDI_TOPIC      },
	{	10, LPGEN("Messaging") "/" LPGEN("Group Chats Log"), LPGEN("Highlight (10x10)"),     "chat_log_highlight",    IDI_HIGHLIGHT  },
	{	10, LPGEN("Messaging") "/" LPGEN("Group Chats Log"), LPGEN("Information (10x10)"),   "chat_log_info",         IDI_INFO       }
};

void AddIcons(void)
{
	int i;
	SKINICONDESC sid = {0};
	char szFile[MAX_PATH];
	GetModuleFileNameA(g_hInst, szFile, MAX_PATH);

	sid.cbSize = sizeof(SKINICONDESC);
	sid.pszDefaultFile = szFile;

	for ( i = 0; i < SIZEOF(iconList); i++ ) {
		sid.cx = sid.cy = iconList[i].size;
		sid.pszSection = (char*)iconList[i].szSection;
		sid.pszDescription = (char*)iconList[i].szDescr;
		sid.pszName = (char*)iconList[i].szName;
		sid.iDefaultIndex = -iconList[i].defIconID;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
}	}

// load icons from the skinning module if available
HICON LoadIconEx( char* pszIcoLibName, BOOL big )
{
	char szTemp[256];
	mir_snprintf(szTemp, SIZEOF(szTemp), "chat_%s", pszIcoLibName);
	return (HICON) CallService(MS_SKIN2_GETICON, big, (LPARAM)szTemp);
}

static void InitSetting(TCHAR** ppPointer, char* pszSetting, TCHAR* pszDefault)
{
	DBVARIANT dbv;
	if ( !DBGetContactSettingTString(NULL, "Chat", pszSetting, &dbv )) {
		replaceStr( ppPointer, dbv.ptszVal );
		DBFreeVariant(&dbv);
	}
	else replaceStr( ppPointer, pszDefault );
}

/////////////////////////////////////////////////////////////////////////////////////////
// General options

#define OPT_FIXHEADINGS (WM_USER+1)

static INT_PTR CALLBACK DlgProcOptions1(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static HTREEITEM hListHeading1 = 0;
	static HTREEITEM hListHeading2= 0;
	static HTREEITEM hListHeading3= 0;
	static HTREEITEM hListHeading4= 0;
	static HTREEITEM hListHeading5= 0;
	static HTREEITEM hListHeading6= 0;
	static HTREEITEM hListHeading0= 0;
	switch (uMsg) 	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_CHECKBOXES),GWL_STYLE,GetWindowLongPtr(GetDlgItem(hwndDlg,IDC_CHECKBOXES),GWL_STYLE)|TVS_NOHSCROLL|TVS_CHECKBOXES);
		hListHeading0 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), LPGEN("Options for using a tabbed interface"), DBGetContactSettingByte(NULL, "Chat", "Branch0Exp", 0)?TRUE:FALSE);
		hListHeading1 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), LPGEN("Appearance and functionality of chat room windows"), DBGetContactSettingByte(NULL, "Chat", "Branch1Exp", 0)?TRUE:FALSE);
		hListHeading2 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), LPGEN("Appearance of the message log"), DBGetContactSettingByte(NULL, "Chat", "Branch2Exp", 0)?TRUE:FALSE);
		hListHeading3 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), LPGEN("Default events to show in new chat rooms if the \'event filter\' is enabled"), DBGetContactSettingByte(NULL, "Chat", "Branch3Exp", 0)?TRUE:FALSE);
		hListHeading4 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), LPGEN("Icons to display in the message log"), DBGetContactSettingByte(NULL, "Chat", "Branch4Exp", 0)?TRUE:FALSE);
		hListHeading5 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), LPGEN("Icons to display in the tray"), DBGetContactSettingByte(NULL, "Chat", "Branch5Exp", 0)?TRUE:FALSE);
		if (PopUpInstalled)
			hListHeading6 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), LPGEN("Pop-ups to display"), DBGetContactSettingByte(NULL, "Chat", "Branch6Exp", 0)?TRUE:FALSE);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading0, branch0, hItemB0, SIZEOF(branch0), 0);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading1, branch1, hItemB1, SIZEOF(branch1), 0);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading2, branch2, hItemB2, SIZEOF(branch2), 0);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading3, branch3, hItemB3, SIZEOF(branch3), 0x03E0);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading4, branch4, hItemB4, SIZEOF(branch4), 0x0000);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading5, branch5, hItemB5, SIZEOF(branch5), 0x1000);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading6, branch6, hItemB6, SIZEOF(branch6), 0x0000);
		SendMessage(hwndDlg, OPT_FIXHEADINGS, 0, 0);
		break;

	case OPT_FIXHEADINGS:
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading1);
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading2);
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading3);
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading4);
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading5);
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading6);
		break;

	case WM_COMMAND:
		if (lParam != 0)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		{
			switch(((LPNMHDR)lParam)->idFrom) {
			case IDC_CHECKBOXES:
				if (((LPNMHDR)lParam)->code==NM_CLICK) {
					TVHITTESTINFO hti;
					hti.pt.x=(short)LOWORD(GetMessagePos());
					hti.pt.y=(short)HIWORD(GetMessagePos());
					ScreenToClient(((LPNMHDR)lParam)->hwndFrom,&hti.pt);
					if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom,&hti)) {
						if (hti.flags&TVHT_ONITEMSTATEICON) {
							TVITEM tvi = {0};
							tvi.mask=TVIF_HANDLE|TVIF_STATE;
							tvi.hItem=hti.hItem;
							TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom,&tvi);
							if (tvi.hItem == hItemB1[0] && INDEXTOSTATEIMAGEMASK(1)==tvi.state)
								TreeView_SetItemState(((LPNMHDR)lParam)->hwndFrom, hItemB1[1], INDEXTOSTATEIMAGEMASK(1),  TVIS_STATEIMAGEMASK);
							if (tvi.hItem == hItemB1[1] && INDEXTOSTATEIMAGEMASK(1)==tvi.state)
								TreeView_SetItemState(((LPNMHDR)lParam)->hwndFrom, hItemB1[0], INDEXTOSTATEIMAGEMASK(1),  TVIS_STATEIMAGEMASK);

							if (tvi.hItem == hListHeading0)
								CheckBranches(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading0);
							else if (tvi.hItem == hListHeading1)
								CheckBranches(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading1);
							else if (tvi.hItem == hListHeading2)
								CheckBranches(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading2);
							else if (tvi.hItem == hListHeading3)
								CheckBranches(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading3);
							else if (tvi.hItem == hListHeading4)
								CheckBranches(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading4);
							else if (tvi.hItem == hListHeading5)
								CheckBranches(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading5);
							else if (tvi.hItem == hListHeading6)
								CheckBranches(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading6);
							else
								PostMessage(hwndDlg, OPT_FIXHEADINGS, 0, 0);
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}	}	}
				break;

			case 0:
				switch (((LPNMHDR)lParam)->code) {
				case PSN_APPLY:
					{
						BYTE b = DBGetContactSettingByte(NULL, "Chat", "Tabs", 1);
						SaveBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), branch0, hItemB0, SIZEOF(branch0));
						SaveBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), branch1, hItemB1, SIZEOF(branch1));
						SaveBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), branch2, hItemB2, SIZEOF(branch2));
						SaveBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), branch3, hItemB3, SIZEOF(branch3));
						SaveBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), branch4, hItemB4, SIZEOF(branch4));
						SaveBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), branch5, hItemB5, SIZEOF(branch5));
						if (PopUpInstalled)
							SaveBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), branch6, hItemB6, SIZEOF(branch6));
						g_Settings.dwIconFlags = DBGetContactSettingDword(NULL, "Chat", "IconFlags", 0x0000);
						g_Settings.dwTrayIconFlags = DBGetContactSettingDword(NULL, "Chat", "TrayIconFlags", 0x1000);
						g_Settings.dwPopupFlags = DBGetContactSettingDword(NULL, "Chat", "PopupFlags", 0x0000);
						g_Settings.StripFormat = (BOOL)DBGetContactSettingByte(NULL, "Chat", "TrimFormatting", 0);
						g_Settings.TrayIconInactiveOnly = (BOOL)DBGetContactSettingByte(NULL, "Chat", "TrayIconInactiveOnly", 1);
						g_Settings.PopUpInactiveOnly = (BOOL)DBGetContactSettingByte(NULL, "Chat", "PopUpInactiveOnly", 1);
						g_Settings.LogIndentEnabled = (DBGetContactSettingByte(NULL, "Chat", "LogIndentEnabled", 1) != 0)?TRUE:FALSE;

						if (b != DBGetContactSettingByte(NULL, "Chat", "Tabs", 1)) {
							SM_BroadcastMessage(NULL, GC_CLOSEWINDOW, 0, 1, FALSE);
							g_Settings.TabsEnable = DBGetContactSettingByte(NULL, "Chat", "Tabs", 1);
						}
						else SM_BroadcastMessage(NULL, GC_SETWNDPROPS, 0, 0, TRUE);
					}
					return TRUE;
		}	}	}
		break;

	case WM_DESTROY:
		{
			BYTE b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading1, TVIS_EXPANDED)&TVIS_EXPANDED?1:0;
			DBWriteContactSettingByte(NULL, "Chat", "Branch1Exp", b);
			b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading2, TVIS_EXPANDED)&TVIS_EXPANDED?1:0;
			DBWriteContactSettingByte(NULL, "Chat", "Branch2Exp", b);
			b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading3, TVIS_EXPANDED)&TVIS_EXPANDED?1:0;
			DBWriteContactSettingByte(NULL, "Chat", "Branch3Exp", b);
			b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading4, TVIS_EXPANDED)&TVIS_EXPANDED?1:0;
			DBWriteContactSettingByte(NULL, "Chat", "Branch4Exp", b);
			b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading5, TVIS_EXPANDED)&TVIS_EXPANDED?1:0;
			DBWriteContactSettingByte(NULL, "Chat", "Branch5Exp", b);
			b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading0, TVIS_EXPANDED)&TVIS_EXPANDED?1:0;
			DBWriteContactSettingByte(NULL, "Chat", "Branch0Exp", b);
			if (PopUpInstalled) {
				b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading6, TVIS_EXPANDED)&TVIS_EXPANDED?1:0;
				DBWriteContactSettingByte(NULL, "Chat", "Branch6Exp", b);
		}	}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Log & other options

static INT_PTR CALLBACK DlgProcOptions2(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SendDlgItemMessage(hwndDlg,IDC_SPIN2,UDM_SETRANGE,0,MAKELONG(5000,0));
		SendDlgItemMessage(hwndDlg,IDC_SPIN2,UDM_SETPOS,0,MAKELONG(DBGetContactSettingWord(NULL,"Chat","LogLimit",100),0));
		SendDlgItemMessage(hwndDlg,IDC_SPIN3,UDM_SETRANGE,0,MAKELONG(10000,0));
		SendDlgItemMessage(hwndDlg,IDC_SPIN3,UDM_SETPOS,0,MAKELONG(DBGetContactSettingWord(NULL,"Chat","LoggingLimit",100),0));
		SendDlgItemMessage(hwndDlg,IDC_SPIN4,UDM_SETRANGE,0,MAKELONG(255,10));
		SendDlgItemMessage(hwndDlg,IDC_SPIN4,UDM_SETPOS,0,MAKELONG(DBGetContactSettingByte(NULL,"Chat","NicklistRowDist",12),0));
		{
			TCHAR* pszGroup = NULL;
			InitSetting(&pszGroup, "AddToGroup", _T("Chat rooms"));
			SetWindowText(GetDlgItem(hwndDlg, IDC_GROUP), pszGroup);
			mir_free(pszGroup);
		}
		{
			TCHAR szTemp[MAX_PATH];
			CallService(MS_UTILS_PATHTORELATIVET, (WPARAM)g_Settings.pszLogDir, (LPARAM)szTemp );
			SetDlgItemText(hwndDlg, IDC_LOGDIRECTORY, szTemp);
		}
		SetDlgItemText(hwndDlg, IDC_HIGHLIGHTWORDS, g_Settings.pszHighlightWords);
		SetDlgItemText(hwndDlg, IDC_LOGTIMESTAMP, g_Settings.pszTimeStampLog);
		SetDlgItemText(hwndDlg, IDC_TIMESTAMP, g_Settings.pszTimeStamp);
		SetDlgItemText(hwndDlg, IDC_OUTSTAMP, g_Settings.pszOutgoingNick);
		SetDlgItemText(hwndDlg, IDC_INSTAMP, g_Settings.pszIncomingNick);
		CheckDlgButton(hwndDlg, IDC_HIGHLIGHT, g_Settings.HighlightEnabled);
		EnableWindow(GetDlgItem(hwndDlg, IDC_HIGHLIGHTWORDS), g_Settings.HighlightEnabled?TRUE:FALSE);
		CheckDlgButton(hwndDlg, IDC_LOGGING, g_Settings.LoggingEnabled);
		EnableWindow(GetDlgItem(hwndDlg, IDC_LOGDIRECTORY), g_Settings.LoggingEnabled?TRUE:FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_FONTCHOOSE), g_Settings.LoggingEnabled?TRUE:FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_LIMIT), g_Settings.LoggingEnabled?TRUE:FALSE);
		break;

	case WM_COMMAND:
		if (( LOWORD(wParam) == IDC_INSTAMP
			|| LOWORD(wParam) == IDC_OUTSTAMP
			|| LOWORD(wParam) == IDC_TIMESTAMP
			|| LOWORD(wParam) == IDC_LOGLIMIT
			|| LOWORD(wParam) == IDC_HIGHLIGHTWORDS
			|| LOWORD(wParam) == IDC_LOGDIRECTORY
			|| LOWORD(wParam) == IDC_LOGTIMESTAMP
			|| LOWORD(wParam) == IDC_NICKROW2
			|| LOWORD(wParam) == IDC_GROUP
			|| LOWORD(wParam) == IDC_LIMIT)
			&& (HIWORD(wParam)!=EN_CHANGE || (HWND)lParam!=GetFocus()))	return 0;

		switch (LOWORD(wParam)) {
		case IDC_LOGGING:
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOGDIRECTORY), IsDlgButtonChecked(hwndDlg, IDC_LOGGING) == BST_CHECKED?TRUE:FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FONTCHOOSE), IsDlgButtonChecked(hwndDlg, IDC_LOGGING) == BST_CHECKED?TRUE:FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMIT), IsDlgButtonChecked(hwndDlg, IDC_LOGGING) == BST_CHECKED?TRUE:FALSE);
			break;

		case IDC_FONTCHOOSE:
		{
			LPITEMIDLIST idList;
			BROWSEINFO bi = {0};
			TCHAR szDirectory[MAX_PATH];
			TCHAR szTemp[MAX_PATH];

			bi.hwndOwner=hwndDlg;
			bi.pszDisplayName=szDirectory;
			bi.lpszTitle=TranslateT("Select Folder");
			bi.ulFlags=BIF_NEWDIALOGSTYLE|BIF_EDITBOX|BIF_RETURNONLYFSDIRS;			
			bi.lpfn=BrowseCallbackProc;
			bi.lParam=(LPARAM)szDirectory;
			idList = SHBrowseForFolder(&bi);
			if ( idList ) {
				SHGetPathFromIDList(idList,szDirectory);
				lstrcat(szDirectory, _T("\\"));
				CallService(MS_UTILS_PATHTORELATIVET, (WPARAM)szDirectory, (LPARAM)szTemp);
				SetDlgItemText(hwndDlg, IDC_LOGDIRECTORY, lstrlen(szTemp) > 1 ? szTemp : _T("Logs\\"));
				CoTaskMemFree(idList);
			}
			break;
		}
		case IDC_HIGHLIGHT:
			EnableWindow(GetDlgItem(hwndDlg, IDC_HIGHLIGHTWORDS), IsDlgButtonChecked(hwndDlg, IDC_HIGHLIGHT) == BST_CHECKED?TRUE:FALSE);
			break;
		}

		if (lParam != (LPARAM)NULL)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0 && ((LPNMHDR)lParam)->code == PSN_APPLY ) {
			int iLen;
			TCHAR * pszText = NULL;

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_HIGHLIGHTWORDS));
			if ( iLen > 0 ) {
				TCHAR *ptszText = mir_alloc((iLen+2) * sizeof(TCHAR));
				TCHAR *p2 = NULL;
				
				if(ptszText) {
				    GetDlgItemText(hwndDlg, IDC_HIGHLIGHTWORDS, ptszText, iLen + 1);
				    p2 = _tcschr(ptszText, ',');
				    while ( p2 ) {
					   *p2 = ' ';
					   p2 = _tcschr(ptszText, ',');
				    }
				    DBWriteContactSettingTString(NULL, "Chat", "HighlightWords", ptszText);
				    mir_free(ptszText);
				}
			}
			else DBDeleteContactSetting(NULL, "Chat", "HighlightWords");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_LOGDIRECTORY));
			if ( iLen > 0 ) {
				pszText = mir_realloc(pszText, (iLen + 1) * sizeof(TCHAR));
				GetDlgItemText(hwndDlg, IDC_LOGDIRECTORY, pszText,iLen + 1);
				DBWriteContactSettingTString(NULL, "Chat", "LogDirectory", pszText);
			}
			else DBDeleteContactSetting(NULL, "Chat", "LogDirectory");

			CallService(MS_UTILS_PATHTOABSOLUTET, (WPARAM)pszText, (LPARAM)g_Settings.pszLogDir);

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_LOGTIMESTAMP));
			if ( iLen > 0 ) {
				pszText = mir_realloc(pszText, (iLen + 1) * sizeof(TCHAR));
				GetDlgItemText(hwndDlg, IDC_LOGTIMESTAMP, pszText, iLen+1);
				DBWriteContactSettingTString(NULL, "Chat", "LogTimestamp", pszText);
			}
			else DBDeleteContactSetting(NULL, "Chat", "LogTimestamp");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_TIMESTAMP));
			if ( iLen > 0 ) {
				pszText = mir_realloc(pszText, (iLen + 1) * sizeof(TCHAR));
				GetDlgItemText(hwndDlg, IDC_TIMESTAMP, pszText, iLen+1);
				DBWriteContactSettingTString(NULL, "Chat", "HeaderTime", pszText);
			}
			else DBDeleteContactSetting(NULL, "Chat", "HeaderTime");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_INSTAMP));
			if ( iLen > 0 ) {
				pszText = mir_realloc(pszText, (iLen + 1) * sizeof(TCHAR));
				GetDlgItemText(hwndDlg, IDC_INSTAMP, pszText,iLen+1);
				DBWriteContactSettingTString(NULL, "Chat", "HeaderIncoming", pszText);
			}
			else DBDeleteContactSetting(NULL, "Chat", "HeaderIncoming");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_OUTSTAMP));
			if ( iLen > 0 ) {
				pszText = mir_realloc(pszText, (iLen + 1) * sizeof(TCHAR));
				GetDlgItemText(hwndDlg, IDC_OUTSTAMP, pszText,iLen+1);
				DBWriteContactSettingTString(NULL, "Chat", "HeaderOutgoing", pszText);
			}
			else DBDeleteContactSetting(NULL, "Chat", "HeaderOutgoing");

			g_Settings.HighlightEnabled = IsDlgButtonChecked(hwndDlg, IDC_HIGHLIGHT) == BST_CHECKED?TRUE:FALSE;
			DBWriteContactSettingByte(NULL, "Chat", "HighlightEnabled", (BYTE)g_Settings.HighlightEnabled);

			g_Settings.LoggingEnabled = IsDlgButtonChecked(hwndDlg, IDC_LOGGING) == BST_CHECKED?TRUE:FALSE;
			DBWriteContactSettingByte(NULL, "Chat", "LoggingEnabled", (BYTE)g_Settings.LoggingEnabled);
			if ( g_Settings.LoggingEnabled )
				CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)g_Settings.pszLogDir);

			iLen = SendDlgItemMessage(hwndDlg,IDC_SPIN2,UDM_GETPOS,0,0);
			DBWriteContactSettingWord(NULL, "Chat", "LogLimit", (WORD)iLen);
			iLen = SendDlgItemMessage(hwndDlg,IDC_SPIN3,UDM_GETPOS,0,0);
			DBWriteContactSettingWord(NULL, "Chat", "LoggingLimit", (WORD)iLen);

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_GROUP));
			if (iLen > 0) {
				pszText = mir_realloc(pszText, (iLen + 1) * sizeof(TCHAR));
				GetDlgItemText(hwndDlg, IDC_GROUP, pszText, iLen+1);
				DBWriteContactSettingTString(NULL, "Chat", "AddToGroup", pszText);
			}
			else DBWriteContactSettingString(NULL, "Chat", "AddToGroup", "");
			mir_free(pszText);

			iLen = SendDlgItemMessage(hwndDlg,IDC_SPIN4,UDM_GETPOS,0,0);
			if (iLen > 0)
				DBWriteContactSettingByte(NULL, "Chat", "NicklistRowDist", (BYTE)iLen);
			else
				DBDeleteContactSetting(NULL, "Chat", "NicklistRowDist");

			FreeMsgLogBitmaps();
			LoadMsgLogBitmaps();
			SM_BroadcastMessage(NULL, GC_SETWNDPROPS, 0, 0, TRUE);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Popup options

static INT_PTR CALLBACK DlgProcOptionsPopup(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		SendDlgItemMessage(hwndDlg, IDC_BKG, CPM_SETCOLOUR,0,g_Settings.crPUBkgColour);
		SendDlgItemMessage(hwndDlg, IDC_TEXT, CPM_SETCOLOUR,0,g_Settings.crPUTextColour);

		if (g_Settings.iPopupStyle ==2)
			CheckDlgButton(hwndDlg, IDC_RADIO2, BST_CHECKED);
		else if (g_Settings.iPopupStyle ==3)
			CheckDlgButton(hwndDlg, IDC_RADIO3, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_RADIO1, BST_CHECKED);

		EnableWindow(GetDlgItem(hwndDlg, IDC_BKG), IsDlgButtonChecked(hwndDlg, IDC_RADIO3) ==BST_CHECKED?TRUE:FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_TEXT), IsDlgButtonChecked(hwndDlg, IDC_RADIO3) ==BST_CHECKED?TRUE:FALSE);

		SendDlgItemMessage(hwndDlg,IDC_SPIN1,UDM_SETRANGE,0,MAKELONG(100,-1));
		SendDlgItemMessage(hwndDlg,IDC_SPIN1,UDM_SETPOS,0,MAKELONG(g_Settings.iPopupTimeout,0));
		break;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDC_TIMEOUT) && (HIWORD(wParam)!=EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;

		if (lParam != (LPARAM)NULL)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		switch (LOWORD(wParam)) {

		case IDC_RADIO1:
		case IDC_RADIO2:
		case IDC_RADIO3:
			EnableWindow(GetDlgItem(hwndDlg, IDC_BKG), IsDlgButtonChecked(hwndDlg, IDC_RADIO3) ==BST_CHECKED?TRUE:FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TEXT), IsDlgButtonChecked(hwndDlg, IDC_RADIO3) ==BST_CHECKED?TRUE:FALSE);
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0 && ((LPNMHDR)lParam)->code == PSN_APPLY ) {
			int iLen;

			if (IsDlgButtonChecked(hwndDlg, IDC_RADIO2) == BST_CHECKED)
				iLen = 2;
			else if (IsDlgButtonChecked(hwndDlg, IDC_RADIO3) == BST_CHECKED)
				iLen = 3;
			else
				iLen = 1;

			g_Settings.iPopupStyle = iLen;
			DBWriteContactSettingByte(NULL, "Chat", "PopupStyle", (BYTE)iLen);

			iLen = SendDlgItemMessage(hwndDlg,IDC_SPIN1,UDM_GETPOS,0,0);
			g_Settings.iPopupTimeout = iLen;
			DBWriteContactSettingWord(NULL, "Chat", "PopupTimeout", (WORD)iLen);

			g_Settings.crPUBkgColour = SendDlgItemMessage(hwndDlg,IDC_BKG,CPM_GETCOLOUR,0,0);
			DBWriteContactSettingDword(NULL, "Chat", "PopupColorBG", (DWORD)SendDlgItemMessage(hwndDlg,IDC_BKG,CPM_GETCOLOUR,0,0));
			g_Settings.crPUTextColour = SendDlgItemMessage(hwndDlg,IDC_TEXT,CPM_GETCOLOUR,0,0);
			DBWriteContactSettingDword(NULL, "Chat", "PopupColorText", (DWORD)SendDlgItemMessage(hwndDlg,IDC_TEXT,CPM_GETCOLOUR,0,0));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static int OptionsInitialize(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {0};

	odp.cbSize = sizeof(odp);
	odp.position = 910000000;
	odp.hInstance = g_hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS1);
	odp.pszGroup = LPGEN("Message Sessions");
	odp.pszTitle = LPGEN("Group chats");
	odp.pszTab = LPGEN("General");
	odp.pfnDlgProc = DlgProcOptions1;
	odp.flags = ODPF_BOLDGROUPS;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);

	odp.position = 910000001;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS2);
	odp.pszTab = LPGEN("Chat Log");
	odp.pfnDlgProc = DlgProcOptions2;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);

	if (PopUpInstalled) {
		odp.position = 910000002;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONSPOPUP);
		odp.pszTitle = LPGEN("Chat");
		odp.pszGroup = LPGEN("Popups");
		odp.pszTab = NULL;
		odp.pfnDlgProc = DlgProcOptionsPopup;
		CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
	}
	return 0;
}

void LoadGlobalSettings(void)
{
	LOGFONT lf;

	g_Settings.LogLimitNames = DBGetContactSettingByte(NULL, "Chat", "LogLimitNames", 1);
	g_Settings.ShowTime = DBGetContactSettingByte(NULL, "Chat", "ShowTimeStamp", 1);
	g_Settings.TabsEnable = DBGetContactSettingByte(NULL, "Chat", "Tabs", 1);
	g_Settings.TabsAtBottom = DBGetContactSettingByte(NULL, "Chat", "TabBottom", 0);
	g_Settings.TabCloseOnDblClick = DBGetContactSettingByte(NULL, "Chat", "TabCloseOnDblClick", 0);
	g_Settings.TabRestore = DBGetContactSettingByte(NULL, "Chat", "TabRestore", 0);
	g_Settings.SoundsFocus = DBGetContactSettingByte(NULL, "Chat", "SoundsFocus", 0);
	g_Settings.ShowTimeIfChanged = (BOOL)DBGetContactSettingByte(NULL, "Chat", "ShowTimeStampIfChanged", 0);
	g_Settings.TimeStampEventColour = (BOOL)DBGetContactSettingByte(NULL, "Chat", "TimeStampEventColour", 0);
	g_Settings.iEventLimit = DBGetContactSettingWord(NULL, "Chat", "LogLimit", 100);
	g_Settings.dwIconFlags = DBGetContactSettingDword(NULL, "Chat", "IconFlags", 0x0000);
	g_Settings.dwTrayIconFlags = DBGetContactSettingDword(NULL, "Chat", "TrayIconFlags", 0x1000);
	g_Settings.dwPopupFlags = DBGetContactSettingDword(NULL, "Chat", "PopupFlags", 0x0000);
	g_Settings.LoggingLimit = DBGetContactSettingWord(NULL, "Chat", "LoggingLimit", 100);
	g_Settings.LoggingEnabled = (BOOL)DBGetContactSettingByte(NULL, "Chat", "LoggingEnabled", 0);
	g_Settings.FlashWindow = (BOOL)DBGetContactSettingByte(NULL, "Chat", "FlashWindow", 0);
	g_Settings.HighlightEnabled = (BOOL)DBGetContactSettingByte(NULL, "Chat", "HighlightEnabled", 1);
	g_Settings.crUserListColor = DBGetContactSettingDword(NULL, "ChatFonts", "Font18Col", RGB(0,0,0));
	g_Settings.crUserListBGColor = DBGetContactSettingDword(NULL, "Chat", "ColorNicklistBG", GetSysColor(COLOR_WINDOW));
	g_Settings.crUserListSelectedBGColor = DBGetContactSettingDword(NULL, "Chat", "ColorNicklistSelectedBG", GetSysColor(COLOR_HIGHLIGHT));
	g_Settings.crUserListHeadingsColor = DBGetContactSettingDword(NULL, "ChatFonts", "Font19Col", RGB(170,170,170));
	g_Settings.crLogBackground = DBGetContactSettingDword(NULL, "Chat", "ColorLogBG", GetSysColor(COLOR_WINDOW));
	g_Settings.StripFormat = (BOOL)DBGetContactSettingByte(NULL, "Chat", "StripFormatting", 0);
	g_Settings.TrayIconInactiveOnly = (BOOL)DBGetContactSettingByte(NULL, "Chat", "TrayIconInactiveOnly", 1);
	g_Settings.PopUpInactiveOnly = (BOOL)DBGetContactSettingByte(NULL, "Chat", "PopUpInactiveOnly", 1);
	g_Settings.AddColonToAutoComplete = (BOOL)DBGetContactSettingByte(NULL, "Chat", "AddColonToAutoComplete", 1);
	g_Settings.iPopupStyle = DBGetContactSettingByte(NULL, "Chat", "PopupStyle", 1);
	g_Settings.iPopupTimeout = DBGetContactSettingWord(NULL, "Chat", "PopupTimeout", 3);
	g_Settings.crPUBkgColour = DBGetContactSettingDword(NULL, "Chat", "PopupColorBG", GetSysColor(COLOR_WINDOW));
	g_Settings.crPUTextColour = DBGetContactSettingDword(NULL, "Chat", "PopupColorText", 0);
	g_Settings.ShowContactStatus = DBGetContactSettingByte(NULL, "Chat", "ShowContactStatus", 0);
	g_Settings.ContactStatusFirst = DBGetContactSettingByte(NULL, "Chat", "ContactStatusFirst", 0);	

	InitSetting( &g_Settings.pszTimeStamp, "HeaderTime", _T("[%H:%M]"));
	InitSetting( &g_Settings.pszTimeStampLog, "LogTimestamp", _T("[%d %b %y %H:%M]"));
	InitSetting( &g_Settings.pszIncomingNick, "HeaderIncoming", _T("%n:"));
	InitSetting( &g_Settings.pszOutgoingNick, "HeaderOutgoing", _T("%n:"));
	InitSetting( &g_Settings.pszHighlightWords, "HighlightWords", _T("%m"));

	{
		TCHAR pszTemp[MAX_PATH];
		DBVARIANT dbv;
		g_Settings.pszLogDir = (TCHAR *)mir_realloc(g_Settings.pszLogDir, MAX_PATH*sizeof(TCHAR));
		if (!DBGetContactSettingTString(NULL, "Chat", "LogDirectory", &dbv)) {
			lstrcpyn(pszTemp, dbv.ptszVal, MAX_PATH);
			DBFreeVariant(&dbv);
		}
		else {
            TCHAR *tmpPath = Utils_ReplaceVarsT(_T("%miranda_logpath%\\Chat"));
            lstrcpyn(pszTemp, tmpPath, SIZEOF(pszTemp)-1);
            mir_free(tmpPath);
        }

		CallService(MS_UTILS_PATHTOABSOLUTET, (WPARAM)pszTemp, (LPARAM)g_Settings.pszLogDir);
	}

	g_Settings.LogIndentEnabled = (DBGetContactSettingByte(NULL, "Chat", "LogIndentEnabled", 1) != 0)?TRUE:FALSE;

	if ( g_Settings.MessageBoxFont )
		DeleteObject( g_Settings.MessageBoxFont );
	LoadMsgDlgFont( 17, &lf, NULL );
	g_Settings.MessageBoxFont = CreateFontIndirect(&lf);

	if ( g_Settings.UserListFont )
		DeleteObject(g_Settings.UserListFont);
	LoadMsgDlgFont(18, &lf, NULL);
	g_Settings.UserListFont = CreateFontIndirect(&lf);

	if (g_Settings.UserListHeadingsFont)
		DeleteObject(g_Settings.UserListHeadingsFont);
	LoadMsgDlgFont(19, &lf, NULL);
	g_Settings.UserListHeadingsFont = CreateFontIndirect(&lf);
	if (hListBkgBrush != NULL) {
		DeleteObject(hListBkgBrush);
	}
	hListBkgBrush = CreateSolidBrush(DBGetContactSettingDword(NULL, "Chat", "ColorNicklistBG", GetSysColor(COLOR_WINDOW)));
	if (hListSelectedBkgBrush != NULL) {
		DeleteObject(hListSelectedBkgBrush);
	}
	hListSelectedBkgBrush = CreateSolidBrush(DBGetContactSettingDword(NULL, "Chat", "ColorNicklistSelectedBG", GetSysColor(COLOR_HIGHLIGHT)));
}

static void FreeGlobalSettings(void)
{
	mir_free(g_Settings.pszTimeStamp);
	mir_free(g_Settings.pszTimeStampLog);
	mir_free(g_Settings.pszIncomingNick);
	mir_free(g_Settings.pszOutgoingNick);
	mir_free(g_Settings.pszHighlightWords);
	mir_free(g_Settings.pszLogDir);
	if ( g_Settings.MessageBoxFont )
		DeleteObject( g_Settings.MessageBoxFont );
	if ( g_Settings.UserListFont )
		DeleteObject( g_Settings.UserListFont );
	if ( g_Settings.UserListHeadingsFont )
		DeleteObject( g_Settings.UserListHeadingsFont );
}

int OptionsInit(void)
{
	LOGFONT lf;

	g_hOptions = HookEvent(ME_OPT_INITIALISE, OptionsInitialize);

	LoadLogFonts();
	LoadMsgDlgFont(18, &lf, NULL);
	lstrcpy(lf.lfFaceName, _T("MS Shell Dlg"));
	lf.lfUnderline = lf.lfItalic = lf.lfStrikeOut = 0;
	lf.lfHeight = -17;
	lf.lfWeight = FW_BOLD;
	g_Settings.NameFont = CreateFontIndirect(&lf);
	g_Settings.UserListFont = NULL;
	g_Settings.UserListHeadingsFont = NULL;
	g_Settings.MessageBoxFont = NULL;
	g_Settings.iSplitterX = DBGetContactSettingWord(NULL, "Chat", "SplitterX", 105);
	g_Settings.iSplitterY = DBGetContactSettingWord(NULL, "Chat", "SplitterY", 90);
	g_Settings.iX = DBGetContactSettingDword(NULL, "Chat", "roomx", -1);
	g_Settings.iY = DBGetContactSettingDword(NULL, "Chat", "roomy", -1);
	g_Settings.iWidth = DBGetContactSettingDword(NULL, "Chat", "roomwidth", -1);
	g_Settings.iHeight = DBGetContactSettingDword(NULL, "Chat", "roomheight", -1);
	LoadGlobalSettings();

	SkinAddNewSoundEx("ChatMessage", LPGEN("Group chats"), LPGEN("Incoming message"));
	SkinAddNewSoundEx("ChatHighlight", LPGEN("Group chats"), LPGEN("Message is highlighted"));
	SkinAddNewSoundEx("ChatAction", LPGEN("Group chats"), LPGEN("User has performed an action"));
	SkinAddNewSoundEx("ChatJoin", LPGEN("Group chats"), LPGEN("User has joined"));
	SkinAddNewSoundEx("ChatPart", LPGEN("Group chats"), LPGEN("User has left"));
	SkinAddNewSoundEx("ChatKick", LPGEN("Group chats"), LPGEN("User has kicked some other user"));
	SkinAddNewSoundEx("ChatMode", LPGEN("Group chats"), LPGEN("User's status was changed"));
	SkinAddNewSoundEx("ChatNick", LPGEN("Group chats"), LPGEN("User has changed name"));
	SkinAddNewSoundEx("ChatNotice", LPGEN("Group chats"), LPGEN("User has sent a notice"));
	SkinAddNewSoundEx("ChatQuit", LPGEN("Group chats"), LPGEN("User has disconnected"));
	SkinAddNewSoundEx("ChatTopic", LPGEN("Group chats"), LPGEN("The topic has been changed"));

	if ( g_Settings.LoggingEnabled )
		CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)g_Settings.pszLogDir);
	{
		LOGFONT lf;
		HFONT hFont;
		int iText;

		LoadMsgDlgFont(0, &lf, NULL);
		hFont = CreateFontIndirect(&lf);
		iText = GetTextPixelSize(MakeTimeStamp(g_Settings.pszTimeStamp, time(NULL)),hFont, TRUE);
		DeleteObject(hFont);
		g_Settings.LogTextIndent = iText;
		g_Settings.LogTextIndent = g_Settings.LogTextIndent*12/10;
	}

	return 0;
}

int OptionsUnInit(void)
{
	FreeGlobalSettings();
	UnhookEvent(g_hOptions);
	DeleteObject(hEditBkgBrush);
	DeleteObject(hListBkgBrush);
	DeleteObject(hListSelectedBkgBrush);
	DeleteObject(g_Settings.NameFont);
	return 0;
}
