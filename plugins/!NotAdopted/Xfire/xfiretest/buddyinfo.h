/*
 *  xfirelib - C++ Library for the xfire protocol.
 *  Copyright (C) 2006 by
 *          Beat Wolf <asraniel@fryx.ch> / http://gfire.sf.net
 *          Herbert Poul <herbert.poul@gmail.com> / http://goim.us
 *    http://xfirelib.sphene.net
 *			dufte <dufte@justmail.de>
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

//packet wird empfangen, nachdem man buddyinfos angefordert hat

#ifndef __BUDDYINFO_H
#define __BUDDYINFO_H

#include "xfirerecvpacketcontent.h"
#include <string>

#define XFIRE_BUDDYINFO 0xAE

namespace xfirelib {
  class BuddyInfoPacket : public XFireRecvPacketContent {
  public:
    virtual ~BuddyInfoPacket() { }
    int getPacketId() { return XFIRE_BUDDYINFO; }

    XFirePacketContent *newPacket() { return new BuddyInfoPacket(); }
    void parseContent(char *buf, int length, int numberOfAtts);

	unsigned char avatarmode;
	unsigned int avatarid;
	unsigned int userid;

  };

};


#endif
