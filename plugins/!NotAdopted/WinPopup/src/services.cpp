/*

WinPopup Protocol plugin for Miranda IM.

Copyright (C) 2004-2010 Nikolay Raspopov

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

#include "stdafx.h"

// Открывает "\\Nick" при помощи ShellExecute() ("Nick" из контакта)
// wParam=hContact
// lParam=0
// Возвращает 0
#define PS_EXPLORE		"/Explore"

// Показывает диалог для ручного добавления контакта по имени/адресу
// wParam=Position
// lParam=0
// Возвращает 0
#define PS_ADDDIALOG	"/AddDialog"

/////////////////////////////////////////////////////////////////////////////
// Сервисные функции

// WinPopup Protocol/Explore
INT_PTR Explore(WPARAM wParam, LPARAM lParam);
// WinPopup Protocol/AddDialog
INT_PTR AddDialog(WPARAM wParam, LPARAM lParam);
// WinPopup Protocol/GetCaps
INT_PTR GetCaps(WPARAM flagNum, LPARAM lParam);
// WinPopup Protocol/GetName
INT_PTR GetName(WPARAM cchName, LPARAM szName);
// WinPopup Protocol/LoadIcon
INT_PTR LoadIcon(WPARAM whichIcon, LPARAM lParam);
// WinPopup Protocol/SetStatus
INT_PTR SetStatus(WPARAM newStatus, LPARAM lParam);
// WinPopup Protocol/SetAwayMsg
INT_PTR SetAwayMsg(WPARAM status_mode, LPARAM szMessage);
// WinPopup Protocol/GetAwayMsg
INT_PTR GetAwayMsg(WPARAM wParam, LPARAM lParam);
// WinPopup Protocol/GetStatus
INT_PTR GetStatus(WPARAM wParam, LPARAM lParam);
// WinPopup Protocol/GetInfo
INT_PTR GetInfo(WPARAM flags, LPARAM lParam);
// WinPopup Protocol/RecvMessage
INT_PTR RecvMessage(WPARAM flags, LPARAM lParam);
// WinPopup Protocol/SendMsg
INT_PTR SendMsg(WPARAM flags, LPARAM lParam);
// WinPopup Protocol/BasicSearch
INT_PTR BasicSearch(WPARAM wParam, LPARAM szId);
// WinPopup Protocol/AddToList
INT_PTR AddToList(WPARAM flags, LPARAM lParam);
// WinPopup Protocol/GetAvatarCaps
INT_PTR GetAvatarCaps(WPARAM wParam, LPARAM lParam);
// WinPopup Protocol/GetAvatarInformation
INT_PTR GetAvatarInfo(WPARAM flags, LPARAM lParam);
// WinPopup Protocol/SetMyAvatar
INT_PTR SetMyAvatar(WPARAM wParam, LPARAM lParam);
// WinPopup Protocol/GetMyAvatar
INT_PTR GetMyAvatar(WPARAM wParam, LPARAM lParam);
// WinPopup Protocol/CreateAccMgrUI
INT_PTR CreateAccMgrUI(WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////
// Перехватчики

// Перехватчик ME_SYSTEM_MODULESLOADED
int __cdecl SYSTEM_MODULESLOADED(WPARAM wParam, LPARAM lParam);
// Перехватчик ME_SYSTEM_PRESHUTDOWN
int __cdecl SYSTEM_PRESHUTDOWN(WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////
// Данные плагина

int hLangpack;

const PROTOCOLDESCRIPTOR pluginPD =
{
	sizeof( PROTOCOLDESCRIPTOR ),
	modname,
	PROTOTYPE_PROTOCOL
};

static PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	modname,
	PLUGIN_MAKE_VERSION (0,0,0,18),
	"Allows you to send and receive messages over Microsoft LAN. "
	"WinPopup and Net Send replacement powered by Miranda IM.",
	"Nikolay Raspopov",
	"ryo-oh-ki@narod.ru",
	"© 2004-2010 Nikolay Raspopov",
	"http://www.cherubicsoft.com/miranda/",
	UNICODE_AWARE,
	// {DE6EE412-ACE3-45db-A329-D618FABB4291}
	{0xde6ee412, 0xace3, 0x45db, {0xa3, 0x29, 0xd6, 0x18, 0xfa, 0xbb, 0x42, 0x91}}
};

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_PROTOCOL, MIID_LAST};

HANDLE			plugin_FnGetCaps			= NULL;
HANDLE			plugin_FnGetName			= NULL;
HANDLE			plugin_FnLoadIcon			= NULL;
HANDLE			plugin_FnSetStatus			= NULL;
HANDLE			plugin_FnGetStatus			= NULL;
HANDLE			plugin_FnGetAwayMsg			= NULL;
HANDLE			plugin_FnSetAwayMsg			= NULL;
HANDLE			plugin_FnGetInfo			= NULL;
HANDLE			plugin_FnSendMsg			= NULL;
HANDLE			plugin_FnRecvMessage		= NULL;
HANDLE			plugin_FnBasicSearch		= NULL;
HANDLE			plugin_FnAddToList			= NULL;
HANDLE			plugin_FnExplore			= NULL;
HANDLE			plugin_FnAddDialog			= NULL;
HANDLE			plugin_FnGetAvatarCaps		= NULL;
HANDLE			plugin_FnGetAvatarInfo		= NULL;
HANDLE			plugin_FnSetMyAvatar		= NULL;
HANDLE			plugin_FnGetMyAvatar		= NULL;
HANDLE			plugin_FnCreateAccMgrUI		= NULL;
HANDLE			plugin_SYSTEM_MODULESLOADED	= NULL;
HANDLE			plugin_SYSTEM_PRESHUTDOWN	= NULL;
HANDLE			plugin_OPT_INITIALISE		= NULL;
HANDLE			plugin_USERINFO_INITIALISE	= NULL;

/////////////////////////////////////////////////////////////////////////////
// Динамические функции загружаемые DllLoaderом

DllLoader		pluginNetapi32 ( _T("netapi32.dll"), false);
DllLoader		pluginAdvapi32 ( _T("advapi32.dll"), false);
FuncPtrType(NET_API_STATUS (NET_API_FUNCTION *) (LMSTR, DWORD, LPBYTE*) ) fnNetWkstaGetInfo;
FuncPtrType(DWORD (NET_API_FUNCTION *) (LPVOID) ) fnNetApiBufferFree;
FuncPtrType(DWORD (NET_API_FUNCTION *) (LPCWSTR, LPCWSTR, LPCWSTR, LPBYTE, DWORD) ) fnNetMessageBufferSend;
FuncPtrType(SC_HANDLE (WINAPI *) (LPCTSTR, LPCTSTR, DWORD) ) fnOpenSCManager;
FuncPtrType(SC_HANDLE (WINAPI *) (SC_HANDLE, LPCTSTR, DWORD) ) fnOpenService;
FuncPtrType(BOOL (WINAPI *) (SC_HANDLE, DWORD, LPSERVICE_STATUS) ) fnControlService;
FuncPtrType(BOOL (WINAPI *) (SC_HANDLE, LPSERVICE_STATUS) ) fnQueryServiceStatus;
FuncPtrType(BOOL (WINAPI *) (SC_HANDLE) ) fnCloseServiceHandle;
FuncPtrType(BOOL (WINAPI *) (SC_HANDLE, DWORD, LPCTSTR*) ) fnStartService;
FuncPtrType(SC_LOCK (WINAPI *) (SC_HANDLE) ) fnLockServiceDatabase;
FuncPtrType(BOOL (WINAPI *) (SC_LOCK) ) fnUnlockServiceDatabase;
FuncPtrType(BOOL (WINAPI *) (SC_HANDLE, DWORD, DWORD, DWORD, LPCTSTR, LPCTSTR, LPDWORD, LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR) ) fnChangeServiceConfig;

#define CreateHook( hService, szName, Function ) \
	_ASSERT( hService == NULL ); \
	hService = HookEvent( szName, Function ); \
	_ASSERT ( hService != NULL ); \
	if ( ! hService ) return 1;

#define DestroyHook( hService ) \
	if ( hService ) { \
		UnhookEvent( hService ); \
		hService = NULL; \
	}

#define CreateServiceFn( hService, szName, Function ) \
	_ASSERT( hService == NULL ); \
	hService = CreateServiceFunction( modname szName, Function ); \
	_ASSERT( hService != NULL ); \
	if ( ! hService ) return 1;

#ifdef _DEBUG
	#define DestroyServiceFn( hService ) \
		if( hService ) { \
			_ASSERT( DestroyServiceFunction( hService ) == 0 ); \
			hService = NULL; \
		}
#else // _DEBUG
	#define DestroyServiceFn( hService ) \
		if( hService ) { \
			DestroyServiceFunction( hService ); \
			hService = NULL; \
		}
#endif // _DEBUG


INT_PTR Explore(WPARAM wParam, LPARAM /* lParam */)
{
	CString sNick = GetNick( (HANDLE)wParam );
	if ( pluginInstalled && ! sNick.IsEmpty() )
	{
		// Вызов UNC имени: \\Nick
		CString cmd( _T("\\\\") );
		cmd += sNick;
		ShellExecute( NULL, NULL, cmd, NULL, NULL, SW_SHOWDEFAULT );
	}
	return 0;
}

