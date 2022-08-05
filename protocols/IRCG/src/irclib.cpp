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
#include "stdafx.h"

#define DCCCHATTIMEOUT 300
#define DCCSENDTIMEOUT 120

using namespace irc;

int CDccSession::nDcc = 0;

static int CompareHandlers( const CIrcHandler* p1, const CIrcHandler* p2 )
{
	return mir_wstrcmp( p1->m_name, p2->m_name );
}

OBJLIST<CIrcHandler> CIrcProto::m_handlers( 30, CompareHandlers );

////////////////////////////////////////////////////////////////////

CIrcMessage::CIrcMessage( CIrcProto* _pro, const wchar_t* lpszCmdLine, int codepage, bool bIncoming, bool bNotify ) :
	m_proto( _pro ),
	m_bIncoming( bIncoming ),
	m_bNotify( bNotify ),
	m_codePage( codepage ),
	parameters( 10 )
{
	ParseIrcCommand(lpszCmdLine);
}

CIrcMessage::CIrcMessage(const CIrcMessage& m) :
	sCommand( m.sCommand ),
	m_bIncoming( m.m_bIncoming ), 
	m_bNotify( m.m_bNotify ),
	m_codePage( m.m_codePage ),
	m_proto( m.m_proto ),
	parameters( m.parameters.getCount())
{
	prefix.sNick = m.prefix.sNick;
	prefix.sUser = m.prefix.sUser;
	prefix.sHost = m.prefix.sHost;

	for (auto &it : m.parameters)
		parameters.insert(new CMStringW(*it));
}

CIrcMessage::~CIrcMessage()
{
}

void CIrcMessage::Reset()
{
	prefix.sNick = prefix.sUser = prefix.sHost = sCommand = L"";
	m_bIncoming = false;
	m_bNotify = true;

	parameters.destroy();
}

CIrcMessage& CIrcMessage::operator=(const CIrcMessage& m)
{
	if (&m != this) {
		sCommand = m.sCommand;
		parameters = m.parameters;
		prefix.sNick = m.prefix.sNick;
		prefix.sUser = m.prefix.sUser;
		prefix.sHost = m.prefix.sHost;
		m_bIncoming = m.m_bIncoming;
		m_bNotify = m.m_bNotify;
	}
	return *this;
}

CIrcMessage& CIrcMessage::operator=(const wchar_t* lpszCmdLine)
{
	Reset();
	ParseIrcCommand(lpszCmdLine);
	return *this;
}

void CIrcMessage::ParseIrcCommand(const wchar_t* lpszCmdLine)
{
	const wchar_t* p1 = lpszCmdLine;
	const wchar_t* p2 = lpszCmdLine;

	// prefix exists ?
	if (*p1 == ':') {
		// break prefix into its components (nick!user@host)
		p2 = ++p1;
		while (*p2 && !wcschr(L" !", *p2))
			++p2;
		prefix.sNick.SetString(p1, p2 - p1);
		if (*p2 != '!')
			goto end_of_prefix;
		p1 = ++p2;
		while (*p2 && !wcschr(L" @", *p2))
			++p2;
		prefix.sUser.SetString(p1, p2 - p1);
		if (*p2 != '@')
			goto end_of_prefix;
		p1 = ++p2;
		while (*p2 && *p2 != ' ')
			++p2;
		prefix.sHost.SetString(p1, p2 - p1);
end_of_prefix:
		while (*p2 && *p2 == ' ')
			++p2;
		p1 = p2;
	}

	// get command
	p2 = p1;
	while (*p2 && *p2 != ' ')
		++p2;

	sCommand.SetString(p1, p2 - p1);
	sCommand.MakeUpper();
	while (*p2 && *p2 == ' ')
		++p2;
	p1 = p2;

	// get parameters
	while (*p1) {
		if (*p1 == ':') {
			++p1;

			// seek end-of-message
			while (*p2)
				++p2;
			parameters.insert(new CMStringW(p1, p2 - p1));
			break;
		}
		else {
			// seek end of parameter
			while (*p2 && *p2 != ' ')
				++p2;
			parameters.insert(new CMStringW(p1, p2 - p1));
			// see next parameter
			while (*p2 && *p2 == ' ')
				++p2;
			p1 = p2;
		}
	}
}

////////////////////////////////////////////////////////////////////

int CIrcProto::getCodepage() const
{
	return (con != nullptr) ? codepage : CP_ACP;
}

void CIrcProto::SendIrcMessage(const wchar_t* msg, bool bNotify, int cp)
{
	if (cp == -1)
		cp = getCodepage();

	if (this) {
		char* str = mir_u2a_cp(msg, cp);
		rtrim(str);
		int cbLen = (int)mir_strlen(str);
		str = (char*)mir_realloc(str, cbLen + 3);
		mir_strcat(str, "\r\n");
		NLSend((const uint8_t*)str, cbLen + 2);
		mir_free(str);

		if (bNotify) {
			CIrcMessage ircMsg(this, msg, cp);
			if (!ircMsg.sCommand.IsEmpty() && ircMsg.sCommand != L"QUIT")
				Notify(&ircMsg);
		}
	}
}

