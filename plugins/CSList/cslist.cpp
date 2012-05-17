/* ========================================================================

                              Custom Status List
                              __________________

  Custom Status List plugin for Miranda-IM (www.miranda-im.org)
  Follower of Custom Status History List by HANAX
  Copyright © 2006-2008 HANAX
  Copyright © 2007-2009 jarvis

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

  ============================================================================

  File name      : $URL$
  Revision       : $Rev$
  Last change on : $Date$
  Last change by : $Author$

  ============================================================================  


  DESCRIPTION:

  Offers List of your Custom Statuses.

// ======================================================================== */

#include "cslist.h"
#include "strpos.h"
#include "legacy.h"

MM_INTERFACE mmi;
LIST_INTERFACE  li;
SortedList *servicesList;

// ====[ MAIN ]===============================================================

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
	CSList::handle = hinstDLL;
	return TRUE;
}


// ====[ PLUGIN INFO ]========================================================

extern "C" __declspec( dllexport ) PLUGININFOEX* MirandaPluginInfoEx( DWORD mirandaVersion )
{
	CSList::dwMirandaVersion = mirandaVersion;
	return &pluginInfoEx;
}

// ====[ INTERFACES ]=========================================================

extern "C" __declspec( dllexport ) const MUUID* MirandaPluginInterfaces( void )
{
	return interfaces;
}


// ====[ LOADER ]=============================================================

extern "C" __declspec( dllexport ) int Load( PLUGINLINK *link )
{
	// link plugin
	pluginLink = link;
	mir_getMMI(&mmi);
	mir_getLI(&li);
	servicesList = li.List_Create(0, 10);

	// support for ComboBoxEx
	{
		INITCOMMONCONTROLSEX icc;
		icc.dwSize = sizeof(icc);
		icc.dwICC = ICC_USEREX_CLASSES;
		InitCommonControlsEx( &icc );
	}

	// set global variables
	// we need 0.8.0.9 core which introduced accounts support for protocols
	CSList::bAccountsSupported = ( CSList::dwMirandaVersion >= PLUGIN_MAKE_VERSION( 0, 8, 0, 9 ) );

	// are we running under Unicode core?
	{
		char szVer[MAX_PATH];

		CallService( MS_SYSTEM_GETVERSIONTEXT, MAX_PATH, ( LPARAM )szVer );
		_strlwr( szVer ); // make sure it is lowercase

		CSList::bUnicodeCore = ( strstr( szVer, "unicode" ) != NULL );
	}

	// TODO: Alerts with wrong combination of ANSI/Unicode Windows/core/plugin

	// create handler
	cslist = new CSList( );

	return FALSE;
}


// ====[ UNLOADER ]===========================================================

extern "C" __declspec( dllexport ) int Unload( void )
{
	for (int i = servicesList->realCount-1; i >= 0; i--)
	{
		HANDLE *hService = (HANDLE *)servicesList->items[i];
		DestroyServiceFunction(hService);
	}
	li.List_Destroy(servicesList);
	mir_free(servicesList);
	delete cslist;

	return FALSE;
}


// ====[ FUN ]================================================================


CSList::CSList( )
{
	this->mainWindow = NULL;
	this->initIcoLib( );
	this->hHookOnOptionsInit = HookEvent( ME_OPT_INITIALISE, this->initOptions );
	//this->hHookOnPluginsLoaded = HookEvent( ME_SYSTEM_MODULESLOADED, this->postLoading );
	this->hHookOnDBSettingChanged = HookEvent( ME_DB_CONTACT_SETTINGCHANGED, this->respondDBChange );
	this->hHookOnStatusMenuBuild = HookEvent( ME_CLIST_PREBUILDSTATUSMENU, this->createMenuItems );
}


CSList::~CSList( )
{
	// unhook events, destroy functions
	UnhookEvent( this->hHookOnStatusMenuBuild );
	UnhookEvent( this->hHookOnPluginsLoaded );
	UnhookEvent( this->hHookOnOptionsInit );
	UnhookEvent( this->hHookOnDBSettingChanged );
	DestroyServiceFunction( this->hServiceShowList );

	// clear IcoLib
	for ( int i = 0; i < SIZEOF(forms); i++ )
	{
		char iconName[64];
		mir_snprintf(iconName, SIZEOF(iconName), "%s_%s", __INTERNAL_NAME, forms[i].pszIconIcoLib);
		CallService( MS_SKIN2_REMOVEICON, 0, ( LPARAM )iconName );
	}
}


int CSList::postLoading( WPARAM wparam, LPARAM lparam )
{
	//cslist->registerHotkeys( );
	cslist->initIcoLib( ); // for < 0.7, but probably useless though :/
	return FALSE;
}


void CSList::registerHotkeys(char buf[200], TCHAR* accName, int Number)
{
	// 0.9+ core hotkeys service
	if ( ServiceExists( MS_HOTKEY_REGISTER ) )
	{
		HOTKEYDESC hotkey = { 0 };
		hotkey.cbSize = sizeof(hotkey);
		hotkey.dwFlags = HKD_TCHAR;
		hotkey.pszName = buf;
		hotkey.ptszDescription = accName;
		hotkey.ptszSection = _T("Custom Status List");
		hotkey.pszService = buf;
		hotkey.DefHotKey = HOTKEYCODE( HOTKEYF_CONTROL | HOTKEYF_SHIFT, '0'+Number );
		CallService( MS_HOTKEY_REGISTER, 0, ( LPARAM )&hotkey );
	}
}

int CSList::initOptions( WPARAM wparam, LPARAM lparam )
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize = sizeof(odp);
	odp.position = 955000000;
	odp.hInstance = CSList::handle;
	odp.pszTemplate = MAKEINTRESOURCEA( IDD_OPTIONS );
	odp.ptszTitle = _T( MODULENAME );
	odp.pfnDlgProc = ( DLGPROC )CSOptionsProc;
	odp.ptszGroup = _T("Status");
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	CallService( MS_OPT_ADDPAGE, wparam, ( LPARAM )&odp );
	return FALSE;
}


