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

#include "common.h"

OmegleProto::OmegleProto(const char* proto_name, const TCHAR* username)
{
	m_iVersion = 2;
	m_szProtoName  = mir_strdup( proto_name );
	m_szModuleName = mir_strdup( proto_name );
	m_tszUserName  = mir_tstrdup( username );

	this->facy.parent = this;

	this->signon_lock_ = CreateMutex( NULL, FALSE, NULL );
	this->log_lock_ = CreateMutex( NULL, FALSE, NULL );
	this->facy.send_message_lock_ = CreateMutex( NULL, FALSE, NULL );
	this->facy.connection_lock_ = CreateMutex( NULL, FALSE, NULL );

	// Group chats
	CreateProtoService(m_szModuleName, PS_JOINCHAT, &OmegleProto::OnJoinChat, this);
	CreateProtoService(m_szModuleName, PS_LEAVECHAT, &OmegleProto::OnLeaveChat, this);

	CreateProtoService(m_szModuleName, PS_CREATEACCMGRUI, &OmegleProto::SvcCreateAccMgrUI, this);
	
	HookProtoEvent(ME_OPT_INITIALISE, &OmegleProto::OnOptionsInit, this);
	HookProtoEvent(ME_GC_EVENT, &OmegleProto::OnChatEvent, this);

	// Create standard network connection
	TCHAR descr[512];
	NETLIBUSER nlu = {sizeof(nlu)};
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_TCHAR;
	nlu.szSettingsModule = m_szModuleName;
	char module[512];
	mir_snprintf(module,SIZEOF(module),"%sAv",m_szModuleName);
	nlu.szSettingsModule = module;
	mir_sntprintf(descr,SIZEOF(descr),TranslateT("%s server connection"),m_tszUserName);
	nlu.ptszDescriptiveName = descr;
	m_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER,0,(LPARAM)&nlu);
	if(m_hNetlibUser == NULL)
		MessageBox(NULL,TranslateT("Unable to get Netlib connection for Omegle"),m_tszUserName,MB_OK);

	facy.set_handle(m_hNetlibUser);

	SkinAddNewSoundExT( "StrangerTyp", m_tszUserName, LPGENT( "Stranger typing" ) );
	SkinAddNewSoundExT( "StrangerTypStop", m_tszUserName, LPGENT( "Stranger stopped typing" ) );
	SkinAddNewSoundExT( "StrangerChange", m_tszUserName, LPGENT( "Changing stranger" ) );
}

OmegleProto::~OmegleProto( )
{
	Netlib_CloseHandle( m_hNetlibUser );

	WaitForSingleObject( this->signon_lock_, IGNORE );
	WaitForSingleObject( this->log_lock_, IGNORE );
	WaitForSingleObject( this->facy.send_message_lock_, IGNORE );
	WaitForSingleObject( this->events_loop_lock_, IGNORE );

	CloseHandle( this->signon_lock_ );
	CloseHandle( this->log_lock_ );
	CloseHandle( this->facy.send_message_lock_ );
	CloseHandle( this->events_loop_lock_ );
	CloseHandle( this->facy.connection_lock_ );

	mir_free( this->facy.nick_ );

	mir_free( m_tszUserName );
	mir_free( m_szModuleName );
	mir_free( m_szProtoName );	
}

//////////////////////////////////////////////////////////////////////////////

DWORD_PTR OmegleProto::GetCaps( int type, HANDLE hContact )
{
	switch(type)
	{
	case PFLAGNUM_1:
		return PF1_IM | PF1_CHAT; // TODO: Why we need PF1_IM to activate smileys? Shouldnt be PF1_CHAT enough?
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAGNUM_4:
		return PF4_IMSENDUTF; // | PF4_SUPPORTTYPING;
	case PFLAG_MAXLENOFMESSAGE:
		return OMEGLE_MESSAGE_LIMIT;
	}
	return 0;
}

HICON OmegleProto::GetIcon(int index)
{
	if(LOWORD(index) == PLI_PROTOCOL)
	{
		HICON ico = (HICON)CallService(MS_SKIN2_GETICON,0,(LPARAM)"Omegle_omegle");
		return CopyIcon(ico);
	} else {
		return 0;
	}
}

