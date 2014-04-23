/*

Omegle plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-13 Robert Pösel

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

OmegleProto::OmegleProto(const char* proto_name, const TCHAR* username) :
	PROTO<OmegleProto>(proto_name, username)
{
	this->facy.parent = this;

	this->signon_lock_ = CreateMutex( NULL, FALSE, NULL );
	this->log_lock_ = CreateMutex( NULL, FALSE, NULL );
	this->facy.send_message_lock_ = CreateMutex( NULL, FALSE, NULL );
	this->facy.connection_lock_ = CreateMutex( NULL, FALSE, NULL );

	// Group chats
	CreateProtoService(PS_JOINCHAT, &OmegleProto::OnJoinChat);
	CreateProtoService(PS_LEAVECHAT, &OmegleProto::OnLeaveChat);

	CreateProtoService(PS_CREATEACCMGRUI, &OmegleProto::SvcCreateAccMgrUI);

	HookProtoEvent(ME_OPT_INITIALISE, &OmegleProto::OnOptionsInit);
	HookProtoEvent(ME_GC_EVENT, &OmegleProto::OnChatEvent);

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

	SkinAddNewSoundExT( "StrangerTyp", m_tszUserName, LPGENT( "Stranger typing" ));
	SkinAddNewSoundExT( "StrangerTypStop", m_tszUserName, LPGENT( "Stranger stopped typing" ));
	SkinAddNewSoundExT( "StrangerChange", m_tszUserName, LPGENT( "Changing stranger" ));
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
}

//////////////////////////////////////////////////////////////////////////////

DWORD_PTR OmegleProto::GetCaps( int type, MCONTACT hContact )
{
	switch(type) {
	case PFLAGNUM_1:
		return PF1_CHAT;
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAGNUM_4:
		return PF4_IMSENDUTF | PF4_SUPPORTTYPING;
	case PFLAG_MAXLENOFMESSAGE:
		return OMEGLE_MESSAGE_LIMIT;
	case PFLAG_UNIQUEIDTEXT:
		return (DWORD_PTR) Translate("Visible name");
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR) "Nick";
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

int OmegleProto::SetStatus( int new_status )
{
	// Routing statuses not supported by Omegle
	switch ( new_status ) {
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

INT_PTR OmegleProto::SvcCreateAccMgrUI(WPARAM wParam,LPARAM lParam)
{
	return (INT_PTR)CreateDialogParam(g_hInstance,MAKEINTRESOURCE(IDD_OmegleACCOUNT),
		(HWND)lParam, OmegleAccountProc, (LPARAM)this );
}

int OmegleProto::OnModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	// Register group chat
	GCREGISTER gcr = {sizeof(gcr)};
	gcr.dwFlags = 0; //GC_TYPNOTIF; //GC_ACKMSG;
	gcr.pszModule = m_szModuleName;
	gcr.ptszDispName = m_tszUserName;
	gcr.iMaxText = OMEGLE_MESSAGE_LIMIT;
	gcr.nColors = 0;
	gcr.pColors = NULL;
	CallService(MS_GC_REGISTER,0,reinterpret_cast<LPARAM>(&gcr));

	return 0;
}

int OmegleProto::OnOptionsInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance   = g_hInstance;
	odp.ptszTitle   = m_tszUserName;
	odp.dwInitParam = LPARAM(this);
	odp.flags       = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_DONTTRANSLATE;

	odp.position    = 271828;
	odp.ptszGroup   = LPGENT("Network");
	odp.ptszTab     = LPGENT("Account");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pfnDlgProc  = OmegleOptionsProc;
	Options_AddPage(wParam, &odp);
	return 0;
}

int OmegleProto::OnPreShutdown(WPARAM wParam,LPARAM lParam)
{
	SetStatus( ID_STATUS_OFFLINE );
	return 0;
}

int OmegleProto::OnContactDeleted(WPARAM wparam,LPARAM)
{
	//MCONTACT hContact = (MCONTACT)wparam;

	OnLeaveChat(NULL, NULL);
	return 0;
}

int OmegleProto::UserIsTyping(MCONTACT hContact, int type)
{
	if (hContact && facy.state_ == STATE_ACTIVE)
		ForkThread(&OmegleProto::SendTypingWorker, new int(type));

	return 0;
}
