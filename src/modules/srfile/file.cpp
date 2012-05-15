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
#include "file.h"

TCHAR* PFTS_StringToTchar( int flags, const PROTOCHAR* s );
int PFTS_CompareWithTchar( PROTOFILETRANSFERSTATUS* ft, const PROTOCHAR* s, TCHAR* r );

static HANDLE hSRFileMenuItem;

static INT_PTR SendFileCommand(WPARAM wParam, LPARAM)
{
	struct FileSendData fsd;
	fsd.hContact=(HANDLE)wParam;
	fsd.ppFiles=NULL;
	CreateDialogParam(hMirandaInst,MAKEINTRESOURCE(IDD_FILESEND),NULL,DlgProcSendFile,(LPARAM)&fsd);
	return 0;
}

static INT_PTR SendSpecificFiles(WPARAM wParam,LPARAM lParam)
{
	FileSendData fsd;
	fsd.hContact=(HANDLE)wParam;
	#if defined( _UNICODE )
		char** ppFiles = ( char** )lParam;
		int count = 0;
		while ( ppFiles[count] != NULL )
			count++;

		fsd.ppFiles = (const TCHAR**)alloca(( count+1 ) * sizeof( void* ));
		for ( int i=0; i < count; i++ )
			fsd.ppFiles[i] = ( const TCHAR* )mir_a2t( ppFiles[i] );
		fsd.ppFiles[ count ] = NULL;
	#else
		fsd.ppFiles=(const char**)lParam;
	#endif
	CreateDialogParam(hMirandaInst,MAKEINTRESOURCE(IDD_FILESEND),NULL,DlgProcSendFile,(LPARAM)&fsd);
	#if defined( _UNICODE )
		for ( int j=0; j < count; j++ )
			mir_free(( void* )fsd.ppFiles[j] );
	#endif
	return 0;
}

static INT_PTR SendSpecificFilesT(WPARAM wParam,LPARAM lParam)
{
	FileSendData fsd;
	fsd.hContact=(HANDLE)wParam;
	fsd.ppFiles=(const TCHAR**)lParam;
	CreateDialogParam(hMirandaInst,MAKEINTRESOURCE(IDD_FILESEND),NULL,DlgProcSendFile,(LPARAM)&fsd);
	return 0;
}

static INT_PTR GetReceivedFilesFolder(WPARAM wParam,LPARAM lParam)
{
  TCHAR buf[MAX_PATH];
	GetContactReceivedFilesDir((HANDLE)wParam,buf,MAX_PATH,TRUE);
  char* dir = mir_t2a(buf);
  lstrcpynA((char*)lParam,dir,MAX_PATH);
  mir_free(dir);
	return 0;
}

static INT_PTR RecvFileCommand(WPARAM, LPARAM lParam)
{
	CreateDialogParam(hMirandaInst,MAKEINTRESOURCE(IDD_FILERECV),NULL,DlgProcRecvFile,lParam);
	return 0;
}

void PushFileEvent( HANDLE hContact, HANDLE hdbe, LPARAM lParam )
{
	CLISTEVENT cle={0};
	cle.cbSize = sizeof(cle);
	cle.hContact = hContact;
	cle.hDbEvent = hdbe;
	cle.lParam = lParam;
	if ( DBGetContactSettingByte(NULL,"SRFile","AutoAccept",0) && !DBGetContactSettingByte(hContact,"CList","NotOnList",0)) {
		CreateDialogParam(hMirandaInst,MAKEINTRESOURCE(IDD_FILERECV),NULL,DlgProcRecvFile,(LPARAM)&cle);
	}
	else {
		SkinPlaySound("RecvFile");

		TCHAR szTooltip[256];
		mir_sntprintf(szTooltip,SIZEOF(szTooltip),TranslateT("File from %s"), cli.pfnGetContactDisplayName( hContact, 0 ));
		cle.ptszTooltip = szTooltip;

		cle.flags |= CLEF_TCHAR;
		cle.hIcon = LoadSkinIcon( SKINICON_EVENT_FILE );
		cle.pszService = "SRFile/RecvFile";
		CallService(MS_CLIST_ADDEVENT,0,(LPARAM)&cle);
}	}

