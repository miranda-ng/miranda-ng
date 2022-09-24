/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-22 Miranda NG team,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// plugin loading functions and global exports.

#include "stdafx.h"

LOGFONT lfDefault = { 0 };

/*
 * miranda interfaces
 */

CMPlugin g_plugin;

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
	// {6CA5F042-7A7F-47CC-A715-FC8C46FBF434}
	{ 0x6ca5f042, 0x7a7f, 0x47cc, { 0xa7, 0x15, 0xfc, 0x8c, 0x46, 0xfb, 0xf4, 0x34 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>("SRMsg", pluginInfoEx),

	// main settings
	bAutoMin(SRMSGMOD_T, "AutoMin", false),
	bAutoCopy(SRMSGMOD_T, "autocopy", true),
	bAutoTabs(SRMSGMOD_T, "autotabs", true),
	bAllowTab(SRMSGMOD_T, "tabmode", false),
	bAutoClose(SRMSGMOD_T, "AutoClose", false),
	bAutoPopup(SRMSGMOD_T, "AutoPopup", false),
	bAutoSplit(SRMSGMOD_T, "autosplit", false),
	bDeleteTemp(SRMSGMOD_T, "deletetemp", false),
	bUseXStatus(SRMSGMOD_T, "use_xicons", true),
	bSendFormat(SRMSGMOD_T, "sendformat", false),
	bHideOnClose(SRMSGMOD_T, "hideonclose", false),
	bStatusOnTabs(SRMSGMOD_T, "tabstatus", true),
	bFlashOnClist(SRMSGMOD_T, "flashcl", false),
	bPasteAndSend(SRMSGMOD_T, "pasteandsend", true),
	bAutoContainer(SRMSGMOD_T, "autocontainer", true),
	bAutoSwitchTabs(SRMSGMOD_T, "autoswitchtabs", true),
	bPopupContainer(SRMSGMOD_T, "cpopup", true),
	bDetailedTooltips(SRMSGMOD_T, "d_tooltips", false),
	bUseSameSplitSize(SRMSGMOD_T, "usesamesplitsize", true),
	bAllowOfflineMultisend(SRMSGMOD_T, "AllowOfflineMultisend", true),

	// advanced options
	bMetaBar(SRMSGMOD_T, "MetaiconBar", true),
	bMetaTab(SRMSGMOD_T, "MetaiconTab", true),
	bShowDesc(SRMSGMOD_T, "ShowClientDescription", false),
	bCloseSend(SRMSGMOD_T, "adv_AutoClose_2", false),
	bErrorPopup(SRMSGMOD_T, "adv_ErrorPopups", true),
	
	// chat settings
	bOpenInDefault(CHAT_MODULE, "DefaultContainer", true),
	bCreateWindowOnHighlight(CHAT_MODULE, "CreateWindowOnHighlight", false),
	bBBCodeInPopups(CHAT_MODULE, "BBCodeInPopups", false),
	bClassicIndicators(CHAT_MODULE, "ClassicIndicators", false),
	bLogClassicIndicators(CHAT_MODULE, "LogClassicIndicators", false),
	bAlternativeSorting(CHAT_MODULE, "AlternativeSorting", true),
	bAnnoyingHighlight(CHAT_MODULE, "AnnoyingHighlight", false),
	bLogSymbols(CHAT_MODULE, "LogSymbols", true),
	bClickableNicks(CHAT_MODULE, "ClickableNicks", true),
	bColorizeNicks(CHAT_MODULE, "ColorizeNicks", true),
	bColorizeNicksInLog(CHAT_MODULE, "ColorizeNicksInLog", true),
	bScaleIcons(CHAT_MODULE, "ScaleIcons", true),
	bNewLineAfterNames(CHAT_MODULE, "NewlineAfterNames", false),

	// typing settings
	bPopups(TypingModule, "TypingPopup", true),
	bTypingNew(TypingModule, "DefaultTyping", true),
	bTypingUnknown(TypingModule, "UnknownTyping", false),

	// log options
	bUseDividers(SRMSGMOD_T, "usedividers", false),
	bLogStatusChanges(SRMSGMOD_T, "logstatuschanges", false),
	bDividersUsePopupConfig(SRMSGMOD_T, "div_popupconfig", false)
{}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_SRMM, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lfDefault), &lfDefault, FALSE);

	hLogger = RegisterSrmmLog(this, "built-in", LPGENW("tabSRMM internal log"), &logBuilder);
	
	Chat_Load();

	return LoadSendRecvMessageModule();
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	UnregisterSrmmLog(hLogger);
	FreeLogFonts();
	Chat_Unload();
	int iRet = SplitmsgShutdown();
	Skin->setupTabCloseBitmap(true);
	Skin->UnloadAeroTabs();
	CleanTempFiles();
	SendLater::shutDown();
	delete Skin;
	delete sendQueue;
	return iRet;
}
