//////////////////////////////////////////////////////////////////////////
// Miranda lan functions

#include "stdafx.h"

#define MCODE_SND_STATUS 1
#define MCODE_SND_NAME 2
#define MCODE_REQ_STATUS 3
#define MCODE_SND_MESSAGE 4
#define MCODE_ACK_MESSAGE 5
#define MCODE_SND_VERSION 6
#define MCODE_REQ_AWAYMSG 7
#define MCODE_SND_AWAYMSG 8

#define FCODE_SND_ACCEPT 1
#define FCODE_SND_FILEREQ 2
#define FCODE_SND_FILESKIP 3
#define FCODE_SND_NEXTFILE 4
#define FCODE_SND_FILEDATA 5

enum enuLEXT
{
	LEXT_SENDMESSAGE,
	LEXT_SEARCH,
	LEXT_GETAWAYMSG,
	LEXT_SENDURL,
};

CMLan::CMLan() :
	m_timer(Miranda_GetSystemWindow(), (UINT_PTR)this)
{
	m_timer.OnEvent = Callback(this, &CMLan::OnTimer);

	LoadSettings();
	SetAllOffline();
}

CMLan::~CMLan()
{
	m_mirStatus = ID_STATUS_OFFLINE;
	StopChecking();
	DeleteCache();
	StopListen();
	Shutdown();

	delete[] m_amesAway;
	delete[] m_amesNa;
	delete[] m_amesOccupied;
	delete[] m_amesDnd;
	delete[] m_amesFfc;
}

void CMLan::DeleteCache()
{
	TContact *pCont = m_pRootContact;
	m_pRootContact = nullptr;
	while (pCont) {
		delete[] pCont->m_nick;
		TContact *pPrev = pCont->m_prev;
		delete pCont;
		pCont = pPrev;
	}
}

int CMLan::GetMirandaStatus()
{
	if (GetMode() != LM_LISTEN)
		return ID_STATUS_OFFLINE;
	return m_mirStatus;
}

void CMLan::SetMirandaStatus(u_int status)
{
	if (status == ID_STATUS_INVISIBLE) {
		ProtoBroadcastAck(MODULENAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_mirStatus, m_mirStatus);
		return;
	}
	u_int old_status = m_mirStatus;
	m_mirStatus = status;
	if (old_status == ID_STATUS_OFFLINE && m_mirStatus != ID_STATUS_OFFLINE) {
		StartChecking();
	}
	else if (old_status != ID_STATUS_OFFLINE && m_mirStatus == ID_STATUS_OFFLINE) {
		StopChecking();
	}
	else if (m_mirStatus != ID_STATUS_OFFLINE && m_mirStatus != old_status) {
		RequestStatus(false);
	}

	ProtoBroadcastAck(MODULENAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_mirStatus);
}

void CMLan::SetAllOffline()
{
	for (auto &hContact : Contacts(MODULENAME)) {
		g_plugin.setWord(hContact, "Status", ID_STATUS_OFFLINE);
		g_plugin.delSetting(hContact, "IP");
	}
	DeleteCache();
}

void CMLan::StartChecking()
{
	if (m_bChecking)
		return;

	for (TContact *cont = m_pRootContact; cont; cont = cont->m_prev)
		cont->m_time = MLAN_CHECK + MLAN_TIMEOUT;

	if (ResetInterfaces())
		LoadSettings();

	m_bChecking = true;
	m_timer.Start(MLAN_SLEEP);
	
	StartListen();
	RequestStatus(true);
}

void CMLan::StopChecking()
{
	if (m_bChecking) {
		m_bChecking = false;
		m_timer.Stop();
	}

	m_mirStatus = ID_STATUS_OFFLINE;
	RequestStatus(false);
	StopListen();

	for (TFileConnection *fc = m_pFileConnectionList; fc; fc = fc->m_pNext)
		fc->Terminate();

	while (m_pFileConnectionList)
		Sleep(10);

	SetAllOffline();
}

void CMLan::OnTimer(CTimer*)
{
	mir_cslock lck(m_csAccessClass);

	for (TContact *cont = m_pRootContact; cont; cont = cont->m_prev) {
		if (cont->m_status != ID_STATUS_OFFLINE) {
			if (cont->m_time)
				cont->m_time--;
			if (cont->m_time == MLAN_TIMEOUT)
				RequestStatus(true, cont->m_addr.S_un.S_addr);
			if (!cont->m_time) {
				cont->m_status = ID_STATUS_OFFLINE;
				MCONTACT hContact = FindContact(cont->m_addr, cont->m_nick, false, false, false);
				if (hContact)
					g_plugin.setWord(hContact, "Status", ID_STATUS_OFFLINE);
			}
		}
	}
}

void CMLan::RequestStatus(bool answer, u_long addr)
{
	TPacket pak;
	memset(&pak, 0, sizeof(pak));
	pak.flReqStatus = answer;
	pak.strName = mir_u2a(m_name);
	SendPacketExt(pak, addr);
	mir_free(pak.strName);
}

void CMLan::SendPacketExt(TPacket& pak, u_long addr)
{
	int pakLen;
	u_char *buf = CreatePacket(pak, &pakLen);
	in_addr _addr;
	_addr.S_un.S_addr = addr;
	SendPacket(_addr, (u_char*)buf, pakLen);
	delete[] buf;
}

MCONTACT CMLan::FindContact(in_addr addr, const char *nick, bool add_to_list, bool make_permanent, bool make_visible, u_int status)
{
	for (auto &res : Contacts(MODULENAME)) {
		u_long caddr = g_plugin.getDword(res, "ipaddr", -1);
		if (caddr == addr.S_un.S_addr) {
			if (make_permanent)
				Contact::PutOnList(res);
			if (make_visible)
				Contact::Hide(res, false);
			return res;
		}
	}

	if (add_to_list) {
		MCONTACT res = db_add_contact();
		Proto_AddToContact(res, MODULENAME);
		g_plugin.setDword(res, "ipaddr", addr.S_un.S_addr);
		g_plugin.setString(res, "Nick", nick);

		if (!make_permanent)
			Contact::RemoveFromList(res);
		if (!make_visible)
			Contact::Hide(res);

		g_plugin.setWord(res, "Status", status);
		return res;
	}

	return NULL;
}