bool CIrcProto::Connect(const CIrcSessionInfo& info)
{
	codepage = m_codepage;

	con = Netlib_OpenConnection(m_hNetlibUser, info.sServer, info.iPort);
	if (con == nullptr) {
		wchar_t szTemp[300];
		mir_snwprintf(szTemp, L"%c5%s %c%s%c (%S: %u).", irc::COLOR, TranslateT("Failed to connect to"), irc::BOLD, m_tszUserName, irc::BOLD, m_sessionInfo.sServer.c_str(), m_sessionInfo.iPort);
		DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, nullptr, szTemp, nullptr, nullptr, NULL, true, false);
		return false;
	}

	FindLocalIP(con); // get the local ip used for filetransfers etc

	if (info.m_iSSL > 0) {
		if (!Netlib_StartSsl(con, nullptr) && info.m_iSSL == 2) {
			Netlib_CloseHandle(con);
			con = nullptr;
			m_info.Reset();
			return false;
		}
	}

	if (Miranda_IsTerminated()) {
		Disconnect();
		return false;
	}

	m_info = info;

	if (m_bUseSASL)
		NLSend("CAP REQ :sasl\r\n");

	// start receiving messages from host
	ForkThread(&CIrcProto::ThreadProc, nullptr);
	Sleep(100);

	if (!m_bUseSASL && info.sPassword.GetLength())
		NLSend("PASS %s\r\n", info.sPassword.c_str());

	NLSend(L"NICK %s\r\n", info.sNick.c_str());

	CMStringW userID = GetWord(info.sUserID.c_str(), 0);
	wchar_t szHostName[MAX_PATH];
	DWORD cbHostName = _countof(szHostName);
	GetComputerName(szHostName, &cbHostName);
	CMStringW HostName = GetWord(szHostName, 0);
	if (userID.IsEmpty())
		userID = L"Miranda";
	if (HostName.IsEmpty())
		HostName = L"host";
	NLSend(L"USER %s %s %s :%s\r\n", userID.c_str(), HostName.c_str(), L"server", info.sFullName.c_str());

	return con != nullptr;
}

void CIrcProto::Disconnect(void)
{
	static const uint32_t dwServerTimeout = 5 * 1000;

	if (con == nullptr)
		return;

	KillIdent();

	if (m_quitMessage[0])
		NLSend(L"QUIT :%s\r\n", m_quitMessage);
	else
		NLSend("QUIT \r\n");

	Sleep(50);

	if (con)
		Netlib_Shutdown(con);

	m_info.Reset();
}

void CIrcProto::Notify(const CIrcMessage* pmsg)
{
	OnIrcMessage(pmsg);
}

int CIrcProto::NLSend(const unsigned char* buf, int cbBuf)
{
	if (!con || !buf)
		return 0;
	
	if (cbBuf == 0)
		cbBuf = (int)mir_strlen((const char *)buf);
	
	return Netlib_Send(con, (const char*)buf, cbBuf, MSG_DUMPASTEXT);
}

int CIrcProto::NLSend(const wchar_t* fmt, ...)
{
	va_list marker;
	va_start(marker, fmt);

	wchar_t szBuf[1024 * 4];
	mir_vsnwprintf(szBuf, _countof(szBuf), fmt, marker);
	va_end(marker);

	char* buf = mir_u2a_cp(szBuf, getCodepage());
	int result = NLSend((unsigned char*)buf, (int)mir_strlen(buf));
	mir_free(buf);
	return result;
}

int CIrcProto::NLSend(const char* fmt, ...)
{
	va_list marker;
	va_start(marker, fmt);

	char szBuf[1024 * 4];
	int cbLen = mir_vsnprintf(szBuf, _countof(szBuf), fmt, marker);
	va_end(marker);

	return NLSend((unsigned char*)szBuf, cbLen);
}

int CIrcProto::NLSendNoScript(const unsigned char* buf, int cbBuf)
{
	if (con)
		return Netlib_Send(con, (const char*)buf, cbBuf, MSG_DUMPASTEXT);

	return 0;
}

int CIrcProto::NLReceive(unsigned char* buf, int cbBuf)
{
	return Netlib_Recv(con, (char*)buf, cbBuf, MSG_DUMPASTEXT);
}

void CIrcProto::KillIdent()
{
	if (hBindPort) {
		HANDLE hPort = hBindPort;
		hBindPort = nullptr;
		Netlib_CloseHandle(hPort);
	}
}

void CIrcProto::InsertIncomingEvent(wchar_t* pszRaw)
{
	CIrcMessage msg(this, pszRaw, true);
	Notify(&msg);
}

void CIrcProto::DoReceive()
{
	char chBuf[1024 * 4 + 1];
	int cbInBuf = 0;

	if (m_info.bIdentServer && m_info.iIdentServerPort != NULL) {
		NETLIBBIND nb = {};
		nb.pfnNewConnection = DoIdent;
		nb.pExtra = this;
		nb.wPort = m_info.iIdentServerPort;
		
		hBindPort = Netlib_BindPort(m_hNetlibUser, &nb);
		if (!hBindPort || nb.wPort != m_info.iIdentServerPort) {
			debugLogA("Error: unable to bind local port %u", m_info.iIdentServerPort);
			KillIdent();
		}
	}

	while (con) {
		int nLinesProcessed = 0;

		int cbRead = NLReceive((unsigned char*)chBuf + cbInBuf, sizeof(chBuf)-cbInBuf - 1);
		if (cbRead <= 0)
			break;

		cbInBuf += cbRead;
		chBuf[cbInBuf] = 0;

		char* pStart = chBuf;
		while (*pStart) {
			char* pEnd;

			// seek end-of-line
			for (pEnd = pStart; *pEnd && *pEnd != '\r' && *pEnd != '\n'; ++pEnd)
				;
			if (*pEnd == 0)
				break; // uncomplete message. stop parsing.

			++nLinesProcessed;

			// replace end-of-line with NULLs and skip
			while (*pEnd == '\r' || *pEnd == '\n')
				*pEnd++ = 0;

			// process single message by monitor objects
			if (*pStart) {
				ptrW ptszMsg;
				if (codepage != CP_UTF8 && m_utfAutodetect && Utf8CheckString(pStart))
					ptszMsg = mir_utf8decodeW(pStart);
				
				if (ptszMsg == nullptr)
					ptszMsg = mir_a2u_cp(pStart, codepage);

				CIrcMessage msg(this, ptszMsg, codepage, true);
				Notify(&msg);
			}

			cbInBuf -= pEnd - pStart;
			pStart = pEnd;
		}

		// discard processed messages
		if (nLinesProcessed != 0)
			memmove(chBuf, pStart, cbInBuf + 1);
	}

	if (con) {
		Netlib_CloseHandle(con);
		con = nullptr;
	}

	// notify monitor objects that the connection has been closed
	Notify(nullptr);
}

