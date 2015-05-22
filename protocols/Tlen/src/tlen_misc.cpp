/*

Jabber Protocol Plugin for Miranda IM
Tlen Protocol Plugin for Miranda NG
Copyright (C) 2002-2004  Santithorn Bunchua
Copyright (C) 2004-2007  Piotr Piastucki

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

#include "tlen.h"
#include "tlen_list.h"

void TlenDBAddEvent(TlenProtocol *proto, MCONTACT hContact, int eventType, DWORD flags, PBYTE pBlob, DWORD cbBlob)
{
	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = proto->m_szModuleName;
	dbei.timestamp = (DWORD) time(NULL);
	dbei.flags = flags;
	dbei.eventType = eventType;
	dbei.cbBlob = cbBlob;
	dbei.pBlob = pBlob;
	db_event_add(hContact, &dbei);
}

void TlenDBAddAuthRequest(TlenProtocol *proto, char *jid, char *nick)
{
	char *s;
	PBYTE pCurBlob;
	PBYTE pBlob;
	DWORD cbBlob;
	MCONTACT hContact;

	if ((hContact=TlenHContactFromJID(proto, jid)) == NULL) {
		hContact = (MCONTACT) CallService(MS_DB_CONTACT_ADD, 0, 0);
		CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)proto->m_szModuleName);
		// strip resource if present
		s = TlenLoginFromJID(jid);
		_strlwr(s);
		db_set_s(hContact, proto->m_szModuleName, "jid", s);
		mir_free(s);
	}
	else {
		db_unset(hContact, proto->m_szModuleName, "Hidden");
	}
	db_set_s(hContact, proto->m_szModuleName, "Nick", nick);
	proto->debugLogA("auth request: %s, %s", jid, nick);
	//blob is: uin(DWORD), hContact(HANDLE), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ), reason(ASCIIZ)
	//blob is: 0(DWORD), hContact(HANDLE), nick(ASCIIZ), ""(ASCIIZ), ""(ASCIIZ), email(ASCIIZ), ""(ASCIIZ)
	cbBlob = sizeof(DWORD) + sizeof(HANDLE) + (int)mir_strlen(nick) + (int)mir_strlen(jid) + 5;
	pBlob = pCurBlob = (PBYTE) mir_alloc(cbBlob);
	*((PDWORD)pCurBlob) = 0; pCurBlob += sizeof(DWORD);
	*((PDWORD)pCurBlob) = (DWORD)hContact; pCurBlob += sizeof(DWORD);
	strcpy((char *) pCurBlob, nick); pCurBlob += mir_strlen(nick)+1;
	*pCurBlob = '\0'; pCurBlob++;		//firstName
	*pCurBlob = '\0'; pCurBlob++;		//lastName
	strcpy((char *) pCurBlob, jid); pCurBlob += mir_strlen(jid)+1;
	*pCurBlob = '\0';					//reason
	TlenDBAddEvent(proto, NULL, EVENTTYPE_AUTHREQUEST, 0, pBlob, cbBlob);
}

char *TlenJIDFromHContact(TlenProtocol *proto, MCONTACT hContact)
{
	char *p = NULL;
	DBVARIANT dbv;
	if (!db_get(hContact, proto->m_szModuleName, "jid", &dbv)) {
		p = mir_strdup(dbv.pszVal);
		db_free(&dbv);
	}
	return p;
}

MCONTACT TlenHContactFromJID(TlenProtocol *proto, const char *jid)
{
	DBVARIANT dbv;
	char *p;
	if (jid == NULL)
		return NULL;

	for (MCONTACT hContact = db_find_first(proto->m_szModuleName); hContact; hContact = db_find_next(hContact, proto->m_szModuleName)) {
		if ( db_get_s(hContact, proto->m_szModuleName, "jid", &dbv))
			continue;

		if ((p=dbv.pszVal) != NULL) {
			if (!stricmp(p, jid)) {	// exact match (node@domain/resource)
				db_free(&dbv);
				return hContact;
			}
		}
		db_free(&dbv);
	}

	return NULL;
}

MCONTACT TlenDBCreateContact(TlenProtocol *proto, char *jid, char *nick, BOOL temporary)
{
	MCONTACT hContact;
	if (jid == NULL || jid[0] == '\0')
		return NULL;

	if ((hContact=TlenHContactFromJID(proto, jid)) == NULL) {
		hContact = (MCONTACT) CallService(MS_DB_CONTACT_ADD, 0, 0);
		CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)proto->m_szModuleName);
		db_set_s(hContact, proto->m_szModuleName, "jid", jid);
		if (nick != NULL && nick[0] != '\0')
			db_set_s(hContact, proto->m_szModuleName, "Nick", nick);
		if (temporary)
			db_set_b(hContact, "CList", "NotOnList", 1);
	}
	return hContact;
}
