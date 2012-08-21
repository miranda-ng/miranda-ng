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

#ifndef __DUMMYXFIREGAMERESOLVER_H
#define __DUMMYXFIREGAMERESOLVER_H

#include "xfiregame.h"
#include "xfiregameresolver.h"
#include "xdebug.h"
#include <string>

namespace xfirelib {

  struct DummyXFireGame;
  class DummyXFireGameResolver : public XFireGameResolver {
  public:
    XFireGame *resolveGame(int gameid, int iterator, BuddyListGamesPacket *packet);

  };


  class DummyXFireGame : public XFireGame {
  public:
    // TODO !!!!!!!!!!!!! this->packet = packet should NOT be used !!
    // we need to create a copy of packet, because it will be deleted as soon
    // as all listeners are notified !!!
    void init(int gameid, std::string gamename, BuddyListGamesPacket *packet, int iterator) {
      this->gameid = gameid;
      this->gamename = gamename;
      this->packet = packet;
        
      memcpy(ip,packet->ips->at(iterator),4);
      port = packet->ports->at(iterator);
      XDEBUG(( "init .. %d / %s\n", gameid, gamename.c_str() ));
    }

    int getGameId() { return gameid; }
    std::string getGameName() { return gamename; }

    BuddyListGamesPacket *packet;
    int gameid;
    char ip[4];
    long port;
    std::string gamename;
  };

};


#endif
