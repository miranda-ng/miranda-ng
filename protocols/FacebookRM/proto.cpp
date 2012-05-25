/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-12 Robert Pösel

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

FacebookProto::FacebookProto(const char* proto_name,const TCHAR* username)
{
	m_iVersion = 2;
	m_szProtoName  = mir_strdup( proto_name );
	m_szModuleName = mir_strdup( proto_name );
	m_tszUserName  = mir_tstrdup( username );

	facy.parent = this;

	signon_lock_ = CreateMutex( NULL, FALSE, NULL );
	avatar_lock_ = CreateMutex( NULL, FALSE, NULL );
	log_lock_ = CreateMutex( NULL, FALSE, NULL );
	update_loop_lock_ = CreateEvent( NULL, FALSE, FALSE, NULL);
	facy.buddies_lock_ = CreateMutex( NULL, FALSE, NULL );
	facy.send_message_lock_ = CreateMutex( NULL, FALSE, NULL );
	facy.fcb_conn_lock_ = CreateMutex( NULL, FALSE, NULL );

	CreateProtoService(m_szModuleName, PS_CREATEACCMGRUI, &FacebookProto::SvcCreateAccMgrUI, this);
	CreateProtoService(m_szModuleName, PS_GETMYAWAYMSG,   &FacebookProto::GetMyAwayMsg,      this);
	CreateProtoService(m_szModuleName, PS_GETMYAVATAR,    &FacebookProto::GetMyAvatar,       this);
	CreateProtoService(m_szModuleName, PS_GETAVATARINFO,  &FacebookProto::GetAvatarInfo,     this);
	CreateProtoService(m_szModuleName, PS_GETAVATARCAPS,  &FacebookProto::GetAvatarCaps,     this);

	CreateProtoService(m_szModuleName, PS_JOINCHAT,  &FacebookProto::OnJoinChat,  this);
	CreateProtoService(m_szModuleName, PS_LEAVECHAT, &FacebookProto::OnLeaveChat, this);

	HookProtoEvent(ME_CLIST_PREBUILDSTATUSMENU,	&FacebookProto::OnBuildStatusMenu,	this);
	HookProtoEvent(ME_OPT_INITIALISE,			&FacebookProto::OnOptionsInit,		this);
	HookProtoEvent(ME_GC_EVENT,					&FacebookProto::OnChatOutgoing,		this);
	HookProtoEvent(ME_IDLE_CHANGED,             &FacebookProto::OnIdleChanged,		this);

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
		MessageBox(NULL,TranslateT("Unable to get Netlib connection for Facebook"),m_tszUserName,MB_OK);

	facy.set_handle(m_hNetlibUser);

	SkinAddNewSoundExT( "Notification", m_tszUserName, LPGENT( "Notification" ) );
	SkinAddNewSoundExT( "NewsFeed", m_tszUserName, LPGENT( "News Feed" ) );
	SkinAddNewSoundExT( "OtherEvent", m_tszUserName, LPGENT( "Other Event" ) );

	char *profile = Utils_ReplaceVars("%miranda_avatarcache%");
	def_avatar_folder_ = std::string(profile)+"\\"+m_szModuleName;
	mir_free(profile);
	hAvatarFolder_ = FoldersRegisterCustomPath(m_szModuleName, "Avatars", def_avatar_folder_.c_str());

	// Set all contacts offline -- in case we crashed
	SetAllContactStatuses( ID_STATUS_OFFLINE );
}

FacebookProto::~FacebookProto( )
{
	Netlib_CloseHandle( m_hNetlibUser );

	WaitForSingleObject( signon_lock_, IGNORE );
	WaitForSingleObject( avatar_lock_, IGNORE );
	WaitForSingleObject( log_lock_, IGNORE );
	WaitForSingleObject( facy.buddies_lock_, IGNORE );
	WaitForSingleObject( facy.send_message_lock_, IGNORE );

	CloseHandle( signon_lock_ );
	CloseHandle( avatar_lock_ );
	CloseHandle( log_lock_ );
	CloseHandle( update_loop_lock_ );
	CloseHandle( facy.buddies_lock_ );
	CloseHandle( facy.send_message_lock_ );
	CloseHandle( facy.fcb_conn_lock_ );

	mir_free( m_tszUserName );
	mir_free( m_szModuleName );
	mir_free( m_szProtoName );
}

