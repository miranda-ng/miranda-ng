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

#include "icqoscar.h"

void CIcqProto::handleLookupFam(BYTE *pBuffer, WORD wBufferLength, snac_header* pSnacHeader)
{
	switch (pSnacHeader->wSubtype) {

	case ICQ_LOOKUP_EMAIL_REPLY: // AIM search reply
		handleLookupEmailReply(pBuffer, wBufferLength, pSnacHeader->dwRef);
		break;

	case ICQ_ERROR:
		{ 
			WORD wError;
			cookie_search *pCookie;

			if (wBufferLength >= 2)
				unpackWord(&pBuffer, &wError);
			else 
				wError = 0;

			if (FindCookie(pSnacHeader->dwRef, NULL, (void**)&pCookie))
			{
				if (wError == 0x14)
					debugLogA("Lookup: No results");

				ReleaseLookupCookie(pSnacHeader->dwRef, pCookie);

				if (wError == 0x14) return;
			}

			LogFamilyError(ICQ_LOOKUP_FAMILY, wError);
			break;
		}

	default:
		debugLogA("Warning: Ignoring SNAC(x%02x,x%02x) - Unknown SNAC (Flags: %u, Ref: %u)", ICQ_LOOKUP_FAMILY, pSnacHeader->wSubtype, pSnacHeader->wFlags, pSnacHeader->dwRef);
		break;
	}
}

void CIcqProto::ReleaseLookupCookie(DWORD dwCookie, cookie_search *pCookie)
{
	FreeCookie(dwCookie);
	SAFE_FREE(&pCookie->szObject);

	if (pCookie->dwMainId && !pCookie->dwStatus)
	{ // we need to wait for main search
		pCookie->dwStatus = 1;
	}
	else
	{ // finish everything
		if (pCookie->dwMainId)
			ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)pCookie->dwMainId, 0);
		else // we are single
			ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)dwCookie, 0);

		SAFE_FREE((void**)&pCookie);
	}
}

void CIcqProto::handleLookupEmailReply(BYTE* buf, WORD wLen, DWORD dwCookie)
{
	ICQSEARCHRESULT sr = {0};
	oscar_tlv_chain *pChain;
	cookie_search *pCookie;

	if (!FindCookie(dwCookie, NULL, (void**)&pCookie))
	{
		debugLogA("Error: Received unexpected lookup reply");
		return;
	}

	debugLogA("SNAC(0x0A,0x3): Lookup reply");

	sr.hdr.cbSize = sizeof(sr);
	sr.hdr.flags = PSR_TCHAR;
	sr.hdr.email = ansi_to_tchar(pCookie->szObject);

	// Syntax check, read chain
	if (wLen >= 4 && (pChain = readIntoTLVChain(&buf, wLen, 0)))
	{
		for (WORD i = 1; TRUE; i++)
		{ // collect the results
			char *szUid = pChain->getString(0x01, i);
			if (!szUid) break;
			sr.hdr.id = ansi_to_tchar(szUid);
			sr.hdr.nick = sr.hdr.id;
			// broadcast the result
			if (pCookie->dwMainId)
				ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)pCookie->dwMainId, (LPARAM)&sr);
			else
				ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)dwCookie, (LPARAM)&sr);
			SAFE_FREE(&sr.hdr.id);
			SAFE_FREE(&szUid);
		}
		disposeChain(&pChain);
	}
	SAFE_FREE(&sr.hdr.email);

	ReleaseLookupCookie(dwCookie, pCookie);
}
