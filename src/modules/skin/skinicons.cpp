/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
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
#include "commonheaders.h"
#include <io.h>

struct StandardIconDescription
{
	int    id;
	const char*  description;
	int    resource_id;
	int    pf2;
	const char*  section;
};

static const struct StandardIconDescription mainIcons[] =
{
	{ SKINICON_OTHER_MIRANDA,         LPGEN("Miranda IM"),      -IDI_MIRANDA        },
	{ SKINICON_EVENT_MESSAGE,         LPGEN("Message"),         -IDI_RECVMSG        },
	{ SKINICON_EVENT_URL,             LPGEN("URL"),             -IDI_URL            },
	{ SKINICON_EVENT_FILE,            LPGEN("File"),            -IDI_FILE           },
	{ SKINICON_OTHER_USERONLINE,      LPGEN("User Online"),     -IDI_USERONLINE     },
	{ SKINICON_OTHER_GROUPOPEN,       LPGEN("Group (Open)"),    -IDI_GROUPOPEN      },
	{ SKINICON_OTHER_GROUPSHUT,       LPGEN("Group (Closed)"),  -IDI_GROUPSHUT      },
	{ SKINICON_OTHER_CONNECTING,      LPGEN("Connecting"),      -IDI_LOAD           },
	{ SKINICON_OTHER_ADDCONTACT,      LPGEN("Add Contact"),     -IDI_ADDCONTACT     },
	{ SKINICON_OTHER_USERDETAILS,     LPGEN("User Details"),    -IDI_USERDETAILS    },
	{ SKINICON_OTHER_HISTORY,         LPGEN("History"),         -IDI_HISTORY        },
	{ SKINICON_OTHER_DOWNARROW,       LPGEN("Down Arrow"),      -IDI_DOWNARROW      },
	{ SKINICON_OTHER_FINDUSER,        LPGEN("Find User"),       -IDI_FINDUSER       },
	{ SKINICON_OTHER_OPTIONS,         LPGEN("Options"),         -IDI_OPTIONS        },
	{ SKINICON_OTHER_SENDEMAIL,       LPGEN("Send E-mail"),     -IDI_SENDEMAIL      },
	{ SKINICON_OTHER_DELETE,          LPGEN("Delete"),          -IDI_DELETE         },
	{ SKINICON_OTHER_RENAME,          LPGEN("Rename"),          -IDI_RENAME         },
	{ SKINICON_OTHER_SMS,             LPGEN("SMS"),             -IDI_SMS            },
	{ SKINICON_OTHER_SEARCHALL,       LPGEN("Search All"),      -IDI_SEARCHALL      },
	{ SKINICON_OTHER_TICK,            LPGEN("Tick"),            -IDI_TICK           },
	{ SKINICON_OTHER_NOTICK,          LPGEN("No Tick"),         -IDI_NOTICK         },
	{ SKINICON_OTHER_HELP,            LPGEN("Help"),            -IDI_HELP           },
	{ SKINICON_OTHER_MIRANDAWEB,      LPGEN("Miranda Website"), -IDI_MIRANDAWEBSITE },
	{ SKINICON_OTHER_TYPING,          LPGEN("Typing"),          -IDI_TYPING         },
	{ SKINICON_OTHER_SMALLDOT,        LPGEN("Small Dot"),       -IDI_SMALLDOT       },
	{ SKINICON_OTHER_FILLEDBLOB,      LPGEN("Filled Blob"),     -IDI_FILLEDBLOB     },
	{ SKINICON_OTHER_EMPTYBLOB,       LPGEN("Empty Blob"),      -IDI_EMPTYBLOB      },
	{ SKINICON_OTHER_UNICODE,         LPGEN("Unicode plugin"),  -IDI_UNICODE        },
	{ SKINICON_OTHER_ANSI,            LPGEN("ANSI plugin"),     -IDI_ANSI           },
	{ SKINICON_OTHER_LOADED,          LPGEN("Running plugin"),  -IDI_LOADED         },
	{ SKINICON_OTHER_NOTLOADED,       LPGEN("Unloaded plugin"), -IDI_NOTLOADED      },
	{ SKINICON_OTHER_UNDO,	          LPGEN("Undo"),            -IDI_UNDO           },
	{ SKINICON_OTHER_WINDOW,          LPGEN("Window"),          -IDI_WINDOW         },
	{ SKINICON_OTHER_WINDOWS,         LPGEN("System"),          -IDI_WINDOWS        },
	{ SKINICON_OTHER_ACCMGR,          LPGEN("Accounts"),        -IDI_ACCMGR         },
	{ SKINICON_OTHER_SHOWHIDE,        LPGEN("ShowHide"),        -IDI_SHOWHIDE       },
	{ SKINICON_OTHER_EXIT,            LPGEN("Exit"),            -IDI_EXIT           },
	{ SKINICON_OTHER_MAINMENU,        LPGEN("Main Menu"),       -IDI_MIRANDA        },
	{ SKINICON_OTHER_STATUS,          LPGEN("Status"),          -IDI_ONLINE         },
	{ SKINICON_CHAT_JOIN,             LPGEN("Join chat"),       -IDI_JOINCHAT       },
	{ SKINICON_CHAT_LEAVE,            LPGEN("Leave chat"),      -IDI_LEAVECHAT      },
	{ SKINICON_OTHER_GROUP,			  LPGEN("Move to Group"),   -IDI_MOVETOGROUP    },
	{ SKINICON_OTHER_ON,			  LPGEN("On"),              -IDI_ON             },
	{ SKINICON_OTHER_OFF,			  LPGEN("Off"),             -IDI_OFF            },
	{ SKINICON_OTHER_STATUS_LOCKED,   LPGEN("Locked status"),   -IDI_STATUS_LOCKED, 0, "Status Icons" },
};

