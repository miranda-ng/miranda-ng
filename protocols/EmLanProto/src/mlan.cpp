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
#define MCODE_SND_URL 9
#define MCODE_ACK_URL 10

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

CMLan::CMLan()
{
	m_RequiredIp = 0;
	m_UseHostName = true;

	m_mirStatus = ID_STATUS_OFFLINE;
	m_pRootContact = 0;

	m_pRootContact = NULL;
	m_hCheckThread = NULL;

	m_handleId = 1;

	m_amesAway = NULL;
	m_amesNa = NULL;
	m_amesOccupied = NULL;
	m_amesDnd = NULL;
	m_amesFfc = NULL;

	m_pFileConnectionList = NULL;

	LoadSettings();

	SetAllOffline();

	//m_hookIcqMsgReq = CreateHookableEvent(ME_ICQ_STATUSMSGREQ);
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
	TContact* pCont = m_pRootContact;
	m_pRootContact = NULL;
	while (pCont)
	{
		delete[] pCont->m_nick;
		TContact* pPrev = pCont->m_prev;
		delete pCont;
		pCont = pPrev;
	}
}

int CMLan::GetMirandaStatus()
{
	if (GetMode()!=LM_LISTEN)
		return ID_STATUS_OFFLINE;
	return m_mirStatus;
}

void CMLan::SetMirandaStatus(u_int status)
{
	if (status==ID_STATUS_INVISIBLE) 
	{
		ProtoBroadcastAck(PROTONAME,NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS, (HANDLE)m_mirStatus, m_mirStatus);
		return;
	}
	u_int old_status = m_mirStatus;
	m_mirStatus = status;
	if (old_status==ID_STATUS_OFFLINE && m_mirStatus!=ID_STATUS_OFFLINE)
	{
		StartChecking();
	}
	else if (old_status!=ID_STATUS_OFFLINE && m_mirStatus==ID_STATUS_OFFLINE)
	{
		StopChecking();
	}
	else if (m_mirStatus!=ID_STATUS_OFFLINE && m_mirStatus!=old_status)
	{
		RequestStatus(false);
	}
	
	ProtoBroadcastAck(PROTONAME,NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS,(HANDLE)old_status,m_mirStatus);
}

void CMLan::SetAllOffline()
{	
	for (MCONTACT hContact = db_find_first(PROTONAME); hContact; hContact = db_find_next(hContact, PROTONAME)) {
		db_set_w(hContact, PROTONAME, "Status", ID_STATUS_OFFLINE);
		db_unset(hContact, PROTONAME, "IP");
	}
	DeleteCache();
}

void CMLan::StartChecking()
{
	if (m_hCheckThread)
		return;

	TContact* cont = m_pRootContact;
	while (cont)
	{
		cont->m_time = MLAN_CHECK + MLAN_TIMEOUT;
		cont = cont->m_prev;
	}

	DWORD threadId;
	m_hCheckThread = CreateThread(NULL, 0, CheckProc, (LPVOID)this, 0, &threadId);
	StartListen();
	RequestStatus(true);
}

void CMLan::StopChecking()
{
	mir_cslock lck(m_csAccessClass);
	if (m_hCheckThread)
	{
		TerminateThread(m_hCheckThread, 0);
		m_hCheckThread = NULL;
	}
	mir_cslock lck2(m_csReceiveThreadLock);
	m_mirStatus = ID_STATUS_OFFLINE;
	RequestStatus(false);
	StopListen();

	TFileConnection* fc = m_pFileConnectionList;
	while (fc)
	{
		fc->Terminate();
		fc = fc->m_pNext;
	}
	while (m_pFileConnectionList)
		Sleep(10);

	SetAllOffline();
}

DWORD WINAPI CMLan::CheckProc(LPVOID lpParameter)
{
	CMLan* lan = (CMLan*)lpParameter;
	lan->Check();
	return 0;
}

void CMLan::Check()
{
	while(1)
	{
		Sleep(MLAN_SLEEP);
		mir_cslock lck(m_csAccessClass);
		TContact* cont = m_pRootContact;
		while (cont)
		{
			if (cont->m_status != ID_STATUS_OFFLINE)
			{
				if (cont->m_time)
					cont->m_time--;
				if (cont->m_time==MLAN_TIMEOUT)
					RequestStatus(true, cont->m_addr.S_un.S_addr);
				if (!cont->m_time)
				{
					cont->m_status = ID_STATUS_OFFLINE;
					MCONTACT hContact = FindContact(cont->m_addr, cont->m_nick, false, false, false);
					if (hContact)
					{
						db_set_w(hContact,PROTONAME,"Status",ID_STATUS_OFFLINE);
					}
				}
			}
			cont = cont->m_prev;
		}
	}
}

void CMLan::RequestStatus(bool answer, u_long addr)
{
	TPacket pak;
	memset(&pak, 0, sizeof(pak));
	pak.flReqStatus = answer;
	pak.strName = m_name;
	SendPacketExt(pak, addr);
}

void CMLan::SendPacketExt(TPacket& pak, u_long addr)
{
	int pakLen;
	u_char* buf = CreatePacket(pak, &pakLen);
	in_addr _addr;
	_addr.S_un.S_addr = addr;
	SendPacket(_addr, (u_char*)buf, pakLen);
	delete[] buf;
}

MCONTACT CMLan::FindContact(in_addr addr, const char* nick,  bool add_to_list, bool make_permanent, bool make_visible, u_int status)
{
	for (MCONTACT res = db_find_first(PROTONAME); res; res = db_find_next(res, PROTONAME)) {
		u_long caddr = db_get_dw(res, PROTONAME, "ipaddr", -1);
		if (caddr==addr.S_un.S_addr) {					
			if (make_permanent)
				db_unset(res,"CList","NotOnList");
			if (make_visible)
				db_unset(res,"CList","Hidden");
			return res;
		}			
	}

	if (add_to_list) {
		MCONTACT res=(MCONTACT)CallService(MS_DB_CONTACT_ADD,0,0);
		CallService(MS_PROTO_ADDTOCONTACT,(WPARAM)res,(LPARAM)PROTONAME);
		db_set_dw(res,PROTONAME, "ipaddr", addr.S_un.S_addr);
		db_set_s(res,PROTONAME, "Nick", nick);

		if (!make_permanent)
			db_set_b(res,"CList","NotOnList",1);
		if (!make_visible)
			db_set_b(res,"CList","Hidden",1);

		db_set_w(res,PROTONAME, "Status", status);
		return res;
	}

	return NULL;
}

