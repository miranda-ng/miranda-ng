/*
IRC plugin for Miranda IM

Copyright (C) 2003-05 Jurgen Persson
Copyright (C) 2007-09 George Hazan

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

#ifndef _IRCWIN_H_
#define _IRCWIN_H_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <objbase.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <malloc.h>
#include <math.h>
#include <winsock.h>
#include <commctrl.h>
#include <time.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "newpluginapi.h"
#include "m_system.h"
#include "m_protocols.h"
#include "m_protosvc.h"
#include "m_protoint.h"
#include <m_clistint.h>
#include "m_options.h"
#include "m_database.h"
#include "m_utils.h"
#include "m_skin.h"
#include "m_netlib.h"
#include "m_langpack.h"
#include "m_chat_int.h"
#include "m_message.h"
#include "m_userinfo.h"
#include "m_contacts.h"
#include "m_button.h"
#include "m_genmenu.h"
#include "m_file.h"
#include "m_ignore.h"
#include "m_chat_int.h"
#include "m_icolib.h"
#include "m_string.h"
#include "m_gui.h"

#include "resource.h"

#define IRC_JOINCHANNEL       "/JoinChannelMenu"
#define IRC_CHANGENICK        "/ChangeNickMenu"
#define IRC_SHOWLIST          "/ShowListMenu"
#define IRC_SHOWSERVER        "/ShowServerMenu"
#define IRC_UM_CHANSETTINGS   "/UMenuChanSettings"
#define IRC_UM_WHOIS          "/UMenuWhois"
#define IRC_UM_DISCONNECT     "/UMenuDisconnect"
#define IRC_UM_IGNORE         "/UMenuIgnore"

#define STR_QUITMESSAGE  L"Miranda NG! Smaller, Faster, Easier. https://miranda-ng.org/"
#define STR_USERINFO     L"I'm a happy Miranda NG user! Get it here: https://miranda-ng.org/"
#define STR_AWAYMESSAGE  L"I'm away from the computer." // Default away
#define DCCSTRING        L" (DCC)"
#define SERVERSMODULE    "IRC Servers"
#define SERVERWINDOW	    L"Network log"

#define DCC_CHAT		1
#define DCC_SEND		2

#define FILERESUME_CANCEL	11

struct CIrcProto;

// special service for tweaking performance, implemented in chat.dll
#define MS_GC_GETEVENTPTR  "GChat/GetNewEventPtr"
typedef int (*GETEVENTFUNC)(WPARAM wParam, LPARAM lParam);
typedef struct
{
	GETEVENTFUNC pfnAddEvent;
}
GCPTRS;

#define IP_AUTO       1
#define IP_MANUAL     2

struct IPRESOLVE      // Contains info about the channels
{
	IPRESOLVE(const char* _addr, int _type) :
		sAddr(_addr),
		iType(_type)
	{
	}

	~IPRESOLVE()
	{
	}

	CMStringA     sAddr;
	int        iType;
};

struct CHANNELINFO   // Contains info about the channels
{
	wchar_t* pszTopic;
	wchar_t* pszMode;
	wchar_t* pszPassword;
	wchar_t* pszLimit;
	BYTE   OwnMode;	/* own mode on the channel. Bitmask:
												0: voice
												1: halfop
												2: op
												3: admin
												4: owner		*/
	int    codepage;
};

struct PERFORM_INFO  // Contains 'm_perform buffer' for different networks
{
	PERFORM_INFO(const char* szSetting, const wchar_t* value) :
		mSetting(szSetting),
		mText(value)
	{
	}

	~PERFORM_INFO()
	{
	}

	CMStringA mSetting;
	CMStringW mText;
};

struct CONTACT // Contains info about users
{
	const wchar_t *name;
	const wchar_t *user;
	const wchar_t *host;
	bool ExactOnly;
	bool ExactWCOnly;
	bool ExactNick;
};

struct TDbSetting
{
	int    offset;
	char*  name;
	int    type;
	size_t size;
	union
	{
		int    defValue;
		wchar_t* defStr;
	};
};

#include "irclib.h"
using namespace irc;

#include "irc_dlg.h"

#include "ircproto.h"

// map actual member functions to their associated IRC command.
// put any number of this macro in the class's constructor.
#define	IRC_MAP_ENTRY(name, member)	\
	m_handlers.insert( new CIrcHandler( L##name, &CIrcProto::OnIrc_##member ));

/////////////////////////////////////////////////////////////////////////////////////////
// Functions

// main.cpp

CIrcProto* GetTimerOwner(UINT_PTR eventId);

VOID CALLBACK IdentTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
VOID CALLBACK KeepAliveTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
VOID CALLBACK OnlineNotifTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
VOID CALLBACK OnlineNotifTimerProc3(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
VOID CALLBACK DCCTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

// options.cpp

void    InitContactMenus(void);
void    UninitContactMenus(void);

void    InitIcons(void);

// tools.cpp
int            __stdcall WCCmp(const wchar_t* wild, const wchar_t* string);
char*          __stdcall IrcLoadFile(wchar_t * szPath);
CMStringW      __stdcall GetWord(const wchar_t* text, int index);
const wchar_t* __stdcall GetWordAddress(const wchar_t* text, int index);
void           __stdcall RemoveLinebreaks(CMStringW& Message);
wchar_t*       __stdcall DoColorCodes(const wchar_t* text, bool bStrip, bool bReplacePercent);

CMStringA      __stdcall GetWord(const char* text, int index);

#pragma comment(lib,"comctl32.lib")

#endif
