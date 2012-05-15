/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2010 Miranda ICQ/IM project,
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
#include "file.h"

#define HM_RECVEVENT    (WM_USER+10)

static int CheckVirusScanned(HWND hwnd,struct FileDlgData *dat,int i)
{
	if(dat->send) return 1;
	if(dat->fileVirusScanned == NULL) return 0;
	if(dat->fileVirusScanned[i]) return 1;
	if(DBGetContactSettingByte(NULL,"SRFile","WarnBeforeOpening",1)==0) return 1;
	return IDYES==MessageBox(hwnd,TranslateT("This file has not yet been scanned for viruses. Are you certain you want to open it?"),TranslateT("File Received"),MB_YESNO|MB_DEFBUTTON2);
}

#define M_VIRUSSCANDONE  (WM_USER+100)
struct virusscanthreadstartinfo {
	TCHAR *szFile;
	int returnCode;
	HWND hwndReply;
};

TCHAR* PFTS_StringToTchar( int flags, const PROTOCHAR* s )
{
#ifdef  _UNICODE
	if ( flags & PFTS_UTF )
		return Utf8DecodeUcs2(( char* )s );
	else if ( flags & PFTS_UNICODE )
		return mir_tstrdup( s );
	else
		return mir_a2t(( char* )s );
#else
	if ( flags & PFTS_UTF ) {
		char *szAnsi = mir_strdup(( char* )s );
		return Utf8Decode(szAnsi, NULL);
	}
	else
		return mir_strdup( s );
#endif
}

int PFTS_CompareWithTchar( PROTOFILETRANSFERSTATUS* ft, const PROTOCHAR* s, TCHAR* r )
{
#ifdef _UNICODE
	if ( ft->flags & PFTS_UTF ) {
		TCHAR* ts = Utf8DecodeUcs2(( char* )s );
		int res = _tcscmp( ts, r );
		mir_free( ts );
		return res;
	}
	else if ( ft->flags & PFTS_UNICODE )
		return _tcscmp( s, r );
	else {
	  TCHAR* ts = mir_a2t(( char* )s );
	  int res = _tcscmp( ts, r );
	  mir_free( ts );
	  return res;
	}
#else
	if ( ft->flags & PFTS_UTF ) {
		char *ts = NEWSTR_ALLOCA(( char* )s );
		return _tcscmp( Utf8Decode(( char* )ts, NULL), r );
	}
	else
		return _tcscmp( s, r );
#endif
}

static void SetOpenFileButtonStyle(HWND hwndButton,int enabled)
{
	EnableWindow(hwndButton,enabled);
}

void FillSendData( FileDlgData* dat, DBEVENTINFO& dbei )
{
	dbei.cbSize = sizeof(dbei);
	dbei.szModule = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)dat->hContact,0);
	dbei.eventType = EVENTTYPE_FILE;
	dbei.flags = DBEF_SENT;
	dbei.timestamp = time(NULL);
	#if defined( _UNICODE )
		char *szFileNames = Utf8EncodeT(dat->szFilenames), *szMsg = Utf8EncodeT(dat->szMsg);
		dbei.flags |= DBEF_UTF;
	#else
		char *szFileNames = dat->szFilenames, *szMsg = dat->szMsg;
	#endif

	dbei.cbBlob = sizeof(DWORD) + lstrlenA(szFileNames)+lstrlenA(szMsg)+2;
	dbei.pBlob=(PBYTE)mir_alloc(dbei.cbBlob);
	*(PDWORD)dbei.pBlob=0;
	lstrcpyA((char*)dbei.pBlob+sizeof(DWORD),szFileNames);
	lstrcpyA((char*)dbei.pBlob+sizeof(DWORD)+lstrlenA(szFileNames)+1,szMsg);

	#if defined( _UNICODE )
		mir_free( szFileNames ), mir_free( szMsg );
	#endif
}

static void __cdecl RunVirusScannerThread(struct virusscanthreadstartinfo *info)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si={0};
	DBVARIANT dbv;
	TCHAR szCmdLine[768];

	if (!DBGetContactSettingTString(NULL,"SRFile", "ScanCmdLine", &dbv)) 
	{
		if(dbv.ptszVal[0]) 
		{
			TCHAR *pszReplace;
			si.cb=sizeof(si);
			pszReplace = _tcsstr(dbv.ptszVal, _T("%f"));
			if (pszReplace) 
			{
				if ( info->szFile[_tcslen(info->szFile) - 1] == '\\')
					info->szFile[_tcslen(info->szFile) - 1] = '\0';
				*pszReplace = 0;
				mir_sntprintf(szCmdLine, SIZEOF(szCmdLine), _T("%s\"%s\"%s"), dbv.ptszVal, info->szFile, pszReplace+2);
			}
			else lstrcpyn(szCmdLine, dbv.ptszVal, SIZEOF(szCmdLine));
			if(CreateProcess(NULL,szCmdLine,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi)) {
				if(WaitForSingleObject(pi.hProcess,3600*1000)==WAIT_OBJECT_0)
					PostMessage(info->hwndReply,M_VIRUSSCANDONE,info->returnCode,0);
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
			}
		}
		DBFreeVariant(&dbv);
	}
	mir_free(info->szFile);
	mir_free(info);
}

