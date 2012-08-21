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



#ifndef __CLIENTLOGINPACKET_H
#define __CLIENTLOGINPACKET_H



#include "xfiresendpacketcontent.h"
#include "variablevalue.h"
#include <string.h>

namespace xfirelib {

  /**
   * (Internal) Packet used to send login/authorization data.
   * Users of xfirelib should not use it, call 
   * Client::connect( std::string username, std::string password )
   * instead.
   */
  class ClientLoginPacket : public XFireSendPacketContent {
  public:
    XFirePacketContent* newPacket() { return new ClientLoginPacket(); }

    int getPacketId() { return 1; }
    int getPacketContent(char *buf);
    int getPacketAttributeCount();
    int getPacketSize() { return 1024; };
    void setUsername(std::string name) {this->name = name;}
    void setPassword(std::string password) {this->password = password; };

    /**
     *Set the salt the server sent us to crypt the password
     *@param salt The VariableValue object that we extracted from the packet
     */
    void setSalt(VariableValue *salt) {this->salt = salt; };

    void parseContent(char *buf, int length, int numberOfAtts) { };
  private:
    void cryptPassword(unsigned char *crypt);
    void hashSha1(const char *string, unsigned char *sha);
    int length;
    std::string name;
    std::string password;
    VariableValue *salt;
  };

};

#endif