void CMLan::OnRecvPacket(u_char* mes, int len, in_addr from)
{
	mir_cslock lck(m_csReceiveThreadLock);

	if (len)
	{
		TPacket pak;
		ParsePacket(pak, mes, len);

		if (pak.idVersion!=0)
		{
			TContact* cont = m_pRootContact;
			while (cont)
			{
				if (cont->m_addr.S_un.S_addr == from.S_un.S_addr)
					break;
				cont = cont->m_prev;
			}
			if (pak.idStatus)
			{
				mir_cslock lck(m_csAccessClass);
				if (!cont)
				{
					if (!pak.strName)
						pak.strName = "Unknown";
					cont = new TContact;
					cont->m_addr = from;
					cont->m_prev = m_pRootContact;
					cont->m_status = ID_STATUS_OFFLINE;
					int nlen = (int)strlen(pak.strName);
					cont->m_nick = new char[nlen+1];
					memcpy(cont->m_nick, pak.strName, nlen+1);
					m_pRootContact = cont;
				}
				else
				{
					if (pak.strName && strcmp(pak.strName, cont->m_nick)!=0)
					{
						delete[] cont->m_nick;
						int nlen = (int)strlen(pak.strName);
						cont->m_nick = new char[nlen+1];
						memcpy(cont->m_nick, pak.strName, nlen+1);
					}
				}
				cont->m_time = MLAN_CHECK + MLAN_TIMEOUT;
				cont->m_ver = pak.idVersion;
				u_int old_status = cont->m_status;
				cont->m_status = pak.idStatus;
				MCONTACT hContact = FindContact(cont->m_addr, cont->m_nick, false, false, false);
				if (hContact)
				{
					db_set_w(hContact,PROTONAME, "Status", cont->m_status);
					if (db_get_dw(hContact,PROTONAME, "RemoteVersion", 0)!=cont->m_ver)
								db_set_dw(hContact,PROTONAME, "RemoteVersion", cont->m_ver);
					if (old_status == ID_STATUS_OFFLINE)
					{
						u_int rip = cont->m_addr.S_un.S_addr;
						int tip = (rip<<24)|((rip&0xff00)<<8)|((rip&0xff0000)>>8)|(rip>>24);
						db_set_dw(hContact, PROTONAME, "IP", tip);
//						HOSTENT* host = gethostbyaddr((const char*)&rip, sizeof(rip), AF_INET);
//						if (host)
//							db_set_s(hContact, PROTONAME, "UID", host->h_name);
					}
				}
			}
			if (pak.flReqStatus)
				RequestStatus(false, from.S_un.S_addr);

			if (pak.strMessage)
			{
				if (!cont)
					RequestStatus(true, from.S_un.S_addr);
				else
				{
					PROTORECVEVENT pre = { 0 };
					pre.timestamp = get_time();
					pre.szMessage = pak.strMessage;
					ProtoChainRecv( FindContact(cont->m_addr, cont->m_nick, true, false, false, cont->m_status),
						pak.flIsUrl ? PSR_URL : PSR_MESSAGE, 0, (LPARAM)&pre );

					TPacket npak;
					memset(&npak, 0, sizeof(npak));
					npak.idAckMessage = pak.idMessage;
					npak.flIsUrl = pak.flIsUrl;
					SendPacketExt(npak, from.S_un.S_addr);
				}
			}

			if (pak.idAckMessage && cont)
			{
				MCONTACT hContact = FindContact(cont->m_addr, cont->m_nick, false, false, false);
				if (hContact)
					ProtoBroadcastAck(PROTONAME, hContact, pak.flIsUrl?ACKTYPE_URL:ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)pak.idAckMessage, 0);
			}

			if (pak.strAwayMessage && cont)
			{
				PROTORECVEVENT pre = { 0 };
				pre.timestamp = get_time();
				pre.szMessage = pak.strAwayMessage;
				pre.lParam = pak.idAckAwayMessage;
				ProtoChainRecv( FindContact(cont->m_addr, cont->m_nick, true, false, false, cont->m_status), PSR_AWAYMSG, 0, (LPARAM)&pre);
			}

			if (pak.idReqAwayMessage && cont)
			{
				MCONTACT hContact = FindContact(cont->m_addr, cont->m_nick, true, false, false);
				// Removed - it causes that whoisreadingawaymessage plugin was not working
//				if (hContact)
//				{
//					int IcqStatus = 0;
//					switch (m_mirStatus)
//					{
//					case ID_STATUS_AWAY: IcqStatus = ICQ_MSGTYPE_GETAWAYMSG; break;
//					case ID_STATUS_NA: IcqStatus = ICQ_MSGTYPE_GETNAMSG; break;
//					case ID_STATUS_OCCUPIED: IcqStatus = ICQ_MSGTYPE_GETOCCUMSG; break;
//					case ID_STATUS_DND: IcqStatus = ICQ_MSGTYPE_GETDNDMSG; break;
//					case ID_STATUS_FREECHAT: IcqStatus = ICQ_MSGTYPE_GETFFCMSG; break;
//					}
//					// HACK: this is a real hack
//					db_set_dw(hContact, "ICQ", "UIN", 1/*0xffffffff*/);
//					NotifyEventHooks(m_hookIcqMsgReq, IcqStatus, 1/*0xffffffff*/);
//					db_unset(hContact, "ICQ", "UIN");
//				}

				mir_cslock lck(m_csAccessAwayMes);

				char* mesAway = NULL;
				switch (m_mirStatus)
				{
				case ID_STATUS_AWAY: mesAway = m_amesAway; break;
				case ID_STATUS_NA: mesAway = m_amesNa; break;
				case ID_STATUS_OCCUPIED: mesAway = m_amesOccupied; break;
				case ID_STATUS_DND: mesAway = m_amesDnd; break;
				case ID_STATUS_FREECHAT: mesAway = m_amesFfc; break;
				}

				if (mesAway)
				{
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

void CMLan::RecvMessageUrl(CCSDATA* ccs)
{
	DBEVENTINFO dbei;
	PROTORECVEVENT *pre=(PROTORECVEVENT*)ccs->lParam;

	memset(&dbei, 0, sizeof(dbei));

	if (!mir_strcmp(ccs->szProtoService, PSR_MESSAGE))
		dbei.eventType = EVENTTYPE_MESSAGE;
	else
		dbei.eventType = EVENTTYPE_URL;

	dbei.cbSize = sizeof(dbei);
	dbei.szModule = PROTONAME;
	dbei.timestamp = pre->timestamp;
	dbei.flags = pre->flags&PREF_CREATEREAD?DBEF_READ:0;
	dbei.cbBlob = mir_tstrlen(pre->szMessage)+1;
	if (!mir_strcmp(ccs->szProtoService, PSR_URL))
	{
		dbei.cbBlob += 2+mir_tstrlen(pre->szMessage+dbei.cbBlob+1);
	}
	dbei.pBlob = (PBYTE)pre->szMessage;

	db_unset(ccs->hContact,"CList","Hidden");

	db_event_add(ccs->hContact, &dbei);
}

INT_PTR CMLan::AddToContactList(u_int flags, EMPSEARCHRESULT* psr)
{
	if (psr->hdr.cbSize!=sizeof(EMPSEARCHRESULT))
		return 0;

	in_addr addr;
	addr.S_un.S_addr = psr->ipaddr;

	bool TempAdd = flags&PALF_TEMPORARY;

	MCONTACT contact = FindContact(addr, psr->hdr.nick, true, !TempAdd, !TempAdd, psr->stat);
	if (contact != NULL) {
		db_set_w(contact,PROTONAME,"Status", psr->stat );
		db_set_w(contact,PROTONAME,"RemoteVersion", psr->ver );
	}

	return (INT_PTR)contact;
}

int CMLan::SendMessageUrl(CCSDATA* ccs, bool isUrl)
{
	DWORD th_id;
	int cid = GetRandomProcId();
	int len;
	if (isUrl)
	{
		len = mir_tstrlen((char*)ccs->lParam);
		((char*)ccs->lParam)[len] = 1;
	}
	TDataHolder* hold = new TDataHolder(ccs, cid, isUrl?LEXT_SENDURL:LEXT_SENDMESSAGE, this);
	if (isUrl)
	{
		((char*)ccs->lParam)[len] = 0;
		hold->msg[len] = 0;
	}
	CloseHandle(CreateThread(NULL,0,LaunchExt,(LPVOID)hold ,0,&th_id));
	return cid;
}

int CMLan::Search(const char* name)
{
	DWORD th_id;
	int cid = GetRandomProcId();
	CloseHandle(CreateThread(NULL,0,LaunchExt,(LPVOID)new TDataHolder(name, cid, LEXT_SEARCH, this),0,&th_id));
	return cid;
}

int CMLan::GetAwayMsg(CCSDATA* ccs)
{
	DWORD th_id;
	int cid = GetRandomProcId();
	CloseHandle(CreateThread(NULL,0,LaunchExt,(LPVOID)new TDataHolder(ccs, cid, LEXT_GETAWAYMSG, this),0,&th_id));
	return cid;
}

int CMLan::RecvAwayMsg(CCSDATA* ccs)
{
	PROTORECVEVENT *pre=(PROTORECVEVENT*)ccs->lParam;
	ProtoBroadcastAck(PROTONAME, ccs->hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)pre->lParam,(LPARAM)pre->szMessage);
	return 0;
}

DWORD WINAPI CMLan::LaunchExt(LPVOID lpParameter)
{
	TDataHolder* hold = (TDataHolder*)lpParameter;
	switch (hold->op)
	{
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
	return 0;
}

void CMLan::SearchExt(TDataHolder* hold)
{
	// TODO: Normal search must be added

	Sleep(0);
	EMPSEARCHRESULT psr;
	memset(&psr,0,sizeof(psr));
	psr.hdr.cbSize=sizeof(psr);

	TContact* cont = m_pRootContact;
	while (cont)
	{
		if (strcmp(hold->msg, cont->m_nick)==0 || strcmp(hold->msg, "*")==0)
		{
			char buf[MAX_HOSTNAME_LEN];
			mir_tstrcpy(buf, cont->m_nick);
			int len = mir_tstrlen(buf);
			buf[len] = '@';
			mir_tstrcpy(buf+len+1, inet_ntoa(cont->m_addr));
			psr.hdr.nick = cont->m_nick;
			psr.hdr.firstName="";
			psr.hdr.lastName="";
			psr.hdr.email=buf;
			psr.ipaddr = cont->m_addr.S_un.S_addr;
			psr.stat = cont->m_status;
			psr.ver = cont->m_ver;

			ProtoBroadcastAck(PROTONAME, NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)hold->id, (LPARAM)&psr);
		}
		cont = cont->m_prev;
	}
	ProtoBroadcastAck(PROTONAME, NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)hold->id, 0);
	delete hold;
}

void CMLan::SendMessageExt(TDataHolder* hold)
{
	Sleep(0);
	if (db_get_w((MCONTACT)hold->hContact, PROTONAME, "Status", ID_STATUS_OFFLINE)==ID_STATUS_OFFLINE)
	{
		Sleep(20);
		ProtoBroadcastAck(PROTONAME, hold->hContact, (hold->op==LEXT_SENDURL)?ACKTYPE_URL:ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)hold->id, 0);
	}
	else
	{
		TPacket pak;
		memset(&pak, 0, sizeof(pak));
		u_long addr = db_get_dw((MCONTACT)hold->hContact, PROTONAME, "ipaddr", 0);
		pak.strMessage = hold->msg;
		pak.idMessage = hold->id;
		if (hold->op==LEXT_SENDURL)
			pak.flIsUrl = true;
		SendPacketExt(pak, addr);
	}
	delete hold;
}

void CMLan::GetAwayMsgExt(TDataHolder* hold)
{
	// TODO: check all other params (offline user, offline protocol)
	Sleep(0);
	TPacket pak;
	memset(&pak, 0, sizeof(pak));
	pak.idReqAwayMessage = hold->id;
	u_long addr = db_get_dw((MCONTACT)hold->hContact, PROTONAME, "ipaddr", 0);
	SendPacketExt(pak, addr);

	ProtoBroadcastAck(PROTONAME, hold->hContact, ACKTYPE_AWAYMSG, ACKRESULT_SENTREQUEST, (HANDLE)hold->id, 0);

	delete hold;
}

int CMLan::SetAwayMsg(u_int status, char* msg)
{
	char** ppMsg;
	switch (status)
	{
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
		*ppMsg = NULL;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// Packets

u_char* CMLan::CreatePacket(TPacket& pak, int* pBufLen)
{
	int len = 1;

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
		len += 1+1+4;

	if (pak.idStatus)
		len += 1+1+2;

	int nameLen;
	if (pak.strName)
	{
		nameLen = mir_tstrlen(pak.strName);
		len += 1+1+nameLen+1;
	}

	if (pak.flReqStatus)
		len += 1+1;

	int mesLen = 0;
	if (pak.strMessage)
	{
		mesLen = mir_tstrlen(pak.strMessage);
		if (pak.flIsUrl)
			mesLen += 1+mir_tstrlen(pak.strMessage+mesLen+1);
		len += 3+1+4+mesLen+1;
	}

	if (pak.idAckMessage)
		len += 1+1+4;

	if (pak.idReqAwayMessage)
		len += 1+1+4;

	int awayLen = 0;
	if (pak.strAwayMessage)
	{
		awayLen = mir_tstrlen(pak.strAwayMessage);
		len += 3+1+4+awayLen+1;
	}

	// Creating packet

	u_char* buf = new u_char[len];
	u_char* pb = buf;

	if (pak.idVersion)
	{
		*pb++ = 1+4;
		*pb++ = MCODE_SND_VERSION;
		*((u_int*)pb) = pak.idVersion;
		pb += sizeof(u_int);
	}

	if (pak.idStatus)
	{
		*pb++ = 3;
		*pb++ = MCODE_SND_STATUS;
		*((u_short*)pb) = pak.idStatus;
		pb += sizeof(u_short);
	}

	if (pak.strName)
	{
		*pb++ = 1+nameLen+1;
		*pb++ = MCODE_SND_NAME;
		memcpy(pb, pak.strName, nameLen);
		pb += nameLen;
		*pb++ = 0;
	}

	if (pak.flReqStatus)
	{
		*pb++ = 2;
		*pb++ = MCODE_REQ_STATUS;
	}

	if (pak.strMessage)
	{
		*pb++ = 255;
		*((u_short*)pb) = 1+4+mesLen+1;
		pb += sizeof(u_short);
		if (pak.flIsUrl)
			*pb++ = MCODE_SND_URL;
		else
			*pb++ = MCODE_SND_MESSAGE;
		*((u_int*)pb) = pak.idMessage;
		pb += sizeof(u_int);
		if (mesLen)
			memcpy(pb, pak.strMessage, mesLen);
		pb += mesLen;
		*pb++ = 0;
	}

	if (pak.idAckMessage)
	{
		*pb++ = 1+4;
		if (pak.flIsUrl)
			*pb++ = MCODE_ACK_URL;
		else
			*pb++ = MCODE_ACK_MESSAGE;
		*((u_int*)pb) = pak.idAckMessage;
		pb += sizeof(u_int);
	}

	if (pak.idReqAwayMessage)
	{
		*pb++ = 1+4;
		*pb++ = MCODE_REQ_AWAYMSG;
		*((u_int*)pb) = pak.idReqAwayMessage;
		pb += sizeof(u_int);
	}

	if (pak.strAwayMessage)
	{
		*pb++ = 255;
		*((u_short*)pb) = 1+4+awayLen+1;
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
		*pBufLen = len;

	return buf;
}

void CMLan::ParsePacket(TPacket& pak, u_char* buf, int len)
{
	memset(&pak, 0, sizeof(pak));
	u_char* buf_end = buf+len;
	while (*buf && buf<buf_end)
	{
		int tlen = *buf++;
		if (tlen==255)
		{
			tlen = *((u_short*)buf);
			buf += sizeof(u_short);
		}
		u_char* pb = buf;
		int comm = *pb++;
		switch (comm)
		{
		case MCODE_SND_STATUS:
			pak.idStatus = *((u_short*)pb);
			break;
		case MCODE_SND_NAME:
			pak.strName = (char*)pb;
			break;
		case MCODE_REQ_STATUS:
			pak.flReqStatus = true;
			break;
		case MCODE_SND_URL:
			pak.flIsUrl = true;
		case MCODE_SND_MESSAGE:
			pak.idMessage = *((u_int*)pb);
			pb += sizeof(u_int);
			pak.strMessage = (char*)pb;
			break;
		case MCODE_ACK_URL:
			pak.flIsUrl = true;
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
	m_RequiredIp = db_get_dw(NULL, PROTONAME, "ipaddr", 0);
	m_UseHostName = db_get_b(NULL, PROTONAME, "UseHostName", 1) != 0;
	if (m_UseHostName) {
		gethostname(m_name, MAX_HOSTNAME_LEN);
		CharLower(m_name);
	}
	else {
		DBVARIANT dbv;
		// Deleting old 'Name' value - using 'Nick' instead of it now
		if ( db_get_s(NULL, PROTONAME, "Nick", &dbv)) {
			if (db_get_s(NULL, PROTONAME, "Name", &dbv))
				dbv.pszVal = "EmLan_User";
			else
				db_unset(NULL, PROTONAME, "Name");
		}
		if (!dbv.pszVal[0])
			dbv.pszVal = "EmLan_User";
		mir_tstrcpy(m_name, dbv.pszVal);
	}
	m_nameLen = mir_tstrlen(m_name);

	if (GetStatus()!=LM_LISTEN)
	{
		int ipcount = GetHostAddrCount();
		for (int i=0; i<ipcount; i++)
		{
			in_addr addr = GetHostAddress(i);
			if (addr.S_un.S_addr == m_RequiredIp)
			{
				SetCurHostAddress(addr);
				break;
			}
		}
	}
}

void CMLan::SaveSettings()
{
	db_set_dw(NULL, PROTONAME, "ipaddr", m_RequiredIp);
	db_set_b(NULL, PROTONAME, "UseHostName", m_UseHostName);
	db_set_s(NULL, PROTONAME, "Nick", m_name);
}

//////////////////////////////////////////////////////////////////////////

CMLan::TFileConnection::TFileConnection()
{
	memset(this, 0, sizeof(TFileConnection));
	m_state = FCS_OK;
}

CMLan::TFileConnection::~TFileConnection()
{
	if (m_pLan)
	{
		m_pLan->FileRemoveFromList(this);
	}
	delete[] m_szDescription;
	if (m_szFiles)
	{
		char** cp = m_szFiles;
		while (*cp)
		{
			delete[] *cp;
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
	while (1)
	{
		u_long len;
		if (ioctlsocket(m_socket, FIONREAD, &len)!=0)
		{
			EMLOGERR();
			return FCS_TERMINATE;
		}
		if (len>=3)
			break;
		if (!halt)
		{
			EMLOG("No data - halting Recv (only " << len << " bytes)");
			m_recSize = -1;
			delete[] m_buf;
			m_buf = NULL;
			return FCS_OK;
		}
		Sleep(10);
		if (m_state==FCS_TERMINATE)
		{
			EMLOG("Terminate requested, exiting recv");
			return FCS_TERMINATE;
		}
	}

	u_short size;
	int res;
	EMLOG("Receiving packet size");
	res = recv(m_socket, (char*)&size, 3, 0);
	if (res==SOCKET_ERROR)
	{
		EMLOGERR();
		return FCS_TERMINATE;
	}
	if (size==0)
	{
		EMLOG("Connection was gracefully closed - size is 0");
		delete m_buf;
		m_buf = NULL;
		m_recSize = 0;
		return FCS_OK;
	}

	Lock();
	delete[] m_buf;
	m_buf = new u_char[size];
	m_recSize = size;

	EMLOG("Waiting for the whole packet (" << size << " bytes)");
	int csize = 0;
	while (csize!=size)
	{
		while(1)
		{
			u_long len;
			if (ioctlsocket(m_socket, FIONREAD, &len) != 0) {
				EMLOGERR();
				return FCS_TERMINATE;
			}
			if (len >= min(size,FILE_MIN_BLOCK))
				break;
			Sleep(10);
			if (m_state == FCS_TERMINATE) {
				EMLOG("Terminate requested, exiting recv");
				return FCS_TERMINATE;
			}
		}
		EMLOG("Getting data (approx " << size << " bytes)");
		Lock();
		res = recv(m_socket, (char*)m_buf+csize, size-csize, 0);
		EMLOGERR();
		EMLOGIF("Connection was gracefully closed", res==0);
		if (res==0 || res==SOCKET_ERROR)
			return FCS_TERMINATE;
		EMLOG("Received " << res << " bytes");
		csize += res;
	}

	EMLOG("Data recv OK");
	return FCS_OK;
}

int CMLan::TFileConnection::SendRaw(u_char* buf, int size)
{
	while (size>0)
	{
		if (m_state==FCS_TERMINATE)
		{
			EMLOG("Terminate requested, exiting sendraw");
			return FCS_TERMINATE;
		}
		int err = send(m_socket, (char*)buf, size, 0);
		if (err==SOCKET_ERROR)
		{
			EMLOGERR();
			return FCS_TERMINATE;
		}
		size -= err;
		buf += err;
		EMLOGIF("Send " << err << " bytes", size==0);
		if (size>0)
		{
			EMLOG("Partial send (only " << err << " bytes");
			Sleep(10);
		}
	}
	return FCS_OK;
}

int CMLan::TFileConnection::Send(u_char* buf, int size)
{
	if (m_state==FCS_TERMINATE)
	{
		EMLOG("Terminate requested, exiting send");
		return FCS_TERMINATE;
	}

	EMLOG("Sending 3 bytes of packet size (" << size << ")");
	if ( SendRaw((u_char*)&size, 3) != FCS_OK )
		return FCS_TERMINATE;
	if ( SendRaw(buf, size) != FCS_OK )
		return FCS_TERMINATE;

	return FCS_OK;
}

void CMLan::FileAddToList(TFileConnection* conn)
{
	mir_cslock lck(m_csFileConnectionList);
	conn->Lock();
	conn->m_pNext = m_pFileConnectionList;
	conn->m_pPrev = NULL;
	if (m_pFileConnectionList)
		m_pFileConnectionList->m_pPrev = conn;
	m_pFileConnectionList = conn;
	conn->m_pLan = this;
}

void CMLan::FileRemoveFromList(TFileConnection* conn)
{
	mir_cslock lck(m_csFileConnectionList);
	conn->Lock();
	if (conn->m_pPrev)
		conn->m_pPrev->m_pNext = conn->m_pNext;
	else
		m_pFileConnectionList = conn->m_pNext;
	if (conn->m_pNext)
		conn->m_pNext->m_pPrev = conn->m_pPrev;
	conn->m_pLan = NULL;
	conn->m_pPrev = NULL;
	conn->m_pNext = NULL;
}

void CMLan::RecvFile(CCSDATA* ccs)
{
	PROTORECVEVENT *pre = (PROTORECVEVENT *)ccs->lParam;
	char *szDesc, *szFile;

	db_unset(ccs->hContact, "CList", "Hidden");

	szFile = pre->szMessage + sizeof(DWORD);
	szDesc = szFile + strlen(szFile) + 1;

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = PROTONAME;
	dbei.timestamp = pre->timestamp;
	dbei.flags = pre->flags & (PREF_CREATEREAD ? DBEF_READ : 0);
	dbei.eventType = EVENTTYPE_FILE;
	dbei.cbBlob = DWORD(sizeof(DWORD) + strlen(szFile) + strlen(szDesc) + 2);
	dbei.pBlob = (PBYTE)pre->szMessage;
	db_event_add(ccs->hContact, &dbei);
}

void CMLan::OnInTCPConnection(u_long addr, SOCKET in_sock)
{
	EMLOG("Received IN TCP connection");
	TContact* cont = m_pRootContact;
	while (cont && cont->m_addr.S_un.S_addr!=addr)
		cont = cont->m_prev;

	// There is no such user in cached list - can not identify him
	if (cont==NULL)
		return;
	EMLOG("Passed contact search (cont is not NULL)");

	TFileConnection* conn = new TFileConnection();
	conn->m_socket = in_sock;
	conn->m_cid = GetRandomProcId();

	if (conn->Recv() || conn->m_recSize==0 || conn->m_buf[0] != FCODE_SND_FILEREQ)
	{
		EMLOG("Not passed synchro data");
		EMLOGIF("Rec size is 0", conn->m_recSize==0);
		EMLOGIF("Wrong data in packet", conn->m_buf[0] != FCODE_SND_FILEREQ);
		delete conn;
		return;
	}

	EMLOG("File added to connectionn list");
	FileAddToList(conn);

	PROTORECVEVENT pre;

	int rcTotalSize = *((int*)(conn->m_buf+1));
	int rcTotalFiles = *((int*)(conn->m_buf+1+4));
	pre.szMessage = new char[conn->m_recSize+rcTotalFiles];
	*((int*)pre.szMessage) = conn->m_cid;
	char* pf_to = pre.szMessage+4;
	char* pf_fr = (char*)conn->m_buf+1+4+4;

	conn->m_szFiles = new char* [rcTotalFiles+1];
	conn->m_szFiles[rcTotalFiles] = NULL;

	for (int i=0; i<rcTotalFiles; i++)
	{
		conn->m_szFiles[i] = _strdup(pf_fr);
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

	if (conn->m_state!=TFileConnection::FCS_ALLOW)
	{
		conn->Send(NULL, 0);
		delete conn;
		return;
	}

	conn->Lock();
	conn->m_state = TFileConnection::FCS_OK;

	u_char buf = FCODE_SND_ACCEPT;
	if (conn->Send(&buf, 1))
	{
		ProtoBroadcastAck(PROTONAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)conn->m_cid, (LPARAM)"Connection aborted");
		delete conn;
		return;
	}

	// Getting current directory
	char path[MAX_PATH];
	char* pathpart;
	GetFullPathName(conn->m_szDir, MAX_PATH, path, &pathpart);
	if (!SetCurrentDirectory(path))
	{
		if (rcTotalFiles==1)
			conn->m_szRenamedFile = _strdup(pathpart);
		*pathpart = 0;
		if (!SetCurrentDirectory(path))
		{
			conn->Send(NULL, 0);
			ProtoBroadcastAck(PROTONAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)conn->m_cid, (LPARAM)"Can't open output directory");
			delete conn;
			return;
		}
	}

	//Starting from next file
	ProtoBroadcastAck(PROTONAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, (HANDLE)conn->m_cid, 0);

	PROTOFILETRANSFERSTATUS fts;
	fts.cbSize = sizeof(fts);
	fts.totalBytes = rcTotalSize;
	fts.totalFiles = rcTotalFiles;
	fts.totalProgress = 0;
	fts.szWorkingDir = conn->m_szDir;
	fts.flags = false;
	fts.hContact = conn->m_hContact;
	fts.pszFiles = conn->m_szFiles;

	bool err = false;

	for (int fileNo=0; fileNo<rcTotalFiles; fileNo++)
	{
		EMLOG("Waiting for 'next file'");
		if (conn->Recv() || conn->m_recSize==0 || conn->m_buf[0] != FCODE_SND_NEXTFILE)
		{
			err = true;
			break;
		}
		EMLOG("Ok");

		fts.szCurrentFile = fts.pszFiles[fileNo];
		fts.currentFileNumber = fileNo;
		fts.currentFileProgress = 0;
		fts.currentFileSize = *((int*)(conn->m_buf+1));
		fts.currentFileTime = get_time();

		EMLOG("Waiting for ACCEPT");
		if (!ProtoBroadcastAck(PROTONAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_FILERESUME, (HANDLE)conn->m_cid, (LPARAM)&fts))
		{
			conn->Lock();
			conn->m_state = TFileConnection::FCS_OVERWRITE;
		}
		else
		{
			while(!conn->m_state)
				Sleep(10);
		}
		EMLOG("Ok");
		EMLOG("Checking if we're terminated");
		if (conn->m_state==TFileConnection::FCS_TERMINATE)
		{
			err = true;
			break;
		}
		EMLOG("Still working");

		u_char snd_buf[5];
		
		EMLOG("Checking if we're skipping file");
		if (conn->m_state==TFileConnection::FCS_SKIP)
		{
			EMLOG("Skipped");
			conn->Lock();
			conn->m_state = TFileConnection::FCS_OK;
			snd_buf[0] = FCODE_SND_FILESKIP;
			if (conn->Send(snd_buf, 1))
			{
				EMLOG("Error during sending 'skip' code'");
				err = true;
				break;
			}
			continue;
		}
		EMLOG("Still processing");

		char* filename = conn->m_szRenamedFile;
		if (!filename)
			filename = conn->m_szFiles[fileNo];

		int mode_open = CREATE_ALWAYS;
		if (conn->m_state==TFileConnection::FCS_RESUME)
			mode_open = OPEN_ALWAYS;

		conn->Lock();
		conn->m_state = TFileConnection::FCS_OK;

		EMLOG("Creating file");
		HANDLE hFile = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_READ, NULL, mode_open, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile==INVALID_HANDLE_VALUE)
		{
			EMLOG("Can't create file");
			conn->Send(NULL, 0);
			ProtoBroadcastAck(PROTONAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)conn->m_cid, (LPARAM)"Can't create file");
			delete conn;
			return;
		}
		EMLOG("Ok");

		snd_buf[0] = FCODE_SND_ACCEPT;
		int fsize = GetFileSize(hFile, NULL);
		*((int*)(snd_buf+1)) = fsize;
		SetFilePointer(hFile, 0, NULL, FILE_END);

		fts.currentFileProgress = fsize;
		fts.totalProgress += fsize;

		EMLOG("Sending ack");
		if (conn->Send(snd_buf, 5))
		{
			EMLOG("Error sending ACK");
			CloseHandle(hFile);
			err = true;
			break;
		}
		EMLOG("Ok");

		EMLOG("Broadcast ack internally");
		ProtoBroadcastAck(PROTONAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)conn->m_cid, (LPARAM)&fts);
		EMLOG("Ok");
		int refr = 0;
		while (fts.currentFileProgress<fts.currentFileSize)
		{
			EMLOG("Waiting for data");
			BOOL isErr = conn->Recv();
			if (isErr || conn->m_recSize==0 || conn->m_buf[0]!=FCODE_SND_FILEDATA)
			{
				EMLOGIF("Error conn->Recv()", isErr);
				EMLOGIF("Error conn->m_recSize!=0", conn->m_recSize==0);
				EMLOGIF("Error conn->m_buf[0]==FCODE_SND_FILEDATA", conn->m_buf[0]!=FCODE_SND_FILEDATA);
				EMLOG("Error");
				err = true;
				break;
			}
			EMLOG("Received");
			DWORD written;
			EMLOG("Writing to file");
			WriteFile(hFile, conn->m_buf+1, conn->m_recSize-1, &written, NULL);
			EMLOG("Ok");
			fts.currentFileProgress += conn->m_recSize-1;
			fts.totalProgress += conn->m_recSize-1;
			refr += conn->m_recSize-1;
			if (refr>=FILE_INFO_REFRESH)
			{
				EMLOG("Refreshing progress bar");
				ProtoBroadcastAck(PROTONAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)conn->m_cid, (LPARAM)&fts);
				refr = 0;
			}
		}
		if (!err)
			ProtoBroadcastAck(PROTONAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)conn->m_cid, (LPARAM)&fts);

		EMLOG("Closing file handle");
		CloseHandle(hFile);

		if (err)
			break;

		delete[] conn->m_szRenamedFile;
		conn->m_szRenamedFile = NULL;
	}

	if (err)
	{
		ProtoBroadcastAck(PROTONAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)conn->m_cid, (LPARAM)"Connection aborted");
		delete conn;
		return;
	}

	ProtoBroadcastAck(PROTONAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (HANDLE)conn->m_cid, 0);

	delete conn;
}

void CMLan::OnOutTCPConnection(u_long addr, SOCKET out_socket, LPVOID lpParameter)
{
	EMLOG("Sending OUT TCP connection");
	TFileConnection* conn = (TFileConnection*)lpParameter;

	if (out_socket == INVALID_SOCKET)
	{
		EMLOG("Can't create OUT socket");
		ProtoBroadcastAck(PROTONAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)conn->m_cid, (LPARAM)"Can't initiate transfer");
		delete conn;
		return;
	}
	conn->m_socket = out_socket;
	EMLOG("Socket is created");

	ProtoBroadcastAck(PROTONAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, (HANDLE)conn->m_cid, 0);

	EMLOG("Added to list");
	FileAddToList(conn);

	u_char buf[FILE_SEND_BLOCK+1];
	char name[MAX_PATH+8];

	buf[0] = FCODE_SND_FILEREQ;
	int len = 1+4+4;
	int size = 0;
	int filecount = 0;
	char** pf = conn->m_szFiles;
	while (*pf)
	{
		// TODO: FIX IT !
		EMLOG("Opening file");
		HANDLE hFile = CreateFile(*pf, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile==INVALID_HANDLE_VALUE)
		{
			EMLOG("Can't open file for reading");
			ProtoBroadcastAck(PROTONAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)conn->m_cid, (LPARAM)"Can't open one of the files");
			delete conn;
			return;
		}
		size += GetFileSize(hFile, NULL);
		filecount++;
		CloseHandle(hFile);

		char* filepart;
		GetFullPathName(*pf, MAX_PATH, (char*)name, &filepart);
		delete[] *pf;
		*pf = _strdup(name);
		strcpy((char*)buf+len, filepart);
		len += (int)strlen(filepart)+1;

		pf++;
	}
	strcpy((char*)buf+len, conn->m_szDescription);
	len += (int)strlen(conn->m_szDescription)+1;

	*((int*)(buf+1)) = size;
	*((int*)(buf+1+4)) = filecount;

	GetCurrentDirectory(MAX_PATH, name);
	conn->m_szDir = _strdup(name);

	PROTOFILETRANSFERSTATUS fts;
	fts.cbSize = sizeof(fts);
	fts.totalBytes = size;
	fts.totalFiles = filecount;
	fts.totalProgress = 0;
	fts.szWorkingDir = conn->m_szDir;
	fts.flags = PFTS_SENDING;
	fts.hContact = conn->m_hContact;
	fts.pszFiles = conn->m_szFiles;

	EMLOG("Sending file size");
	if (conn->Send(buf, len))
	{
		EMLOG("Failed");
		ProtoBroadcastAck(PROTONAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)conn->m_cid, (LPARAM)"Connection aborted");
		delete conn;
		return;
	}

	EMLOG("Waiting for ACK");
	if (conn->Recv() || conn->m_recSize==0 || conn->m_buf[0]!=FCODE_SND_ACCEPT)
	{
		EMLOG("Failed");
		ProtoBroadcastAck(PROTONAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_DENIED, (HANDLE)conn->m_cid, 0);
		delete conn;
		return;
	}

	bool err = false;

	for (int fileNo=0; fileNo<filecount; fileNo++)
	{
		EMLOG("Opening file for reading (once more)");
		HANDLE hFile = CreateFile(conn->m_szFiles[fileNo] , GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile==INVALID_HANDLE_VALUE)
		{
			EMLOG("Failed");
			conn->Send(NULL, 0);
			ProtoBroadcastAck(PROTONAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)conn->m_cid, (LPARAM)"Can't open file");
			delete conn;
			return;
		}

		EMLOG("Sending broadcast NEXT FILE");
		ProtoBroadcastAck(PROTONAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, (HANDLE)conn->m_cid, 0);
		EMLOG("Ok");

		u_char snd_buf[5];
		snd_buf[0] = FCODE_SND_NEXTFILE;
		int fsize = GetFileSize(hFile, NULL);
		*((int*)(snd_buf+1)) = fsize;
		EMLOG("Sending file size");
		if (conn->Send(snd_buf, 5))
		{
			CloseHandle(hFile);
			err = true;
			break;
		}
		EMLOG("Ok");

		EMLOG("Waiting for ACK");
		if (conn->Recv() || conn->m_recSize==0 || (conn->m_buf[0]!=FCODE_SND_ACCEPT && conn->m_buf[0]!=FCODE_SND_FILESKIP))
		{
			CloseHandle(hFile);
			err = true;
			break;
		}
		EMLOG("Ok");

		if (conn->m_buf[0]!=FCODE_SND_FILESKIP)
		{
			EMLOG("File is not skipped");
			int filepos = *((int*)(conn->m_buf+1));
			SetFilePointer(hFile, filepos, NULL, FILE_BEGIN);

			fts.szCurrentFile = fts.pszFiles[fileNo];
			fts.currentFileTime = get_time();
			fts.currentFileNumber = fileNo;
			fts.currentFileProgress = filepos;
			fts.totalProgress += filepos;
			fts.currentFileSize = fsize;
			EMLOG("Starting data transfer");
			ProtoBroadcastAck(PROTONAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)conn->m_cid, (LPARAM)&fts);
			EMLOG("Ok");
			int refr = 0;

			fsize -= filepos;

			while (fsize>0)
			{
				DWORD readbytes;
				int tosend = FILE_SEND_BLOCK;
				if (tosend>fsize)
					tosend = fsize;
				EMLOG("Reading file data");
				ReadFile(hFile, buf+1, tosend, &readbytes, NULL);
				EMLOG("Ok");
				buf[0] = FCODE_SND_FILEDATA;

				if (readbytes!=tosend)
				{
					EMLOG("Error during reading file. File was changed");
					CloseHandle(hFile);
					conn->Send(NULL, 0);
					ProtoBroadcastAck(PROTONAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)conn->m_cid, (LPARAM)"Can't read file");
					delete conn;
					return;
				}
				EMLOG("Sending data buffer");
				if (conn->Send(buf, tosend+1))
				{
					//CloseHandle(hFile);
					err = true;
					break;
				}
				EMLOG("Ok");

				fts.currentFileProgress += tosend;
				fts.totalProgress += tosend;
				fsize -= tosend;
				refr += tosend;
				if (refr>=FILE_INFO_REFRESH || fsize<=0)
				{
					EMLOG("Refreshing file info");
					ProtoBroadcastAck(PROTONAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)conn->m_cid, (LPARAM)&fts);
					refr = 0;
					EMLOG("Checking for 'abort'");
					if (conn->Recv(false) || conn->m_recSize!=-1)
					{
						EMLOG("Aborted");
						//CloseHandle(hFile);
						err = true;
						break;
					}
					EMLOG("Ok");
				}

				if (conn->m_state)
				{
					EMLOG("Interrupted by user");
					conn->Send(NULL, 0);
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

	if (err)
	{
		EMLOG("There was error during file transfering");
		conn->Send(NULL, 0);
		ProtoBroadcastAck(PROTONAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)conn->m_cid, (LPARAM)"Connection aborted");
		delete conn;
		return;
	}

	ProtoBroadcastAck(PROTONAME, conn->m_hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (HANDLE)conn->m_cid, 0);

	delete conn;
}

