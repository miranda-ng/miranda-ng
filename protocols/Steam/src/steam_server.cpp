/*
Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

void __cdecl CSteamProto::ServerThread(void *)
{
	// load web socket servers first if needed
	int iTimeDiff = db_get_dw(0, STEAM_MODULE, DBKEY_HOSTS_DATE);
	if (!db_get_dw(0, STEAM_MODULE, DBKEY_HOSTS_COUNT) || time(0) - iTimeDiff > 3600 * 24 * 7) { // once a week
		// if (!SendRequest())
	}
}
