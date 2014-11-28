#include "stdafx.h"

#include "Xfire_gamelist.h"

//liefert bestimmtes game zurück
Xfire_game* Xfire_gamelist::getGame(unsigned int dbid)
{
	if (dbid < gamelist.size())
	{
		return gamelist.at(dbid);
	}
	return NULL;
}

//liefert bestimmtes game zurück
Xfire_game* Xfire_gamelist::getGamebyGameid(unsigned int gameid)
{
	for (unsigned int i = 0; i < gamelist.size(); i++) {
		Xfire_game* game = (Xfire_game*)gamelist.at(i);
		if (game->id == gameid)
			return game;
	}
	return NULL;
}

void Xfire_gamelist::readGamelist(int anz) {
	//spiele einzeln einlesen
	for (int i = 0; i < anz; i++)
	{
		//erzeuge gameobject
		Xfire_game* game = new Xfire_game();

		if (game) {
			//lese das spiel ein
			game->readFromDB(i);
			//icons laden
			Xfire_icon_cache icon = this->iconmngr.getGameIconEntry(game->id);
			game->setIcon(icon.hicon, icon.handle);
			//in die liste einfügen
			gamelist.push_back(game);
		}
	}
}

//setzt den ingamestatus
void Xfire_gamelist::SetGameStatus(BOOL status)
{
	ingame = status;
}

Xfire_gamelist::Xfire_gamelist()
{
	nextgameid = 0;
	ingame = FALSE;
	InitializeCriticalSection(&gamlistMutex);
}

//dekonstruktor
Xfire_gamelist::~Xfire_gamelist() {
	for (unsigned int i = 0; i < gamelist.size(); i++) {
		Xfire_game* game = (Xfire_game*)gamelist.at(i);
		if (game) delete game;
	}
	gamelist.clear();
	DeleteCriticalSection(&gamlistMutex);
}

//hole das nächste game
BOOL Xfire_gamelist::getnextGame(Xfire_game**currentgame)
{
	//kein pointer dann false zurückliefern
	if (currentgame == NULL)
		return FALSE;

	//is die derzeitige id kleiner wie die anzahl an games, dann passendes game zurückliefern
	if (nextgameid < gamelist.size())
	{
		*currentgame = gamelist.at(nextgameid);
		nextgameid++;

		//muss das spiel geskippt werden, dann nochmal funktion aufrufen um das nächste game zubekommen
		if ((*currentgame)->skip || (*currentgame)->id == 32 || (*currentgame)->id == 33 || (*currentgame)->id == 34 || (*currentgame)->id == 35)
		{
			return getnextGame(currentgame);
		}

		return TRUE;
	}
	else
	{
		//liste durchgearbeitet, nextid wieder auf 0 setzen
		nextgameid = 0;
		return FALSE;
	}
}

void Xfire_gamelist::Block(BOOL block)
{
	if (block)
		EnterCriticalSection(&gamlistMutex);
	else
		LeaveCriticalSection(&gamlistMutex);
}

//fügt simple ein gameobject in den vector ein
void Xfire_gamelist::Addgame(Xfire_game* newgame)
{
	//spielicon auslesen
	Xfire_icon_cache icon = this->iconmngr.getGameIconEntry(newgame->id);
	newgame->setIcon(icon.hicon, icon.handle);

	gamelist.push_back(newgame);
}

//entfernt ein game aus der liste
BOOL Xfire_gamelist::Removegame(int id)
{
	std::vector<Xfire_game *>::iterator i = gamelist.begin();
	while (i != gamelist.end()) {
		if ((*i)->id == id) {
			(*i)->remoteMenuitem();
			gamelist.erase(i);
			return TRUE;
		}
		++i;
	}
	return FALSE;
}

//setzt die aktuelle id als currentgame
int Xfire_gamelist::CurrentGame()
{
	this->SetGameStatus(TRUE);
	return nextgameid - 1;
}

//erzeugt das menu der gameliste
void Xfire_gamelist::createStartmenu()
{
	//keine games in der internen liste? abbruch
	if (gamelist.size() == 0)
		return;

	//sortierarray
	int* sorttemp = new int[gamelist.size()];

	//zuordnungen setzen
	for (unsigned int i = 0; i < gamelist.size(); i++)
		sorttemp[i] = i;

	//sortiert mit bubblesortalgo
	BOOL changed = FALSE;
	do
	{
		changed = FALSE;
		for (unsigned int i = 1; i < gamelist.size(); i++)
		{
			Xfire_game* game = (Xfire_game*)gamelist.at(sorttemp[i - 1]);
			Xfire_game* game2 = (Xfire_game*)gamelist.at(sorttemp[i]);
			//sortieren
			if (strcmp(game->name, game2->name) > 0)
			{
				int tempi = sorttemp[i - 1];
				sorttemp[i - 1] = sorttemp[i];
				sorttemp[i] = tempi;
				changed = TRUE;
			}
		}
	} while (changed);

	//menüpunkte anlegen
	for (unsigned int i = 0; i < gamelist.size(); i++) {
		Xfire_game* game = (Xfire_game*)gamelist.at(sorttemp[i]);
		game->createMenuitem(i, sorttemp[i]);
	}

	//entferne sortarray
	delete[] sorttemp;

	//dummymenu entfernen
	removeDummyMenuItem();
}

