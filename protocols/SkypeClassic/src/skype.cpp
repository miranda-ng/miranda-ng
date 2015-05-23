/*

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


*/

#include "skype.h"
#include "debug.h"
#include "skypeapi.h"
#include "skypesvc.h"
#include "contacts.h"
#include "utf8.h"
#include "pthread.h"
#include "gchat.h"
#include "m_toptoolbar.h"
#include "msglist.h"
#include "memlist.h"
#include "filexfer.h"
#include <sys/timeb.h>
#ifdef _WIN64
#if (_MSC_VER < 1500)
#pragma comment (lib, "bufferoverflowU.lib")
#endif
#endif


POPUPDATAT MessagePopup;

// Exported Globals
HWND hSkypeWnd = NULL, g_hWnd = NULL, hSkypeWndSecondary = NULL, hForbiddenSkypeWnd = NULL;
HANDLE SkypeReady, SkypeMsgReceived, hInitChat = NULL, httbButton = NULL, FetchMessageEvent = NULL;
BOOL SkypeInitialized = FALSE, MirandaShuttingDown = FALSE, PopupServiceExists = FALSE;
BOOL UseSockets = FALSE, bSkypeOut = FALSE, bProtocolSet = FALSE, bIsImoproxy = FALSE, bHasFileXfer = FALSE;
char skype_path[MAX_PATH], protocol = 2, *pszProxyCallout = NULL, g_szProtoName[_MAX_FNAME] = "SkypeClassic";
int SkypeStatus = ID_STATUS_OFFLINE, hSearchThread = -1, receivers = 1;
long sendwatchers = 0, rcvwatchers = 0;
UINT ControlAPIAttach, ControlAPIDiscover;
LONG AttachStatus = -1;
HINSTANCE hInst;
HANDLE hProtocolAvatarsFolder;
char DefaultAvatarsFolder[MAX_PATH + 1];
int hLangpack = 0;

CRITICAL_SECTION RingAndEndcallMutex, QueryThreadMutex, TimeMutex;

// Module Internal Globals
HANDLE MessagePumpReady;
HANDLE hBuddyAdded = NULL;
HANDLE hMenuAddSkypeContact = NULL;

DWORD msgPumpThreadId = 0;
#ifdef SKYPEBUG_OFFLN
HANDLE GotUserstatus;
#endif

BOOL bModulesLoaded = FALSE;
char *RequestedStatus = NULL;	// To fix Skype-API Statusmode-bug
char cmdMessage[12] = "MESSAGE", cmdPartner[8] = "PARTNER";	// Compatibility commands



// Direct assignment of user properties to a DB-Setting
struct settings_map {
	char *SkypeSetting;
	char *MirandaSetting;
} m_settings[] = {
		{ "LANGUAGE", "Language1" },
		{ "PROVINCE", "State" },
		{ "CITY", "City" },
		{ "PHONE_HOME", "Phone" },
		{ "PHONE_OFFICE", "CompanyPhone" },
		{ "PHONE_MOBILE", "Cellular" },
		{ "HOMEPAGE", "Homepage" },
		{ "ABOUT", "About" }
};

BOOL(WINAPI *MyEnableThemeDialogTexture)(HANDLE, DWORD) = 0;

HMODULE hUxTheme = 0;

// function pointers, use typedefs for casting to shut up the compiler when using GetProcAddress()

typedef BOOL(WINAPI *PITA)();
typedef HANDLE(WINAPI *POTD)(HWND, LPCWSTR);
typedef UINT(WINAPI *PDTB)(HANDLE, HDC, int, int, RECT *, RECT *);
typedef UINT(WINAPI *PCTD)(HANDLE);
typedef UINT(WINAPI *PDTT)(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, DWORD, RECT *);

PITA pfnIsThemeActive = 0;
POTD pfnOpenThemeData = 0;
PDTB pfnDrawThemeBackground = 0;
PCTD pfnCloseThemeData = 0;
PDTT pfnDrawThemeText = 0;

#define FIXED_TAB_SIZE 100                  // default value for fixed width tabs

typedef struct {
	char msgnum[16];
	BOOL getstatus;
	BOOL bIsRead;
	BOOL bDontMarkSeen;
	BOOL QueryMsgDirection;
	BOOL bUseTimestamp;
	TYP_MSGLENTRY *pMsgEntry;
} fetchmsg_arg;

typedef struct {
	MCONTACT hContact;
	char szId[16];
} msgsendwt_arg;

#ifdef USE_REAL_TS
typedef struct {
	MCONTACT hContact;
	time_t timestamp;
} arg_dbaddevent;
arg_dbaddevent m_AddEventArg = {0};
CRITICAL_SECTION AddEventMutex;
#endif

/*
 * visual styles support (XP+)
 * returns 0 on failure
 */

int InitVSApi()
{
	if ((hUxTheme = LoadLibraryA("uxtheme.dll")) == 0)
		return 0;

	pfnIsThemeActive = (PITA)GetProcAddress(hUxTheme, "IsThemeActive");
	pfnOpenThemeData = (POTD)GetProcAddress(hUxTheme, "OpenThemeData");
	pfnDrawThemeBackground = (PDTB)GetProcAddress(hUxTheme, "DrawThemeBackground");
	pfnCloseThemeData = (PCTD)GetProcAddress(hUxTheme, "CloseThemeData");
	pfnDrawThemeText = (PDTT)GetProcAddress(hUxTheme, "DrawThemeText");

	MyEnableThemeDialogTexture = (BOOL(WINAPI *)(HANDLE, DWORD))GetProcAddress(hUxTheme, "EnableThemeDialogTexture");
	if (pfnIsThemeActive != 0 && pfnOpenThemeData != 0 && pfnDrawThemeBackground != 0 && pfnCloseThemeData != 0 && pfnDrawThemeText != 0) {
		return 1;
	}
	return 0;
}

/*
 * unload uxtheme.dll
 */

int FreeVSApi()
{
	if (hUxTheme != 0)
		FreeLibrary(hUxTheme);
	return 0;
}

// Plugin Info
PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {A71F8335-7B87-4432-B8A3-81479431C6F5}
	{ 0xa71f8335, 0x7b87, 0x4432, { 0xb8, 0xa3, 0x81, 0x47, 0x94, 0x31, 0xc6, 0xf5 } }
};

#define MAPDND	1	// Map Occupied to DND status and say that you support it
//#define MAPNA   1 // Map NA status to Away and say that you support it

/*                           P R O G R A M                                */


/*
 * ShowMessage
 *
 * Shows a popup, if the popup plugin is enabled.
 * mustShow: 1 -> If Popup-Plugin is not available/disabled, show Message
 *                in a Messagewindow
 *                If the Popup-Plugin is enabled, let the message stay on
 *                screen until someone clicks it away.
 *           0 -> If Popup-Plugin is not available/disabled, skip message
 * Returns 0 on success, -1 on failure
 *
 */
int ShowMessage(int iconID, TCHAR *lpzText, int mustShow) {
	if (db_get_b(NULL, SKYPE_PROTONAME, "SuppressErrors", 0)) return -1;

	if (bModulesLoaded && PopupServiceExists && ServiceExists(MS_POPUP_ADDPOPUPT) && db_get_b(NULL, SKYPE_PROTONAME, "UsePopup", 0) && !MirandaShuttingDown) {
		BOOL showPopup, popupWindowColor;
		unsigned int popupBackColor, popupTextColor;
		int popupTimeSec;

		popupTimeSec = db_get_dw(NULL, SKYPE_PROTONAME, "popupTimeSecErr", 4);
		popupTextColor = db_get_dw(NULL, SKYPE_PROTONAME, "popupTextColorErr", GetSysColor(COLOR_WINDOWTEXT));
		popupBackColor = db_get_dw(NULL, SKYPE_PROTONAME, "popupBackColorErr", GetSysColor(COLOR_BTNFACE));
		popupWindowColor = (0 != db_get_b(NULL, SKYPE_PROTONAME, "popupWindowColorErr", TRUE));
		showPopup = (0 != db_get_b(NULL, SKYPE_PROTONAME, "showPopupErr", TRUE));

		MessagePopup.lchContact = NULL;
		MessagePopup.lchIcon = LoadIcon(hInst, MAKEINTRESOURCE(iconID));
		MessagePopup.colorBack = !popupWindowColor ? popupBackColor : GetSysColor(COLOR_BTNFACE);
		MessagePopup.colorText = !popupWindowColor ? popupTextColor : GetSysColor(COLOR_WINDOWTEXT);
		MessagePopup.iSeconds = popupTimeSec;
		MessagePopup.PluginData = (void *)1;

		mir_tstrcpy(MessagePopup.lptzText, lpzText);

#ifdef _UNICODE
		mbstowcs(MessagePopup.lptzContactName, SKYPE_PROTONAME, mir_strlen(SKYPE_PROTONAME) + 1);
#else
		mir_tstrcpy(MessagePopup.lptzContactName, SKYPE_PROTONAME);
#endif

		if (showPopup)
			CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&MessagePopup, 0);

		return 0;
	}
	else {

		if (mustShow == 1) MessageBox(NULL, lpzText, _T("Skype Protocol"), MB_OK | MB_ICONWARNING);
		return 0;
	}
}

int ShowMessageA(int iconID, char *lpzText, int mustShow) {
	WCHAR *lpwText;
	int iRet;
	size_t len = mbstowcs(NULL, lpzText, mir_strlen(lpzText));
	if (len == -1 || !(lpwText = (WCHAR*)calloc(len + 1, sizeof(WCHAR)))) return -1;
	mbstowcs(lpwText, lpzText, mir_strlen(lpzText));
	iRet = ShowMessage(iconID, lpwText, mustShow);
	free(lpwText);
	return iRet;
}

// processing Hooks

int HookContactAdded(WPARAM wParam, LPARAM) {
	MCONTACT hContact = (MCONTACT) wParam;
	char *szProto = GetContactProto(hContact);
	if (szProto != NULL && !mir_strcmp(szProto, SKYPE_PROTONAME))
		add_contextmenu(hContact);
	return 0;
}

int HookContactDeleted(WPARAM wParam, LPARAM) {
	MCONTACT hContact = (MCONTACT) wParam;
	char *szProto = GetContactProto(hContact);
	if (szProto != NULL && !mir_strcmp(szProto, SKYPE_PROTONAME)) {
		DBVARIANT dbv;
		if (db_get_s(hContact, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) return 1;
		int retval = SkypeSend("SET USER %s BUDDYSTATUS 1", dbv.pszVal);
		db_free(&dbv);
		if (retval) return 1;
	}
	return 0;
}

void GetInfoThread(void *arg)
{
	MCONTACT hContact = (MCONTACT)arg;
	DBVARIANT dbv;
	int i;
	char *ptr;
	BOOL bSetNick = FALSE;
	// All properties are already handled in the WndProc, so we just consume the 
	// messages here to do proper ERROR handling
	// If you add something here, be sure to handle it in WndProc, but let it
	// fall through there so that message gets added to the queue in order to be
	// consumed by SkypeGet
	char *pszProps[] = {
		"BIRTHDAY", "COUNTRY", "SEX", "MOOD_TEXT", "TIMEZONE", "IS_VIDEO_CAPABLE" };

	LOG(("GetInfoThread started."));
	EnterCriticalSection(&QueryThreadMutex);
	if (db_get_s(hContact, SKYPE_PROTONAME, SKYPE_NAME, &dbv))
	{
		LOG(("GetInfoThread terminated, cannot find Skype Name for contact %08X.", hContact));
		LeaveCriticalSection(&QueryThreadMutex);
		return;
	}

	if (ptr = SkypeGet("USER", dbv.pszVal, "DISPLAYNAME")) {
		// WndProc sets Nick accordingly
		if (*ptr) bSetNick = TRUE;
		free(ptr);
	}

	if (ptr = SkypeGet("USER", dbv.pszVal, "FULLNAME")) {
		if (*ptr && !bSetNick && db_get_b(NULL, SKYPE_PROTONAME, "ShowFullname", 1)) {
			// No Displayname and FULLNAME requested
			db_set_utf(hContact, SKYPE_PROTONAME, "Nick", ptr);
			bSetNick = TRUE;
		}
		free(ptr);
	}

	if (!bSetNick) {
		// Still no nick set, so use SKYPE Nickname
		db_set_s(hContact, SKYPE_PROTONAME, "Nick", dbv.pszVal);
	}


	if (!bIsImoproxy)
	{
		for (i = 0; i < sizeof(pszProps) / sizeof(pszProps[0]); i++)
			if (ptr = SkypeGet("USER", dbv.pszVal, pszProps[i])) free(ptr);
	}
	else {
		if (ptr = SkypeGet("USER", dbv.pszVal, "MOOD_TEXT")) free(ptr);
	}

	if (protocol >= 7 || bIsImoproxy) {
		// Notify about the possibility of an avatar
		ACKDATA ack = { 0 };
		ack.cbSize = sizeof(ACKDATA);
		ack.szModule = SKYPE_PROTONAME;
		ack.hContact = hContact;
		ack.type = ACKTYPE_AVATAR;
		ack.result = ACKRESULT_STATUS;

		CallService(MS_PROTO_BROADCASTACK, 0, (LPARAM)&ack);
		//if (ptr=SkypeGet ("USER", dbv.pszVal, "RICH_MOOD_TEXT")) free (ptr);
	}

	if (!bIsImoproxy)
	{
		for (i = 0; i < sizeof(m_settings) / sizeof(m_settings[0]); i++)
			if (ptr = SkypeGet("USER", dbv.pszVal, m_settings[i].SkypeSetting)) free(ptr);
	}

	ProtoBroadcastAck(SKYPE_PROTONAME, hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE)1, 0);
	LeaveCriticalSection(&QueryThreadMutex);
	db_free(&dbv);
	LOG(("GetInfoThread terminated gracefully."));
}

time_t SkypeTime(time_t *timer)
{
	struct _timeb tb;

	EnterCriticalSection(&TimeMutex);
	_ftime(&tb);
	if (timer) *timer = tb.time;
	LeaveCriticalSection(&TimeMutex);
	return tb.time;
}


void BasicSearchThread(char *nick) {
	PROTOSEARCHRESULT psr = { 0 };
	char *cmd = NULL, *token = NULL, *ptr = NULL;
	time_t st;

	LOG(("BasicSearchThread started."));
	EnterCriticalSection(&QueryThreadMutex);
	SkypeTime(&st);
	if (SkypeSend("SEARCH USERS %s", nick) == 0 && (cmd = SkypeRcvTime("USERS", st, INFINITE))) {
		if (strncmp(cmd, "ERROR", 5)) {
			psr.cbSize = sizeof(psr);
			char *nextoken = 0;
			for (token = strtok_r(cmd + 5, ", ", &nextoken); token; token = strtok_r(NULL, ", ", &nextoken)) {
				psr.nick = psr.id = _A2T(token);
				psr.lastName = NULL;
				psr.firstName = NULL;
				psr.email = NULL;
				if (ptr = SkypeGet("USER", token, "FULLNAME")) {
					// We cannot use strtok() to seperate first & last name here,
					// because we already use it for parsing the user list
					// So we use our own function
					if (psr.lastName = _A2T(strchr(ptr, ' '))) {
						*psr.lastName = 0;
						psr.lastName++;
						LOG(("BasicSearchThread: lastName=%s", psr.lastName));
					}
					psr.firstName = _A2T(ptr);
					LOG(("BasicSearchThread: firstName=%s", psr.firstName));
				}
				ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)hSearchThread, (LPARAM)(PROTOSEARCHRESULT*)&psr);
				if (ptr) free(ptr);
			}
		}
		else {
			OUT(cmd);
		}
		free(cmd);
	}
	ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)hSearchThread, 0);
	free(nick);
	LeaveCriticalSection(&QueryThreadMutex);
	LOG(("BasicSearchThread terminated gracefully."));
	return;
}

