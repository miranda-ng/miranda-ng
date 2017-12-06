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
	"account", LPGENW("Account"), LPGENW("Account:"), L"%sys:account%", nullptr, nullptr, nullptr,
	"birth", LPGENW("Birthday"), LPGENW("Birthday:"), L"%birthday_date% (%birthday_age%) @ Next: %birthday_next%", "birthdate", "birthage", "birthnext",
	"client", LPGENW("Client"), LPGENW("Client:"), L"%raw:/MirVer%", nullptr, nullptr, nullptr,
	"email", LPGENW("Email"), LPGENW("Email:"), L"%raw:/e-mail%", nullptr, nullptr, nullptr,
	"gender", LPGENW("Gender"), LPGENW("Gender:"), L"%gender%", nullptr, nullptr, nullptr,
	"homepage", LPGENW("Homepage"), LPGENW("Homepage:"), L"%raw:/Homepage%", nullptr, nullptr, nullptr,
	"id", LPGENW("Identifier"), L"%sys:uidname|UID^!MetaContacts%:", L"%sys:uid%", nullptr, nullptr, nullptr,
	"idle", LPGENW("Idle"), LPGENW("Idle:"), L"%idle% (%idle_diff% ago)", "idle", "idlediff", nullptr,
	"ip", L"IP", L"IP:", L"%ip%", "ip", nullptr, nullptr,
	"ipint", LPGENW("IP internal"), LPGENW("IP internal:"), L"%ip_internal%", "ipint", nullptr, nullptr,
	"lastmsg", LPGENW("Last message"), LPGENW("Last message: (%sys:last_msg_reltime% ago)"), L"%sys:last_msg%", nullptr, nullptr, nullptr,
	"listening", LPGENW("Listening to"), LPGENW("Listening to:"), L"%raw:/ListeningTo%", nullptr, nullptr, nullptr,
	"name", LPGENW("Name"), LPGENW("Name:"), L"%raw:/FirstName|% %raw:/LastName%", nullptr, nullptr, nullptr,
	"received", LPGENW("Number of received messages"), LPGENW("Number of msg [IN]:"), L"%sys:msg_count_in%", nullptr, nullptr, nullptr,
	"sended", LPGENW("Number of sent messages"), LPGENW("Number of msg [OUT]:"), L"%sys:msg_count_out%", nullptr, nullptr, nullptr,
	"status", LPGENW("Status"), LPGENW("Status:"), L"%Status%", "status", nullptr, nullptr,
	"statusmsg", LPGENW("Status message"), LPGENW("Status message:"), L"%sys:status_msg%", nullptr, nullptr, nullptr,
	"time", LPGENW("Contact time"), LPGENW("Time:"), L"%sys:time%", nullptr, nullptr, nullptr,
	"xtitle", LPGENW("xStatus title"), LPGENW("xStatus title:"), L"%xsname%", "xname", nullptr, nullptr,
	"xtext", LPGENW("xStatus text"), LPGENW("xStatus text:"), L"%raw:/XStatusMsg%", nullptr, nullptr, nullptr,
	"acttitle", LPGENW("[jabber.dll] Activity title"), LPGENW("Activity title:"), L"%raw:AdvStatus/?dbsetting(%subject%,Protocol,p)/activity/title%", nullptr, nullptr, nullptr,
	"acttext", LPGENW("[jabber.dll] Activity text"), LPGENW("Activity text:"), L"%raw:AdvStatus/?dbsetting(%subject%,Protocol,p)/activity/text%", nullptr, nullptr, nullptr,
	"lastseentime", LPGENW("[seenplugin.dll] Last seen time"), LPGENW("Last seen time:"), L"%lastseen_date% @ %lastseen_time%", "lsdate", "lstime", nullptr,
	"lastseenstatus", LPGENW("[seenplugin.dll] Last seen status"), LPGENW("Last seen status:"), L"%lastseen_status% (%lastseen_ago% ago)", "lsstatus", "lsago", nullptr,
	"cond", LPGENW("[weather.dll] Condition"), LPGENW("Condition:"), L"%raw:Current/Condition%", nullptr, nullptr, nullptr,
	"humidity", LPGENW("[weather.dll] Humidity"), LPGENW("Humidity:"), L"%raw:Current/Humidity%", nullptr, nullptr, nullptr,
	"minmaxtemp", LPGENW("[weather.dll] Max/Min temperature"), LPGENW("Max/Min:"), L"%raw:Current/High%/%raw:Current/Low%", nullptr, nullptr, nullptr,
	"moon", LPGENW("[weather.dll] Moon"), LPGENW("Moon:"), L"%raw:Current/Moon%", nullptr, nullptr, nullptr,
	"pressure", LPGENW("[weather.dll] Pressure"), LPGENW("Pressure:"), L"%raw:Current/Pressure% (%raw:Current/Pressure Tendency%)", nullptr, nullptr, nullptr,
	"sunrise", LPGENW("[weather.dll] Sunrise"), LPGENW("Sunrise:"), L"%raw:Current/Sunrise%", nullptr, nullptr, nullptr,
	"sunset", LPGENW("[weather.dll] Sunset"), LPGENW("Sunset:"), L"%raw:Current/Sunset%", nullptr, nullptr, nullptr,
	"temp", LPGENW("[weather.dll] Temperature"), LPGENW("Temperature:"), L"%raw:Current/Temperature%", nullptr, nullptr, nullptr,
	"uptime", LPGENW("[weather.dll] Update time"), LPGENW("Update time:"), L"%raw:Current/Update%", nullptr, nullptr, nullptr,
	"uvindex", LPGENW("[weather.dll] UV Index"), LPGENW("UV Index:"), L"%raw:Current/UV% - %raw:Current/UVI%", nullptr, nullptr, nullptr,
	"vis", LPGENW("[weather.dll] Visibility"), LPGENW("Visibility:"), L"%raw:Current/Visibility%", nullptr, nullptr, nullptr,
	"wind", LPGENW("[weather.dll] Wind"), LPGENW("Wind:"), L"%raw:Current/Wind Direction% (%raw:Current/Wind Direction DEG%)/%raw:Current/Wind Speed%", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr	
};

