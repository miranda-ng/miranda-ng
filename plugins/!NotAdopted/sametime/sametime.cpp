// sametime.cpp: Defines the entry point for the DLL application.

#include "common.h"
#include "options.h"
#include "session.h"
#include "userlist.h"
#include "messaging.h"
#include "files.h"
#include "conference.h"
#include "utils.h"

#define FAILED_MESSAGE_HANDLE			99998

char PROTO[64];
char PROTO_GROUPS[128];

bool unicode_chat = false;
int code_page = CP_ACP;
MM_INTERFACE mmi;
UTF8_INTERFACE utfi;

HINSTANCE hInst;
PLUGINLINK *pluginLink;

HANDLE hNetlibUser = 0;

HANDLE hWindowEventHook = 0, hIdleEventHook = 0;

HANDLE mainThread;
DWORD mainThreadId;

int previous_status, current_status;

bool is_idle = false;

// plugin stuff
PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"Sametime Protocol",
	PLUGIN_MAKE_VERSION(0, 5, 9, 1),
	"Implementation of Instant Messaging for the Lotus Sametime protocol.",
	"Scott Ellis",
	"mail@scottellis.com.au",
	"© 2005 Scott Ellis",
	"http://www.scottellis.com.au/",
	UNICODE_AWARE,		//not transient
	0,		//doesn't replace anything built-in
	{ 0xf1b0ba1b, 0xc91, 0x4313, { 0x85, 0xeb, 0x22, 0x50, 0x69, 0xd4, 0x4d, 0x1 } } // {F1B0BA1B-0C91-4313-85EB-225069D44D01}
};

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

extern "C" SAMETIME_API PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	pluginInfo.cbSize = sizeof(PLUGININFO);
	return (PLUGININFO*)&pluginInfo;
}

static const MUUID interfaces[] = {MIID_PROTOCOL, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}


extern "C" SAMETIME_API PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}



// protocol related services
int GetCaps(WPARAM wParam,LPARAM lParam)
{
	int ret = 0;
    switch (wParam) {
        case PFLAGNUM_1:
			//ret = PF1_IM | PF1_BASICSEARCH | PF1_ADDSEARCHRES | PF1_MODEMSG | PF1_FILE | PF1_CHAT;
			ret = PF1_IM | PF1_BASICSEARCH | PF1_EXTSEARCHUI | PF1_ADDSEARCHRES | PF1_MODEMSG | PF1_FILE | PF1_CHAT;
			break;
        case PFLAGNUM_2:
			ret = PF2_ONLINE | PF2_SHORTAWAY | PF2_HEAVYDND | PF2_LIGHTDND;
			break;
        case PFLAGNUM_3:
			ret = PF2_ONLINE | PF2_SHORTAWAY | PF2_HEAVYDND;
            break;
        case PFLAGNUM_4:
			ret = PF4_SUPPORTTYPING;
            break;
        case PFLAGNUM_5:
			ret = PF2_LIGHTDND;
            break;
        case PFLAG_UNIQUEIDTEXT:
            ret = (int) Translate("Id");
            break;
        case PFLAG_MAXLENOFMESSAGE:
            ret = MAX_MESSAGE_SIZE;
            break;
        case PFLAG_UNIQUEIDSETTING:
            ret = (int) "stid";
            break;
    }
    return ret;
}

/** Copy the name of the protocole into lParam
* @param wParam :	max size of the name
* @param lParam :	reference to a char *, which will hold the name
*/
int GetName(WPARAM wParam,LPARAM lParam)
{
	char *name = (char *)Translate(PROTO);
	size_t size = min(strlen(name),wParam-1);	// copy only the first size bytes.
	if(strncpy((char *)lParam,name,size)==NULL)
		return 1;
	((char *)lParam)[size]='\0';
	return 0;
}

