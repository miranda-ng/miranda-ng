/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
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
#include <m_plugins.h>

// list of hooks

static int compareHooks( const THook* p1, const THook* p2 )
{
	return strcmp( p1->name, p2->name );
}

static LIST<THook> hooks( 50, compareHooks );

struct THookToMainThreadItem
{
	THook* hook;
	HANDLE hDoneEvent;
	WPARAM wParam;
	LPARAM lParam;
	int result;
};

// list of services

struct TService
{
	DWORD nameHash;
	HINSTANCE hOwner;
	union {
		MIRANDASERVICE pfnService;
		MIRANDASERVICEPARAM pfnServiceParam;
		MIRANDASERVICEOBJ pfnServiceObj;
		MIRANDASERVICEOBJPARAM pfnServiceObjParam;
	};
	int flags;
	LPARAM lParam;
	void* object;
	char name[1];
};

LIST<TService> services( 100, NumericKeySortT );

typedef struct
{
	HANDLE hDoneEvent;
	WPARAM wParam;
	LPARAM lParam;
	int result;
	const char *name;
}
	TServiceToMainThreadItem;

// other static variables
static BOOL bServiceMode = FALSE;
static CRITICAL_SECTION csHooks,csServices;
static DWORD  mainThreadId;
static int    hookId = 1;
static HANDLE hMainThread;
static HANDLE hMissingService;
static THook *pLastHook = NULL;

HINSTANCE GetInstByAddress( void* codePtr );

void LangPackDropUnusedItems( void );

void ParseCommandLine();		// core: IDD_WAITRESTART
int LoadSystemModule(void);		// core: m_system.h services
int LoadNewPluginsModuleInfos(void); // core: preloading plugins
int LoadNewPluginsModule(void);	// core: N.O. plugins
int LoadSslModule(void);
int LoadNetlibModule(void);		// core: network
void NetlibInitSsl(void);
int LoadLangPackModule(void);	// core: translation
int LoadProtocolsModule(void);	// core: protocol manager
int LoadAccountsModule(void);    // core: account manager
int LoadIgnoreModule(void);		// protocol filter: ignore

int LoadSendRecvUrlModule(void);	//send/recv
int LoadSendRecvEMailModule(void);	//send/recv
int LoadSendRecvAuthModule(void);	//send/recv
int LoadSendRecvFileModule(void);	//send/recv

int LoadContactListModule(void);// ui: clist
int LoadOptionsModule(void);	// ui: options dialog
int LoadFindAddModule(void);	// ui: search/add users
int LoadSkinIcons(void);
int LoadSkinSounds(void);
int LoadSkinHotkeys(void);
int LoadHelpModule(void);		// ui: help stuff
int LoadUserInfoModule(void);	// ui: user info
int LoadHistoryModule(void);	// ui: history viewer
int LoadAwayMsgModule(void);	// ui: setting away messages
int LoadVisibilityModule(void);	// ui: visibility control
int LoadCLUIModule(void);		// ui: CList UI
int LoadPluginOptionsModule(void);	// ui: plugin viewer
int LoadAddContactModule(void);	// ui: authcontrol contacts
int LoadIdleModule(void);		// rnd: report idle information
int LoadAutoAwayModule(void);	// ui: away
int LoadUserOnlineModule(void);	// ui: online alert
int LoadUtilsModule(void);		// ui: utils (has a few window classes, like HyperLink)
int LoadCLCModule(void);		// window class: CLC control
int LoadButtonModule(void);		// window class: button class
int LoadContactsModule(void);    // random: contact
int LoadFontserviceModule(void); // ui: font manager
int LoadIcoLibModule(void);   // ui: icons manager
int LoadUpdateNotifyModule(void); // random: update notification
int LoadServiceModePlugin(void);
int LoadErrorsModule();

