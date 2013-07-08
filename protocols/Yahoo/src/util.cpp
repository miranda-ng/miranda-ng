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

int CYahooProto::DebugLog( const char *fmt, ... )
{
	char str[ 4096 ];
	va_list vararg;
	va_start( vararg, fmt );
	
	int tBytes = _vsnprintf( str, sizeof( str ), fmt, vararg );
	if ( tBytes > 0)
		str[ tBytes ] = 0;

	va_end( vararg );
	
	return CallService(MS_NETLIB_LOG, (WPARAM)m_hNetlibUser, (LPARAM)str);
}

extern HANDLE g_hNetlibUser;

int DebugLog( const char *fmt, ... )
{
	char str[ 4096 ];
	va_list vararg;
	va_start( vararg, fmt );
	
	int tBytes = _vsnprintf( str, sizeof( str ), fmt, vararg );
	if ( tBytes > 0)
		str[ tBytes ] = 0;

	va_end( vararg );
	
	return CallService(MS_NETLIB_LOG, (WPARAM)g_hNetlibUser, (LPARAM)str);
}

int CYahooProto::GetByte( const char* valueName, int parDefltValue )
{
	return db_get_b( NULL, m_szModuleName, valueName, parDefltValue );
}

int CYahooProto::SetByte( const char* valueName, int parValue )
{
	return db_set_b( NULL, m_szModuleName, valueName, parValue );
}

int CYahooProto::GetByte( HANDLE hContact, const char* valueName, int parDefltValue )
{
	return db_get_b( hContact, m_szModuleName, valueName, parDefltValue );
}

int CYahooProto::SetByte( HANDLE hContact, const char* valueName, int parValue )
{
	return db_set_b( hContact, m_szModuleName, valueName, parValue );
}


DWORD CYahooProto::GetDword( HANDLE hContact, const char* valueName, DWORD parDefltValue )
{
	return db_get_dw( hContact, m_szModuleName, valueName, parDefltValue );
}

DWORD CYahooProto::SetDword( const char* valueName, DWORD parValue )
{
	return db_set_dw( NULL, m_szModuleName, valueName, parValue);
}

DWORD CYahooProto::GetDword( const char* valueName, DWORD parDefltValue )
{
	return db_get_dw( NULL, m_szModuleName, valueName, parDefltValue );
}

DWORD CYahooProto::SetDword( HANDLE hContact, const char* valueName, DWORD parValue )
{
	return db_set_dw( hContact, m_szModuleName, valueName, parValue);
}


WORD CYahooProto::SetWord( HANDLE hContact, const char* valueName, int parValue )
{
	return db_set_w( hContact, m_szModuleName, valueName, parValue );
}

