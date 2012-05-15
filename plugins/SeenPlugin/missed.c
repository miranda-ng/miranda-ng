/*
"Last Seen mod" plugin for Miranda IM
Copyright ( C ) 2002-03  micron-x
Copyright ( C ) 2005-07  Y.B.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

File name      : $URL: http://svn.berlios.de/svnroot/repos/mgoodies/trunk/lastseen-mod/missed.c $
Revision       : $Rev: 1570 $
Last change on : $Date: 2007-12-30 01:30:07 +0300 (Вс, 30 дек 2007) $
Last change by : $Author: y_b $
*/
#include "seen.h"

#include <m_ignore.h>



MISSEDCONTACTS mcs;
extern HANDLE ehmissed;
extern HINSTANCE hInstance;



WPARAM IsUserMissed(WPARAM contact)
{
	int loop=0;

	for(;loop<mcs.count;loop++)
	{
		if(mcs.wpcontact[loop]==contact)
			return MAKEWPARAM(1,loop);
	}

	return 0;
}



int RemoveUser(int pos)
{
	int loop;

	for(loop=pos;loop<mcs.count-1;loop++)
		mcs.wpcontact[loop]=mcs.wpcontact[loop+1];

	mcs.count--;

	return 0;
}



int ResetMissed(void)
{
	HANDLE hcontact=NULL;

	hcontact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	while(hcontact!=NULL)
	{
		DBWriteContactSettingByte(hcontact,S_MOD,"Missed",0);
		hcontact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hcontact,0);
	}

	ZeroMemory(&mcs,sizeof(mcs));

	return 0;
}


int CheckIfOnline(void)
{
	HANDLE hcontact;
//	char *szProto;
//	WORD status;

	hcontact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	while(hcontact!=NULL)
	{
/*		szProto=(char *)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hcontact,0);
		status=(szProto==NULL)?ID_STATUS_OFFLINE:DBGetContactSettingWord(hcontact,szProto,"Status",ID_STATUS_OFFLINE);
		if(status!=ID_STATUS_OFFLINE)
*/
		if(CallService(MS_CLIST_GETCONTACTICON,(WPARAM)hcontact,0)!=ICON_OFFLINE)
			DBWriteContactSettingByte(hcontact,S_MOD,"Missed",2);

		hcontact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hcontact,0);
	}

	return 0;
}



INT_PTR CALLBACK MissedDlgProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	POINT pt;
	RECT rcinit,rcresized,rcb,rcd;
	HWND htemp;
		
	switch(msg){

		case WM_INITDIALOG:
			TranslateDialogDefault(hdlg);

			htemp=GetDlgItem(hdlg,IDC_CONTACTS);
			GetWindowRect(htemp,&rcinit);
			SetWindowPos(htemp,NULL,0,0,rcinit.right-rcinit.left,mcs.count*(rcinit.bottom-rcinit.top)/2,SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE);
			GetWindowRect(htemp,&rcresized);

			htemp=GetDlgItem(hdlg,IDOK);
			GetWindowRect(htemp,&rcb);
			pt.x=rcb.left;
			pt.y=rcb.top;

			ScreenToClient(hdlg,&pt);
			MoveWindow(htemp,pt.x,pt.y+(rcresized.bottom-rcinit.bottom),(rcb.right-rcb.left),(rcb.bottom-rcb.top),FALSE);
			GetWindowRect(hdlg,&rcd);
			SetWindowPos(hdlg,NULL,0,0,rcd.right-rcd.left,rcd.bottom-rcd.top+(rcresized.bottom-rcinit.bottom),SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE);
			
			SetDlgItemText(hdlg,IDC_CONTACTS,(LPCSTR)lparam);
			ShowWindow(hdlg,SW_SHOWNOACTIVATE);
			break;

		case WM_CLOSE:
			EndDialog(hdlg,0);
			break;

		case WM_COMMAND:
			if(LOWORD(wparam)==IDOK)
				SendMessage(hdlg,WM_CLOSE,0,0);
			break;
	}

	return 0;
}




