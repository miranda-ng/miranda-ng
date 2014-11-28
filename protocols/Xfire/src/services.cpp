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
 *  Based on J. Lawler              - BaseProtocol
 *			 Herbert Poul/Beat Wolf - xfirelib
 *
 *  Miranda ICQ: the free icq client for MS Windows
 *  Copyright (C) 2000-2008  Richard Hughes, Roland Rabien & Tristan Van de Vreede
 *
 */

#include "stdafx.h"

#include "baseProtocol.h"

BOOL IsContactMySelf(std::string buddyusername) {
	DBVARIANT dbv;

	//nur wenn option aktiv, sonst immer FALSE
	if (!db_get_b(NULL, protocolname, "skipmyself", 0))
		return FALSE;

	if (!db_get(NULL, protocolname, "login", &dbv))
	{
		if (!lstrcmpiA(dbv.pszVal, buddyusername.c_str()))
		{
			db_free(&dbv);
			return TRUE;
		}
		db_free(&dbv);
	}
	else
		return FALSE;

	return FALSE;
}

//liefert vollendateipfad vom eigenen avatar zurück, wenn definiert
INT_PTR GetMyAvatar(WPARAM wparam, LPARAM lparam) {
	DBVARIANT dbv;

	if (!db_get(NULL, protocolname, "MyAvatarFile", &dbv))
	{
		strncpy((char*)wparam, dbv.pszVal, (int)lparam);
		db_free(&dbv);
		return 0;
	}

	return -1;
}

//liefert vollendateipfad vom eigenen avatar zurück, wenn definiert
int mBotNotify(WPARAM wparam, LPARAM lparam) {

	if (wparam) {
		CallService(MBOT_TRIGGER, (WPARAM)"xfireingame", 1);
	}
	else
	{
		CallService(MBOT_TRIGGER, (WPARAM)"xfireingame", 0);
	}

	return 0;
}