//schreibt derzeitige gameliste in die db
void Xfire_gamelist::writeDatabase()
{
	for (unsigned int i = 0; i < gamelist.size(); i++) {
		Xfire_game* game = (Xfire_game*)gamelist.at(i);
		if (game) game->writeToDB(i);
	}

	//anzahl aktualisieren
	db_set_w(NULL, protocolname, "foundgames", gamelist.size());
}

//prüft ob ein game schon in der liste is
BOOL Xfire_gamelist::Gameinlist(int id, int*dbid)
{
	for (unsigned int i = 0; i < gamelist.size(); i++) {
		Xfire_game* game = (Xfire_game*)gamelist.at(i);
		if (game)
		{
			if (game->id == id)
			{
				//soll eine dbid zurückgeliefert werden? dann setzen
				if (dbid)
					*dbid = i;

				return TRUE;
			}
		}
	}
	return FALSE;
}

//gibt die anzahl der spiele zurück
int Xfire_gamelist::countGames()
{
	return gamelist.size();
}

//leert das startmenu
void Xfire_gamelist::clearStartmenu()
{
	//erstelle dummyeintrag, für workaround
	createDummyMenuItem();

	//einzelene menüpunkte entfernen
	for (unsigned int i = 0; i < gamelist.size(); i++) {
		Xfire_game* game = (Xfire_game*)gamelist.at(i);
		if (game) game->remoteMenuitem();
	}
}

//dekonstruktor
void Xfire_gamelist::clearGamelist() {
	createDummyMenuItem();
	for (unsigned int i = 0; i < gamelist.size(); i++) {
		Xfire_game* game = (Xfire_game*)gamelist.at(i);
		if (game) delete game;
	}
	gamelist.clear();
}

//erstellt ein dummyeintrag
void Xfire_gamelist::createDummyMenuItem()
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 500090001;
	mi.pszName = Translate("Please wait...");
	mi.hIcon = LoadIcon(hinstance, MAKEINTRESOURCE(ID_OP));
	mi.popupPosition = 500084000;
	mi.pszPopupName = Translate("Start game");
	mi.pszContactOwner = protocolname;
	dummymenuitem = Menu_AddMainMenuItem(&mi);
}

//entfernt dummymenueintrag
void Xfire_gamelist::removeDummyMenuItem()
{
	CallService(MO_REMOVEMENUITEM, (WPARAM)dummymenuitem, 0);
}

//säubert die datenbank spiel einträge udn trägt custom spiele vorher nach
void Xfire_gamelist::clearDatabase(BOOL dontaddcustom)
{
	BOOL somethingfound = TRUE;
	int i3 = 0;

	//db durchsuchen
	while (somethingfound) {
		somethingfound = FALSE;
		//customeintrag? dann ab in die gameliste damit
		if (this->readBytefromDB("gamecustom", i3) == 1)
		{
			//customgame nur in die liste adden wenn es gewollt is
			if (!dontaddcustom)
			{
				//erzeuge gameobject
				Xfire_game* game = new Xfire_game();
				//lese das spiel ein
				game->readFromDB(i3);
				//spielicon auslesen
				/*Xfire_icon_cache icon=this->iconmngr.getGameIconEntry(game->id);
				game->setIcon(icon.hicon,icon.handle);
				//in die liste einfügen
				gamelist.push_back(game);*/
				this->Addgame(game);
			}
		}

		if (this->removeDBEntry("gamecustom", i3)) somethingfound = TRUE;
		if (this->removeDBEntry("gameid", i3)) somethingfound = TRUE;
		if (this->removeDBEntry("gamepath", i3)) somethingfound = TRUE;
		if (this->removeDBEntry("gamepwargs", i3)) somethingfound = TRUE;
		if (this->removeDBEntry("gameuserargs", i3)) somethingfound = TRUE;
		if (this->removeDBEntry("gamecmdline", i3)) somethingfound = TRUE;
		if (this->removeDBEntry("gamencmdline", i3)) somethingfound = TRUE;
		if (this->removeDBEntry("gamelaunch", i3)) somethingfound = TRUE;
		if (this->removeDBEntry("gamenetargs", i3)) somethingfound = TRUE;
		if (this->removeDBEntry("gamemulti", i3)) somethingfound = TRUE;
		if (this->removeDBEntry("gamesendid", i3)) somethingfound = TRUE;
		if (this->removeDBEntry("gamesetsmsg", i3)) somethingfound = TRUE;

		for (int i2 = 0; i2 < 10; i2++)
			if (this->removeDBEntry("gamepath", i3, i2)) somethingfound = TRUE;
		i3++;
	}
}