// added by TioDuke
void GetDisplaynameThread(char *) {
	DBVARIANT dbv;
	char *ptr;

	LOG(("GetDisplaynameThread started."));
	if (db_get_s(NULL, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) {
		LOG(("GetDisplaynameThread terminated."));
		return;
	}
	EnterCriticalSection(&QueryThreadMutex);
	if ((ptr = SkypeGet("USER", dbv.pszVal, "FULLNAME"))) {
		if (*ptr) db_set_utf(NULL, SKYPE_PROTONAME, "Nick", ptr);
		free(ptr);
	}
	db_free(&dbv);
	LeaveCriticalSection(&QueryThreadMutex);
	LOG(("GetDisplaynameThread terminated gracefully."));
}


// Starts importing history from Skype
INT_PTR ImportHistory(WPARAM wParam, LPARAM) {
	MCONTACT hContact = (MCONTACT)wParam;
	DBVARIANT dbv;
	if (db_get_b(hContact, SKYPE_PROTONAME, "ChatRoom", 0)) {
		if (db_get_s(hContact, SKYPE_PROTONAME, "ChatRoomID", &dbv))
			return 0;
		SkypeSend("GET CHAT %s CHATMESSAGES", dbv.pszVal);
	}
	else {
		if (db_get_s(hContact, SKYPE_PROTONAME, SKYPE_NAME, &dbv))
			return 0;
		SkypeSend("SEARCH %sS %s", cmdMessage, dbv.pszVal);
	}
	db_free(&dbv);
	return 0;
}

int SearchFriends(void) {
	char *ptr, *token, *pStat;
	int iRet = 0;
	time_t st;

	SkypeTime(&st);
	if (SkypeSend("SEARCH FRIENDS") != -1 && (ptr = SkypeRcvTime("USERS", st, INFINITE)))
	{
		if (strncmp(ptr, "ERROR", 5)) {
			if (ptr[5]) {
				char *nextoken = 0;
				for (token = strtok_r(ptr + 5, ", ", &nextoken); token; token = strtok_r(NULL, ", ", &nextoken)) {
					if (!(pStat = SkypeGet("USER", token, "ONLINESTATUS")))
					{
						iRet = -1;
						break;
					}
					free(pStat);
				}
			}
		}
		else iRet = -1;
		free(ptr);
	}
	else iRet = -1;
	return iRet;
}

static void QueryUserWaitingAuthorization(char *pszNick, char *pszAuthRq)
{
	MCONTACT hContact = add_contact(pszNick, PALF_TEMPORARY);
	char *lastname = NULL, *pCurBlob, *authmsg = NULL;

	LOG(("Awaiting auth: %s", pszNick));
	PROTORECVEVENT pre = { 0 };
	pre.flags = 0;
	pre.timestamp = (DWORD)SkypeTime(NULL);

	CCSDATA ccs = { 0 };
	ccs.szProtoService = PSR_AUTH;
	ccs.hContact = hContact;
	ccs.wParam = 0;
	ccs.lParam = (LPARAM)&pre;

	/* blob is: */
	//DWORD protocolSpecific MCONTACT hContact
	//ASCIIZ nick, firstName, lastName, e-mail, requestReason
	char *firstname = SkypeGet("USER", pszNick, "FULLNAME");
	if (firstname) {
		if (lastname = strchr(firstname, ' ')) {
			*lastname = 0;
			lastname++;
		}
	}

	pre.lParam = sizeof(DWORD)+sizeof(HANDLE)+mir_strlen(pszNick) + 5;
	if (firstname) pre.lParam += mir_strlen(firstname);
	if (lastname) pre.lParam += mir_strlen(lastname);
	if (pszAuthRq) authmsg = strdup(pszAuthRq);
	if (authmsg || ((protocol >= 4 || bIsImoproxy) && (authmsg = SkypeGetID("USER", pszNick, "RECEIVEDAUTHREQUEST"))))
		pre.lParam += mir_strlen(authmsg);
	if (pre.szMessage = pCurBlob = (char *)calloc(1, pre.lParam)) {
		pCurBlob += sizeof(DWORD); // Not used
		memcpy(pCurBlob, &hContact, sizeof(HANDLE));	pCurBlob += sizeof(HANDLE);

		sprintf(pCurBlob, "%s%c%s%c%s%c%c%s", pszNick, 0, firstname ? firstname : "", 0, lastname ? lastname : "", 0, 0, authmsg ? authmsg : "");

		CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
		free(pre.szMessage);
	}
	if (firstname) free(firstname);
	if (authmsg) free(authmsg);
	return;
}

void __cdecl ProcessAuthRq(void *pPmsg) {
	char *nick, *auth;

	strtok((char*)pPmsg, " ");
	nick = strtok(NULL, " ");
	strtok(NULL, " ");
	auth = strtok(NULL, "");
	QueryUserWaitingAuthorization(nick, auth);
	free(pPmsg);
}

void __cdecl SearchUsersWaitingMyAuthorization() {
	if (SkypeSend("#UWA SEARCH USERSWAITINGMYAUTHORIZATION"))
		return;

	char *cmd = SkypeRcv("#UWA USERS", INFINITE);
	if (!cmd)
		return;
	if (!strncmp(cmd, "ERROR", 5)) {
		free(cmd);
		return;
	}

	char *nextoken = 0, *token = strtok_r(cmd + 10, ", ", &nextoken);
	while (token) {
		QueryUserWaitingAuthorization(token, NULL);
		token = strtok_r(NULL, ", ", &nextoken);
	}
	free(cmd);
	return;
}

void SearchFriendsThread(char *) {
	if (!SkypeInitialized) return;
	LOG(("SearchFriendsThread started."));
	EnterCriticalSection(&QueryThreadMutex);
	SkypeInitialized = FALSE;
	SearchFriends();
	SkypeInitialized = TRUE;
	LeaveCriticalSection(&QueryThreadMutex);
	LOG(("SearchFriendsThread terminated gracefully."));
}

void __cdecl SearchRecentChats(void *) {
	if (SkypeSend("#RCH SEARCH RECENTCHATS"))
		return;
	char *cmd = SkypeRcv("#RCH CHATS", INFINITE);
	if (!cmd)
		return;
	if (!strncmp(cmd, "ERROR", 5)) {
		free(cmd);
		return;
	}

	char *token, *nextoken = 0;
	for (token = strtok_r(cmd + 10, ", ", &nextoken); token; token = strtok_r(NULL, ", ", &nextoken)) {
		char *pszStatus = SkypeGet("CHAT", token, "STATUS");

		if (pszStatus) {
			if (!mir_strcmp(pszStatus, "MULTI_SUBSCRIBED")) {
				// Add chatrooms for active multisubscribed chats
				/*if (!find_chatA(token)) */
				EnterCriticalSection(&QueryThreadMutex);
				ChatStart(token, TRUE);
				char *pszTopic = SkypeGet("CHAT", token, "TOPIC");
				if (pszTopic) {
					if (!*pszTopic) {
						free(pszTopic);
						pszTopic = SkypeGet("CHAT", token, "FRIENDLYNAME");
					}
					TCHAR *psztChatName = make_nonutf_tchar_string((const unsigned char*)token);
					TCHAR *psztTopic = make_tchar_string((const unsigned char*)pszTopic);
					SetChatTopic(psztChatName, psztTopic, FALSE);
					free_nonutf_tchar_string(psztChatName);
					free(psztTopic);
					free(pszTopic);
				}
				LeaveCriticalSection(&QueryThreadMutex);
			}
			free(pszStatus);
		}
	}
	free(cmd);
	return;
}


void __cdecl SkypeSystemInit(char *dummy) {
	static BOOL Initializing = FALSE;
	LOG(("SkypeSystemInit thread started."));
	if (SkypeInitialized || Initializing)
	{
		LOG(("SkypeSystemInit terminated, nothing to do."));
		return;
	}
	Initializing = TRUE;
	// Do initial Skype-Tasks
	logoff_contacts(FALSE);
	// Add friends

	// Clear currentuserhandle entries from queue
	while (testfor("CURRENTUSERHANDLE", 0));
	if (SkypeSend(SKYPE_PROTO) == -1 || !testfor("PROTOCOL", INFINITE) ||
		SkypeSend("GET CURRENTUSERHANDLE") == -1 ||
		SkypeSend("GET PRIVILEGE SKYPEOUT") == -1) {
		Initializing = FALSE;
		LOG(("SkypeSystemInit thread stopped with failure."));
		return;
	}

	DBVARIANT dbv = { 0 };
	if (db_get_s(NULL, SKYPE_PROTONAME, "LoginUserName", &dbv) == 0)
	{
		if (*dbv.pszVal)
		{
			char *pszUser;

			// Username is set in Plugin, therefore we need to match it
			// against CURRENTUSERHANDLE
			if (pszUser = SkypeRcv("CURRENTUSERHANDLE", INFINITE))
			{
				memmove(pszUser, pszUser + 18, mir_strlen(pszUser + 17));
				if (_stricmp(dbv.pszVal, pszUser))
				{
					// Doesn't match, maybe we have a second Skype instance we have to take
					// care of? If in doubt, let's wait a while for it to report its hWnd to us.
					LOG(("Userhandle %s doesn't match username %s from settings", pszUser, dbv.pszVal));
					if (!hSkypeWndSecondary) Sleep(3000);
					if (hSkypeWndSecondary)
					{
						hSkypeWnd = hSkypeWndSecondary;
						hSkypeWndSecondary = NULL;
						Initializing = FALSE;
						while (testfor("CURRENTUSERHANDLE", 0));
						LOG(("Trying to init secondary Skype instance"));
						SkypeSystemInit(dummy);
					}
					else
					{
						hForbiddenSkypeWnd = hSkypeWnd;

						// If we need to start Skype as secondary instance, we should do it now
						if (db_get_b(NULL, SKYPE_PROTONAME, "StartSkype", 1) &&
							db_get_b(NULL, SKYPE_PROTONAME, "secondary", 0))
						{
							int oldstatus;

							hSkypeWnd = NULL;
							AttachStatus = -1;
							if (g_hWnd) KillTimer(g_hWnd, 1);
							oldstatus = SkypeStatus;
							InterlockedExchange((long *)&SkypeStatus, ID_STATUS_CONNECTING);
							ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldstatus, SkypeStatus);
							ConnectToSkypeAPI(skype_path, 1);
						}
						if (hForbiddenSkypeWnd == hSkypeWnd && !hSkypeWndSecondary)
						{
							int oldstatus;
							char szError[256];
							sprintf(szError,
								Translate("Username '%s' provided by Skype API doesn't match username '%s' in your settings. Please either remove username setting in your configuration or correct it. Will not connect!"),
								pszUser, dbv.pszVal);
							OUTPUTA(szError);
							Initializing = FALSE;
							AttachStatus = -1;
							logoff_contacts(FALSE);
							if (g_hWnd) KillTimer(g_hWnd, 1);
							hSkypeWnd = NULL;
							oldstatus = SkypeStatus;
							InterlockedExchange((long *)&SkypeStatus, ID_STATUS_OFFLINE);
							ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldstatus, SkypeStatus);
						}
					}
				}
				free(pszUser);
			}
		}
		db_free(&dbv);
		if (!Initializing) return;
	}

#ifdef SKYPEBUG_OFFLN
	if (!ResetEvent(GotUserstatus) || SkypeSend("GET USERSTATUS") == -1 ||
		WaitForSingleObject(GotUserstatus, INFINITE) == WAIT_FAILED)
	{
		LOG(("SkypeSystemInit thread stopped with failure."));
		Initializing = FALSE;
		return;
	}
	if (SkypeStatus != ID_STATUS_OFFLINE)
#endif
		if (SearchFriends() == -1) {
		LOG(("SkypeSystemInit thread stopped with failure."));
		Initializing = FALSE;
		return;
		}
	if (protocol >= 5 || bIsImoproxy) {
		SkypeSend("CREATE APPLICATION libpurple_typing");
		testfor("CREATE APPLICATION libpurple_typing", 2000);
		char *pszErr;
		if (SkypeSend("#FT FILE") == 0 && (pszErr = SkypeRcvTime("#FT ERROR", SkypeTime(NULL), 1000)))
		{
			bHasFileXfer = !strncmp(pszErr+4, "ERROR 510", 9);
			free(pszErr);
		}
	}
	else
		bHasFileXfer = FALSE;

	if (protocol >= 5) {
		SearchUsersWaitingMyAuthorization();
		if (db_get_b(NULL, SKYPE_PROTONAME, "UseGroupchat", 0))
			SearchRecentChats(NULL);
	}
	SkypeSend("SEARCH MISSED%sS", cmdMessage);


#ifndef SKYPEBUG_OFFLN
	if (SkypeSend("GET USERSTATUS")==-1)
	{
		LOG (("SkypeSystemInit thread stopped with failure."));
		Initializing=FALSE;
		return;
	}
#endif
	SetTimer(g_hWnd, 1, PING_INTERVAL, NULL);
	SkypeInitialized = TRUE;
	Initializing = FALSE;
	LOG(("SkypeSystemInit thread terminated gracefully."));
	return;
}

void FirstLaunch(char *) {
	int counter = 0;

	LOG(("FirstLaunch thread started."));
	if (!db_get_b(NULL, SKYPE_PROTONAME, "StartSkype", 1) || ConnectToSkypeAPI(skype_path, FALSE) == -1)
	{
		int oldstatus = SkypeStatus;

		LOG(("OnModulesLoaded starting offline.."));
		InterlockedExchange((long *)&SkypeStatus, ID_STATUS_OFFLINE);
		ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldstatus, SkypeStatus);
	}
	if (AttachStatus == -1 || AttachStatus == SKYPECONTROLAPI_ATTACH_REFUSED || AttachStatus == SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE) {
		LOG(("FirstLaunch thread stopped because of invalid Attachstatus."));
		return;
	}

	// When you launch Skype and Attach is Successfull, it still takes some time
	// until it becomes available for receiving messages.
	// Let's probe this with PINGing
	LOG(("CheckIfApiIsResponding Entering test loop"));
	for (;;) {
		LOG(("Test #%d", counter));
		if (SkypeSend("PING") == -1) counter++; else break;
		if (counter >= 20) {
			OUTPUT(TranslateT("Cannot reach Skype API, plugin disfunct."));
			LOG(("FirstLaunch thread stopped: cannot reach Skype API."));
			return;
		}
		Sleep(500);
	}
	LOG(("CheckIfApiIsResponding: Testing for PONG"));
	testfor("PONG", 2000); // Flush PONG from MsgQueue

	pthread_create((pThreadFunc)SkypeSystemInit, NULL);
	LOG(("FirstLaunch thread terminated gracefully."));
}

int CreateTopToolbarButton(WPARAM, LPARAM) {
	TTBButton ttb = { 0 };

	ttb.cbSize = sizeof(ttb);
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.hIconHandleDn = ttb.hIconHandleUp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_CALL));
	ttb.pszService = SKYPEOUT_CALL;
	ttb.name = Translate("Do a SkypeOut-call");
	if ((int)(TopToolbar_AddButton(&ttb)) == -1) httbButton = 0;
	return 0;
}


int OnModulesLoaded(WPARAM, LPARAM) {
	bModulesLoaded = TRUE;

	PopupServiceExists = ServiceExists(MS_POPUP_ADDPOPUPT);

	logoff_contacts(FALSE);

	HookEventsLoaded();
	GCInit();

	add_contextmenu(NULL);
	if (ServiceExists(MS_GC_REGISTER))
	{
		static COLORREF crCols[1] = { 0 };
		char szEvent[MAXMODULELABELLENGTH];

		GCREGISTER gcr = { sizeof(gcr) };
		gcr.dwFlags = GC_CHANMGR; // |GC_ACKMSG; // TODO: Not implemented yet
		gcr.ptszDispName = _T("Skype protocol");
		gcr.pszModule = SKYPE_PROTONAME;
		if (CallService(MS_GC_REGISTER, 0, (LPARAM)&gcr))
			OUTPUT(TranslateT("Unable to register with group chat module!"));

		_snprintf(szEvent, sizeof(szEvent), "%s\\ChatInit", SKYPE_PROTONAME);
		hInitChat = CreateHookableEvent(szEvent);
		HookEvent(szEvent, ChatInit);

		HookEvent(ME_GC_EVENT, GCEventHook);
		HookEvent(ME_GC_BUILDMENU, GCMenuHook);
		CreateServiceFunction(SKYPE_CHATNEW, SkypeChatCreate);
		CreateProtoService(PS_LEAVECHAT, GCOnLeaveChat);
		CreateProtoService(PS_JOINCHAT, GCOnJoinChat);
	}
	// Try folder service first
	hProtocolAvatarsFolder = NULL;
	if (ServiceExists(MS_FOLDERS_REGISTER_PATH))
	{
		char *tmpPath;

		if (!ServiceExists(MS_UTILS_REPLACEVARS) || !(tmpPath = Utils_ReplaceVars("%miranda_avatarcache%")))
			tmpPath = PROFILE_PATH;
		mir_snprintf(DefaultAvatarsFolder, SIZEOF(DefaultAvatarsFolder), "%s\\%s", tmpPath, SKYPE_PROTONAME);
		hProtocolAvatarsFolder = (HANDLE)FoldersRegisterCustomPath(SKYPE_PROTONAME, "Avatars Cache", DefaultAvatarsFolder);
	}

	if (hProtocolAvatarsFolder == NULL)
	{
		// Use defaults
		CallService(MS_DB_GETPROFILEPATH, (WPARAM)MAX_PATH, (LPARAM)DefaultAvatarsFolder);
		mir_snprintf(DefaultAvatarsFolder, SIZEOF(DefaultAvatarsFolder), "%s\\%s", DefaultAvatarsFolder, SKYPE_PROTONAME);
		CreateDirectoryA(DefaultAvatarsFolder, NULL);
	}

	pthread_create((pThreadFunc)FirstLaunch, NULL);
	return 0;
}

