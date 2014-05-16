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

#include "commonheaders.h"

GlobalMessageData g_dat;

static int ackevent(WPARAM wParam, LPARAM lParam);

extern int    Chat_ModulesLoaded(WPARAM wParam,LPARAM lParam);

static const char *buttonIcons[] = {"scriver_CLOSEX", "scriver_QUOTE", "scriver_SMILEY", 
									"scriver_ADD", NULL, "scriver_USERDETAILS", "scriver_HISTORY", 
									"scriver_SEND"};

static const char *chatButtonIcons[] = {"scriver_CLOSEX", 
									"chat_bold", "chat_italics", "chat_underline", 
									"chat_fgcol", "chat_bkgcol", 
									"chat_smiley", "chat_history", 
									"chat_filter", "chat_settings", "chat_nicklist", "scriver_SEND"};

static IconItem iconList[] =
{
	{ LPGEN("Add contact"),            "scriver_ADD",          IDI_ADDCONTACT  }, //  1
	{ LPGEN("User's details"),         "scriver_USERDETAILS",  IDI_USERDETAILS }, //  2
	{ LPGEN("User's history"),         "scriver_HISTORY",      IDI_HISTORY     }, //  3
	{ LPGEN("Send message"),           "scriver_SEND",         IDI_SEND        }, //  4
	{ LPGEN("Smiley button"),          "scriver_SMILEY",       IDI_SMILEY      }, //  5
	{ LPGEN("User is typing"),         "scriver_TYPING",       IDI_TYPING      }, //  6
	{ LPGEN("Typing notification off"), "scriver_TYPINGOFF",    IDI_TYPINGOFF   }, //  7
	{ LPGEN("Unicode is on"),          "scriver_UNICODEON",    IDI_UNICODEON   }, //  8
	{ LPGEN("Unicode is off"),         "scriver_UNICODEOFF",   IDI_UNICODEOFF  }, //  9
	{ LPGEN("Sending"),                "scriver_DELIVERING",   IDI_TIMESTAMP   }, // 10
	{ LPGEN("Quote button"),           "scriver_QUOTE",        IDI_QUOTE       }, // 11
	{ LPGEN("Close button"),           "scriver_CLOSEX",       IDI_CLOSEX      }, // 12
	{ LPGEN("Icon overlay"),           "scriver_OVERLAY",      IDI_OVERLAY     }, // 13
	{ LPGEN("Incoming message (10x10)"),"scriver_INCOMING",    IDI_INCOMING, 10}, // 14
	{ LPGEN("Outgoing message (10x10)"),"scriver_OUTGOING",    IDI_OUTGOING, 10}, // 15
	{ LPGEN("Notice (10x10)"),         "scriver_NOTICE",       IDI_NOTICE,   10}, // 16

	{ LPGEN("Window Icon"),            "chat_window",          IDI_CHANMGR     }, //  1 
	{ LPGEN("Text color"),             "chat_fgcol",           IDI_COLOR       }, //  2
	{ LPGEN("Background color") ,      "chat_bkgcol",          IDI_BKGCOLOR    }, //  3
	{ LPGEN("Bold"),                   "chat_bold",            IDI_BBOLD       }, //  4
	{ LPGEN("Italics"),                "chat_italics",         IDI_BITALICS    }, //  5
	{ LPGEN("Underlined"),             "chat_underline",       IDI_BUNDERLINE  }, //  6
	{ LPGEN("Smiley button"),          "chat_smiley",          IDI_SMILEY      }, //  7
	{ LPGEN("Room history"),           "chat_history",         IDI_HISTORY     }, //  8
	{ LPGEN("Room settings"),          "chat_settings",        IDI_TOPICBUT    }, //  9
	{ LPGEN("Event filter disabled"),  "chat_filter",          IDI_FILTER      }, // 10
	{ LPGEN("Event filter enabled"),   "chat_filter2",         IDI_FILTER2     }, // 11
	{ LPGEN("Hide nick list"),          "chat_nicklist",        IDI_NICKLIST    }, // 12
	{ LPGEN("Show nick list"),          "chat_nicklist2",       IDI_NICKLIST2   }, // 13
	{ LPGEN("Icon overlay"),           "chat_overlay",         IDI_OVERLAY     }, // 14
	{ LPGEN("Status 1 (10x10)"),       "chat_status0",         IDI_STATUS0,  10}, // 15
	{ LPGEN("Status 2 (10x10)"),       "chat_status1",         IDI_STATUS1,  10}, // 16
	{ LPGEN("Status 3 (10x10)"),       "chat_status2",         IDI_STATUS2,  10}, // 17
	{ LPGEN("Status 4 (10x10)"),       "chat_status3",         IDI_STATUS3,  10}, // 18
	{ LPGEN("Status 5 (10x10)"),       "chat_status4",         IDI_STATUS4,  10}, // 19
	{ LPGEN("Status 6 (10x10)"),       "chat_status5",         IDI_STATUS5,  10}, // 20
	
	{ LPGEN("Message in (10x10)"),     "chat_log_message_in",  IDI_INCOMING, 10}, //  1
	{ LPGEN("Message out (10x10)"),    "chat_log_message_out", IDI_OUTGOING, 10}, //  2
	{ LPGEN("Action (10x10)"),         "chat_log_action",      IDI_ACTION,   10}, //  3
	{ LPGEN("Add Status (10x10)"),     "chat_log_addstatus",   IDI_ADDSTATUS,10}, //  4
	{ LPGEN("Remove status (10x10)"),  "chat_log_removestatus", IDI_REMSTATUS,10}, //  5
	{ LPGEN("Join (10x10)"),           "chat_log_join",        IDI_JOIN,     10}, //  6
	{ LPGEN("Leave (10x10)"),          "chat_log_part",        IDI_PART,     10}, //  7
	{ LPGEN("Quit (10x10)"),           "chat_log_quit",        IDI_QUIT,     10}, //  8
	{ LPGEN("Kick (10x10)"),           "chat_log_kick",        IDI_KICK,     10}, //  9
	{ LPGEN("Nickchange (10x10)"),     "chat_log_nick",        IDI_NICK,     10}, // 10
	{ LPGEN("Notice (10x10)"),         "chat_log_notice",   IDI_CHAT_NOTICE, 10}, // 11
	{ LPGEN("Topic (10x10)"),          "chat_log_topic",       IDI_TOPIC,    10}, // 12
	{ LPGEN("Highlight (10x10)"),      "chat_log_highlight",   IDI_NOTICE,   10}, // 13
	{ LPGEN("Information (10x10)"),    "chat_log_info",        IDI_INFO,     10}, // 14
};

