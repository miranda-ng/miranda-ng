/*
    Variables Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "variables.h"

extern struct LIST_INTERFACE li;

typedef struct {
	TOKENREGISTEREX tr;
	DWORD nameHash;
}
	TokenRegisterEntry;

struct
{
	TokenRegisterEntry** items;
	int count, limit, increment;
	FSortFunc sortFunc;
}
static tokens;

static CRITICAL_SECTION csRegister;

unsigned long int hashlittle(void *key, size_t length, unsigned long int initval);

static DWORD NameHashFunction(TCHAR *tszStr)
{
	return (DWORD)hashlittle(tszStr, _tcslen(tszStr)*sizeof(TCHAR), 0);
}

static TokenRegisterEntry* FindTokenRegisterByName(TCHAR *name)
{
	int idx;
	TokenRegisterEntry temp;
	temp.nameHash = NameHashFunction( name );
	if ( li.List_GetIndex(( SortedList* )&tokens, &temp, &idx ))
		return tokens.items[ idx ];
   
	return NULL;
}

int registerIntToken(TCHAR *szToken, TCHAR *(*parseFunction)(ARGUMENTSINFO *ai), int extraFlags, char* szHelpText)
{
	TOKENREGISTEREX tr = { 0 };
	tr.cbSize = sizeof(tr);
	tr.flags = TRF_FREEMEM|TRF_TCHAR|TRF_PARSEFUNC|extraFlags;
	tr.memType = TR_MEM_VARIABLES;
	tr.szHelpText = szHelpText;
	tr.tszTokenString = szToken;
	tr.parseFunctionT = parseFunction;

	return registerToken(0, (LPARAM)&tr);
}

int deRegisterToken(TCHAR *token) {
	
	TokenRegisterEntry *tre;
	
	if (token == NULL) {
		return -1;
	}	
	EnterCriticalSection(&csRegister);
	tre = FindTokenRegisterByName( token );
	if ( tre == NULL ) {
		LeaveCriticalSection(&csRegister);
		return -1;
	}

	li.List_RemovePtr(( SortedList* )&tokens, tre );
	LeaveCriticalSection(&csRegister);

	if ( !( tre->tr.flags & TRF_PARSEFUNC ) && tre->tr.szService != NULL )
		free( tre->tr.szService );

	if ( tre->tr.tszTokenString != NULL )
		free( tre->tr.tszTokenString );

	if ( tre->tr.szHelpText != NULL )
		free( tre->tr.szHelpText );

	if ( (tre->tr.flags & TRF_CLEANUP ) && !( tre->tr.flags & TRF_CLEANUPFUNC ) && tre->tr.szCleanupService != NULL )
		free( tre->tr.szCleanupService );

	free( tre );
	return 0;
}

INT_PTR registerToken(WPARAM wParam, LPARAM lParam)
{
	DWORD hash;
	int idx;
	TokenRegisterEntry *tre;

	TOKENREGISTEREX *newVr = ( TOKENREGISTEREX* )lParam;
	if ( newVr == NULL || newVr->szTokenString == NULL || newVr->cbSize <= 0 )
		return -1;

	if ( newVr->flags & TRF_TCHAR ) {
		deRegisterToken( newVr->tszTokenString );
		hash = NameHashFunction( newVr->tszTokenString );
	}
	else {
#ifdef UNICODE
		WCHAR *wtoken;

		wtoken = a2u( newVr->szTokenString );
		deRegisterToken( wtoken );
		hash = NameHashFunction( wtoken );
		free( wtoken );
#else
		deRegisterToken( newVr->szTokenString );
		hash = NameHashFunction( newVr->szTokenString );
#endif
	}

	tre = ( TokenRegisterEntry* )malloc( sizeof( TokenRegisterEntry ));
	if ( tre == NULL )
		return -1;

	memcpy( &tre->tr, newVr, newVr->cbSize );
	tre->nameHash = hash;
	if ( !_tcscmp( newVr->tszTokenString, _T("alias"))) 
		log_debugA("alias");

	if ( !( newVr->flags & TRF_PARSEFUNC ) && newVr->szService != NULL )
		tre->tr.szService = _strdup( newVr->szService );

	if ( newVr->flags & TRF_TCHAR )
		tre->tr.tszTokenString = _tcsdup( newVr->tszTokenString );
	else
#ifdef UNICODE
		tre->tr.tszTokenString = a2u( newVr->szTokenString );
#else
		tre->tr.szTokenString = _strdup( newVr->szTokenString );
#endif

	if ( newVr->szHelpText != NULL )
		tre->tr.szHelpText = _strdup( newVr->szHelpText );

	if (( newVr->flags & TRF_CLEANUP ) && !( newVr->flags & TRF_CLEANUPFUNC ) && newVr->szCleanupService != NULL )
		tre->tr.szCleanupService = _strdup( newVr->szCleanupService );

	EnterCriticalSection(&csRegister);
	li.List_GetIndex(( SortedList* )&tokens, tre, &idx );
	li.List_Insert(( SortedList* )&tokens, tre, idx );
	LeaveCriticalSection(&csRegister);

	return 0;
}

TOKENREGISTEREX *searchRegister(TCHAR *tvar, int type)
{
	TokenRegisterEntry *tre;
	TOKENREGISTEREX *retVr;

	if ( tvar == NULL )
		return 0;

	EnterCriticalSection( &csRegister );
	tre = FindTokenRegisterByName( tvar );
	if ( tre == NULL || ( type != 0 && (tre->tr.flags & ( TRF_FIELD | TRF_FUNCTION )) != 0 && !( tre->tr.flags & type )))
		retVr = NULL;
	else
		retVr = &tre->tr;

	LeaveCriticalSection(&csRegister);
	return retVr;
}

TCHAR *parseFromRegister(ARGUMENTSINFO *ai)
{
	int callRes;
	TCHAR *temp, *res;
	TOKENREGISTEREX *thisVr, trCopy;
	
	if ( (ai == NULL) || (ai->argc == 0) || (ai->targv[0] == NULL) ) {
		return NULL;
	}
	callRes = 0;
	temp = res = NULL;
	thisVr = NULL;
	EnterCriticalSection(&csRegister);

	/* note the following limitation: you cannot add/remove tokens during a call from a different thread */
	thisVr = searchRegister( ai->targv[0], 0 );
	if ( thisVr == NULL ) {
		LeaveCriticalSection(&csRegister);
		return NULL;
	}

	trCopy = *thisVr;