void FetchMessageThread(fetchmsg_arg *pargs) {
	char *who = NULL, *type = NULL, *chat = NULL, *users = NULL, *msg = NULL, *status = NULL;
	char *ptr, *msgptr, szPartnerHandle[32], szBuf[128];
	int direction = 0, msglen = 0;
	DWORD timestamp = 0, lwr = 0;
	CCSDATA ccs = { 0 };
	PROTORECVEVENT pre = { 0 };
	MCONTACT hContact = NULL, hChat = NULL;
	MEVENT hDbEvent;
	DBEVENTINFO dbei = { 0 };
	DBVARIANT dbv = { 0 };
	fetchmsg_arg args;
	BOOL bEmoted = FALSE, isGroupChat = FALSE, bHasPartList = FALSE;
	BOOL bUseGroupChat = db_get_b(NULL, SKYPE_PROTONAME, "UseGroupchat", 0);

	if (!pargs) return;
	args = *pargs;
	free(pargs);

	sprintf(szPartnerHandle, "%s_HANDLE", cmdPartner);
	pre.lParam = strtoul(args.msgnum, NULL, 10);
	if (args.bIsRead) pre.flags |= PREF_CREATEREAD;
	//pEvent = MsgList_FindMessage(pre.lParam);

	// Get Timestamp
	if (!args.pMsgEntry || !args.pMsgEntry->tEdited) {
		timestamp=(DWORD)SkypeTime(NULL);
		/* We normally don't use the real timestamp, if it's not history import.
		 * Why? -> Because if you are sending a message while there are still
		 * incoming messages that get processed, msgs are inserted into the
		 * DB with correct timestamp, but message sending dialog shows garbled
		 * messages then because he cannot deal with the situation of incoming
		 * messages that are prior to last sent message */
#ifndef USE_REAL_TS
		if (args.bUseTimestamp)
#endif
		{
			if (!(ptr=SkypeGet (cmdMessage, args.msgnum, "TIMESTAMP"))) return;
			if (strncmp(ptr, "ERROR", 5)) timestamp=atol(ptr);
			free(ptr);
		}
	}
	else timestamp = (DWORD)(args.pMsgEntry->tEdited);

	__try {
		// Get Chatname (also to determine if we need to relay this to a groupchat)
		if (!(chat = SkypeGetErr(cmdMessage, args.msgnum, "CHATNAME"))) __leave;
		if (hChat = find_chatA(chat)) isGroupChat = TRUE;

		// Get chat status
		if ((status = SkypeGetErr("CHAT", chat, "STATUS")) &&
			!mir_strcmp(status, "MULTI_SUBSCRIBED")) isGroupChat = TRUE;

		// Get chat type
		if (!(type = SkypeGetErr(cmdMessage, args.msgnum, "TYPE"))) __leave;
		bEmoted = mir_strcmp(type, "EMOTED") == 0;
		if (mir_strcmp(type, "MULTI_SUBSCRIBED") == 0) isGroupChat = TRUE;

		// Group chat handling
		if (isGroupChat && mir_strcmp(type, "TEXT") && mir_strcmp(type, "SAID") && mir_strcmp(type, "UNKNOWN") && !bEmoted) {
			if (bUseGroupChat) {
				BOOL bAddedMembers = FALSE;

				if (!mir_strcmp(type, "SAWMEMBERS") || !mir_strcmp(type, "CREATEDCHATWITH"))
				{
					// We have a new Groupchat
					LOG(("FetchMessageThread CHAT SAWMEMBERS"));
					if (!hChat) ChatStart(chat, FALSE);
					__leave;
				}
				if (!mir_strcmp(type, "KICKED"))
				{
					if (!hChat) __leave;
					GCDEST gcd = { SKYPE_PROTONAME, make_nonutf_tchar_string((const unsigned char*)chat), GC_EVENT_KICK };
					GCEVENT gce = { sizeof(gce), &gcd };
					gce.dwFlags = GCEF_ADDTOLOG;
					gce.time = timestamp;

					if (users = SkypeGetErr(cmdMessage, args.msgnum, "USERS")) {
						CONTACTINFO ci = { 0 };
						ci.hContact = find_contact(users);
						gce.ptszUID = make_nonutf_tchar_string((const unsigned char*)users);
						if (who = SkypeGetErr(cmdMessage, args.msgnum, szPartnerHandle)) {
							gce.ptszStatus = make_nonutf_tchar_string((const unsigned char*)who);
							ci.cbSize = sizeof(ci);
							ci.szProto = SKYPE_PROTONAME;
							ci.dwFlag = CNF_DISPLAY | CNF_TCHAR;
							if (ci.hContact && !CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) gce.ptszNick = ci.pszVal;
							else gce.ptszNick = gce.ptszUID;

							CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
							RemChatContact(GetChat(gcd.ptszID), gce.ptszUID);
							free_nonutf_tchar_string((void*)gce.ptszStatus);
							if (ci.pszVal) mir_free(ci.pszVal);
						}
						free_nonutf_tchar_string((void*)gce.ptszUID);
					}
					free_nonutf_tchar_string((void*)gcd.ptszID);
					__leave;
				}
				if (!mir_strcmp(type, "SETROLE"))
				{
					gchat_contact *gcContact;
					char *pszRole;

					// FROM_HANDLE - Wer hats gesetzt
					// USERS - Wessen Rolle wurde gesetzt
					// ROLE - Die neue Rolle
					if (!hChat) __leave;
					GCDEST gcd = { SKYPE_PROTONAME, make_nonutf_tchar_string((const unsigned char*)chat), GC_EVENT_REMOVESTATUS };
					GCEVENT gce = { sizeof(gce), &gcd };
					gce.dwFlags = GCEF_ADDTOLOG;
					gce.time = timestamp;

					if (users = SkypeGetErr(cmdMessage, args.msgnum, "USERS")) {
						gce.ptszUID = make_nonutf_tchar_string((const unsigned char*)users);
						if (who = SkypeGetErr(cmdMessage, args.msgnum, szPartnerHandle)) {
							CONTACTINFO ci = { 0 };
							ci.cbSize = sizeof(ci);
							ci.szProto = SKYPE_PROTONAME;
							ci.dwFlag = CNF_DISPLAY | CNF_TCHAR;
							ci.hContact = find_contact(who);
							if (ci.hContact && !CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) {
								gce.ptszText = _tcsdup(ci.pszVal);
								mir_free(ci.pszVal);
								ci.pszVal = NULL;
							}
							else gce.ptszText = make_tchar_string((const unsigned char*)who);

							ci.hContact = find_contact(users);
							if (ci.hContact && !CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) gce.ptszNick = ci.pszVal;
							else gce.ptszNick = gce.ptszUID;

							if (gcContact = GetChatContact(GetChat(gcd.ptszID), gce.ptszUID))
							{
								gce.ptszStatus = gcContact->szRole;
								CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
							}
							if (pszRole = SkypeGetErr(cmdMessage, args.msgnum, "ROLE")) {
								gce.ptszStatus = make_nonutf_tchar_string((const unsigned char*)pszRole);
								gcd.iType = GC_EVENT_ADDSTATUS;
								CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
								free_nonutf_tchar_string((void*)gce.ptszStatus);
								free(pszRole);
							}
							free((void*)gce.ptszText);
							if (ci.pszVal) mir_free(ci.pszVal);
						}
						free_nonutf_tchar_string((void*)gce.ptszUID);
					}
					free_nonutf_tchar_string((void*)gcd.ptszID);
					__leave;
				}
				if (!mir_strcmp(type, "SETTOPIC"))
				{
					LOG(("FetchMessageThread CHAT SETTOPIC"));
					GCDEST gcd = { SKYPE_PROTONAME, make_nonutf_tchar_string((const unsigned char*)chat), GC_EVENT_TOPIC };
					GCEVENT gce = { sizeof(gce), &gcd };
					gce.dwFlags = GCEF_ADDTOLOG;
					gce.time = timestamp;
					if (who = SkypeGetErr(cmdMessage, args.msgnum, szPartnerHandle)) {
						CONTACTINFO ci = { 0 };
						ci.hContact = find_contact(who);
						gce.ptszUID = make_nonutf_tchar_string((const unsigned char*)who);
						ci.cbSize = sizeof(ci);
						ci.szProto = SKYPE_PROTONAME;
						ci.dwFlag = CNF_DISPLAY | CNF_TCHAR;
						if (ci.hContact && !CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) gce.ptszNick = ci.pszVal;
						else gce.ptszNick = gce.ptszUID;

						if (ptr = SkypeGetErr(cmdMessage, args.msgnum, "BODY")) {
							gce.ptszText = make_tchar_string((const unsigned char*)ptr);
							CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
							free((void*)gce.ptszText);
							free(ptr);
						}
						free_nonutf_tchar_string((void*)gce.ptszUID);
						if (ci.pszVal) mir_free(ci.pszVal);
					}
					free_nonutf_tchar_string((void*)gcd.ptszID);
					if (!args.bDontMarkSeen)
					{
						MsgList_Add((DWORD)pre.lParam, -1);
						SkypeSend("SET %s %s SEEN", cmdMessage, args.msgnum);
					}
					__leave;
				}
				if (!mir_strcmp(type, "LEFT") || (bAddedMembers = mir_strcmp(type, "ADDEDMEMBERS") == 0))
				{
					LOG(("FetchMessageThread CHAT LEFT or ADDEDMEMBERS"));
					if (bAddedMembers) {
						GCDEST gcd = { SKYPE_PROTONAME, make_nonutf_tchar_string((const unsigned char*)chat), GC_EVENT_ACTION };
						GCEVENT gce = { sizeof(gce), &gcd };
						gce.dwFlags = GCEF_ADDTOLOG;
						gce.time = timestamp;
						if (users = SkypeGetErr(cmdMessage, args.msgnum, "USERS")) {
							CMString(FORMAT, _T("%S %s"), users, TranslateT("invited"));
							gce.ptszText = make_tchar_string((const unsigned char*)users);
							if (who = SkypeGetErr(cmdMessage, args.msgnum, szPartnerHandle)) {
								DBVARIANT dbv;
								if (db_get_s(NULL, SKYPE_PROTONAME, SKYPE_NAME, &dbv) == 0) {
									gce.bIsMe = mir_strcmp(who, dbv.pszVal) == 0;
									db_free(&dbv);
								}
								gce.ptszUID = make_nonutf_tchar_string((const unsigned char*)who);

								CONTACTINFO ci = { 0 };
								ci.cbSize = sizeof(ci);
								if (!gce.bIsMe)
									ci.hContact = find_contact(who);
								ci.szProto = SKYPE_PROTONAME;
								ci.dwFlag = CNF_DISPLAY | CNF_TCHAR;
								if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci))
									gce.ptszNick = ci.pszVal;
								else
									gce.ptszNick = gce.ptszUID;

								CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
								free_nonutf_tchar_string((void*)gce.ptszUID);
								if (ci.pszVal) mir_free(ci.pszVal);
							}
							if (gce.ptszText) free((void*)gce.ptszText);
						}
						free_nonutf_tchar_string((void*)gcd.ptszID);
					}
					if (!args.QueryMsgDirection) SkypeSend("GET CHAT %s MEMBERS", chat);
					__leave;
				}
			}
			__leave;
		}

		// Need to get the status?
		if (args.getstatus) {
			char *status;

			if (protocol < 4) InterlockedIncrement(&rcvwatchers);
			status = SkypeGetID(cmdMessage, args.msgnum, "STATUS");
			if (protocol < 4) InterlockedDecrement(&rcvwatchers);
			if (!status) __leave;
			if (!mir_strcmp(status, "SENT")) direction = DBEF_SENT;
			free(status);
		}

		// Who sent it?
		if (!(who = SkypeGetErr(cmdMessage, args.msgnum, szPartnerHandle))) __leave;

		// Get contact handle
		LOG(("FetchMessageThread Finding contact handle"));
		db_get_s(NULL, SKYPE_PROTONAME, SKYPE_NAME, &dbv);
		if (dbv.pszVal && !mir_strcmp(who, dbv.pszVal))
		{
			// It's from me.. But to whom?
			// CHATMESSAGE .. USERS doesn't return anything, so we have to query the CHAT-Object
			if (!(ptr = SkypeGetErr("CHAT", chat, "ACTIVEMEMBERS"))) {
				db_free(&dbv);
				__leave;
			}

			char *pTok, *nextoken = 0;
			for (pTok = strtok_r(ptr, " ", &nextoken); pTok; pTok = strtok_r(NULL, " ", &nextoken)) {
				if (mir_strcmp(pTok, dbv.pszVal)) break; // Take the first dude in the list who is not me
			}

			if (!pTok) {
				free(ptr);
				db_free(&dbv);
				__leave; // We failed
			}
			free(who);
			who = (char *)memmove(ptr, pTok, mir_strlen(pTok) + 1);
			direction = DBEF_SENT;
		}
		db_free(&dbv);

		if (!(hContact = find_contact(who))) {
			// Permanent adding of user obsolete, we use the BUDDYSTATUS now (bug #0000005)
			ResetEvent(hBuddyAdded);
			SkypeSend("GET USER %s BUDDYSTATUS", who);
			WaitForSingleObject(hBuddyAdded, INFINITE);
			if (!(hContact = find_contact(who))) {
				// Arrgh, the user has been deleted from contact list.
				// In this case, we add him temp. to receive the msg at least.
				hContact = add_contact(who, PALF_TEMPORARY);
			}
		}

		if (mir_strcmp(type, "FILETRANSFER") == 0)
		{
			// Our custom Skypekit FILETRANSFER extension
			bHasFileXfer = TRUE;
			pre.timestamp = timestamp;
			FXHandleRecv(&pre, hContact);
			__leave;
		}

		// Text which was sent (on edited msg, BODY may already be in queue, check)
		sprintf(szBuf, "GET %s %s BODY", cmdMessage, args.msgnum);
		if (!args.pMsgEntry || !args.pMsgEntry->tEdited || !(ptr = SkypeRcv(szBuf + 4, 1000)))
		{
			if (SkypeSend(szBuf) == -1 || !(ptr = SkypeRcv(szBuf + 4, INFINITE)))
				__leave;
		}
		if (strncmp(ptr, "ERROR", 5)) {
			msgptr = ptr + mir_strlen(szBuf + 4) + 1;
			bHasPartList = strncmp(msgptr, "<partlist ", 10) == 0;
			if (args.pMsgEntry && args.pMsgEntry->tEdited) {
				// Mark the message as edited
				if (!*msgptr && args.pMsgEntry->hEvent != -1) {
					// Empty message and edited -> Delete event
					if ((int)(hContact = db_event_getContact(args.pMsgEntry->hEvent)) != -1) {
						db_event_delete(hContact, args.pMsgEntry->hEvent);
						free(ptr);
						__leave;
					}
				}
				else {
					msgptr -= 9;
					memcpy(msgptr, "[EDITED] ", 9);
				}
			}
			if (bEmoted && !isGroupChat) {
				CONTACTINFO ci = { 0 };
				int newlen;
				char *pMsg, *pszUTFnick = NULL;
				ci.cbSize = sizeof(ci);
				ci.szProto = SKYPE_PROTONAME;
				ci.dwFlag = CNF_DISPLAY | CNF_TCHAR;
				if (ci.hContact = hContact) {
					CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci);
					if (ci.pszVal) {
#ifdef _UNICODE
						pszUTFnick = (char*)make_utf8_string(ci.pszVal);
#else
						utf8_encode (ci.pszVal, &pszUTFnick);
#endif
						mir_free(ci.pszVal);
					}
				}
				newlen = int(mir_strlen(msgptr) + (pszUTFnick ? mir_strlen(pszUTFnick) : 0) + 9);
				if (pMsg = (char *)malloc(newlen)) {
					sprintf(pMsg, "** %s%s%s **", (pszUTFnick ? pszUTFnick : ""), (pszUTFnick ? " " : ""), (char*)msgptr);
					free(ptr);
					ptr = msgptr = pMsg;
				}
				if (pszUTFnick) free(pszUTFnick);
			}

			if (!isGroupChat) {				// I guess Groupchat doesn't support UTF8?
				msg = ptr;
			}
			else {
				// Older version has to decode either UTF8->ANSI or UTF8->UNICODE
				// This could be replaced by mir_getUTFI - functions for Miranda 0.5+ builds, but we stay
				// 0.4 compatible for backwards compatibility. Unfortunately this requires us to link with utf8.c
				if (utf8_decode(msgptr, &msg) == -1) {
					free(ptr);
					__leave;
				}
				msgptr = msg;
				free(ptr);
			}
			msglen = (int)mir_strlen(msgptr) + 1;
		}
		else {
			free(ptr);
			__leave;
		}
		// skype sends some xml statics after a call has finished. Check if thats the case and suppress it if necessary...
		if ((db_get_b(NULL, SKYPE_PROTONAME, "SuppressCallSummaryMessage", 1) &&
			bHasPartList) || msgptr[0] == 0) __leave;

		if (isGroupChat && bUseGroupChat) {
			DBVARIANT dbv = { 0 };

			LOG(("FetchMessageThread This is a group chat message"));
			if (!hChat) ChatStart(chat, FALSE);
			GCDEST gcd = { SKYPE_PROTONAME, make_nonutf_tchar_string((const unsigned char*)chat), bEmoted ? GC_EVENT_ACTION : GC_EVENT_MESSAGE };
			GCEVENT gce = { sizeof(gce), &gcd };
			if ((gce.bIsMe = (direction&DBEF_SENT) ? TRUE : FALSE) && db_get_s(NULL, SKYPE_PROTONAME, SKYPE_NAME, &dbv) == 0)
			{
				free(who);
				who = _strdup(dbv.pszVal);
				db_free(&dbv);
			}
			gce.ptszUID = make_nonutf_tchar_string((const unsigned char*)who);
			gce.ptszNick = gce.ptszUID;

			CONTACTINFO ci = { 0 };
			ci.cbSize = sizeof(ci);
			ci.szProto = SKYPE_PROTONAME;
			ci.dwFlag = CNF_DISPLAY | CNF_TCHAR;
			ci.hContact = !gce.bIsMe ? hContact : NULL;
			if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci))
				gce.ptszNick = ci.pszVal;
			gce.time = timestamp > 0 ? timestamp : (DWORD)SkypeTime(NULL);
			gce.ptszText = (TCHAR*)(msgptr + msglen);
			gce.dwFlags = GCEF_ADDTOLOG;
			CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
			MsgList_Add((DWORD)pre.lParam, -1);	// Mark as groupchat
			if (ci.pszVal) mir_free(ci.pszVal);
			free_nonutf_tchar_string((void*)gce.ptszUID);
			free_nonutf_tchar_string((void*)gcd.ptszID);

			// Yes, we have successfully read the msg
			if (!args.bDontMarkSeen)
				SkypeSend("SET %s %s SEEN", cmdMessage, args.msgnum);
			__leave;
		}

		if (args.QueryMsgDirection || (direction&DBEF_SENT)) {
			// Check if the timestamp is valid
			dbei.cbSize = sizeof(dbei);
			dbei.cbBlob = 0;
			if (hDbEvent = db_event_first(hContact)) {
				db_event_get(hDbEvent, &dbei);
				lwr = dbei.timestamp;
			}
			dbei.cbSize = sizeof(dbei);
			dbei.cbBlob = 0;
			dbei.timestamp = 0;
			if (hDbEvent = db_event_last(hContact))
				db_event_get(hDbEvent, &dbei);
			LOG(("FetchMessageThread timestamp %ld between %ld and %ld", timestamp, lwr, dbei.timestamp));
			if (timestamp < lwr || (direction&DBEF_SENT)) {
				TYP_MSGLENTRY *pme;

				LOG(("FetchMessageThread Adding event"));
				if (!(dbei.szModule = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, hContact, 0)))
					dbei.szModule = SKYPE_PROTONAME;
				dbei.cbBlob = msglen;
				dbei.pBlob = (PBYTE)msgptr;
				dbei.timestamp = timestamp > 0 ? timestamp : (DWORD)SkypeTime(NULL);
				dbei.flags = direction;
				if (pre.flags & PREF_CREATEREAD)
					dbei.flags |= DBEF_READ;
				dbei.eventType = EVENTTYPE_MESSAGE;
				pme = MsgList_Add((DWORD)pre.lParam, db_event_add(hContact, &dbei));

				// We could call MS_PROTO_CHAINSEND if we want to have MetaContact adding the history for us,
				// however we all know that CCSDATA doesn't contain timestamp-information which is
				// really bad on importing history for example, as all messages would be added with current
				// timestamp. This would cause unreliable jumbled timestamps in metacontact, so we better do this
				// ourself.
				if (db_mc_isSub(hContact)) {
					DWORD dwMetaLink = db_get_dw(hContact, "MetaContacts", "MetaLink", MAXDWORD);
					MCONTACT hMetaContact;

					if (dwMetaLink != MAXDWORD && (hMetaContact = GetMetaHandle(dwMetaLink))) {
						dbei.szModule = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hMetaContact, 0);
						pme->hMetaEvent = db_event_add(hMetaContact, &dbei);
					}
				}

				if (!args.QueryMsgDirection && !args.bDontMarkSeen)
					SkypeSend("SET %s %s SEEN", cmdMessage, args.msgnum);
			}
		}


		if (!(direction&DBEF_SENT) && (!args.QueryMsgDirection || (args.QueryMsgDirection && timestamp > dbei.timestamp))) {
			LOG(("FetchMessageThread Normal message add..."));
			// Normal message received, process it
			ccs.szProtoService = PSR_MESSAGE;
			ccs.hContact = hContact;
			ccs.wParam = 0;
			ccs.lParam = (LPARAM)&pre;
			pre.flags |= direction;
			if (isGroupChat && db_get_b(NULL, SKYPE_PROTONAME, "MarkGroupchatRead", 0))
				pre.flags |= PREF_CREATEREAD;
			pre.timestamp = timestamp > 0 ? timestamp : (DWORD)SkypeTime(NULL);
			pre.szMessage = msgptr;
			CallServiceSync(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);

			// Yes, we have successfully read the msg
			if (!args.bDontMarkSeen) SkypeSend("SET %s %s SEEN", cmdMessage, args.msgnum);
		}
	}
	__finally {
		if (status) free(status);
		if (msg) free(msg);
		if (users) free(users);
		if (chat) free(chat);
		if (type) free(type);
		if (who) free(who);
	}

}