void UnloadUtilsModule(void);
void UnloadButtonModule(void);
void UnloadClcModule(void);
void UnloadContactListModule(void);
void UnloadEventsModule(void);
void UnloadIdleModule(void);
void UnloadLangPackModule(void);
void UnloadSslModule(void);
void UnloadNetlibModule(void);
void UnloadNewPlugins(void);
void UnloadUpdateNotifyModule(void);
void UnloadIcoLibModule(void);
void UnloadSkinSounds(void);
void UnloadSkinHotkeys(void);
void UnloadProtocolsModule(void);
void UnloadAccountsModule(void);
void UnloadErrorsModule(void);

int LoadIcoTabsModule();
int LoadHeaderbarModule();
int LoadDescButtonModule();
int LoadDefaultModules(void)
{
    //load order is very important for these
	if (LoadSystemModule()) return 1;
	if (LoadLangPackModule()) return 1;		// langpack will be a system module in the new order so this is moved here
	ParseCommandLine();						// IDD_WAITRESTART need langpack  so this is moved here
	if (LoadUtilsModule()) return 1;		//order not important for this, but no dependencies and no point in pluginising
	if (LoadIcoTabsModule()) return 1;
	if (LoadHeaderbarModule()) return 1;
	if (LoadNewPluginsModuleInfos()) return 1;

	// database is available here
	if (LoadButtonModule()) return 1;
	if (LoadIcoLibModule()) return 1;
	if (LoadSkinIcons()) return 1;

//	if (LoadErrorsModule()) return 1;

	bServiceMode = LoadServiceModePlugin();
	switch ( bServiceMode )
	{
		case 1:	return 0; // stop loading here
		case 0: break;
		default: return 1;
	}

	//this info will be available at LoadNewPluginsModule()
	INT_PTR *disableDefaultModule=(INT_PTR*)CallService(MS_PLUGINS_GETDISABLEDEFAULTARRAY,0,0);

	if (LoadSkinSounds()) return 1;
	if (LoadSkinHotkeys()) return 1;
	if (LoadFontserviceModule()) return 1;

	if (LoadDescButtonModule()) return 1;
	if (LoadOptionsModule()) return 1;
	if (LoadNetlibModule()) return 1;
	if (LoadProtocolsModule()) return 1;
	    LoadDbAccounts();                    // retrieves the account array from a database
	if (LoadContactsModule()) return 1;
	if (LoadContactListModule()) return 1;
	if (LoadAddContactModule()) return 1;
	if (LoadNewPluginsModule()) return 1;    // will call Load() on everything, clist will load first

	LangPackDropUnusedItems();

	if (!disableDefaultModule[DEFMOD_SSL]) if (LoadSslModule()) return 1;
	NetlibInitSsl();

	if (LoadAccountsModule()) return 1;

    //order becomes less important below here
	if (!disableDefaultModule[DEFMOD_UIFINDADD]) if (LoadFindAddModule()) return 1;
	if (!disableDefaultModule[DEFMOD_UIUSERINFO]) if (LoadUserInfoModule()) return 1;
	if (!disableDefaultModule[DEFMOD_SRURL]) if (LoadSendRecvUrlModule()) return 1;
	if (!disableDefaultModule[DEFMOD_SREMAIL]) if (LoadSendRecvEMailModule()) return 1;
	if (!disableDefaultModule[DEFMOD_SRAUTH]) if (LoadSendRecvAuthModule()) return 1;
	if (!disableDefaultModule[DEFMOD_SRFILE]) if (LoadSendRecvFileModule()) return 1;
	if (!disableDefaultModule[DEFMOD_UIHELP]) if (LoadHelpModule()) return 1;
	if (!disableDefaultModule[DEFMOD_UIHISTORY]) if (LoadHistoryModule()) return 1;
	if (!disableDefaultModule[DEFMOD_RNDIDLE]) if (LoadIdleModule()) return 1;
	if (!disableDefaultModule[DEFMOD_RNDAUTOAWAY]) if (LoadAutoAwayModule()) return 1;
	if (!disableDefaultModule[DEFMOD_RNDUSERONLINE]) if (LoadUserOnlineModule()) return 1;
	if (!disableDefaultModule[DEFMOD_SRAWAY]) if (LoadAwayMsgModule()) return 1;
	if (!disableDefaultModule[DEFMOD_RNDIGNORE]) if (LoadIgnoreModule()) return 1;
	if (!disableDefaultModule[DEFMOD_UIVISIBILITY]) if (LoadVisibilityModule()) return 1;
	if (!disableDefaultModule[DEFMOD_UPDATENOTIFY]) if (LoadUpdateNotifyModule()) return 1;
	return 0;
}