void RegisterIcons(void)
{
	HookEvent(ME_SKIN2_ICONSCHANGED, IconsChanged);

	Icon_Register(g_hInst, LPGEN("Single Messaging"), iconList,    16);
	Icon_Register(g_hInst, LPGEN("Group chats"),      iconList+16, 20);
	Icon_Register(g_hInst, LPGEN("Single Messaging"), iconList+36, 14);
}

/////////////////////////////////////////////////////////////////////////////////////////

int ImageList_AddIcon_Ex(HIMAGELIST hIml, int id)
{
	HICON hIcon = LoadSkinnedIcon(id);
	int res = ImageList_AddIcon(hIml, hIcon);
	Skin_ReleaseIcon(hIcon);
	return res;
}

int ImageList_AddIcon_Ex2(HIMAGELIST hIml, HICON hIcon)
{
	int res = ImageList_AddIcon(hIml, hIcon);
	Skin_ReleaseIcon(hIcon);
	return res;
}

int ImageList_ReplaceIcon_Ex(HIMAGELIST hIml, int nIndex, int id)
{
	HICON hIcon = LoadSkinnedIcon(id);
	int res = ImageList_ReplaceIcon(hIml, nIndex, hIcon);
	Skin_ReleaseIcon(hIcon);
	return res;
}

int ImageList_AddIcon_ProtoEx(HIMAGELIST hIml, const char* szProto, int status)
{
	HICON hIcon = LoadSkinnedProtoIcon(szProto, status);
	int res = ImageList_AddIcon(hIml, hIcon);
	Skin_ReleaseIcon(hIcon);
	return res;
}

void ReleaseIcons()
{
	Skin_ReleaseIcon(g_dat.hMsgIcon);
	Skin_ReleaseIcon(g_dat.hMsgIconBig);
	Skin_ReleaseIcon(g_dat.hIconChatBig);
}

HICON GetCachedIcon(const char *name)
{
	for (int i = 0; i < SIZEOF(iconList); i++)
	if (!strcmp(iconList[i].szName, name))
		return Skin_GetIconByHandle(iconList[i].hIcolib);

	return NULL;
}