void CMLan::OnRecvPacket(u_char *mes, int len, in_addr from)
{
	if (len) {
		TPacket pak;
		ParsePacket(pak, mes, len);

		if (pak.idVersion != 0) {
			TContact* cont = m_pRootContact;
			while (cont) {
				if (cont->m_addr.S_un.S_addr == from.S_un.S_addr)
					break;
				cont = cont->m_prev;
			}
			if (pak.idStatus) {
				mir_cslock lck(m_csAccessClass);
				if (!cont) {
					if (!pak.strName)
						pak.strName = "Unknown";
					cont = new TContact;
					cont->m_addr = from;
					cont->m_prev = m_pRootContact;
					cont->m_status = ID_STATUS_OFFLINE;
					int nlen = (int)mir_strlen(pak.strName);
					cont->m_nick = new char[nlen + 1];
					memcpy(cont->m_nick, pak.strName, nlen + 1);
					m_pRootContact = cont;
				}
				else {
					if (pak.strName && mir_strcmp(pak.strName, cont->m_nick) != 0) {
						delete[] cont->m_nick;
						int nlen = (int)mir_strlen(pak.strName);
						cont->m_nick = new char[nlen + 1];
						memcpy(cont->m_nick, pak.strName, nlen + 1);
					}
				}
				cont->m_time = MLAN_CHECK + MLAN_TIMEOUT;
				cont->m_ver = pak.idVersion;
				u_int old_status = cont->m_status;
				cont->m_status = pak.idStatus;
				MCONTACT hContact = FindContact(cont->m_addr, cont->m_nick, false, false, false);
				if (hContact) {
					g_plugin.setWord(hContact, "Status", cont->m_status);
					if (g_plugin.getDword(hContact, "RemoteVersion", 0) != cont->m_ver)
						g_plugin.setDword(hContact, "RemoteVersion", cont->m_ver);
					if (old_status == ID_STATUS_OFFLINE) {
						u_int rip = cont->m_addr.S_un.S_addr;
						int tip = (rip << 24) | ((rip & 0xff00) << 8) | ((rip & 0xff0000) >> 8) | (rip >> 24);
						g_plugin.setDword(hContact, "IP", tip);
					}
				}
			}
			if (pak.flReqStatus)
				RequestStatus(false, from.S_un.S_addr);

			if (pak.strMessage) {
				if (!cont)
					RequestStatus(true, from.S_un.S_addr);
				else {
					PROTORECVEVENT pre = { 0 };
					pre.timestamp = get_time();
					pre.szMessage = pak.strMessage;
					ProtoChainRecv(FindContact(cont->m_addr, cont->m_nick, true, false, false, cont->m_status), PSR_MESSAGE, 0, (LPARAM)&pre);

					TPacket npak;
					memset(&npak, 0, sizeof(npak));
					npak.idAckMessage = pak.idMessage;
					SendPacketExt(npak, from.S_un.S_addr);
				}
			}

			if (pak.idAckMessage && cont) {
				MCONTACT hContact = FindContact(cont->m_addr, cont->m_nick, false, false, false);
				if (hContact)
					ProtoBroadcastAck(MODULENAME, hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)pak.idAckMessage, 0);
			}

			if (pak.strAwayMessage && cont) {
				PROTORECVEVENT pre = { 0 };
				pre.timestamp = get_time();
				pre.szMessage = pak.strAwayMessage;
				pre.lParam = pak.idAckAwayMessage;
				ProtoChainRecv(FindContact(cont->m_addr, cont->m_nick, true, false, false, cont->m_status), PSR_AWAYMSG, 0, (LPARAM)&pre);
			}

			if (pak.idReqAwayMessage && cont) {
				mir_cslock lck(m_csAccessAwayMes);

				char* mesAway = nullptr;
				switch (m_mirStatus) {
				case ID_STATUS_AWAY: mesAway = m_amesAway; break;
				case ID_STATUS_NA: mesAway = m_amesNa; break;
				case ID_STATUS_OCCUPIED: mesAway = m_amesOccupied; break;
				case ID_STATUS_DND: mesAway = m_amesDnd; break;
				case ID_STATUS_FREECHAT: mesAway = m_amesFfc; break;
				}

				if (mesAway) {
					TPacket npak;
					memset(&npak, 0, sizeof(npak));
					npak.idAckAwayMessage = pak.idReqAwayMessage;
					npak.strAwayMessage = mesAway;
					SendPacketExt(npak, cont->m_addr.S_un.S_addr);
				}
			}
		}
	}
}

void CMLan::RecvMessageUrl(CCSDATA *ccs)
{
	Contact::Hide(ccs->hContact, false);

	PROTORECVEVENT *pre = (PROTORECVEVENT*)ccs->lParam;
	ptrA szMessage(mir_utf8encode(pre->szMessage));

	DBEVENTINFO dbei = {};
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.szModule = MODULENAME;
	dbei.timestamp = pre->timestamp;
	dbei.flags = DBEF_UTF + ((pre->flags & PREF_CREATEREAD) ? DBEF_READ : 0);
	dbei.cbBlob = (uint32_t)mir_strlen(szMessage) + 1;
	dbei.pBlob = (uint8_t*)szMessage.get();
	db_event_add(ccs->hContact, &dbei);
}

INT_PTR CMLan::AddToContactList(u_int flags, EMPSEARCHRESULT *psr)
{
	if (psr->cbSize != sizeof(EMPSEARCHRESULT))
		return 0;

	in_addr addr;
	addr.S_un.S_addr = psr->ipaddr;

	bool TempAdd = flags & PALF_TEMPORARY;

	MCONTACT contact = FindContact(addr, _T2A(psr->nick.w), true, !TempAdd, !TempAdd, psr->stat);
	if (contact != NULL) {
		g_plugin.setWord(contact, "Status", psr->stat);
		g_plugin.setWord(contact, "RemoteVersion", psr->ver);
	}

	return (INT_PTR)contact;
}

int CMLan::SendMessageUrl(CCSDATA *ccs)
{
	int cid = GetRandomProcId();
	mir_forkthread(LaunchExt, new TDataHolder(ccs, cid, LEXT_SENDMESSAGE, this));
	return cid;
}

