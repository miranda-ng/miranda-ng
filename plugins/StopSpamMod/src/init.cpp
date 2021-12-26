/* Copyright (C) Miklashevsky Roman, sss, elzor
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

CMPlugin g_plugin;

HANDLE hEventFilter = nullptr, hOptInitialise = nullptr, hSettingChanged = nullptr;

BOOL gbDosServiceExist = 0;
BOOL gbVarsServiceExist = 0;

uint32_t gbMaxQuestCount = 5;
BOOL gbInfTalkProtection = 0;
BOOL gbAddPermanent = 0;
BOOL gbHandleAuthReq = 1;
BOOL gbSpecialGroup = 0;
BOOL gbHideContacts = 1;
BOOL gbIgnoreContacts = 0;
BOOL gbExclude = 1;
BOOL gbDelExcluded = 0;
BOOL gbDelAllTempory = 0;
BOOL gbHistoryLog = 0;
BOOL gbCaseInsensitive = 0;
BOOL gbRegexMatch = 0;
BOOL gbInvisDisable = 0;
BOOL gbIgnoreURL = 1;
BOOL gbLogToFile=0;
BOOL gbAutoAuth=0;
BOOL gbAutoAddToServerList=0;
BOOL gbAutoReqAuth=1;
BOOL gbMathExpression = 0;

HANDLE hStopSpamLogDirH = nullptr;

wstring gbSpammersGroup = L"Spammers";
wstring gbAutoAuthGroup	= L"NotSpammers";

wstring gbQuestion;
wstring gbAnswer;
wstring gbCongratulation;
std::wstring gbAuthRepl;
extern const wchar_t *defQuestion, *defCongrats, *defAuthReply;
extern int RemoveTmp(WPARAM,LPARAM);

/////////////////////////////////////////////////////////////////////////////////////////
// returns plugin's extended information

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {94CED94C-A94A-4BB1-ACBD-5CC6EBB689D4}
	{0x94ced94c, 0xa94a, 0x4bb1, {0xac, 0xbd, 0x5c, 0xc6, 0xeb, 0xb6, 0x89, 0xd4}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

extern wstring DBGetContactSettingStringPAN(MCONTACT hContact, char const * szModule, char const * szSetting, wstring errorValue);

void InitVars()
{
	gbSpammersGroup = DBGetContactSettingStringPAN(0, MODULENAME, "SpammersGroup", L"Spammers");
	gbAnswer = DBGetContactSettingStringPAN(0, MODULENAME, "answer", L"nospam");
	gbInfTalkProtection = g_plugin.getByte("infTalkProtection", 0);
	gbAddPermanent = g_plugin.getByte("addPermanent", 0);
	gbMaxQuestCount = g_plugin.getDword("maxQuestCount", 5);
	gbHandleAuthReq = g_plugin.getByte("handleAuthReq", 1);
	gbQuestion = DBGetContactSettingStringPAN(0, MODULENAME, "question", TranslateW(defQuestion));
	gbAnswer = DBGetContactSettingStringPAN(0, MODULENAME, "answer", L"nospam");
	gbCongratulation = DBGetContactSettingStringPAN(0, MODULENAME, "congratulation", TranslateW(defCongrats));
	gbAuthRepl = DBGetContactSettingStringPAN(0, MODULENAME, "authrepl", TranslateW(defAuthReply));
	gbSpecialGroup = g_plugin.getByte("SpecialGroup", 0);
	gbHideContacts = g_plugin.getByte("HideContacts", 0);
	gbIgnoreContacts = g_plugin.getByte("IgnoreContacts", 0);
	gbExclude = g_plugin.getByte("ExcludeContacts", 1);
	gbDelExcluded = g_plugin.getByte("DelExcluded", 0);
	gbDelAllTempory = g_plugin.getByte("DelAllTempory", 0);
	gbCaseInsensitive = g_plugin.getByte("CaseInsensitive", 0);
	gbRegexMatch = g_plugin.getByte("RegexMatch", 0);
	gbInvisDisable = g_plugin.getByte("DisableInInvis", 0);
	gbIgnoreURL = g_plugin.getByte("IgnoreURL", 0);
	gbAutoAuthGroup = DBGetContactSettingStringPAN(0, MODULENAME, "AutoAuthGroup", L"Not Spammers");
	gbAutoAuth = g_plugin.getByte("AutoAuth", 0);
	gbAutoAddToServerList = g_plugin.getByte("AutoAddToServerList", 0);
	gbAutoReqAuth = g_plugin.getByte("AutoReqAuth", 0);
	gbLogToFile = g_plugin.getByte("LogSpamToFile", 0);
	gbHistoryLog = g_plugin.getByte("HistoryLog", 0);
	gbMathExpression = g_plugin.getByte("MathExpression", 0);
}

static int OnSystemModulesLoaded(WPARAM, LPARAM)
{
	if (ServiceExists(MS_VARS_FORMATSTRING))
		gbVarsServiceExist = TRUE;

	InitVars();
	if(gbDelAllTempory || gbDelExcluded)
		mir_forkthread(&CleanThread);
	
	// Folders plugin support
	hStopSpamLogDirH = FoldersRegisterCustomPathW(LPGEN("StopSpam"), LPGEN("StopSpam Logs"), FOLDER_LOGS);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	CreateServiceFunction("/RemoveTmp", (MIRANDASERVICE)RemoveTmp);
	
	HookEvent(ME_DB_EVENT_ADDED, OnDbEventAdded);
	HookEvent(ME_DB_EVENT_FILTER_ADD, OnDbEventFilterAdd);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnDbContactSettingChanged);
	HookEvent(ME_OPT_INITIALISE, OnOptInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnSystemModulesLoaded);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x60ce7660, 0x5a5, 0x4234, 0x99, 0xb6, 0x55, 0x21, 0xed, 0xa0, 0xb8, 0x32);
	mi.position = -0x7FFFFFFF;
	mi.hIcolibItem = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);
	mi.name.a = LPGEN("Remove Temporary Contacts");
	mi.pszService = "/RemoveTmp";
	Menu_AddMainMenuItem(&mi);

	return 0;
}