int CSList::respondDBChange( WPARAM wparam, LPARAM lparam )
{
	HANDLE hContact = ( HANDLE )wparam;
	DBCONTACTWRITESETTING *cws = ( DBCONTACTWRITESETTING* )lparam;

	// if user changes his UIN or JID on any account
	if (hContact == NULL && (!lstrcmpA(cws->szSetting, "UIN") || !lstrcmpA(cws->szSetting, "jid")))
	{
		cslist->rebuildMenuItems();
		return 0;
	}

	// own setting or not a status-related setting
	else if ( hContact == NULL || lstrcmpA( cws->szSetting, "Status" ) )
		return 0;

	DWORD dwEasteredUin = getDword( "EasterEggedUIN", 0x0 );
	if ( dwEasteredUin == 0x0 ) // no spyed UIN found
		return 0;

	DBVARIANT dbv = { 0 };
	DBGetContactSettingString( hContact, "Protocol", "p", &dbv );

	DWORD dwUin = DBGetContactSettingDword( hContact, dbv.pszVal, "UIN", 0x0 );

	if ( dwUin != 0x0 ) // if this is an ICQ user
		if ( dwUin == dwEasteredUin ) // if spyed UIN is equivalent to this contact
			if ( DBGetContactSettingWord( hContact, "UserOnline", "OldStatus", 0 ) == ID_STATUS_OFFLINE )
				if ( cws->value.wVal > ID_STATUS_OFFLINE )
					cslist->setStatus( IDC_CANCEL, NULL, "");

	return FALSE;
}


int CSList::createMenuItems( WPARAM wparam, LPARAM lparam )
{
	int ProtoCount = 0;

	ForAllProtocols(countProtos, &ProtoCount);

	if (ProtoCount == 0)
		return FALSE;

	ForAllProtocols(addProtoStatusMenuItem, 0);

	return FALSE;
}


void CSList::rebuildMenuItems( )
{
	CLIST_INTERFACE* pcli;
	pcli = ( CLIST_INTERFACE* )CallService( MS_CLIST_RETRIEVE_INTERFACE, 0, 0 );
	if ( pcli && pcli->version > 4 )
		pcli->pfnReloadProtoMenus( );
}


void CSList::setStatus( WORD code, StatusItem* item,  char* protoName)
{
	if ( code == IDCLOSE )
		return;

	PROTOACCOUNT* pdescr = (PROTOACCOUNT*)CallService(MS_PROTO_GETACCOUNT, 0, (LPARAM)protoName);
	if (pdescr == NULL)
		return;
	int statusToSet = -1;
	if (lstrcmpA(pdescr->szProtoName, "ICQ") == 0)
	{
		ICQ_CUSTOM_STATUS ics = {0};
		ics.cbSize = sizeof(ICQ_CUSTOM_STATUS);
		ics.flags = CSSF_MASK_STATUS | CSSF_MASK_NAME | CSSF_MASK_MESSAGE | CSSF_TCHAR;

		if (code == IDC_CANCEL)
		{
			statusToSet = 0;
			ics.ptszName = _T("");
			ics.ptszMessage = _T("");
		}
		else if (code == IDOK && item != NULL)
		{
			statusToSet = item->iIcon + 1;
			ics.ptszName = variables_parsedup(item->tszTitle, NULL, NULL);
			ics.ptszMessage = variables_parsedup(item->tszMessage, NULL, NULL);
		}
		else return;

		ics.status = &statusToSet;

		char protoService[64];
		mir_snprintf(protoService, SIZEOF(protoService), "%s%s", protoName, PS_ICQ_SETCUSTOMSTATUSEX);
		CallService(protoService, 0, (LPARAM)&ics);
	}
	if (lstrcmpA(pdescr->szProtoName, "JABBER") == 0)
	{
		JABBER_CUSTOM_STATUS ics = {0};
		ics.cbSize = sizeof(ICQ_CUSTOM_STATUS);
		ics.flags = CSSF_MASK_STATUS | CSSF_MASK_NAME | CSSF_MASK_MESSAGE | CSSF_TCHAR;

		if (code == IDC_CANCEL)
		{
			statusToSet = 0;
			ics.ptszMessage = _T("");
		}
		else if (code == IDOK && item != NULL)
		{
			statusToSet = item->iIcon + 1;
			ics.ptszMessage = variables_parsedup(item->tszMessage, NULL, NULL);
		}
		else return;

		ics.status = &statusToSet;

		char protoService[64];
		mir_snprintf(protoService, SIZEOF(protoService), "%s%s", protoName, JS_SETXSTATUSEX);
		CallService(protoService, 0, (LPARAM)&ics);
	}
}

void CSList::initIcoLib( )
{
	if ( !ServiceExists( MS_SKIN2_ADDICON ) )
		return;

	// init icons of buttons
	TCHAR tszFile[MAX_PATH];
	GetModuleFileName( CSList::handle, tszFile, MAX_PATH );

	SKINICONDESC sid = { 0 };
	sid.cbSize = sizeof(SKINICONDESC);
	sid.ptszDefaultFile = tszFile;
	sid.flags = SIDF_ALL_TCHAR;
	sid.cx = sid.cy = 16;
	sid.ptszSection = _T(MODULENAME);

	for ( int i = 0; i < SIZEOF(forms); i++ )
	{
		char szSettingName[64];
		mir_snprintf( szSettingName, SIZEOF(szSettingName), "%s_%s", __INTERNAL_NAME, forms[i].pszIconIcoLib );

		sid.pszName = szSettingName;
		sid.ptszDescription = forms[i].ptszDescr;
		sid.iDefaultIndex = -forms[i].iconNoIcoLib;
		forms[i].hIcoLibItem = ( HANDLE )CallService( MS_SKIN2_ADDICON, 0, ( LPARAM )&sid );
	}
}


INT_PTR CSList::showList(WPARAM wparam, LPARAM lparam, LPARAM param)
{
	cslist->mainWindow = new CSWindow((char*)param);
	ForkThread(&cslist->mainWindow->showWindow, cslist->mainWindow);
	return 0;
}


void CSList::closeList( HWND hwnd )
{
	ForkThread( &cslist->mainWindow->closeWindow, hwnd );
	delete cslist->mainWindow;
	cslist->mainWindow = NULL;
}


void CSList::ForAllProtocols( pForAllProtosFunc pFunc, void* arg )
{
	int protoCount;
	PROTOACCOUNT** pdesc;
	CallService( MS_PSEUDO_PROTO_PROTOACCSENUM, ( WPARAM )&protoCount, ( LPARAM )&pdesc );

	for ( int i = 0; i < protoCount; i++ )
	{
		if (lstrcmpA(pdesc[i]->szProtoName, "ICQ") == 0)
		{
			DWORD uin = 0;
			uin = DBGetContactSettingDword(NULL, pdesc[i]->szModuleName, "UIN", 0);
			if (uin == 0)
				continue;
		}
		else if (lstrcmpA(pdesc[i]->szProtoName, "JABBER") == 0)
		{
			DBVARIANT dbv = {0};
			DBGetContactSettingTString(NULL, pdesc[i]->szModuleName, "jid", &dbv);
			if (dbv.ptszVal == _T(""))
				continue;
		}
		else
			continue;
		pFunc(pdesc[i]->szModuleName , arg);
	}
}