void __cdecl CIrcProto::ThreadProc(void*)
{
	Thread_SetName("IRC: CIrcProto");
	DoReceive();
	m_info.Reset();
}

void CIrcProto::AddDCCSession(MCONTACT, CDccSession *dcc)
{
	mir_cslock lck(m_dcc);

	CDccSession* p = m_dcc_chats.find(dcc);
	if (p)
		m_dcc_chats.remove(p);

	m_dcc_chats.insert(dcc);
}

void CIrcProto::AddDCCSession(DCCINFO*, CDccSession *dcc)
{
	mir_cslock lck(m_dcc);
	m_dcc_xfers.insert(dcc);
}

void CIrcProto::RemoveDCCSession(MCONTACT hContact)
{
	mir_cslock lck(m_dcc);

	for (auto &it : m_dcc_chats)
		if (it->di->hContact == hContact) {
			m_dcc_chats.removeItem(&it);
			break;
		}
}

void CIrcProto::RemoveDCCSession(DCCINFO *pdci)
{
	mir_cslock lck(m_dcc);

	for (auto &it : m_dcc_xfers) {
		if (it->di == pdci) {
			m_dcc_xfers.removeItem(&it);
			break;
		}
	}
}

CDccSession* CIrcProto::FindDCCSession(MCONTACT hContact)
{
	mir_cslock lck(m_dcc);

	for (auto &it : m_dcc_chats)
		if (it->di->hContact == hContact)
			return it;

	return nullptr;
}

CDccSession* CIrcProto::FindDCCSession(DCCINFO *pdci)
{
	mir_cslock lck(m_dcc);

	for (auto &it : m_dcc_xfers)
		if (it->di == pdci)
			return it;

	return nullptr;
}

CDccSession* CIrcProto::FindDCCSendByPort(int iPort)
{
	mir_cslock lck(m_dcc);

	for (auto &p : m_dcc_xfers)
		if (p->di->iType == DCC_SEND && p->di->bSender && iPort == p->di->iPort)
			return p;

	return nullptr;
}

CDccSession* CIrcProto::FindDCCRecvByPortAndName(int iPort, const wchar_t* szName)
{
	mir_cslock lck(m_dcc);

	for (auto &p : m_dcc_xfers) {
		DBVARIANT dbv;
		if (!getWString(p->di->hContact, "Nick", &dbv)) {
			if (p->di->iType == DCC_SEND && !p->di->bSender && !mir_wstrcmpi(szName, dbv.pwszVal) && iPort == p->di->iPort) {
				db_free(&dbv);
				return p;
			}
			db_free(&dbv);
		}
	}

	return nullptr;
}

CDccSession* CIrcProto::FindPassiveDCCSend(int iToken)
{
	mir_cslock lck(m_dcc);

	for (auto &it : m_dcc_xfers)
		if (it->iToken == iToken)
			return it;

	return nullptr;
}

CDccSession* CIrcProto::FindPassiveDCCRecv(CMStringW sName, CMStringW sToken)
{
	mir_cslock lck(m_dcc);

	for (auto &p : m_dcc_xfers)
		if (sToken == p->di->sToken && sName == p->di->sContactName)
			return p;

	return nullptr;
}

void CIrcProto::DisconnectAllDCCSessions(bool Shutdown)
{
	if (m_disconnectDCCChats || Shutdown) {
		mir_cslock lck(m_dcc);
		for (auto &it : m_dcc_chats)
			it->Disconnect();
	}
}

void CIrcProto::CheckDCCTimeout(void)
{
	mir_cslock lck(m_dcc);

	for (auto &p : m_dcc_chats)
		if (time(0) > p->tLastActivity + DCCCHATTIMEOUT)
			p->Disconnect();

	for (auto &p : m_dcc_xfers)
		if (time(0) > p->tLastActivity + DCCSENDTIMEOUT)
			p->Disconnect();
}

////////////////////////////////////////////////////////////////////

CIrcIgnoreItem::CIrcIgnoreItem(const wchar_t* _mask, const wchar_t* _flags) :
	mask(_mask),
	flags(_flags)
{
}

CIrcIgnoreItem::CIrcIgnoreItem(int codepage, const char* _mask, const char* _flags) :
	mask((wchar_t*)_A2T(_mask, codepage)),
	flags((wchar_t*)_A2T(_flags, codepage))
{
}

CIrcIgnoreItem::~CIrcIgnoreItem()
{
}

////////////////////////////////////////////////////////////////////

CIrcSessionInfo::CIrcSessionInfo() :
	iPort(0),
	bIdentServer(false),
	iIdentServerPort(0)
{
}

CIrcSessionInfo::CIrcSessionInfo(const CIrcSessionInfo& si) :
	sServer(si.sServer),
	sServerName(si.sServerName),
	iPort(si.iPort),
	sNick(si.sNick),
	sUserID(si.sUserID),
	sFullName(si.sFullName),
	sPassword(si.sPassword),
	bIdentServer(si.bIdentServer),
	m_iSSL(si.m_iSSL),
	sIdentServerType(si.sIdentServerType),
	iIdentServerPort(si.iIdentServerPort)
{
}

void CIrcSessionInfo::Reset()
{
	sServer = "";
	sServerName = L"";
	iPort = 0;
	sNick = L"";
	sUserID = L"";
	sFullName = L"";
	sPassword = "";
	bIdentServer = false;
	bNickFlag = false;
	m_iSSL = 0;
	sIdentServerType = L"";
	iIdentServerPort = 0;
}