/** Loads the icon corresponding to the status
* Called by the CList when the status changes.
* @param wParam :	one of the following values : \n
					<tt>PLI_PROTOCOL | PLI_ONLINE | PLI_OFFLINE</tt>
* @return			an \c HICON in which the icon has been loaded.
*/
int LoadIcon(WPARAM wParam,LPARAM lParam) 
{
	
	UINT id;
    switch (wParam & 0xFFFF)
	{
        case PLI_PROTOCOL:
			id = IDI_ICON_PROTO;
			break;
		case PLI_ONLINE:
			id = IDI_ICON_PROTO;
			break;
		case PLI_OFFLINE:
			id = IDI_ICON_PROTO;
			break;
		default:
            return (int) (HICON) NULL;
    }

    return (int) LoadImage(hInst, MAKEINTRESOURCE(id), IMAGE_ICON,
						GetSystemMetrics(wParam & PLIF_SMALL ? SM_CXSMICON : SM_CXICON),
						GetSystemMetrics(wParam & PLIF_SMALL ? SM_CYSMICON : SM_CYICON), 0);
	return 0;
}


/** Changes the status and notifies everybody
* @param wParam :	The new mode
* @param lParam :	Allways set to 0.
*/
int SetStatus(WPARAM wParam,LPARAM lParam)
{
	if((int)wParam != ID_STATUS_OFFLINE) {
		if(current_status == ID_STATUS_OFFLINE)
			LogIn((int)wParam, hNetlibUser);
		else
			SetSessionStatus((int)wParam);
	} else if(current_status != ID_STATUS_OFFLINE && (int)wParam == ID_STATUS_OFFLINE) {
		LogOut();
	}

	return 0;
}

/** Returns the current status
*/
int GetStatus(WPARAM wParam,LPARAM lParam)
{
	return current_status;
}


//////////////////////////////////////////////////////////
/// Copied from MSN plugin - sent acks need to be from different thread :(
//////////////////////////////////////////////////////////
typedef struct tag_TFakeAckParams
{
	HANDLE	hEvent;
	HANDLE	hContact;
	LPARAM	lParam;
} TFakeAckParams;


static DWORD CALLBACK sttFakeAckInfoSuccess( LPVOID param )
{
	TFakeAckParams *tParam = ( TFakeAckParams* )param;
	WaitForSingleObject( tParam->hEvent, INFINITE );

	Sleep( 100 );
	ProtoBroadcastAck(PROTO, tParam->hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, ( HANDLE )1, 0 );

	CloseHandle( tParam->hEvent );
	free(tParam);
	return 0;
}

static DWORD CALLBACK sttFakeAckMessageSuccess( LPVOID param )
{
	TFakeAckParams *tParam = ( TFakeAckParams* )param;
	WaitForSingleObject( tParam->hEvent, INFINITE );

	Sleep( 100 );
	ProtoBroadcastAck(PROTO, tParam->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, ( HANDLE )tParam->lParam, 0 );

	CloseHandle( tParam->hEvent );
	free(tParam);
	return 0;
}

static DWORD CALLBACK sttFakeAckMessageFailed( LPVOID param )
{
	TFakeAckParams *tParam = ( TFakeAckParams* )param;
	WaitForSingleObject( tParam->hEvent, INFINITE );

	Sleep( 100 );
	ProtoBroadcastAck(PROTO, tParam->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)FAILED_MESSAGE_HANDLE, tParam->lParam);

	CloseHandle( tParam->hEvent );
	free(tParam);
	return 0;
}

int GetInfo(WPARAM wParam, LPARAM lParam) {
    CCSDATA *ccs = (CCSDATA *) lParam;
	DWORD dwThreadId;
	HANDLE hEvent;
	TFakeAckParams *tfap;

	hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	tfap = (TFakeAckParams *)malloc(sizeof(TFakeAckParams));
	tfap->hContact = ccs->hContact;
	tfap->hEvent = hEvent;

	CloseHandle( CreateThread( NULL, 0, sttFakeAckInfoSuccess, tfap, 0, &dwThreadId ));
	SetEvent( hEvent );
	
	return 0;
}

int BasicSearch(WPARAM wParam, LPARAM lParam) {
	const char *szId = (const char *)lParam;

	return (int)SearchForUser(szId);
}

int AddToList(WPARAM wParam, LPARAM lParam) {
	MYPROTOSEARCHRESULT *mpsr = (MYPROTOSEARCHRESULT *)lParam;

	return (int)AddSearchedUser(mpsr, wParam & PALF_TEMPORARY);
}