void CSList::addProtoStatusMenuItem( char* protoName, void* arg )
{
	char buf[200];
	PROTOACCOUNT* pdescr;
	pdescr = (PROTOACCOUNT*)CallService(MS_PROTO_GETACCOUNT, 0, (LPARAM)protoName);
	if (pdescr == NULL)
		return;

	HANDLE hService;
	mir_snprintf(buf, SIZEOF(buf), "CSList/ShowList/%s", protoName);
	if (!ServiceExists(buf))
		hService = CreateServiceFunctionParam(buf, CSList::showList, (LPARAM)protoName);
	li.List_Insert(servicesList, hService, servicesList->realCount);

	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.flags = CMIF_TCHAR;
	if (ServiceExists(MS_SKIN2_GETICON))
		mi.icolibItem = (HANDLE)LoadIconExEx("icon", IDI_CSLIST);
	else
		mi.hIcon = LoadIconExEx("icon", IDI_CSLIST);
	mi.ptszName = _T(MODULENAME);
	mi.position = 2000040000;
	mi.pszService = buf;
	mi.ptszPopupName = pdescr->tszAccountName;
	CallService(MS_CLIST_ADDSTATUSMENUITEM, 0, (LPARAM)&mi);

	cslist->registerHotkeys(buf, pdescr->tszAccountName, pdescr->iOrder);
}


void CSList::countProtos( char* protoName, void* arg )
{
	int* protosEnabled = (int*)arg;
	*protosEnabled = ( *protosEnabled )++;
}


void CSList::countPlusModProtos( char* protoName, void* arg )
{
	int* protosExtraEnabled = ( int* )arg;
	if ( DBGetContactSettingDword( NULL, protoName, "NonStandartXstatus", 0 ) ||
	     DBGetContactSettingDword( NULL, protoName, "NonStandardXstatus", 0 ) )
		*protosExtraEnabled = ( *protosExtraEnabled )++;
}

void CSList::importCustomStatusUIStatusesFromAllProtos( char* protoName, void* arg )
{
	int* result = ( int* )arg;

	for ( int i = 1; i <= 37; i++ )
	{
		DBVARIANT dbv = { 0 };
		char bufTitle[32], bufMessage[32];
		StatusItem* si = new StatusItem( );

		si->iIcon = i - 1;

		mir_snprintf( bufTitle, 32, "XStatus%luName", i );
		DBGetContactSettingTString( NULL, protoName, bufTitle, &dbv );
		lstrcpy( si->tszTitle, dbv.ptszVal );

		mir_snprintf( bufMessage, 32, "XStatus%luMsg", i );
		DBGetContactSettingTString( NULL, protoName, bufMessage, &dbv );
		lstrcpy( si->tszMessage, dbv.ptszVal );

		if ( lstrlen( si->tszTitle ) || lstrlen( si->tszMessage ) )
		{
			cslist->mainWindow->itemslist->list->add( si );
			cslist->mainWindow->bSomethingChanged = TRUE;
		}
		else
			delete si;

		if ( *result == IDYES )
		{
			DBDeleteContactSetting( NULL, protoName, bufTitle );
			DBDeleteContactSetting( NULL, protoName, bufMessage );
		}
	}
	cslist->mainWindow->listview->reinitItems( cslist->mainWindow->itemslist->list->getListHead( ) );
}


CSWindow::CSWindow(char* protoName)
{
	this->protoName = protoName;
	this->handle = NULL;
	this->bExtraIcons = getByte( "AllowExtraIcons", DEFAULT_ALLOW_EXTRA_ICONS );
	this->itemslist = new CSItemsList(this->protoName);
	this->listview = NULL;
	this->addModifyDlg = NULL;
	this->bSomethingChanged = FALSE;
	this->filterString = NULL;
}


CSWindow::~CSWindow( )
{
	delete this->itemslist;
	SAFE_FREE( ( void** )&this->filterString );
}


void CSWindow::showWindow( void* arg )
{
	CSWindow* csw = ( CSWindow* )arg;

	while ( csw == NULL )
		SleepEx( 10, FALSE );

	DialogBoxParam( CSList::handle, MAKEINTRESOURCE( IDD_CSLIST ), NULL, ( DLGPROC )CSWindowProc, ( LPARAM )csw );
}


void CSWindow::closeWindow( void* arg )
{
	HWND hwnd = ( HWND )arg;
	EndDialog( hwnd, FALSE );
}


void CSWindow::initIcons()
{
	PROTOACCOUNT* pdescr = (PROTOACCOUNT*)CallService(MS_PROTO_GETACCOUNT, 0, (LPARAM)protoName);
	if (pdescr == NULL)
		return;

	if (lstrcmpA(pdescr->szProtoName, "ICQ") == 0)
	{
		DWORD uin = 0;
		uin = DBGetContactSettingDword(NULL, pdescr->szModuleName, "UIN", 0);
		if (uin != 0)
		{
			this->icons = ImageList_Create( 16, 16,  IsWinVerXPPlus( ) ? ILC_COLOR32 | ILC_MASK : ILC_COLOR16 | ILC_MASK, XSTATUS_COUNT_EXTENDED, 0 );
			if ( this->icons )
			{
				for ( int i = 0; i < ( ( this->bExtraIcons ) ? XSTATUS_COUNT_EXTENDED : XSTATUS_COUNT ); i++ )
				{
					char szTemp[MAX_PATH];
					mir_snprintf(szTemp, SIZEOF(szTemp), "%s_xstatus%d", protoName, i);
					ImageList_AddIcon( this->icons, ( HICON )CallService( MS_SKIN2_GETICON, 0, ( LPARAM )szTemp ) );
				}
			}
		}
	}
	else if (lstrcmpA(pdescr->szProtoName, "JABBER") == 0)
	{
		DBVARIANT dbv = {0};
		DBGetContactSettingTString(NULL, pdescr->szModuleName, "jid", &dbv);
		if (dbv.ptszVal != _T(""))
		{
			this->icons = ImageList_Create( 16, 16,  IsWinVerXPPlus( ) ? ILC_COLOR32 | ILC_MASK : ILC_COLOR16 | ILC_MASK, MOODS_COUNT, 0 );
			if ( this->icons )
			{
				for ( int i = 0; i < MOODS_COUNT; i++ )
				{
					char szTemp[MAX_PATH];
					mir_snprintf(szTemp, SIZEOF(szTemp), "%s_%s", protoName, moods_names[i].szTag);
					ImageList_AddIcon( this->icons, ( HICON )CallService( MS_SKIN2_GETICON, 0, ( LPARAM )szTemp ) );
				}
			}
		}
	}
}


