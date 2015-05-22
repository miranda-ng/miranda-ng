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

#include "common.h"

PRESETITEM presetItems[] = 
{
	"account", LPGENT("Account"), LPGENT("Account:"), _T("%sys:account%"), 0, 0, 0,
	"birth", LPGENT("Birthday"), LPGENT("Birthday:"), _T("%birthday_date% (%birthday_age%) @ Next: %birthday_next%"), "birthdate", "birthage", "birthnext",
	"client", LPGENT("Client"), LPGENT("Client:"), _T("%raw:/MirVer%"), 0, 0, 0,
	"email", LPGENT("Email"), LPGENT("Email:"), _T("%raw:/e-mail%"), 0, 0, 0,
	"gender", LPGENT("Gender"), LPGENT("Gender:"), _T("%gender%"), 0, 0, 0,
	"homepage", LPGENT("Homepage"), LPGENT("Homepage:"), _T("%raw:/Homepage%"), 0, 0, 0,
	"id", LPGENT("Identifier"), _T("%sys:uidname|UID^!MetaContacts%:"), _T("%sys:uid%"), 0, 0, 0,
	"idle", LPGENT("Idle"), LPGENT("Idle:"), _T("%idle% (%idle_diff% ago)"), "idle", "idlediff", 0,
	"ip", _T("IP"), _T("IP:"), _T("%ip%"), "ip", 0, 0,
	"ipint", LPGENT("IP internal"), LPGENT("IP internal:"), _T("%ip_internal%"), "ipint", 0, 0,
	"lastmsg", LPGENT("Last message"), LPGENT("Last message: (%sys:last_msg_reltime% ago)"), _T("%sys:last_msg%"), 0, 0, 0,
	"listening", LPGENT("Listening to"), LPGENT("Listening to:"), _T("%raw:/ListeningTo%"), 0, 0, 0,
	"name", LPGENT("Name"), LPGENT("Name:"), _T("%raw:/FirstName|% %raw:/LastName%"), 0, 0, 0,
	"received", LPGENT("Number of received messages"), LPGENT("Number of msg [IN]:"), _T("%sys:msg_count_in%"), 0, 0, 0,
	"sended", LPGENT("Number of sent messages"), LPGENT("Number of msg [OUT]:"), _T("%sys:msg_count_out%"), 0, 0, 0,
	"status", LPGENT("Status"), LPGENT("Status:"), _T("%Status%"), "status", 0, 0,
	"statusmsg", LPGENT("Status message"), LPGENT("Status message:"), _T("%sys:status_msg%"), 0, 0, 0,
	"time", LPGENT("Contact time"), LPGENT("Time:"), _T("%sys:time%"), 0, 0, 0,
	"xtitle", LPGENT("xStatus title"), LPGENT("xStatus title:"), _T("%xsname%"), "xname", 0, 0,
	"xtext", LPGENT("xStatus text"), LPGENT("xStatus text:"), _T("%raw:/XStatusMsg%"), 0, 0, 0,
	"acttitle", LPGENT("[jabber.dll] Activity title"), LPGENT("Activity title:"), _T("%raw:AdvStatus/?dbsetting(%subject%,Protocol,p)/activity/title%"), 0, 0, 0,
	"acttext", LPGENT("[jabber.dll] Activity text"), LPGENT("Activity text:"), _T("%raw:AdvStatus/?dbsetting(%subject%,Protocol,p)/activity/text%"), 0, 0, 0,
	"lastseentime", LPGENT("[seenplugin.dll] Last seen time"), LPGENT("Last seen time:"), _T("%lastseen_date% @ %lastseen_time%"), "lsdate", "lstime", 0,
	"lastseenstatus", LPGENT("[seenplugin.dll] Last seen status"), LPGENT("Last seen status:"), _T("%lastseen_status% (%lastseen_ago% ago)"), "lsstatus", "lsago", 0,
	"cond", LPGENT("[weather.dll] Condition"), LPGENT("Condition:"), _T("%raw:Current/Condition%"), 0, 0, 0,
	"humidity", LPGENT("[weather.dll] Humidity"), LPGENT("Humidity:"), _T("%raw:Current/Humidity%"), 0, 0, 0,
	"minmaxtemp", LPGENT("[weather.dll] Max/Min temperature"), LPGENT("Max/Min:"), _T("%raw:Current/High%/%raw:Current/Low%"), 0, 0, 0,
	"moon", LPGENT("[weather.dll] Moon"), LPGENT("Moon:"), _T("%raw:Current/Moon%"), 0, 0, 0,
	"pressure", LPGENT("[weather.dll] Pressure"), LPGENT("Pressure:"), _T("%raw:Current/Pressure% (%raw:Current/Pressure Tendency%)"), 0, 0, 0,
	"sunrise", LPGENT("[weather.dll] Sunrise"), LPGENT("Sunrise:"), _T("%raw:Current/Sunrise%"), 0, 0, 0,
	"sunset", LPGENT("[weather.dll] Sunset"), LPGENT("Sunset:"), _T("%raw:Current/Sunset%"), 0, 0, 0,
	"temp", LPGENT("[weather.dll] Temperature"), LPGENT("Temperature:"), _T("%raw:Current/Temperature%"), 0, 0, 0,
	"uptime", LPGENT("[weather.dll] Update time"), LPGENT("Update time:"), _T("%raw:Current/Update%"), 0, 0, 0,
	"uvindex", LPGENT("[weather.dll] UV Index"), LPGENT("UV Index:"), _T("%raw:Current/UV% - %raw:Current/UVI%"), 0, 0, 0,
	"vis", LPGENT("[weather.dll] Visibility"), LPGENT("Visibility:"), _T("%raw:Current/Visibility%"), 0, 0, 0,
	"wind", LPGENT("[weather.dll] Wind"), LPGENT("Wind:"), _T("%raw:Current/Wind Direction% (%raw:Current/Wind Direction DEG%)/%raw:Current/Wind Speed%"), 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0	
};

PRESETSUBST presetSubsts[] = 
{
	"gender", _T("gender"), DVT_PROTODB, NULL, "Gender", 5,
	"status", _T("Status"), DVT_PROTODB, NULL, "Status", 1,
	"ip", _T("ip"), DVT_PROTODB, NULL, "IP", 7,
	"ipint", _T("ip_internal"), DVT_PROTODB, NULL, "RealIP", 7,
	"idle", _T("idle"), DVT_PROTODB, NULL, "IdleTS", 2,
	"idlediff", _T("idle_diff"), DVT_PROTODB, NULL, "IdleTS", 3,
	"xname", _T("xsname"), DVT_PROTODB, NULL, "XStatusName", 17,
	"lsdate", _T("lastseen_date"), DVT_DB, "SeenModule", NULL, 8,
	"lstime", _T("lastseen_time"), DVT_DB, "SeenModule", NULL, 10,
	"lsstatus", _T("lastseen_status"), DVT_DB, "SeenModule", "OldStatus", 1,
	"lsago", _T("lastseen_ago"), DVT_DB, "SeenModule", "seenTS", 3,
	"birthdate", _T("birthday_date"), DVT_PROTODB, NULL, "Birth", 8,
	"birthage", _T("birthday_age"), DVT_PROTODB, NULL, "Birth", 9,
	"birthnext", _T("birthday_next"), DVT_PROTODB, NULL, "Birth", 12,
	"logondate", _T("logon_date"), DVT_PROTODB, NULL, "LogonTS", 15, 
	"logontime", _T("logon_time"), DVT_PROTODB, NULL, "LogonTS", 13,
	"logonago", _T("logon_ago"), DVT_PROTODB, NULL, "LogonTS", 3,
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

