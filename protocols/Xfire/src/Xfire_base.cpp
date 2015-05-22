#include "stdafx.h"

#include "Xfire_base.h"
#include "variables.h"

//rechnet die einzelnen chars des strings zusammen
BYTE Xfire_base::accStringByte(char* str){
	BYTE temp = 0;

	//keins tirng? bye bye
	if (str == NULL)
		return 0;

	for (unsigned int i = 0; i < (int)mir_strlen(str); i++)
	{
		temp += str[i];
	}

	return temp;
}

//lowercased einen string
void Xfire_base::strtolower(char*str)
{
	//keins tirng? bye bye
	if (str == NULL)
		return;

	//lowercase it :)
	for (unsigned int i = 0; i < (int)mir_strlen(str); i++)
	{
		str[i] = tolower(str[i]);
	}
}

void Xfire_base::strtolowerT(TCHAR*str)
{
	//keins tirng? bye bye
	if (str == NULL)
		return;

	//lowercase it :)
	for (unsigned int i = 0; i < (int)mir_tstrlen(str); i++)
	{
		str[i] = tolower(str[i]);
	}
}

//uppercased einen string
void Xfire_base::strtoupper(char*str)
{
	//keins tirng? bye bye
	if (str == NULL)
		return;

	//lowercase it :)
	for (unsigned int i = 0; i < (int)mir_strlen(str); i++)
	{
		str[i] = toupper(str[i]);
	}
}

//setzt einen string
void Xfire_base::setString(char*from, char**to)
{
	//keine quelle, kein ziel? dann nix machen
	if (from == NULL || to == NULL)
		return;

	//stringgröße auslesen
	int size = mir_strlen(from);

	//bestehenden zielpointer leeren
	if (*to != NULL)
		delete[] * to;

	//neuen string anlegen
	*to = new char[size + 1];

	//string in neues array kopieren
	strcpy_s(*to, size + 1, from);
}

//setzt einen string
void Xfire_base::appendString(char*from, char**to)
{
	//keine quelle, kein ziel? dann nix machen
	if (from == NULL || to == NULL || *to == NULL)
		return;

	//stringgröße auslesen
	int size = mir_strlen(from);
	int size2 = mir_strlen(*to);

	//temporären pointer anlegen
	char* append = new char[size + size2 + 1];

	//string in neues array kopieren
	strcpy_s(append, size + size2 + 1, *to);
	//zusätzlichen stirng anhängen
	strcat_s(append, size + size2 + 1, from);
	//altes to entfernen
	delete[] * to;
	//appendstring zuordnen
	*to = append;
}

//liest einen stringval aus der db und setzt einen string für
void Xfire_base::readStringfromDB(char*name, unsigned int dbid, char**to)
{
	//keine quelle, kein ziel? dann nix machen
	if (name == NULL || to == NULL)
		return;

	//wert aus der dblesen
	mir_snprintf(temp, SIZEOF(temp), "%s_%i", name, dbid);
	if (!db_get_s(NULL, protocolname, temp, &dbv))
	{
		//string setzen
		setString(dbv.pszVal, to);
		//dbval wieder freigeben
		db_free(&dbv);
	}
}

//liest einen stringval aus der db welches unterid hat und setzt einen string für
void Xfire_base::readStringfromDB(char*name, unsigned int dbid, int id, char**to)
{
	//keine quelle, kein ziel? dann nix machen
	if (name == NULL || to == NULL)
		return;

	//wert aus der dblesen
	mir_snprintf(temp, SIZEOF(temp), "%s_%i_%i", name, dbid, id);
	if (!db_get_s(NULL, protocolname, temp, &dbv))
	{
		//string setzen
		setString(dbv.pszVal, to);
		//dbval wieder freigeben
		db_free(&dbv);
	}
}

//liest einen stringval aus der db und setzt einen string für
void Xfire_base::readUtf8StringfromDB(char*name, unsigned int dbid, char**to)
{
	//keine quelle, kein ziel? dann nix machen
	if (name == NULL || to == NULL)
		return;

	//wert aus der dblesen
	mir_snprintf(temp, SIZEOF(temp), "%s_%i", name, dbid);
	if (!db_get_utf(NULL, protocolname, temp, &dbv))
	{
		//string setzen
		setString(dbv.pszVal, to);
		//dbval wieder freigeben
		db_free(&dbv);
	}
}

//liest einen stringval aus der db welches unterid hat und setzt einen string für
void Xfire_base::readUtf8StringfromDB(char*name, unsigned int dbid, int id, char**to)
{
	//keine quelle, kein ziel? dann nix machen
	if (name == NULL || to == NULL)
		return;

	//wert aus der dblesen
	mir_snprintf(temp, SIZEOF(temp), "%s_%i_%i", name, dbid, id);
	if (!db_get_utf(NULL, protocolname, temp, &dbv))
	{
		//string setzen
		setString(dbv.pszVal, to);
		//dbval wieder freigeben
		db_free(&dbv);
	}
}