void CSWindow::deinitIcons( )
{
	ImageList_Destroy( this->icons );
}


void CSWindow::initButtons( )
{
	for (int i = 0; i < SIZEOF(forms); i++)
	{
		if ( forms[i].idc < 0 )
			continue;

		SendDlgItemMessage( this->handle, forms[i].idc, BM_SETIMAGE, IMAGE_ICON, ( LPARAM )LoadIconExEx( forms[i].pszIconIcoLib, forms[i].iconNoIcoLib ) );
		SendDlgItemMessage( this->handle, forms[i].idc, BUTTONSETASFLATBTN, 0, 0 ); //maybe set as BUTTONSETDEFAULT?
		SendDlgItemMessage( this->handle, forms[i].idc, BUTTONADDTOOLTIP, ( WPARAM )TranslateTS(forms[i].ptszTitle), BATF_TCHAR );
	}
}

void CSWindow::loadWindowPosition( )
{
	RECT rect = { 0 };
	int width = GetSystemMetrics( SM_CXSCREEN );
	int height = GetSystemMetrics( SM_CYSCREEN );
	GetWindowRect( this->handle, &rect );
	int x, y, defX, defY;
	defX = x = ( width  + 1 - ( rect.right  - rect.left ) ) >> 1;
	defY = y = ( height + 1 - ( rect.bottom - rect.top  ) ) >> 1;
	if ( getByte( "RememberWindowPosition", DEFAULT_REMEMBER_WINDOW_POSITION ) )
	{
		x = getWord( "PositionX", defX );
		y = getWord( "PositionY", defY );
	}
	if ( x > width || y > height || x < 0 || y < 0 )
	{
		x = defX;
		y = defY;
  }

	MoveWindow( this->handle,
	            x, y,
	            ( rect.right - rect.left ),
	            ( rect.bottom - rect.top ),
	            TRUE );
}


void CSWindow::toggleEmptyListMessage( )
{
	HWND hwnd = GetDlgItem( this->handle, IDC_NO_ITEMS );
	ShowWindow( hwnd, ( ListView_GetItemCount( this->listview->handle ) > 0 ) ? FALSE : TRUE );
	SetForegroundWindow( hwnd );
	hwnd = GetDlgItem( this->handle, IDC_ADD_SAMPLE );
	ShowWindow( hwnd, ( ListView_GetItemCount( this->listview->handle ) > 0 ) ? FALSE : TRUE );
	SetForegroundWindow( hwnd );
}


BOOL CSWindow::itemPassedFilter( ListItem< StatusItem >* li )
{
	TCHAR filter[MAX_PATH];
	GetDlgItemText( this->handle, IDC_FILTER_FIELD, filter, MAX_PATH );

	if ( lstrlen( filter ) )
	{
		TCHAR title[EXTRASTATUS_TITLE_LIMIT], message[EXTRASTATUS_MESSAGE_LIMIT];
		lstrcpy( title, li->item->tszTitle ); lstrcpy( message, li->item->tszMessage );
		if ( strpos( _tcslwr( title ), _tcslwr( filter ) ) == -1 )
			if ( strpos( _tcslwr( message ), _tcslwr( filter ) ) == -1 )
				return FALSE;
	}

	return TRUE;
}


void CSWindow::toggleFilter( )
{
	HWND hFilter = GetDlgItem( this->handle, IDC_FILTER_FIELD );
	BOOL isEnabled = !IsWindowEnabled( hFilter ) ? TRUE : FALSE; // ! = + isEnabled = !isEnabled in one
	EnableWindow( hFilter, isEnabled );
	ShowWindow( hFilter, isEnabled );
	CheckDlgButton( this->handle, IDC_FILTER, isEnabled );
	SetForegroundWindow( hFilter );
	if ( isEnabled )
		SetFocus( hFilter );
	else
	{
		TCHAR filterText[255];
		GetDlgItemText(this->handle, IDC_FILTER_FIELD, filterText, SIZEOF(filterText));
		if ( lstrlen( filterText ) > 0 )
			SetDlgItemText( this->handle, IDC_FILTER_FIELD, TEXT( "" ) );
	}
}


BOOL CSWindow::toggleButtons( )
{
	int selection = ListView_GetSelectedItemMacro( this->listview->handle );
	BOOL validSelection = ( selection >= 0 && ( unsigned int )selection < this->itemslist->list->getCount( ) ) ? TRUE : FALSE;
	BOOL filterEnabled = IsWindowVisible( GetDlgItem( this->handle, IDC_FILTER_FIELD ) );
	BOOL somethingChanged = this->bSomethingChanged;

	EnableWindow( GetDlgItem( this->handle, IDC_ADD       ), !filterEnabled );
	EnableWindow( GetDlgItem( this->handle, IDC_MODIFY    ), validSelection && !filterEnabled );
	EnableWindow( GetDlgItem( this->handle, IDC_REMOVE    ), validSelection && !filterEnabled );
	EnableWindow( GetDlgItem( this->handle, IDC_FAVOURITE ), validSelection && !filterEnabled );
	EnableWindow( GetDlgItem( this->handle, IDC_UNDO      ), somethingChanged && !filterEnabled );
	EnableWindow( GetDlgItem( this->handle, IDC_IMPORT    ), !filterEnabled );
//	EnableWindow( GetDlgItem( this->handle, IDC_GLOBAL    ), !filterEnabled );
	EnableWindow( GetDlgItem( this->handle, IDOK          ), validSelection );

	return validSelection;
}


CSAMWindow::CSAMWindow( WORD action, CSWindow* parent )
{
	this->action = action;
	this->parent = parent;
	this->bChanged = FALSE;
	this->hCombo = this->hMessage = NULL;
	
	if ( this->action == IDC_ADD )
		this->item = new StatusItem( );
	else
		this->item = new StatusItem( *this->parent->itemslist->list->get( this->parent->listview->getPositionInList( ) ) );

}


CSAMWindow::~CSAMWindow( )
{
	if ( ! this->bChanged )
		delete this->item;
}


void CSAMWindow::exec( )
{
	DialogBoxParam( CSList::handle, MAKEINTRESOURCE( IDD_ADDMODIFY ), NULL, ( DLGPROC )CSAMWindowProc, ( LPARAM )this );
}