INT_PTR AddDialog (WPARAM /* wParam */, LPARAM /* lParam */)
{
	AddDialog( NULL );
	return 0;
}

INT_PTR GetCaps (WPARAM flagNum, LPARAM /* lParam */)
{
	INT_PTR nReturn = 0;
	switch ( flagNum )
	{
		case PFLAGNUM_1:
			nReturn = PF1_IM | PF1_BASICSEARCH | PF1_MODEMSG | PF1_PEER2PEER;
			break;

		case PFLAGNUM_2:
		case PFLAGNUM_3:
			nReturn = PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY |
				PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT |
				PF2_ONTHEPHONE | PF2_OUTTOLUNCH;
			break;

		case PFLAGNUM_4:
			nReturn = PF4_AVATARS;
			break;

		case PFLAGNUM_5:
			nReturn = 0;
			break;

		case PFLAG_UNIQUEIDTEXT:
			nReturn = (INT_PTR)Translate ("Computer,User,Group");
			break;

		case PFLAG_UNIQUEIDSETTING:
			nReturn = (INT_PTR)"Nick";
			break;

		case PFLAG_MAXLENOFMESSAGE:
			// HANDLE hContact = (HANDLE) lParam;
			nReturn = 0x7fffffff;
			break;
	}
	return nReturn;
}

