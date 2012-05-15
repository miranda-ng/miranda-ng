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

int LoadProtoChains(void);
int LoadProtoOptions( void );

HANDLE hAccListChanged;
static HANDLE hAckEvent,hTypeEvent;
static BOOL bModuleInitialized = FALSE;

typedef struct
{
	const char* name;
	int id;
}
	TServiceListItem;

static int CompareServiceItems( const TServiceListItem* p1, const TServiceListItem* p2 )
{	return strcmp( p1->name, p2->name );
}

static LIST<TServiceListItem> serviceItems( 10, CompareServiceItems );

//------------------------------------------------------------------------------------

static int CompareProtos( const PROTOCOLDESCRIPTOR* p1, const PROTOCOLDESCRIPTOR* p2 )
{	return strcmp( p1->szName, p2->szName );
}

static LIST<PROTOCOLDESCRIPTOR> protos( 10, CompareProtos );

static INT_PTR Proto_BroadcastAck(WPARAM wParam, LPARAM lParam)
{
#ifdef _UNICODE
	ACKDATA *ack = (ACKDATA*)lParam;
	if (ack && ack->type == ACKTYPE_AVATAR && ack->hProcess)
	{
		PROTO_AVATAR_INFORMATION* ai = (PROTO_AVATAR_INFORMATION*)ack->hProcess;
		if (ai->cbSize == sizeof(PROTO_AVATAR_INFORMATION))
		{
			PROTO_AVATAR_INFORMATIONW aiw = { sizeof(aiw), ai->hContact, ai->format };
			MultiByteToWideChar(CP_ACP, 0, ai->filename, -1, aiw.filename, SIZEOF(aiw.filename));

			ack->hProcess = &aiw;
		}	
	}
#endif

	return NotifyEventHooks(hAckEvent, wParam, lParam);
}

INT_PTR __fastcall MyCallProtoService( const char *szModule, const char *szService, WPARAM wParam, LPARAM lParam );
void FreeFilesMatrix( TCHAR ***files );

PROTOCOLDESCRIPTOR* __fastcall Proto_IsProtocolLoaded( const char* szProtoName )
{
	if ( szProtoName ) {
		PROTOCOLDESCRIPTOR tmp;
		tmp.szName = ( char* )szProtoName;
		return protos.find( &tmp );
	}
	return NULL;
}

INT_PTR srvProto_IsLoaded(WPARAM, LPARAM lParam)
{
	return (INT_PTR)Proto_GetAccount(( char* )lParam );
}

INT_PTR Proto_EnumProtocols(WPARAM wParam,LPARAM lParam)
{
	*( int* )wParam = protos.getCount();
	*( PROTOCOLDESCRIPTOR*** )lParam = protos.getArray();
	return 0;
}

static PROTO_INTERFACE* defInitProto( const char* szModuleName, const TCHAR* )
{
	return AddDefaultAccount( szModuleName );
}

