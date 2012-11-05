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



#ifndef __SENDMESSAGEPACKET_H
#define __SENDMESSAGEPACKET_H



#include "xfiresendpacketcontent.h"
#include "variablevalue.h"
#include <string.h>
#include "client.h"

namespace xfirelib {

  class SendMessagePacket : public XFireSendPacketContent {
  public:
    SendMessagePacket() {
      imindex = 0;
    }
    virtual ~SendMessagePacket() { }

    void init(Client *client, string username, string message);
    void setSid(const char *sid);

    XFirePacketContent* newPacket() { return new SendMessagePacket(); }

    int getPacketId() { return 2; }
    int getPacketContent(char *buf);
    int getPacketAttributeCount() {return 2;};
    int getPacketSize() { return 5024; };

    /**
     * SID of the user to who the message should be sent.
     */
    char sid[16];
    /**
     * A running counter for each buddy. (will be initialized to 0 by default.. and.. 
     * shouldn't be a problem to leave it 0)
     */
    long imindex;
    /**
     * Message body to be sent.
     */
    std::string message;

  protected:
    void initIMIndex();

    static std::map<std::string,int> imindexes;
    
  };

};

#endif