void LoadGlobalIcons()
{
	int i;

	g_dat.hMsgIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
	g_dat.hMsgIconBig = LoadSkinnedIconBig(SKINICON_EVENT_MESSAGE);
	g_dat.hIconChatBig = Skin_GetIcon("chat_window", true);

	ImageList_RemoveAll(g_dat.hButtonIconList);
	ImageList_RemoveAll(g_dat.hChatButtonIconList);
	ImageList_RemoveAll(g_dat.hHelperIconList);
	ImageList_RemoveAll(g_dat.hSearchEngineIconList);
	for (i = 0; i < SIZEOF(buttonIcons); i++) {
		if (buttonIcons[i] == NULL)
			ImageList_AddIcon_ProtoEx(g_dat.hButtonIconList, NULL, ID_STATUS_OFFLINE);
		else
			ImageList_AddIcon(g_dat.hButtonIconList, GetCachedIcon(buttonIcons[i]));
	}
	for (i = 0; i < SIZEOF(chatButtonIcons); i++)
		ImageList_AddIcon(g_dat.hChatButtonIconList, GetCachedIcon(chatButtonIcons[i]));

	ImageList_AddIcon(g_dat.hHelperIconList, GetCachedIcon("scriver_OVERLAY"));
	int overlayIcon = ImageList_AddIcon(g_dat.hHelperIconList, GetCachedIcon("scriver_OVERLAY"));
	ImageList_SetOverlayImage(g_dat.hHelperIconList, overlayIcon, 1);
	for (i = IDI_GOOGLE; i < IDI_LASTICON; i++) {
		HICON hIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(i), IMAGE_ICON, 0, 0, 0);
		ImageList_AddIcon(g_dat.hSearchEngineIconList, hIcon);
		DestroyIcon(hIcon);
	}
}

static struct { UINT cpId; const TCHAR *cpName; } cpTable[] =
{
	{ 874, LPGENT("Thai") }, //
	{ 932, LPGENT("Japanese") }, //
	{ 936, LPGENT("Simplified Chinese") }, //
	{ 949, LPGENT("Korean") }, //
	{ 950, LPGENT("Traditional Chinese") }, //
	{ 1250, LPGENT("Central European") }, //
	{ 1251, LPGENT("Cyrillic") }, //
	{ 1252, LPGENT("Latin I") }, //
	{ 1253, LPGENT("Greek") }, //
	{ 1254, LPGENT("Turkish") }, //
	{ 1255, LPGENT("Hebrew") }, //
	{ 1256, LPGENT("Arabic") }, //
	{ 1257, LPGENT("Baltic") }, //
	{ 1258, LPGENT("Vietnamese") }, //
	{ 1361, LPGENT("Korean (Johab)") }
};

static BOOL CALLBACK LangAddCallback(CHAR * str)
{
	int i, count;
	UINT cp;
	cp = atoi(str);
	count = sizeof(cpTable) / sizeof(cpTable[0]);
	for (i = 0; i < count && cpTable[i].cpId != cp; i++);
	if (i < count)
		AppendMenu(g_dat.hMenuANSIEncoding, MF_STRING, cp, TranslateTS(cpTable[i].cpName));

	return TRUE;
}

void LoadInfobarFonts()
{
	LOGFONT lf;
	LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, &lf, NULL);
	g_dat.minInputAreaHeight = db_get_dw(NULL, SRMMMOD, SRMSGSET_AUTORESIZELINES, SRMSGDEFSET_AUTORESIZELINES) * abs(lf.lfHeight) * g_dat.logPixelSY / 72;
	if (g_dat.hInfobarBrush != NULL)
		DeleteObject(g_dat.hInfobarBrush);

	g_dat.hInfobarBrush = CreateSolidBrush(db_get_dw(NULL, SRMMMOD, SRMSGSET_INFOBARBKGCOLOUR, SRMSGDEFSET_INFOBARBKGCOLOUR));
}

void InitGlobals()
{
	HDC hdc = GetDC(NULL);

	ZeroMemory(&g_dat, sizeof(struct GlobalMessageData));
	g_dat.hMessageWindowList = WindowList_Create();
	g_dat.hParentWindowList = WindowList_Create();

	HookEvent(ME_PROTO_ACK, ackevent);
	ReloadGlobals();
	g_dat.lastParent = NULL;
	g_dat.lastChatParent = NULL;
	g_dat.hTabIconList = NULL;
	g_dat.tabIconListUsage = NULL;
	g_dat.tabIconListUsageSize = 0;
	g_dat.hButtonIconList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);
	g_dat.hChatButtonIconList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);
	g_dat.hTabIconList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);
	g_dat.hHelperIconList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);
	g_dat.hSearchEngineIconList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);
	g_dat.draftList = NULL;
	g_dat.logPixelSX = GetDeviceCaps(hdc, LOGPIXELSX);
	g_dat.logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
	LoadInfobarFonts();
	ReleaseDC(NULL, hdc);
}

