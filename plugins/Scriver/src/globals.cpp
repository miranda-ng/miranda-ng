/*
Scriver

Copyright 2000-2012 Miranda ICQ/IM project,

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
#include "commonheaders.h"
#include "m_ieview.h"

struct GlobalMessageData *g_dat=NULL;
extern PSLWA pSetLayeredWindowAttributes;

HANDLE hEventSkin2IconsChanged;

static int ackevent(WPARAM wParam, LPARAM lParam);

extern int    Chat_ModulesLoaded(WPARAM wParam,LPARAM lParam);
extern int    Chat_PreShutdown(WPARAM wParam,LPARAM lParam);

static const char *buttonIcons[] = {"scriver_CLOSEX", "scriver_QUOTE", "scriver_SMILEY", 
									"scriver_ADD", NULL, "scriver_USERDETAILS", "scriver_HISTORY", 
									"scriver_SEND"};

static const char *chatButtonIcons[] = {"scriver_CLOSEX", 
									"chat_bold", "chat_italics", "chat_underline", 
									"chat_fgcol", "chat_bkgcol", 
									"chat_smiley", "chat_history", 
									"chat_filter", "chat_settings", "chat_nicklist", "scriver_SEND"};

typedef struct IconDefStruct 
{
	char* section;
	char *name;
	int   defaultIndex;
	char *description;
	int size;
} IconDef;

static const IconDef iconList[] = {
	{LPGEN("Single Messaging"), "scriver_ADD", IDI_ADDCONTACT, LPGEN("Add contact")},
	{LPGEN("Single Messaging"), "scriver_USERDETAILS", IDI_USERDETAILS, LPGEN("User's details")},
	{LPGEN("Single Messaging"), "scriver_HISTORY", IDI_HISTORY, LPGEN("User's history")},
	{LPGEN("Single Messaging"), "scriver_SEND", IDI_SEND, LPGEN("Send message")},
	{LPGEN("Single Messaging"), "scriver_SMILEY", IDI_SMILEY, LPGEN("Smiley button")},
	{LPGEN("Single Messaging"), "scriver_TYPING", IDI_TYPING, LPGEN("User is typing")},
	{LPGEN("Single Messaging"), "scriver_TYPINGOFF", IDI_TYPINGOFF, LPGEN("Typing notification off")},
	{LPGEN("Single Messaging"), "scriver_UNICODEON", IDI_UNICODEON, LPGEN("Unicode is on")},
	{LPGEN("Single Messaging"), "scriver_UNICODEOFF", IDI_UNICODEOFF, LPGEN("Unicode is off")},
	{LPGEN("Single Messaging"), "scriver_DELIVERING", IDI_TIMESTAMP, LPGEN("Sending")},
	{LPGEN("Single Messaging"), "scriver_QUOTE", IDI_QUOTE, LPGEN("Quote button")},
	{LPGEN("Single Messaging"), "scriver_CLOSEX", IDI_CLOSEX, LPGEN("Close button")},
	{LPGEN("Single Messaging"), "scriver_OVERLAY", IDI_OVERLAY, LPGEN("Icon overlay")},
	{LPGEN("Single Messaging"), "scriver_INCOMING", IDI_INCOMING, LPGEN("Incoming message (10x10)"),10},
	{LPGEN("Single Messaging"), "scriver_OUTGOING", IDI_OUTGOING, LPGEN("Outgoing message (10x10)"),10},
	{LPGEN("Single Messaging"), "scriver_NOTICE", IDI_NOTICE, LPGEN("Notice (10x10)"),10},
	{LPGEN("Group Chats"), "chat_window", IDI_CHANMGR, LPGEN("Window Icon")},
	{LPGEN("Group Chats"), "chat_fgcol", IDI_COLOR, LPGEN("Text colour")},
	{LPGEN("Group Chats"), "chat_bkgcol", IDI_BKGCOLOR, LPGEN("Background colour")},
	{LPGEN("Group Chats"), "chat_bold", IDI_BBOLD, LPGEN("Bold")},
	{LPGEN("Group Chats"), "chat_italics", IDI_BITALICS, LPGEN("Italics")},
	{LPGEN("Group Chats"), "chat_underline", IDI_BUNDERLINE, LPGEN("Underlined")},
	{LPGEN("Group Chats"), "chat_smiley", IDI_SMILEY, LPGEN("Smiley button")},
	{LPGEN("Group Chats"), "chat_history", IDI_HISTORY, LPGEN("Room history")},
	{LPGEN("Group Chats"), "chat_settings", IDI_TOPICBUT, LPGEN("Room settings")},
	{LPGEN("Group Chats"), "chat_filter", IDI_FILTER, LPGEN("Event filter disabled")},
	{LPGEN("Group Chats"), "chat_filter2", IDI_FILTER2, LPGEN("Event filter enabled")},
	{LPGEN("Group Chats"), "chat_nicklist", IDI_NICKLIST, LPGEN("Hide userlist")},
	{LPGEN("Group Chats"), "chat_nicklist2", IDI_NICKLIST2, LPGEN("Show userlist")},
	{LPGEN("Group Chats"), "chat_overlay", IDI_OVERLAY, LPGEN("Icon overlay")},
	{LPGEN("Group Chats"), "chat_status0", IDI_STATUS0, LPGEN("Status 1 (10x10)"),10},
	{LPGEN("Group Chats"), "chat_status1", IDI_STATUS1, LPGEN("Status 2 (10x10"),10},
	{LPGEN("Group Chats"), "chat_status2", IDI_STATUS2, LPGEN("Status 3 (10x10)"),10},
	{LPGEN("Group Chats"), "chat_status3", IDI_STATUS3, LPGEN("Status 4 (10x10)"),10},
	{LPGEN("Group Chats"), "chat_status4", IDI_STATUS4, LPGEN("Status 5 (10x10)"),10},
	{LPGEN("Group Chats"), "chat_status5", IDI_STATUS5, LPGEN("Status 6 (10x10)"),10},
	{LPGEN("Group Chats Log"), "chat_log_message_in", IDI_INCOMING, LPGEN("Message in (10x10)"),10},
	{LPGEN("Group Chats Log"), "chat_log_message_out", IDI_OUTGOING, LPGEN("Message out (10x10)"),10},
	{LPGEN("Group Chats Log"), "chat_log_action", IDI_ACTION, LPGEN("Action (10x10)"),10},
	{LPGEN("Group Chats Log"), "chat_log_addstatus", IDI_ADDSTATUS, LPGEN("Add Status (10x10)"),10},
	{LPGEN("Group Chats Log"), "chat_log_removestatus", IDI_REMSTATUS, LPGEN("Remove status (10x10)"),10},
	{LPGEN("Group Chats Log"), "chat_log_join", IDI_JOIN, LPGEN("Join (10x10)"),10},
	{LPGEN("Group Chats Log"), "chat_log_part", IDI_PART, LPGEN("Leave (10x10)"),10},
	{LPGEN("Group Chats Log"), "chat_log_quit", IDI_QUIT, LPGEN("Quit (10x10)"),10},
	{LPGEN("Group Chats Log"), "chat_log_kick", IDI_KICK, LPGEN("Kick (10x10)"),10},
	{LPGEN("Group Chats Log"), "chat_log_nick", IDI_NICK, LPGEN("Nickchange (10x10)"),10},
	{LPGEN("Group Chats Log"), "chat_log_notice", IDI_CHAT_NOTICE, LPGEN("Notice (10x10)"),10},
	{LPGEN("Group Chats Log"), "chat_log_topic", IDI_TOPIC, LPGEN("Topic (10x10)"),10},
	{LPGEN("Group Chats Log"), "chat_log_highlight", IDI_NOTICE, LPGEN("Highlight (10x10)"),10},
	{LPGEN("Group Chats Log"), "chat_log_info", IDI_INFO, LPGEN("Information (10x10)"),10}
};

HICON hIconList[SIZEOF(iconList)];

BOOL IsStaticIcon(HICON hIcon) {
	int i;
	for (i = 0; i < SIZEOF(hIconList); i++) {
		if (hIcon == hIconList[i]) {
			return TRUE;
		}
	}
	return FALSE;
}

void ReleaseIconSmart(HICON hIcon) {
	if (!IsStaticIcon(hIcon))
		Skin_ReleaseIcon(hIcon);
}

int ImageList_AddIcon_Ex(HIMAGELIST hIml, int id) {
	HICON hIcon = LoadSkinnedIcon(id);
 	int res = ImageList_AddIcon(hIml, hIcon);
 	Skin_ReleaseIcon(hIcon);
 	return res;
}

int ImageList_AddIcon_Ex2(HIMAGELIST hIml, HICON hIcon) {
 	int res = ImageList_AddIcon(hIml, hIcon);
 	Skin_ReleaseIcon(hIcon);
 	return res;
}

int ImageList_ReplaceIcon_Ex(HIMAGELIST hIml, int nIndex, int id) {
	HICON hIcon = LoadSkinnedIcon(id);
	int res = ImageList_ReplaceIcon(hIml, nIndex, hIcon);
	Skin_ReleaseIcon(hIcon);
	return res;
}

int ImageList_AddIcon_ProtoEx(HIMAGELIST hIml, const char* szProto, int status) {
	HICON hIcon = LoadSkinnedProtoIcon(szProto, status);
 	int res = ImageList_AddIcon(hIml, hIcon);
	Skin_ReleaseIcon(hIcon);
 	return res;
}

void RegisterIcons(void)
{
	hEventSkin2IconsChanged = HookEvent_Ex(ME_SKIN2_ICONSCHANGED, IconsChanged);

	TCHAR path[MAX_PATH];
	char  szSection[200];
	GetModuleFileName(g_hInst, path, MAX_PATH);

	SKINICONDESC sid = { 0 };
	sid.cbSize = sizeof(SKINICONDESC);
	sid.flags = SIDF_PATH_TCHAR;
	sid.ptszDefaultFile = path;
	sid.pszSection = szSection;
	for (int i = 0; i < SIZEOF(iconList); i++) {
		mir_snprintf(szSection, SIZEOF(szSection), "%s/%s", LPGEN("Messaging"), iconList[i].section);
		sid.pszName = (char*)iconList[i].name;
		sid.cx = sid.cy = iconList[i].size;
		sid.iDefaultIndex = -iconList[i].defaultIndex;
		sid.pszDescription = iconList[i].description;
		Skin_AddIcon(&sid);
	}
}

void ReleaseIcons()
{
	for (int i = 0; i < SIZEOF(hIconList); i++)
		if (hIconList[i] != NULL)
			Skin_ReleaseIcon(hIconList[i]);

	Skin_ReleaseIcon(g_dat->hMsgIcon);
	Skin_ReleaseIcon(g_dat->hMsgIconBig);
	Skin_ReleaseIcon(g_dat->hIconChatBig);
}

HICON GetCachedIcon(const char *name)
{
	int i;
	for (i = 0; i < SIZEOF(iconList); i++) {
		if (!strcmp(iconList[i].name, name)) {
			return hIconList[i];
		}
	}
	return NULL;
}

void LoadGlobalIcons() {
	int i;
	int overlayIcon;
	for (i = 0; i < SIZEOF(iconList); i++)
		hIconList[i] = Skin_GetIcon(iconList[i].name);

	g_dat->hMsgIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
	g_dat->hMsgIconBig = LoadSkinnedIconBig(SKINICON_EVENT_MESSAGE);
	g_dat->hIconChatBig = Skin_GetIcon("chat_window");

	ImageList_RemoveAll(g_dat->hButtonIconList);
	ImageList_RemoveAll(g_dat->hChatButtonIconList);
	ImageList_RemoveAll(g_dat->hHelperIconList);
	ImageList_RemoveAll(g_dat->hSearchEngineIconList);
	for (i=0; i<SIZEOF(buttonIcons); i++) {
		if (buttonIcons[i] == NULL) {
			ImageList_AddIcon_ProtoEx(g_dat->hButtonIconList, NULL, ID_STATUS_OFFLINE);
		} else {
			ImageList_AddIcon(g_dat->hButtonIconList, GetCachedIcon(buttonIcons[i]));
		}
	}
	for (i=0; i<SIZEOF(chatButtonIcons); i++) {
		ImageList_AddIcon(g_dat->hChatButtonIconList, GetCachedIcon(chatButtonIcons[i]));
	}
	ImageList_AddIcon(g_dat->hHelperIconList, GetCachedIcon("scriver_OVERLAY"));
	overlayIcon = ImageList_AddIcon(g_dat->hHelperIconList, GetCachedIcon("scriver_OVERLAY"));
	ImageList_SetOverlayImage(g_dat->hHelperIconList, overlayIcon, 1);
	for (i=0; i<IDI_FOODNETWORK - IDI_GOOGLE + 1; i++) {
		HICON hIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_GOOGLE + i), IMAGE_ICON, 0, 0, 0);
		ImageList_AddIcon(g_dat->hSearchEngineIconList, hIcon);
		DestroyIcon(hIcon);
	}
}

static BOOL CALLBACK LangAddCallback(CHAR * str) {
	int i, count;
	UINT cp;
	static struct { UINT cpId; const TCHAR *cpName; } cpTable[] = {
		{	874,	_T("Thai") },
		{	932,	_T("Japanese") },
		{	936,	_T("Simplified Chinese") },
		{	949,	_T("Korean") },
		{	950,	_T("Traditional Chinese") },
		{	1250,	_T("Central European") },
		{	1251,	_T("Cyrillic") },
		{	1252,	_T("Latin I") },
		{	1253,	_T("Greek") },
		{	1254,	_T("Turkish") },
		{	1255,	_T("Hebrew") },
		{	1256,	_T("Arabic") },
		{	1257,	_T("Baltic") },
		{	1258,	_T("Vietnamese") },
		{	1361,	_T("Korean (Johab)") }
	};
    cp = atoi(str);
	count = sizeof(cpTable)/sizeof(cpTable[0]);
	for (i=0; i<count && cpTable[i].cpId!=cp; i++);
	if (i < count) {
        AppendMenu(g_dat->hMenuANSIEncoding, MF_STRING, cp, TranslateTS(cpTable[i].cpName));
	}
	return TRUE;
}

void LoadInfobarFonts()
{
	LOGFONT lf;
	LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, &lf, NULL, FALSE);
	g_dat->minInputAreaHeight = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_AUTORESIZELINES, SRMSGDEFSET_AUTORESIZELINES) * abs(lf.lfHeight) * g_dat->logPixelSY / 72;
	if (g_dat->hInfobarBrush != NULL) {
		DeleteObject(g_dat->hInfobarBrush);
	}
	g_dat->hInfobarBrush = CreateSolidBrush(DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_INFOBARBKGCOLOUR, SRMSGDEFSET_INFOBARBKGCOLOUR));
}

void InitGlobals() {
	HDC hdc = GetDC(NULL);
	g_dat = (struct GlobalMessageData *)mir_alloc(sizeof(struct GlobalMessageData));
	ZeroMemory(g_dat, sizeof(struct GlobalMessageData));
	g_dat->hMessageWindowList = (HANDLE) CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
	g_dat->hParentWindowList = (HANDLE) CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);

	HookEvent_Ex(ME_PROTO_ACK, ackevent);
	ReloadGlobals();
	g_dat->lastParent = NULL;
	g_dat->lastChatParent = NULL;
	g_dat->hTabIconList = NULL;
	g_dat->tabIconListUsage = NULL;
	g_dat->tabIconListUsageSize = 0;
	g_dat->hButtonIconList = ImageList_Create(16, 16, IsWinVerXPPlus() ? ILC_COLOR32 | ILC_MASK : ILC_COLOR8 | ILC_MASK, 0, 0);
	g_dat->hChatButtonIconList = ImageList_Create(16, 16, IsWinVerXPPlus() ? ILC_COLOR32 | ILC_MASK : ILC_COLOR8 | ILC_MASK, 0, 0);
	g_dat->hTabIconList = ImageList_Create(16, 16, IsWinVerXPPlus() ? ILC_COLOR32 | ILC_MASK : ILC_COLOR8 | ILC_MASK, 0, 0);
	g_dat->hHelperIconList = ImageList_Create(16, 16, IsWinVerXPPlus() ? ILC_COLOR32 | ILC_MASK : ILC_COLOR8 | ILC_MASK, 0, 0);
	g_dat->hSearchEngineIconList = ImageList_Create(16, 16, IsWinVerXPPlus() ? ILC_COLOR32 | ILC_MASK : ILC_COLOR8 | ILC_MASK, 0, 0);
	g_dat->draftList = NULL;
	g_dat->logPixelSX = GetDeviceCaps(hdc, LOGPIXELSX);
	g_dat->logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
	LoadInfobarFonts();
	ReleaseDC(NULL, hdc);
}

void FreeGlobals() {
	if (g_dat) {
		if (g_dat->hInfobarBrush != NULL) {
			DeleteObject(g_dat->hInfobarBrush);
		}
		if (g_dat->draftList != NULL) tcmdlist_free(g_dat->draftList);
		if (g_dat->hTabIconList)
			ImageList_Destroy(g_dat->hTabIconList);
		if (g_dat->hButtonIconList)
			ImageList_Destroy(g_dat->hButtonIconList);
		if (g_dat->hChatButtonIconList)
			ImageList_Destroy(g_dat->hChatButtonIconList);
		if (g_dat->hHelperIconList)
			ImageList_Destroy(g_dat->hHelperIconList);
		if (g_dat->hSearchEngineIconList)
			ImageList_Destroy(g_dat->hSearchEngineIconList);
		if (g_dat->hMenuANSIEncoding)
			DestroyMenu(g_dat->hMenuANSIEncoding);
		mir_free(g_dat->tabIconListUsage);
		mir_free(g_dat);
		g_dat = NULL;
	}
}

void ReloadGlobals() {
	g_dat->smileyAddInstalled =  ServiceExists(MS_SMILEYADD_SHOWSELECTION);
	g_dat->popupInstalled =  ServiceExists(MS_POPUP_ADDPOPUPEX);
	g_dat->ieviewInstalled =  ServiceExists(MS_IEVIEW_WINDOW);
	g_dat->flags = 0;
	g_dat->flags2 = 0;
//	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SENDBUTTON, SRMSGDEFSET_SENDBUTTON))
//		g_dat->flags |= SMF_SENDBTN;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_AVATARENABLE, SRMSGDEFSET_AVATARENABLE)) {
		g_dat->flags |= SMF_AVATAR;
	}
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWPROGRESS, SRMSGDEFSET_SHOWPROGRESS))
		g_dat->flags |= SMF_SHOWPROGRESS;

	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWLOGICONS, SRMSGDEFSET_SHOWLOGICONS))
		g_dat->flags |= SMF_SHOWICONS;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWTIME, SRMSGDEFSET_SHOWTIME))
		g_dat->flags |= SMF_SHOWTIME;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWSECONDS, SRMSGDEFSET_SHOWSECONDS))
		g_dat->flags |= SMF_SHOWSECONDS;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWDATE, SRMSGDEFSET_SHOWDATE))
		g_dat->flags |= SMF_SHOWDATE;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_USELONGDATE, SRMSGDEFSET_USELONGDATE))
		g_dat->flags |= SMF_LONGDATE;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_USERELATIVEDATE, SRMSGDEFSET_USERELATIVEDATE))
		g_dat->flags |= SMF_RELATIVEDATE;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_GROUPMESSAGES, SRMSGDEFSET_GROUPMESSAGES))
		g_dat->flags |= SMF_GROUPMESSAGES;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_MARKFOLLOWUPS, SRMSGDEFSET_MARKFOLLOWUPS))
		g_dat->flags |= SMF_MARKFOLLOWUPS;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_MESSAGEONNEWLINE, SRMSGDEFSET_MESSAGEONNEWLINE))
		g_dat->flags |= SMF_MSGONNEWLINE;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_DRAWLINES, SRMSGDEFSET_DRAWLINES))
		g_dat->flags |= SMF_DRAWLINES;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_HIDENAMES, SRMSGDEFSET_HIDENAMES))
		g_dat->flags |= SMF_HIDENAMES;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_AUTOPOPUP, SRMSGDEFSET_AUTOPOPUP))
		g_dat->flags |= SMF_AUTOPOPUP;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_STAYMINIMIZED, SRMSGDEFSET_STAYMINIMIZED))
		g_dat->flags |= SMF_STAYMINIMIZED;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SAVEDRAFTS, SRMSGDEFSET_SAVEDRAFTS))
		g_dat->flags |= SMF_SAVEDRAFTS;

	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_DELTEMP, SRMSGDEFSET_DELTEMP))
		g_dat->flags |= SMF_DELTEMP;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SENDONENTER, SRMSGDEFSET_SENDONENTER))
		g_dat->flags |= SMF_SENDONENTER;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SENDONDBLENTER, SRMSGDEFSET_SENDONDBLENTER))
		g_dat->flags |= SMF_SENDONDBLENTER;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_STATUSICON, SRMSGDEFSET_STATUSICON))
		g_dat->flags |= SMF_STATUSICON;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_INDENTTEXT, SRMSGDEFSET_INDENTTEXT))
		g_dat->flags |= SMF_INDENTTEXT;

	g_dat->openFlags = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_POPFLAGS, SRMSGDEFSET_POPFLAGS);
	g_dat->indentSize = DBGetContactSettingWord(NULL, SRMMMOD, SRMSGSET_INDENTSIZE, SRMSGDEFSET_INDENTSIZE);
    g_dat->logLineColour = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_LINECOLOUR, SRMSGDEFSET_LINECOLOUR);

	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_USETABS, SRMSGDEFSET_USETABS))
		g_dat->flags2 |= SMF2_USETABS;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_TABSATBOTTOM, SRMSGDEFSET_TABSATBOTTOM))
		g_dat->flags2 |= SMF2_TABSATBOTTOM;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SWITCHTOACTIVE, SRMSGDEFSET_SWITCHTOACTIVE))
		g_dat->flags2 |= SMF2_SWITCHTOACTIVE;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_LIMITNAMES, SRMSGDEFSET_LIMITNAMES))
		g_dat->flags2 |= SMF2_LIMITNAMES;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_HIDEONETAB, SRMSGDEFSET_HIDEONETAB))
		g_dat->flags2 |= SMF2_HIDEONETAB;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SEPARATECHATSCONTAINERS, SRMSGDEFSET_SEPARATECHATSCONTAINERS))
		g_dat->flags2 |= SMF2_SEPARATECHATSCONTAINERS;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_TABCLOSEBUTTON, SRMSGDEFSET_TABCLOSEBUTTON))
		g_dat->flags2 |= SMF2_TABCLOSEBUTTON;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_LIMITTABS, SRMSGDEFSET_LIMITTABS))
		g_dat->flags2 |= SMF2_LIMITTABS;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_LIMITCHATSTABS, SRMSGDEFSET_LIMITCHATSTABS))
		g_dat->flags2 |= SMF2_LIMITCHATSTABS;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_HIDECONTAINERS, SRMSGDEFSET_HIDECONTAINERS))
		g_dat->flags2 |= SMF2_HIDECONTAINERS;

	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWSTATUSBAR, SRMSGDEFSET_SHOWSTATUSBAR))
		g_dat->flags2 |= SMF2_SHOWSTATUSBAR;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWTITLEBAR, SRMSGDEFSET_SHOWTITLEBAR))
		g_dat->flags2 |= SMF2_SHOWTITLEBAR;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWBUTTONLINE, SRMSGDEFSET_SHOWBUTTONLINE))
		g_dat->flags2 |= SMF2_SHOWTOOLBAR;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWINFOBAR, SRMSGDEFSET_SHOWINFOBAR))
		g_dat->flags2 |= SMF2_SHOWINFOBAR;

	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWTYPING, SRMSGDEFSET_SHOWTYPING))
		g_dat->flags2 |= SMF2_SHOWTYPING;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGWIN, SRMSGDEFSET_SHOWTYPINGWIN))
		g_dat->flags2 |= SMF2_SHOWTYPINGWIN;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGNOWIN, SRMSGDEFSET_SHOWTYPINGNOWIN))
		g_dat->flags2 |= SMF2_SHOWTYPINGTRAY;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGCLIST, SRMSGDEFSET_SHOWTYPINGCLIST))
		g_dat->flags2 |= SMF2_SHOWTYPINGCLIST;
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGSWITCH, SRMSGDEFSET_SHOWTYPINGSWITCH))
		g_dat->flags2 |= SMF2_SHOWTYPINGSWITCH;

	if (LOBYTE(LOWORD(GetVersion())) >= 5  && pSetLayeredWindowAttributes != NULL) {
		if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_USETRANSPARENCY, SRMSGDEFSET_USETRANSPARENCY))
			g_dat->flags2 |= SMF2_USETRANSPARENCY;
		g_dat->activeAlpha = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_ACTIVEALPHA, SRMSGDEFSET_ACTIVEALPHA);
		g_dat->inactiveAlpha = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_INACTIVEALPHA, SRMSGDEFSET_INACTIVEALPHA);
	}
	if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_USEIEVIEW, SRMSGDEFSET_USEIEVIEW))
		g_dat->flags |= SMF_USEIEVIEW;

	g_dat->buttonVisibility = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_BUTTONVISIBILITY, SRMSGDEFSET_BUTTONVISIBILITY);
	g_dat->chatBbuttonVisibility = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_CHATBUTTONVISIBILITY, SRMSGDEFSET_CHATBUTTONVISIBILITY);

	g_dat->limitNamesLength = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_LIMITNAMESLEN, SRMSGDEFSET_LIMITNAMESLEN);
	g_dat->limitTabsNum = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_LIMITTABSNUM, SRMSGDEFSET_LIMITTABSNUM);
	g_dat->limitChatsTabsNum = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_LIMITCHATSTABSNUM, SRMSGDEFSET_LIMITCHATSTABSNUM);

}

static int ackevent(WPARAM wParam, LPARAM lParam) {
	ACKDATA *pAck = (ACKDATA *)lParam;

	if (!pAck) return 0;
	else if (pAck->type==ACKTYPE_MESSAGE) {
		ACKDATA *ack = (ACKDATA *) lParam;
		DBEVENTINFO dbei = { 0 };
		HANDLE hNewEvent;
		MessageSendQueueItem * item;
		HWND hwndSender;

		item = FindSendQueueItem((HANDLE)pAck->hContact, (HANDLE)pAck->hProcess);
		if (item != NULL) {
			hwndSender = item->hwndSender;
			if (ack->result == ACKRESULT_FAILED) {
				if (item->hwndErrorDlg != NULL) {
					item = FindOldestPendingSendQueueItem(hwndSender, (HANDLE)pAck->hContact);
				}
				if (item != NULL && item->hwndErrorDlg == NULL) {
					if (hwndSender != NULL) {
						ErrorWindowData *ewd = (ErrorWindowData *) mir_alloc(sizeof(ErrorWindowData));
						ewd->szName = GetNickname(item->hContact, item->proto);
						ewd->szDescription = a2t((char *) ack->lParam);
						ewd->szText = GetSendBufferMsg(item);
						ewd->hwndParent = hwndSender;
						ewd->queueItem = item;
						SendMessage(hwndSender, DM_STOPMESSAGESENDING, 0, 0);
						SendMessage(hwndSender, DM_SHOWERRORMESSAGE, 0, (LPARAM)ewd);
					} else {
						RemoveSendQueueItem(item);
					}
				}
				return 0;
			}

			dbei.cbSize = sizeof(dbei);
			dbei.eventType = EVENTTYPE_MESSAGE;
			dbei.flags = DBEF_SENT | (( item->flags & PREF_RTL) ? DBEF_RTL : 0 );
			if ( item->flags & PREF_UTF )
				dbei.flags |= DBEF_UTF;
			dbei.szModule = GetContactProto(item->hContact);
			dbei.timestamp = time(NULL);
			dbei.cbBlob = lstrlenA(item->sendBuffer) + 1;
			if ( !( item->flags & PREF_UTF ))
				dbei.cbBlob *= sizeof(TCHAR) + 1;
			dbei.pBlob = (PBYTE) item->sendBuffer;
			hNewEvent = (HANDLE) CallService(MS_DB_EVENT_ADD, (WPARAM) item->hContact, (LPARAM) & dbei);

			if (item->hwndErrorDlg != NULL) {
				DestroyWindow(item->hwndErrorDlg);
			}

			if (RemoveSendQueueItem(item) && DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_AUTOCLOSE, SRMSGDEFSET_AUTOCLOSE)) {
				if (hwndSender != NULL) {
					DestroyWindow(hwndSender);
				}
			} else if (hwndSender != NULL) {
				SendMessage(hwndSender, DM_STOPMESSAGESENDING, 0, 0);
				SkinPlaySound("SendMsg");
			}
		}
	}
	return 0;
}
