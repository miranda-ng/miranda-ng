// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2009 Joe Kucera
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// -----------------------------------------------------------------------------

#include "icqoscar.h"

void CIcqProto::handleAuthorizationFam(BYTE *pBuffer, WORD wBufferLength, snac_header *pSnacHeader, serverthread_info *info)
{
	switch (pSnacHeader->wSubtype) {

	case ICQ_SIGNON_ERROR:
		{
			WORD wError;

			if (wBufferLength >= 2)
				unpackWord(&pBuffer, &wError);
			else
				wError = 0;

			LogFamilyError(ICQ_AUTHORIZATION_FAMILY, wError);
			break;
		}

	case ICQ_SIGNON_AUTH_KEY:
		handleAuthKeyResponse(pBuffer, wBufferLength, info);
		break;

	case ICQ_SIGNON_LOGIN_REPLY:
		handleLoginReply(pBuffer, wBufferLength, info);
		break;

	default:
		debugLogA("Warning: Ignoring SNAC(x%02x,x%02x) - Unknown SNAC (Flags: %u, Ref: %u)", ICQ_AUTHORIZATION_FAMILY, pSnacHeader->wSubtype, pSnacHeader->wFlags, pSnacHeader->dwRef);
		break;
	}
}

static void icq_encryptPassword(const char *szPassword, BYTE *encrypted)
{
	BYTE table[] =
	{
		0xf3, 0x26, 0x81, 0xc4,
		0x39, 0x86, 0xdb, 0x92,
		0x71, 0xa3, 0xb9, 0xe6,
		0x53, 0x7a, 0x95, 0x7c
	};

	for (int i = 0; szPassword[i]; i++)
		encrypted[i] = (szPassword[i] ^ table[i % 16]);
}

void CIcqProto::sendClientAuth(const char *szKey, WORD wKeyLen, BOOL bSecure)
{
	char szUin[UINMAXLEN];
	WORD wUinLen;
	icq_packet packet;

	wUinLen = strlennull(strUID(m_dwLocalUIN, szUin));

	packet.wLen = (m_bLegacyFix ? 65 : 70) + sizeof(CLIENT_ID_STRING) + wUinLen + wKeyLen + (m_bSecureConnection ? 4 : 0);

	if (bSecure)
	{
		serverPacketInit(&packet, (WORD)(packet.wLen + 10));
		packFNACHeader(&packet, ICQ_AUTHORIZATION_FAMILY, ICQ_SIGNON_LOGIN_REQUEST, 0, 0);
	}
	else
	{
		write_flap(&packet, ICQ_LOGIN_CHAN);
		packDWord(&packet, 0x00000001);
	}
	packTLV(&packet, 0x0001, wUinLen, (LPBYTE)szUin);

	if (bSecure)
	{ // Pack MD5 auth digest
		packTLV(&packet, 0x0025, wKeyLen, (BYTE*)szKey);
		packDWord(&packet, 0x004C0000); // empty TLV(0x4C): unknown
	}
	else
	{ // Pack old style password hash
		BYTE hash[20];

		icq_encryptPassword(szKey, hash);
		packTLV(&packet, 0x0002, wKeyLen, hash);
	}

	// Pack client identification details.
	packTLV(&packet, 0x0003, (WORD)sizeof(CLIENT_ID_STRING)-1, (LPBYTE)CLIENT_ID_STRING);
	packTLVWord(&packet, 0x0017, CLIENT_VERSION_MAJOR);
	packTLVWord(&packet, 0x0018, CLIENT_VERSION_MINOR);
	packTLVWord(&packet, 0x0019, CLIENT_VERSION_LESSER);
	packTLVWord(&packet, 0x001a, CLIENT_VERSION_BUILD);
	packTLVWord(&packet, 0x0016, CLIENT_ID_CODE);
	packTLVDWord(&packet, 0x0014, CLIENT_DISTRIBUTION);
	packTLV(&packet, 0x000f, 0x0002, (LPBYTE)CLIENT_LANGUAGE);
	packTLV(&packet, 0x000e, 0x0002, (LPBYTE)CLIENT_COUNTRY);
	if (!m_bLegacyFix)
		packTLV(&packet, 0x0094, 0x0001, &m_bConnectionLost); // CLIENT_RECONNECT flag
	if (m_bSecureConnection)
		packDWord(&packet, 0x008C0000); // empty TLV(0x8C): use SSL

	sendServPacket(&packet);
}

void CIcqProto::handleAuthKeyResponse(BYTE *buf, WORD wPacketLen, serverthread_info *info)
{
	WORD wKeyLen;
	char szKey[64] = {0};
	mir_md5_state_t state;
	BYTE digest[16];

#ifdef _DEBUG
	debugLogA("Received %s", "ICQ_SIGNON_AUTH_KEY");
#endif

	if (wPacketLen < 2)
	{
		debugLogA("Malformed %s", "ICQ_SIGNON_AUTH_KEY");
		icq_LogMessage(LOG_FATAL, LPGEN("Secure login failed.\nInvalid server response."));
		SetCurrentStatus(ID_STATUS_OFFLINE);
		return;
	}

	unpackWord(&buf, &wKeyLen);
	wPacketLen -= 2;

	if (!wKeyLen || wKeyLen > wPacketLen || wKeyLen > sizeof(szKey))
	{
		debugLogA("Invalid length in %s: %u", "ICQ_SIGNON_AUTH_KEY", wKeyLen);
		icq_LogMessage(LOG_FATAL, LPGEN("Secure login failed.\nInvalid key length."));
		SetCurrentStatus(ID_STATUS_OFFLINE);
		return;
	}

	unpackString(&buf, szKey, wKeyLen);

	mir_md5_init(&state);
	mir_md5_append(&state, info->szAuthKey, info->wAuthKeyLen);
	mir_md5_finish(&state, digest);

	mir_md5_init(&state);
	mir_md5_append(&state, (LPBYTE)szKey, wKeyLen);
	mir_md5_append(&state, digest, 16);
	mir_md5_append(&state, (LPBYTE)CLIENT_MD5_STRING, sizeof(CLIENT_MD5_STRING)-1);
	mir_md5_finish(&state, digest);

#ifdef _DEBUG
	debugLogA("Sending ICQ_SIGNON_LOGIN_REQUEST to login server");
#endif
	sendClientAuth((char*)digest, 0x10, TRUE);
}
