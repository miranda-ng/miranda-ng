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

using namespace std;

namespace xfirelib
{
	PacketReader::PacketReader(Socket *socket)
	{
		m_socket = socket;
		m_packetListeners = new vector<PacketListener *>();

		initPackets();
	}
	void PacketReader::setSocket(Socket *socket)
	{
		m_socket = socket;
	}
	PacketReader::~PacketReader()
	{
		// TODO: delete each packetListener ..
		delete m_packetListeners;

		while (!m_packets->empty()) {
			delete m_packets->at(m_packets->size() - 1);
			m_packets->pop_back();
		}
		delete m_packets;
	}

	void PacketReader::initPackets()
	{
		m_packets = new vector <XFirePacketContent *>();
		m_packets->push_back(new ClientInformationPacket());
		m_packets->push_back(new AuthPacket());
		m_packets->push_back(new LoginFailedPacket());
		m_packets->push_back(new LoginSuccessPacket());
		m_packets->push_back(new MessagePacket());
		m_packets->push_back(new BuddyListOnlinePacket());
		m_packets->push_back(new BuddyListNamesPacket());
		m_packets->push_back(new BuddyListGamesPacket());
		m_packets->push_back(new BuddyListGames2Packet());
		m_packets->push_back(new OtherLoginPacket());
		m_packets->push_back(new InviteBuddyPacket());
		m_packets->push_back(new InviteRequestPacket());
		m_packets->push_back(new RecvRemoveBuddyPacket());
		m_packets->push_back(new RecvDidPacket());
		m_packets->push_back(new RecvStatusMessagePacket());
		m_packets->push_back(new RecvOldVersionPacket());
		m_packets->push_back(new RecvPrefsPacket());
		//neue packetklassen hinzugefügt - dufte
		m_packets->push_back(new FriendsBuddyListNamesPacket());
		m_packets->push_back(new ClanBuddyListNamesPacket());
		m_packets->push_back(new XFireClanPacket());
		m_packets->push_back(new GameInfoPacket());
		m_packets->push_back(new ClanInvitationPacket());
		m_packets->push_back(new XFireFoundBuddys());
		m_packets->push_back(new BuddyInfoPacket());
		m_packets->push_back(new RecvBuddyChangedNick());
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

	void PacketReader::run()
	{
		// start receiving on socket...
		XDEBUG(("Starting run() method... \n"));
		while (socket != NULL) {
			string str;
			//int b = socket->recv(str);
			XFirePacket *packet = new XFirePacket(this);
			XDEBUG(("Waiting for next packet... \n"));
			if (packet == NULL) continue;
			packet->recvPacket(m_socket);
			XINFO(("Received packet\n"));
			if (packet->getContent() != NULL) {
				fireListeners(packet);
			}
			else {
				XDEBUG(("Packet Content was NULL ... Unknown Packet Id ??\n"));
			}
			XDEBUG(("Notified Listeners\n"));// << b << "bytes: " << str << endl;
			delete packet->getContent();
			delete packet;
		}
	}

	void PacketReader::fireListeners(XFirePacket *packet)
	{
		for (vector<PacketListener *>::iterator it = m_packetListeners->begin();
			it != m_packetListeners->end(); ++it) {
			(*it)->receivedPacket(packet);
		}
	}

	XFirePacketContent *PacketReader::getPacketContentClass(int packetId)
	{
		XDEBUG(("Searching for a content class...\n"));
		for (uint i = 0; i < m_packets->size(); i++)
			if (m_packets->at(i)->getPacketId() == packetId) return m_packets->at(i);
		XDEBUG(("None Found\n"));
		return NULL;
	}

	void PacketReader::addPacketListener(PacketListener *listener)
	{
		m_packetListeners->push_back(listener);
	}
};
