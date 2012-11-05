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

#ifndef __XFIREGAMERESOLVER_H
#define __XFIREGAMERESOLVER_H

#include "buddylistgamespacket.h"
#include "xfiregame.h"
#include <string>

namespace xfirelib {
  /**
   * This is a virtual class which users of the library can
   * subclass so they can use their own gameid <-> XFireGame object
   * resolver (Implementors might want to consider that their application
   * also requires backward converting .. from game to gameid since
   * SendGameStatusPacket requires a gameid.
   */
  class XFireGameResolver {
  public:
    virtual ~XFireGameResolver() { }

    /**
     * this method should resolve the gameid 
     * and return a new XFireGame object or NULL if it was not resolvable.
     * (if more information is needed, it can be taken from packet->xxx[iterator])
     */
    virtual XFireGame *resolveGame(int gameid, int iterator, BuddyListGamesPacket *packet) = 0;
  };

};

#endif
