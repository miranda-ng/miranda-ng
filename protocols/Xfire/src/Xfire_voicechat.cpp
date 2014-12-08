#include "stdafx.h"
#include "Xfire_voicechat.h"

//konstruktor
Xfire_voicechat::Xfire_voicechat() {
	this->resetCurrentvoicestatus();
	ipport = NULL;
	tsrDLL = NULL;
	tsrGetServerInfo = NULL;
	pid = 0;
}

//dekonstruktor
Xfire_voicechat::~Xfire_voicechat()
{
	//geladene tsr remote dll freigeben
	if (tsrDLL) {
		FreeLibrary(tsrDLL);
		tsrDLL = NULL;
	}
}

//init
void Xfire_voicechat::initVoicechat()
{
	//tsremotedll laden
	tsrDLL = this->loadTSR();
}

//prüft ob das paket schonmal versendet wurde, soll unnötigen nwtraffic reduzieren, *ÜBERLEGUNG* ob wirklich notwendig
BOOL Xfire_voicechat::alreadySend(SendGameStatus2Packet* packet) {
	if (packet == NULL)
		return FALSE;

	if (packet->ip[3] != lastpacket.ip[3] ||
		packet->ip[2] != lastpacket.ip[2] ||
		packet->ip[1] != lastpacket.ip[1] ||
		packet->ip[0] != lastpacket.ip[0] ||
		packet->port != lastpacket.port) {
		lastpacket = *packet;
		return TRUE;
	}

	return FALSE;
}

//prüft nach laufenden voicechat anwendungen
BOOL Xfire_voicechat::checkVoicechat(SendGameStatus2Packet* packet) {
	//kein gültiger verweis?
	if (packet == NULL)
		return FALSE;
	//jeh nach letzten status handeln
	switch (currentvoice) {
	case XFIREVOICECHAT_TS2:
		if (checkforTS2(packet)) {
			return alreadySend(packet);
		}
		else
		{
			//kein ts2 mehr? dann paket restten
			resetSendGameStatus2Packet(packet);
			resetCurrentvoicestatus();
			//in db schreiben
			writeToDatabase(packet);
			return TRUE;
		}
		break;
	case XFIREVOICECHAT_TS3:
		if (checkforTS3(packet)) {
			return alreadySend(packet);
		}
		else
		{
			//kein ts3 mehr? dann paket restten
			resetSendGameStatus2Packet(packet);
			resetCurrentvoicestatus();
			//in db schreiben
			writeToDatabase(packet);
			return TRUE;
		}
		break;
	case XFIREVOICECHAT_MUMBLE:
		if (checkforMumble(packet)) {
			return alreadySend(packet);
		}
		else
		{
			//kein mumble mehr? dann paket restten
			resetSendGameStatus2Packet(packet);
			resetCurrentvoicestatus();
			//in db schreiben
			writeToDatabase(packet);
			return TRUE;
		}
		break;
	default:
		//prüfe nach ts3
		if (checkforTS3(packet)) {
			return alreadySend(packet);
		}
		//prüfe nach ts2
		else if (checkforTS2(packet)) {
			return alreadySend(packet);
		}
		//prüfe nach mumble
		else if (checkforMumble(packet)) {
			return alreadySend(packet);
		}
		break;
	};

	return FALSE;
}

//setzte currentvoice auf 0 zurück, falls es einen disconnect gab
void Xfire_voicechat::resetCurrentvoicestatus() {
	currentvoice = XFIREVOICECHAT_NOVOICE;
	lastpacket.ip[3] = 0;
	lastpacket.ip[2] = 0;
	lastpacket.ip[1] = 0;
	lastpacket.ip[0] = 0;
	lastpacket.port = 0;
	pid = 0;
}

//resettet das packet auf 0
void Xfire_voicechat::resetSendGameStatus2Packet(SendGameStatus2Packet* packet) {
	if (packet == NULL)
		return;
	//voiceid
	packet->gameid = XFIREVOICECHAT_NOVOICE;
	//ip zuweisen
	packet->ip[3] = 0;
	packet->ip[2] = 0;
	packet->ip[1] = 0;
	packet->ip[0] = 0;
	//port zuweisen
	packet->port = 0;
}

