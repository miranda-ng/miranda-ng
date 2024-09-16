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
	bCascade(SRMM_MODULE, "Cascade", true),
	bAutoMin(SRMM_MODULE, "AutoMin", false),
	bAutoCopy(SRMM_MODULE, "AutoCopy", true),
	bTopmost(SRMM_MODULE, "Topmost", false),
	bDelTemp(SRMM_MODULE, "DeleteTempCont", false),
	bTypingNew(SRMM_MODULE, "DefaultTyping", true),
	bAutoClose(SRMM_MODULE, "AutoClose", false),
	bAutoPopup(SRMM_MODULE, "AutoPopupMsg", false),
	bSaveDrafts(SRMM_MODULE, "SaveDrafts", false),
	bSendFormat(SRMM_MODULE, "SendFormat", false),
	bTypingUnknown(SRMM_MODULE, "UnknownTyping", false),
	bHideContainer(SRMM_MODULE, "HideContainers", false),
	bStayMinimized(SRMM_MODULE, "StayMinimized", false),
	bSavePerContact(SRMM_MODULE, "SavePerContact", false),

	bShowAvatar(SRMM_MODULE, "AvatarEnable", true),
	bShowProgress(SRMM_MODULE, "ShowProgress", false),
	bShowIcons(SRMM_MODULE, "ShowLogIcon", true),
	bShowTime(SRMM_MODULE, "ShowTime", true),
	bShowSeconds(SRMM_MODULE, "ShowSeconds", true),
	bShowDate(SRMM_MODULE, "ShowDate", false),
	bLongDate(SRMM_MODULE, "UseLongDate", false),
	bRelativeDate(SRMM_MODULE, "UseRelativeDate", false),
	bDrawLines(SRMM_MODULE, "DrawLines", false),
	bHideNames(SRMM_MODULE, "HideNames", true),
	bIndentText(SRMM_MODULE, "IndentText", false),
	bGroupMessages(SRMM_MODULE, "GroupMessages", false),
	bMarkFollowups(SRMM_MODULE, "MarkFollowUps", false),
	bMsgOnNewline(SRMM_MODULE, "MessageOnNewLine", false),
	bUseTransparency(SRMM_MODULE, "UseTransparency", false),

	bShowToolBar(SRMM_MODULE, "ShowButtonLine", true),
	bShowInfoBar(SRMM_MODULE, "ShowInfoBar", true),
	bShowTitleBar(SRMM_MODULE, "ShowTitleBar", true),
	bShowStatusBar(SRMM_MODULE, "ShowStatusBar", true),

	bUseTabs(SRMM_MODULE, "UseTabs", true),
	bLimitTabs(SRMM_MODULE, "LimitTabs", false),
	bLimitChatTabs(SRMM_MODULE, "LimitChatsTabs", false),
	bLimitNames(SRMM_MODULE, "LimitNamesOnTabs", true),
	bHideOneTab(SRMM_MODULE, "HideOneTab", true),
	bTabsAtBottom(SRMM_MODULE, "TabsPosition", false),
	bSeparateChats(SRMM_MODULE, "SeparateChatsContainers", false),
	bTabCloseButton(SRMM_MODULE, "TabCloseButton", false),
	bSwitchToActive(SRMM_MODULE, "SwitchToActiveTab", false),

	bShowTyping(SRMM_MODULE, "ShowTyping", true),
	bShowTypingWin(SRMM_MODULE, "ShowTypingWin", true),
	bShowTypingTray(SRMM_MODULE, "ShowTypingTray", false),
	bShowTypingClist(SRMM_MODULE, "ShowTypingClist", true),
	bShowTypingSwitch(SRMM_MODULE, "ShowTypingSwitch", true),

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
	iAutoResizeLines(SRMM_MODULE, "AutoResizeLines", 2),

	bSoundsFocus(CHAT_MODULE, "SoundsFocus", false),
	bAddColonToAutoComplete(CHAT_MODULE, "AddColonToAutoComplete", true)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_SRMM, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	if (IsWinVer7Plus())
		CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_ALL, IID_ITaskbarList3, (void**)&pTaskbarInterface);

	hLogger = RegisterSrmmLog(this, "built-in", LPGENW("Scriver internal log"), &logBuilder);

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
