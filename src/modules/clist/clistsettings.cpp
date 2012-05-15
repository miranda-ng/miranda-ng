/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2010 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

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
#include "commonheaders.h"
#include "clc.h"

SortedList* clistCache = NULL;

static int compareContacts( ClcCacheEntryBase* p1, ClcCacheEntryBase* p2 )
{
	return ( char* )p1->hContact - ( char* )p2->hContact;
}

void InitDisplayNameCache(void)
{
	clistCache = List_Create( 0, 50 );
	clistCache->sortFunc = ( FSortFunc )compareContacts;
}

void FreeDisplayNameCache(void)
{
	if ( clistCache != NULL ) {
		int i;
		for ( i = 0; i < clistCache->realCount; i++) {
			cli.pfnFreeCacheItem(( ClcCacheEntryBase* )clistCache->items[i] );
			mir_free( clistCache->items[i] );
		}

		List_Destroy( clistCache ); 
		mir_free(clistCache);
		clistCache = NULL;
}	}

// default handlers for the cache item creation and destruction

ClcCacheEntryBase* fnCreateCacheItem( HANDLE hContact )
{
	ClcCacheEntryBase* p = ( ClcCacheEntryBase* )mir_calloc( sizeof( ClcCacheEntryBase ));
	if ( p == NULL )
		return NULL;

	p->hContact = hContact;
	return p;
}

void fnCheckCacheItem( ClcCacheEntryBase* p )
{
	DBVARIANT dbv;
	if ( p->group == NULL ) {
		if ( !DBGetContactSettingTString( p->hContact, "CList", "Group", &dbv )) {
			p->group = mir_tstrdup( dbv.ptszVal );
			mir_free( dbv.ptszVal );
		}
		else p->group = mir_tstrdup( _T("") );
	}

	if ( p->isHidden == -1 )
		p->isHidden = DBGetContactSettingByte( p->hContact, "CList", "Hidden", 0 );
}

void fnFreeCacheItem( ClcCacheEntryBase* p )
{
	if ( p->name ) { mir_free( p->name ); p->name = NULL; }
	#if defined( _UNICODE )
		if ( p->szName ) { mir_free( p->szName); p->szName = NULL; }
	#endif
	if ( p->group ) { mir_free( p->group ); p->group = NULL; }
	p->isHidden = -1;
}

ClcCacheEntryBase* fnGetCacheEntry(HANDLE hContact)
{
	ClcCacheEntryBase* p;
	int idx;
	if ( !List_GetIndex( clistCache, &hContact, &idx )) {	
		if (( p = cli.pfnCreateCacheItem( hContact )) != NULL ) {
			List_Insert( clistCache, p, idx );
			cli.pfnInvalidateDisplayNameCacheEntry( p );
		}
	}
	else p = ( ClcCacheEntryBase* )clistCache->items[idx];

	cli.pfnCheckCacheItem( p );
	return p;
}

void fnInvalidateDisplayNameCacheEntry(HANDLE hContact)
{
	if (hContact == INVALID_HANDLE_VALUE) {
		FreeDisplayNameCache();
		InitDisplayNameCache();
		SendMessage(cli.hwndContactTree, CLM_AUTOREBUILD, 0, 0);
	}
	else {
		int idx;
		if ( List_GetIndex( clistCache, &hContact, &idx ))
			cli.pfnFreeCacheItem(( ClcCacheEntryBase* )clistCache->items[idx] );
}	}

