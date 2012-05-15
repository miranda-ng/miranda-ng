/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project, 
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

extern HMENU hMenuMain;
extern BOOL (WINAPI *MySetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD);
static INT_PTR CALLBACK DlgProcCluiOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK DlgProcSBarOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern HANDLE hFrameHelperStatusBar;
extern void ReAssignExtraIcons();
extern int CluiProtocolStatusChanged(WPARAM wParam,LPARAM lParam);
extern int UseOwnerDrawStatusBar;
extern int OnStatusBarBackgroundChange();

static UINT expertOnlyControls[]={IDC_BRINGTOFRONT, IDC_AUTOSIZE,IDC_STATIC21,IDC_MAXSIZEHEIGHT,IDC_MAXSIZESPIN,IDC_STATIC22,IDC_AUTOSIZEUPWARD,IDC_SHOWMAINMENU,IDC_SHOWCAPTION,IDC_CLIENTDRAG};
int CluiOptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize=sizeof(odp);
	odp.position=0;
	odp.hInstance=g_hInst;
	odp.pszTemplate=MAKEINTRESOURCEA(IDD_OPT_CLUI);
	odp.pszTitle=LPGEN("Window");
	odp.pszGroup=LPGEN("Contact List");
	odp.pfnDlgProc=DlgProcCluiOpts;
	odp.flags=ODPF_BOLDGROUPS;
	odp.nIDBottomSimpleControl=IDC_STWINDOWGROUP;
	odp.expertOnlyControls=expertOnlyControls;
	odp.nExpertOnlyControls=sizeof(expertOnlyControls)/sizeof(expertOnlyControls[0]);
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	odp.pszTemplate=MAKEINTRESOURCEA(IDD_OPT_SBAR);
	odp.pszTitle=LPGEN("Status Bar");
	odp.pfnDlgProc=DlgProcSBarOpts;
	odp.flags=ODPF_BOLDGROUPS|ODPF_EXPERTONLY;
	odp.nIDBottomSimpleControl=0;
	odp.nExpertOnlyControls=0;
	odp.expertOnlyControls=NULL;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	return 0;
}

