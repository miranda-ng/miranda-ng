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
	const char* sid, 
	const char* nick, 
	const char* firstName,
	const char* lastName,
	const char* reason)
{	
	PROTORECVEVENT pre = {0};

	CCSDATA ccs = {0};
	ccs.szProtoService = PSR_AUTH;
	ccs.hContact = this->AddContactBySid(sid, nick);
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

void CSkypeProto::RaiseMessageReceivedEvent(
	DWORD timestamp,
	const char* sid, 
	const char* nick, 
	const char* message)
{	
	PROTORECVEVENT pre = {0};

	CCSDATA ccs = {0};
	ccs.szProtoService = PSR_MESSAGE;
	ccs.hContact = this->AddContactBySid(sid, nick);
	ccs.wParam = 0;
	ccs.lParam = (LPARAM)&pre;
	pre.flags = PREF_UTF;
	pre.timestamp = timestamp;
	pre.szMessage = (char *)message;
	
	::CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
}

void CSkypeProto::RaiseMessageSendedEvent(
	DWORD timestamp,
	const char* sid, 
	const char* nick, 
	const char* message)
{	
	PROTORECVEVENT pre = {0};

	CCSDATA ccs = {0};
	ccs.szProtoService = PSR_MESSAGE;
	ccs.hContact = this->AddContactBySid(sid, nick);
	ccs.wParam = 0;
	ccs.lParam = (LPARAM)&pre;
	pre.flags = PREF_UTF;
	pre.timestamp = timestamp;
	pre.szMessage = (char *)message;
	
	::CallService(MS_PROTO_CHAINSEND, 0, (LPARAM)&ccs);
}

void CSkypeProto::RaiseFileReceivedEvent(
	DWORD timestamp,
	const char* sid, 
	const char* nick, 
	const char* message)
{	
	PROTORECVFILET pre = {0};

	CCSDATA ccs = {0};
	ccs.szProtoService = PSR_FILE;
	ccs.hContact = this->AddContactBySid(sid, nick);
	ccs.wParam = 0;
	ccs.lParam = (LPARAM)&pre;
	pre.flags = PREF_UTF;
	pre.timestamp = timestamp;
	//pre.szMessage = (char *)message;
	
	::CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
}