int RecvMessage(WPARAM wParam, LPARAM lParam) {
	DBEVENTINFO dbei;
	CCSDATA* ccs = (CCSDATA*)lParam;
	PROTORECVEVENT* pre = (PROTORECVEVENT*)ccs->lParam;

	DBDeleteContactSetting(ccs->hContact, "CList", "Hidden");

	ZeroMemory(&dbei, sizeof(dbei));
	dbei.cbSize = sizeof(dbei);
	dbei.szModule = PROTO;
	dbei.timestamp = pre->timestamp;
	dbei.flags = (pre->flags & PREF_CREATEREAD) ? DBEF_READ : 0;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = strlen(pre->szMessage) + 1;
	if ( pre->flags & PREF_UNICODE )
		dbei.cbBlob *= ( sizeof( wchar_t )+1 );
	dbei.pBlob = (PBYTE)pre->szMessage;

	CallService(MS_DB_EVENT_ADD, (WPARAM)ccs->hContact, (LPARAM)&dbei);
	return 0;
}

int STSendMessage(WPARAM wParam, LPARAM lParam) {
	if (lParam)
	{
		CCSDATA* ccs = (CCSDATA*)lParam;

		char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)ccs->hContact, 0);

		if(!proto || strcmp(proto, PROTO) != 0 || DBGetContactSettingWord(ccs->hContact, PROTO, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE) {
			HANDLE hEvent;
			TFakeAckParams *tfap;

			hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

			tfap = (TFakeAckParams *)malloc(sizeof(TFakeAckParams));
			tfap->hContact = ccs->hContact;
			tfap->hEvent = hEvent;
			tfap->lParam = 0;

			CloseHandle( CreateThread( NULL, 0, sttFakeAckMessageFailed, tfap, 0, 0 ));
			SetEvent( hEvent );
			return FAILED_MESSAGE_HANDLE;
		}

		int ret = 0;
		char *p = (char *)ccs->lParam;
		if(ccs->wParam & PREF_UNICODE)
			ret = (int)SendMessageToUserW(ccs->hContact, (wchar_t *)&p[strlen(p) + 1]);
		else
			ret = (int)SendMessageToUser(ccs->hContact, (char *)ccs->lParam);

		DWORD dwThreadId;
		HANDLE hEvent;
		TFakeAckParams *tfap;

		hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

		tfap = (TFakeAckParams *)malloc(sizeof(TFakeAckParams));
		tfap->hContact = ccs->hContact;
		tfap->hEvent = hEvent;
		tfap->lParam = (LPARAM)ret;

		CloseHandle( CreateThread( NULL, 0, sttFakeAckMessageSuccess, tfap, 0, &dwThreadId ));
		SetEvent( hEvent );

		return ret;
	}
	return 0;
}

int STSendMessageW(WPARAM wParam, LPARAM lParam) {
	if (lParam)
	{
		CCSDATA* ccs = (CCSDATA*)lParam;

		char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)ccs->hContact, 0);

		if(!proto || strcmp(proto, PROTO) != 0 || DBGetContactSettingWord(ccs->hContact, PROTO, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE) {
			DWORD dwThreadId;
			HANDLE hEvent;
			TFakeAckParams *tfap;

			hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

			tfap = (TFakeAckParams *)malloc(sizeof(TFakeAckParams));
			tfap->hContact = ccs->hContact;
			tfap->hEvent = hEvent;
			tfap->lParam = 0;

			CloseHandle( CreateThread( NULL, 0, sttFakeAckMessageFailed, tfap, 0, &dwThreadId ));
			SetEvent( hEvent );
			return FAILED_MESSAGE_HANDLE;
		}

		int ret = 0;
		char *p = (char *)ccs->lParam;
		ret = (int)SendMessageToUserW(ccs->hContact, (wchar_t *)&p[strlen(p) + 1]);

		DWORD dwThreadId;
		HANDLE hEvent;
		TFakeAckParams *tfap;

		hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

		tfap = (TFakeAckParams *)malloc(sizeof(TFakeAckParams));
		tfap->hContact = ccs->hContact;
		tfap->hEvent = hEvent;
		tfap->lParam = (LPARAM)ret;

		CloseHandle( CreateThread( NULL, 0, sttFakeAckMessageSuccess, tfap, 0, &dwThreadId ));
		SetEvent( hEvent );

		return ret;
	}
	return 0;
}

int SetAwayMessage(WPARAM wParam, LPARAM lParam) {
	SetSessionAwayMessage(wParam, (char *)lParam);
	return 0;
}

