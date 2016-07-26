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
	"account", LPGENW("Account"), LPGENW("Account:"), L"%sys:account%", 0, 0, 0,
	"birth", LPGENW("Birthday"), LPGENW("Birthday:"), L"%birthday_date% (%birthday_age%) @ Next: %birthday_next%", "birthdate", "birthage", "birthnext",
	"client", LPGENW("Client"), LPGENW("Client:"), L"%raw:/MirVer%", 0, 0, 0,
	"email", LPGENW("Email"), LPGENW("Email:"), L"%raw:/e-mail%", 0, 0, 0,
	"gender", LPGENW("Gender"), LPGENW("Gender:"), L"%gender%", 0, 0, 0,
	"homepage", LPGENW("Homepage"), LPGENW("Homepage:"), L"%raw:/Homepage%", 0, 0, 0,
	"id", LPGENW("Identifier"), L"%sys:uidname|UID^!MetaContacts%:", L"%sys:uid%", 0, 0, 0,
	"idle", LPGENW("Idle"), LPGENW("Idle:"), L"%idle% (%idle_diff% ago)", "idle", "idlediff", 0,
	"ip", L"IP", L"IP:", L"%ip%", "ip", 0, 0,
	"ipint", LPGENW("IP internal"), LPGENW("IP internal:"), L"%ip_internal%", "ipint", 0, 0,
	"lastmsg", LPGENW("Last message"), LPGENW("Last message: (%sys:last_msg_reltime% ago)"), L"%sys:last_msg%", 0, 0, 0,
	"listening", LPGENW("Listening to"), LPGENW("Listening to:"), L"%raw:/ListeningTo%", 0, 0, 0,
	"name", LPGENW("Name"), LPGENW("Name:"), L"%raw:/FirstName|% %raw:/LastName%", 0, 0, 0,
	"received", LPGENW("Number of received messages"), LPGENW("Number of msg [IN]:"), L"%sys:msg_count_in%", 0, 0, 0,
	"sended", LPGENW("Number of sent messages"), LPGENW("Number of msg [OUT]:"), L"%sys:msg_count_out%", 0, 0, 0,
	"status", LPGENW("Status"), LPGENW("Status:"), L"%Status%", "status", 0, 0,
	"statusmsg", LPGENW("Status message"), LPGENW("Status message:"), L"%sys:status_msg%", 0, 0, 0,
	"time", LPGENW("Contact time"), LPGENW("Time:"), L"%sys:time%", 0, 0, 0,
	"xtitle", LPGENW("xStatus title"), LPGENW("xStatus title:"), L"%xsname%", "xname", 0, 0,
	"xtext", LPGENW("xStatus text"), LPGENW("xStatus text:"), L"%raw:/XStatusMsg%", 0, 0, 0,
	"acttitle", LPGENW("[jabber.dll] Activity title"), LPGENW("Activity title:"), L"%raw:AdvStatus/?dbsetting(%subject%,Protocol,p)/activity/title%", 0, 0, 0,
	"acttext", LPGENW("[jabber.dll] Activity text"), LPGENW("Activity text:"), L"%raw:AdvStatus/?dbsetting(%subject%,Protocol,p)/activity/text%", 0, 0, 0,
	"lastseentime", LPGENW("[seenplugin.dll] Last seen time"), LPGENW("Last seen time:"), L"%lastseen_date% @ %lastseen_time%", "lsdate", "lstime", 0,
	"lastseenstatus", LPGENW("[seenplugin.dll] Last seen status"), LPGENW("Last seen status:"), L"%lastseen_status% (%lastseen_ago% ago)", "lsstatus", "lsago", 0,
	"cond", LPGENW("[weather.dll] Condition"), LPGENW("Condition:"), L"%raw:Current/Condition%", 0, 0, 0,
	"humidity", LPGENW("[weather.dll] Humidity"), LPGENW("Humidity:"), L"%raw:Current/Humidity%", 0, 0, 0,
	"minmaxtemp", LPGENW("[weather.dll] Max/Min temperature"), LPGENW("Max/Min:"), L"%raw:Current/High%/%raw:Current/Low%", 0, 0, 0,
	"moon", LPGENW("[weather.dll] Moon"), LPGENW("Moon:"), L"%raw:Current/Moon%", 0, 0, 0,
	"pressure", LPGENW("[weather.dll] Pressure"), LPGENW("Pressure:"), L"%raw:Current/Pressure% (%raw:Current/Pressure Tendency%)", 0, 0, 0,
	"sunrise", LPGENW("[weather.dll] Sunrise"), LPGENW("Sunrise:"), L"%raw:Current/Sunrise%", 0, 0, 0,
	"sunset", LPGENW("[weather.dll] Sunset"), LPGENW("Sunset:"), L"%raw:Current/Sunset%", 0, 0, 0,
	"temp", LPGENW("[weather.dll] Temperature"), LPGENW("Temperature:"), L"%raw:Current/Temperature%", 0, 0, 0,
	"uptime", LPGENW("[weather.dll] Update time"), LPGENW("Update time:"), L"%raw:Current/Update%", 0, 0, 0,
	"uvindex", LPGENW("[weather.dll] UV Index"), LPGENW("UV Index:"), L"%raw:Current/UV% - %raw:Current/UVI%", 0, 0, 0,
	"vis", LPGENW("[weather.dll] Visibility"), LPGENW("Visibility:"), L"%raw:Current/Visibility%", 0, 0, 0,
	"wind", LPGENW("[weather.dll] Wind"), LPGENW("Wind:"), L"%raw:Current/Wind Direction% (%raw:Current/Wind Direction DEG%)/%raw:Current/Wind Speed%", 0, 0, 0,
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