void CSAMWindow::setCombo()
{
	PROTOACCOUNT* pdescr = (PROTOACCOUNT*)CallService(MS_PROTO_GETACCOUNT, 0, (LPARAM)this->parent->protoName);
	if (pdescr == NULL)
		return;

	if (lstrcmpA(pdescr->szProtoName, "ICQ") == 0)
	{
		DWORD uin = 0;
		uin = DBGetContactSettingDword(NULL, pdescr->szModuleName, "UIN", 0);
		if (uin != 0)
		{
			SendMessage( this->hCombo, CBEM_SETIMAGELIST, 0, ( LPARAM )this->parent->icons );
			for ( int i = 0; i < ( ( this->parent->bExtraIcons ) ? XSTATUS_COUNT_EXTENDED : XSTATUS_COUNT ); i++ )
			{
				COMBOBOXEXITEM cbi = { 0 };
				cbi.mask = CBEIF_IMAGE | CBEIF_TEXT | CBEIF_SELECTEDIMAGE;
				cbi.pszText = TranslateTS(xstatus_names[i].ptszTitle);
				cbi.iImage = xstatus_names[i].iId - 1;
				cbi.iItem = i;
				cbi.iSelectedImage = i;
				SendMessage( this->hCombo, CBEM_INSERTITEM, 0, ( LPARAM )&cbi );
			}
			SendMessage( this->hCombo, CB_SETCURSEL, 0, 0 ); // first 0 sets selection to top
		}
	}
	else if (lstrcmpA(pdescr->szProtoName, "JABBER") == 0)
	{
		DBVARIANT dbv = {0};
		DBGetContactSettingTString(NULL, pdescr->szModuleName, "jid", &dbv);
		if (dbv.ptszVal != _T(""))
		{
			SendMessage( this->hCombo, CBEM_SETIMAGELIST, 0, ( LPARAM )this->parent->icons );
			for ( int i = 0; i < MOODS_COUNT; i++ )
			{
				COMBOBOXEXITEM cbi = { 0 };
				cbi.mask = CBEIF_IMAGE | CBEIF_TEXT | CBEIF_SELECTEDIMAGE;
				cbi.pszText = TranslateTS(moods_names[i].ptszTitle);
				cbi.iImage = moods_names[i].iId - 1;
				cbi.iItem = i;
				cbi.iSelectedImage = i;
				SendMessage( this->hCombo, CBEM_INSERTITEM, 0, ( LPARAM )&cbi );
			}
			SendMessage( this->hCombo, CB_SETCURSEL, 0, 0 ); // first 0 sets selection to top
		}
	}
}

void CSAMWindow::fillDialog( )
{
	if ( this->action == IDC_ADD )
	{
        SetWindowText( this->handle, TranslateT("Add new item") );
        SetDlgItemText( this->handle, IDOK, TranslateT("Add") );
	}
	else
	{
        SetWindowText( this->handle, TranslateT("Modify item") );
        SetDlgItemText( this->handle, IDOK, TranslateT("Modify") );
	}

	SendMessage( this->hCombo, CB_SETCURSEL, this->item->iIcon, 0 );
	SetDlgItemText( this->handle, IDC_MESSAGE, this->item->tszMessage );
}


void CSAMWindow::checkFieldLimit( WORD action, WORD item )
{
	BOOL type = ( item == IDC_MESSAGE ) ? TRUE : FALSE;
	unsigned int limit = type ? EXTRASTATUS_MESSAGE_LIMIT : EXTRASTATUS_TITLE_LIMIT;

	if ( action == EN_CHANGE )
	{
		TCHAR* ptszInputText = (TCHAR*)mir_alloc((limit + 8) * sizeof(TCHAR));

		GetDlgItemText( this->handle, item, ptszInputText, limit + 8 );

		if ( _tcslen( ptszInputText ) > limit )
		{
#if ( WINVER >= 0x501 )
			TCHAR tszPopupTip[MAX_PATH];
			EDITBALLOONTIP ebt = { 0 };
			ebt.cbStruct = sizeof(ebt);
			ebt.pszTitle = TranslateT("Warning");
			mir_sntprintf(tszPopupTip, SIZEOF(tszPopupTip), TranslateT("This field doesn't accept string longer than %d characters. The string will be truncated."), limit);
			ebt.pszText = tszPopupTip;
			ebt.ttiIcon = TTI_WARNING;
			SendMessage( GetDlgItem( this->handle, item ), EM_SHOWBALLOONTIP, 0, ( LPARAM )&ebt );
#endif
			TCHAR* ptszOutputText = (TCHAR*)mir_alloc((limit + 1) * sizeof(TCHAR));
			GetDlgItemText( this->handle, item, ptszOutputText, limit + 1 );
			SetDlgItemText( this->handle, item, ptszOutputText );
			mir_free( ptszOutputText );
		}
		mir_free( ptszInputText );
	}
}


void CSAMWindow::checkItemValidity()
{
	COMBOBOXEXITEM cbi = { 0 };
	cbi.mask = CBEIF_IMAGE;
	cbi.iItem = SendDlgItemMessage( this->handle, IDC_COMBO, CB_GETCURSEL, 0, 0 );
	SendDlgItemMessage( this->handle, IDC_COMBO, CBEM_GETITEM, 0, ( LPARAM )&cbi );

	if ( this->item->iIcon != cbi.iImage )
		( this->item->iIcon = cbi.iImage ) && ( this->bChanged = TRUE );

	TCHAR tszInputMessage[EXTRASTATUS_MESSAGE_LIMIT];

	GetDlgItemText( this->handle, IDC_MESSAGE, tszInputMessage, EXTRASTATUS_MESSAGE_LIMIT );

	PROTOACCOUNT* pdescr = (PROTOACCOUNT*)CallService(MS_PROTO_GETACCOUNT, 0, (LPARAM)this->parent->protoName);
	if (pdescr == NULL)
		return;

	if (lstrcmpA(pdescr->szProtoName, "ICQ") == 0)
	{
		int i = SendMessage(this->hCombo, CB_GETCURSEL, 0, 0);
		lstrcpy(this->item->tszTitle, TranslateTS(xstatus_names[i].ptszTitle));
	}
	if (lstrcmpA(pdescr->szProtoName, "JABBER") == 0)
	{
		int i = SendMessage(this->hCombo, CB_GETCURSEL, 0, 0);
		lstrcpy(this->item->tszTitle, TranslateTS(moods_names[i].ptszTitle));
	}
	if ( lstrcmp( this->item->tszMessage, tszInputMessage ) )
		( lstrcpy( this->item->tszMessage, tszInputMessage ) ) && ( this->bChanged = TRUE );
}