int CMLan::Search(const char* name)
{
	int cid = GetRandomProcId();
	mir_forkthread(LaunchExt, new TDataHolder(name, cid, LEXT_SEARCH, this));
	return cid;
}

int CMLan::GetAwayMsg(CCSDATA *ccs)
{
	int cid = GetRandomProcId();
	mir_forkthread(LaunchExt, new TDataHolder(ccs, cid, LEXT_GETAWAYMSG, this));
	return cid;
}

int CMLan::RecvAwayMsg(CCSDATA *ccs)
{
	PROTORECVEVENT *pre = (PROTORECVEVENT*)ccs->lParam;
	ProtoBroadcastAck(MODULENAME, ccs->hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)pre->lParam, (LPARAM)pre->szMessage);
	return 0;
}

void __cdecl CMLan::LaunchExt(void *lpParameter)
{
	TDataHolder *hold = (TDataHolder*)lpParameter;
	switch (hold->op) {
	case LEXT_SENDMESSAGE:
	case LEXT_SENDURL:
		hold->lan->SendMessageExt(hold);
		break;
	case LEXT_SEARCH:
		hold->lan->SearchExt(hold);
		break;
	case LEXT_GETAWAYMSG:
		hold->lan->GetAwayMsgExt(hold);
		break;
	}
}

void CMLan::SearchExt(TDataHolder *hold)
{
	// TODO: Normal search must be added
	Sleep(0);
	EMPSEARCHRESULT psr;
	memset(&psr, 0, sizeof(psr));
	psr.cbSize = sizeof(psr);

	for (TContact *cont = m_pRootContact; cont; cont = cont->m_prev) {
		if (mir_strcmp(hold->msg, cont->m_nick) == 0 || mir_strcmp(hold->msg, "*") == 0) {
			char buf[MAX_HOSTNAME_LEN];
			mir_strcpy(buf, cont->m_nick);
			size_t len = mir_strlen(buf);
			buf[len] = '@';
			mir_strcpy(buf + len + 1, inet_ntoa(cont->m_addr));
			psr.nick.a = cont->m_nick;
			psr.firstName.a = "";
			psr.lastName.a = "";
			psr.email.a = buf;
			psr.ipaddr = cont->m_addr.S_un.S_addr;
			psr.stat = cont->m_status;
			psr.ver = cont->m_ver;
			ProtoBroadcastAck(MODULENAME, NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)hold->id, (LPARAM)&psr);
		}
	}

	// search string might contain some ip address
	ULONG addr = inet_addr(hold->msg);
	if (addr != INADDR_NONE) {
		psr.nick.a = hold->msg;
		psr.firstName.a = "";
		psr.lastName.a = "";
		psr.email.a = hold->msg;
		psr.ipaddr = addr;
		psr.stat = ID_STATUS_OFFLINE;
		psr.ver = 0;
		ProtoBroadcastAck(MODULENAME, NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)hold->id, (LPARAM)&psr);
	}

	ProtoBroadcastAck(MODULENAME, NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)hold->id, 0);
	delete hold;
}

void CMLan::SendMessageExt(TDataHolder *hold)
{
	Sleep(0);
	if (g_plugin.getWord(hold->hContact, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE) {
		Sleep(20);
		ProtoBroadcastAck(MODULENAME, hold->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)hold->id);
	}
	else {
		TPacket pak;
		memset(&pak, 0, sizeof(pak));
		u_long addr = g_plugin.getDword(hold->hContact, "ipaddr");
		pak.strMessage = mir_utf8decode(hold->msg, 0);
		pak.idMessage = hold->id;
		SendPacketExt(pak, addr);
	}
	delete hold;
}

void CMLan::GetAwayMsgExt(TDataHolder *hold)
{
	// TODO: check all other params (offline user, offline protocol)
	Sleep(0);
	TPacket pak;
	memset(&pak, 0, sizeof(pak));
	pak.idReqAwayMessage = hold->id;
	u_long addr = g_plugin.getDword(hold->hContact, "ipaddr");
	SendPacketExt(pak, addr);

	ProtoBroadcastAck(MODULENAME, hold->hContact, ACKTYPE_AWAYMSG, ACKRESULT_SENTREQUEST, (HANDLE)hold->id, 0);

	delete hold;
}