static int FileEventAdded(WPARAM wParam,LPARAM lParam)
{
	DWORD dwSignature;

	DBEVENTINFO dbei={0};
	dbei.cbSize = sizeof(dbei);
	dbei.cbBlob = sizeof( DWORD );
	dbei.pBlob = ( PBYTE )&dwSignature;
	CallService( MS_DB_EVENT_GET, lParam, ( LPARAM )&dbei );
	if ( dbei.flags&(DBEF_SENT|DBEF_READ) || dbei.eventType != EVENTTYPE_FILE || dwSignature == 0 )
		return 0;

	PushFileEvent(( HANDLE )wParam, ( HANDLE )lParam, 0 );
	return 0;
}

int SRFile_GetRegValue(HKEY hKeyBase,const TCHAR *szSubKey,const TCHAR *szValue,TCHAR *szOutput,int cbOutput)
{
	HKEY hKey;
	DWORD cbOut=cbOutput;

	if ( RegOpenKeyEx( hKeyBase,szSubKey,0,KEY_QUERY_VALUE,&hKey ) != ERROR_SUCCESS)
		return 0;
	
	if ( RegQueryValueEx( hKey,szValue,NULL,NULL,(PBYTE)szOutput, &cbOut ) != ERROR_SUCCESS ) {
		RegCloseKey(hKey);
		return 0;
	}

	RegCloseKey(hKey);
	return 1;
}

void GetSensiblyFormattedSize(__int64 size,TCHAR *szOut,int cchOut,int unitsOverride,int appendUnits,int *unitsUsed)
{
	if(!unitsOverride) {
		if(size<1000) unitsOverride=UNITS_BYTES;
		else if(size<100*1024) unitsOverride=UNITS_KBPOINT1;
		else if(size<1024*1024) unitsOverride=UNITS_KBPOINT0;
		else if(size<1024*1024*1024) unitsOverride=UNITS_MBPOINT2;
    else unitsOverride=UNITS_GBPOINT3;
	}
	if(unitsUsed) *unitsUsed=unitsOverride;
	switch(unitsOverride) {
		case UNITS_BYTES: mir_sntprintf(szOut,cchOut,_T("%u%s%s"),(int)size,appendUnits?_T(" "):_T(""),appendUnits?TranslateT("bytes"):_T("")); break;
		case UNITS_KBPOINT1: mir_sntprintf(szOut,cchOut,_T("%.1lf%s"),size/1024.0,appendUnits?_T(" KB"):_T("")); break;
		case UNITS_KBPOINT0: mir_sntprintf(szOut,cchOut,_T("%u%s"),(int)(size/1024),appendUnits?_T(" KB"):_T("")); break;
		case UNITS_GBPOINT3: mir_sntprintf(szOut,cchOut,_T("%.3f%s"),(size >> 20)/1024.0,appendUnits?_T(" GB"):_T("")); break;
		default: mir_sntprintf(szOut,cchOut,_T("%.2lf%s"),size/1048576.0,appendUnits?_T(" MB"):_T("")); break;
	}
}

// Tripple redirection sucks but is needed to nullify the array pointer
void FreeFilesMatrix(TCHAR ***files)
{
	if (*files == NULL)
		return;

	// Free each filename in the pointer array
	TCHAR **pFile = *files;
	while (*pFile != NULL)
	{
		mir_free(*pFile);
		*pFile = NULL;
		pFile++;
	}

	// Free the array itself
	mir_free(*files);
	*files = NULL;
}

void FreeProtoFileTransferStatus(PROTOFILETRANSFERSTATUS *fts)
{
	mir_free(fts->tszCurrentFile);
	if(fts->ptszFiles) {
		for( int i=0;i<fts->totalFiles;i++) mir_free(fts->ptszFiles[i]);
		mir_free(fts->ptszFiles);
	}
	mir_free(fts->tszWorkingDir);
}

void CopyProtoFileTransferStatus(PROTOFILETRANSFERSTATUS *dest, PROTOFILETRANSFERSTATUS *src)
{
	*dest=*src;
	if ( src->tszCurrentFile ) dest->tszCurrentFile = PFTS_StringToTchar(src->flags, src->tszCurrentFile);
	if ( src->ptszFiles ) {
		dest->ptszFiles = (TCHAR**)mir_alloc(sizeof(TCHAR*)*src->totalFiles);
		for( int i=0; i < src->totalFiles; i++ )
			dest->ptszFiles[i] = PFTS_StringToTchar(src->flags, src->ptszFiles[i] );
	}
	if ( src->tszWorkingDir ) dest->tszWorkingDir = PFTS_StringToTchar(src->flags, src->tszWorkingDir );
	dest->flags &= ~PFTS_UTF;
	dest->flags |= PFTS_TCHAR;
}

