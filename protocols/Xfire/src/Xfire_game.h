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
#include <vector>

using namespace std;

//externe funktion die das spielstarten steuern
extern INT_PTR StartGame(WPARAM wParam, LPARAM lParam, LPARAM fParam);

class Xfire_game : public Xfire_base {
public:
	//id des spiels
	unsigned int id;
	//handle des menuitems
	HGENMENU menuhandle;
	//spiel soll bei der detection übersprungen werden
	BOOL skip;
	//voicehat?
	BOOL voicechat;
	//es soll kein icq und co status gesetzt werden
	BOOL noicqstatus;
	//handelt es sich um ein "custom" spiel
	BOOL custom;
	//soll ein spez. status gesetzt werden
	BOOL setstatusmsg;
	//iconhandle von miranda
	HANDLE iconhandl;
	//hicon vom icon des spiels
	HICON hicon;
	//dont show in startmenü
	BOOL notinstartmenu;

	//pfad des spiels wenn es laufen sollte
	char* path;
	//startparameter des spiels
	char* launchparams;
	//netzwerkparameter
	char* networkparams;
	//username parameter
	char* userparams;
	//passwort paramter
	char* pwparams;
	//mustcontain parameter
	char* mustcontain;
	//notcontain parameter
	char* notcontain;
	//spielname
	char* name;
	//extraparameter
	char* extraparams;
	//custom gamename
	char* customgamename;
	//custom statusmsg for xfire
	char* statusmsg;

	//mehrere pfade
	vector<char*> mpath;

	//zu sende gameid
	signed short send_gameid;

	//konstruktor
	Xfire_game() {
		//vars leer setzen
		path = NULL;
		launchparams = NULL;
		networkparams = NULL;
		userparams = NULL;
		pwparams = NULL;
		mustcontain = NULL;
		notcontain = NULL;
		name = NULL;
		menuhandle = NULL;
		iconhandl = NULL;
		hicon = NULL;
		extraparams = NULL;
		customgamename = NULL;
		statusmsg = NULL;
		id = skip = noicqstatus = custom = setstatusmsg = send_gameid = notinstartmenu = voicechat = 0;
	}
	//dekonstruktor
	~Xfire_game() {
		//entferne dyn arrays
		if (path)
		{
			delete[] path;
			path = NULL;
		}
		if (launchparams)
		{
			delete[] launchparams;
			launchparams = NULL;
		}
		if (networkparams)
		{
			delete[] networkparams;
			networkparams = NULL;
		}
		if (userparams)
		{
			delete[] userparams;
			userparams = NULL;
		}
		if (pwparams)
		{
			delete[] pwparams;
			pwparams = NULL;
		}
		if (mustcontain)
		{
			delete[] mustcontain;
			mustcontain = NULL;
		}
		if (notcontain)
		{
			delete[] notcontain;
			notcontain = NULL;
		}
		if (name)
		{
			delete[] name;
			name = NULL;
		}
		if (extraparams)
		{
			delete[] extraparams;
			extraparams = NULL;
		}
		if (customgamename)
		{
			delete[] customgamename;
			customgamename = NULL;
		}
		if (statusmsg)
		{
			delete[] statusmsg;
			statusmsg = NULL;
		}

		for (unsigned int i = 0; i < mpath.size(); i++) {
			if (mpath.at(i) != NULL)
				delete mpath.at(i);
		}
		mpath.clear();

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