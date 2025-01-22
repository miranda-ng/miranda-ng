/*
Weather Protocol plugin for Miranda IM
Copyright (C) 2005-2009 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2002-2005 Calvin Che

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef M_WEATHER_H__
#define M_WEATHER_H__ 1

//============  WEATHER CONDITION STRUCT  ============

// weather conditions  (added in v0.1.2.0)
struct WEATHERINFO
{
	MCONTACT hContact;
	TCHAR id[128];
	TCHAR city[128];
	TCHAR update[64];
	TCHAR cond[128];
	TCHAR temp[16];
	TCHAR low[16];
	TCHAR high[16];
	TCHAR feel[16];
	TCHAR wind[16];
	TCHAR winddir[64];
	TCHAR dewpoint[16];
	TCHAR pressure[16];
	TCHAR humid[16];
	TCHAR vis[16];
	TCHAR sunrise[32];
	TCHAR sunset[32];
};

// ===============  WEATHER SERVICES  ================

// Enable or disable weather protocol.
// WPARAM = FALSE to toggle, TRUE to use the LPARAM
// LPARAM = TRUE to enable, FALSE to disable
#define MS_WEATHER_ENABLED		"Weather/EnableDisable"

// Update all weather info
// WPARAM = LPARAM = NULL
#define MS_WEATHER_UPDATEALL	"Weather/UpdateAll"

// Update all weather info + erase the old ones
// WPARAM = LPARAM = NULL
#define MS_WEATHER_REFRESHALL	"Weather/RefreshAll"

// parse the string to turn it to weather display
// WPARAM = (WEATHERINFO*)hContact
// LPARAM = (char*)display_str
#define MS_WEATHER_GETDISPLAY	"Weather/GetDisplay"

#endif //M_WEATHER_H__
