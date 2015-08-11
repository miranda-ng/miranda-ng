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

#ifndef __BUDDYLISTGAMESPACKET_H
#define __BUDDYLISTGAMESPACKET_H

#define XFIRE_BUDDYS_GAMES_ID 135

#include "xfirerecvpacketcontent.h"
#include "variablevalue.h"

namespace xfirelib {
	using namespace std;

	class BuddyListGamesPacket : public XFireRecvPacketContent {
	public:
		BuddyListGamesPacket();
		virtual ~BuddyListGamesPacket();

		XFirePacketContent* newPacket() { return new BuddyListGamesPacket(); }

		virtual int getPacketId() { return XFIRE_BUDDYS_GAMES_ID; }
		int getPacketContent(char*) { return 0; }
		int getPacketAttributeCount() { return 0; };
		int getPacketSize() { return 1024; };
		virtual void parseContent(char *buf, int length, int numberOfAtts);

		vector<char *> *ips;
		vector<long> *ports;
		vector<long> *gameids;
		vector<long> *gameids2;
		vector<char *> *sids;
		int type;
	};
};


#endif
