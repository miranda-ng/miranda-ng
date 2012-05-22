/*

Omegle plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-12 Robert Pösel

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once

class OmegleProto : public PROTO_INTERFACE
{
public:
	OmegleProto( const char *proto_name, const TCHAR *username );
	~OmegleProto( );

	__inline void* operator new( size_t size )
	{
		return calloc( 1, size );
	}
	__inline void operator delete( void* p )
	{
		free( p );
	}

	inline const char* ModuleName( ) const
	{
		return m_szModuleName;
	}

	inline bool isOnline( )
	{
		return ( m_iStatus != ID_STATUS_OFFLINE && m_iStatus != ID_STATUS_CONNECTING );
	}

	inline bool isOffline( )
	{
		return ( m_iStatus == ID_STATUS_OFFLINE );
	}

	// PROTO_INTERFACE
	virtual	HANDLE   __cdecl AddToList( int flags, PROTOSEARCHRESULT* psr );
	virtual	HANDLE   __cdecl AddToListByEvent( int flags, int iContact, HANDLE hDbEvent );

	virtual	int      __cdecl Authorize( HANDLE hDbEvent );
	virtual	int      __cdecl AuthDeny( HANDLE hDbEvent, const PROTOCHAR* szReason );
	virtual	int      __cdecl AuthRecv( HANDLE hContact, PROTORECVEVENT* );
	virtual	int      __cdecl AuthRequest( HANDLE hContact, const PROTOCHAR* szMessage );

	virtual	HANDLE   __cdecl ChangeInfo( int iInfoType, void* pInfoData );

	virtual	HANDLE   __cdecl FileAllow( HANDLE hContact, HANDLE hTransfer, const PROTOCHAR* szPath );
	virtual	int      __cdecl FileCancel( HANDLE hContact, HANDLE hTransfer );
	virtual	int      __cdecl FileDeny( HANDLE hContact, HANDLE hTransfer, const PROTOCHAR* szReason );
	virtual	int      __cdecl FileResume( HANDLE hTransfer, int* action, const PROTOCHAR** szFilename );

	virtual	DWORD_PTR __cdecl GetCaps( int type, HANDLE hContact = NULL );
	virtual	HICON     __cdecl GetIcon( int iconIndex );
	virtual	int       __cdecl GetInfo( HANDLE hContact, int infoType );

	virtual	HANDLE    __cdecl SearchBasic( const PROTOCHAR* id );
	virtual	HANDLE    __cdecl SearchByEmail( const PROTOCHAR* email );
	virtual	HANDLE    __cdecl SearchByName( const PROTOCHAR* nick, const PROTOCHAR* firstName, const PROTOCHAR* lastName );
	virtual	HWND      __cdecl SearchAdvanced( HWND owner );
	virtual	HWND      __cdecl CreateExtendedSearchUI( HWND owner );

	virtual	int       __cdecl RecvContacts( HANDLE hContact, PROTORECVEVENT* );
	virtual	int       __cdecl RecvFile( HANDLE hContact, PROTOFILEEVENT* );
	virtual	int       __cdecl RecvMsg( HANDLE hContact, PROTORECVEVENT* );
	virtual	int       __cdecl RecvUrl( HANDLE hContact, PROTORECVEVENT* );

	virtual	int       __cdecl SendContacts( HANDLE hContact, int flags, int nContacts, HANDLE* hContactsList );
	virtual	HANDLE    __cdecl SendFile( HANDLE hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles );
	virtual	int       __cdecl SendMsg( HANDLE hContact, int flags, const char* msg );
	virtual	int       __cdecl SendUrl( HANDLE hContact, int flags, const char* url );

	virtual	int       __cdecl SetApparentMode( HANDLE hContact, int mode );
	virtual	int       __cdecl SetStatus( int iNewStatus );

	virtual	HANDLE    __cdecl GetAwayMsg( HANDLE hContact );
	virtual	int       __cdecl RecvAwayMsg( HANDLE hContact, int mode, PROTORECVEVENT* evt );
	virtual	int       __cdecl SendAwayMsg( HANDLE hContact, HANDLE hProcess, const char* msg );
	virtual	int       __cdecl SetAwayMsg( int iStatus, const PROTOCHAR* msg );

	virtual	int       __cdecl UserIsTyping( HANDLE hContact, int type );

	virtual	int       __cdecl OnEvent( PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam );

	// Services
	int  __cdecl SvcCreateAccMgrUI( WPARAM, LPARAM );

	// Events
	int  __cdecl OnModulesLoaded(WPARAM, LPARAM);
	int  __cdecl OnOptionsInit(WPARAM, LPARAM);
	int  __cdecl OnContactDeleted(WPARAM,LPARAM);
	int  __cdecl OnPreShutdown(WPARAM,LPARAM);
	int  __cdecl OnPrebuildContactMenu(WPARAM,LPARAM);

	// Chat handling
	int  __cdecl OnChatEvent(WPARAM,LPARAM);
	int  __cdecl OnJoinChat(WPARAM,LPARAM);
	int  __cdecl OnLeaveChat(WPARAM,LPARAM);

	// Loops
	void __cdecl EventsLoop(void*);

	// Worker threads
	void __cdecl SignOn(void*);
	void __cdecl SignOff(void*);

	void __cdecl SendMsgWorker(void*);
	void __cdecl SendTypingWorker(void*);
	
	void __cdecl NewChatWorker(void*);
	void __cdecl StopChatWorker(void*);

	void StopChat(bool disconnect = true);
	void NewChat();

	// Contacts handling
	//bool    IsMyContact(HANDLE, bool include_chat = false);

	// Chat handling
 	void AddChat(const TCHAR *id,const TCHAR *name);
	void UpdateChat(const TCHAR *name, const TCHAR *message, bool addtochat = true);
	void AddChatContact(const TCHAR *nick);
	void DeleteChatContact(const TCHAR *name);
	void SetChatStatus(int);
	void ClearChat();	
	void SetTopic(const TCHAR *topic = NULL);

	// Connection client
	Omegle_client facy;

	HANDLE  signon_lock_;
	HANDLE  log_lock_;
	HANDLE  events_loop_lock_;

	HANDLE  m_hNetlibUser;

	static void CALLBACK APC_callback(ULONG_PTR p);

	// Logging
	int Log(const char *fmt,...);
};
