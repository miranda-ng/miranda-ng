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

File name      : $URL: http://svn.berlios.de/svnroot/repos/mgoodies/trunk/lastseen-mod/userinfo.c $
Revision       : $Rev: 1570 $
Last change on : $Date: 2007-12-30 01:30:07 +0300 (Вс, 30 дек 2007) $
Last change by : $Author: y_b $
*/
#include "seen.h"



WNDPROC MainProc;



extern HINSTANCE hInstance;
extern DWORD dwmirver;



BOOL CALLBACK EditProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	switch(msg){
		case WM_SETCURSOR:
			SetCursor(LoadCursor(NULL,IDC_ARROW));
			return 1;

		default:
			break;
	}
	return CallWindowProc(MainProc,hdlg,msg,wparam,lparam);
}



INT_PTR CALLBACK UserinfoDlgProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	char *szout;
	DBVARIANT dbv;
	
	switch(msg){

		case WM_INITDIALOG:
			MainProc=(WNDPROC)SetWindowLongPtr(GetDlgItem(hdlg,IDC_INFOTEXT),GWLP_WNDPROC,(LONG)EditProc);
			szout=strdup(ParseString((!DBGetContactSetting(NULL,S_MOD,"UserStamp",&dbv)?dbv.pszVal:DEFAULT_USERSTAMP),(HANDLE)lparam,0));
			SetDlgItemText(hdlg,IDC_INFOTEXT,szout);
			if(!strcmp(szout,Translate("<unknown>")))
			EnableWindow(GetDlgItem(hdlg,IDC_INFOTEXT),FALSE);
			free(szout);
			DBFreeVariant(&dbv);
			break;

		case WM_COMMAND:
			if(HIWORD(wparam)==EN_SETFOCUS)
				SetFocus(GetParent(hdlg));
			break;
	}

	return 0;
}



int UserinfoInit(WPARAM wparam,LPARAM lparam)
{
	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO,lparam,0);
	if (IsWatchedProtocol(proto)){
		OPTIONSDIALOGPAGE uip;
		ZeroMemory(&uip,sizeof(uip));
		uip.cbSize=sizeof(uip);
		uip.hInstance=hInstance;
		uip.pszTemplate=MAKEINTRESOURCE(IDD_USERINFO);
		uip.pszTitle="Last seen";
		uip.pfnDlgProc=UserinfoDlgProc;
		CallService(MS_USERINFO_ADDPAGE,wparam,(LPARAM)&uip);
	}
	return 0;
}