static DWORD CALLBACK sttRecvAway( LPVOID param )
{
	TFakeAckParams *tParam = ( TFakeAckParams* )param;
	WaitForSingleObject( tParam->hEvent, INFINITE );

	Sleep( 100 );
	UserRecvAwayMessage(tParam->hContact);

	CloseHandle( tParam->hEvent );
	free(tParam);
	return 0;
}

int GetAwayMessage(WPARAM wParam, LPARAM lParam) {
	if (lParam && current_status != ID_STATUS_OFFLINE)
	{
		CCSDATA* ccs = (CCSDATA*)lParam;

		DWORD dwThreadId;
		HANDLE hEvent;
		TFakeAckParams *tfap;

		hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

		tfap = (TFakeAckParams *)malloc(sizeof(TFakeAckParams));
		tfap->hContact = ccs->hContact;
		tfap->hEvent = hEvent;

		CloseHandle( CreateThread( NULL, 0, sttRecvAway, tfap, 0, &dwThreadId ));
		SetEvent( hEvent );

		return 1;
	}
	return 0;
}

int RecvAwayMessage(WPARAM wParam, LPARAM lParam) {
	CCSDATA* ccs = (CCSDATA*)lParam;
	PROTORECVEVENT* pre = (PROTORECVEVENT*)ccs->lParam;

	char *msg = utfi.utf8_decodecp(pre->szMessage, code_page, 0);
	ProtoBroadcastAck(PROTO, ccs->hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)msg);
	mir_free(msg);

	return 0;
}

int SendTyping(WPARAM wParam, LPARAM lParam) {
	SendTyping((HANDLE)wParam, lParam == PROTOTYPE_SELFTYPING_ON);
	return 0;
}

int RecvFile(WPARAM wParam, LPARAM lParam)
{
	DBEVENTINFO dbei;
	CCSDATA* ccs = (CCSDATA*)lParam;
	PROTORECVEVENT* pre = (PROTORECVEVENT*)ccs->lParam;
	char* szDesc;
	char* szFile;

	DBDeleteContactSetting(ccs->hContact, "CList", "Hidden");

	szFile = pre->szMessage + sizeof(DWORD);
	szDesc = szFile + strlen(szFile) + 1;

	ZeroMemory(&dbei, sizeof(dbei));
	dbei.cbSize = sizeof(dbei);
	dbei.szModule = PROTO;
	dbei.timestamp = pre->timestamp;
	dbei.flags = (pre->flags & PREF_CREATEREAD) ? DBEF_READ : 0;
	dbei.eventType = EVENTTYPE_FILE;
	dbei.cbBlob = sizeof(DWORD) + strlen(szFile) + strlen(szDesc) + 2;
	dbei.pBlob = (PBYTE)pre->szMessage;

	CallService(MS_DB_EVENT_ADD, (WPARAM)ccs->hContact, (LPARAM)&dbei);

	return 0;
}

int AllowFile(WPARAM wParam, LPARAM lParam) {
	if (lParam)
	{
		CCSDATA* ccs = (CCSDATA*)lParam;
		return (int)AcceptFileTransfer(ccs->hContact, (HANDLE)(ccs->wParam), (char *)ccs->lParam);

	}
	return 0;
}

int DenyFile(WPARAM wParam, LPARAM lParam) {
	if (lParam)
	{
		CCSDATA* ccs = (CCSDATA*)lParam;
		RejectFileTransfer((HANDLE)(ccs->wParam));
	}
	return 0;
}