void UnloadDefaultModules(void)
{
	UnloadAccountsModule();
	UnloadNewPlugins();
	UnloadProtocolsModule();
	UnloadSkinSounds();
	UnloadSkinHotkeys();
//	UnloadErrorsModule();
	UnloadIcoLibModule();
	UnloadUtilsModule();
	UnloadButtonModule();
	UnloadClcModule();
	UnloadContactListModule();
	UnloadEventsModule();
	UnloadIdleModule();
	UnloadUpdateNotifyModule();
	UnloadNetlibModule();
	UnloadSslModule();
	UnloadLangPackModule();
}

int InitialiseModularEngine(void)
{
	InitializeCriticalSection(&csHooks);
	InitializeCriticalSection(&csServices);

	mainThreadId=GetCurrentThreadId();
	DuplicateHandle(GetCurrentProcess(),GetCurrentThread(),GetCurrentProcess(),&hMainThread,0,FALSE,DUPLICATE_SAME_ACCESS);

	hMissingService = CreateHookableEvent(ME_SYSTEM_MISSINGSERVICE);
	return 0;
}

void DestroyModularEngine(void)
{
	int i;
	THook* p;
	EnterCriticalSection( &csHooks );
	for( i=0; i < hooks.getCount(); i++ ) {
		p = hooks[i];
 		if ( p->subscriberCount )
			mir_free( p->subscriber );
		DeleteCriticalSection( &p->csHook );
		mir_free( p );
	}
	hooks.destroy();
	LeaveCriticalSection( &csHooks );
	DeleteCriticalSection( &csHooks );

	EnterCriticalSection( &csServices );
	for ( i=0; i < services.getCount(); i++ )
		mir_free( services[i] );

	services.destroy();
	LeaveCriticalSection( &csServices );
 	DeleteCriticalSection( &csServices );
	CloseHandle( hMainThread );
}

///////////////////////////////HOOKS

HANDLE CreateHookableEvent(const char *name)
{
	THook* ret;
	int    idx;

	if ( name == NULL )
		return NULL;

	EnterCriticalSection( &csHooks );
	if (( idx = hooks.getIndex(( THook* )name )) != -1 ) {
		LeaveCriticalSection( &csHooks );
		return NULL;
	}

	ret = ( THook* )mir_alloc( sizeof( THook ));
	strncpy( ret->name, name, sizeof( ret->name )); ret->name[ MAXMODULELABELLENGTH-1 ] = 0;
	ret->id = hookId++;
	ret->subscriberCount = 0;
	ret->subscriber = NULL;
	ret->pfnHook = NULL;
	InitializeCriticalSection( &ret->csHook );
	hooks.insert( ret );

	LeaveCriticalSection( &csHooks );
	return ( HANDLE )ret;
}

int DestroyHookableEvent( HANDLE hEvent )
{
	int idx;
	THook* p;

	EnterCriticalSection( &csHooks );
	if ( pLastHook == ( THook* )hEvent )
		pLastHook = NULL;

	if (( idx = hooks.getIndex(( THook* )hEvent )) == -1 ) {
      LeaveCriticalSection(&csHooks);
		return 1;
	}
	p = hooks[idx];
	if ( p->subscriberCount ) {
		mir_free( p->subscriber );
		p->subscriber = NULL;
		p->subscriberCount = 0;
	}
	hooks.remove( idx );
	DeleteCriticalSection( &p->csHook );
	mir_free( p );

	LeaveCriticalSection( &csHooks );
	return 0;
}

int SetHookDefaultForHookableEvent(HANDLE hEvent, MIRANDAHOOK pfnHook)
{
	THook* p = ( THook* )hEvent;

	EnterCriticalSection(&csHooks);
	if ( hooks.getIndex( p ) != -1 )
		p->pfnHook = pfnHook;
	LeaveCriticalSection(&csHooks);
	return 0;
}

