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

#define MAX_MRU_DIRS    5

static BOOL CALLBACK ClipSiblingsChildEnumProc(HWND hwnd, LPARAM)
{
	SetWindowLongPtr(hwnd,GWL_STYLE,GetWindowLongPtr(hwnd,GWL_STYLE)|WS_CLIPSIBLINGS);
	return TRUE;
}

static void GetLowestExistingDirName(const TCHAR *szTestDir,TCHAR *szExistingDir,int cchExistingDir)
{
	DWORD dwAttributes;
	TCHAR *pszLastBackslash;

	lstrcpyn(szExistingDir,szTestDir,cchExistingDir);
	while((dwAttributes=GetFileAttributes(szExistingDir))!=INVALID_FILE_ATTRIBUTES && !(dwAttributes&FILE_ATTRIBUTE_DIRECTORY)) {
		pszLastBackslash=_tcsrchr(szExistingDir,'\\');
		if(pszLastBackslash==NULL) {*szExistingDir='\0'; break;}
		*pszLastBackslash='\0';
	}
	if(szExistingDir[0]=='\0') GetCurrentDirectory(cchExistingDir,szExistingDir);
}

static const TCHAR InvalidFilenameChars[] = _T("\\/:*?\"<>|"); 
void RemoveInvalidFilenameChars(TCHAR *tszString)
{
	size_t i;
	if (tszString) {
		for(i=_tcscspn(tszString,InvalidFilenameChars); tszString[i]; i+=_tcscspn(tszString+i+1,InvalidFilenameChars)+1)
			if(tszString[i] >= 0) 
				tszString[i] = _T('_');
	}
}

static const TCHAR InvalidPathChars[] = _T("*?\"<>|"); // "\/:" are excluded as they are allowed in file path
void RemoveInvalidPathChars(TCHAR *tszString)
{
	size_t i;
	if (tszString) {
		for(i=_tcscspn(tszString,InvalidPathChars); tszString[i]; i+=_tcscspn(tszString+i+1,InvalidPathChars)+1)
			if(tszString[i] >= 0) 
				tszString[i] = _T('_');
	}
}

static INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
	TCHAR szDir[MAX_PATH];
	switch(uMsg) {
	case BFFM_INITIALIZED:
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
		break;
	case BFFM_SELCHANGED:
		if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir))
			SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
		break;
	}
	return 0;
}

int BrowseForFolder(HWND hwnd,TCHAR *szPath)
{
	BROWSEINFO bi={0};
	LPITEMIDLIST pidlResult;

	bi.hwndOwner=hwnd;
	bi.pszDisplayName=szPath;
	bi.lpszTitle=TranslateT("Select Folder");
	bi.ulFlags=BIF_NEWDIALOGSTYLE|BIF_EDITBOX|BIF_RETURNONLYFSDIRS;				// Use this combo instead of BIF_USENEWUI
	bi.lpfn=BrowseCallbackProc;
	bi.lParam=(LPARAM)szPath;

	pidlResult=SHBrowseForFolder(&bi);
	if(pidlResult) {
		SHGetPathFromIDList(pidlResult,szPath);
		lstrcat(szPath,_T("\\"));
		CoTaskMemFree(pidlResult);
	}
	return pidlResult != NULL;
}

static REPLACEVARSARRAY sttVarsToReplace[] =
{
	{ ( TCHAR* )"///", ( TCHAR* )"//" },
	{ ( TCHAR* )"//", ( TCHAR* )"/" },
	{ ( TCHAR* )"()", ( TCHAR* )"" },
	{ NULL, NULL }
};

static void patchDir( TCHAR* str, size_t strSize )
{
	REPLACEVARSDATA dat = { 0 };
	dat.cbSize = sizeof( dat );
	dat.dwFlags = RVF_TCHAR;
	dat.variables = sttVarsToReplace;

	TCHAR* result = ( TCHAR* )CallService( MS_UTILS_REPLACEVARS, (WPARAM)str, (LPARAM)&dat );
	if ( result ) {
		_tcsncpy( str, result, strSize );
		mir_free( result );
	}

	size_t len = lstrlen( str );
	if ( len+1 < strSize && str[len-1] != '\\' )
		lstrcpy( str+len, _T("\\") );
}

