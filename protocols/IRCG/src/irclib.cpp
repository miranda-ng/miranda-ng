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

#define DCCCHATTIMEOUT 300
#define DCCSENDTIMEOUT 120

using namespace irc;

int CDccSession::nDcc = 0;

static int CompareHandlers( const CIrcHandler* p1, const CIrcHandler* p2 )
{
	return lstrcmp( p1->m_name, p2->m_name );
}

OBJLIST<CIrcHandler> CIrcProto::m_handlers( 30, CompareHandlers );

////////////////////////////////////////////////////////////////////

CIrcMessage::CIrcMessage( CIrcProto* _pro, const TCHAR* lpszCmdLine, int codepage, bool bIncoming, bool bNotify ) :
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

	for (int i = 0; i < m.parameters.getCount(); i++)
		parameters.insert(new CMString(m.parameters[i]));
}

CIrcMessage::~CIrcMessage()
{
}

void CIrcMessage::Reset()
{
	prefix.sNick = prefix.sUser = prefix.sHost = sCommand = _T("");
	m_bIncoming = false;
	m_bNotify = true;

	parameters.destroy();
}

CIrcMessage& CIrcMessage::operator = (const CIrcMessage& m)
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

CIrcMessage& CIrcMessage::operator = (const TCHAR* lpszCmdLine)
{
	Reset();
	ParseIrcCommand(lpszCmdLine);
	return *this;
}

void CIrcMessage::ParseIrcCommand(const TCHAR* lpszCmdLine)
{
	const TCHAR* p1 = lpszCmdLine;
	const TCHAR* p2 = lpszCmdLine;

	// prefix exists ?
	if (*p1 == ':') {
		// break prefix into its components (nick!user@host)
		p2 = ++p1;
		while (*p2 && !_tcschr(_T(" !"), *p2))
			++p2;
		prefix.sNick.SetString(p1, p2 - p1);
		if (*p2 != '!')
			goto end_of_prefix;
		p1 = ++p2;
		while (*p2 && !_tcschr(_T(" @"), *p2))
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
			parameters.insert(new CMString(p1, p2 - p1));
			break;
		}
		else {
			// seek end of parameter
			while (*p2 && *p2 != ' ')
				++p2;
			parameters.insert(new CMString(p1, p2 - p1));
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
	return (con != NULL) ? codepage : CP_ACP;
}

void CIrcProto::SendIrcMessage(const TCHAR* msg, bool bNotify, int codepage)
{
	if (codepage == -1)
		codepage = getCodepage();

	if (this) {
		char* str = mir_t2a_cp(msg, codepage);
		rtrim(str);
		int cbLen = (int)strlen(str);
		str = (char*)mir_realloc(str, cbLen + 3);
		strcat(str, "\r\n");
		NLSend((const BYTE*)str, cbLen + 2);
		mir_free(str);

		if (bNotify) {
			CIrcMessage ircMsg(this, msg, codepage);
			if (!ircMsg.sCommand.IsEmpty() && ircMsg.sCommand != _T("QUIT"))
				Notify(&ircMsg);
		}
	}
}

bool CIrcProto::Connect(const CIrcSessionInfo& info)
{
	codepage = m_codepage;

	NETLIBOPENCONNECTION ncon = { 0 };
	ncon.cbSize = sizeof(ncon);
	ncon.szHost = info.sServer.c_str();
	ncon.wPort = info.iPort;
	con = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)m_hNetlibUser, (LPARAM)&ncon);
	if (con == NULL) {
		TCHAR szTemp[300];
		mir_sntprintf(szTemp, SIZEOF(szTemp), _T("\0035%s \002%s\002 (%S: %u)."),
			TranslateT("Failed to connect to"), si.sNetwork.c_str(), si.sServer.c_str(), si.iPort);
		DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, NULL, szTemp, NULL, NULL, NULL, true, false);
		return false;
	}

	FindLocalIP(con); // get the local ip used for filetransfers etc

	if (info.m_iSSL > 0) {
		if (!CallService(MS_NETLIB_STARTSSL, (WPARAM)con, 0) && info.m_iSSL == 2) {
			Netlib_CloseHandle(con);
			con = NULL;
			m_info.Reset();
			return false;
		}
	}

	if (Miranda_Terminated()) {
		Disconnect();
		return false;
	}

	m_info = info;

	// start receiving messages from host
	ForkThread(&CIrcProto::ThreadProc, NULL);
	Sleep(100);
	if (info.sPassword.GetLength())
		NLSend("PASS %s\r\n", info.sPassword.c_str());
	NLSend(_T("NICK %s\r\n"), info.sNick.c_str());

	CMString m_userID = GetWord(info.sUserID.c_str(), 0);
	TCHAR szHostName[MAX_PATH];
	DWORD cbHostName = SIZEOF(szHostName);
	GetComputerName(szHostName, &cbHostName);
	CMString HostName = GetWord(szHostName, 0);
	if (m_userID.IsEmpty())
		m_userID = _T("Miranda");
	if (HostName.IsEmpty())
		HostName = _T("host");
	NLSend(_T("USER %s %s %s :%s\r\n"), m_userID.c_str(), HostName.c_str(), _T("server"), info.sFullName.c_str());

	return con != NULL;
}