static INT_PTR CALLBACK DlgProcCluiOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);
			CheckDlgButton(hwndDlg, IDC_BRINGTOFRONT, DBGetContactSettingByte(NULL,"CList","BringToFront",SETTING_BRINGTOFRONT_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ONTOP, DBGetContactSettingByte(NULL,"CList","OnTop",SETTING_ONTOP_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TOOLWND, DBGetContactSettingByte(NULL,"CList","ToolWindow",SETTING_TOOLWINDOW_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_MIN2TRAY, DBGetContactSettingByte(NULL,"CList","Min2Tray",SETTING_MIN2TRAY_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			if(IsDlgButtonChecked(hwndDlg,IDC_TOOLWND)) EnableWindow(GetDlgItem(hwndDlg,IDC_MIN2TRAY),FALSE);
			CheckDlgButton(hwndDlg, IDC_SHOWCAPTION, DBGetContactSettingByte(NULL,"CLUI","ShowCaption",SETTING_SHOWCAPTION_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWMAINMENU, DBGetContactSettingByte(NULL,"CLUI","ShowMainMenu",SETTING_SHOWMAINMENU_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLIENTDRAG, DBGetContactSettingByte(NULL,"CLUI","ClientAreaDrag",SETTING_CLIENTDRAG_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			if(!IsDlgButtonChecked(hwndDlg,IDC_SHOWCAPTION)) {
				EnableWindow(GetDlgItem(hwndDlg,IDC_MIN2TRAY),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_TOOLWND),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_TITLETEXT),FALSE);
			}
			CheckDlgButton(hwndDlg, IDC_FADEINOUT, DBGetContactSettingByte(NULL,"CLUI","FadeInOut",0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_AUTOSIZE, DBGetContactSettingByte(NULL,"CLUI","AutoSize",0) ? BST_CHECKED : BST_UNCHECKED);			
			CheckDlgButton(hwndDlg, IDC_DROPSHADOW, DBGetContactSettingByte(NULL,"CList","WindowShadow",0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ONDESKTOP, DBGetContactSettingByte(NULL,"CList","OnDesktop", 0) ? BST_CHECKED : BST_UNCHECKED);
			SendDlgItemMessage(hwndDlg,IDC_MAXSIZESPIN,UDM_SETRANGE,0,MAKELONG(100,0));
			SendDlgItemMessage(hwndDlg,IDC_MAXSIZESPIN,UDM_SETPOS,0,DBGetContactSettingByte(NULL,"CLUI","MaxSizeHeight",75));
			CheckDlgButton(hwndDlg, IDC_AUTOSIZEUPWARD, DBGetContactSettingByte(NULL,"CLUI","AutoSizeUpward",0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_AUTOHIDE, DBGetContactSettingByte(NULL,"CList","AutoHide",SETTING_AUTOHIDE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			SendDlgItemMessage(hwndDlg,IDC_HIDETIMESPIN,UDM_SETRANGE,0,MAKELONG(900,1));
			SendDlgItemMessage(hwndDlg,IDC_HIDETIMESPIN,UDM_SETPOS,0,MAKELONG(DBGetContactSettingWord(NULL,"CList","HideTime",SETTING_HIDETIME_DEFAULT),0));
			EnableWindow(GetDlgItem(hwndDlg,IDC_HIDETIME),IsDlgButtonChecked(hwndDlg,IDC_AUTOHIDE));
			EnableWindow(GetDlgItem(hwndDlg,IDC_HIDETIMESPIN),IsDlgButtonChecked(hwndDlg,IDC_AUTOHIDE));
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC01),IsDlgButtonChecked(hwndDlg,IDC_AUTOHIDE));
			if(!IsDlgButtonChecked(hwndDlg,IDC_AUTOSIZE)) {
				EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC21),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC22),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_MAXSIZEHEIGHT),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_MAXSIZESPIN),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_AUTOSIZEUPWARD),FALSE);
			}

			{	DBVARIANT dbv;
				char *s;
				char szUin[20];

				if(DBGetContactSettingString(NULL,"CList","TitleText",&dbv)==0&&(dbv.pszVal))
				{
					s=mir_strdup(dbv.pszVal);
					mir_free(dbv.pszVal);
				}
				else
				{
					s=mir_strdup(MIRANDANAME);
				}

				if(s)
				{				
					SetDlgItemTextA(hwndDlg,IDC_TITLETEXT,s);
					mir_free(s);
				}

				SendDlgItemMessage(hwndDlg,IDC_TITLETEXT,CB_ADDSTRING,0,(LPARAM)MIRANDANAME);
				wsprintfA(szUin,"%u",DBGetContactSettingDword(NULL,"ICQ","UIN",0));
				SendDlgItemMessage(hwndDlg,IDC_TITLETEXT,CB_ADDSTRING,0,(LPARAM)szUin);
				
				if(!DBGetContactSettingString(NULL,"ICQ","Nick",&dbv)) {
					SendDlgItemMessage(hwndDlg,IDC_TITLETEXT,CB_ADDSTRING,0,(LPARAM)dbv.pszVal);
					mir_free(dbv.pszVal);
					dbv.pszVal=NULL;
				}
				if(!DBGetContactSettingString(NULL,"ICQ","FirstName",&dbv)) {
					SendDlgItemMessage(hwndDlg,IDC_TITLETEXT,CB_ADDSTRING,0,(LPARAM)dbv.pszVal);
					mir_free(dbv.pszVal);
					dbv.pszVal=NULL;
				}
				if(!DBGetContactSettingString(NULL,"ICQ","e-mail",&dbv)) {
					SendDlgItemMessage(hwndDlg,IDC_TITLETEXT,CB_ADDSTRING,0,(LPARAM)dbv.pszVal);
					mir_free(dbv.pszVal);
					dbv.pszVal=NULL;
				}
			}
			if(!IsWinVer2000Plus()) {
				EnableWindow(GetDlgItem(hwndDlg,IDC_FADEINOUT),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_TRANSPARENT),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_DROPSHADOW),FALSE);
			}
			else CheckDlgButton(hwndDlg,IDC_TRANSPARENT,DBGetContactSettingByte(NULL,"CList","Transparent",SETTING_TRANSPARENT_DEFAULT)?BST_CHECKED:BST_UNCHECKED);
			if(!IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT)) {
				EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC11),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC12),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_TRANSACTIVE),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_TRANSINACTIVE),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_ACTIVEPERC),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_INACTIVEPERC),FALSE);
			}
			SendDlgItemMessage(hwndDlg,IDC_TRANSACTIVE,TBM_SETRANGE,FALSE,MAKELONG(1,255));
			SendDlgItemMessage(hwndDlg,IDC_TRANSINACTIVE,TBM_SETRANGE,FALSE,MAKELONG(1,255));
			SendDlgItemMessage(hwndDlg,IDC_TRANSACTIVE,TBM_SETPOS,TRUE,DBGetContactSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT));
			SendDlgItemMessage(hwndDlg,IDC_TRANSINACTIVE,TBM_SETPOS,TRUE,DBGetContactSettingByte(NULL,"CList","AutoAlpha",SETTING_AUTOALPHA_DEFAULT));
			SendMessage(hwndDlg,WM_HSCROLL,0x12345678,0);

			// EXTRA Icons
			if (ServiceExists("ExtraIcon/Register"))
			{
				ShowWindow(GetDlgItem(hwndDlg,IDC_EXTRA_FRAME), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_EXTRA_PROTO), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_EXTRA_WEB), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_EXTRA_EMAIL), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_EXTRA_CELLULAR), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_EXTRA_ADV1), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_EXTRA_ADV2), SW_HIDE);
			}
			else
			{
				CheckDlgButton(hwndDlg, IDC_EXTRA_PROTO, DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_PROTO",1) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_EXTRA_WEB, DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_WEB",1) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_EXTRA_EMAIL, DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_EMAIL",1) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_EXTRA_CELLULAR, DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_SMS",1) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_EXTRA_ADV1, DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_ADV1",1) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_EXTRA_ADV2, DBGetContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_ADV2",1) ? BST_CHECKED : BST_UNCHECKED);			
			}

			return TRUE;
			
		case WM_COMMAND:
			if(LOWORD(wParam)==IDC_AUTOHIDE) {
				EnableWindow(GetDlgItem(hwndDlg,IDC_HIDETIME),IsDlgButtonChecked(hwndDlg,IDC_AUTOHIDE));
				EnableWindow(GetDlgItem(hwndDlg,IDC_HIDETIMESPIN),IsDlgButtonChecked(hwndDlg,IDC_AUTOHIDE));
				EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC01),IsDlgButtonChecked(hwndDlg,IDC_AUTOHIDE));
			}
			else if(LOWORD(wParam)==IDC_TRANSPARENT) {
				EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC11),IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT));
				EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC12),IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT));
				EnableWindow(GetDlgItem(hwndDlg,IDC_TRANSACTIVE),IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT));
				EnableWindow(GetDlgItem(hwndDlg,IDC_TRANSINACTIVE),IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT));
				EnableWindow(GetDlgItem(hwndDlg,IDC_ACTIVEPERC),IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT));
				EnableWindow(GetDlgItem(hwndDlg,IDC_INACTIVEPERC),IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT));
			}
			else if(LOWORD(wParam)==IDC_AUTOSIZE) {
				EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC21),IsDlgButtonChecked(hwndDlg,IDC_AUTOSIZE));
				EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC22),IsDlgButtonChecked(hwndDlg,IDC_AUTOSIZE));
				EnableWindow(GetDlgItem(hwndDlg,IDC_MAXSIZEHEIGHT),IsDlgButtonChecked(hwndDlg,IDC_AUTOSIZE));
				EnableWindow(GetDlgItem(hwndDlg,IDC_MAXSIZESPIN),IsDlgButtonChecked(hwndDlg,IDC_AUTOSIZE));
				EnableWindow(GetDlgItem(hwndDlg,IDC_AUTOSIZEUPWARD),IsDlgButtonChecked(hwndDlg,IDC_AUTOSIZE));
			}
			else if(LOWORD(wParam)==IDC_TOOLWND) {
				EnableWindow(GetDlgItem(hwndDlg,IDC_MIN2TRAY),!IsDlgButtonChecked(hwndDlg,IDC_TOOLWND));
			}
			else if(LOWORD(wParam)==IDC_SHOWCAPTION) {
				EnableWindow(GetDlgItem(hwndDlg,IDC_TOOLWND),IsDlgButtonChecked(hwndDlg,IDC_SHOWCAPTION));
				EnableWindow(GetDlgItem(hwndDlg,IDC_MIN2TRAY),!IsDlgButtonChecked(hwndDlg,IDC_TOOLWND) && IsDlgButtonChecked(hwndDlg,IDC_SHOWCAPTION));
				EnableWindow(GetDlgItem(hwndDlg,IDC_TITLETEXT),IsDlgButtonChecked(hwndDlg,IDC_SHOWCAPTION));
			}

			if ((LOWORD(wParam)==IDC_HIDETIME || LOWORD(wParam)==IDC_TITLETEXT || LOWORD(wParam)==IDC_MAXSIZEHEIGHT) &&
				(HIWORD(wParam)!=EN_CHANGE || (HWND)lParam!=GetFocus()))
				return 0;
			
			// Enable apply button
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case WM_HSCROLL:
			{	char str[10];
				wsprintfA(str,"%d%%",100*SendDlgItemMessage(hwndDlg,IDC_TRANSINACTIVE,TBM_GETPOS,0,0)/255);
				SetDlgItemTextA(hwndDlg,IDC_INACTIVEPERC,str);
				wsprintfA(str,"%d%%",100*SendDlgItemMessage(hwndDlg,IDC_TRANSACTIVE,TBM_GETPOS,0,0)/255);
				SetDlgItemTextA(hwndDlg,IDC_ACTIVEPERC,str);
			}
			if(wParam!=0x12345678) SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case WM_NOTIFY:
			switch (((LPNMHDR)lParam)->code)
			{
				case PSN_APPLY:
					DBWriteContactSettingByte(NULL,"CList","OnTop",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_ONTOP));
					SetWindowPos(pcli->hwndContactList, IsDlgButtonChecked(hwndDlg,IDC_ONTOP)?HWND_TOPMOST:HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

					DBWriteContactSettingByte(NULL,"CList","ToolWindow",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_TOOLWND));
					DBWriteContactSettingByte(NULL,"CList","BringToFront",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_BRINGTOFRONT));
					if(IsDlgButtonChecked(hwndDlg,IDC_TOOLWND)) {
						// Window must be hidden to dynamically remove the taskbar button.
						// See http://msdn.microsoft.com/library/en-us/shellcc/platform/shell/programmersguide/shell_int/shell_int_programming/taskbar.asp
						WINDOWPLACEMENT p;
						p.length = sizeof(p);
						GetWindowPlacement(pcli->hwndContactList,&p);
						ShowWindow(pcli->hwndContactList,SW_HIDE);
						SetWindowLong(pcli->hwndContactList,GWL_EXSTYLE,GetWindowLong(pcli->hwndContactList,GWL_EXSTYLE)|WS_EX_TOOLWINDOW|WS_EX_WINDOWEDGE);
						SetWindowPlacement(pcli->hwndContactList,&p);
					}
					else
						SetWindowLong(pcli->hwndContactList,GWL_EXSTYLE,GetWindowLong(pcli->hwndContactList,GWL_EXSTYLE)&~WS_EX_TOOLWINDOW);

					if (IsDlgButtonChecked(hwndDlg,IDC_ONDESKTOP)) {
						HWND hProgMan=FindWindowA("Progman",NULL);
						if (IsWindow(hProgMan)) SetParent(pcli->hwndContactList,hProgMan);
					} else {
						SetParent(pcli->hwndContactList,NULL);
					}

					DBWriteContactSettingByte(NULL,"CLUI","ShowCaption",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_SHOWCAPTION));
					DBWriteContactSettingByte(NULL,"CLUI","ShowMainMenu",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_SHOWMAINMENU));
					DBWriteContactSettingByte(NULL,"CLUI","ClientAreaDrag",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_CLIENTDRAG));

					if(IsDlgButtonChecked(hwndDlg,IDC_SHOWCAPTION))
						SetWindowLong(pcli->hwndContactList,GWL_STYLE,GetWindowLong(pcli->hwndContactList,GWL_STYLE)|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX);
					else
						SetWindowLong(pcli->hwndContactList,GWL_STYLE,GetWindowLong(pcli->hwndContactList,GWL_STYLE)&~(WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX));

					if(!IsDlgButtonChecked(hwndDlg,IDC_SHOWMAINMENU)) SetMenu(pcli->hwndContactList,NULL);
					else SetMenu(pcli->hwndContactList,hMenuMain);

					SetWindowPos(pcli->hwndContactList,0,0,0,0,0,SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
					RedrawWindow(pcli->hwndContactList,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);

					DBWriteContactSettingByte(NULL,"CList","Min2Tray",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_MIN2TRAY));
					if(IsIconic(pcli->hwndContactList) && !IsDlgButtonChecked(hwndDlg,IDC_TOOLWND))
						ShowWindow(pcli->hwndContactList,IsDlgButtonChecked(hwndDlg,IDC_MIN2TRAY)?SW_HIDE:SW_SHOW);

					{	TCHAR title[256];
						GetDlgItemText(hwndDlg,IDC_TITLETEXT,title,SIZEOF(title));
						DBWriteContactSettingTString(NULL,"CList","TitleText",title);
						SetWindowText(pcli->hwndContactList,title);
					}
					DBWriteContactSettingByte(NULL,"CLUI","FadeInOut",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_FADEINOUT));
					DBWriteContactSettingByte(NULL,"CLUI","AutoSize",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_AUTOSIZE));
					DBWriteContactSettingByte(NULL,"CLUI","MaxSizeHeight",(BYTE)GetDlgItemInt(hwndDlg,IDC_MAXSIZEHEIGHT,NULL,FALSE));
					DBWriteContactSettingByte(NULL,"CLUI","AutoSizeUpward",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_AUTOSIZEUPWARD));
					DBWriteContactSettingByte(NULL,"CList","AutoHide",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_AUTOHIDE));
					DBWriteContactSettingWord(NULL,"CList","HideTime",(WORD)SendDlgItemMessage(hwndDlg,IDC_HIDETIMESPIN,UDM_GETPOS,0,0));

					DBWriteContactSettingByte(NULL,"CList","Transparent",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT));
					DBWriteContactSettingByte(NULL,"CList","Alpha",(BYTE)SendDlgItemMessage(hwndDlg,IDC_TRANSACTIVE,TBM_GETPOS,0,0));
					DBWriteContactSettingByte(NULL,"CList","AutoAlpha",(BYTE)SendDlgItemMessage(hwndDlg,IDC_TRANSINACTIVE,TBM_GETPOS,0,0));
					DBWriteContactSettingByte(NULL,"CList","WindowShadow",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_DROPSHADOW));
					DBWriteContactSettingByte(NULL,"CList","OnDesktop",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_ONDESKTOP));
					if(IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT))	{
						SetWindowLong(pcli->hwndContactList, GWL_EXSTYLE, GetWindowLong(pcli->hwndContactList, GWL_EXSTYLE) | WS_EX_LAYERED);
						if(MySetLayeredWindowAttributes) MySetLayeredWindowAttributes(pcli->hwndContactList, RGB(0,0,0), (BYTE)DBGetContactSettingByte(NULL,"CList","AutoAlpha",SETTING_AUTOALPHA_DEFAULT), LWA_ALPHA);
					}
					else {
						SetWindowLong(pcli->hwndContactList, GWL_EXSTYLE, GetWindowLong(pcli->hwndContactList, GWL_EXSTYLE) & ~WS_EX_LAYERED);
					}
					SendMessage(pcli->hwndContactTree,WM_SIZE,0,0);	//forces it to send a cln_listsizechanged

					if (!ServiceExists("ExtraIcon/Register"))
					{
						DBWriteContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_PROTO",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_EXTRA_PROTO));
						DBWriteContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_WEB",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_EXTRA_WEB));
						DBWriteContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_EMAIL",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_EXTRA_EMAIL));
						DBWriteContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_SMS",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_EXTRA_CELLULAR));
						DBWriteContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_ADV1",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_EXTRA_ADV1));
						DBWriteContactSettingByte(NULL,CLUIFrameModule,"EXTRA_ICON_ADV2",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_EXTRA_ADV2));
						//SetAllExtraIcons()	
						ReAssignExtraIcons();
					}			
					
					return TRUE;
			}
			break;
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProcSBarOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);
			CheckDlgButton(hwndDlg, IDC_SHOWSBAR, DBGetContactSettingByte(NULL,"CLUI","ShowSBar",1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_USECONNECTINGICON, DBGetContactSettingByte(NULL,"CLUI","UseConnectingIcon",1) ? BST_CHECKED : BST_UNCHECKED);
			{	BYTE showOpts=DBGetContactSettingByte(NULL,"CLUI","SBarShow",1);
				CheckDlgButton(hwndDlg, IDC_SHOWICON, showOpts&1 ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_SHOWPROTO, showOpts&2 ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_SHOWSTATUS, showOpts&4 ? BST_CHECKED : BST_UNCHECKED);
			}
			CheckDlgButton(hwndDlg, IDC_RIGHTSTATUS, DBGetContactSettingByte(NULL,"CLUI","SBarRightClk",0) ? BST_UNCHECKED : BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_RIGHTMIRANDA, !IsDlgButtonChecked(hwndDlg,IDC_RIGHTSTATUS) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_EQUALSECTIONS, DBGetContactSettingByte(NULL,"CLUI","EqualSections",0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SBPANELBEVEL, DBGetContactSettingByte(NULL,"CLUI","SBarBevel",1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWSIZEGRIP, DBGetContactSettingByte(NULL,"CLUI","SBarUseSizeGrip",1) ? BST_CHECKED : BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_USEOWNERDRAW, DBGetContactSettingByte(NULL,"CLUI","UseOwnerDrawStatusBar",1) ? BST_CHECKED : BST_UNCHECKED);
			
			SendDlgItemMessage(hwndDlg,IDC_BKGCOLOUR,CPM_SETCOLOUR,0,DBGetContactSettingDword(NULL,"CLUI","SBarBKColor",CLR_DEFAULT));

			if(!IsDlgButtonChecked(hwndDlg,IDC_SHOWSBAR)) {
				EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWICON),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWPROTO),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWSTATUS),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_RIGHTSTATUS),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_RIGHTMIRANDA),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_EQUALSECTIONS),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_SBPANELBEVEL),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWSIZEGRIP),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_USECONNECTINGICON),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_USEOWNERDRAW),FALSE);

			}
			return TRUE;
		case WM_COMMAND:
			if(LOWORD(wParam)==IDC_SHOWSBAR) {
				EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWICON),IsDlgButtonChecked(hwndDlg,IDC_SHOWSBAR));
				EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWPROTO),IsDlgButtonChecked(hwndDlg,IDC_SHOWSBAR));
				EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWSTATUS),IsDlgButtonChecked(hwndDlg,IDC_SHOWSBAR));
				EnableWindow(GetDlgItem(hwndDlg,IDC_RIGHTSTATUS),IsDlgButtonChecked(hwndDlg,IDC_SHOWSBAR));
				EnableWindow(GetDlgItem(hwndDlg,IDC_RIGHTMIRANDA),IsDlgButtonChecked(hwndDlg,IDC_SHOWSBAR));
				EnableWindow(GetDlgItem(hwndDlg,IDC_EQUALSECTIONS),IsDlgButtonChecked(hwndDlg,IDC_SHOWSBAR));
				EnableWindow(GetDlgItem(hwndDlg,IDC_SBPANELBEVEL),IsDlgButtonChecked(hwndDlg,IDC_SHOWSBAR));
				EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWSIZEGRIP),IsDlgButtonChecked(hwndDlg,IDC_SHOWSBAR));
				EnableWindow(GetDlgItem(hwndDlg,IDC_USECONNECTINGICON),IsDlgButtonChecked(hwndDlg,IDC_SHOWSBAR));	
				EnableWindow(GetDlgItem(hwndDlg,IDC_USEOWNERDRAW),IsDlgButtonChecked(hwndDlg,IDC_SHOWSBAR));	

			}
			if (LOWORD(wParam)==IDC_DEFBKCOLOR)
			{
							SendDlgItemMessage(hwndDlg,IDC_BKGCOLOUR,CPM_SETCOLOUR,0,CLR_DEFAULT);
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);		
			}

			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case WM_NOTIFY:
			switch (((LPNMHDR)lParam)->code)
			{
				case PSN_APPLY:
					{
					int frameopt;

					COLORREF col;
					col=SendDlgItemMessage(hwndDlg,IDC_BKGCOLOUR,CPM_GETCOLOUR,0,0);
					DBWriteContactSettingDword(NULL,"CLUI","SBarBKColor",col);
						
						
						
					DBWriteContactSettingByte(NULL,"CLUI","ShowSBar",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_SHOWSBAR));
					DBWriteContactSettingByte(NULL,"CLUI","SBarShow",(BYTE)((IsDlgButtonChecked(hwndDlg,IDC_SHOWICON)?1:0)|(IsDlgButtonChecked(hwndDlg,IDC_SHOWPROTO)?2:0)|(IsDlgButtonChecked(hwndDlg,IDC_SHOWSTATUS)?4:0)));
					DBWriteContactSettingByte(NULL,"CLUI","SBarRightClk",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_RIGHTMIRANDA));
					DBWriteContactSettingByte(NULL,"CLUI","EqualSections",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_EQUALSECTIONS));
					DBWriteContactSettingByte(NULL,"CLUI","SBarBevel",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_SBPANELBEVEL));
					DBWriteContactSettingByte(NULL,"CLUI","SBarUseSizeGrip",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_SHOWSIZEGRIP));
					
					DBWriteContactSettingByte(NULL,"CLUI","UseConnectingIcon",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_USECONNECTINGICON));
					DBWriteContactSettingByte(NULL,"CLUI","UseOwnerDrawStatusBar",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_USEOWNERDRAW));
					UseOwnerDrawStatusBar=DBGetContactSettingByte(NULL,"CLUI","UseOwnerDrawStatusBar",0);
/*
					if(IsDlgButtonChecked(hwndDlg,IDC_SHOWSBAR)) ShowWindow(pcli->hwndStatus,SW_SHOW);
					else ShowWindow(pcli->hwndStatus,SW_HIDE);
*/
					
					frameopt=CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS,MAKEWPARAM(FO_FLAGS,hFrameHelperStatusBar),0);
				
					frameopt=frameopt & (~F_VISIBLE);		

					if(IsDlgButtonChecked(hwndDlg,IDC_SHOWSBAR)) 
					{
						ShowWindow(pcli->hwndStatus,SW_SHOW);
						frameopt|=F_VISIBLE;
					}
					else 
					{
						ShowWindow(pcli->hwndStatus,SW_HIDE);
					}
				    CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS,MAKEWPARAM(FO_FLAGS,hFrameHelperStatusBar),frameopt);

					SendMessage(pcli->hwndContactList,WM_SIZE,0,0);
					
					OnStatusBarBackgroundChange();
					CluiProtocolStatusChanged(0,0);			
					return TRUE;
					}
			}
			break;
	}
	return FALSE;
}