static INT_PTR Proto_RegisterModule(WPARAM, LPARAM lParam)
{
	PROTOCOLDESCRIPTOR* pd = ( PROTOCOLDESCRIPTOR* )lParam, *p;
	if ( pd->cbSize != sizeof( PROTOCOLDESCRIPTOR ) && pd->cbSize != PROTOCOLDESCRIPTOR_V3_SIZE )
		return 1;

	p = ( PROTOCOLDESCRIPTOR* )mir_alloc( sizeof( PROTOCOLDESCRIPTOR ));
	if ( !p )
		return 2;

	if ( pd->cbSize == PROTOCOLDESCRIPTOR_V3_SIZE ) {
		memset( p, 0, sizeof( PROTOCOLDESCRIPTOR ));
		p->cbSize = PROTOCOLDESCRIPTOR_V3_SIZE;
		p->type = pd->type;
		if ( p->type == PROTOTYPE_PROTOCOL ) {
			// let's create a new container
			PROTO_INTERFACE* ppi = AddDefaultAccount( pd->szName );
			if ( ppi ) {
				PROTOACCOUNT* pa = Proto_GetAccount( pd->szName );
				if ( pa == NULL ) {
					pa = (PROTOACCOUNT*)mir_calloc( sizeof( PROTOACCOUNT ));
					pa->cbSize = sizeof(PROTOACCOUNT);
					pa->type = PROTOTYPE_PROTOCOL;
					pa->szModuleName = mir_strdup( pd->szName );
					pa->szProtoName = mir_strdup( pd->szName );
					pa->tszAccountName = mir_a2t( pd->szName );
					pa->bIsVisible = pa->bIsEnabled = TRUE;
					pa->iOrder = accounts.getCount();
					accounts.insert( pa );
				}
				pa->bOldProto = TRUE;
				pa->ppro = ppi;
				p->fnInit = defInitProto;
				p->fnUninit = FreeDefaultAccount;
			}
		}
	}
	else *p = *pd;
	p->szName = mir_strdup( pd->szName );
	protos.insert( p );
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Basic core services

static INT_PTR Proto_RecvFile(WPARAM,LPARAM lParam)
{
	CCSDATA* ccs = ( CCSDATA* )lParam;
	PROTORECVEVENT* pre = ( PROTORECVEVENT* )ccs->lParam;
	char* szFile = pre->szMessage + sizeof( DWORD );
	char* szDescr = szFile + strlen( szFile ) + 1;

	// Suppress the standard event filter
	if ( pre->lParam != NULL )
		*( DWORD* )pre->szMessage = 0;

	DBEVENTINFO dbei = { 0 };
	dbei.cbSize = sizeof( dbei );
	dbei.szModule = ( char* )CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)ccs->hContact, 0);
	dbei.timestamp = pre->timestamp;
	dbei.flags = ( pre->flags & PREF_CREATEREAD ) ? DBEF_READ : 0;
	dbei.flags |= ( pre->flags & PREF_UTF ) ? DBEF_UTF : 0;
	dbei.eventType = EVENTTYPE_FILE;
	dbei.cbBlob = (DWORD)(sizeof( DWORD ) + strlen( szFile ) + strlen( szDescr ) + 2);
	dbei.pBlob = ( PBYTE )pre->szMessage;
	HANDLE hdbe = ( HANDLE )CallService( MS_DB_EVENT_ADD, ( WPARAM )ccs->hContact, ( LPARAM )&dbei );

	if ( pre->lParam != NULL )
		PushFileEvent( ccs->hContact, hdbe, pre->lParam );
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void sttRecvCreateBlob( DBEVENTINFO& dbei, int fileCount, char** pszFiles, char* szDescr )
{
	dbei.cbBlob = sizeof( DWORD );
	{
		for ( int i=0; i < fileCount; i++ )
			dbei.cbBlob += lstrlenA( pszFiles[i] ) + 1;
	}
	
	dbei.cbBlob += lstrlenA( szDescr ) + 1;

	if (( dbei.pBlob = ( BYTE* )mir_alloc( dbei.cbBlob )) == 0 )
		return;

	*( DWORD* )dbei.pBlob = 0;
	BYTE* p = dbei.pBlob + sizeof( DWORD );
	for ( int i=0; i < fileCount; i++ ) {
		strcpy(( char* )p, pszFiles[i] );
		p += lstrlenA( pszFiles[i] ) + 1;
	}
	strcpy(( char* )p, ( szDescr == NULL ) ? "" : szDescr );
}

static INT_PTR Proto_RecvFileT(WPARAM,LPARAM lParam)
{
	CCSDATA* ccs = ( CCSDATA* )lParam;
	PROTORECVFILET* pre = ( PROTORECVFILET* )ccs->lParam;
	if ( pre->fileCount == 0 )
		return 0;

	DBEVENTINFO dbei = { 0 };
	dbei.cbSize = sizeof( dbei );
	dbei.szModule = ( char* )CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)ccs->hContact, 0);
	dbei.timestamp = pre->timestamp;
	dbei.flags = ( pre->flags & PREF_CREATEREAD ) ? DBEF_READ : 0;
	dbei.eventType = EVENTTYPE_FILE;

	char** pszFiles = ( char** )alloca( pre->fileCount * sizeof(char*));
	{
		for ( int i=0; i < pre->fileCount; i++ )
			pszFiles[i] = Utf8EncodeT( pre->ptszFiles[i] );
	}
	char* szDescr = Utf8EncodeT( pre->tszDescription );
	dbei.flags |= DBEF_UTF;
	sttRecvCreateBlob( dbei, pre->fileCount, pszFiles, szDescr );
	{
		for ( int i=0; i < pre->fileCount; i++ )
			mir_free( pszFiles[i] );
	}
	mir_free( szDescr );

	HANDLE hdbe = ( HANDLE )CallService( MS_DB_EVENT_ADD, ( WPARAM )ccs->hContact, ( LPARAM )&dbei );

	PushFileEvent( ccs->hContact, hdbe, pre->lParam );
	mir_free( dbei.pBlob );
	return 0;
}

static INT_PTR Proto_RecvMessage(WPARAM,LPARAM lParam)
{
	CCSDATA *ccs = ( CCSDATA* )lParam;
	PROTORECVEVENT *pre = ( PROTORECVEVENT* )ccs->lParam;

    if (pre->szMessage == NULL) return NULL;

	DBEVENTINFO dbei = { 0 };
	dbei.cbSize = sizeof( dbei );
	dbei.szModule = ( char* )CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)ccs->hContact, 0);
	dbei.timestamp = pre->timestamp;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = (DWORD)strlen( pre->szMessage ) + 1;
	if ( pre->flags & PREF_CREATEREAD )
		dbei.flags |= DBEF_READ;
	if ( pre->flags & PREF_UTF )
		dbei.flags |= DBEF_UTF;
	if ( pre->flags & PREF_UNICODE )
		dbei.cbBlob += sizeof( wchar_t )*( (DWORD)wcslen(( wchar_t* )&pre->szMessage[dbei.cbBlob+1] )+1 );

	dbei.pBlob = ( PBYTE ) pre->szMessage;
	return CallService( MS_DB_EVENT_ADD, ( WPARAM ) ccs->hContact, ( LPARAM )&dbei );
}

