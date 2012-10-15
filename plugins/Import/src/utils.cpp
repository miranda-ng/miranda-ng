/*

Import plugin for Miranda NG

Copyright (C) 2012 George Hazan

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "import.h"
#include "version.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////////////////

BOOL IsProtocolLoaded(char* pszProtocolName)
{
	return CallService(MS_PROTO_ISPROTOCOLLOADED, 0, (LPARAM)pszProtocolName) ? TRUE : FALSE;
}

// ------------------------------------------------
// Creates a group with a specified name in the
// Miranda contact list.
// If contact is specified adds it to group
// ------------------------------------------------
// Returns 1 if successful and 0 when it fails.
int CreateGroup(const TCHAR* group, HANDLE hContact)
{
	if (group == NULL)
		return 0;

	size_t cbName = _tcslen(group);
	TCHAR *tszGrpName = (TCHAR*)_alloca(( cbName+2 )*sizeof( TCHAR ));
	tszGrpName[0] = 1 | GROUPF_EXPANDED;
	_tcscpy(tszGrpName+1, group);

	// Check for duplicate & find unused id
	char groupIdStr[11];
	for (int groupId = 0; ; groupId++) {
		DBVARIANT dbv;
		itoa(groupId, groupIdStr,10);
		if (DBGetContactSettingTString(NULL, "CListGroups", groupIdStr, &dbv))
			break;

		if ( !lstrcmp(dbv.ptszVal + 1, tszGrpName + 1 )) {
			if (hContact)
				DBWriteContactSettingTString( hContact, "CList", "Group", tszGrpName+1 );
			else
				AddMessage( LPGENT("Skipping duplicate group %s."), tszGrpName + 1);

			DBFreeVariant(&dbv);
			return 0;
		}

		DBFreeVariant(&dbv);
	}

	DBWriteContactSettingTString( NULL, "CListGroups", groupIdStr, tszGrpName );

	if (hContact)
		DBWriteContactSettingTString( hContact, "CList", "Group", tszGrpName+1 );

	return 1;
}

// Returns TRUE if the event already exist in the database
BOOL IsDuplicateEvent(HANDLE hContact, DBEVENTINFO dbei)
{
	static DWORD dwPreviousTimeStamp = -1;
	static HANDLE hPreviousContact = INVALID_HANDLE_VALUE;
	static HANDLE hPreviousDbEvent = NULL;

	HANDLE hExistingDbEvent;
	DWORD dwEventTimeStamp;
	DBEVENTINFO dbeiExisting;

	// get last event
	if (!(hExistingDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDLAST, (WPARAM)hContact, 0)))
		return FALSE;

	ZeroMemory(&dbeiExisting, sizeof(dbeiExisting));
	dbeiExisting.cbSize = sizeof(dbeiExisting);
	CallService(MS_DB_EVENT_GET, (WPARAM)hExistingDbEvent, (LPARAM)&dbeiExisting);
	dwEventTimeStamp = dbeiExisting.timestamp;

	// compare with last timestamp
	if (dbei.timestamp > dwEventTimeStamp) {
		// remember event
		hPreviousDbEvent = hExistingDbEvent;
		dwPreviousTimeStamp = dwEventTimeStamp;
		return FALSE;
	}

	if (hContact != hPreviousContact) {
		hPreviousContact = hContact;
		// remember event
		hPreviousDbEvent = hExistingDbEvent;
		dwPreviousTimeStamp = dwEventTimeStamp;

		// get first event
		if (!(hExistingDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDFIRST, (WPARAM)hContact, 0)))
			return FALSE;

		ZeroMemory(&dbeiExisting, sizeof(dbeiExisting));
		dbeiExisting.cbSize = sizeof(dbeiExisting);
		CallService(MS_DB_EVENT_GET, (WPARAM)hExistingDbEvent, (LPARAM)&dbeiExisting);
		dwEventTimeStamp = dbeiExisting.timestamp;

		// compare with first timestamp
		if (dbei.timestamp <= dwEventTimeStamp) {
			// remember event
			dwPreviousTimeStamp = dwEventTimeStamp;
			hPreviousDbEvent = hExistingDbEvent;

			if ( dbei.timestamp != dwEventTimeStamp )
				return FALSE;
		}
	}

	// check for equal timestamps
	if (dbei.timestamp == dwPreviousTimeStamp) {
		ZeroMemory(&dbeiExisting, sizeof(dbeiExisting));
		dbeiExisting.cbSize = sizeof(dbeiExisting);
		CallService(MS_DB_EVENT_GET, (WPARAM)hPreviousDbEvent, (LPARAM)&dbeiExisting);

		if ((dbei.timestamp == dbeiExisting.timestamp) &&
			(dbei.eventType == dbeiExisting.eventType) &&
			(dbei.cbBlob == dbeiExisting.cbBlob) &&
			((dbei.flags&DBEF_SENT) == (dbeiExisting.flags&DBEF_SENT)))
			return TRUE;

		// find event with another timestamp
		hExistingDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDNEXT, (WPARAM)hPreviousDbEvent, 0);
		while (hExistingDbEvent != NULL) {
			ZeroMemory(&dbeiExisting, sizeof(dbeiExisting));
			dbeiExisting.cbSize = sizeof(dbeiExisting);
			CallService(MS_DB_EVENT_GET, (WPARAM)hExistingDbEvent, (LPARAM)&dbeiExisting);

			if (dbeiExisting.timestamp != dwPreviousTimeStamp) {
				// use found event
				hPreviousDbEvent = hExistingDbEvent;
				dwPreviousTimeStamp = dbeiExisting.timestamp;
				break;
			}

			hPreviousDbEvent = hExistingDbEvent;
			hExistingDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDNEXT, (WPARAM)hExistingDbEvent, 0);
		}
	}

	hExistingDbEvent = hPreviousDbEvent;

	if (dbei.timestamp <= dwPreviousTimeStamp) {
		// look back
		while (hExistingDbEvent != NULL) {
			ZeroMemory(&dbeiExisting, sizeof(dbeiExisting));
			dbeiExisting.cbSize = sizeof(dbeiExisting);
			CallService(MS_DB_EVENT_GET, (WPARAM)hExistingDbEvent, (LPARAM)&dbeiExisting);

			if (dbei.timestamp > dbeiExisting.timestamp) {
				// remember event
				hPreviousDbEvent = hExistingDbEvent;
				dwPreviousTimeStamp = dbeiExisting.timestamp;
				return FALSE;
			}

			// Compare event with import candidate
			if ((dbei.timestamp == dbeiExisting.timestamp) &&
				(dbei.eventType == dbeiExisting.eventType) &&
				(dbei.cbBlob == dbeiExisting.cbBlob) &&
				((dbei.flags & DBEF_SENT) == (dbeiExisting.flags & DBEF_SENT)))
			{
				// remember event
				hPreviousDbEvent = hExistingDbEvent;
				dwPreviousTimeStamp = dbeiExisting.timestamp;
				return TRUE;
			}

			// Get previous event in chain
			hExistingDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDPREV, (WPARAM)hExistingDbEvent, 0);
		}
	}
	else {
		// look forward
		while (hExistingDbEvent != NULL) {
			ZeroMemory(&dbeiExisting, sizeof(dbeiExisting));
			dbeiExisting.cbSize = sizeof(dbeiExisting);
			CallService(MS_DB_EVENT_GET, (WPARAM)hExistingDbEvent, (LPARAM)&dbeiExisting);

			if (dbei.timestamp < dbeiExisting.timestamp) {
				// remember event
				hPreviousDbEvent = hExistingDbEvent;
				dwPreviousTimeStamp = dbeiExisting.timestamp;
				return FALSE;
			}

			// Compare event with import candidate
			if ((dbei.timestamp == dbeiExisting.timestamp) &&
				(dbei.eventType == dbeiExisting.eventType) &&
				(dbei.cbBlob == dbeiExisting.cbBlob) &&
				((dbei.flags&DBEF_SENT) == (dbeiExisting.flags&DBEF_SENT)))
			{
				// remember event
				hPreviousDbEvent = hExistingDbEvent;
				dwPreviousTimeStamp = dbeiExisting.timestamp;
				return TRUE;
			}

			// Get next event in chain
			hExistingDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDNEXT, (WPARAM)hExistingDbEvent, 0);
		}
	}
	// reset last event
	hPreviousContact = INVALID_HANDLE_VALUE;
	return FALSE;
}
