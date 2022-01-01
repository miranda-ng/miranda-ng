/*

Import plugin for Miranda NG

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

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

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Creates a group with a specified name in the
// Miranda contact list.
// If contact is specified adds it to group
// ------------------------------------------------
// Returns 1 if successful and 0 when it fails.

int CreateGroup(const wchar_t *group, MCONTACT hContact)
{
	if (group == nullptr)
		return 0;

	if (Clist_GroupExists(group)) {
		if (hContact)
			db_set_ws(hContact, "CList", "Group", group);
		else
			AddMessage(LPGENW("Skipping duplicate group %s."), group);
		return 0;
	}

	Clist_GroupCreate(NULL, group);
	if (hContact)
		db_set_ws(hContact, "CList", "Group", group);
	return 1;
}

// Returns TRUE if the event already exist in the database
bool IsDuplicateEvent(MCONTACT hContact, DBEVENTINFO dbei)
{
	static uint32_t dwPreviousTimeStamp = -1;
	static MCONTACT hPreviousContact = INVALID_CONTACT_ID;
	static MEVENT hPreviousDbEvent = NULL;

	// get last event
	MEVENT hExistingDbEvent = db_event_last(hContact);
	if (hExistingDbEvent == NULL)
		return FALSE;

	DBEVENTINFO dbeiExisting = {};
	db_event_get(hExistingDbEvent, &dbeiExisting);
	uint32_t dwEventTimeStamp = dbeiExisting.timestamp;

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
		db_event_get(hPreviousDbEvent, &dbeiExisting);

		if (dbei == dbeiExisting)
			return TRUE;

		// find event with another timestamp
		hExistingDbEvent = db_event_next(hContact, hPreviousDbEvent);
		while (hExistingDbEvent != NULL) {
			memset(&dbeiExisting, 0, sizeof(dbeiExisting));
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
			db_event_get(hExistingDbEvent, &dbeiExisting);

			if (dbei.timestamp > dbeiExisting.timestamp) {
				// remember event
				hPreviousDbEvent = hExistingDbEvent;
				dwPreviousTimeStamp = dbeiExisting.timestamp;
				return FALSE;
			}

			// Compare event with import candidate
			if (dbei == dbeiExisting) {
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
			db_event_get(hExistingDbEvent, &dbeiExisting);

			if (dbei.timestamp < dbeiExisting.timestamp) {
				// remember event
				hPreviousDbEvent = hExistingDbEvent;
				dwPreviousTimeStamp = dbeiExisting.timestamp;
				return FALSE;
			}

			// Compare event with import candidate
			if (dbei == dbeiExisting) {
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
// rtl integers

uint32_t RLInteger(const uint8_t *p)
{
	uint32_t ret = 0;
	for (int i = 0; i < 4; i++) {
		ret <<= 8;
		ret += p[i];
	}
	return ret;
}

uint32_t RLWord(const uint8_t *p)
{
	return (p[0] << 8) + p[1];
}

/////////////////////////////////////////////////////////////////////////////////////////
// icons

static IconItem iconList[] =
{
	{ LPGEN("Import..."), "import_main", IDI_IMPORT }
};

void RegisterIcons()
{
	g_plugin.registerIcon("Import", iconList);
}