PRESETSUBST presetSubsts[] = 
{
	"gender", L"gender", DVT_PROTODB, nullptr, "Gender", 5,
	"status", L"Status", DVT_PROTODB, nullptr, "Status", 1,
	"ip", L"ip", DVT_PROTODB, nullptr, "IP", 7,
	"ipint", L"ip_internal", DVT_PROTODB, nullptr, "RealIP", 7,
	"idle", L"idle", DVT_PROTODB, nullptr, "IdleTS", 2,
	"idlediff", L"idle_diff", DVT_PROTODB, nullptr, "IdleTS", 3,
	"xname", L"xsname", DVT_PROTODB, nullptr, "XStatusName", 17,
	"lsdate", L"lastseen_date", DVT_DB, "SeenModule", nullptr, 8,
	"lstime", L"lastseen_time", DVT_DB, "SeenModule", nullptr, 10,
	"lsstatus", L"lastseen_status", DVT_DB, "SeenModule", "OldStatus", 1,
	"lsago", L"lastseen_ago", DVT_DB, "SeenModule", "seenTS", 3,
	"birthdate", L"birthday_date", DVT_PROTODB, nullptr, "Birth", 8,
	"birthage", L"birthday_age", DVT_PROTODB, nullptr, "Birth", 9,
	"birthnext", L"birthday_next", DVT_PROTODB, nullptr, "Birth", 12,
	"logondate", L"logon_date", DVT_PROTODB, nullptr, "LogonTS", 15, 
	"logontime", L"logon_time", DVT_PROTODB, nullptr, "LogonTS", 13,
	"logonago", L"logon_ago", DVT_PROTODB, nullptr, "LogonTS", 3,
	nullptr, nullptr, DVT_DB, nullptr, nullptr, 0
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
	nullptr,0
};

PRESETITEM *GetPresetItemByName(char *szName)
{
	for (int i = 0; presetItems[i].szID; i++)
	{
		if (mir_strcmp(presetItems[i].szID, szName) == 0)
			return &presetItems[i];
	}

	return nullptr;
}

PRESETSUBST *GetPresetSubstByName(char *szName)
{
	if (!szName) return nullptr;
	for (int i = 0; presetSubsts[i].szID; i++)
	{
		if (mir_strcmp(presetSubsts[i].szID, szName) == 0)
			return &presetSubsts[i];
	}

	return nullptr;
}