INT_PTR GetName(WPARAM cchName, LPARAM szName)
{
	lstrcpynA( (char*) szName, modtitle, static_cast <int>( cchName ) );
	return 0;
}

INT_PTR LoadIcon(WPARAM whichIcon, LPARAM /* lParam */)
{
	UINT id;
	switch ( whichIcon & 0xFFFF )
	{
		case PLI_PROTOCOL:
			id = IDI_WINPOPUP;
			break;

		case PLI_ONLINE:
			id = IDI_ONLINE;
			break;

		case PLI_OFFLINE:
			id = IDI_OFFLINE;
			break;

		default:
			return 0;	
	}
	return (INT_PTR)LoadImage (pluginModule, MAKEINTRESOURCE (id), IMAGE_ICON,
		(whichIcon & PLIF_SMALL ? 16 : 32), (whichIcon & PLIF_SMALL ? 16 : 32), 0);
}

INT_PTR SetStatus (WPARAM newStatus, LPARAM /* lParam */)
{
	if ( pluginInstalled && pluginCurrentStatus != (int)newStatus )
	{
		LOG ("SetStatus from \"%s\" to \"%s\"",
			STATUS2TEXT(pluginCurrentStatus), STATUS2TEXT(newStatus));

		pluginRequestedStatus = newStatus;
		switch (newStatus) {
			case ID_STATUS_OFFLINE:
				GotoOffline ();
				break;
			case ID_STATUS_FREECHAT:
			case ID_STATUS_INVISIBLE:
				newStatus = ID_STATUS_ONLINE;
			case ID_STATUS_ONLINE:
			case ID_STATUS_AWAY:
			case ID_STATUS_DND:
			case ID_STATUS_NA:
			case ID_STATUS_OCCUPIED:
			case ID_STATUS_ONTHEPHONE:
			case ID_STATUS_OUTTOLUNCH:
			default:
				GotoOnline ();
				break;
		}
	}
	return 0;
}

INT_PTR SetAwayMsg (WPARAM status_mode, LPARAM szMessage)
{
	pluginStatusMessage[ status_mode ] = ( szMessage ? (LPCSTR) szMessage : "" );
	return 0;
}

INT_PTR GetAwayMsg(WPARAM /* wParam */, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA*)lParam;
	if ( pluginInstalled && ccs && ccs->hContact )
	{
		if ( ContactData* data = new ContactData )
		{
			data->hContact = ccs->hContact;
			HANDLE cookie = data->cookie = GenerateCookie();
			mir_forkthread( GetAwayMsgThread, data );
			return (INT_PTR)cookie;
		}
	}
	return 0;
}

