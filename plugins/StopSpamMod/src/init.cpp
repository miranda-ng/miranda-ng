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

HINSTANCE hInst;

BOOL gbDosServiceExist = 0;
BOOL gbVarsServiceExist = 0;

DWORD gbMaxQuestCount = 5;
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

HANDLE hStopSpamLogDirH=0;

wstring gbSpammersGroup = L"Spammers";
wstring gbAutoAuthGroup	= L"NotSpammers";

wstring gbQuestion;
wstring gbAnswer;
wstring gbCongratulation;
std::wstring gbAuthRepl;
extern wchar_t const * defQuestion;
extern int RemoveTmp(WPARAM,LPARAM);

/////////////////////////////////////////////////////////////////////////////////////////
// returns plugin's extended information

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {94CED94C-A94A-4BB1-ACBD-5CC6EBB689D4}
	{0x94ced94c, 0xa94a, 0x4bb1, {0xac, 0xbd, 0x5c, 0xc6, 0xeb, 0xb6, 0x89, 0xd4}}
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfoEx;
}

extern wstring DBGetContactSettingStringPAN(MCONTACT hContact, char const * szModule, char const * szSetting, wstring errorValue);

void InitVars()
{
	gbSpammersGroup = DBGetContactSettingStringPAN(NULL, pluginName, "SpammersGroup", L"Spammers");
	gbAnswer = DBGetContactSettingStringPAN(NULL, pluginName, "answer", L"nospam");
	gbCongratulation = DBGetContactSettingStringPAN(NULL, pluginName, "congratulation", L"Congratulations! You just passed human/robot test. Now you can write me a message.");
	gbInfTalkProtection = db_get_b(NULL, pluginName, "infTalkProtection", 0);
	gbAddPermanent = db_get_b(NULL, pluginName, "addPermanent", 0);
	gbMaxQuestCount = db_get_dw(NULL, pluginName, "maxQuestCount", 5);
	gbHandleAuthReq = db_get_b(NULL, pluginName, "handleAuthReq", 1);
	gbQuestion = DBGetContactSettingStringPAN(NULL, pluginName, "question", defQuestion);
	gbAnswer = DBGetContactSettingStringPAN(NULL, pluginName, "answer", L"nospam");
	gbCongratulation = DBGetContactSettingStringPAN(NULL, pluginName, "congratulation", L"Congratulations! You just passed human/robot test. Now you can write me a message.");
	gbAuthRepl = DBGetContactSettingStringPAN(NULL, pluginName, "authrepl", L"StopSpam: send a message and reply to an anti-spam bot question.");
	gbSpecialGroup = db_get_b(NULL, pluginName, "SpecialGroup", 0);
	gbHideContacts = db_get_b(NULL, pluginName, "HideContacts", 0);
	gbIgnoreContacts = db_get_b(NULL, pluginName, "IgnoreContacts", 0);
	gbExclude = db_get_b(NULL, pluginName, "ExcludeContacts", 1);
	gbDelExcluded = db_get_b(NULL, pluginName, "DelExcluded", 0);
	gbDelAllTempory = db_get_b(NULL, pluginName, "DelAllTempory", 0);
	gbCaseInsensitive = db_get_b(NULL, pluginName, "CaseInsensitive", 0);
	gbRegexMatch = db_get_b(NULL, pluginName, "RegexMatch", 0);
	gbInvisDisable = db_get_b(NULL, pluginName, "DisableInInvis", 0);
	gbIgnoreURL = db_get_b(NULL, pluginName, "IgnoreURL", 0);
	gbAutoAuthGroup = DBGetContactSettingStringPAN(NULL, pluginName, "AutoAuthGroup", L"Not Spammers");
	gbAutoAuth = db_get_b(NULL, pluginName, "AutoAuth", 0);
	gbAutoAddToServerList = db_get_b(NULL, pluginName, "AutoAddToServerList", 0);
	gbAutoReqAuth = db_get_b(NULL, pluginName, "AutoReqAuth", 0);
	gbLogToFile = db_get_b(NULL, pluginName, "LogSpamToFile", 0);
	gbHistoryLog = db_get_b(NULL, pluginName, "HistoryLog", 0);
	gbMathExpression = db_get_b(NULL, pluginName, "MathExpression", 0);

}

static int OnSystemModulesLoaded(WPARAM, LPARAM)
{
/*	if (ServiceExists(MS_DOS_SERVICE))
		gbDosServiceExist = TRUE; */
	if (ServiceExists(MS_VARS_FORMATSTRING))
		gbVarsServiceExist = TRUE;
	InitVars();
	void CleanThread();
	if(gbDelAllTempory || gbDelExcluded)
		new boost::thread(&CleanThread);
	
	// Folders plugin support
	hStopSpamLogDirH = FoldersRegisterCustomPathT(LPGEN("StopSpam"), LPGEN("StopSpam Logs"), FOLDER_LOGS);
	return 0;
}

HANDLE hEventFilter = 0, hOptInitialise = 0, hSettingChanged = 0;


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
	hInst = hinstDLL;
	return TRUE;
}

int hLangpack = 0;
CLIST_INTERFACE *pcli;

extern "C" int __declspec(dllexport) Load()
{
	mir_getLP(&pluginInfoEx);
	pcli = Clist_GetInterface();

	CreateServiceFunction("/RemoveTmp", (MIRANDASERVICE)RemoveTmp);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnSystemModulesLoaded);
	HookEvent(ME_DB_EVENT_ADDED, OnDbEventAdded);
	HookEvent(ME_DB_EVENT_FILTER_ADD, OnDbEventFilterAdd);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnDbContactSettingChanged);
	HookEvent(ME_OPT_INITIALISE, OnOptInit);

	CMenuItem mi;
	SET_UID(mi, 0x60ce7660, 0x5a5, 0x4234, 0x99, 0xb6, 0x55, 0x21, 0xed, 0xa0, 0xb8, 0x32);
	mi.position = -0x7FFFFFFF;
	mi.hIcolibItem = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);
	mi.name.a = LPGEN("Remove Temporary Contacts");
	mi.pszService = "/RemoveTmp";

	Menu_AddMainMenuItem(&mi);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