WORD CYahooProto::GetWord( HANDLE hContact, const char* valueName, int parDefltValue )
{
	return db_get_w( hContact, m_szModuleName, valueName, parDefltValue );
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

int CYahooProto::GetString(const char* name, DBVARIANT* result)
{	return db_get_s(NULL, m_szModuleName, name, result);
}

int CYahooProto::GetString(HANDLE hContact, const char* name, DBVARIANT* result)
{	return db_get_s(hContact, m_szModuleName, name, result);
}

int CYahooProto::GetStringUtf(HANDLE hContact, const char* name, DBVARIANT* result)
{	return db_get_utf(hContact, m_szModuleName, name, result);
}

void CYahooProto::SetString(const char* name, const char* value)
{	db_set_s(NULL, m_szModuleName, name, value);
}

void CYahooProto::SetString( HANDLE hContact, const char* name, const char* value)
{	db_set_s(hContact, m_szModuleName, name, value);
}

void CYahooProto::SetStringT( HANDLE hContact, const char* name, const TCHAR* value)
{	db_set_ts(hContact, m_szModuleName, name, value);
}

DWORD CYahooProto::SetStringUtf(HANDLE hContact, const char* valueName, const char* parValue)
{	return db_set_utf(hContact, m_szModuleName, valueName, parValue);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Popups

static LRESULT CALLBACK PopupWindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch(message) {
	case WM_COMMAND:
		DebugLog("[PopupWindowProc] WM_COMMAND");
		if ( HIWORD(wParam) == STN_CLICKED) {
			char *szURL = (char*)PUGetPluginData(hWnd);
			if (szURL != NULL)
				CallService(MS_UTILS_OPENURL, TRUE, (LPARAM)szURL);

			PUDeletePopup(hWnd);
			return 0;
		}
		break;

	case WM_CONTEXTMENU:
		DebugLog("[PopupWindowProc] WM_CONTEXTMENU");
		PUDeletePopup(hWnd); 
		return TRUE;

	case UM_FREEPLUGINDATA:
		DebugLog("[PopupWindowProc] UM_FREEPLUGINDATA");
		{
			char *szURL = (char *)PUGetPluginData(hWnd);
			if (szURL != NULL) 
				free(szURL);
		}

		return TRUE;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

int CYahooProto::ShowPopup(const TCHAR* nickname, const TCHAR* msg, const char *szURL)
{
	if ( !ServiceExists(MS_POPUP_ADDPOPUPT)) 
		return 0;

	POPUPDATAT ppd = { 0 };
	ppd.PluginWindowProc = PopupWindowProc;
	lstrcpyn(ppd.lptzContactName, nickname, SIZEOF(ppd.lptzContactName));
	lstrcpyn(ppd.lptzText, msg, SIZEOF(ppd.lptzText));

	if (szURL != NULL) {
		ppd.lchIcon = LoadIconEx( !lstrcmpiA(szURL, "http://mail.yahoo.com") ? "mail" : "calendar");
		ppd.PluginData = (void*)strdup(szURL);
	}
	else ppd.lchIcon = LoadIconEx("yahoo");
	
	DebugLog("[MS_POPUP_ADDPOPUP] Generating a popup for [%S] %S", nickname, msg);
	
	PUAddPopupT(&ppd);
	return 1;
}

int CYahooProto::ShowNotification(const TCHAR *title, const TCHAR *info, DWORD flags)
{
	if (ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) {
		MIRANDASYSTRAYNOTIFY err;
		err.szProto = m_szModuleName;
		err.cbSize = sizeof(err);
		err.tszInfoTitle = (TCHAR*)title;
		err.tszInfo = (TCHAR*)info;
		err.dwInfoFlags = flags | NIIF_INTERN_UNICODE;
		err.uTimeout = 1000 * 3;
		INT_PTR ret = CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM) & err);
		if (ret == 0)
			return 1;
	} 

	MessageBox(NULL, info, title, MB_OK | MB_ICONINFORMATION);
	return 0;
}

void CYahooProto::ShowError(const TCHAR *title, const TCHAR *buff)
{
	if ( GetByte("ShowErrors", 1)) 
		if ( !ShowPopup(title, buff, NULL))
			ShowNotification(title, buff, NIIF_ERROR);
}

int __cdecl CYahooProto::OnSettingChanged(WPARAM wParam, LPARAM lParam)
{
	if (!wParam || !m_bLoggedIn)
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*) lParam;
	if ( !strcmp(cws->szSetting, "ApparentMode")) {
		DebugLog("DB Setting changed.  YAHOO user's visible setting changed.");

		DBVARIANT dbv;
		if (!GetString((HANDLE)wParam, YAHOO_LOGINID, &dbv)) {
			int iAdd = (ID_STATUS_OFFLINE == GetWord((HANDLE) wParam, "ApparentMode", 0));
			stealth(dbv.pszVal, iAdd);
			db_free(&dbv);
		}
	}
	return 0;
}

bool CYahooProto::IsMyContact(HANDLE hContact)
{
	if (!hContact)
		return false;

	char* szProto = GetContactProto(hContact);
	return szProto && !strcmp(szProto, m_szModuleName);
}

extern PLUGININFOEX pluginInfo;

/*
 * Thanks Robert for the following function. Copied from AIM plugin.
 */
void CYahooProto::YAHOO_utils_logversion()
{
    char str[256];
    _snprintf(str, sizeof(str), "Yahoo v%d.%d.%d.%d", (pluginInfo.version >> 24) & 0xFF, (pluginInfo.version >> 16) & 0xFF,
              (pluginInfo.version >> 8) & 0xFF, pluginInfo.version & 0xFF);
    DebugLog(str);
}

void SetButtonCheck(HWND hwndDlg, int CtrlID, BOOL bCheck)
{
	HWND hwndCtrl = GetDlgItem(hwndDlg, CtrlID);
	
	Button_SetCheck(hwndCtrl, (bCheck) ? BST_CHECKED : BST_UNCHECKED);
}