void CIrcProto::Disconnect(void)
{
	static const DWORD dwServerTimeout = 5 * 1000;

	if (con == NULL)
		return;

	KillIdent();

	if (m_quitMessage && m_quitMessage[0])
		NLSend(_T("QUIT :%s\r\n"), m_quitMessage);
	else
		NLSend("QUIT \r\n");

	Sleep(50);

	if (con)
		Netlib_Shutdown(con);

	m_info.Reset();
	return;
}

void CIrcProto::Notify(const CIrcMessage* pmsg)
{
	OnIrcMessage(pmsg);
}

int CIrcProto::NLSend(const unsigned char* buf, int cbBuf)
{
	if (!con || !buf)
		return 0;
	if (m_scriptingEnabled && cbBuf == 0)
		cbBuf = lstrlenA((const char *)buf);
	return Netlib_Send(con, (const char*)buf, cbBuf, MSG_DUMPASTEXT);
}

int CIrcProto::NLSend(const TCHAR* fmt, ...)
{
	va_list marker;
	va_start(marker, fmt);

	TCHAR szBuf[1024 * 4];
	mir_vsntprintf(szBuf, SIZEOF(szBuf), fmt, marker);
	va_end(marker);

	char* buf = mir_t2a_cp(szBuf, getCodepage());
	int result = NLSend((unsigned char*)buf, (int)strlen(buf));
	mir_free(buf);
	return result;
}

int CIrcProto::NLSend(const char* fmt, ...)
{
	va_list marker;
	va_start(marker, fmt);

	char szBuf[1024 * 4];
	int cbLen = mir_vsnprintf(szBuf, SIZEOF(szBuf), fmt, marker);
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
		hBindPort = NULL;
		Netlib_CloseHandle(hPort);
	}
}

void CIrcProto::InsertIncomingEvent(TCHAR* pszRaw)
{
	CIrcMessage msg(this, pszRaw, true);
	Notify(&msg);
	return;
}

void CIrcProto::createMessageFromPchar(const char* p)
{
	TCHAR* ptszMsg;
	if (codepage != CP_UTF8 && m_utfAutodetect) {
		if (mir_utf8decodecp(NEWSTR_ALLOCA(p), codepage, &ptszMsg) == NULL)
			ptszMsg = mir_a2t_cp(p, codepage);
	}
	else ptszMsg = mir_a2t_cp(p, codepage);
	CIrcMessage msg(this, ptszMsg, codepage, true);
	Notify(&msg);
	mir_free(ptszMsg);
}

