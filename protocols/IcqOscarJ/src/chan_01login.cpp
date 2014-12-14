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

void CIcqProto::handleLoginChannel(BYTE*, size_t, serverthread_info *info)
{
	icq_packet packet;

	debugLogA("Received SRV_HELLO from %s", info->isLoginServer ? "login server" : "communication server");

	// isLoginServer is "1" if we just received SRV_HELLO
	if (info->isLoginServer) {
		if (m_bSecureLogin) {
			debugLogA("Sending %s to %s", "CLI_HELLO", "login server");

			packet.wLen = 12;
			write_flap(&packet, ICQ_LOGIN_CHAN);
			packDWord(&packet, 0x00000001);
			packTLVDWord(&packet, 0x8003, 0x00100000); // unknown
			sendServPacket(&packet);  // greet login server

			char szUin[UINMAXLEN];
			size_t wUinLen = mir_strlen(strUID(m_dwLocalUIN, szUin));

			debugLogA("Sending %s to %s", "ICQ_SIGNON_AUTH_REQUEST", "login server");

			serverPacketInit(&packet, 14 + wUinLen);
			packFNACHeader(&packet, ICQ_AUTHORIZATION_FAMILY, ICQ_SIGNON_AUTH_REQUEST, 0, 0);
			packTLV(&packet, 0x0001, wUinLen, (LPBYTE)szUin);
			sendServPacket(&packet);  // request login digest
		}
		else {
			sendClientAuth((char*)info->szAuthKey, info->wAuthKeyLen, FALSE);
			debugLogA("Sent CLI_IDENT to %s", "login server");
		}

		info->isLoginServer = false;
		if (info->cookieDataLen) {
			SAFE_FREE((void**)&info->cookieData);
			info->cookieDataLen = 0;
		}
	}
	else {
		if (info->cookieDataLen) {
			wLocalSequence = generate_flap_sequence();

			serverCookieInit(&packet, info->cookieData, info->cookieDataLen);
			sendServPacket(&packet);

			debugLogA("Sent CLI_IDENT to %s", "communication server");

			SAFE_FREE((void**)&info->cookieData);
			info->cookieDataLen = 0;
		}
		else // We need a cookie to identify us to the communication server
			debugLogA("Error: Connected to %s without a cookie!", "communication server");
	}
}