void FetchMessageThreadSync(fetchmsg_arg *pargs) {
	// Secure this thread with a mutex.
	// This is needed to ensure that we get called after an old msg in the queue has
	// been added so that MsgList_FindEntry will find it.
	WaitForSingleObject(FetchMessageEvent, 30000);	// Wait max. 30 sec. for previous message fetch to complete
	if ((pargs->pMsgEntry = MsgList_FindMessage(strtoul(pargs->msgnum, NULL, 10))) && !pargs->pMsgEntry->tEdited) {
		// Better don't do this, as we set the msg as read and with this code, we would 
		// mark messages not opened by user as read which isn't that good
		/*
		if (pargs->bIsRead && pMsgEvent->hEvent != INVALID_HANDLE_VALUE)
		{
		MCONTACT hContact;
		if ((int)(hContact = (MCONTACT)CallService (MS_DB_EVENT_GETCONTACT, (WPARAM)pMsgEntry->hEvent, 0)) != -1)
		CallService (MS_DB_EVENT_MARKREAD, hContact, (LPARAM)hDBEvent);
		}
		*/
		free(pargs);
	}
	else FetchMessageThread(pargs);
	SetEvent(FetchMessageEvent);
}

static int MsglCmpProc(const void *pstPElement, const void *pstPToFind)
{
	return mir_strcmp((char*)((fetchmsg_arg*)pstPElement)->pMsgEntry, (char*)((fetchmsg_arg*)pstPToFind)->pMsgEntry);
}

void MessageListProcessingThread(char *str) {
	char *token, *nextoken = 0, *chat = NULL;
	fetchmsg_arg *args;
	TYP_LIST *hListMsgs = List_Init(32);

	// Frst we need to sort the message timestamps
	for ((token = strtok_r(str+1, ", ", &nextoken)); token; token = strtok_r(NULL, ", ", &nextoken)) {
		if (args = (fetchmsg_arg*)calloc(1, sizeof(fetchmsg_arg) + sizeof(DWORD))) {
			mir_strncpy(args->msgnum, token, sizeof(args->msgnum)-1);
			args->getstatus = TRUE;
			args->bIsRead = *str;
			args->bDontMarkSeen = *str;
			args->QueryMsgDirection = TRUE;
			args->pMsgEntry = (TYP_MSGLENTRY*)SkypeGet("CHATMESSAGE", token, "TIMESTAMP");
			if (!chat) chat = SkypeGet("CHATMESSAGE", token, "CHATNAME");
			if (args->pMsgEntry) List_InsertSort(hListMsgs, MsglCmpProc, args);
			else free(args);
		}
	}
	int nCount = List_Count(hListMsgs);
	for (int i = 0; i < nCount; i++) {
		args = (fetchmsg_arg*)List_ElementAt(hListMsgs, i);
		free(args->pMsgEntry);
		args->pMsgEntry = NULL;
		args->bUseTimestamp = TRUE;
		FetchMessageThreadSync(args);
	}
	if (chat) {
		SkypeSend("GET CHAT %s MEMBERS", chat);
		free(chat);
	}
	List_Exit(hListMsgs);
	free(str);
}

char *GetCallerHandle(char *szSkypeMsg) {
	return SkypeGet(szSkypeMsg, "PARTNER_HANDLE", "");
}


MCONTACT GetCallerContact(char *szSkypeMsg)
{
	char *szHandle = GetCallerHandle(szSkypeMsg);
	if (!szHandle)
		return NULL;
	MCONTACT hContact = find_contact(szHandle);
	if (!hContact) {
		// If it's a SkypeOut-contact, PARTNER_HANDLE = SkypeOUT number
		DBVARIANT dbv;
		int tCompareResult;

		for (hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact)) {
			if (db_get_s(hContact, SKYPE_PROTONAME, "SkypeOutNr", &dbv)) continue;
			tCompareResult = mir_strcmp(dbv.pszVal, szHandle);
			db_free(&dbv);
			if (tCompareResult) continue; else break;
		}
	}
	free(szHandle);
	if (!hContact) { LOG(("GetCallerContact Not found!")); }
	return hContact;
}

MCONTACT GetMetaHandle(DWORD dwId)
{
	for (MCONTACT hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact)) {
		char *szProto = GetContactProto(hContact);
		if (szProto != NULL && !mir_strcmp(szProto, "MetaContacts") &&
			db_get_dw(hContact, "MetaContacts", "MetaID", MAXDWORD) == dwId)
			return hContact;
	}
	return 0;
}

LRESULT CALLBACK InCallPopUpProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		break;

	case WM_CONTEXTMENU:
		SendMessage(hwnd, UM_DESTROYPOPUP, 0, 0);
		break;
	case UM_FREEPLUGINDATA:
		//Here we'd free our own data, if we had it.
		return FALSE;
	case UM_INITPOPUP:
		break;
	case UM_DESTROYPOPUP:
		break;
	case WM_NOTIFY:
	default:
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void RingThread(char *szSkypeMsg) {
	DBVARIANT dbv;
	DBEVENTINFO dbei = { 0 };
	char *ptr = NULL;

	// We use a single critical section for the RingThread- and the EndCallThread-functions
	// so that only one function is running at the same time. This is needed, because when
	// a initated and unaccepted call (which is still ringing) is hangup/canceled, skype
	// sends two messages. First "CALL xxx STATUS RINGING" .. second "CALL xx STATUS CANCELED".
	// This starts two independend threads (first: RingThread; second: EndCallThread). Now 
	// the two message are processed in reverse order sometimes. This causes the EndCallThread to
	// delete the contacts "CallId" property and after that the RingThread saves the contacts 
	// "CallId" again. After that its not possible to call this contact, because the plugin
	// thinks that there is already a call going and the hangup-function isnt working, because 
	// skype doesnt accept status-changes for finished calls. The CriticalSection syncronizes
	// the threads and the messages are processed in correct order. 
	// Not the best solution, but it works.
	EnterCriticalSection(&RingAndEndcallMutex);

	LOG(("RingThread started."));
	if (protocol >= 5) SkypeSend("MINIMIZE");
	MCONTACT hContact = GetCallerContact(szSkypeMsg);
	if (hContact) {
		// Make sure that an answering thread is not already in progress so that we don't get
		// the 'Incoming call' event twice
		if (!db_get_s(hContact, SKYPE_PROTONAME, "CallId", &dbv)) {
			db_free(&dbv);
			LOG(("RingThread terminated."));
			goto l_exitRT;
		}
		db_set_s(hContact, SKYPE_PROTONAME, "CallId", szSkypeMsg);
	}

	if (!(ptr = SkypeGet(szSkypeMsg, "TYPE", ""))) {
		LOG(("RingThread terminated."));
		goto l_exitRT;;
	}

	if (!strncmp(ptr, "INCOMING", 8)) {
		if (!hContact) {
			char *szHandle = GetCallerHandle(szSkypeMsg);

			if (szHandle) {
				if (!(hContact = add_contact(szHandle, PALF_TEMPORARY))) {
					free(szHandle);
					goto l_exitRT;
				}
				db_unset(hContact, "CList", "Hidden");
				db_set_w(hContact, SKYPE_PROTONAME, "Status", (WORD)SkypeStatusToMiranda("SKYPEOUT"));
				db_set_s(hContact, SKYPE_PROTONAME, "SkypeOutNr", szHandle);
				free(szHandle);
			}
			else goto l_exitRT;
		}
	}
	
	dbei.cbSize = sizeof(dbei);
	dbei.eventType = EVENTTYPE_CALL;
	dbei.szModule = SKYPE_PROTONAME;
	dbei.timestamp = (DWORD)SkypeTime(NULL);
	dbei.pBlob = (unsigned char*)Translate("Phone call");
	dbei.cbBlob = (int)mir_strlen((const char*)dbei.pBlob) + 1;
	if (!strncmp(ptr, "INCOMING", 8))
	{
		TCHAR *lpzContactName = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR);
		if (PopupServiceExists)
		{
			BOOL showPopup, popupWindowColor;
			unsigned int popupBackColor, popupTextColor;
			int popupTimeSec;
			POPUPDATAT InCallPopup;

			popupTimeSec = db_get_dw(NULL, SKYPE_PROTONAME, "popupTimeSec", 4);
			popupTextColor = db_get_dw(NULL, SKYPE_PROTONAME, "popupTextColor", GetSysColor(COLOR_WINDOWTEXT));
			popupBackColor = db_get_dw(NULL, SKYPE_PROTONAME, "popupBackColor", GetSysColor(COLOR_BTNFACE));
			popupWindowColor = (0 != db_get_b(NULL, SKYPE_PROTONAME, "popupWindowColor", TRUE));
			showPopup = (0 != db_get_b(NULL, SKYPE_PROTONAME, "showPopup", TRUE));

			InCallPopup.lchContact = hContact;
			InCallPopup.lchIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CALL));
			InCallPopup.colorBack = !popupWindowColor ? popupBackColor : GetSysColor(COLOR_BTNFACE);
			InCallPopup.colorText = !popupWindowColor ? popupTextColor : GetSysColor(COLOR_WINDOWTEXT);
			InCallPopup.iSeconds = popupTimeSec;
			InCallPopup.PluginWindowProc = InCallPopUpProc;
			InCallPopup.PluginData = (void *)1;

			mir_tstrncpy(InCallPopup.lptzText, TranslateT("Incoming Skype call"), MAX_SECONDLINE);

			mir_tstrncpy(InCallPopup.lptzContactName, lpzContactName, MAX_CONTACTNAME);

			if (showPopup)
				CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&InCallPopup, 0);

		}

		CLISTEVENT cle = { 0 };
		cle.cbSize = sizeof(cle);
		cle.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CALL));
		cle.pszService = SKYPE_ANSWERCALL;
		dbei.flags = DBEF_READ;
		cle.hContact = hContact;
		cle.hDbEvent = db_event_add(hContact, &dbei);
		cle.flags = CLEF_TCHAR;
		TCHAR toolTip[256];
		mir_sntprintf(toolTip, SIZEOF(toolTip), TranslateT("Incoming call from %s"), lpzContactName);
		cle.ptszTooltip = toolTip;
		CallServiceSync(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
	}
	else
	{
		dbei.flags = DBEF_SENT;
		db_event_add(hContact, &dbei);
	}

l_exitRT:
	if (ptr) free(ptr);
	free(szSkypeMsg);
	LeaveCriticalSection(&RingAndEndcallMutex);
}

