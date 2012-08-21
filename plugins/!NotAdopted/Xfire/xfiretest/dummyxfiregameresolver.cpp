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
#include "dummyxfiregameresolver.h"
#include "xdebug.h"

namespace xfirelib {

  XFireGame *DummyXFireGameResolver::resolveGame(int gameid, int iterator, BuddyListGamesPacket *packet) {
    XDEBUG2( "Resolving GameId: %d\n", gameid );
    DummyXFireGame *game = new DummyXFireGame();
    switch(gameid) {
    case 2: game->init( gameid, "America's Army: Special Forces", packet,iterator ); break;
    case 3: game->init( gameid, "Unreal Tournament", packet,iterator ); break;
    case 4: game->init( gameid, "Unreal Tournament 2003", packet ,iterator); break;
    case 5: game->init( gameid, "Counter-Strike 1.6", packet ,iterator); break;
    case 32: game->init( gameid, "Teamspeak", packet ,iterator); break;
    case 4181: game->init( gameid, "Unreal Tournament 2004", packet,iterator ); break;
    case 4578: game->init( gameid, "Battlefield 2", packet ,iterator); break;
    case 4611: game->init( gameid, "Live for Speed", packet,iterator ); break;
    default:
      game->init( gameid, "Non Resolvegame", packet,iterator );
      break;
    }
    return game;
  }


};