int CMLan::SetAwayMsg(u_int status, char* msg)
{
	char **ppMsg;
	switch (status) {
	case ID_STATUS_AWAY:
		ppMsg = &m_amesAway;
		break;
	case ID_STATUS_NA:
		ppMsg = &m_amesNa;
		break;
	case ID_STATUS_OCCUPIED:
		ppMsg = &m_amesOccupied;
		break;
	case ID_STATUS_DND:
		ppMsg = &m_amesDnd;
		break;
	case ID_STATUS_FREECHAT:
		ppMsg = &m_amesFfc;
		break;
	default:
		return 1;
	}
	mir_cslock lck(m_csAccessAwayMes);
	delete[] *ppMsg;
	if (msg)
		*ppMsg = _strdup(msg);
	else
		*ppMsg = nullptr;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// Packets

u_char* CMLan::CreatePacket(TPacket &pak, int *pBufLen)
{
	size_t len = 1;

	if (pak.idVersion != -1)
		pak.idVersion = __FILEVERSION_DWORD;
	else
		pak.idVersion = 0;
	if (pak.idStatus != -1)
		pak.idStatus = m_mirStatus;
	else
		pak.idStatus = -1;

	// Searching for packet len
	if (pak.idVersion)
		len += 1 + 1 + 4;

	if (pak.idStatus)
		len += 1 + 1 + 2;

	size_t nameLen = 0;
	if (pak.strName) {
		nameLen = mir_strlen(pak.strName);
		len += 1 + 1 + nameLen + 1;
	}

	if (pak.flReqStatus)
		len += 1 + 1;

	size_t mesLen = 0;
	if (pak.strMessage) {
		mesLen = mir_strlen(pak.strMessage);
		len += 3 + 1 + 4 + mesLen + 1;
	}

	if (pak.idAckMessage)
		len += 1 + 1 + 4;

	if (pak.idReqAwayMessage)
		len += 1 + 1 + 4;

	size_t awayLen = 0;
	if (pak.strAwayMessage) {
		awayLen = mir_strlen(pak.strAwayMessage);
		len += 3 + 1 + 4 + awayLen + 1;
	}

	// Creating packet
	u_char *buf = new u_char[len];
	u_char *pb = buf;

	if (pak.idVersion) {
		*pb++ = 1 + 4;
		*pb++ = MCODE_SND_VERSION;
		*((u_int*)pb) = pak.idVersion;
		pb += sizeof(u_int);
	}

	if (pak.idStatus) {
		*pb++ = 3;
		*pb++ = MCODE_SND_STATUS;
		*((u_short*)pb) = pak.idStatus;
		pb += sizeof(u_short);
	}

	if (pak.strName) {
		*pb++ = 1 + (uint8_t)nameLen + 1;
		*pb++ = MCODE_SND_NAME;
		memcpy(pb, pak.strName, nameLen);
		pb += nameLen;
		*pb++ = 0;
	}

	if (pak.flReqStatus) {
		*pb++ = 2;
		*pb++ = MCODE_REQ_STATUS;
	}

	if (pak.strMessage) {
		*pb++ = 255;
		*((u_short*)pb) = 1 + 4 + (uint8_t)mesLen + 1;
		pb += sizeof(u_short);
		*pb++ = MCODE_SND_MESSAGE;
		*((u_int*)pb) = pak.idMessage;
		pb += sizeof(u_int);
		if (mesLen)
			memcpy(pb, pak.strMessage, mesLen);
		pb += mesLen;
		*pb++ = 0;
	}

	if (pak.idAckMessage) {
		*pb++ = 1 + 4;
		*pb++ = MCODE_ACK_MESSAGE;
		*((u_int*)pb) = pak.idAckMessage;
		pb += sizeof(u_int);
	}

	if (pak.idReqAwayMessage) {
		*pb++ = 1 + 4;
		*pb++ = MCODE_REQ_AWAYMSG;
		*((u_int*)pb) = pak.idReqAwayMessage;
		pb += sizeof(u_int);
	}

	if (pak.strAwayMessage) {
		*pb++ = 255;
		*((u_short*)pb) = 1 + 4 + (uint8_t)awayLen + 1;
		pb += sizeof(u_short);
		*pb++ = MCODE_SND_AWAYMSG;
		*((u_int*)pb) = pak.idAckAwayMessage;
		pb += sizeof(u_int);
		if (awayLen)
			memcpy(pb, pak.strAwayMessage, awayLen);
		pb += awayLen;
		*pb++ = 0;
	}

	*pb++ = 0;

	if (pBufLen)
		*pBufLen = (int)len;

	return buf;
}

void CMLan::ParsePacket(TPacket& pak, u_char* buf, int len)
{
	memset(&pak, 0, sizeof(pak));
	u_char *buf_end = buf + len;
	while (*buf && buf < buf_end) {
		int tlen = *buf++;
		if (tlen == 255) {
			tlen = *((u_short*)buf);
			buf += sizeof(u_short);
		}
		u_char *pb = buf;
		int comm = *pb++;
		switch (comm) {
		case MCODE_SND_STATUS:
			pak.idStatus = *((u_short*)pb);
			break;
		case MCODE_SND_NAME:
			pak.strName = (char*)pb;
			break;
		case MCODE_REQ_STATUS:
			pak.flReqStatus = true;
			break;
		case MCODE_SND_MESSAGE:
			pak.idMessage = *((u_int*)pb);
			pb += sizeof(u_int);
			pak.strMessage = (char*)pb;
			break;
		case MCODE_ACK_MESSAGE:
			pak.idAckMessage = *((u_int*)pb);
			//pb += sizeof(u_int);
			break;
		case MCODE_SND_VERSION:
			pak.idVersion = *((u_int*)pb);
			//pb += sizeof(u_int);
			break;
		case MCODE_REQ_AWAYMSG:
			pak.idReqAwayMessage = *((u_int*)pb);
			//pb += sizeof(u_int);
			break;
		case MCODE_SND_AWAYMSG:
			pak.idAckAwayMessage = *((u_int*)pb);
			pb += sizeof(u_int);
			pak.strAwayMessage = (char*)pb;
			break;
		}
		buf += tlen;
	}
}

//////////////////////////////////////////////////////////////////////////
// Settings

void CMLan::LoadSettings()
{
	m_RequiredIp = g_plugin.getDword("ipaddr");
	m_UseHostName = g_plugin.getByte("UseHostName", 1) != 0;
	if (m_UseHostName) {
		m_nameLen = MAX_HOSTNAME_LEN;
		GetComputerName(m_name, &m_nameLen);
		CharLower(m_name);
	}
	else {
		ptrW nick(g_plugin.getWStringA("Nick"));
		if (!nick)
			nick = mir_wstrdup(L"EmLan_User");
		mir_wstrcpy(m_name, nick);
	}
	m_nameLen = (int)mir_wstrlen(m_name);

	if (GetStatus() != LM_LISTEN) {
		int ipcount = GetHostAddrCount();
		for (int i = 0; i < ipcount; i++) {
			in_addr addr = GetHostAddress(i);
			if (addr.S_un.S_addr == m_RequiredIp) {
				SetCurHostAddress(addr);
				break;
			}
		}
	}
}

void CMLan::SaveSettings()
{
	g_plugin.setDword("ipaddr", m_RequiredIp);
	g_plugin.setByte("UseHostName", m_UseHostName);
	g_plugin.setWString("Nick", m_name);
}

//////////////////////////////////////////////////////////////////////////

CMLan::TFileConnection::TFileConnection()
{
	memset(this, 0, sizeof(TFileConnection));
	m_state = FCS_OK;
}

CMLan::TFileConnection::~TFileConnection()
{
	if (m_pLan) {
		m_pLan->FileRemoveFromList(this);
	}
	delete[] m_szDescription;
	if (m_szFiles) {
		wchar_t **cp = m_szFiles;
		while (*cp) {
			delete[] * cp;
			cp++;
		}
		delete[] m_szFiles;
	}

	delete[] m_buf;
	delete[] m_szDir;
	delete[] m_szRenamedFile;
}

int CMLan::TFileConnection::Recv(bool halt)
{
	// It is supposed that we're having not less then 2 bytes buffer size :)
	EMLOG("Checking for data");
	while (true) {
		u_long len;
		if (ioctlsocket(m_socket, FIONREAD, &len) != 0) {
			EMLOGERR();
			return FCS_TERMINATE;
		}
		if (len >= 3)
			break;
		if (!halt) {
			EMLOG("No data - halting Recv (only " << len << " bytes)");
			m_recSize = -1;
			delete[] m_buf;
			m_buf = nullptr;
			return FCS_OK;
		}
		Sleep(10);
		if (m_state == FCS_TERMINATE) {
			EMLOG("Terminate requested, exiting recv");
			return FCS_TERMINATE;
		}
	}

	u_short size;
	int res;
	EMLOG("Receiving packet size");
	res = recv(m_socket, (char*)&size, 3, 0);
	if (res == SOCKET_ERROR) {
		EMLOGERR();
		return FCS_TERMINATE;
	}
	if (size == 0) {
		EMLOG("Connection was gracefully closed - size is 0");
		delete[] m_buf;
		m_buf = nullptr;
		m_recSize = 0;
		return FCS_OK;
	}

	{
		mir_cslock lck(m_csAccess);
		delete[] m_buf;
		m_buf = new u_char[size];
		m_recSize = size;
	}

	EMLOG("Waiting for the whole packet (" << size << " bytes)");
	u_long csize = 0;
	while (csize != size) {
		while (true) {
			u_long len;
			if (ioctlsocket(m_socket, FIONREAD, &len) != 0) {
				EMLOGERR();
				return FCS_TERMINATE;
			}
			if (len >= min(u_long(size), FILE_MIN_BLOCK))
				break;
			Sleep(10);
			if (m_state == FCS_TERMINATE) {
				EMLOG("Terminate requested, exiting recv");
				return FCS_TERMINATE;
			}
		}
		EMLOG("Getting data (approx " << size << " bytes)");
		{
			mir_cslock lck(m_csAccess);
			res = recv(m_socket, (char*)m_buf + csize, size - csize, 0);
		}
		EMLOGERR();
		EMLOGIF("Connection was gracefully closed", res == 0);
		if (res == 0 || res == SOCKET_ERROR)
			return FCS_TERMINATE;
		EMLOG("Received " << res << " bytes");
		csize += res;
	}

	EMLOG("Data recv OK");
	return FCS_OK;
}

int CMLan::TFileConnection::SendRaw(u_char* buf, int size)
{
	while (size > 0) {
		if (m_state == FCS_TERMINATE) {
			EMLOG("Terminate requested, exiting sendraw");
			return FCS_TERMINATE;
		}
		int err = send(m_socket, (char*)buf, size, 0);
		if (err == SOCKET_ERROR) {
			EMLOGERR();
			return FCS_TERMINATE;
		}
		size -= err;
		buf += err;
		EMLOGIF("Send " << err << " bytes", size == 0);
		if (size > 0) {
			EMLOG("Partial send (only " << err << " bytes");
			Sleep(10);
		}
	}
	return FCS_OK;
}

int CMLan::TFileConnection::Send(u_char* buf, int size)
{
	if (m_state == FCS_TERMINATE) {
		EMLOG("Terminate requested, exiting send");
		return FCS_TERMINATE;
	}

	EMLOG("Sending 3 bytes of packet size (" << size << ")");
	if (SendRaw((u_char*)&size, 3) != FCS_OK)
		return FCS_TERMINATE;
	if (SendRaw(buf, size) != FCS_OK)
		return FCS_TERMINATE;

	return FCS_OK;
}

void CMLan::FileAddToList(TFileConnection *conn)
{
	mir_cslock lck(m_csFileConnectionList);
	mir_cslock connLck(conn->m_csAccess);
	conn->m_pNext = m_pFileConnectionList;
	conn->m_pPrev = nullptr;
	if (m_pFileConnectionList)
		m_pFileConnectionList->m_pPrev = conn;
	m_pFileConnectionList = conn;
	conn->m_pLan = this;
}

void CMLan::FileRemoveFromList(TFileConnection *conn)
{
	mir_cslock lck(m_csFileConnectionList);
	mir_cslock connLck(conn->m_csAccess);
	if (conn->m_pPrev)
		conn->m_pPrev->m_pNext = conn->m_pNext;
	else
		m_pFileConnectionList = conn->m_pNext;
	if (conn->m_pNext)
		conn->m_pNext->m_pPrev = conn->m_pPrev;
	conn->m_pLan = nullptr;
	conn->m_pPrev = nullptr;
	conn->m_pNext = nullptr;
}

void CMLan::RecvFile(CCSDATA *ccs)
{
	PROTORECVEVENT *pre = (PROTORECVEVENT *)ccs->lParam;

	Contact::Hide(ccs->hContact, false);

	char *szFile = pre->szMessage + sizeof(uint32_t);
	char *szDesc = szFile + mir_strlen(szFile) + 1;

	DBEVENTINFO dbei = {};
	dbei.szModule = MODULENAME;
	dbei.timestamp = pre->timestamp;
	dbei.flags = pre->flags & (PREF_CREATEREAD ? DBEF_READ : 0);
	dbei.eventType = EVENTTYPE_FILE;
	dbei.cbBlob = uint32_t(sizeof(uint32_t) + mir_strlen(szFile) + mir_strlen(szDesc) + 2);
	dbei.pBlob = (uint8_t*)pre->szMessage;
	db_event_add(ccs->hContact, &dbei);
}

void CMLan::OnInTCPConnection(u_long addr, SOCKET in_sock)
{
	EMLOG("Received IN TCP connection");
	TContact *cont = m_pRootContact;
	while (cont && cont->m_addr.S_un.S_addr != addr)
		cont = cont->m_prev;

	// There is no such user in cached list - can not identify him
	if (cont == nullptr)
		return;
	EMLOG("Passed contact search (cont is not NULL)");

	TFileConnection *conn = new TFileConnection();
	conn->m_socket = in_sock;
	conn->m_cid = GetRandomProcId();

	if (conn->Recv() || conn->m_recSize == 0 || conn->m_buf[0] != FCODE_SND_FILEREQ) {
		EMLOG("Not passed synchro data");
		EMLOGIF("Rec size is 0", conn->m_recSize == 0);
		EMLOGIF("Wrong data in packet", conn->m_buf[0] != FCODE_SND_FILEREQ);
		delete conn;
		return;
	}

	EMLOG("File added to connectionn list");
	FileAddToList(conn);

	int rcTotalSize = *((int*)(conn->m_buf + 1));
	int rcTotalFiles = *((int*)(conn->m_buf + 1 + 4));

	PROTORECVEVENT pre;
	pre.szMessage = new char[conn->m_recSize + rcTotalFiles];
	*((int*)pre.szMessage) = conn->m_cid;
	char* pf_to = pre.szMessage + 4;
	char* pf_fr = (char*)conn->m_buf + 1 + 4 + 4;

	conn->m_szFiles = new wchar_t*[rcTotalFiles + 1];
	conn->m_szFiles[rcTotalFiles] = nullptr;

	for (int i = 0; i < rcTotalFiles; i++) {
		conn->m_szFiles[i] = mir_a2u(pf_fr);
		if (i)
			*pf_to++ = ' ';
		while (*pf_fr)
			*pf_to++ = *pf_fr++;
		pf_fr++;
		*pf_to++ = ';';
	}
	*pf_to++ = 0;

	while (*pf_fr)
		*pf_to++ = *pf_fr++;
	*pf_to++ = *pf_fr++;

	conn->m_hContact = FindContact(cont->m_addr, cont->m_nick, true, false, false, cont->m_status);
	pre.flags = 0;
	pre.timestamp = get_time();
	pre.lParam = 0;
	ProtoChainRecv(conn->m_hContact, PSR_FILE, 0, (LPARAM)&pre);

	delete[] pre.szMessage;

	while (!conn->m_state)
		Sleep(10);

	if (conn->m_state != TFileConnection::FCS_ALLOW) {
		conn->Send(nullptr, 0);
		delete conn;
		return;
	}

	conn->m_state = TFileConnection::FCS_OK;

	u_char buf = FCODE_SND_ACCEPT;
	if (conn->Send(&buf, 1)) {
		ProtoBroadcastAck(MODULENAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)conn->m_cid, (LPARAM)"Connection aborted");
		delete conn;
		return;
	}

	// Getting current directory
	wchar_t path[MAX_PATH];
	wchar_t *pathpart;
	GetFullPathName(conn->m_szDir, MAX_PATH, path, &pathpart);
	if (!SetCurrentDirectory(path)) {
		if (rcTotalFiles == 1)
			conn->m_szRenamedFile = mir_wstrdup(pathpart);
		*pathpart = 0;
		if (!SetCurrentDirectory(path)) {
			conn->Send(nullptr, 0);
			ProtoBroadcastAck(MODULENAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)conn->m_cid, (LPARAM)"Can't open output directory");
			delete conn;
			return;
		}
	}

	//Starting from next file
	ProtoBroadcastAck(MODULENAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, (HANDLE)conn->m_cid, 0);

	PROTOFILETRANSFERSTATUS fts;
	fts.flags = PFTS_UNICODE;
	fts.hContact = conn->m_hContact;
	fts.totalBytes = rcTotalSize;
	fts.totalFiles = rcTotalFiles;
	fts.totalProgress = 0;
	fts.szWorkingDir.w = conn->m_szDir;
	fts.pszFiles.w = conn->m_szFiles;

	bool err = false;

	for (int fileNo = 0; fileNo < rcTotalFiles; fileNo++) {
		EMLOG("Waiting for 'next file'");
		if (conn->Recv() || conn->m_recSize == 0 || conn->m_buf[0] != FCODE_SND_NEXTFILE) {
			err = true;
			break;
		}
		EMLOG("Ok");

		fts.szCurrentFile.a = fts.pszFiles.a[fileNo];
		fts.currentFileNumber = fileNo;
		fts.currentFileProgress = 0;
		fts.currentFileSize = *((int*)(conn->m_buf + 1));
		fts.currentFileTime = get_time();

		EMLOG("Waiting for ACCEPT");
		if (!ProtoBroadcastAck(MODULENAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_FILERESUME, (HANDLE)conn->m_cid, (LPARAM)&fts)) {
			conn->m_state = TFileConnection::FCS_OVERWRITE;
		}
		else {
			while (!conn->m_state)
				Sleep(10);
		}
		EMLOG("Ok");
		EMLOG("Checking if we're terminated");
		if (conn->m_state == TFileConnection::FCS_TERMINATE) {
			err = true;
			break;
		}
		EMLOG("Still working");

		u_char snd_buf[5];

		EMLOG("Checking if we're skipping file");
		if (conn->m_state == TFileConnection::FCS_SKIP) {
			EMLOG("Skipped");
			conn->m_state = TFileConnection::FCS_OK;
			snd_buf[0] = FCODE_SND_FILESKIP;
			if (conn->Send(snd_buf, 1)) {
				EMLOG("Error during sending 'skip' code'");
				err = true;
				break;
			}
			continue;
		}
		EMLOG("Still processing");

		wchar_t* filename = conn->m_szRenamedFile;
		if (!filename)
			filename = conn->m_szFiles[fileNo];

		int mode_open = CREATE_ALWAYS;
		if (conn->m_state == TFileConnection::FCS_RESUME)
			mode_open = OPEN_ALWAYS;

		conn->m_state = TFileConnection::FCS_OK;

		EMLOG("Creating file");
		HANDLE hFile = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_READ, nullptr, mode_open, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile == INVALID_HANDLE_VALUE) {
			EMLOG("Can't create file");
			conn->Send(nullptr, 0);
			ProtoBroadcastAck(MODULENAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)conn->m_cid, (LPARAM)"Can't create file");
			delete conn;
			return;
		}
		EMLOG("Ok");

		snd_buf[0] = FCODE_SND_ACCEPT;
		int fsize = GetFileSize(hFile, nullptr);
		*((int*)(snd_buf + 1)) = fsize;
		SetFilePointer(hFile, 0, nullptr, FILE_END);

		fts.currentFileProgress = fsize;
		fts.totalProgress += fsize;

		EMLOG("Sending ack");
		if (conn->Send(snd_buf, 5)) {
			EMLOG("Error sending ACK");
			CloseHandle(hFile);
			err = true;
			break;
		}
		EMLOG("Ok");

		EMLOG("Broadcast ack internally");
		ProtoBroadcastAck(MODULENAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)conn->m_cid, (LPARAM)&fts);
		EMLOG("Ok");
		int refr = 0;
		while (fts.currentFileProgress < fts.currentFileSize) {
			EMLOG("Waiting for data");
			BOOL isErr = conn->Recv();
			if (isErr || conn->m_recSize == 0 || conn->m_buf[0] != FCODE_SND_FILEDATA) {
				EMLOGIF("Error conn->Recv()", isErr);
				EMLOGIF("Error conn->m_recSize!=0", conn->m_recSize == 0);
				EMLOGIF("Error conn->m_buf[0]==FCODE_SND_FILEDATA", conn->m_buf[0] != FCODE_SND_FILEDATA);
				EMLOG("Error");
				err = true;
				break;
			}
			EMLOG("Received");
			DWORD written;
			EMLOG("Writing to file");
			WriteFile(hFile, conn->m_buf + 1, conn->m_recSize - 1, &written, nullptr);
			EMLOG("Ok");
			fts.currentFileProgress += conn->m_recSize - 1;
			fts.totalProgress += conn->m_recSize - 1;
			refr += conn->m_recSize - 1;
			if (refr >= FILE_INFO_REFRESH) {
				EMLOG("Refreshing progress bar");
				ProtoBroadcastAck(MODULENAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)conn->m_cid, (LPARAM)&fts);
				refr = 0;
			}
		}
		if (!err)
			ProtoBroadcastAck(MODULENAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)conn->m_cid, (LPARAM)&fts);

		EMLOG("Closing file handle");
		CloseHandle(hFile);

		if (err)
			break;

		delete[] conn->m_szRenamedFile;
		conn->m_szRenamedFile = nullptr;
	}

	if (err)
		ProtoBroadcastAck(MODULENAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)conn->m_cid, (LPARAM)"Connection aborted");
	else
		ProtoBroadcastAck(MODULENAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (HANDLE)conn->m_cid, 0);

	delete conn;
}

