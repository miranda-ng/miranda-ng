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

#ifndef SRMM_COMMONHEADERS_H
#define SRMM_COMMONHEADERS_H

#define COMPILE_MULTIMON_STUBS
#define SRMM_OWN_STRUCTURES

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <richedit.h>
#include <richole.h>

#include <malloc.h>
#include <math.h>
#include <time.h>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_button.h>
#include <m_clistint.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_contacts.h>
#include <m_options.h>
#include <m_userinfo.h>
#include <m_history.h>
#include <m_message.h>
#include <m_file.h>
#include <m_fontservice.h>
#include <m_icolib.h>
#include <m_avatars.h>
#include <m_hotkeys.h>
#include <m_popup.h>
#include <m_timezones.h>
#include <m_xstatus.h>
#include <m_chat_int.h>
#include <m_srmm_int.h>

#include <m_ieview.h>
#include <m_smileyadd.h>
#include <m_metacontacts.h>

struct CMPlugin : public PLUGIN<CMPlugin>
{
	HANDLE hLogger;

	CMOption<uint8_t> bSavePerContact, bCascade, bStayMinimized, bAutoMin, bSaveDrafts, bDelTemp, bHideContainer, bAutoPopup;
	CMOption<uint8_t> bUseTransparency, bTopmost, bAutoClose, bTypingNew, bTypingUnknown;
	CMOption<uint8_t> bShowTitleBar, bShowStatusBar, bShowToolBar, bShowInfoBar;
	CMOption<uint8_t> bShowAvatar, bShowProgress, bShowIcons, bShowTime, bShowSeconds, bShowDate, bLongDate, bRelativeDate;
	CMOption<uint8_t>	bGroupMessages, bMarkFollowups, bMsgOnNewline, bDrawLines, bHideNames, bIndentText;
	CMOption<uint8_t> bUseTabs, bLimitTabs, bLimitChatTabs, bLimitNames, bSeparateChats, bTabCloseButton, bHideOneTab, bTabsAtBottom, bSwitchToActive;
	CMOption<uint8_t> bShowTyping, bShowTypingWin, bShowTypingTray, bShowTypingClist, bShowTypingSwitch;
	CMOption<uint8_t> iFlashCount, iHistoryMode;
	CMOption<uint16_t> iLimitNames, iLimitTabs, iLimitChatTabs, iLoadCount, iLoadTime, iIndentSize, iAutoResizeLines;
	CMOption<uint32_t> iPopFlags, iMsgTimeout, iActiveAlpha, iInactiveAlpha;

	CMPlugin();

	int Load() override;
	int Unload() override;
};

#include "resource.h"
#include "version.h"

#ifndef IMF_AUTOFONTSIZEADJUST
#define IMF_AUTOFONTSIZEADJUST	0x0010
#endif
#ifndef IMF_AUTOKEYBOARD
#define IMF_AUTOKEYBOARD		0x0001
#endif
#ifndef SES_EXTENDBACKCOLOR
#define SES_EXTENDBACKCOLOR	4
#endif
#ifndef ST_NEWCHARS
#define ST_NEWCHARS		4
#endif
#ifndef CFM_WEIGHT
#define	CFM_WEIGHT			0x00400000
#endif

extern HCURSOR hDragCursor;
extern ITaskbarList3 *pTaskbarInterface;

void LoadInfobarFonts();

#define SPLITTER_HEIGHT   4
#define TOOLBAR_HEIGHT   24
#define INFO_BAR_HEIGHT  54
#define INFO_BAR_INNER_HEIGHT (INFO_BAR_HEIGHT - 3)
#define INFO_BAR_AVATAR_HEIGHT INFO_BAR_INNER_HEIGHT
#define BOTTOM_RIGHT_AVATAR_HEIGHT 64
#define INFO_BAR_COLOR COLOR_INACTIVEBORDER

int Chat_Load();
int OptInitialise(WPARAM wParam, LPARAM lParam);
int FontServiceFontsChanged(WPARAM wParam, LPARAM lParam);
int StatusIconPressed(WPARAM wParam, LPARAM lParam);

CSrmmLogWindow *logBuilder(CMsgDialog &pDlg);

#include "cmdlist.h"
#include "sendqueue.h"
#include "globals.h"
#include "tabs.h"
#include "msgs.h"
#include "utils.h"
#include "input.h"
#include "statusicon.h"
#include "chat.h"

extern GlobalMessageData g_dat;
extern LIST<CMsgDialog> g_arDialogs;

#endif
