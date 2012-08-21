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



#ifndef __CLIENTVERSIONPACKET_H
#define __CLIENTVERSIONPACKET_H

#include "xfiresendpacketcontent.h"

namespace xfirelib {

  /**
   * (Internal) Packet used to send client version information.
   * it is sent in Client::connect( std::string, std::string )
   * so users of xfirelib won't need this packet.
   */
  class ClientVersionPacket : public XFireSendPacketContent {
  public:
    XFirePacketContent* newPacket() { return new ClientVersionPacket(); }

    int getPacketId() { return 3; }
    int getPacketContent(char *buf);
    int getPacketAttributeCount();
    int getPacketSize() { return 1024; };
    void setProtocolVersion(int version);
    void parseContent(char *buf, int length, int numberOfAtts) { };
  private:
    int length;
    int version;
  };

};

#endif