static void SetFilenameControls(HWND hwndDlg, struct FileDlgData *dat, PROTOFILETRANSFERSTATUS *fts)
{
	TCHAR msg[MAX_PATH];
	TCHAR *fnbuf = NULL, *fn = NULL;
	SHFILEINFO shfi = {0};

	if ( fts->tszCurrentFile ) {
		fnbuf = mir_tstrdup( fts->tszCurrentFile );
		if (( fn = _tcsrchr( fnbuf, '\\' )) == NULL )
			fn = fnbuf;
		else fn++;
	}

	if (dat->hIcon) DestroyIcon(dat->hIcon); dat->hIcon = NULL;

	if (fn && (fts->totalFiles > 1)) {
		mir_sntprintf(msg, SIZEOF(msg), _T("%s: %s (%d %s %d)"),
			cli.pfnGetContactDisplayName( fts->hContact, 0 ),
			fn, fts->currentFileNumber+1, TranslateT("of"), fts->totalFiles);

		SHGetFileInfo(fn, FILE_ATTRIBUTE_DIRECTORY, &shfi, sizeof(shfi), SHGFI_USEFILEATTRIBUTES|SHGFI_ICON|SHGFI_SMALLICON);
		dat->hIcon = shfi.hIcon;
	} 
	else if (fn) {
		mir_sntprintf(msg, SIZEOF(msg), _T("%s: %s"), cli.pfnGetContactDisplayName( fts->hContact, 0 ), fn);

		SHGetFileInfo(fn, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi), SHGFI_USEFILEATTRIBUTES|SHGFI_ICON|SHGFI_SMALLICON);
		dat->hIcon = shfi.hIcon;
	} 
	else {
		lstrcpyn(msg, cli.pfnGetContactDisplayName( fts->hContact, 0 ), SIZEOF(msg));
		HICON hIcon = LoadSkinIcon(SKINICON_OTHER_DOWNARROW);
		dat->hIcon = CopyIcon(hIcon);
		IconLib_ReleaseIcon(hIcon, NULL);
	}

	mir_free( fnbuf );
	
	SendDlgItemMessage(hwndDlg, IDC_FILEICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)dat->hIcon);
	SetDlgItemText(hwndDlg, IDC_CONTACTNAME, msg);
}

enum { FTS_TEXT, FTS_PROGRESS, FTS_OPEN };
static void SetFtStatus(HWND hwndDlg, TCHAR *text, int mode)
{
	SetDlgItemText(hwndDlg,IDC_STATUS,TranslateTS(text));
	SetDlgItemText(hwndDlg,IDC_TRANSFERCOMPLETED,TranslateTS(text));

	ShowWindow(GetDlgItem(hwndDlg,IDC_STATUS), (mode == FTS_TEXT)?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg,IDC_ALLFILESPROGRESS), (mode == FTS_PROGRESS)?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg,IDC_TRANSFERCOMPLETED), (mode == FTS_OPEN)?SW_SHOW:SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg,IDC_FILEICON), (mode == FTS_OPEN)?SW_SHOW:SW_HIDE);
}

static void HideProgressControls(HWND hwndDlg)
{
	RECT rc;
	char buf[64];

	GetWindowRect(GetDlgItem(hwndDlg, IDC_ALLPRECENTS), &rc);
	MapWindowPoints(NULL, hwndDlg, (LPPOINT)&rc, 2);
	SetWindowPos(hwndDlg, NULL, 0, 0, 100, rc.bottom+3, SWP_NOMOVE|SWP_NOZORDER);
	ShowWindow(GetDlgItem(hwndDlg, IDC_ALLTRANSFERRED), SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_ALLSPEED), SW_HIDE);

	_strtime(buf);
	SetDlgItemTextA(hwndDlg, IDC_ALLPRECENTS, buf);

	PostMessage(GetParent(hwndDlg), WM_FT_RESIZE, 0, (LPARAM)hwndDlg);
}

static int FileTransferDlgResizer(HWND, LPARAM, UTILRESIZECONTROL *urc)
{
	switch(urc->wId) {
		case IDC_CONTACTNAME:
		case IDC_STATUS:
		case IDC_ALLFILESPROGRESS:
		case IDC_TRANSFERCOMPLETED:
			return RD_ANCHORX_WIDTH|RD_ANCHORY_TOP;
		case IDC_FRAME:
			return RD_ANCHORX_WIDTH|RD_ANCHORY_BOTTOM;
		case IDC_ALLPRECENTS:
		case IDCANCEL:
		case IDC_OPENFILE:
		case IDC_OPENFOLDER:
			return RD_ANCHORX_RIGHT|RD_ANCHORY_TOP;

		case IDC_ALLTRANSFERRED:
			urc->rcItem.right = urc->rcItem.left + (urc->rcItem.right - urc->rcItem.left - urc->dlgOriginalSize.cx + urc->dlgNewSize.cx) / 3;
			return RD_ANCHORX_CUSTOM|RD_ANCHORY_CUSTOM;

		case IDC_ALLSPEED:
			urc->rcItem.right = urc->rcItem.right - urc->dlgOriginalSize.cx + urc->dlgNewSize.cx;
			urc->rcItem.left = urc->rcItem.left + (urc->rcItem.right - urc->rcItem.left) / 3;
			return RD_ANCHORX_CUSTOM|RD_ANCHORY_CUSTOM;
	}
	return RD_ANCHORX_LEFT|RD_ANCHORY_TOP;
}

