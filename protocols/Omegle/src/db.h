/*

Omegle plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-15 Robert Pösel

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once

// DB macros
#define getU8String( setting, dest )        db_get_utf( NULL, m_szModuleName, setting, dest )
#define setU8String( setting, value )       db_set_utf( NULL, m_szModuleName, setting, value )
// DB settings
#define OMEGLE_KEY_TIMEOUTS_LIMIT	"TimeoutsLimit" // [HIDDEN]

#define OMEGLE_KEY_ASL				"MessageAsl"
#define OMEGLE_KEY_HI				"MessageHi"
#define OMEGLE_KEY_HI_ENABLED		"MessageHiEnabled"
#define OMEGLE_KEY_NAME				"Nick"
#define OMEGLE_KEY_DONT_STOP		"DontStop"
#define OMEGLE_KEY_NO_CLEAR			"NoClear"
#define OMEGLE_KEY_MEET_COMMON		"MeetCommon"
#define OMEGLE_KEY_INTERESTS		"Interests"
#define OMEGLE_KEY_REUSE_QUESTION	"ReuseQuestion"
#define OMEGLE_KEY_SERVER			"Server"
#define OMEGLE_KEY_LANGUAGE         "Language"
#define OMEGLE_KEY_SERVER_INFO		"GetServerInfo"
#define OMEGLE_KEY_LAST_QUESTION	"LastQuestion"
#define OMEGLE_KEY_AUTO_CONNECT		"AutoConnect"
