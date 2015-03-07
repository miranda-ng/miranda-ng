// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera
// Copyright © 2012-2014 Miranda NG Team
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  Handles packet & message cookies
// -----------------------------------------------------------------------------

#include "icqoscar.h"

#define INVALID_COOKIE_INDEX -1

void CIcqProto::RemoveExpiredCookies()
{
	time_t tNow = time(NULL);

	for (int i = cookies.getCount() - 1; i >= 0; i--) {
		icq_cookie_info *cookie = cookies[i];
		if ((cookie->dwTime + COOKIE_TIMEOUT) < tNow) {
			cookies.remove(i);
			SAFE_FREE((void**)&cookie);
		}
	}
}

// Generate and allocate cookie
DWORD CIcqProto::AllocateCookie(BYTE bType, WORD wIdent, MCONTACT hContact, void *pvExtra)
{
	mir_cslock l(cookieMutex);

	DWORD dwThisSeq = wCookieSeq++;
	dwThisSeq &= 0x7FFF;
	dwThisSeq |= wIdent << 0x10;

	icq_cookie_info* p = (icq_cookie_info*)SAFE_MALLOC(sizeof(icq_cookie_info));
	if (p) {
		p->bType = bType;
		p->dwCookie = dwThisSeq;
		p->hContact = hContact;
		p->pvExtra = pvExtra;
		p->dwTime = time(NULL);
		cookies.insert(p);
	}
	return dwThisSeq;
}

DWORD CIcqProto::GenerateCookie(WORD wIdent)
{
	mir_cslock l(cookieMutex);

	DWORD dwThisSeq = wCookieSeq++;
	dwThisSeq &= 0x7FFF;
	dwThisSeq |= wIdent << 0x10;
	return dwThisSeq;
}

int CIcqProto::GetCookieType(DWORD dwCookie)
{
	mir_cslock l(cookieMutex);

	int i = cookies.getIndex((icq_cookie_info*)&dwCookie);
	if (i != INVALID_COOKIE_INDEX)
		i = cookies[i]->bType;

	return i;
}

int CIcqProto::FindCookie(DWORD dwCookie, MCONTACT *phContact, void **ppvExtra)
{
	mir_cslock l(cookieMutex);

	int i = cookies.getIndex((icq_cookie_info*)&dwCookie);
	if (i != INVALID_COOKIE_INDEX) {
		if (phContact)
			*phContact = cookies[i]->hContact;
		if (ppvExtra)
			*ppvExtra = cookies[i]->pvExtra;

		// Cookie found
		return 1;
	}

	return 0;
}

int CIcqProto::FindCookieByData(void *pvExtra, DWORD *pdwCookie, MCONTACT *phContact)
{
	mir_cslock l(cookieMutex);

	for (int i = 0; i < cookies.getCount(); i++) {
		if (pvExtra == cookies[i]->pvExtra) {
			if (phContact)
				*phContact = cookies[i]->hContact;
			if (pdwCookie)
				*pdwCookie = cookies[i]->dwCookie;

			// Cookie found
			return 1;
		}
	}

	return 0;
}

int CIcqProto::FindCookieByType(BYTE bType, DWORD *pdwCookie, MCONTACT *phContact, void** ppvExtra)
{
	mir_cslock l(cookieMutex);

	for (int i = 0; i < cookies.getCount(); i++) {
		if (bType == cookies[i]->bType) {
			if (pdwCookie)
				*pdwCookie = cookies[i]->dwCookie;
			if (phContact)
				*phContact = cookies[i]->hContact;
			if (ppvExtra)
				*ppvExtra = cookies[i]->pvExtra;

			// Cookie found
			return 1;
		}
	}

	return 0;
}