#ifdef UNICODE
	// ai contains WCHARs, convert to chars because the tr doesn't support WCHARs
	if ( !( thisVr->flags & TRF_TCHAR )) {
		// unicode variables calls a non-unicode plugin
		unsigned int j;
		ARGUMENTSINFO cAi;
		
		memcpy(&cAi, ai, sizeof(ARGUMENTSINFO));
		cAi.argv = ( char** )malloc(ai->argc*sizeof(char *));
		for ( j=0; j < ai->argc; j++ )
			cAi.argv[j] = u2a( ai->targv[j] );

		if ( thisVr->flags & TRF_PARSEFUNC )
			callRes = (int)thisVr->parseFunction( &cAi );
		else if ( thisVr->szService != NULL)
			callRes = CallService( thisVr->szService, (WPARAM)0, (LPARAM)&cAi );

		for ( j=0; j < cAi.argc; j++ )
			if ( cAi.argv[j] != NULL )
				free( cAi.argv[j] );

		if ((char *)callRes != NULL )
			res = a2u(( char* )callRes );
	}
	else {
		// unicode variables calls unicode plugin
		if ( thisVr->flags & TRF_PARSEFUNC )
			callRes = (int)thisVr->parseFunctionT( ai );
		else if ( thisVr->szService != NULL )
			callRes = CallService( thisVr->szService, (WPARAM)0, (LPARAM)ai );

		if (( TCHAR* )callRes != NULL )
			res = _tcsdup(( TCHAR* )callRes );
	}
#else
	// non-unicode variables, should call non-unicode plugin (assume)
	// ai contains chars, tr shouldn't use WCHARs
	if ( thisVr->flags & TRF_PARSEFUNC )
		callRes = (int)thisVr->parseFunction(ai);
	else if ( thisVr->szService != NULL )
		callRes = CallService( thisVr->szService, (WPARAM)0, (LPARAM)ai );

	if (( char* )callRes != NULL )
		res = _strdup(( char* )callRes );
#endif

	if (( void* )callRes != NULL ) {
		if ( trCopy.flags & TRF_CLEANUP ) {
			if ( trCopy.flags & TRF_CLEANUPFUNC )
				trCopy.cleanupFunctionT(( TCHAR* )callRes );
			else if ( trCopy.szCleanupService != NULL )
				CallService( trCopy.szCleanupService, 0, (LPARAM)callRes );
		}
		if ( trCopy.flags & TRF_FREEMEM ) {
			if ( trCopy.memType == TR_MEM_MIRANDA )
				mir_free(( void* )callRes );
			else if ( trCopy.memType == TR_MEM_VARIABLES )
				free((void *)callRes);
		}
	}
	LeaveCriticalSection(&csRegister);
	return res;
}

TOKENREGISTEREX* getTokenRegister( int i )
{
	TOKENREGISTEREX *retVr;
	
	EnterCriticalSection(&csRegister);
	retVr = ( i >= tokens.count || i < 0 ) ? NULL : &tokens.items[i]->tr;
	LeaveCriticalSection( &csRegister );
	// beware! a pointer is returned here, no copy
	return retVr;
}

int getTokenRegisterCount()
{
	int retVal;

	EnterCriticalSection(&csRegister);
	retVal = tokens.count;
	LeaveCriticalSection(&csRegister);

	return retVal;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int CompareTokens( const TokenRegisterEntry* p1, const TokenRegisterEntry* p2 )
{
	if ( p1->nameHash == p2->nameHash )
		return 0;

	return ( p1->nameHash < p2->nameHash ) ? -1 : 1;
}

int initTokenRegister()
{
	InitializeCriticalSection(&csRegister);

	tokens.sortFunc = ( FSortFunc )CompareTokens;
	tokens.increment = 100;
	return 0;
}

int deinitTokenRegister()
{
	int i;
	EnterCriticalSection(&csRegister);

	for ( i=0; i < tokens.count; i++ ) {
		TokenRegisterEntry *tre = tokens.items[ i ];
		if ( !( tre->tr.flags & TRF_PARSEFUNC ) && tre->tr.szService != NULL )
			free( tre->tr.szService );

		if ( tre->tr.tszTokenString != NULL )
			free( tre->tr.tszTokenString );

		if ( tre->tr.szHelpText != NULL )
			free( tre->tr.szHelpText );

		if (( tre->tr.flags & TRF_CLEANUP ) && !( tre->tr.flags & TRF_CLEANUPFUNC ) && tre->tr.szCleanupService != NULL )
			free( tre->tr.szCleanupService );

		free( tre );
	}
	li.List_Destroy(( SortedList* )&tokens );

	LeaveCriticalSection(&csRegister);
	DeleteCriticalSection(&csRegister);

	return 0;
}
