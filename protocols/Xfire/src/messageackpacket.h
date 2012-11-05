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

#ifndef __MESSAGEACKPACKET_H
#define __MESSAGEACKPACKET_H

#include "xfiresendpacketcontent.h"

#define XFIRE_MESSAGE_ACK_ID 2

namespace xfirelib {

  /**
   * (Internal) Packet used to acknowledge a received message.
   * It is of no use to users of the library because it is already
   * sent by the Client.
   */
  class MessageACKPacket : public XFireSendPacketContent {
  public:
    MessageACKPacket();
    ~MessageACKPacket();

    XFirePacketContent* newPacket() { return new MessageACKPacket(); }


    int getPacketId() { return XFIRE_MESSAGE_ACK_ID; }
    int getPacketContent(char *buf);
    int getPacketAttributeCount() { return 2; }
    int getPacketSize() { return 1024; };
    void parseContent(char *buf, int length, int numberOfAtts);

    char sid[16];
    long imindex;

  private:
  };

};


#endif