//schreibt derzetigen status in die mirandadb für variables usw
void Xfire_voicechat::writeToDatabase(SendGameStatus2Packet* packet) {
	//für sprintf
	char temp[32];

	if (packet == NULL || packet->gameid == XFIREVOICECHAT_NOVOICE) {
		//einträge aus der db entfernen
		db_unset(NULL, protocolname, "VServerIP");
		db_unset(NULL, protocolname, "currentvoicename");
		//zurück
		return;
	}
	//ip speichern
	mir_snprintf(temp, SIZEOF(temp), "%d.%d.%d.%d:%d", (unsigned char)packet->ip[3], (unsigned char)packet->ip[2], (unsigned char)packet->ip[1], (unsigned char)packet->ip[0], packet->port);
	db_set_s(NULL, protocolname, "VServerIP", temp);
	//namen jeh nach id schreiben
	switch (packet->gameid) {
	case XFIREVOICECHAT_TS3:
		db_set_s(NULL, protocolname, "currentvoicename", "Teamspeak 3");
		break;
	case XFIREVOICECHAT_TS2:
		db_set_s(NULL, protocolname, "currentvoicename", "Teamspeak 2");
		break;
	case XFIREVOICECHAT_MUMBLE:
		db_set_s(NULL, protocolname, "currentvoicename", "Mumble");
		break;
	case XFIREVOICECHAT_VENTRILO:
		db_set_s(NULL, protocolname, "currentvoicename", "Ventrilo");
		break;
	};
}

//versucht die TSR zuladen
HMODULE Xfire_voicechat::loadTSR(char* path, BOOL nolocaltest) {
	TCHAR pathtotsr[MAX_PATH] = _T("");

	/*if (path)
		; was tun*/
	_tcscat_s(pathtotsr, MAX_PATH, _T("TSRemote.dll"));

	//versuche dll zuladen
	HMODULE tsrDLL = LoadLibrary(pathtotsr);
	//konnte nicht geladen werden 
	if (!tsrDLL)
	{
		XFireLog("TSRemote.dll load failed!");

		//bei keinem lokalen test abbruch
		if (nolocaltest) return NULL;

		//nochmal engl. lokal versuchen
		tsrDLL = LoadLibrary(_T("C:\\Program Files\\Teamspeak2_RC2\\client_sdk\\TSRemote.dll"));

		if (!tsrDLL) {
			XFireLog("TSRemote.dll load faild (using standard installationpath)!");

			//deutsches sys?
			tsrDLL = LoadLibrary(_T("C:\\Programme\\Teamspeak2_RC2\\client_sdk\\TSRemote.dll"));

			if (!tsrDLL)
				XFireLog("TSRemote.dll load failed (using standard installationpath2)!");

			//aufgeben
			return NULL;
		}
	}

	XFireLog("TSRemote.dll successfully loaded!");

	//getserverinfo funktion holen
	tsrGetServerInfo = (LPtsrGetServerInfo)GetProcAddress(tsrDLL, "tsrGetServerInfo");

	return tsrDLL;
}


//teamspeak 3 detection, benötigt ts3plugin
BOOL Xfire_voicechat::checkforTS3(SendGameStatus2Packet* packet) {
	ts3IPPORT* ipport = NULL;
	//kein gültiger verweis?
	if (packet == NULL)
		return FALSE;
	//existiert ein filemap?
	HANDLE hMapObject = OpenFileMappingA(FILE_MAP_READ, FALSE, "$ts3info4xfire$");
	//nicht gefunden, dann kein ts3
	if (hMapObject == NULL)
		return FALSE;
	//versuch ipport zubesorgen
	ipport = (ts3IPPORT *)MapViewOfFile(hMapObject, FILE_MAP_READ, 0, 0, sizeof(ts3IPPORT));
	//fehler beim zugriff auf filemap?
	if (ipport == NULL)
	{
		CloseHandle(hMapObject);
		return FALSE;
	}

	//wenn kein port, dann stimmt was mit der ip nicht, paket resetten
	if (ipport->port == 0) {
		//packet resetten
		resetSendGameStatus2Packet(packet);
		//in db schreiben
		writeToDatabase(packet);
		return TRUE;
	}

	//voiceid zuweisen
	this->currentvoice = XFIREVOICECHAT_TS3;
	packet->gameid = XFIREVOICECHAT_TS3;
	//ip zuweisen
	packet->ip[3] = ipport->ip[3];
	packet->ip[2] = ipport->ip[2];
	packet->ip[1] = ipport->ip[1];
	packet->ip[0] = ipport->ip[0];
	//port zuweisen
	packet->port = ipport->port;
	//unmap, handle schließem
	UnmapViewOfFile(ipport);
	CloseHandle(hMapObject);
	//in db schreiben
	writeToDatabase(packet);
	//ts3 gefunden
	return TRUE;
}

