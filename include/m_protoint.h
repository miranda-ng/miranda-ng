/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project,
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

#ifndef M_PROTOINT_H__
#define M_PROTOINT_H__ 1

typedef enum 
{
	EV_PROTO_ONLOAD,
	EV_PROTO_ONREADYTOEXIT,
	EV_PROTO_ONEXIT,
	EV_PROTO_ONRENAME,
	EV_PROTO_ONOPTIONS,
	EV_PROTO_ONERASE,
	EV_PROTO_ONMENU,
	EV_PROTO_ONCONTACTDELETED,
	EV_PROTO_DBSETTINGSCHANGED,
}
	PROTOEVENTTYPE;

#if MIRANDA_VER >= 0x0900
	#define PROTOCHAR TCHAR
	#define PROTOFILEEVENT PROTORECVFILET
#else
	#define PROTOCHAR char
	#define PROTOFILEEVENT PROTORECVFILE
#endif

#ifndef __cplusplus
typedef struct tagPROTO_INTERFACE_VTBL
{
	HANDLE    ( *AddToList )( struct tagPROTO_INTERFACE*, int flags, PROTOSEARCHRESULT* psr );
	HANDLE    ( *AddToListByEvent )( struct tagPROTO_INTERFACE*, int flags, int iContact, HANDLE hDbEvent );

	int       ( *Authorize )( struct tagPROTO_INTERFACE*, HANDLE hContact );
	int       ( *AuthDeny )( struct tagPROTO_INTERFACE*, HANDLE hContact, const TCHAR* szReason );
	int       ( *AuthRecv )( struct tagPROTO_INTERFACE*, HANDLE hContact, PROTORECVEVENT* );
	int       ( *AuthRequest )( struct tagPROTO_INTERFACE*, HANDLE hContact, const TCHAR* szMessage );

	HANDLE    ( *ChangeInfo )( struct tagPROTO_INTERFACE*, int iInfoType, void* pInfoData );

	HANDLE    ( *FileAllow )( struct tagPROTO_INTERFACE*, HANDLE hContact, HANDLE hTransfer, const PROTOCHAR* szPath );
	int       ( *FileCancel )( struct tagPROTO_INTERFACE*, HANDLE hContact, HANDLE hTransfer );
	int       ( *FileDeny )( struct tagPROTO_INTERFACE*, HANDLE hContact, HANDLE hTransfer, const PROTOCHAR* szReason );
	int       ( *FileResume )( struct tagPROTO_INTERFACE*, HANDLE hTransfer, int* action, const PROTOCHAR** szFilename );

	DWORD_PTR ( *GetCaps )( struct tagPROTO_INTERFACE*, int type, HANDLE hContact );
	HICON     ( *GetIcon )( struct tagPROTO_INTERFACE*, int iconIndex );
	int       ( *GetInfo )( struct tagPROTO_INTERFACE*, HANDLE hContact, int infoType );

	HANDLE    ( *SearchBasic )( struct tagPROTO_INTERFACE*, const PROTOCHAR* id );
	HANDLE    ( *SearchByEmail )( struct tagPROTO_INTERFACE*, const PROTOCHAR* email );
	HANDLE    ( *SearchByName )( struct tagPROTO_INTERFACE*, const PROTOCHAR* nick, const PROTOCHAR* firstName, const PROTOCHAR* lastName );
	HWND      ( *SearchAdvanced )( struct tagPROTO_INTERFACE*, HWND owner );
	HWND      ( *CreateExtendedSearchUI )( struct tagPROTO_INTERFACE*, HWND owner );

	int       ( *RecvContacts )( struct tagPROTO_INTERFACE*, HANDLE hContact, PROTORECVEVENT* );
	int       ( *RecvFile )( struct tagPROTO_INTERFACE*, HANDLE hContact, PROTOFILEEVENT* );
	int       ( *RecvMsg )( struct tagPROTO_INTERFACE*, HANDLE hContact, PROTORECVEVENT* );
	int       ( *RecvUrl )( struct tagPROTO_INTERFACE*, HANDLE hContact, PROTORECVEVENT* );

	int       ( *SendContacts )( struct tagPROTO_INTERFACE*, HANDLE hContact, int flags, int nContacts, HANDLE* hContactsList );
	HANDLE    ( *SendFile )( struct tagPROTO_INTERFACE*, HANDLE hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles );
	int       ( *SendMsg )( struct tagPROTO_INTERFACE*, HANDLE hContact, int flags, const char* msg );
	int       ( *SendUrl )( struct tagPROTO_INTERFACE*, HANDLE hContact, int flags, const char* url );

	int       ( *SetApparentMode )( struct tagPROTO_INTERFACE*, HANDLE hContact, int mode );
	int       ( *SetStatus )( struct tagPROTO_INTERFACE*, int iNewStatus );

	HANDLE    ( *GetAwayMsg )( struct tagPROTO_INTERFACE*, HANDLE hContact );
	int       ( *RecvAwayMsg )( struct tagPROTO_INTERFACE*, HANDLE hContact, int mode, PROTORECVEVENT* evt );
	int       ( *SendAwayMsg )( struct tagPROTO_INTERFACE*, HANDLE hContact, HANDLE hProcess, const char* msg );
	int       ( *SetAwayMsg )( struct tagPROTO_INTERFACE*, int iStatus, const PROTOCHAR* msg );

	int       ( *UserIsTyping )( struct tagPROTO_INTERFACE*, HANDLE hContact, int type );

	int       ( *OnEvent )( struct tagPROTO_INTERFACE*, PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam );
}
	PROTO_INTERFACE_VTBL;
