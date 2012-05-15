/*
IRC plugin for Miranda IM

Copyright (C) 2003-05 Jurgen Persson
Copyright (C) 2007-09 George Hazan

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

#include "irc.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Standard functions

int CIrcProto::getByte( const char* name, BYTE defaultValue )
{	return DBGetContactSettingByte( NULL, m_szModuleName, name, defaultValue );
}

int CIrcProto::getByte( HANDLE hContact, const char* name, BYTE defaultValue )
{	return DBGetContactSettingByte(hContact, m_szModuleName, name, defaultValue );
}

int CIrcProto::getDword( const char* name, DWORD defaultValue )
{	return DBGetContactSettingDword( NULL, m_szModuleName, name, defaultValue );
}

int CIrcProto::getDword( HANDLE hContact, const char* name, DWORD defaultValue )
{	return DBGetContactSettingDword(hContact, m_szModuleName, name, defaultValue );
}

int CIrcProto::getString( const char* name, DBVARIANT* result )
{	return DBGetContactSettingString( NULL, m_szModuleName, name, result );
}

int CIrcProto::getString( HANDLE hContact, const char* name, DBVARIANT* result )
{	return DBGetContactSettingString( hContact, m_szModuleName, name, result );
}

int CIrcProto::getTString( const char* name, DBVARIANT* result )
{	return DBGetContactSettingTString( NULL, m_szModuleName, name, result );
}

int CIrcProto::getTString( HANDLE hContact, const char* name, DBVARIANT* result )
{	return DBGetContactSettingTString( hContact, m_szModuleName, name, result );
}

int CIrcProto::getWord( const char* name, WORD defaultValue )
{	return DBGetContactSettingWord( NULL, m_szModuleName, name, defaultValue );
}

int CIrcProto::getWord( HANDLE hContact, const char* name, WORD defaultValue )
{	return DBGetContactSettingWord(hContact, m_szModuleName, name, defaultValue );
}

void CIrcProto::setByte( const char* name, BYTE value )
{	DBWriteContactSettingByte(NULL, m_szModuleName, name, value );
}

void CIrcProto::setByte( HANDLE hContact, const char* name, BYTE value )
{	DBWriteContactSettingByte(hContact, m_szModuleName, name, value );
}

void CIrcProto::setDword( const char* name, DWORD value )
{	DBWriteContactSettingDword(NULL, m_szModuleName, name, value );
}

void CIrcProto::setDword( HANDLE hContact, const char* name, DWORD value )
{	DBWriteContactSettingDword(hContact, m_szModuleName, name, value );
}

void CIrcProto::setString( const char* name, const char* value )
{	DBWriteContactSettingString(NULL, m_szModuleName, name, value );
}

void CIrcProto::setString( HANDLE hContact, const char* name, const char* value )
{	DBWriteContactSettingString(hContact, m_szModuleName, name, value );
}

void CIrcProto::setTString( const char* name, const TCHAR* value )
{	DBWriteContactSettingTString(NULL, m_szModuleName, name, value );
}

void CIrcProto::setTString( HANDLE hContact, const char* name, const TCHAR* value )
{	DBWriteContactSettingTString(hContact, m_szModuleName, name, value );
}

void CIrcProto::setWord( const char* name, int value )
{	DBWriteContactSettingWord(NULL, m_szModuleName, name, value );
}

void CIrcProto::setWord( HANDLE hContact, const char* name, int value )
{	DBWriteContactSettingWord(hContact, m_szModuleName, name, value );
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIrcProto::AddToJTemp(TCHAR op, CMString& sCommand)
{
	CMString res;	

	int pos = 0;
	for ( ;; ) {
		CMString tmp = sCommand.Tokenize( _T(","), pos );
		if ( pos == -1 )
			break;

		tmp = op + tmp;
		if ( res.IsEmpty() )
			res = tmp;
		else
			res += _T(" ") + tmp;
	}

	DBVARIANT dbv;
	if ( !getTString( "JTemp", &dbv )) {
		res = CMString(dbv.ptszVal) + _T(" ") + res;
		DBFreeVariant( &dbv );
	}

	setTString("JTemp", res.c_str());
}

void CIrcProto::ircFork( IrcThreadFunc pFunc, void* arg )
{
	unsigned threadID;
	CloseHandle(( HANDLE )::mir_forkthreadowner(( pThreadFuncOwner )( *( void** )&pFunc ), this, arg, &threadID ));
}

HANDLE CIrcProto::ircForkEx( IrcThreadFunc pFunc, void* arg )
{
	unsigned threadID;
	return (HANDLE)::mir_forkthreadowner(( pThreadFuncOwner )( *( void** )&pFunc ), this, arg, &threadID );
}

void CIrcProto::IrcHookEvent( const char* szEvent, IrcEventFunc pFunc )
{
	::HookEventObj( szEvent, ( MIRANDAHOOKOBJ )*( void** )&pFunc, this );
}

char* __stdcall rtrim( char *string )
{
   char* p = string + strlen( string ) - 1;
   while ( p >= string ) {
		if ( *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r' )
         break;

		*p-- = 0;
   }
   return string;
}

CMString __stdcall GetWord(const TCHAR* text, int index)
{
	if ( text && *text ) {
		TCHAR* p1 = (TCHAR*)text;
		TCHAR* p2 = NULL;

		while (*p1 == ' ')
			p1++;

		if (*p1 != '\0') {
			for (int i =0; i < index; i++) {
				p2 = _tcschr( p1, ' ' );
				if ( !p2 )
					p2 = _tcschr( p1, '\0' );
				else
					while ( *p2 == ' ' )
						p2++;

				p1 = p2;
			}

			p2 = _tcschr(p1, ' ');
			if( !p2 )
				p2 = _tcschr(p1, '\0');

			if (p1 != p2)
				return CMString( p1, p2-p1 );
	}	}

	return CMString();
}

const TCHAR* __stdcall GetWordAddress(const TCHAR* text, int index)
{
	if( !text || !lstrlen(text))
		return text;

	const TCHAR* temp = text;

	while (*temp == ' ')
		temp++;

	if (index == 0)
		return temp;

	for (int i = 0; i < index; i++) {
		temp = _tcschr(temp, ' ');
		if ( !temp )
			temp = ( TCHAR* )_tcschr(text, '\0');
		else
			while (*temp == ' ')
				temp++;
		text = temp;
	}

	return temp;
}

void __stdcall RemoveLinebreaks( CMString& Message )
{
	while ( Message.Find( _T("\r\n\r\n"), 0) != -1 )
		ReplaceString( Message, _T("\r\n\r\n"), _T("\r\n"));

	if (Message.Find( _T("\r\n"), 0) == 0)
		Message.Delete(0,2);

	if ( (Message.GetLength() > 1) && (Message.Find(_T("\r\n"), Message.GetLength()-2) == 0) )
		Message.Delete(Message.GetLength()-2, 2);
}

#if defined( _UNICODE )
String& __stdcall ReplaceString ( String& text, const char* replaceme, const char* newword )
{
	if ( !text.IsEmpty() && replaceme != NULL) {
		int i = 0;
		while (( i = text.Find(replaceme, i)) != -1 ) {
			text.Delete(i,lstrlenA(replaceme));
			text.Insert(i, newword);
			i = i + lstrlenA(newword);
	}	}

	return text;
}
#endif

CMString& __stdcall ReplaceString ( CMString& text, const TCHAR* replaceme, const TCHAR* newword)
{
	if ( !text.IsEmpty() && replaceme != NULL) {
		int i = 0;
		while (( i = text.Find(replaceme, i)) != -1 ) {
			text.Delete(i,lstrlen(replaceme));
			text.Insert(i, newword);
			i = i + lstrlen(newword);
	}	}

	return text;
}

char* __stdcall IrcLoadFile( TCHAR* szPath)
{
	char * szContainer = NULL;
	DWORD dwSiz = 0;
	FILE *hFile = _tfopen(szPath, _T("rb"));	
	if ( hFile != NULL )
	{
		fseek(hFile,0,SEEK_END); // seek to end
		dwSiz = ftell(hFile); // size
		fseek(hFile,0,SEEK_SET); // seek back to original pos
		szContainer = new char [dwSiz+1];
		fread(szContainer, 1, dwSiz, hFile);
		szContainer[dwSiz] = '\0';
		fclose(hFile);
		return szContainer;
	}

	return 0;
}

int __stdcall WCCmp( const TCHAR* wild, const TCHAR* string )
{
	if ( wild == NULL || !lstrlen(wild) || string == NULL || !lstrlen(string))
		return 1;

	const TCHAR *cp = NULL, *mp = NULL;
	while ((*string) && (*wild != '*')) {
		if ((*wild != *string) && (*wild != '?'))
			return 0;

		wild++;
		string++;
	}
		
	while (*string) {
		if (*wild == '*') {
			if (!*++wild)
				return 1;

			mp = wild;
			cp = string+1;
		}
		else if ((*wild == *string) || (*wild == '?')) {
			wild++;
			string++;
		}
		else {
			wild = mp;
			string = cp++;
	}	}
		
	while (*wild == '*')
		wild++;

	return !*wild;
}	

bool CIrcProto::IsChannel(const TCHAR* sName) 
{
	return ( sChannelPrefixes.Find( sName[0] ) != -1 );
}

#if defined( _UNICODE )
String __stdcall GetWord(const char* text, int index)
{
	if ( text && text[0] ) {
		char* p1 = (char*)text;
		char* p2 = NULL;

		while (*p1 == ' ')
			p1++;

		if (*p1 != '\0') {
			for (int i =0; i < index; i++) {
				p2 = strchr( p1, ' ' );
				if ( !p2 )
					p2 = strchr( p1, '\0' );
				else
					while ( *p2 == ' ' )
						p2++;

				p1 = p2;
			}

			p2 = strchr(p1, ' ');
			if(!p2)
				p2 = strchr(p1, '\0');

			if (p1 != p2)
				return String( p1, p2-p1+1 );
	}	}

	return String();
}

bool CIrcProto::IsChannel(const char* sName) 
{
	return ( sChannelPrefixes.Find( sName[0] ) != -1 );
}
#endif

TCHAR* __stdcall my_strstri(const TCHAR* s1, const TCHAR* s2) 
{ 
	int i,j,k; 
	for(i=0;s1[i];i++) 
		for(j=i,k=0; _totlower(s1[j]) == _totlower(s2[k]);j++,k++) 
			if(!s2[k+1]) 
				return ( TCHAR* )(s1+i); 

	return NULL; 
} 

TCHAR* __stdcall DoColorCodes (const TCHAR* text, bool bStrip, bool bReplacePercent)
{
	static TCHAR szTemp[4000]; szTemp[0] = '\0';
	TCHAR* p = szTemp;
	bool bBold = false;
	bool bUnderline = false;
	bool bItalics = false;

	if ( !text )
		return szTemp;

	while ( *text != '\0' ) {
		int iFG = -1;
		int iBG = -1;

		switch( *text ) {
		case '%': //escape
			*p++ = '%'; 
			if ( bReplacePercent )
				*p++ = '%'; 
			text++;
			break;

		case 2: //bold
			if ( !bStrip ) {
				*p++ = '%'; 
				*p++ = bBold ? 'B' : 'b'; 
			}
			bBold = !bBold;
			text++;
			break;

		case 15: //reset
			if ( !bStrip ) {
				*p++ = '%'; 
				*p++ = 'r'; 
			}
			bUnderline = false;
			bBold = false;
			text++;
			break;

		case 22: //italics
			if ( !bStrip ) {
				*p++ = '%'; 
				*p++ = bItalics ? 'I' : 'i'; 
			}
			bItalics = !bItalics;
			text++;
			break;

		case 31: //underlined
			if ( !bStrip ) {
				*p++ = '%'; 
				*p++ = bUnderline ? 'U' : 'u'; 
			}
			bUnderline = !bUnderline;
			text++;
			break;

		case 3: //colors
			text++;

			// do this if the colors should be reset to default
			if ( *text <= 47 || *text >= 58 || *text == '\0' ) {
				if ( !bStrip ) {
					*p++ = '%';
					*p++ = 'C';
					*p++ = '%';
					*p++ = 'F';
				}
				break;
			}
			else { // some colors should be set... need to find out who
				TCHAR buf[3];

				// fix foreground index
				if ( text[1] > 47 && text[1] < 58 && text[1] != '\0')
					lstrcpyn( buf, text, 3 );
				else
					lstrcpyn( buf, text, 2 );
				text += lstrlen( buf );
				iFG = _ttoi( buf );

				// fix background color
				if ( *text == ',' && text[1] > 47 && text[1] < 58 && text[1] != '\0' ) {
					text++;

					if ( text[1] > 47 && text[1] < 58 && text[1] != '\0' )
						lstrcpyn( buf, text, 3 );
					else
						lstrcpyn( buf, text, 2 );
					text += lstrlen( buf );
					iBG = _ttoi( buf );
			}	}
			
			if ( iFG >= 0 && iFG != 99 )
				while( iFG > 15 )
					iFG -= 16;
			if ( iBG >= 0 && iBG != 99 )
				while( iBG > 15 )
					iBG -= 16;

			// create tag for chat.dll
			if ( !bStrip ) {
				TCHAR buf[10];
				if ( iFG >= 0 && iFG != 99 ) {
					*p++ = '%';
					*p++ = 'c';

					mir_sntprintf( buf, SIZEOF(buf), _T("%02u"), iFG );
					for (int i = 0; i<2; i++)
						*p++ = buf[i];
				}
				else if (iFG == 99) {
					*p++ = '%';
					*p++ = 'C';
				}
				
				if ( iBG >= 0 && iBG != 99 ) {
					*p++ = '%';
					*p++ = 'f';

					mir_sntprintf( buf, SIZEOF(buf), _T("%02u"), iBG );
					for ( int i = 0; i<2; i++ )
						*p++ = buf[i];
				}
				else if ( iBG == 99 ) {
					*p++ = '%';
					*p++ = 'F';
			}	}
			break;

		default:
			*p++ = *text++;
			break;
	}	}
	
	*p = '\0';
	return szTemp;
}

INT_PTR CIrcProto::CallChatEvent(WPARAM wParam, LPARAM lParam)
{
	GCEVENT * gce = (GCEVENT *)lParam;
	INT_PTR iVal = 0;

	// first see if the scripting module should modify or stop this event
	if ( m_bMbotInstalled && m_scriptingEnabled && gce 
		&& gce->time != 0 && (gce->pDest->pszID == NULL 
		|| lstrlen(gce->pDest->ptszID) != 0 && lstrcmpi(gce->pDest->ptszID , SERVERWINDOW)))
	{
		GCEVENT *gcevent= (GCEVENT*) lParam;
		GCEVENT *gcetemp = NULL;
		WPARAM wp = wParam;
		gcetemp = (GCEVENT *)mir_alloc(sizeof(GCEVENT));
		gcetemp->pDest = (GCDEST *)mir_alloc(sizeof(GCDEST));
		gcetemp->pDest->iType = gcevent->pDest->iType;
		gcetemp->dwFlags = gcevent->dwFlags;
		gcetemp->bIsMe = gcevent->bIsMe;
		gcetemp->cbSize = sizeof(GCEVENT);
		gcetemp->dwItemData = gcevent->dwItemData;
		gcetemp->time = gcevent->time;
		gcetemp->pDest->ptszID = mir_tstrdup( gcevent->pDest->ptszID );
		gcetemp->pDest->pszModule = mir_strdup( gcevent->pDest->pszModule );
		gcetemp->ptszText = mir_tstrdup( gcevent->ptszText );
		gcetemp->ptszUID = mir_tstrdup( gcevent->ptszUID );
		gcetemp->ptszNick = mir_tstrdup( gcevent->ptszNick );
		gcetemp->ptszStatus = mir_tstrdup( gcevent->ptszStatus );
		gcetemp->ptszUserInfo = mir_tstrdup( gcevent->ptszUserInfo );

		if ( Scripting_TriggerMSPGuiIn( &wp, gcetemp ) && gcetemp ) {
			//MBOT CORRECTIONS
			//if ( gcetemp && gcetemp->pDest && gcetemp->pDest->ptszID ) {
			if ( gcetemp && gcetemp->pDest && gcetemp->pDest->ptszID && 
				!my_strstri(gcetemp->pDest->ptszID, (IsConnected()) ? m_info.sNetwork.c_str() : TranslateT("Offline")) ) {

				CMString sTempId = MakeWndID( gcetemp->pDest->ptszID );
				mir_realloc( gcetemp->pDest->ptszID, sizeof(TCHAR)*(sTempId.GetLength() + 1));
				lstrcpyn(gcetemp->pDest->ptszID, sTempId.c_str(), sTempId.GetLength()+1); 
			}
			iVal = CallServiceSync(MS_GC_EVENT, wp, (LPARAM) gcetemp);
		}

		if ( gcetemp ) {
			mir_free(( void* )gcetemp->pszNick);
			mir_free(( void* )gcetemp->pszUID);
			mir_free(( void* )gcetemp->pszStatus);
			mir_free(( void* )gcetemp->pszUserInfo);
			mir_free(( void* )gcetemp->pszText);
			mir_free(( void* )gcetemp->pDest->pszID);
			mir_free(( void* )gcetemp->pDest->pszModule);
			mir_free(( void* )gcetemp->pDest);
			mir_free(( void* )gcetemp);
		}

		return iVal;
	}

	return CallServiceSync( MS_GC_EVENT, wParam, ( LPARAM )gce );
}

INT_PTR CIrcProto::DoEvent(int iEvent, const TCHAR* pszWindow, const TCHAR* pszNick, 
			const TCHAR* pszText, const TCHAR* pszStatus, const TCHAR* pszUserInfo, 
			DWORD_PTR dwItemData, bool bAddToLog, bool bIsMe, time_t timestamp)
{						   
	GCDEST gcd = {0};
	GCEVENT gce = {0};
	CMString sID;
	CMString sText = _T("");

	if ( iEvent == GC_EVENT_INFORMATION && bIsMe && !bEcho )
		return false;

	if ( pszText ) {
		if (iEvent != GC_EVENT_SENDMESSAGE)
			sText = DoColorCodes(pszText, FALSE, TRUE);
		else
			sText = pszText;
	}

	if ( pszWindow ) {
		if ( lstrcmpi( pszWindow, SERVERWINDOW))
			sID = pszWindow + (CMString)_T(" - ") + m_info.sNetwork;
		else
			sID = pszWindow;
		gcd.ptszID = (TCHAR*)sID.c_str();
	}
	else gcd.ptszID = NULL;

	gcd.pszModule = m_szModuleName;
	gcd.iType = iEvent;

	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = &gcd;
	gce.ptszStatus = pszStatus;
	gce.dwFlags = GC_TCHAR + ((bAddToLog) ? GCEF_ADDTOLOG : 0);
	gce.ptszNick = pszNick;
	gce.ptszUID = pszNick;
	if (iEvent == GC_EVENT_TOPIC)
	  gce.ptszUserInfo = pszUserInfo;
	else
	  gce.ptszUserInfo = m_showAddresses ? pszUserInfo : NULL;

	if ( !sText.IsEmpty() )
		gce.ptszText = sText.c_str();

	gce.dwItemData = dwItemData;
	if(timestamp == 1)
		gce.time = time(NULL);
	else
		gce.time = timestamp;
	gce.bIsMe = bIsMe;
	return CallChatEvent((WPARAM)0, (LPARAM)&gce);
}

CMString CIrcProto::ModeToStatus(int sMode) 
{
	if ( sUserModes.Find( sMode ) != -1 ) {
		switch( sMode ) {
		case 'q':
			return (CMString)_T("Owner");
		case 'o':
			return (CMString)_T("Op");
		case 'v':
			return (CMString)_T("Voice");
		case 'h':
			return (CMString)_T("Halfop");
		case 'a':
			return (CMString)_T("Admin");
		default:
			return (CMString)_T("Unknown");
	}	}

	return (CMString)_T("Normal");
}

CMString CIrcProto::PrefixToStatus(int cPrefix) 
{
	const TCHAR* p = _tcschr( sUserModePrefixes.c_str(), cPrefix );
	if ( p ) {
		int index = int( p - sUserModePrefixes.c_str());
		return ModeToStatus( sUserModes[index] );
	}

	return (CMString)_T("Normal");
}

/////////////////////////////////////////////////////////////////////////////////////////
// Timer functions 

struct TimerPair
{
	TimerPair( CIrcProto* _pro, UINT_PTR _id ) :
		ppro( _pro ),
		idEvent( _id )
	{}

	UINT_PTR idEvent;
	CIrcProto* ppro;
};

static int CompareTimers( const TimerPair* p1, const TimerPair* p2 )
{
	if ( p1->idEvent < p2->idEvent )
		return -1;
	return ( p1->idEvent == p2->idEvent ) ? 0 : 1;
}

static OBJLIST<TimerPair> timers( 10, CompareTimers );
static CRITICAL_SECTION timers_cs;

void InitTimers( void )
{
	InitializeCriticalSection( &timers_cs );
}

void UninitTimers( void )
{
	EnterCriticalSection( &timers_cs );
	timers.destroy();
	LeaveCriticalSection( &timers_cs );
	DeleteCriticalSection( &timers_cs );
}

CIrcProto* GetTimerOwner( UINT_PTR nIDEvent )
{
	CIrcProto* result;

	EnterCriticalSection( &timers_cs );
	TimerPair temp( NULL, nIDEvent );
	int idx = timers.getIndex( &temp );
	if ( idx == -1 )
		result = NULL;
	else
		result = timers[ idx ].ppro;
	LeaveCriticalSection( &timers_cs );
	return result;
}

void CIrcProto::SetChatTimer(UINT_PTR &nIDEvent,UINT uElapse, TIMERPROC lpTimerFunc)
{
	if (nIDEvent)
		KillChatTimer(nIDEvent);

	nIDEvent = SetTimer( NULL, NULL, uElapse, lpTimerFunc);

	EnterCriticalSection( &timers_cs );
	timers.insert( new TimerPair( this, nIDEvent ));
	LeaveCriticalSection( &timers_cs );
}

void CIrcProto::KillChatTimer(UINT_PTR &nIDEvent)
{
	if ( nIDEvent ) {
		EnterCriticalSection( &timers_cs );
		TimerPair temp( this, nIDEvent );
		int idx = timers.getIndex( &temp );
		if ( idx != -1 )
			timers.remove( idx );

		LeaveCriticalSection( &timers_cs );

		KillTimer(NULL, nIDEvent);
		nIDEvent = NULL;
}	}

/////////////////////////////////////////////////////////////////////////////////////////

int CIrcProto::SetChannelSBText(CMString sWindow, CHANNELINFO * wi)
{
	CMString sTemp = _T("");
	if(wi->pszMode)
	{
		sTemp += _T("[");
		sTemp += wi->pszMode;
		sTemp += _T("] ");
	}
	if(wi->pszTopic)
		sTemp += wi->pszTopic;
	sTemp = DoColorCodes(sTemp.c_str(), TRUE, FALSE);
	return DoEvent(GC_EVENT_SETSBTEXT, sWindow.c_str(), NULL, sTemp.c_str(), NULL, NULL, NULL, FALSE, FALSE, 0);
}

CMString CIrcProto::MakeWndID(const TCHAR* sWindow)
{
	TCHAR buf[200];
	mir_sntprintf( buf, SIZEOF(buf), _T("%s - %s"), sWindow, (IsConnected()) ? m_info.sNetwork.c_str() : TranslateT("Offline"));
	return CMString(buf);
}

bool CIrcProto::FreeWindowItemData(CMString window, CHANNELINFO* wis)
{
	CHANNELINFO* wi;
	if ( !wis )
		wi = (CHANNELINFO *)DoEvent(GC_EVENT_GETITEMDATA, window.c_str(), NULL, NULL, NULL, NULL, NULL, FALSE, FALSE, 0);
	else
		wi = wis;
	if ( wi ) {
		delete[] wi->pszLimit;
		delete[]wi->pszMode;
		delete[]wi->pszPassword;
		delete[]wi->pszTopic;
		delete wi;
		return true;
	}
	return false;
}

bool CIrcProto::AddWindowItemData(CMString window, const TCHAR* pszLimit, const TCHAR* pszMode, const TCHAR* pszPassword, const TCHAR* pszTopic)
{
	CHANNELINFO* wi = (CHANNELINFO *)DoEvent(GC_EVENT_GETITEMDATA, window.c_str(), NULL, NULL, NULL, NULL, NULL, FALSE, FALSE, 0);
	if ( wi ) {
		if ( pszLimit ) {
			wi->pszLimit = ( TCHAR* )realloc( wi->pszLimit, sizeof(TCHAR)*(lstrlen(pszLimit)+1));
			lstrcpy( wi->pszLimit, pszLimit );
		}
		if ( pszMode ) {
			wi->pszMode = ( TCHAR* )realloc( wi->pszMode, sizeof(TCHAR)*(lstrlen(pszMode)+1));
			lstrcpy( wi->pszMode, pszMode );
		}
		if ( pszPassword ) {
			wi->pszPassword = ( TCHAR* )realloc( wi->pszPassword, sizeof(TCHAR)*(lstrlen(pszPassword)+1));
			lstrcpy( wi->pszPassword, pszPassword );
		}
		if ( pszTopic ) {
			wi->pszTopic = ( TCHAR* )realloc( wi->pszTopic, sizeof(TCHAR)*(lstrlen(pszTopic)+1));
			lstrcpy( wi->pszTopic, pszTopic );
		}

		SetChannelSBText(window, wi);
		return true;
	}
	return false;
}

void CIrcProto::CreateProtoService( const char* serviceName, IrcServiceFunc pFunc )
{
	char temp[MAXMODULELABELLENGTH];
	mir_snprintf( temp, sizeof(temp), "%s%s", m_szModuleName, serviceName );
	CreateServiceFunctionObj( temp, ( MIRANDASERVICEOBJ )*( void** )&pFunc, this );
}

void CIrcProto::FindLocalIP(HANDLE con) // inspiration from jabber
{
	// Determine local IP
	int socket = CallService( MS_NETLIB_GETSOCKET, (WPARAM) con, 0);
	if ( socket != INVALID_SOCKET ) {
		struct sockaddr_in saddr;
		int len = sizeof(saddr);
		getsockname(socket, (struct sockaddr *) &saddr, &len);
		lstrcpynA(m_myLocalHost, inet_ntoa(saddr.sin_addr), 49);
		m_myLocalPort = ntohs(saddr.sin_port );
}	} 

void CIrcProto::DoUserhostWithReason(int type, CMString reason, bool bSendCommand, CMString userhostparams, ...)
{
	TCHAR temp[4096];
	CMString S = _T("");
	switch( type ) {
	case 1:
		S = _T("USERHOST");
		break;
	case 2:
		S = _T("WHO");
		break;
	default:
		S = _T("USERHOST");
		break;
	}

	va_list ap;
	va_start(ap, userhostparams);
	mir_vsntprintf(temp, SIZEOF(temp), (S + _T(" ") + userhostparams).c_str(), ap);
	va_end(ap);

	// Add reason
	if ( type == 1 )
		vUserhostReasons.insert( new CMString( reason ));
	else if ( type == 2 )
		vWhoInProgress.insert( new CMString( reason));

	// Do command
	if ( IsConnected() && bSendCommand )
		SendIrcMessage( temp, false );
}

CMString CIrcProto::GetNextUserhostReason(int type)
{
	CMString reason = _T("");
	switch( type ) {
	case 1:
		if ( !vUserhostReasons.getCount())
			return CMString();

		// Get reason
		reason = vUserhostReasons[0];
		vUserhostReasons.remove( 0 );
		break;
	case 2:
		if ( !vWhoInProgress.getCount())
			return CMString();

		// Get reason
		reason = vWhoInProgress[0];
		vWhoInProgress.remove( 0 );
		break;
	}

	return reason;
}

CMString CIrcProto::PeekAtReasons( int type )
{
	switch ( type ) {
	case 1:
		if (!vUserhostReasons.getCount())
			return CMString();
		return vUserhostReasons[0];

	case 2:
		if (!vWhoInProgress.getCount())
			return CMString();
		return vWhoInProgress[0];

	}
	return CMString();
}

void CIrcProto::ClearUserhostReasons(int type)
{
	switch (type) {
	case 1:
		vUserhostReasons.destroy();
		break;
	case 2:
		vWhoInProgress.destroy();
		break;
}	}

////////////////////////////////////////////////////////////////////

SERVER_INFO::~SERVER_INFO()
{
	mir_free( m_name );
	mir_free( m_address );
	mir_free( m_group );
}