void CMLan::OnOutTCPConnection(u_long, SOCKET out_socket, LPVOID lpParameter)
{
	EMLOG("Sending OUT TCP connection");
	TFileConnection *conn = (TFileConnection *)lpParameter;

	if (out_socket == INVALID_SOCKET) {
		EMLOG("Can't create OUT socket");
		ProtoBroadcastAck(MODULENAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)conn->m_cid, (LPARAM)"Can't initiate transfer");
		delete conn;
		return;
	}
	conn->m_socket = out_socket;
	EMLOG("Socket is created");

	ProtoBroadcastAck(MODULENAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, (HANDLE)conn->m_cid, 0);

	EMLOG("Added to list");
	FileAddToList(conn);

	u_char buf[FILE_SEND_BLOCK + 1];
	wchar_t name[MAX_PATH + 8];

	buf[0] = FCODE_SND_FILEREQ;
	int len = 1 + 4 + 4;
	int size = 0;
	int filecount = 0;
	wchar_t **pf = conn->m_szFiles;
	while (*pf) {
		// TODO: FIX IT !
		EMLOG("Opening file");
		HANDLE hFile = CreateFile(*pf, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
		if (hFile == INVALID_HANDLE_VALUE) {
			EMLOG("Can't open file for reading");
			ProtoBroadcastAck(MODULENAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)conn->m_cid, (LPARAM)"Can't open one of the files");
			delete conn;
			return;
		}
		size += GetFileSize(hFile, nullptr);
		filecount++;
		CloseHandle(hFile);

		wchar_t *filepart;
		GetFullPathName(*pf, MAX_PATH, name, &filepart);
		free(*pf);
		*pf = mir_wstrdup(name);
		mir_strcpy((char *)buf + len, _T2A(filepart));
		len += (int)mir_wstrlen(filepart) + 1;

		pf++;
	}
	mir_strcpy((char *)buf + len, _T2A(conn->m_szDescription));
	len += (int)mir_wstrlen(conn->m_szDescription) + 1;

	*((int *)(buf + 1)) = size;
	*((int *)(buf + 1 + 4)) = filecount;

	GetCurrentDirectory(MAX_PATH, name);
	conn->m_szDir = mir_wstrdup(name);

	PROTOFILETRANSFERSTATUS fts;
	fts.flags = PFTS_SENDING | PFTS_UNICODE;
	fts.hContact = conn->m_hContact;
	fts.totalBytes = size;
	fts.totalFiles = filecount;
	fts.totalProgress = 0;
	fts.szWorkingDir.w = conn->m_szDir;
	fts.pszFiles.w = conn->m_szFiles;

	EMLOG("Sending file size");
	if (conn->Send(buf, len)) {
		EMLOG("Failed");
		ProtoBroadcastAck(MODULENAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)conn->m_cid, (LPARAM)"Connection aborted");
		delete conn;
		return;
	}

	EMLOG("Waiting for ACK");
	if (conn->Recv() || conn->m_recSize == 0 || conn->m_buf[0] != FCODE_SND_ACCEPT) {
		EMLOG("Failed");
		ProtoBroadcastAck(MODULENAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_DENIED, (HANDLE)conn->m_cid, 0);
		delete conn;
		return;
	}

	bool err = false;

	for (int fileNo = 0; fileNo < filecount; fileNo++) {
		EMLOG("Opening file for reading (once more)");
		HANDLE hFile = CreateFile(conn->m_szFiles[fileNo], GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile == INVALID_HANDLE_VALUE) {
			EMLOG("Failed");
			conn->Send(nullptr, 0);
			ProtoBroadcastAck(MODULENAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)conn->m_cid, (LPARAM)"Can't open file");
			delete conn;
			return;
		}

		EMLOG("Sending broadcast NEXT FILE");
		ProtoBroadcastAck(MODULENAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, (HANDLE)conn->m_cid, 0);
		EMLOG("Ok");

		u_char snd_buf[5];
		snd_buf[0] = FCODE_SND_NEXTFILE;
		int fsize = GetFileSize(hFile, nullptr);
		*((int *)(snd_buf + 1)) = fsize;
		EMLOG("Sending file size");
		if (conn->Send(snd_buf, 5)) {
			CloseHandle(hFile);
			err = true;
			break;
		}
		EMLOG("Ok");

		EMLOG("Waiting for ACK");
		if (conn->Recv() || conn->m_recSize == 0 || (conn->m_buf[0] != FCODE_SND_ACCEPT && conn->m_buf[0] != FCODE_SND_FILESKIP)) {
			CloseHandle(hFile);
			err = true;
			break;
		}
		EMLOG("Ok");

		if (conn->m_buf[0] != FCODE_SND_FILESKIP) {
			EMLOG("File is not skipped");
			int filepos = *((int*)(conn->m_buf + 1));
			SetFilePointer(hFile, filepos, nullptr, FILE_BEGIN);

			fts.szCurrentFile.a = fts.pszFiles.a[fileNo];
			fts.currentFileTime = get_time();
			fts.currentFileNumber = fileNo;
			fts.currentFileProgress = filepos;
			fts.totalProgress += filepos;
			fts.currentFileSize = fsize;
			EMLOG("Starting data transfer");
			ProtoBroadcastAck(MODULENAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)conn->m_cid, (LPARAM)&fts);
			EMLOG("Ok");
			int refr = 0;

			fsize -= filepos;

			while (fsize > 0) {
				DWORD readbytes;
				int tosend = FILE_SEND_BLOCK;
				if (tosend > fsize)
					tosend = fsize;
				EMLOG("Reading file data");
				ReadFile(hFile, buf + 1, tosend, &readbytes, nullptr);
				EMLOG("Ok");
				buf[0] = FCODE_SND_FILEDATA;

				if ((int)readbytes != tosend) {
					EMLOG("Error during reading file. File was changed");
					CloseHandle(hFile);
					conn->Send(nullptr, 0);
					ProtoBroadcastAck(MODULENAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)conn->m_cid, (LPARAM)"Can't read file");
					delete conn;
					return;
				}
				EMLOG("Sending data buffer");
				if (conn->Send(buf, tosend + 1)) {
					//CloseHandle(hFile);
					err = true;
					break;
				}
				EMLOG("Ok");

				fts.currentFileProgress += tosend;
				fts.totalProgress += tosend;
				fsize -= tosend;
				refr += tosend;
				if (refr >= FILE_INFO_REFRESH || fsize <= 0) {
					EMLOG("Refreshing file info");
					ProtoBroadcastAck(MODULENAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)conn->m_cid, (LPARAM)&fts);
					refr = 0;
					EMLOG("Checking for 'abort'");
					if (conn->Recv(false) || conn->m_recSize != -1) {
						EMLOG("Aborted");
						//CloseHandle(hFile);
						err = true;
						break;
					}
					EMLOG("Ok");
				}

				if (conn->m_state) {
					EMLOG("Interrupted by user");
					conn->Send(nullptr, 0);
					//CloseHandle(hFile);
					err = true;
					break;
				}
			}
		}
		CloseHandle(hFile);
		if (err)
			break;
	}

	if (err) {
		EMLOG("There was error during file transfering");
		conn->Send(nullptr, 0);
		ProtoBroadcastAck(MODULENAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)conn->m_cid, (LPARAM)"Connection aborted");
	}
	else ProtoBroadcastAck(MODULENAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (HANDLE)conn->m_cid, 0);

	delete conn;
}