int CallHookSubscribers( HANDLE hEvent, WPARAM wParam, LPARAM lParam )
{
	int i, returnVal = 0;
	THook* p = ( THook* )hEvent;
	if ( p == NULL )
		return -1;

	EnterCriticalSection( &p->csHook );

	// NOTE: We've got the critical section while all this lot are called. That's mostly safe, though.
	for ( i = 0; i < p->subscriberCount; i++ ) {
		THookSubscriber* s = &p->subscriber[i];
		switch ( s->type ) {
			case 1:	returnVal = s->pfnHook( wParam, lParam );	break;
			case 2:	returnVal = s->pfnHookParam( wParam, lParam, s->lParam ); break;
			case 3:	returnVal = s->pfnHookObj( s->object, wParam, lParam ); break;
			case 4:	returnVal = s->pfnHookObjParam( s->object, wParam, lParam, s->lParam ); break;
			case 5:	returnVal = SendMessage( s->hwnd, s->message, wParam, lParam ); break;
			default: continue;
		}
		if ( returnVal )
			break;
	}

	// check for no hooks and call the default hook if any
	if ( p->subscriberCount == 0 && p->pfnHook != 0 )
		returnVal = p->pfnHook( wParam, lParam );

	LeaveCriticalSection( &p->csHook );
	return returnVal;
}

static int checkHook( HANDLE hHook )
{
	if ( hHook == NULL )
		return -1;

	EnterCriticalSection( &csHooks );
	if ( pLastHook != hHook || !pLastHook ) {
		if ( hooks.getIndex(( THook* )hHook ) == -1 ) {
			LeaveCriticalSection( &csHooks );
			return -1;
		}
		pLastHook = ( THook* )hHook;
	}
	LeaveCriticalSection( &csHooks );
	return 0;
}

static void CALLBACK HookToMainAPCFunc(ULONG_PTR dwParam)
{
	THookToMainThreadItem* item = ( THookToMainThreadItem* )dwParam;

	if ( checkHook( item->hook ) == -1 )
		item->result = -1;
	else
		item->result = CallHookSubscribers( item->hook, item->wParam, item->lParam );
	SetEvent( item->hDoneEvent );
}

int NotifyEventHooks( HANDLE hEvent, WPARAM wParam, LPARAM lParam )
{
	extern HWND hAPCWindow;

	if ( GetCurrentThreadId() != mainThreadId ) {
		THookToMainThreadItem item;

		item.hDoneEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
		item.hook = ( THook* )hEvent;
		item.wParam = wParam;
		item.lParam = lParam;

		QueueUserAPC( HookToMainAPCFunc, hMainThread, ( ULONG_PTR )&item );
		PostMessage( hAPCWindow, WM_NULL, 0, 0 ); // let it process APC even if we're in a common dialog
		WaitForSingleObject( item.hDoneEvent, INFINITE );
		CloseHandle( item.hDoneEvent );
		return item.result;
	}

	return ( checkHook( hEvent ) == -1 ) ? -1 : CallHookSubscribers( hEvent, wParam, lParam );
}

static HANDLE HookEventInt( int type, const char* name, MIRANDAHOOK hookProc, void* object, LPARAM lParam )
{
	int idx;
	THook* p;
	HANDLE ret;

	EnterCriticalSection( &csHooks );
	if (( idx = hooks.getIndex(( THook* )name )) == -1 ) {
		#ifdef _DEBUG
			OutputDebugStringA("Attempt to hook: \t");
			OutputDebugStringA(name);
			OutputDebugStringA("\n");
		#endif
		LeaveCriticalSection(&csHooks);
		return NULL;
	}

	p = hooks[ idx ];
	p->subscriber = ( THookSubscriber* )mir_realloc( p->subscriber, sizeof( THookSubscriber )*( p->subscriberCount+1 ));
	p->subscriber[ p->subscriberCount ].type = type;
	p->subscriber[ p->subscriberCount ].pfnHook = hookProc;
	p->subscriber[ p->subscriberCount ].object = object;
	p->subscriber[ p->subscriberCount ].lParam = lParam;
	p->subscriber[ p->subscriberCount ].hOwner  = GetInstByAddress( hookProc );
	p->subscriberCount++;

	ret = ( HANDLE )(( p->id << 16 ) | p->subscriberCount );
	LeaveCriticalSection( &csHooks );
	return ret;
}