CSListView::CSListView( HWND hwnd, CSWindow* parent )
{
	this->handle = hwnd;
	this->parent = parent;

#if (_WIN32_WINNT >= 0x501)
	LVGROUP lg = { 0 };
	lg.cbSize = sizeof(LVGROUP);
	lg.mask = LVGF_HEADER | LVGF_GROUPID;

	lg.pszHeader = TranslateT("Favourites");
	lg.iGroupId = 0;
	ListView_InsertGroup( this->handle, -1, &lg );

	lg.pszHeader = TranslateT("Regular statuses");
	lg.iGroupId = 1;
	ListView_InsertGroup( this->handle, -1, &lg );
	ListView_EnableGroupView( this->handle, TRUE );
#endif

	LVCOLUMN lvc = { 0 };
	lvc.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 0x00;
	lvc.pszText = TEXT( "" );
	lvc.cx = 0x16;
	SendMessage( this->handle, LVM_INSERTCOLUMN, 0, ( LPARAM )&lvc );
	lvc.pszText = TranslateT("Title");
	lvc.cx = 0x64;
	SendMessage( this->handle, LVM_INSERTCOLUMN, 1, ( LPARAM )&lvc );
	lvc.pszText = TranslateT("Message");
	lvc.cx = 0xa8;
	SendMessage( this->handle, LVM_INSERTCOLUMN, 2, ( LPARAM )&lvc );

//	ListView_SetExtendedListViewStyle( this->handle, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP );
#if (_WIN32_IE >= 0x0400)
	ListView_SetExtendedListViewStyleEx( this->handle, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP | LVS_EX_INFOTIP );
#endif
	ListView_SetImageList( this->handle, this->parent->icons, LVSIL_SMALL );
}


void CSListView::addItem( StatusItem* item, int itemNumber )
{
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_IMAGE | LVIF_GROUPID | LVIF_PARAM;
	lvi.cchTextMax = 256;
	lvi.iItem = ListView_GetItemCount( this->handle );
	lvi.lParam = itemNumber;
#if ( WINVER >= 0x501 )
	lvi.iGroupId = ( item->bFavourite == TRUE ) ? 0 : 1;
#endif

	// first column
	lvi.iSubItem = 0;
	lvi.iImage = item->iIcon; // use selected xicon
	SendMessage( this->handle, LVM_INSERTITEM, 0, ( LPARAM )&lvi );

	// second column
	lvi.mask = LVIF_TEXT;
	lvi.iSubItem = 1;
	lvi.pszText = item->tszTitle;
	SendMessage( this->handle, LVM_SETITEM, 0, ( LPARAM )&lvi );

	// third column
	lvi.iSubItem = 2;
	lvi.pszText = item->tszMessage;
	SendMessage( this->handle, LVM_SETITEM, 0, ( LPARAM )&lvi );
}


void CSListView::initItems( ListItem< StatusItem >* items )
{
	ListItem< StatusItem >* help = items;
	for ( int i = 0; help != NULL; help = help->next, i++ )
		if ( parent->itemPassedFilter( help ) )
			addItem( help->item, i );
}


void CSListView::reinitItems( ListItem< StatusItem >* items )
{
	EnableWindow( this->handle, FALSE );
	this->removeItems( );
	this->initItems( items );
	EnableWindow( this->handle, TRUE );
}


void CSListView::removeItems( )
{
	ListView_DeleteAllItems( this->handle );
}


int CSListView::getPositionInList( )
{
	LVITEM lvi = { 0 };
	lvi.iItem = ListView_GetSelectedItemMacro( this->parent->listview->handle );
	lvi.iSubItem = 0;
	lvi.mask = LVIF_PARAM;
	ListView_GetItem( this->parent->listview->handle, &lvi );
	return lvi.lParam;
}


void CSListView::setFullFocusedSelection( int selection )
{
	ListView_SetItemState(  this->handle, -1, 0,     LVIS_SELECTED );
	ListView_EnsureVisible( this->handle, selection, FALSE );
	ListView_SetItemState(  this->handle, selection, LVIS_SELECTED, LVIS_SELECTED );
	ListView_SetItemState(  this->handle, selection, LVIS_FOCUSED , LVIS_FOCUSED  );
	SetFocus( this->handle );
}


// ====[ THREAD FORK ]========================================================

void ForkThread( pThreadFunc pFunc, void* arg )
{
	mir_forkthread( pFunc, arg );
}


// ====[ LIST MANAGEMENT ]====================================================

CSItemsList::CSItemsList(char* protoName)
{
	this->list = new List< StatusItem >( this->compareItems );
	this->loadItems(protoName);
}


CSItemsList::~CSItemsList( )
{
	delete this->list;
}


int CSItemsList::compareItems( const StatusItem* p1, const StatusItem* p2 )
{
	int favRes = 0, icoRes = 0, ttlRes = 0, msgRes = 0;

	if ( p1->bFavourite < p2->bFavourite )
	    favRes = 1;
	else if ( p1->bFavourite > p2->bFavourite )
	    favRes = -1;

	int result;
	if ( p1->iIcon > p2->iIcon )
		icoRes = 1;
	else if ( p1->iIcon < p2->iIcon )
		icoRes = -1;

	result = lstrcmp( p1->tszTitle, p2->tszTitle );
	ttlRes = result;

	result = lstrcmp( p1->tszMessage, p2->tszMessage );
	msgRes = result;

	if ( !icoRes && !ttlRes && !msgRes )
		return 0;

	if ( favRes != 0 )
		return favRes;
	if ( icoRes != 0 )
		return icoRes;
	if ( ttlRes != 0 )
		return ttlRes;
	if ( msgRes != 0 )
		return msgRes;

	return 0;
}


void CSItemsList::loadItems(char* protoName)
{
	char dbSetting[32];
	mir_snprintf(dbSetting, SIZEOF(dbSetting), "%s_ItemsCount", protoName);
	unsigned int itemsCount = getWord(dbSetting, DEFAULT_ITEMS_COUNT);

	for (unsigned int i = 1; i <= itemsCount; i++)
	{
		StatusItem* item = new StatusItem();
		DBVARIANT dbv = {DBVT_TCHAR};
		mir_snprintf(dbSetting, SIZEOF(dbSetting), "%s_Item%dIcon", protoName, i);
		item->iIcon = getByte(dbSetting, DEFAULT_ITEM_ICON);
		mir_snprintf(dbSetting, SIZEOF(dbSetting), "%s_Item%dTitle", protoName, i);
		getTString(dbSetting, &dbv);
		lstrcpy(item->tszTitle, dbv.ptszVal);
		mir_snprintf(dbSetting, SIZEOF(dbSetting), "%s_Item%dMessage", protoName, i);
		getTString(dbSetting, &dbv);
		lstrcpy(item->tszMessage, dbv.ptszVal);
		mir_snprintf(dbSetting, SIZEOF(dbSetting), "%s_Item%dFavourite", protoName, i);
		item->bFavourite = (BOOL)getByte(dbSetting, DEFAULT_ITEM_IS_FAVOURITE);
		this->list->add(item);
	}
}