int SendFiles(WPARAM wParam, LPARAM lParam) {
	if (lParam && current_status != ID_STATUS_OFFLINE)
	{
		CCSDATA* ccs = (CCSDATA*)lParam;

		if (ccs->hContact && ccs->lParam && ccs->wParam) {
			HANDLE hContact = ccs->hContact;
			char** files = (char**)ccs->lParam;
			char* pszDesc = (char*)ccs->wParam;

			if (DBGetContactSettingWord(hContact, PROTO, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE) {
				return (int)SendFilesToUser(hContact, files, pszDesc);
			}

		}
	}
	return 0; // failure
}

int CancelFile(WPARAM wParam, LPARAM lParam) {
	if (lParam)
	{
		CCSDATA* ccs = (CCSDATA*)lParam;
		CancelFileTransfer((HANDLE)(ccs->wParam));
	}
	return 0;
}

void CreatePluginServices() {
	// protocol
	CreateProtoServiceFunction(PROTO, PS_GETCAPS, GetCaps);
	CreateProtoServiceFunction(PROTO, PS_GETNAME, GetName);
	CreateProtoServiceFunction(PROTO, PS_SETSTATUS, SetStatus);
	CreateProtoServiceFunction(PROTO, PS_GETSTATUS, GetStatus);
	CreateProtoServiceFunction(PROTO, PS_LOADICON,LoadIcon);
	CreateProtoServiceFunction(PROTO, PSS_GETINFO,GetInfo);

	CreateProtoServiceFunction(PROTO, PS_BASICSEARCH,BasicSearch);
	CreateProtoServiceFunction(PROTO, PS_ADDTOLIST,AddToList);

	CreateProtoServiceFunction(PROTO, PSR_MESSAGE, RecvMessage);

	CreateProtoServiceFunction(PROTO, PSS_MESSAGE, STSendMessage);
	CreateProtoServiceFunction(PROTO, PSS_MESSAGE"W", STSendMessageW);

	CreateProtoServiceFunction(PROTO, PS_SETAWAYMSG,SetAwayMessage);
	CreateProtoServiceFunction(PROTO, PSS_GETAWAYMSG,GetAwayMessage);
	CreateProtoServiceFunction(PROTO, PSR_AWAYMSG,RecvAwayMessage);

	CreateProtoServiceFunction(PROTO, PSS_USERISTYPING, SendTyping);

	CreateProtoServiceFunction(PROTO, PSR_FILE, RecvFile);
	CreateProtoServiceFunction(PROTO, PSS_FILEALLOW, AllowFile);
	CreateProtoServiceFunction(PROTO, PSS_FILEDENY, DenyFile);
	CreateProtoServiceFunction(PROTO, PSS_FILE, SendFiles);

	CreateProtoServiceFunction(PROTO, PSS_FILECANCEL, CancelFile);

	CreateProtoServiceFunction(PROTO, PS_CREATEADVSEARCHUI, CreateSearchDialog);
	CreateProtoServiceFunction(PROTO, PS_SEARCHBYADVANCED, SearchFromDialog);
}

void DestroyProtoServiceFunction(char *szModule, char *szService) {
	char szFullServiceName[512];
	strcpy(szFullServiceName, szModule);
	strcat(szFullServiceName, szService);
	DestroyServiceFunction(szFullServiceName);
}

void DestroyPluginServices() {
	DestroyProtoServiceFunction(PROTO,PSS_USERISTYPING);

	DestroyProtoServiceFunction(PROTO, PSR_FILE);
	DestroyProtoServiceFunction(PROTO, PSS_FILEALLOW);
	DestroyProtoServiceFunction(PROTO, PSS_FILEDENY);
	DestroyProtoServiceFunction(PROTO, PSS_FILE);

	DestroyProtoServiceFunction(PROTO, PSS_FILECANCEL);

	DestroyProtoServiceFunction(PROTO, PS_BASICSEARCH);
	DestroyProtoServiceFunction(PROTO, PS_ADDTOLIST);

	DestroyProtoServiceFunction(PROTO, PSR_MESSAGE);

	DestroyProtoServiceFunction(PROTO, PSS_MESSAGE);
	DestroyProtoServiceFunction(PROTO, PSS_MESSAGE"W");

	DestroyProtoServiceFunction(PROTO, PS_SETAWAYMSG);
	DestroyProtoServiceFunction(PROTO, PSS_GETAWAYMSG);
	DestroyProtoServiceFunction(PROTO, PSR_AWAYMSG); 

	DestroyProtoServiceFunction(PROTO, PSS_USERISTYPING);

	DestroyProtoServiceFunction(PROTO, PS_GETCAPS);
	DestroyProtoServiceFunction(PROTO, PS_GETNAME);
	DestroyProtoServiceFunction(PROTO, PS_SETSTATUS);
	DestroyProtoServiceFunction(PROTO, PS_GETSTATUS);
	DestroyProtoServiceFunction(PROTO, PS_LOADICON);
	DestroyProtoServiceFunction(PROTO, PSS_GETINFO);
}

int WindowEvent(WPARAM wParam, LPARAM lParam) {
	MessageWindowEventData *mwed = (MessageWindowEventData *)lParam;

	if(DBGetContactSettingByte(mwed->hContact, PROTO, "ChatRoom", 0))
		return 0;

	if(mwed && (mwed->uType == MSG_WINDOW_EVT_CLOSING || mwed->uType == MSG_WINDOW_EVT_CLOSE))
		CloseIm(mwed->hContact);

	return 0;
}

int IdleChanged(WPARAM wParam, LPARAM lParam) {
	if(!(lParam & IDF_PRIVACY)) {
		is_idle = lParam & IDF_ISIDLE ? true : false;
		SetIdle(is_idle);
	}

	return 0;
}

int OnModulesLoaded(WPARAM wParam, LPARAM lParam) {

	// register with chat module

	GCREGISTER gcr = {0};
	gcr.cbSize = sizeof(gcr);
	gcr.pszModule = PROTO;
	gcr.pszModuleDispName = Translate(PROTO);
	gcr.dwFlags = GC_TCHAR;
	int ret = CallService(MS_GC_REGISTER, 0, (LPARAM)(GCREGISTER *) &gcr);
	if(ret == GC_REGISTER_NOUNICODE) {
		gcr.dwFlags = 0;
		ret = CallService(MS_GC_REGISTER, 0, (LPARAM)(GCREGISTER *) &gcr);
		if(ret) 
			MessageBox(0, TranslateT("You must install chat.dll to enable group chat."), TranslateT("Sametime Error"), MB_OK);
	} else {
		if(ret)
			MessageBox(0, TranslateT("You must install chat.dll to enable group chat."), TranslateT("Sametime Error"), MB_OK);
		else {
			//MessageBox(0, _T("Unicode chat"), _T("msg"), MB_OK);
			unicode_chat = true;
		}
	}
	//int ret = CallService(MS_GC_REGISTER, 0, (LPARAM)(GCREGISTER *) &gcr);
	//if(ret) MessageBox(0, TranslateT("You must install chat.dll to enable group chat."), TranslateT("Sametime Error"), MB_OK);
	
	if(ServiceExists(MS_UPDATE_REGISTER)) {
		// register with updater
		Update update = {0};
		char szVersion[16];

		update.cbSize = sizeof(Update);

		update.szComponentName = pluginInfo.shortName;
		update.pbVersion = (BYTE *)CreateVersionString(pluginInfo.version, szVersion);
		update.cpbVersion = strlen((char *)update.pbVersion);

		// these are the three lines that matter - the archive, the page containing the version string, and the text (or data) 
		// before the version that we use to locate it on the page
		// (note that if the update URL and the version URL point to standard file listing entries, the backend xml
		// data will be used to check for updates rather than the actual web page - this is not true for beta urls)
		update.szBetaUpdateURL = "http://sje.twosx.net/sametime.zip";
		update.szBetaVersionURL = "http://sje.twosx.net/ver_sametime.html";
		update.pbBetaVersionPrefix = (BYTE *)"Sametime Protocol version ";
		
		update.cpbBetaVersionPrefix = strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}

	NETLIBUSER nl_user = {0};
	nl_user.cbSize = sizeof(nl_user);
	nl_user.szSettingsModule = PROTO;
	//nl_user.flags = NUF_OUTGOING | (http_proxying_enabled ? NUF_HTTPCONNS : 0);
	nl_user.flags = NUF_OUTGOING | NUF_HTTPCONNS;
	nl_user.szDescriptiveName = PROTO;

	hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nl_user);
	
	hWindowEventHook = HookEvent(ME_MSG_WINDOWEVENT, WindowEvent);

	hIdleEventHook = HookEvent(ME_IDLE_CHANGED, IdleChanged);

	// funny logic :) ... try to avoid message boxes
	// if want baloons but no balloons, try popups
	// if want popups but no popups, try baloons
	// if, after that, you want balloons but no balloons, revert to message boxes
	if(options.err_method == ED_BAL && !ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) options.err_method = ED_POP; 
	if(options.err_method == ED_POP && !ServiceExists(MS_POPUP_SHOWMESSAGE)) options.err_method = ED_BAL;
	if(options.err_method == ED_BAL && !ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) options.err_method = ED_MB;

	InitSessionMenu();
	InitConferenceMenu();

	return 0;
}