void EndCallThread(char *szSkypeMsg) {
	MCONTACT hContact = NULL;
	MEVENT hDbEvent;
	DBEVENTINFO dbei = { 0 };
	DBVARIANT dbv;

	// We use a single critical section for the RingThread- and the EndCallThread-functions
	// so that only one function is running at the same time. This is needed, because when
	// a initated and unaccepted call (which is still ringing) is hangup/canceled, skype
	// sends two messages. First "CALL xxx STATUS RINGING" .. second "CALL xx STATUS CANCELED".
	// This starts two independend threads (first: RingThread; second: EndCallThread). Now 
	// the two message are processed in reverse order sometimes. This causes the EndCallThread to
	// delete the contacts "CallId" property and after that the RingThread saves the contacts 
	// "CallId" again. After that its not possible to call this contact, because the plugin
	// thinks that there is already a call going and the hangup-function isnt working, because 
	// skype doesnt accept status-changes for finished calls. The CriticalSection syncronizes
	// the threads and the messages are processed in correct order. 
	// Not the best solution, but it works.
	EnterCriticalSection(&RingAndEndcallMutex);

	LOG(("EndCallThread started."));
	if (szSkypeMsg) {
		for (hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact)) {
			if (db_get_s(hContact, SKYPE_PROTONAME, "CallId", &dbv)) continue;
			int tCompareResult = mir_strcmp(dbv.pszVal, szSkypeMsg);
			db_free(&dbv);
			if (!tCompareResult)
				break;
		}
	}
	if (hContact)
	{
		db_unset(hContact, SKYPE_PROTONAME, "CallId");

		dbei.cbSize = sizeof(dbei);
		hDbEvent = db_event_firstUnread(hContact);
		while (hDbEvent) {
			dbei.cbBlob = 0;
			db_event_get(hDbEvent, &dbei);
			if (!(dbei.flags&(DBEF_SENT | DBEF_READ)) && dbei.eventType == EVENTTYPE_CALL) {
				db_event_markRead(hContact, hDbEvent);
				CallService(MS_CLIST_REMOVEEVENT, hContact, (LPARAM)hDbEvent);
			}
			free(dbei.pBlob);
			hDbEvent = db_event_next(hContact, hDbEvent);
		}

		if (!db_get_s(hContact, SKYPE_PROTONAME, "SkypeOutNr", &dbv)) {
			db_free(&dbv);
			if (!mir_strcmp((char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, hContact, 0), SKYPE_PROTONAME) &&
				db_get_b(hContact, "CList", "NotOnList", 0)
				)
				CallService(MS_DB_CONTACT_DELETE, hContact, 0);
		}
	}
	free(szSkypeMsg);
	LeaveCriticalSection(&RingAndEndcallMutex);
}

void HoldCallThread(char *szSkypeMsg) {
	LOG(("HoldCallThread started"));
	if (!szSkypeMsg) {
		LOG(("HoldCallThread terminated."));
		return;
	}
	MCONTACT hContact = GetCallerContact(szSkypeMsg);
	if (hContact) {
		db_set_b(hContact, SKYPE_PROTONAME, "OnHold", 1);
	}
	free(szSkypeMsg);
	LOG(("HoldCallThread terminated gracefully"));
}

void ResumeCallThread(char *szSkypeMsg) {
	LOG(("ResumeCallThread started"));
	if (!szSkypeMsg) {
		LOG(("ResumeCallThread terminated."));
		return;
	}
	MCONTACT hContact = GetCallerContact(szSkypeMsg);
	if (hContact) {
		db_unset(hContact, SKYPE_PROTONAME, "OnHold");
	}
	free(szSkypeMsg);
	LOG(("ResumeCallThread terminated gracefully."));
}

int SetUserStatus(void) {
	if (RequestedStatus && AttachStatus != -1) {
		if (SkypeSend("SET USERSTATUS %s", RequestedStatus) == -1) return 1;
	}
	return 0;
}

void LaunchSkypeAndSetStatusThread(void *) {

	/*	   if (!db_get_b(NULL, SKYPE_PROTONAME, "UnloadOnOffline", 0)) {
			   logoff_contacts();
			   return 1;
			   }
			   */
	int oldStatus = SkypeStatus;
	static BOOL bLaunching = FALSE;

	if (bLaunching) return;
	bLaunching = TRUE;
	LOG(("LaunchSkypeAndSetStatusThread started."));
	InterlockedExchange((long *)&SkypeStatus, ID_STATUS_CONNECTING);
	ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, SkypeStatus);

	if (ConnectToSkypeAPI(skype_path, 1) != -1) {
		pthread_create((pThreadFunc)SkypeSystemInit, NULL);
		//InterlockedExchange((long *)&SkypeStatus, newStatus);
		//ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, SkypeStatus);
		SetUserStatus();
	}

	LOG(("LaunchSkypeAndSetStatusThread terminated gracefully."));
	bLaunching = FALSE;
}

