#include "stdafx.h"

#include "Xfire_game.h"
#include "pwd_dlg.h" //passwort eingabe dlg

//prüft nach, ob das game das nötige extragameargs im launcherstring hat
BOOL Xfire_game::haveExtraGameArgs()
{
	//kein launcher stirng, dann abbruch
	if (!m_launchparams)
		return FALSE;

	//wenn platzhalter vorhanden, dann TRUE zurück
	if (inString(m_launchparams, "%UA_LAUNCHER_EXTRA_ARGS%"))
		return TRUE;

	return FALSE;
}

//startes das spiel
BOOL Xfire_game::start_game(char*ip, unsigned int port, char*)
{
	//launchparam prüfen ob gefüllt?
	if (m_launchparams == NULL)
		return FALSE;

	//ist launchparam großgenug für eibne urlprüfung?
	if (mir_strlen(m_launchparams) > 5) {
		//launchparams ne url? dann openurl funktion von miranda verwenden
		if (m_launchparams[0] == 'h'&&
			m_launchparams[1] == 't'&&
			m_launchparams[2] == 't'&&
			m_launchparams[3] == 'p'&&
			m_launchparams[4] == ':') {
			Utils_OpenUrl(m_launchparams);
			return 0;
		}
	}

	//spiel hat netwerkparameter?
	int networksize = 0;
	char* mynetworkparams = NULL;

	if (m_networkparams) {
		if (ip) {
			char portstr[6] = "";
			int pwsize = 255;

			//größe des netzwerparams berechnen
			if (m_pwparams)
				pwsize += mir_strlen(m_pwparams);

			mynetworkparams = new char[mir_strlen(m_networkparams) + pwsize];
			strcpy_s(mynetworkparams, mir_strlen(m_networkparams) + pwsize, m_networkparams);

			//port begrenzen
			port = port % 65535;
			//port in string wandeln
			mir_snprintf(portstr, _countof(portstr), "%d", port);

			str_replace(mynetworkparams, "%UA_GAME_HOST_NAME%", ip);
			str_replace(mynetworkparams, "%UA_GAME_HOST_PORT%", portstr);

			//passwort dialog, nur wenn SHIFT gehalten wird beim join, da sonst immer gefragt wird
			if (GetAsyncKeyState(VK_LSHIFT) && m_pwparams) {
				char password[256] = ""; //passwort maximal 255 zeichen

				if (ShowPwdDlg(password)) {
					char* mypwargs = new char[pwsize];

					//speicher frei?
					if (mypwargs != NULL) {
						strcpy_s(mypwargs, pwsize, m_pwparams);
						str_replace(mypwargs, "%UA_GAME_HOST_PASSWORD%", password);
						str_replace(mynetworkparams, "%UA_LAUNCHER_PASSWORD_ARGS%", mypwargs);
						delete[] mypwargs;
					}
					else
						str_replace(mynetworkparams, "%UA_LAUNCHER_PASSWORD_ARGS%", "");
				}
				else {
					str_replace(mynetworkparams, "%UA_LAUNCHER_PASSWORD_ARGS%", "");
				}
			}
			else
				str_replace(mynetworkparams, "%UA_LAUNCHER_PASSWORD_ARGS%", "");

			//bestimmte felder erstmal leer ersetzen			
			str_replace(mynetworkparams, "%UA_LAUNCHER_RCON_ARGS%", "");
		}

		if (mynetworkparams)
			networksize = mir_strlen(mynetworkparams) + mir_strlen(m_networkparams);
	}

	//extra parameter
	int extraparamssize = 0;
	if (m_extraparams) {
		extraparamssize = mir_strlen(m_extraparams);
	}

	//temporäres array anlegen
	char*temp = NULL;
	temp = new char[mir_strlen(m_launchparams) + networksize + extraparamssize + 1];

	if (temp == NULL) {
		//wenn nwparams gesetzt, leeren
		if (mynetworkparams)
			delete[] mynetworkparams;

		return FALSE;
	}

	//launcherstring ins temporäre array
	strcpy_s(temp, mir_strlen(m_launchparams) + 1, m_launchparams);

	//netzwerkparameter ?
	if (mynetworkparams) {
		str_replace(temp, "%UA_LAUNCHER_NETWORK_ARGS%", mynetworkparams);
		delete[] mynetworkparams;
	}
	else
		str_replace(temp, "%UA_LAUNCHER_NETWORK_ARGS%", "");

	if (m_extraparams)
		str_replace(temp, "%UA_LAUNCHER_EXTRA_ARGS%", m_extraparams);
	else
		str_replace(temp, "%UA_LAUNCHER_EXTRA_ARGS%", "");

	//auf createprocess umgebaut
	STARTUPINFOA         si = { sizeof(si) };
	PROCESS_INFORMATION  pi;

	// MessageBoxA(NULL,temp,temp,0);

	//starten
	if (CreateProcessA(0, temp, 0, 0, FALSE, 0, 0, GetLaunchPath(temp), &si, &pi) == 0) {
		//schlug fehl, dann runas methode verwenden
		char*exe = strrchr(temp, '\\');
		if (exe == 0) {
			delete[] temp;
			return FALSE;
		}
		*exe = 0;
		exe++;
		char*params = strchr(exe, '.');
		if (params != 0) {
			params = strchr(params, ' ');
			if (params != 0) {
				*params = 0;
				params++;
			}
		}


		SHELLEXECUTEINFOA sei = { 0 };
		sei.cbSize = sizeof(sei);
		sei.hwnd = NULL;
		sei.lpVerb = "runas";
		sei.lpFile = exe;
		sei.lpParameters = params;
		sei.lpDirectory = temp;
		sei.nShow = SW_SHOWNORMAL;
		ShellExecuteExA(&sei);
	}
	delete[] temp;
	return TRUE;
}