//////////////////////////////////////////////////////////////////////////////

DWORD_PTR FacebookProto::GetCaps( int type, HANDLE hContact )
{
	switch(type)
	{
	case PFLAGNUM_1: // TODO: Other caps available: PF1_BASICSEARCH, PF1_SEARCHBYEMAIL
	{
		DWORD_PTR flags = PF1_IM | PF1_CHAT | PF1_SERVERCLIST | PF1_AUTHREQ | /*PF1_ADDED |*/ PF1_BASICSEARCH | PF1_USERIDISEMAIL | PF1_SEARCHBYEMAIL | PF1_SEARCHBYNAME | PF1_ADDSEARCHRES; // | PF1_VISLIST | PF1_INVISLIST;
		
		if ( getByte( FACEBOOK_KEY_SET_MIRANDA_STATUS, 0 ) )
			return flags |= PF1_MODEMSG;
		else
			return flags |= PF1_MODEMSGRECV;
	}
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_ONTHEPHONE | PF2_IDLE; // | PF2_SHORTAWAY;
	case PFLAGNUM_3:
		if ( getByte( FACEBOOK_KEY_SET_MIRANDA_STATUS, 0 ) )
			return PF2_ONLINE; // | PF2_SHORTAWAY;
		else
			return 0;
	case PFLAGNUM_4:
		return PF4_NOCUSTOMAUTH | PF4_FORCEADDED | PF4_IMSENDUTF | PF4_AVATARS | PF4_SUPPORTTYPING | PF4_NOAUTHDENYREASON | PF4_IMSENDOFFLINE;
	case PFLAGNUM_5:
		return PF2_ONTHEPHONE;
	case PFLAG_MAXLENOFMESSAGE:
		return FACEBOOK_MESSAGE_LIMIT;
	case PFLAG_UNIQUEIDTEXT:
		return (int) "Facebook ID";
	case PFLAG_UNIQUEIDSETTING:
		return (int) FACEBOOK_KEY_ID;
	}
	return 0;
}

HICON FacebookProto::GetIcon(int index)
{
	if(LOWORD(index) == PLI_PROTOCOL)
	{
		HICON ico = (HICON)CallService(MS_SKIN2_GETICON,0,(LPARAM)"Facebook_facebook");
		return CopyIcon(ico);
	} else {
		return 0;
	}
}

//////////////////////////////////////////////////////////////////////////////

int FacebookProto::SetStatus( int new_status )
{
	LOG("===== Beginning SetStatus process");
	
	// Routing statuses not supported by Facebook
	switch ( new_status )
	{
	case ID_STATUS_INVISIBLE:
	case ID_STATUS_OFFLINE:	
		m_iDesiredStatus = new_status;
		break;

	// TODO RM: needed/useful?
	case ID_STATUS_CONNECTING:
		m_iDesiredStatus = ID_STATUS_OFFLINE;
		break;

	case ID_STATUS_IDLE:	
	default:
		m_iDesiredStatus = ID_STATUS_INVISIBLE;
		if (DBGetContactSettingByte(NULL,m_szModuleName,FACEBOOK_KEY_MAP_STATUSES, DEFAULT_MAP_STATUSES))
			break;
	case ID_STATUS_ONLINE:
	case ID_STATUS_FREECHAT:
		m_iDesiredStatus = ID_STATUS_ONLINE;
		break;
	}

	if ( m_iStatus == ID_STATUS_CONNECTING )
	{
		LOG("===== Status is connecting, no change");
		return 0;
	}

	if ( m_iStatus == m_iDesiredStatus)
	{
		LOG("===== Statuses are same, no change");
		return 0;
	}

	facy.invisible_ = ( new_status == ID_STATUS_INVISIBLE );
  
	ForkThread( &FacebookProto::ChangeStatus, this );

	return 0;
}

