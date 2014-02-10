/*
	LogService - Plugin for Miranda IM
	Copyright (c) 2006-2008 Chervov Dmitry

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __M_LOGSERVICE_H
#define __M_LOGSERVICE_H

#define MIID_LOGSERVICE {0xe60bc9eb, 0xa099, 0x4846, {0xbc, 0x11, 0xba, 0x39, 0xf6, 0x60, 0x8b, 0x94}}
// {E60BC9EB-A099-4846-BC11-BA39F6608B94}


// LS_REGINFO::Flags constants
#define LSRF_WCHAR 1 // specifies that LS_REGINFO::szTitle, szDefLogPath and szDefFormat are WCHAR*
#ifdef _UNICODE
	#define LSRF_TCHAR LSRF_WCHAR
#else
	#define LSRF_TCHAR 0
#endif

typedef struct {
	int cbSize; // sizeof(LS_REGINFO)
	char *szID; // Log ID; it's a good idea to use your plugin name here
	union
	{
		char *szTitle; // Title shown in the options. Cannot be NULL or empty. This is translated by LogService automatically
		WCHAR *wszTitle;
		TCHAR *tszTitle;
	};
	union
	{
		char *szDefLogPath; // Default log file path, may contain variables. May be NULL - in this case the default path is "<log ID>.log". Usually it's relative to <Miranda profile dir>\Logs dir, but it can be changed by user through Folders plugin.
		WCHAR *wszDefLogPath; // if there's no Variables plugin installed, LogService will use szDefLogPath with all the variables removed from it
		TCHAR *tszDefLogPath;
	};
	union
	{
		char *szDefFormat; // Default log format; contains variables. May be NULL - in this case the default formatting is "`[`!cdate()-!ctime()`]`  %extratext%"
		WCHAR *wszDefFormat;
		TCHAR *tszDefFormat;
	};
	int Flags;
} LS_REGINFO;

// MS_LOGSERVICE_REGISTER
// Registers a log. Your plugin can register several different logs with different settings. This service must be called once for every needed log ID at every startup.
// wParam = (WPARAM)(LS_REGINFO*)pri - pointer to LS_REGINFO item
// lParam = 0
// returns 0 on success
#define MS_LOGSERVICE_REGISTER "LogService/Register"

__inline static INT_PTR logservice_register(char *szID, TCHAR *tszTitle, TCHAR *tszDefLogPath, TCHAR *tszDefFormat)
{
	LS_REGINFO ri;
	ZeroMemory(&ri, sizeof(LS_REGINFO));
	ri.cbSize = sizeof(LS_REGINFO);
	ri.szID = szID;
	ri.tszTitle = tszTitle;
	ri.tszDefLogPath = tszDefLogPath;
	ri.tszDefFormat = tszDefFormat;
	ri.Flags = LSRF_TCHAR;
	return CallService(MS_LOGSERVICE_REGISTER, (WPARAM)&ri, 0);
}



// LS_MSGINFO::Flags constants
#define LSMF_WCHAR 1 // specifies that LS_MSGINFO::szMsg is a WCHAR*
#ifdef _UNICODE
	#define LSMF_TCHAR LSMF_WCHAR
#else
	#define LSMF_TCHAR 0
#endif

typedef struct {
	int cbSize; // sizeof(LS_MSGINFO)
	char *szID;
	MCONTACT hContact; // may be NULL if no contact is associated with the message
	union
	{
		char *szMsg; // the message
		WCHAR *wszMsg;
		TCHAR *tszMsg;
	};
	int Flags;
} LS_MSGINFO;

// MS_LOGSERVICE_LOG
// Logs szMsg message. You don't have to specify in szMsg anything else than the actual message. i.e. LogService will take care of date, time, contact nick etc by itself, using the format string
// wParam = (WPARAM)(LS_MSGINFO*)pmi - pointer to LS_MSGINFO item
// lParam = 0
// returns 0 on success
#define MS_LOGSERVICE_LOG "LogService/Log"

__inline static INT_PTR logservice_log(char *szID, MCONTACT hContact, TCHAR *tszMsg)
{
	LS_MSGINFO mi;
	ZeroMemory(&mi, sizeof(LS_MSGINFO));
	mi.cbSize = sizeof(LS_MSGINFO);
	mi.szID = szID;
	mi.hContact = hContact;
	mi.tszMsg = tszMsg;
	mi.Flags = LSMF_TCHAR;
	return CallService(MS_LOGSERVICE_LOG, (WPARAM)&mi, 0);
}


// LS_LOGINFO::Flags constants
#define LSLI_WCHAR 1 // [in]; specifies that LS_LOGINFO::szLogPath is a WCHAR*
#ifdef _UNICODE
	#define LSLI_TCHAR LSLI_WCHAR
#else
	#define LSLI_TCHAR 0
#endif
#define LSLI_LOGENABLED 2 // [out]; LogService will set this flag if log with ID szID is enabled in the options. Setting this flag before calling MS_LOGSERVICE_GETLOGINFO is ignored. This flag is independent of hContact.

typedef struct {
	int cbSize; // [in]; sizeof(LS_LOGINFO)
	char *szID; // [in]
	MCONTACT hContact; // [in]; may be NULL
	union
	{
		char *szLogPath; // [in]; pointer to a string to receive log file name, including full path. May be NULL. The string must be at least MAX_PATH characters long
		WCHAR *wszLogPath;
		TCHAR *tszLogPath;
	};
	int Flags; // [in,out]
} LS_LOGINFO;

// MS_LOGSERVICE_GETLOGINFO
// Returns various information about log with ID szID.
// wParam = (WPARAM)(LS_LOGINFO*)pli - pointer to LS_LOGINFO item
// lParam = 0
// If szFileName is not NULL, MS_LOGSERVICE_GETLOGINFO gets full log file path by szID and hContact and copies it to *szLogPath
// Also the service will set LSLI_LOGENABLED flag if the specified log is enabled in the options.
// returns 0 on success
#define MS_LOGSERVICE_GETLOGINFO "LogService/GetLogInfo"


/*
1) Example of the simpliest way to use LogService:

	// define szID
		#define LOG_ID "MyPluginName"

	// in ME_SYSTEM_MODULESLOADED handler:
		logservice_register(LOG_ID, LPGENT("My plugin - log title"), NULL, NULL);

	// whenever you need to log something:
		logservice_log(LOG_ID, NULL, _T("Test message"));
	// (don't forget to specify hContact instead of NULL here if there's a contact associated with the message)

MyPluginName.log will be created with the following contents:
[20.08.2007-14:30:00]  Test message


2) If you want to offer additional customizability of log format using Variables, but still want the log to be usable even when Variables plugin is not installed, you can specify different messages depending on existence of MS_VARS_FORMATSTRING service. This example will explain how to do this and also will show you some other useful hints related to Variables plugin.

	// define szID
		#define LOG_ID "ClientChangeNotify"

	// For example, say, we want to append user's ICQ UIN (or Yahoo ID, etc) to file name, to log CCN events to different files, depending on protocol (i.e. ClientChangeNotify_310927.log for ICQ). That's why custom log file path with variables is used here:

		logservice_register(LOG_ID, LPGENT("ClientChangeNotify"),
			_T("ClientChangeNotify?puts(p,?dbsetting(%subject%,Protocol,p))?if2(_?dbsetting(,?get(p),?pinfo(?get(p),uidsetting)),).log"),
			TranslateT("`[`!cdate()-!ctime()`]`  ?cinfo(%subject%,display) (?cinfo(%subject%,id)) changed client to %extratext%"));

	// When Variables plugin is not installed, LogService will automatically cut all the variables from the log path, and we'll get usual "ClientChangeNotify.log" - so everyting is ok here.
	// But note that %extratext% in the log format is supposed to contain only client name in CCN, and without some special measures, we would get something like this in the log when Variables plugin is not installed:
	// [20.08.2007-14:30:00]  Miranda IM 0.7.0.33 alpha (ICQ v0.3.8.105 alpha)
	// Without at least contact nick, such log will be just useless. So when logging, we'll handle this case in a special way:

		if (ServiceExists(MS_VARS_FORMATSTRING))
		{ // Variables plugin is installed
			logservice_log(LOG_ID, hContact, tszClientName);
		} else
		{ // Variables plugin is not installed, so we have to generate the string by ourselves, using some simple predefined format:
			TCHAR tszNickAndClient[1024];
			mir_sntprintf(tszNickAndClient, SIZEOF(tszNickAndClient), TranslateT("%s changed his client to %s"),
				(TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR), tszClientName);
			logservice_log(LOG_ID, hContact, tszNickAndClient);
		}

3) The other solution to the case when there's no Variables plugin, is to sacrifice customizability of log format for a simplier implementation:

	// define szID
		#define LOG_ID "ClientChangeNotify"

	// in ME_SYSTEM_MODULESLOADED handler:
		logservice_register(LOG_ID, LPGENT("ClientChangeNotify"),
			_T("ClientChangeNotify?puts(p,?dbsetting(%subject%,Protocol,p))?if2(_?dbsetting(,?get(p),?pinfo(?get(p),uidsetting)),).log"),
			NULL);

	// logging:
		TCHAR tszNickAndClient[1024];
		mir_sntprintf(tszNickAndClient, SIZEOF(tszNickAndClient), TranslateT("%s changed his client to %s"),
			(TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR), tszClientName);
		logservice_log(LOG_ID, hContact, tszNickAndClient);

	// Note that %extratext% now always contains the whole "<contact> changed his client to <client>" string, and user is almost unable to customize this; perhaps only by using another translation or some advanced Variables scripts.
*/

#endif // __M_LOGSERVICE_H