/////////////////////////////////////////////////////////////////////////////////////////
// User Typing Notification services

static int Proto_ValidTypingContact(HANDLE hContact, char *szProto)
{
	if ( !hContact || !szProto )
		return 0;

	return ( CallProtoService(szProto,PS_GETCAPS,PFLAGNUM_4,0) & PF4_SUPPORTTYPING ) ? 1 : 0;
}

static INT_PTR Proto_SelfIsTyping(WPARAM wParam,LPARAM lParam)
{
	if ( lParam == PROTOTYPE_SELFTYPING_OFF || lParam == PROTOTYPE_SELFTYPING_ON ) {
		char* szProto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, wParam, 0 );
		if ( !szProto )
			return 0;

		if ( Proto_ValidTypingContact(( HANDLE )wParam, szProto ))
			CallProtoService( szProto, PSS_USERISTYPING, wParam, lParam );
	}

	return 0;
}

static INT_PTR Proto_ContactIsTyping(WPARAM wParam,LPARAM lParam)
{
	int type = (int)lParam;
	char *szProto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, wParam, 0 );
	if ( !szProto )
		return 0;

	if ( CallService( MS_IGNORE_ISIGNORED, wParam, IGNOREEVENT_TYPINGNOTIFY ))
		return 0;

	if ( type < PROTOTYPE_CONTACTTYPING_OFF )
		return 0;

	if ( Proto_ValidTypingContact(( HANDLE )wParam, szProto ))
		NotifyEventHooks( hTypeEvent, wParam, lParam );

	return 0;
}

void Proto_SetStatus(const char* szProto, unsigned status)
{
	if (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND) 
	{
		TCHAR* awayMsg = (TCHAR* )CallService(MS_AWAYMSG_GETSTATUSMSGW, (WPARAM) status, (LPARAM) szProto);
		if ((INT_PTR)awayMsg == CALLSERVICE_NOTFOUND) 
		{
			char* awayMsgA = (char*)CallService(MS_AWAYMSG_GETSTATUSMSG, (WPARAM) status, (LPARAM) szProto);
			if ((INT_PTR)awayMsgA != CALLSERVICE_NOTFOUND)
			{
				awayMsg = mir_a2t(awayMsgA);
				mir_free(awayMsgA);
			}
		}
		if ((INT_PTR)awayMsg != CALLSERVICE_NOTFOUND) 
		{
			CallProtoService(szProto, PS_SETAWAYMSGT, status, (LPARAM) awayMsg);
			mir_free(awayMsg);
	}	}
	CallProtoService(szProto, PS_SETSTATUS, status, 0);
}

#ifdef _UNICODE
char** __fastcall Proto_FilesMatrixA( wchar_t **files )
{
	if ( files == NULL ) return NULL;

	int count = 0;
	while( files[ count++ ] );

	char** filesA = ( char** )mir_alloc( count * sizeof( char* ));
	for( int i = 0; i < count; ++i )
		filesA[ i ] = mir_u2a( files[ i ] );

	return filesA;
}