void FreeGlobals()
{
	if (g_dat.hInfobarBrush != NULL)
		DeleteObject(g_dat.hInfobarBrush);
	if (g_dat.draftList != NULL)
		tcmdlist_free(g_dat.draftList);
	if (g_dat.hTabIconList)
		ImageList_Destroy(g_dat.hTabIconList);
	if (g_dat.hButtonIconList)
		ImageList_Destroy(g_dat.hButtonIconList);
	if (g_dat.hChatButtonIconList)
		ImageList_Destroy(g_dat.hChatButtonIconList);
	if (g_dat.hHelperIconList)
		ImageList_Destroy(g_dat.hHelperIconList);
	if (g_dat.hSearchEngineIconList)
		ImageList_Destroy(g_dat.hSearchEngineIconList);
	if (g_dat.hMenuANSIEncoding)
		DestroyMenu(g_dat.hMenuANSIEncoding);
	mir_free(g_dat.tabIconListUsage);
	
	WindowList_Destroy(g_dat.hMessageWindowList);
	WindowList_Destroy(g_dat.hParentWindowList);

	ZeroMemory(&g_dat, sizeof(g_dat));
}

void ReloadGlobals()
{
	g_dat.flags = 0;
	g_dat.flags2 = 0;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_AVATARENABLE, SRMSGDEFSET_AVATARENABLE))
		g_dat.flags |= SMF_AVATAR;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWPROGRESS, SRMSGDEFSET_SHOWPROGRESS))
		g_dat.flags |= SMF_SHOWPROGRESS;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWLOGICONS, SRMSGDEFSET_SHOWLOGICONS))
		g_dat.flags |= SMF_SHOWICONS;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTIME, SRMSGDEFSET_SHOWTIME))
		g_dat.flags |= SMF_SHOWTIME;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWSECONDS, SRMSGDEFSET_SHOWSECONDS))
		g_dat.flags |= SMF_SHOWSECONDS;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWDATE, SRMSGDEFSET_SHOWDATE))
		g_dat.flags |= SMF_SHOWDATE;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_USELONGDATE, SRMSGDEFSET_USELONGDATE))
		g_dat.flags |= SMF_LONGDATE;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_USERELATIVEDATE, SRMSGDEFSET_USERELATIVEDATE))
		g_dat.flags |= SMF_RELATIVEDATE;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_GROUPMESSAGES, SRMSGDEFSET_GROUPMESSAGES))
		g_dat.flags |= SMF_GROUPMESSAGES;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_MARKFOLLOWUPS, SRMSGDEFSET_MARKFOLLOWUPS))
		g_dat.flags |= SMF_MARKFOLLOWUPS;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_MESSAGEONNEWLINE, SRMSGDEFSET_MESSAGEONNEWLINE))
		g_dat.flags |= SMF_MSGONNEWLINE;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_DRAWLINES, SRMSGDEFSET_DRAWLINES))
		g_dat.flags |= SMF_DRAWLINES;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_HIDENAMES, SRMSGDEFSET_HIDENAMES))
		g_dat.flags |= SMF_HIDENAMES;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_AUTOPOPUP, SRMSGDEFSET_AUTOPOPUP))
		g_dat.flags |= SMF_AUTOPOPUP;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_STAYMINIMIZED, SRMSGDEFSET_STAYMINIMIZED))
		g_dat.flags |= SMF_STAYMINIMIZED;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SAVEDRAFTS, SRMSGDEFSET_SAVEDRAFTS))
		g_dat.flags |= SMF_SAVEDRAFTS;

	if (db_get_b(NULL, SRMMMOD, SRMSGSET_DELTEMP, SRMSGDEFSET_DELTEMP))
		g_dat.flags |= SMF_DELTEMP;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SENDONENTER, SRMSGDEFSET_SENDONENTER))
		g_dat.flags |= SMF_SENDONENTER;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SENDONDBLENTER, SRMSGDEFSET_SENDONDBLENTER))
		g_dat.flags |= SMF_SENDONDBLENTER;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_STATUSICON, SRMSGDEFSET_STATUSICON))
		g_dat.flags |= SMF_STATUSICON;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_INDENTTEXT, SRMSGDEFSET_INDENTTEXT))
		g_dat.flags |= SMF_INDENTTEXT;

	g_dat.openFlags = db_get_dw(NULL, SRMMMOD, SRMSGSET_POPFLAGS, SRMSGDEFSET_POPFLAGS);
	g_dat.indentSize = db_get_w(NULL, SRMMMOD, SRMSGSET_INDENTSIZE, SRMSGDEFSET_INDENTSIZE);
	g_dat.logLineColour = db_get_dw(NULL, SRMMMOD, SRMSGSET_LINECOLOUR, SRMSGDEFSET_LINECOLOUR);

	if (db_get_b(NULL, SRMMMOD, SRMSGSET_USETABS, SRMSGDEFSET_USETABS))
		g_dat.flags2 |= SMF2_USETABS;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_TABSATBOTTOM, SRMSGDEFSET_TABSATBOTTOM))
		g_dat.flags2 |= SMF2_TABSATBOTTOM;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SWITCHTOACTIVE, SRMSGDEFSET_SWITCHTOACTIVE))
		g_dat.flags2 |= SMF2_SWITCHTOACTIVE;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_LIMITNAMES, SRMSGDEFSET_LIMITNAMES))
		g_dat.flags2 |= SMF2_LIMITNAMES;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_HIDEONETAB, SRMSGDEFSET_HIDEONETAB))
		g_dat.flags2 |= SMF2_HIDEONETAB;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SEPARATECHATSCONTAINERS, SRMSGDEFSET_SEPARATECHATSCONTAINERS))
		g_dat.flags2 |= SMF2_SEPARATECHATSCONTAINERS;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_TABCLOSEBUTTON, SRMSGDEFSET_TABCLOSEBUTTON))
		g_dat.flags2 |= SMF2_TABCLOSEBUTTON;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_LIMITTABS, SRMSGDEFSET_LIMITTABS))
		g_dat.flags2 |= SMF2_LIMITTABS;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_LIMITCHATSTABS, SRMSGDEFSET_LIMITCHATSTABS))
		g_dat.flags2 |= SMF2_LIMITCHATSTABS;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_HIDECONTAINERS, SRMSGDEFSET_HIDECONTAINERS))
		g_dat.flags2 |= SMF2_HIDECONTAINERS;

	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWSTATUSBAR, SRMSGDEFSET_SHOWSTATUSBAR))
		g_dat.flags2 |= SMF2_SHOWSTATUSBAR;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTITLEBAR, SRMSGDEFSET_SHOWTITLEBAR))
		g_dat.flags2 |= SMF2_SHOWTITLEBAR;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWBUTTONLINE, SRMSGDEFSET_SHOWBUTTONLINE))
		g_dat.flags2 |= SMF2_SHOWTOOLBAR;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWINFOBAR, SRMSGDEFSET_SHOWINFOBAR))
		g_dat.flags2 |= SMF2_SHOWINFOBAR;

	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTYPING, SRMSGDEFSET_SHOWTYPING))
		g_dat.flags2 |= SMF2_SHOWTYPING;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGWIN, SRMSGDEFSET_SHOWTYPINGWIN))
		g_dat.flags2 |= SMF2_SHOWTYPINGWIN;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGNOWIN, SRMSGDEFSET_SHOWTYPINGNOWIN))
		g_dat.flags2 |= SMF2_SHOWTYPINGTRAY;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGCLIST, SRMSGDEFSET_SHOWTYPINGCLIST))
		g_dat.flags2 |= SMF2_SHOWTYPINGCLIST;
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_SHOWTYPINGSWITCH, SRMSGDEFSET_SHOWTYPINGSWITCH))
		g_dat.flags2 |= SMF2_SHOWTYPINGSWITCH;

	if (LOBYTE(LOWORD(GetVersion())) >= 5) {
		if (db_get_b(NULL, SRMMMOD, SRMSGSET_USETRANSPARENCY, SRMSGDEFSET_USETRANSPARENCY))
			g_dat.flags2 |= SMF2_USETRANSPARENCY;
		g_dat.activeAlpha = db_get_dw(NULL, SRMMMOD, SRMSGSET_ACTIVEALPHA, SRMSGDEFSET_ACTIVEALPHA);
		g_dat.inactiveAlpha = db_get_dw(NULL, SRMMMOD, SRMSGSET_INACTIVEALPHA, SRMSGDEFSET_INACTIVEALPHA);
	}
	if (db_get_b(NULL, SRMMMOD, SRMSGSET_USEIEVIEW, SRMSGDEFSET_USEIEVIEW))
		g_dat.flags |= SMF_USEIEVIEW;

	g_dat.buttonVisibility = db_get_dw(NULL, SRMMMOD, SRMSGSET_BUTTONVISIBILITY, SRMSGDEFSET_BUTTONVISIBILITY);
	g_dat.chatBbuttonVisibility = db_get_dw(NULL, SRMMMOD, SRMSGSET_CHATBUTTONVISIBILITY, SRMSGDEFSET_CHATBUTTONVISIBILITY);

	g_dat.limitNamesLength = db_get_dw(NULL, SRMMMOD, SRMSGSET_LIMITNAMESLEN, SRMSGDEFSET_LIMITNAMESLEN);
	g_dat.limitTabsNum = db_get_dw(NULL, SRMMMOD, SRMSGSET_LIMITTABSNUM, SRMSGDEFSET_LIMITTABSNUM);
	g_dat.limitChatsTabsNum = db_get_dw(NULL, SRMMMOD, SRMSGSET_LIMITCHATSTABSNUM, SRMSGDEFSET_LIMITCHATSTABSNUM);
}

