
//This file is part of Msg_Export a Miranda IM plugin
//Copyright (C)2002 Kennet Nielsen ( http://sourceforge.net/projects/msg-export/ )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <windows.h>
#include <Shellapi.h>

//#include "../Miranda-IM/resource.h"

#include "utils.h"
#include "options.h"
#include "FileViewer.h"
#include "Glob.h"

#include "resource.h"


#define MS_SHOW_EXPORT_HISTORY "History/ShowExportHistory"

HINSTANCE hInstance = NULL;
int hLangpack = 0;

// static so they can not be used from other modules ( sourcefiles )
static HANDLE hEventOptionsInitialize = 0;
static HANDLE hDBEventAdded = 0;
static HANDLE hDBContactDeleted = 0;
static HANDLE hEventSystemInit = 0;
static HANDLE hEventSystemShutdown = 0;

static HANDLE hServiceFunc = 0;

static HANDLE hOpenHistoryMenuItem = 0;

/////////////////////////////////////////////////////
// Remember to update the Version in the resource !!!
/////////////////////////////////////////////////////

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"Message export (mod by ring0)",
	PLUGIN_MAKE_VERSION(3,1,0,3),
	"Exports every message, URL or File you receive to a text file.\r\n"
	"Messages are exported to one file per user, users may be set to use the same file",
	"Kennet Nielsen, mod by ring0",
	"Kennet_N@ofir.dk",
	"© 2002 Kennet Nielsen",
	"http://sourceforge.net/projects/msg-export/",
	UNICODE_AWARE,
	{ 0x46102b07, 0xc215, 0x4162, { 0x9c, 0x83, 0xd3, 0x77, 0x88, 0x1d, 0xa7, 0xcc } } // {46102B07-C215-4162-9C83-D377881DA7CC}
};


/////////////////////////////////////////////////////////////////////
// Member Function : ShowExportHistory
// Type            : Global
// Parameters      : wParam - (WPARAM)(HANDLE)hContact
//                   lParam - ?
// Returns         : static int
// Description     : Called when user selects my menu item "Open Exported History"
//                   
// References      : -
// Remarks         : -
// Created         : 020422 , 22 April 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

static int ShowExportHistory(WPARAM wParam,LPARAM /*lParam*/)
{
	if( bUseInternalViewer() )
	{
		bShowFileViewer( (HANDLE)wParam );
		return 0;
	}
	bOpenExternaly( (HANDLE)wParam );
	return 0;
}

/////////////////////////////////////////////////////////////////////
// Member Function : nSystemShutdown
// Type            : Global
// Parameters      : wparam - 0
//                   lparam - 0
// Returns         : int
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 020428 , 28 April 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