INT_PTR GetStatus (WPARAM /* wParam */, LPARAM /* lParam */)
{
	return (INT_PTR)pluginCurrentStatus;
}

INT_PTR GetInfo (WPARAM /* flags */, LPARAM lParam)
{
	CCSDATA* ccs = (CCSDATA*)lParam;
	if ( pluginInstalled && ccs && ccs->hContact )
	{
		mir_forkthread( GetInfoThread, ccs->hContact );
		return 0;
	}
	return 1;
}

INT_PTR RecvMessage (WPARAM /* flags */, LPARAM lParam)
{
	CCSDATA* ccs = (CCSDATA*) lParam;
	if ( pluginInstalled && ccs && ccs->hContact && ccs->lParam)
	{
		PROTORECVEVENT *pre = (PROTORECVEVENT*) ccs->lParam;

		// Добавление сообщения
		db_unset (ccs->hContact, "CList", "Hidden");
		DBEVENTINFO ei = { 0 };
		ei.cbSize = sizeof (DBEVENTINFO);
		ei.szModule = modname;
		ei.timestamp = pre->timestamp;
		ei.flags = (pre->flags & PREF_CREATEREAD) ? DBEF_READ : 0u;
		ei.eventType = EVENTTYPE_MESSAGE;
		ei.cbBlob = (DWORD)( lstrlenA (pre->szMessage) + 1 );
		ei.pBlob = (PBYTE) pre->szMessage;
		CallService (MS_DB_EVENT_ADD, (WPARAM) ccs->hContact, (LPARAM) &ei);
	}
	return 0;
}

INT_PTR SendMsg(WPARAM /* flags */, LPARAM lParam)
{
	CCSDATA* ccs = (CCSDATA*) lParam;
	if ( pluginInstalled && ccs && ccs->hContact && ccs->lParam )
	{
		if ( SendMsgData* data = new SendMsgData )
		{
			data->hContact = ccs->hContact;
			data->text = (LPCSTR)ccs->lParam;
			HANDLE cookie = data->cookie = GenerateCookie();
			mir_forkthread( SendMsgThread, data );
			return (INT_PTR)cookie;
		}
	}
	return 0;
}

INT_PTR BasicSearch(WPARAM /* wParam */, LPARAM szId)
{
	if ( pluginInstalled && szId )
	{
		return pluginSearch.StartSearch( CA2T( (LPCSTR)szId ) );
	}
	return 0;
}

INT_PTR AddToList(WPARAM flags, LPARAM lParam)
{
	PROTOSEARCHRESULT* psr = (PROTOSEARCHRESULT*) lParam;
	if ( psr && psr->cbSize >= sizeof (PROTOSEARCHRESULT) && psr->nick && *psr->nick )
	{
		CString sName( psr->nick );
		CString sNotes( psr->firstName );
		return (INT_PTR)AddToListByName( sName, flags, sNotes, false, false );
	}
	return 0;
}

INT_PTR GetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	switch ( wParam )
	{
	case AF_MAXSIZE:
		if ( POINT* size = (POINT*)lParam )
		{
			size->x = 300; // -1 - не работает из-за несоответствия avs.dll стандарту
			size->y = 300; // -1 - не работает из-за несоответствия avs.dll стандарту
		}
		break;

	case AF_PROPORTION:
		return PIP_NONE;

	case AF_FORMATSUPPORTED:
		switch ( lParam )
		{
		case PA_FORMAT_UNKNOWN:
		case PA_FORMAT_PNG:
		case PA_FORMAT_JPEG:
		case PA_FORMAT_ICON:
		case PA_FORMAT_BMP:
		case PA_FORMAT_GIF:
		case PA_FORMAT_SWF:
		case PA_FORMAT_XML:
			return 1;
		}
		break;

	case AF_ENABLED:
		return 1;

	case AF_DONTNEEDDELAYS:
		return 1;

	case AF_MAXFILESIZE:
		return MAX_AVATAR_SIZE;

	case AF_DELAYAFTERFAIL:
		return 10 * 60 * 1000; // 10 минут
	}

	return 0;
}