void CSItemsList::saveItems(char* protoName)
{
	unsigned int i;
	char dbSetting[32];
	mir_snprintf(dbSetting, SIZEOF(dbSetting), "%s_ItemsCount", protoName);
	unsigned int oldItemsCount = getWord(dbSetting, DEFAULT_ITEMS_COUNT);

	for (i = 1; i <= this->list->getCount(); i++)
	{
		StatusItem* item = this->list->get( i - 1 );
		mir_snprintf(dbSetting, SIZEOF(dbSetting), "%s_Item%dIcon", protoName, i);
		setByte(dbSetting, item->iIcon);
		mir_snprintf(dbSetting, SIZEOF(dbSetting), "%s_Item%dTitle", protoName, i);
		setTString(dbSetting, item->tszTitle);
		mir_snprintf(dbSetting, SIZEOF(dbSetting), "%s_Item%dMessage", protoName, i);
		setTString(dbSetting, item->tszMessage);
		mir_snprintf(dbSetting, SIZEOF(dbSetting), "%s_Item%dFavourite", protoName, i);
		setByte(dbSetting, item->bFavourite);
	}

	mir_snprintf(dbSetting, SIZEOF(dbSetting), "%s_ItemsCount", protoName);
	setWord(dbSetting, this->list->getCount());

	for ( ; i <= oldItemsCount; i++)
	{
		mir_snprintf(dbSetting, SIZEOF(dbSetting), "%s_Item%dIcon", protoName, i);
		deleteSetting(dbSetting);
		mir_snprintf(dbSetting, SIZEOF(dbSetting), "%s_Item%dTitle", protoName, i);
		deleteSetting(dbSetting);
		mir_snprintf(dbSetting, SIZEOF(dbSetting), "%s_Item%dMessage", protoName, i);
		deleteSetting(dbSetting);
		mir_snprintf(dbSetting, SIZEOF(dbSetting), "%s_Item%dFavourite", protoName, i);
		deleteSetting(dbSetting);
	}
}


// ====[ PROCEDURES ]=========================================================

INT_PTR CALLBACK CSWindowProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	CSWindow* csw = ( CSWindow* )GetWindowLongPtr( hwnd, GWLP_USERDATA );

	switch ( message )
	{

	case WM_INITDIALOG:
		csw = ( CSWindow* )lparam;
		SetWindowLongPtr( hwnd, GWLP_USERDATA, lparam );
		csw->handle = hwnd;
		csw->initIcons();
		csw->initButtons( );
		csw->listview = new CSListView( GetDlgItem( hwnd, IDC_CSLIST ), csw );
		csw->listview->initItems( csw->itemslist->list->getListHead( ) );
		csw->toggleButtons( );
		csw->toggleEmptyListMessage( );
		csw->loadWindowPosition( );
		SetWindowText(hwnd, TranslateT(MODULENAME));
		TranslateDialogDefault(hwnd);
		return TRUE;

	case WM_COMMAND:
		switch ( LOWORD( wparam ) )
		{
		case IDC_MODIFY:
		case IDC_ADD:
			csw->addModifyDlg = new CSAMWindow( LOWORD( wparam ), csw );
			csw->addModifyDlg->exec( );
			if ( csw->addModifyDlg->bChanged )
			{
				if ( LOWORD( wparam ) == IDC_MODIFY )
					csw->itemslist->list->remove( csw->listview->getPositionInList( ) );

				int selection = csw->itemslist->list->add( csw->addModifyDlg->item );
				csw->bSomethingChanged = TRUE;
				csw->listview->reinitItems( csw->itemslist->list->getListHead( ) );
				csw->listview->setFullFocusedSelection( selection );
				csw->toggleButtons( );
				csw->toggleEmptyListMessage( );
			}
			delete csw->addModifyDlg;
			break;

		case IDC_REMOVE:
			if ( getByte( "ConfirmDeletion", DEFAULT_PLUGIN_CONFIRM_ITEMS_DELETION ) )
				if ( MessageBox( hwnd, TranslateT("Do you really want to delete selected item?"), TranslateT(MODULENAME), MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION ) == IDNO )
					break;
			csw->itemslist->list->remove( csw->listview->getPositionInList( ) );
			csw->bSomethingChanged = TRUE;
			csw->listview->reinitItems( csw->itemslist->list->getListHead( ) );
			csw->toggleButtons( );
			csw->toggleEmptyListMessage( );
			break;

		case IDC_FAVOURITE:
			{
				int selection = csw->listview->getPositionInList( );
				StatusItem* f = new StatusItem( *csw->itemslist->list->get( selection ) );
				f->bFavourite = ! f->bFavourite;
				csw->itemslist->list->remove( selection );
				selection = csw->itemslist->list->add( f );
				csw->bSomethingChanged = TRUE;
				csw->listview->reinitItems( csw->itemslist->list->getListHead( ) );
				csw->listview->setFullFocusedSelection( selection );
				csw->toggleButtons( );
			}
			break;

		case IDC_UNDO:
			{
			csw->itemslist->list->destroy( );
			csw->itemslist->loadItems(csw->protoName);
			csw->bSomethingChanged = FALSE;
			csw->listview->reinitItems( csw->itemslist->list->getListHead( ) );
			csw->toggleButtons( );
			csw->toggleEmptyListMessage( );
			break;
			}

		case IDC_IMPORT:
			{
				int result = getByte( "DeleteAfterImport", DEFAULT_PLUGIN_DELETE_AFTER_IMPORT );
				if ( result == TRUE )
					result = IDYES;
				else
				{
					result = MessageBox( hwnd, TranslateT("Do you want old database entries to be deleted after Import?"),
					                     TranslateT(MODULENAME), MB_YESNOCANCEL | MB_DEFBUTTON2 | MB_ICONQUESTION );
					if ( result == IDCANCEL ) break;
				}

				CSList::ForAllProtocols( CSList::importCustomStatusUIStatusesFromAllProtos, ( void* )&result );
				csw->bSomethingChanged = TRUE;
				csw->toggleButtons( );
				csw->toggleEmptyListMessage( );
			}
			break;

		case IDC_FILTER:
			csw->toggleFilter( );
			csw->toggleButtons( );
			break;

		case IDC_FILTER_FIELD:
			if ( HIWORD( wparam ) == EN_CHANGE )
				csw->listview->reinitItems( csw->itemslist->list->getListHead( ) );
			break;

		case IDCLOSE:    // close and save, no custom status 
        case IDCANCEL:   // close and save, no custom status 
        case IDC_CANCEL:   // close and save, cancel custom status
        case IDOK:       // close and save, set selected custom status
			if ( LOWORD( wparam ) == IDOK && csw->toggleButtons( ) )
				cslist->setStatus(IDOK, csw->itemslist->list->get(csw->listview->getPositionInList()), csw->protoName);
			if ( LOWORD( wparam ) == IDC_CANCEL )
				cslist->setStatus(IDC_CANCEL, NULL, csw->protoName);
			if ( csw->bSomethingChanged )
				if (csw->itemslist!=NULL)
					csw->itemslist->saveItems(csw->protoName);
			csw->saveWindowPosition( csw->handle );
			delete csw->listview;
			csw->deinitIcons( );
			cslist->closeList( hwnd );
			break;

		}
		return FALSE;

	case WM_NOTIFY:
		if ( wparam == IDC_CSLIST )
		{
			NMHDR* pnmh = ( NMHDR* )lparam;
			switch ( pnmh->code )
			{

			case NM_DBLCLK:
				PostMessage( hwnd, WM_COMMAND, IDOK, 0L );
				break;

			case LVN_ITEMCHANGED:
			case NM_CLICK:
				csw->toggleButtons( );
				break;

			}
		}
		return FALSE;

	case WM_CTLCOLORSTATIC:
		SetTextColor( ( HDC )wparam, RGB( 174, 174, 174 ) );
		if ( ( ( HWND )lparam == GetDlgItem( hwnd, IDC_NO_ITEMS ) ) ||
		     ( ( HWND )lparam == GetDlgItem( hwnd, IDC_ADD_SAMPLE ) ) )
			return ( BOOL )GetStockObject( WHITE_BRUSH );
		return FALSE;
	}
	return FALSE;
}


