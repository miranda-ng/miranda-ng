#include "stdafx.h"

#include "Xfire_game.h"
#include "pwd_dlg.h" //passwort eingabe dlg

//pr�ft nach, ob das game das n�tige extragameargs im launcherstring hat
BOOL Xfire_game::haveExtraGameArgs() {
	//kein launcher stirng, dann abbruch
	if (!this->launchparams)
		return FALSE;

	//wenn platzhalter vorhanden, dann TRUE zur�ck
	if (this->inString(this->launchparams, "%UA_LAUNCHER_EXTRA_ARGS%"))
		return TRUE;

	return FALSE;
}

//startes das spiel
BOOL Xfire_game::start_game(char*ip, unsigned int port, char*pw) {
	//launchparam pr�fen ob gef�llt?
	if (this->launchparams == NULL)
		return FALSE;

	//ist launchparam gro�genug f�r eibne urlpr�fung?
	if (mir_strlen(this->launchparams) > 5)
	{
		//launchparams ne url? dann openurl funktion von miranda verwenden
		if (this->launchparams[0] == 'h'&&
			this->launchparams[1] == 't'&&
			this->launchparams[2] == 't'&&
			this->launchparams[3] == 'p'&&
			this->launchparams[4] == ':')
		{
			Utils_OpenUrl(this->launchparams);
			return 0;
		}
	}

	//spiel hat netwerkparameter?
	int networksize = 0;
	char* mynetworkparams = NULL;

	if (this->networkparams)
	{
		if (ip)
		{
			char portstr[6] = "";
			int pwsize = 255;

			//gr��e des netzwerparams berechnen
			if (this->pwparams)
				pwsize += mir_strlen(this->pwparams);

			mynetworkparams = new char[mir_strlen(this->networkparams) + pwsize];
			strcpy_s(mynetworkparams, mir_strlen(this->networkparams) + pwsize, this->networkparams);

			//port begrenzen
			port = port % 65535;
			//port in string wandeln
			mir_snprintf(portstr, _countof(portstr), "%d", port);

			str_replace(mynetworkparams, "%UA_GAME_HOST_NAME%", ip);
			str_replace(mynetworkparams, "%UA_GAME_HOST_PORT%", portstr);

			//passwort dialog, nur wenn SHIFT gehalten wird beim join, da sonst immer gefragt wird
			if (GetAsyncKeyState(VK_LSHIFT) && this->pwparams){
				char password[256] = ""; //passwort maximal 255 zeichen

				if (ShowPwdDlg(password)) {
					char* mypwargs = new char[pwsize];

					//speicher frei?
					if (mypwargs != NULL) {
						strcpy_s(mypwargs, pwsize, this->pwparams);
						str_replace(mypwargs, "%UA_GAME_HOST_PASSWORD%", password);
						str_replace(mynetworkparams, "%UA_LAUNCHER_PASSWORD_ARGS%", mypwargs);
						delete[] mypwargs;
					}
					else
						str_replace(mynetworkparams, "%UA_LAUNCHER_PASSWORD_ARGS%", "");
				}
				else
				{
					str_replace(mynetworkparams, "%UA_LAUNCHER_PASSWORD_ARGS%", "");
				}
			}
			else
				str_replace(mynetworkparams, "%UA_LAUNCHER_PASSWORD_ARGS%", "");

			//bestimmte felder erstmal leer ersetzen			
			str_replace(mynetworkparams, "%UA_LAUNCHER_RCON_ARGS%", "");
		}

		if (mynetworkparams)
			networksize = mir_strlen(mynetworkparams) + mir_strlen(this->networkparams);
	}

	//extra parameter
	int extraparamssize = 0;
	if (this->extraparams)
	{
		extraparamssize = mir_strlen(this->extraparams);
	}

	//tempor�res array anlegen
	char*temp = NULL;
	temp = new char[mir_strlen(this->launchparams) + networksize + extraparamssize + 1];

	if (temp == NULL)
	{
		//wenn nwparams gesetzt, leeren
		if (mynetworkparams)
			delete[] mynetworkparams;

		return FALSE;
	}

	//launcherstring ins tempor�re array
	strcpy_s(temp, mir_strlen(this->launchparams) + 1, this->launchparams);

	//netzwerkparameter ?
	if (mynetworkparams)
	{
		str_replace(temp, "%UA_LAUNCHER_NETWORK_ARGS%", mynetworkparams);
		delete[] mynetworkparams;
	}
	else
		str_replace(temp, "%UA_LAUNCHER_NETWORK_ARGS%", "");

	if (this->extraparams)
		str_replace(temp, "%UA_LAUNCHER_EXTRA_ARGS%", this->extraparams);
	else
		str_replace(temp, "%UA_LAUNCHER_EXTRA_ARGS%", "");

	//auf createprocess umgebaut
	STARTUPINFOA         si = { sizeof(si) };
	PROCESS_INFORMATION  pi;

	// MessageBoxA(NULL,temp,temp,0);

	//starten
	if (CreateProcessA(0, temp, 0, 0, FALSE, 0, 0, GetLaunchPath(temp), &si, &pi) == 0)
	{
		//schlug fehl, dann runas methode verwenden
		char*exe = strrchr(temp, '\\');
		if (exe == 0)
		{
			delete[] temp;
			return FALSE;
		}
		*exe = 0;
		exe++;
		char*params = strchr(exe, '.');
		if (params != 0)
		{
			params = strchr(params, ' ');
			if (params != 0)
			{
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

//pr�ft den �bergebenen pfad, ob es dem spiel vergleichbar
BOOL Xfire_game::checkpath(PROCESSENTRY32* processInfo)
{
	//gibts net, weg mit dir
	if (this->path == NULL)
		return FALSE;

	//versuche ein processhandle des speils zubekommen
	HANDLE op = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processInfo->th32ProcessID);

	if (op)
	{
		//varaibele wohin der pfad eingelesen wird
		TCHAR fpath[MAX_PATH] = _T("");

		//lese den pfad des spiels aus
		GetModuleFileNameEx(op, NULL, fpath, _countof(fpath));

		//8.3 pfade umwandeln, nur wenn sich eine tilde im string befindet
		if (_tcschr(fpath, '~'))
			GetLongPathName(fpath, fpath, _countof(fpath));

		//alles in kelinbuchstaben umwandeln
		this->strtolowerT(fpath);

		if (this->wildcmp(_A2T(this->path), fpath))
			//if (mir_strcmp(this->path,fpath)==0)
		{
			//pfad stimmt �berein, commandline pr�fen
			if (checkCommandLine(op, this->mustcontain, this->notcontain))
			{
				//handle zuamachen
				CloseHandle(op);
				//positive antwort an die gamedetection
				return TRUE;
			}
		}
		else //pr�fe den multipfad
		{
			int size = mpath.size();
			for (int j = 0; j < size; j++)
			{
				if (mir_tstrcmpi(_A2T(mpath.at(j)), fpath) == 0)
				{
					//pfad stimmt �berein, commandline pr�fen
					if (checkCommandLine(op, this->mustcontain, this->notcontain))
					{
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
	else //if (this->mustcontain==NULL&&this->notcontain==NULL) //spiele die was bestimmtes im pfad ben�tigen skippen
	{
		char* exename = strrchr(this->path, '\\') + 1;

		//kleiner fix bei fehlerhaften pfaden kann keine exe ermittelt werden also SKIP
		if ((unsigned int)exename == 0x1)
			return FALSE;

		//vergleich die exenamen
		if (_stricmp(exename, _T2A(processInfo->szExeFile)) == 0)
		{
			return TRUE;
		}
		else //anderen pfade des games noch durchpr�fen
		{
			int size = mpath.size();
			for (int j = 0; j < size; j++)
			{
				//exenamen rausfischen
				char* exename = strrchr(mpath.at(j), '\\') + 1;

				//mhn keien exe, n�chsten pfad pr�fen
				if ((unsigned int)exename == 0x1)
					continue;

				//exe vergleichen
				if (_stricmp(exename, _T2A(processInfo->szExeFile)) == 0)
				{
					//positive antwort an die gamedetection
					return TRUE;
				}
			}
		}
	}

	//keine �bereinstimmung
	return FALSE;
}

//icondaten setzen
void Xfire_game::setIcon(HICON hicon, HANDLE handle) {
	this->hicon = hicon;
	this->iconhandl = handle;
}

//liest gamewerte aus der db
void Xfire_game::readFromDB(unsigned dbid)
{
	//lese alle string werte aus der db und bef�lle die passenden variablen damit
	this->readStringfromDB("gamepath", dbid, &this->path);

	//8.3 fix, pr�fe auf ~ pfad, wenn ja pfad var umwalnd in longname
	if (this->path)
	{
		BOOL found = FALSE;
		for (unsigned int i = 0; i < mir_strlen(this->path); i++)
		{
			if (this->path[i] == '~')
			{
				found = TRUE;
				break;
			}
		}
		//gefunden? dann stirng wandeln und in pfad speichern
		if (found) {
			char ctemp[MAX_PATH] = "";
			strcpy_s(ctemp, MAX_PATH, this->path);
			GetLongPathNameA(ctemp, ctemp, sizeof(ctemp));
			this->strtolower(ctemp);
			this->setString(ctemp, &this->path);
		}
	}

	this->readStringfromDB("gamelaunch", dbid, &this->launchparams);
	this->readStringfromDB("gamenetargs", dbid, &this->networkparams);
	this->readStringfromDB("gamepwargs", dbid, &this->pwparams);
	this->readStringfromDB("gameuserargs", dbid, &this->userparams);
	this->readStringfromDB("gamecmdline", dbid, &this->mustcontain);
	this->readStringfromDB("gamencmdline", dbid, &this->notcontain);

	//alle sonstigen werte
	this->id = this->readWordfromDB("gameid", dbid);
	this->send_gameid = this->readWordfromDB("gamesendid", dbid);
	if (this->send_gameid == 0)
		this->send_gameid = this->id;
	this->setstatusmsg = this->readWordfromDB("gamesetsmsg", dbid, 0);
	this->custom = this->readBytefromDB("gamecustom", dbid, 0);
	this->skip = this->readBytefromDB("gameskip", this->id, 0);
	this->notinstartmenu = this->readBytefromDB("notinstartmenu", this->id, 0);
	this->noicqstatus = this->readBytefromDB("gamenostatus", this->id, 0);
	this->readStringfromDB("gameextraparams", this->id, &this->extraparams);
	this->readStringfromDB("customgamename", this->id, &this->customgamename);
	this->readUtf8StringfromDB("statusmsg", this->id, &this->statusmsg);

	//mehrere pfade
	int size = this->readWordfromDB("gamemulti", dbid, 0);
	for (int j = 0; j < size; j++)
	{
		char* tpath = NULL;
		this->readStringfromDB("gamepath", dbid, j, &tpath);

		if (tpath)
		{
			mpath.push_back(tpath);
		}
	}

	//spielname auslesen, wird sp�ter f�r die sortierung gebraucht
	setNameandIcon();
}

//l�d spielnamen aus, sowie icon
void Xfire_game::setNameandIcon()
{
	if (this->customgamename)
	{
		this->setString(this->customgamename, &this->name);
	}
	else
	{
		//std::string game=GetGame(this->id,0,&this->iconhandl,&this->hicon,TRUE);
		//zielbuffer f�r den namen
		char buf[XFIRE_MAXSIZEOFGAMENAME] = "Unknown Game";
		//name des spiels auslesen
		this->getGamename(this->id, buf, XFIRE_MAXSIZEOFGAMENAME);
		//namen setzen
		this->setString(buf, &this->name);
	}
}

//liest gamewerte aus der db
void Xfire_game::writeToDB(unsigned dbid)
{
	//alle stringwerte schreiben
	this->writeStringtoDB("gamepath", dbid, this->path);
	this->writeStringtoDB("gamelaunch", dbid, this->launchparams);
	this->writeStringtoDB("gamenetargs", dbid, this->networkparams);
	this->writeStringtoDB("gamepwargs", dbid, this->pwparams);
	this->writeStringtoDB("gameuserargs", dbid, this->userparams);
	this->writeStringtoDB("gamecmdline", dbid, this->mustcontain);
	this->writeStringtoDB("gamencmdline", dbid, this->notcontain);

	//alle sonstigen werte
	this->writeWordtoDB("gameid", dbid, this->id);
	if (this->send_gameid != 0 && this->send_gameid != this->id) this->writeWordtoDB("gamesendid", dbid, this->send_gameid);
	if (this->setstatusmsg != 0) this->writeWordtoDB("gamesetsmsg", dbid, this->setstatusmsg);
	if (this->custom != 0) this->writeBytetoDB("gamecustom", dbid, this->custom);
	//wenn gesetzt, dann eintrag machen
	if (this->skip != 0)
		this->writeBytetoDB("gameskip", this->id, this->skip);
	else //wenn nicht eintrag aus db l�schen
		this->removeDBEntry("gameskip", this->id);
	if (this->notinstartmenu != 0)
		this->writeBytetoDB("notinstartmenu", this->id, this->notinstartmenu);
	else //wenn nicht eintrag aus db l�schen
		this->removeDBEntry("notinstartmenu", this->id);
	if (this->noicqstatus != 0)
		this->writeBytetoDB("gamenostatus", this->id, this->noicqstatus);
	else //wenn nicht eintrag aus db l�schen
		this->removeDBEntry("gamenostatus", this->id);
	if (this->extraparams != 0)
		this->writeStringtoDB("gameextraparams", this->id, this->extraparams);
	else //wenn nicht eintrag aus db l�schen
		this->removeDBEntry("gameextraparams", this->id);
	if (this->customgamename != 0)
		this->writeStringtoDB("customgamename", this->id, this->customgamename);
	else //wenn nicht eintrag aus db l�schen
		this->removeDBEntry("customgamename", this->id);
	if (this->statusmsg != 0)
		this->writeUtf8StringtoDB("statusmsg", this->id, this->statusmsg);
	else //wenn nicht eintrag aus db l�schen
		this->removeDBEntry("statusmsg", this->id);



	//mehrere pfade
	int size = mpath.size();
	if (size > 0)
	{
		this->writeWordtoDB("gamemulti", dbid, mpath.size());
		for (int j = 0; j < size; j++)
		{
			this->writeStringtoDB("gamepath", dbid, j, mpath.at(j));
		}
	}

	//sendid 0 dann standard id reinladen
	if (this->send_gameid == 0)
		this->send_gameid = this->id;
}

//erzeugt ein men�punkt
void Xfire_game::createMenuitem(unsigned int pos, int dbid)
{
	char servicefunction[100];
	mir_strcpy(servicefunction, protocolname);
	mir_strcat(servicefunction, "StartGame%d");

	if (dbid < 0)
		dbid = pos;

	CLISTMENUITEM mi = { 0 };
	mi.hParentMenu = Menu_CreateRoot(MO_MAIN, LPGENW("Start game"), 500084000);

	mir_snprintf(temp, _countof(temp), servicefunction, this->id);
	//wenn die servicefunktion schon exisitert vernichten, hehe
	if (ServiceExists(temp))
		DestroyServiceFunction(temp);
	CreateServiceFunctionParam(temp, StartGame, this->id);
	mi.pszService = temp;
	mi.position = 500090002 + pos;
	mi.icolibItem = this->hicon ? this->hicon : LoadIcon(hinstance, MAKEINTRESOURCE(ID_OP));
	mi.name.a = menuitemtext(this->name);
	this->menuhandle = Menu_AddMainMenuItem(&mi);

	//menu aktualisieren ob hidden
	this->refreshMenuitem();
}

//entfernt men�punkt
void Xfire_game::remoteMenuitem()
{
	if (menuhandle != NULL)
	{
		Menu_RemoveItem(menuhandle);
		menuhandle = NULL;
	}
}

//aktualisiert men�punkt ob hidden
void Xfire_game::refreshMenuitem()
{
	if (menuhandle != NULL)
		Menu_ShowItem(menuhandle, !this->notinstartmenu);
}