int nSystemShutdown(WPARAM /*wparam*/,LPARAM /*lparam*/)
{
	if( hEventOptionsInitialize )
	{
		UnhookEvent(hEventOptionsInitialize);
		hEventOptionsInitialize = 0;
	}

	if( hDBEventAdded )
	{
		UnhookEvent(hDBEventAdded);
		hDBEventAdded = 0;
	}

	if( hDBContactDeleted )
	{
		UnhookEvent(hDBContactDeleted);
		hDBContactDeleted = 0;
	}

	if( hServiceFunc )
	{
		DestroyServiceFunction( hServiceFunc );
		hServiceFunc = 0;
	}

	if( hEventSystemInit )
	{
		UnhookEvent(hEventSystemInit);
		hEventSystemInit = 0;
	}

	if( hEventSystemShutdown )
	{
		UnhookEvent(hEventSystemShutdown); // here we unhook the fun we are in, might not bee good
		hEventSystemShutdown = 0;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////
// Member Function : MainInit
// Type            : Global
// Parameters      : wparam - ?
//                   lparam - ?
// Returns         : int
// Description     : Called when system modules has been loaded
//                   
// References      : -
// Remarks         : -
// Created         : 020422 , 22 April 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

int MainInit(WPARAM /*wparam*/,LPARAM /*lparam*/)
{

	Initilize();

	bReadMirandaDirAndPath();
	UpdateFileToColWidth();

	hDBEventAdded = HookEvent( ME_DB_EVENT_ADDED , nExportEvent );
	if( !hDBEventAdded )
		MessageBox( NULL , LPGENT("Failed to HookEvent ME_DB_EVENT_ADDED") , MSG_BOX_TITEL , MB_OK );


	hDBContactDeleted = HookEvent( ME_DB_CONTACT_DELETED , nContactDeleted );
	if( !hDBContactDeleted )
		MessageBox( NULL , LPGENT("Failed to HookEvent ME_DB_CONTACT_DELETED") , MSG_BOX_TITEL , MB_OK );
	

	hEventOptionsInitialize = HookEvent( ME_OPT_INITIALISE , OptionsInitialize );
	if( !hEventOptionsInitialize )
		MessageBox( NULL , LPGENT("Failed to HookEvent ME_OPT_INITIALISE") , MSG_BOX_TITEL , MB_OK );


	CLISTMENUITEM mi;
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.flags=0;
	mi.pszContactOwner=NULL;    //all contacts
	mi.hIcon=LoadIcon(hInstance,MAKEINTRESOURCE(IDI_EXPORT_MESSAGE));

	if( bReplaceHistory )
	{
		mi.position= 1000090000;
		mi.pszName=LPGEN("View &History");
		mi.pszService=MS_HISTORY_SHOWCONTACTHISTORY;
	}
	else
	{
		mi.position = 1000090100;
		mi.pszName=LPGEN("Open E&xported History");
		mi.pszService=MS_SHOW_EXPORT_HISTORY;
	}
	hOpenHistoryMenuItem  = Menu_AddContactMenuItem(&mi);

	if( !hOpenHistoryMenuItem )
		MessageBox( NULL , LPGENT("Failed to add menu item Open Exported History\nCallService(MS_CLIST_ADDCONTACTMENUITEM,...)") , MSG_BOX_TITEL , MB_OK );

/*
	hEventSystemShutdown = HookEvent( ME_SYSTEM_SHUTDOWN , nSystemShutdown );

	if( !hEventSystemShutdown )
		MessageBox( NULL , "Failed to HookEvent ME_SYSTEM_SHUTDOWN" , MSG_BOX_TITEL , MB_OK );
*/

/*
	_TCHAR szBuf[ 10000 ];
	for( int n = 0 ; n < 1000 ; n++ )
	{
		for( int y = 0 ; y < n ; y++ )
		{
			szBuf[ y ] = '0' + y%10;//((n + y) % 8 ) ? ('0' + ((n + y) % 10))  : ' ' ;
		}
		szBuf[ y ] = 0;

		HANDLE hFile = CreateFile( "C:\\test.txt" , GENERIC_WRITE , FILE_SHARE_READ , 0 ,OPEN_ALWAYS , FILE_ATTRIBUTE_NORMAL , NULL );
		SetFilePointer( hFile , 0 , 0 , FILE_END );

		bWriteNewLine( hFile , 10 );
		bWriteNewLine( hFile , 0 );
		bWriteNewLine( hFile , 120 );

		DWORD dwBytesWritten;
		WriteFile( hFile , "\r\n\r\n" , 4 , &dwBytesWritten , NULL );

		CloseHandle( hFile );
	//}*/
	
	return 0;
}



/////////////////////////////////////////////////////////////////////
// Member Function : DllMain
// Type            : Global
// Parameters      : hinst       - ?
//                   fdwReason   - ?
//                   lpvReserved - ?
// Returns         : BOOL WINAPI
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 020422 , 22 April 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinst,DWORD /*fdwReason*/,LPVOID /*lpvReserved*/)
{
	hInstance=hinst;
	return 1;
}


#ifdef __cplusplus
extern "C" {
#endif


/////////////////////////////////////////////////////////////////////
// Member Function : MirandaPluginInfo
// Type            : Global
// Parameters      : mirandaVersion - ?
// Returns         : 
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 020422 , 22 April 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD /*mirandaVersion*/)
{
	return &pluginInfo;
}

static const MUUID interfaces[] = { MIID_HISTORYEXPORT, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID * MirandaPluginInterfaces(void)
{
	return interfaces;
}

/////////////////////////////////////////////////////////////////////
// Member Function : Load
// Type            : Global
// Parameters      : link - ?
// Returns         : int
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 020422 , 22 April 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

int __declspec(dllexport)Load()
{
	mir_getLP(&pluginInfo);
	hEventSystemInit = HookEvent(ME_SYSTEM_MODULESLOADED,MainInit);

	if( !hEventSystemInit )
	{
		MessageBox( NULL , LPGENT("Failed to HookEvent ME_SYSTEM_MODULESLOADED") , MSG_BOX_TITEL , MB_OK );
		return 0;
	}

	nMaxLineWidth = DBGetContactSettingWord( NULL , MODULE , "MaxLineWidth" , nMaxLineWidth );
	if( nMaxLineWidth < 5 )
		nMaxLineWidth = 5;

	sExportDir  = _DBGetString( NULL , MODULE , "ExportDir" , _T("%dbpath%\\MsgExport\\") );
	sDefaultFile = _DBGetString( NULL , MODULE , "DefaultFile" , _T("%nick%.txt") );

	sTimeFormat = _DBGetString( NULL , MODULE , "TimeFormat" , _T("d s") );

	sFileViewerPrg = _DBGetString( NULL , MODULE , "FileViewerPrg" , _T("") );
	bUseInternalViewer( DBGetContactSettingByte( NULL , MODULE , "UseInternalViewer" , bUseInternalViewer() ) != 0 );

	bReplaceHistory = DBGetContactSettingByte( NULL , MODULE , "ReplaceHistory" , bReplaceHistory ) != 0;
	bAppendNewLine = DBGetContactSettingByte( NULL , MODULE , "AppendNewLine" , bAppendNewLine ) != 0;
	bUseUtf8InNewFiles = DBGetContactSettingByte( NULL , MODULE , "UseUtf8InNewFiles" , bUseUtf8InNewFiles ) != 0;
	bUseLessAndGreaterInExport = DBGetContactSettingByte( NULL , MODULE , "UseLessAndGreaterInExport" , bUseLessAndGreaterInExport ) != 0;

	enRenameAction = (ENDialogAction)DBGetContactSettingByte( NULL , MODULE , "RenameAction" , enRenameAction );
	enDeleteAction = (ENDialogAction)DBGetContactSettingByte( NULL , MODULE , "DeleteAction" , enDeleteAction );;

	// Plugin sweeper support
	DBWriteContactSettingTString(NULL,"Uninstall","Message Export",_T(MODULE));

	if( bReplaceHistory )
	{
		hServiceFunc = CreateServiceFunction(MS_HISTORY_SHOWCONTACTHISTORY,(MIRANDASERVICE)ShowExportHistory); //this need new code
/*		if( hServiceFunc )
		{
			int *disableDefaultModule=(int*)CallService(MS_PLUGINS_GETDISABLEDEFAULTARRAY,0,0);
			if( disableDefaultModule )
			{
				disableDefaultModule[DEFMOD_UIHISTORY] = TRUE;
			}
			else
			{
				DestroyServiceFunction( hServiceFunc );
				hServiceFunc = 0;
			}
		}*/

		if( ! hServiceFunc ) 
			MessageBox( NULL , LPGENT("Failed to replace Miranda History.\r\nThis is most likely due to changes in Miranda.") , MSG_BOX_TITEL , MB_OK );
	}

	if( ! hServiceFunc )
	{
		hServiceFunc = CreateServiceFunction(MS_SHOW_EXPORT_HISTORY,(MIRANDASERVICE)ShowExportHistory);
	}

	if( ! hServiceFunc )
	{
		MessageBox( NULL , LPGENT("Failed to CreateServiceFunction MS_SHOW_EXPORT_HISTORY") , MSG_BOX_TITEL , MB_OK );
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////
// Member Function : Unload
// Type            : Global
// Parameters      : none
// Returns         : 
// Description     : 
//                   
// References      : -
// Remarks         : -
// Created         : 020422 , 22 April 2002
// Developer       : KN   
/////////////////////////////////////////////////////////////////////

__declspec(dllexport)int Unload(void)
{
	//if( !hEventSystemShutdown ) // we will try to unload anyway 
	{
		nSystemShutdown(0,0);
	}
	Uninitilize();
	bUseInternalViewer( false );
	return 0;
}

#ifdef __cplusplus
}
#endif
