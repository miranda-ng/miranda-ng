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
#include "invitebuddypacket.h"
#include "variablevalue.h"

namespace xfirelib {
  using namespace std;

  InviteBuddyPacket::InviteBuddyPacket() {
    names = new vector<string>;
    msgs = new vector<string>;
  }
  InviteBuddyPacket::~InviteBuddyPacket() {
    delete names;
    delete msgs;
  }

  void InviteBuddyPacket::parseContent(char *buf, int length, int numberOfAtts) {
    /* Useless.. we only receive Packet 138 for invitations
    VariableValue val;
    int index = 0;
    index += val.readName( buf, index );
    int namecount = (unsigned char)buf[index++];
    for(int i = 0 ; i < namecount ; i++) {
      int namelen = (unsigned char)buf[index++];
      index++; // ignore 00
      index += val.readValue( buf, index, namelen );
      this->msg->push_back(string(val.getValue(),val.getValueLength()));
    }
    index += val.readName( buf, index );
    
    int msgcount = (unsigned char)buf[index++];
    for(int i = 0 ; i < msgcount ; i++) {
      int msglen = (unsigned char)buf[index++];
      index++; // ignore 00
      index += val.readValue(buf,index,msglen);
      this->msg->push_back(string(val.getValue(),val.getValueLength()));
    }
    */
  }
  int InviteBuddyPacket::getPacketContent(char *buf) {
    int index = 0;
    VariableValue val;
    val.setName("name");
    index += val.writeName(buf,index);
    buf[index++] = names->size();
    for(uint i = 0 ; i < names->size() ; i++) {
      string name = names->at(i);
      buf[index++] = name.length();
      buf[index++] = 0;
      memcpy(buf+index,name.c_str(),name.length());
      index+=name.length();
    }
    val.setName("msg");
    index += val.writeName(buf,index);
    buf[index++] = msgs->size();
    for(uint i = 0 ; i < msgs->size() ; i++) {
      string msg = msgs->at(i);
      buf[index++] = msg.length();
      buf[index++] = 0;
      memcpy(buf+index,msg.c_str(),msg.length());
      index+=msg.length();
    }
    return index;
  }

  void InviteBuddyPacket::addInviteName(string name, string msg) {
    this->names->push_back(name);
    this->msgs->push_back(msg);
  }


};
