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
#include "m_clist.h"
#include "m_options.h"
#include "m_database.h"
#include "m_utils.h"
#include "m_skin.h"
#include "m_netlib.h"
#include "m_langpack.h"
#include "m_chat_int.h"
#include "m_message.h"
#include "m_userinfo.h"
#include "m_addcontact.h"
#include "m_button.h"
#include "m_genmenu.h"
#include "m_file.h"
#include "m_ignore.h"
#include "m_chat_int.h"
#include "m_icolib.h"
#include "m_string.h"
#include "win2k.h"
#include "m_gui.h"

#include "resource.h"

#define IRC_QUICKCONNECT      "/QuickConnectMenu"
#define IRC_JOINCHANNEL       "/JoinChannelMenu"
#define IRC_CHANGENICK        "/ChangeNickMenu"
#define IRC_SHOWLIST          "/ShowListMenu"
#define IRC_SHOWSERVER        "/ShowServerMenu"
#define IRC_UM_CHANSETTINGS   "/UMenuChanSettings"
#define IRC_UM_WHOIS          "/UMenuWhois"
#define IRC_UM_DISCONNECT     "/UMenuDisconnect"
#define IRC_UM_IGNORE         "/UMenuIgnore"

#define STR_QUITMESSAGE  L"\002Miranda NG!\002 Smaller, Faster, Easier. https://miranda-ng.org/"
#define STR_USERINFO     L"I'm a happy Miranda NG user! Get it here: https://miranda-ng.org/"
#define STR_AWAYMESSAGE  L"I'm away from the computer." // Default away
#define DCCSTRING        L" (DCC)"
#define SERVERSMODULE    "IRC Servers"
#define SERVERWINDOW	 L"Network log"

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

struct SERVER_INFO  // Contains info about different servers
{
	~SERVER_INFO();

	char *m_name, *m_address, *m_group;
	int  m_portStart, m_portEnd, m_iSSL;
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

/////////////////////////////////////////////////////////////////////////////////////////

typedef bool (CIrcProto::*PfnIrcMessageHandler)(const CIrcMessage *pmsg);

#pragma pack(4)

struct CIrcHandler
{
	CIrcHandler(const wchar_t* _name, PfnIrcMessageHandler _handler) :
		m_name(_name),
		m_handler(_handler)
	{
	}

	const wchar_t* m_name;
	PfnIrcMessageHandler m_handler;
};
#pragma pack()

struct CIrcProto : public PROTO<CIrcProto>
{
	CIrcProto(const char*, const wchar_t*);
	~CIrcProto();

	// Protocol interface

	virtual	MCONTACT  __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);

	virtual	int       __cdecl Authorize(MEVENT hDbEvent);
	virtual	int       __cdecl AuthDeny(MEVENT hDbEvent, const wchar_t* szReason);

