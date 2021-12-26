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
#ifndef SRMM_GLOBALS_H
#define SRMM_GLOBALS_H

typedef struct ImageListUsageEntry_tag
{
	int index;
	int used;
} ImageListUsageEntry;

enum SendMode
{
	SEND_ON_ENTER,
	SEND_ON_DBL_ENTER,
	SEND_ON_CTRL_ENTER,
	SEND_ON_SHIFT_ENTER
};

struct WindowFlags
{
	bool bAutoPopup : 1;
	bool bStayMinimized : 1;
	bool bCloseOnSend : 1;
	bool bMinimizeOnSend : 1;
	bool bSaveDrafts : 1;
	bool bDelTemp : 1;
	bool bShowProgress : 1;
	bool bShowAvatar : 1;
	bool bRtl : 1;
	bool bShowIcons : 1;
	bool bHideNames : 1;
	bool bShowTime : 1;
	bool bShowDate : 1;
	bool bLongDate : 1;
	bool bRelativeDate : 1;
	bool bShowSeconds : 1;
	bool bGroupMessages : 1;
	bool bMarkFollowups : 1;
	bool bMsgOnNewline : 1;
	bool bDrawLines : 1;
	bool bIndentText : 1;
	bool bOriginalAvatarH : 1;
};

struct TabFlags
{
	bool bUseTabs : 1;
	bool bHideOneTab : 1;
	bool bTabsAtBottom : 1;
	bool bLimitNames : 1;
	bool bSwitchToActive : 1;
	bool bSeparateChats : 1;
	bool bTabCloseButton : 1;
	bool bLimitTabs : 1;
	bool bLimitChatTabs : 1;
	bool bHideContainer : 1;
	bool bShowInfoBar : 1;
	bool bShowStatusBar : 1;
	bool bShowTitleBar : 1;
	bool bShowToolBar : 1;
	bool bUseTransparency : 1;
	bool bShowTyping : 1;
	bool bShowTypingWin : 1;
	bool bShowTypingTray : 1;
	bool bShowTypingClist : 1;
	bool bShowTypingSwitch : 1;
};

struct GlobalMessageData
{
	union {
		uint32_t dwFlags;
		WindowFlags flags;
	};

	union {
		uint32_t dwFlags2;
		TabFlags flags2;
	};

	uint32_t      openFlags;
	uint32_t      limitNamesLength;
	int        activeAlpha;
	int        inactiveAlpha;
	SendMode   sendMode;
	int        tabIconListUsageSize;
	int        smileyAddInstalled;
	int        limitTabsNum;
	int        limitChatsTabsNum;
	int        indentSize;
	HIMAGELIST hTabIconList;
	HIMAGELIST hButtonIconList;
	HIMAGELIST hChatButtonIconList;
	HIMAGELIST hHelperIconList;
	HBRUSH     hInfobarBrush;
	int        toolbarPosition;
	HWND       hFocusWnd;
	uint32_t      logLineColour;
	int        logPixelSX;
	int        logPixelSY;
	HICON      hMsgIcon;
	HICON      hMsgIconBig;
	HICON      hIconChatBig;
	int        minInputAreaHeight;
	wchar_t    wszTitleFormat[200];

	MWindowList hParentWindowList;
	struct ParentWindowData *lastParent;
	struct ParentWindowData *lastChatParent;
	ImageListUsageEntry *tabIconListUsage;
};

int IconsChanged(WPARAM wParam, LPARAM lParam);
int SmileySettingsChanged(WPARAM wParam, LPARAM lParam);
void InitGlobals();
void FreeGlobals();
void ReloadGlobals();
void RegisterIcons();
void ReleaseIcons();
void LoadGlobalIcons();
void RegisterFontServiceFonts();

int ImageList_AddIcon_Ex(HIMAGELIST hIml, int id);
int ImageList_AddIcon_Ex2(HIMAGELIST hIml, HICON hIcon);
int ImageList_ReplaceIcon_Ex(HIMAGELIST hIml, int nIndex, int id);
int ImageList_AddIcon_ProtoEx(HIMAGELIST hIml, const char *szProto, int status);

void StreamInTestEvents(HWND hEditWnd, GlobalMessageData *gdat);

#endif
