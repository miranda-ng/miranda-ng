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

GlobalMessageData g_dat;

static int buttonIcons[] =
{
	IDI_CLOSEX, IDI_QUOTE, IDI_ADDCONTACT, 0,
	IDI_USERDETAILS, IDI_HISTORY, IDI_SEND
};

static int chatButtonIcons[] =
{
	IDI_CLOSEX, IDI_BBOLD, IDI_BITALICS, IDI_BUNDERLINE,
	IDI_COLOR, IDI_BKGCOLOR, IDI_HISTORY, IDI_FILTER,
	IDI_TOPICBUT, IDI_NICKLIST, IDI_SEND
};

static IconItem iconList1[] =
{
	{ LPGEN("Add contact"),             "ADD",         IDI_ADDCONTACT  }, //  1
	{ LPGEN("User's details"),          "USERDETAILS", IDI_USERDETAILS }, //  2
	{ LPGEN("User's history"),          "HISTORY",     IDI_HISTORY     }, //  3
	{ LPGEN("Send message"),            "SEND",        IDI_SEND        }, //  4
	{ LPGEN("Smiley button"),           "SMILEY",      IDI_SMILEY      }, //  5
	{ LPGEN("User is typing"),          "TYPING",      IDI_TYPING      }, //  6
	{ LPGEN("Typing notification off"), "TYPINGOFF",   IDI_TYPINGOFF   }, //  7
	{ LPGEN("Sending"),                 "DELIVERING",  IDI_TIMESTAMP   }, //  8
	{ LPGEN("Quote button"),            "QUOTE",       IDI_QUOTE       }, //  9
	{ LPGEN("Close button"),            "CLOSEX",      IDI_CLOSEX      }, // 10
	{ LPGEN("Icon overlay"),            "OVERLAY",     IDI_OVERLAY     }, // 11
	{ LPGEN("Incoming message (10x10)"),"INCOMING",    IDI_INCOMING, 10}, // 12
	{ LPGEN("Outgoing message (10x10)"),"OUTGOING",    IDI_OUTGOING, 10}, // 13  
	{ LPGEN("Notice (10x10)"),          "NOTICE",      IDI_NOTICE,   10}, // 14  
};

static IconItem iconList2[] =
{
	{ LPGEN("Window Icon"),            "window",          IDI_CHANMGR     }, //  1 
	{ LPGEN("Text color"),             "fgcol",           IDI_COLOR       }, //  2
	{ LPGEN("Background color") ,      "bkgcol",          IDI_BKGCOLOR    }, //  3
	{ LPGEN("Bold"),                   "bold",            IDI_BBOLD       }, //  4
	{ LPGEN("Italics"),                "italics",         IDI_BITALICS    }, //  5
	{ LPGEN("Underlined"),             "underline",       IDI_BUNDERLINE  }, //  6
	{ LPGEN("Smiley button"),          "smiley",          IDI_SMILEY      }, //  7
	{ LPGEN("Room history"),           "history",         IDI_HISTORY     }, //  8
	{ LPGEN("Room settings"),          "settings",        IDI_TOPICBUT    }, //  9
	{ LPGEN("Event filter disabled"),  "filter",          IDI_FILTER      }, // 10
	{ LPGEN("Event filter enabled"),   "filter2",         IDI_FILTER2     }, // 11
	{ LPGEN("Hide nick list"),         "nicklist",        IDI_NICKLIST    }, // 12
	{ LPGEN("Show nick list"),         "nicklist2",       IDI_NICKLIST2   }, // 13
	{ LPGEN("Icon overlay"),           "overlay",         IDI_OVERLAY     }, // 14
};

static IconItem iconList3[] =
{
	{ LPGEN("Message in (10x10)"),     "log_message_in",  IDI_INCOMING, 10}, //  1
	{ LPGEN("Message out (10x10)"),    "log_message_out", IDI_OUTGOING, 10}, //  2
	{ LPGEN("Action (10x10)"),         "log_action",      IDI_ACTION,   10}, //  3
	{ LPGEN("Add Status (10x10)"),     "log_addstatus",   IDI_ADDSTATUS,10}, //  4
	{ LPGEN("Remove status (10x10)"),  "log_removestatus", IDI_REMSTATUS,10}, //  5
	{ LPGEN("Join (10x10)"),           "log_join",        IDI_JOIN,     10}, //  6
	{ LPGEN("Leave (10x10)"),          "log_part",        IDI_PART,     10}, //  7
	{ LPGEN("Quit (10x10)"),           "log_quit",        IDI_QUIT,     10}, //  8
	{ LPGEN("Kick (10x10)"),           "log_kick",        IDI_KICK,     10}, //  9
	{ LPGEN("Nick change (10x10)"),     "log_nick",        IDI_NICK,     10}, // 10
	{ LPGEN("Notice (10x10)"),         "log_notice",   IDI_CHAT_NOTICE, 10}, // 11
	{ LPGEN("Topic (10x10)"),          "log_topic",       IDI_TOPIC,    10}, // 12
	{ LPGEN("Highlight (10x10)"),      "log_highlight",   IDI_NOTICE,   10}, // 13
	{ LPGEN("Information (10x10)"),    "log_info",        IDI_INFO,     10}, // 14
};