HANDLE hMainIcons[SIZEOF(mainIcons)];

static const struct StandardIconDescription statusIcons[] =
{
	{ ID_STATUS_OFFLINE,         LPGEN("Offline"),          -IDI_OFFLINE,       0xFFFFFFFF     },
	{ ID_STATUS_ONLINE,          LPGEN("Online"),           -IDI_ONLINE,        PF2_ONLINE     },
	{ ID_STATUS_AWAY,            LPGEN("Away"),             -IDI_AWAY,          PF2_SHORTAWAY  },
	{ ID_STATUS_NA,              LPGEN("NA"),               -IDI_NA,            PF2_LONGAWAY   },
	{ ID_STATUS_OCCUPIED,        LPGEN("Occupied"),         -IDI_OCCUPIED,      PF2_LIGHTDND   },
	{ ID_STATUS_DND,             LPGEN("DND"),              -IDI_DND,           PF2_HEAVYDND   },
	{ ID_STATUS_FREECHAT,        LPGEN("Free for chat"),    -IDI_FREE4CHAT,     PF2_FREECHAT   },
	{ ID_STATUS_INVISIBLE,       LPGEN("Invisible"),        -IDI_INVISIBLE,     PF2_INVISIBLE  },
	{ ID_STATUS_ONTHEPHONE,      LPGEN("On the phone"),     -IDI_ONTHEPHONE,    PF2_ONTHEPHONE },
	{ ID_STATUS_OUTTOLUNCH,      LPGEN("Out to lunch"),     -IDI_OUTTOLUNCH,    PF2_OUTTOLUNCH }
};

HANDLE hStatusIcons[SIZEOF(statusIcons)];

const char* mainIconsFmt   = "core_main_";
const char* statusIconsFmt = "core_status_";
const char* protoIconsFmt  = LPGEN("%s Icons");

#define PROTOCOLS_PREFIX "Status Icons/"
#define GLOBAL_PROTO_NAME "*"




// load small icon (shared) it's not need to be destroyed

static HICON LoadSmallIconShared(HINSTANCE hInstance, LPCTSTR lpIconName)
{
	int cx = GetSystemMetrics(SM_CXSMICON);
	return ( HICON )LoadImage( hInstance, lpIconName, IMAGE_ICON,cx, cx, LR_DEFAULTCOLOR | LR_SHARED );
}

