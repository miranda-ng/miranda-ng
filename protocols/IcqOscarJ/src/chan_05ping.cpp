// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2009 Joe Kucera
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

void CIcqProto::handlePingChannel(BYTE *buf, WORD datalen)
{
	debugLogA("Warning: Ignoring server packet on PING channel");
}


void __cdecl CIcqProto::KeepAliveThread(void *arg)
{
	serverthread_info *info = (serverthread_info*)arg;
	icq_packet packet;
	DWORD dwInterval = getDword("KeepAliveInterval", KEEPALIVE_INTERVAL);

	debugLogA("Keep alive thread starting.");

	info->hKeepAliveEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	for (;;)
	{
		DWORD dwWait = ICQWaitForSingleObject(info->hKeepAliveEvent, dwInterval);
		if (serverThreadHandle == NULL) // connection lost, end
			break;
		if (dwWait == WAIT_TIMEOUT)
		{
			// Send a keep alive packet to server
			packet.wLen = 0;
			write_flap(&packet, ICQ_PING_CHAN);
			sendServPacket(&packet);
		}
		else if (dwWait == WAIT_IO_COMPLETION)
			// Possible shutdown in progress
			if (Miranda_Terminated()) break;
			else
				break;
	}

	debugLogA("Keep alive thread ended.");

	CloseHandle(info->hKeepAliveEvent);
	info->hKeepAliveEvent = NULL;
}


void CIcqProto::StartKeepAlive(serverthread_info *info)
{
	if (info->hKeepAliveEvent) // start only once
		return;

	if (getByte("KeepAlive", DEFAULT_KEEPALIVE_ENABLED))
		CloseHandle( ForkThreadEx(&CIcqProto::KeepAliveThread, info, 0));
}


void CIcqProto::StopKeepAlive(serverthread_info *info)
{	// finish keep alive thread
	if (info->hKeepAliveEvent)
		SetEvent(info->hKeepAliveEvent);
}
