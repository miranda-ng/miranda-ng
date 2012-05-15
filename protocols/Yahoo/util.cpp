/*
 * $Id: util.cpp 13557 2011-04-09 02:26:58Z borkra $
 *
 * myYahoo Miranda Plugin 
 *
 * Authors: Gennady Feldman (aka Gena01) 
 *          Laurent Marechal (aka Peorth)
 *
 * This code is under GPL and is based on AIM, MSN and Miranda source code.
 * I want to thank Robert Rainwater and George Hazan for their code and support
 * and for answering some of my questions during development of this plugin.
 */
#include "yahoo.h"
#include <windowsx.h>
#include <m_popup.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_langpack.h>
#include <m_skin.h>
#include <m_utils.h>
#include <m_options.h>

#include "resource.h"

void CYahooProto::YCreateService( const char* szService, YServiceFunc serviceProc )
{
	char str[ 255 ];
	unsigned int len;
	
	len = snprintf(str, sizeof(str), "%s%s", m_szModuleName, szService);
	
	if (len >= sizeof(str) ) {
		DebugLog("[YCreateService] Failed Registering Service: %s. Reason: buffer too small?", szService);
		return;
	}
	
	::CreateServiceFunctionObj( str, ( MIRANDASERVICEOBJ )*( void** )&serviceProc, this );
}

void CYahooProto::YForkThread( YThreadFunc pFunc, void *param )
{
	UINT lthreadID;
	
	CloseHandle(( HANDLE )::mir_forkthreadowner(( pThreadFuncOwner ) *( void** )&pFunc, this, param, &lthreadID));
}

void CYahooProto::YHookEvent( const char* szEvent, YEventFunc handler )
{
	::HookEventObj( szEvent, ( MIRANDAHOOKOBJ )*( void** )&handler, this );
}

INT_PTR __stdcall YAHOO_CallService( const char* szSvcName, WPARAM wParam, LPARAM lParam )
{
	return CallService( szSvcName, wParam, lParam );
}

int CYahooProto::DebugLog( const char *fmt, ... )
{
	char str[ 4096 ];
	va_list vararg;
	va_start( vararg, fmt );
	
	int tBytes = _vsnprintf( str, sizeof( str ), fmt, vararg );
	if ( tBytes > 0 )
		str[ tBytes ] = 0;

	va_end( vararg );
	
	return CallService( MS_NETLIB_LOG, ( WPARAM )m_hNetlibUser, ( LPARAM )str );
}

extern HANDLE g_hNetlibUser;

int DebugLog( const char *fmt, ... )
{
	char str[ 4096 ];
	va_list vararg;
	va_start( vararg, fmt );
	
	int tBytes = _vsnprintf( str, sizeof( str ), fmt, vararg );
	if ( tBytes > 0 )
		str[ tBytes ] = 0;

	va_end( vararg );
	
	return CallService( MS_NETLIB_LOG, ( WPARAM )g_hNetlibUser, ( LPARAM )str );
}

int CYahooProto::GetByte( const char* valueName, int parDefltValue )
{
	return DBGetContactSettingByte( NULL, m_szModuleName, valueName, parDefltValue );
}

int CYahooProto::SetByte( const char* valueName, int parValue )
{
	return DBWriteContactSettingByte( NULL, m_szModuleName, valueName, parValue );
}

int CYahooProto::GetByte( HANDLE hContact, const char* valueName, int parDefltValue )
{
	return DBGetContactSettingByte( hContact, m_szModuleName, valueName, parDefltValue );
}

int CYahooProto::SetByte( HANDLE hContact, const char* valueName, int parValue )
{
	return DBWriteContactSettingByte( hContact, m_szModuleName, valueName, parValue );
}


DWORD CYahooProto::GetDword( HANDLE hContact, const char* valueName, DWORD parDefltValue )
{
	return DBGetContactSettingDword( hContact, m_szModuleName, valueName, parDefltValue );
}

DWORD CYahooProto::SetDword( const char* valueName, DWORD parValue )
{
    return DBWriteContactSettingDword( NULL, m_szModuleName, valueName, parValue);
}

DWORD CYahooProto::GetDword( const char* valueName, DWORD parDefltValue )
{
	return DBGetContactSettingDword( NULL, m_szModuleName, valueName, parDefltValue );
}

DWORD CYahooProto::SetDword( HANDLE hContact, const char* valueName, DWORD parValue )
{
    return DBWriteContactSettingDword( hContact, m_szModuleName, valueName, parValue);
}