INT_PTR CALLBACK DlgProcFileTransfer(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	FileDlgData *dat = (FileDlgData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg)
	{
		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);
			dat = (FileDlgData*)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			dat->hNotifyEvent=HookEventMessage(ME_PROTO_ACK,hwndDlg,HM_RECVEVENT);
			dat->transferStatus.currentFileNumber = -1;
			if(dat->send) {
				dat->fs=(HANDLE)CallContactService(dat->hContact,PSS_FILET,(WPARAM)dat->szMsg,(LPARAM)dat->files);
				SetFtStatus(hwndDlg, LPGENT("Request sent, waiting for acceptance..."), FTS_TEXT);
				SetOpenFileButtonStyle(GetDlgItem(hwndDlg,IDC_OPENFILE),1);
				dat->waitingForAcceptance=1;
				// hide "open" button since it may cause potential access violations...
				ShowWindow(GetDlgItem(hwndDlg, IDC_OPENFILE), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_OPENFOLDER), SW_HIDE);
			}
			else {	//recv
				CreateDirectoryTreeT(dat->szSavePath);
				dat->fs=(HANDLE)CallContactService(dat->hContact,PSS_FILEALLOWT,(WPARAM)dat->fs,(LPARAM)dat->szSavePath);
				dat->transferStatus.tszWorkingDir = mir_tstrdup(dat->szSavePath);
				if(DBGetContactSettingByte(dat->hContact,"CList","NotOnList",0)) dat->resumeBehaviour=FILERESUME_ASK;
				else dat->resumeBehaviour=DBGetContactSettingByte(NULL,"SRFile","IfExists",FILERESUME_ASK);
				SetFtStatus(hwndDlg, LPGENT("Waiting for connection..."), FTS_TEXT);
			}
			{
				/* check we actually got an fs handle back from the protocol */
				if (!dat->fs) {
					SetFtStatus(hwndDlg, LPGENT("Unable to initiate transfer."), FTS_TEXT);
					dat->waitingForAcceptance=0;
				}
			}
			{	LOGFONT lf;
				HFONT hFont;
				hFont=(HFONT)SendDlgItemMessage(hwndDlg,IDC_CONTACTNAME,WM_GETFONT,0,0);
				GetObject(hFont,sizeof(lf),&lf);
				lf.lfWeight=FW_BOLD;
				hFont=CreateFontIndirect(&lf);
				SendDlgItemMessage(hwndDlg,IDC_CONTACTNAME,WM_SETFONT,(WPARAM)hFont,0);
			}

			{	SHFILEINFO shfi = {0};
				SHGetFileInfo(_T(""), FILE_ATTRIBUTE_DIRECTORY, &shfi, sizeof(shfi), SHGFI_USEFILEATTRIBUTES|SHGFI_ICON|SHGFI_SMALLICON);
				dat->hIconFolder = shfi.hIcon;
			}

			dat->hIcon = NULL;

			SendDlgItemMessage(hwndDlg, IDC_CONTACT, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM)LoadSkinnedProtoIcon((char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)dat->hContact, 0), ID_STATUS_ONLINE));
			SendDlgItemMessage(hwndDlg, IDC_CONTACT, BUTTONADDTOOLTIP, (WPARAM)LPGEN("Contact menu"), 0);
			SendDlgItemMessage(hwndDlg, IDC_CONTACT, BUTTONSETASFLATBTN, 0, 0);

			Button_SetIcon_IcoLib(hwndDlg, IDC_OPENFILE, SKINICON_OTHER_DOWNARROW, LPGEN("Open..."));
			SendDlgItemMessage(hwndDlg, IDC_OPENFILE, BUTTONSETASPUSHBTN, 0, 0);

			SendDlgItemMessage(hwndDlg, IDC_OPENFOLDER, BM_SETIMAGE, IMAGE_ICON, (LPARAM)dat->hIconFolder);
			SendDlgItemMessage(hwndDlg, IDC_OPENFOLDER, BUTTONADDTOOLTIP, (WPARAM)LPGEN("Open folder"), 0);
			SendDlgItemMessage(hwndDlg, IDC_OPENFOLDER, BUTTONSETASFLATBTN, 0, 0);
		
			Button_SetIcon_IcoLib(hwndDlg, IDCANCEL, SKINICON_OTHER_DELETE, LPGEN("Cancel"));

			SetDlgItemText(hwndDlg, IDC_CONTACTNAME, cli.pfnGetContactDisplayName( dat->hContact, 0 ));

			if(!dat->waitingForAcceptance) SetTimer(hwndDlg,1,1000,NULL);
			return TRUE;
		case WM_TIMER:
			MoveMemory(dat->bytesRecvedHistory+1,dat->bytesRecvedHistory,sizeof(dat->bytesRecvedHistory)-sizeof(dat->bytesRecvedHistory[0]));
			dat->bytesRecvedHistory[0]=dat->transferStatus.totalProgress;
			if ( dat->bytesRecvedHistorySize < SIZEOF(dat->bytesRecvedHistory))
				dat->bytesRecvedHistorySize++;

			{	TCHAR szSpeed[32], szTime[32], szDisplay[96];
				SYSTEMTIME st;
				ULARGE_INTEGER li;
				FILETIME ft;

				GetSensiblyFormattedSize((dat->bytesRecvedHistory[0]-dat->bytesRecvedHistory[dat->bytesRecvedHistorySize-1])/dat->bytesRecvedHistorySize,szSpeed,SIZEOF(szSpeed),0,1,NULL);
				if(dat->bytesRecvedHistory[0]==dat->bytesRecvedHistory[dat->bytesRecvedHistorySize-1])
					lstrcpy(szTime,_T("??:??:??"));
				else {
					li.QuadPart=BIGI(10000000)*(dat->transferStatus.currentFileSize-dat->transferStatus.currentFileProgress)*dat->bytesRecvedHistorySize/(dat->bytesRecvedHistory[0]-dat->bytesRecvedHistory[dat->bytesRecvedHistorySize-1]);
					ft.dwHighDateTime=li.HighPart; ft.dwLowDateTime=li.LowPart;
					FileTimeToSystemTime(&ft,&st);
					GetTimeFormat(LOCALE_USER_DEFAULT,TIME_FORCE24HOURFORMAT|TIME_NOTIMEMARKER,&st,NULL,szTime,SIZEOF(szTime));
				}
				if(dat->bytesRecvedHistory[0]!=dat->bytesRecvedHistory[dat->bytesRecvedHistorySize-1]) {
					li.QuadPart=BIGI(10000000)*(dat->transferStatus.totalBytes-dat->transferStatus.totalProgress)*dat->bytesRecvedHistorySize/(dat->bytesRecvedHistory[0]-dat->bytesRecvedHistory[dat->bytesRecvedHistorySize-1]);
					ft.dwHighDateTime=li.HighPart; ft.dwLowDateTime=li.LowPart;
					FileTimeToSystemTime(&ft,&st);
					GetTimeFormat(LOCALE_USER_DEFAULT,TIME_FORCE24HOURFORMAT|TIME_NOTIMEMARKER,&st,NULL,szTime,SIZEOF(szTime));
				}

				mir_sntprintf(szDisplay,SIZEOF(szDisplay),_T("%s/%s  (%s %s)"),szSpeed,TranslateT("sec"),szTime,TranslateT("remaining"));
				SetDlgItemText(hwndDlg,IDC_ALLSPEED,szDisplay);
			}
			break;

		case WM_MEASUREITEM:
			return CallService(MS_CLIST_MENUMEASUREITEM,wParam,lParam);

		case WM_DRAWITEM:
			return CallService(MS_CLIST_MENUDRAWITEM,wParam,lParam);

		case WM_FT_CLEANUP:
			if (!dat->fs)
			{
				PostMessage(GetParent(hwndDlg), WM_FT_REMOVE, 0, (LPARAM)hwndDlg);
				DestroyWindow(hwndDlg);
			}
			break;

		case WM_COMMAND:
			if ( CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam),MPCF_CONTACTMENU), (LPARAM)dat->hContact ))
				break;

			switch (LOWORD(wParam)) 
			{
				case IDOK:
				case IDCANCEL:
					PostMessage(GetParent(hwndDlg), WM_FT_REMOVE, 0, (LPARAM)hwndDlg);
					DestroyWindow(hwndDlg);
					break;

				case IDC_CONTACT:
					{	RECT rc;
						HMENU hMenu=(HMENU)CallService(MS_CLIST_MENUBUILDCONTACT,(WPARAM)dat->hContact,0);
						GetWindowRect((HWND)lParam,&rc);
						TrackPopupMenu(hMenu,0,rc.left,rc.bottom,0,hwndDlg,NULL);
						DestroyMenu(hMenu);
						break;
					}

				case IDC_TRANSFERCOMPLETED:
					if (dat->transferStatus.currentFileNumber <= 1 && CheckVirusScanned(hwndDlg, dat, 0))
					{
						ShellExecute(NULL, NULL, dat->files[0], NULL, NULL, SW_SHOW);
						break;
					}

				case IDC_OPENFOLDER:
					if ( dat ) 
					{
						TCHAR* path = dat->transferStatus.tszWorkingDir;
						if (!path || !path[0])
						{
							path = NEWTSTR_ALLOCA(dat->transferStatus.tszCurrentFile);
							TCHAR* p = _tcsrchr(path, '\\'); if (p) *p = 0;
						}

						if (path) ShellExecute(NULL, _T("open"), path, NULL, NULL, SW_SHOW);
					}
					break;

				case IDC_OPENFILE:
				{
					TCHAR **files;
					HMENU hMenu;
					RECT rc;
					int ret;

					if (dat->send)
						if (dat->files == NULL)
							files = dat->transferStatus.ptszFiles;
						else
							files = dat->files;
					else
						files=dat->files;

					hMenu = CreatePopupMenu();
					AppendMenu(hMenu, MF_STRING, 1, TranslateT("Open folder"));
					AppendMenu(hMenu, MF_SEPARATOR, 0, 0);

					if (files && *files)
					{
						int i, limit;
						TCHAR *pszFilename,*pszNewFileName;

						if (dat->send)
							limit = dat->transferStatus.totalFiles;
						else
							limit = dat->transferStatus.currentFileNumber;

						// Loop over all transfered files and add them to the menu
						for (i = 0; i < limit; i++) {
							pszFilename = _tcsrchr(files[i], '\\');
							if (pszFilename == NULL)
								pszFilename = files[i];
							else
								pszFilename++;
							{
								if (pszFilename) {
									size_t cbFileNameLen = _tcslen(pszFilename);

									pszNewFileName = (TCHAR*)mir_alloc( cbFileNameLen*2*sizeof( TCHAR ));
									TCHAR *p = pszNewFileName;
									for (size_t pszlen=0; pszlen < cbFileNameLen; pszlen++) {
										*p++ = pszFilename[pszlen];
										if (pszFilename[pszlen]=='&')
											*p++ = '&';
									}
									*p = '\0';
									AppendMenu(hMenu, MF_STRING, i+10, pszNewFileName);
									mir_free(pszNewFileName);
								}
							}
						}
					}

					GetWindowRect((HWND)lParam, &rc);
					CheckDlgButton(hwndDlg, IDC_OPENFILE, BST_CHECKED);
					ret = TrackPopupMenu(hMenu, TPM_RETURNCMD|TPM_RIGHTALIGN, rc.right, rc.bottom, 0, hwndDlg, NULL);
					CheckDlgButton(hwndDlg, IDC_OPENFILE, BST_UNCHECKED);
					DestroyMenu(hMenu);

					if (ret == 1)
					{
						TCHAR* path = dat->transferStatus.tszWorkingDir;
						if (!path || !path[0])
						{
							path = NEWTSTR_ALLOCA(dat->transferStatus.tszCurrentFile);
							TCHAR* p = _tcsrchr(path, '\\'); if (p) *p = 0;
						}

						if (path) ShellExecute(NULL, _T("open"), path, NULL, NULL, SW_SHOW);
					}
					else if (ret && CheckVirusScanned(hwndDlg, dat, ret))
						ShellExecute(NULL, NULL, files[ret-10], NULL, NULL, SW_SHOW);

					break;
				}
			}
			break;
		case M_FILEEXISTSDLGREPLY:
		{	PROTOFILERESUME *pfr=(PROTOFILERESUME*)lParam;
			TCHAR *szOriginalFilename=(TCHAR*)wParam;
			char *szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)dat->hContact,0);

			EnableWindow(hwndDlg,TRUE);
			switch(pfr->action) {
				case FILERESUME_CANCEL:
					if (dat->fs) CallContactService(dat->hContact,PSS_FILECANCEL,(WPARAM)dat->fs,0);
					dat->fs=NULL;
					mir_free(szOriginalFilename);
					if(pfr->szFilename) mir_free((char*)pfr->szFilename);
					mir_free(pfr);
					return 0;
				case FILERESUME_RESUMEALL:
				case FILERESUME_OVERWRITEALL:
					dat->resumeBehaviour=pfr->action;
					pfr->action&=~FILERESUMEF_ALL;
					break;
				case FILERESUME_RENAMEALL:
					pfr->action=FILERESUME_RENAME;
					{	TCHAR *pszExtension,*pszFilename;
						int i;
						if((pszFilename = _tcsrchr(szOriginalFilename,'\\'))==NULL) pszFilename=szOriginalFilename;
						if((pszExtension = _tcsrchr(pszFilename+1,'.'))==NULL) pszExtension=pszFilename+lstrlen(pszFilename);
						if(pfr->szFilename) mir_free((TCHAR*)pfr->szFilename);
						pfr->szFilename = (TCHAR*)mir_alloc(sizeof(TCHAR)*((pszExtension-szOriginalFilename)+21+lstrlen(pszExtension)));
						for(i=1;;i++) {
							_stprintf((TCHAR*)pfr->szFilename,_T("%.*s (%u)%s"),pszExtension-szOriginalFilename,szOriginalFilename,i,pszExtension);
							if(_taccess(pfr->szFilename,0)!=0)
								break;
						}
					}
					break;
			}
			mir_free(szOriginalFilename);
			CallProtoService(szProto,PS_FILERESUMET,(WPARAM)dat->fs,(LPARAM)pfr);
			if(pfr->szFilename) mir_free((char*)pfr->szFilename);
			mir_free(pfr);
			break;
		}
		case HM_RECVEVENT:
		{	ACKDATA *ack=(ACKDATA*)lParam;
			if (ack->hProcess!=dat->fs) break; /* icq abuses this sometimes */
			if(ack->hContact!=dat->hContact) break;
			if(ack->type!=ACKTYPE_FILE) break;

			if(dat->waitingForAcceptance) {
				SetTimer(hwndDlg,1,1000,NULL);
				dat->waitingForAcceptance=0;
			}
			switch(ack->result) {
				case ACKRESULT_SENTREQUEST: SetFtStatus(hwndDlg, LPGENT("Decision sent"), FTS_TEXT); break;
				case ACKRESULT_CONNECTING: SetFtStatus(hwndDlg, LPGENT("Connecting..."), FTS_TEXT); break;
				case ACKRESULT_CONNECTPROXY: SetFtStatus(hwndDlg, LPGENT("Connecting to proxy..."), FTS_TEXT); break;
				case ACKRESULT_CONNECTED: SetFtStatus(hwndDlg, LPGENT("Connected"), FTS_TEXT); break;
				case ACKRESULT_LISTENING: SetFtStatus(hwndDlg, LPGENT("Waiting for connection..."), FTS_TEXT); break;
				case ACKRESULT_INITIALISING: SetFtStatus(hwndDlg, LPGENT("Initialising..."), FTS_TEXT); break;
				case ACKRESULT_NEXTFILE:
					SetFtStatus(hwndDlg, LPGENT("Moving to next file..."), FTS_TEXT);
					SetDlgItemTextA(hwndDlg,IDC_FILENAME,"");
					if(dat->transferStatus.currentFileNumber==1 && dat->transferStatus.totalFiles>1 && !dat->send)
						SetOpenFileButtonStyle(GetDlgItem(hwndDlg,IDC_OPENFILE),1);
					if(dat->transferStatus.currentFileNumber!=-1 && dat->files && !dat->send && DBGetContactSettingByte(NULL,"SRFile","UseScanner",VIRUSSCAN_DISABLE)==VIRUSSCAN_DURINGDL) {
						if(GetFileAttributes(dat->files[dat->transferStatus.currentFileNumber])&FILE_ATTRIBUTE_DIRECTORY)
							PostMessage(hwndDlg,M_VIRUSSCANDONE,dat->transferStatus.currentFileNumber,0);
						else {
							virusscanthreadstartinfo *vstsi;
							vstsi=(struct virusscanthreadstartinfo*)mir_alloc(sizeof(struct virusscanthreadstartinfo));
							vstsi->hwndReply = hwndDlg;
							vstsi->szFile = mir_tstrdup(dat->files[dat->transferStatus.currentFileNumber]);
							vstsi->returnCode = dat->transferStatus.currentFileNumber;
							forkthread((void (*)(void*))RunVirusScannerThread,0,vstsi);
						}
					}
					break;
				case ACKRESULT_FILERESUME:
				{
					UpdateProtoFileTransferStatus(&dat->transferStatus, (PROTOFILETRANSFERSTATUS*)ack->lParam);
					PROTOFILETRANSFERSTATUS *fts = &dat->transferStatus; 

					SetFilenameControls( hwndDlg, dat, fts );
					int res = _taccess( fts->tszCurrentFile, 0 );
					if ( res )
						break;

					SetFtStatus(hwndDlg, LPGENT("File already exists"), FTS_TEXT);
					if(dat->resumeBehaviour==FILERESUME_ASK) {
						TDlgProcFileExistsParam param = { hwndDlg, fts };
						ShowWindow(hwndDlg,SW_SHOWNORMAL);
						CreateDialogParam(hMirandaInst,MAKEINTRESOURCE(IDD_FILEEXISTS),hwndDlg,DlgProcFileExists,(LPARAM)&param);
						EnableWindow(hwndDlg,FALSE);
					}
					else {
						PROTOFILERESUME *pfr;
						pfr=(PROTOFILERESUME*)mir_alloc(sizeof(PROTOFILERESUME));
						pfr->action = dat->resumeBehaviour;
						pfr->szFilename = NULL;
						PostMessage(hwndDlg,M_FILEEXISTSDLGREPLY,(WPARAM)mir_tstrdup(fts->tszCurrentFile),(LPARAM)pfr);
					}
					SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,1);
					return TRUE;
				}
				case ACKRESULT_DATA:
				{
					PROTOFILETRANSFERSTATUS *fts=(PROTOFILETRANSFERSTATUS*)ack->lParam;
					TCHAR str[64], str2[64], szSizeDone[32], szSizeTotal[32];//,*contactName;
					int units;

					if ( dat->fileVirusScanned==NULL )
						dat->fileVirusScanned=(int*)mir_calloc(sizeof(int) * fts->totalFiles);

					// This needs to be here - otherwise we get holes in the files array
					if (!dat->send) 
					{
						if (dat->files == NULL)
							dat->files = (TCHAR**)mir_calloc((fts->totalFiles + 1) * sizeof(TCHAR*));
						if (fts->currentFileNumber < fts->totalFiles && dat->files[fts->currentFileNumber] == NULL) 
						{
							if (fts->cbSize == sizeof(PROTOFILETRANSFERSTATUS_V1))
							{
								PROTOFILETRANSFERSTATUS_V1 *fts1 = (PROTOFILETRANSFERSTATUS_V1*)fts;
								dat->files[fts->currentFileNumber] = PFTS_StringToTchar(0, (PROTOCHAR*)fts1->currentFile);
							}
							else
								dat->files[fts->currentFileNumber] = PFTS_StringToTchar(fts->flags, fts->tszCurrentFile);
						}
					}

					/* HACK: for 0.3.3, limit updates to around 1.1 ack per second */
					if (fts->totalProgress != fts->totalBytes && GetTickCount() < (dat->dwTicks + 650)) break; // the last update was less than a second ago!
					dat->dwTicks = GetTickCount();

					// Update local transfer status with data from protocol
					UpdateProtoFileTransferStatus(&dat->transferStatus, fts);
					fts = &dat->transferStatus;

					bool firstTime = false;
					if ((GetWindowLong(GetDlgItem(hwndDlg, IDC_ALLFILESPROGRESS), GWL_STYLE) & WS_VISIBLE) == 0)
					{
						SetFtStatus(hwndDlg, ( fts->flags & PFTS_SENDING ) ? LPGENT("Sending...") : LPGENT("Receiving..."), FTS_PROGRESS);
						SetFilenameControls(hwndDlg, dat, fts);
						firstTime = true;
					}

					const unsigned long lastPos = SendDlgItemMessage(hwndDlg, IDC_ALLFILESPROGRESS, PBM_GETPOS, 0, 0);
					const unsigned long nextPos = fts->totalBytes ? (BIGI(100) * fts->totalProgress / fts->totalBytes) : 0;
					if (lastPos != nextPos || firstTime)
					{
						SendDlgItemMessage(hwndDlg, IDC_ALLFILESPROGRESS, PBM_SETPOS, nextPos, 0);
						mir_sntprintf(str, SIZEOF(str), _T("%u%%"), nextPos);
						SetDlgItemText(hwndDlg, IDC_ALLPRECENTS, str);
					}

					GetSensiblyFormattedSize(fts->totalBytes, szSizeTotal, SIZEOF(szSizeTotal), 0, 1, &units);
					GetSensiblyFormattedSize(fts->totalProgress, szSizeDone, SIZEOF(szSizeDone), units, 0, NULL);
					mir_sntprintf(str, SIZEOF(str), _T("%s/%s"), szSizeDone, szSizeTotal);
					str2[0] = 0;
					GetDlgItemText(hwndDlg, IDC_ALLTRANSFERRED, str2, SIZEOF(str2));
					if (_tcscmp(str, str2)) SetDlgItemText(hwndDlg, IDC_ALLTRANSFERRED, str);
					break;
				}

				case ACKRESULT_SUCCESS:
				case ACKRESULT_FAILED:
				case ACKRESULT_DENIED:
				{

					HideProgressControls(hwndDlg);
					KillTimer(hwndDlg,1);
					if (!dat->send)
						SetOpenFileButtonStyle(GetDlgItem(hwndDlg,IDC_OPENFILE),1);
					SetDlgItemText(hwndDlg,IDCANCEL,TranslateT("Close"));
					if (dat->hNotifyEvent) 
						UnhookEvent(dat->hNotifyEvent);
					dat->hNotifyEvent=NULL;
					
					if (ack->result == ACKRESULT_DENIED)
					{
						dat->fs=NULL; /* protocol will free structure */
						SkinPlaySound("FileDenied");
						SetFtStatus(hwndDlg, LPGENT("File transfer denied"), FTS_TEXT);
					} else if (ack->result == ACKRESULT_FAILED)
					{
						dat->fs=NULL; /* protocol will free structure */
						SkinPlaySound("FileFailed");
						SetFtStatus(hwndDlg, LPGENT("File transfer failed"), FTS_TEXT);
					} else {
						SkinPlaySound("FileDone");
						if (dat->send)
						{
							dat->fs=NULL; /* protocol will free structure */
							SetFtStatus(hwndDlg, LPGENT("Transfer completed."), FTS_TEXT);

							DBEVENTINFO dbei={0};
							FillSendData( dat, dbei );
							CallService(MS_DB_EVENT_ADD,(WPARAM)dat->hContact,(LPARAM)&dbei);
							if (dbei.pBlob)
								mir_free(dbei.pBlob);
							dat->files=NULL;   //protocol library frees this
						
						} else {
							SetFtStatus(hwndDlg,
								(dat->transferStatus.totalFiles == 1) ?
								LPGENT("Transfer completed, open file.") :
								LPGENT("Transfer completed, open folder."),
								FTS_OPEN);

							int useScanner=DBGetContactSettingByte(NULL,"SRFile","UseScanner",VIRUSSCAN_DISABLE);
							if (useScanner!=VIRUSSCAN_DISABLE) {
								struct virusscanthreadstartinfo *vstsi;
								vstsi=(struct virusscanthreadstartinfo*)mir_alloc(sizeof(struct virusscanthreadstartinfo));
								vstsi->hwndReply=hwndDlg;
								if(useScanner==VIRUSSCAN_DURINGDL) {
									vstsi->returnCode=dat->transferStatus.currentFileNumber;
									if ( GetFileAttributes(dat->files[dat->transferStatus.currentFileNumber])&FILE_ATTRIBUTE_DIRECTORY) {
										PostMessage(hwndDlg,M_VIRUSSCANDONE,vstsi->returnCode,0);
										mir_free(vstsi);
										vstsi=NULL;
									}
									else vstsi->szFile = mir_tstrdup(dat->files[dat->transferStatus.currentFileNumber]);
								}
								else {
									vstsi->szFile = mir_tstrdup(dat->transferStatus.tszWorkingDir);
									vstsi->returnCode = -1;
								}
								SetFtStatus(hwndDlg, LPGENT("Scanning for viruses..."), FTS_TEXT);
								if(vstsi) forkthread((void (*)(void*))RunVirusScannerThread,0,vstsi);
							} else {
								dat->fs=NULL; /* protocol will free structure */
							}
							dat->transferStatus.currentFileNumber=dat->transferStatus.totalFiles;
						} // else dat->send
						
					} // else ack->result
					
					PostMessage(GetParent(hwndDlg), WM_FT_COMPLETED, ack->result, (LPARAM)hwndDlg);
					break;
			}
			break;
		} // switch ack->result
		} break; // case HM_RECVEVENT
		case M_VIRUSSCANDONE:
		{	int done=1,i;
			if((int)wParam==-1) {
				for(i=0;i<dat->transferStatus.totalFiles;i++) dat->fileVirusScanned[i]=1;
			}
			else {
				dat->fileVirusScanned[wParam]=1;
				for(i=0;i<dat->transferStatus.totalFiles;i++) if(!dat->fileVirusScanned[i]) {done=0; break;}
			}
			if (done)
			{
				dat->fs=NULL; /* protocol will free structure */
				SetFtStatus(hwndDlg, LPGENT("Transfer and virus scan complete"), FTS_TEXT);
			}
			break;
		}
		case WM_SIZE:
		{
			UTILRESIZEDIALOG urd={0};
			urd.cbSize=sizeof(urd);
			urd.hwndDlg=hwndDlg;
			urd.hInstance=hMirandaInst;
			urd.lpTemplate=MAKEINTRESOURCEA(IDD_FILETRANSFERINFO);
			urd.pfnResizer=FileTransferDlgResizer;
			CallService(MS_UTILS_RESIZEDIALOG,0,(LPARAM)&urd);

			RedrawWindow(GetDlgItem(hwndDlg, IDC_ALLTRANSFERRED), NULL, NULL, RDW_INVALIDATE|RDW_NOERASE);
			RedrawWindow(GetDlgItem(hwndDlg, IDC_ALLSPEED), NULL, NULL, RDW_INVALIDATE|RDW_NOERASE);
			RedrawWindow(GetDlgItem(hwndDlg, IDC_CONTACTNAME), NULL, NULL, RDW_INVALIDATE|RDW_NOERASE);
			RedrawWindow(GetDlgItem(hwndDlg, IDC_STATUS), NULL, NULL, RDW_INVALIDATE|RDW_NOERASE);
			break;
		}
		case WM_DESTROY:
			KillTimer(hwndDlg, 1);

			HFONT hFont = (HFONT)SendDlgItemMessage(hwndDlg,IDC_CONTACTNAME,WM_GETFONT,0,0);
			DeleteObject(hFont);

			Button_FreeIcon_IcoLib(hwndDlg, IDC_CONTACT);
			Button_FreeIcon_IcoLib(hwndDlg, IDC_OPENFILE);
			Button_FreeIcon_IcoLib(hwndDlg, IDCANCEL);

			FreeFileDlgData(dat);
			break;
	}
	return FALSE;
}

void FreeFileDlgData( FileDlgData* dat )
{
	if(dat->fs)
		CallContactService(dat->hContact,PSS_FILECANCEL,(WPARAM)dat->fs,0);
	dat->fs = NULL;

	if (dat->hPreshutdownEvent) UnhookEvent(dat->hPreshutdownEvent);
	if (dat->hNotifyEvent) UnhookEvent(dat->hNotifyEvent);
	dat->hNotifyEvent = NULL;

	FreeProtoFileTransferStatus(&dat->transferStatus);
	FreeFilesMatrix(&dat->files);

	mir_free(dat->fileVirusScanned);
	Safe_DestroyIcon(dat->hIcon);
	Safe_DestroyIcon(dat->hIconFolder);
	mir_free(dat);
}
