#include "skype.h"

bool CSkypeProto::IsMessageInDB(MCONTACT hContact, DWORD timestamp, SEBinary &guid, int flag)
{
	for (HANDLE hDbEvent = ::db_event_last(hContact); hDbEvent; hDbEvent = ::db_event_prev(hContact, hDbEvent))
	{
		DBEVENTINFO dbei = { sizeof(dbei) };
		dbei.cbBlob = ::db_event_getBlobSize(hDbEvent);
		if (dbei.cbBlob < guid.size())
			continue;

		mir_ptr<BYTE> blob((PBYTE)::mir_alloc(dbei.cbBlob));
		dbei.pBlob = blob;
		::db_event_get(hDbEvent, &dbei);

		if (dbei.timestamp < timestamp)
			break;

		int sendFlag = dbei.flags & DBEF_SENT;
		if ((dbei.eventType == EVENTTYPE_MESSAGE || dbei.eventType == SKYPE_DB_EVENT_TYPE_EMOTE) && sendFlag == flag)
			if (::memcmp(&dbei.pBlob[dbei.cbBlob - guid.size()], guid.data(), guid.size()) == 0)
				return true;
	}

	return false;
}

HANDLE CSkypeProto::AddDBEvent(MCONTACT hContact, WORD type, DWORD timestamp, DWORD flags, DWORD cbBlob, PBYTE pBlob)
{
	DBEVENTINFO dbei = { sizeof(dbei) };
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

	MCONTACT hContact = this->AddContact(contact);

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

	this->AddDBEvent(hContact, EVENTTYPE_AUTHREQUEST, time(NULL), DBEF_UTF, cbBlob, pBlob);
}

void CSkypeProto::RaiseMessageSentEvent(MCONTACT hContact, DWORD timestamp, SEBinary &guid, const char *message, bool isUnread)
{
	if (this->IsMessageInDB(hContact, timestamp, guid, DBEF_SENT))
		return;

	int guidLen = (int)guid.size();

	int  msgLen = (int)::strlen(message) + 1;
	ptrA msg((char *)::mir_alloc(msgLen + guidLen));

	::strcpy(msg, message);
	msg[msgLen - 1] = 0;
	::memcpy((char *)&msg[msgLen], guid.data(), guidLen);

	DWORD flags = DBEF_UTF | DBEF_SENT;
	if ( !isUnread)
		flags |= DBEF_READ;

	this->AddDBEvent(
		hContact, 
		EVENTTYPE_MESSAGE,
		timestamp, 
		flags, 
		msgLen + guidLen, 
		(PBYTE)(char*)msg);
}