int CMLan::SendFile(CCSDATA* ccs)
{
	int cid = GetRandomProcId();

	TFileConnection* conn = new TFileConnection();
	conn->m_cid = cid;
	conn->m_hContact = ccs->hContact;

	conn->m_szDescription = _strdup((char*)ccs->wParam);
	int files = 0;
	char** ppszFiles = (char**)ccs->lParam;
	while (ppszFiles[files])
		files++;
	conn->m_szFiles = new char* [files+1];
	for (int i=0; i<files; i++)
		conn->m_szFiles[i] = _strdup(ppszFiles[i]);
	conn->m_szFiles[files] = NULL;

	u_long addr = db_get_dw(ccs->hContact, PROTONAME, "ipaddr", 0);
	CreateTCPConnection(addr, (LPVOID)conn);

	return cid;
}

int CMLan::FileAllow(CCSDATA* ccs)
{
	int cid = (int)ccs->wParam;
	TFileConnection* conn = m_pFileConnectionList;
	while (conn)
	{
		if (conn->m_cid == cid)
			break;
		conn = conn->m_pNext;
	}
	if (!conn)
		return 0;

	conn->Lock();
	conn->m_state = TFileConnection::FCS_ALLOW;
	conn->m_szDir = _strdup((char*)ccs->lParam);
	return cid;
}