////////////////////////////////////////////////////////////////////

void CIrcProto::OnIrcMessage(const CIrcMessage* pmsg)
{
	if (pmsg != nullptr) {
		PfnIrcMessageHandler pfn = FindMethod(pmsg->sCommand.c_str());
		if (pfn) {
			// call member function. if it returns 'false',
			// call the default handling
			if (!(this->*pfn)(pmsg))
				OnIrcDefault(pmsg);
		}
		else // handler not found. call default handler
			OnIrcDefault(pmsg);
	}
	else OnIrcDisconnected();
}

PfnIrcMessageHandler CIrcProto::FindMethod(const wchar_t* lpszName)
{
	CIrcHandler temp(lpszName, nullptr);
	CIrcHandler* p = m_handlers.find(&temp);
	return (p == nullptr) ? nullptr : p->m_handler;
}

////////////////////////////////////////////////////////////////////

#define IPC_ADDR_SIZE				4		/* Size of IP address, change for IPv6. */

char* ConvertIntegerToIP(unsigned long int_ip_addr)
{
	IN_ADDR intemp;
	IN_ADDR in;
	intemp.S_un.S_addr = int_ip_addr;

	in.S_un.S_un_b.s_b1 = intemp.S_un.S_un_b.s_b4;
	in.S_un.S_un_b.s_b2 = intemp.S_un.S_un_b.s_b3;
	in.S_un.S_un_b.s_b3 = intemp.S_un.S_un_b.s_b2;
	in.S_un.S_un_b.s_b4 = intemp.S_un.S_un_b.s_b1;

	return inet_ntoa(in);
}

unsigned long ConvertIPToInteger(char* IP)
{
	IN_ADDR in;
	IN_ADDR intemp;

	if (IP == nullptr || mir_strlen(IP) == 0)
		return 0;

	intemp.S_un.S_addr = inet_addr(IP);

	in.S_un.S_un_b.s_b1 = intemp.S_un.S_un_b.s_b4;
	in.S_un.S_un_b.s_b2 = intemp.S_un.S_un_b.s_b3;
	in.S_un.S_un_b.s_b3 = intemp.S_un.S_un_b.s_b2;
	in.S_un.S_un_b.s_b4 = intemp.S_un.S_un_b.s_b1;
	return in.S_un.S_addr;
}

////////////////////////////////////////////////////////////////////

// initialize basic stuff needed for the dcc objects, also start a timer for checking the status of connections (timeouts)
CDccSession::CDccSession(CIrcProto* _pro, DCCINFO *pdci) :
	m_proto(_pro),
	NewFileName(nullptr),
	dwWhatNeedsDoing(0),
	tLastPercentageUpdate(0),
	dwTotal(0),
	iGlobalToken(),
	dwResumePos(0),
	hEvent(nullptr),
	con(nullptr),
	hBindPort(nullptr)
{
	tLastActivity = time(0);

	di = pdci; // Setup values passed to the constructor

	memset(&pfts, 0, sizeof(PROTOFILETRANSFERSTATUS));

	if (di->iType == DCC_SEND && di->bSender == false)
		hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

	if (nDcc == 0)
		m_proto->SetChatTimer(m_proto->DCCTimer, 20 * 1000, DCCTimerProc);

	nDcc++; // increase the count of existing objects

	iGlobalToken++;
	if (iGlobalToken == 1000)
		iGlobalToken = 1;
	iToken = iGlobalToken;

	iPacketSize = m_proto->getWord("PacketSize", 4096);

	if (di->dwAdr)
		m_proto->setDword(di->hContact, "IP", di->dwAdr); // mtooltip stuff
}

CDccSession::~CDccSession() // destroy all that needs destroying
{
	if (di->iType == DCC_SEND) {
		// ack SUCCESS or FAILURE
		if (dwTotal == di->dwSize)
			ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, di);
		else
			ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, di);
	}

	if (di->iType == DCC_CHAT) {
		CDccSession *dcc = m_proto->FindDCCSession(di->hContact);
		if (dcc && this == dcc) {
			m_proto->RemoveDCCSession(di->hContact); // objects automatically remove themselves from the list of objects
			m_proto->setWord(di->hContact, "Status", ID_STATUS_OFFLINE);
		}
	}

	if (di->iType == DCC_SEND)
		m_proto->RemoveDCCSession(di);

	if (hEvent != nullptr) {
		SetEvent(hEvent);
		CloseHandle(hEvent);
		hEvent = nullptr;
	}

	delete di;
	nDcc--;

	if (nDcc < 0)
		nDcc = 0;

	if (nDcc == 0)
		m_proto->KillChatTimer(m_proto->DCCTimer); // destroy the timer when no dcc objects remain
}

int CDccSession::NLSend(const unsigned char* buf, int cbBuf)
{
	tLastActivity = time(0);

	if (con)
		return Netlib_Send(con, (const char*)buf, cbBuf, di->iType == DCC_CHAT ? MSG_DUMPASTEXT : MSG_NODUMP);

	return 0;
}

int CDccSession::NLReceive(const unsigned char* buf, int cbBuf)
{
	int n = 0;

	if (con)
		n = Netlib_Recv(con, (char*)buf, cbBuf, di->iType == DCC_CHAT ? MSG_DUMPASTEXT : MSG_NODUMP);

	tLastActivity = time(0);
	return n;
}

int CDccSession::SendStuff(const wchar_t* fmt)
{
	CMStringA buf = _T2A(fmt, m_proto->getCodepage());
	return NLSend((const unsigned char*)buf.c_str(), buf.GetLength());
}