void RegisterIcons(void)
{
	HookEvent(ME_SKIN_ICONSCHANGED, IconsChanged);

	g_plugin.registerIcon(LPGEN("Single Messaging"), iconList1, "scriver");
	g_plugin.registerIcon(LPGEN("Group chats"),      iconList2, "chat");
	g_plugin.registerIcon(LPGEN("Single Messaging"), iconList3, "chat");
}

/////////////////////////////////////////////////////////////////////////////////////////

static int ackevent(WPARAM, LPARAM lParam)
{
	ACKDATA *pAck = (ACKDATA *)lParam;
	if (!pAck)
		return 0;

	if (pAck->type != ACKTYPE_MESSAGE)
		return 0;

	MCONTACT hContact = pAck->hContact;
	MessageSendQueueItem *item = FindSendQueueItem(hContact, pAck->hProcess);
	if (item == nullptr)
		item = FindSendQueueItem(hContact = db_mc_getMeta(pAck->hContact), pAck->hProcess);
	if (item == nullptr)
		return 0;

	auto *pSender = item->pDlg;
	if (pAck->result == ACKRESULT_FAILED) {
		if (item->hwndErrorDlg != nullptr)
			item = FindOldestPendingSendQueueItem(pSender, hContact);

		if (item != nullptr && item->hwndErrorDlg == nullptr) {
			if (pSender != nullptr) {
				pSender->StopMessageSending();
				pSender->ShowError((wchar_t *)pAck->lParam, item);
			}
			else RemoveSendQueueItem(item);
		}
		return 0;
	}

	hContact = (db_mc_isMeta(hContact)) ? db_mc_getSrmmSub(item->hContact) : item->hContact;

	DBEVENTINFO dbei = {};
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.flags = DBEF_UTF | DBEF_SENT | ((item->flags & PREF_RTL) ? DBEF_RTL : 0);
	dbei.szModule = Proto_GetBaseAccountName(hContact);
	dbei.timestamp = time(0);
	dbei.cbBlob = (int)mir_strlen(item->sendBuffer) + 1;
	dbei.pBlob = (uint8_t*)item->sendBuffer;
	dbei.szId = (char *)pAck->lParam;

	MessageWindowEvent evt = { item->hSendId, hContact, &dbei };
	NotifyEventHooks(g_chatApi.hevPreCreate, 0, (LPARAM)&evt);

	item->sendBuffer = (char *)dbei.pBlob;
	db_event_add(hContact, &dbei);

	if (item->hwndErrorDlg != nullptr)
		DestroyWindow(item->hwndErrorDlg);

	if (RemoveSendQueueItem(item) && g_plugin.bAutoClose) {
		if (pSender != nullptr)
			pSender->Close();
	}
	else if (pSender != nullptr) {
		pSender->StopMessageSending();
		Skin_PlaySound("SendMsg");
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int ImageList_AddIcon_Ex(HIMAGELIST hIml, int id)
{
	HICON hIcon = Skin_LoadIcon(id);
	int res = ImageList_AddIcon(hIml, hIcon);
	IcoLib_ReleaseIcon(hIcon);
	return res;
}

int ImageList_AddIcon_Ex2(HIMAGELIST hIml, HICON hIcon)
{
	int res = ImageList_AddIcon(hIml, hIcon);
	IcoLib_ReleaseIcon(hIcon);
	return res;
}

int ImageList_ReplaceIcon_Ex(HIMAGELIST hIml, int nIndex, int id)
{
	HICON hIcon = Skin_LoadIcon(id);
	int res = ImageList_ReplaceIcon(hIml, nIndex, hIcon);
	IcoLib_ReleaseIcon(hIcon);
	return res;
}

int ImageList_AddIcon_ProtoEx(HIMAGELIST hIml, const char* szProto, int status)
{
	HICON hIcon = Skin_LoadProtoIcon(szProto, status);
	int res = ImageList_AddIcon(hIml, hIcon);
	IcoLib_ReleaseIcon(hIcon);
	return res;
}

void ReleaseIcons()
{
	IcoLib_ReleaseIcon(g_dat.hMsgIcon);
	IcoLib_ReleaseIcon(g_dat.hMsgIconBig);
	IcoLib_ReleaseIcon(g_dat.hIconChatBig);
}

void LoadGlobalIcons()
{
	g_dat.hMsgIcon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
	g_dat.hMsgIconBig = Skin_LoadIcon(SKINICON_EVENT_MESSAGE, true);
	g_dat.hIconChatBig = IcoLib_GetIcon("chat_window", true);

	ImageList_RemoveAll(g_dat.hButtonIconList);
	ImageList_RemoveAll(g_dat.hChatButtonIconList);
	ImageList_RemoveAll(g_dat.hHelperIconList);
	
	for (auto &it : buttonIcons) {
		if (it == 0)
			ImageList_AddIcon_ProtoEx(g_dat.hButtonIconList, nullptr, ID_STATUS_OFFLINE);
		else
			g_plugin.addImgListIcon(g_dat.hButtonIconList, it);
	}
	
	for (auto &it : chatButtonIcons)
		g_plugin.addImgListIcon(g_dat.hChatButtonIconList, it);

	g_plugin.addImgListIcon(g_dat.hHelperIconList, IDI_OVERLAY);

	int overlayIcon = g_plugin.addImgListIcon(g_dat.hHelperIconList, IDI_OVERLAY);
	ImageList_SetOverlayImage(g_dat.hHelperIconList, overlayIcon, 1);
}

static struct { UINT cpId; const wchar_t *cpName; } cpTable[] =
{
	{ 874, LPGENW("Thai") }, //
	{ 932, LPGENW("Japanese") }, //
	{ 936, LPGENW("Simplified Chinese") }, //
	{ 949, LPGENW("Korean") }, //
	{ 950, LPGENW("Traditional Chinese") }, //
	{ 1250, LPGENW("Central European") }, //
	{ 1251, LPGENW("Cyrillic") }, //
	{ 1252, LPGENW("Latin I") }, //
	{ 1253, LPGENW("Greek") }, //
	{ 1254, LPGENW("Turkish") }, //
	{ 1255, LPGENW("Hebrew") }, //
	{ 1256, LPGENW("Arabic") }, //
	{ 1257, LPGENW("Baltic") }, //
	{ 1258, LPGENW("Vietnamese") }, //
	{ 1361, LPGENW("Korean (Johab)") }
};

void LoadInfobarFonts()
{
	LOGFONT lf;
	LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, &lf, nullptr);
	g_dat.minInputAreaHeight = g_plugin.iAutoResizeLines * abs(lf.lfHeight) * g_dat.logPixelSY / 72;
	
	if (g_dat.hInfobarBrush != nullptr)
		DeleteObject(g_dat.hInfobarBrush);
	g_dat.hInfobarBrush = CreateSolidBrush(g_plugin.getDword(SRMSGSET_INFOBARBKGCOLOUR, SRMSGDEFSET_INFOBARBKGCOLOUR));
}

void InitGlobals()
{
	HDC hdc = GetDC(nullptr);

	memset(&g_dat, 0, sizeof(struct GlobalMessageData));
	g_dat.hParentWindowList = WindowList_Create();

	HookEvent(ME_PROTO_ACK, ackevent);
	ReloadGlobals();
	g_dat.lastParent = nullptr;
	g_dat.lastChatParent = nullptr;
	g_dat.hTabIconList = nullptr;
	g_dat.tabIconListUsage = nullptr;
	g_dat.tabIconListUsageSize = 0;
	g_dat.hButtonIconList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);
	g_dat.hChatButtonIconList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);
	g_dat.hTabIconList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);
	g_dat.hHelperIconList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);
	g_dat.logPixelSX = GetDeviceCaps(hdc, LOGPIXELSX);
	g_dat.logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
	LoadInfobarFonts();
	ReleaseDC(nullptr, hdc);
}

