/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-12  George Hazan
Copyright ( C ) 2007     Maxim Mluhov

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "jabber.h"

#include <fcntl.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "jabber_list.h"
#include "jabber_iq.h"
#include "jabber_caps.h"
#include "m_file.h"
#include "m_addcontact.h"
#include "jabber_disco.h"
#include "m_proto_listeningto.h"

/////////////////////////////////////////////////////////////////////////////////////////
// OnContactDeleted - processes a contact deletion

int CJabberProto::OnContactDeleted( WPARAM wParam, LPARAM )
{
	if ( !m_bJabberOnline )	// should never happen
		return 0;

	DBVARIANT dbv;
	if ( !JGetStringT(( HANDLE ) wParam, JGetByte( (HANDLE ) wParam, "ChatRoom", 0 )?(char*)"ChatRoomID":(char*)"jid", &dbv )) {
		if ( ListExist( LIST_ROSTER, dbv.ptszVal )) {
			if ( !_tcschr( dbv.ptszVal, _T( '@' ))) {
				TCHAR szStrippedJid[JABBER_MAX_JID_LEN];
				JabberStripJid( m_ThreadInfo->fullJID, szStrippedJid, SIZEOF(szStrippedJid));
				TCHAR *szDog = _tcschr( szStrippedJid, _T('@'));
				if ( szDog && _tcsicmp( szDog + 1, dbv.ptszVal ))
					m_ThreadInfo->send( XmlNodeIq( _T("set"), SerialNext(), dbv.ptszVal ) << XQUERY( _T(JABBER_FEAT_REGISTER)) << XCHILD( _T("remove")));
			}

			// Remove from roster, server also handles the presence unsubscription process.
			m_ThreadInfo->send( XmlNodeIq( _T("set"), SerialNext()) << XQUERY( _T(JABBER_FEAT_IQ_ROSTER))
				<< XCHILD( _T("item")) << XATTR( _T("jid"), dbv.ptszVal ) << XATTR( _T("subscription"), _T("remove")));
		}

		JFreeVariant( &dbv );
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberDbSettingChanged - process database changes

static TCHAR* sttSettingToTchar( DBCONTACTWRITESETTING* cws )
{
	switch( cws->value.type ) {
	case DBVT_ASCIIZ:
		return mir_a2t( cws->value.pszVal );

	case DBVT_UTF8:
		return mir_utf8decodeT( cws->value.pszVal );

	case DBVT_WCHAR:
		return mir_u2t( cws->value.pwszVal );
	}
	return NULL;
}

void __cdecl CJabberProto::OnRenameGroup( DBCONTACTWRITESETTING* cws, HANDLE hContact )
{
	DBVARIANT jid, dbv;
	if ( JGetStringT( hContact, "jid", &jid ))
		return;

	JABBER_LIST_ITEM* item = ListGetItemPtr( LIST_ROSTER, jid.ptszVal );
	JFreeVariant( &jid );
	if ( item == NULL )
		return;

	TCHAR* nick;
	if ( !DBGetContactSettingTString( hContact, "CList", "MyHandle", &dbv )) {
		nick = mir_tstrdup( dbv.ptszVal );
		JFreeVariant( &dbv );
	}
	else if ( !JGetStringT( hContact, "Nick", &dbv )) {
		nick = mir_tstrdup( dbv.ptszVal );
		JFreeVariant( &dbv );
	}
	else nick = JabberNickFromJID( item->jid );
	if ( nick == NULL )
		return;

	if ( cws->value.type == DBVT_DELETED ) {
		if ( item->group != NULL ) {
			Log( "Group set to nothing" );
			AddContactToRoster( item->jid, nick, NULL );
		}
	}
	else {
		TCHAR* p = sttSettingToTchar( cws );
		if ( cws->value.pszVal != NULL && lstrcmp( p, item->group )) {
			Log( "Group set to " TCHAR_STR_PARAM, p );
			if ( p )
				AddContactToRoster( item->jid, nick, p );
		}
		mir_free( p );
	}
	mir_free( nick );
}

void __cdecl CJabberProto::OnRenameContact( DBCONTACTWRITESETTING* cws, HANDLE hContact )
{
	DBVARIANT jid;
	if ( JGetStringT( hContact, "jid", &jid ))
		return;

	JABBER_LIST_ITEM* item = ListGetItemPtr( LIST_ROSTER, jid.ptszVal );
	JFreeVariant( &jid );
	if ( item == NULL )
		return;

	if ( cws->value.type == DBVT_DELETED ) {
		TCHAR* nick = ( TCHAR* )JCallService( MS_CLIST_GETCONTACTDISPLAYNAME, ( WPARAM )hContact, GCDNF_NOMYHANDLE | GCDNF_TCHAR );
		AddContactToRoster( item->jid, nick, item->group );
		mir_free(nick);
		return;
	}

	TCHAR* newNick = sttSettingToTchar( cws );
	if ( newNick ) {
		if ( lstrcmp( item->nick, newNick )) {
			Log( "Renaming contact " TCHAR_STR_PARAM ": " TCHAR_STR_PARAM " -> " TCHAR_STR_PARAM, item->jid, item->nick, newNick );
			AddContactToRoster( item->jid, newNick, item->group );
		}
		mir_free( newNick );
}	}

void __cdecl CJabberProto::OnAddContactForever( DBCONTACTWRITESETTING* cws, HANDLE hContact )
{
	if ( cws->value.type != DBVT_DELETED && !( cws->value.type==DBVT_BYTE && cws->value.bVal==0 ))
		return;

	DBVARIANT jid, dbv;
	if ( JGetStringT( hContact, "jid", &jid ))
		return;

	TCHAR *nick;
	Log( "Add " TCHAR_STR_PARAM " permanently to list", jid.pszVal );
	if ( !DBGetContactSettingTString( hContact, "CList", "MyHandle", &dbv )) {
		nick = mir_tstrdup( dbv.ptszVal );
		JFreeVariant( &dbv );
	}
	else if ( !JGetStringT( hContact, "Nick", &dbv )) {
		nick = mir_tstrdup( dbv.ptszVal );
		JFreeVariant( &dbv );
	}
	else nick = JabberNickFromJID( jid.ptszVal );
	if ( nick == NULL ) {
		JFreeVariant( &jid );
		return;
	}

	if ( !DBGetContactSettingTString( hContact, "CList", "Group", &dbv )) {
		AddContactToRoster( jid.ptszVal, nick, dbv.ptszVal );
		JFreeVariant( &dbv );
	}
	else AddContactToRoster( jid.ptszVal, nick, NULL );

	m_ThreadInfo->send( XmlNode( _T("presence")) << XATTR( _T("to"), jid.ptszVal ) << XATTR( _T("type"), _T("subscribe")));

	SendGetVcard( jid.ptszVal );

	mir_free( nick );
	DBDeleteContactSetting( hContact, "CList", "Hidden" );
	JFreeVariant( &jid );
}

int __cdecl CJabberProto::OnDbSettingChanged( WPARAM wParam, LPARAM lParam )
{
	HANDLE hContact = ( HANDLE ) wParam;
	if ( hContact == NULL || !m_bJabberOnline )
		return 0;

	DBCONTACTWRITESETTING* cws = ( DBCONTACTWRITESETTING* )lParam;
	if ( strcmp( cws->szModule, "CList" ))
		return 0;

	if ( !strcmp( cws->szSetting, "Group" ))
		OnRenameGroup( cws, hContact );
	else if ( !strcmp( cws->szSetting, "MyHandle" ))
		OnRenameContact( cws, hContact );
	else if ( !strcmp( cws->szSetting, "NotOnList" ))
		OnAddContactForever( cws, hContact );

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnIdleChanged - tracks idle start time for XEP-0012 support

int CJabberProto::OnIdleChanged( WPARAM, LPARAM lParam )
{
	// don't report idle time, if user disabled
	if (lParam & IDF_PRIVACY) {
		m_tmJabberIdleStartTime = 0;
		return 0;
	}

	if ( lParam & IDF_ISIDLE ) {
		MIRANDA_IDLE_INFO mii = { 0 };
		mii.cbSize = sizeof( mii );
		CallService( MS_IDLE_GETIDLEINFO, 0, (LPARAM)&mii );
		m_tmJabberIdleStartTime = time( 0 ) - mii.idleTime * 60;
	} else
		m_tmJabberIdleStartTime = 0;
	return 0;
}