int FacebookProto::SetAwayMsg( int status, const PROTOCHAR *msg )
{
	if (!msg)
	{
		last_status_msg_.clear();
		return 0;
	}

	char *narrow = mir_utf8encodeT(msg);
	if (last_status_msg_ != narrow) last_status_msg_ = narrow;
	utils::mem::detract(narrow);

	if (isOnline() && getByte(FACEBOOK_KEY_SET_MIRANDA_STATUS, DEFAULT_SET_MIRANDA_STATUS))
	{
		ForkThread(&FacebookProto::SetAwayMsgWorker, this, NULL);
	}
	return 0;
}

void FacebookProto::SetAwayMsgWorker(void *)
{
	if ( !last_status_msg_.empty() )
		facy.set_status( last_status_msg_ );
}

HANDLE FacebookProto::SearchBasic( const PROTOCHAR* id )
{
	if (isOffline())
		return 0;
	
	TCHAR* email = mir_tstrdup(id);
	ForkThread(&FacebookProto::SearchAckThread, this, (void*)email);

	return email;
}

HANDLE FacebookProto::SearchByEmail( const PROTOCHAR* email )
{
	return SearchBasic(email);
}

HANDLE FacebookProto::SearchByName( const PROTOCHAR* nick, const PROTOCHAR* firstName, const PROTOCHAR* lastName )
{
	TCHAR arg[200];
	_sntprintf (arg, SIZEOF(arg), _T("%s %s %s"), nick, firstName, lastName);
	return SearchBasic(arg);
}

HANDLE FacebookProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	char *id = mir_t2a_cp(psr->id, CP_UTF8);
	char *name = mir_t2a_cp(psr->firstName, CP_UTF8);
	char *surname = mir_t2a_cp(psr->lastName, CP_UTF8);

	facebook_user fbu;
	fbu.user_id = id;
	fbu.real_name = name;
	fbu.real_name += " ";
	fbu.real_name += surname;

	HANDLE hContact = AddToContactList(&fbu, FACEBOOK_CONTACT_NONE, false, fbu.real_name.c_str());
	if (hContact) {
		if (flags & PALF_TEMPORARY)
		{
			DBWriteContactSettingByte(hContact, "Clist", "Hidden", 1);
			DBWriteContactSettingByte(hContact, "Clist", "NotOnList", 1);
		}
		else if (DBGetContactSettingByte(hContact, "CList", "NotOnList", 0))
		{
			DBDeleteContactSetting(hContact, "CList", "Hidden");
			DBDeleteContactSetting(hContact, "CList", "NotOnList");
		}
	}

	mir_free(id);
	mir_free(name);
	mir_free(surname);

	return hContact;
}

int FacebookProto::AuthRequest(HANDLE hContact,const PROTOCHAR *message)
{
	return RequestFriendship((WPARAM)hContact, NULL);
}

int FacebookProto::Authorize(HANDLE hContact)
{
	return ApproveFriendship((WPARAM)hContact, NULL);
}