static int ackevent(WPARAM wParam, LPARAM lParam)
{
	ACKDATA *pAck = (ACKDATA *)lParam;
	if (!pAck)
		return 0;

	if (pAck->type != ACKTYPE_MESSAGE)
		return 0;

	MCONTACT hContact = pAck->hContact;
	MessageSendQueueItem *item = FindSendQueueItem(hContact, (HANDLE)pAck->hProcess);
	if (item == NULL)
		item = FindSendQueueItem(hContact = db_mc_getMeta(pAck->hContact), (HANDLE)pAck->hProcess);
	if (item == NULL)
		return 0;

	HWND hwndSender = item->hwndSender;
	if (pAck->result == ACKRESULT_FAILED) {
		if (item->hwndErrorDlg != NULL)
			item = FindOldestPendingSendQueueItem(hwndSender, hContact);

		if (item != NULL && item->hwndErrorDlg == NULL) {
			if (hwndSender != NULL) {
				ErrorWindowData *ewd = (ErrorWindowData *)mir_alloc(sizeof(ErrorWindowData));
				ewd->szName = GetNickname(item->hContact, item->proto);
				ewd->szDescription = mir_a2t((char *)pAck->lParam);
				ewd->szText = GetSendBufferMsg(item);
				ewd->hwndParent = hwndSender;
				ewd->queueItem = item;
				SendMessage(hwndSender, DM_STOPMESSAGESENDING, 0, 0);
				SendMessage(hwndSender, DM_SHOWERRORMESSAGE, 0, (LPARAM)ewd);
			}
			else
				RemoveSendQueueItem(item);
		}
		return 0;
	}

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.flags = DBEF_SENT | ((item->flags & PREF_RTL) ? DBEF_RTL : 0);
	if (item->flags & PREF_UTF)
		dbei.flags |= DBEF_UTF;
	dbei.szModule = GetContactProto(item->hContact);
	dbei.timestamp = time(NULL);
	dbei.cbBlob = lstrlenA(item->sendBuffer) + 1;
	if (!(item->flags & PREF_UTF))
		dbei.cbBlob *= sizeof(TCHAR) + 1;
	dbei.pBlob = (PBYTE)item->sendBuffer;

	MessageWindowEvent evt = { sizeof(evt), (int)item->hSendId, item->hContact, &dbei };
	NotifyEventHooks(hHookWinWrite, 0, (LPARAM)&evt);

	item->sendBuffer = (char *)dbei.pBlob;
	db_event_add(item->hContact, &dbei);

	if (item->hwndErrorDlg != NULL)
		DestroyWindow(item->hwndErrorDlg);

	if (RemoveSendQueueItem(item) && db_get_b(NULL, SRMMMOD, SRMSGSET_AUTOCLOSE, SRMSGDEFSET_AUTOCLOSE)) {
		if (hwndSender != NULL)
			DestroyWindow(hwndSender);
	}
	else if (hwndSender != NULL) {
		SendMessage(hwndSender, DM_STOPMESSAGESENDING, 0, 0);
		SkinPlaySound("SendMsg");
	}

	return 0;
}