// called when the user wants to connect/create a new connection given the parameters in the constructor.
int CDccSession::Connect()
{
	if (!di->bSender || di->bReverse) {
		if (!con)
			mir_forkthread(ConnectProc, this); // spawn a new thread for time consuming activities, ie when connecting to a remote computer
		return true;
	}

	if (!con)
		return SetupConnection(); // no need to spawn thread for setting up a listening port locally

	return false;
}

void __cdecl CDccSession::ConnectProc(void *pparam)
{
	Thread_SetName("IRC: ConnectProc");

	CDccSession* pThis = (CDccSession*)pparam;
	if (!pThis->con)
		pThis->SetupConnection();
}

// small function to setup the address and port of the remote computer fror passive filetransfers
void CDccSession::SetupPassive(uint32_t adress, uint32_t port)
{
	di->dwAdr = adress;
	di->iPort = (int)port;

	m_proto->setDword(di->hContact, "IP", di->dwAdr); // mtooltip stuff
}

int CDccSession::SetupConnection()
{
	// if it is a dcc chat connection make sure it is "offline" to begin with, since no connection exists still
	if (di->iType == DCC_CHAT)
		m_proto->setWord(di->hContact, "Status", ID_STATUS_OFFLINE);

	// Set up stuff needed for the filetransfer dialog (if it is a filetransfer)
	if (di->iType == DCC_SEND) {
		file[0] = (wchar_t*)di->sFileAndPath.c_str();
		file[1] = nullptr;

		pfts.szCurrentFile.w = (wchar_t*)di->sFileAndPath.c_str();
		pfts.szWorkingDir.w = (wchar_t*)di->sPath.c_str();
		pfts.hContact = di->hContact;
		pfts.flags = PFTS_UNICODE + ((di->bSender) ? PFTS_SENDING : PFTS_RECEIVING);
		pfts.totalFiles = 1;
		pfts.currentFileNumber = 0;
		pfts.totalBytes = di->dwSize;
		pfts.currentFileSize = pfts.totalBytes;
		pfts.pszFiles.w = file;
		pfts.totalProgress = 0;
		pfts.currentFileProgress = 0;
		pfts.currentFileTime = (unsigned long)time(0);
	}

	// create a listening socket for outgoing chat/send requests. The remote computer connects to this computer. Used for both chat and filetransfer.
	if (di->bSender && !di->bReverse) {
		NETLIBBIND nb = {};
		nb.pfnNewConnection = DoIncomingDcc; // this is the (helper) function to be called once an incoming connection is made. The 'real' function that is called is IncomingConnection()
		nb.pExtra = this;
		
		hBindPort = Netlib_BindPort(m_proto->hNetlibDCC, &nb);
		if (hBindPort == nullptr) {
			delete this; // dcc objects destroy themselves when the connection has been closed or failed for some reasson.
			return 0;
		}

		di->iPort = nb.wPort; // store the port internally so it is possible to search for it (for resuming of filetransfers purposes)
		return nb.wPort; // return the created port so it can be sent to the remote computer in a ctcp/dcc command
	}

	// If a remote computer initiates a chat session this is used to connect to the remote computer (user already accepted at this point). 
	// also used for connecting to a remote computer for remote file transfers
	if (di->iType == DCC_CHAT && !di->bSender || di->iType == DCC_SEND && di->bSender && di->bReverse)
		con = Netlib_OpenConnection(m_proto->hNetlibDCC, ConvertIntegerToIP(di->dwAdr), di->iPort);

	// If a remote computer initiates a filetransfer this is used to connect to that computer (the user has chosen to accept but it is possible the file exists/needs to be resumed etc still)
	if (di->iType == DCC_SEND && !di->bSender) {

		// this following code is for miranda to be able to show the resume/overwrite etc dialog if the file that we are receiving already exists. 
		// It must be done before we create the connection or else the other party will begin sending packets while the user is still deciding if 
		// s/he wants to resume/cancel or whatever. Just the way dcc is...

		// if the file exists (dialog is shown) WaitForSingleObject() till the dialog is closed and PS_FILERESUME has been processed. 
		// dwWhatNeedsDoing will be set using InterlockedExchange() (from other parts of the code depending on action) before SetEvent() is called.
		// If the user has chosen rename then InterlockedExchange() will be used for setting NewFileName to a string containing the new name.
		// Furthermore dwResumePos will be set using InterlockedExchange() to indicate what the file position to start from is.
		if (ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_FILERESUME, (void *)di, (LPARAM)&pfts)) {
			WaitForSingleObject(hEvent, INFINITE);
			switch (dwWhatNeedsDoing) {
			case FILERESUME_RENAME:
				// If the user has chosen to rename the file we need to change variables accordingly. NewFileName has been set using
				// InterlockedExchange()
				if (NewFileName) { // the user has chosen to rename the new incoming file.
					di->sFileAndPath = NewFileName;

					int i = di->sFileAndPath.ReverseFind('\\');
					if (i != -1) {
						di->sPath = di->sFileAndPath.Mid(0, i + 1);
						di->sFile = di->sFileAndPath.Mid(i + 1);
					}

					pfts.szCurrentFile.w = di->sFileAndPath.GetBuffer();
					pfts.szWorkingDir.w = di->sPath.GetBuffer();
					pfts.totalBytes = di->dwSize;
					pfts.currentFileSize = pfts.totalBytes;

					delete[] NewFileName;
					NewFileName = nullptr;
				}
				break;

			case FILERESUME_OVERWRITE:
			case FILERESUME_RESUME:
				// no action needed at this point, just break out of the switch statement
				break;

			case FILERESUME_CANCEL:
				return FALSE;

			case FILERESUME_SKIP:
			default:
				delete this; // per usual dcc objects destroy themselves when they fail or when connection is closed
				return FALSE;
			}
		}

		// hack for passive filetransfers
		if (di->iType == DCC_SEND && !di->bSender && di->bReverse) {
			NETLIBBIND nb = {};
			nb.pfnNewConnection = DoIncomingDcc; // this is the (helper) function to be called once an incoming connection is made. The 'real' function that is called is IncomingConnection()
			nb.pExtra = this;

			hBindPort = Netlib_BindPort(m_proto->hNetlibDCC, &nb);
			if (hBindPort == nullptr) {
				m_proto->DoEvent(GC_EVENT_INFORMATION, nullptr, m_proto->m_info.sNick.c_str(), LPGENW("DCC ERROR: Unable to bind local port for passive file transfer"), nullptr, nullptr, NULL, true, false);
				delete this; // dcc objects destroy themselves when the connection has been closed or failed for some reasson.
				return 0;
			}

			di->iPort = nb.wPort; // store the port internally so it is possible to search for it (for resuming of filetransfers purposes)

			CMStringW sFileWithQuotes = di->sFile;

			// if spaces in the filename surround with quotes
			if (sFileWithQuotes.Find(' ', 0) != -1) {
				sFileWithQuotes.Insert(0, L"\"");
				sFileWithQuotes.Insert(sFileWithQuotes.GetLength(), L"\"");
			}

			// send out DCC RECV command for passive filetransfers
			unsigned long ulAdr = 0;
			if (m_proto->m_manualHost)
				ulAdr = ConvertIPToInteger(m_proto->m_mySpecifiedHostIP);
			else
				ulAdr = m_proto->m_IPFromServer ? ConvertIPToInteger(m_proto->m_myHost) : nb.dwExternalIP;

			if (di->iPort && ulAdr)
				m_proto->PostIrcMessage(L"/CTCP %s DCC SEND %s %u %u %I64u %s", di->sContactName.c_str(), sFileWithQuotes.c_str(), ulAdr, di->iPort, di->dwSize, di->sToken.c_str());

			return TRUE;
		}

		// connect to the remote computer from which you are receiving the file (now all actions to take (resume/overwrite etc) have been decided
		con = Netlib_OpenConnection(m_proto->hNetlibDCC, ConvertIntegerToIP(di->dwAdr), di->iPort);
	}

	// if for some reason the plugin has failed to connect to the remote computer the object is destroyed.
	if (con == nullptr) {
		delete this;
		return FALSE; // failed to connect
	}

	// if it is a chat connection set the user to online now since we now know there is a connection
	if (di->iType == DCC_CHAT)
		m_proto->setWord(di->hContact, "Status", ID_STATUS_ONLINE);

	// spawn a new thread to handle receiving/sending of data for the new chat/filetransfer connection to the remote computer
	mir_forkthread(ThreadProc, this);

	return con != nullptr;
}

