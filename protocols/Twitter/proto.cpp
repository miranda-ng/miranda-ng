/*
Copyright © 2009 Jim Porter

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

#include "proto.h"

#include "utility.h"
#include "theme.h"
#include "ui.h"

#include "m_folders.h"
#include "m_historyevents.h"

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <direct.h>

TwitterProto::TwitterProto(const char *proto_name,const TCHAR *username)
{
	m_szProtoName  = mir_strdup (proto_name);
	m_szModuleName = mir_strdup (proto_name);
	m_tszUserName  = mir_tstrdup(username);

	CreateProtoService(m_szModuleName,PS_CREATEACCMGRUI,
		&TwitterProto::SvcCreateAccMgrUI,this);
	CreateProtoService(m_szModuleName,PS_GETNAME,  &TwitterProto::GetName,    this);
	CreateProtoService(m_szModuleName,PS_GETSTATUS,&TwitterProto::GetStatus,  this);

	CreateProtoService(m_szModuleName,PS_JOINCHAT, &TwitterProto::OnJoinChat, this);
	CreateProtoService(m_szModuleName,PS_LEAVECHAT,&TwitterProto::OnLeaveChat,this);

	CreateProtoService(m_szModuleName,PS_GETMYAVATAR,&TwitterProto::GetAvatar,this);
	CreateProtoService(m_szModuleName,PS_SETMYAVATAR,&TwitterProto::SetAvatar,this);

	HookProtoEvent(ME_DB_CONTACT_DELETED,       &TwitterProto::OnContactDeleted,     this);
	HookProtoEvent(ME_CLIST_PREBUILDSTATUSMENU, &TwitterProto::OnBuildStatusMenu,    this);
	HookProtoEvent(ME_OPT_INITIALISE,           &TwitterProto::OnOptionsInit,        this);

	char *profile = Utils_ReplaceVars("%miranda_avatarcache%");
	def_avatar_folder_ = std::string(profile)+"\\"+m_szModuleName;
	mir_free(profile);
	hAvatarFolder_ = FoldersRegisterCustomPath(m_szModuleName,"Avatars",
		def_avatar_folder_.c_str());

	// Initialize hotkeys
	char text[512];
	HOTKEYDESC hkd = {sizeof(hkd)};
	hkd.cbSize = sizeof(hkd);
	hkd.pszName = text;
	hkd.pszService = text;
	hkd.pszSection = m_szModuleName; // Section title; TODO: use username?

	mir_snprintf(text,SIZEOF(text),"%s/Tweet",m_szModuleName);
	hkd.pszDescription = "Send Tweet";
	CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hkd);

	signon_lock_  = CreateMutex(0,false,0);
	avatar_lock_  = CreateMutex(0,false,0);
	twitter_lock_ = CreateMutex(0,false,0);

	SetAllContactStatuses(ID_STATUS_OFFLINE); // In case we crashed last time

	// set Tokens and stuff

	//mirandas keys
	ConsumerKey = OAUTH_CONSUMER_KEY;
	ConsumerSecret = OAUTH_CONSUMER_SECRET;

	//codebrook's keys
	//wstring ConsumerKey = L"T6XLGzrkfsJAgU59dbIjSA";
	//wstring ConsumerSecret = L"xsvm2NAksjsJGw63RMWAtec3Lz5uiBusfVt48gbdKLg";

	AccessUrl = L"http://twitter.com/oauth/access_token";
	AuthorizeUrl = L"http://twitter.com/oauth/authorize?oauth_token=%s";
	RequestUrl = L"http://twitter.com/oauth/request_token?some_other_parameter=hello&another_one=goodbye#meep"; // threw in some parameters for fun, and to test UrlGetQuery
	UserTimelineUrl = L"http://twitter.com/statuses/user_timeline.json";
}

TwitterProto::~TwitterProto()
{
	twit_.Disconnect();

	if(hNetlib_)
		Netlib_CloseHandle(hNetlib_);
	if(hAvatarNetlib_)
		Netlib_CloseHandle(hAvatarNetlib_);

	CloseHandle(twitter_lock_);
	CloseHandle(avatar_lock_);
	CloseHandle(signon_lock_);

	mir_free(m_szProtoName);
	mir_free(m_szModuleName);
	mir_free(m_tszUserName);
}

// *************************

DWORD TwitterProto::GetCaps(int type,HANDLE hContact)
{
	switch(type)
	{
	case PFLAGNUM_1:
		return PF1_IM | PF1_MODEMSGRECV | PF1_BASICSEARCH | PF1_SEARCHBYEMAIL |
			PF1_SERVERCLIST | PF1_CHANGEINFO;
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAGNUM_3:
		return PF2_ONLINE;
	case PFLAGNUM_4:
		return PF4_NOCUSTOMAUTH | PF4_IMSENDUTF | PF4_AVATARS;
	case PFLAG_MAXLENOFMESSAGE:
		return 159; // 140 + <max length of a users name (15 apparently)> + 4 ("RT @").  this allows for the new style retweets
	case PFLAG_UNIQUEIDTEXT:
		return (int) "Username";
	case PFLAG_UNIQUEIDSETTING:
		return (int) TWITTER_KEY_UN;
	}
	return 0;
}

HICON TwitterProto::GetIcon(int index)
{
	if(LOWORD(index) == PLI_PROTOCOL)
	{
		HICON ico = (HICON)CallService(MS_SKIN2_GETICON,0,(LPARAM)"Twitter_twitter");
		return CopyIcon(ico);
	}
	else
		return 0;
}

// *************************

int TwitterProto::RecvMsg(HANDLE hContact,PROTORECVEVENT *pre)
{
	CCSDATA ccs = { hContact,PSR_MESSAGE,0,reinterpret_cast<LPARAM>(pre) };
	return CallService(MS_PROTO_RECVMSG,0,reinterpret_cast<LPARAM>(&ccs));
}

// *************************

struct send_direct
{
	send_direct(HANDLE hContact,const std::string &msg) : hContact(hContact),msg(msg) {}
	HANDLE hContact;
	std::string msg;
};

void TwitterProto::SendSuccess(void *p)
{
	if(p == 0)
		return;
	send_direct *data = static_cast<send_direct*>(p);

	DBVARIANT dbv;
	if( !DBGetContactSettingString(data->hContact,m_szModuleName,TWITTER_KEY_UN,&dbv) )
	{
		ScopedLock s(twitter_lock_);
		twit_.send_direct(dbv.pszVal,data->msg);

		ProtoBroadcastAck(m_szModuleName,data->hContact,ACKTYPE_MESSAGE,ACKRESULT_SUCCESS,
			(HANDLE)1,0);
		DBFreeVariant(&dbv);
	}

	delete data;
}

int TwitterProto::SendMsg(HANDLE hContact,int flags,const char *msg)
{
	if(m_iStatus != ID_STATUS_ONLINE)
		return 0;

	ForkThread(&TwitterProto::SendSuccess, this,new send_direct(hContact,msg));
	return 1;
}

// *************************

int TwitterProto::SetStatus(int new_status)
{
	int old_status = m_iStatus;
	if(new_status == m_iStatus)
		return 0;

	m_iDesiredStatus = new_status;
	// 40072 - 40080 are the "online" statuses, basically every status except offline.  see statusmodes.h
	if(new_status >= 40072 && new_status <= 40080)
	{

		m_iDesiredStatus = ID_STATUS_ONLINE; //i think i have to set this so it forces the twitter proto to be online (and not away, DND, etc)

		// if we're already connecting and they want to go online, BAIL!  we're already trying to connect you dumbass
		if(old_status == ID_STATUS_CONNECTING)
			return 0;

		// if we're already connected, and we change to another connected status, don't try and reconnect!
		if (old_status >= 40072 && old_status <= 40080)
			return 0;

		// i think here we tell the proto interface struct that we're connecting, just so it knows
		m_iStatus = ID_STATUS_CONNECTING;
		// ok.. here i think we're telling the core that this protocol something.. but why?
		ProtoBroadcastAck(m_szModuleName,0,ACKTYPE_STATUS,ACKRESULT_SUCCESS,
			(HANDLE)old_status,m_iStatus);

		ForkThread(&TwitterProto::SignOn,this);
	}
	else if(new_status == ID_STATUS_OFFLINE)
	{
		twit_.Disconnect();
		m_iStatus = m_iDesiredStatus;
		SetAllContactStatuses(ID_STATUS_OFFLINE);

		ProtoBroadcastAck(m_szModuleName,0,ACKTYPE_STATUS,ACKRESULT_SUCCESS,
			(HANDLE)old_status,m_iStatus);
	}

	return 0;
}

// *************************

int TwitterProto::OnEvent(PROTOEVENTTYPE event,WPARAM wParam,LPARAM lParam)
{
	switch(event)
	{
	case EV_PROTO_ONLOAD:    return OnModulesLoaded(wParam,lParam);
	case EV_PROTO_ONEXIT:    return OnPreShutdown  (wParam,lParam);
	case EV_PROTO_ONOPTIONS: return OnOptionsInit  (wParam,lParam);
	}

	return 1;
}

// *************************

int TwitterProto::SvcCreateAccMgrUI(WPARAM wParam,LPARAM lParam)
{
	return (int)CreateDialogParam(g_hInstance,MAKEINTRESOURCE(IDD_TWITTERACCOUNT), 
		 (HWND)lParam, first_run_dialog, (LPARAM)this );
}

int TwitterProto::GetName(WPARAM wParam,LPARAM lParam)
{
	lstrcpynA(reinterpret_cast<char*>(lParam),m_szProtoName,wParam);
	return 0;
}

int TwitterProto::GetStatus(WPARAM wParam,LPARAM lParam)
{
	return m_iStatus;
}

int TwitterProto::ReplyToTweet(WPARAM wParam,LPARAM lParam)
{
	// TODO: support replying to tweets instead of just users
	HANDLE hContact = reinterpret_cast<HANDLE>(wParam);

	HWND hDlg = CreateDialogParam(g_hInstance,MAKEINTRESOURCE(IDD_TWEET),
		 (HWND)0,tweet_proc,reinterpret_cast<LPARAM>(this));

	DBVARIANT dbv;
	if( !DBGetContactSettingString(hContact,m_szModuleName,TWITTER_KEY_UN,&dbv) )
	{
		SendMessage(hDlg,WM_SETREPLY,reinterpret_cast<WPARAM>(dbv.pszVal),0);
		DBFreeVariant(&dbv);
	}

	ShowWindow(hDlg,SW_SHOW);

	return 0;
}

int TwitterProto::VisitHomepage(WPARAM wParam,LPARAM lParam)
{
	HANDLE hContact = reinterpret_cast<HANDLE>(wParam);

	DBVARIANT dbv;
	if( !DBGetContactSettingString(hContact,m_szModuleName,"Homepage",&dbv) )
	{
		CallService(MS_UTILS_OPENURL,1,reinterpret_cast<LPARAM>(dbv.pszVal));
		DBFreeVariant(&dbv);
	}
	else
	{
		// TODO: remove this
		if( !DBGetContactSettingString(hContact,m_szModuleName,TWITTER_KEY_UN,&dbv) )
		{
			std::string url = profile_base_url(twit_.get_base_url())+
				http::url_encode(dbv.pszVal);
			DBWriteContactSettingString(hContact,m_szModuleName,"Homepage",url.c_str());

			CallService(MS_UTILS_OPENURL,1,reinterpret_cast<LPARAM>(url.c_str()));
			DBFreeVariant(&dbv);
		}
	}

	return 0;
}

// *************************

int TwitterProto::OnBuildStatusMenu(WPARAM wParam,LPARAM lParam)
{
	HGENMENU hRoot = pcli->pfnGetProtocolMenu(m_szModuleName);
	if (hRoot == NULL)
		return 0;

	CLISTMENUITEM mi = {sizeof(mi)};

	char text[200];
	strcpy(text,m_szModuleName);
	char *tDest = text+strlen(text);
	mi.pszService = text;

	mi.hParentMenu = hRoot;
	mi.flags = CMIF_ICONFROMICOLIB|CMIF_ROOTHANDLE;
	mi.position = 1001;

	HANDLE m_hMenuRoot = reinterpret_cast<HGENMENU>( CallService(
		MS_CLIST_ADDSTATUSMENUITEM,0,reinterpret_cast<LPARAM>(&mi)) );

	// TODO: Disable this menu item when offline
	// "Send Tweet..."
	CreateProtoService(m_szModuleName,"/Tweet",&TwitterProto::OnTweet,this);
	strcpy(tDest,"/Tweet");
	mi.pszName = LPGEN("Send Tweet...");
	mi.popupPosition = 200001;
	mi.icolibItem = GetIconHandle("tweet");
	HANDLE m_hMenuBookmarks = reinterpret_cast<HGENMENU>( CallService(
		MS_CLIST_ADDSTATUSMENUITEM,0,reinterpret_cast<LPARAM>(&mi)) );

	return 0;
}

int TwitterProto::OnOptionsInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {sizeof(odp)};
	odp.position    = 271828;
	odp.hInstance   = g_hInstance;
	odp.ptszGroup   = LPGENT("Network");
	odp.ptszTitle   = m_tszUserName;
	odp.dwInitParam = LPARAM(this);
	odp.flags       = ODPF_BOLDGROUPS | ODPF_TCHAR;

	odp.ptszTab     = LPGENT("Basic");
    odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pfnDlgProc  = options_proc;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	if(ServiceExists(MS_POPUP_ADDPOPUPT))
	{
		odp.ptszTab     = LPGENT("Popups");
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_POPUPS);
		odp.pfnDlgProc  = popup_options_proc;
		CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	}

	return 0;
}

int TwitterProto::OnTweet(WPARAM wParam,LPARAM lParam)
{
	if(m_iStatus != ID_STATUS_ONLINE)
		return 1;

	HWND hDlg = CreateDialogParam(g_hInstance,MAKEINTRESOURCE(IDD_TWEET),
		 (HWND)0,tweet_proc,reinterpret_cast<LPARAM>(this));
	ShowWindow(hDlg,SW_SHOW);
	return 0;
}

int TwitterProto::OnModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	TCHAR descr[512];
	NETLIBUSER nlu = {sizeof(nlu)};
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_TCHAR;
	nlu.szSettingsModule = m_szModuleName;

	// Create standard network connection
	mir_sntprintf(descr,SIZEOF(descr),TranslateT("%s server connection"),m_tszUserName);
	nlu.ptszDescriptiveName = descr;
	hNetlib_ = (HANDLE)CallService(MS_NETLIB_REGISTERUSER,0,(LPARAM)&nlu);
	if(hNetlib_ == 0)
		MessageBoxA(0,"Unable to get Netlib connection for Twitter","",0);

	// Create avatar network connection (TODO: probably remove this)
	char module[512];
	mir_snprintf(module,SIZEOF(module),"%sAv",m_szModuleName);
	nlu.szSettingsModule = module;
	mir_sntprintf(descr,SIZEOF(descr),TranslateT("%s avatar connection"),m_tszUserName);
	nlu.ptszDescriptiveName = descr;
	hAvatarNetlib_ = (HANDLE)CallService(MS_NETLIB_REGISTERUSER,0,(LPARAM)&nlu);
	if(hAvatarNetlib_ == 0)
		MessageBoxA(0,"Unable to get avatar Netlib connection for Twitter","",0);

	twit_.set_handle(hNetlib_);

	GCREGISTER gcr = {sizeof(gcr)};
	gcr.pszModule = m_szModuleName;
	gcr.pszModuleDispName = m_szModuleName;
	gcr.iMaxText = 159;  
	CallService(MS_GC_REGISTER,0,reinterpret_cast<LPARAM>(&gcr));

	if (ServiceExists(MS_HISTORYEVENTS_REGISTER))
	{
		HISTORY_EVENT_HANDLER heh = {0};
		heh.cbSize = sizeof(heh);
		heh.module = m_szModuleName;
		heh.name = "tweet";
		heh.description = "Tweet";
		heh.eventType = TWITTER_DB_EVENT_TYPE_TWEET;
		heh.defaultIconName = "Twitter_tweet";
		heh.flags = HISTORYEVENTS_FLAG_SHOW_IM_SRMM
					| HISTORYEVENTS_FLAG_EXPECT_CONTACT_NAME_BEFORE
// Not sure:		| HISTORYEVENTS_FLAG_FLASH_MSG_WINDOW
					| HISTORYEVENTS_REGISTERED_IN_ICOLIB;
		CallService(MS_HISTORYEVENTS_REGISTER, (WPARAM) &heh, 0);
	}
	else
	{
		DBEVENTTYPEDESCR evt = {sizeof(evt)};
		evt.eventType = TWITTER_DB_EVENT_TYPE_TWEET;
		evt.module = m_szModuleName;
		evt.descr = "Tweet";
		evt.flags = DETF_HISTORY | DETF_MSGWINDOW;
		CallService(MS_DB_EVENT_REGISTERTYPE,0,reinterpret_cast<LPARAM>(&evt));
	}

	return 0;
}

int TwitterProto::OnPreShutdown(WPARAM wParam,LPARAM lParam)
{
	Netlib_Shutdown(hNetlib_);
	Netlib_Shutdown(hAvatarNetlib_);
	return 0;
}

int TwitterProto::OnPrebuildContactMenu(WPARAM wParam,LPARAM lParam)
{
	HANDLE hContact = reinterpret_cast<HANDLE>(wParam);
	if(IsMyContact(hContact))
		ShowContactMenus(true);

	return 0;
}

int TwitterProto::ShowPinDialog()
{
	HWND hDlg = (HWND)DialogBoxParam(g_hInstance,MAKEINTRESOURCE(IDD_TWITTERPIN),
		 (HWND)0,pin_proc,reinterpret_cast<LPARAM>(this));
	ShowWindow(hDlg,SW_SHOW);
	return 0;
}

void TwitterProto::ShowPopup(const wchar_t *text, int Error)
{
	POPUPDATAT popup = {};
	_sntprintf(popup.lptzContactName,MAX_CONTACTNAME,TranslateT("%s Protocol"),m_tszUserName);
	wcs_to_tcs(CP_UTF8,text,popup.lptzText,MAX_SECONDLINE);

	if (Error) {
		popup.iSeconds = -1;
		popup.colorBack = 0x000000FF;
		popup.colorText = 0x00FFFFFF;
	}

	if(ServiceExists(MS_POPUP_ADDPOPUPT))
		CallService(MS_POPUP_ADDPOPUPT,reinterpret_cast<WPARAM>(&popup),0);
	else
		MessageBox(0,popup.lptzText,popup.lptzContactName,0);
}

void TwitterProto::ShowPopup(const char *text, int Error)
{
	POPUPDATAT popup = {};
	_sntprintf(popup.lptzContactName,MAX_CONTACTNAME,TranslateT("%s Protocol"),m_tszUserName);
	mbcs_to_tcs(CP_UTF8,text,popup.lptzText,MAX_SECONDLINE);
	if (Error) {
		popup.iSeconds = -1;
		popup.colorBack = 0x000000FF;
		popup.colorText = 0x00FFFFFF;
	}

	if(ServiceExists(MS_POPUP_ADDPOPUPT))
		CallService(MS_POPUP_ADDPOPUPT,reinterpret_cast<WPARAM>(&popup),0);
	else
		MessageBox(0,popup.lptzText,popup.lptzContactName,0);
}

int TwitterProto::LOG(const char *fmt,...)
{
	va_list va;
	char text[1024];
	if (!hNetlib_)
		return 0;

	va_start(va,fmt);
	mir_vsnprintf(text,sizeof(text),fmt,va);
	va_end(va);

	return CallService(MS_NETLIB_LOG,(WPARAM)hNetlib_,(LPARAM)text);
}

int TwitterProto::WLOG(const char* first, const wstring last)
{
	char *str1 = new char[1024*96];
	sprintf(str1,"%ls", last.c_str());

	return LOG(first, str1); 
}

// TODO: the more I think about it, the more I think all twit.* methods should
// be in MessageLoop
void TwitterProto::SendTweetWorker(void *p)
{
	if(p == 0)
		return;
	char *text = static_cast<char*>(p);



	if (strlen(text) > 140) { // looks like the chat max outgoing msg thing doesn't work, so i'll do it here.
		char * errorPopup = new char[280]; // i hate c strings ... i should use std::string here.  why did i use char* ???  need to delete[] or use std::String
		sprintf(errorPopup, "Don't be crazy! Everyone knows the max tweet size is 140, and you're trying to fit %d chars in there?", strlen(text));
		ShowPopup(errorPopup, 1);
		delete[] errorPopup;
		return;
	}

	ScopedLock s(twitter_lock_);
	twit_.set_status(text);

	mir_free(text);
}

void TwitterProto::UpdateSettings()
{
	if(db_byte_get(0,m_szModuleName,TWITTER_KEY_CHATFEED,0))
	{
		if(!in_chat_)
			OnJoinChat(0,0);
	}
	else
	{
		if(in_chat_)
			OnLeaveChat(0,0);

		for(HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
			hContact;
			hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0) )
		{
			if(!IsMyContact(hContact,true))
				continue;

			if(db_byte_get(hContact,m_szModuleName,"ChatRoom",0))
				CallService(MS_DB_CONTACT_DELETE,reinterpret_cast<WPARAM>(hContact),0);
		}
	}
}

std::string TwitterProto::GetAvatarFolder()
{
	char path[MAX_PATH];
	if(hAvatarFolder_ && FoldersGetCustomPath(hAvatarFolder_,path,sizeof(path), "") == 0)
		return path;
	else
		return def_avatar_folder_;
}

int TwitterProto::GetAvatar(WPARAM,LPARAM)
{
	return 0;
}

int TwitterProto::SetAvatar(WPARAM,LPARAM)
{
	return 0;
}