// load small icon (not shared) it IS NEED to be destroyed
static HICON LoadSmallIcon(HINSTANCE hInstance, LPCTSTR lpIconName)
{
	HICON hIcon = NULL;				  // icon handle
	int index = -( int )lpIconName;
	TCHAR filename[MAX_PATH] = {0};
	GetModuleFileName( hInstance, filename, MAX_PATH );
	ExtractIconEx( filename, index, NULL, &hIcon, 1 );
	return hIcon;
}

// load small icon from hInstance
HICON LoadIconEx(HINSTANCE hInstance, LPCTSTR lpIconName, BOOL bShared)
{
	HICON hResIcon = bShared ? LoadSmallIcon(hInstance,lpIconName) : LoadSmallIconShared(hInstance,lpIconName);
	if ( !hResIcon ) { //Icon not found in hInstance lets try to load it from core
		HINSTANCE hCoreInstance=hMirandaInst;
		if ( hCoreInstance != hInstance )
			hResIcon = bShared ? LoadSmallIcon(hCoreInstance,lpIconName) : LoadSmallIconShared(hCoreInstance,lpIconName);
	}
	return hResIcon;
}

int ImageList_AddIcon_NotShared(HIMAGELIST hIml, LPCTSTR szResource)
{
	HICON hTempIcon=LoadIconEx( hMirandaInst, szResource, 0);
	int res = ImageList_AddIcon(hIml, hTempIcon);
	Safe_DestroyIcon(hTempIcon);
	return res;
}

int ImageList_AddIcon_IconLibLoaded(HIMAGELIST hIml, int iconId)
{
	HICON hIcon = LoadSkinIcon( iconId );
	int res = ImageList_AddIcon(hIml, hIcon);
	IconLib_ReleaseIcon(hIcon,0);
	return res;
}

int ImageList_AddIcon_ProtoIconLibLoaded(HIMAGELIST hIml, const char* szProto, int iconId)
{
	HICON hIcon = LoadSkinProtoIcon( szProto, iconId );
	int res = ImageList_AddIcon(hIml, hIcon);
	IconLib_ReleaseIcon(hIcon,0);
	return res;
}

int ImageList_ReplaceIcon_NotShared(HIMAGELIST hIml, int iIndex, HINSTANCE hInstance, LPCTSTR szResource)
{
	HICON hTempIcon = LoadIconEx(hInstance, szResource, 0);
	int res = ImageList_ReplaceIcon(hIml, iIndex, hTempIcon);
	Safe_DestroyIcon(hTempIcon);
	return res;
}

int ImageList_ReplaceIcon_IconLibLoaded(HIMAGELIST hIml, int nIndex, HICON hIcon)
{
	int res = ImageList_ReplaceIcon(hIml,nIndex, hIcon);
	IconLib_ReleaseIcon(hIcon,0);
	return res;
}

void Window_SetIcon_IcoLib(HWND hWnd, int iconId)
{
	SendMessage(hWnd, WM_SETICON, ICON_BIG,   ( LPARAM )LoadSkinIcon( iconId, true ));
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, ( LPARAM )LoadSkinIcon( iconId ));
}

void Window_SetProtoIcon_IcoLib(HWND hWnd, const char* szProto, int iconId)
{
	SendMessage(hWnd, WM_SETICON, ICON_BIG,   ( LPARAM )LoadSkinProtoIcon( szProto, iconId, true ));
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, ( LPARAM )LoadSkinProtoIcon( szProto, iconId ));
}

void Window_FreeIcon_IcoLib(HWND hWnd)
{
	IconLib_ReleaseIcon(( HICON )SendMessage(hWnd, WM_SETICON, ICON_BIG, 0), NULL);
	IconLib_ReleaseIcon(( HICON )SendMessage(hWnd, WM_SETICON, ICON_SMALL, 0), NULL);
}