//teamspeak 2 detection mit hilfe der tsr
BOOL Xfire_voicechat::checkforTS2(SendGameStatus2Packet* packet) {
	TtsrServerInfo serverinfo = { 0 };

	//get funktion ist nicht initialisiert
	if (this->tsrGetServerInfo == NULL || packet == NULL)
	{
		return FALSE;
	}

	//infos holen
	this->tsrGetServerInfo(&serverinfo);

	//auswerten wenn serverip gesetzt
	if (serverinfo.ServerIp[0] != 0)
	{
		char * pos = strrchr(serverinfo.ServerIp, ':');
		if (pos == 0)
		{
			return FALSE;
		}

		*pos = 0;
		unsigned int ip = inet_addr(serverinfo.ServerIp);
		pos++;
		int port = atoi(pos);

		//port zuweisen
		packet->port = port;
		//ip zuweisen
		packet->ip[3] = LOBYTE(LOWORD(ip));
		packet->ip[2] = HIBYTE(LOWORD(ip));
		packet->ip[1] = LOBYTE(HIWORD(ip));
		packet->ip[0] = HIBYTE(HIWORD(ip));
		//gameid/voice zuweisen
		packet->gameid = XFIREVOICECHAT_TS2;
		this->currentvoice = XFIREVOICECHAT_TS2;
		//ab in die db
		writeToDatabase(packet);

		return TRUE;
	}

	return FALSE;
}

//detection für mumble
BOOL Xfire_voicechat::checkforMumble(SendGameStatus2Packet* packet) {
	//kein gültiger verweis?
	if (packet == NULL)
		return FALSE;

	//gültige pid
	if (this->pid != 0 && !this->isValidPid(this->pid))
	{
		this->pid = 0;
		return FALSE;
	}
	else {
		if (!this->getPidByProcessName(_T("mumble.exe"), &this->pid)) {
			return FALSE;
		}
	}

	DWORD size = 0;
	MIB_TCPTABLE_OWNER_PID* ptab = NULL;
	//tcptabelle holen
	GetExtendedTcpTable(NULL, &size, FALSE, AF_INET, TCP_TABLE_OWNER_PID_CONNECTIONS, 0);
	//überhaupt was drin?
	if (size) {
		ptab = (MIB_TCPTABLE_OWNER_PID*)malloc(size);
		//liste auslesen
		if (GetExtendedTcpTable(ptab, &size, FALSE, AF_INET, TCP_TABLE_OWNER_PID_CONNECTIONS, 0) == NO_ERROR)
		{
			for (unsigned int i = 0; i < ptab->dwNumEntries; i++)
			{
				if (ptab->table[i].dwOwningPid == this->pid && ptab->table[i].dwLocalAddr != ptab->table[i].dwRemoteAddr) //verbindung gefunden, hoffentlich
				{
					unsigned char*rip = (unsigned char*)&ptab->table[i].dwRemoteAddr;
					XFireLog("IP %x,%x", ptab->table[i].dwRemoteAddr, ptab->table[i].dwRemotePort);
					//ipzuweisen
					packet->ip[3] = rip[0];
					packet->ip[2] = rip[1];
					packet->ip[1] = rip[2];
					packet->ip[0] = rip[3];
					//portzuweisen
					packet->port = r(ptab->table[i].dwRemotePort);
					//mumble
					packet->gameid = XFIREVOICECHAT_MUMBLE;
					this->currentvoice = XFIREVOICECHAT_MUMBLE;
					//table wieder freigeben
					delete ptab;
					//mumble läuft + ip gefunden also TRUE
					return TRUE;
				}
			}
		}
		delete ptab;
	}

	return FALSE;
}