//prüft den übergebenen pfad, ob es dem spiel vergleichbar
BOOL Xfire_game::checkpath(PROCESSENTRY32* processInfo)
{
	//gibts net, weg mit dir
	if (m_path == NULL)
		return FALSE;

	//versuche ein processhandle des speils zubekommen
	HANDLE op = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processInfo->th32ProcessID);

	if (op) {
		//varaibele wohin der pfad eingelesen wird
		TCHAR fpath[MAX_PATH] = _T("");

		//lese den pfad des spiels aus
		GetModuleFileNameEx(op, NULL, fpath, _countof(fpath));

		//8.3 pfade umwandeln, nur wenn sich eine tilde im string befindet
		if (_tcschr(fpath, '~'))
			GetLongPathName(fpath, fpath, _countof(fpath));

		//alles in kelinbuchstaben umwandeln
		strtolowerT(fpath);

		if (wildcmp(_A2T(m_path), fpath))
			//if (mir_strcmp(m_path,fpath)==0)
		{
			//pfad stimmt überein, commandline prüfen
			if (checkCommandLine(op, m_mustcontain, m_notcontain)) {
				//handle zuamachen
				CloseHandle(op);
				//positive antwort an die gamedetection
				return TRUE;
			}
		}
		else //prüfe den multipfad
		{
			int size = m_mpath.size();
			for (int j = 0; j < size; j++) {
				if (mir_tstrcmpi(_A2T(m_mpath.at(j)), fpath) == 0) {
					//pfad stimmt überein, commandline prüfen
					if (checkCommandLine(op, m_mustcontain, m_notcontain)) {
						//handle zumachen
						CloseHandle(op);
						//positive antwort an die gamedetection
						return TRUE;
					}
				}
			}
		}
		//is nich das game, handle zumachen
		CloseHandle(op);
	}
	else //if (m_mustcontain==NULL&&m_notcontain==NULL) //spiele die was bestimmtes im pfad benötigen skippen
	{
		char *exename = strrchr(m_path, '\\') + 1;
		if ((INT_PTR)exename == 0x1)
			return FALSE;

		//vergleich die exenamen
		if (_stricmp(exename, _T2A(processInfo->szExeFile)) == 0)
			return TRUE;

		//anderen pfade des games noch durchprüfen
		int size = m_mpath.size();
		for (int j = 0; j < size; j++) {
			//mhn keien exe, nächsten pfad prüfen
			exename = strrchr(m_mpath.at(j), '\\') + 1;
			if ((INT_PTR)exename == 0x1)
				continue;

			//exe vergleichen
			if (_stricmp(exename, _T2A(processInfo->szExeFile)) == 0) //positive antwort an die gamedetection
				return TRUE;
		}
	}

	//keine übereinstimmung
	return FALSE;
}

