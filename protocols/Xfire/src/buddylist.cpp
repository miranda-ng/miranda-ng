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

#include "buddylist.h"
#include "buddylistonlinepacket.h"
#include "buddylistgamespacket.h"
#include "buddylistgames2packet.h"
#include "recvremovebuddypacket.h"
#include "recvstatusmessagepacket.h"
#include "recvbuddychangednick.h"
#include "xdebug.h"

//buddylist verarbeitung von der main in buddylist verschoben, um doppelte schleifenverarbeitung zuunterbinden
#include "baseProtocol.h"

extern MCONTACT handlingBuddys(xfirelib::BuddyListEntry *entry, int clan=0,char* group=NULL,BOOL dontscan=FALSE);
extern void setBuddyStatusMsg(xfirelib::BuddyListEntry *entry);

namespace xfirelib
{

	using namespace std;

	BuddyList::BuddyList(Client *client)
	{
		m_entries = new vector<BuddyListEntry *>;

		//clan entries initialisieren - dufte
		m_entriesClan = new vector<BuddyListEntry *>;

		m_client = client;
		m_client->addPacketListener(this);
	}
	BuddyList::~BuddyList()
	{
		for (vector<BuddyListEntry *>::iterator it = m_entries->begin();
			it != m_entries->end(); it++) {
			delete *it;
		}
		delete m_entries;

		//clan entries entfernen - dufte
		for (vector<BuddyListEntry *>::iterator it = m_entriesClan->begin();
			it != m_entriesClan->end(); it++) {
			delete *it;
		}
		delete m_entriesClan;
	}

	BuddyListEntry *BuddyList::getBuddyById(long userid)
	{
		for (uint i = 0; i < m_entries->size(); i++) {
			BuddyListEntry *entry = m_entries->at(i);
			if (entry->m_userid == userid)
				return entry;
		}

		//clan entries durchsuchen - dufte
		for (uint i = 0; i < m_entriesClan->size(); i++) {
			BuddyListEntry *entry = m_entriesClan->at(i);
			if (entry->m_userid == userid)
				return entry;
		}

		return 0;
	}

	BuddyListEntry *BuddyList::getBuddyByName(string username)
	{
		for (uint i = 0; i < m_entries->size(); i++) {
			BuddyListEntry *entry = m_entries->at(i);
			if (entry->m_username == username)
				return entry;
		}

		//clan entries durchsuchen - dufte
		for (uint i = 0; i < m_entriesClan->size(); i++) {
			BuddyListEntry *entry = m_entriesClan->at(i);
			if (entry->m_username == username)
				return entry;
		}

		return 0;
	}

	BuddyListEntry *BuddyList::getBuddyBySid(const char *sid)
	{
		for (uint i = 0; i < m_entries->size(); i++) {
			BuddyListEntry *entry = m_entries->at(i);
			if (memcmp((void *)sid, (void *)entry->m_sid, 16) == 0)
				return entry;

		}

		//clan entries durchsuchen - dufte
		for (uint i = 0; i < m_entriesClan->size(); i++) {
			BuddyListEntry *entry = m_entriesClan->at(i);
			if (memcmp((void *)sid, (void *)entry->m_sid, 16) == 0)
				return entry;

		}

		return 0;
	}

	void BuddyList::initEntries(BuddyListNamesPacket *buddyNames)
	{
		for (uint i = 0; i < buddyNames->usernames->size(); i++) {
			BuddyListEntry *entry = new BuddyListEntry;
			entry->m_username = buddyNames->usernames->at(i);
			entry->m_userid = buddyNames->userids->at(i);
			entry->m_nick = buddyNames->nicks->at(i);
			//buddies in miranda verarbeiten
			handlingBuddys(entry, 0, NULL);
			m_entries->push_back(entry);
		}
	}

	// funktion zum initialisieren der clanliste - dufte
	void BuddyList::initEntriesClan(ClanBuddyListNamesPacket *buddyNames)
	{
		for (uint i = 0; i < buddyNames->usernames->size(); i++) {
			BuddyListEntry *entry = new BuddyListEntry;
			entry->m_username = buddyNames->usernames->at(i);
			entry->m_userid = buddyNames->userids->at(i);
			entry->m_nick = buddyNames->nicks->at(i);
			entry->m_clanid = buddyNames->clanid;
			/* ## buddies im miranda verarbietn */
			char temp[255];
			char * dummy;
			mir_snprintf(temp, "Clan_%d", entry->m_clanid);

			DBVARIANT dbv;
			if (!db_get(NULL, protocolname, temp, &dbv))
				dummy = dbv.pszVal;
			else
				dummy = NULL;

			handlingBuddys(entry, entry->m_clanid, dummy);
			m_entriesClan->push_back(entry);
		}
	}