void Button_SetIcon_IcoLib(HWND hwndDlg, int itemId, int iconId, const char* tooltip)
{
	HWND hWnd = GetDlgItem( hwndDlg, itemId );
	SendMessage( hWnd, BM_SETIMAGE, IMAGE_ICON, ( LPARAM )LoadSkinIcon( iconId ));
	SendMessage( hWnd, BUTTONSETASFLATBTN, 0, 0 );
	SendMessage( hWnd, BUTTONADDTOOLTIP, (WPARAM)tooltip, 0);
}

void Button_FreeIcon_IcoLib(HWND hwndDlg, int itemId)
{
	HICON hIcon = ( HICON )SendDlgItemMessage(hwndDlg, itemId, BM_SETIMAGE, IMAGE_ICON, 0 );
	IconLib_ReleaseIcon(hIcon,0);
}

//
//  wParam = szProto
//  lParam = status
//
HICON LoadSkinProtoIcon( const char* szProto, int status, bool big )
{
	int i, statusIndx = -1;
	char iconName[MAX_PATH];
	HICON hIcon;
	DWORD caps2 = ( szProto == NULL ) ? ( DWORD )-1 : CallProtoService(szProto,PS_GETCAPS,PFLAGNUM_2,0);

	if ( status >= ID_STATUS_CONNECTING && status < ID_STATUS_CONNECTING+MAX_CONNECT_RETRIES ) {
		mir_snprintf( iconName, SIZEOF(iconName), "%s%d", mainIconsFmt, 7 );
		return IcoLib_GetIcon( iconName, big );
	}

	for ( i = 0; i < SIZEOF(statusIcons); i++ ) {
		if ( statusIcons[i].id == status ) {
			statusIndx = i;
			break;
	}	}

	if ( statusIndx == -1 )
		return NULL;

	if ( !szProto ) {
		// Only return a protocol specific icon if there is only one protocol
		// Otherwise return the global icon. This affects the global status menu mainly.
		if ( accounts.getCount() == 1 ) {
			HICON hIcon;

			// format: core_status_%proto%statusindex
			mir_snprintf(iconName, SIZEOF(iconName), "%s%s%d", statusIconsFmt, szProto, statusIndx);

			hIcon = IcoLib_GetIcon( iconName, big );
			if ( hIcon )
				return hIcon;
		}

		// format: core_status_%s%d
		mir_snprintf(iconName, SIZEOF(iconName), "%s%s%d", statusIconsFmt, GLOBAL_PROTO_NAME, statusIndx);
		return IcoLib_GetIcon( iconName, big );
	}

	// format: core_status_%s%d
	mir_snprintf(iconName, SIZEOF(iconName), "%s%s%d", statusIconsFmt, szProto, statusIndx);
	hIcon = IcoLib_GetIcon( iconName, big );
	if ( hIcon == NULL && ( caps2 == 0 || ( caps2 & statusIcons[statusIndx].pf2 ))) {
		PROTOACCOUNT* pa = Proto_GetAccount( szProto );
		if ( pa ) {
			TCHAR szPath[MAX_PATH], szFullPath[MAX_PATH], *str;
			SKINICONDESC sid = { 0 };

			//
			//  Queried protocol isn't in list, adding
			//
			TCHAR tszSection[MAX_PATH];
			mir_sntprintf( tszSection, SIZEOF(tszSection), _T("%s%s"), _T(PROTOCOLS_PREFIX), pa->tszAccountName );
			sid.ptszSection = tszSection;

			sid.cbSize = sizeof(sid);
			sid.flags = SIDF_ALL_TCHAR;

			GetModuleFileName( hMirandaInst, szPath, MAX_PATH );
			str = _tcsrchr( szPath, '\\' );
			if ( str != NULL )
				*str = 0;
			mir_sntprintf( szFullPath, SIZEOF(szFullPath), _T("%s\\Icons\\proto_") _T(TCHAR_STR_PARAM) _T(".dll"), szPath, pa->szProtoName );
			if ( GetFileAttributes( szFullPath ) != INVALID_FILE_ATTRIBUTES )
				sid.ptszDefaultFile = szFullPath;
			else {
				mir_sntprintf( szFullPath, SIZEOF(szFullPath), _T("%s\\Plugins\\") _T(TCHAR_STR_PARAM) _T(".dll"), szPath, szProto );
				if (( int )ExtractIconEx( szFullPath, statusIcons[i].resource_id, NULL, &hIcon, 1 ) > 0 ) {
					DestroyIcon( hIcon );
					sid.ptszDefaultFile = szFullPath;
					hIcon = NULL;
				}

				if ( sid.pszDefaultFile == NULL ) {
					if ( str != NULL )
						*str = '\\';
					sid.ptszDefaultFile = szPath;
			}	}

			//
			// Add global icons to list
			//
			{
				int lowidx, highidx;
				if ( caps2 == 0 )
					lowidx = statusIndx, highidx = statusIndx+1;
				else
					lowidx = 0, highidx = SIZEOF(statusIcons);

				for ( i = lowidx; i < highidx; i++ ) {
					if ( caps2 == 0 || ( caps2 & statusIcons[i].pf2 )) {
						// format: core_%s%d
						mir_snprintf( iconName, SIZEOF(iconName), "%s%s%d", statusIconsFmt, szProto, i );
						sid.pszName = iconName;
						sid.ptszDescription = cli.pfnGetStatusModeDescription( statusIcons[i].id, 0 );
						sid.iDefaultIndex = statusIcons[i].resource_id;
						IcoLib_AddNewIcon( &sid );
		}	}	}	}

		// format: core_status_%s%d
		mir_snprintf( iconName, SIZEOF(iconName), "%s%s%d", statusIconsFmt, szProto, statusIndx );
		hIcon = IcoLib_GetIcon( iconName, big );
		if ( hIcon )
			return hIcon;
	}

	if ( hIcon == NULL ) {
		mir_snprintf( iconName, SIZEOF(iconName), "%s%s%d", statusIconsFmt, GLOBAL_PROTO_NAME, statusIndx );
		hIcon = IcoLib_GetIcon( iconName, big );
	}

	return hIcon;
}

