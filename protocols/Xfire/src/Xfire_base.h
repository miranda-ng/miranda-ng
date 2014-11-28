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
	Beinhaltet Basisfunktionen
	*/

#ifndef _XFIRE_BASE
#define _XFIRE_BASE

#include "baseProtocol.h"

extern HANDLE XFireWorkingFolder;
extern HANDLE XFireIconFolder;

#define XFIRE_MAXSIZEOFGAMENAME 80

class Xfire_base {
private:
	//vars
	DBVARIANT dbv;
public:
	char temp[128];
	//funktionen/tools
	void setString(char*from, char**to);
	void appendString(char*from, char**to);
	void readStringfromDB(char*name, unsigned int dbid, char**to);
	void readStringfromDB(char*name, unsigned int dbid, int id, char**to);
	void readUtf8StringfromDB(char*name, unsigned int dbid, char**to);
	void readUtf8StringfromDB(char*name, unsigned int dbid, int id, char**to);
	unsigned char readBytefromDB(char*name, unsigned int dbid, int defaultval = 0);
	unsigned int readWordfromDB(char*name, unsigned int dbid, int defaultval = 0);
	BOOL removeDBEntry(char*name, unsigned int dbid);
	BOOL removeDBEntry(char*name, unsigned int dbid, int id2);
	void writeStringtoDB(char*name, unsigned int dbid, int id, char*val);
	void writeStringtoDB(char*name, unsigned int dbid, char*val);
	void writeUtf8StringtoDB(char*name, unsigned int dbid, int id, char*val);
	void writeUtf8StringtoDB(char*name, unsigned int dbid, char*val);
	void writeBytetoDB(char*name, unsigned int dbid, int val);
	void writeWordtoDB(char*name, unsigned int dbid, int val);
	void strtolower(char*);
	void strtolowerT(TCHAR*);
	void strtoupper(char*);
	void strreplace(char*search, char*replace, char**data);
	BYTE accStringByte(char*);
	BOOL inString(char*str, char*search, char**pos = NULL);
	BOOL wildcmp(const TCHAR *search, const TCHAR *text);
	// ini funktionen
	BOOL getIniValue(unsigned int gameid, const char* valname, char*out, int sizeofout = 255);
	BOOL getIniValue(unsigned int gameid, unsigned int subid, const char* valname, char*out, int sizeofout = 255);
	BOOL getIniPath(char*path);
	BOOL getIconPath(char*path);
	// gamename
	BOOL getGamename(unsigned int gameid, char* out, int outsize = 255);
	//gdi+
	HICON createHICONfromdata(LPVOID data, unsigned int size);
	HBITMAP createHBITMAPfromdata(LPVOID data, unsigned int size);
	//processzeugs
	BOOL isValidPid(DWORD pid);
	BOOL getPidByProcessName(TCHAR *name, DWORD *pid);
};

#endif