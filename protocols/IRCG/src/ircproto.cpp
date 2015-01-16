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

#include "irc.h"
#include "version.h"

#include <m_genmenu.h>

static volatile LONG g_msgid = 1;

static int CompareSessions(const CDccSession* p1, const CDccSession* p2)
{
	return INT_PTR(p1->di->hContact) - INT_PTR(p2->di->hContact);
}

CIrcProto::CIrcProto(const char* szModuleName, const TCHAR* tszUserName) :
	PROTO<CIrcProto>(szModuleName, tszUserName),
	m_dcc_chats(10, CompareSessions),
	m_dcc_xfers(10, CompareSessions),
	m_ignoreItems(10),
	vUserhostReasons(10),
	vWhoInProgress(10)
{
	m_evWndCreate = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	CreateProtoService(PS_GETMYAWAYMSG, &CIrcProto::GetMyAwayMsg);

	CreateProtoService(PS_CREATEACCMGRUI, &CIrcProto::SvcCreateAccMgrUI);
	CreateProtoService(PS_JOINCHAT, &CIrcProto::OnJoinChat);
	CreateProtoService(PS_LEAVECHAT, &CIrcProto::OnLeaveChat);

	CreateProtoService(IRC_JOINCHANNEL, &CIrcProto::OnJoinMenuCommand);
	CreateProtoService(IRC_QUICKCONNECT, &CIrcProto::OnQuickConnectMenuCommand);
	CreateProtoService(IRC_CHANGENICK, &CIrcProto::OnChangeNickMenuCommand);
	CreateProtoService(IRC_SHOWLIST, &CIrcProto::OnShowListMenuCommand);
	CreateProtoService(IRC_SHOWSERVER, &CIrcProto::OnShowServerMenuCommand);
	CreateProtoService(IRC_UM_CHANSETTINGS, &CIrcProto::OnMenuChanSettings);
	CreateProtoService(IRC_UM_WHOIS, &CIrcProto::OnMenuWhois);
	CreateProtoService(IRC_UM_DISCONNECT, &CIrcProto::OnMenuDisconnect);
	CreateProtoService(IRC_UM_IGNORE, &CIrcProto::OnMenuIgnore);

	CreateProtoService("/DblClickEvent", &CIrcProto::OnDoubleclicked);
	CreateProtoService("/InsertRawIn", &CIrcProto::Scripting_InsertRawIn);
	CreateProtoService("/InsertRawOut", &CIrcProto::Scripting_InsertRawOut);
	CreateProtoService("/InsertGuiIn", &CIrcProto::Scripting_InsertGuiIn);
	CreateProtoService("/InsertGuiOut", &CIrcProto::Scripting_InsertGuiOut);
	CreateProtoService("/GetIrcData", &CIrcProto::Scripting_GetIrcData);

	codepage = CP_ACP;

	InitPrefs();

	db_set_resident(m_szModuleName, "Status");

	CList_SetAllOffline(true);

	IRC_MAP_ENTRY("PING", PING)
		IRC_MAP_ENTRY("JOIN", JOIN)
		IRC_MAP_ENTRY("QUIT", QUIT)
		IRC_MAP_ENTRY("KICK", KICK)
		IRC_MAP_ENTRY("MODE", MODE)
		IRC_MAP_ENTRY("NICK", NICK)
		IRC_MAP_ENTRY("PART", PART)
		IRC_MAP_ENTRY("PRIVMSG", PRIVMSG)
		IRC_MAP_ENTRY("TOPIC", TOPIC)
		IRC_MAP_ENTRY("NOTICE", NOTICE)
		IRC_MAP_ENTRY("PING", PINGPONG)
		IRC_MAP_ENTRY("PONG", PINGPONG)
		IRC_MAP_ENTRY("INVITE", INVITE)
		IRC_MAP_ENTRY("ERROR", ERROR)
		IRC_MAP_ENTRY("001", WELCOME)
		IRC_MAP_ENTRY("002", YOURHOST)
		IRC_MAP_ENTRY("005", SUPPORT)
		IRC_MAP_ENTRY("223", WHOIS_OTHER)			//CodePage info
		IRC_MAP_ENTRY("254", NOOFCHANNELS)
		IRC_MAP_ENTRY("263", TRYAGAIN)
		IRC_MAP_ENTRY("264", WHOIS_OTHER)			//Encryption info (SSL connect)
		IRC_MAP_ENTRY("301", WHOIS_AWAY)
		IRC_MAP_ENTRY("302", USERHOST_REPLY)
		IRC_MAP_ENTRY("305", BACKFROMAWAY)
		IRC_MAP_ENTRY("306", SETAWAY)
		IRC_MAP_ENTRY("307", WHOIS_AUTH)
		IRC_MAP_ENTRY("310", WHOIS_OTHER)
		IRC_MAP_ENTRY("311", WHOIS_NAME)
		IRC_MAP_ENTRY("312", WHOIS_SERVER)
		IRC_MAP_ENTRY("313", WHOIS_OTHER)
		IRC_MAP_ENTRY("315", WHO_END)
		IRC_MAP_ENTRY("317", WHOIS_IDLE)
		IRC_MAP_ENTRY("318", WHOIS_END)
		IRC_MAP_ENTRY("319", WHOIS_CHANNELS)
		IRC_MAP_ENTRY("320", WHOIS_AUTH)
		IRC_MAP_ENTRY("321", LISTSTART)
		IRC_MAP_ENTRY("322", LIST)
		IRC_MAP_ENTRY("323", LISTEND)
		IRC_MAP_ENTRY("324", MODEQUERY)
		IRC_MAP_ENTRY("330", WHOIS_AUTH)
		IRC_MAP_ENTRY("332", INITIALTOPIC)
		IRC_MAP_ENTRY("333", INITIALTOPICNAME)
		IRC_MAP_ENTRY("352", WHO_REPLY)
		IRC_MAP_ENTRY("353", NAMES)
		IRC_MAP_ENTRY("366", ENDNAMES)
		IRC_MAP_ENTRY("367", BANLIST)
		IRC_MAP_ENTRY("368", BANLISTEND)
		IRC_MAP_ENTRY("346", BANLIST)
		IRC_MAP_ENTRY("347", BANLISTEND)
		IRC_MAP_ENTRY("348", BANLIST)
		IRC_MAP_ENTRY("349", BANLISTEND)
		IRC_MAP_ENTRY("371", WHOIS_OTHER)
		IRC_MAP_ENTRY("376", ENDMOTD)
		IRC_MAP_ENTRY("401", WHOIS_NO_USER)
		IRC_MAP_ENTRY("403", JOINERROR)
		IRC_MAP_ENTRY("416", WHOTOOLONG)
		IRC_MAP_ENTRY("421", UNKNOWN)
		IRC_MAP_ENTRY("422", ENDMOTD)
		IRC_MAP_ENTRY("433", NICK_ERR)
		IRC_MAP_ENTRY("471", JOINERROR)
		IRC_MAP_ENTRY("473", JOINERROR)
		IRC_MAP_ENTRY("474", JOINERROR)
		IRC_MAP_ENTRY("475", JOINERROR)
		IRC_MAP_ENTRY("671", WHOIS_OTHER)			//Encryption info (SSL connect)
}

