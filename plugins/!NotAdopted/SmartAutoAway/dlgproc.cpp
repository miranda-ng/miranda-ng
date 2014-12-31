/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-06 Miranda ICQ/IM project,
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
#include "globals.h"

INT_PTR CALLBACK DlgProcAutoAwayOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
//	char str[MAXMODULELABELLENGTH];
	static short int tabprotosel;
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			short int t,p,protoListed;
			TCITEM tci;
			RECT theTabSpace;
			RECT rcClient;
			{
				RECT rcTab, rcDlg;
				TabCtrl_GetItemRect(GetDlgItem(hwndDlg, IDC_OPT_SAA_TAB),0,&rcTab);
				theTabSpace.top = rcTab.bottom; // the size of the tab
				GetWindowRect(GetDlgItem(hwndDlg, IDC_OPT_SAA_TAB), &rcTab);
				GetWindowRect(hwndDlg, &rcDlg);
				theTabSpace.bottom = rcTab.bottom -rcTab.top  -theTabSpace.top;
				theTabSpace.top =  rcTab.top -rcDlg.top +theTabSpace.top;
				theTabSpace.left = rcTab.left - rcDlg.left;
				theTabSpace.right = rcTab.right-rcTab.left;
			}
			theDialog = hwndDlg;
			TranslateDialogDefault(hwndDlg);
			protoListed = 0;
			for (p = 0 ; p < protoCount ;p++) {
				int caps=0;
				if ( !isInterestingProto(p) ) continue;
				caps = CallProtoService(accounts[p]->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0);
				PROTOACCOUNT* protoAccount=ProtoGetAccount(accounts[p]->szModuleName);
				if ((int)protoAccount!=CALLSERVICE_NOTFOUND){
					SendDlgItemMessage(hwndDlg, IDC_AAPROTOCOL, CB_ADDSTRING, 0, (LPARAM)protoAccount->tszAccountName );
				} else SendDlgItemMessageA(hwndDlg, IDC_AAPROTOCOL, CB_ADDSTRING, 0, (LPARAM)accounts[p]->szModuleName );
				protoListed++;
			}
			tabprotosel = db_get_w(NULL, AA_MODULE, AA_PROTOSELECTION, 0);
			t = (tabprotosel&0x0F00) >> 8; if (t>3) t=3;
			p = (tabprotosel&0x00FF);
			if ((p>=protoListed)) p= protoListed-1;
			SendDlgItemMessage(hwndDlg, IDC_AAPROTOCOL, CB_SETCURSEL, p, 0);
			SetDlgItems(hwndDlg, courProtocolSelection = GetCourSelProtocol(p));

			tci.mask = TCIF_PARAM|TCIF_TEXT;

#ifdef SAA_PLUGIN
			theDialogAA =  CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_OPT_SAA_AA), hwndDlg, DlgProcOptsAA);
#else
			theDialogAA =  CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_OPT_SAA_AA), hwndDlg, DlgProcOptsAA);
#endif
			tci.lParam = (LPARAM)theDialogAA;
			tci.pszText = TranslateT("Statuses");
			GetClientRect(theDialogAA,&rcClient);
			SendMessage(GetDlgItem(hwndDlg, IDC_OPT_SAA_TAB), TCM_INSERTITEM, 0, (LPARAM)&tci);
			MoveWindow((HWND)theDialogAA,theTabSpace.left+(theTabSpace.right-rcClient.right)/2,
				theTabSpace.top+(theTabSpace.bottom-rcClient.bottom)/2,
				rcClient.right,rcClient.bottom,1);
			ShowWindow((HWND)tci.lParam, (t==0)?SW_SHOW:SW_HIDE);


#ifdef SAA_PLUGIN
			theDialogIdle = CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_OPT_SAA_IDLE), hwndDlg, DlgProcOptsIdle);
#else
			theDialogIdle = CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_OPT_SAA_IDLE), hwndDlg, DlgProcOptsIdle);
#endif
			tci.lParam = (LPARAM)theDialogIdle;
			tci.pszText = TranslateT("Idle");
			GetClientRect(theDialogIdle,&rcClient);
			SendMessage(GetDlgItem(hwndDlg, IDC_OPT_SAA_TAB), TCM_INSERTITEM, (WPARAM)1, (LPARAM)&tci);
			MoveWindow((HWND)theDialogIdle,theTabSpace.left+(theTabSpace.right-rcClient.right)/2,
				theTabSpace.top+(theTabSpace.bottom-rcClient.bottom)/2,
				rcClient.right,rcClient.bottom,1);
			ShowWindow((HWND)tci.lParam, (t==1)?SW_SHOW:SW_HIDE);

#ifdef SAA_PLUGIN
			theDialogIdleMessages = CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_OPT_SAA_IDLEMESSAGES), hwndDlg, DlgProcOptsIdleMessages);