int OnPreShutdown(WPARAM wParam, LPARAM lParam) {
	if(current_status != ID_STATUS_OFFLINE) LogOut();

	UnhookEvent(hIdleEventHook);
	UnhookEvent(hWindowEventHook);

	return 0;
}

int OnShutdown(WPARAM wParam, LPARAM lParam) {

	//DeinitUtils();
	DestroyPluginServices();

	DeinitConferenceMenu();
	DeinitSessionMenu();

	Netlib_CloseHandle(hNetlibUser);

	return 0;
}

void SetAllOffline() {
	char *proto;
	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 ), hContactNext;
	while ( hContact != NULL )
	{
		proto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact,0 );
		if ( proto && !strcmp( PROTO, proto)) {
			if(DBGetContactSettingByte(hContact, PROTO, "ChatRoom", 0)) {
				// clean up after chat plugin
				hContactNext = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
				CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
				hContact = hContactNext;
				continue;
			} else {
				DBWriteContactSettingWord( hContact, PROTO, "Status", ID_STATUS_OFFLINE);
				DBWriteContactSettingDword(hContact, PROTO, "IdleTS", 0);
			}
		}

		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	
}

void DeleteAllContacts() {
	char *proto;
	DBVARIANT dbv;
	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 ), hContactNext;
	while ( hContact != NULL )
	{
		hContactNext = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
		
		if(!DBGetContactSetting(hContact, "Protocol", "p", &dbv)) {
			//proto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact,0 );
			proto = dbv.pszVal;
			if ( proto && !strcmp( PROTO, proto)) {
				CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
			}
			DBFreeVariant(&dbv);
		}

		hContact = hContactNext;
	}	
}

