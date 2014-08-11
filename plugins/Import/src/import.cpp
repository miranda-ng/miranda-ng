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

time_t dwSinceDate = 0;

HWND hdlgProgress;

static DWORD nDupes, nContactsCount, nMessagesCount, nGroupsCount, nSkippedEvents, nSkippedContacts;
static MIDatabase *srcDb, *dstDb;

/////////////////////////////////////////////////////////////////////////////////////////

static int myGet(MCONTACT hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv)
{
	dbv->type = 0;
	return srcDb->GetContactSetting(hContact, szModule, szSetting, dbv);
}

static TCHAR* myGetWs(MCONTACT hContact, const char *szModule, const char *szSetting)
{
	DBVARIANT dbv = { DBVT_TCHAR };
	return srcDb->GetContactSettingStr(hContact, szModule, szSetting, &dbv) ? NULL : dbv.ptszVal;
}

static BOOL myGetS(MCONTACT hContact, const char *szModule, const char *szSetting, char* dest)
{
	DBVARIANT dbv = { DBVT_ASCIIZ };
	dbv.pszVal = dest; dbv.cchVal = 100;
	return srcDb->GetContactSettingStatic(hContact, szModule, szSetting, &dbv);
}

static void mySet(MCONTACT hContact, const char *module, const char *var, DBVARIANT *dbv)
{
	DBCONTACTWRITESETTING dbw;
	dbw.szModule = module;
	dbw.szSetting = var;
	dbw.value = *dbv;
	dstDb->WriteContactSetting(hContact, &dbw);
}

/////////////////////////////////////////////////////////////////////////////////////////

static MCONTACT HContactFromNumericID(char *pszProtoName, char *pszSetting, DWORD dwID)
{
	MCONTACT hContact = dstDb->FindFirstContact();
	while (hContact != NULL) {
		if ( db_get_dw((MCONTACT)hContact, pszProtoName, pszSetting, 0) == dwID) {
			char* szProto = GetContactProto((MCONTACT)hContact);
			if (szProto != NULL && !lstrcmpA(szProto, pszProtoName))
				return hContact;
		}
		hContact = dstDb->FindNextContact((MCONTACT)hContact);
	}
	return INVALID_CONTACT_ID;
}

static MCONTACT HContactFromID(char *pszProtoName, char *pszSetting, TCHAR *pwszID)
{
	MCONTACT hContact = dstDb->FindFirstContact();
	while (hContact != NULL) {
		char* szProto = GetContactProto((MCONTACT)hContact);
		if ( !lstrcmpA(szProto, pszProtoName)) {
			ptrW id(db_get_tsa((MCONTACT)hContact, pszProtoName, pszSetting));
			if ( !lstrcmp(pwszID, id))
				return hContact;
		}

		hContact = (MCONTACT)dstDb->FindNextContact(hContact);
	}
	return INVALID_CONTACT_ID;
}

static MCONTACT HistoryImportFindContact(HWND hdlgProgress, char* szModuleName, DWORD uin, int addUnknown)
{
	MCONTACT hContact = HContactFromNumericID(szModuleName, "UIN", uin);
	if (hContact == NULL) {
		AddMessage( LPGENT("Ignored event from/to self"));
		return INVALID_CONTACT_ID;
	}

	if (hContact != INVALID_CONTACT_ID)
		return hContact;

	if (!addUnknown)
		return INVALID_CONTACT_ID;

	hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
	CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)szModuleName);
	db_set_dw((MCONTACT)hContact, szModuleName, "UIN", uin);
	AddMessage( LPGENT("Added contact %u (found in history)"), uin );
	return hContact;
}

/////////////////////////////////////////////////////////////////////////////////////////