LRESULT APIENTRY WndProc(HWND hWndDlg, UINT message, UINT wParam, LONG lParam)
{
	PCOPYDATASTRUCT CopyData;
	char *ptr, *szSkypeMsg = NULL, *nick, *buf;
	static char *onlinestatus = NULL;
	static BOOL RestoreUserStatus = FALSE;
	int sstat, oldstatus, flag;
	MCONTACT hContact;
	fetchmsg_arg *args;
	static int iReentranceCnt = 0;

	iReentranceCnt++;
	switch (message) {
	case WM_COPYDATA:
		LOG(("WM_COPYDATA start"));
		if (hSkypeWnd == (HWND)wParam) {
			char *pData;
			CopyData = (PCOPYDATASTRUCT)lParam;
			pData = (char*)CopyData->lpData;
			while (*pData == ' ') pData++;
			szSkypeMsg = _strdup((char*)pData);
			ReplyMessage(1);
			LOG(("< %s", szSkypeMsg));

			if (!strncmp(szSkypeMsg, "CONNSTATUS", 10)) {
				if (!strncmp(szSkypeMsg + 11, "LOGGEDOUT", 9)) {
					SkypeInitialized = FALSE;
					ResetEvent(SkypeReady);
					AttachStatus = -1;
					sstat = ID_STATUS_OFFLINE;
					if (g_hWnd) KillTimer(g_hWnd, 1);
					logoff_contacts(TRUE);
				}
				else
					sstat = SkypeStatusToMiranda(szSkypeMsg + 11);

				if (sstat) {
					oldstatus = SkypeStatus;
					InterlockedExchange((long*)&SkypeStatus, sstat);
					ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldstatus, SkypeStatus);
					if (sstat != ID_STATUS_OFFLINE) {
						if (sstat != ID_STATUS_CONNECTING && (oldstatus == ID_STATUS_OFFLINE || oldstatus == ID_STATUS_CONNECTING)) {

							SkypeInitialized = FALSE;
							pthread_create((pThreadFunc)SkypeSystemInit, NULL);
						}
						if (db_get_b(NULL, SKYPE_PROTONAME, "KeepState", 0)) RestoreUserStatus = TRUE;
					}

					//					if (SkypeStatus==ID_STATUS_ONLINE) SkypeSend("SEARCH MISSEDMESSAGES");
				}
				//				break;
			}
			if (!strncmp(szSkypeMsg, "USERSTATUS", 10)) {
				//				if ((sstat=SkypeStatusToMiranda(szSkypeMsg+11)) && SkypeStatus!=ID_STATUS_CONNECTING) {
				if ((sstat = SkypeStatusToMiranda(szSkypeMsg + 11))) {
					if (RestoreUserStatus && RequestedStatus) {
						RestoreUserStatus = FALSE;
						SkypeSend("SET USERSTATUS %s", RequestedStatus);
					}
					oldstatus = SkypeStatus;
					InterlockedExchange((long*)&SkypeStatus, sstat);
					ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldstatus, sstat);
#ifdef SKYPEBUG_OFFLN
					if ((oldstatus == ID_STATUS_OFFLINE || oldstatus == ID_STATUS_CONNECTING) &&
						SkypeStatus != ID_STATUS_CONNECTING && SkypeStatus != ID_STATUS_OFFLINE)
						pthread_create((pThreadFunc)SearchFriendsThread, NULL);
#endif
				}
#ifdef SKYPEBUG_OFFLN
				SetEvent(GotUserstatus);
#endif
				break;
			}
			if (!strncmp(szSkypeMsg, "APPLICATION libpurple_typing", 28)) {
				char *nextoken = 0, *p;

				if (p = strtok_r(szSkypeMsg + 29, " ", &nextoken))
				{
					if (!mir_strcmp(p, "STREAMS")) {
						char *pStr;

						while (p = strtok_r(NULL, " ", &nextoken)) {
							if (pStr = strchr(p, ':')) {
								*pStr = 0;
								if (hContact = find_contact(p)) {
									*pStr = ':';
									db_set_s(hContact, SKYPE_PROTONAME, "Typing_Stream", p);
								}
							}
						}
					}
					else if (!mir_strcmp(p, "DATAGRAM")) {
						if (p = strtok_r(NULL, " ", &nextoken)) {
							char *pStr;

							if (pStr = strchr(p, ':')) {
								*pStr = 0;
								if (hContact = find_contact(p)) {
									*pStr = ':';
									db_set_s(hContact, SKYPE_PROTONAME, "Typing_Stream", p);

									if (p = strtok_r(NULL, " ", &nextoken)) {
										LPARAM lTyping = PROTOTYPE_CONTACTTYPING_OFF;

										if (!mir_strcmp(p, "PURPLE_TYPING")) lTyping = PROTOTYPE_CONTACTTYPING_INFINITE;
										CallService(MS_PROTO_CONTACTISTYPING, hContact, lTyping);
										break;
									}
								}
							}
						}
					}
				}
			}
			if (!strncmp(szSkypeMsg, "USER ", 5)) {
				char *nextoken = 0;

				buf = _strdup(szSkypeMsg + 5);
				if ((nick = strtok_r(buf, " ", &nextoken)) && (ptr = strtok_r(NULL, " ", &nextoken)))
				{
					if (mir_strcmp(ptr, "BUDDYSTATUS")) {
						if (!mir_strcmp(ptr, "RECEIVEDAUTHREQUEST")) {
							pthread_create((pThreadFunc)ProcessAuthRq, strdup(szSkypeMsg));
							free(buf);
							break;
						}

						if (!(hContact = find_contact(nick)) && mir_strcmp(ptr, "FULLNAME")) {
							SkypeSend("GET USER %s BUDDYSTATUS", nick);
							free(buf);
							break;
						}

						if (!mir_strcmp(ptr, "ONLINESTATUS")) {
							if (SkypeStatus != ID_STATUS_OFFLINE)
							{
								db_set_w(hContact, SKYPE_PROTONAME, "Status", (WORD)SkypeStatusToMiranda(ptr + 13));
								if ((WORD)SkypeStatusToMiranda(ptr + 13) != ID_STATUS_OFFLINE)
								{
									LOG(("WndProc Status is not offline so get user info"));
									pthread_create(GetInfoThread, (void*)hContact);
								}
							}
						}

						/* We handle the following properties right here in the wndProc, in case that
						* Skype protocol broadcasts them to us.
						*
						* However, we still let them be added to the Message queue im memory, as they
						* may get consumed by GetInfoThread.
						* This is necessary to have a proper error handling in case the property is
						* not supported (i.e. imo2sproxy).
						*
						* If one of the property GETs returns an error, the error-message has to be
						* removed from the message queue, as the error is the answer to the query.
						* If we don't remove the ERRORs from the list, another consumer may see the ERROR
						* as a reply to his query and process it.
						* In case the SKYPE Protocol really broadcasts one of these messages without being
						* requested by GetInfoThread (i.e. MOOD_TEXT), the garbage collector will take
						* care of them and remove them after some time.
						* This may not be the most efficient way, but ensures that we finally do proper
						* error handling.
						*/
						if (!mir_strcmp(ptr, "FULLNAME")) {
							char *nm = strtok_r(NULL, " ", &nextoken);

							if (nm)
							{
								db_set_utf(hContact, SKYPE_PROTONAME, "FirstName", nm);
								if (!(nm = strtok_r(NULL, "", &nextoken)))
									db_unset(hContact, SKYPE_PROTONAME, "LastName");
								else
									db_set_utf(hContact, SKYPE_PROTONAME, "LastName", nm);
							}
						}
						else if (!mir_strcmp(ptr, "BIRTHDAY")) {
							int y, m, d;
							if (sscanf(ptr + 9, "%04d%02d%02d", &y, &m, &d) == 3) {
								db_set_w(hContact, SKYPE_PROTONAME, "BirthYear", (WORD)y);
								db_set_b(hContact, SKYPE_PROTONAME, "BirthMonth", (BYTE)m);
								db_set_b(hContact, SKYPE_PROTONAME, "BirthDay", (BYTE)d);
							}
							else {
								db_unset(hContact, SKYPE_PROTONAME, "BirthYear");
								db_unset(hContact, SKYPE_PROTONAME, "BirthMonth");
								db_unset(hContact, SKYPE_PROTONAME, "BirthDay");
							}
						}
						else if (!mir_strcmp(ptr, "COUNTRY")) {
							if (ptr[8]) {
								struct CountryListEntry *countries;
								int countryCount;
								CallService(MS_UTILS_GETCOUNTRYLIST, (WPARAM)&countryCount, (LPARAM)&countries);
								for (int i = 0; i < countryCount; i++) {
									if (countries[i].id == 0 || countries[i].id == 0xFFFF) continue;
									if (!_stricmp(countries[i].szName, ptr + 8))
									{
										db_set_w(hContact, SKYPE_PROTONAME, "Country", (BYTE)countries[i].id);
										break;
									}
								}
							}
							else db_unset(hContact, SKYPE_PROTONAME, "Country");
						}
						else if (!mir_strcmp(ptr, "SEX")) {
							if (ptr[4]) {
								BYTE sex = 0;
								if (!_stricmp(ptr + 4, "MALE")) sex = 0x4D;
								if (!_stricmp(ptr + 4, "FEMALE")) sex = 0x46;
								if (sex) db_set_b(hContact, SKYPE_PROTONAME, "Gender", sex);
							}
							else db_unset(hContact, SKYPE_PROTONAME, "Gender");
						}
						else if (!mir_strcmp(ptr, "MOOD_TEXT")) {
							LOG(("WndProc MOOD_TEXT"));
							db_set_utf(hContact, "CList", "StatusMsg", ptr + 10);
						}
						else if (!mir_strcmp(ptr, "TIMEZONE")){
							time_t temp;
							struct tm tms;
							int value = atoi(ptr + 9), tz;

							LOG(("WndProc: TIMEZONE %s", nick));

							if (value && !db_get_b(NULL, SKYPE_PROTONAME, "IgnoreTimeZones", 0)) {
								temp = SkypeTime(NULL);
								tms = *localtime(&temp);
								//memcpy(&tms,localtime(&temp), sizeof(tm));
								//tms = localtime(&temp)
								tz = (value >= 86400) ? (256 - ((2 * (atoi(ptr + 9) - 86400)) / 3600)) : ((-2 * (atoi(ptr + 9) - 86400)) / 3600);
								if (tms.tm_isdst == 1 && db_get_b(NULL, SKYPE_PROTONAME, "UseTimeZonePatch", 0))
								{
									LOG(("WndProc: Using the TimeZonePatch"));
									db_set_b(hContact, "UserInfo", "Timezone", (BYTE)(tz + 2));
								}
								else
								{
									LOG(("WndProc: Not using the TimeZonePatch"));
									db_set_b(hContact, "UserInfo", "Timezone", (BYTE)(tz + 0));
								}
							}
							else 	{
								LOG(("WndProc: Deleting the TimeZone in UserInfo Section"));
								db_unset(hContact, "UserInfo", "Timezone");
							}
						}
						else if (!mir_strcmp(ptr, "IS_VIDEO_CAPABLE")){
							if (!_stricmp(ptr + 17, "True"))
								db_set_s(hContact, SKYPE_PROTONAME, "MirVer", "Skype 2.0");
							else
								db_set_s(hContact, SKYPE_PROTONAME, "MirVer", "Skype");
						}
						else if (!mir_strcmp(ptr, "ISBLOCKED")){
							if (!_stricmp(ptr + 10, "True"))
								db_set_b(hContact, SKYPE_PROTONAME, "IsBlocked", 1);
							else
								db_unset(hContact, SKYPE_PROTONAME, "IsBlocked");
						}
						else if (!mir_strcmp(ptr, "RICH_MOOD_TEXT")) {
							db_set_s(hContact, SKYPE_PROTONAME, "MirVer", "Skype 3.0");
						}
						else if (!mir_strcmp(ptr, "DISPLAYNAME")) {
							// Skype Bug? -> If nickname isn't customised in the Skype-App, this won't return anything :-(
							if (ptr[12])
								db_set_utf(hContact, SKYPE_PROTONAME, "Nick", ptr + 12);
						}
						else
						{
							// Other proerties that can be directly assigned to a DB-Value
							for (int i = 0; i < sizeof(m_settings) / sizeof(m_settings[0]); i++) {
								if (!mir_strcmp(ptr, m_settings[i].SkypeSetting)) {
									char *pszProp = ptr + mir_strlen(m_settings[i].SkypeSetting) + 1;
									if (*pszProp)
										db_set_utf(hContact, SKYPE_PROTONAME, m_settings[i].MirandaSetting, pszProp);
									else
										db_unset(hContact, SKYPE_PROTONAME, m_settings[i].MirandaSetting);
								}
							}
						}
					}
					else { // BUDDYSTATUS:
						flag = 0;
						switch (atoi(ptr + 12)) {
						case 1: if (hContact = find_contact(nick)) CallService(MS_DB_CONTACT_DELETE, hContact, 0); break;
						case 0: break;
						case 2: flag = PALF_TEMPORARY;
						case 3: add_contact(nick, flag);
							SkypeSend("GET USER %s ONLINESTATUS", nick);
							break;
						}
						free(buf);
						if (!SetEvent(hBuddyAdded))
							TellError(GetLastError());
						break;
					}
				}
				free(buf);
			}
			if (!strncmp(szSkypeMsg, "CURRENTUSERHANDLE", 17)) {	// My username
				DBVARIANT dbv = { 0 };

				if (db_get_s(NULL, SKYPE_PROTONAME, "LoginUserName", &dbv) ||
					!*dbv.pszVal || _stricmp(szSkypeMsg + 18, dbv.pszVal) == 0)
				{
					db_set_s(NULL, SKYPE_PROTONAME, SKYPE_NAME, szSkypeMsg + 18);
					db_set_s(NULL, SKYPE_PROTONAME, "Nick", szSkypeMsg + 18);
					pthread_create((pThreadFunc)GetDisplaynameThread, NULL);
				}
				if (dbv.pszVal) db_free(&dbv);
			}
			if (strstr(szSkypeMsg, "AUTOAWAY") || !strncmp(szSkypeMsg, "OPEN ", 5) ||
				(SkypeInitialized && !strncmp(szSkypeMsg, "PONG", 4)) ||
				!strncmp(szSkypeMsg, "MINIMIZE", 8))
			{
				// Currently we do not process these messages  
				break;
			}
			if (!strncmp(szSkypeMsg, "CHAT ", 5)) {
				// Currently we only process these notifications
				if (db_get_b(NULL, SKYPE_PROTONAME, "UseGroupchat", 0) &&
					(ptr = strchr(szSkypeMsg, ' ')) && (ptr = strchr(++ptr, ' ')))
				{
					if (strncmp(ptr, " MEMBERS", 8) == 0) {
						LOG(("WndProc AddMembers"));
						pthread_create((pThreadFunc)AddMembersThread, _strdup(szSkypeMsg));
					}
					else
						if (strncmp(ptr, " FRIENDLYNAME ", 14) == 0) {
							// Chat session name
							MCONTACT hContact;

							*ptr = 0;
							if (hContact = find_chatA(szSkypeMsg + 5))
							{
								if (db_get_w(hContact, SKYPE_PROTONAME, "Status", ID_STATUS_OFFLINE) !=
									ID_STATUS_OFFLINE)
								{
									GCDEST gcd = { SKYPE_PROTONAME, make_nonutf_tchar_string((const unsigned char*)szSkypeMsg + 5), GC_EVENT_CHANGESESSIONAME };
									GCEVENT gce = { sizeof(gce), &gcd };
									gce.ptszText = make_tchar_string((const unsigned char*)ptr + 14);
									if (gce.ptszText) {
										CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
										db_set_ts(hContact, SKYPE_PROTONAME, "Nick", gce.ptszText);
										free((void*)gce.ptszText);
									}
									free_nonutf_tchar_string((void*)gcd.ptszID);
								}
							}
							*ptr = ' ';
						}
						else if (strncmp(ptr, " CHATMESSAGES ", 14) == 0) {
							int iLen=mir_strlen(ptr+14)+1;
							char *pParam=(char*)calloc(iLen+1, 1);
							*pParam=TRUE;
							memcpy(pParam+1, ptr+14, iLen);
							pthread_create((pThreadFunc)MessageListProcessingThread, pParam);
							break;
						}
				}
			}
			if (!strncmp(szSkypeMsg, "CALL ", 5)) {
				// incoming calls are already processed by Skype, so no need for us
				// to do this.
				// However we can give a user the possibility to hang up a call via Miranda's
				// context menu
				if (ptr = strstr(szSkypeMsg, " STATUS ")) {
					ptr[0] = 0; ptr += 8;
					if (!mir_strcmp(ptr, "RINGING") || !mir_strcmp(ptr, "ROUTING")) pthread_create((pThreadFunc)RingThread, _strdup(szSkypeMsg));
					if (!mir_strcmp(ptr, "FAILED") || !mir_strcmp(ptr, "FINISHED") ||
						!mir_strcmp(ptr, "MISSED") || !mir_strcmp(ptr, "REFUSED") ||
						!mir_strcmp(ptr, "BUSY") || !mir_strcmp(ptr, "CANCELLED"))
						pthread_create((pThreadFunc)EndCallThread, _strdup(szSkypeMsg));
					if (!mir_strcmp(ptr, "ONHOLD") || !mir_strcmp(ptr, "LOCALHOLD") ||
						!mir_strcmp(ptr, "REMOTEHOLD")) pthread_create((pThreadFunc)HoldCallThread, _strdup(szSkypeMsg));
					if (!mir_strcmp(ptr, "INPROGRESS")) pthread_create((pThreadFunc)ResumeCallThread, _strdup(szSkypeMsg));
					break;
				}
				else if ((!strstr(szSkypeMsg, "PARTNER_HANDLE") && !strstr(szSkypeMsg, "FROM_HANDLE"))
					&& !strstr(szSkypeMsg, "TYPE")) break;
			}
			if (!strncmp(szSkypeMsg, "PRIVILEGE SKYPEOUT", 18)) {
				if (!strncmp(szSkypeMsg + 19, "TRUE", 4)) {
					if (!bSkypeOut) {
						CLISTMENUITEM mi = { 0 };

						bSkypeOut = TRUE;
						mi.cbSize = sizeof(mi);
						mi.position = -2000005000;
						mi.flags = 0;
						mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CALLSKYPEOUT));
						mi.pszContactOwner = SKYPE_PROTONAME;
						mi.pszName = Translate("Do a SkypeOut-call");
						mi.pszService = SKYPEOUT_CALL;
						Menu_AddMainMenuItem(&mi);
					}

				}
				else {
					bSkypeOut = FALSE;
					if (httbButton) {
						CallService(MS_TTB_REMOVEBUTTON, (WPARAM)httbButton, 0);
						httbButton = 0;
					}
				}
				break;
			}
			if (!strncmp(szSkypeMsg, "MESSAGES", 8) || !strncmp(szSkypeMsg, "CHATMESSAGES", 12)) {
				char *pMsgs;
				int iLen;
				if (mir_strlen(szSkypeMsg) <= (UINT)((pMsgs=strchr(szSkypeMsg, ' ')) - szSkypeMsg + 1))
				{
					LOG(("%s %d %s %d", szSkypeMsg, (UINT)(strchr(szSkypeMsg, ' ') - szSkypeMsg + 1),
						strchr(szSkypeMsg, ' '), mir_strlen(szSkypeMsg)));
					break;
				}
				LOG(("MessageListProcessingThread launched"));
				char *pParam=(char*)calloc((iLen=mir_strlen(pMsgs)+1)+1, 1);
				memcpy(pParam+1, pMsgs, iLen);
				pthread_create((pThreadFunc)MessageListProcessingThread, pParam);
				break;
			}
			if (!strncmp(szSkypeMsg, "MESSAGE", 7) || !strncmp(szSkypeMsg, "CHATMESSAGE", 11))
			{
				char *pMsgNum;
				TYP_MSGLENTRY *pEntry;

				if ((pMsgNum = strchr(szSkypeMsg, ' ')) && (ptr = strchr(++pMsgNum, ' ')))
				{
					BOOL bFetchMsg = FALSE;

					if (strncmp(ptr, " EDITED_TIMESTAMP", 17) == 0) {
						ptr[0] = 0;
						if (pEntry = MsgList_FindMessage(strtoul(pMsgNum, NULL, 10))) {
							pEntry->tEdited = atol(ptr + 18);
						}
						bFetchMsg = TRUE;
					}
					else bFetchMsg = (strncmp(ptr, " STATUS RE", 10) == 0 && !rcvwatchers) ||
						(strncmp(ptr, " STATUS SENT", 12) == 0 && !sendwatchers);

					if (bFetchMsg) {
						// If new message is available, fetch it
						ptr[0] = 0;
						if (!(args = (fetchmsg_arg *)calloc(1, sizeof(*args)))) break;
						mir_strncpy(args->msgnum, pMsgNum, sizeof(args->msgnum)-1);
						args->getstatus = FALSE;
						//args->bIsRead = strncmp(ptr+8, "READ", 4) == 0;
						pthread_create((pThreadFunc)FetchMessageThreadSync, args);
						break;
					}
				}
			}
			if (bHasFileXfer && !strncmp(szSkypeMsg, "FILETRANSFER", 12))
				FXHandleMessage(szSkypeMsg+13);
			if (!strncmp(szSkypeMsg, "ERROR 68", 8)) {
				LOG(("We got a sync problem :( ->  SendMessage() will try to recover..."));
				break;
			}
			if (!strncmp(szSkypeMsg, "PROTOCOL ", 9)) {
				if ((protocol = (char)atoi(szSkypeMsg + 9)) >= 3) {
					mir_strcpy(cmdMessage, "CHATMESSAGE");
					mir_strcpy(cmdPartner, "FROM");
				}
				bProtocolSet = TRUE;

				if (protocol < 5 && !hMenuAddSkypeContact && db_get_b(NULL, SKYPE_PROTONAME, "EnableMenu", 1))
					hMenuAddSkypeContact = add_mainmenu();
			}
			LOG(("SkypeMsgAdd launched"));
			SkypeMsgAdd(szSkypeMsg);
			ReleaseSemaphore(SkypeMsgReceived, receivers, NULL);
		}
		break;

	case WM_TIMER:
		if (iReentranceCnt > 1) break;
		if (!bIsImoproxy) SkypeSend("PING");
		SkypeMsgCollectGarbage(MAX_MSG_AGE);
		MsgList_CollectGarbage();
		if (receivers > 1)
		{
			LOG(("Watchdog WARNING: there are still %d receivers waiting for MSGs", receivers));
		}
		break;

	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_DESTROY:
		KillTimer(hWndDlg, 1);
		break;
	case WM_COPYDATALOCAL:
		--iReentranceCnt;
		return WndProc(hWndDlg, WM_COPYDATA, wParam, lParam);

	default:
		if (message == ControlAPIAttach) {
			// Skype responds with Attach to the discover-message
			if ((HWND)wParam == hForbiddenSkypeWnd) {
				ResetEvent(SkypeReady);
				break;
			}
			AttachStatus = lParam;
			if (lParam == SKYPECONTROLAPI_ATTACH_SUCCESS) {
				LOG(("AttachStatus success, got hWnd %08X", (HWND)wParam));

				if (hSkypeWnd && (HWND)wParam != hSkypeWnd && IsWindow(hSkypeWnd))
					hSkypeWndSecondary = (HWND)wParam;
				else {
					hSkypeWnd = (HWND)wParam;	   // Skype gave us the communication window handle
					hSkypeWndSecondary = NULL;
				}
			}
			if (AttachStatus != SKYPECONTROLAPI_ATTACH_API_AVAILABLE &&
				AttachStatus != SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE)
			{
				LOG(("Attaching: SkypeReady fired, Attachstatus is %d", AttachStatus));
				SetEvent(SkypeReady);
			}
			AttachStatus = lParam;
			break;
		}
		--iReentranceCnt;
		return DefWindowProc(hWndDlg, message, wParam, lParam);
	}
	LOG(("WM_COPYDATA exit (%08X)", message));
	if (szSkypeMsg) free(szSkypeMsg);
	--iReentranceCnt;
	return 1;
}

void TellError(DWORD err) {
	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (TCHAR*)lpMsgBuf, _T("GetLastError"), MB_OK | MB_ICONINFORMATION);
	LocalFree(lpMsgBuf);
	return;
}


// SERVICES //
INT_PTR SkypeSetStatus(WPARAM wParam, LPARAM lParam)
{
	int oldStatus, iRet;
	BOOL UseCustomCommand, UnloadOnOffline;

	if (MirandaShuttingDown) return 0;
	LOG(("SkypeSetStatus enter"));
	UseCustomCommand = db_get_b(NULL, SKYPE_PROTONAME, "UseCustomCommand", 0);
	UnloadOnOffline = db_get_b(NULL, SKYPE_PROTONAME, "UnloadOnOffline", 0);

	//if (!SkypeInitialized && !db_get_b(NULL, SKYPE_PROTONAME, "UnloadOnOffline", 0)) return 0;

	// Workaround for Skype status-bug
	if (wParam == ID_STATUS_OFFLINE) logoff_contacts(TRUE);
	if (SkypeStatus == wParam) return 0;
	oldStatus = SkypeStatus;

	if (wParam == ID_STATUS_CONNECTING) return 0;
#ifdef MAPDND
	if (wParam == ID_STATUS_OCCUPIED || wParam == ID_STATUS_ONTHEPHONE) wParam = ID_STATUS_DND;
	if (wParam == ID_STATUS_OUTTOLUNCH) wParam = ID_STATUS_NA;
#endif
#ifdef MAPNA
	if (wParam==ID_STATUS_NA) wParam = ID_STATUS_AWAY;
#endif

	RequestedStatus = MirandaStatusToSkype((int)wParam);

	/*
	if (SkypeStatus != ID_STATUS_OFFLINE)
	{
	InterlockedExchange((long*)&SkypeStatus, wParam);
	ProtoBroadcastAck(SKYPE_PROTONAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, SkypeStatus);
	}
	*/

	if (wParam == ID_STATUS_OFFLINE && UnloadOnOffline)
	{
		if (UseCustomCommand)
		{
			DBVARIANT dbv;
			if (!db_get_s(NULL, SKYPE_PROTONAME, "CommandLine", &dbv))
			{
				CloseSkypeAPI(dbv.pszVal);
				db_free(&dbv);
			}
		}
		else CloseSkypeAPI(skype_path);
	}
	else if (AttachStatus == -1)
	{
		pthread_create(LaunchSkypeAndSetStatusThread, (void *)wParam);
		return 0;
	}

	iRet = SetUserStatus();
	LOG(("SkypeSetStatus exit"));
	return iRet;
}

void __stdcall SendBroadcast(MCONTACT hContact, int type, int result, HANDLE hProcess, LPARAM lParam)
{
	ACKDATA ack = { sizeof(ACKDATA) };
	ack.szModule = SKYPE_PROTONAME;
	ack.hContact = hContact;
	ack.type = type;
	ack.result = result;
	ack.hProcess = hProcess;
	ack.lParam = lParam;
	CallService(MS_PROTO_BROADCASTACK, 0, (LPARAM)&ack);
}

static void __cdecl SkypeGetAwayMessageThread(void *param)
{
	MCONTACT hContact = (MCONTACT) param;
	DBVARIANT dbv;
	if (!db_get_ts(hContact, "CList", "StatusMsg", &dbv)) {
		SendBroadcast(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)dbv.ptszVal);
		db_free(&dbv);
	}
	else SendBroadcast(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, 0);
}

INT_PTR SkypeGetAwayMessage(WPARAM, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA*)lParam;
	pthread_create(SkypeGetAwayMessageThread, (void*)ccs->hContact);
	return 1;
}

#define POLYNOMIAL (0x488781ED) /* This is the CRC Poly */
#define TOPBIT (1 << (WIDTH - 1)) /* MSB */
#define WIDTH 32

