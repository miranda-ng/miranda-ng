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

#ifndef __TEST_H
#define __TEST_H

#include <string>
#include <vector>
#include "../packetlistener.h"
#include "../xfirepacket.h"
#include "../client.h"
#include "../inviterequestpacket.h"

namespace xfirelibtest {
  using namespace std;
  using namespace xfirelib;

  class XFireTestClient : public PacketListener {

  public:
    XFireTestClient(string username, string password);
    ~XFireTestClient();
    void run();

    void receivedPacket(XFirePacket *packet);

  private:
    vector<string> explodeString(string s, string e);
    string joinString(vector<string> s, int startindex, int endindex=-1, string delimiter=" ");
    void printBuddyList();

    Client *client;
    string *lastInviteRequest;

    string username;
    string password;
  };
};


#endif
