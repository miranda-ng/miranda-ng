/*
 *  xfirelib - C++ Library for the xfire protocol.
 *  Copyright (C) 2006 by
 *          Beat Wolf <asraniel@fryx.ch> / http://gfire.sf.net
 *          Herbert Poul <herbert.poul@gmail.com> / http://goim.us
 *			dufte <dufte@justmail.de>
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

#ifndef __RECVBUDDYCHANGEDNICK_H
#define __RECVBUDDYCHANGEDNICK_H

//packet liest neue nicks ein

#include "xfirerecvpacketcontent.h"

#define XFIRE_RECVBUDDYCHANGEDNICK 161

namespace xfirelib {
	class RecvBuddyChangedNick : public XFireRecvPacketContent {
	public:
		virtual ~RecvBuddyChangedNick() { }
		int getPacketId() { return XFIRE_RECVBUDDYCHANGEDNICK; }

		XFirePacketContent *newPacket() { return new RecvBuddyChangedNick(); }
		void parseContent(char *buf, int length, int numberOfAtts);

		long userid;

		std::string newnick;
		void* entry; // handle eingef�gt, damit ich schnell den buddy killen kann - dufte
	};
};

#endif