bool IsUnicodeOS()
{
	OSVERSIONINFOW		os;
	memset(&os, 0, sizeof(OSVERSIONINFOW));
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
	return (GetVersionExW(&os) != 0);
}
 
extern "C" SAMETIME_API int Load(PLUGINLINK *link)
{

	pluginLink=link;

	if(!IsUnicodeOS()) {
		MessageBox(0, TranslateT("This plugin requires a Unicode capable Windows installation."), TranslateT("Sametime Error"), MB_OK | MB_ICONERROR);
		return 1;
	}

	if ( !ServiceExists( MS_DB_CONTACT_GETSETTING_STR )) {
		MessageBox( 0, TranslateT( "This plugin requires db3x plugin version 0.5.1.0 or later." ), TranslateT("Sametime Error"), MB_OK );
		return 1;
	}

	DuplicateHandle( GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &mainThread, THREAD_SET_CONTEXT, FALSE, 0 );
	mainThreadId = GetCurrentThreadId();

	previous_status = current_status = ID_STATUS_OFFLINE;

	if(ServiceExists(MS_LANGPACK_GETCODEPAGE))
		code_page = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);

	mir_getMMI(&mmi);
	mir_getUTFI(&utfi);


	if(ServiceExists(MS_DB_SETSETTINGRESIDENT)) { // 0.6+
		char buff[256];
		mir_snprintf(buff, 256, "%s/%s", PROTO, "Status");
		CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM)buff);
		mir_snprintf(buff, 256, "%s/%s", PROTO, "IdleTS");
		CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM)buff);
	}

	InitAwayMsg();
	InitCritSection();

	// Get module name from DLL file name
	{
		char* str1;
		char str2[MAX_PATH];

		GetModuleFileNameA(hInst, str2, MAX_PATH);
		str1 = strrchr(str2, '\\');
		if (str1 != NULL && strlen(str1+1) > 4) {
			strncpy(PROTO, str1+1, strlen(str1+1)-4);
			PROTO[strlen(str1+1)-3] = 0;
		}
		CharUpperA(PROTO);

		strcpy(PROTO_GROUPS, PROTO);
		strcat(PROTO_GROUPS, "_GROUPS");
	}

	PROTOCOLDESCRIPTOR pd = {0};
	pd.cbSize = sizeof(pd);
	pd.szName = PROTO;
	pd.type = PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE,0,(LPARAM)&pd);

	InitUtils();

	CreatePluginServices();

	LoadOptions();

	//DeleteAllContacts();	//!!!
	
	SetAllOffline();

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);	
	HookEvent(ME_OPT_INITIALISE, OptInit );
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);
	HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);

	return 0;
}

extern "C" SAMETIME_API int Unload(void)
{
	DeinitUtils();

	DeinitAwayMsg();
	DeinitCritSection();

	return 0;
}