// called by netlib for incoming connections on a listening socket (chat/filetransfer)
int CDccSession::IncomingConnection(HNETLIBCONN hConnection, uint32_t dwIP)
{
	con = hConnection;
	if (con == nullptr) {
		delete this;
		return false; // failed to connect
	}

	m_proto->setDword(di->hContact, "IP", dwIP); // mToolTip stuff

	if (di->iType == DCC_CHAT)
		m_proto->setWord(di->hContact, "Status", ID_STATUS_ONLINE); // set chat to online

	// same as above, spawn a new thread to handle receiving/sending of data for the new incoming chat/filetransfer connection  
	mir_forkthread(ThreadProc, this);
	return true;
}

// here we decide which function to use for communicating with the remote computer, depending on connection type
void __cdecl CDccSession::ThreadProc(void *pparam)
{
	Thread_SetName("IRC: CDccSession::ThreadProc");

	CDccSession* pThis = (CDccSession*)pparam;

	// if it is an incoming connection on a listening port, then we should close the listenting port so only one can connect (the one you offered
	// the connection to) can connect and not evil IRCopers with haxxored IRCDs
	if (pThis->hBindPort) {
		Netlib_CloseHandle(pThis->hBindPort);
		pThis->hBindPort = nullptr;
	}

	if (pThis->di->iType == DCC_CHAT)
		pThis->DoChatReceive(); // dcc chat

	else if (!pThis->di->bSender)
		pThis->DoReceiveFile(); // receive a file

	else if (pThis->di->bSender)
		pThis->DoSendFile(); // send a file
}