#else
			theDialogIdleMessages = CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_OPT_SAA_IDLEMESSAGES), hwndDlg, DlgProcOptsIdleMessages);
#endif
			tci.lParam = (LPARAM)theDialogIdleMessages;
			tci.pszText = TranslateT("Status messages");
			GetClientRect(theDialogIdleMessages,&rcClient);
			SendMessage(GetDlgItem(hwndDlg, IDC_OPT_SAA_TAB), TCM_INSERTITEM, (WPARAM)2, (LPARAM)&tci);
			MoveWindow((HWND)theDialogIdleMessages,theTabSpace.left+(theTabSpace.right-rcClient.right)/2,
				theTabSpace.top+(theTabSpace.bottom-rcClient.bottom)/2,
				rcClient.right,rcClient.bottom,1);
			ShowWindow((HWND)tci.lParam, (t==2)?SW_SHOW:SW_HIDE);

#ifdef SAA_PLUGIN
			theDialogReconnect = CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_OPT_SAA_RECONNECT), hwndDlg, DlgProcOptsReconnect);
#else
			theDialogReconnect = CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_OPT_SAA_RECONNECT), hwndDlg, DlgProcOptsReconnect);
#endif
			tci.lParam = (LPARAM)theDialogReconnect;
			tci.pszText = TranslateT("Reconnect");
			GetClientRect(theDialogReconnect,&rcClient);
			SendMessage(GetDlgItem(hwndDlg, IDC_OPT_SAA_TAB), TCM_INSERTITEM, (WPARAM)3, (LPARAM)&tci);
			MoveWindow((HWND)theDialogReconnect,theTabSpace.left+(theTabSpace.right-rcClient.right)/2,
				theTabSpace.top+(theTabSpace.bottom-rcClient.bottom)/2,
				rcClient.right,rcClient.bottom,1);
			ShowWindow((HWND)tci.lParam, (t==3)?SW_SHOW:SW_HIDE);
			TabCtrl_SetCurSel(GetDlgItem(hwndDlg, IDC_OPT_SAA_TAB),t);

			if(enableThemeDialogTexture) {
					enableThemeDialogTexture(theDialogAA, ETDT_ENABLETAB);
					enableThemeDialogTexture(theDialogIdle, ETDT_ENABLETAB);
					enableThemeDialogTexture(theDialogIdleMessages, ETDT_ENABLETAB);
					enableThemeDialogTexture(theDialogReconnect, ETDT_ENABLETAB);
			}
			return TRUE;
		}

		case WM_COMMAND:
		{
            switch (LOWORD(wParam)) {
				case IDC_AAPROTOCOL:
					{
						int cour = SendDlgItemMessage(hwndDlg, IDC_AAPROTOCOL, CB_GETCURSEL, 0, 0);
						courProtocolSelection = GetCourSelProtocol(cour);
						if ( HIWORD(wParam) != CBN_SELCHANGE ) return TRUE;
						db_set_w(NULL, AA_MODULE, AA_PROTOSELECTION,
							(unsigned short)(tabprotosel=courProtocolSelection|(tabprotosel&0xFF00)));
						SetDlgItems(hwndDlg, courProtocolSelection);
						SetDlgItemsReconnect(theDialogReconnect,courProtocolSelection);
						SetDlgItemsAA(theDialogAA,courProtocolSelection);
						SetDlgItemsIdleMessages(theDialogIdleMessages,courProtocolSelection);
						break;
					}
				case IDC_APPLYTOALL:
					{
						int i;
						for (i=0;i<protoCount;i++){
							if (isInterestingProto(i)) {
								awayStatuses[i]=awayStatuses[courProtocolSelection];
								onlyIfBits[i]=onlyIfBits[courProtocolSelection];
								reconnectOpts[i]=reconnectOpts[courProtocolSelection];
								if (messCaps[i]) {
									_tcscpy(mesgHere[i],mesgHerePerm[courProtocolSelection]);
									_tcscpy(mesgShort[i],mesgShortPerm[courProtocolSelection]);
									_tcscpy(mesgLong[i],mesgLongPerm[courProtocolSelection]);
								}
							}
						}
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						break;
					}
			} //switch
			break;
		}
		case PSM_CHANGED:
#ifdef _DEBUG
			MessageBoxA(hwndDlg,"Child dialog changed","EventHapened",0);