TCHAR* fnGetContactDisplayName( HANDLE hContact, int mode )
{
	CONTACTINFO ci;
	TCHAR *buffer;
	ClcCacheEntryBase* cacheEntry = NULL;

	if ( mode & GCDNF_NOCACHE )
		mode &= ~GCDNF_NOCACHE;
	else if ( mode != GCDNF_NOMYHANDLE) {
		cacheEntry = cli.pfnGetCacheEntry( hContact );
		if ( cacheEntry->name )
			return cacheEntry->name;
	}
	ZeroMemory(&ci, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.hContact = hContact;
	if (ci.hContact == NULL)
		ci.szProto = "ICQ";
	ci.dwFlag = ((mode == GCDNF_NOMYHANDLE) ? CNF_DISPLAYNC : CNF_DISPLAY) | CNF_TCHAR;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
		if (ci.type == CNFT_ASCIIZ) {
			if (cacheEntry == NULL)
				return ci.pszVal;

			cacheEntry->name = ci.pszVal;
			#if defined( _UNICODE )
				cacheEntry->szName = mir_u2a( ci.pszVal );
			#endif
			return ci.pszVal;
		}

		if (ci.type == CNFT_DWORD) {
			if (cacheEntry == NULL) {
				buffer = (TCHAR*) mir_alloc(15 * sizeof( TCHAR ));
				_ltot(ci.dVal, buffer, 10 );
				return buffer;
			}
			else {
				buffer = (TCHAR*) mir_alloc(15 * sizeof( TCHAR ));
				_ltot(ci.dVal, buffer, 10 );
				cacheEntry->name = buffer;
				#if defined( _UNICODE )
					cacheEntry->szName = mir_u2a( buffer );
				#endif
				return buffer;
	}	}	}

	CallContactService(hContact, PSS_GETINFO, SGIF_MINIMAL, 0);
	buffer = TranslateT("(Unknown Contact)");
	return ( cacheEntry == NULL ) ? mir_tstrdup( buffer ) : buffer;
}

INT_PTR GetContactDisplayName(WPARAM wParam, LPARAM lParam)
{
	CONTACTINFO ci;
	ClcCacheEntryBase* cacheEntry = NULL;
	char *buffer;
	HANDLE hContact = (HANDLE)wParam;

	if ( lParam & GCDNF_UNICODE )
		return ( INT_PTR )cli.pfnGetContactDisplayName(hContact, lParam & ~GCDNF_UNICODE );

	if ((int) lParam != GCDNF_NOMYHANDLE) {
		cacheEntry = cli.pfnGetCacheEntry(hContact);
		#if defined( _UNICODE )
			if ( cacheEntry->szName )
				return (INT_PTR)cacheEntry->szName;
		#else
			if ( cacheEntry->name )
				return (INT_PTR)cacheEntry->name;
		#endif
	}
	ZeroMemory(&ci, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.hContact = hContact;
	if (ci.hContact == NULL)
		ci.szProto = "ICQ";
	ci.dwFlag = ((lParam == GCDNF_NOMYHANDLE) ? CNF_DISPLAYNC : CNF_DISPLAY) | CNF_TCHAR;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
		if (ci.type == CNFT_ASCIIZ) {
			if (cacheEntry == NULL) {
				#if defined( _UNICODE )
					buffer = mir_u2a( ci.pszVal );
					mir_free(ci.pszVal);
				#else
					buffer = ci.pszVal;
				#endif
				return (INT_PTR) buffer;
			}
			else {
				cacheEntry->name = ci.pszVal;
				#if defined( _UNICODE )
					cacheEntry->szName = mir_u2a( ci.pszVal );
					return (INT_PTR)cacheEntry->szName;
				#else
					return (INT_PTR)cacheEntry->name;
				#endif
			}
		}
		if (ci.type == CNFT_DWORD) {
			if (cacheEntry == NULL) {
				buffer = ( char* )mir_alloc(15);
				_ltoa(ci.dVal, buffer, 10 );
				return (INT_PTR) buffer;
			}
			else {
				buffer = ( char* )mir_alloc(15);
				_ltoa(ci.dVal, buffer, 10 );
				#if defined( _UNICODE )
					cacheEntry->szName = buffer;
					cacheEntry->name = mir_a2u( buffer );
				#else
					cacheEntry->name = buffer;
				#endif
				return (INT_PTR) buffer;
	}	}	}

	CallContactService(hContact, PSS_GETINFO, SGIF_MINIMAL, 0);
	buffer = Translate("(Unknown Contact)");
	return (INT_PTR) buffer;
}