HANDLE GetSkinIconHandle( int idx )
{
	int i;
	for ( i = 0; i < SIZEOF(mainIcons); i++ )
		if ( idx == mainIcons[i].id )
			return hMainIcons[i];

	return NULL;
}

HICON LoadSkinIcon( int idx, bool big )
{
	//
	//  Query for global status icons
	//
	if ( idx < SKINICON_EVENT_MESSAGE ) {
		if ( idx >= SIZEOF( statusIcons ))
			return NULL;

		return LoadSkinProtoIcon( NULL, statusIcons[ idx ].id, big );
	}

	return IcoLib_GetIconByHandle( GetSkinIconHandle( idx ), big );
}

/////////////////////////////////////////////////////////////////////////////////////////
// Initializes the icon skin module

static void convertOneProtocol( char* moduleName, char* iconName )
{
	char* pm = moduleName + strlen( moduleName );
	char* pi = iconName + strlen( iconName );
	DBVARIANT dbv;
	int i;

	for ( i = 0; i < SIZEOF(statusIcons); i++ ) {
		_itoa( statusIcons[i].id, pm, 10 );

		if ( !DBGetContactSettingTString( NULL, "Icons", moduleName, &dbv ) ) {
			_itoa( i, pi, 10 );

			DBWriteContactSettingTString( NULL, "SkinIcons", iconName, dbv.ptszVal );
			DBFreeVariant( &dbv );

			DBDeleteContactSetting( NULL, "Icons", moduleName );
}	}	}

static INT_PTR sttLoadSkinIcon( WPARAM wParam, LPARAM lParam )
{
	switch (lParam)
	{
	case 0:
		return (INT_PTR)LoadSkinIcon( wParam );

	case 1:
		return (INT_PTR)GetSkinIconHandle( wParam );

	case 2:
		return (INT_PTR)LoadSkinIcon( wParam, true );
	}

	return 0;
}

static INT_PTR sttLoadSkinProtoIcon( WPARAM wParam, LPARAM lParam )
{
	return (INT_PTR)LoadSkinProtoIcon( (char*)wParam, (int)lParam, false );
}