//schreibt einen stringval in die db welche unterid hat
void Xfire_base::writeStringtoDB(char*name, unsigned int dbid, int id, char*val)
{
	//keine quelle, kein ziel? dann nix machen
	if (name == NULL || val == NULL)
		return;

	//wert aus der dblesen
	mir_snprintf(temp, SIZEOF(temp), "%s_%i_%i", name, dbid, id);
	db_set_s(NULL, protocolname, temp, val);
}

//schreibt einen stringval in die db welche unterid hat
void Xfire_base::writeStringtoDB(char*name, unsigned int dbid, char*val)
{
	//keine quelle, kein ziel? dann nix machen
	if (name == NULL || val == NULL)
		return;

	//wert aus der dblesen
	mir_snprintf(temp, SIZEOF(temp), "%s_%i", name, dbid);
	db_set_s(NULL, protocolname, temp, val);
}

//schreibt einen stringval in die db welche unterid hat
void Xfire_base::writeUtf8StringtoDB(char*name, unsigned int dbid, int id, char*val)
{
	//keine quelle, kein ziel? dann nix machen
	if (name == NULL || val == NULL)
		return;

	//wert aus der dblesen
	mir_snprintf(temp, SIZEOF(temp), "%s_%i_%i", name, dbid, id);
	db_set_utf(NULL, protocolname, temp, val);
}

//schreibt einen stringval in die db welche unterid hat
void Xfire_base::writeUtf8StringtoDB(char*name, unsigned int dbid, char*val)
{
	//keine quelle, kein ziel? dann nix machen
	if (name == NULL || val == NULL)
		return;

	//wert aus der dblesen
	mir_snprintf(temp, SIZEOF(temp), "%s_%i", name, dbid);
	db_set_utf(NULL, protocolname, temp, val);
}


//schreibt einen bytewert in die db
void Xfire_base::writeBytetoDB(char*name, unsigned int dbid, int val)
{
	//keine quelle, kein ziel? dann nix machen
	if (name == NULL)
		return;

	//wert aus der dblesen
	mir_snprintf(temp, SIZEOF(temp), "%s_%i", name, dbid);
	db_set_b(NULL, protocolname, temp, val);
}

//schreibt einen wordwert in die db
void Xfire_base::writeWordtoDB(char*name, unsigned int dbid, int val)
{
	//keine quelle, kein ziel? dann nix machen
	if (name == NULL)
		return;

	//wert aus der dblesen
	mir_snprintf(temp, SIZEOF(temp), "%s_%i", name, dbid);
	db_set_w(NULL, protocolname, temp, val);
}


//liest einen byteval aus der db und gibt es zurück
unsigned char Xfire_base::readBytefromDB(char*name, unsigned int dbid, int defaultval)
{
	//keine quelle, kein ziel? dann nix machen
	if (name == NULL)
		return 0;

	//wert aus der dblesen
	mir_snprintf(temp, SIZEOF(temp), "%s_%i", name, dbid);
	return db_get_b(NULL, protocolname, temp, defaultval);
}

//liest einen wordval aus der db und gibt es zurück
unsigned int Xfire_base::readWordfromDB(char*name, unsigned int dbid, int defaultval)
{
	//keine quelle, kein ziel? dann nix machen
	if (name == NULL)
		return 0;

	//wert aus der dblesen
	mir_snprintf(temp, SIZEOF(temp), "%s_%i", name, dbid);
	return db_get_w(NULL, protocolname, temp, defaultval);
}

//entfernt einen dbeintrag
BOOL Xfire_base::removeDBEntry(char*name, unsigned int dbid)
{
	//kein name?
	if (name == NULL)
		return 0;

	//wert aus der dblesen
	mir_snprintf(temp, SIZEOF(temp), "%s_%i", name, dbid);

	//eintrag entfernen
	if (!db_get(NULL, protocolname, temp, &dbv))
	{
		db_free(&dbv);
		db_unset(NULL, protocolname, temp);

		return TRUE;
	}
	return FALSE;
}

//entfernt einen dbeintrag welche unterid's hat
BOOL Xfire_base::removeDBEntry(char*name, unsigned int dbid, int id)
{
	//kein name?
	if (name == NULL)
		return 0;

	//wert aus der dblesen
	mir_snprintf(temp, SIZEOF(temp), "%s_%i_%i", name, dbid, id);

	//eintrag entfernen
	if (!db_get(NULL, protocolname, temp, &dbv))
	{
		db_free(&dbv);
		db_unset(NULL, protocolname, temp);

		return TRUE;
	}
	return FALSE;
}

