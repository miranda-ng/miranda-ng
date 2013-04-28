#include "skype_proto.h"

bool CSkypeProto::IsMessageInDB(HANDLE hContact, DWORD timestamp, const char *guid, int flag)
{
	bool result = false;

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
			if (::memcmp(&dbei.pBlob[dbei.cbBlob - 32], guid, 32) == 0)
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

HANDLE CSkypeProto::AddDBEvent(HANDLE hContact, WORD type, DWORD timestamp, DWORD flags, DWORD cbBlob, PBYTE pBlob)
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

void CSkypeProto::RaiseAuthRequestEvent(DWORD timestamp, CContact::Ref contact)
{
	char *sid = ::mir_strdup(contact->GetSid());
	char *nick = ::mir_strdup(contact->GetNick());

	SEString data;

	contact->GetPropReceivedAuthrequest(data);
	char *reason = ::mir_strdup(data);

	SEString last;
	contact->GetFullname(data, last);
	char *firstName = ::mir_strdup(data);
	char *lastName = ::mir_strdup(last);

	HANDLE hContact = this->AddContact(contact);

	/*blob is: 0(DWORD), hContact(DWORD), nick(ASCIIZ), firstName(ASCIIZ), lastName(ASCIIZ), sid(ASCIIZ), reason(ASCIIZ)*/
	DWORD cbBlob = (DWORD)
		(sizeof(DWORD) * 2 + 
		::strlen(nick) + 
		::strlen(firstName) + 
		::strlen(lastName) + 
		::strlen(sid) + 
		::strlen(reason) + 
		5);

	PBYTE pBlob, pCurBlob;
	pCurBlob = pBlob = (PBYTE)::mir_alloc(cbBlob);

	*((PDWORD)pCurBlob) = 0;
	pCurBlob += sizeof(DWORD);
	*((PDWORD)pCurBlob) = (DWORD)hContact;
	pCurBlob += sizeof(DWORD);
	::strcpy((char *)pCurBlob, nick);
	pCurBlob += ::strlen(nick) + 1;
	::strcpy((char *)pCurBlob, firstName);
	pCurBlob += ::strlen(firstName) + 1;
	::strcpy((char *)pCurBlob, lastName);
	pCurBlob += ::strlen(lastName) + 1;
	::strcpy((char *)pCurBlob, sid);
	pCurBlob += ::strlen(sid) + 1;
	::strcpy((char *)pCurBlob, reason);

	this->AddDBEvent(hContact, EVENTTYPE_AUTHREQUEST, time(NULL), PREF_UTF, cbBlob, pBlob);
}

void CSkypeProto::RaiseMessageReceivedEvent(HANDLE hContact, DWORD timestamp, const char *guid, const char *message, bool isUnreaded)
{
	if ( !isUnreaded)
		if (this->IsMessageInDB(hContact, timestamp, guid))
			return;

	PROTORECVEVENT recv;
	recv.flags = PREF_UTF;
	recv.lParam = (LPARAM)guid;
	recv.timestamp = timestamp;
	recv.szMessage = ::mir_strdup(message);

	::ProtoChainRecvMsg(hContact, &recv);
}

void CSkypeProto::RaiseMessageSendedEvent(HANDLE hContact, DWORD timestamp, const char *guid, const char *message, bool isUnreaded)
{
	if (this->IsMessageInDB(hContact, timestamp, guid, DBEF_SENT))
		return;

	int guidLen = (int)::strlen(guid);

	int  msgLen = (int)::strlen(message) + 1;
	char *msg = (char *)::mir_alloc(msgLen + guidLen);

	::strcpy(msg, message);
	msg[msgLen - 1] = 0;
	::memcpy((char *)&msg[msgLen], guid, 32);

	DWORD flags = DBEF_UTF | DBEF_SENT;
	if ( !isUnreaded)
		flags |= DBEF_READ;

	this->AddDBEvent(
		hContact, 
		EVENTTYPE_MESSAGE, 
		timestamp, 
		flags, 
		msgLen + guidLen, 
		(PBYTE)msg);
}