// this is done when the user is initiating a filetransfer to a remote computer
void CDccSession::DoSendFile()
{
	// initialize the filetransfer dialog
	ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, di);
	ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, di);

	uint16_t wPacketSize = m_proto->getWord("DCCPacketSize", 1024 * 4);

	if (wPacketSize < 256)
		wPacketSize = 256;

	if (wPacketSize > 32 * 1024)
		wPacketSize = 32 * 1024;

	uint8_t* chBuf = new uint8_t[wPacketSize + 1];

	// is there a connection?
	if (con) {
		// open the file for reading
		int hFile = _wopen(di->sFileAndPath.c_str(), _O_RDONLY | _O_BINARY, _S_IREAD);
		if (hFile >= 0) {
			unsigned __int64 dwLastAck = 0;

			// if the user has chosen to resume a file, dwResumePos will contain a value (set using InterlockedExchange())
			// and then the variables and the file pointer are changed accordingly.
			if (dwResumePos && dwWhatNeedsDoing == FILERESUME_RESUME) {
				_lseeki64(hFile, dwResumePos, SEEK_SET);
				dwTotal = dwResumePos;
				dwLastAck = dwResumePos;
				pfts.totalProgress = dwResumePos;
				pfts.currentFileProgress = dwResumePos;
			}

			// initial ack to set the 'percentage-ready meter' to the correct value
			ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_DATA, di, (LPARAM)&pfts);

			tLastActivity = time(0);

			// create a packet receiver to handle receiving ack's from the remote computer.
			HANDLE hPackrcver = Netlib_CreatePacketReceiver(con, sizeof(uint32_t));

			NETLIBPACKETRECVER npr = {};
			npr.dwTimeout = 60 * 1000;
			npr.bufferSize = sizeof(uint32_t);

			// until the connection is dropped it will spin around in this while() loop
			while (con) {
				// read a packet
				int iRead = _read(hFile, chBuf, wPacketSize);
				if (iRead <= 0)
					break; // break out if everything has already been read

				// send the package
				int cbSent = NLSend((unsigned char*)chBuf, iRead);
				if (cbSent <= 0)
					break; // break out if connection is lost or a transmission error has occured

				if (!con)
					break;

				dwTotal += cbSent;

				// block connection and receive ack's from remote computer (if applicable)
				if (m_proto->m_DCCMode == 0) {
					uint32_t dwRead = 0;
					uint32_t dwPacket = NULL;

					do {
						dwRead = Netlib_GetMorePackets(hPackrcver, &npr);
						npr.bytesUsed = sizeof(uint32_t);

						if (dwRead <= 0)
							break; // connection closed, or a timeout occurred.

						dwPacket = *(uint32_t*)npr.buffer;
						dwLastAck = ntohl(dwPacket);

					}
					while (con && dwTotal != dwLastAck);

					if (!con || dwRead <= 0)
						goto DCC_STOP;
				}

				if (m_proto->m_DCCMode == 1) {
					uint32_t dwRead = 0;
					uint32_t dwPacket = 0;

					do {
						dwRead = Netlib_GetMorePackets(hPackrcver, &npr);
						npr.bytesUsed = sizeof(uint32_t);
						if (dwRead <= 0)
							break; // connection closed, or a timeout occurred.

						dwPacket = *(uint32_t*)npr.buffer;
						dwLastAck = ntohl(dwPacket);
					}
					while (con && (di->dwSize != dwTotal
					&& dwTotal - dwLastAck >= 100 * 1024
					|| di->dwSize == dwTotal // get the last packets when the whole file has been sent
					&& dwTotal != dwLastAck));

					if (!con || dwRead <= 0)
						goto DCC_STOP;
				}

				// update the filetransfer dialog's 'percentage-ready meter' once per second only to save cpu
				if (tLastPercentageUpdate < time(0)) {
					tLastPercentageUpdate = time(0);
					pfts.totalProgress = dwTotal;
					pfts.currentFileProgress = dwTotal;
					ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_DATA, di, (LPARAM)&pfts);
				}

				// close the connection once the whole file has been sent an completely ack'ed
				if (dwLastAck >= di->dwSize) {
					Netlib_CloseHandle(con);
					con = nullptr;
				}
			}

		DCC_STOP:
			// need to close the connection if it isn't allready
			if (con) {
				Netlib_CloseHandle(con);
				con = nullptr;
			}

			// ack the progress one final time
			tLastActivity = time(0);
			pfts.totalProgress = dwTotal;
			pfts.currentFileProgress = dwTotal;
			ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_DATA, di, (LPARAM)&pfts);

			_close(hFile);
		}
		else { // file was not possible to open for reading
			ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, di);
			if (con) {
				Netlib_CloseHandle(con);
				con = nullptr;
			}
		}
	}

	delete[]chBuf;
	delete this; // ... and hopefully all went right, cuz here the object is deleted in any case
}

// This is called when receiving a file from a remote computer.
void CDccSession::DoReceiveFile()
{
	// initialize the filetransfer dialog
	ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, di);

	uint8_t chBuf[1024 * 32 + 1];

	// do some stupid thing so  the filetransfer dialog shows the right thing
	ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, di);

	// open the file for writing (and reading in case it is a resume)
	int hFile = _wopen(di->sFileAndPath.c_str(),
		(dwWhatNeedsDoing == FILERESUME_RESUME ? _O_APPEND : _O_TRUNC | _O_CREAT) | _O_RDWR | _O_BINARY,
		_S_IREAD | _S_IWRITE);
	if (hFile >= 0) {
		unsigned __int64 dwLastAck = 0;

		// dwResumePos and dwWhatNeedsDoing has possibly been set using InterlockedExchange()
		// if set it is a resume and we adjust variables and the file pointer accordingly.
		if (dwResumePos && dwWhatNeedsDoing == FILERESUME_RESUME) {
			_lseeki64(hFile, dwResumePos, SEEK_SET);
			dwTotal = dwResumePos;
			dwLastAck = dwResumePos;
			pfts.totalProgress = dwResumePos;
			pfts.currentFileProgress = dwResumePos;
		}

		// send an initial ack for the percentage-ready meter
		ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_DATA, di, (LPARAM)&pfts);

		// the while loop will spin around till the connection is dropped, locally or by the remote computer.
		while (con) {
			// read
			int cbRead = NLReceive((unsigned char*)chBuf, sizeof(chBuf));
			if (cbRead <= 0)
				break;

			// write it to the file
			_write(hFile, chBuf, cbRead);

			dwTotal += cbRead;

			// this snippet sends out an ack for every 4 kb received in send ahead
			// or every packet for normal mode
			if (!di->bTurbo) {
				uint32_t no = dwTotal;
				no = htonl(no);
				NLSend((unsigned char *)&no, sizeof(uint32_t));
				dwLastAck = dwTotal;
			}
			else dwLastAck = dwTotal;

			// sets the 'last update time' to check for timed out connections, and also make sure we only
			// ack the 'percentage-ready meter' only once a second to save CPU.
			if (tLastPercentageUpdate < time(0)) {
				tLastPercentageUpdate = time(0);
				pfts.totalProgress = dwTotal;
				pfts.currentFileProgress = dwTotal;
				ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_DATA, di, (LPARAM)&pfts);
			}

			// if file size is known and everything is received then disconnect 
			if (di->dwSize && di->dwSize == dwTotal) {
				Netlib_CloseHandle(con);
				con = nullptr;
			}
		}
		// receiving loop broken locally or by remote computer, just some cleaning up left....

		pfts.totalProgress = dwTotal;
		pfts.currentFileProgress = dwTotal;
		ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_DATA, di, (LPARAM)&pfts);
		_close(hFile);
	}
	else {
		ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, di);
		if (con) { // file not possible to open for writing so we ack FAILURE and close the handle
			Netlib_CloseHandle(con);
			con = nullptr;
		}
	}

	delete this; // and finally the object is deleted
}