static INT_PTR sttLoadSkinProtoIconBig( WPARAM wParam, LPARAM lParam )
{
	return (INT_PTR)LoadSkinProtoIcon( (char*)wParam, (int)lParam, true );
}

int LoadSkinIcons(void)
{
	SKINICONDESC sid;
	int i, j = 0;
	char iconName[MAX_PATH], moduleName[MAX_PATH];
    TCHAR modulePath[MAX_PATH];
	DBVARIANT dbv;

	//
	//  Perform "1st-time running import"

	for ( i = 0; i < SIZEOF(mainIcons); i++ ) {
		_itoa( mainIcons[i].id, moduleName, 10 );
		if ( DBGetContactSettingTString( NULL, "Icons", moduleName, &dbv ))
			break;

		mir_snprintf( iconName, SIZEOF(iconName), "%s%d", mainIconsFmt, i );

		DBWriteContactSettingTString( NULL, "SkinIcons", iconName, dbv.ptszVal );
		DBFreeVariant( &dbv );

		DBDeleteContactSetting( NULL, "Icons", moduleName );
	}

	for ( ;; ) {
		// get the next protocol name
		moduleName[0] = 'p';
		moduleName[1] = 0;
		_itoa( j++, moduleName+1, 100 );
		if ( DBGetContactSettingTString( NULL, "Icons", moduleName, &dbv ))
			break;

		DBDeleteContactSetting( NULL, "Icons", moduleName );

		// make old skinicons' prefix
		mir_snprintf( moduleName, SIZEOF(moduleName), TCHAR_STR_PARAM, dbv.ptszVal );
		// make IcoLib's prefix
		mir_snprintf( iconName, SIZEOF(iconName), "%s" TCHAR_STR_PARAM, statusIconsFmt, dbv.ptszVal );

		convertOneProtocol( moduleName, iconName );
		DBFreeVariant( &dbv );
	}
	moduleName[0] = 0;
	strcpy(iconName, "core_status_" GLOBAL_PROTO_NAME);
	convertOneProtocol( moduleName, iconName );

	CreateServiceFunction( MS_SKIN_LOADICON, sttLoadSkinIcon );
	CreateServiceFunction( MS_SKIN_LOADPROTOICON, sttLoadSkinProtoIcon );
	CreateServiceFunction( MS_SKIN_LOADPROTOICONBIG, sttLoadSkinProtoIconBig );

	ZeroMemory( &sid, sizeof(sid) );
	sid.cbSize = sizeof(sid);
	GetModuleFileName(NULL, modulePath, SIZEOF(modulePath));
	sid.ptszDefaultFile = modulePath;
    sid.flags = SIDF_PATH_TCHAR;
	sid.pszName = iconName;

	//
	//  Add main icons to list
	//
	for ( i = 0; i < SIZEOF(mainIcons); i++ ) {
		mir_snprintf( iconName, SIZEOF(iconName), "%s%d", mainIconsFmt, i );
		sid.pszSection = mainIcons[i].section == NULL ? "Main Icons" : (char*)mainIcons[i].section;
		sid.pszDescription = (char*)mainIcons[i].description;
		sid.iDefaultIndex = mainIcons[i].resource_id;
		hMainIcons[i] = IcoLib_AddNewIcon( &sid );
	}
	//
	// Add global icons to list
	//
	sid.pszSection = PROTOCOLS_PREFIX "Global";
	//
	// Asterisk is used, to avoid conflict with proto-plugins
	// 'coz users can't rename it to name with '*'
	for ( i = 0; i < SIZEOF(statusIcons); i++ ) {
		mir_snprintf( iconName, SIZEOF(iconName), "%s%s%d", statusIconsFmt, GLOBAL_PROTO_NAME, i );
		sid.pszName = iconName;
		sid.pszDescription = (char*)statusIcons[i].description;
		sid.iDefaultIndex = statusIcons[i].resource_id;
		hStatusIcons[i] = IcoLib_AddNewIcon( &sid );
	}
	return 0;
}
