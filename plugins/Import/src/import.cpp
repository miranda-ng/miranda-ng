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
#include <m_db_int.h>

time_t dwSinceDate = 0;

HWND hdlgProgress;

static DWORD nDupes, nContactsCount, nMessagesCount, nGroupsCount, nSkippedEvents, nSkippedContacts;
static MIDatabase *srcDb, *dstDb;

/////////////////////////////////////////////////////////////////////////////////////////

static int myGet(HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv)
{
	dbv->type = 0;
	DBCONTACTGETSETTING dgs = { szModule, szSetting, dbv };
	return srcDb->GetContactSetting(hContact, &dgs);
}

static TCHAR* myGetWs(HANDLE hContact, const char *szModule, const char *szSetting)
{
	DBVARIANT dbv = { DBVT_TCHAR };
	DBCONTACTGETSETTING dgs = { szModule, szSetting, &dbv };
	return ( srcDb->GetContactSettingStr(hContact, &dgs)) ? NULL : dbv.ptszVal;
}

static char* myGetS(HANDLE hContact, const char *szModule, const char *szSetting)
{
	DBVARIANT dbv = { DBVT_ASCIIZ };
	DBCONTACTGETSETTING dgs = { szModule, szSetting, &dbv };
	return ( srcDb->GetContactSettingStr(hContact, &dgs)) ? NULL : dbv.pszVal;
}

static void mySet(HANDLE hContact, const char *module, const char *var, DBVARIANT *dbv)
{
	DBCONTACTWRITESETTING dbw;
	dbw.szModule = module;
	dbw.szSetting = var;
	dbw.value = *dbv;
	dstDb->WriteContactSetting(hContact, &dbw);
}

/////////////////////////////////////////////////////////////////////////////////////////

static HANDLE HContactFromNumericID(char* pszProtoName, char* pszSetting, DWORD dwID)
{
	HANDLE hContact = dstDb->FindFirstContact();
	while (hContact != NULL) {
		if ( db_get_dw(hContact, pszProtoName, pszSetting, 0) == dwID) {
			char* szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
			if (szProto != NULL && !lstrcmpA(szProto, pszProtoName))
				return hContact;
		}
		hContact = dstDb->FindNextContact(hContact);
	}
	return INVALID_HANDLE_VALUE;
}

static HANDLE HContactFromID(char* pszProtoName, char* pszSetting, char* pszID)
{
	HANDLE hContact = dstDb->FindFirstContact();
	while (hContact != NULL) {
		char* szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		if ( !lstrcmpA(szProto, pszProtoName)) {
			mir_ptr<char> id( db_get_sa(hContact, pszProtoName, pszSetting));
			if ( !lstrcmpA(pszID, id))
				return hContact;
		}

		hContact = dstDb->FindNextContact(hContact);
	}
	return INVALID_HANDLE_VALUE;
}

static HANDLE HistoryImportFindContact(HWND hdlgProgress, char* szModuleName, DWORD uin, int addUnknown)
{
	HANDLE hContact = HContactFromNumericID(szModuleName, "UIN", uin);
	if (hContact == NULL) {
		AddMessage( LPGEN("Ignored event from/to self"));
		return INVALID_HANDLE_VALUE;
	}

	if (hContact != INVALID_HANDLE_VALUE)
		return hContact;

	if (!addUnknown)
		return INVALID_HANDLE_VALUE;

	hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
	CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)szModuleName);
	DBWriteContactSettingDword(hContact, szModuleName, "UIN", uin);
	AddMessage( LPGEN("Added contact %u (found in history)"), uin );
	return hContact;
}

/////////////////////////////////////////////////////////////////////////////////////////

