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

int OnLoadModule(void);
int OnUnloadModule(void);

CMPlugin g_plugin;

ITaskbarList3 *pTaskbarInterface;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {84636F78-2057-4302-8A65-23A16D46844C}
	{ 0x84636f78, 0x2057, 0x4302, { 0x8a, 0x65, 0x23, 0xa1, 0x6d, 0x46, 0x84, 0x4c } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(SRMM_MODULE, pluginInfoEx),
	bCascade(SRMM_MODULE, "Cascade", 1),
	bAutoMin(SRMM_MODULE, "AutoMin", 0),
	bTopmost(SRMM_MODULE, "Topmost", 0),
	bDelTemp(SRMM_MODULE, "DeleteTempCont", 0),
	bTypingNew(SRMM_MODULE, "DefaultTyping", 1),
	bAutoClose(SRMM_MODULE, "AutoClose", 0),
	bAutoPopup(SRMM_MODULE, "AutoPopupMsg", 0),
	bSaveDrafts(SRMM_MODULE, "SaveDrafts", 0),
	bTypingUnknown(SRMM_MODULE, "UnknownTyping", 0),
	bHideContainer(SRMM_MODULE, "HideContainers", 0),
	bStayMinimized(SRMM_MODULE, "StayMinimized", 0),
	bSavePerContact(SRMM_MODULE, "SavePerContact", 0),

	bShowAvatar(SRMM_MODULE, "AvatarEnable", 1),
	bShowProgress(SRMM_MODULE, "ShowProgress", 0),
	bShowIcons(SRMM_MODULE, "ShowLogIcon", 1),
	bShowTime(SRMM_MODULE, "ShowTime", 1),
	bShowSeconds(SRMM_MODULE, "ShowSeconds", 1),
	bShowDate(SRMM_MODULE, "ShowDate", 0),
	bLongDate(SRMM_MODULE, "UseLongDate", 0),
	bRelativeDate(SRMM_MODULE, "UseRelativeDate", 0),
	bDrawLines(SRMM_MODULE, "DrawLines", 0),
	bHideNames(SRMM_MODULE, "HideNames", 1),
	bIndentText(SRMM_MODULE, "IndentText", 0),
	bGroupMessages(SRMM_MODULE, "GroupMessages", 0),
	bMarkFollowups(SRMM_MODULE, "MarkFollowUps", 0),
	bMsgOnNewline(SRMM_MODULE, "MessageOnNewLine", 0),
	bUseTransparency(SRMM_MODULE, "UseTransparency", 0),

	bShowToolBar(SRMM_MODULE, "ShowButtonLine", 1),
	bShowInfoBar(SRMM_MODULE, "ShowInfoBar", 1),
	bShowTitleBar(SRMM_MODULE, "ShowTitleBar", 1),
	bShowStatusBar(SRMM_MODULE, "ShowStatusBar", 1),

	bUseTabs(SRMM_MODULE, "UseTabs", 1),
	bLimitTabs(SRMM_MODULE, "LimitTabs", 0),
	bLimitChatTabs(SRMM_MODULE, "LimitChatsTabs", 0),
	bLimitNames(SRMM_MODULE, "LimitNamesOnTabs", 1),
	bHideOneTab(SRMM_MODULE, "HideOneTab", 1),
	bTabsAtBottom(SRMM_MODULE, "TabsPosition", 0),
	bSeparateChats(SRMM_MODULE, "SeparateChatsContainers", 0),
	bTabCloseButton(SRMM_MODULE, "TabCloseButton", 0),
	bSwitchToActive(SRMM_MODULE, "SwitchToActiveTab", 0),

	bShowTyping(SRMM_MODULE, "ShowTyping", 1),
	bShowTypingWin(SRMM_MODULE, "ShowTypingWin", 1),
	bShowTypingTray(SRMM_MODULE, "ShowTypingTray", 0),
	bShowTypingClist(SRMM_MODULE, "ShowTypingClist", 1),
	bShowTypingSwitch(SRMM_MODULE, "ShowTypingSwitch", 1),

	iLimitNames(SRMM_MODULE, "LimitNamesLength", 20),
	iLimitTabs(SRMM_MODULE, "LimitTabsNum", 10),
	iLimitChatTabs(SRMM_MODULE, "LimitChatsTabsNum", 10),
	iLoadCount(SRMM_MODULE, "LoadCount", 10),
	iLoadTime(SRMM_MODULE, "LoadTime", 10),
	iPopFlags(SRMM_MODULE, "PopupFlags", 0),
	iFlashCount(SRMM_MODULE, "FlashMax", 3),
	iIndentSize(SRMM_MODULE, "IndentSize", 0),
	iActiveAlpha(SRMM_MODULE, "ActiveAlpha", 0),
	iInactiveAlpha(SRMM_MODULE, "InactiveAlpha", 0),
	iMsgTimeout(SRMM_MODULE, "MessageTimeout", 10),
	iHistoryMode(SRMM_MODULE, "LoadHistory", LOADHISTORY_UNREAD),
	iAutoResizeLines(SRMM_MODULE, "AutoResizeLines", 2)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_SRMM, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	if (IsWinVer7Plus())
		CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_ALL, IID_ITaskbarList3, (void**)&pTaskbarInterface);

	hLogger = RegisterSrmmLog("built-in", LPGENW("Scriver internal log"), &logBuilder);

	switch (getByte("UseIEView", -1)) {
	case 1:
		setString("Logger", "ieview");
		__fallthrough;

	case 0:
		delSetting("UseIEView");
	}

	return OnLoadModule();
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	UnregisterSrmmLog(hLogger);
	if (pTaskbarInterface)
		pTaskbarInterface->Release();
	return OnUnloadModule();
}
