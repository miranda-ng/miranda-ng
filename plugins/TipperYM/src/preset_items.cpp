/*
Copyright (C) 2006-2007 Scott Ellis
Copyright (C) 2007-2011 Jan Holub

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#include "stdafx.h"

PRESETITEM presetItems[] = 
{
	"account", LPGENT("Account"), LPGENT("Account:"), L"%sys:account%", 0, 0, 0,
	"birth", LPGENT("Birthday"), LPGENT("Birthday:"), L"%birthday_date% (%birthday_age%) @ Next: %birthday_next%", "birthdate", "birthage", "birthnext",
	"client", LPGENT("Client"), LPGENT("Client:"), L"%raw:/MirVer%", 0, 0, 0,
	"email", LPGENT("Email"), LPGENT("Email:"), L"%raw:/e-mail%", 0, 0, 0,
	"gender", LPGENT("Gender"), LPGENT("Gender:"), L"%gender%", 0, 0, 0,
	"homepage", LPGENT("Homepage"), LPGENT("Homepage:"), L"%raw:/Homepage%", 0, 0, 0,
	"id", LPGENT("Identifier"), L"%sys:uidname|UID^!MetaContacts%:", L"%sys:uid%", 0, 0, 0,
	"idle", LPGENT("Idle"), LPGENT("Idle:"), L"%idle% (%idle_diff% ago)", "idle", "idlediff", 0,
	"ip", L"IP", L"IP:", L"%ip%", "ip", 0, 0,
	"ipint", LPGENT("IP internal"), LPGENT("IP internal:"), L"%ip_internal%", "ipint", 0, 0,
	"lastmsg", LPGENT("Last message"), LPGENT("Last message: (%sys:last_msg_reltime% ago)"), L"%sys:last_msg%", 0, 0, 0,
	"listening", LPGENT("Listening to"), LPGENT("Listening to:"), L"%raw:/ListeningTo%", 0, 0, 0,
	"name", LPGENT("Name"), LPGENT("Name:"), L"%raw:/FirstName|% %raw:/LastName%", 0, 0, 0,
	"received", LPGENT("Number of received messages"), LPGENT("Number of msg [IN]:"), L"%sys:msg_count_in%", 0, 0, 0,
	"sended", LPGENT("Number of sent messages"), LPGENT("Number of msg [OUT]:"), L"%sys:msg_count_out%", 0, 0, 0,
	"status", LPGENT("Status"), LPGENT("Status:"), L"%Status%", "status", 0, 0,
	"statusmsg", LPGENT("Status message"), LPGENT("Status message:"), L"%sys:status_msg%", 0, 0, 0,
	"time", LPGENT("Contact time"), LPGENT("Time:"), L"%sys:time%", 0, 0, 0,
	"xtitle", LPGENT("xStatus title"), LPGENT("xStatus title:"), L"%xsname%", "xname", 0, 0,
	"xtext", LPGENT("xStatus text"), LPGENT("xStatus text:"), L"%raw:/XStatusMsg%", 0, 0, 0,
	"acttitle", LPGENT("[jabber.dll] Activity title"), LPGENT("Activity title:"), L"%raw:AdvStatus/?dbsetting(%subject%,Protocol,p)/activity/title%", 0, 0, 0,
	"acttext", LPGENT("[jabber.dll] Activity text"), LPGENT("Activity text:"), L"%raw:AdvStatus/?dbsetting(%subject%,Protocol,p)/activity/text%", 0, 0, 0,
	"lastseentime", LPGENT("[seenplugin.dll] Last seen time"), LPGENT("Last seen time:"), L"%lastseen_date% @ %lastseen_time%", "lsdate", "lstime", 0,
	"lastseenstatus", LPGENT("[seenplugin.dll] Last seen status"), LPGENT("Last seen status:"), L"%lastseen_status% (%lastseen_ago% ago)", "lsstatus", "lsago", 0,
	"cond", LPGENT("[weather.dll] Condition"), LPGENT("Condition:"), L"%raw:Current/Condition%", 0, 0, 0,
	"humidity", LPGENT("[weather.dll] Humidity"), LPGENT("Humidity:"), L"%raw:Current/Humidity%", 0, 0, 0,
	"minmaxtemp", LPGENT("[weather.dll] Max/Min temperature"), LPGENT("Max/Min:"), L"%raw:Current/High%/%raw:Current/Low%", 0, 0, 0,
	"moon", LPGENT("[weather.dll] Moon"), LPGENT("Moon:"), L"%raw:Current/Moon%", 0, 0, 0,
	"pressure", LPGENT("[weather.dll] Pressure"), LPGENT("Pressure:"), L"%raw:Current/Pressure% (%raw:Current/Pressure Tendency%)", 0, 0, 0,
	"sunrise", LPGENT("[weather.dll] Sunrise"), LPGENT("Sunrise:"), L"%raw:Current/Sunrise%", 0, 0, 0,
	"sunset", LPGENT("[weather.dll] Sunset"), LPGENT("Sunset:"), L"%raw:Current/Sunset%", 0, 0, 0,
	"temp", LPGENT("[weather.dll] Temperature"), LPGENT("Temperature:"), L"%raw:Current/Temperature%", 0, 0, 0,
	"uptime", LPGENT("[weather.dll] Update time"), LPGENT("Update time:"), L"%raw:Current/Update%", 0, 0, 0,
	"uvindex", LPGENT("[weather.dll] UV Index"), LPGENT("UV Index:"), L"%raw:Current/UV% - %raw:Current/UVI%", 0, 0, 0,
	"vis", LPGENT("[weather.dll] Visibility"), LPGENT("Visibility:"), L"%raw:Current/Visibility%", 0, 0, 0,
	"wind", LPGENT("[weather.dll] Wind"), LPGENT("Wind:"), L"%raw:Current/Wind Direction% (%raw:Current/Wind Direction DEG%)/%raw:Current/Wind Speed%", 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0	
};

PRESETSUBST presetSubsts[] = 
{
	"gender", L"gender", DVT_PROTODB, NULL, "Gender", 5,
	"status", L"Status", DVT_PROTODB, NULL, "Status", 1,
	"ip", L"ip", DVT_PROTODB, NULL, "IP", 7,
	"ipint", L"ip_internal", DVT_PROTODB, NULL, "RealIP", 7,
	"idle", L"idle", DVT_PROTODB, NULL, "IdleTS", 2,
	"idlediff", L"idle_diff", DVT_PROTODB, NULL, "IdleTS", 3,
	"xname", L"xsname", DVT_PROTODB, NULL, "XStatusName", 17,
	"lsdate", L"lastseen_date", DVT_DB, "SeenModule", NULL, 8,
	"lstime", L"lastseen_time", DVT_DB, "SeenModule", NULL, 10,
	"lsstatus", L"lastseen_status", DVT_DB, "SeenModule", "OldStatus", 1,
	"lsago", L"lastseen_ago", DVT_DB, "SeenModule", "seenTS", 3,
	"birthdate", L"birthday_date", DVT_PROTODB, NULL, "Birth", 8,
	"birthage", L"birthday_age", DVT_PROTODB, NULL, "Birth", 9,
	"birthnext", L"birthday_next", DVT_PROTODB, NULL, "Birth", 12,
	"logondate", L"logon_date", DVT_PROTODB, NULL, "LogonTS", 15, 
	"logontime", L"logon_time", DVT_PROTODB, NULL, "LogonTS", 13,
	"logonago", L"logon_ago", DVT_PROTODB, NULL, "LogonTS", 3,
	0, 0, DVT_DB, 0, 0, 0
};

DEFAULTITEM defaultItemList[] = 
{
	"statusmsg",	true,
	"-",				false,
	"lastmsg",		true ,
	"-",				false,
	"client",		false,
	"homepage",		false,
	"email",		false,
	"birth",		false,
	"name",			false,
	"-",				false,
	"time",			false,
	"id",			false,
	"status",		false,
	0,0
};

PRESETITEM *GetPresetItemByName(char *szName)
{
	for (int i = 0; presetItems[i].szID; i++)
	{
		if (mir_strcmp(presetItems[i].szID, szName) == 0)
			return &presetItems[i];
	}

	return NULL;
}

PRESETSUBST *GetPresetSubstByName(char *szName)
{
	if (!szName) return NULL;
	for (int i = 0; presetSubsts[i].szID; i++)
	{
		if (mir_strcmp(presetSubsts[i].szID, szName) == 0)
			return &presetSubsts[i];
	}

	return NULL;
}

