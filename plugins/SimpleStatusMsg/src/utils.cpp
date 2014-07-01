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

static LIST<void> arProtoHooks(5);

static IconItem iconList[] =
{
	{ LPGEN("Delete selected"),           "cross",   IDI_CROSS   },
	{ LPGEN("Recent message"),            "recent",  IDI_HISTORY },
	{ LPGEN("Predefined message"),        "predef",  IDI_MESSAGE },
	{ LPGEN("Add to predefined"),         "add",     IDI_PLUS    },
	{ LPGEN("Clear history"),             "clear",   IDI_CHIST   },
	{ LPGEN("Copy Away message"),         "copy",    IDI_COPY    },
	{ LPGEN("Change status message"),     "csmsg",   IDI_CSMSG,  },
	{ LPGEN("Go to URL in Away message"), "gotourl", IDI_GOTOURL }
};

void IconsInit(void)
{
	Icon_Register(g_hInst, LPGEN("Simple Status Message"), iconList, SIZEOF(iconList), "SimpleStatusMsg");
}

HICON LoadIconEx(const char* name)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, sizeof(szSettingName), "SimpleStatusMsg_%s", name);
	return Skin_GetIcon(szSettingName);
}

HANDLE GetIconHandle(int iconId)
{
	for(int i = 0; i < SIZEOF(iconList); i++)
		if (iconList[i].defIconID == iconId)
			return iconList[i].hIcolib;

	return NULL;
}

void ReleaseIconEx(const char* name)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, sizeof(szSettingName), "SimpleStatusMsg_%s",  name);
	Skin_ReleaseIcon(szSettingName);
}

HANDLE HookProtoEvent(const char *szModule, const char *szEvent, MIRANDAHOOKPARAM hookProc)
{
	char szProtoEvent[MAXMODULELABELLENGTH];
	mir_snprintf(szProtoEvent, sizeof(szProtoEvent), "%s%s", szModule, szEvent);
	HANDLE res = HookEventParam(szProtoEvent, hookProc, (LPARAM)szModule);
	arProtoHooks.insert(res);
	return res;
}

void UnhookProtoEvents(void)
{
	for (int i = 0; i < arProtoHooks.getCount(); ++i)
		UnhookEvent( arProtoHooks[i] );
	arProtoHooks.destroy();
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
		case ID_STATUS_OUTTOLUNCH: return TranslateT("Mmm... food.");
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
		return (int)db_get_w(NULL, "SimpleStatusMsg", szSetting, ID_STATUS_OFFLINE);

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
		status_mode = db_get_w(NULL, "SimpleStatusMsg", szSetting, ID_STATUS_OFFLINE);
		if (status_mode == ID_STATUS_CURRENT)
		{
			// load status used for this proto last time
			mir_snprintf(szSetting, SIZEOF(szSetting), "Last%sStatus", szProto);
			status_mode = db_get_w(NULL, "SimpleStatusMsg", szSetting, ID_STATUS_OFFLINE);
		}
		return status_mode;
	}
	return (int)db_get_w(NULL, "SimpleStatusMsg", "StartupStatus", ID_STATUS_OFFLINE);
}