//sucht innerhalb eines strings ein anderen string und liefert true zurück wenn gefunden
BOOL Xfire_base::inString(char*str, char*search, char**pos) {
	//leere pointer?, dann FALSE zurück
	if (str == NULL || search == NULL)
	{
		//poszeiger, falls übergeben, auf NULL setzen
		if (pos) *pos = NULL;
		return FALSE;
	}

	//ist der gesuchte string größer, wie der string wo gesucht werden soll? dann FALSE zurück
	unsigned int sizeofsearch = mir_strlen(search);
	if (sizeofsearch > mir_strlen(str))
	{
		//poszeiger, falls übergeben, auf NULL setzen
		if (pos) *pos = NULL;
		return FALSE;
	}

	char* src = str;
	char* s = search;

	while (*src != 0)
	{
		if (*src == *s) //gleich, dann zum nächsten buchstaben springen
			s++;
		else if (*s == 0) //string ende erreicht? dann true
		{
			//poszeiger mit aktueller pos füllen, da position gefunden
			if (pos) *pos = src - sizeofsearch;
			return TRUE;
		}
		else //anderer biuchstabe? dann wieder zurück zum anfangsbuchstaben
			s = search;
		src++;
	}
	if (*s == 0) //string ende erreicht? dann true
	{
		//poszeiger mit aktueller pos füllen, da position gefunden
		if (pos) *pos = src - sizeofsearch;
		return TRUE;
	}

	//poszeiger, falls übergeben, auf NULL setzen
	if (pos) *pos = NULL;
	//nix gefunden, FALSE
	return FALSE;
}

void Xfire_base::strreplace(char*search, char*replace, char**data) {
	if (replace == NULL)
		replace = "";

	//leere pointer?, dann zurück
	if (search == NULL || data == NULL || *data == NULL)
	{
		return;
	}

	char* pos = NULL;
	//gesuchten string suchen
	if (this->inString(*data, search, &pos))
	{
		//gefunden? dann replace
		*pos = 0;
		char* newdata = NULL;
		//alles vorm gefunden anhängen
		this->setString(*data, &newdata);
		//ersetzendes anhängen
		this->appendString(replace, &newdata);
		//poszeiger um die größe des zusuchenden strings erhöhen
		pos += mir_strlen(search);
		//rest anhängen
		this->appendString(pos, &newdata);
		//alten string löschen
		delete[] * data;
		*data = NULL;
		//neuen auf diesen setzen
		this->setString(newdata, data);
		//temporären newdata löschen
		delete[] newdata;
	}
}

//stringvergleich mit wildcards
BOOL Xfire_base::wildcmp(const TCHAR *search, const TCHAR *text) {
	//keine gültigen strings, dann abbruch
	if (search == NULL || text == NULL || *text == 0 || *search == 0)
		return FALSE;

	//wildcardmodus
	int wildc = 0;

	do {
		//wilcard gefunden?
		if (*search == '*')
		{
			//wildcardmodus an
			wildc = 1;
			//nächsten suchzeichen
			search++;
			//wenn searchzeichen 0, dann 1 zurückgeben, da der rest des searchstings egal ist
			if (*search == 0)
				return TRUE;
		}

		//prüfe searchzeichen mit textzeichen, aber nut wenn kein wildcardmodus aktiv ist
		if (*search != *text && !wildc)
			return FALSE;

		//kein wildcardmodus
		if (!wildc)
		{
			//nächstes suchzeichen
			search++;
		}
		//wenn suchzeichen und textzeichen gleich ist
		else if (*search == *text)
		{
			//den wildcardmodus abschalten
			wildc = 0;
			//nächstes suchzeichen
			search++;
		}
		//nächstes textzeichen
		text++;
		//wenn textende erreicht und suchzeichen noch vorhanden, dann 0 zurückgegebn
		if (*text == 0 && *search != 0)
			return FALSE;
	}
	//solange weiter bis kein suchzeichen mehr vorhanden is
	while (*search != 0);

	//textzeichen übrig 0 zurück
	if (*text != 0)
		return FALSE;

	//volle übereinstimmung
	return TRUE;
}

//wrapper wenn nur gameid angegeben
BOOL Xfire_base::getIniValue(unsigned int gameid, const char* valname, char*out, int sizeofout) {
	return Xfire_base::getIniValue(gameid, 0, valname, out, sizeofout);
}


//läd einen eintrag aus der ini
BOOL Xfire_base::getIniValue(unsigned int gameid, unsigned int subid, const char* valname, char*out, int sizeofout) {
	//kein ziel oder kein variablenname, dann FALSE zurück
	if (!out || !valname)
		return FALSE;

	char idstring[15];
	char path[MAX_PATH];

	//pfad einlesen
	if (!getIniPath(path))
		return FALSE; //kein pfad bug?!?!

	if (subid == 0) {
		mir_snprintf(idstring, SIZEOF(idstring), "%d", gameid);

		if (xfire_GetPrivateProfileString(idstring, valname, "", out, sizeofout, path))
			return TRUE;

		subid++;
	}

	mir_snprintf(idstring, SIZEOF(idstring), "%d_%d", gameid, subid);

	if (xfire_GetPrivateProfileString(idstring, valname, "", out, sizeofout, path))
		return TRUE;

	return FALSE;
}

