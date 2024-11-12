/*

Copyright 2000-12 Miranda IM, 2012-24 Miranda NG team,
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
	PLUGIN<CMPlugin>(SRMM_MODULE, pluginInfoEx),

	bShowDate(SRMM_MODULE, "ShowDate", false),
	bShowTime(SRMM_MODULE, "ShowTime", true),
	bShowSecs(SRMM_MODULE, "ShowSeconds", true),
	bShowIcons(SRMM_MODULE, "ShowLogIcon", true),
	bShowAvatar(SRMM_MODULE, "AvatarEnable", true),
	bShowButtons(SRMM_MODULE, "ShowButtonLine", true),

	bTypingNew(SRMM_MODULE, "DefaultTyping", true),
	bTypingUnknown(SRMM_MODULE, "UnknownTyping", false),

	bShowTyping(SRMM_MODULE, "ShowTyping", true),
	bShowTypingWin(SRMM_MODULE, "ShowTypingWin", true),
	bShowTypingTray(SRMM_MODULE, "ShowTypingTray", false),
	bShowTypingClist(SRMM_MODULE, "ShowTypingClist", true),

	bCascade(SRMM_MODULE, "Cascade", true),
	bAutoMin(SRMM_MODULE, "AutoMin", false),
	bAutoCopy(SRMM_MODULE, "AutoCopy", true),
	bAutoClose(SRMM_MODULE, "AutoClose", false),
	bShowNames(SRMM_MODULE, "ShowNames", false),
	bShowFormat(SRMM_MODULE, "ShowFormatting", true),
	bSendButton(SRMM_MODULE, "UseSendButton", false),
	bCtrlSupport(SRMM_MODULE, "SupportCtrlUpDn", true),
	bShowReadChar(SRMM_MODULE, "ShowCharCount", false),
	bDeleteTempCont(SRMM_MODULE, "DeleteTempCont", false),
	bSavePerContact(SRMM_MODULE, "SavePerContact", false),
	bDoNotStealFocus(SRMM_MODULE, "DoNotStealFocus", false),
	bUseStatusWinIcon(SRMM_MODULE, "UseStatusWinIcon", true),

	bLimitAvatarHeight(SRMM_MODULE, "AvatarLimitHeight", true),
	iAvatarHeight(SRMM_MODULE, "AvatarHeight", 60),

	popupFlags(SRMM_MODULE, "PopupFlags", 0),
	nFlashMax(SRMM_MODULE, "FlashMax", 5),

	msgTimeout(SRMM_MODULE, "MessageTimeout", 65000),

	nLoadCount(SRMM_MODULE, "LoadCount", 10),
	nLoadTime(SRMM_MODULE, "LoadTime", 10),

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