INT_PTR InvalidateDisplayName(WPARAM wParam, LPARAM)
{
	cli.pfnInvalidateDisplayNameCacheEntry((HANDLE)wParam);
	return 0;
}

int ContactAdded(WPARAM wParam, LPARAM)
{
	cli.pfnChangeContactIcon((HANDLE)wParam, cli.pfnIconFromStatusMode((char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0), ID_STATUS_OFFLINE, NULL), 1);
	cli.pfnSortContacts();
	return 0;
}

int ContactDeleted(WPARAM wParam, LPARAM)
{
	CallService(MS_CLUI_CONTACTDELETED, wParam, 0);
	return 0;
}

int ContactSettingChanged(WPARAM wParam, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *) lParam;
	DBVARIANT dbv;
	HANDLE hContact = (HANDLE)wParam;

	// Early exit
	if ( hContact == NULL)
		return 0;

	dbv.pszVal = NULL;
	if (!DBGetContactSetting(hContact, "Protocol", "p", &dbv)) {
		if (!strcmp(cws->szModule, dbv.pszVal)) {
			cli.pfnInvalidateDisplayNameCacheEntry(hContact);
			if (!strcmp(cws->szSetting, "UIN") || !strcmp(cws->szSetting, "Nick") || !strcmp(cws->szSetting, "FirstName")
				|| !strcmp(cws->szSetting, "LastName") || !strcmp(cws->szSetting, "e-mail")) {
					CallService(MS_CLUI_CONTACTRENAMED, wParam, 0);
				}
			else if (!strcmp(cws->szSetting, "Status")) {
				if (!DBGetContactSettingByte(hContact, "CList", "Hidden", 0)) {
					if (DBGetContactSettingByte(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT)) {
						// User's state is changing, and we are hideOffline-ing
						if (cws->value.wVal == ID_STATUS_OFFLINE) {
							cli.pfnChangeContactIcon(hContact, cli.pfnIconFromStatusMode(cws->szModule, cws->value.wVal, hContact), 0);
							CallService(MS_CLUI_CONTACTDELETED, wParam, 0);
							mir_free(dbv.pszVal);
							return 0;
						}
						cli.pfnChangeContactIcon(hContact, cli.pfnIconFromStatusMode(cws->szModule, cws->value.wVal, hContact), 1);
					}
					cli.pfnChangeContactIcon(hContact, cli.pfnIconFromStatusMode(cws->szModule, cws->value.wVal, hContact), 0);
				}
			}
			else {
				mir_free(dbv.pszVal);
				return 0;
			}
			cli.pfnSortContacts();
	}	}

	if (!strcmp(cws->szModule, "CList")) {
		if (!strcmp(cws->szSetting, "Hidden")) {
			if (cws->value.type == DBVT_DELETED || cws->value.bVal == 0) {
				char *szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
				cli.pfnChangeContactIcon(hContact, cli.pfnIconFromStatusMode(szProto, szProto == NULL ? ID_STATUS_OFFLINE : DBGetContactSettingWord(hContact, szProto, "Status", ID_STATUS_OFFLINE), hContact), 1);
			}
			else
				CallService(MS_CLUI_CONTACTDELETED, wParam, 0);
		}
		if (!strcmp(cws->szSetting, "MyHandle"))
			cli.pfnInvalidateDisplayNameCacheEntry(hContact);
	}

	if (!strcmp(cws->szModule, "Protocol")) {
		if (!strcmp(cws->szSetting, "p")) {
			char *szProto;
			if (cws->value.type == DBVT_DELETED)
				szProto = NULL;
			else
				szProto = cws->value.pszVal;
			cli.pfnChangeContactIcon(hContact,
				cli.pfnIconFromStatusMode(szProto,
					szProto == NULL ? ID_STATUS_OFFLINE : DBGetContactSettingWord(hContact, szProto, "Status",
					ID_STATUS_OFFLINE), hContact), 0);
	}	}

	// Clean up
	if (dbv.pszVal)
		mir_free(dbv.pszVal);

	return 0;
}
