/*

Import plugin for Miranda IM

Copyright (C) 2001-2005 Martin Öberg, Richard Hughes, Roland Rabien & Tristan Van de Vreede

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

void FreeVariant( DBVARIANT* dbv );
void WriteVariant( HANDLE hContact, const char* module, const char* var, DBVARIANT* dbv );

BOOL IsDuplicateEvent(HANDLE hContact, DBEVENTINFO dbei);


int nImportOption;
int nCustomOptions;

int      cICQAccounts = 0;
char  ** szICQModuleName = NULL;
TCHAR ** tszICQAccountName = NULL;
int      iICQAccount = 0;

static HANDLE hHookModulesLoaded = NULL;
static HANDLE hHookOnExit = NULL;
static HANDLE hImportService = NULL;


INT_PTR CALLBACK WizardDlgProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);

HINSTANCE hInst;
PLUGINLINK *pluginLink;
static HWND hwndWizard = NULL;
int hLangpack;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"Import contacts and messages",
	__VERSION_DWORD,
	"Imports contacts and messages from Mirabilis ICQ and Miranda IM.",
	"Miranda team",
	"info@miranda-im.org",
	"© 2000-2010 Martin Öberg, Richard Hughes, Dmitry Kuzkin, George Hazan",
	"http://www.miranda-im.org",
	UNICODE_AWARE,
	0,	//{2D77A746-00A6-4343-BFC5-F808CDD772EA}
      {0x2d77a746, 0xa6, 0x4343, { 0xbf, 0xc5, 0xf8, 0x8, 0xcd, 0xd7, 0x72, 0xea }}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

static INT_PTR ImportCommand(WPARAM wParam,LPARAM lParam)
{
	if (IsWindow(hwndWizard)) {
		SetForegroundWindow(hwndWizard);
		SetFocus(hwndWizard);
	}
	else hwndWizard = CreateDialog(hInst, MAKEINTRESOURCE(IDD_WIZARD), NULL, WizardDlgProc);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MirandaPluginInfoEx - returns an information about a plugin

__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if (mirandaVersion < MIRANDA_VERSION_CORE)
		return NULL;

	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MirandaPluginInterfaces - returns the protocol interface to the core

static const MUUID interfaces[] = {MIID_IMPORT, MIID_LAST};

__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Performs a primary set of actions upon plugin loading

static int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	int nProtocols = 0;
	int n;
	PROTOCOLDESCRIPTOR **ppProtos = NULL;

	if (DBGetContactSettingByte(NULL, IMPORT_MODULE, IMP_KEY_FR, 0))
		return 0;

	// Only autorun import wizard if at least one protocol is installed
	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&nProtocols, (LPARAM)&ppProtos);
	for (n=0; n < nProtocols; n++) {
		if (ppProtos[n]->type == PROTOTYPE_PROTOCOL) {
			CallService(IMPORT_SERVICE, 0, 0);
			DBWriteContactSettingByte(NULL, IMPORT_MODULE, IMP_KEY_FR, 1);
			break;
	}	}
	return 0;
}

static int OnExit(WPARAM wParam, LPARAM lParam)
{
	if ( hwndWizard )
		SendMessage(hwndWizard, WM_CLOSE, 0, 0);
	return 0;
}

int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;
	mir_getMMI( &mmi );
	mir_getUTFI( &utfi );
	mir_getLP( &pluginInfo );

	hImportService = CreateServiceFunction(IMPORT_SERVICE, ImportCommand);
	{
		CLISTMENUITEM mi;
		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_IMPORT));
		mi.pszName = LPGEN("&Import...");
		mi.position = 500050000;
		mi.pszService = IMPORT_SERVICE;
		CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&mi);
	}
	hHookModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hHookOnExit = HookEvent(ME_SYSTEM_OKTOEXIT, OnExit);
	{
		INITCOMMONCONTROLSEX icex;
		icex.dwSize = sizeof(icex);
		icex.dwICC = ICC_DATE_CLASSES;
		InitCommonControlsEx(&icex);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload a plugin

int __declspec(dllexport) Unload(void)
{
	if (hHookModulesLoaded)
		UnhookEvent(hHookModulesLoaded);
	if (hHookOnExit)
		UnhookEvent(hHookOnExit);
	if (hImportService)
		DestroyServiceFunction(hImportService);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL IsProtocolLoaded(char* pszProtocolName)
{
	return CallService(MS_PROTO_ISPROTOCOLLOADED, 0, (LPARAM)pszProtocolName) ? TRUE : FALSE;
}

BOOL EnumICQAccounts()
{
	int count, i = 0;
	PROTOACCOUNT ** accs;
	
	while (cICQAccounts)
	{
		cICQAccounts--;
		free(szICQModuleName[cICQAccounts]);
		free(tszICQAccountName[cICQAccounts]);
	}
	
	ProtoEnumAccounts(&count, &accs);
	szICQModuleName   = (char**)realloc(szICQModuleName, count * sizeof(char**));
	tszICQAccountName = (TCHAR**)realloc(tszICQAccountName, count * sizeof(TCHAR**));
	while (i < count)
	{
		if ((0 == strcmp(ICQOSCPROTONAME, accs[i]->szProtoName)) && accs[i]->bIsEnabled)
		{
			szICQModuleName[cICQAccounts] = strdup(accs[i]->szModuleName);
			tszICQAccountName[cICQAccounts] = _tcsdup(accs[i]->tszAccountName);
			cICQAccounts++;
		}
		i++;
	}
	return cICQAccounts != 0;
}

void FreeICQAccountsList()
{
	while (cICQAccounts)
	{
		cICQAccounts--;
		free(szICQModuleName[cICQAccounts]);
		free(tszICQAccountName[cICQAccounts]);
	}

	if (szICQModuleName)
		free(szICQModuleName);
	if (tszICQAccountName)
		free(tszICQAccountName);

	szICQModuleName = NULL;
	tszICQAccountName = NULL;
}

HANDLE HContactFromNumericID(char* pszProtoName, char* pszSetting, DWORD dwID)
{
	char* szProto;
	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact != NULL)
	{
		if (DBGetContactSettingDword(hContact, pszProtoName, pszSetting, 0) == dwID)
		{
			szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
			if (szProto != NULL && !strcmp(szProto, pszProtoName))
				return hContact;
		}
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
	return INVALID_HANDLE_VALUE;
}

HANDLE HContactFromID(char* pszProtoName, char* pszSetting, char* pszID)
{
	DBVARIANT dbv;
	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact != NULL) {
		char* szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		if ( !lstrcmpA(szProto, pszProtoName)) {
			if (DBGetContactSettingString(hContact, pszProtoName, pszSetting, &dbv) == 0) {
				if (strcmp(pszID, dbv.pszVal) == 0) {
					DBFreeVariant(&dbv);
					return hContact;
				}
				DBFreeVariant(&dbv);
			}
		}

		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
	return INVALID_HANDLE_VALUE;
}

HANDLE HistoryImportFindContact(HWND hdlgProgress, char* szModuleName, DWORD uin, int addUnknown)
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

HANDLE AddContact(HWND hdlgProgress, char* pszProtoName, char* pszUniqueSetting,
						DBVARIANT* id, DBVARIANT* nick, DBVARIANT* group)
{
	HANDLE hContact;
	char szid[ 40 ];
	char* pszUserID = ( id->type == DBVT_DWORD ) ? _ltoa( id->dVal, szid, 10 ) : id->pszVal;

	hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
	if ( CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)pszProtoName) != 0) {
		CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
		AddMessage( LPGEN("Failed to add %s contact %s"), pszProtoName, pszUserID );
		FreeVariant( id );
		FreeVariant( nick );
		FreeVariant( group );
		return INVALID_HANDLE_VALUE;
	}

	WriteVariant( hContact, pszProtoName, pszUniqueSetting, id );

	if ( group->type )
		CreateGroup( group->type, group->pszVal, hContact );

	if ( nick->type && nick->pszVal[0] ) {
		WriteVariant( hContact, "CList", "MyHandle", nick );
		if (nick->type == DBVT_UTF8) {
			char *tmp = mir_utf8decodeA(nick->pszVal);
			AddMessage( LPGEN("Added %s contact %s, '%s'"), pszProtoName, pszUserID, tmp );
			mir_free(tmp);
		}
		else AddMessage( LPGEN("Added %s contact %s, '%s'"), pszProtoName, pszUserID, nick->pszVal );
	}
	else AddMessage( LPGEN("Added %s contact %s"), pszProtoName, pszUserID );

	FreeVariant( id );
	FreeVariant( nick );
	FreeVariant( group );
	return hContact;
}

// ------------------------------------------------
// Creates a group with a specified name in the
// Miranda contact list.
// If contact is specified adds it to group
// ------------------------------------------------
// Returns 1 if successful and 0 when it fails.
int CreateGroup(BYTE type, const char* name, HANDLE hContact)
{
	int groupId;
	TCHAR *tmp, *tszGrpName;
	char groupIdStr[11];
	size_t cbName;

	if (type == DBVT_UTF8)
		tmp = mir_utf8decodeT( name );
    else if (type == DBVT_WCHAR)
        tmp = mir_u2t(( wchar_t* )name );
    else
		tmp = mir_a2t( name );

	if ( tmp == NULL )
		return 0;

	cbName = _tcslen(tmp);
	tszGrpName = _alloca(( cbName+2 )*sizeof( TCHAR ));
	tszGrpName[0] = 1 | GROUPF_EXPANDED;
	_tcscpy( tszGrpName+1, tmp );
	mir_free( tmp );

	// Check for duplicate & find unused id
	for (groupId = 0; ; groupId++) {
		DBVARIANT dbv;
		itoa(groupId, groupIdStr,10);
		if (DBGetContactSettingTString(NULL, "CListGroups", groupIdStr, &dbv))
			break;

		if ( !lstrcmp(dbv.ptszVal + 1, tszGrpName + 1 )) {
			if (hContact)
		        DBWriteContactSettingTString( hContact, "CList", "Group", tszGrpName+1 );
			else {
				char *str = mir_t2a(tszGrpName + 1);
				AddMessage( LPGEN("Skipping duplicate group %s."), str);
				mir_free(str);
			}

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
	if (dbei.timestamp > dwEventTimeStamp)
	{
		// remember event
		hPreviousDbEvent = hExistingDbEvent;
		dwPreviousTimeStamp = dwEventTimeStamp;
		return FALSE;
	}

	if (hContact != hPreviousContact)
	{
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
		if (dbei.timestamp <= dwEventTimeStamp)
		{
		    // remember event
			dwPreviousTimeStamp = dwEventTimeStamp;
			hPreviousDbEvent = hExistingDbEvent;

			if ( dbei.timestamp != dwEventTimeStamp )
				return FALSE;
		}

	}
	// check for equal timestamps
	if (dbei.timestamp == dwPreviousTimeStamp)
	{
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
		while (hExistingDbEvent != NULL)
		{
			ZeroMemory(&dbeiExisting, sizeof(dbeiExisting));
			dbeiExisting.cbSize = sizeof(dbeiExisting);
			CallService(MS_DB_EVENT_GET, (WPARAM)hExistingDbEvent, (LPARAM)&dbeiExisting);

			if (dbeiExisting.timestamp != dwPreviousTimeStamp)
			{
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

	if (dbei.timestamp <= dwPreviousTimeStamp)
	{
		// look back
		while (hExistingDbEvent != NULL)
		{
			ZeroMemory(&dbeiExisting, sizeof(dbeiExisting));
			dbeiExisting.cbSize = sizeof(dbeiExisting);
			CallService(MS_DB_EVENT_GET, (WPARAM)hExistingDbEvent, (LPARAM)&dbeiExisting);

			if (dbei.timestamp > dbeiExisting.timestamp)
			{
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

			// Get previous event in chain
			hExistingDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDPREV, (WPARAM)hExistingDbEvent, 0);
		}

    }
    else
    {
		// look forward
		while (hExistingDbEvent != NULL)
		{
			ZeroMemory(&dbeiExisting, sizeof(dbeiExisting));
			dbeiExisting.cbSize = sizeof(dbeiExisting);
			CallService(MS_DB_EVENT_GET, (WPARAM)hExistingDbEvent, (LPARAM)&dbeiExisting);

			if (dbei.timestamp < dbeiExisting.timestamp)
			{
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
