/*

Import plugin for Miranda NG

Copyright (C) 2012-14 George Hazan

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

/////////////////////////////////////////////////////////////////////////////////////////
// Creates a group with a specified name in the
// Miranda contact list.
// If contact is specified adds it to group
// ------------------------------------------------
// Returns 1 if successful and 0 when it fails.

int CreateGroup(const TCHAR *group, MCONTACT hContact)
{
	if (group == NULL)
		return 0;

	size_t cbName = _tcslen(group);
	TCHAR *tszGrpName = (TCHAR*)_alloca((cbName + 2)*sizeof(TCHAR));
	tszGrpName[0] = 1 | GROUPF_EXPANDED;
	_tcscpy(tszGrpName + 1, group);

	// Check for duplicate & find unused id
	char groupIdStr[11];
	for (int groupId = 0;; groupId++) {
		itoa(groupId, groupIdStr, 10);
		ptrT tszDbGroup(db_get_tsa(NULL, "CListGroups", groupIdStr));
		if (tszDbGroup == NULL)
			break;

		if (!mir_tstrcmp((TCHAR*)tszDbGroup+1, tszGrpName+1)) {
			if (hContact)
				db_set_ts(hContact, "CList", "Group", tszGrpName + 1);
			else
				AddMessage(LPGENT("Skipping duplicate group %s."), tszGrpName + 1);
			return 0;
		}
	}

	db_set_ts(NULL, "CListGroups", groupIdStr, tszGrpName);

	if (hContact)
		db_set_ts(hContact, "CList", "Group", tszGrpName + 1);

	return 1;
}

static bool IsEqualEvent(const DBEVENTINFO &ev1, const DBEVENTINFO &ev2)
{
	return (ev1.timestamp == ev2.timestamp &&
			  ev1.eventType == ev2.eventType &&
			  ev1.cbBlob == ev2.cbBlob &&
			  (ev1.flags & DBEF_SENT) == (ev2.flags & DBEF_SENT));
}

// Returns TRUE if the event already exist in the database
bool IsDuplicateEvent(MCONTACT hContact, DBEVENTINFO dbei)
{
	static DWORD dwPreviousTimeStamp = -1;
	static MCONTACT hPreviousContact = INVALID_CONTACT_ID;
	static HANDLE hPreviousDbEvent = NULL;

	// get last event
	HANDLE hExistingDbEvent = db_event_last(hContact);
	if (hExistingDbEvent == NULL)
		return FALSE;

	DBEVENTINFO dbeiExisting = { sizeof(dbeiExisting) };
	db_event_get(hExistingDbEvent, &dbeiExisting);
	DWORD dwEventTimeStamp = dbeiExisting.timestamp;

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
		if (!(hExistingDbEvent = db_event_first(hContact)))
			return FALSE;

		memset(&dbeiExisting, 0, sizeof(dbeiExisting));
		dbeiExisting.cbSize = sizeof(dbeiExisting);
		db_event_get(hExistingDbEvent, &dbeiExisting);
		dwEventTimeStamp = dbeiExisting.timestamp;

		// compare with first timestamp
		if (dbei.timestamp <= dwEventTimeStamp) {
			// remember event
			dwPreviousTimeStamp = dwEventTimeStamp;
			hPreviousDbEvent = hExistingDbEvent;
			if (dbei.timestamp != dwEventTimeStamp)
				return FALSE;
		}
	}

	// check for equal timestamps
	if (dbei.timestamp == dwPreviousTimeStamp) {
		memset(&dbeiExisting, 0, sizeof(dbeiExisting));
		dbeiExisting.cbSize = sizeof(dbeiExisting);
		db_event_get(hPreviousDbEvent, &dbeiExisting);

		if (IsEqualEvent(dbei, dbeiExisting))
			return TRUE;

		// find event with another timestamp
		hExistingDbEvent = db_event_next(hContact, hPreviousDbEvent);
		while (hExistingDbEvent != NULL) {
			memset(&dbeiExisting, 0, sizeof(dbeiExisting));
			dbeiExisting.cbSize = sizeof(dbeiExisting);
			db_event_get(hExistingDbEvent, &dbeiExisting);

			if (dbeiExisting.timestamp != dwPreviousTimeStamp) {
				// use found event
				hPreviousDbEvent = hExistingDbEvent;
				dwPreviousTimeStamp = dbeiExisting.timestamp;
				break;
			}

			hPreviousDbEvent = hExistingDbEvent;
			hExistingDbEvent = db_event_next(hContact, hExistingDbEvent);
		}
	}

	hExistingDbEvent = hPreviousDbEvent;

	if (dbei.timestamp <= dwPreviousTimeStamp) {
		// look back
		while (hExistingDbEvent != NULL) {
			memset(&dbeiExisting, 0, sizeof(dbeiExisting));
			dbeiExisting.cbSize = sizeof(dbeiExisting);
			db_event_get(hExistingDbEvent, &dbeiExisting);

			if (dbei.timestamp > dbeiExisting.timestamp) {
				// remember event
				hPreviousDbEvent = hExistingDbEvent;
				dwPreviousTimeStamp = dbeiExisting.timestamp;
				return FALSE;
			}

			// Compare event with import candidate
			if (IsEqualEvent(dbei, dbeiExisting)) {
				// remember event
				hPreviousDbEvent = hExistingDbEvent;
				dwPreviousTimeStamp = dbeiExisting.timestamp;
				return TRUE;
			}

			// Get previous event in chain
			hExistingDbEvent = db_event_prev(hContact, hExistingDbEvent);
		}
	}
	else {
		// look forward
		while (hExistingDbEvent != NULL) {
			memset(&dbeiExisting, 0, sizeof(dbeiExisting));
			dbeiExisting.cbSize = sizeof(dbeiExisting);
			db_event_get(hExistingDbEvent, &dbeiExisting);

			if (dbei.timestamp < dbeiExisting.timestamp) {
				// remember event
				hPreviousDbEvent = hExistingDbEvent;
				dwPreviousTimeStamp = dbeiExisting.timestamp;
				return FALSE;
			}

			// Compare event with import candidate
			if (IsEqualEvent(dbei, dbeiExisting)) {
				// remember event
				hPreviousDbEvent = hExistingDbEvent;
				dwPreviousTimeStamp = dbeiExisting.timestamp;
				return TRUE;
			}

			// Get next event in chain
			hExistingDbEvent = db_event_next(hContact, hExistingDbEvent);
		}
	}
	// reset last event
	hPreviousContact = INVALID_CONTACT_ID;
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// icons

static IconItem iconList[] =
{
	{ LPGEN("Import..."), "import_main", IDI_IMPORT }
};

HICON GetIcon(int iIconId)
{
	for (int i = 0; i < SIZEOF(iconList); i++)
		if (iconList[i].defIconID == iIconId)
			return Skin_GetIconByHandle(iconList[i].hIcolib);

	return NULL;
}

HANDLE GetIconHandle(int iIconId)
{
	for (int i = 0; i < SIZEOF(iconList); i++)
		if (iconList[i].defIconID == iIconId)
			return iconList[i].hIcolib;

	return NULL;
}

void RegisterIcons()
{
	Icon_Register(hInst, "Import", iconList, SIZEOF(iconList));
}