int CMLan::SendFile(CCSDATA *ccs)
{
	int cid = GetRandomProcId();

	TFileConnection *conn = new TFileConnection();
	conn->m_cid = cid;
	conn->m_hContact = ccs->hContact;

	conn->m_szDescription = mir_wstrdup((wchar_t *)ccs->wParam);
	int files = 0;
	wchar_t **ppszFiles = (wchar_t **)ccs->lParam;
	while (ppszFiles[files])
		files++;
	conn->m_szFiles = new wchar_t *[files + 1];
	for (int i = 0; i < files; i++)
		conn->m_szFiles[i] = mir_wstrdup(ppszFiles[i]);
	conn->m_szFiles[files] = nullptr;

	u_long addr = g_plugin.getDword(ccs->hContact, "ipaddr");
	CreateTCPConnection(addr, (LPVOID)conn);

	return cid;
}

int CMLan::FileAllow(CCSDATA *ccs)
{
	int cid = (int)ccs->wParam;
	TFileConnection *conn = m_pFileConnectionList;
	while (conn) {
		if (conn->m_cid == cid)
			break;
		conn = conn->m_pNext;
	}
	if (!conn)
		return 0;

	mir_cslock connLck(conn->m_csAccess);
	conn->m_state = TFileConnection::FCS_ALLOW;
	conn->m_szDir = mir_wstrdup((wchar_t *)ccs->lParam);
	return cid;
}

