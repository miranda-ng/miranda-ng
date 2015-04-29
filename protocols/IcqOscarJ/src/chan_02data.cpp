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
//  Handle channel 2 (Data) packets
// -----------------------------------------------------------------------------

#include "stdafx.h"

void CIcqProto::handleDataChannel(BYTE *pBuffer, size_t wBufferLength, serverthread_info *info)
{
	snac_header snacHeader = {0};

	if (!unpackSnacHeader(&snacHeader, &pBuffer, &wBufferLength) || !snacHeader.bValid)
		debugLogA("Error: Failed to parse SNAC header");
	else {
		if (snacHeader.wFlags & 0x8000)
			debugLogA(" Received SNAC(x%02X,x%02X), version %u", snacHeader.wFamily, snacHeader.wSubtype, snacHeader.wVersion);
		else
			debugLogA(" Received SNAC(x%02X,x%02X)", snacHeader.wFamily, snacHeader.wSubtype);

		switch (snacHeader.wFamily) {

		case ICQ_SERVICE_FAMILY:
			handleServiceFam(pBuffer, wBufferLength, &snacHeader, info);
			break;

		case ICQ_LOCATION_FAMILY:
			handleLocationFam(pBuffer, wBufferLength, &snacHeader);
			break;

		case ICQ_BUDDY_FAMILY:
			handleBuddyFam(pBuffer, wBufferLength, &snacHeader, info);
			break;

		case ICQ_MSG_FAMILY:
			handleMsgFam(pBuffer, wBufferLength, &snacHeader);
			break;

		case ICQ_BOS_FAMILY:
			handleBosFam(pBuffer, wBufferLength, &snacHeader);
			break;

		case ICQ_LOOKUP_FAMILY:
			handleLookupFam(pBuffer, wBufferLength, &snacHeader);
			break;

		case ICQ_STATS_FAMILY:
			handleStatusFam(pBuffer, wBufferLength, &snacHeader);
			break;

		case ICQ_LISTS_FAMILY:
			handleServCListFam(pBuffer, wBufferLength, &snacHeader, info);
			break;

		case ICQ_EXTENSIONS_FAMILY:
			handleIcqExtensionsFam(pBuffer, wBufferLength, &snacHeader);
			break;

		case ICQ_AUTHORIZATION_FAMILY:
			handleAuthorizationFam(pBuffer, wBufferLength, &snacHeader, info);
			break;

		default:
			debugLogA("Ignoring SNAC(x%02X,x%02X) - FAMILYx%02X not implemented", snacHeader.wFamily, snacHeader.wSubtype, snacHeader.wFamily);
			break;
		}
	}
}


int unpackSnacHeader(snac_header *pSnacHeader, BYTE **pBuffer, size_t *pwBufferLength)
{
	WORD wRef1, wRef2;

	// Check header
	if (!pSnacHeader)
		return 0;

	// 10 bytes is the minimum size of a header
	if (*pwBufferLength < 10) {
		// Buffer overflow
		pSnacHeader->bValid = FALSE;
		return 1;
	}

	// Unpack all the standard data
	unpackWord(pBuffer, &(pSnacHeader->wFamily));
	unpackWord(pBuffer, &(pSnacHeader->wSubtype));
	unpackWord(pBuffer, &(pSnacHeader->wFlags));
	unpackWord(pBuffer, &wRef1); // unpack reference id (sequence)
	unpackWord(pBuffer, &wRef2); // command
	pSnacHeader->dwRef = wRef1 | (wRef2 << 0x10);

	*pwBufferLength -= 10;

	// If flag bit 15 is set, we also have a version tag
	// (...at least that is what I think it is)
	if (pSnacHeader->wFlags & 0x8000) {
		if (*pwBufferLength >= 2) {
			WORD wExtraBytes = 0;

			unpackWord(pBuffer, &wExtraBytes);
			*pwBufferLength -= 2;

			if (*pwBufferLength >= wExtraBytes) {
				if (wExtraBytes == 6) {
					*pBuffer += 4; // TLV type and length?
					unpackWord(pBuffer, &(pSnacHeader->wVersion));
					*pwBufferLength -= wExtraBytes;
					pSnacHeader->bValid = TRUE;
				}
				else if (wExtraBytes == 0x0E) {
					*pBuffer += 8; // TLV(2) - unknown
					*pBuffer += 4;
					unpackWord(pBuffer, &(pSnacHeader->wVersion));
					*pwBufferLength -= wExtraBytes;
					pSnacHeader->bValid = TRUE;
				}
				else {
					*pBuffer += wExtraBytes;
					*pwBufferLength -= wExtraBytes;
					pSnacHeader->bValid = TRUE;
				}
			}
			else // Buffer overflow
				pSnacHeader->bValid = FALSE;
		}
		else // Buffer overflow
			pSnacHeader->bValid = FALSE;
	}
	else pSnacHeader->bValid = TRUE;

	return 1;
}


void CIcqProto::LogFamilyError(WORD wFamily, WORD wError)
{
	char *msg;

	switch (wError) {
		case 0x01: msg = "Invalid SNAC header"; break;
		case 0x02: msg = "Server rate limit exceeded"; break;
		case 0x03: msg = "Client rate limit exceeded"; break;
		case 0x04: msg = "Recipient is not logged in"; break;
		case 0x05: msg = "Requested service unavailable"; break;
		case 0x06: msg = "Requested service not defined"; break;
		case 0x07: msg = "You sent obsolete SNAC"; break;
		case 0x08: msg = "Not supported by server"; break;
		case 0x09: msg = "Not supported by client"; break;
		case 0x0A: msg = "Refused by client"; break;
		case 0x0B: msg = "Reply too big"; break;
		case 0x0C: msg = "Responses lost"; break;
		case 0x0D: msg = "Request denied"; break;
		case 0x0E: msg = "Incorrect SNAC format"; break;
		case 0x0F: msg = "Insufficient rights"; break;
		case 0x10: msg = "In local permit/deny (recipient blocked)"; break;
		case 0x11: msg = "Sender is too evil"; break;
		case 0x12: msg = "Receiver is too evil"; break;
		case 0x13: msg = "User temporarily unavailable"; break;
		case 0x14: msg = "No match"; break;
		case 0x15: msg = "List overflow"; break;
		case 0x16: msg = "Request ambiguous"; break;
		case 0x17: msg = "Server queue full"; break;
		case 0x18: msg = "Not while on AOL"; break;
		case 0x19: msg = "Query failed"; break;
		case 0x1A: msg = "Timeout"; break;
		case 0x1C: msg = "General failure"; break;
		case 0x1D: msg = "Progress"; break;
		case 0x1E: msg = "In free area"; break;
		case 0x1F: msg = "Restricted by parental controls"; break;
		case 0x20: msg = "Remote restricted by parental controls"; break;
		default:   msg = ""; break;
	}

	debugLogA("SNAC(x%02X,x01) - Error(%u): %s", wFamily, wError, msg);
}
