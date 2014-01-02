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

#include "packetreader.h"

#include <iostream>
#include "xfirepacket.h"

#include "clientinformationpacket.h"
#include "authpacket.h"
#include "loginfailedpacket.h"
#include "loginsuccesspacket.h"
#include "buddylistonlinepacket.h"
#include "buddylistnamespacket.h"
#include "clanbuddylistnamespacket.h"
#include "xfireclanpacket.h"
#include "buddylistgamespacket.h"
#include "buddylistgames2packet.h"
#include "messagepacket.h"
#include "otherloginpacket.h"
#include "invitebuddypacket.h"
#include "inviterequestpacket.h"
#include "recvremovebuddypacket.h"
#include "recvstatusmessagepacket.h"
#include "recvoldversionpacket.h"
#include "recvdidpacket.h"
#include "recvprefspacket.h"
#include "gameinfopacket.h"
#include "claninvitationpacket.h"
#include "xfirefoundbuddys.h"
#include "buddyinfo.h"
#include "friendsoffriendlist.h"
#include "recvbuddychangednick.h"

#include "xdebug.h"
//#include "packetlistener.h"

namespace xfirelib {
  using namespace std;

  PacketReader::PacketReader(Socket *socket) {
    this->socket = socket;
    this->packetListeners = new vector<PacketListener *>();

    initPackets();
  }
  void PacketReader::setSocket(Socket *socket) {
    this->socket = socket;
  }
  PacketReader::~PacketReader() {
    // TODO: delete each packetListener ..
    delete packetListeners;

    while(!packets->empty()) { delete packets->at(packets->size()-1); packets->pop_back(); }
    delete packets;
  }

  void PacketReader::initPackets() {
    packets = new vector <XFirePacketContent *>();
    packets->push_back( new ClientInformationPacket() );
    packets->push_back( new AuthPacket() );
    packets->push_back( new LoginFailedPacket() );
    packets->push_back( new LoginSuccessPacket() );
    packets->push_back( new MessagePacket() );
    packets->push_back( new BuddyListOnlinePacket() );
    packets->push_back( new BuddyListNamesPacket() );
    packets->push_back( new BuddyListGamesPacket() );
    packets->push_back( new BuddyListGames2Packet() );
    packets->push_back( new OtherLoginPacket() );
    packets->push_back( new InviteBuddyPacket() );
    packets->push_back( new InviteRequestPacket() );
    packets->push_back( new RecvRemoveBuddyPacket() );
    packets->push_back( new RecvDidPacket() );
    packets->push_back( new RecvStatusMessagePacket() );
    packets->push_back( new RecvOldVersionPacket() );
    packets->push_back( new RecvPrefsPacket() );
	//neue packetklassen hinzugefügt - dufte
	packets->push_back( new FriendsBuddyListNamesPacket() );
	packets->push_back( new ClanBuddyListNamesPacket() );
	packets->push_back( new XFireClanPacket() );
	packets->push_back( new GameInfoPacket() );
	packets->push_back( new ClanInvitationPacket() );
	packets->push_back( new XFireFoundBuddys() );
	packets->push_back( new BuddyInfoPacket() );
	packets->push_back( new RecvBuddyChangedNick() );
  }


  void *muh(void *ptr);

  /* I moved thread starting to Client

  void PacketReader::startListening() {
    PacketReader *myself = this;
    void* (*func)(void*) = &xfirelib::PacketReader::thread_start;
    XINFO(("About to start thread\n"));
    int ret = pthread_create( &readthread, NULL, func, (void*)myself );
    XDEBUG(("ret: %d\n",ret));
  }
  */

  void PacketReader::run() {
    // start receiving on socket...
    XDEBUG(("Starting run() method... \n"));
    while(socket != NULL) {
      string str;
      //int b = socket->recv(str);
      XFirePacket *packet = new XFirePacket(this);
      XDEBUG(("Waiting for next packet... \n"));
	  if (packet==NULL) continue;
      packet->recvPacket( socket );
      XINFO(("Received packet\n"));
      if (packet->getContent() != NULL) {
	fireListeners( packet );
      } else {
	XDEBUG(("Packet Content was NULL ... Unknown Packet Id ??\n"));
      }
      XDEBUG(("Notified Listeners\n"));// << b << "bytes: " << str << endl;
      delete packet->getContent();
      delete packet;
    }
  }

  void PacketReader::fireListeners( XFirePacket *packet ) {
    for(vector<PacketListener *>::iterator it = packetListeners->begin() ;
	it != packetListeners->end() ; ++it) {
      (*it)->receivedPacket( packet );
    }
  }

  XFirePacketContent *PacketReader::getPacketContentClass(int packetId) {
    XDEBUG(("Searching for a content class...\n"));
    for(uint i = 0 ; i < packets->size() ; i++)
      if (packets->at(i)->getPacketId() == packetId) return packets->at(i);
    XDEBUG(("None Found\n"));
    return NULL;
  }

  void PacketReader::addPacketListener( PacketListener *listener ) {
    packetListeners->push_back( listener );
  }
};