int CIcqProto::FindMessageCookie(DWORD dwMsgID1, DWORD dwMsgID2, DWORD *pdwCookie, MCONTACT *phContact, cookie_message_data **ppvExtra)
{
	mir_cslock l(cookieMutex);

	for (int i = 0; i < cookies.getCount(); i++) {
		if (cookies[i]->bType == CKT_MESSAGE || cookies[i]->bType == CKT_FILE || cookies[i]->bType == CKT_REVERSEDIRECT) {
			// message cookie found
			cookie_message_data *pCookie = (cookie_message_data*)cookies[i]->pvExtra;

			if (pCookie->dwMsgID1 == dwMsgID1 && pCookie->dwMsgID2 == dwMsgID2) {
				if (phContact)
					*phContact = cookies[i]->hContact;
				if (pdwCookie)
					*pdwCookie = cookies[i]->dwCookie;
				if (ppvExtra)
					*ppvExtra = pCookie;

				// Cookie found
				return 1;
			}
		}
	}

	return 0;
}

void CIcqProto::FreeCookie(DWORD dwCookie)
{
	mir_cslock l(cookieMutex);

	int i = cookies.getIndex((icq_cookie_info*)&dwCookie);
	if (i != INVALID_COOKIE_INDEX) {
		// Cookie found, remove from list
		icq_cookie_info *cookie = cookies[i];

		cookies.remove(i);
		SAFE_FREE((void**)&cookie);
	}

	RemoveExpiredCookies();
}

void CIcqProto::FreeCookieByData(BYTE bType, void *pvExtra)
{
	mir_cslock l(cookieMutex);

	for (int i = 0; i < cookies.getCount(); i++) {
		icq_cookie_info *cookie = cookies[i];
		if (bType == cookie->bType && pvExtra == cookie->pvExtra) {
			// Cookie found, remove from list
			cookies.remove(i);
			SAFE_FREE((void**)&cookie);
			break;
		}
	}

	RemoveExpiredCookies();
}

void CIcqProto::ReleaseCookie(DWORD dwCookie)
{
	mir_cslock l(cookieMutex);

	int i = cookies.getIndex(( icq_cookie_info* )&dwCookie );
	if (i != INVALID_COOKIE_INDEX) {
		// Cookie found, remove from list
		icq_cookie_info *cookie = cookies[i];

		cookies.remove(i);
		SAFE_FREE((void**)&cookie->pvExtra);
		SAFE_FREE((void**)&cookie);
	}
	RemoveExpiredCookies();
}

void CIcqProto::InitMessageCookie(cookie_message_data *pCookie)
{
	DWORD dwMsgID1;
	DWORD dwMsgID2;

	do {
		// ensure that message ids are unique
		dwMsgID1 = time(NULL);
		dwMsgID2 = RandRange(0, 0x0FFFF);
	}
		while (FindMessageCookie(dwMsgID1, dwMsgID2, NULL, NULL, NULL));

	if (pCookie) {
		pCookie->dwMsgID1 = dwMsgID1;
		pCookie->dwMsgID2 = dwMsgID2;
	}
}

cookie_message_data* CIcqProto::CreateMessageCookie(WORD bMsgType, BYTE bAckType)
{
	cookie_message_data *pCookie = (cookie_message_data*)SAFE_MALLOC(sizeof(cookie_message_data));
	if (pCookie) {
		pCookie->bMessageType = bMsgType;
		pCookie->nAckType = bAckType;

		InitMessageCookie(pCookie);
	}
	return pCookie;
}

cookie_message_data* CIcqProto::CreateMessageCookieData(BYTE bMsgType, MCONTACT hContact, DWORD dwUin, int bUseSrvRelay)
{
	BYTE bAckType;
	WORD wStatus = getContactStatus(hContact);

	if (!getByte(hContact, "SlowSend", getByte("SlowSend", DEFAULT_SLOWSEND)) || (!dwUin && wStatus == ID_STATUS_OFFLINE))
		bAckType = ACKTYPE_NONE;
	else if (bUseSrvRelay)
		bAckType = ACKTYPE_CLIENT;
	else
		bAckType = ACKTYPE_SERVER;

	cookie_message_data* pCookieData = CreateMessageCookie(bMsgType, bAckType);

	// set flag for offline messages - to allow proper error handling
	if (wStatus == ID_STATUS_OFFLINE || wStatus == ID_STATUS_INVISIBLE)
		pCookieData->isOffline = TRUE;

	return pCookieData;
}