CIrcProto::~CIrcProto()
{
	if (con) {
		Netlib_CloseHandle(con);
		con = NULL;
	}

	Netlib_CloseHandle(m_hNetlibUser); m_hNetlibUser = NULL;
	Netlib_CloseHandle(hNetlibDCC); hNetlibDCC = NULL;

	if (hMenuRoot)
		CallService(MO_REMOVEMENUITEM, (WPARAM)hMenuRoot, 0);

	mir_free(m_alias);

	CloseHandle(m_evWndCreate);
	KillChatTimer(OnlineNotifTimer);
	KillChatTimer(OnlineNotifTimer3);
}

////////////////////////////////////////////////////////////////////////////////////////
// OnModulesLoaded - performs hook registration

static COLORREF crCols[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

static int sttCheckPerform(const char *szSetting, LPARAM lParam)
{
	if (!_strnicmp(szSetting, "PERFORM:", 8)) {
		CMStringA s = szSetting;
		s.MakeUpper();
		if (s != szSetting) {
			OBJLIST<CMStringA>* p = (OBJLIST<CMStringA>*)lParam;
			p->insert(new CMStringA(szSetting));
		}
	}
	return 0;
}

int CIrcProto::OnModulesLoaded(WPARAM, LPARAM)
{
	NETLIBUSER nlu = { 0 };
	TCHAR name[128];

	db_unset(NULL, m_szModuleName, "JTemp");

	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_TCHAR;
	nlu.szSettingsModule = m_szModuleName;
	mir_sntprintf(name, SIZEOF(name), TranslateT("%s server connection"), m_tszUserName);
	nlu.ptszDescriptiveName = name;
	m_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_TCHAR;
	char szTemp2[256];
	mir_snprintf(szTemp2, SIZEOF(szTemp2), "%s DCC", m_szModuleName);
	nlu.szSettingsModule = szTemp2;
	mir_sntprintf(name, SIZEOF(name), TranslateT("%s client-to-client connections"), m_tszUserName);
	nlu.ptszDescriptiveName = name;
	hNetlibDCC = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	GCREGISTER gcr = { sizeof(GCREGISTER) };
	gcr.dwFlags = GC_CHANMGR | GC_BOLD | GC_ITALICS | GC_UNDERLINE | GC_COLOR | GC_BKGCOLOR;
	gcr.nColors = 16;
	gcr.pColors = colors;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	CallServiceSync(MS_GC_REGISTER, NULL, (LPARAM)&gcr);

	HookProtoEvent(ME_GC_EVENT, &CIrcProto::GCEventHook);
	HookProtoEvent(ME_GC_BUILDMENU, &CIrcProto::GCMenuHook);

	GCSESSION gcw = { sizeof(GCSESSION) };
	gcw.iType = GCW_SERVER;
	gcw.ptszID = SERVERWINDOW;
	gcw.pszModule = m_szModuleName;
	gcw.ptszName = NEWTSTR_ALLOCA((TCHAR*)_A2T(m_network));
	CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM)&gcw);

	GCDEST gcd = { m_szModuleName, SERVERWINDOW, GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };
	if (m_useServer && !m_hideServerWindow)
		CallChatEvent(WINDOW_VISIBLE, (LPARAM)&gce);
	else
		CallChatEvent(WINDOW_HIDDEN, (LPARAM)&gce);

	TCHAR szTemp[MAX_PATH];
	mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%%miranda_path%%\\Plugins\\%S_perform.ini"), m_szModuleName);
	TCHAR *szLoadFileName = Utils_ReplaceVarsT(szTemp);
	char* pszPerformData = IrcLoadFile(szLoadFileName);
	if (pszPerformData != NULL) {
		char *p1 = pszPerformData, *p2 = pszPerformData;
		while ((p1 = strstr(p2, "NETWORK: ")) != NULL) {
			p1 += 9;
			p2 = strchr(p1, '\n');
			CMStringA sNetwork(p1, int(p2 - p1 - 1));
			sNetwork.MakeUpper();
			p1 = p2;
			p2 = strstr(++p1, "\nNETWORK: ");
			if (!p2)
				p2 = p1 + mir_strlen(p1) - 1;
			if (p1 == p2)
				break;

			*p2++ = 0;
			setString(("PERFORM:" + sNetwork).c_str(), rtrim(p1));
		}
		delete[] pszPerformData;
		::_tremove(szLoadFileName);
	}
	mir_free(szLoadFileName);

	if (!getByte("PerformConversionDone", 0)) {
		OBJLIST<CMStringA> performToConvert(10);
		DBCONTACTENUMSETTINGS dbces = { 0 };
		dbces.pfnEnumProc = sttCheckPerform;
		dbces.lParam = (LPARAM)&performToConvert;
		dbces.szModule = m_szModuleName;
		CallService(MS_DB_CONTACT_ENUMSETTINGS, NULL, (LPARAM)&dbces);

		for (int i = 0; i < performToConvert.getCount(); i++) {
			CMStringA s = performToConvert[i];
			DBVARIANT dbv;
			if (!getTString(s, &dbv)) {
				db_unset(NULL, m_szModuleName, s);
				s.MakeUpper();
				setTString(s, dbv.ptszVal);
				db_free(&dbv);
			}
		}

		setByte("PerformConversionDone", 1);
	}

	InitIgnore();

	HookProtoEvent(ME_USERINFO_INITIALISE, &CIrcProto::OnInitUserInfo);
	HookProtoEvent(ME_OPT_INITIALISE, &CIrcProto::OnInitOptionsPages);

	if (m_nick[0]) {
		TCHAR szBuf[40];
		if (mir_tstrlen(m_alternativeNick) == 0) {
			mir_sntprintf(szBuf, SIZEOF(szBuf), _T("%s%u"), m_nick, rand() % 9999);
			setTString("AlernativeNick", szBuf);
			mir_tstrncpy(m_alternativeNick, szBuf, 30);
		}

		if (mir_tstrlen(m_name) == 0) {
			mir_sntprintf(szBuf, SIZEOF(szBuf), _T("Miranda%u"), rand() % 9999);
			setTString("Name", szBuf);
			mir_tstrncpy(m_name, szBuf, 200);
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// AddToList - adds a contact to the contact list

MCONTACT __cdecl CIrcProto::AddToList(int, PROTOSEARCHRESULT* psr)
{
	if (m_iStatus == ID_STATUS_OFFLINE || m_iStatus == ID_STATUS_CONNECTING)
		return 0;

	TCHAR *id = psr->id ? psr->id : psr->nick;
	id = psr->flags & PSR_UNICODE ? mir_u2t((wchar_t*)id) : mir_a2t((char*)id);

	CONTACT user = { id, NULL, NULL, true, false, false };
	MCONTACT hContact = CList_AddContact(&user, true, false);

	if (hContact) {
		DBVARIANT dbv1;
		CMString S = _T("S");

		if (getByte(hContact, "AdvancedMode", 0) == 0) {
			S += user.name;
			DoUserhostWithReason(1, S, true, user.name);
		}
		else {
			if (!getTString(hContact, "UWildcard", &dbv1)) {
				S += dbv1.ptszVal;
				DoUserhostWithReason(2, S, true, dbv1.ptszVal);
				db_free(&dbv1);
			}
			else {
				S += user.name;
				DoUserhostWithReason(2, S, true, user.name);
			}
		}
		if (getByte("MirVerAutoRequest", 1))
			PostIrcMessage(_T("/PRIVMSG %s \001VERSION\001"), user.name);
	}

	mir_free(id);
	return hContact;
}

////////////////////////////////////////////////////////////////////////////////////////
// AddToList - adds a contact to the contact list

MCONTACT __cdecl CIrcProto::AddToListByEvent(int, int, MEVENT)
{
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
// AuthAllow - processes the successful authorization

int __cdecl CIrcProto::Authorize(MEVENT)
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// AuthDeny - handles the unsuccessful authorization

int __cdecl CIrcProto::AuthDeny(MEVENT, const TCHAR*)
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// PSR_AUTH

int __cdecl CIrcProto::AuthRecv(MCONTACT, PROTORECVEVENT*)
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// PSS_AUTHREQUEST

int __cdecl CIrcProto::AuthRequest(MCONTACT, const TCHAR*)
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// FileAllow - starts a file transfer

HANDLE __cdecl CIrcProto::FileAllow(MCONTACT, HANDLE hTransfer, const TCHAR* szPath)
{
	DCCINFO* di = (DCCINFO*)hTransfer;

	if (!IsConnected()) {
		delete di;
		return (HANDLE)szPath;
	}

	di->sPath = szPath;
	di->sFileAndPath = di->sPath + di->sFile;

	CDccSession* dcc = new CDccSession(this, di);
	AddDCCSession(di, dcc);
	dcc->Connect();
	return di;
}

////////////////////////////////////////////////////////////////////////////////////////
// FileCancel - cancels a file transfer

int __cdecl CIrcProto::FileCancel(MCONTACT, HANDLE hTransfer)
{
	DCCINFO* di = (DCCINFO*)hTransfer;

	CDccSession* dcc = FindDCCSession(di);
	if (dcc) {
		InterlockedExchange(&dcc->dwWhatNeedsDoing, (long)FILERESUME_CANCEL);
		SetEvent(dcc->hEvent);
		dcc->Disconnect();
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// FileDeny - denies a file transfer

int __cdecl CIrcProto::FileDeny(MCONTACT, HANDLE hTransfer, const TCHAR*)
{
	DCCINFO* di = (DCCINFO*)hTransfer;
	delete di;
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// FileResume - processes file renaming etc

int __cdecl CIrcProto::FileResume(HANDLE hTransfer, int* action, const TCHAR** szFilename)
{
	DCCINFO* di = (DCCINFO*)hTransfer;

	long i = (long)*action;

	CDccSession* dcc = FindDCCSession(di);
	if (dcc) {
		InterlockedExchange(&dcc->dwWhatNeedsDoing, i);
		if (*action == FILERESUME_RENAME) {
			TCHAR* szTemp = _tcsdup(*szFilename);
			InterlockedExchangePointer((PVOID*)&dcc->NewFileName, szTemp);
		}

		if (*action == FILERESUME_RESUME) {
			unsigned __int64 dwPos = 0;

			struct _stati64 statbuf;
			if (_tstati64(di->sFileAndPath.c_str(), &statbuf) == 0 && (statbuf.st_mode & _S_IFDIR) == 0)
				dwPos = statbuf.st_size;

			CMString sFileWithQuotes = di->sFile;

			// if spaces in the filename surround witrh quotes
			if (sFileWithQuotes.Find(' ', 0) != -1) {
				sFileWithQuotes.Insert(0, _T("\""));
				sFileWithQuotes.Insert(sFileWithQuotes.GetLength(), _T("\""));
			}

			if (di->bReverse)
				PostIrcMessage(_T("/PRIVMSG %s \001DCC RESUME %s 0 %I64u %s\001"), di->sContactName.c_str(), sFileWithQuotes.c_str(), dwPos, dcc->di->sToken.c_str());
			else
				PostIrcMessage(_T("/PRIVMSG %s \001DCC RESUME %s %u %I64u\001"), di->sContactName.c_str(), sFileWithQuotes.c_str(), di->iPort, dwPos);

			return 0;
		}

		SetEvent(dcc->hEvent);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// GetCaps - return protocol capabilities bits

DWORD_PTR __cdecl CIrcProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_BASICSEARCH | PF1_MODEMSG | PF1_FILE | PF1_CHAT | PF1_CANRENAMEFILE | PF1_PEER2PEER | PF1_IM;

	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY;

	case PFLAGNUM_3:
		return PF2_SHORTAWAY;

	case PFLAGNUM_4:
		return PF4_NOAUTHDENYREASON | PF4_NOCUSTOMAUTH | PF4_IMSENDUTF;

	case PFLAG_UNIQUEIDTEXT:
		return (DWORD_PTR)Translate("Nickname");

	case PFLAG_MAXLENOFMESSAGE:
		return 400;

	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR) "Nick";
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// GetInfo - retrieves a contact info

int __cdecl CIrcProto::GetInfo(MCONTACT, int)
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SearchBasic - searches the contact by JID

struct AckBasicSearchParam
{
	PROTOCHAR buf[50];
};

void __cdecl CIrcProto::AckBasicSearch(void* param)
{
	PROTOSEARCHRESULT psr = { sizeof(psr) };
	psr.flags = PSR_TCHAR;
	psr.id = ((AckBasicSearchParam*)param)->buf;
	psr.nick = ((AckBasicSearchParam*)param)->buf;
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, (LPARAM)& psr);
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);
	delete param;
}

HANDLE __cdecl CIrcProto::SearchBasic(const PROTOCHAR* szId)
{
	if (szId) {
		if (m_iStatus != ID_STATUS_OFFLINE && m_iStatus != ID_STATUS_CONNECTING &&
			szId && szId[0] && !IsChannel(szId)) {
			AckBasicSearchParam* param = new AckBasicSearchParam;
			mir_tstrncpy(param->buf, szId, 50);
			ForkThread(&CIrcProto::AckBasicSearch, param);
			return (HANDLE)1;
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// SearchByEmail - searches the contact by its e-mail

HANDLE __cdecl CIrcProto::SearchByEmail(const PROTOCHAR*)
{
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
// upsupported search functions

HANDLE __cdecl CIrcProto::SearchByName(const PROTOCHAR*, const PROTOCHAR*, const PROTOCHAR*)
{
	return NULL;
}

HWND __cdecl CIrcProto::CreateExtendedSearchUI(HWND)
{
	return NULL;
}

HWND __cdecl CIrcProto::SearchAdvanced(HWND)
{
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
// RecvContacts

int __cdecl CIrcProto::RecvContacts(MCONTACT, PROTORECVEVENT*)
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// RecvFile

int __cdecl CIrcProto::RecvFile(MCONTACT hContact, PROTORECVFILET* evt)
{
	return Proto_RecvFile(hContact, evt);
}

////////////////////////////////////////////////////////////////////////////////////////
// RecvMsg

int __cdecl CIrcProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT* evt)
{
	return Proto_RecvMessage(hContact, evt);
}

////////////////////////////////////////////////////////////////////////////////////////
// RecvUrl

int __cdecl CIrcProto::RecvUrl(MCONTACT, PROTORECVEVENT*)
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendContacts

int __cdecl CIrcProto::SendContacts(MCONTACT, int, int, MCONTACT*)
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendFile - sends a file

HANDLE __cdecl CIrcProto::SendFile(MCONTACT hContact, const TCHAR*, TCHAR** ppszFiles)
{
	DCCINFO* dci = NULL;
	int iPort = 0;
	int index = 0;
	unsigned __int64 size = 0;

	// do not send to channels :-P
	if (isChatRoom(hContact))
		return 0;

	// stop if it is an active type filetransfer and the user's IP is not known
	unsigned long ulAdr = 0;
	if (m_manualHost)
		ulAdr = ConvertIPToInteger(m_mySpecifiedHostIP);
	else
		ulAdr = ConvertIPToInteger(m_IPFromServer ? m_myHost : m_myLocalHost);

	if (!m_DCCPassive && !ulAdr) {
		DoEvent(GC_EVENT_INFORMATION, 0, m_info.sNick.c_str(), TranslateT("DCC ERROR: Unable to automatically resolve external IP"), NULL, NULL, NULL, true, false);
		return 0;
	}

	if (ppszFiles[index]) {

		//get file size
		while (ppszFiles[index]) {
			struct _stati64 statbuf;
			if (_tstati64(ppszFiles[index], &statbuf) == 0 && (statbuf.st_mode & _S_IFDIR) == 0) {
				size = statbuf.st_size;
				break;
			}
			index++;
		}

		if (size == 0) {
			DoEvent(GC_EVENT_INFORMATION, 0, m_info.sNick.c_str(), TranslateT("DCC ERROR: No valid files specified"), NULL, NULL, NULL, true, false);
			return 0;
		}

		DBVARIANT dbv;
		if (!getTString(hContact, "Nick", &dbv)) {
			// set up a basic DCCINFO struct and pass it to a DCC object
			dci = new DCCINFO;
			dci->sFileAndPath = ppszFiles[index];

			int i = dci->sFileAndPath.ReverseFind('\\');
			if (i != -1) {
				dci->sPath = dci->sFileAndPath.Mid(0, i + 1);
				dci->sFile = dci->sFileAndPath.Mid(i + 1, dci->sFileAndPath.GetLength());
			}

			CMString sFileWithQuotes = dci->sFile;

			// if spaces in the filename surround witrh quotes
			if (sFileWithQuotes.Find(' ', 0) != -1) {
				sFileWithQuotes.Insert(0, _T("\""));
				sFileWithQuotes.Insert(sFileWithQuotes.GetLength(), _T("\""));
			}

			dci->hContact = hContact;
			dci->sContactName = dbv.ptszVal;
			dci->iType = DCC_SEND;
			dci->bReverse = m_DCCPassive ? true : false;
			dci->bSender = true;
			dci->dwSize = size;

			// create new dcc object
			CDccSession* dcc = new CDccSession(this, dci);

			// keep track of all objects created
			AddDCCSession(dci, dcc);

			// need to make sure that %'s are doubled to avoid having chat interpret as color codes
			CMString sFileCorrect = dci->sFile;
			sFileCorrect.Replace(_T("%"), _T("%%"));

			// is it an reverse filetransfer (receiver acts as server)
			if (dci->bReverse) {
				TCHAR szTemp[256];
				PostIrcMessage(_T("/CTCP %s DCC SEND %s 200 0 %I64u %u"),
					dci->sContactName.c_str(), sFileWithQuotes.c_str(), dci->dwSize, dcc->iToken);

				mir_sntprintf(szTemp, SIZEOF(szTemp),
					TranslateT("DCC reversed file transfer request sent to %s [%s]"),
					dci->sContactName.c_str(), sFileCorrect.c_str());
				DoEvent(GC_EVENT_INFORMATION, 0, m_info.sNick.c_str(), szTemp, NULL, NULL, NULL, true, false);

				if (m_sendNotice) {
					mir_sntprintf(szTemp, SIZEOF(szTemp),
						_T("/NOTICE %s I am sending the file '\002%s\002' (%I64u kB) to you, please accept it. [Reverse transfer]"),
						dci->sContactName.c_str(), sFileCorrect.c_str(), dci->dwSize / 1024);
					PostIrcMessage(szTemp);
				}
			}
			else { // ... normal filetransfer.
				iPort = dcc->Connect();
				if (iPort) {
					TCHAR szTemp[256];
					PostIrcMessage(_T("/CTCP %s DCC SEND %s %u %u %I64u"),
						dci->sContactName.c_str(), sFileWithQuotes.c_str(), ulAdr, iPort, dci->dwSize);

					mir_sntprintf(szTemp, SIZEOF(szTemp),
						TranslateT("DCC file transfer request sent to %s [%s]"),
						dci->sContactName.c_str(), sFileCorrect.c_str());
					DoEvent(GC_EVENT_INFORMATION, 0, m_info.sNick.c_str(), szTemp, NULL, NULL, NULL, true, false);

					if (m_sendNotice) {
						mir_sntprintf(szTemp, SIZEOF(szTemp),
							_T("/NOTICE %s I am sending the file '\002%s\002' (%I64u kB) to you, please accept it. [IP: %s]"),
							dci->sContactName.c_str(), sFileCorrect.c_str(), dci->dwSize / 1024, (TCHAR*)_A2T(ConvertIntegerToIP(ulAdr)));
						PostIrcMessage(szTemp);
					}
				}
				else DoEvent(GC_EVENT_INFORMATION, 0, m_info.sNick.c_str(),
					TranslateT("DCC ERROR: Unable to bind local port"), NULL, NULL, NULL, true, false);
			}

			// fix for sending multiple files
			index++;
			while (ppszFiles[index]) {
				if (_taccess(ppszFiles[index], 0) == 0) {
					PostIrcMessage(_T("/DCC SEND %s %S"), dci->sContactName.c_str(), ppszFiles[index]);
				}
				index++;
			}

			db_free(&dbv);
		}
	}

	if (dci)
		return dci;
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendMessage - sends a message

struct TFakeAckParam
{
	__inline TFakeAckParam(MCONTACT _hContact, int _msgid) :
		hContact(_hContact), msgid(_msgid)
	{}

	MCONTACT hContact;
	int    msgid;
};

void __cdecl CIrcProto::AckMessageFail(void *info)
{
	ProtoBroadcastAck((MCONTACT)info, ACKTYPE_MESSAGE, ACKRESULT_FAILED, NULL, (LPARAM)Translate("The protocol is not online"));
}

void __cdecl CIrcProto::AckMessageFailDcc(void *info)
{
	ProtoBroadcastAck((MCONTACT)info, ACKTYPE_MESSAGE, ACKRESULT_FAILED, NULL, (LPARAM)Translate("The dcc chat connection is not active"));
}

void __cdecl CIrcProto::AckMessageSuccess(void *info)
{
	TFakeAckParam *param = (TFakeAckParam*)info;
	ProtoBroadcastAck(param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)param->msgid, 0);
	delete param;
}

int __cdecl CIrcProto::SendMsg(MCONTACT hContact, int flags, const char* pszSrc)
{
	BYTE bDcc = getByte(hContact, "DCC", 0);
	WORD wStatus = getWord(hContact, "Status", ID_STATUS_OFFLINE);
	if (bDcc && wStatus != ID_STATUS_ONLINE) {
		ForkThread(&CIrcProto::AckMessageFailDcc, (void*)hContact);
		return 0;
	}
	if (!bDcc && (m_iStatus == ID_STATUS_OFFLINE || m_iStatus == ID_STATUS_CONNECTING)) {
		ForkThread(&CIrcProto::AckMessageFail, (void*)hContact);
		return 0;
	}

	int codepage = getCodepage();

	TCHAR *result;
	if (flags & PREF_UNICODE) {
		const char* p = strchr(pszSrc, '\0');
		if (p != pszSrc) {
			while (*(++p) == '\0')
				;
			result = mir_u2t_cp((wchar_t*)p, codepage);
		}
		else result = mir_a2t_cp(pszSrc, codepage);
	}
	else if (flags & PREF_UTF)
		mir_utf8decode(NEWSTR_ALLOCA(pszSrc), &result);
	else
		result = mir_a2t_cp(pszSrc, codepage);

	PostIrcMessageWnd(NULL, hContact, result);
	mir_free(result);

	int seq = InterlockedIncrement(&g_msgid);
	ForkThread(&CIrcProto::AckMessageSuccess, new TFakeAckParam(hContact, seq));
	return seq;
}

////////////////////////////////////////////////////////////////////////////////////////
// SendUrl

int __cdecl CIrcProto::SendUrl(MCONTACT, int, const char*)
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SetApparentMode - sets the visibility status

int __cdecl CIrcProto::SetApparentMode(MCONTACT, int)
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// SetStatus - sets the protocol status

int __cdecl CIrcProto::SetStatus(int iNewStatus)
{
	return SetStatusInternal(iNewStatus, false);
}

int CIrcProto::SetStatusInternal(int iNewStatus, bool bIsInternal)
{
	if (iNewStatus != ID_STATUS_OFFLINE && !m_network[0]) {
		if (m_nick[0] && !m_disableDefaultServer) {
			CQuickDlg* dlg = new CQuickDlg(this);
			dlg->GetProto()->m_quickComboSelection = dlg->GetProto()->m_serverComboSelection + 1;
			dlg->Show();
			HWND hwnd = dlg->GetHwnd();
			SetWindowTextA(hwnd, "Miranda IRC");
			SetDlgItemText(hwnd, IDC_TEXT, TranslateT("Please choose an IRC-network to go online. This network will be the default."));
			SetDlgItemText(hwnd, IDC_CAPTION, TranslateT("Default network"));
			WindowSetIcon(hwnd, IDI_MAIN);
			ShowWindow(hwnd, SW_SHOW);
			SetActiveWindow(hwnd);
		}
		return 0;
	}

	if (iNewStatus != ID_STATUS_OFFLINE && !m_nick[0] || !m_userID[0] || !m_name[0]) {
		MIRANDASYSTRAYNOTIFY msn;
		msn.cbSize = sizeof(MIRANDASYSTRAYNOTIFY);
		msn.szProto = m_szModuleName;
		msn.tszInfoTitle = TranslateT("IRC error");
		msn.tszInfo = TranslateT("Connection cannot be established! You have not completed all necessary fields (Nickname, User ID and m_name).");
		msn.dwInfoFlags = NIIF_ERROR | NIIF_INTERN_UNICODE;
		msn.uTimeout = 15000;
		CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM)&msn);
		return 0;
	}

	if (!bIsInternal)
		m_iDesiredStatus = iNewStatus;

	if ((iNewStatus == ID_STATUS_ONLINE || iNewStatus == ID_STATUS_AWAY || iNewStatus == ID_STATUS_FREECHAT) && !IsConnected()) //go from offline to online
	{
		if (!m_bConnectThreadRunning)
			ConnectToServer();
	}
	else if ((iNewStatus == ID_STATUS_ONLINE || iNewStatus == ID_STATUS_FREECHAT) && IsConnected() && m_iStatus == ID_STATUS_AWAY) //go to online while connected
	{
		m_statusMessage = _T("");
		PostIrcMessage(_T("/AWAY"));
		return 0;
	}
	else if (iNewStatus == ID_STATUS_OFFLINE && IsConnected()) //go from online/away to offline
		DisconnectFromServer();
	else if (iNewStatus == ID_STATUS_OFFLINE && !IsConnected()) //offline to offline
	{
		return 0;
	}
	else if (iNewStatus == ID_STATUS_AWAY && IsConnected()) //go to away while connected
	{
		PostIrcMessage(_T("/AWAY %s"), m_statusMessage.Mid(0, 450).c_str());
		return 0;
	}
	else if (iNewStatus == ID_STATUS_ONLINE && IsConnected()) //already online
		return 0;
	else
		SetStatusInternal(ID_STATUS_AWAY, true);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// GetAwayMsg - returns a contact's away message

HANDLE __cdecl CIrcProto::GetAwayMsg(MCONTACT hContact)
{
	WhoisAwayReply = _T("");
	DBVARIANT dbv;

	// bypass chat contacts.
	if (!isChatRoom(hContact)) {
		if (hContact && !getTString(hContact, "Nick", &dbv)) {
			int i = getWord(hContact, "Status", ID_STATUS_OFFLINE);
			if (i != ID_STATUS_AWAY) {
				db_free(&dbv);
				return 0;
			}
			CMString S = _T("WHOIS ");
			S += dbv.ptszVal;
			if (IsConnected())
				SendIrcMessage(S.c_str(), false);
			db_free(&dbv);
		}
	}

	return (HANDLE)1;
}

////////////////////////////////////////////////////////////////////////////////////////
// PSR_AWAYMSG

int __cdecl CIrcProto::RecvAwayMsg(MCONTACT, int, PROTORECVEVENT*)
{
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SetAwayMsg - sets the away status message

int __cdecl CIrcProto::SetAwayMsg(int status, const TCHAR* msg)
{
	switch (status) {
	case ID_STATUS_ONLINE:     case ID_STATUS_INVISIBLE:   case ID_STATUS_FREECHAT:
	case ID_STATUS_CONNECTING: case ID_STATUS_OFFLINE:
		break;

	default:
		CMString newStatus = msg;
		newStatus.Replace(_T("\r\n"), _T(" "));
		if (m_statusMessage.IsEmpty() || msg == NULL || m_statusMessage != newStatus) {
			if (msg == NULL || *msg == 0)
				m_statusMessage = STR_AWAYMESSAGE;
			else
				m_statusMessage = newStatus;

			if (m_iStatus == ID_STATUS_AWAY)
				PostIrcMessage(_T("/AWAY %s"), m_statusMessage.Mid(0, 450).c_str());
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// UserIsTyping - sends a UTN notification

int __cdecl CIrcProto::UserIsTyping(MCONTACT, int)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnEvent - maintain protocol events

int __cdecl CIrcProto::OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam)
{
	switch (eventType) {
	case EV_PROTO_ONLOAD:    return OnModulesLoaded(0, 0);
	case EV_PROTO_ONEXIT:    return OnPreShutdown(0, 0);
	case EV_PROTO_ONOPTIONS: return OnInitOptionsPages(wParam, lParam);

	case EV_PROTO_ONMENU:
		InitMainMenus();
		break;

	case EV_PROTO_ONRENAME:
		if (hMenuRoot) {
			CLISTMENUITEM mi = { sizeof(mi) };
			mi.flags = CMIM_NAME | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
			mi.ptszName = m_tszUserName;
			Menu_ModifyItem(hMenuRoot, &mi);
		}
		break;

	case EV_PROTO_ONCONTACTDELETED:
		return OnContactDeleted(wParam, lParam);

	case EV_PROTO_DBSETTINGSCHANGED:
		return OnDbSettingChanged(wParam, lParam);
	}
	return 1;
}
