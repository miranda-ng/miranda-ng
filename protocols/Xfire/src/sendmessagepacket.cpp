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

#include "sendmessagepacket.h"
#include "xfireutils.h"
#include <string.h>
#include <iostream>
#include "xdebug.h"

#include "sendmessagepacket.h"
#include "xfireutils.h"
#include <string.h>
#include <iostream>
#include "xdebug.h"

namespace xfirelib {
  using namespace std;

  std::map<std::string,int> SendMessagePacket::imindexes;

  void SendMessagePacket::init(Client *client, string username, string message) {
    BuddyListEntry *entry = client->getBuddyList()->getBuddyByName(username);
    if (entry) {
      setSid(entry->sid);
    }
    this->message = message;
    initIMIndex();
  }

  void SendMessagePacket::initIMIndex() {
    string str_sid(sid);
    if ( imindexes.count( str_sid ) < 1 )
      imindex = imindexes[str_sid] = 1;
    else
      imindex = ++imindexes[str_sid];  
  }

  void SendMessagePacket::setSid(const char *sid) {
    memcpy(this->sid,sid,16);
  }

  int SendMessagePacket::getPacketContent(char *buf) {
    if ( imindex == 0 ) initIMIndex();

    int index = 0;
    VariableValue val;
    val.setName("sid");
    val.setValue(sid,16);

    index += val.writeName(buf,index);
    buf[index++] = 3;
    index += val.writeValue(buf,index);

    val.setName("peermsg");
    index += val.writeName(buf,index);
    buf[index++] = 5;
    //buf[index++] = 7;
    buf[index++] = 3;

    val.setName("msgtype");
    val.setValueFromLong(0,4);
    index += val.writeName(buf,index);
    buf[index++] = 2;
    index += val.writeValue(buf,index);

    val.setName("imindex");
    val.setValueFromLong(imindex,4);
    index += val.writeName(buf,index);
    buf[index++] = 02;
    index += val.writeValue(buf,index);

    val.setName("im");
    val.setValue((char*)message.c_str(),message.size());
    index += val.writeName(buf,index);
    buf[index++] = 01;
    buf[index++] = message.size()%256;
    buf[index++] = (int)message.size()/256;
    index += val.writeValue(buf,index);

    return index;
  }
}