	void BuddyList::updateFriendsofFriend(FriendsBuddyListNamesPacket* friends)
	{
		for (uint i = 0; i < friends->sids->size(); i++) {
			BuddyListEntry *entry = getBuddyBySid(friends->sids->at(i));
			if (entry) {
				XDEBUG2("Friends of Friend %s!\n", friends->usernames->at(i).c_str());
				entry->m_nick = friends->nicks->at(i);
				entry->m_username = friends->usernames->at(i);
				entry->m_userid = friends->userids->at(i);
			}
			else XERROR(("updateFriendsofFriendBuddies: sid not found!\n"));
		}
	}

	void BuddyList::updateOnlineBuddies(BuddyListOnlinePacket* buddiesOnline)
	{
		for (uint i = 0; i < buddiesOnline->userids->size(); i++) {
			BuddyListEntry *entry = getBuddyById(buddiesOnline->userids->at(i));
			if (entry) {
				entry->setSid(buddiesOnline->sids->at(i));
				//buddies in miranda verarbeiten
				handlingBuddys(entry, 0, NULL);
			}
			else XERROR(("updateOnlineBuddies: Could not find buddy with this sid!\n"));
		}
	}

	void BuddyList::updateBuddiesGame(BuddyListGamesPacket* buddiesGames)
	{
		bool isFirst = buddiesGames->getPacketId() == XFIRE_BUDDYS_GAMES_ID;
		for (uint i = 0; i < buddiesGames->sids->size(); i++) {
			BuddyListEntry *entry = getBuddyBySid(buddiesGames->sids->at(i));
			if (!entry) {
				//nicht zuordbare sids zuordnen
				XERROR("Add dummy Contact in buddylist for friends of friends!\n");
				BuddyListEntry *newentry = new BuddyListEntry;
				newentry->m_username = "";
				newentry->m_userid = 0;
				newentry->m_nick = "";
				newentry->setSid(buddiesGames->sids->at(i));
				m_entries->push_back(newentry);
				//nochmal entry suchen
				entry = newentry; //getBuddyBySid( buddiesGames->sids->at(i) );
			}
			if (entry) {
				if (isFirst) {
					entry->m_game = buddiesGames->gameids->at(i);
					delete entry->m_gameObj; entry->m_gameObj = NULL;
				}
				else {
					entry->m_game2 = buddiesGames->gameids->at(i);
					delete entry->m_game2Obj; entry->m_game2Obj = NULL;
				}
				XDEBUG(("Resolving Game... \n"));
				XFireGameResolver *resolver = m_client->getGameResolver();
				if (resolver) {
					XDEBUG(("Resolving Game... \n"));
					if (isFirst)
						entry->m_gameObj = resolver->resolveGame(entry->m_game, i, buddiesGames);
					else
						entry->m_game2Obj = resolver->resolveGame(entry->m_game2, i, buddiesGames);
				}
				else {
					XDEBUG(("No GameResolver ? :(\n"));
				}
				XDEBUG(("%s: Game (%ld): %s / Game2 (%ld): %s\n",
					entry->username.c_str(),
					entry->game,
					(entry->gameObj == NULL ? "UNKNOWN" : entry->gameObj->getGameName().c_str()),
					entry->game2,
					(entry->game2Obj == NULL ? "UNKNOWN" : entry->game2Obj->getGameName().c_str())
					));
			}
			else XERROR("updateBuddiesGame: Could not find buddy with this sid!\n");
		}
	}