INT_PTR CALLBACK CSAMWindowProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	CSAMWindow* csamw = ( CSAMWindow* )GetWindowLongPtr( hwnd, GWLP_USERDATA );

	switch ( message )
	{

		case WM_INITDIALOG:
			{
				csamw = ( CSAMWindow* )lparam;
				SetWindowLongPtr( hwnd, GWLP_USERDATA, lparam );
				csamw->handle = hwnd;
				EnableWindow( csamw->parent->handle, FALSE );
				csamw->hCombo = GetDlgItem( hwnd, IDC_COMBO );
				csamw->hMessage = GetDlgItem( hwnd, IDC_MESSAGE );
				csamw->setCombo();
				csamw->fillDialog();
				TranslateDialogDefault(hwnd);
			}
			break;

		case WM_COMMAND:
			switch ( LOWORD( wparam ) )
			{
			case IDC_MESSAGE:
				csamw->checkFieldLimit( HIWORD( wparam ), LOWORD( wparam ) );
				break;
			case IDOK:
				csamw->checkItemValidity( );
			case IDCANCEL:
				EnableWindow( csamw->parent->handle, TRUE );
				EndDialog( hwnd, LOWORD( wparam ) );
				break;

			}
			break;

	}
	return FALSE;
}


INT_PTR CALLBACK CSOptionsProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	switch ( message )
	{

	case WM_INITDIALOG:
		{
			CheckDlgButton( hwnd, IDC_CONFIRM_DELETION,
							getByte( "ConfirmDeletion", DEFAULT_PLUGIN_CONFIRM_ITEMS_DELETION ) ?
							BST_CHECKED : BST_UNCHECKED );

			CheckDlgButton( hwnd, IDC_DELETE_AFTER_IMPORT,
							getByte( "DeleteAfterImport", DEFAULT_PLUGIN_DELETE_AFTER_IMPORT ) ?
							BST_CHECKED : BST_UNCHECKED );

			CheckDlgButton( hwnd, IDC_REMEMBER_POSITION,
			                getByte( "RememberWindowPosition", DEFAULT_REMEMBER_WINDOW_POSITION ) ?
			                BST_CHECKED : BST_UNCHECKED );

			unsigned int protosExtraEnabled = 1;

			CSList::ForAllProtocols( CSList::countPlusModProtos, ( void* )&protosExtraEnabled );

			if ( protosExtraEnabled ) // some protocol(s) allow(s) use of extra icons
			{
				EnableWindow( GetDlgItem( hwnd, IDC_ALLOW_EXTRA_ICONS ), TRUE );
				CheckDlgButton( hwnd, IDC_ALLOW_EXTRA_ICONS, getByte( "AllowExtraIcons",
				                DEFAULT_ALLOW_EXTRA_ICONS ) );
			}

			TranslateDialogDefault(hwnd);
		}
		return TRUE;

	case WM_NOTIFY:
		switch ( ( ( LPNMHDR )lparam )->code )
		{

		case PSN_APPLY:
			{
				setByte( "ConfirmDeletion", IsDlgButtonChecked( hwnd, IDC_CONFIRM_DELETION ) ? 1 : 0 );
				setByte( "DeleteAfterImport", IsDlgButtonChecked( hwnd, IDC_DELETE_AFTER_IMPORT ) ? 1 : 0 );
				setByte( "AllowExtraIcons", IsDlgButtonChecked( hwnd, IDC_ALLOW_EXTRA_ICONS ) ? 1 : 0 );
				setByte( "RememberWindowPosition", IsDlgButtonChecked( hwnd, IDC_REMEMBER_POSITION ) ? 1 : 0 );

				cslist->rebuildMenuItems( );
			}
			break;

		}
		return TRUE;

	}
	return FALSE;
}


// ====[ HELPERS ]============================================================

HICON LoadIconExEx( const char* IcoLibName, int NonIcoLibIcon )
{
	if ( ServiceExists( MS_SKIN2_GETICON ) ) {
		char szSettingName[64];
		mir_snprintf(szSettingName, SIZEOF(szSettingName), "%s_%s", __INTERNAL_NAME, IcoLibName);
		return ( HICON )CallService( MS_SKIN2_GETICON, 0, ( LPARAM )szSettingName );
	}
	else
		return ( HICON )LoadImage( cslist->handle, MAKEINTRESOURCE( NonIcoLibIcon ), IMAGE_ICON, 0, 0, 0 );
}