int FacebookProto::AuthDeny(HANDLE hContact,const PROTOCHAR *reason)
{
	// TODO: hide from facebook requests list
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
// SERVICES

int FacebookProto::GetMyAwayMsg( WPARAM wParam, LPARAM lParam )
{
	DBVARIANT dbv = { DBVT_TCHAR };
	if ( !getTString( "StatusMsg", &dbv ) && lstrlen( dbv.ptszVal ) != 0 )
	{
		int res = (lParam & SGMA_UNICODE) ? (INT_PTR)mir_t2u(dbv.ptszVal) : (INT_PTR)mir_t2a(dbv.ptszVal);
		DBFreeVariant( &dbv );
		return res;
	} else {
		return 0;
	}
}

int FacebookProto::OnIdleChanged( WPARAM wParam, LPARAM lParam )
{
	if (m_iStatus == ID_STATUS_INVISIBLE || m_iStatus <= ID_STATUS_OFFLINE)
		return 0;

	bool bIdle = (lParam & IDF_ISIDLE) != 0;
	bool bPrivacy = (lParam & IDF_PRIVACY) != 0;

	if (facy.is_idle_ && !bIdle)
	{
		facy.is_idle_ = false;
		SetStatus(m_iDesiredStatus);
	}
	else if (!facy.is_idle_ && bIdle && !bPrivacy && m_iDesiredStatus != ID_STATUS_INVISIBLE)
	{
		facy.is_idle_ = true;
		SetStatus(ID_STATUS_IDLE);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////

int FacebookProto::OnEvent(PROTOEVENTTYPE event,WPARAM wParam,LPARAM lParam)
{
	switch(event)
	{
	case EV_PROTO_ONLOAD:
		return OnModulesLoaded(wParam,lParam);

	case EV_PROTO_ONEXIT:
		return OnPreShutdown(wParam,lParam);
	
	case EV_PROTO_ONOPTIONS:
		return OnOptionsInit(wParam,lParam);

	case EV_PROTO_ONCONTACTDELETED:
 		return OnContactDeleted(wParam,lParam);
	}

	return 1;
}

//////////////////////////////////////////////////////////////////////////////
// EVENTS

int FacebookProto::SvcCreateAccMgrUI(WPARAM wParam,LPARAM lParam)
{
	return (int)CreateDialogParam(g_hInstance,MAKEINTRESOURCE(IDD_FACEBOOKACCOUNT), 
		 (HWND)lParam, FBAccountProc, (LPARAM)this );
}

int FacebookProto::OnModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	// Register group chat
	GCREGISTER gcr = {sizeof(gcr)};
	gcr.dwFlags = 0; //GC_ACKMSG;
	gcr.pszModule = m_szModuleName;
	gcr.pszModuleDispName = m_szModuleName;
	gcr.iMaxText = FACEBOOK_MESSAGE_LIMIT;
	gcr.nColors = 0;
	gcr.pColors = NULL;
	CallService(MS_GC_REGISTER,0,reinterpret_cast<LPARAM>(&gcr));

	return 0;
}

int FacebookProto::OnPreShutdown(WPARAM wParam,LPARAM lParam)
{
	SetStatus( ID_STATUS_OFFLINE );
	return 0;
}

int FacebookProto::OnOptionsInit(WPARAM wParam,LPARAM lParam)
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
	odp.pfnDlgProc  = FBOptionsProc;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	
	odp.position    = 271829;
	odp.ptszTab     = LPGENT("Advanced");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_ADVANCED);
	odp.pfnDlgProc  = FBOptionsAdvancedProc;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	odp.position    = 271830;
	if(ServiceExists(MS_POPUP_ADDPOPUPT))
		odp.ptszGroup   = LPGENT("Popups");
	odp.ptszTab     = LPGENT("Events");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_EVENTS);
	odp.pfnDlgProc  = FBEventsProc;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	return 0;
}

int FacebookProto::OnMind(WPARAM,LPARAM)
{
	HWND hDlg = CreateDialogParam( g_hInstance, MAKEINTRESOURCE( IDD_MIND ),
		 ( HWND )0, FBMindProc, reinterpret_cast<LPARAM>( this ) );
	ShowWindow( hDlg, SW_SHOW );
	return FALSE;
}

