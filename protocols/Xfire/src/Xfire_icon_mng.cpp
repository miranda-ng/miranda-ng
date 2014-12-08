#include "stdafx.h"
#include "Xfire_icon_mng.h"

//liefert den handle eines icons zurück
HANDLE Xfire_icon_mng::getGameIconHandle(unsigned int gameid) {
	Xfire_icon_cache entry = { 0 };

	//icon im cache dann zurückliefern
	if (getIconfromCache(gameid, &entry))
		return entry.handle;

	//ansonsten gameicon laden
	return this->LoadGameIcon(gameid).handle;
}

//liefert den index des icons zurück
unsigned int Xfire_icon_mng::getGameIconId(unsigned int gameid) {
	for (unsigned int i = 0; i < iconcache.size(); i++)
	{
		if (iconcache.at(i).gameid == gameid)
		{
			return i;
		}
	}
	return -1;
}

//gameicon mit hilfe von id zurückliefern
HICON Xfire_icon_mng::getGameIconFromId(unsigned int id) {
	//id nur im bereich zurückliefern
	if (id<0 || id>iconcache.size() - 1)
		return NULL;

	return iconcache.at(id).hicon;
}

//liefert das hicon zurück
HICON Xfire_icon_mng::getGameIcon(unsigned int gameid) {
	Xfire_icon_cache entry = { 0 };

	//icon im cache dann zurückliefern
	if (getIconfromCache(gameid, &entry))
		return entry.hicon;

	return this->LoadGameIcon(gameid).hicon;
}

//liefert den icon eintrag zurück
Xfire_icon_cache Xfire_icon_mng::getGameIconEntry(unsigned int gameid) {
	Xfire_icon_cache entry = { 0 };

	//icon im cache dann zurückliefern
	if (getIconfromCache(gameid, &entry))
		return entry;

	return this->LoadGameIcon(gameid);
}

//sucht nach dem spielicon im cache
BOOL Xfire_icon_mng::getIconfromCache(unsigned int gameid, Xfire_icon_cache* out) {
	//kein ziel, keine prüfung
	if (out == NULL)
		return FALSE;

	for (unsigned int i = 0; i < iconcache.size(); i++)
	{
		if (iconcache.at(i).gameid == gameid)
		{
			*out = iconcache.at(i);
			return TRUE;
		}
	}

	//nicht gefunden
	return FALSE;
}

//dekonstruktor
Xfire_icon_mng::~Xfire_icon_mng() {
	//geladene icons wieder freigeben
	for (unsigned int i = 0; i < iconcache.size(); i++)
	{
		if (iconcache.at(i).hicon)
		{
			DestroyIcon(iconcache.at(i).hicon);
			iconcache.at(i).hicon = NULL;
		}
	}
	iconcache.clear();

	//Icons.dll wieder freigeben
	if (hIconDll) {
		FreeLibrary(hIconDll);
		hIconDll = NULL;
	}
}

//konstruktor
Xfire_icon_mng::Xfire_icon_mng() {
	hIconDll = NULL;
}

//erzeugt aus HICON ein Handle, welches in Miranda in der Clist angewendet werden kann
HANDLE Xfire_icon_mng::createIconHandle(HICON hicon) {
	if (!hicon)
		return NULL;

	return (HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM)hicon, 0);
}

//eigentliche laderoutine
Xfire_icon_cache Xfire_icon_mng::LoadGameIcon(unsigned int gameid) {
	Xfire_icon_cache entry = { 0 };

	//shortname
	char shortname[255] = "";

	if (!getIniValue(gameid, "ShortName", shortname))
		return entry;

	//spielid zuweisen
	entry.gameid = gameid;

	//Icons.dll noch nicht geladen?!?
	if (!hIconDll)
	{
		//versuch die Icons.dll zuladen
		char path[MAX_PATH] = "";
		if (!getIconPath(path))
			return entry;
		strcat_s(path, MAX_PATH, IconsdllName);

		hIconDll = LoadLibraryA(path);
	}

	//dll konnte geladen werden
	if (hIconDll) {
		char resourcename[255];
		//kurznamen des spiels uppercasen und .ICO anhängen
		mir_snprintf(resourcename, SIZEOF(resourcename), "XF_%s.ICO", shortname);
		Xfire_base::strtoupper(resourcename);

		//versuche die resource zufinden
		HRSRC hrsrc = FindResourceA(hIconDll, resourcename, "ICONS");
		if (hrsrc) {
			//aus der resource ein HICON erstellen
			int size = SizeofResource(hIconDll, hrsrc);
			//iconresource laden
			HGLOBAL hglobal = LoadResource(hIconDll, hrsrc);
			if (hglobal) {
				//lock
				LPVOID data = LockResource(hglobal);
				if (data) {
					//erzeuge ein handle für das icon und ab in den cache damit
					entry.hicon = this->createHICONfromdata(data, size);
					UnlockResource(hglobal);
				}
				FreeResource(hglobal);
			}
		}
	}

	//kein icon in der dll, dann aus dem internet laden
	if (!entry.hicon)
		entry.hicon = downloadIcon(shortname);

	//wenn ein hicon erzeugt wurde, dann handle erstellen und in den cache laden
	if (entry.hicon)
		entry.handle = this->createIconHandle(entry.hicon);

	//eintrag in den cache, selbst wenn kein icon geladen werden konnte
	this->iconcache.push_back(entry);

	return entry;
}

//icon vom xfire server laden
HICON Xfire_icon_mng::downloadIcon(char* shortname) {
	//nur vom internetladen, wenn die option aktiv ist
	if (!db_get_b(NULL, protocolname, "xfiresitegameico", 0))
		return NULL;

	//keinen namen angegeben?
	if (!shortname)
		return NULL;

	//buffer
	char*buf = NULL;
	unsigned int size = 0;

	char url[255] = "http://media.xfire.com/xfire/xf/images/icons/";
	strcat_s(url, 255, shortname);
	strcat_s(url, 255, ".gif");

	//verscuhe das icon aus dem inet zulasen
	if (GetWWWContent2(url, NULL, FALSE, &buf, &size))
	{
		//aus dem buffer ein hicon erzeugen
		HICON hicon = this->createHICONfromdata(buf, size);
		//speicher freigeben
		delete[] buf;

		return hicon;
	}

	return NULL;
}

//setzt alle handles der icons neu
int Xfire_icon_mng::resetIconHandles() {
	for (unsigned int i = 0; i < iconcache.size(); i++)
	{
		iconcache.at(i).handle = this->createIconHandle(iconcache.at(i).hicon);
	}
	return 0;
}