WORD CYahooProto::SetWord( HANDLE hContact, const char* valueName, int parValue )
{
	return DBWriteContactSettingWord( hContact, m_szModuleName, valueName, parValue );
}

WORD CYahooProto::GetWord( HANDLE hContact, const char* valueName, int parDefltValue )
{
	return DBGetContactSettingWord( hContact, m_szModuleName, valueName, parDefltValue );
}

DWORD CYahooProto::Set_Protocol( HANDLE hContact, int protocol )
{
	char *s=NULL;
	
	SetWord(hContact, "yprotoid", protocol);
	
	switch (protocol) {
		case YAHOO_IM_YAHOO: s = "Yahoo"; break; /* Yahoo, nothing special here */
		case YAHOO_IM_MSN: s = "Windows Live (MSN)"; break;
		case YAHOO_IM_LCS: s = "LCS"; break;
		case YAHOO_IM_SAMETIME: s = "Lotus Sametime"; break;
	} 
	
	if (protocol != YAHOO_IM_YAHOO)
		SetString(hContact, "MirVer", s);
	
	SetString(hContact, "Transport", s);
	return 0;
}

int CYahooProto::SendBroadcast( HANDLE hContact, int type, int result, HANDLE hProcess, LPARAM lParam )
{
	ACKDATA ack;

	ZeroMemory(&ack, sizeof(ack) );

	ack.cbSize = sizeof( ACKDATA );
	ack.szModule = m_szModuleName; 
	ack.hContact = hContact;
	ack.type = type; 
	ack.result = result;
	ack.hProcess = hProcess; 
	ack.lParam = lParam;
	return YAHOO_CallService( MS_PROTO_BROADCASTACK, 0, ( LPARAM )&ack );
}

int CYahooProto::GetString( const char* name, DBVARIANT* result )
{	return DBGetContactSettingString( NULL, m_szModuleName, name, result );
}

int CYahooProto::GetString( HANDLE hContact, const char* name, DBVARIANT* result )
{	return DBGetContactSettingString( hContact, m_szModuleName, name, result );
}

void CYahooProto::SetString( const char* name, const char* value )
{	DBWriteContactSettingString(NULL, m_szModuleName, name, value );
}

void CYahooProto::SetString( HANDLE hContact, const char* name, const char* value )
{	DBWriteContactSettingString(hContact, m_szModuleName, name, value );
}

void CYahooProto::SetStringT( HANDLE hContact, const char* name, const TCHAR* value )
{	DBWriteContactSettingTString(hContact, m_szModuleName, name, value );
}

DWORD CYahooProto::SetStringUtf( HANDLE hContact, const char* valueName, const char* parValue )
{
	return DBWriteContactSettingStringUtf( hContact, m_szModuleName, valueName, parValue );
}

/////////////////////////////////////////////////////////////////////////////////////////
// Popups

static int CALLBACK PopupWindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message ) {
	case WM_COMMAND:
		DebugLog("[PopupWindowProc] WM_COMMAND");
		if ( HIWORD( wParam ) == STN_CLICKED) {
			char *szURL = (char *)PUGetPluginData( hWnd );
			if ( szURL != NULL ) 
				CallService( MS_UTILS_OPENURL, TRUE, ( LPARAM )szURL );    

			PUDeletePopUp( hWnd );
			return 0;
		}
		break;

	case WM_CONTEXTMENU:
		DebugLog("[PopupWindowProc] WM_CONTEXTMENU");
		PUDeletePopUp( hWnd ); 
		return TRUE;

	case UM_FREEPLUGINDATA:
		DebugLog("[PopupWindowProc] UM_FREEPLUGINDATA");
		{
			char *szURL = (char *)PUGetPluginData( hWnd );
			if ( szURL != NULL ) 
				free(szURL);
		}

		return TRUE;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

int CYahooProto::ShowPopup( const char* nickname, const char* msg, const char *szURL )
{
	POPUPDATAEX ppd;

	if ( !ServiceExists( MS_POPUP_ADDPOPUPEX )) 
		return 0;

	ZeroMemory(&ppd, sizeof(ppd) );
	lstrcpyA( ppd.lpzContactName, nickname );
	lstrcpyA( ppd.lpzText, msg );

	ppd.PluginWindowProc = ( WNDPROC )PopupWindowProc;

	if (szURL != NULL) {
		if (lstrcmpiA(szURL, "http://mail.yahoo.com") == 0) {
			ppd.lchIcon = LoadIconEx( "mail" );
		} else {
			ppd.lchIcon = LoadIconEx( "calendar" );
		}
		
		ppd.PluginData =  (void *) strdup( szURL );
	} else {
		ppd.lchIcon = LoadIconEx( "yahoo" );
	}
	
	DebugLog("[MS_POPUP_ADDPOPUPEX] Generating a popup for [%s] %s", nickname, msg);
	
	YAHOO_CallService( MS_POPUP_ADDPOPUPEX, (WPARAM)&ppd, 0 );
	
	return 1;
}

int CYahooProto::ShowNotification(const char *title, const char *info, DWORD flags)
{
	if (ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) {
		MIRANDASYSTRAYNOTIFY err;
		int ret;

		err.szProto = m_szModuleName;
		err.cbSize = sizeof(err);
		err.szInfoTitle = (char *)title;
		err.szInfo = (char *)info;
		err.dwInfoFlags = flags;
		err.uTimeout = 1000 * 3;
		ret = CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM) & err);

		if (ret == 0)
			return 1;
	} 

	MessageBoxA(NULL, info, title, MB_OK | MB_ICONINFORMATION);
	return 0;
}