void GetContactReceivedFilesDir(HANDLE hContact, TCHAR *szDir, int cchDir, BOOL patchVars)
{
	DBVARIANT dbv;
	TCHAR szTemp[MAX_PATH];
	szTemp[0] = 0;

	if ( !DBGetContactSettingTString( NULL, "SRFile", "RecvFilesDirAdv", &dbv)) {
		if ( lstrlen( dbv.ptszVal ) > 0 )
			lstrcpyn( szTemp, dbv.ptszVal, SIZEOF( szTemp ));
		DBFreeVariant( &dbv );
	}

	if ( !szTemp[0] )
#ifdef _UNICODE
		mir_sntprintf( szTemp, SIZEOF(szTemp), _T("%%mydocuments%%\\%s\\%%userid%%"), TranslateT("My Received Files"));
#else
		mir_sntprintf( szTemp, SIZEOF(szTemp), _T("%%mydocuments%%\\%s\\%%userid%%"), "My Received Files");
#endif

	if ( hContact ) {
		REPLACEVARSDATA dat = { 0 };
		REPLACEVARSARRAY rvaVarsToReplace[4];
		rvaVarsToReplace[0].lptzKey   = _T("nick");
		rvaVarsToReplace[0].lptzValue = mir_tstrdup((TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR));
		rvaVarsToReplace[1].lptzKey   = _T("userid");
		rvaVarsToReplace[1].lptzValue = GetContactID(hContact);
		rvaVarsToReplace[2].lptzKey   = _T("proto");
		rvaVarsToReplace[2].lptzValue = mir_a2t((char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact,0));
		rvaVarsToReplace[3].lptzKey   = NULL;
		rvaVarsToReplace[3].lptzValue = NULL;
		for (int i=0; i < (SIZEOF(rvaVarsToReplace)-1);i++)
			RemoveInvalidFilenameChars(rvaVarsToReplace[i].lptzValue);

		dat.cbSize = sizeof( dat );
		dat.dwFlags = RVF_TCHAR;
		dat.variables = rvaVarsToReplace;
		dat.hContact = hContact;
		TCHAR* result = ( TCHAR* )CallService( MS_UTILS_REPLACEVARS, (WPARAM)szTemp, (LPARAM)&dat );
		if ( result ) {
			_tcsncpy( szTemp, result, SIZEOF(szTemp));
			mir_free( result );
			for (int i=0; i < (SIZEOF(rvaVarsToReplace)-1);i++)
				mir_free(rvaVarsToReplace[i].lptzValue);
	}	}

	if (patchVars)
		patchDir( szTemp, SIZEOF(szTemp));
	RemoveInvalidPathChars(szTemp);
	lstrcpyn( szDir, szTemp, cchDir );
}

void GetReceivedFilesDir(TCHAR *szDir, int cchDir)
{
	DBVARIANT dbv;
	TCHAR szTemp[MAX_PATH];
	szTemp[0] = 0;

	if ( !DBGetContactSettingTString( NULL, "SRFile", "RecvFilesDirAdv", &dbv )) {
		if ( lstrlen( dbv.ptszVal ) > 0 )
			lstrcpyn( szTemp, dbv.ptszVal, SIZEOF( szTemp ));
		DBFreeVariant(&dbv);
	}

	if ( !szTemp[0] )
#ifdef _UNICODE
		mir_sntprintf( szTemp, SIZEOF(szTemp), _T("%%mydocuments%%\\%s"), TranslateT("My Received Files"));
#else
		mir_sntprintf( szTemp, SIZEOF(szTemp), _T("%%mydocuments%%\\%s"), "My Received Files");
#endif

	patchDir( szTemp, SIZEOF(szTemp));
	RemoveInvalidPathChars(szTemp);
	lstrcpyn( szDir, szTemp, cchDir );
}

