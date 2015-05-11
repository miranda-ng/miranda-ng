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

#ifndef _XFIRE_AVATAR_LOADER
#define _XFIRE_AVATAR_LOADER

#include "client.h"
#include "Xfire_base.h"
#include "getbuddyinfo.h"
#include <vector>

using namespace xfirelib;

struct Xfire_avatar_process {
	MCONTACT hcontact;
	char username[128];
	unsigned int userid;
};

class Xfire_avatar_loader : public Xfire_base {
private:
	static void loadThread(LPVOID lparam);
	xfirelib::Client *client;
	mir_cs avatarMutex;
public:
	vector<Xfire_avatar_process> list;
	BOOL threadrunning;
	BOOL loadAvatar(MCONTACT hcontact, char*username, unsigned int userid);
	Xfire_avatar_loader(xfirelib::Client* client);
	~Xfire_avatar_loader();
};

#endif