//////////////////////////////////////////////////////////////////////////////

int OmegleProto::SetStatus( int new_status )
{
	// Routing statuses not supported by Omegle
	switch ( new_status )
	{
	case ID_STATUS_OFFLINE:
	case ID_STATUS_CONNECTING:
		new_status = ID_STATUS_OFFLINE;
		break;
	default:
		new_status = ID_STATUS_ONLINE;
		break;
	}

	m_iDesiredStatus = new_status;

	if ( new_status == m_iStatus)
	{
		return 0;
	}

	if ( m_iStatus == ID_STATUS_CONNECTING && new_status != ID_STATUS_OFFLINE )
	{
		return 0;		
	}

	if ( new_status == ID_STATUS_OFFLINE )
	{
		ForkThread( &OmegleProto::SignOff, this );
	}
	else
	{
		ForkThread( &OmegleProto::SignOn, this );
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

int OmegleProto::OnEvent(PROTOEVENTTYPE event,WPARAM wParam,LPARAM lParam)
{
	switch(event)
	{
	case EV_PROTO_ONLOAD:
		return OnModulesLoaded(wParam,lParam);

	case EV_PROTO_ONEXIT:
		return OnPreShutdown  (wParam,lParam);
	
	case EV_PROTO_ONOPTIONS:
		return OnOptionsInit  (wParam,lParam);

	case EV_PROTO_ONCONTACTDELETED:
		return OnContactDeleted(wParam,lParam);
	}

	return 1;
}

//////////////////////////////////////////////////////////////////////////////
// EVENTS

int OmegleProto::SvcCreateAccMgrUI(WPARAM wParam,LPARAM lParam)
{
	return (int)CreateDialogParam(g_hInstance,MAKEINTRESOURCE(IDD_OmegleACCOUNT),
		(HWND)lParam, OmegleAccountProc, (LPARAM)this );
}

int OmegleProto::OnModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	// Register group chat
	GCREGISTER gcr = {sizeof(gcr)};
	gcr.dwFlags = GC_TYPNOTIF; //GC_ACKMSG;
	gcr.pszModule = m_szModuleName;
	gcr.pszModuleDispName = m_szModuleName;
	gcr.iMaxText = OMEGLE_MESSAGE_LIMIT;
	gcr.nColors = 0;
	gcr.pColors = NULL;
	CallService(MS_GC_REGISTER,0,reinterpret_cast<LPARAM>(&gcr));

	return 0;
}

int OmegleProto::OnOptionsInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {sizeof(odp)};
	odp.hInstance   = g_hInstance;
	odp.ptszTitle   = m_tszUserName;
	odp.dwInitParam = LPARAM(this);
	odp.flags       = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_DONTTRANSLATE;

	odp.position    = 271828;
	odp.ptszGroup   = LPGENT("Network");
	odp.ptszTab     = LPGENT("Account");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pfnDlgProc  = OmegleOptionsProc;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	return 0;
}

int OmegleProto::OnPreShutdown(WPARAM wParam,LPARAM lParam)
{
	SetStatus( ID_STATUS_OFFLINE );
	return 0;
}

int OmegleProto::OnContactDeleted(WPARAM wparam,LPARAM)
{
	//HANDLE hContact = (HANDLE)wparam;

	OnLeaveChat(NULL, NULL);
	return 0;
}


//////////////////////////////////////////////////////////////////////////////
// OTHER

/*bool OmegleProto::IsMyContact(HANDLE hContact, bool include_chat)
{
	const char *proto = reinterpret_cast<char*>( CallService(MS_PROTO_GETCONTACTBASEPROTO,
		reinterpret_cast<WPARAM>(hContact),0) );

	if( proto && strcmp(m_szModuleName,proto) == 0 )
	{
		if( include_chat )
			return true;
		else
			return DBGetContactSettingByte(hContact,m_szModuleName,"ChatRoom",0) == 0;
	} else {
		return false;
	}
}*/
