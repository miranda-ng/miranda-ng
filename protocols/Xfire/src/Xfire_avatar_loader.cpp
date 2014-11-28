#include "stdafx.h"
#include "Xfire_avatar_loader.h"

Xfire_avatar_loader::Xfire_avatar_loader(xfirelib::Client* client) {
	threadrunning = FALSE;
	this->client = client;
	InitializeCriticalSection(&this->avatarMutex);
}

Xfire_avatar_loader::~Xfire_avatar_loader() {
	//liste leeren, damit der laufende thread abgebrochen wird
	list.clear();
	//warten bis der thread geschlossen wurde
	EnterCriticalSection(&this->avatarMutex);
	LeaveCriticalSection(&this->avatarMutex);
	//critical section entfernen
	DeleteCriticalSection(&this->avatarMutex);
}

void Xfire_avatar_loader::loadThread(LPVOID lparam) {
	Xfire_avatar_loader *loader = (Xfire_avatar_loader*)lparam;

	//kein loader, dann abbruch
	if (!loader)
		return;

	EnterCriticalSection(&loader->avatarMutex);
	loader->threadrunning = TRUE;

	while (1){
		//keinen avatarload auftrag mehr
		if (!loader->list.size())
			break;

		//letzten load process holen
		Xfire_avatar_process process = loader->list.back();

		//buddyinfo abfragen
		GetBuddyInfo buddyinfo;
		buddyinfo.userid = process.userid;
		if (loader->client)
			if (loader->client->connected)
			{
			loader->client->send(&buddyinfo);
			}
			else //nicht mehr verbunden? dann liste leeren und schleife abbrechen
			{
				loader->list.clear();
				break;
			}

		//auftrag entfernen
		loader->list.pop_back();

		Sleep(1000);
	}

	loader->threadrunning = FALSE;
	LeaveCriticalSection(&loader->avatarMutex);

	return;
}

BOOL Xfire_avatar_loader::loadAvatar(MCONTACT hcontact, char*username, unsigned int userid)
{
	Xfire_avatar_process process = { 0 };

	//struktur füllen
	process.hcontact = hcontact;
	if (username)
		strcpy_s(process.username, 128, username);
	process.userid = userid;

	//Avataranfrage an die liste übergeben
	this->list.push_back(process);

	if (!threadrunning && client != NULL) {
		mir_forkthread(Xfire_avatar_loader::loadThread, (LPVOID)this);
	}

	return TRUE;
}