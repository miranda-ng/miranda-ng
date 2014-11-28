/*
 *  Plugin of miranda IM(ICQ) for Communicating with users of the XFire Network.
 *
 *  Copyright (C) 2010 by
 *          dufte <dufte@justmail.de>
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
 *
 *
 *  Miranda ICQ: the free icq client for MS Windows
 *  Copyright (C) 2000-2008  Richard Hughes, Roland Rabien & Tristan Van de Vreede
 *
 */

/*
	Klasse soll die Gameliste verwalten. Dazu gehört das dynamische Einfügen/Löschen/Verändern von Games.
	Erstellen/Sortieren der Menüpunkte...
	*/

#ifndef _XFIRE_GAMELIST
#define _XFIRE_GAMELIST

#include "Xfire_game.h"
#include "Xfire_icon_mng.h"
#include <vector>

using namespace std;

class Xfire_gamelist : public Xfire_base {
private:
	vector<Xfire_game*> gamelist;
	HANDLE dummymenuitem;
	unsigned int nextgameid;
	BOOL ingame;
	CRITICAL_SECTION gamlistMutex;

	void removeDummyMenuItem();
	void createDummyMenuItem();
public:
	Xfire_icon_mng iconmngr;

	void readGamelist(int anz);
	void clearGamelist();
	void createStartmenu();
	void clearStartmenu();
	void clearDatabase(BOOL dontaddcustom = FALSE);
	void writeDatabase();
	BOOL Gameinlist(int id, int*dbid = NULL);
	void Addgame(Xfire_game* newgame);
	BOOL Xfire_gamelist::Removegame(int id);
	int countGames();
	~Xfire_gamelist();
	Xfire_gamelist();
	BOOL getnextGame(Xfire_game**currentgame);
	BOOL Ingame(){ return ingame; }
	void SetGameStatus(BOOL ingame);
	int CurrentGame();
	Xfire_game* getGame(unsigned int dbid);
	Xfire_game* getGamebyGameid(unsigned int gameid);
	void Block(BOOL block);
};

#endif