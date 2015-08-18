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
	Beinhaltet alle Funktionen und Informationen des Spiels
	*/

#ifndef _XFIRE_GAME
#define _XFIRE_GAME

#include "Xfire_base.h"

using namespace std;

//externe funktion die das spielstarten steuern
extern INT_PTR StartGame(WPARAM wParam, LPARAM lParam, LPARAM fParam);

class Xfire_game : public Xfire_base
{
public:
	//id des spiels
	unsigned int m_id;
	//handle des menuitems
	HGENMENU m_menuhandle;
	//spiel soll bei der detection übersprungen werden
	BOOL m_skip;
	//voicehat?
	BOOL m_voicechat;
	//es soll kein icq und co status gesetzt werden
	BOOL m_noicqstatus;
	//handelt es sich um ein "custom" spiel
	BOOL m_custom;
	//soll ein spez. status gesetzt werden
	BOOL m_setstatusmsg;
	//iconhandle von miranda
	HANDLE m_iconhandl;
	//hicon vom icon des spiels
	HICON m_hicon;
	//dont show in startmenü
	BOOL m_notinstartmenu;

	//pfad des spiels wenn es laufen sollte
	char* m_path;
	//startparameter des spiels
	char* m_launchparams;
	//netzwerkparameter
	char* m_networkparams;
	//username parameter
	char* m_userparams;
	//passwort paramter
	char* m_pwparams;
	//mustcontain parameter
	char* m_mustcontain;
	//notcontain parameter
	char* m_notcontain;
	//spielname
	char* m_name;
	//extraparameter
	char* m_extraparams;
	//custom gamename
	char* m_customgamename;
	//custom statusmsg for xfire
	char* m_statusmsg;

	//mehrere pfade
	vector<char*> m_mpath;

	//zu sende gameid
	signed short m_send_gameid;

	//konstruktor
	Xfire_game()
	{
		//vars leer setzen
		m_path = NULL;
		m_launchparams = NULL;
		m_networkparams = NULL;
		m_userparams = NULL;
		m_pwparams = NULL;
		m_mustcontain = NULL;
		m_notcontain = NULL;
		m_name = NULL;
		m_menuhandle = NULL;
		m_iconhandl = NULL;
		m_hicon = NULL;
		m_extraparams = NULL;
		m_customgamename = NULL;
		m_statusmsg = NULL;
		m_id = m_skip = m_noicqstatus = m_custom = m_setstatusmsg = m_send_gameid = m_notinstartmenu = m_voicechat = 0;
	}
	//dekonstruktor
	~Xfire_game()
	{
		//entferne dyn arrays
		delete[] m_path;
		delete[] m_launchparams;
		delete[] m_networkparams;
		delete[] m_userparams;
		delete[] m_pwparams;
		delete[] m_mustcontain;
		delete[] m_notcontain;
		delete[] m_name;
		delete[] m_extraparams;
		delete[] m_customgamename;
		delete[] m_statusmsg;

		for (unsigned int i = 0; i < m_mpath.size(); i++)
			if (m_mpath.at(i) != NULL)
				delete m_mpath.at(i);
		m_mpath.clear();

		remoteMenuitem();
	}

	//funktionen
	void readFromDB(unsigned dbid);
	void writeToDB(unsigned dbid);
	void createMenuitem(unsigned int pos, int dbid = -1);
	void refreshMenuitem();
	void remoteMenuitem();
	void setNameandIcon();
	void setIcon(HICON hicon, HANDLE handle);
	BOOL checkpath(PROCESSENTRY32* processInfo);
	BOOL start_game(char*ip = NULL, unsigned int port = 0, char*pw = NULL);
	BOOL haveExtraGameArgs();
};

#endif