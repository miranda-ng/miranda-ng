/*
 *  xfirelib - C++ Library for the xfire protocol.
 *  Copyright (C) 2006 by
 *          Beat Wolf <asraniel@fryx.ch> / http://gfire.sf.net
 *          Herbert Poul <herbert.poul@gmail.com> / http://goim.us
 *    http://xfirelib.sphene.net
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "stdafx.h"

#include "buddylistgames2packet.h"
#include "xdebug.h"

namespace xfirelib
{
	BuddyListGames2Packet::BuddyListGames2Packet() : BuddyListGamesPacket()
	{
	}

	void BuddyListGames2Packet::parseContent(char *buf, int length, int numberOfAtts)
	{
		XDEBUG("....Parsing Packet Content of game 2 ??\n")
			this->BuddyListGamesPacket::parseContent(buf, length, numberOfAtts);
		XDEBUG2("Parsed Packet Content of game 2 ??? ( %ld )\n", gameids->at(0))
	}
};