static int GetFileHash(char* filename)
{
	HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	int remainder = 0, byte, bit;
	char data[1024];
	DWORD dwRead;

	if (hFile == INVALID_HANDLE_VALUE) return 0;

	do
	{
		// Read file chunk
		dwRead = 0;
		ReadFile(hFile, data, 1024, &dwRead, NULL);

		/* loop through each byte of data */
		for (byte = 0; byte < (int)dwRead; ++byte) {
			/* store the next byte into the remainder */
			remainder ^= (data[byte] << (WIDTH - 8));
			/* calculate for all 8 bits in the byte */
			for (bit = 8; bit > 0; --bit) {
				/* check if MSB of remainder is a one */
				if (remainder & TOPBIT)
					remainder = (remainder << 1) ^ POLYNOMIAL;
				else
					remainder = (remainder << 1);
			}
		}
	} while (dwRead == 1024);

	CloseHandle(hFile);

	return remainder;
}

static int _GetFileSize(char* filename)
{
	HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;
	int size = GetFileSize(hFile, NULL);
	CloseHandle(hFile);
	return size;
}

/* RetrieveUserAvatar
 *
 * Purpose: Get a user avatar from skype itself
 * Params : param=(void *)(HANDLE)hContact
 */
void RetrieveUserAvatar(void *param)
{
	MCONTACT hContact = (MCONTACT)param;
	if (hContact == NULL)
		return;

	char AvatarFile[MAX_PATH + 1], AvatarTmpFile[MAX_PATH + 10], *pszTempFile;

	// Mount default ack
	ACKDATA ack = { 0 };
	ack.cbSize = sizeof(ACKDATA);
	ack.szModule = SKYPE_PROTONAME;
	ack.hContact = hContact;
	ack.type = ACKTYPE_AVATAR;
	ack.result = ACKRESULT_FAILED;

	PROTO_AVATAR_INFORMATION AI = { 0 };
	AI.cbSize = sizeof(AI);
	AI.hContact = hContact;

	// Get skype name
	DBVARIANT dbv;
	if (db_get_s(hContact, SKYPE_PROTONAME, SKYPE_NAME, &dbv) == 0)
	{
		if (dbv.pszVal)
		{
			// Get filename
			FoldersGetCustomPath(hProtocolAvatarsFolder, AvatarFile, sizeof(AvatarFile), DefaultAvatarsFolder);
			if (!*AvatarFile) mir_strcpy(AvatarFile, DefaultAvatarsFolder);
			mir_snprintf(AvatarTmpFile, SIZEOF(AvatarTmpFile), "AVATAR 1 %s\\%s_tmp.jpg", AvatarFile, dbv.pszVal);
			pszTempFile = AvatarTmpFile + 9;
			mir_snprintf(AvatarFile, SIZEOF(AvatarFile), "%s\\%s.jpg", AvatarFile, dbv.pszVal);

			// Just to be sure
			DeleteFileA(pszTempFile);
			HANDLE file = CreateFileA(pszTempFile, 0, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (file != INVALID_HANDLE_VALUE)
			{
				CloseHandle(file);
				char *ptr = SkypeGet("USER", dbv.pszVal, AvatarTmpFile);
				if (ptr)
				{
					if (strncmp(ptr, "ERROR", 5) &&
						GetFileAttributesA(pszTempFile) != INVALID_FILE_ATTRIBUTES)
					{
						ack.result = ACKRESULT_SUCCESS;

						// Is no avatar image?
						if (!db_get_b(NULL, SKYPE_PROTONAME, "ShowDefaultSkypeAvatar", 0)
							&& GetFileHash(pszTempFile) == 0x8d34e05d && _GetFileSize(pszTempFile) == 3751)
						{
							// Has no avatar
							AI.format = PA_FORMAT_UNKNOWN;
							ack.hProcess = (HANDLE)&AI;
							DeleteFileA(AvatarFile);
						}
						else
						{
							// Got it
							MoveFileExA(pszTempFile, AvatarFile, MOVEFILE_REPLACE_EXISTING);
							AI.format = PA_FORMAT_JPEG;
							mir_strncpy(AI.filename, AvatarFile,SIZEOF(AI.filename)-1);
							ack.hProcess = (HANDLE)&AI;
						}

					}
					free(ptr);
				}
				DeleteFileA(pszTempFile);
			}

		}
		db_free(&dbv);
	}
	CallService(MS_PROTO_BROADCASTACK, 0, (LPARAM)&ack);
}


/* SkypeGetAvatarInfo
 *
 * Purpose: Set user avatar in profile
 * Params : wParam=0
 *			lParam=(LPARAM)(const char*)filename
 * Returns: 0 - Success
 *		   -1 - Failure
 */
INT_PTR SkypeGetAvatarInfo(WPARAM wParam, LPARAM lParam)
{

	DBVARIANT dbv;
	PROTO_AVATAR_INFORMATION *AI = (PROTO_AVATAR_INFORMATION*)lParam;
	if (AI->hContact == NULL) // User
	{
		if (!db_get_s(NULL, SKYPE_PROTONAME, "AvatarFile", &dbv))
		{
			mir_strncpy(AI->filename, dbv.pszVal, sizeof(AI->filename));
			db_free(&dbv);
			return GAIR_SUCCESS;
		}
		else
			return GAIR_NOAVATAR;
	}
	else // Contact 
	{
		char AvatarFile[MAX_PATH + 1];

		if (protocol < 7 && !bIsImoproxy)
			return GAIR_NOAVATAR;

		if (wParam & GAIF_FORCE)
		{
			// Request anyway
			pthread_create(RetrieveUserAvatar, (void *)AI->hContact);
			return GAIR_WAITFOR;
		}

		if (db_get_s(AI->hContact, SKYPE_PROTONAME, SKYPE_NAME, &dbv))
			// No skype name ??
			return GAIR_NOAVATAR;

		if (dbv.pszVal == NULL)
		{
			// No skype name ??
			db_free(&dbv);
			return GAIR_NOAVATAR;
		}

		// Get filename
		FoldersGetCustomPath(hProtocolAvatarsFolder, AvatarFile, sizeof(AvatarFile), DefaultAvatarsFolder);
		mir_snprintf(AvatarFile, SIZEOF(AvatarFile), "%s\\%s.jpg", AvatarFile, dbv.pszVal);
		db_free(&dbv);

		// Check if the file exists
		if (GetFileAttributesA(AvatarFile) == INVALID_FILE_ATTRIBUTES)
			return GAIR_NOAVATAR;

		// Return the avatar
		AI->format = PA_FORMAT_JPEG;
		mir_strcpy(AI->filename, AvatarFile);
		return GAIR_SUCCESS;
	}
}


/* SkypeGetAvatarCaps
 *
 * Purpose: Query avatar caps for a protocol
 * Params : wParam=One of AF_*
 *			lParam=Depends on wParam
 * Returns: Depends on wParam
 */
INT_PTR SkypeGetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case AF_MAXSIZE:
	{
		POINT *p = (POINT *)lParam;
		if (p == NULL)
			return -1;

		p->x = 96;
		p->y = 96;
		return 0;
	}
	case AF_PROPORTION:
	{
		return PIP_NONE;
	}
	case AF_FORMATSUPPORTED:
	{
		if (lParam == PA_FORMAT_PNG || lParam == PA_FORMAT_JPEG)
			return TRUE;
		else
			return FALSE;
	}
	case AF_ENABLED:
	{
		return TRUE;
	}
	case AF_DONTNEEDDELAYS:
	{
		return FALSE;
	}
	}
	return -1;
}


INT_PTR SkypeGetStatus(WPARAM, LPARAM) {
	return SkypeStatus;
}

INT_PTR SkypeGetInfo(WPARAM, LPARAM lParam) {
	CCSDATA *ccs = (CCSDATA *)lParam;
	pthread_create(GetInfoThread, (void*)ccs->hContact);
	return 0;
}

INT_PTR SkypeAddToList(WPARAM wParam, LPARAM lParam) {
	LOG(("SkypeAddToList Adding API function called"));
	PROTOSEARCHRESULT *psr = (PROTOSEARCHRESULT*)lParam;
	if (psr->cbSize != sizeof(PROTOSEARCHRESULT) || !psr->nick) return 0;
	LOG(("SkypeAddToList OK"));
	return (INT_PTR)add_contact(_T2A(psr->nick), (DWORD)wParam);
}

INT_PTR SkypeBasicSearch(WPARAM, LPARAM lParam) {
	LOG(("SkypeBasicSearch %s", (char *)lParam));
	if (!SkypeInitialized) return 0;
	return (hSearchThread = pthread_create((pThreadFunc)BasicSearchThread, _strdup((char *)lParam)));
}

#ifdef USE_REAL_TS
static INT_PTR EventAddHook(WPARAM wParam, LPARAM lParam)
{	
	MCONTACT hContact = (MCONTACT)wParam;
	DBEVENTINFO *dbei=(DBEVENTINFO*)lParam;
	if (dbei && hContact == m_AddEventArg.hContact && dbei->eventType==EVENTTYPE_MESSAGE && (dbei->flags & DBEF_SENT) &&
		mir_strcmp(dbei->szModule, SKYPE_PROTONAME) == 0) {
		dbei->timestamp = m_AddEventArg.timestamp;
	}
	return 0;
}
#endif

void MessageSendWatchThread(void *a) {
	char *str, *err, *ptr, *nexttoken;
	HANDLE hDBAddEvent = NULL;
	msgsendwt_arg *arg = (msgsendwt_arg*)a;

	LOG(("MessageSendWatchThread started."));

	if (db_get_b(NULL, SKYPE_PROTONAME, "NoAck", 1))
	{
		ProtoBroadcastAck(SKYPE_PROTONAME, arg->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)1, 0);
		str=NULL;
	}
	else
		str = SkypeRcvMsg(arg->szId, SkypeTime(NULL) - 1, arg->hContact, db_get_dw(NULL, "SRMsg", "MessageTimeout", TIMEOUT_MSGSEND));

	InterlockedDecrement(&sendwatchers);
	if (str)
	{
		if (!db_get_b(arg->hContact, SKYPE_PROTONAME, "ChatRoom", 0)) {
			if (err = GetSkypeErrorMsg(str)) {
				ProtoBroadcastAck(SKYPE_PROTONAME, arg->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)1, (LPARAM)Translate(err));
				free(err);
				free(str);
				free(arg);
				LOG(("MessageSendWatchThread terminated."));
				return;
			}
			/* The USE_REAL_TS code would correct our Sent-Timestamp to the real time that the
			 * event was sent according to the clock of the machine Skype is running on.
			 * However msg-Dialog has problems with this.
			 */
#ifdef USE_REAL_TS
			EnterCriticalSection(&AddEventMutex);
#endif
			if ((ptr=strtok_r(str, " ", &nexttoken)) && (*ptr!='#' || (ptr=strtok_r(NULL, " ", &nexttoken))) &&
				(ptr=strtok_r(NULL, " ", &nexttoken))) {
				/* Use this to ensure that main thread doesn't pick up sent message */
				MsgList_Add(strtoul(ptr, NULL, 10), -1);
#ifdef USE_REAL_TS
				if (err=SkypeGet (cmdMessage, ptr, "TIMESTAMP")) {
					m_AddEventArg.hContact = arg->hContact;
					m_AddEventArg.timestamp = atoi(err);
					free(err);
					hDBAddEvent = HookEvent(ME_DB_EVENT_FILTER_ADD,EventAddHook);
				}
#endif
			}
			ProtoBroadcastAck(SKYPE_PROTONAME, arg->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)1, 0);
#ifdef USE_REAL_TS
			if (hDBAddEvent) UnhookEvent(hDBAddEvent);
			LeaveCriticalSection(&AddEventMutex);
#endif
		}
		free(str);
		LOG(("MessageSendWatchThread terminated gracefully."));
	}
	free(arg);
}

INT_PTR SkypeSendMessage(WPARAM, LPARAM lParam) {
	CCSDATA *ccs = (CCSDATA *)lParam;
	DBVARIANT dbv;
	char *msg = (char *)ccs->lParam, *mymsgcmd = cmdMessage, szId[16] = { 0 };
	static DWORD dwMsgNum = 0;
	BYTE bIsChatroom = 0 != db_get_b(ccs->hContact, SKYPE_PROTONAME, "ChatRoom", 0);

	if (bIsChatroom)
	{
		if (db_get_s(ccs->hContact, SKYPE_PROTONAME, "ChatRoomID", &dbv))
			return 0;
		mymsgcmd = "CHATMESSAGE";
	}
	else
	{
		if (db_get_s(ccs->hContact, SKYPE_PROTONAME, SKYPE_NAME, &dbv))
			return 0;
		mymsgcmd = "MESSAGE";
	}

	if (protocol >= 4) {
		InterlockedIncrement((LONG*)&dwMsgNum);
		sprintf(szId, "#M%d ", dwMsgNum++);
	}
	InterlockedIncrement(&sendwatchers);
	bool sendok = true;
	if (!msg || SkypeSend("%s%s %s %s", szId, mymsgcmd, dbv.pszVal, msg))
		sendok = false;
	db_free(&dbv);

	if (sendok) {
		msgsendwt_arg *psendarg = (msgsendwt_arg*)calloc(1, sizeof(msgsendwt_arg));

		if (psendarg) {
			psendarg->hContact = ccs->hContact;
			mir_strcpy(psendarg->szId, szId);
			pthread_create(MessageSendWatchThread, psendarg);
		}
		else
			InterlockedDecrement(&sendwatchers);
		return 1;
	}
	else
		InterlockedDecrement(&sendwatchers);
	if (!bIsChatroom)
		ProtoBroadcastAck(SKYPE_PROTONAME, ccs->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)1, (LPARAM)Translate("Connection to Skype lost"));
	return 0;
}

INT_PTR SkypeRecvMessage(WPARAM, LPARAM lParam)
{
	DBEVENTINFO dbei = { 0 };
	CCSDATA *ccs = (CCSDATA *)lParam;
	PROTORECVEVENT *pre = (PROTORECVEVENT *)ccs->lParam;

	db_unset(ccs->hContact, "CList", "Hidden");
	dbei.cbSize = sizeof(dbei);
	dbei.szModule = SKYPE_PROTONAME;
	dbei.timestamp = pre->timestamp;
	if (pre->flags & PREF_CREATEREAD)
		dbei.flags |= DBEF_READ;
	dbei.flags |= DBEF_UTF;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = (int)mir_strlen(pre->szMessage) + 1;
	dbei.pBlob = (PBYTE)pre->szMessage;
	MsgList_Add((DWORD)pre->lParam, db_event_add(ccs->hContact, &dbei));
	return 0;
}

INT_PTR SkypeUserIsTyping(WPARAM wParam, LPARAM lParam) {
	if (protocol < 5 && !bIsImoproxy) return 0;

	DBVARIANT dbv = { 0 };
	MCONTACT hContact = (MCONTACT)wParam;
	if (db_get_s(hContact, SKYPE_PROTONAME, "Typing_Stream", &dbv)) {
		if (db_get_s(hContact, SKYPE_PROTONAME, SKYPE_NAME, &dbv) == 0) {
			char szCmd[256];
			_snprintf(szCmd, sizeof(szCmd),
				"ALTER APPLICATION libpurple_typing CONNECT %s", dbv.pszVal);
			SkypeSend(szCmd);
			db_free(&dbv);
			testfor(szCmd, 2000);
			// TODO: We should somehow cache the typing notify result and send it
			// after we got a connection, but in the meantime this notification won't
			// get sent on first run
		}
		return 0;
	}

	char szWhat[128];
	sprintf(szWhat, "ALTER APPLICATION libpurple_typing DATAGRAM %s", dbv.pszVal);
	SkypeSend("%s %s", szWhat,
		(lParam == PROTOTYPE_SELFTYPING_ON ? "PURPLE_TYPING" : "PURPLE_NOT_TYPING"));
	testfor(szWhat, 2000);
	db_free(&dbv);
	return 0;
}


INT_PTR SkypeSendAuthRequest(WPARAM, LPARAM lParam) {
	CCSDATA* ccs = (CCSDATA*)lParam;
	DBVARIANT dbv;
	if (!ccs->lParam || db_get_s(ccs->hContact, SKYPE_PROTONAME, SKYPE_NAME, &dbv))
		return 1;
	int retval = SkypeSend("SET USER %s BUDDYSTATUS 2 %s", dbv.pszVal, (char *)ccs->lParam);
	db_free(&dbv);
	if (retval) return 1; else return 0;
}

INT_PTR SkypeRecvAuth(WPARAM, LPARAM lParam) {
	CCSDATA *ccs = (CCSDATA*)lParam;
	PROTORECVEVENT *pre = (PROTORECVEVENT*)ccs->lParam;

	db_unset(ccs->hContact, "CList", "Hidden");

	DBEVENTINFO dbei = { 0 };
	dbei.cbSize = sizeof(dbei);
	dbei.szModule = SKYPE_PROTONAME;
	dbei.timestamp = pre->timestamp;
	dbei.flags = DBEF_UTF | ((pre->flags & PREF_CREATEREAD) ? DBEF_READ : 0);
	dbei.eventType = EVENTTYPE_AUTHREQUEST;
	dbei.cbBlob = (int)pre->lParam;
	dbei.pBlob = (PBYTE)pre->szMessage;

	db_event_add(NULL, &dbei);
	return 0;
}

