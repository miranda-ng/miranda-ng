#include "skype_proto.h"

bool CSkypeProto::IsMessageInDB(HANDLE hContact, DWORD timestamp, const char* guid, int flag)
{
	bool result = false;

	HANDLE hDbEvent = ::db_event_last(hContact);
	while (hDbEvent) 
	{
		DBEVENTINFO dbei = { sizeof(dbei) };
		dbei.cbBlob = ::db_event_getBlobSize(hDbEvent);
		dbei.pBlob = (PBYTE)::mir_alloc(dbei.cbBlob);
		::db_event_get(hDbEvent, &dbei);

		/*if (dbei.timestamp < timestamp)
		{
			::mir_free(dbei.pBlob);
			break;
		}*/

		int sendFlag = dbei.flags & DBEF_SENT;
		if (dbei.eventType == EVENTTYPE_MESSAGE && sendFlag == flag)
		{
			if (::memcmp(&dbei.pBlob[dbei.cbBlob], guid, 32) == 0)
			{
				::mir_free(dbei.pBlob);
				result = true;
				break;
			}
		}

		::mir_free(dbei.pBlob);
		hDbEvent = ::db_event_prev(hDbEvent);		
	}

	return result;
}

HANDLE CSkypeProto::AddDataBaseEvent(HANDLE hContact, WORD type, DWORD timestamp, DWORD flags, DWORD cbBlob, PBYTE pBlob)
{
	DBEVENTINFO dbei = { 0 };
	dbei.cbSize = sizeof(dbei);
	dbei.szModule = this->m_szModuleName;
	dbei.timestamp = timestamp;
	dbei.eventType = type;
	dbei.cbBlob = cbBlob;
	dbei.pBlob = pBlob;
	dbei.flags = flags;

	return ::db_event_add(hContact, &dbei);
}

void CSkypeProto::RaiseAuthRequestEvent(
	DWORD timestamp,
	CContact::Ref contact)
{	
	char *sid = ::mir_strdup(contact->GetSid());
	char *nick = ::mir_strdup(contact->GetNick());

	SEString data;

	contact->GetPropReceivedAuthrequest(data);
	char* reason = ::mir_strdup(data);

	SEString last;
	contact->GetFullname(data, last);
	char* firstName = ::mir_strdup(data);
	char* lastName = ::mir_strdup(last);

	HANDLE hContact = this->AddContact(contact);

	/*blob is: 0(DWORD), hContact(DWORD), nick(ASCIIZ), firstName(ASCIIZ), lastName(ASCIIZ), sid(ASCIIZ), reason(ASCIIZ)*/
	DWORD cbBlob = (DWORD)
		(sizeof(DWORD) * 2) + 
		::strlen(nick) + 
		::strlen(firstName) + 
		::strlen(lastName) + 
		::strlen(sid) + 
		::strlen(reason) + 		
		5;

	PBYTE pBlob, pCurBlob;	
	pCurBlob = pBlob = (PBYTE)::mir_alloc(cbBlob);
	
	*((PDWORD)pCurBlob) = 0; pCurBlob += sizeof(DWORD);
	*((PDWORD)pCurBlob) = (DWORD)hContact; pCurBlob += sizeof(DWORD);
	::strcpy((char*)pCurBlob, nick); pCurBlob += ::strlen(nick) + 1;
	::strcpy((char*)pCurBlob, firstName); pCurBlob += ::strlen(firstName) + 1;
	::strcpy((char*)pCurBlob, lastName); pCurBlob += ::strlen(lastName) + 1;
	::strcpy((char*)pCurBlob, sid); pCurBlob += ::strlen(sid) + 1;
	::strcpy((char*)pCurBlob, reason);
	
	this->AddDataBaseEvent(hContact, EVENTTYPE_AUTHREQUEST, time(NULL), PREF_UTF, cbBlob, pBlob);
}

void CSkypeProto::RaiseMessageReceivedEvent(
	HANDLE hContact,
	DWORD timestamp,
	const char *guid,
	const wchar_t *message,
	bool isNeedCheck)
{	
	//if (isNeedCheck)
	//	if (this->IsMessageInDB(hContact, timestamp, guid))
	//		return;

	PROTORECVEVENT recv;
	recv.flags = PREF_UTF;
	recv.lParam = (LPARAM)guid;
	recv.timestamp = timestamp;
	recv.szMessage = ::mir_utf8encodeW(message);
	
	::ProtoChainRecvMsg(hContact, &recv);
}

void CSkypeProto::RaiseMessageSendedEvent(
	HANDLE hContact,
	DWORD timestamp,
	const char *guid,
	const wchar_t *message)
{	
	//if (this->IsMessageInDB(hContact, timestamp, guid, DBEF_SENT))
	//	return;

	char *msg = ::mir_utf8encodeW(message);

	DBEVENTINFO dbei = { 0 };
	dbei.cbSize = sizeof(dbei);
	dbei.szModule = this->m_szModuleName;
	dbei.timestamp = timestamp;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = (DWORD)::strlen(msg) + 1;
	dbei.pBlob = (PBYTE)msg;
	dbei.flags = PREF_UTF | DBEF_SENT;

	::db_event_add(hContact, &dbei);
}

//void CSkypeProto::RaiseFileReceivedEvent(
//	DWORD timestamp,
//	const char* sid, 
//	const char* nick, 
//	const char* message)
//{	
//	PROTORECVFILET pre = {0};
//
//	CCSDATA ccs = {0};
//	ccs.szProtoService = PSR_FILE;
//	ccs.hContact = this->AddContactBySid(sid, nick);
//	ccs.wParam = 0;
//	ccs.lParam = (LPARAM)&pre;
//	pre.flags = PREF_UTF;
//	pre.timestamp = timestamp;
//	//pre.szMessage = (char *)message;
//	
//	::CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
//}