BOOL Xfire_base::getIniPath(char*path) {
	//kein ziel abbruch
	if (!path)
		return FALSE;
	strcpy(path, XFireGetFoldersPath("IniFile"));
	strcat_s(path, MAX_PATH, "xfire_games.ini");
	return TRUE;
}

BOOL Xfire_base::getIconPath(char*path) {
	//kein ziel abbruch
	if (!path)
		return FALSE;
	strcpy(path, XFireGetFoldersPath("IconsFile"));
	return TRUE;
}

BOOL Xfire_base::getGamename(unsigned int gameid, char* out, int outsize){
	//kein ziel
	if (!out)
		return FALSE;

	if (!getIniValue(gameid, "LongName", out, outsize))
	{
		//customnamen laden, wenn vorhanden
		DBVARIANT dbv;
		char dbstr[XFIRE_MAXSIZEOFGAMENAME];
		mir_snprintf(dbstr, SIZEOF(dbstr), "customgamename_%d", gameid);
		if (!db_get(NULL, protocolname, dbstr, &dbv)) {
			strncpy_s(out, outsize, dbv.pszVal, _TRUNCATE);
			db_free(&dbv);
			return TRUE;
		}
		else
			return FALSE;
	}
	return TRUE;
}

//wandelt einen buffer mit größe in ein hicon mit hilfe von gdi+ um
HICON Xfire_base::createHICONfromdata(LPVOID data, unsigned int size) {
	//zielspeichern anlegen
	HGLOBAL buffer = GlobalAlloc(GMEM_MOVEABLE, size);
	if (buffer) {
		LPVOID data2 = GlobalLock(buffer);
		if (data2) {
			memcpy(data2, data, size);

			IStream* stream = NULL;
			if (CreateStreamOnHGlobal(data2, FALSE, &stream) == S_OK)
			{
				HICON hicon = NULL;
				Gdiplus::Bitmap image(stream);

				image.GetHICON(&hicon);
				stream->Release();

				GlobalUnlock(buffer);
				GlobalFree(buffer);

				return hicon;
			}
			GlobalUnlock(buffer);
		}
		GlobalFree(buffer);
	}
	return NULL;
}

//wandelt einen buffer mit größe in ein hicon mit hilfe von gdi+ um
HBITMAP Xfire_base::createHBITMAPfromdata(LPVOID data, unsigned int size) {
	//zielspeichern anlegen
	HGLOBAL buffer = GlobalAlloc(GMEM_MOVEABLE, size);
	if (buffer) {
		LPVOID data2 = GlobalLock(buffer);
		if (data2) {
			memcpy(data2, data, size);

			IStream* stream = NULL;
			if (CreateStreamOnHGlobal(data2, FALSE, &stream) == S_OK)
			{
				HBITMAP hbitmap = NULL;
				Gdiplus::Bitmap image(stream);

				image.GetHBITMAP(0, &hbitmap);
				stream->Release();

				GlobalUnlock(buffer);
				GlobalFree(buffer);

				return hbitmap;
			}
			GlobalUnlock(buffer);
		}
		GlobalFree(buffer);
	}
	return NULL;
}

//prüft ob processid noch gültig is
BOOL Xfire_base::isValidPid(DWORD pid) {
	DWORD aProcesses[1024], cbNeeded, cProcesses;

	if (pid == NULL)
		return FALSE;

	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
		return FALSE;

	cProcesses = cbNeeded / sizeof(DWORD);

	for (unsigned int i = 0; i < cProcesses; i++)
		if (aProcesses[i] == pid)
		{
		return TRUE;
		}

	return FALSE;
}

//sucht nach einen process und liefert die pid
BOOL Xfire_base::getPidByProcessName(TCHAR *name, DWORD *pid) {
	if (pid == NULL || name == NULL)
		return FALSE;

	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
	PROCESSENTRY32* processInfo = new PROCESSENTRY32;
	processInfo->dwSize = sizeof(PROCESSENTRY32);

	while (Process32Next(hSnapShot, processInfo) != FALSE)
	{
		if (processInfo->th32ProcessID != 0) {
			if (mir_tstrcmpi(processInfo->szExeFile, name) == 0)
			{
				*pid = processInfo->th32ProcessID;
				CloseHandle(hSnapShot);
				return TRUE;
			}
		}
	}
	CloseHandle(hSnapShot);
	return FALSE;
}