//icondaten setzen
void Xfire_game::setIcon(HICON hicon, HANDLE handle)
{
	m_hicon = hicon;
	m_iconhandl = handle;
}

//liest gamewerte aus der db
void Xfire_game::readFromDB(unsigned dbid)
{
	//lese alle string werte aus der db und befülle die passenden variablen damit
	this->readStringfromDB("gamepath", dbid, &m_path);

	//8.3 fix, prüfe auf ~ pfad, wenn ja pfad var umwalnd in longname
	if (m_path) {
		BOOL found = FALSE;
		for (unsigned int i = 0; i < mir_strlen(m_path); i++) {
			if (m_path[i] == '~') {
				found = TRUE;
				break;
			}
		}
		//gefunden? dann stirng wandeln und in pfad speichern
		if (found) {
			char ctemp[MAX_PATH] = "";
			strcpy_s(ctemp, MAX_PATH, m_path);
			GetLongPathNameA(ctemp, ctemp, sizeof(ctemp));
			this->strtolower(ctemp);
			setString(ctemp, &m_path);
		}
	}

	readStringfromDB("gamelaunch", dbid, &m_launchparams);
	readStringfromDB("gamenetargs", dbid, &m_networkparams);
	readStringfromDB("gamepwargs", dbid, &m_pwparams);
	readStringfromDB("gameuserargs", dbid, &m_userparams);
	readStringfromDB("gamecmdline", dbid, &m_mustcontain);
	readStringfromDB("gamencmdline", dbid, &m_notcontain);

	//alle sonstigen werte
	m_id = this->readWordfromDB("gameid", dbid);
	m_send_gameid = this->readWordfromDB("gamesendid", dbid);
	if (m_send_gameid == 0)
		m_send_gameid = m_id;
	m_setstatusmsg = this->readWordfromDB("gamesetsmsg", dbid, 0);
	m_custom = this->readBytefromDB("gamecustom", dbid, 0);
	m_skip = this->readBytefromDB("gameskip", m_id, 0);
	m_notinstartmenu = this->readBytefromDB("notinstartmenu", m_id, 0);
	m_noicqstatus = this->readBytefromDB("gamenostatus", m_id, 0);
	readStringfromDB("gameextraparams", m_id, &m_extraparams);
	readStringfromDB("customgamename", m_id, &m_customgamename);
	this->readUtf8StringfromDB("statusmsg", m_id, &m_statusmsg);

	//mehrere pfade
	int size = this->readWordfromDB("gamemulti", dbid, 0);
	for (int j = 0; j < size; j++) {
		char* tpath = NULL;
		this->readStringfromDB("gamepath", dbid, j, &tpath);

		if (tpath) {
			m_mpath.push_back(tpath);
		}
	}

	//spielname auslesen, wird später für die sortierung gebraucht
	setNameandIcon();
}

//läd spielnamen aus, sowie icon
void Xfire_game::setNameandIcon()
{
	if (m_customgamename) {
		setString(m_customgamename, &m_name);
	}
	else {
		//std::string game=GetGame(m_id,0,&m_iconhandl,&m_hicon,TRUE);
		//zielbuffer für den namen
		char buf[XFIRE_MAXSIZEOFGAMENAME] = "Unknown Game";
		//name des spiels auslesen
		this->getGamename(m_id, buf, XFIRE_MAXSIZEOFGAMENAME);
		//namen setzen
		setString(buf, &m_name);
	}
}

