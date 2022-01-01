/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-17 Robert Pösel, 2017-22 Miranda NG team

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

#define MODULENAME "Facebook"

// Contact DB keys
#define DBKEY_LOGIN              "Email"
#define DBKEY_ID                 "ID"
#define DBKEY_SID                "SID"
#define DBKEY_NICK               "Nick"
#define DBKEY_PASS               "Password"
#define DBKEY_CLIENT_ID          "ClientID"
#define DBKEY_DEVICE_ID          "DeviceID"
#define DBKEY_AVATAR             "Avatar"
#define DBKEY_CONTACT_TYPE       "ContactType"
#define DBKEY_TOKEN              "Token"
#define DBKEY_SYNC_TOKEN         "SyncToken"

// Account DB keys
#define DBKEY_SET_MIRANDA_STATUS "SetMirandaStatus"

// Hidden account DB keys (can't be changed through GUI)
#define DBKEY_LOCALE "Locale" // [HIDDEN] - (string) en_US, cs_CZ, etc. (requires restart to apply)
