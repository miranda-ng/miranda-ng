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

#ifndef __BUDDYLISTONLINEPACKET_H
#define __BUDDYLISTONLINEPACKET_H

#include "xfirerecvpacketcontent.h"
#include "variablevalue.h"


#define XFIRE_BUDDYS_ONLINE_ID 132

namespace xfirelib {
	using namespace std;
	class BuddyListOnlinePacket : public XFireRecvPacketContent {
	public:
		XFirePacketContent* newPacket() { return new BuddyListOnlinePacket(); }


		int getPacketId() { return XFIRE_BUDDYS_ONLINE_ID; }
		int getPacketContent(char*) { return 0; }
		int getPacketAttributeCount() { return 0; };
		int getPacketSize() { return 1024; };
		void parseContent(char *buf, int length, int numberOfAtts);

		vector<long> *userids;
		vector<char *> *sids;
	};
};


#endif
