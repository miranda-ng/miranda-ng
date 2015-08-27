#include "stdafx.h"
#include "Xfire_avatar_loader.h"

Xfire_avatar_loader::Xfire_avatar_loader(xfirelib::Client* client)
{
	m_threadrunning = FALSE;
	m_client = client;
}

Xfire_avatar_loader::~Xfire_avatar_loader()
{
	//liste leeren, damit der laufende thread abgebrochen wird
	m_list.clear();
}

void Xfire_avatar_loader::loadThread(void *arg)
{
	Xfire_avatar_loader *loader = (Xfire_avatar_loader*)arg;

	//kein loader, dann abbruch
	if (!loader)
		return;

	mir_cslock lck(loader->m_avatarMutex);
	loader->m_threadrunning = TRUE;

	while (1) {
		//keinen avatarload auftrag mehr
		if (!loader->m_list.size())
			break;

		//letzten load process holen
		Xfire_avatar_process process = loader->m_list.back();

		//buddyinfo abfragen
		GetBuddyInfo buddyinfo;
		buddyinfo.userid = process.m_userid;
		if (loader->m_client)
			if (loader->m_client->m_connected) {
				loader->m_client->send(&buddyinfo);
			}
			else //nicht mehr verbunden? dann liste leeren und schleife abbrechen
			{
				loader->m_list.clear();
				break;
			}

		//auftrag entfernen
		loader->m_list.pop_back();

		Sleep(1000);
	}

	loader->m_threadrunning = FALSE;

	return;
}

BOOL Xfire_avatar_loader::loadAvatar(MCONTACT hcontact, char*username, unsigned int userid)
{
	Xfire_avatar_process process = { 0 };

	//struktur füllen
	process.m_hcontact = hcontact;
	if (username)
		strcpy_s(process.m_username, 128, username);
	process.m_userid = userid;

	//Avataranfrage an die liste übergeben
	this->m_list.push_back(process);

	if (!m_threadrunning && m_client != NULL) {
		mir_forkthread(Xfire_avatar_loader::loadThread, this);
	}

	return TRUE;
}
