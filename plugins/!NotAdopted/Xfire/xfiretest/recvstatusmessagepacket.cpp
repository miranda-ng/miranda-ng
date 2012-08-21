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

#include "recvstatusmessagepacket.h"

#include "variablevalue.h"
#include "xdebug.h"
#include <vector>
#include <string>

namespace xfirelib {
  RecvStatusMessagePacket::RecvStatusMessagePacket() {
	  centries=0;
  }

  void RecvStatusMessagePacket::parseContent(char *buf, int length, int numberOfAtts) {
    int index = 0;
    int numberOfIds = 0;
    VariableValue val;
    index += val.readName(buf,index);
    index ++; // ignore 03 ??
    index ++; // jump to counter
    numberOfIds = (unsigned char) buf[index];
    index ++; // Ignore 00 ??
    index ++;
    sids = new std::vector<char *>;
     for(int i = 0 ; i < numberOfIds ; i++) {
      index += val.readValue(buf,index,16);
      char *sid = new char[16];
      memcpy(sid,val.getValue(),16);
      sids->push_back(sid);
    }

    index += val.readName(buf,index);
    XDEBUG(( "valname %s\n", val.getName().c_str() ));
    index ++; // Ignore 04 ??
    msgs = new std::vector<std::string>;
    index = readStrings(msgs,buf,index);

  }


int RecvStatusMessagePacket::readStrings(std::vector<std::string> *strings, char *buf, int index) {
    VariableValue friends;
    index += friends.readValue(buf,index);
    index ++; // Ignore 00
    int numberOfStrings = friends.getValueAsLong();
    XDEBUG3( "name: %s numberOfStrings: %d\n", friends.getName().c_str(), numberOfStrings );
    for(int i = 0 ; i < numberOfStrings ; i++) {
      int length = (unsigned char)buf[index++];
      index++;
      index += friends.readValue(buf,index,length);
      std::string stringvalue = std::string(friends.getValue(),length);
      strings->push_back(stringvalue);
      XDEBUG3( "String length: %2d : %s\n", length, stringvalue.c_str() );
    }
    return index;
  }
};
