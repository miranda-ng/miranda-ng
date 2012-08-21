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

#ifndef __RECVMESSAGEPACKET_H
#define __RECVMESSAGEPACKET_H

#define XFIRE_RECV_STATUSMESSAGE_PACKET_ID 154

#include "xfirerecvpacketcontent.h"
#include "buddylist.h"
#include <string>
#include <vector>

namespace xfirelib {

  class RecvStatusMessagePacket : public XFireRecvPacketContent {
  public:
    RecvStatusMessagePacket();
    virtual ~RecvStatusMessagePacket() { }
    XFirePacketContent *newPacket() { return new RecvStatusMessagePacket; }
    int getPacketId() { return XFIRE_RECV_STATUSMESSAGE_PACKET_ID; }

    void parseContent(char *buf, int length, int numberOfAtts);
	char * getSid(int i){ return sids->at(i); }

    std::vector<char *> *sids;
    std::vector<std::string> *msgs;
	BuddyListEntry** entries;
	int centries;

  private:
    int readStrings(std::vector<std::string> *strings, char *buf, int index);
  };

};


#endif