void FreeGlobals()
{
	if (g_dat.hInfobarBrush != nullptr)
		DeleteObject(g_dat.hInfobarBrush);
	if (g_dat.hTabIconList)
		ImageList_Destroy(g_dat.hTabIconList);
	if (g_dat.hButtonIconList)
		ImageList_Destroy(g_dat.hButtonIconList);
	if (g_dat.hChatButtonIconList)
		ImageList_Destroy(g_dat.hChatButtonIconList);
	if (g_dat.hHelperIconList)
		ImageList_Destroy(g_dat.hHelperIconList);
	mir_free(g_dat.tabIconListUsage);

	WindowList_Destroy(g_dat.hParentWindowList);

	memset(&g_dat, 0, sizeof(g_dat));
}

void ReloadGlobals()
{
	g_dat.dwFlags = 0;
	g_dat.dwFlags2 = 0;

	g_dat.flags.bShowAvatar = g_plugin.bShowAvatar;
	g_dat.flags.bShowProgress = g_plugin.bShowProgress;
	g_dat.flags.bShowIcons = g_plugin.bShowIcons;
	g_dat.flags.bShowTime = g_plugin.bShowTime;
	g_dat.flags.bShowSeconds = g_plugin.bShowSeconds;
	g_dat.flags.bShowDate = g_plugin.bShowDate;
	g_dat.flags.bLongDate = g_plugin.bLongDate;
	g_dat.flags.bRelativeDate = g_plugin.bRelativeDate;
	g_dat.flags.bGroupMessages = g_plugin.bGroupMessages;
	g_dat.flags.bMarkFollowups = g_plugin.bMarkFollowups;
	g_dat.flags.bMsgOnNewline = g_plugin.bMsgOnNewline;
	g_dat.flags.bDrawLines = g_plugin.bDrawLines;
	g_dat.flags.bHideNames = g_plugin.bHideNames;
	g_dat.flags.bIndentText = g_plugin.bIndentText;
	
	g_dat.flags.bAutoPopup = g_plugin.bAutoPopup;
	g_dat.flags.bStayMinimized = g_plugin.bStayMinimized;
	g_dat.flags.bSaveDrafts = g_plugin.bSaveDrafts;
	g_dat.flags.bDelTemp = g_plugin.bDelTemp;

	g_dat.openFlags = g_plugin.iPopFlags;
	g_dat.indentSize = g_plugin.iIndentSize;
	g_dat.logLineColour = g_plugin.getDword(SRMSGSET_LINECOLOUR, SRMSGDEFSET_LINECOLOUR);

	g_dat.flags2.bUseTabs = g_plugin.bUseTabs;
	g_dat.flags2.bTabsAtBottom = g_plugin.bTabsAtBottom;
	g_dat.flags2.bSwitchToActive = g_plugin.bSwitchToActive;
	g_dat.flags2.bLimitNames = g_plugin.bLimitNames;
	g_dat.flags2.bHideOneTab = g_plugin.bHideOneTab;
	g_dat.flags2.bSeparateChats = g_plugin.bSeparateChats;
	g_dat.flags2.bTabCloseButton = g_plugin.bTabCloseButton;
	g_dat.flags2.bLimitTabs = g_plugin.bLimitTabs;
	g_dat.flags2.bLimitChatTabs = g_plugin.bLimitChatTabs;
	g_dat.flags2.bHideContainer = g_plugin.bHideContainer;
	g_dat.flags2.bUseTransparency = g_plugin.bUseTransparency;

	g_dat.flags2.bShowStatusBar = g_plugin.bShowStatusBar;
	g_dat.flags2.bShowTitleBar = g_plugin.bShowTitleBar;
	g_dat.flags2.bShowToolBar = g_plugin.bShowToolBar;
	g_dat.flags2.bShowInfoBar = g_plugin.bShowInfoBar;

	g_dat.flags2.bShowTyping = g_plugin.bShowTyping;
	g_dat.flags2.bShowTypingWin = g_plugin.bShowTypingWin;
	g_dat.flags2.bShowTypingTray = g_plugin.bShowTypingTray;
	g_dat.flags2.bShowTypingClist = g_plugin.bShowTypingClist;
	g_dat.flags2.bShowTypingSwitch = g_plugin.bShowTypingSwitch;

	g_dat.activeAlpha = g_plugin.iActiveAlpha;
	g_dat.inactiveAlpha = g_plugin.iInactiveAlpha;
	
	g_dat.limitNamesLength = g_plugin.iLimitNames;
	g_dat.limitTabsNum = g_plugin.iLimitTabs;
	g_dat.limitChatsTabsNum = g_plugin.iLimitChatTabs;

	ptrW wszTitleFormat(g_plugin.getWStringA(SRMSGSET_WINDOWTITLE));
	if (wszTitleFormat == nullptr)
		g_dat.wszTitleFormat[0] = 0;
	else
		wcsncpy_s(g_dat.wszTitleFormat, wszTitleFormat, _TRUNCATE);
}
