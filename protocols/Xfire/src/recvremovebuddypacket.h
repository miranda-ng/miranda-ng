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

#ifndef __RECVREMOVEBUDDYPACKET_H
#define __RECVREMOVEBUDDYPACKET_H

#include "xfirerecvpacketcontent.h"
#include <string>

#define XFIRE_RECVREMOVEBUDDYPACKET 139

namespace xfirelib {
	class RecvRemoveBuddyPacket : public XFireRecvPacketContent {
	public:
		virtual ~RecvRemoveBuddyPacket() { }
		int getPacketId() { return XFIRE_RECVREMOVEBUDDYPACKET; }

		XFirePacketContent *newPacket() { return new RecvRemoveBuddyPacket(); }
		void parseContent(char *buf, int length, int numberOfAtts);

		long userid;

		/**
		* I've added this attribute altough it is not part of the actual packet
		* because by the time the packet content reaches the client
		* application the user will no longer be in the BuddyList .. so no
		* way for the client application to know which buddy was just removed.
		* (it will be set by the BuddyList, not when parsing the packet)
		*/
		std::string username;
		MCONTACT handle; // handle eingefügt, damit ich schnell den buddy killen kann - dufte
	};

};


#endif
