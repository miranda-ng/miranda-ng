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

#define SMF_AUTOPOPUP				0x00000001
#define SMF_STAYMINIMIZED			0x00000002
#define SMF_CLOSEONSEND				0x00000004
#define SMF_MINIMIZEONSEND			0x00000008
#define SMF_SAVEDRAFTS				0x00000040
#define SMF_DELTEMP					0x00000080
#define SMF_SENDONENTER				0x00000100
#define SMF_SENDONDBLENTER			0x00000200
#define SMF_SHOWPROGRESS			0x00000400
#define SMF_AVATAR					0x00000800
#define SMF_STATUSICON				0x00002000
#define SMF_RTL						0x00004000
#define SMF_USEIEVIEW				0x00010000
#define SMF_SHOWICONS				0x00020000
#define SMF_HIDENAMES				0x00040000
#define SMF_SHOWTIME				0x00080000
#define SMF_SHOWDATE				0x00100000
#define SMF_LONGDATE				0x00200000
#define SMF_RELATIVEDATE			0x00400000
#define SMF_SHOWSECONDS				0x00800000
#define SMF_GROUPMESSAGES			0x01000000
#define SMF_MARKFOLLOWUPS			0x02000000
#define SMF_MSGONNEWLINE			0x04000000
#define SMF_DRAWLINES				0x08000000
#define SMF_INDENTTEXT				0x10000000
#define SMF_ORIGINALAVATARH			0x20000000

#define SMF2_USETABS					0x00000001
#define SMF2_HIDEONETAB					0x00000002
#define SMF2_TABSATBOTTOM				0x00000004
#define SMF2_LIMITNAMES					0x00000008
#define SMF2_SWITCHTOACTIVE				0x00000010
#define SMF2_SEPARATECHATSCONTAINERS	0x00000020
#define SMF2_TABCLOSEBUTTON				0x00000040
#define SMF2_LIMITTABS					0x00000080
#define SMF2_LIMITCHATSTABS				0x00000100
#define SMF2_HIDECONTAINERS				0x00000200
#define SMF2_SHOWINFOBAR				0x00000400
#define SMF2_SHOWSTATUSBAR				0x00010000
#define SMF2_SHOWTITLEBAR				0x00020000
#define SMF2_SHOWTOOLBAR				0x00040000
#define SMF2_USETRANSPARENCY			0x00080000
#define SMF2_SHOWTYPING					0x01000000
#define SMF2_SHOWTYPINGWIN				0x02000000
#define SMF2_SHOWTYPINGTRAY				0x04000000
#define SMF2_SHOWTYPINGCLIST			0x08000000
#define SMF2_SHOWTYPINGSWITCH			0x10000000

typedef struct ImageListUsageEntry_tag
{
	int index;
	int used;
} ImageListUsageEntry;


struct GlobalMessageData
{
	unsigned int flags;
	unsigned int flags2;
	HANDLE hMessageWindowList;
	DWORD openFlags;
	HANDLE hParentWindowList;
	ParentWindowData *lastParent;
	ParentWindowData *lastChatParent;
	DWORD limitNamesLength;
	int activeAlpha;
	int inactiveAlpha;
	int tabIconListUsageSize;
	ImageListUsageEntry *tabIconListUsage;
	int smileyAddInstalled;
	int popupInstalled;
	int ieviewInstalled;
	int buttonVisibility;
	int chatBbuttonVisibility;
	int limitTabsNum;
	int limitChatsTabsNum;
	int indentSize;
	HIMAGELIST hTabIconList;
	HIMAGELIST hButtonIconList;
	HIMAGELIST hChatButtonIconList;
	HIMAGELIST hHelperIconList;
	HIMAGELIST hSearchEngineIconList;
	HBRUSH hInfobarBrush;
	int toolbarPosition;
	HWND hFocusWnd;
	DWORD logLineColour;
	int logPixelSX;
	int logPixelSY;
	HICON hMsgIcon;
	HICON hMsgIconBig;
	HICON hIconChatBig;
	int minInputAreaHeight;
};

int IconsChanged(WPARAM wParam, LPARAM lParam);
int SmileySettingsChanged(WPARAM wParam, LPARAM lParam);
void InitGlobals();
void FreeGlobals();
void ReloadGlobals();
void RegisterIcons();
void ReleaseIcons();
void LoadGlobalIcons();
HICON GetCachedIcon(const char *name);
void RegisterFontServiceFonts();
int ScriverRestoreWindowPosition(HWND hwnd, MCONTACT hContact, const char *szModule, const char *szNamePrefix, int flags, int showCmd);

int ImageList_AddIcon_Ex(HIMAGELIST hIml, int id);
int ImageList_AddIcon_Ex2(HIMAGELIST hIml, HICON hIcon);
int ImageList_ReplaceIcon_Ex(HIMAGELIST hIml, int nIndex, int id);
int ImageList_AddIcon_ProtoEx(HIMAGELIST hIml, const char *szProto, int status);

void StreamInTestEvents(HWND hEditWnd, GlobalMessageData *gdat);

#endif