	virtual	HANDLE    __cdecl FileAllow(MCONTACT hContact, HANDLE hTransfer, const wchar_t *szPath);
	virtual	int       __cdecl FileCancel(MCONTACT hContact, HANDLE hTransfer);
	virtual	int       __cdecl FileDeny(MCONTACT hContact, HANDLE hTransfer, const wchar_t *szReason);
	virtual	int       __cdecl FileResume(HANDLE hTransfer, int *action, const wchar_t **szFilename);

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);

	virtual	HANDLE    __cdecl SearchBasic(const wchar_t* id);

	virtual	HANDLE    __cdecl SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles);
	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg);

	virtual	int       __cdecl SetStatus(int iNewStatus);

	virtual	HANDLE    __cdecl GetAwayMsg(MCONTACT hContact);
	virtual	int       __cdecl SetAwayMsg(int m_iStatus, const wchar_t *msg);

	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam);

	// Services
	INT_PTR __cdecl SvcCreateAccMgrUI(WPARAM, LPARAM);
	INT_PTR __cdecl GetMyAwayMsg(WPARAM, LPARAM);

	INT_PTR __cdecl OnChangeNickMenuCommand(WPARAM, LPARAM);
	INT_PTR __cdecl OnDoubleclicked(WPARAM, LPARAM);
	INT_PTR __cdecl OnJoinChat(WPARAM, LPARAM);
	INT_PTR __cdecl OnJoinMenuCommand(WPARAM, LPARAM);
	INT_PTR __cdecl OnLeaveChat(WPARAM, LPARAM);
	INT_PTR __cdecl OnMenuChanSettings(WPARAM, LPARAM);
	INT_PTR __cdecl OnMenuDisconnect(WPARAM, LPARAM);
	INT_PTR __cdecl OnMenuIgnore(WPARAM, LPARAM);
	INT_PTR __cdecl OnMenuWhois(WPARAM, LPARAM);
	INT_PTR __cdecl OnQuickConnectMenuCommand(WPARAM, LPARAM);
	INT_PTR __cdecl OnShowListMenuCommand(WPARAM, LPARAM);
	INT_PTR __cdecl OnShowServerMenuCommand(WPARAM, LPARAM);

	// Events
	int __cdecl OnContactDeleted(WPARAM, LPARAM);
	int __cdecl OnInitOptionsPages(WPARAM, LPARAM);
	int __cdecl OnInitUserInfo(WPARAM, LPARAM);
	int __cdecl OnModulesLoaded(WPARAM, LPARAM);
	int __cdecl OnMenuPreBuild(WPARAM, LPARAM);
	int __cdecl OnPreShutdown(WPARAM, LPARAM);
	int __cdecl OnDbSettingChanged(WPARAM, LPARAM);

	int __cdecl GCEventHook(WPARAM, LPARAM);
	int __cdecl GCMenuHook(WPARAM, LPARAM);

	// Data

	char     m_serverName[100];
	char     m_password[500];
	wchar_t  m_identSystem[10];
	char     m_network[30];
	char     m_portStart[10];
	char     m_portEnd[10];
	int      m_iSSL;
	wchar_t  m_identPort[10];
	wchar_t  m_nick[30], m_pNick[30];
	wchar_t  m_alternativeNick[30];
	wchar_t  m_name[200];
	wchar_t  m_userID[200];
	wchar_t  m_quitMessage[400];
	wchar_t  m_userInfo[500];
	char     m_myHost[50];
	char     m_mySpecifiedHost[500];
	char     m_mySpecifiedHostIP[50];
	char     m_myLocalHost[50];
	WORD     m_myLocalPort;
	wchar_t *m_alias;
	int      m_serverComboSelection;
	int      m_quickComboSelection;
	int      m_onlineNotificationTime;
	int      m_onlineNotificationLimit;
	BYTE     m_scriptingEnabled;
	BYTE     m_IPFromServer;
	BYTE     m_showAddresses;
	BYTE     m_disconnectDCCChats;
	BYTE     m_disableErrorPopups;
	BYTE     m_rejoinChannels;
	BYTE     m_rejoinIfKicked;
	BYTE     m_hideServerWindow;
	BYTE     m_ident;
	BYTE     m_identTimer;
	BYTE     m_disableDefaultServer;
	BYTE     m_autoOnlineNotification;
	BYTE     m_sendKeepAlive;
	BYTE     m_joinOnInvite;
	BYTE     m_perform;
	BYTE     m_forceVisible;
	BYTE     m_ignore;
	BYTE     m_ignoreChannelDefault;
	BYTE     m_useServer;
	BYTE     m_DCCFileEnabled;
	BYTE     m_DCCChatEnabled;
	BYTE     m_DCCChatAccept;
	BYTE     m_DCCChatIgnore;
	BYTE     m_DCCPassive;
	BYTE     m_DCCMode;
	WORD     m_DCCPacketSize;
	BYTE     m_manualHost;
	BYTE     m_oldStyleModes;
	BYTE     m_channelAwayNotification;
	BYTE     m_sendNotice;
	BYTE     m_utfAutodetect;
	int      m_codepage;
	COLORREF colors[16];
	HICON    hIcon[13];

	OBJLIST<CMStringW> vUserhostReasons;
	OBJLIST<CMStringW> vWhoInProgress;

	mir_cs   cs;
	mir_cs   m_gchook;
	mir_cs   m_resolve;
	HANDLE   m_evWndCreate;

	CMStringW m_statusMessage;
	int      m_iTempCheckTime;

	CIrcSessionInfo si;

	int       m_portCount;
	DWORD     m_bConnectRequested;
	DWORD     m_bConnectThreadRunning;

	HGENMENU    hMenuQuick, hMenuServer, hMenuJoin, hMenuNick, hMenuList;
	HNETLIBUSER hNetlibDCC;

	bool  bTempDisableCheck, bTempForceCheck, bEcho;
	bool	nickflag;

	bool     bPerformDone;

	CJoinDlg *m_joinDlg;
	CListDlg *m_listDlg;
	CNickDlg *m_nickDlg;
	CWhoisDlg *m_whoisDlg;
	CQuickDlg *m_quickDlg;
	CManagerDlg *m_managerDlg;
	CIgnorePrefsDlg *m_ignoreDlg;

	int m_noOfChannels, m_manualWhoisCount;
	CMStringA sChannelModes, sUserModes;
	CMStringW sChannelPrefixes, sUserModePrefixes, WhoisAwayReply;

	// clist.cpp
	MCONTACT CList_AddContact(CONTACT *user, bool InList, bool SetOnline);
	bool     CList_SetAllOffline(BYTE ChatsToo);
	MCONTACT CList_SetOffline(CONTACT *user);
	MCONTACT CList_FindContact(CONTACT *user);
	BOOL     CList_AddDCCChat(const CMStringW &name, const CMStringW &hostmask, unsigned long adr, int port);

	// commandmonitor.cpp
	UINT_PTR IdentTimer, InitTimer, KeepAliveTimer, OnlineNotifTimer, OnlineNotifTimer3;

	int  AddOutgoingMessageToDB(MCONTACT hContact, const wchar_t *msg);
	bool DoOnConnect(const CIrcMessage *pmsg);
	int  DoPerform(const char *event);
	void __cdecl ResolveIPThread(void *di);

	bool AddIgnore(const wchar_t *mask, const wchar_t *mode, const wchar_t *network);
	int  IsIgnored(const CMStringW &nick, const CMStringW &address, const CMStringW &host, char type);
	int  IsIgnored(CMStringW user, char type);
	bool RemoveIgnore(const wchar_t *mask);

	// input.cpp
	CMStringW DoAlias(const wchar_t *text, wchar_t *window);
	BOOL      DoHardcodedCommand(CMStringW text, wchar_t *window, MCONTACT hContact);
	CMStringW DoIdentifiers(CMStringW text, const wchar_t *window);
	void      FormatMsg(CMStringW &text);
	bool      PostIrcMessageWnd(wchar_t *pszWindow, MCONTACT hContact, const wchar_t *szBuf);
	bool      PostIrcMessage(const wchar_t *fmt, ...);

	// irclib.cpp
	UINT_PTR	DCCTimer;
	void     SendIrcMessage(const wchar_t*, bool bNotify = true, int codepage = -1);

	// ircproto.cpp
	void __cdecl AckBasicSearch(void* param);
	void __cdecl AckMessageFail(void* info);
	void __cdecl AckMessageFailDcc(void* info);
	void __cdecl AckMessageSuccess(void* info);

	int  SetStatusInternal(int iNewStatus, bool bIsInternal);

	//options.cpp
	HWND m_hwndConnect;

	OBJLIST<CIrcIgnoreItem> m_ignoreItems;

	int       m_channelNumber;
	CMStringW m_whoReply;
	CMStringW sNamesList;
	CMStringW sTopic;
	CMStringW sTopicName;
	CMStringW sTopicTime;
	CMStringW m_namesToWho;
	CMStringW m_channelsToWho;
	CMStringW m_namesToUserhost;

	void    InitPrefs(void);
	void    InitIgnore(void);

	void    ReadSettings(TDbSetting* sets, int count);
	void    RewriteIgnoreSettings(void);
	void    WriteSettings(TDbSetting* sets, int count);

	// output
	BOOL   ShowMessage(const CIrcMessage *pmsg);

	// scripting.cpp
	INT_PTR  __cdecl Scripting_InsertRawIn(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl Scripting_InsertRawOut(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl Scripting_InsertGuiIn(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl Scripting_InsertGuiOut(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl Scripting_GetIrcData(WPARAM wparam, LPARAM lparam);

	// services.cpp
	void   ConnectToServer(void);
	void   DisconnectFromServer(void);
	void   InitMainMenus(void);

	void __cdecl ConnectServerThread(void*);
	void __cdecl DisconnectServerThread(void*);

	// tools.cpp
	void      AddToJTemp(wchar_t op, CMStringW& sCommand);
	bool      AddWindowItemData(CMStringW window, const wchar_t *pszLimit, const wchar_t *pszMode, const wchar_t *pszPassword, const wchar_t *pszTopic);
	INT_PTR   DoEvent(int iEvent, const wchar_t *pszWindow, const wchar_t *pszNick, const wchar_t *pszText, const wchar_t *pszStatus, const wchar_t *pszUserInfo, DWORD_PTR dwItemData, bool bAddToLog, bool bIsMe, time_t timestamp = 1);
	void      FindLocalIP(HNETLIBCONN con);
	bool      FreeWindowItemData(CMStringW window, CHANNELINFO* wis);
	bool      IsChannel(const char* sName);
	bool      IsChannel(const wchar_t* sName);
	void      KillChatTimer(UINT_PTR &nIDEvent);
	CMStringW MakeWndID(const wchar_t* sWindow);
	CMStringW ModeToStatus(int sMode);
	CMStringW PrefixToStatus(int cPrefix);
	int       SetChannelSBText(CMStringW sWindow, CHANNELINFO *wi);
	void      SetChatTimer(UINT_PTR &nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc);

	void      ClearUserhostReasons(int type);
	void      DoUserhostWithReason(int type, CMStringW reason, bool bSendCommand, CMStringW userhostparams, ...);
	CMStringW GetNextUserhostReason(int type);
	CMStringW PeekAtReasons(int type);

	////////////////////////////////////////////////////////////////////////////////////////
	// former CIrcSession class

	void AddDCCSession(MCONTACT hContact, CDccSession *dcc);
	void AddDCCSession(DCCINFO *pdci, CDccSession *dcc);
	void RemoveDCCSession(MCONTACT hContact);
	void RemoveDCCSession(DCCINFO *pdci);

	CDccSession* FindDCCSession(MCONTACT hContact);
	CDccSession* FindDCCSession(DCCINFO *pdci);
	CDccSession* FindDCCSendByPort(int iPort);
	CDccSession* FindDCCRecvByPortAndName(int iPort, const wchar_t *szName);
	CDccSession* FindPassiveDCCSend(int iToken);
	CDccSession* FindPassiveDCCRecv(CMStringW sName, CMStringW sToken);

	void DisconnectAllDCCSessions(bool Shutdown);
	void CheckDCCTimeout(void);

	bool Connect(const CIrcSessionInfo &info);
	void Disconnect(void);
	void KillIdent(void);

	int NLSend(const wchar_t* fmt, ...);
	int NLSend(const char* fmt, ...);
	int NLSend(const unsigned char* buf, int cbBuf);
	int NLSendNoScript(const unsigned char* buf, int cbBuf);
	int NLReceive(unsigned char* buf, int cbBuf);
	void InsertIncomingEvent(wchar_t* pszRaw);

	__inline bool IsConnected() const { return con != NULL; }

	// send-to-stream operators
	int getCodepage() const;
	__inline void setCodepage(int aPage) { codepage = aPage; }

	CIrcSessionInfo m_info;

protected:
	int codepage;
	HNETLIBCONN con;
	HNETLIBBIND hBindPort;
	void DoReceive();
	LIST<CDccSession> m_dcc_chats;
	LIST<CDccSession> m_dcc_xfers;

private:
	mir_cs m_dcc; // protect the dcc objects

	void createMessageFromPchar(const char* p);
	void Notify(const CIrcMessage *pmsg);
	void __cdecl ThreadProc(void *pparam);

	////////////////////////////////////////////////////////////////////////////////////////
	// former CIrcMonitor class

	bool OnIrc_PING(const CIrcMessage *pmsg);
	bool OnIrc_WELCOME(const CIrcMessage *pmsg);
	bool OnIrc_YOURHOST(const CIrcMessage *pmsg);
	bool OnIrc_NICK(const CIrcMessage *pmsg);
	bool OnIrc_PRIVMSG(const CIrcMessage *pmsg);
	bool OnIrc_JOIN(const CIrcMessage *pmsg);
	bool OnIrc_QUIT(const CIrcMessage *pmsg);
	bool OnIrc_PART(const CIrcMessage *pmsg);
	bool OnIrc_KICK(const CIrcMessage *pmsg);
	bool OnIrc_MODE(const CIrcMessage *pmsg);
	bool OnIrc_USERHOST_REPLY(const CIrcMessage *pmsg);
	bool OnIrc_MODEQUERY(const CIrcMessage *pmsg);
	bool OnIrc_NAMES(const CIrcMessage *pmsg);
	bool OnIrc_ENDNAMES(const CIrcMessage *pmsg);
	bool OnIrc_INITIALTOPIC(const CIrcMessage *pmsg);
	bool OnIrc_INITIALTOPICNAME(const CIrcMessage *pmsg);
	bool OnIrc_TOPIC(const CIrcMessage *pmsg);
	bool OnIrc_TRYAGAIN(const CIrcMessage *pmsg);
	bool OnIrc_NOTICE(const CIrcMessage *pmsg);
	bool OnIrc_WHOIS_NAME(const CIrcMessage *pmsg);
	bool OnIrc_WHOIS_CHANNELS(const CIrcMessage *pmsg);
	bool OnIrc_WHOIS_SERVER(const CIrcMessage *pmsg);
	bool OnIrc_WHOIS_AWAY(const CIrcMessage *pmsg);
	bool OnIrc_WHOIS_IDLE(const CIrcMessage *pmsg);
	bool OnIrc_WHOIS_END(const CIrcMessage *pmsg);
	bool OnIrc_WHOIS_OTHER(const CIrcMessage *pmsg);
	bool OnIrc_WHOIS_AUTH(const CIrcMessage *pmsg);
	bool OnIrc_WHOIS_NO_USER(const CIrcMessage *pmsg);
	bool OnIrc_NICK_ERR(const CIrcMessage *pmsg);
	bool OnIrc_ENDMOTD(const CIrcMessage *pmsg);
	bool OnIrc_LISTSTART(const CIrcMessage *pmsg);
	bool OnIrc_LIST(const CIrcMessage *pmsg);
	bool OnIrc_LISTEND(const CIrcMessage *pmsg);
	bool OnIrc_BANLIST(const CIrcMessage *pmsg);
	bool OnIrc_BANLISTEND(const CIrcMessage *pmsg);
	bool OnIrc_SUPPORT(const CIrcMessage *pmsg);
	bool OnIrc_BACKFROMAWAY(const CIrcMessage *pmsg);
	bool OnIrc_SETAWAY(const CIrcMessage *pmsg);
	bool OnIrc_JOINERROR(const CIrcMessage *pmsg);
	bool OnIrc_UNKNOWN(const CIrcMessage *pmsg);
	bool OnIrc_ERROR(const CIrcMessage *pmsg);
	bool OnIrc_NOOFCHANNELS(const CIrcMessage *pmsg);
	bool OnIrc_PINGPONG(const CIrcMessage *pmsg);
	bool OnIrc_INVITE(const CIrcMessage *pmsg);
	bool OnIrc_WHO_END(const CIrcMessage *pmsg);
	bool OnIrc_WHO_REPLY(const CIrcMessage *pmsg);
	bool OnIrc_WHOTOOLONG(const CIrcMessage *pmsg);

	bool IsCTCP(const CIrcMessage *pmsg);

	void OnIrcDefault(const CIrcMessage *pmsg);
	void OnIrcDisconnected();

	static OBJLIST<CIrcHandler> m_handlers;

	PfnIrcMessageHandler FindMethod(const wchar_t* lpszName);

	void OnIrcMessage(const CIrcMessage *pmsg);
	CMStringW sNick4Perform;
};

// map actual member functions to their associated IRC command.
// put any number of this macro in the class's constructor.
#define	IRC_MAP_ENTRY(name, member)	\
	m_handlers.insert( new CIrcHandler( L##name, &CIrcProto::OnIrc_##member ));

/////////////////////////////////////////////////////////////////////////////////////////
// Functions

//main.cpp
extern HINSTANCE hInst;

extern LIST<CIrcProto> g_Instances;

extern OBJLIST<SERVER_INFO> g_servers;

CIrcProto* GetTimerOwner(UINT_PTR eventId);

VOID CALLBACK IdentTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
VOID CALLBACK KeepAliveTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
VOID CALLBACK OnlineNotifTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
VOID CALLBACK OnlineNotifTimerProc3(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
VOID CALLBACK DCCTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

// options.cpp

void    InitServers(void);
void    RereadServers(void);

void    InitContactMenus(void);
void    UninitContactMenus(void);

void    InitIcons(void);
HICON   LoadIconEx(int iIndex, bool big = false);
HANDLE  GetIconHandle(int iconId);

//tools.cpp
int          __stdcall WCCmp(const wchar_t* wild, const wchar_t* string);
char*        __stdcall IrcLoadFile(wchar_t * szPath);
CMStringW     __stdcall GetWord(const wchar_t* text, int index);
const wchar_t* __stdcall GetWordAddress(const wchar_t* text, int index);
void         __stdcall RemoveLinebreaks(CMStringW& Message);
wchar_t*       __stdcall my_strstri(const wchar_t *s1, const wchar_t *s2);
wchar_t*       __stdcall DoColorCodes(const wchar_t* text, bool bStrip, bool bReplacePercent);

CMStringA   __stdcall GetWord(const char* text, int index);

#pragma comment(lib,"comctl32.lib")

#endif