static wchar_t** __fastcall Proto_FilesMatrixU( char **files )
{
	if ( files == NULL ) return NULL;

	int count = 0;
	while( files[ count++ ] );

	wchar_t** filesU = ( wchar_t** )mir_alloc( count * sizeof( wchar_t* ));
	for( int i = 0; i < count; ++i )
		filesU[ i ] = mir_a2u( files[ i ] );

	return filesU;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// 0.8.0+ - accounts

PROTOACCOUNT* __fastcall Proto_GetAccount( const char* accName )
{
	int idx;
	PROTOACCOUNT temp;
	temp.szModuleName = ( char* )accName;
	if (( idx = accounts.getIndex( &temp )) == -1 )
		return NULL;

	return accounts[idx];
}

static INT_PTR srvProto_GetAccount(WPARAM, LPARAM lParam)
{
	return ( INT_PTR )Proto_GetAccount(( char* )lParam );
}

static INT_PTR Proto_EnumAccounts(WPARAM wParam, LPARAM lParam)
{
	*( int* )wParam = accounts.getCount();
	*( PROTOACCOUNT*** )lParam = accounts.getArray();
	return 0;
}

bool __fastcall Proto_IsAccountEnabled( PROTOACCOUNT* pa )
{
	return pa && (( pa->bIsEnabled && !pa->bDynDisabled ) || pa->bOldProto );
}

static INT_PTR srvProto_IsAccountEnabled(WPARAM, LPARAM lParam)
{
	return ( INT_PTR )Proto_IsAccountEnabled(( PROTOACCOUNT* )lParam);
}

bool __fastcall Proto_IsAccountLocked( PROTOACCOUNT* pa )
{
	return pa && DBGetContactSettingByte(NULL, pa->szModuleName, "LockMainStatus", 0) != 0;
}

static INT_PTR srvProto_IsAccountLocked(WPARAM, LPARAM lParam)
{
	return ( INT_PTR )Proto_IsAccountLocked( Proto_GetAccount(( char* )lParam ));
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CallProtoServiceInt( HANDLE hContact, const char *szModule, const char *szService, WPARAM wParam, LPARAM lParam )
{
	PROTOACCOUNT* pa = Proto_GetAccount( szModule );
	if ( pa && !pa->bOldProto ) {
		PROTO_INTERFACE* ppi;
		if (( ppi = pa->ppro ) == NULL )
			return CALLSERVICE_NOTFOUND;
		else {
			TServiceListItem *item = serviceItems.find(( TServiceListItem* )&szService );
			if ( item ) {
				switch( item->id ) {
					case  1: 
#ifdef _UNICODE
						if ( ppi->m_iVersion > 1 || !((( PROTOSEARCHRESULT* )lParam)->flags & PSR_UNICODE))
							return ( INT_PTR )ppi->AddToList( wParam, (PROTOSEARCHRESULT*)lParam ); 
						else {
							PROTOSEARCHRESULT *psr = ( PROTOSEARCHRESULT* )lParam;
							PROTOSEARCHRESULT *psra =( PROTOSEARCHRESULT* )mir_alloc( psr->cbSize );
							memcpy( psra, psr, psr->cbSize );
							psra->nick      = ( PROTOCHAR* )mir_u2a( psr->nick );
							psra->firstName = ( PROTOCHAR* )mir_u2a( psr->firstName );
							psra->lastName  = ( PROTOCHAR* )mir_u2a( psr->lastName );
							psra->email     = ( PROTOCHAR* )mir_u2a( psr->email );
							
							INT_PTR res = ( INT_PTR )ppi->AddToList( wParam, psra );
							
							mir_free( psra->nick );
							mir_free( psra->firstName );
							mir_free( psra->lastName );
							mir_free( psra->email );
							mir_free( psra );
							
							return res;
						}
#else
						return ( INT_PTR )ppi->AddToList( wParam, (PROTOSEARCHRESULT*)lParam ); 
#endif
					case  2: return ( INT_PTR )ppi->AddToListByEvent( LOWORD(wParam), HIWORD(wParam), (HANDLE)lParam ); 
					case  3: return ( INT_PTR )ppi->Authorize( ( HANDLE )wParam ); 
					case  4:
						if ( ppi->m_iVersion > 1 )
							return ( INT_PTR )ppi->AuthDeny(( HANDLE )wParam,  StrConvT(( char* )lParam )); 
						else
							return ( INT_PTR )ppi->AuthDeny(( HANDLE )wParam, ( PROTOCHAR* )lParam ); 
					case  5: return ( INT_PTR )ppi->AuthRecv( hContact, ( PROTORECVEVENT* )lParam ); 
					case  6:
						if ( ppi->m_iVersion > 1 )
							return ( INT_PTR )ppi->AuthRequest( hContact,  StrConvT(( char* )lParam )); 
						else
							return ( INT_PTR )ppi->AuthRequest( hContact, ( PROTOCHAR* )lParam ); 
					case  7: return ( INT_PTR )ppi->ChangeInfo( wParam, ( void* )lParam ); 
					case  8:
						if ( ppi->m_iVersion > 1 )
							return ( INT_PTR )ppi->FileAllow( hContact, ( HANDLE )wParam,  StrConvT(( char* )lParam )); 
						else
							return ( INT_PTR )ppi->FileAllow( hContact, ( HANDLE )wParam, ( PROTOCHAR* )lParam ); 
					case  9: return ( INT_PTR )ppi->FileCancel( hContact, ( HANDLE )wParam ); 
					case  10:
						if ( ppi->m_iVersion > 1 )
							return ( INT_PTR )ppi->FileDeny( hContact, ( HANDLE )wParam,  StrConvT(( char* )lParam )); 
						else
							return ( INT_PTR )ppi->FileDeny( hContact, ( HANDLE )wParam, ( PROTOCHAR* )lParam ); 
					case 11: {
						PROTOFILERESUME* pfr = ( PROTOFILERESUME* )lParam;
#ifdef _UNICODE
						if ( ppi->m_iVersion > 1 ) {
							PROTOCHAR* szFname = mir_a2t(( char* )pfr->szFilename );
							INT_PTR res = ( INT_PTR )ppi->FileResume(( HANDLE )wParam, &pfr->action, 
								( const PROTOCHAR** )&szFname);
							mir_free(( PROTOCHAR* )pfr->szFilename ); 
							pfr->szFilename = ( PROTOCHAR* )mir_t2a( szFname ); mir_free( szFname ); 
						}
						else
#endif
							return ( INT_PTR )ppi->FileResume(( HANDLE )wParam, &pfr->action, 
								( const PROTOCHAR** )&pfr->szFilename );
					}
					case 12: return ( INT_PTR )ppi->GetCaps( wParam, (HANDLE)lParam );
					case 13: return ( INT_PTR )ppi->GetIcon( wParam );
					case 14: return ( INT_PTR )ppi->GetInfo( hContact, wParam );;
					case 15: 
						if ( ppi->m_iVersion > 1 )
							return ( INT_PTR )ppi->SearchBasic( StrConvT(( char* )lParam ));
						else
							return ( INT_PTR )ppi->SearchBasic(( TCHAR* )lParam ); 
					case 16: 
						if ( ppi->m_iVersion > 1 )
							return ( INT_PTR )ppi->SearchByEmail( StrConvT(( char* )lParam ));
						else
							return ( INT_PTR )ppi->SearchByEmail(( TCHAR* )lParam ); 
					case 17: {
						PROTOSEARCHBYNAME* psbn = ( PROTOSEARCHBYNAME* )lParam;
						if ( ppi->m_iVersion > 1 )
							return ( INT_PTR )ppi->SearchByName( StrConvT(( char* )psbn->pszNick ), 
								StrConvT(( char* )psbn->pszFirstName ), StrConvT(( char* )psbn->pszLastName ));
						else
							return ( INT_PTR )ppi->SearchByName( psbn->pszNick, psbn->pszFirstName, psbn->pszLastName ); 
					}
					case 18: return ( INT_PTR )ppi->SearchAdvanced( ( HWND )lParam ); 
					case 19: return ( INT_PTR )ppi->CreateExtendedSearchUI ( ( HWND )lParam ); 
					case 20: return ( INT_PTR )ppi->RecvContacts( hContact, ( PROTORECVEVENT* )lParam ); 
					case 21: return ( INT_PTR )ppi->RecvFile( hContact, ( PROTOFILEEVENT* )lParam ); 
					case 22: return ( INT_PTR )ppi->RecvMsg( hContact, ( PROTORECVEVENT* )lParam ); 
					case 23: return ( INT_PTR )ppi->RecvUrl( hContact, ( PROTORECVEVENT* )lParam ); 
					case 24: return ( INT_PTR )ppi->SendContacts( hContact, LOWORD( wParam ), HIWORD( wParam ), 
								 ( HANDLE* )lParam ); 
					case 25:
#ifdef _UNICODE
						if ( ppi->m_iVersion > 1 ) {
							TCHAR** files = Proto_FilesMatrixU(( char** )lParam );
							INT_PTR res = ( INT_PTR )ppi->SendFile( hContact, StrConvT(( char* )wParam ), ( TCHAR** )files ); 
							if ( res == 0 ) FreeFilesMatrix( &files );
							return res;
						}
						else
#endif
							return ( INT_PTR )ppi->SendFile( hContact, ( TCHAR* )wParam, ( TCHAR** )lParam );
					case 26: return ( INT_PTR )ppi->SendMsg( hContact, wParam, ( const char* )lParam ); 
					case 27: return ( INT_PTR )ppi->SendUrl( hContact, wParam, ( const char* )lParam ); 
					case 28: return ( INT_PTR )ppi->SetApparentMode( hContact, wParam ); 
					case 29: return ( INT_PTR )ppi->SetStatus( wParam ); 
					case 30: return ( INT_PTR )ppi->GetAwayMsg( hContact ); 
					case 31: return ( INT_PTR )ppi->RecvAwayMsg( hContact, wParam, ( PROTORECVEVENT* )lParam ); 
					case 32: return ( INT_PTR )ppi->SendAwayMsg( hContact, ( HANDLE )wParam, ( const char* )lParam ); 
					case 33: 
						if ( ppi->m_iVersion > 1 )
							return ( INT_PTR )ppi->SetAwayMsg( wParam, StrConvT(( char* )lParam ));
						else
							return ( INT_PTR )ppi->SetAwayMsg( wParam, ( TCHAR* )lParam ); 
					case 34: return ( INT_PTR )ppi->UserIsTyping( ( HANDLE )wParam, lParam ); 
					case 35: lstrcpynA(( char* )lParam, ppi->m_szModuleName, wParam ); return 0; 
					case 36: return ppi->m_iStatus; 
					
#ifdef _UNICODE
					case 100:
						if ( ppi->m_iVersion > 1 )
							return ( INT_PTR )ppi->SetAwayMsg( wParam, ( TCHAR* )lParam );
						else
							return ( INT_PTR )ppi->SetAwayMsg( wParam, StrConvA(( TCHAR* )lParam )); 
					case 102:
						if ( ppi->m_iVersion > 1 )
							return ( INT_PTR )ppi->SendFile( hContact, ( TCHAR* )wParam, ( TCHAR** )lParam );
						else {
							char** files = Proto_FilesMatrixA(( TCHAR** )lParam );
							INT_PTR res = ( INT_PTR )ppi->SendFile( hContact, StrConvA(( TCHAR* )wParam ), ( TCHAR** )files ); 
							if ( res == 0 ) FreeFilesMatrix(( TCHAR*** )&files );
							return res;
						}
					case 103:
						if ( ppi->m_iVersion > 1 )
							return ( INT_PTR )ppi->FileAllow( hContact, ( HANDLE )wParam, ( TCHAR* )lParam );
						else
							return ( INT_PTR )ppi->FileAllow( hContact, ( HANDLE )wParam, StrConvA(( TCHAR* )lParam )); 
					case 104:
						if ( ppi->m_iVersion > 1 )
							return ( INT_PTR )ppi->FileDeny( hContact, ( HANDLE )wParam, ( TCHAR* )lParam );
						else
							return ( INT_PTR )ppi->FileDeny( hContact, ( HANDLE )wParam, StrConvA(( TCHAR* )lParam )); 
					case 105: {
						PROTOFILERESUME* pfr = ( PROTOFILERESUME* )lParam;
						if ( ppi->m_iVersion > 1 )
							return ( INT_PTR )ppi->FileResume(( HANDLE )wParam, &pfr->action, 
								( const PROTOCHAR** )&pfr->szFilename );
						else {
							char* szFname = mir_t2a( pfr->szFilename );
							INT_PTR res = ( INT_PTR )ppi->FileResume(( HANDLE )wParam, &pfr->action, 
								( const PROTOCHAR** )&szFname);
							mir_free( szFname );
					}	}
					case 106:
						if ( ppi->m_iVersion > 1 )
							return ( INT_PTR )ppi->AuthRequest( hContact, ( const TCHAR* )lParam );
						else
							return ( INT_PTR )ppi->AuthRequest( hContact, StrConvA(( const TCHAR* )lParam )); 
					case 107:
						if ( ppi->m_iVersion > 1 )
							return ( INT_PTR )ppi->AuthDeny(( HANDLE )wParam, ( const TCHAR* )lParam );
						else
							return ( INT_PTR )ppi->AuthDeny(( HANDLE )wParam, StrConvA(( const TCHAR* )lParam )); 
					case 108:
						if ( ppi->m_iVersion > 1 )
							return ( INT_PTR )ppi->SearchBasic(( const TCHAR* )lParam );
						else
							return ( INT_PTR )ppi->SearchBasic(StrConvA(( const TCHAR* )lParam )); 
					case 109: {
						PROTOSEARCHBYNAME* psbn = ( PROTOSEARCHBYNAME* )lParam;
						if ( ppi->m_iVersion > 1 )
							return ( INT_PTR )ppi->SearchByName( psbn->pszNick, psbn->pszFirstName, psbn->pszLastName ); 
						else
							return ( INT_PTR )ppi->SearchByName( StrConvA(( TCHAR* )psbn->pszNick ), 
								StrConvA(( TCHAR* )psbn->pszFirstName ), StrConvA(( TCHAR* )psbn->pszLastName ));
					}
					case 110:
						if ( ppi->m_iVersion > 1 )
							return ( INT_PTR )ppi->SearchByEmail(( const TCHAR* )lParam );
						else
							return ( INT_PTR )ppi->SearchByEmail(StrConvA(( const TCHAR* )lParam )); 
#endif
	}	}	}	}

#ifdef _UNICODE
	if ( strcmp( szService, PS_ADDTOLIST ) == 0 ) {
		PROTOSEARCHRESULT *psr = ( PROTOSEARCHRESULT* )lParam;
		PROTOSEARCHRESULT *psra =( PROTOSEARCHRESULT* )mir_alloc( psr->cbSize );
		memcpy( psra, psr, psr->cbSize );
		psra->nick      = ( PROTOCHAR* )mir_u2a( psr->nick );
		psra->firstName = ( PROTOCHAR* )mir_u2a( psr->firstName );
		psra->lastName  = ( PROTOCHAR* )mir_u2a( psr->lastName );
		psra->email     = ( PROTOCHAR* )mir_u2a( psr->email );
		
		INT_PTR res = MyCallProtoService( szModule, szService, wParam, ( LPARAM )psra );
		
		mir_free( psra->nick );
		mir_free( psra->firstName );
		mir_free( psra->lastName );
		mir_free( psra->email );
		mir_free( psra );
		
		return res;
	}
#endif

	INT_PTR res = MyCallProtoService( szModule, szService, wParam, lParam );

#ifdef _UNICODE
	if ( res == CALLSERVICE_NOTFOUND && pa && pa->bOldProto && pa->ppro && strchr( szService, 'W' )) {
		TServiceListItem *item = serviceItems.find(( TServiceListItem* )&szService );
		if ( !item ) return res;

		switch( item->id ) {
			case 100:
				return ( INT_PTR )pa->ppro->SetAwayMsg( wParam, ( TCHAR* )lParam );
			case 102: {
				CCSDATA *ccs = ( CCSDATA* )lParam;
				return ( INT_PTR )pa->ppro->SendFile( ccs->hContact, ( TCHAR* )ccs->wParam, ( TCHAR** )ccs->lParam );
			}
			case 103: {
				CCSDATA *ccs = ( CCSDATA* )lParam;
				return ( INT_PTR )pa->ppro->FileAllow( ccs->hContact, ( HANDLE )ccs->wParam, ( TCHAR* )ccs->lParam );
			}
			case 104: {
				CCSDATA *ccs = ( CCSDATA* )lParam;
				return ( INT_PTR )pa->ppro->FileDeny( ccs->hContact, ( HANDLE )ccs->wParam, ( TCHAR* )ccs->lParam );
			}
			case 105: {
				PROTOFILERESUME* pfr = ( PROTOFILERESUME* )lParam;
				return ( INT_PTR )pa->ppro->FileResume(( HANDLE )wParam, &pfr->action, &pfr->szFilename );
			}
			case 106: {
				CCSDATA *ccs = ( CCSDATA* )lParam;
				return ( INT_PTR )pa->ppro->AuthRequest( ccs->hContact, ( const TCHAR* )ccs->lParam );
			}
			case 107:
				return ( INT_PTR )pa->ppro->AuthDeny(( HANDLE )wParam, ( const TCHAR* )lParam );
			case 108:
				return ( INT_PTR )pa->ppro->SearchBasic(( const TCHAR* )lParam );
			case 109: {
				PROTOSEARCHBYNAME* psbn = ( PROTOSEARCHBYNAME* )lParam;
				return ( INT_PTR )pa->ppro->SearchByName( psbn->pszNick, psbn->pszFirstName, psbn->pszLastName ); 
			}
			case 110:
				return ( INT_PTR )pa->ppro->SearchByEmail(( const TCHAR* )lParam );
	}	}
#endif

	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CallContactService( HANDLE hContact, const char *szProtoService, WPARAM wParam, LPARAM lParam )
{
	int i;
	DBVARIANT dbv;
	INT_PTR ret;
	PROTOACCOUNT* pa;
	CCSDATA ccs = { hContact, szProtoService, wParam, lParam };

	for ( i = 0;; i++ ) {
		char str[10];
		_itoa( i, str, 10 );
		if ( DBGetContactSettingString( hContact, "_Filter", str, &dbv ))
			break;

		if (( ret = CallProtoServiceInt( hContact, dbv.pszVal, szProtoService, i+1, ( LPARAM)&ccs )) != CALLSERVICE_NOTFOUND ) {
			//chain was started, exit
			mir_free( dbv.pszVal );
			return ret;
		}
		mir_free( dbv.pszVal );
	}
	if ( DBGetContactSettingString( hContact, "Protocol", "p", &dbv ))
		return 1;

	pa = Proto_GetAccount( dbv.pszVal );
	if ( pa == NULL || pa->ppro == NULL )
		ret = 1;
	else {
		if ( pa->bOldProto )
			ret = CallProtoServiceInt( hContact, dbv.pszVal, szProtoService, (WPARAM)(-1), ( LPARAM)&ccs );
		else
			ret = CallProtoServiceInt( hContact, dbv.pszVal, szProtoService, wParam, lParam );
		if ( ret == CALLSERVICE_NOTFOUND )
			ret = 1;
	}

	mir_free( dbv.pszVal );
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void InsertServiceListItem( int id, const char* szName )
{
	TServiceListItem* p = ( TServiceListItem* )mir_alloc( sizeof( TServiceListItem ));
	p->id = id;
	p->name = szName;
	serviceItems.insert( p );
}

int LoadProtocolsModule(void)
{
	bModuleInitialized = TRUE;

	if ( LoadProtoChains() )
		return 1;

	InsertServiceListItem(  1, PS_ADDTOLIST );
	InsertServiceListItem(  2, PS_ADDTOLISTBYEVENT );
	InsertServiceListItem(  3, PS_AUTHALLOW );
	InsertServiceListItem(  4, PS_AUTHDENY );
	InsertServiceListItem(  5, PSR_AUTH );
	InsertServiceListItem(  6, PSS_AUTHREQUEST );
	InsertServiceListItem(  7, PS_CHANGEINFO );
	InsertServiceListItem(  8, PSS_FILEALLOW );
	InsertServiceListItem(  9, PSS_FILECANCEL );
	InsertServiceListItem( 10, PSS_FILEDENY );
	InsertServiceListItem( 11, PS_FILERESUME );
	InsertServiceListItem( 12, PS_GETCAPS );
	InsertServiceListItem( 13, PS_LOADICON );
	InsertServiceListItem( 14, PSS_GETINFO );
	InsertServiceListItem( 15, PS_BASICSEARCH );
	InsertServiceListItem( 16, PS_SEARCHBYEMAIL );
	InsertServiceListItem( 17, PS_SEARCHBYNAME );
	InsertServiceListItem( 18, PS_SEARCHBYADVANCED );
	InsertServiceListItem( 19, PS_CREATEADVSEARCHUI );
	InsertServiceListItem( 20, PSR_CONTACTS );
	InsertServiceListItem( 21, PSR_FILE );
	InsertServiceListItem( 22, PSR_MESSAGE );
	InsertServiceListItem( 23, PSR_URL );
	InsertServiceListItem( 24, PSS_CONTACTS );
	InsertServiceListItem( 25, PSS_FILE );
	InsertServiceListItem( 26, PSS_MESSAGE );
	InsertServiceListItem( 27, PSS_URL );
	InsertServiceListItem( 28, PSS_SETAPPARENTMODE );
	InsertServiceListItem( 29, PS_SETSTATUS );
	InsertServiceListItem( 30, PSS_GETAWAYMSG );
	InsertServiceListItem( 31, PSR_AWAYMSG );
	InsertServiceListItem( 32, PSS_AWAYMSG );
	InsertServiceListItem( 33, PS_SETAWAYMSG );
	InsertServiceListItem( 34, PSS_USERISTYPING );
	InsertServiceListItem( 35, PS_GETNAME );
	InsertServiceListItem( 36, PS_GETSTATUS );

#ifdef _UNICODE
	InsertServiceListItem( 100, PS_SETAWAYMSGW );
	InsertServiceListItem( 102, PSS_FILEW );
	InsertServiceListItem( 103, PSS_FILEALLOWW );
	InsertServiceListItem( 104, PSS_FILEDENYW );
	InsertServiceListItem( 105, PS_FILERESUMEW );
	InsertServiceListItem( 106, PSS_AUTHREQUESTW );
	InsertServiceListItem( 107, PS_AUTHDENYW );
	InsertServiceListItem( 108, PS_BASICSEARCHW );
	InsertServiceListItem( 109, PS_SEARCHBYNAMEW );
	InsertServiceListItem( 110, PS_SEARCHBYEMAILW );
#endif

	hAckEvent = CreateHookableEvent(ME_PROTO_ACK);
	hTypeEvent = CreateHookableEvent(ME_PROTO_CONTACTISTYPING);
	hAccListChanged = CreateHookableEvent(ME_PROTO_ACCLISTCHANGED);

	CreateServiceFunction( MS_PROTO_BROADCASTACK,     Proto_BroadcastAck     );
	CreateServiceFunction( MS_PROTO_ISPROTOCOLLOADED, srvProto_IsLoaded      );
	CreateServiceFunction( MS_PROTO_ENUMPROTOS,       Proto_EnumProtocols    );
	CreateServiceFunction( MS_PROTO_REGISTERMODULE,   Proto_RegisterModule   );
	CreateServiceFunction( MS_PROTO_SELFISTYPING,     Proto_SelfIsTyping     );
	CreateServiceFunction( MS_PROTO_CONTACTISTYPING,  Proto_ContactIsTyping  );

	CreateServiceFunction( MS_PROTO_RECVFILE,         Proto_RecvFile         );
	CreateServiceFunction( MS_PROTO_RECVFILET,        Proto_RecvFileT        );
	CreateServiceFunction( MS_PROTO_RECVMSG,          Proto_RecvMessage      );

	CreateServiceFunction( "Proto/EnumProtocols",     Proto_EnumAccounts     );
	CreateServiceFunction( MS_PROTO_ENUMACCOUNTS,     Proto_EnumAccounts     );
	CreateServiceFunction( MS_PROTO_GETACCOUNT,       srvProto_GetAccount    );

	CreateServiceFunction( MS_PROTO_ISACCOUNTENABLED, srvProto_IsAccountEnabled );
	CreateServiceFunction( MS_PROTO_ISACCOUNTLOCKED,  srvProto_IsAccountLocked );

	return LoadProtoOptions();
}

void UnloadProtocolsModule()
{
	int i;

	if ( !bModuleInitialized ) return;

	if ( hAckEvent ) {
		DestroyHookableEvent(hAckEvent);
		hAckEvent = NULL;
	}
	if ( hAccListChanged ) {
		DestroyHookableEvent(hAccListChanged);
		hAccListChanged = NULL;
	}

	if ( protos.getCount() ) {
		for( i=0; i < protos.getCount(); i++ ) {
			mir_free( protos[i]->szName);
			mir_free( protos[i] );
		}
		protos.destroy();
	}

	for ( i=0; i < serviceItems.getCount(); i++ )
		mir_free( serviceItems[i] );
	serviceItems.destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////

pfnUninitProto GetProtocolDestructor( char* szProto )
{
	int idx;	
	PROTOCOLDESCRIPTOR temp;
	temp.szName = szProto;
	if (( idx = protos.getIndex( &temp )) != -1 )
		return protos[idx]->fnUninit;

	return NULL;
}