void UpdateProtoFileTransferStatus(PROTOFILETRANSFERSTATUS *dest, PROTOFILETRANSFERSTATUS *src)
{
	if (src->cbSize == sizeof(PROTOFILETRANSFERSTATUS_V1))
	{
		PROTOFILETRANSFERSTATUS_V1 *src1 = (PROTOFILETRANSFERSTATUS_V1*)src;
		src = (PROTOFILETRANSFERSTATUS*)alloca(sizeof(PROTOFILETRANSFERSTATUS));

		src->cbSize               = sizeof(PROTOFILETRANSFERSTATUS);
		src->hContact             = src1->hContact;
		src->flags                = src1->sending ? PFTS_SENDING : 0;
		src->pszFiles             = src1->files;
		src->totalFiles           = src1->totalFiles;
		src->currentFileNumber    = src1->currentFileNumber;
		src->totalBytes           = src1->totalBytes;
		src->totalProgress        = src1->totalProgress;
		src->szWorkingDir         = src1->workingDir;
		src->szCurrentFile        = src1->currentFile;
		src->currentFileSize      = src1->currentFileSize;
		src->currentFileProgress  = src1->currentFileProgress;
		src->currentFileTime      = src1->currentFileTime;
	}

	dest->hContact = src->hContact;
	dest->flags = src->flags;
	if ( dest->totalFiles != src->totalFiles ) {
		for( int i=0;i<dest->totalFiles;i++) mir_free(dest->ptszFiles[i]);
		mir_free(dest->ptszFiles);
		dest->ptszFiles = NULL;
		dest->totalFiles = src->totalFiles;
	}
	if ( src->ptszFiles ) {
		if ( !dest->ptszFiles )
			dest->ptszFiles = ( TCHAR** )mir_calloc( sizeof(TCHAR*)*src->totalFiles);
		for ( int i=0; i < src->totalFiles; i++ )
			if ( !dest->ptszFiles[i] || !src->ptszFiles[i] || PFTS_CompareWithTchar( src, src->ptszFiles[i], dest->ptszFiles[i] )) {
				mir_free( dest->ptszFiles[i] );
				if ( src->ptszFiles[i] )
					dest->ptszFiles[i] = PFTS_StringToTchar( src->flags, src->ptszFiles[i] );
				else
					dest->ptszFiles[i] = NULL;
			}
	}
	else if (dest->ptszFiles) {
		for( int i=0; i < dest->totalFiles; i++ )
			mir_free(dest->ptszFiles[i]);
		mir_free( dest->ptszFiles );
		dest->ptszFiles = NULL;
	}

	dest->currentFileNumber = src->currentFileNumber;
	dest->totalBytes = src->totalBytes;
	dest->totalProgress = src->totalProgress;
	if (src->tszWorkingDir && (!dest->tszWorkingDir || PFTS_CompareWithTchar( src, src->tszWorkingDir, dest->tszWorkingDir))) {
		mir_free( dest->tszWorkingDir );
		if ( src->tszWorkingDir )
			dest->tszWorkingDir = PFTS_StringToTchar( src->flags, src->tszWorkingDir );
		else
			dest->tszWorkingDir = NULL;
	}

	if ( !dest->tszCurrentFile || !src->tszCurrentFile || PFTS_CompareWithTchar( src, src->tszCurrentFile, dest->tszCurrentFile )) {
		mir_free( dest->tszCurrentFile );
		if ( src->tszCurrentFile )
			dest->tszCurrentFile = PFTS_StringToTchar( src->flags, src->tszCurrentFile );
		else
			dest->tszCurrentFile = NULL;
	}
	dest->currentFileSize = src->currentFileSize;
	dest->currentFileProgress = src->currentFileProgress;
	dest->currentFileTime = src->currentFileTime;
  dest->flags &= ~PFTS_UTF;
  dest->flags |= PFTS_TCHAR;
}

static void RemoveUnreadFileEvents(void)
{
	DBEVENTINFO dbei={0};
	HANDLE hDbEvent,hContact;

	dbei.cbSize=sizeof(dbei);
	hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	while(hContact) {
		hDbEvent=(HANDLE)CallService(MS_DB_EVENT_FINDFIRSTUNREAD,(WPARAM)hContact,0);
		while(hDbEvent) {
			dbei.cbBlob=0;
			CallService(MS_DB_EVENT_GET,(WPARAM)hDbEvent,(LPARAM)&dbei);
			if(!(dbei.flags&(DBEF_SENT|DBEF_READ)) && dbei.eventType==EVENTTYPE_FILE)
				CallService(MS_DB_EVENT_MARKREAD,(WPARAM)hContact,(LPARAM)hDbEvent);
			hDbEvent=(HANDLE)CallService(MS_DB_EVENT_FINDNEXT,(WPARAM)hDbEvent,0);
		}
		hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0);
	}
}