int CMLan::FileDeny(CCSDATA* ccs)
{
	int cid = (int)ccs->wParam;
	TFileConnection* conn = m_pFileConnectionList;
	while (conn)
	{
		if (conn->m_cid == cid)
			break;
		conn = conn->m_pNext;
	}
	if (!conn)
		return 0;

	conn->Lock();
	conn->m_state = TFileConnection::FCS_TERMINATE;
	return 0;
}

int CMLan::FileCancel(CCSDATA* ccs)
{
	int cid = (int)ccs->wParam;
	TFileConnection* conn = m_pFileConnectionList;
	while (conn)
	{
		if (conn->m_cid == cid)
			break;
		conn = conn->m_pNext;
	}
	if (!conn)
		return 0;

	conn->Lock();
	conn->m_state = TFileConnection::FCS_TERMINATE;
	return 0;
}

int CMLan::FileResume(int cid, PROTOFILERESUME* pfr)
{
	//int cid = (int)ccs->wParam;
	//PROTOFILERESUME* pfr = (PROTOFILERESUME*)ccs->lParam;

	TFileConnection* conn = m_pFileConnectionList;
	while (conn)
	{
		if (conn->m_cid == cid)
			break;
		conn = conn->m_pNext;
	}
	if (!conn)
		return 0;

	conn->Lock();
	switch (pfr->action)
	{
	case FILERESUME_OVERWRITE:
		conn->m_state = TFileConnection::FCS_OVERWRITE;
		break;
	case FILERESUME_RESUME:
		conn->m_state = TFileConnection::FCS_RESUME;
		break;
	case FILERESUME_RENAME:
		conn->m_state = TFileConnection::FCS_RENAME;
		delete[] conn->m_szRenamedFile;
		conn->m_szRenamedFile = _strdup(pfr->szFilename);
		break;
	case FILERESUME_SKIP:
		conn->m_state = TFileConnection::FCS_SKIP;
		break;
	}

	return 0;
}
