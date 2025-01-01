/*
Copyright (c) 2015-25 Miranda NG team (https://miranda-ng.org)

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

struct {
	int type;
	char *name;
	int flags;
}
static g_SkypeDBTypes[] =
{
	{ SKYPE_DB_EVENT_TYPE_INCOMING_CALL, LPGEN("Incoming call"), DETF_NONOTIFY },
	{ SKYPE_DB_EVENT_TYPE_ACTION, LPGEN("Action"), 0 },
	{ SKYPE_DB_EVENT_TYPE_CALL_INFO, LPGEN("Call information"), 0 },
	{ SKYPE_DB_EVENT_TYPE_FILETRANSFER_INFO, LPGEN("File transfer information"), 0 },
	{ SKYPE_DB_EVENT_TYPE_MOJI, LPGEN("Moji"), 0 },
	{ SKYPE_DB_EVENT_TYPE_UNKNOWN, LPGEN("Unknown event"), 0 },
};

void CSkypeProto::InitDBEvents()
{
	// custom event
	DBEVENTTYPEDESCR dbEventType = {};
	dbEventType.module = m_szModuleName;
	dbEventType.flags = DETF_HISTORY | DETF_MSGWINDOW;
	dbEventType.iconService = MODULE "/GetEventIcon";
	dbEventType.textService = MODULE "/GetEventText";

	for (auto &cur : g_SkypeDBTypes) {
		dbEventType.eventType = cur.type;
		dbEventType.descr = Translate(cur.name);
		dbEventType.flags |= cur.flags;

		DbEvent_RegisterType(&dbEventType);

		dbEventType.flags &= (~cur.flags);
	}
}
