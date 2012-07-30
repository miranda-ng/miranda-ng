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


#include "headers.h"



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
BOOL gbDosServiceIntegration = 0;
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

tstring gbSpammersGroup = _T("Spammers");
tstring gbAutoAuthGroup	= _T("NotSpammers");

tstring gbQuestion;
tstring gbAnswer;
tstring gbCongratulation;
std::wstring gbAuthRepl;
extern char * pluginDescription;
extern TCHAR const * defQuestion;
extern int RemoveTmp(WPARAM,LPARAM);



/////////////////////////////////////////////////////////////////////////////////////////
// returns plugin's extended information

// {553811EE-DEB6-48b8-8902-A8A00C1FD679}
#define MIID_STOPSPAM { 0x553811ee, 0xdeb6, 0x48b8, { 0x89, 0x2, 0xa8, 0xa0, 0xc, 0x1f, 0xd6, 0x79 } }

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	0,
	PLUGIN_MAKE_VERSION(0, 0, 2, 0),
	pluginDescription,
	"Roman Miklashevsky, sss, Elzor",
	"sss123next@list.ru",
	"© 2004-2012 Roman Miklashevsky, A. Petkevich, Kosh&chka, sss, Elzor",
	"http://sss.chaoslab.ru/tracker/mim_plugs/",
	UNICODE_AWARE,	
	MIID_STOPSPAM
};


char *date()
{
	static char d[11];
	char *tmp = __DATE__, m[4], mn[3] = "01";
	m[0]=tmp[0];
	m[1]=tmp[1];
	m[2]=tmp[2];
	if(strstr(m,"Jan"))
		strcpy(mn,"01");
	else if(strstr(m,"Feb"))
		strcpy(mn,"02");
	else if(strstr(m,"Mar"))
		strcpy(mn,"03");
	else if(strstr(m,"Apr"))
		strcpy(mn,"04");
	else if(strstr(m,"May"))
		strcpy(mn,"05");
	else if(strstr(m,"Jun"))
		strcpy(mn,"06");
	else if(strstr(m,"Jul"))
		strcpy(mn,"07");
	else if(strstr(m,"Aug"))
		strcpy(mn,"08");
	else if(strstr(m,"Sep"))
		strcpy(mn,"09");
	else if(strstr(m,"Oct"))
		strcpy(mn,"10");
	else if(strstr(m,"Nov"))
		strcpy(mn,"11");
	else if(strstr(m,"Dec"))
		strcpy(mn,"12");
	d[0]=tmp[7];
	d[1]=tmp[8];
	d[2]=tmp[9];
	d[3]=tmp[10];
	d[4]='.';
	d[5]=mn[0];
	d[6]=mn[1];
	d[7]='.';
	if (tmp[4] == ' ')
		d[8] = '0';
	else
		d[8]=tmp[4];
	d[9]=tmp[5];
	return d;
}


extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if ( mirandaVersion < PLUGIN_MAKE_VERSION( 0, 7, 0, 1 ))
		return NULL;
	{
	  static char plugname[52];
	  strcpy(plugname, pluginName" mod [");
	  strcat(plugname, date());
	  strcat(plugname, " ");
	  strcat(plugname, __TIME__);
	  strcat(plugname, "]");
	  pluginInfoEx.shortName = plugname;
	}

	return &pluginInfoEx;
}

extern tstring DBGetContactSettingStringPAN(HANDLE hContact, char const * szModule, char const * szSetting, tstring errorValue);

