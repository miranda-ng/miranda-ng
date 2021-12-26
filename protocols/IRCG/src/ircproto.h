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

#ifndef _IRCPROTO_H_
#define _IRCPROTO_H_

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
	MCONTACT AddToList(int flags, PROTOSEARCHRESULT* psr) override;
			   
	int      Authorize(MEVENT hDbEvent) override;
	int      AuthDeny(MEVENT hDbEvent, const wchar_t* szReason) override;
			   
	HANDLE   FileAllow(MCONTACT hContact, HANDLE hTransfer, const wchar_t *szPath) override;
	int      FileCancel(MCONTACT hContact, HANDLE hTransfer) override;
	int      FileDeny(MCONTACT hContact, HANDLE hTransfer, const wchar_t *szReason) override;
	int      FileResume(HANDLE hTransfer, int action, const wchar_t *szFilename) override;
			   
	INT_PTR  GetCaps(int type, MCONTACT hContact = NULL) override;
			   
	HANDLE   SearchBasic(const wchar_t* id) override;
			   
	HANDLE   SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles) override;
	int      SendMsg(MCONTACT hContact, int flags, const char* msg) override;
			   
	int      SetStatus(int iNewStatus) override;
			   
	HANDLE   GetAwayMsg(MCONTACT hContact) override;
	int      SetAwayMsg(int m_iStatus, const wchar_t *msg) override;
			   
	void     OnBuildProtoMenu(void) override;
	void     OnContactDeleted(MCONTACT) override;
	void     OnModulesLoaded() override;
	void     OnShutdown() override;

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
	INT_PTR __cdecl OnShowListMenuCommand(WPARAM, LPARAM);
	INT_PTR __cdecl OnShowServerMenuCommand(WPARAM, LPARAM);

	// Events
	int __cdecl OnInitOptionsPages(WPARAM, LPARAM);
	int __cdecl OnInitUserInfo(WPARAM, LPARAM);
	int __cdecl OnMenuPreBuild(WPARAM, LPARAM);
	int __cdecl OnDbSettingChanged(WPARAM, LPARAM);

	int __cdecl GCEventHook(WPARAM, LPARAM);
	int __cdecl GCMenuHook(WPARAM, LPARAM);

	// Data

	char     m_serverName[100];
	char     m_password[500];
	wchar_t  m_identSystem[10];
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
	uint16_t     m_myLocalPort;
	wchar_t *m_alias;
	int      m_quickComboSelection;
	int      m_onlineNotificationTime;
	int      m_onlineNotificationLimit;
	uint8_t     m_IPFromServer;
	uint8_t     m_showAddresses;
	uint8_t     m_disconnectDCCChats;
	uint8_t     m_disableErrorPopups;
	uint8_t     m_rejoinChannels;
	uint8_t     m_rejoinIfKicked;
	uint8_t     m_hideServerWindow;
	uint8_t     m_ident;
	uint8_t     m_identTimer;
	uint8_t     m_autoOnlineNotification;
	uint8_t     m_sendKeepAlive;
	uint8_t     m_joinOnInvite;
	uint8_t     m_perform;
	uint8_t     m_forceVisible;
	uint8_t     m_ignore;
	uint8_t     m_ignoreChannelDefault;
	uint8_t     m_useServer;
	uint8_t     m_DCCFileEnabled;
	uint8_t     m_DCCChatEnabled;
	uint8_t     m_DCCChatAccept;
	uint8_t     m_DCCChatIgnore;
	uint8_t     m_DCCPassive;
	uint8_t     m_DCCMode;
	uint16_t     m_DCCPacketSize;
	uint8_t     m_manualHost;
	uint8_t     m_oldStyleModes;
	uint8_t     m_channelAwayNotification;
	uint8_t     m_sendNotice;
	uint8_t     m_utfAutodetect;
	uint8_t     m_bUseSASL;
	int      m_codepage;
	COLORREF colors[16];
	HICON    hIcon[13];

	OBJLIST<CMStringW> vUserhostReasons;
	OBJLIST<CMStringW> vWhoInProgress;

	mir_cs   m_csSession, m_csGcHook, m_csResolve, m_csList;

	CMStringW m_statusMessage;
	int      m_iTempCheckTime;

	CIrcSessionInfo m_sessionInfo;
	SESSION_INFO *m_pServer;

	int       m_portCount;
	uint32_t     m_bConnectRequested;
	uint32_t     m_bConnectThreadRunning;

	HGENMENU    hMenuQuick, hMenuServer, hMenuJoin, hMenuNick, hMenuList;
	HNETLIBUSER hNetlibDCC;

	bool  bTempDisableCheck, bTempForceCheck, bEcho;
	bool	nickflag;

	bool     bPerformDone;

	CJoinDlg *m_joinDlg;
	CListDlg *m_listDlg;
	CNickDlg *m_nickDlg;
	CWhoisDlg *m_whoisDlg;
	CManagerDlg *m_managerDlg;
	CIrcBaseDlg *m_ignoreDlg;

	int m_noOfChannels, m_manualWhoisCount;
	CMStringA sChannelModes, sUserModes;
	CMStringW sChannelPrefixes, sUserModePrefixes, WhoisAwayReply;

	// clist.cpp
	MCONTACT CList_AddContact(CONTACT *user, bool InList, bool SetOnline);
	bool     CList_SetAllOffline(uint8_t ChatsToo);
	MCONTACT CList_SetOffline(CONTACT *user);
	MCONTACT CList_FindContact(CONTACT *user);
	BOOL     CList_AddDCCChat(const CMStringW &name, const CMStringW &hostmask, unsigned long adr, int port);

	// commandmonitor.cpp
	UINT_PTR IdentTimer, InitTimer, KeepAliveTimer, OnlineNotifTimer, OnlineNotifTimer3;

	int  AddOutgoingMessageToDB(MCONTACT hContact, const wchar_t *msg);
	bool DoOnConnect(const CIrcMessage *pmsg);
	int  DoPerform(const char *event);
	void __cdecl DoPerformThread(void *di);
	void __cdecl ResolveIPThread(void *di);

	bool AddIgnore(const wchar_t *mask, const wchar_t *mode);
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

	// services.cpp
	void   ConnectToServer(void);
	void   DisconnectFromServer(void);

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
	CMStringW ModeToStatus(int sMode);
	CMStringW PrefixToStatus(int cPrefix);
	int       SetChannelSBText(CMStringW sWindow, CHANNELINFO *wi);
	void      SetChatTimer(UINT_PTR &nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc);

	void      ClearUserhostReasons(int type);
	void      DoUserhostWithReason(int type, CMStringW reason, bool bSendCommand, const wchar_t *userhostparams, ...);
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

	__inline bool IsConnected() const { return con != nullptr; }

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
	bool OnIrc_AUTHENTICATE(const CIrcMessage *pmsg);
	bool OnIrc_AUTH_OK(const CIrcMessage *pmsg);
	bool OnIrc_AUTH_FAIL(const CIrcMessage *pmsg);
	bool OnIrc_CAP(const CIrcMessage *pmsg);

	bool IsCTCP(const CIrcMessage *pmsg);

	void OnIrcDefault(const CIrcMessage *pmsg);
	void OnIrcDisconnected();

	static OBJLIST<CIrcHandler> m_handlers;

	PfnIrcMessageHandler FindMethod(const wchar_t* lpszName);

	void OnIrcMessage(const CIrcMessage *pmsg);
	CMStringW sNick4Perform;
};

struct CMPlugin : public ACCPROTOPLUGIN<CIrcProto>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

#endif // _IRCPROTO_H_