int FacebookProto::VisitProfile(WPARAM wParam,LPARAM lParam)
{
	HANDLE hContact = reinterpret_cast<HANDLE>(wParam);

	DBVARIANT dbv;
	if( wParam != 0 && !DBGetContactSettingString(hContact,m_szModuleName,"Homepage",&dbv) )
	{
		CallService(MS_UTILS_OPENURL,1,reinterpret_cast<LPARAM>(dbv.pszVal));
		DBFreeVariant(&dbv);
	}
	else if (DBGetContactSettingByte(hContact,m_szModuleName,"ChatRoom",0))
	{
		std::string url = FACEBOOK_URL_GROUP;
		if (!DBGetContactSettingString(hContact,m_szModuleName,"ChatRoomID",&dbv)) {
			url += dbv.pszVal;
			DBFreeVariant(&dbv);
		}
		CallService(MS_UTILS_OPENURL,1,reinterpret_cast<LPARAM>(url.c_str()));
	} else {
		// TODO: why isn't wParam == 0 when is status menu moved to main menu?
		CallService(MS_UTILS_OPENURL,1,reinterpret_cast<LPARAM>(FACEBOOK_URL_PROFILE));
	}

	return 0;
}

int FacebookProto::CancelFriendship(WPARAM wParam,LPARAM lParam)
{
	if (wParam == NULL || isOffline())
		return 0;

	bool deleting = (lParam == 1);

	HANDLE hContact = reinterpret_cast<HANDLE>(wParam);

	// Ignore groupchats and, if deleting, also not-friends
	if (DBGetContactSettingByte(hContact, m_szModuleName, "ChatRoom", 0)
		|| (deleting && DBGetContactSettingByte(hContact, m_szModuleName, FACEBOOK_KEY_CONTACT_TYPE, 0) != FACEBOOK_CONTACT_FRIEND))
		return 0;

	DBVARIANT dbv;
	char str[256];

	if ( !DBGetContactSettingUTF8String(hContact, m_szModuleName, FACEBOOK_KEY_NAME, &dbv) ) {
		mir_snprintf(str,SIZEOF(str),Translate("Do you want to cancel your friendship with '%s'?"), dbv.pszVal);
		DBFreeVariant(&dbv);
	} else if( !DBGetContactSettingUTF8String(hContact,m_szModuleName,FACEBOOK_KEY_ID,&dbv) ) {
		mir_snprintf(str,SIZEOF(str),Translate("Do you want to cancel your friendship with '%s'?"), dbv.pszVal);
		DBFreeVariant(&dbv);
	}

	TCHAR *text = mir_a2t_cp(str, CP_UTF8);
	if (MessageBox( 0, text, m_tszUserName, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2 ) == IDYES) {
		
		if( !DBGetContactSettingString(hContact,m_szModuleName,FACEBOOK_KEY_ID,&dbv) )
		{
			std::string* id = new std::string(dbv.pszVal);

			if (deleting) {
				facebook_user* fbu = facy.buddies.find( (*id) );
				if (fbu != NULL) {
					fbu->handle = NULL;
				}
			}

			ForkThread( &FacebookProto::DeleteContactFromServer, this, ( void* )id );
			DBFreeVariant(&dbv);
		}
				
	}
	mir_free(text);

	return 0;
}

int FacebookProto::RequestFriendship(WPARAM wParam,LPARAM lParam)
{
	if (wParam == NULL || isOffline())
		return 0;

	HANDLE hContact = reinterpret_cast<HANDLE>(wParam);

	DBVARIANT dbv;
	if( !DBGetContactSettingString(hContact,m_szModuleName,FACEBOOK_KEY_ID,&dbv) )
	{
		std::string* id = new std::string(dbv.pszVal);
		ForkThread( &FacebookProto::AddContactToServer, this, ( void* )id );
		DBFreeVariant(&dbv);
	}

	return 0;
}

int FacebookProto::ApproveFriendship(WPARAM wParam,LPARAM lParam)
{
	if (wParam == NULL || isOffline())
		return 0;

	HANDLE *hContact = new HANDLE(reinterpret_cast<HANDLE>(wParam));
	ForkThread( &FacebookProto::ApproveContactToServer, this, ( void* )hContact );

	return 0;
}