static int SRFilePreBuildMenu(WPARAM wParam, LPARAM)
{
	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | CMIF_HIDDEN;

	char *szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
	if (szProto != NULL) {
		if ( CallProtoService(szProto, PS_GETCAPS,PFLAGNUM_1, 0 ) & PF1_FILESEND) {
			if ( CallProtoService(szProto, PS_GETCAPS,PFLAGNUM_4, 0 ) & PF4_OFFLINEFILES )
				mi.flags = CMIM_FLAGS;
			else if ( DBGetContactSettingWord(( HANDLE )wParam, szProto, "Status", ID_STATUS_OFFLINE ) != ID_STATUS_OFFLINE )
				mi.flags = CMIM_FLAGS;
	}	}

	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hSRFileMenuItem, (LPARAM)&mi);
	return 0;
}

static int SRFileModulesLoaded(WPARAM, LPARAM)
{
	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(mi);
	mi.position = -2000020000;
	mi.icolibItem = GetSkinIconHandle( SKINICON_EVENT_FILE );
	mi.pszName = LPGEN("&File");
	mi.pszService = MS_FILE_SENDFILE;
	mi.flags = CMIF_ICONFROMICOLIB;
	hSRFileMenuItem = ( HANDLE )CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	RemoveUnreadFileEvents();
	return 0;
}

INT_PTR FtMgrShowCommand(WPARAM, LPARAM)
{
	FtMgr_Show(true, true);
	return 0;
}

INT_PTR openContRecDir(WPARAM wparam, LPARAM)
{
	TCHAR szContRecDir[MAX_PATH];
	HANDLE hContact = (HANDLE)wparam;
	GetContactReceivedFilesDir(hContact, szContRecDir, SIZEOF(szContRecDir),TRUE);
	ShellExecute(0, _T("open"), szContRecDir, 0, 0, SW_SHOW);
	return 0;
}

INT_PTR openRecDir(WPARAM, LPARAM)
{
	TCHAR szContRecDir[MAX_PATH];
	GetReceivedFilesDir(szContRecDir, SIZEOF(szContRecDir));
	ShellExecute(0, _T("open"), szContRecDir, 0, 0, SW_SHOW);
	return 0;
}

int LoadSendRecvFileModule(void)
{
	CreateServiceFunction("FtMgr/Show", FtMgrShowCommand);

	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(mi);
	mi.flags = CMIF_ICONFROMICOLIB;
	mi.icolibItem = GetSkinIconHandle( SKINICON_EVENT_FILE );
	mi.position = 1900000000;
	mi.pszName = LPGEN("File &Transfers...");
	mi.pszService = "FtMgr/Show"; //MS_PROTO_SHOWFTMGR;
	CallService( MS_CLIST_ADDMAINMENUITEM, 0, ( LPARAM )&mi );

	HookEvent(ME_SYSTEM_MODULESLOADED,SRFileModulesLoaded);
	HookEvent(ME_DB_EVENT_ADDED,FileEventAdded);
	HookEvent(ME_OPT_INITIALISE,FileOptInitialise);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, SRFilePreBuildMenu);

	CreateServiceFunction(MS_FILE_SENDFILE,SendFileCommand);
	CreateServiceFunction(MS_FILE_SENDSPECIFICFILES,SendSpecificFiles);
	CreateServiceFunction(MS_FILE_SENDSPECIFICFILEST,SendSpecificFilesT);
	CreateServiceFunction(MS_FILE_GETRECEIVEDFILESFOLDER,GetReceivedFilesFolder);
	CreateServiceFunction("SRFile/RecvFile",RecvFileCommand);

	CreateServiceFunction("SRFile/OpenContRecDir",openContRecDir);
	CreateServiceFunction("SRFile/OpenRecDir",openRecDir);

	SkinAddNewSoundEx("RecvFile",   "File", "Incoming");
	SkinAddNewSoundEx("FileDone",   "File", "Complete");
	SkinAddNewSoundEx("FileFailed", "File", "Error");
	SkinAddNewSoundEx("FileDenied", "File", "Denied");
	return 0;
}