void CIrcProto::DoReceive()
{
	char chBuf[1024 * 4 + 1];
	int cbInBuf = 0;

	if (m_info.bIdentServer && m_info.iIdentServerPort != NULL) {
		NETLIBBIND nb = { 0 };
		nb.cbSize = sizeof(NETLIBBIND);
		nb.pfnNewConnectionV2 = DoIdent;
		nb.pExtra = this;
		nb.wPort = m_info.iIdentServerPort;
		hBindPort = (HANDLE)CallService(MS_NETLIB_BINDPORT, (WPARAM)m_hNetlibUser, (LPARAM)&nb);
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
		chBuf[cbInBuf] = '\0';

		char* pStart = chBuf;
		while (*pStart) {
			char* pEnd;

			// seek end-of-line
			for (pEnd = pStart; *pEnd && *pEnd != '\r' && *pEnd != '\n'; ++pEnd)
				;
			if (*pEnd == '\0')
				break; // uncomplete message. stop parsing.

			++nLinesProcessed;

			// replace end-of-line with NULLs and skip
			while (*pEnd == '\r' || *pEnd == '\n')
				*pEnd++ = '\0';

			// process single message by monitor objects
			if (*pStart) {
				if (m_scriptingEnabled) {
					char* pszTemp = mir_strdup(pStart);

					if (pszTemp) {
						char* p1 = pszTemp;
						// replace end-of-line with NULLs
						while (*p1 != '\0') {
							if (*p1 == '\r' || *p1 == '\n')
								*p1 = '\0';
							p1++;
						}

						createMessageFromPchar(pszTemp);
					}

					mir_free(pszTemp);
				}
				else createMessageFromPchar(pStart);
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
		con = NULL;
	}

	// notify monitor objects that the connection has been closed
	Notify(NULL);
}

void __cdecl CIrcProto::ThreadProc(void*)
{
	DoReceive();
	m_info.Reset();
}

void CIrcProto::AddDCCSession(MCONTACT, CDccSession* dcc)
{
	mir_cslock lck(m_dcc);

	CDccSession* p = m_dcc_chats.find(dcc);
	if (p)
		m_dcc_chats.remove(p);

	m_dcc_chats.insert(dcc);
}

void CIrcProto::AddDCCSession(DCCINFO*, CDccSession* dcc)
{
	mir_cslock lck(m_dcc);
	m_dcc_xfers.insert(dcc);
}

void CIrcProto::RemoveDCCSession(MCONTACT hContact)
{
	mir_cslock lck(m_dcc);

	for (int i = 0; i < m_dcc_chats.getCount(); i++)
		if (m_dcc_chats[i]->di->hContact == hContact) {
			m_dcc_chats.remove(i);
			break;
		}
}

void CIrcProto::RemoveDCCSession(DCCINFO* pdci)
{
	mir_cslock lck(m_dcc);

	for (int i = 0; i < m_dcc_xfers.getCount(); i++) {
		if (m_dcc_xfers[i]->di == pdci) {
			m_dcc_xfers.remove(i);
			break;
		}
	}
}

CDccSession* CIrcProto::FindDCCSession(MCONTACT hContact)
{
	mir_cslock lck(m_dcc);

	for (int i = 0; i < m_dcc_chats.getCount(); i++)
		if (m_dcc_chats[i]->di->hContact == hContact)
			return m_dcc_chats[i];

	return 0;
}

CDccSession* CIrcProto::FindDCCSession(DCCINFO* pdci)
{
	mir_cslock lck(m_dcc);

	for (int i = 0; i < m_dcc_xfers.getCount(); i++)
		if (m_dcc_xfers[i]->di == pdci)
			return m_dcc_xfers[i];

	return 0;
}

CDccSession* CIrcProto::FindDCCSendByPort(int iPort)
{
	mir_cslock lck(m_dcc);

	for (int i = 0; i < m_dcc_xfers.getCount(); i++) {
		CDccSession *p = m_dcc_xfers[i];
		if (p->di->iType == DCC_SEND && p->di->bSender && iPort == p->di->iPort)
			return p;
	}

	return 0;
}

CDccSession* CIrcProto::FindDCCRecvByPortAndName(int iPort, const TCHAR* szName)
{
	mir_cslock lck(m_dcc);

	for (int i = 0; i < m_dcc_xfers.getCount(); i++) {
		CDccSession* p = m_dcc_xfers[i];
		DBVARIANT dbv;
		if (!getTString(p->di->hContact, "Nick", &dbv)) {
			if (p->di->iType == DCC_SEND && !p->di->bSender && !lstrcmpi(szName, dbv.ptszVal) && iPort == p->di->iPort) {
				db_free(&dbv);
				return p;
			}
			db_free(&dbv);
		}
	}

	return 0;
}

CDccSession* CIrcProto::FindPassiveDCCSend(int iToken)
{
	mir_cslock lck(m_dcc);

	for (int i = 0; i < m_dcc_xfers.getCount(); i++)
		if (m_dcc_xfers[i]->iToken == iToken)
			return m_dcc_xfers[i];

	return 0;
}

CDccSession* CIrcProto::FindPassiveDCCRecv(CMString sName, CMString sToken)
{
	mir_cslock lck(m_dcc);

	for (int i = 0; i < m_dcc_xfers.getCount(); i++) {
		CDccSession *p = m_dcc_xfers[i];
		if (sToken == p->di->sToken && sName == p->di->sContactName)
			return p;
	}

	return 0;
}

void CIrcProto::DisconnectAllDCCSessions(bool Shutdown)
{
	mir_cslock lck(m_dcc);

	for (int i = 0; i < m_dcc_chats.getCount(); i++)
		if (m_disconnectDCCChats || Shutdown)
			m_dcc_chats[i]->Disconnect();
}

void CIrcProto::CheckDCCTimeout(void)
{
	mir_cslock lck(m_dcc);

	for (int i = 0; i < m_dcc_chats.getCount(); i++) {
		CDccSession* p = m_dcc_chats[i];
		if (time(0) > p->tLastActivity + DCCCHATTIMEOUT)
			p->Disconnect();
	}

	for (int j = 0; j < m_dcc_xfers.getCount(); j++) {
		CDccSession* p = m_dcc_xfers[j];
		if (time(0) > p->tLastActivity + DCCSENDTIMEOUT)
			p->Disconnect();
	}
}

////////////////////////////////////////////////////////////////////

CIrcIgnoreItem::CIrcIgnoreItem(const TCHAR* _mask, const TCHAR* _flags, const TCHAR* _network) :
	mask(_mask),
	flags(_flags),
	network(_network)
{
}

CIrcIgnoreItem::CIrcIgnoreItem(int codepage, const char* _mask, const char* _flags, const char* _network) :
	mask((TCHAR*)_A2T(_mask, codepage)),
	flags((TCHAR*)_A2T(_flags, codepage)),
	network((TCHAR*)_A2T(_network, codepage))
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
	sNetwork(si.sNetwork),
	iIdentServerPort(si.iIdentServerPort)
{
}

void CIrcSessionInfo::Reset()
{
	sServer = "";
	sServerName = _T("");
	iPort = 0;
	sNick = _T("");
	sUserID = _T("");
	sFullName = _T("");
	sPassword = "";
	bIdentServer = false;
	bNickFlag = false;
	m_iSSL = 0;
	sIdentServerType = _T("");
	iIdentServerPort = 0;
	sNetwork = _T("");
}

////////////////////////////////////////////////////////////////////

void CIrcProto::OnIrcMessage(const CIrcMessage* pmsg)
{
	if (pmsg != NULL) {
		PfnIrcMessageHandler pfn = FindMethod(pmsg->sCommand.c_str());
		if (pfn) {
			// call member function. if it returns 'false',
			// call the default handling
			__try {
				if (!(this->*pfn)(pmsg))
					OnIrcDefault(pmsg);
			}
			__except (EXCEPTION_EXECUTE_HANDLER) // dedicated to Sava :)
			{
				debugLogA("IRC handler feels sick: %S", pmsg->sCommand.c_str());
			}
		}
		else // handler not found. call default handler
			OnIrcDefault(pmsg);
	}
	else OnIrcDisconnected();
}

PfnIrcMessageHandler CIrcProto::FindMethod(const TCHAR* lpszName)
{
	CIrcHandler temp(lpszName, NULL);
	CIrcHandler* p = m_handlers.find(&temp);
	return (p == NULL) ? NULL : p->m_handler;
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

	if (IP == 0 || lstrlenA(IP) == 0)
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
CDccSession::CDccSession(CIrcProto* _pro, DCCINFO* pdci) :
	m_proto(_pro),
	NewFileName(0),
	dwWhatNeedsDoing(0),
	tLastPercentageUpdate(0),
	dwTotal(0),
	iGlobalToken(),
	dwResumePos(0),
	hEvent(0),
	con(0),
	hBindPort(0)
{
	tLastActivity = time(0);

	di = pdci; // Setup values passed to the constructor

	ZeroMemory(&pfts, sizeof(PROTOFILETRANSFERSTATUS));
	pfts.cbSize = sizeof(PROTOFILETRANSFERSTATUS);

	if (di->iType == DCC_SEND && di->bSender == false)
		hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

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
			ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (void *)di, 0);
		else
			ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (void *)di, 0);
	}

	if (di->iType == DCC_CHAT) {
		CDccSession* dcc = m_proto->FindDCCSession(di->hContact);
		if (dcc && this == dcc) {
			m_proto->RemoveDCCSession(di->hContact); // objects automatically remove themselves from the list of objects
			m_proto->setWord(di->hContact, "Status", ID_STATUS_OFFLINE);
		}
	}

	if (di->iType == DCC_SEND)
		m_proto->RemoveDCCSession(di);

	if (hEvent != NULL) {
		SetEvent(hEvent);
		CloseHandle(hEvent);
		hEvent = NULL;
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

int CDccSession::SendStuff(const TCHAR* fmt)
{
	String buf = _T2A(fmt, m_proto->getCodepage());
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
	CDccSession* pThis = (CDccSession*)pparam;
	if (!pThis->con)
		pThis->SetupConnection();
}

// small function to setup the address and port of the remote computer fror passive filetransfers
void CDccSession::SetupPassive(DWORD adress, DWORD port)
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
		file[0] = (TCHAR*)di->sFileAndPath.c_str();
		file[1] = 0;

		pfts.tszCurrentFile = (TCHAR*)di->sFileAndPath.c_str();
		pfts.tszWorkingDir = (TCHAR*)di->sPath.c_str();
		pfts.hContact = di->hContact;
		pfts.flags = PFTS_TCHAR + ((di->bSender) ? PFTS_SENDING : PFTS_RECEIVING);
		pfts.totalFiles = 1;
		pfts.currentFileNumber = 0;
		pfts.totalBytes = di->dwSize;
		pfts.currentFileSize = pfts.totalBytes;
		pfts.ptszFiles = file;
		pfts.totalProgress = 0;
		pfts.currentFileProgress = 0;
		pfts.currentFileTime = (unsigned long)time(0);
	}

	// create a listening socket for outgoing chat/send requests. The remote computer connects to this computer. Used for both chat and filetransfer.
	if (di->bSender && !di->bReverse) {
		NETLIBBIND nb = { 0 };
		nb.cbSize = sizeof(NETLIBBIND);
		nb.pfnNewConnectionV2 = DoIncomingDcc; // this is the (helper) function to be called once an incoming connection is made. The 'real' function that is called is IncomingConnection()
		nb.pExtra = (void *)this;
		nb.wPort = 0;
		hBindPort = (HANDLE)CallService(MS_NETLIB_BINDPORT, (WPARAM)m_proto->hNetlibDCC, (LPARAM)&nb);

		if (hBindPort == NULL) {
			delete this; // dcc objects destroy themselves when the connection has been closed or failed for some reasson.
			return 0;
		}

		di->iPort = nb.wPort; // store the port internally so it is possible to search for it (for resuming of filetransfers purposes)
		return nb.wPort; // return the created port so it can be sent to the remote computer in a ctcp/dcc command
	}

	// If a remote computer initiates a chat session this is used to connect to the remote computer (user already accepted at this point). 
	// also used for connecting to a remote computer for remote file transfers
	if (di->iType == DCC_CHAT && !di->bSender || di->iType == DCC_SEND && di->bSender && di->bReverse) {
		NETLIBOPENCONNECTION ncon = { 0 };
		ncon.cbSize = sizeof(ncon);
		ncon.szHost = ConvertIntegerToIP(di->dwAdr);
		ncon.wPort = (WORD)di->iPort;
		con = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)m_proto->hNetlibDCC, (LPARAM)&ncon);
	}

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
						di->sFile = di->sFileAndPath.Mid(i + 1, di->sFileAndPath.GetLength());
					}

					pfts.tszCurrentFile = (TCHAR*)di->sFileAndPath.c_str();
					pfts.tszWorkingDir = (TCHAR*)di->sPath.c_str();
					pfts.totalBytes = di->dwSize;
					pfts.currentFileSize = pfts.totalBytes;

					delete[] NewFileName;
					NewFileName = NULL;
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
			NETLIBBIND nb = { 0 };
			nb.cbSize = sizeof(NETLIBBIND);
			nb.pfnNewConnectionV2 = DoIncomingDcc; // this is the (helper) function to be called once an incoming connection is made. The 'real' function that is called is IncomingConnection()
			nb.pExtra = (void *)this;
			nb.wPort = 0;
			hBindPort = (HANDLE)CallService(MS_NETLIB_BINDPORT, (WPARAM)m_proto->hNetlibDCC, (LPARAM)&nb);

			if (hBindPort == NULL) {
				m_proto->DoEvent(GC_EVENT_INFORMATION, 0, m_proto->m_info.sNick.c_str(), LPGENT("DCC ERROR: Unable to bind local port for passive file transfer"), NULL, NULL, NULL, true, false);
				delete this; // dcc objects destroy themselves when the connection has been closed or failed for some reasson.
				return 0;
			}

			di->iPort = nb.wPort; // store the port internally so it is possible to search for it (for resuming of filetransfers purposes)

			CMString sFileWithQuotes = di->sFile;

			// if spaces in the filename surround with quotes
			if (sFileWithQuotes.Find(' ', 0) != -1) {
				sFileWithQuotes.Insert(0, _T("\""));
				sFileWithQuotes.Insert(sFileWithQuotes.GetLength(), _T("\""));
			}

			// send out DCC RECV command for passive filetransfers
			unsigned long ulAdr = 0;
			if (m_proto->m_manualHost)
				ulAdr = ConvertIPToInteger(m_proto->m_mySpecifiedHostIP);
			else
				ulAdr = m_proto->m_IPFromServer ? ConvertIPToInteger(m_proto->m_myHost) : nb.dwExternalIP;

			if (di->iPort && ulAdr)
				m_proto->PostIrcMessage(_T("/CTCP %s DCC SEND %s %u %u %I64u %s"), di->sContactName.c_str(), sFileWithQuotes.c_str(), ulAdr, di->iPort, di->dwSize, di->sToken.c_str());

			return TRUE;
		}

		// connect to the remote computer from which you are receiving the file (now all actions to take (resume/overwrite etc) have been decided
		NETLIBOPENCONNECTION ncon = { 0 };
		ncon.cbSize = sizeof(ncon);
		ncon.szHost = ConvertIntegerToIP(di->dwAdr);
		ncon.wPort = (WORD)di->iPort;

		con = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)m_proto->hNetlibDCC, (LPARAM)&ncon);
	}

	// if for some reason the plugin has failed to connect to the remote computer the object is destroyed.
	if (con == NULL) {
		delete this;
		return FALSE; // failed to connect
	}

	// if it is a chat connection set the user to online now since we now know there is a connection
	if (di->iType == DCC_CHAT)
		m_proto->setWord(di->hContact, "Status", ID_STATUS_ONLINE);

	// spawn a new thread to handle receiving/sending of data for the new chat/filetransfer connection to the remote computer
	mir_forkthread(ThreadProc, this);

	return con != NULL;
}

