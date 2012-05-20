/*

Simple Status Message plugin for Miranda IM
Copyright (C) 2006-2011 Bartosz 'Dezeath' Bia³ek, (C) 2005 Harven

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include "commonheaders.h"
#include "simplestatusmsg.h"

static HANDLE *hHookList = NULL;
static HANDLE *hProtoHookList = NULL;
static HANDLE *hServiceList = NULL;
static int HookCount = 0;
static int ProtoHookCount = 0;
static int ServiceCount = 0;

struct tagiconList
{
	const TCHAR *szDescr;
	const char	*szName;
	int			defIconID;
}
static const iconList[] =
{
	{ LPGENT("Delete Selected"),			"cross",	IDI_CROSS		},
	{ LPGENT("Recent Message"),				"recent",	IDI_HISTORY		},
	{ LPGENT("Predefined Message"),			"predef",	IDI_MESSAGE		},
	{ LPGENT("Add to Predefined"),			"add",		IDI_PLUS		},
	{ LPGENT("Clear History"),				"clear",	IDI_CHIST		},
	{ LPGENT("Copy Away Message"),			"copy",		IDI_COPY		},
	{ LPGENT("Change Status Message"),		"csmsg",	IDI_CSMSG,		},
	{ LPGENT("Go to URL in Away Message"),	"gotourl",	IDI_GOTOURL		}
};

HANDLE hIconLibItem[SIZEOF(iconList)];

void IconsInit(void)
{
	SKINICONDESC sid = {0};
	char szFile[MAX_PATH];
	char szSettingName[100];

	GetModuleFileNameA(g_hInst, szFile, MAX_PATH);
	sid.cbSize = sizeof(SKINICONDESC);
	sid.flags = SIDF_TCHAR;
	sid.pszDefaultFile = szFile;
	sid.ptszSection = _T("Simple Status Message");
	for (int i = 0; i < SIZEOF(iconList); i++)
	{
		mir_snprintf(szSettingName, SIZEOF(szSettingName), "SimpleStatusMsg_%s", iconList[i].szName);
		sid.pszName = szSettingName;
		sid.ptszDescription = (TCHAR*)iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].defIconID;
		hIconLibItem[i] = (HANDLE)CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
	}
}

HICON LoadIconEx(const char* name)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, sizeof(szSettingName), "SimpleStatusMsg_%s", name);
	return (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)szSettingName);
}

HANDLE GetIconHandle(int iconId)
{
	for(int i = 0; i < SIZEOF(iconList); i++)
		if (iconList[i].defIconID == iconId) return hIconLibItem[i];
	return NULL;
}

void ReleaseIconEx(const char* name)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, sizeof(szSettingName), "SimpleStatusMsg_%s",  name);
	CallService(MS_SKIN2_RELEASEICON, 0, (LPARAM)szSettingName);
}

HANDLE HookEventEx(const char *szEvent, MIRANDAHOOK hookProc)
{
	HookCount++;
	hHookList = (HANDLE *)mir_realloc(hHookList, sizeof(HANDLE) * HookCount);
	return hHookList[HookCount - 1] = HookEvent(szEvent, hookProc);
}

void UnhookEvents(void)
{
	if (hHookList == NULL) return;
	for (int i = 0; i < HookCount; ++i)
		if (hHookList[i] != NULL) UnhookEvent(hHookList[i]);
	mir_free(hHookList);
	hHookList = NULL;
	HookCount = 0;
}

HANDLE HookProtoEvent(const char *szModule, const char *szEvent, MIRANDAHOOKPARAM hookProc)
{
	char szProtoEvent[MAXMODULELABELLENGTH];
	mir_snprintf(szProtoEvent, sizeof(szProtoEvent), "%s%s", szModule, szEvent);
	ProtoHookCount++;
	hProtoHookList = (HANDLE *)mir_realloc(hProtoHookList, sizeof(HANDLE) * ProtoHookCount);
	return hProtoHookList[ProtoHookCount - 1] = HookEventParam(szProtoEvent, hookProc, (LPARAM)szModule);
}

void UnhookProtoEvents(void)
{
	if (hProtoHookList == NULL) return;
	for (int i = 0; i < ProtoHookCount; ++i)
		if (hProtoHookList[i] != NULL) UnhookEvent(hProtoHookList[i]);
	mir_free(hProtoHookList);
	hProtoHookList = NULL;
	ProtoHookCount = 0;
}

HANDLE CreateServiceFunctionEx(const char *name, MIRANDASERVICE serviceProc)
{
	ServiceCount++;
	hServiceList = (HANDLE *)mir_realloc(hServiceList, sizeof(HANDLE) * ServiceCount);
	return hServiceList[ServiceCount - 1] = CreateServiceFunction(name, serviceProc);
}

void DestroyServiceFunctionsEx(void)
{
	for (int i = 0; i < ServiceCount; ++i)
		if (hServiceList[i] != NULL) DestroyServiceFunction(hServiceList[i]);
	mir_free(hServiceList);
	ServiceCount = 0;
}

// Generate random number in a specified range
int GetRandom(int from, int to)
{
	if ((to - from) < 1) return from;
	unsigned randnum;
	CallService(MS_UTILS_GETRANDOM, sizeof(randnum), (LPARAM)&randnum);
	return ((randnum % (to - from + 1)) + from);
}

// From SRAway module
const TCHAR *GetDefaultMessage(int status)
{
	switch (status)
	{
		case ID_STATUS_AWAY: return TranslateT("I've been away since %time%.");
		case ID_STATUS_NA: return TranslateT("Give it up, I'm not in!");
		case ID_STATUS_OCCUPIED: return TranslateT("Not right now.");
		case ID_STATUS_DND: return TranslateT("Give a guy some peace, would ya?");
		case ID_STATUS_FREECHAT: return TranslateT("I'm a chatbot!");
		case ID_STATUS_ONLINE: return TranslateT("Yep, I'm here.");
		case ID_STATUS_OFFLINE: return TranslateT("Nope, not here.");
		case ID_STATUS_INVISIBLE: return TranslateT("I'm hiding from the mafia.");
		case ID_STATUS_ONTHEPHONE: return TranslateT("That'll be the phone.");
		case ID_STATUS_OUTTOLUNCH: return TranslateT("Mmm...food.");
		case ID_STATUS_IDLE: return TranslateT("idleeeeeeee");
	}
	return NULL;
}

const char *StatusModeToDbSetting(int status, const char *suffix)
{
	const char *prefix;
	static char str[64];

	switch (status) 
	{
		case ID_STATUS_AWAY:       prefix = "Away";	    break;
		case ID_STATUS_NA:         prefix = "Na";	    break;
		case ID_STATUS_DND:        prefix = "Dnd";      break;
		case ID_STATUS_OCCUPIED:   prefix = "Occupied"; break;
		case ID_STATUS_FREECHAT:   prefix = "FreeChat"; break;
		case ID_STATUS_ONLINE:     prefix = "On";       break;
		case ID_STATUS_OFFLINE:    prefix = "Off";      break;
		case ID_STATUS_INVISIBLE:  prefix = "Inv";      break;
		case ID_STATUS_ONTHEPHONE: prefix = "Otp";      break;
		case ID_STATUS_OUTTOLUNCH: prefix = "Otl";      break;
		case ID_STATUS_IDLE:       prefix = "Idl";      break;
		default: return NULL;
	}
	mir_snprintf(str, SIZEOF(str), "%s%s", prefix, suffix);
	return str;
}

int GetCurrentStatus(const char *szProto)
{
	if (szProto)
	{
		char szSetting[80];
		mir_snprintf(szSetting, SIZEOF(szSetting), "Cur%sStatus", szProto);
		return (int)DBGetContactSettingWord(NULL, "SimpleStatusMsg", szSetting, ID_STATUS_OFFLINE);

	}
	return CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
}

int GetStartupStatus(const char *szProto)
{
	if (szProto)
	{
		int  status_mode;
		char szSetting[80];

		mir_snprintf(szSetting, SIZEOF(szSetting), "Startup%sStatus", szProto);
		status_mode = DBGetContactSettingWord(NULL, "SimpleStatusMsg", szSetting, ID_STATUS_OFFLINE);
		if (status_mode == ID_STATUS_CURRENT)
		{
			// load status used for this proto last time
			mir_snprintf(szSetting, SIZEOF(szSetting), "Last%sStatus", szProto);
			status_mode = DBGetContactSettingWord(NULL, "SimpleStatusMsg", szSetting, ID_STATUS_OFFLINE);
		}
		return status_mode;
	}
	return (int)DBGetContactSettingWord(NULL, "SimpleStatusMsg", "StartupStatus", ID_STATUS_OFFLINE);
}