INT_PTR GetAvatarInfo(WPARAM /*flags*/, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATION* pai = (PROTO_AVATAR_INFORMATION*)lParam;
	if ( pai && pai->cbSize >= sizeof( PROTO_AVATAR_INFORMATION ) )
	{
		if ( ContactData* data = new ContactData )
		{
			data->hContact = pai->hContact;
			/*HANDLE cookie =*/ data->cookie = GenerateCookie();
			mir_forkthread( GetAvatarInfoThread, data );
			return GAIR_WAITFOR;
		}
	}
	return GAIR_NOAVATAR;
}

INT_PTR SetMyAvatar(WPARAM /*wParam*/, LPARAM lParam)
{
	CA2T szFilename( (LPCSTR)lParam );

	TCHAR szPath[ MAX_PATH ];
	GetAvatarCache( szPath );

	if ( szFilename )
	{
		// Сохранение аватара
		lstrcat( szPath, _T("MyAvatar") );
		lstrcat( szPath, _tcsrchr( szFilename, _T('.') ) );

		if ( lstrcmpi( szPath, szFilename ) == 0 )
		{
			// Тот же самый файл - ничего не делаем
		}
		else
		{
			// Другой файл - копирование аватара к себе
			SHFILEOPSTRUCT sfo = {};
			sfo.hwnd = GetDesktopWindow();
			sfo.wFunc = FO_COPY;
			TCHAR szFrom[ MAX_PATH ] = {};
			lstrcpy( szFrom, szFilename );
			sfo.pFrom = szFrom;
			sfo.pTo = szPath;
			sfo.fFlags = FOF_ALLOWUNDO | FOF_FILESONLY | FOF_NORECURSION |
				FOF_NOCONFIRMATION;
			SHFileOperation( &sfo );
		}

		db_set_ts( NULL, modname, "AvatarFile",
			_tcsrchr( szPath, _T('\\') ) + 1 );
	}
	else
	{
		// Удаление аватара в корзину
		DBVARIANT dbv = {};
		if ( ! db_get_ts( NULL, modname, "AvatarFile", &dbv ) )
		{
			lstrcat( szPath, dbv.ptszVal );

			SHFILEOPSTRUCT sfo = {};
			sfo.hwnd = GetDesktopWindow();
			sfo.wFunc = FO_DELETE;
			sfo.pFrom = szPath;
			sfo.fFlags = FOF_ALLOWUNDO | FOF_FILESONLY | FOF_NORECURSION |
				FOF_NOCONFIRMATION;
			SHFileOperation( &sfo );

			db_free( &dbv );

			db_unset( NULL, modname, "AvatarFile" );
		}
	}

	return 0;
}

INT_PTR GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	LPSTR szFilename = (LPSTR)wParam;
	int nLength = (int)lParam;
	bool ret = false;
	
	if ( szFilename == NULL || nLength < MAX_PATH )
		return -1;

	TCHAR szPath[ MAX_PATH ];
	GetAvatarCache( szPath );

	DBVARIANT dbv = {};
	if ( ! db_get_ts( NULL, modname, "AvatarFile", &dbv ) )
	{
		lstrcat( szPath, dbv.ptszVal );

		ret = ( GetFileAttributes( szPath ) != INVALID_FILE_ATTRIBUTES );

		db_free( &dbv );
	}

	if ( ! ret )
		return -1;

	lstrcpyA( szFilename, CT2A( szPath ) );

	return 0;
}

static INT_PTR CALLBACK DlgProcCreateAccMgrUI(HWND hwndDlg, UINT Msg,
	WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	switch ( Msg )
	{
	case WM_INITDIALOG:
		{
			TranslateDialogDefault( hwndDlg );
			return TRUE;
		}
	}
	return FALSE;
}

INT_PTR CreateAccMgrUI(WPARAM /*wParam*/, LPARAM lParam)
{
	return (INT_PTR)CreateDialogParam( pluginModule, MAKEINTRESOURCE( IDD_CREATE ),
		(HWND)lParam, DlgProcCreateAccMgrUI, 0 );
}