	void BuddyList::receivedPacket(XFirePacket *packet)
	{
		XFirePacketContent *content = packet->getContent();
		if (content == 0) return;
		XDEBUG2("hmm... %d\n", content->getPacketId());
		switch (content->getPacketId()) {
		case XFIRE_BUDDYS_NAMES_ID:
			XINFO(("Received Buddy List..\n"));
			initEntries((BuddyListNamesPacket*)content);
			break;

		case XFIRE_CLAN_BUDDYS_NAMES_ID:
			XINFO(("Received Clan Buddy List..\n"));
			initEntriesClan((ClanBuddyListNamesPacket*)content);
			break;
			//neue nicks updaten, dufte
		case XFIRE_RECVBUDDYCHANGEDNICK:
			{
				RecvBuddyChangedNick* recvchangednick = (RecvBuddyChangedNick*)content;
				XINFO(("Received new nick of a buddy..\n"));
				BuddyListEntry* entry = NULL;
				entry = getBuddyById(recvchangednick->userid);
				if (entry) {
					entry->m_nick = recvchangednick->newnick;
					recvchangednick->entry = (void*)entry;
					handlingBuddys(entry, 0, NULL);
				}
			}
			break;

		case XFIRE_BUDDYS_ONLINE_ID:
			XINFO(("Received Buddy Online Packet..\n"));
			updateOnlineBuddies((BuddyListOnlinePacket *)content);
			break;

		case XFIRE_FRIENDS_BUDDYS_NAMES_ID:
			XINFO(("Received Friends of Friend..\n"));
			updateFriendsofFriend((FriendsBuddyListNamesPacket *)content);
			break;

		case XFIRE_BUDDYS_GAMES2_ID:
		case XFIRE_BUDDYS_GAMES_ID:
			XINFO(("Recieved the game a buddy is playing..\n"));
			updateBuddiesGame((BuddyListGamesPacket *)content);
			break;

		case XFIRE_RECVREMOVEBUDDYPACKET:
			{
				RecvRemoveBuddyPacket *p = (RecvRemoveBuddyPacket*)content;
				XDEBUG2("Buddy was removed from contact list (userid: %ld)\n", p->userid);
				std::vector<BuddyListEntry *>::iterator i = m_entries->begin();
				while (i != m_entries->end()) {
					if ((*i)->m_userid == p->userid) {
						BuddyListEntry *buddy = *i;
						XINFO(("%s (%s) was removed from BuddyList.\n", buddy->username.c_str(), buddy->nick.c_str()));
						p->username = buddy->m_username;
						p->handle = buddy->m_hcontact; // handle übergeben - dufte
						m_entries->erase(i);
						break; // we are done.
					}
					++i;
				}
			}
			break;

		case XFIRE_RECV_STATUSMESSAGE_PACKET_ID:
			{
				RecvStatusMessagePacket *status = (RecvStatusMessagePacket*)content;

				//status->entries=new PBuddyListEntry[status->sids->size];

				for (uint i = 0; i < status->sids->size(); i++) {
					BuddyListEntry *entry = getBuddyBySid(status->sids->at(i));
					//status->entries[i]=entry;

					if (entry == NULL) {
						XERROR(("No such Entry - Got StatusMessage from someone who is not in the buddylist ??\n"));
						return;
					}
					else {
						entry->m_statusmsg = status->msgs->at(i);
						setBuddyStatusMsg(entry); //auf eine funktion reduziert, verringert cpuauslastung und beseitigt das
						//das problem der fehlenden statusmsg
					}
				}

				break;
			}
		}
	}

	BuddyListEntry::~BuddyListEntry()
	{
		if (m_lastpopup) {
			delete[] m_lastpopup;
			m_lastpopup = NULL;
		}
	}

	BuddyListEntry::BuddyListEntry()
	{
		memset(m_sid, 0, 16);
		m_statusmsg = std::string();
		m_game = 0;
		m_game2 = 0;
		m_gameObj = NULL;
		m_game2Obj = NULL;
		m_hcontact = NULL;
		m_clanid = 0;
		m_lastpopup = NULL;
	}

	bool BuddyListEntry::isOnline()
	{
		for (int i = 0; i < 16; i++)
			if (m_sid[i])
				return true;

		return false;
	}

	void BuddyListEntry::setSid(const char *sid)
	{
		int s = 0;
		for (int i = 0; i < 16; i++) { //wenn buddy offline geht, seine gameinfos zurücksetzen - dufte
			if (sid[i]) {
				s = 1;
				break;
			}
		}
		if (s) {
			m_statusmsg = std::string();
			m_game = 0;
			m_game2 = 0;
			m_gameObj = NULL;
			m_game2Obj = NULL;
		}
		memcpy(m_sid, sid, 16);
	}
};

