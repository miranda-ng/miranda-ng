/*
 *  Plugin of miranda IM(ICQ) for Communicating with users of the XFire Network.
 *
 *  Copyright (C) 2009 by
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

//Klasse behandelt alle Iconsachen unter Xfire

#ifndef _XFIRE_ICON_MNG
#define _XFIRE_ICON_MNG

#include "Xfire_base.h"
#include <vector>

const char IconsdllName[] = "Icons.dll";

#pragma comment(lib,"ole32.lib")

using namespace std;

//eintrag eines icons
struct Xfire_icon_cache {
	unsigned int gameid;
	HANDLE handle;
	HICON hicon;
};

//icon loader, cache manager
class Xfire_icon_mng : public Xfire_base {
private:
	vector<Xfire_icon_cache> iconcache;
	BOOL getIconfromCache(unsigned int gameid, Xfire_icon_cache* out);
	Xfire_icon_cache LoadGameIcon(unsigned int gameid);
	HANDLE createIconHandle(HICON hicon);
	HICON downloadIcon(char* shortname);

	HINSTANCE hIconDll;
public:
	HANDLE getGameIconHandle(unsigned int gameid);
	HICON getGameIcon(unsigned int gameid);
	unsigned int getGameIconId(unsigned int gameid);
	HICON getGameIconFromId(unsigned int gameid);
	Xfire_icon_cache getGameIconEntry(unsigned int gameid);
	int resetIconHandles();

	Xfire_icon_mng();
	~Xfire_icon_mng();
};

#endif