void InitVars()
{
	gbDosServiceIntegration = DBGetContactSettingByte(NULL, pluginName, "DOSIntegration", 0);
	gbSpammersGroup = DBGetContactSettingStringPAN(NULL, pluginName, "SpammersGroup", _T("Spammers"));
	gbAnswer = DBGetContactSettingStringPAN(NULL, pluginName, "answer", _T("nospam"));
	gbCongratulation = DBGetContactSettingStringPAN(NULL, pluginName, "congratulation", _T("Congratulations! You just passed human/robot test. Now you can write me a message."));
	gbInfTalkProtection = DBGetContactSettingByte(NULL, pluginName, "infTalkProtection", 0);
	gbAddPermanent = DBGetContactSettingByte(NULL, pluginName, "addPermanent", 0);
	gbMaxQuestCount = DBGetContactSettingDword(NULL, pluginName, "maxQuestCount", 5);
	gbHandleAuthReq = DBGetContactSettingByte(NULL, pluginName, "handleAuthReq", 1);
	gbQuestion = DBGetContactSettingStringPAN(NULL, pluginName, "question", defQuestion);
	gbAnswer = DBGetContactSettingStringPAN(NULL, pluginName, "answer", _T("nospam"));
	gbCongratulation = DBGetContactSettingStringPAN(NULL, pluginName, "congratulation", _T("Congratulations! You just passed human/robot test. Now you can write me a message."));
	gbAuthRepl = DBGetContactSettingStringPAN(NULL, pluginName, "authrepl", _T("StopSpam: send a message and reply to a anti-spam bot question."));
	gbSpecialGroup = DBGetContactSettingByte(NULL, pluginName, "SpecialGroup", 0);
	gbHideContacts = DBGetContactSettingByte(NULL, pluginName, "HideContacts", 0);
	gbIgnoreContacts = DBGetContactSettingByte(NULL, pluginName, "IgnoreContacts", 0);
	gbExclude = DBGetContactSettingByte(NULL, pluginName, "ExcludeContacts", 1);
	gbDelExcluded = DBGetContactSettingByte(NULL, pluginName, "DelExcluded", 0);
	gbDelAllTempory = DBGetContactSettingByte(NULL, pluginName, "DelAllTempory", 0);
	gbCaseInsensitive = DBGetContactSettingByte(NULL, pluginName, "CaseInsensitive", 0);
	gbRegexMatch = DBGetContactSettingByte(NULL, pluginName, "RegexMatch", 0);
	gbInvisDisable = DBGetContactSettingByte(NULL, pluginName, "DisableInInvis", 0);
	gbIgnoreURL = DBGetContactSettingByte(NULL, pluginName, "IgnoreURL", 0);
	gbAutoAuthGroup = DBGetContactSettingStringPAN(NULL, pluginName, "AutoAuthGroup", _T("Not Spammers"));
	gbAutoAuth=DBGetContactSettingByte(NULL, pluginName, "AutoAuth", 0); 
	gbAutoAddToServerList=DBGetContactSettingByte(NULL, pluginName, "AutoAddToServerList", 0); 
	gbAutoReqAuth=DBGetContactSettingByte(NULL, pluginName, "AutoReqAuth", 0); 
	gbLogToFile=DBGetContactSettingByte(NULL, pluginName, "LogSpamToFile", 0); 
	gbHistoryLog = DBGetContactSettingByte(NULL, pluginName, "HistoryLog", 0); 
	gbMathExpression = DBGetContactSettingByte(NULL, pluginName, "MathExpression", 0); 

}

static int OnSystemModulesLoaded(WPARAM wParam,LPARAM lParam)
{
/*	if (ServiceExists(MS_DOS_SERVICE))
		gbDosServiceExist = TRUE; */
	if (ServiceExists(MS_VARS_FORMATSTRING))
		gbVarsServiceExist = TRUE;
	InitVars();
	void CleanThread();
	if(gbDelAllTempory || gbDelExcluded)
		boost::thread *thr = new boost::thread(&CleanThread);
	// Folders plugin support
	if (ServiceExists(MS_FOLDERS_REGISTER_PATH))
	{
		hStopSpamLogDirH = (HANDLE) FoldersRegisterCustomPath("StopSpam", "StopSpam Logs",
			PROFILE_PATH "\\" CURRENT_PROFILE "\\StopSpamLog");
	}
	return 0;
}

HANDLE hEventFilter = 0, hOptInitialise = 0, hSettingChanged = 0;


BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	/*if(DLL_PROCESS_ATTACH == fdwReason)
		hInst=hinstDLL;
	return TRUE;*/
	hInst = hinstDLL;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// returns plugin's interfaces information

static const MUUID interfaces[] = { MIID_STOPSPAM, MIID_LAST };

extern "C"  __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

int hLangpack = 0;

extern "C" int __declspec(dllexport) Load()
{
	mir_getLP(&pluginInfoEx);
	CLISTMENUITEM mi;
	CreateServiceFunction("/RemoveTmp", (MIRANDASERVICE)RemoveTmp);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnSystemModulesLoaded);
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=-0x7FFFFFFF;
	mi.flags=0;
	mi.hIcon=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	mi.pszName="Remove Temporary Contacts";
	mi.pszService="/RemoveTmp";

	Menu_AddMainMenuItem(&mi);

	miranda::EventHooker::HookAll();
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	miranda::EventHooker::UnhookAll();
	return 0;
}
