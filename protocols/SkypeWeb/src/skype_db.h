/*
Copyright (c) 2015-21 Miranda NG team (https://miranda-ng.org)

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

#ifndef _SKYPE_DB_H_
#define _SKYPE_DB_H_

enum SKYPE_DB_EVENT_TYPE
{
	SKYPE_DB_EVENT_TYPE_ACTION = 10001,
	SKYPE_DB_EVENT_TYPE_INCOMING_CALL,
	SKYPE_DB_EVENT_TYPE_CALL_INFO,
	SKYPE_DB_EVENT_TYPE_FILETRANSFER_INFO,
	SKYPE_DB_EVENT_TYPE_URIOBJ,
	SKYPE_DB_EVENT_TYPE_EDITED_MESSAGE,
	SKYPE_DB_EVENT_TYPE_MOJI,
	SKYPE_DB_EVENT_TYPE_FILE,
	SKYPE_DB_EVENT_TYPE_UNKNOWN
};

#define SKYPE_SETTINGS_ID       "SkypeId"
#define SKYPE_SETTINGS_LOGIN    "Skypename"
#define SKYPE_SETTINGS_USERTYPE "UserType"
#define SKYPE_SETTINGS_PASSWORD "Password"
#define SKYPE_SETTINGS_GROUP    "DefaultGroup"

struct CSkypeOptions
{
	CMOption<BYTE> bAutoHistorySync;
	CMOption<BYTE> bMarkAllAsUnread;

	CMOption<BYTE> bUseHostnameAsPlace;
	CMOption<wchar_t*> wstrPlace;

	CMOption<BYTE> bUseBBCodes;

	CMOption<wchar_t*> wstrCListGroup;

	CSkypeOptions(PROTO_INTERFACE *proto) :

		bAutoHistorySync(proto, "AutoSync", true),
		bMarkAllAsUnread(proto, "MarkMesUnread", true),

		wstrPlace(proto, "Place", L""),
		bUseHostnameAsPlace(proto, "UseHostName", true),

		bUseBBCodes(proto, "UseBBCodes", true),

		wstrCListGroup(proto, SKYPE_SETTINGS_GROUP, L"Skype")
	{
	}

};

#endif //_SKYPE_DB_H_