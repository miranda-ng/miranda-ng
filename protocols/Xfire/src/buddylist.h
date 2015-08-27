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

#ifndef __BUDDYLIST_H
#define __BUDDYLIST_H

#include "buddylistnamespacket.h"
#include "clanbuddylistnamespacket.h"
#include "buddylistgamespacket.h"
#include "buddylistonlinepacket.h"
#include "packetlistener.h"
#include "client.h"
#include "xfiregame.h"
#include "friendsoffriendlist.h"


namespace xfirelib {
	using namespace std;

	struct Client;
	struct BuddyListEntry;

	class BuddyList : public PacketListener {
	public:
		BuddyList(Client *client);
		~BuddyList();

		vector <BuddyListEntry *> * getEntries() { return m_entries; }
		vector <BuddyListEntry *> * getEntriesClan() { return m_entriesClan; }

		void receivedPacket(XFirePacket *packet);
		BuddyListEntry *getBuddyById(long userid);
		BuddyListEntry *getBuddyBySid(const char *sid);
		BuddyListEntry *getBuddyByName(string username);
	private:
		void initEntries(BuddyListNamesPacket* buddynames);
		void initEntriesClan(ClanBuddyListNamesPacket* buddynames);
		void updateOnlineBuddies(BuddyListOnlinePacket* buddiesOnline);
		void updateBuddiesGame(BuddyListGamesPacket* buddiesGames);
		void updateFriendsofFriend(FriendsBuddyListNamesPacket* friends);

		Client *m_client;
		vector <BuddyListEntry *> *m_entries;
		vector <BuddyListEntry *> *m_entriesClan;
	};


	class BuddyListEntry {
	public:
		BuddyListEntry();
		~BuddyListEntry();
		bool isOnline();

		void setSid(const char *sid);

		long m_userid;
		char m_sid[16];
		string m_nick;
		string m_username;
		string m_statusmsg;
		string m_gameinfo;
		long m_game;
		long m_game2;
		MCONTACT m_hcontact;
		int m_clanid;

		//lastpopup
		char* m_lastpopup;

		XFireGame *m_gameObj;
		XFireGame *m_game2Obj;
	};

	typedef BuddyListEntry *PBuddyListEntry;
};


#endif