HANDLE HookEvent( const char* name, MIRANDAHOOK hookProc )
{
	return HookEventInt( 1, name, hookProc, 0, 0 );
}

HANDLE HookEventParam( const char* name, MIRANDAHOOKPARAM hookProc, LPARAM lParam )
{
	return HookEventInt( 2, name, (MIRANDAHOOK)hookProc, 0, lParam );
}

HANDLE HookEventObj( const char* name, MIRANDAHOOKOBJ hookProc, void* object)
{
	return HookEventInt( 3, name, (MIRANDAHOOK)hookProc, object, 0 );
}

HANDLE HookEventObjParam( const char* name, MIRANDAHOOKOBJPARAM hookProc, void* object, LPARAM lParam )
{
	return HookEventInt( 4, name, (MIRANDAHOOK)hookProc, object, lParam );
}

HANDLE HookEventMessage( const char* name, HWND hwnd, UINT message )
{
	int idx;
	THook* p;
	HANDLE ret;

	EnterCriticalSection( &csHooks );
	if (( idx = hooks.getIndex(( THook* )name )) == -1 ) {
		#ifdef _DEBUG
			MessageBoxA(NULL,"Attempt to hook non-existant event",name,MB_OK);
		#endif
		LeaveCriticalSection(&csHooks);
		return NULL;
	}

	p = hooks[ idx ];
	p->subscriber = ( THookSubscriber* )mir_realloc( p->subscriber, sizeof( THookSubscriber )*( p->subscriberCount+1 ));
	p->subscriber[ p->subscriberCount ].type = 5;
	p->subscriber[ p->subscriberCount ].hwnd = hwnd;
	p->subscriber[ p->subscriberCount ].message = message;
	p->subscriberCount++;

	ret = ( HANDLE )(( p->id << 16 ) | p->subscriberCount );
	LeaveCriticalSection( &csHooks );
	return ret;
}

int UnhookEvent( HANDLE hHook )
{
	int i;
	THook* p = NULL;

	int hookId = ( int )hHook >> 16;
	int subscriberId = (( int )hHook & 0xFFFF ) - 1;

    if (hHook == NULL) return 0;

    EnterCriticalSection( &csHooks );
	for ( i = 0; i < hooks.getCount(); i++ ) {
		if ( hooks[i]->id == hookId ) {
			p = hooks[i];
			break;
	}	}

	if ( p == NULL ) {
		LeaveCriticalSection( &csHooks );
		return 1;
	}

	if ( subscriberId >= p->subscriberCount || subscriberId < 0 ) {
		LeaveCriticalSection( &csHooks );
		return 1;
	}

	p->subscriber[subscriberId].type    = 0;
	p->subscriber[subscriberId].pfnHook = NULL;
	p->subscriber[subscriberId].hOwner  = NULL;
	while( p->subscriberCount && p->subscriber[p->subscriberCount-1].type == 0 )
		p->subscriberCount--;
	if ( p->subscriberCount == 0 ) {
		if ( p->subscriber ) mir_free( p->subscriber );
		p->subscriber = NULL;
	}
	LeaveCriticalSection( &csHooks );
	return 0;
}

void KillModuleEventHooks( HINSTANCE hInst )
{
	int i, j;

	EnterCriticalSection(&csHooks);
	for ( i = hooks.getCount()-1; i >= 0; i-- ) {
		if ( hooks[i]->subscriberCount == 0 )
			continue;

		for ( j = hooks[i]->subscriberCount-1; j >= 0; j-- ) {
			if ( hooks[i]->subscriber[j].hOwner == hInst ) {
				char szModuleName[ MAX_PATH ];
				GetModuleFileNameA( hooks[i]->subscriber[j].hOwner, szModuleName, sizeof(szModuleName));
				Netlib_Logf( NULL, "A hook %08x for event '%s' was abnormally deleted because module '%s' didn't released it",
					hooks[i]->subscriber[j].pfnHook, hooks[i]->name, szModuleName );
				UnhookEvent(( HANDLE )(( hooks[i]->id << 16 ) + j + 1 ));
				if ( hooks[i]->subscriberCount == 0 )
					break;
	}	}	}

	LeaveCriticalSection(&csHooks);
}