char *__skypeauth(WPARAM wParam) {
	if (!SkypeInitialized) return NULL;

	DBEVENTINFO dbei = { 0 };
	dbei.cbSize = sizeof(dbei);
	if (((dbei.cbBlob = db_event_getBlobSize(wParam)) == -1 ||
		!(dbei.pBlob = (unsigned char*)malloc(dbei.cbBlob))))
	{
		return NULL;
	}

	if (db_event_get(wParam, &dbei) || dbei.eventType != EVENTTYPE_AUTHREQUEST || mir_strcmp(dbei.szModule, SKYPE_PROTONAME))
	{
		free(dbei.pBlob);
		return NULL;
	}
	return (char *)dbei.pBlob;
}

INT_PTR SkypeAuthAllow(WPARAM wParam, LPARAM) {
	char *pBlob = __skypeauth(wParam);
	if (pBlob)
	{
		int retval = SkypeSend("SET USER %s ISAUTHORIZED TRUE", pBlob + sizeof(DWORD) + sizeof(HANDLE));
		free(pBlob);
		if (!retval) return 0;
	}
	return 1;
}

INT_PTR SkypeAuthDeny(WPARAM wParam, LPARAM) {
	char *pBlob = __skypeauth(wParam);
	if (pBlob)
	{
		int retval = SkypeSend("SET USER %s ISAUTHORIZED FALSE", pBlob + sizeof(DWORD) + sizeof(HANDLE));
		free(pBlob);
		if (!retval) return 0;
	}
	return 1;
}


INT_PTR SkypeAddToListByEvent(WPARAM wParam, LPARAM) {
	char *pBlob = __skypeauth(wParam);
	if (pBlob)
	{
		MCONTACT hContact = add_contact(pBlob + sizeof(DWORD) + sizeof(HANDLE), LOWORD(wParam));
		free(pBlob);
		if (hContact) return (INT_PTR)hContact;
	}
	return 0;
}

INT_PTR SkypeRegisterProxy(WPARAM, LPARAM lParam) {
	if (!lParam) {
		free(pszProxyCallout);
		pszProxyCallout = NULL;
	}
	pszProxyCallout = _strdup((char*)lParam);
	bIsImoproxy = TRUE;
	return 0;
}


void CleanupNicknames(char *) {

	LOG(("CleanupNicknames Cleaning up..."));
	for (MCONTACT hContact = db_find_first(SKYPE_PROTONAME); hContact != NULL; hContact = db_find_next(hContact,SKYPE_PROTONAME)) {
		if (db_get_b(hContact, SKYPE_PROTONAME, "ChatRoom", 0) == 0)
		{
			DBVARIANT dbv;
			if (!db_get_s(hContact, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) {
				DBVARIANT dbv2;
				if (!db_get_s(hContact, SKYPE_PROTONAME, "Nick", &dbv2)) {
					db_unset(hContact, SKYPE_PROTONAME, "Nick");
					GetInfoThread((void*)hContact);
					db_free(&dbv2);
				}
				db_free(&dbv);
			}
		}
	}
	OUTPUT(TranslateT("Cleanup finished."));
}

/////////////////////////////////////////////////////////////////////////////////////////
// EnterBitmapFileName - enters a bitmap filename

int __stdcall EnterBitmapFileName(char* szDest)
{
	char szFilter[512];
	OPENFILENAMEA ofn = { 0 };
	*szDest = 0;

	CallService(MS_UTILS_GETBITMAPFILTERSTRINGS, sizeof szFilter, (LPARAM)szFilter);
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFilter = szFilter;
	ofn.lpstrFile = szDest;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrDefExt = "bmp";
	if (!GetOpenFileNameA(&ofn))
		return 1;

	return ERROR_SUCCESS;
}

int MirandaExit(WPARAM, LPARAM) {
	MirandaShuttingDown = TRUE;
	return 0;
}

int OkToExit(WPARAM, LPARAM) {
	//	logoff_contacts();
	MirandaShuttingDown = TRUE;

	// Trigger all semaphores and events just to be sure that there is no deadlock
	ReleaseSemaphore(SkypeMsgReceived, receivers, NULL);
	SetEvent(SkypeReady);
	SetEvent(MessagePumpReady);
#ifdef SKYPEBUG_OFFLN
	SetEvent(GotUserstatus);
#endif
	SetEvent(hBuddyAdded);

	SkypeFlush();
	PostMessage(g_hWnd, WM_CLOSE, 0, 0);
	return 0;
}


struct PLUGINDI {
	char **szSettings;
	int dwCount;
};

// Taken from pluginopts.c and modified
int EnumOldPluginName(const char *szSetting, LPARAM lParam)
{
	struct PLUGINDI *pdi = (struct PLUGINDI*)lParam;
	if (pdi && lParam) {
		pdi->szSettings = (char**)realloc(pdi->szSettings, (pdi->dwCount + 1)*sizeof(char*));
		pdi->szSettings[pdi->dwCount++] = _strdup(szSetting);
	}
	return 0;
}

// Are there any Skype users on list? 
// 1 --> Yes
// 0 --> No
int AnySkypeusers(void)
{
	DBVARIANT dbv;
	int tCompareResult;

	// already on list?
	for (MCONTACT hContact = db_find_first();
		hContact != NULL;
		hContact = db_find_next(hContact))
	{
		// GETCONTACTBASEPROTO doesn't work on not loaded protocol, therefore get 
		// protocol from DB
		if (db_get_s(hContact, "Protocol", "p", &dbv)) continue;
		tCompareResult = !mir_strcmp(dbv.pszVal, SKYPE_PROTONAME);
		db_free(&dbv);
		if (tCompareResult) return 1;
	}
	return 0;
}


/*void UpgradeName(char *OldName)
{
DBCONTACTENUMSETTINGS cns;
DBCONTACTWRITESETTING cws;
DBVARIANT dbv;
MCONTACT hContact=NULL;
struct PLUGINDI pdi;

LOG(("Updating old database settings if there are any..."));
cns.pfnEnumProc=EnumOldPluginName;
cns.lParam=(LPARAM)&pdi;
cns.szModule=OldName;
cns.ofsSettings=0;

hContact = db_find_first();

for ( ;; ) {
memset(&pdi,0,sizeof(pdi));
CallService(MS_DB_CONTACT_ENUMSETTINGS,hContact,(LPARAM)&cns);
// Upgrade Protocol settings to new string
if (pdi.szSettings) {
int i;

LOG(("We're currently upgrading..."));
for (i=0;i<pdi.dwCount;i++) {
if (!db_get_s(hContact, OldName, pdi.szSettings[i], &dbv)) {
cws.szModule=SKYPE_PROTONAME;
cws.szSetting=pdi.szSettings[i];
cws.value=dbv;
if (!CallService(MS_DB_CONTACT_WRITESETTING,hContact,(LPARAM)&cws))
db_unset(hContact,OldName,pdi.szSettings[i]);
db_free(&dbv);
}
free(pdi.szSettings[i]);
}
free(pdi.szSettings);
}
// Upgrade Protocol assignment, if we are not main contact
if (hContact && !db_get_s(hContact, "Protocol", "p", &dbv)) {
if (!mir_strcmp(dbv.pszVal, OldName))
db_set_s(hContact, "Protocol", "p", SKYPE_PROTONAME);
db_free(&dbv);
}
if (!hContact) break;
hContact = db_find_next(hContact);
}

db_set_b(NULL, SKYPE_PROTONAME, "UpgradeDone", (BYTE)1);
return;
}*/

void __cdecl MsgPump(char *)
{
	MSG msg;

	WNDCLASS WndClass;

	// Create window class
	WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInst;
	WndClass.hIcon = NULL;
	WndClass.hCursor = NULL;
	WndClass.hbrBackground = NULL;
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = _T("SkypeApiDispatchWindow");
	RegisterClass(&WndClass);
	// Do not check the retval of RegisterClass, because on non-unicode
	// win98 it will fail, as it is a stub that returns false() there

	// Create main window
	g_hWnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
		_T("SkypeApiDispatchWindow"), _T(""), WS_BORDER | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 128, 128, NULL, 0, (HINSTANCE)WndClass.hInstance, 0);

	LOG(("Created Dispatch window with handle %08X", (long)g_hWnd));
	if (!g_hWnd) {
		OUTPUT(TranslateT("Cannot create window."));
		TellError(GetLastError());
		SetEvent(MessagePumpReady);
		return;
	}
	ShowWindow(g_hWnd, 0);
	UpdateWindow(g_hWnd);
	msgPumpThreadId = GetCurrentThreadId();
	SetEvent(MessagePumpReady);

	LOG(("Messagepump started."));
	while (GetMessage(&msg, NULL, 0, 0) > 0 && !Miranda_Terminated()) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	UnregisterClass(WndClass.lpszClassName, hInst);
	LOG(("Messagepump stopped."));
}

// DLL Stuff //

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MUUID_SKYPE_CALL, MIID_LAST };

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD/* fdwReason*/, LPVOID /*lpvReserved*/)
{
	hInst = hinstDLL;
	return TRUE;
}


int PreShutdown(WPARAM, LPARAM) {
	PostThreadMessage(msgPumpThreadId, WM_QUIT, 0, 0);
	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	DWORD Buffsize;
	HKEY MyKey;
	BOOL SkypeInstalled;
	WSADATA wsaData;

	mir_getLP(&pluginInfo);

	// RM: commented so it will always use predefined name - or was this really needed?
	///GetModuleFileNameA( hInst, path, sizeof( path ));
	///_splitpath (path, NULL, NULL, SKYPE_PROTONAME, NULL);
	///CharUpperA( SKYPE_PROTONAME );

	InitializeCriticalSection(&RingAndEndcallMutex);
	InitializeCriticalSection(&QueryThreadMutex);
	InitializeCriticalSection(&TimeMutex);
#ifdef USE_REAL_TS
	InitializeCriticalSection(&AddEventMutex);
#endif


#ifdef _DEBUG
	init_debug();
#endif

	LOG(("Load: Skype Plugin loading..."));

	// We need to upgrade SKYPE_PROTOCOL internal name to Skype if not already done
	/*	if (!db_get_b(NULL, SKYPE_PROTONAME, "UpgradeDone", 0))
			UpgradeName("SKYPE_PROTOCOL");*/

	// Initialisation of Skype MsgQueue must be done because of Cleanup in end and
	// Mutex is also initialized here.
	LOG(("SkypeMsgInit initializing Skype MSG-queue"));
	if (SkypeMsgInit() == -1) {
		OUTPUT(TranslateT("Memory allocation error on startup."));
		return 0;
	}

	// On first run on new profile, ask user, if he wants to enable the plugin in
	// this profile
	// --> Fixing Issue #0000006 from bugtracker.
	if (!db_get_b(NULL, SKYPE_PROTONAME, "FirstRun", 0)) {
		db_set_b(NULL, SKYPE_PROTONAME, "FirstRun", 1);
		if (AnySkypeusers() == 0) // First run, it seems :)
			if (MessageBox(NULL, TranslateT("This seems to be the first time that you're running the Skype protocol plugin. Do you want to enable the protocol for this Miranda profile? If you chose NO, you can always enable it in the plugin options later."), _T("Welcome!"), MB_ICONQUESTION | MB_YESNO) == IDNO) {
			char path[MAX_PATH], *filename;
			GetModuleFileNameA(hInst, path, sizeof(path));
			if (filename = strrchr(path, '\\') + 1)
				db_set_b(NULL, "PluginDisable", filename, 1);
			return 0;
			}
	}


	// Check if Skype is installed
	SkypeInstalled = TRUE;
	BOOL UseCustomCommand = (BYTE)db_get_b(NULL, SKYPE_PROTONAME, "UseCustomCommand", 0);
	UseSockets = (BOOL)db_get_b(NULL, SKYPE_PROTONAME, "UseSkype2Socket", 0);

	if (!UseSockets && !UseCustomCommand)
	{
		if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Skype\\Phone"), 0, KEY_READ, &MyKey) != ERROR_SUCCESS)
		{
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\Skype\\Phone"), 0, KEY_READ, &MyKey) != ERROR_SUCCESS)
			{
				SkypeInstalled = FALSE;
			}
		}

		Buffsize = sizeof(skype_path);

		if (SkypeInstalled == FALSE || RegQueryValueExA(MyKey, "SkypePath", NULL, NULL, (unsigned char *)skype_path, &Buffsize) != ERROR_SUCCESS)
		{
			//OUTPUT("Skype was not found installed :( \nMaybe you are using portable Skype.");
			RegCloseKey(MyKey);
			skype_path[0] = 0;
			//return 0;
		}
		RegCloseKey(MyKey);
	}
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// Start Skype connection 
	if (!(ControlAPIAttach = RegisterWindowMessage(_T("SkypeControlAPIAttach"))) || !(ControlAPIDiscover = RegisterWindowMessage(_T("SkypeControlAPIDiscover"))))
	{
		OUTPUT(TranslateT("Cannot register window message."));
		return 0;
	}

	SkypeMsgReceived = CreateSemaphore(NULL, 0, MAX_MSGS, NULL);
	if (!(SkypeReady = CreateEvent(NULL, TRUE, FALSE, NULL)) ||
		!(MessagePumpReady = CreateEvent(NULL, FALSE, FALSE, NULL)) ||
#ifdef SKYPEBUG_OFFLN
		!(GotUserstatus = CreateEvent(NULL, TRUE, FALSE, NULL)) ||
#endif
		!(hBuddyAdded = CreateEvent(NULL, FALSE, FALSE, NULL)) ||
		!(FetchMessageEvent = CreateEvent(NULL, FALSE, TRUE, NULL))) {
		OUTPUT(TranslateT("Unable to create mutex!"));
		return 0;
	}

	/* Register the module */
	PROTOCOLDESCRIPTOR pd = { PROTOCOLDESCRIPTOR_V3_SIZE };
	pd.szName = SKYPE_PROTONAME;
	pd.type = PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	CreateServices();
	HookEvents();
	InitVSApi();
	MsgList_Init();

	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);

	// Startup Message-pump
	pthread_create((pThreadFunc)MsgPump, NULL);
	WaitForSingleObject(MessagePumpReady, INFINITE);
	return 0;
}



extern "C" int __declspec(dllexport) Unload(void)
{
	BOOL UseCustomCommand = db_get_b(NULL, SKYPE_PROTONAME, "UseCustomCommand", 0);
	BOOL Shutdown = db_get_b(NULL, SKYPE_PROTONAME, "Shutdown", 0);

	LOG(("Unload started"));

	if (Shutdown && (skype_path[0] || UseCustomCommand)) {

		if (UseCustomCommand)
		{
			DBVARIANT dbv;
			if (!db_get_s(NULL, SKYPE_PROTONAME, "CommandLine", &dbv))
			{
				char szAbsolutePath[MAX_PATH];

				TranslateMirandaRelativePathToAbsolute(dbv.pszVal, szAbsolutePath, FALSE);
				_spawnl(_P_NOWAIT, szAbsolutePath, szAbsolutePath, "/SHUTDOWN", NULL);
				LOG(("Unload Sent /shutdown to %s", szAbsolutePath));
				db_free(&dbv);
			}
		}
		else
		{
			_spawnl(_P_NOWAIT, skype_path, skype_path, "/SHUTDOWN", NULL);
			LOG(("Unload Sent /shutdown to %s", skype_path));
		}

	}
	SkypeMsgCleanup();
	//WSACleanup();
	FreeVSApi();
	DestroyHookableEvent(hInitChat);
	GCExit();
	MsgList_Exit();

	CloseHandle(SkypeReady);
	CloseHandle(SkypeMsgReceived);
#ifdef SKYPEBUG_OFFLN
	CloseHandle(GotUserstatus);
#endif
	CloseHandle(MessagePumpReady);
	CloseHandle(hBuddyAdded);
	CloseHandle(FetchMessageEvent);

	DeleteCriticalSection(&RingAndEndcallMutex);
	DeleteCriticalSection(&QueryThreadMutex);

	SkypeRegisterProxy(0, 0);
	LOG(("Unload: Shutdown complete"));
#ifdef _DEBUG
	end_debug();
#endif
#ifdef USE_REAL_TS
	DeleteCriticalSection(&AddEventMutex);
#endif
	DeleteCriticalSection(&TimeMutex);
	return 0;
}