// called by netlib for incoming connections on a listening socket (chat/filetransfer)
int CDccSession::IncomingConnection(HANDLE hConnection, DWORD dwIP)
{
	con = hConnection;
	if (con == NULL) {
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
	CDccSession* pThis = (CDccSession*)pparam;

	// if it is an incoming connection on a listening port, then we should close the listenting port so only one can connect (the one you offered
	// the connection to) can connect and not evil IRCopers with haxxored IRCDs
	if (pThis->hBindPort) {
		Netlib_CloseHandle(pThis->hBindPort);
		pThis->hBindPort = NULL;
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
	ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, (void *)di, 0);
	ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, (void *)di, 0);

	WORD wPacketSize = m_proto->getWord("DCCPacketSize", 1024 * 4);

	if (wPacketSize < 256)
		wPacketSize = 256;

	if (wPacketSize > 32 * 1024)
		wPacketSize = 32 * 1024;

	BYTE* chBuf = new BYTE[wPacketSize + 1];

	// is there a connection?
	if (con) {
		// open the file for reading
		int hFile = _topen(di->sFileAndPath.c_str(), _O_RDONLY | _O_BINARY, _S_IREAD);
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
			ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_DATA, (void *)di, (LPARAM)&pfts);

			tLastActivity = time(0);

			// create a packet receiver to handle receiving ack's from the remote computer.
			HANDLE hPackrcver = (HANDLE)CallService(MS_NETLIB_CREATEPACKETRECVER, (WPARAM)con, sizeof(DWORD));
			NETLIBPACKETRECVER npr;
			npr.cbSize = sizeof(NETLIBPACKETRECVER);
			npr.dwTimeout = 60 * 1000;
			npr.bufferSize = sizeof(DWORD);
			npr.bytesUsed = 0;

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
					DWORD dwRead = 0;
					DWORD dwPacket = NULL;

					do {
						dwRead = CallService(MS_NETLIB_GETMOREPACKETS, (WPARAM)hPackrcver, (LPARAM)&npr);
						npr.bytesUsed = sizeof(DWORD);

						if (dwRead <= 0)
							break; // connection closed, or a timeout occurred.

						dwPacket = *(DWORD*)npr.buffer;
						dwLastAck = ntohl(dwPacket);

					}
					while (con && dwTotal != dwLastAck);

					if (!con || dwRead <= 0)
						goto DCC_STOP;
				}

				if (m_proto->m_DCCMode == 1) {
					DWORD dwRead = 0;
					DWORD dwPacket = 0;

					do {
						dwRead = CallService(MS_NETLIB_GETMOREPACKETS, (WPARAM)hPackrcver, (LPARAM)&npr);
						npr.bytesUsed = sizeof(DWORD);
						if (dwRead <= 0)
							break; // connection closed, or a timeout occurred.

						dwPacket = *(DWORD*)npr.buffer;
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
					ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_DATA, (void *)di, (LPARAM)&pfts);
				}

				// close the connection once the whole file has been sent an completely ack'ed
				if (dwLastAck >= di->dwSize) {
					Netlib_CloseHandle(con);
					con = NULL;
				}
			}

		DCC_STOP:
			// need to close the connection if it isn't allready
			if (con) {
				Netlib_CloseHandle(con);
				con = NULL;
			}

			// ack the progress one final time
			tLastActivity = time(0);
			pfts.totalProgress = dwTotal;
			pfts.currentFileProgress = dwTotal;
			ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_DATA, (void *)di, (LPARAM)&pfts);

			_close(hFile);
		}
		else { // file was not possible to open for reading
			ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (void *)di, 0);
			if (con) {
				Netlib_CloseHandle(con);
				con = NULL;
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
	ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, (void *)di, 0);

	BYTE chBuf[1024 * 32 + 1];

	// do some stupid thing so  the filetransfer dialog shows the right thing
	ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, (void *)di, 0);

	// open the file for writing (and reading in case it is a resume)
	int hFile = _topen(di->sFileAndPath.c_str(),
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
		ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_DATA, (void *)di, (LPARAM)&pfts);

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
				DWORD no = dwTotal;
				no = htonl(no);
				NLSend((unsigned char *)&no, sizeof(DWORD));
				dwLastAck = dwTotal;
			}
			else dwLastAck = dwTotal;

			// sets the 'last update time' to check for timed out connections, and also make sure we only
			// ack the 'percentage-ready meter' only once a second to save CPU.
			if (tLastPercentageUpdate < time(0)) {
				tLastPercentageUpdate = time(0);
				pfts.totalProgress = dwTotal;
				pfts.currentFileProgress = dwTotal;
				ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_DATA, (void *)di, (LPARAM)&pfts);
			}

			// if file size is known and everything is received then disconnect 
			if (di->dwSize && di->dwSize == dwTotal) {
				Netlib_CloseHandle(con);
				con = NULL;
			}
		}
		// receiving loop broken locally or by remote computer, just some cleaning up left....

		pfts.totalProgress = dwTotal;
		pfts.currentFileProgress = dwTotal;
		ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_DATA, (void *)di, (LPARAM)&pfts);
		_close(hFile);
	}
	else {
		ProtoBroadcastAck(m_proto->m_szModuleName, di->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (void *)di, 0);
		if (con) { // file not possible to open for writing so we ack FAILURE and close the handle
			Netlib_CloseHandle(con);
			con = NULL;
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
		chBuf[cbInBuf] = '\0';

		char* pStart = chBuf;
		while (*pStart) {
			char* pEnd;

			// seek end-of-line
			for (pEnd = pStart; *pEnd && *pEnd != '\r' && *pEnd != '\n'; ++pEnd)
				;
			if (*pEnd == '\0')
				break; // uncomplete message. stop parsing.

			++nLinesProcessed;

			// replace end-of-line with NULLs and skip
			while (*pEnd == '\r' || *pEnd == '\n')
				*pEnd++ = '\0';

			if (*pStart) {
				// send it off to some messaging module
				PROTORECVEVENT pre = { 0 };
				pre.timestamp = (DWORD)time(NULL);
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
		hBindPort = NULL;
	}

	// if 'con' exists it is cuz a connection exists. 
	// Terminating 'con' will cause any spawned threads to die and then the object will destroy itself.
	if (con) {
		Netlib_CloseHandle(con);
		con = NULL;
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
void DoIncomingDcc(HANDLE hConnection, DWORD dwRemoteIP, void * p1)
{
	CDccSession* dcc = (CDccSession*)p1;
	dcc->IncomingConnection(hConnection, dwRemoteIP);
}

// ident server

void strdel(char* parBuffer, int len)
{
	char *p;
	for (p = parBuffer + len; *p != 0; p++)
		p[-len] = *p;

	p[-len] = '\0';
}

void DoIdent(HANDLE hConnection, DWORD, void* extra)
{
	CIrcProto *ppro = (CIrcProto*)extra;

	char szBuf[1024];
	int cbTotal = 0;

	while (true) {
		int cbRead = Netlib_Recv(hConnection, szBuf + cbTotal, sizeof(szBuf)-1 - cbTotal, 0);
		if (cbRead == SOCKET_ERROR || cbRead == 0)
			break;

		cbTotal += cbRead;
		szBuf[cbTotal] = '\0';

LBL_Parse:
		char* EOLPos = strstr(szBuf, "\r\n");
		if (EOLPos == NULL)
			continue;

		EOLPos[0] = EOLPos[1] = '\0';
		rtrim(szBuf);
		ppro->debugLogA("Got Ident request: %s", szBuf);

		unsigned int PeerPortNrRcvd = 0, LocalPortNrRcvd = 0;
		int iParamCnt = sscanf(szBuf, "%d , %d", &LocalPortNrRcvd, &PeerPortNrRcvd);

		int cbLen = 0;
		char buf[1024 * 4];

		if (iParamCnt != 2)
			cbLen = mir_snprintf(buf, SIZEOF(buf), "%s : ERROR : UNKNOWN-ERROR\r\n", szBuf);
		else {
			for (int i = 0; i < g_Instances.getCount(); i++) {
				if (PeerPortNrRcvd == g_Instances[i]->m_info.iPort && LocalPortNrRcvd == g_Instances[i]->m_myLocalPort) {
					cbLen = mir_snprintf(buf, SIZEOF(buf), "%s : USERID : %S : %S\r\n",
						szBuf, g_Instances[i]->m_info.sIdentServerType.c_str(), g_Instances[i]->m_info.sUserID.c_str());
					break;
				}
			}

			if (cbLen == 0)
				cbLen = mir_snprintf(buf, SIZEOF(buf), "%s : ERROR : INVALID-PORT\r\n", szBuf);
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