void KillObjectEventHooks( void* pObject )
{
	int i, j;

	EnterCriticalSection(&csHooks);
	for ( i = hooks.getCount()-1; i >= 0; i-- ) {
		if ( hooks[i]->subscriberCount == 0 )
			continue;

		for ( j = hooks[i]->subscriberCount-1; j >= 0; j-- ) {
			if ( hooks[i]->subscriber[j].object == pObject ) {
				UnhookEvent(( HANDLE )(( hooks[i]->id << 16 ) + j + 1 ));
				if ( hooks[i]->subscriberCount == 0 )
					break;
	}	}	}

	LeaveCriticalSection(&csHooks);
}

/////////////////////SERVICES

static __inline TService* FindServiceByName( const char *name )
{
	unsigned hash = hashstr( name );
	return services.find(( TService* )&hash );
}

static HANDLE CreateServiceInt( int type, const char *name, MIRANDASERVICE serviceProc, void* object, LPARAM lParam)
{
	if ( name == NULL ) {
#ifdef _DEBUG
		MessageBoxA(0,"Someone tried to create a NULL'd service, see call stack for more info","",0);
		DebugBreak();
#endif
		return NULL;
	}

	TService tmp;
	tmp.nameHash = hashstr( name );

	EnterCriticalSection( &csServices );

	if ( services.getIndex( &tmp ) != -1 ) {
		LeaveCriticalSection( &csServices );
		return NULL;
	}

	TService* p = ( TService* )mir_alloc( sizeof( *p ) + strlen( name ));
	strcpy( p->name, name );
	p->nameHash   = tmp.nameHash;
	p->pfnService = serviceProc;
	p->hOwner     = GetInstByAddress( serviceProc );
	p->flags      = type;
	p->lParam     = lParam;
	p->object     = object;
	services.insert( p );

	LeaveCriticalSection( &csServices );
	return ( HANDLE )tmp.nameHash;
}

HANDLE CreateServiceFunction( const char *name, MIRANDASERVICE serviceProc )
{
	return CreateServiceInt( 0, name, serviceProc, 0, 0 );
}

HANDLE CreateServiceFunctionParam(const char *name,MIRANDASERVICEPARAM serviceProc,LPARAM lParam)
{
	return CreateServiceInt( 1, name, (MIRANDASERVICE)serviceProc, 0, lParam );
}

HANDLE CreateServiceFunctionObj(const char *name,MIRANDASERVICEOBJ serviceProc,void* object)
{
	return CreateServiceInt( 2, name, (MIRANDASERVICE)serviceProc, object, 0 );
}

HANDLE CreateServiceFunctionObjParam(const char *name,MIRANDASERVICEOBJPARAM serviceProc,void* object,LPARAM lParam)
{
	return CreateServiceInt( 3, name, (MIRANDASERVICE)serviceProc, object, lParam );
}

int DestroyServiceFunction(HANDLE hService)
{
	int idx;

	EnterCriticalSection( &csServices );
	if (( idx = services.getIndex(( TService* )&hService )) != -1 ) {
		mir_free( services[idx] );
		services.remove( idx );
	}

	LeaveCriticalSection(&csServices);
	return 0;
}

int ServiceExists(const char *name)
{
	if ( name == NULL )
		return FALSE;

	EnterCriticalSection( &csServices );
	int ret = FindServiceByName( name ) != NULL;
	LeaveCriticalSection( &csServices );
	return ret;
}