/////////////////////////////////////////////////////////////////////////////
// Интерфейсы плагина

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfoEx);
	GetVersionEx( &pluginOS );

	if (pluginNetapi32.LoadLibrary (pluginModule))
	{
		pluginNetapi32.LoadFunc (fnNetWkstaGetInfo, "NetWkstaGetInfo");
		pluginNetapi32.LoadFunc (fnNetApiBufferFree, "NetApiBufferFree");
		pluginNetapi32.LoadFunc (fnNetMessageBufferSend, "NetMessageBufferSend");
	}

	if (pluginAdvapi32.LoadLibrary (pluginModule))
	{
		pluginAdvapi32.LoadFunc (fnOpenSCManager, "OpenSCManagerW");
		pluginAdvapi32.LoadFunc (fnOpenService, "OpenServiceW");
		pluginAdvapi32.LoadFunc (fnStartService, "StartServiceW");
		pluginAdvapi32.LoadFunc (fnChangeServiceConfig, "ChangeServiceConfigW");
		pluginAdvapi32.LoadFunc (fnControlService, "ControlService");
		pluginAdvapi32.LoadFunc (fnQueryServiceStatus, "QueryServiceStatus");
		pluginAdvapi32.LoadFunc (fnCloseServiceHandle, "CloseServiceHandle");
		pluginAdvapi32.LoadFunc (fnLockServiceDatabase, "LockServiceDatabase");
		pluginAdvapi32.LoadFunc (fnUnlockServiceDatabase, "UnlockServiceDatabase");
	}

	_ASSERT( pluginInternalState == NULL );
	pluginInternalState = CreateEvent( NULL, TRUE, TRUE, NULL );
	_ASSERT( pluginInternalState != NULL );

	INT_PTR retCallService = CallService (MS_PROTO_REGISTERMODULE, 0, (LPARAM) &pluginPD);
	_ASSERT (retCallService != CALLSERVICE_NOTFOUND);
	if ( retCallService == CALLSERVICE_NOTFOUND ) return 1;

	CreateServiceFn( plugin_FnGetCaps, PS_GETCAPS, GetCaps );
	CreateServiceFn( plugin_FnGetName, PS_GETNAME, GetName );
	CreateServiceFn( plugin_FnLoadIcon, PS_LOADICON, LoadIcon );
	CreateServiceFn( plugin_FnSetStatus, PS_SETSTATUS, SetStatus );
	CreateServiceFn( plugin_FnGetStatus, PS_GETSTATUS, GetStatus );
	CreateServiceFn( plugin_FnSetAwayMsg, PS_SETAWAYMSG, SetAwayMsg );
	CreateServiceFn( plugin_FnGetAwayMsg, PSS_GETAWAYMSG, GetAwayMsg );
	CreateServiceFn( plugin_FnGetInfo, PSS_GETINFO, GetInfo );
	CreateServiceFn( plugin_FnSendMsg, PSS_MESSAGE, SendMsg );
	CreateServiceFn( plugin_FnRecvMessage, PSR_MESSAGE, RecvMessage );
	CreateServiceFn( plugin_FnBasicSearch, PS_BASICSEARCH, BasicSearch );
	CreateServiceFn( plugin_FnAddToList, PS_ADDTOLIST, AddToList );
	CreateServiceFn( plugin_FnExplore, PS_EXPLORE, Explore );
	CreateServiceFn( plugin_FnAddDialog, PS_ADDDIALOG, AddDialog);
	CreateServiceFn( plugin_FnGetAvatarCaps, PS_GETAVATARCAPS, GetAvatarCaps );
	CreateServiceFn( plugin_FnGetAvatarInfo, PS_GETAVATARINFO, GetAvatarInfo );
	CreateServiceFn( plugin_FnSetMyAvatar, PS_SETMYAVATAR, SetMyAvatar );
	CreateServiceFn( plugin_FnGetMyAvatar, PS_GETMYAVATAR, GetMyAvatar );
	CreateServiceFn( plugin_FnCreateAccMgrUI, PS_CREATEACCMGRUI, CreateAccMgrUI );

	CreateHook( plugin_SYSTEM_MODULESLOADED, ME_SYSTEM_MODULESLOADED, SYSTEM_MODULESLOADED );
	CreateHook( plugin_SYSTEM_PRESHUTDOWN, ME_SYSTEM_PRESHUTDOWN, SYSTEM_PRESHUTDOWN );


	return 0;
}