//liest gamewerte aus der db
void Xfire_game::writeToDB(unsigned dbid)
{
	//alle stringwerte schreiben
	writeStringtoDB("gamepath", dbid, m_path);
	writeStringtoDB("gamelaunch", dbid, m_launchparams);
	writeStringtoDB("gamenetargs", dbid, m_networkparams);
	writeStringtoDB("gamepwargs", dbid, m_pwparams);
	writeStringtoDB("gameuserargs", dbid, m_userparams);
	writeStringtoDB("gamecmdline", dbid, m_mustcontain);
	writeStringtoDB("gamencmdline", dbid, m_notcontain);

	//alle sonstigen werte
	this->writeWordtoDB("gameid", dbid, m_id);
	if (m_send_gameid != 0 && m_send_gameid != m_id) this->writeWordtoDB("gamesendid", dbid, m_send_gameid);
	if (m_setstatusmsg != 0) writeWordtoDB("gamesetsmsg", dbid, m_setstatusmsg);
	if (m_custom != 0) writeBytetoDB("gamecustom", dbid, m_custom);
	//wenn gesetzt, dann eintrag machen
	if (m_skip != 0)
		writeBytetoDB("gameskip", m_id, m_skip);
	else //wenn nicht eintrag aus db löschen
		removeDBEntry("gameskip", m_id);
	if (m_notinstartmenu != 0)
		writeBytetoDB("notinstartmenu", m_id, m_notinstartmenu);
	else //wenn nicht eintrag aus db löschen
		removeDBEntry("notinstartmenu", m_id);
	if (m_noicqstatus != 0)
		writeBytetoDB("gamenostatus", m_id, m_noicqstatus);
	else //wenn nicht eintrag aus db löschen
		removeDBEntry("gamenostatus", m_id);
	if (m_extraparams != 0)
		writeStringtoDB("gameextraparams", m_id, m_extraparams);
	else //wenn nicht eintrag aus db löschen
		removeDBEntry("gameextraparams", m_id);
	if (m_customgamename != 0)
		writeStringtoDB("customgamename", m_id, m_customgamename);
	else //wenn nicht eintrag aus db löschen
		removeDBEntry("customgamename", m_id);
	if (m_statusmsg != 0)
		this->writeUtf8StringtoDB("statusmsg", m_id, m_statusmsg);
	else //wenn nicht eintrag aus db löschen
		removeDBEntry("statusmsg", m_id);

	// mehrere pfade
	int size = m_mpath.size();
	if (size > 0) {
		this->writeWordtoDB("gamemulti", dbid, m_mpath.size());
		for (int j = 0; j < size; j++) {
			writeStringtoDB("gamepath", dbid, j, m_mpath.at(j));
		}
	}

	// sendid 0 dann standard id reinladen
	if (m_send_gameid == 0)
		m_send_gameid = m_id;
}

// erzeugt ein menüpunkt
void Xfire_game::createMenuitem(unsigned int pos, int dbid)
{
	char servicefunction[100];
	mir_strcpy(servicefunction, protocolname);
	mir_strcat(servicefunction, "StartGame%d");

	if (dbid < 0)
		dbid = pos;

	CMenuItem mi;
	mi.root = Menu_CreateRoot(MO_MAIN, LPGENT("Start game"), 500084000);

	mir_snprintf(m_temp, _countof(m_temp), servicefunction, m_id);
	//wenn die servicefunktion schon exisitert vernichten, hehe
	if (ServiceExists(m_temp))
		DestroyServiceFunction(m_temp);
	CreateServiceFunctionParam(m_temp, StartGame, m_id);
	mi.pszService = m_temp;
	mi.position = 500090002 + pos;
	mi.hIcolibItem = m_hicon ? m_hicon : LoadIcon(hinstance, MAKEINTRESOURCE(ID_OP));
	mi.name.a = menuitemtext(m_name);
	m_menuhandle = Menu_AddMainMenuItem(&mi);

	//menu aktualisieren ob hidden
	this->refreshMenuitem();
}

//entfernt menüpunkt
void Xfire_game::remoteMenuitem()
{
	if (m_menuhandle != NULL) {
		Menu_RemoveItem(m_menuhandle);
		m_menuhandle = NULL;
	}
}

//aktualisiert menüpunkt ob hidden
void Xfire_game::refreshMenuitem()
{
	if (m_menuhandle != NULL)
		Menu_ShowItem(m_menuhandle, !m_notinstartmenu);
}