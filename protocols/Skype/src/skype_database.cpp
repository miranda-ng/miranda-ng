#include "skype_proto.h"

HANDLE CSkypeProto::AddDataBaseEvent(HANDLE hContact, WORD type, DWORD time, DWORD flags, DWORD cbBlob, PBYTE pBlob)
{
	DBEVENTINFO dbei = {0};

	dbei.cbSize = sizeof(dbei);
	dbei.szModule = this->m_szModuleName;
	dbei.timestamp = time;
	dbei.flags = flags;
	dbei.eventType = type;
	dbei.cbBlob = cbBlob;
	dbei.pBlob = pBlob;

	return (HANDLE)CallService(MS_DB_EVENT_ADD, (WPARAM)hContact, (LPARAM)&dbei);
}

void CSkypeProto::RaiseAuthRequestEvent(
	DWORD timestamp,
	CContact::Ref contact)
{	
	SEString data;

	contact->GetPropSkypename(data);
	char *sid = ::mir_strdup(data);

	contact->GetPropDisplayname(data);
	char *nick = ::mir_utf8decodeA(data);

	contact->GetPropReceivedAuthrequest(data);
	char* reason = ::mir_utf8decodeA((const char*)data);

	contact->GetPropFullname(data);
	char* fullname = ::mir_utf8decodeA((const char*)data);

	char* firstName = strtok(fullname, " ");
	char* lastName = strtok(NULL, " ");
	if (lastName == NULL)
	{
		lastName = "";
	}

	PROTORECVEVENT pre = {0};

	CCSDATA ccs = {0};
	ccs.szProtoService = PSR_AUTH;
	ccs.hContact = this->AddContact(contact);
	ccs.wParam = 0;
	ccs.lParam = (LPARAM)&pre;
	pre.timestamp = timestamp;
	pre.lParam = (DWORD)
		(sizeof(DWORD) * 2) + 
		::strlen(nick) + 
		::strlen(firstName) + 
		::strlen(lastName) + 
		::strlen(sid) + 
		::strlen(reason) + 		
		5;

	/*blob is: 0(DWORD), hContact(DWORD), nick(ASCIIZ), firstName(ASCIIZ), lastName(ASCIIZ), sid(ASCIIZ), reason(ASCIIZ)*/
	char *pCurBlob = pre.szMessage = (char*)::mir_alloc(pre.lParam);
	
	*((PDWORD)pCurBlob) = 0; pCurBlob += sizeof(DWORD);
	*((PDWORD)pCurBlob) = (DWORD)ccs.hContact; pCurBlob += sizeof(DWORD);
	::strcpy((char*)pCurBlob, nick); pCurBlob += ::strlen(nick) + 1;
	::strcpy((char*)pCurBlob, firstName); pCurBlob += ::strlen(firstName) + 1;
	::strcpy((char*)pCurBlob, lastName); pCurBlob += ::strlen(lastName) + 1;
	::strcpy((char*)pCurBlob, sid); pCurBlob += ::strlen(sid) + 1;
	::strcpy((char*)pCurBlob, reason);
	
	::CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
}

bool CSkypeProto::IsMessageInDB(HANDLE hContact, DWORD timestamp, const char* message, int flag)
{
	bool result = false;

	int length = ::strlen(message);

	HANDLE hDbEvent = ::db_event_last(hContact);
	while (hDbEvent) 
	{
		DBEVENTINFO dbei = { sizeof(dbei) };
		dbei.cbBlob = ::db_event_getBlobSize(hDbEvent);
		dbei.pBlob = (PBYTE)::mir_alloc(dbei.cbBlob);
		::db_event_get(hDbEvent, &dbei);

		if (dbei.timestamp < timestamp)
		{
			::mir_free(dbei.pBlob);
			break;
		}

		int sendFlag = dbei.flags & DBEF_SENT;
		if (dbei.eventType == EVENTTYPE_MESSAGE && sendFlag == flag)
		{
			char *dbMessage = (char *)dbei.pBlob;
			if (::strncmp(dbMessage, message, length) == 0 && dbei.timestamp == timestamp)
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

void CSkypeProto::RaiseMessageReceivedEvent(
	HANDLE hContact,
	DWORD timestamp,
	const char* message,
	bool isNeedCheck)
{	
	if (isNeedCheck)
		if (this->IsMessageInDB(hContact, timestamp, message))
			return;

	PROTORECVEVENT recv;
	recv.flags = PREF_UTF;
	recv.timestamp = timestamp;
	recv.szMessage = ::mir_strdup(message);
	
	::ProtoChainRecvMsg(hContact, &recv);
}

void CSkypeProto::RaiseMessageSendedEvent(
	HANDLE hContact,
	DWORD timestamp,
	const char* message)
{	
	if (this->IsMessageInDB(hContact, timestamp, message, DBEF_SENT))
		return;

	DBEVENTINFO dbei = { 0 };
	dbei.cbSize = sizeof(dbei);
	dbei.szModule = this->m_szModuleName;
	dbei.timestamp = timestamp;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = (DWORD)::strlen(message) + 1;
	dbei.pBlob = (PBYTE)::mir_strdup(message);
	dbei.flags = DBEF_UTF | DBEF_SENT;

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