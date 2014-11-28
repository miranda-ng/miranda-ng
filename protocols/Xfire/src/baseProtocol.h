/*
 *  Plugin of miranda IM(ICQ) for Communicating with users of the XFire Network.
 *
 *  Copyright (C) 2010 by
 *          dufte <dufte@justmail.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 *  Based on J. Lawler              - BaseProtocol
 *			 Herbert Poul/Beat Wolf - xfirelib
 *
 *  Miranda ICQ: the free icq client for MS Windows
 *  Copyright (C) 2000-2008  Richard Hughes, Roland Rabien & Tristan Van de Vreede
 *
 */

//=====================================================
//	Includes (yea why not include lots of stuff :D )
//=====================================================

#pragma once

#ifndef _BASEPROTO_H
#define _BASEPROTO_H

#include "services.h"

//=======================================================
//	Definitions
//=======================================================
#define protocolname		"XFire" //no spaces here :)
#define PLUGIN_TITLE		LPGEN("XFire Protocol")

//=======================================================
//	Defines
//=======================================================
//General
extern HINSTANCE hinstance;
extern int bpStatus;

//Services.c
INT_PTR GetCaps(WPARAM wParam, LPARAM lParam);
INT_PTR GetName(WPARAM wParam, LPARAM lParam);
INT_PTR SetStatus(WPARAM wParam, LPARAM lParam);
INT_PTR GetStatus(WPARAM wParam, LPARAM lParam);
INT_PTR TMLoadIcon(WPARAM wParam, LPARAM lParam);
INT_PTR SetNickName(WPARAM newnick, LPARAM lparam);

BOOL IsXFireContact(MCONTACT h);
int displayPopup(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType, HICON hhicon = NULL);
BOOL CreateToolTip(int toolID, HWND hDlg, CHAR* pText);
void EnableDlgItem(HWND hwndDlg, UINT control, int state);
BOOL str_replace(char*src, char*find, char*rep);
extern void Message(LPVOID msg);
extern void MessageE(LPVOID msg);
char* GetLaunchPath(char*launch);
unsigned short r(unsigned short data);
BOOL GetServerIPPort(DWORD pid, char*localaddrr, unsigned long localaddr, char*ip1, char*ip2, char*ip3, char*ip4, long*port);
BOOL GetServerIPPort2(DWORD pid, char*localaddrr, unsigned long localaddr, char*ip1, char*ip2, char*ip3, char*ip4, long*port);
BOOL FindTeamSpeak(DWORD*pid, int*vid);
char*menuitemtext(char*mtext);
BOOL checkCommandLine(HANDLE hProcess, char * mustcontain, char * mustnotcontain);
char * getItem(char * string, char delim, int count);
BOOL GetWWWContent(char*host, char* request, char*filename, BOOL dontoverwrite);
BOOL GetWWWContent2(char*address, char*filename, BOOL dontoverwrite, char**tobuf = NULL, unsigned int* size = NULL);
BOOL CheckWWWContent(char*address);
unsigned int getfilesize(char*path);
void UpdateMyXFireIni(LPVOID dummy);
void UpdateMyIcons(LPVOID dummy);
BOOL IsContactMySelf(std::string buddyusername);
DWORD xfire_GetPrivateProfileString(__in   LPCSTR lpAppName, __in   LPCSTR lpKeyName, __in   LPCSTR lpDefault, __out  LPSTR lpReturnedString, __in   DWORD nSize, __in   LPCSTR lpFileName);
BOOL mySleep(int ms, HANDLE evt);
void __stdcall XFireLog(const char* fmt, ...);

#define ID_STATUS_RECONNECT ID_STATUS_OFFLINE-1

struct CONTACT // Contains info about users
{
	TCHAR* name;
	TCHAR* user;
	TCHAR* host;
	bool ExactOnly;
	bool ExactWCOnly;
	bool ExactNick;
};


#define MSGBOX(msg) mir_forkthread(Message,(LPVOID)msg)
#define MSGBOXE(msg)  mir_forkthread(MessageE,(LPVOID)msg)

#pragma comment(lib,"Advapi32.lib")
#pragma comment(lib,"Psapi.lib")
#pragma comment(lib,"gdiplus.lib")
#pragma comment(lib,"comdlg32.lib")
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"shell32.lib")

//=====================================================
//	Definitions
//=====================================================

struct GameIco {
	int gameid;
	HANDLE handle;
	HICON hicon;
};

struct XFireContact {
	char * username;
	char * nick;
	int id;
	int sid;
};

struct XFireAvatar {
	int type;
	char backup[256];
	char file[256];
	char rfile[256];
};

struct XFire_FoundGame
{
	int gameid;
	int gameid2;
	short send_gameid;
	char path[256];
	char mpath[9][256];
	int morepaths;
	char launchparams[1024];
	char networkparams[1024];
	char userparams[1024];
	char pwparams[1024];
	char mustcontain[1024];
	char notcontain[1024];
	char temp[128];
	BOOL setstatusmsg;
	BOOL custom;
	BOOL skip;
	BOOL noicqstatus;
	HANDLE menuitem;
};

struct XFire_SetAvatar
{
	MCONTACT hContact;
	char* username;
};

struct gServerstats {
	int players;
	int maxplayers;
	char name[512];
	char map[512];
	char gametype[512];
	char fgametype[512];
	BOOL password;
};

struct GameServerQuery_query {
	int xfiregameid;
	char ip[16];
	WORD port;
	HANDLE handle; //will be overwritten
	int queryengine; // immer 0
};

#define XFIRE_MAX_STATIC_STRING_LEN 1024
#define XFIRE_SCAN_VAL 0x3

#define XFIRE_GAME_ICON 0
#define XFIRE_VOICE_ICON 1


typedef struct {
	int cbSize;
	char *szProto; // pointer to protocol modulename (NULL means global)
	union
	{
		char *szMsg;
		WCHAR *wszMsg;
		TCHAR *tszMsg;
	}; // pointer to the status message _format_ (i.e. it's an unparsed message containing variables, in any case. NAS takes care of parsing) (may be NULL - means that there's no specific message for this protocol - then the global status message will be used)
	/*
		Be aware that MS_NAS_GETSTATE allocates memory for szMsg through Miranda's
		memory management interface (MS_SYSTEM_GET_MMI). And MS_NAS_SETSTATE
		expects szMsg to be allocated through the same service. MS_NAS_SETSTATE deallocates szMsg.
		*/
	WORD status; // status mode. 0 means current (NAS will overwrite 0 with the current status mode)
	// for MS_NAS_GETSTATE if the specified status is not 0, MS_NAS_GETSTATE will return the default/last status message (depends on settings) - i.e. the same message that will be shown by default when user changes status to the specified one. please note that, for example, if current status mode is ID_STATUS_AWAY, then status messages returned by MS_NAS_GETSTATE for status=0 and status=ID_STATUS_AWAY may be different! for status=ID_STATUS_AWAY it always returns the default/last status message, and for status=0 it returns _current_ status message.
	int Flags;
} NAS_PROTOINFO;

#endif