static MCONTACT AddContact(HWND hdlgProgress, char* szProto, char* pszUniqueSetting, DBVARIANT* id, const TCHAR* pszUserID, TCHAR *nick, TCHAR *group)
{
	MCONTACT hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
	if (CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)szProto) != 0) {
		CallService(MS_DB_CONTACT_DELETE, hContact, 0);
		AddMessage(LPGENT("Failed to add %S contact %s"), szProto, pszUserID);
		return INVALID_CONTACT_ID;
	}

	mySet(hContact, szProto, pszUniqueSetting, id);

	CreateGroup(group, (MCONTACT)hContact);

	if (nick && *nick) {
		db_set_ws((MCONTACT)hContact, "CList", "MyHandle", nick);
		AddMessage(LPGENT("Added %S contact %s, '%s'"), szProto, pszUserID, nick);
	}
	else AddMessage(LPGENT("Added %S contact %s"), szProto, pszUserID);

	srcDb->FreeVariant(id);
	return hContact;
}

static int ImportGroup(const char* szSettingName, LPARAM lParam)
{
	int* pnGroups = (int*)lParam;

	TCHAR* tszGroup = myGetWs(NULL, "CListGroups", szSettingName);
	if (tszGroup != NULL) {
		if ( CreateGroup( tszGroup+1, NULL ))
			pnGroups[0]++;
		mir_free(tszGroup);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int ImportGroups()
{
	int nGroups = 0;

	DBCONTACTENUMSETTINGS param = { 0 };
	param.szModule = "CListGroups";
	param.pfnEnumProc = ImportGroup;
	param.lParam = (LPARAM)&nGroups;
	srcDb->EnumContactSettings(NULL, &param);
	return nGroups;
}

static MCONTACT ImportContact(MCONTACT hSrc)
{
	MCONTACT hDst;
	TCHAR id[ 40 ], *pszUniqueID;
	char  szProto[100];

	// Check what protocol this contact belongs to
	if (myGetS(hSrc, "Protocol", "p", szProto)) {
		AddMessage(LPGENT("Skipping contact with no protocol"));
		return NULL;
	}

	if (!IsProtocolLoaded(szProto)) {
		AddMessage(LPGENT("Skipping contact, %S not installed."), szProto);
		return NULL;
	}

	// Skip protocols with no unique id setting (some non IM protocols return NULL)
	char* pszUniqueSetting = (char*)CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
	if (!pszUniqueSetting || (INT_PTR)pszUniqueSetting == CALLSERVICE_NOTFOUND) {
		AddMessage(LPGENT("Skipping non-IM contact (%S)"), szProto);
		return NULL;
	}

	DBVARIANT dbv;
	if (myGet(hSrc, szProto, pszUniqueSetting, &dbv)) {
		AddMessage(LPGENT("Skipping %S contact, ID not found"), szProto);
		return NULL;
	}

	// Does the contact already exist?
	switch (dbv.type) {
	case DBVT_DWORD:
		pszUniqueID = _ltot(dbv.dVal, id, 10);
		hDst = HContactFromNumericID(szProto, pszUniqueSetting, dbv.dVal);
		break;

	case DBVT_ASCIIZ:
		pszUniqueID = NEWTSTR_ALLOCA(_A2T(dbv.pszVal));
		hDst = HContactFromID(szProto, pszUniqueSetting, pszUniqueID);
		break;

	case DBVT_WCHAR:
		pszUniqueID = NEWTSTR_ALLOCA(dbv.ptszVal);
		hDst = HContactFromID(szProto, pszUniqueSetting, pszUniqueID);
		break;
	}

	if (hDst != INVALID_CONTACT_ID) {
		AddMessage(LPGENT("Skipping duplicate %S contact %s"), szProto, pszUniqueID);
		srcDb->FreeVariant(&dbv);
		return NULL;
	}

	TCHAR *tszGroup = myGetWs(hSrc, "CList", "Group"), *tszNick = myGetWs(hSrc, "CList", "MyHandle");
	if (tszNick == NULL)
		tszNick = myGetWs(hSrc, szProto, "Nick");

	hDst = AddContact(hdlgProgress, szProto, pszUniqueSetting, &dbv, pszUniqueID, tszNick, tszGroup);
	mir_free(tszGroup), mir_free(tszNick);

	if (hDst != INVALID_CONTACT_ID) {
		// Hidden?
		if (!myGet(hSrc, "CList", "Hidden", &dbv)) {
			mySet(hDst, "CList", "Hidden", &dbv);
			srcDb->FreeVariant(&dbv);
		}

		// Ignore settings
		if (!myGet(hSrc, "Ignore", "Mask1", &dbv)) {
			mySet(hDst, "Ignore", "Mask1", &dbv);
			srcDb->FreeVariant(&dbv);
		}

		// Apparent mode
		if (!myGet(hSrc, szProto, "ApparentMode", &dbv)) {
			mySet(hDst, szProto, "ApparentMode", &dbv);
			srcDb->FreeVariant(&dbv);
		}

		// Nick
		if (!myGet(hSrc, szProto, "Nick", &dbv)) {
			mySet(hDst, szProto, "Nick", &dbv);
			srcDb->FreeVariant(&dbv);
		}

		// Myhandle
		if (!myGet(hSrc, szProto, "MyHandle", &dbv)) {
			mySet(hDst, szProto, "MyHandle", &dbv);
			srcDb->FreeVariant(&dbv);
		}

		// First name
		if (!myGet(hSrc, szProto, "FirstName", &dbv)) {
			mySet(hDst, szProto, "FirstName", &dbv);
			srcDb->FreeVariant(&dbv);
		}

		// Last name
		if (!myGet(hSrc, szProto, "LastName", &dbv)) {
			mySet(hDst, szProto, "LastName", &dbv);
			srcDb->FreeVariant(&dbv);
		}

		// About
		if (!myGet(hSrc, szProto, "About", &dbv)) {
			mySet(hDst, szProto, "About", &dbv);
			srcDb->FreeVariant(&dbv);
		}
	}
	else AddMessage(LPGENT("Unknown error while adding %S contact %s"), szProto, pszUniqueID);

	return hDst;
}

// This function should always be called after contact import. That is
// why there are no messages for errors related to contacts. Those
// would only be a repetition of the messages printed during contact
// import.

static MCONTACT convertContact(MCONTACT hContact)
{
	// Check what protocol this contact belongs to
	char szProto[100];
	if (myGetS(hContact, "Protocol", "p", szProto))
		return INVALID_CONTACT_ID;

	// Protocol installed?
	if (!IsProtocolLoaded(szProto))
		return INVALID_CONTACT_ID;

	// Is contact in database?
	char* pszUniqueSetting = (char*)CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);

	// Skip protocols with no unique id setting (some non IM protocols return NULL)
	MCONTACT hDst = INVALID_CONTACT_ID;
	if (pszUniqueSetting && (INT_PTR)pszUniqueSetting != CALLSERVICE_NOTFOUND) {
		DBVARIANT dbv;
		if (!myGet(hContact, szProto, pszUniqueSetting, &dbv)) {
			switch (dbv.type) {
			case DBVT_DWORD:
				hDst = HContactFromNumericID(szProto, pszUniqueSetting, dbv.dVal);
				break;

			case DBVT_ASCIIZ:
				hDst = HContactFromID(szProto, pszUniqueSetting, _A2T(dbv.pszVal));
				break;

			case DBVT_WCHAR:
				hDst = HContactFromID(szProto, pszUniqueSetting, dbv.ptszVal);
				break;
			}
			srcDb->FreeVariant(&dbv);
		}
	}
	return hDst;
}

static void ImportHistory(MCONTACT hContact, PROTOACCOUNT **protocol, int protoCount)
{
	// Is it contats history import?
	MCONTACT hDst = (protoCount == 0) ? convertContact(hContact) : NULL; //system history import

	// OK to import this chain?
	if (hDst == INVALID_CONTACT_ID) {
		nSkippedContacts++;
		return;
	}

	int i = 0, skipAll = 0;
	DWORD cbAlloc = 4096;
	BYTE* eventBuf = (PBYTE)mir_alloc(cbAlloc);

	// Get the start of the event chain
	HANDLE hEvent = srcDb->FindFirstEvent(hContact);
	while (hEvent) {
		int skip = 0;

		// Copy the event and import it
		DBEVENTINFO dbei = { sizeof(DBEVENTINFO) };
		dbei.cbBlob = srcDb->GetBlobSize(hEvent);
		if (dbei.cbBlob > cbAlloc) {
			cbAlloc = dbei.cbBlob + 4096 - dbei.cbBlob % 4096;
			eventBuf = (PBYTE)mir_realloc(eventBuf, cbAlloc);
		}
		dbei.pBlob = eventBuf;

		if (!srcDb->GetEvent(hEvent, &dbei)) {
			// check protocols during system history import
			if (hDst == NULL) {
				skipAll = 1;
				for (int i = 0; i < protoCount; i++)
					if (!strcmp(dbei.szModule, protocol[i]->szModuleName)) {
						skipAll = 0;
						break;
					}

				skip = skipAll;
			}

			// custom filtering
			if (!skip && nImportOption == IMPORT_CUSTOM) {
				BOOL sent = (dbei.flags & DBEF_SENT);

				if (dbei.timestamp < (DWORD)dwSinceDate)
					skip = 1;

				if (!skip) {
					if (hDst) {
						skip = 1;
						switch (dbei.eventType) {
						case EVENTTYPE_MESSAGE:
							if ((sent ? IOPT_MSGSENT : IOPT_MSGRECV) & nCustomOptions)
								skip = 0;
							break;
						case EVENTTYPE_FILE:
							if ((sent ? IOPT_FILESENT : IOPT_FILERECV) & nCustomOptions)
								skip = 0;
							break;
						case EVENTTYPE_URL:
							if ((sent ? IOPT_URLSENT : IOPT_URLRECV) & nCustomOptions)
								skip = 0;
							break;
						default:
							if ((sent ? IOPT_OTHERSENT : IOPT_OTHERRECV) & nCustomOptions)
								skip = 0;
							break;
						}
					}
					else if (!(nCustomOptions & IOPT_SYSTEM))
						skip = 1;
				}

				if (skip)
					nSkippedEvents++;
			}

			if (!skip) {
				// Check for duplicate entries
				if (!IsDuplicateEvent((MCONTACT)hDst, dbei)) {
					// Add dbevent
					if (dstDb->AddEvent(hDst, &dbei) != NULL)
						nMessagesCount++;
					else
						AddMessage(LPGENT("Failed to add message"));
				}
				else
					nDupes++;
			}
		}

		if (!(i % 10)) {
			MSG msg;
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		// skip this chain if needed
		if (skipAll)
			break;

		// Get next event
		hEvent = srcDb->FindNextEvent(hContact, hEvent);
		i++;
	}
	mir_free(eventBuf);
}

/////////////////////////////////////////////////////////////////////////////////////////

void MirandaImport(HWND hdlg)
{
	DWORD dwTimer;
	char* pszModuleName = NULL;

	// Just to keep the macros happy
	hdlgProgress = hdlg;
	if ((dstDb = GetCurrentDatabase()) == NULL) {
		AddMessage(LPGENT("Error retrieving current profile, exiting."));
		return;
	}

	DATABASELINK *dblink = FindDatabasePlugin(importFile);
	if (dblink == NULL) {
		AddMessage(LPGENT("There's no database driver to open the input file, exiting."));
		return;
	}

	if ((srcDb = dblink->Load(importFile, TRUE)) == NULL) {
		AddMessage(LPGENT("Error loading source file, exiting."));
		return;
	}

	// Reset statistics
	nSkippedEvents = 0;
	nDupes = 0;
	nContactsCount = 0;
	nMessagesCount = 0;
	nGroupsCount = 0;
	nSkippedContacts = 0;
	SetProgress(0);

	// Get number of contacts
	int nNumberOfContacts = srcDb->GetContactCount();
	AddMessage(LPGENT("Number of contacts in database: %d"), nNumberOfContacts);
	AddMessage(_T(""));

	// Configure database for fast writing
	dstDb->SetCacheSafetyMode(FALSE);

	// Start benchmark timer
	dwTimer = time(NULL);

	// Import Groups
	if (nImportOption == IMPORT_ALL || (nCustomOptions & IOPT_GROUPS)) {
		AddMessage(LPGENT("Importing groups."));
		nGroupsCount = ImportGroups();
		if (nGroupsCount == -1)
			AddMessage(LPGENT("Group import failed."));

		AddMessage(_T(""));
	}
	// End of Import Groups

	// Import Contacts
	if (nImportOption != IMPORT_CUSTOM || (nCustomOptions & IOPT_CONTACTS)) {
		AddMessage(LPGENT("Importing contacts."));
		int i = 1;
		MCONTACT hContact = srcDb->FindFirstContact();
		while (hContact != NULL) {
			if (ImportContact(hContact))
				nContactsCount++;

			// Update progress bar
			SetProgress(100 * i / nNumberOfContacts);
			i++;

			// Process queued messages
			MSG msg;
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			// Get next contact in chain
			hContact = srcDb->FindNextContact(hContact);
		}
	}
	else AddMessage(LPGENT("Skipping new contacts import."));
	AddMessage(_T(""));
	// End of Import Contacts

	// Import history
	if (nImportOption != IMPORT_CONTACTS) {
		// Import NULL contact message chain
		if (nImportOption == IMPORT_ALL || (nCustomOptions & IOPT_SYSTEM)) {
			AddMessage(LPGENT("Importing system history."));

			int protoCount;
			PROTOACCOUNT **accs;
			CallService(MS_PROTO_ENUMACCOUNTS, (WPARAM)&protoCount, (LPARAM)&accs);

			if (protoCount > 0)
				ImportHistory(NULL, accs, protoCount);
		}
		else AddMessage(LPGENT("Skipping system history import."));

		AddMessage(_T(""));

		// Import other contact messages
		if (nImportOption == IMPORT_ALL || (nCustomOptions & 2046)) { // 2 - 1024 types
			AddMessage(LPGENT("Importing history."));
			MCONTACT hContact = srcDb->FindFirstContact();
			for (int i = 1; hContact != NULL; i++) {
				ImportHistory(hContact, NULL, NULL);

				SetProgress(100 * i / nNumberOfContacts);
				hContact = srcDb->FindNextContact(hContact);
			}
		}
		else AddMessage(LPGENT("Skipping history import."));

		AddMessage(_T(""));
	}
	// End of Import History

	// Restore database writing mode
	dstDb->SetCacheSafetyMode(TRUE);

	// Clean up before exit
	dblink->Unload(srcDb);

	// Stop timer
	dwTimer = time(NULL) - dwTimer;

	// Print statistics
	AddMessage(LPGENT("Import completed in %d seconds."), dwTimer);
	SetProgress(100);
	AddMessage((nImportOption == IMPORT_CONTACTS) ?
		LPGENT("Added %d contacts and %d groups.") : LPGENT("Added %d contacts, %d groups and %d events."),
		nContactsCount, nGroupsCount, nMessagesCount);

	if (nImportOption != IMPORT_CONTACTS) {
		if (nSkippedContacts)
			AddMessage(LPGENT("Skipped %d contacts."), nSkippedContacts);

		AddMessage((nImportOption == IMPORT_CUSTOM) ?
			LPGENT("Skipped %d duplicates and %d filtered events.") : LPGENT("Skipped %d duplicates."),
			nDupes, nSkippedEvents);
	}
}