int ShowMissed(void)
{
	int loop=0;
	char sztemp[1024]="",szcount[7];

	if(!mcs.count) return 0;

	for(;loop<mcs.count;loop++)
	{
		strcat(sztemp,(const char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,mcs.wpcontact[loop],0));
		if(DBGetContactSettingByte(NULL,S_MOD,"MissedOnes_Count",0))
		{
			wsprintf(szcount," [%i]",mcs.times[loop]);
			strcat(sztemp,szcount);
		}

		strcat(sztemp,"\n");
	}

	CreateDialogParam(hInstance,MAKEINTRESOURCE(IDD_MISSED),NULL,MissedDlgProc,(LPARAM)&sztemp[0]);

	return 0;
}

/*

int LogStatus(WPARAM wparam,LPARAM lparam)
{
	DBCONTACTWRITESETTING *cws;
	WPARAM wpvar;
	
	cws=(DBCONTACTWRITESETTING *)lparam;

	if(strcmp(cws->szSetting,"Status") || (strcmp(cws->szModule,"ICQ") && strcmp(cws->szModule,"MSN")) || (HANDLE)wparam==NULL) return 0;

	if(CallService(MS_IGNORE_ISIGNORED,wparam,IGNOREEVENT_USERONLINE))
		return 0;

	if(cws->value.wVal==ID_STATUS_OFFLINE)
	{
		if(DBGetContactSettingByte((HANDLE)wparam,S_MOD,"Missed",0)==1)
		{
			mcs.times[mcs.count]++;
			mcs.wpcontact[mcs.count++]=wparam;
			DBWriteContactSettingByte((HANDLE)wparam,S_MOD,"Missed",0);
		}

		else if(DBGetContactSettingByte((HANDLE)wparam,S_MOD,"Missed",0)==3)
			mcs.times[HIWORD(IsUserMissed(wparam))]++;

		return 0;
	}

	wpvar=IsUserMissed(wparam);
	if(LOWORD(wpvar))
		DBWriteContactSettingByte((HANDLE)wparam,S_MOD,"Missed",3);

	else
		DBWriteContactSettingByte((HANDLE)wparam,S_MOD,"Missed",1);

	return 0;
}

*/

int Test(WPARAM wparam,LPARAM lparam)
{
	if(lparam<ICON_OFFLINE || lparam>ICON_INVIS)
		return 0;

	if(CallService(MS_IGNORE_ISIGNORED,wparam,IGNOREEVENT_USERONLINE))
		return 0;

	if(DBGetContactSettingByte((HANDLE)wparam,S_MOD,"Missed",0)==2)
		return 0;

	switch(lparam){

		case ICON_OFFLINE:
			if(DBGetContactSettingByte((HANDLE)wparam,S_MOD,"Missed",0)==1)
			{
				WORD missed=IsUserMissed(wparam);

				if(!LOWORD(missed))
				{
					mcs.times[mcs.count]=1;
					mcs.wpcontact[mcs.count++]=wparam;
				}
				else
					mcs.times[HIWORD(missed)]++;

				DBWriteContactSettingByte((HANDLE)wparam,S_MOD,"Missed",0);
			}
			break;

		case ICON_ONLINE:
		case ICON_AWAY:
		case ICON_NA:
		case ICON_OCC:
		case ICON_DND:
		case ICON_FREE:
		case ICON_INVIS:
			DBWriteContactSettingByte((HANDLE)wparam,S_MOD,"Missed",1);
			break;
	}
		
	return 0;
}



int ModeChange_mo(WPARAM wparam,LPARAM lparam)
{
	ACKDATA *ack;
	int isetting=0;

	ack=(ACKDATA *)lparam;

	if(ack->type!=ACKTYPE_STATUS || ack->result!=ACKRESULT_SUCCESS || ack->hContact!=NULL) return 0;

	isetting=CallProtoService(ack->szModule,PS_GETSTATUS,0,0);

	switch(isetting){

		case ID_STATUS_AWAY:
		case ID_STATUS_DND:
		case ID_STATUS_NA:
			if(ehmissed==NULL)
			{
				ZeroMemory(&mcs,sizeof(mcs));
				CheckIfOnline();
				//ehmissed=HookEvent(ME_DB_CONTACT_SETTINGCHANGED,LogStatus);
				ehmissed=HookEvent(ME_CLIST_CONTACTICONCHANGED,Test);
			}
			break;

		default:
			if(ehmissed!=NULL)
			{
				UnhookEvent(ehmissed);
				ehmissed=NULL;
				ShowMissed();
				ResetMissed();
			}
			break;
	}

	return 0;
}