INT_PTR CALLBACK DlgProcRecvFile(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct FileDlgData *dat;

	dat=(struct FileDlgData*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG: {
		TCHAR *contactName;
		TCHAR szPath[450];
		CLISTEVENT* cle = (CLISTEVENT*)lParam;

		TranslateDialogDefault(hwndDlg);

		dat=(struct FileDlgData*)mir_calloc(sizeof(struct FileDlgData));
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,(LONG_PTR)dat);
		dat->hContact = cle->hContact;
		dat->hDbEvent = cle->hDbEvent;
		dat->hPreshutdownEvent = HookEventMessage(ME_SYSTEM_PRESHUTDOWN,hwndDlg,M_PRESHUTDOWN);
		dat->dwTicks = GetTickCount();

		EnumChildWindows(hwndDlg,ClipSiblingsChildEnumProc,0);

		Window_SetIcon_IcoLib(hwndDlg, SKINICON_EVENT_FILE);
		Button_SetIcon_IcoLib(hwndDlg, IDC_ADD, SKINICON_OTHER_ADDCONTACT, LPGEN("Add Contact Permanently to List"));
		Button_SetIcon_IcoLib(hwndDlg, IDC_DETAILS, SKINICON_OTHER_USERDETAILS, LPGEN("View User's Details"));
		Button_SetIcon_IcoLib(hwndDlg, IDC_HISTORY, SKINICON_OTHER_HISTORY, LPGEN("View User's History"));
		Button_SetIcon_IcoLib(hwndDlg, IDC_USERMENU, SKINICON_OTHER_DOWNARROW, LPGEN("User Menu"));

		contactName = cli.pfnGetContactDisplayName( dat->hContact, 0 );
		SetDlgItemText(hwndDlg,IDC_FROM,contactName);
		GetContactReceivedFilesDir(dat->hContact,szPath,SIZEOF(szPath),TRUE);
		SetDlgItemText(hwndDlg,IDC_FILEDIR,szPath);
		{
			int i;
			char idstr[32];
			DBVARIANT dbv;

			if (shAutoComplete)
				shAutoComplete(GetWindow(GetDlgItem(hwndDlg,IDC_FILEDIR),GW_CHILD),1);

			for(i=0;i<MAX_MRU_DIRS;i++) {
				mir_snprintf(idstr, SIZEOF(idstr), "MruDir%d",i);
				if(DBGetContactSettingTString(NULL,"SRFile",idstr,&dbv)) break;
				SendDlgItemMessage(hwndDlg,IDC_FILEDIR,CB_ADDSTRING,0,(LPARAM)dbv.ptszVal);
				DBFreeVariant(&dbv);
			}
		}

		CallService(MS_DB_EVENT_MARKREAD,(WPARAM)dat->hContact,(LPARAM)dat->hDbEvent);
		{
			DBEVENTINFO dbei={0};
			TCHAR datetimestr[64];
			char buf[540];

			dbei.cbSize=sizeof(dbei);
			dbei.cbBlob=CallService(MS_DB_EVENT_GETBLOBSIZE,(WPARAM)dat->hDbEvent,0);
			dbei.pBlob=(PBYTE)mir_alloc(dbei.cbBlob);
			CallService(MS_DB_EVENT_GET,(WPARAM)dat->hDbEvent,(LPARAM)&dbei);
			dat->fs = cle->lParam ? (HANDLE)cle->lParam : (HANDLE)*(PDWORD)dbei.pBlob;
			lstrcpynA(buf, (char*)dbei.pBlob+4, min(dbei.cbBlob+1,SIZEOF(buf)));
			TCHAR* ptszFileName = DbGetEventStringT( &dbei, buf );
			SetDlgItemText(hwndDlg,IDC_FILENAMES,ptszFileName);
			mir_free(ptszFileName);
			lstrcpynA(buf, (char*)dbei.pBlob+4+strlen((char*)dbei.pBlob+4)+1, min((int)(dbei.cbBlob-4-strlen((char*)dbei.pBlob+4)),SIZEOF(buf)));
			TCHAR* ptszDescription = DbGetEventStringT( &dbei, buf );
			SetDlgItemText(hwndDlg,IDC_MSG,ptszDescription);
			mir_free(ptszDescription);
			mir_free(dbei.pBlob);

			tmi.printTimeStamp(NULL, dbei.timestamp, _T("t d"), datetimestr, SIZEOF(datetimestr), 0);
			SetDlgItemText(hwndDlg, IDC_DATE, datetimestr);
		}
		{
			char* szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)dat->hContact, 0);
			if (szProto) {
				CONTACTINFO ci;
				int hasName = 0;
				char buf[128];
				ZeroMemory(&ci,sizeof(ci));

				ci.cbSize = sizeof(ci);
				ci.hContact = dat->hContact;
				ci.szProto = szProto;
				ci.dwFlag = CNF_UNIQUEID;
				if (!CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&ci)) {
					switch(ci.type) {
					case CNFT_ASCIIZ:
						hasName = 1;
						mir_snprintf(buf, SIZEOF(buf), "%s", ci.pszVal);
						mir_free(ci.pszVal);
						break;
					case CNFT_DWORD:
						hasName = 1;
						mir_snprintf(buf, SIZEOF(buf),"%u",ci.dVal);
						break;
				}	}
				if (hasName)
					SetDlgItemTextA(hwndDlg, IDC_NAME, buf );
				else
					SetDlgItemText(hwndDlg, IDC_NAME, contactName);
		}	}

		if(DBGetContactSettingByte(dat->hContact,"CList","NotOnList",0)) {
			RECT rcBtn1,rcBtn2,rcDateCtrl;
			GetWindowRect(GetDlgItem(hwndDlg,IDC_ADD),&rcBtn1);
			GetWindowRect(GetDlgItem(hwndDlg,IDC_USERMENU),&rcBtn2);
			GetWindowRect(GetDlgItem(hwndDlg,IDC_DATE),&rcDateCtrl);
			SetWindowPos(GetDlgItem(hwndDlg,IDC_DATE),0,0,0,rcDateCtrl.right-rcDateCtrl.left-(rcBtn2.left-rcBtn1.left),rcDateCtrl.bottom-rcDateCtrl.top,SWP_NOZORDER|SWP_NOMOVE);
		}
		else if(DBGetContactSettingByte(NULL,"SRFile","AutoAccept",0)) {
			//don't check auto-min here to fix BUG#647620
			PostMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)GetDlgItem(hwndDlg,IDOK));
		}
		if(!DBGetContactSettingByte(dat->hContact,"CList","NotOnList",0))
			ShowWindow(GetDlgItem(hwndDlg, IDC_ADD),SW_HIDE);
		return TRUE;
	}

	case WM_MEASUREITEM:
		return CallService(MS_CLIST_MENUMEASUREITEM,wParam,lParam);

	case WM_DRAWITEM:
		{	LPDRAWITEMSTRUCT dis=(LPDRAWITEMSTRUCT)lParam;
			if(dis->hwndItem==GetDlgItem(hwndDlg, IDC_PROTOCOL)) {
				char *szProto;

				szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)dat->hContact,0);
				if (szProto) {
					HICON hIcon;

					hIcon=(HICON)CallProtoService(szProto,PS_LOADICON,PLI_PROTOCOL|PLIF_SMALL,0);
					if (hIcon) {
						DrawIconEx(dis->hDC,dis->rcItem.left,dis->rcItem.top,hIcon,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0,NULL,DI_NORMAL);
						DestroyIcon(hIcon);
		}	}	}	}
		return CallService(MS_CLIST_MENUDRAWITEM,wParam,lParam);

	case WM_COMMAND:
		if ( CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam),MPCF_CONTACTMENU), (LPARAM)dat->hContact ))
			break;

		switch ( LOWORD( wParam )) {
		case IDC_FILEDIRBROWSE:
			{
				TCHAR szDirName[MAX_PATH],szExistingDirName[MAX_PATH];

				GetDlgItemText(hwndDlg,IDC_FILEDIR,szDirName,SIZEOF(szDirName));
				GetLowestExistingDirName(szDirName,szExistingDirName,SIZEOF(szExistingDirName));
				if(BrowseForFolder(hwndDlg,szExistingDirName))
					SetDlgItemText(hwndDlg,IDC_FILEDIR,szExistingDirName);
			}
            break;

		case IDOK:
			{	//most recently used directories
				TCHAR szRecvDir[MAX_PATH],szDefaultRecvDir[MAX_PATH];
				GetDlgItemText(hwndDlg,IDC_FILEDIR,szRecvDir,SIZEOF(szRecvDir));
				RemoveInvalidPathChars(szRecvDir);
				GetContactReceivedFilesDir(NULL,szDefaultRecvDir,SIZEOF(szDefaultRecvDir),TRUE);
				if(_tcsnicmp(szRecvDir,szDefaultRecvDir,lstrlen(szDefaultRecvDir))) {
					char idstr[32];
					int i;
					DBVARIANT dbv;
					for(i=MAX_MRU_DIRS-2;i>=0;i--) {
						mir_snprintf(idstr, SIZEOF(idstr), "MruDir%d",i);
						if(DBGetContactSettingTString(NULL,"SRFile",idstr,&dbv)) continue;
						mir_snprintf(idstr, SIZEOF(idstr), "MruDir%d",i+1);
						DBWriteContactSettingTString(NULL,"SRFile",idstr,dbv.ptszVal);
						DBFreeVariant(&dbv);
					}
					DBWriteContactSettingTString(NULL,"SRFile",idstr,szRecvDir);
				}
			}
			EnableWindow(GetDlgItem(hwndDlg,IDC_FILENAMES),FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_MSG),FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_FILEDIR),FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_FILEDIRBROWSE),FALSE);

			GetDlgItemText(hwndDlg,IDC_FILEDIR,dat->szSavePath,SIZEOF(dat->szSavePath));
			GetDlgItemText(hwndDlg,IDC_FILE,dat->szFilenames,SIZEOF(dat->szFilenames));
			GetDlgItemText(hwndDlg,IDC_MSG,dat->szMsg,SIZEOF(dat->szMsg));
			dat->hwndTransfer = FtMgr_AddTransfer(dat);
			SetWindowLongPtr( hwndDlg, GWLP_USERDATA, 0);
			//check for auto-minimize here to fix BUG#647620
			if(DBGetContactSettingByte(NULL,"SRFile","AutoAccept",0) && DBGetContactSettingByte(NULL,"SRFile","AutoMin",0)) {
				ShowWindow(hwndDlg,SW_HIDE);
				ShowWindow(hwndDlg,SW_SHOWMINNOACTIVE);
			}
			DestroyWindow(hwndDlg);
            break;

		case IDCANCEL:
			if (dat->fs) CallContactService(dat->hContact,PSS_FILEDENYT,(WPARAM)dat->fs,(LPARAM)TranslateT("Cancelled"));
			dat->fs=NULL; /* the protocol will free the handle */
			DestroyWindow(hwndDlg);
            break;

		case IDC_ADD:
			{	ADDCONTACTSTRUCT acs={0};

				acs.handle=dat->hContact;
				acs.handleType=HANDLE_CONTACT;
				acs.szProto="";
				CallService(MS_ADDCONTACT_SHOW,(WPARAM)hwndDlg,(LPARAM)&acs);
				if(!DBGetContactSettingByte(dat->hContact,"CList","NotOnList",0))
					ShowWindow(GetDlgItem(hwndDlg,IDC_ADD), SW_HIDE);
			}
            break;

		case IDC_USERMENU:
			{	RECT rc;
				HMENU hMenu=(HMENU)CallService(MS_CLIST_MENUBUILDCONTACT,(WPARAM)dat->hContact,0);
				GetWindowRect((HWND)lParam,&rc);
				TrackPopupMenu(hMenu,0,rc.left,rc.bottom,0,hwndDlg,NULL);
				DestroyMenu(hMenu);
			}
			break;

		case IDC_DETAILS:
			CallService(MS_USERINFO_SHOWDIALOG,(WPARAM)dat->hContact,0);
            break;

		case IDC_HISTORY:
			CallService(MS_HISTORY_SHOWCONTACTHISTORY,(WPARAM)dat->hContact,0);
            break;
		}
		break;

	case WM_DESTROY:
		Window_FreeIcon_IcoLib(hwndDlg);
		Button_FreeIcon_IcoLib(hwndDlg,IDC_ADD);
		Button_FreeIcon_IcoLib(hwndDlg,IDC_DETAILS);
		Button_FreeIcon_IcoLib(hwndDlg,IDC_HISTORY);
		Button_FreeIcon_IcoLib(hwndDlg,IDC_USERMENU);

		if ( dat ) FreeFileDlgData( dat );
        break;
	}
	return FALSE;
}