#endif
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, (unsigned int)hwndDlg, 0);
			break;
		case WM_NOTIFY:
		{
		  switch(((LPNMHDR)lParam)->idFrom) {
            case 0:	{
			  BOOL CommandApply;
			  if ( (CommandApply = lParam && ((LPNMHDR)lParam)->code == PSN_APPLY) || (lParam && ((LPNMHDR)lParam)->code == PSN_RESET) ) {
				int j=0;
#ifdef _DEBUG
				//MessageBox(hwndDlg,CommandApply?"Apply":"Cancel","EventHapened",0);
#endif
				for (j = 0 ; j < protoCount ; j++) {
					if ( isInterestingProto(j) ) {
						char str[256];
						if (CommandApply) {
							onlyIfBitsPerm[j] = onlyIfBits[j];
							awayStatusesPerm[j] = awayStatuses[j];
							reconnectOptsPerm[j] = reconnectOpts[j];
							sprintf(str,AA_BITSONLYIF,accounts[j]->szModuleName);
							db_set_dw(NULL,AA_MODULE,str,onlyIfBitsPerm[j]);
							sprintf(str,AA_AWAYSTATUSES,accounts[j]->szModuleName);
							db_set_w(NULL,AA_MODULE,str,awayStatusesPerm[j]);
							sprintf(str,AA_RECONNECTOPTS,accounts[j]->szModuleName);
							db_set_dw(NULL,AA_MODULE,str,reconnectOptsPerm[j]);
							if (messCaps[j]) {
								idleMessOptsPerm[j] = idleMessOpts[j];
								sprintf(str,idleMsgOptionsName,accounts[j]->szModuleName);
								db_set_dw(NULL,AA_MODULE,str,idleMessOptsPerm[j]);

								sprintf(str,idleMsgOptionsTextHere,accounts[j]->szModuleName);
								_tcscpy(mesgHerePerm[j],mesgHere[j]);
								db_set_ts(NULL,AA_MODULE,str,mesgHerePerm[j]);

								sprintf(str,idleMsgOptionsTextShort,accounts[j]->szModuleName);
								_tcscpy(mesgShortPerm[j],mesgShort[j]);
								db_set_ts(NULL,AA_MODULE,str,mesgShortPerm[j]);

								sprintf(str,idleMsgOptionsTextLong,accounts[j]->szModuleName);
								_tcscpy(mesgLongPerm[j],mesgLong[j]);
								db_set_ts(NULL,AA_MODULE,str,mesgLongPerm[j]);
							}
						} else {
							onlyIfBits[j] = onlyIfBitsPerm[j];
							awayStatuses[j] = awayStatusesPerm[j];
							reconnectOpts[j] = reconnectOptsPerm[j];
							idleMessOpts[j] = idleMessOptsPerm[j];
							if (messCaps[j]) {
								_tcscpy(mesgHere[j],mesgHerePerm[j]);
								_tcscpy(mesgShort[j],mesgShortPerm[j]);
								_tcscpy(mesgLong[j],mesgLongPerm[j]);
							}
						}
					}
				}
				if (CommandApply) {
					idleOptsPerm = idleOpts;
					db_set_dw(NULL,AA_MODULE,AA_IDLEOPTIONS,idleOptsPerm);
				} else {
					idleOpts = idleOptsPerm;
				}
			  } //if PSN_APPLY
              break;
            case IDC_OPT_SAA_TAB:
               switch (((LPNMHDR)lParam)->code)
               {
                  case TCN_SELCHANGING:
                     {
                        TCITEM tci;
                        tci.mask = TCIF_PARAM;
                        TabCtrl_GetItem(GetDlgItem(hwndDlg,IDC_OPT_SAA_TAB),TabCtrl_GetCurSel(GetDlgItem(hwndDlg,IDC_OPT_SAA_TAB)),&tci);
                        ShowWindow((HWND)tci.lParam,SW_HIDE);
                     }
                  break;
                  case TCN_SELCHANGE:
                     {
                        TCITEM tci;
						short int t;
                        tci.mask = TCIF_PARAM;
						t = TabCtrl_GetCurSel(GetDlgItem(hwndDlg,IDC_OPT_SAA_TAB));
                        TabCtrl_GetItem(GetDlgItem(hwndDlg,IDC_OPT_SAA_TAB),t,&tci);
						db_set_w(NULL, AA_MODULE, AA_PROTOSELECTION,
							(unsigned short)(tabprotosel=(t<<8)|(tabprotosel&0xF0FF)));
                        ShowWindow((HWND)tci.lParam,SW_SHOW);
						{ BOOL enableW = (tci.lParam!=(long)theDialogIdle);
							EnableWindow(GetDlgItem(hwndDlg,IDC_AAPROTOCOL),enableW);
							EnableWindow(GetDlgItem(hwndDlg,IDC_APPLYTOALL),enableW);
						}
                     }
                  break;
               }
			   break;
			}
		  }//end case(LPNMHDR)lParam)->idFrom

		}
//WTF???		if(wParam!=0x12345678) SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	}
	return FALSE;
}