void CYahooProto::ShowError(const char *title, const char *buff)
{
	if (GetByte( "ShowErrors", 1 )) 
		if (!ShowPopup(title, buff, NULL))
			ShowNotification(title, buff, NIIF_ERROR);
}

INT_PTR __cdecl CYahooProto::OnSettingChanged(WPARAM wParam, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *) lParam;

	if ((HANDLE) wParam == NULL)
		return 0;
	if (!m_bLoggedIn)
		return 0;

	if (!strcmp(cws->szModule, "CList")) {
		// A temporary contact has been added permanently
		if (!strcmp(cws->szSetting, "NotOnList")) {
			if (DBGetContactSettingByte((HANDLE) wParam, "CList", "Hidden", 0))
				return 0;
			if (cws->value.type == DBVT_DELETED || (cws->value.type == DBVT_BYTE && cws->value.bVal == 0)) {
				DBVARIANT dbv;

				if ( !DBGetContactSettingString( (HANDLE) wParam, m_szModuleName, YAHOO_LOGINID, &dbv )){
					DebugLog("Adding Permanently %s to list.", dbv.pszVal);
					AddBuddy(dbv.pszVal,GetWord((HANDLE) wParam, "yprotoid", 0), "miranda", NULL);
					DBFreeVariant(&dbv);
				}
			}
		}
	}else if (!strcmp(cws->szModule, m_szModuleName) && !strcmp(cws->szSetting, "ApparentMode")) {
		DBVARIANT dbv;

		DebugLog("DB Setting changed.  YAHOO user's visible setting changed.");

		if ( !DBGetContactSettingString( (HANDLE) wParam, m_szModuleName, YAHOO_LOGINID, &dbv )){
			int iAdd = (ID_STATUS_OFFLINE == DBGetContactSettingWord((HANDLE) wParam, m_szModuleName, "ApparentMode", 0));
			stealth(dbv.pszVal, iAdd);
			DBFreeVariant(&dbv);
		}
	}
	return 0;
}

char* YAHOO_GetContactName( HANDLE hContact )
{
	return ( char* )YAHOO_CallService( MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) hContact, 0 );
}

extern PLUGININFOEX pluginInfo;

/*
 * Thanks Robert for the following function. Copied from AIM plugin.
 */
void CYahooProto::YAHOO_utils_logversion()
{
    char str[256];

#ifdef YAHOO_CVSBUILD
    _snprintf(str, sizeof(str), "Yahoo v%d.%d.%d.%da (%s %s)", (pluginInfo.version >> 24) & 0xFF, (pluginInfo.version >> 16) & 0xFF,
              (pluginInfo.version >> 8) & 0xFF, pluginInfo.version & 0xFF, __DATE__, __TIME__);
#else
    _snprintf(str, sizeof(str), "Yahoo v%d.%d.%d.%d", (pluginInfo.version >> 24) & 0xFF, (pluginInfo.version >> 16) & 0xFF,
              (pluginInfo.version >> 8) & 0xFF, pluginInfo.version & 0xFF);
#endif
    DebugLog(str);
#ifdef YAHOO_CVSBUILD
    DebugLog("You are using a development version of Yahoo.  Please make sure you are using the latest version before posting bug reports.");
#endif
}

void SetButtonCheck(HWND hwndDlg, int CtrlID, BOOL bCheck)
{
	HWND hwndCtrl = GetDlgItem(hwndDlg, CtrlID);
	
	Button_SetCheck(hwndCtrl, (bCheck)?BST_CHECKED:BST_UNCHECKED);
}