#endif

typedef struct tagPROTO_INTERFACE
{
	#ifndef __cplusplus
		PROTO_INTERFACE_VTBL* vtbl;
	#endif

	int    m_iStatus,
	       m_iDesiredStatus,
	       m_iXStatus,
	       m_iVersion;  // version 2 or higher designate support of Unicode services
	TCHAR* m_tszUserName;
	char*  m_szProtoName;
	char*  m_szModuleName;

	DWORD  reserved[ 40 ];

	#ifdef __cplusplus
	virtual	HANDLE   __cdecl AddToList( int flags, PROTOSEARCHRESULT* psr ) = 0;
	virtual	HANDLE   __cdecl AddToListByEvent( int flags, int iContact, HANDLE hDbEvent ) = 0;

	virtual	int      __cdecl Authorize( HANDLE hDbEvent ) = 0;
	virtual	int      __cdecl AuthDeny( HANDLE hDbEvent, const PROTOCHAR* szReason ) = 0;
	virtual	int      __cdecl AuthRecv( HANDLE hContact, PROTORECVEVENT* ) = 0;
	virtual	int      __cdecl AuthRequest( HANDLE hContact, const PROTOCHAR* szMessage ) = 0;

	virtual	HANDLE   __cdecl ChangeInfo( int iInfoType, void* pInfoData ) = 0;

	virtual	HANDLE   __cdecl FileAllow( HANDLE hContact, HANDLE hTransfer, const PROTOCHAR* szPath ) = 0;
	virtual	int      __cdecl FileCancel( HANDLE hContact, HANDLE hTransfer ) = 0;
	virtual	int      __cdecl FileDeny( HANDLE hContact, HANDLE hTransfer, const PROTOCHAR* szReason ) = 0;
	virtual	int      __cdecl FileResume( HANDLE hTransfer, int* action, const PROTOCHAR** szFilename ) = 0;

	virtual	DWORD_PTR __cdecl GetCaps( int type, HANDLE hContact = NULL ) = 0;
	virtual	HICON     __cdecl GetIcon( int iconIndex ) = 0;
	virtual	int       __cdecl GetInfo( HANDLE hContact, int infoType ) = 0;

	virtual	HANDLE    __cdecl SearchBasic( const PROTOCHAR* id ) = 0;
	virtual	HANDLE    __cdecl SearchByEmail( const PROTOCHAR* email ) = 0;
	virtual	HANDLE    __cdecl SearchByName( const PROTOCHAR* nick, const PROTOCHAR* firstName, const PROTOCHAR* lastName ) = 0;
	virtual	HWND      __cdecl SearchAdvanced( HWND owner ) = 0;
	virtual	HWND      __cdecl CreateExtendedSearchUI( HWND owner ) = 0;

	virtual	int       __cdecl RecvContacts( HANDLE hContact, PROTORECVEVENT* ) = 0;
	virtual	int       __cdecl RecvFile( HANDLE hContact, PROTOFILEEVENT* ) = 0;
	virtual	int       __cdecl RecvMsg( HANDLE hContact, PROTORECVEVENT* ) = 0;
	virtual	int       __cdecl RecvUrl( HANDLE hContact, PROTORECVEVENT* ) = 0;

	virtual	int       __cdecl SendContacts( HANDLE hContact, int flags, int nContacts, HANDLE* hContactsList ) = 0;
	virtual	HANDLE    __cdecl SendFile( HANDLE hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles ) = 0;
	virtual	int       __cdecl SendMsg( HANDLE hContact, int flags, const char* msg ) = 0;
	virtual	int       __cdecl SendUrl( HANDLE hContact, int flags, const char* url ) = 0;

	virtual	int       __cdecl SetApparentMode( HANDLE hContact, int mode ) = 0;
	virtual	int       __cdecl SetStatus( int iNewStatus ) = 0;

	virtual	HANDLE    __cdecl GetAwayMsg( HANDLE hContact ) = 0;
	virtual	int       __cdecl RecvAwayMsg( HANDLE hContact, int mode, PROTORECVEVENT* evt ) = 0;
	virtual	int       __cdecl SendAwayMsg( HANDLE hContact, HANDLE hProcess, const char* msg ) = 0;
	virtual	int       __cdecl SetAwayMsg( int iStatus, const PROTOCHAR* msg ) = 0;

	virtual	int       __cdecl UserIsTyping( HANDLE hContact, int type ) = 0;

	virtual	int       __cdecl OnEvent( PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam ) = 0;
	#endif
}
	PROTO_INTERFACE;

#endif // M_PROTOINT_H__