static HANDLE AddContact(HWND hdlgProgress, char* pszProtoName, char* pszUniqueSetting, DBVARIANT* id, TCHAR *nick, TCHAR *group)
{
	HANDLE hContact;
	char szid[ 40 ];
	char* pszUserID = ( id->type == DBVT_DWORD ) ? _ltoa( id->dVal, szid, 10 ) : id->pszVal;

	hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
	if ( CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)pszProtoName) != 0) {
		CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
		AddMessage( LPGEN("Failed to add %s contact %s"), pszProtoName, pszUserID );
		srcDb->FreeVariant( id );
		return INVALID_HANDLE_VALUE;
	}

	mySet( hContact, pszProtoName, pszUniqueSetting, id );

	CreateGroup(group, hContact);

	if (nick && *nick) {
		db_set_ws(hContact, "CList", "MyHandle", nick );
		AddMessage( LPGEN("Added %s contact %s, '%S'"), pszProtoName, pszUserID, nick);
	}
	else AddMessage( LPGEN("Added %s contact %s"), pszProtoName, pszUserID);

	srcDb->FreeVariant( id );
	return hContact;
}

static int ImportGroup(const char* szSettingName, LPARAM lParam)
{
	int* pnGroups = (int*)lParam;

	TCHAR* tszGroup = myGetWs(NULL, "CListGroups", szSettingName);
	if (tszGroup != NULL) {
		if ( CreateGroup( tszGroup, NULL ))
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

static HANDLE ImportContact(HANDLE hSrc)
{
	HANDLE hDst;
	char* pszUserName;
	char id[ 40 ];

	// Check what protocol this contact belongs to
	mir_ptr<char> pszProtoName( myGetS(hSrc, "Protocol", "p"));
	if ( !pszProtoName) {
		AddMessage( LPGEN("Skipping contact with no protocol"));
		return NULL;
	}

	if ( !IsProtocolLoaded(pszProtoName)) {
		AddMessage( LPGEN("Skipping contact, %s not installed."), pszProtoName);
		return NULL;
	}

	// Skip protocols with no unique id setting (some non IM protocols return NULL)
	char* pszUniqueSetting = (char*)CallProtoService(pszProtoName, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
	if ( !pszUniqueSetting || (INT_PTR)pszUniqueSetting == CALLSERVICE_NOTFOUND ) {
		AddMessage( LPGEN("Skipping non-IM contact (%s)"), pszProtoName );
		return NULL;
	}

	DBVARIANT dbv;
	if ( myGet(hSrc, pszProtoName, pszUniqueSetting, &dbv)) {
		AddMessage( LPGEN("Skipping %s contact, ID not found"), pszProtoName);
		return NULL;
	}

	// Does the contact already exist?
	if ( dbv.type == DBVT_DWORD ) {
		pszUserName = _ltoa(dbv.dVal, id, 10);
		hDst = HContactFromNumericID( pszProtoName, pszUniqueSetting, dbv.dVal );
	}
	else {
		pszUserName = NEWSTR_ALLOCA(dbv.pszVal);
		hDst = HContactFromID( pszProtoName, pszUniqueSetting, dbv.pszVal );
	}

	if (hDst != INVALID_HANDLE_VALUE) {
		AddMessage( LPGEN("Skipping duplicate %s contact %s"), pszProtoName, pszUserName );
		srcDb->FreeVariant( &dbv );
		return NULL;
	}

	TCHAR *tszGroup = myGetWs(hSrc, "CList", "Group"), *tszNick = myGetWs(hSrc, "CList", "MyHandle");
	if (tszNick == NULL)
		tszNick = myGetWs(hSrc, pszProtoName, "Nick");

	hDst = AddContact(hdlgProgress, pszProtoName, pszUniqueSetting, &dbv, tszNick, tszGroup);
	mir_free(tszGroup), mir_free(tszNick);

	if ( hDst != INVALID_HANDLE_VALUE) {
		// Hidden?
		if ( myGet(hSrc, "CList", "Hidden", &dbv )) {
			mySet(hDst, "CList", "Hidden", &dbv);
			srcDb->FreeVariant(&dbv);
		}

		// Ignore settings
		if ( myGet(hSrc, "Ignore", "Mask1", &dbv )) {
			mySet( hDst, "Ignore", "Mask1", &dbv );
			srcDb->FreeVariant(&dbv);
		}

		// Apparent mode
		if ( myGet(hSrc, pszProtoName, "ApparentMode", &dbv )) {
			mySet( hDst, pszProtoName, "ApparentMode", &dbv );
			srcDb->FreeVariant(&dbv);
		}

		// Nick
		if ( myGet(hSrc, pszProtoName, "Nick", &dbv )) {
			mySet( hDst, pszProtoName, "Nick", &dbv );
			srcDb->FreeVariant(&dbv);
		}

		// Myhandle
		if ( myGet(hSrc, pszProtoName, "MyHandle", &dbv )) {
			mySet( hDst, pszProtoName, "MyHandle", &dbv );
			srcDb->FreeVariant(&dbv);
		}

		// First name
		if ( myGet(hSrc, pszProtoName, "FirstName", &dbv )) {
			mySet( hDst, pszProtoName, "FirstName", &dbv );
			srcDb->FreeVariant(&dbv);
		}

		// Last name
		if ( myGet(hSrc, pszProtoName, "LastName", &dbv )) {
			mySet( hDst, pszProtoName, "LastName", &dbv );
			srcDb->FreeVariant(&dbv);
		}

		// About
		if ( myGet(hSrc, pszProtoName, "About", &dbv )) {
			mySet( hDst, pszProtoName, "About", &dbv );
			srcDb->FreeVariant(&dbv);
		}
	}
	else AddMessage( LPGEN("Unknown error while adding %s contact %s"), pszProtoName, pszUserName );

	return hDst;
}

// This function should always be called after contact import. That is
// why there are no messages for errors related to contacts. Those
// would only be a repetition of the messages printed during contact
// import.

static void ImportHistory(HANDLE hContact, PROTOACCOUNT **protocol, int protoCount)
{
	HANDLE hDst = INVALID_HANDLE_VALUE;

	// Is it contats history import?
	if (protoCount == 0) {
		// Check what protocol this contact belongs to
		char* szProto = myGetS(hContact, "Protocol", "p");
		if (szProto != NULL) {
			// Protocol installed?
			if ( IsProtocolLoaded(szProto)) {
				// Is contact in database?
				char* pszUniqueSetting = (char*)CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);

				// Skip protocols with no unique id setting (some non IM protocols return NULL)
				if ( pszUniqueSetting && ( INT_PTR )pszUniqueSetting != CALLSERVICE_NOTFOUND ) {
					DBVARIANT dbv;
					if ( myGet(hContact, szProto, pszUniqueSetting, &dbv)) {
						if ( dbv.type == DBVT_DWORD )
							hDst = HContactFromNumericID(szProto, pszUniqueSetting, dbv.dVal);
						else
							hDst = HContactFromID(szProto, pszUniqueSetting, dbv.pszVal);
						srcDb->FreeVariant( &dbv );
			}	}	}
			
			mir_free(szProto);
		}
	}
	else hDst = NULL; //system history import

	// OK to import this chain?
	if (hDst == INVALID_HANDLE_VALUE) {
		nSkippedContacts++;
		return;
	}

	int i = 0, skipAll = 0;
	bool bIsVoidContact = dstDb->GetEventCount(hDst) == 0;

	// Get the start of the event chain
	HANDLE hEvent = srcDb->FindFirstEvent(hContact);
	while (hEvent) {
		int skip = 0;

		// Copy the event and import it
		DBEVENTINFO dbei = { 0 };
		if ( !srcDb->GetEvent(hEvent, &dbei)) {
			// check protocols during system history import
			if (hDst == NULL) {
				skipAll = 1;
				for (int i = 0; i < protoCount; i++)
					if ( !strcmp(dbei.szModule, protocol[i]->szModuleName)) {
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
						switch(dbei.eventType) {
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
					else if ( !( nCustomOptions & IOPT_SYSTEM ))
						skip = 1;
				}

				if (skip)
					nSkippedEvents++;
			}

			if (!skip) {
				// Check for duplicate entries
				if ( !IsDuplicateEvent( hDst, dbei )) {
					// Add dbevent
					if (!bIsVoidContact)
						dbei.flags &= ~DBEF_FIRST;
					if (dstDb->AddEvent(hDst, &dbei) != NULL)
						nMessagesCount++;
					else
						AddMessage( LPGEN("Failed to add message"));
				}
				else
					nDupes++;
			}
		}

		if ( !( i%10 )) {
			MSG msg;
			if ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
		}	}

		// skip this chain if needed
		if ( skipAll )
			break;

		// Get next event
		hEvent = srcDb->FindNextEvent(hEvent);
		i++;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void MirandaImport(HWND hdlg)
{
	DWORD dwTimer;
	char* pszModuleName = NULL;

	// Just to keep the macros happy
	hdlgProgress = hdlg;
	if ((dstDb = GetCurrentDatabase()) == NULL) {
		AddMessage( LPGEN("Error retrieving current profile, exiting."));
		return;
	}

	DATABASELINK* dblink = FindDatabasePlugin(importFile);
	if (dblink == NULL) {
		AddMessage( LPGEN("There's no database driver to open the input file, exiting."));
		return;
	}

	if ((srcDb = dblink->Load(importFile)) == NULL) {
		AddMessage( LPGEN("Error loading source file, exiting."));
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
	AddMessage( LPGEN("Number of contacts in database: %d"), nNumberOfContacts );
	AddMessage( "" );

	// Configure database for fast writing
	dstDb->SetCacheSafetyMode(FALSE);

	// Start benchmark timer
	dwTimer = time(NULL);

	// Import Groups
	if (nImportOption == IMPORT_ALL || (nCustomOptions & IOPT_GROUPS)) {
		AddMessage( LPGEN("Importing groups."));
		nGroupsCount = ImportGroups();
		if (nGroupsCount == -1)
			AddMessage( LPGEN("Group import failed."));

		AddMessage( "" );
	}
	// End of Import Groups

	// Import Contacts
	if (nImportOption != IMPORT_CUSTOM || (nCustomOptions & IOPT_CONTACTS)) {
		AddMessage( LPGEN("Importing contacts."));
		int i = 1;
		HANDLE hContact = srcDb->FindFirstContact();
		while (hContact != NULL) {
			if ( ImportContact(hContact))
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
	else AddMessage( LPGEN("Skipping new contacts import."));
	AddMessage( "" );
	// End of Import Contacts

	// Import history
	if (nImportOption != IMPORT_CONTACTS) {
		// Import NULL contact message chain
		if (nImportOption == IMPORT_ALL || (nCustomOptions & IOPT_SYSTEM)) {
			AddMessage( LPGEN("Importing system history."));

			int protoCount;
			PROTOACCOUNT **accs;
			CallService(MS_PROTO_ENUMACCOUNTS, (WPARAM)&protoCount, (LPARAM)&accs);

			if (protoCount > 0)
				ImportHistory(NULL, accs, protoCount);
		}
		else AddMessage( LPGEN("Skipping system history import."));

		AddMessage( "" );

		// Import other contact messages
		if (nImportOption == IMPORT_ALL || (nCustomOptions & 2046)) { // 2 - 1024 types
			AddMessage( LPGEN("Importing history."));
			HANDLE hContact = srcDb->FindFirstContact();
			for(int i=1; hContact != NULL; i++) {
				ImportHistory(hContact, NULL, NULL);

				SetProgress(100 * i / nNumberOfContacts);
				hContact = srcDb->FindNextContact(hContact);
			}
		}
		else AddMessage( LPGEN("Skipping history import."));

		AddMessage( "" );
	}
	// End of Import History

	// Restore database writing mode
	dstDb->SetCacheSafetyMode(TRUE);

	// Clean up before exit
	dblink->Unload(srcDb);

	// Stop timer
	dwTimer = time(NULL) - dwTimer;

	// Print statistics
	AddMessage( LPGEN("Import completed in %d seconds."), dwTimer );
	SetProgress(100);
	AddMessage((nImportOption == IMPORT_CONTACTS) ?
		LPGEN("Added %d contacts and %d groups.") : LPGEN("Added %d contacts, %d groups and %d events."),
		nContactsCount, nGroupsCount, nMessagesCount);

	if ( nImportOption != IMPORT_CONTACTS ) {
		if (nSkippedContacts)
			AddMessage( LPGEN("Skipped %d contacts."), nSkippedContacts );

		AddMessage((nImportOption == IMPORT_CUSTOM) ?
			LPGEN("Skipped %d duplicates and %d filtered events.") : LPGEN("Skipped %d duplicates."),
			nDupes, nSkippedEvents);
	}
}
