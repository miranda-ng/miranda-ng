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

#include "socket.h"

#ifndef __PACKETREADER_H
#define __PACKETREADER_H

#include <vector>
#include "xfirepacketcontent.h"
#include "packetlistener.h"

namespace xfirelib {
  struct PacketListener;
  struct XFirePacket;

class PacketReader {
 public:
  PacketReader(Socket *socket);
  ~PacketReader();

  void setSocket(Socket *socket);
  void startListening();
  XFirePacketContent *getPacketContentClass(int packetId);
  void addPacketListener( PacketListener *listener );
  void run();
 private:
  void initPackets();
  void fireListeners( XFirePacket *packet );

  Socket *socket;
  std::vector <XFirePacketContent *> *packets;
  std::vector <PacketListener *> *packetListeners;
};

};

#endif