int CMLan::FileDeny(CCSDATA *ccs)
{
	int cid = (int)ccs->wParam;
	TFileConnection *conn = m_pFileConnectionList;
	while (conn) {
		if (conn->m_cid == cid)
			break;
		conn = conn->m_pNext;
	}
	if (!conn)
		return 0;

	conn->m_state = TFileConnection::FCS_TERMINATE;
	return 0;
}

int CMLan::FileCancel(CCSDATA *ccs)
{
	int cid = (int)ccs->wParam;
	TFileConnection *conn = m_pFileConnectionList;
	while (conn) {
		if (conn->m_cid == cid)
			break;
		conn = conn->m_pNext;
	}
	if (!conn)
		return 0;

	conn->m_state = TFileConnection::FCS_TERMINATE;
	return 0;
}

int CMLan::FileResume(int cid, PROTOFILERESUME *pfr)
{
	TFileConnection *conn = m_pFileConnectionList;
	while (conn) {
		if (conn->m_cid == cid)
			break;
		conn = conn->m_pNext;
	}
	if (!conn)
		return 0;

	mir_cslock connLck(conn->m_csAccess);
	switch (pfr->action) {
	case FILERESUME_OVERWRITE:
		conn->m_state = TFileConnection::FCS_OVERWRITE;
		break;
	case FILERESUME_RESUME:
		conn->m_state = TFileConnection::FCS_RESUME;
		break;
	case FILERESUME_RENAME:
		conn->m_state = TFileConnection::FCS_RENAME;
		delete[] conn->m_szRenamedFile;
		conn->m_szRenamedFile = mir_wstrdup(pfr->szFilename);
		break;
	case FILERESUME_SKIP:
		conn->m_state = TFileConnection::FCS_SKIP;
		break;
	}

	return 0;
}