INT_PTR CallService(const char *name,WPARAM wParam,LPARAM lParam)
{
	#ifdef _DEBUG
		if (name==NULL) {
			MessageBoxA(0,"Someone tried to CallService(NULL,..) see stack trace for details","",0);
			DebugBreak();
			return CALLSERVICE_NOTFOUND;
		}
	#else
		if (name==NULL) return CALLSERVICE_NOTFOUND;
	#endif

	EnterCriticalSection(&csServices);
	TService *pService = FindServiceByName(name);
	if (pService==NULL) {
		LeaveCriticalSection(&csServices);
		#ifdef _DEBUG
			//MessageBoxA(NULL,"Attempt to call non-existant service",name,MB_OK);
			OutputDebugStringA("Missing service called: \t");
			OutputDebugStringA(name);
			OutputDebugStringA("\n");
		#endif
/*		{	MISSING_SERVICE_PARAMS params = { name, wParam, lParam };
			int result = NotifyEventHooks(hMissingService,0,(LPARAM)&params);
			if (result != 0)
				return params.lParam;
		} */
		return CALLSERVICE_NOTFOUND;
	}

	MIRANDASERVICE pfnService = pService->pfnService;
	int flags = pService->flags;
	LPARAM fnParam = pService->lParam;
	void* object = pService->object;
	LeaveCriticalSection(&csServices);
	switch( flags ) {
		case 1:  return ((MIRANDASERVICEPARAM)pfnService)(wParam,lParam,fnParam);
		case 2:  return ((MIRANDASERVICEOBJ)pfnService)(object,wParam,lParam);
		case 3:  return ((MIRANDASERVICEOBJPARAM)pfnService)(object,wParam,lParam,fnParam);
		default: return pfnService(wParam,lParam);
}	}

static void CALLBACK CallServiceToMainAPCFunc(ULONG_PTR dwParam)
{
	TServiceToMainThreadItem *item = (TServiceToMainThreadItem*) dwParam;
	item->result = CallService(item->name, item->wParam, item->lParam);
	SetEvent(item->hDoneEvent);
}

INT_PTR CallServiceSync(const char *name, WPARAM wParam, LPARAM lParam)
{
	extern HWND hAPCWindow;

	if (name==NULL) return CALLSERVICE_NOTFOUND;
	// the service is looked up within the main thread, since the time it takes
	// for the APC queue to clear the service being called maybe removed.
	// even thou it may exists before the call, the critsec can't be locked between calls.
	if (GetCurrentThreadId() != mainThreadId) {
		TServiceToMainThreadItem item;
		item.wParam = wParam;
		item.lParam = lParam;
		item.name = name;
		item.hDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		QueueUserAPC(CallServiceToMainAPCFunc, hMainThread, (ULONG_PTR) &item);
		PostMessage(hAPCWindow,WM_NULL,0,0); // let this get processed in its own time
		WaitForSingleObject(item.hDoneEvent, INFINITE);
		CloseHandle(item.hDoneEvent);
		return item.result;
	}

   return CallService(name, wParam, lParam);
}

int CallFunctionAsync( void (__stdcall *func)(void *), void *arg)
{
	extern HWND hAPCWindow;
	int r = QueueUserAPC(( void (__stdcall *)( ULONG_PTR ))func, hMainThread, ( ULONG_PTR )arg );
	PostMessage(hAPCWindow,WM_NULL,0,0);
	return r;
}

void KillModuleServices( HINSTANCE hInst )
{
	int i;

	EnterCriticalSection(&csServices);
	for ( i = services.getCount()-1; i >= 0; i-- ) {
		if ( services[i]->hOwner == hInst ) {
			char szModuleName[ MAX_PATH ];
			GetModuleFileNameA( services[i]->hOwner, szModuleName, sizeof(szModuleName));
			Netlib_Logf( NULL, "A service function '%s' was abnormally deleted because module '%s' didn't released it",
				services[i]->name, szModuleName );
			DestroyServiceFunction(( HANDLE )services[i]->nameHash );
	}	}

	LeaveCriticalSection(&csServices);
}

void KillObjectServices( void* pObject )
{
	int i;

	EnterCriticalSection(&csServices);
	for ( i = services.getCount()-1; i >= 0; i-- )
		if ( services[i]->object == pObject )
			DestroyServiceFunction(( HANDLE )services[i]->nameHash );

	LeaveCriticalSection(&csServices);
}