// this function handles receiving text in dcc chats and then send it in the protochain. very uncomplicated...
// For sending text the SendStuff() function is called (with the help of some function in CIrcProto to find the right
// Dcc object). See CIrcProto for info on how the dcc objects are stored, retrieved and deleted.

void CDccSession::DoChatReceive()
{
	char chBuf[1024 * 4 + 1];
	int cbInBuf = 0;

	// loop to spin around while there is a connection
	while (con) {
		int cbRead;
		int nLinesProcessed = 0;

		cbRead = NLReceive((unsigned char*)chBuf + cbInBuf, sizeof(chBuf)-cbInBuf - 1);
		if (cbRead <= 0)
			break;

		cbInBuf += cbRead;
		chBuf[cbInBuf] = 0;

		char* pStart = chBuf;
		while (*pStart) {
			char* pEnd;

			// seek end-of-line
			for (pEnd = pStart; *pEnd && *pEnd != '\r' && *pEnd != '\n'; ++pEnd)
				;
			if (*pEnd == 0)
				break; // uncomplete message. stop parsing.

			++nLinesProcessed;

			// replace end-of-line with NULLs and skip
			while (*pEnd == '\r' || *pEnd == '\n')
				*pEnd++ = 0;

			if (*pStart) {
				// send it off to some messaging module
				PROTORECVEVENT pre = { 0 };
				pre.timestamp = (uint32_t)time(0);
				pre.szMessage = pStart;
				ProtoChainRecvMsg(di->hContact, &pre);
			}

			cbInBuf -= pEnd - pStart;
			pStart = pEnd;
		}

		// discard processed messages
		if (nLinesProcessed != 0)
			memmove(chBuf, pStart, cbInBuf + 1);
	}

	delete this; // delete the object when the connection is dropped
}

// disconnect the stuff
int CDccSession::Disconnect()
{
	if (hBindPort) {
		Netlib_CloseHandle(hBindPort);
		hBindPort = nullptr;
	}

	// if 'con' exists it is cuz a connection exists. 
	// Terminating 'con' will cause any spawned threads to die and then the object will destroy itself.
	if (con) {
		Netlib_CloseHandle(con);
		con = nullptr;
	}
	else delete this; // if 'con' do not exist (no connection made so far from the object) the object is destroyed

	return TRUE;
}

////////////////////////////////////////////////////////////////////
// check if the dcc chats should disconnect ( default 5 minute timeout )

VOID CALLBACK DCCTimerProc(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	CIrcProto *ppro = GetTimerOwner(idEvent);
	if (ppro)
		ppro->CheckDCCTimeout();
}

// helper function for incoming dcc connections.
void DoIncomingDcc(HNETLIBCONN hConnection, uint32_t dwRemoteIP, void * p1)
{
	CDccSession *dcc = (CDccSession*)p1;
	dcc->IncomingConnection(hConnection, dwRemoteIP);
}

// ident server

void DoIdent(HNETLIBCONN hConnection, uint32_t, void* extra)
{
	CIrcProto *ppro = (CIrcProto*)extra;

	char szBuf[1024];
	int cbTotal = 0;

	while (true) {
		int cbRead = Netlib_Recv(hConnection, szBuf + cbTotal, sizeof(szBuf)-1 - cbTotal, 0);
		if (cbRead == SOCKET_ERROR || cbRead == 0)
			break;

		cbTotal += cbRead;
		szBuf[cbTotal] = 0;

LBL_Parse:
		char* EOLPos = strstr(szBuf, "\r\n");
		if (EOLPos == nullptr)
			continue;

		EOLPos[0] = EOLPos[1] = 0;
		rtrim(szBuf);
		ppro->debugLogA("Got Ident request: %s", szBuf);

		unsigned int PeerPortNrRcvd = 0, LocalPortNrRcvd = 0;
		int iParamCnt = sscanf(szBuf, "%d , %d", &LocalPortNrRcvd, &PeerPortNrRcvd);

		int cbLen = 0;
		char buf[1024 * 4];

		if (iParamCnt != 2)
			cbLen = mir_snprintf(buf, "%s : ERROR : UNKNOWN-ERROR\r\n", szBuf);
		else {
			for (auto &it : CMPlugin::g_arInstances)
				if (PeerPortNrRcvd == it->m_info.iPort && LocalPortNrRcvd == it->m_myLocalPort) {
					cbLen = mir_snprintf(buf, "%s : USERID : %S : %S\r\n",
						szBuf, it->m_info.sIdentServerType.c_str(), it->m_info.sUserID.c_str());
					break;
				}

			if (cbLen == 0)
				cbLen = mir_snprintf(buf, "%s : ERROR : INVALID-PORT\r\n", szBuf);
		}

		if (Netlib_Send(hConnection, (const char*)buf, cbLen, 0) > 0)
			ppro->debugLogA("Sent Ident answer: %s", buf);
		else
			ppro->debugLogA("Sending Ident answer failed.");

		if (ppro->m_identTimer)
			break;

		cbTotal -= EOLPos + 2 - szBuf;
		strdel(szBuf, int(EOLPos + 2 - szBuf));
		goto LBL_Parse;
	}
	Netlib_CloseHandle(hConnection);
}
