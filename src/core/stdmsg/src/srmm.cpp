/*

Copyright 2000-12 Miranda IM, 2012-23 Miranda NG team,
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

int LoadSendRecvMessageModule(void);
void SplitmsgShutdown(void);

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	MIRANDA_VERSION_DWORD,
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	{ 0x657fe89b, 0xd121, 0x40c2, { 0x8a, 0xc9, 0xb9, 0xfa, 0x57, 0x55, 0xb3, 0x0D } } //{657FE89B-D121-40c2-8AC9-B9FA5755B30D}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(SRMMMOD, pluginInfoEx),

	bShowDate(SRMMMOD, "ShowDate", false),
	bShowTime(SRMMMOD, "ShowTime", true),
	bShowSecs(SRMMMOD, "ShowSeconds", true),
	bShowIcons(SRMMMOD, "ShowLogIcon", true),
	bShowAvatar(SRMMMOD, "AvatarEnable", true),
	bShowButtons(SRMMMOD, "ShowButtonLine", true),

	bTypingNew(SRMMMOD, "DefaultTyping", true),
	bTypingUnknown(SRMMMOD, "UnknownTyping", false),

	bShowTyping(SRMMMOD, "ShowTyping", true),
	bShowTypingWin(SRMMMOD, "ShowTypingWin", true),
	bShowTypingTray(SRMMMOD, "ShowTypingTray", false),
	bShowTypingClist(SRMMMOD, "ShowTypingClist", true),

	bCascade(SRMMMOD, "Cascade", true),
	bAutoMin(SRMMMOD, "AutoMin", false),
	bAutoClose(SRMMMOD, "AutoClose", false),
	bShowNames(SRMMMOD, "ShowNames", false),
	bShowFormat(SRMMMOD, "ShowFormatting", true),
	bSendButton(SRMMMOD, "UseSendButton", false),
	bCtrlSupport(SRMMMOD, "SupportCtrlUpDn", true),
	bShowReadChar(SRMMMOD, "ShowCharCount", false),
	bDeleteTempCont(SRMMMOD, "DeleteTempCont", false),
	bSavePerContact(SRMMMOD, "SavePerContact", false),
	bDoNotStealFocus(SRMMMOD, "DoNotStealFocus", false),
	bUseStatusWinIcon(SRMMMOD, "UseStatusWinIcon", true),

	bLimitAvatarHeight(SRMMMOD, "AvatarLimitHeight", true),
	iAvatarHeight(SRMMMOD, "AvatarHeight", 60),

	popupFlags(SRMMMOD, "PopupFlags", 0),
	nFlashMax(SRMMMOD, "FlashMax", 5),

	msgTimeout(SRMMMOD, "MessageTimeout", 65000),

	iLoadHistory(SRMMMOD, "LoadHistory", LOADHISTORY_UNREAD),
	nLoadCount(SRMMMOD, "LoadCount", 10),
	nLoadTime(SRMMMOD, "LoadTime", 10),

	// chat options
	bSoundsFocus(CHAT_MODULE, "SoundsFocus", false),

	// chat log options
	bAddColonToAutoComplete(CHAT_MODULE, "AddColonToAutoComplete", true)
{}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_SRMM, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	hLogger = RegisterSrmmLog(this, "built-in", LPGENW("StdMsg internal log"), &logBuilder);

	Load_ChatModule();
	return LoadSendRecvMessageModule();
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	UnregisterSrmmLog(hLogger);
	SplitmsgShutdown();
	Unload_ChatModule();
	return 0;
}