extern "C" int __declspec(dllexport) Unload()
{
#ifdef CHAT_ENABLED
	ChatUnregister();
#endif // CHAT_ENABLED

	DestroyHook( plugin_USERINFO_INITIALISE );
	DestroyHook( plugin_OPT_INITIALISE );
	DestroyHook( plugin_SYSTEM_PRESHUTDOWN );
	DestroyHook( plugin_SYSTEM_MODULESLOADED );

	DestroyServiceFn( plugin_FnExplore );
	DestroyServiceFn( plugin_FnAddDialog );
	DestroyServiceFn( plugin_FnGetCaps );
	DestroyServiceFn( plugin_FnGetName );
	DestroyServiceFn( plugin_FnLoadIcon );
	DestroyServiceFn( plugin_FnSetStatus );
	DestroyServiceFn( plugin_FnGetStatus );
	DestroyServiceFn( plugin_FnSetAwayMsg );
	DestroyServiceFn( plugin_FnGetAwayMsg );
	DestroyServiceFn( plugin_FnGetInfo );
	DestroyServiceFn( plugin_FnSendMsg );
	DestroyServiceFn( plugin_FnRecvMessage );
	DestroyServiceFn( plugin_FnBasicSearch );
	DestroyServiceFn( plugin_FnAddToList );
	DestroyServiceFn( plugin_FnGetAvatarCaps );
	DestroyServiceFn( plugin_FnGetAvatarInfo );
	DestroyServiceFn( plugin_FnSetMyAvatar );
	DestroyServiceFn( plugin_FnGetMyAvatar );
	DestroyServiceFn( plugin_FnCreateAccMgrUI );

	if ( pluginNetLibUser != NULL )
		Netlib_CloseHandle( pluginNetLibUser );
	pluginNetLibUser = NULL;

	if ( pluginInternalState != NULL )
		CloseHandle( pluginInternalState );
	pluginInternalState = NULL;

	pluginNetapi32.FreeLibrary();
	pluginAdvapi32.FreeLibrary();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Перехватчики

int __cdecl SYSTEM_MODULESLOADED (WPARAM /* wParam */, LPARAM /* lParam */)
{
	_ASSERT (pluginInstalled == true);
	_ASSERT (pluginInitialized == false);

	_ASSERT (pluginNetLibUser == NULL);
	const NETLIBUSER nlu =
	{
		sizeof( NETLIBUSER ),
		modname,
		modname,
		0,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		0		
	};
	pluginNetLibUser = (HANDLE)CallService( MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu );
	_ASSERT (pluginNetLibUser);

	CreateHook( plugin_OPT_INITIALISE, ME_OPT_INITIALISE, OPT_INITIALISE );
	CreateHook( plugin_USERINFO_INITIALISE, ME_USERINFO_INITIALISE, USERINFO_INITIALISE );

	// Установка иконок статусов
	struct
	{
		int icon_id;
		int status;
	}
	const StatusIcons [] =
	{
		{ IDI_OFFLINE, ID_STATUS_OFFLINE },
		{ IDI_ONLINE, ID_STATUS_ONLINE },
		{ IDI_AWAY, ID_STATUS_AWAY },
		{ IDI_DND, ID_STATUS_DND },
		{ IDI_NA, ID_STATUS_NA },
		{ IDI_OCCUPIED, ID_STATUS_OCCUPIED },
		{ IDI_FREECHAT, ID_STATUS_FREECHAT },
		{ IDI_INVISIBLE, ID_STATUS_INVISIBLE },
		{ IDI_ONTHEPHONE, ID_STATUS_ONTHEPHONE },
		{ IDI_OUTTOLUNCH, ID_STATUS_OUTTOLUNCH },
		{ 0, 0 }
	};
	TCHAR path [ MAX_PATH * 2 ] = { 0 };
	TCHAR icon [ 32 ] = { 0 };
	DWORD len = GetModuleFileName (pluginModule, path, MAX_PATH);
	lstrcpy( path + len, _T(",-") );
	len += 2;
	for (int i = 0; StatusIcons[i].icon_id ; i++)
	{
		wsprintf( path + len, _T("%d"), StatusIcons[i].icon_id );
		wsprintf( icon, modname_t _T("%d"), StatusIcons[i].status );
		DBVARIANT dbv = {};
		if ( ! db_get( NULL, "Icons", CT2A( icon ), &dbv ) )
			db_free( &dbv );
		else
			db_set_ts( NULL, "Icons", CT2A( icon ), path );
	}

	// Определение имени компьютера
	DWORD iMachineNameLength = MAX_COMPUTERNAME_LENGTH + 2;
	GetComputerName(
		pluginMachineName.GetBuffer( (int)iMachineNameLength ), &iMachineNameLength );
	pluginMachineName.ReleaseBuffer();
	SetNick( NULL, pluginMachineName );

	// Определение имени пользователя
	DWORD iUserNameLength = UNLEN + 2;
	GetUserName(
		pluginUserName.GetBuffer( (int)iUserNameLength ), &iUserNameLength );
	pluginUserName.ReleaseBuffer();
	db_set_ts( NULL, modname, "User", pluginUserName );

	// Определение имени рабочей группы
	if ( pluginOS.dwPlatformId == VER_PLATFORM_WIN32_NT )
	{
		WKSTA_INFO_100* info = NULL;
		NET_API_STATUS err = fnNetWkstaGetInfo (NULL, 100, (LPBYTE*) &info);
		if (err == NERR_Success && info)
		{
			LPTSTR langroup = mir_u2t( info->wki100_langroup );
			pluginDomainName = langroup;
			fnNetApiBufferFree (info);
			mir_free( langroup );
		}
		// Альтернатива?
		// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon
		// DefaultDomainName
	}
	else
	{
		HKEY hKey = NULL;
		if (ERROR_SUCCESS == RegOpenKeyEx (HKEY_LOCAL_MACHINE,
			_T("System\\CurrentControlSet\\Services\\VxD\\VNETSUP"), 0, KEY_READ, &hKey))
		{
			DWORD type, size = MAX_PATH;
			RegQueryValueEx (hKey, _T("Workgroup"), 0, &type,
				(LPBYTE) pluginDomainName.GetBuffer (MAX_PATH + 1), &size);
			pluginDomainName.ReleaseBuffer( (int)size );
			RegCloseKey (hKey);
		}
	}
	db_set_ts (NULL, modname, "Workgroup", pluginDomainName);

	// Регистрация в Chat
#ifdef CHAT_ENABLED
	pluginChatEnabled = ChatRegister();
#endif // CHAT_ENABLED

	// Добавление меню
	CLISTMENUITEM miExplore =
	{
		sizeof( CLISTMENUITEM ),
		(LPSTR)TranslateT( "Explore" ),
		CMIF_TCHAR,
		0,
		(HICON)LoadImage( pluginModule, MAKEINTRESOURCE( IDI_EXPLORE ),
			IMAGE_ICON, 16, 16, LR_SHARED ),
		modname PS_EXPLORE,
		NULL,
		0,
		0,
		modname
	};
	Menu_AddContactMenuItem(&miExplore);

	CLISTMENUITEM miAddContact =
	{
		sizeof( CLISTMENUITEM ),
		NULL,
		CMIF_TCHAR,
		500090000,
		(HICON)LoadImage( pluginModule, MAKEINTRESOURCE( IDI_WINPOPUP ),
			IMAGE_ICON, 16, 16, LR_SHARED ),
		modname PS_ADDDIALOG,
		NULL,
		0,
		0,
		NULL
	};
	miAddContact.ptszName = (LPTSTR)TranslateT( "Add contact..." );
	miAddContact.ptszPopupName = (LPTSTR)modtitle_t;

	HANDLE hMenuItem = Menu_AddMainMenuItem(&miAddContact);
	// Замена иконки
	miAddContact.flags = CMIM_ICON;
	miAddContact.hIcon = (HICON)LoadImage( pluginModule,
		MAKEINTRESOURCE( IDI_ADD_COMPUTER ), IMAGE_ICON, 16, 16, LR_SHARED ),
	CallService( MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItem, (LPARAM)&miAddContact );

	// Разрешение на активные действия (например, переход в онлайн)
	pluginInitialized = true;

	// Восстановление статуса который мог быть проигнорирован
	SetStatus( pluginRequestedStatus, 0 );

	return 0;
}

int __cdecl SYSTEM_PRESHUTDOWN (WPARAM /* wParam */, LPARAM /* lParam */)
{
	// Запрет на активные действия (например, переход в онлайн)
	pluginInstalled = false;

	// Ожидание подтверждения останова, т.к. переход в оффлайн выполняется асинхронно
	// Вызов из потока миранды, где было создано окно
	do
	{
		MSG msg;
		while ( PeekMessage( &msg, NULL, NULL, NULL, PM_REMOVE ) )
		{
			if ( IsDialogMessage( msg.hwnd, &msg ) ) continue;
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}
	while ( MsgWaitForMultipleObjects( 1, &pluginInternalState, FALSE,
		INFINITE, QS_ALLINPUT ) == WAIT_OBJECT_0 + 1 );

	return 0;
}
