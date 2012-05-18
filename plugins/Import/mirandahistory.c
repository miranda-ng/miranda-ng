/*
Miranda ICQ: the free icq client for MS Windows
Copyright (C) 2000-2  Richard Hughes, Roland Rabien & Tristan Van de Vreede

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
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include "resource.h"
#include "import.h"

BOOL CALLBACK MirandaPageProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK FinishedPageProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK ProgressPageProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);
extern char importFile[MAX_PATH];
extern void (*DoImport)(HWND);
static void OldMirandaHistoryImport(HWND hdlgProgress);
HANDLE HContactFromUIN(DWORD uin);
HANDLE HistoryImportFindContact(HWND hdlgProgress,DWORD uin,int addUnknown);
static DWORD importOptions;

BOOL CALLBACK MirandaHistoryPageProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message) {
		case WM_INITDIALOG:
			CheckDlgButton(hdlg,IDC_ADDUNKNOWN,BST_CHECKED);
			CheckDlgButton(hdlg,IDC_MSGRECV,BST_CHECKED);
			CheckDlgButton(hdlg,IDC_MSGSENT,BST_CHECKED);
			CheckDlgButton(hdlg,IDC_URLRECV,BST_CHECKED);
			CheckDlgButton(hdlg,IDC_URLSENT,BST_CHECKED);
			CheckDlgButton(hdlg,IDC_AUTHREQ,BST_CHECKED);
			CheckDlgButton(hdlg,IDC_ADDED,BST_CHECKED);
			return TRUE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_BACK:
					PostMessage(GetParent(hdlg),WIZM_GOTOPAGE,IDD_MIRANDADB,(LPARAM)MirandaPageProc);
					break;
				case IDOK:
					importOptions=0;
					if(IsDlgButtonChecked(hdlg,IDC_ADDUNKNOWN)) importOptions|=IOPT_ADDUNKNOWN;
					if(IsDlgButtonChecked(hdlg,IDC_MSGSENT)) importOptions|=IOPT_MSGSENT;
					if(IsDlgButtonChecked(hdlg,IDC_MSGRECV)) importOptions|=IOPT_MSGRECV;
					if(IsDlgButtonChecked(hdlg,IDC_URLSENT)) importOptions|=IOPT_URLSENT;
					if(IsDlgButtonChecked(hdlg,IDC_URLRECV)) importOptions|=IOPT_URLRECV;
					if(IsDlgButtonChecked(hdlg,IDC_AUTHREQ)) importOptions|=IOPT_AUTHREQ;
					if(IsDlgButtonChecked(hdlg,IDC_ADDED)) importOptions|=IOPT_ADDED;
					DoImport=OldMirandaHistoryImport;
					PostMessage(GetParent(hdlg),WIZM_GOTOPAGE,IDD_PROGRESS,(LPARAM)ProgressPageProc);
					break;
				case IDCANCEL:
					PostMessage(GetParent(hdlg),WM_CLOSE,0,0);
					break;
			}
			break;
	}
	return FALSE;
}

#define HISTORY_MSGRECV 1
#define HISTORY_MSGSEND 2
#define HISTORY_URLRECV 3
#define HISTORY_URLSEND 4
#define HISTORY_AUTHREQ 5
#define HISTORY_ADDED 6
#define issent(t)  ((t)==HISTORY_MSGSEND || (t)==HISTORY_URLSEND)
static int historyTypeToOption[]={0,IOPT_MSGRECV,IOPT_MSGSENT,IOPT_URLRECV,IOPT_URLSENT,IOPT_AUTHREQ,IOPT_ADDED};

static PBYTE ReadHistoryLines(FILE *fp,int *cbBlob)
{
	char str[2048];
	char *blob;
	int ofs;
	*cbBlob=0;
	blob=NULL;
	while(fgets(str,sizeof(str),fp) && lstrcmp(str,"\xEE\xEE\xEE\xEE\r\n")) {
		ofs=*cbBlob;
		*cbBlob+=lstrlen(str);
		blob=(char*)realloc(blob,*cbBlob+1);
		lstrcpy(blob+ofs,str);
	}
	if(*cbBlob) {
		(*cbBlob)--;
		blob[*cbBlob-1]=0;
	}
	else {
		*cbBlob=1;
		blob=(char*)malloc(1);
		blob[0]=0;
	}
	return (PBYTE)blob;
}

static void OldMirandaHistoryImport(HWND hdlgProgress)
{
	int fileSize;
	FILE *fp;
	char str[2048],*eol,*timeofs;
	HANDLE hContact;
	int type;
	DWORD uin;
	struct tm tmEventTime;
	DBEVENTINFO dbei;

	AddMessage("Old Miranda history import routines initialised");
	fp=fopen(importFile,"rb");
	AddMessage("Calibrating status support functions");
	fseek(fp,0,SEEK_END);
	fileSize=ftell(fp);
	fseek(fp,0,SEEK_SET);
	SetProgress(0);

	ZeroMemory(&dbei,sizeof(dbei));
	dbei.cbSize=sizeof(dbei);
	dbei.szModule=ICQOSCPROTONAME;

	while(!feof(fp)) {
		SetProgress(100*ftell(fp)/fileSize);
		if(fgets(str,sizeof(str),fp)==NULL) break;
		eol=str+lstrlen(str)-1;

		while(*eol=='\r' || *eol=='\n' && eol!=str-1)
			*(eol--)=0;

		if(lstrlen(str)<20) continue;
		type=*eol;
		uin=strtoul(str,NULL,10);
		if(uin==0) continue;   //skip MSN
		timeofs=str+lstrlen(str)-20;
		tmEventTime.tm_hour=atoi(timeofs);
		timeofs=strchr(timeofs,':');
		if(timeofs==NULL) continue;
		tmEventTime.tm_min=atoi(timeofs+1);
		tmEventTime.tm_sec=0;
		tmEventTime.tm_isdst=-1;
		tmEventTime.tm_mday=atoi(timeofs+7);
		tmEventTime.tm_mon=atoi(timeofs+10)-1;
		tmEventTime.tm_year=atoi(timeofs+13)-1900;
		dbei.timestamp=mktime(&tmEventTime)+_timezone;
		if(!(importOptions&historyTypeToOption[type])) continue;
		hContact=HistoryImportFindContact(hdlgProgress,uin,importOptions&IOPT_ADDUNKNOWN);
		if(hContact==INVALID_HANDLE_VALUE) break;
		dbei.flags=issent(type)?DBEF_SENT:DBEF_READ;
		switch(type) {
			case HISTORY_MSGRECV:
			case HISTORY_MSGSEND:
				dbei.eventType=EVENTTYPE_MESSAGE;
				dbei.pBlob=ReadHistoryLines(fp,&dbei.cbBlob);
				CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&dbei);
				break;
			case HISTORY_URLRECV:
			case HISTORY_URLSEND:
				dbei.eventType=EVENTTYPE_URL;
				dbei.pBlob=ReadHistoryLines(fp,&dbei.cbBlob);
				{	char *endOfUrl;
					endOfUrl=strchr(dbei.pBlob,'\r');
					if(endOfUrl!=NULL) {
						*endOfUrl=0;
						dbei.cbBlob--;
						MoveMemory(endOfUrl+1,endOfUrl+2,dbei.cbBlob-(endOfUrl-dbei.pBlob));
					}
				}
				CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&dbei);
				break;
			case HISTORY_AUTHREQ:
				dbei.eventType=EVENTTYPE_AUTHREQUEST;
				dbei.pBlob=ReadHistoryLines(fp,&dbei.cbBlob);
				dbei.pBlob=(PBYTE)realloc(dbei.pBlob,dbei.cbBlob+8);
				MoveMemory(dbei.pBlob+8,dbei.pBlob,dbei.cbBlob);
				*(PDWORD)dbei.pBlob=uin;
				*(char*)(dbei.pBlob+4)=0;	//leave nick, first, last, email blank
				*(char*)(dbei.pBlob+5)=0;
				*(char*)(dbei.pBlob+6)=0;
				*(char*)(dbei.pBlob+7)=0;
				dbei.cbBlob+=8;
				CallService(MS_DB_EVENT_ADD,(WPARAM)(HANDLE)NULL,(LPARAM)&dbei);
				break;
			case HISTORY_ADDED:
				dbei.eventType=EVENTTYPE_ADDED;
				dbei.pBlob=(PBYTE)malloc(8);
				dbei.cbBlob=8;
				*(PDWORD)dbei.pBlob=uin;
				*(char*)(dbei.pBlob+4)=0;	//leave nick, first, last, email blank
				*(char*)(dbei.pBlob+5)=0;
				*(char*)(dbei.pBlob+6)=0;
				*(char*)(dbei.pBlob+7)=0;
				CallService(MS_DB_EVENT_ADD,(WPARAM)(HANDLE)NULL,(LPARAM)&dbei);
				break;
		}
	}
	AddMessage("Terminating cached I/O access");
	fclose(fp);
	AddMessage("Import completed successfully");
	SetProgress(100);
}