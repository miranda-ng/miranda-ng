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

void CIcqProto::handlePingChannel(BYTE *buf, size_t datalen)
{
	debugLogA("Warning: Ignoring server packet on PING channel");
}

void CIcqProto::StartKeepAlive(serverthread_info *info)
{
	if (getByte("KeepAlive", DEFAULT_KEEPALIVE_ENABLED))
		info->tmPing = time(0) + KEEPALIVE_INTERVAL;
	else
		info->tmPing = -1;
}

void CIcqProto::StopKeepAlive(serverthread_info *info)
{
	info->tmPing = -1;
}

void CIcqProto::CheckKeepAlive(serverthread_info *info)
{
	if (info->tmPing == -1)
		return;

	if (time(0) >= info->tmPing) {
		// Send a keep alive packet to server
		icq_packet packet = { 0 };
		write_flap(&packet, ICQ_PING_CHAN);
		sendServPacket(&packet);

		StartKeepAlive(info);
	}
}
