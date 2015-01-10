/*
Miranda-IM SMS Plugin
Copyright (C) 2001-2  Richard Hughes
Copyright (C) 2007-2014  Rozhuk Ivan

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
---------------------------------------------------------------------------

This was the original words.
This plugin was modified by Ariel Shulman (NuKe007).
For any comments, problems, etc. contact me at Miranda-IM forums or E-Mail or ICQ.
All the information needed you can find at www.nuke007.tk
Enjoy the code and use it smartly!
*/

#include "common.h"

#define TIMERID_MSGSEND		0
#define TIMEDOUT_CANCEL		0
#define TIMEDOUT_RETRY		1
#define DM_TIMEOUTDECIDED	(WM_USER+18)


//Defnition needed to the SMS window list
typedef struct
{
	LIST_MT_ITEM lmtListMTItem;
	HWND hWnd;
	HBRUSH hBkgBrush;
	HANDLE hProcess;
	MCONTACT hContact;
	MCONTACT hMyContact;
	HTREEITEM hItemSend;
	BOOL bMultiple;
	size_t dwContactsListCount;
	MCONTACT *phContactsList;
	DBEVENTINFO *pdbei;
} SEND_SMS_WINDOW_DATA;

void			AddContactPhonesToCombo		(HWND hWnd,MCONTACT hContact);
void			SendSMSWindowFillTreeView	(HWND hWnd);
size_t			GetSMSMessageLenMax			(HWND hWndDlg);

#define GET_WINDOW_DATA(hWndDlg)	((SEND_SMS_WINDOW_DATA*)GetWindowLongPtr(hWndDlg,GWLP_USERDATA))

DWORD SendSMSWindowInitialize()
{
	return ListMTInitialize(&ssSMSSettings.lmtSendSMSWindowsListMT, 0);
}

void SendSMSWindowDestroy()
{
	SEND_SMS_WINDOW_DATA *psswdWindowData;

	ListMTLock(&ssSMSSettings.lmtSendSMSWindowsListMT);
	while (ListMTItemGetFirst(&ssSMSSettings.lmtSendSMSWindowsListMT, NULL, (LPVOID*)&psswdWindowData) == NO_ERROR)
		SendSMSWindowRemove(psswdWindowData->hWnd);

	ListMTUnLock(&ssSMSSettings.lmtSendSMSWindowsListMT);
	ListMTDestroy(&ssSMSSettings.lmtSendSMSWindowsListMT);
}

INT_PTR CALLBACK SendSmsDlgProc(HWND hWndDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	SEND_SMS_WINDOW_DATA *psswdWindowData=GET_WINDOW_DATA(hWndDlg);

	switch(message){
	case WM_INITDIALOG:
		TranslateDialogDefault(hWndDlg); //Translate intially - bid
		////////AddWinHandle(GetDlgItem(hWndDlg,IDC_MESSAGE));
		
		psswdWindowData=(SEND_SMS_WINDOW_DATA*)lParam;
		SetWindowLongPtr(hWndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
		
		mir_subclassWindow(GetDlgItem(hWndDlg, IDC_MESSAGE), MessageSubclassProc);

		SendMessage(hWndDlg,WM_SETICON,ICON_BIG,(LPARAM)LoadSkinnedIcon(SKINICON_OTHER_SMS));
		SendDlgItemMessage(hWndDlg,IDC_HISTORY,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadSkinnedIcon(SKINICON_OTHER_HISTORY));
		{
			HICON hIcon;
			HIMAGELIST hIml=ImageList_Create(16,16,ILC_COLOR32|ILC_MASK,0,30);
			TreeView_SetImageList(GetDlgItem(hWndDlg,IDC_NUMBERSLIST),hIml,TVSIL_NORMAL);	
			hIcon=LoadSkinnedIcon(SKINICON_OTHER_NOTICK);
			ImageList_AddIcon(hIml,hIcon);
			hIcon=LoadSkinnedIcon(SKINICON_OTHER_TICK);
			ImageList_AddIcon(hIml,hIcon);
			hIcon=(HICON)LoadImage(ssSMSSettings.hInstance,MAKEINTRESOURCE(IDI_HALFTICK),IMAGE_ICON,0,0,LR_SHARED);
			ImageList_AddIcon(hIml,hIcon);
		}
		{
			LOGFONT lf;
			HFONT hFont=(HFONT)SendDlgItemMessage(hWndDlg,IDC_MESSAGE,WM_GETFONT,0,0);
			if (hFont && hFont!=(HFONT)SendDlgItemMessage(hWndDlg,IDOK,WM_GETFONT,0,0)) DeleteObject(hFont);
			LoadMsgDlgFont(MSGFONTID_MESSAGEAREA,&lf,NULL);
			hFont=CreateFontIndirect(&lf);
			SendDlgItemMessage(hWndDlg,IDC_MESSAGE,WM_SETFONT,(WPARAM)hFont,MAKELPARAM(TRUE,0));

			COLORREF colour=db_get_dw(NULL,SRMMMOD,SRMSGSET_INPBKGCOLOUR,SRMSGDEFSET_BKGCOLOUR);
			psswdWindowData->hBkgBrush=CreateSolidBrush(colour);
		}
		{
			HWND hwndToolTips=CreateWindowEx(WS_EX_TOPMOST,TOOLTIPS_CLASS,TEXT(""),WS_POPUP,0,0,0,0,NULL,NULL,GetModuleHandle(NULL),NULL);
			TOOLINFO ti;
			memset(&ti, 0, sizeof(ti));
			ti.cbSize=sizeof(ti);
			ti.uFlags=TTF_IDISHWND|TTF_SUBCLASS;
			ti.uId=(UINT)GetDlgItem(hWndDlg,IDC_HISTORY);
			ti.lpszText=TranslateT("View User's History");
			SendMessage(hwndToolTips,TTM_ADDTOOL,0,(LPARAM)&ti);
			ti.uId=(UINT)GetDlgItem(hWndDlg,IDC_ADDNUMBER);
			ti.lpszText=TranslateT("Add Number To The Multiple List");
			SendMessage(hwndToolTips,TTM_ADDTOOL,0,(LPARAM)&ti);
			ti.uId=(UINT)GetDlgItem(hWndDlg,IDC_SAVENUMBER);
			ti.lpszText=TranslateT("Save Number To The User's Details Phonebook");
			SendMessage(hwndToolTips,TTM_ADDTOOL,0,(LPARAM)&ti);
			ti.uId=(UINT)GetDlgItem(hWndDlg,IDC_MULTIPLE);
			ti.lpszText=TranslateT("Show/Hide Multiple List");
			SendMessage(hwndToolTips,TTM_ADDTOOL,0,(LPARAM)&ti);
			ti.uId=(UINT)GetDlgItem(hWndDlg,IDC_COUNT);
			ti.lpszText=TranslateT("Shows How Much Chars You've Typed");
			SendMessage(hwndToolTips,TTM_ADDTOOL,0,(LPARAM)&ti);
		}
		{
			RECT rcWin,rcList;
			GetWindowRect(hWndDlg,&rcWin);
			GetWindowRect(GetDlgItem(hWndDlg,IDC_NUMBERSLIST),&rcList);
			SetWindowPos(hWndDlg,0,rcWin.left,rcWin.top,rcWin.right-rcWin.left - (rcList.right-rcList.left) - 10,rcWin.bottom - rcWin.top,SWP_NOZORDER|SWP_NOMOVE);
		}

		SendSMSWindowUpdateAccountList(hWndDlg);

		{	
			size_t dwSignLen = 0;

			if (DB_SMS_GetByte(NULL,"UseSignature",SMS_DEFAULT_USESIGNATURE))
			if (DB_SMS_GetStaticStringW(NULL,"Signature",tszSign,SIZEOF(tszSign),&dwSignLen))
			{
				SetDlgItemText(hWndDlg,IDC_MESSAGE,tszSign);

				if (DB_SMS_GetByte(NULL,"SignaturePos",SMS_DEFAULT_SIGNATUREPOS)) SendDlgItemMessage(hWndDlg,IDC_MESSAGE,EM_SETSEL,dwSignLen,dwSignLen);
				EnableWindow(GetDlgItem(hWndDlg,IDOK),dwSignLen);
			}

			TCHAR tszSign[1024];
			mir_sntprintf(tszSign,SIZEOF(tszSign),_T("%d/%d"),dwSignLen,GetSMSMessageLenMax(hWndDlg));
			SetDlgItemText(hWndDlg,IDC_COUNT,tszSign);
		}

		if (Utils_RestoreWindowPosition(hWndDlg,(DB_SMS_GetByte(NULL,"SavePerContact",0)? psswdWindowData->hMyContact:NULL),PROTOCOL_NAMEA,"send"))
		{// def pos
			SetWindowPos(hWndDlg,0,200,200,400,350,SWP_NOZORDER);
		}
		InvalidateRect(GetDlgItem(hWndDlg,IDC_MESSAGE),NULL,FALSE);
		return TRUE;
	case WM_GETMINMAXINFO:
		if (psswdWindowData->bMultiple)
		{
			((LPMINMAXINFO)lParam)->ptMinTrackSize.x=461; 
			((LPMINMAXINFO)lParam)->ptMinTrackSize.y=230;
		}else{
			((LPMINMAXINFO)lParam)->ptMinTrackSize.x=300; 
			((LPMINMAXINFO)lParam)->ptMinTrackSize.y=230;
		}
		break;
	case WM_SIZE:
		{
			int cx,cy;
			RECT rcWin;
			GetWindowRect(hWndDlg,&rcWin);
			if (psswdWindowData->bMultiple)
			{
				cx = rcWin.right-rcWin.left-181;
				cy = rcWin.bottom-rcWin.top;
			}else{
				cx = rcWin.right-rcWin.left;
				cy = rcWin.bottom-rcWin.top;
			}
			cx -= 10; /* XXX: fix window sizes for aero. */
			cy -= 10;
			SetWindowPos(GetDlgItem(hWndDlg,IDC_MESSAGE),0,0,0,cx - 14,cy - 132,SWP_NOZORDER|SWP_NOMOVE);
			SetWindowPos(GetDlgItem(hWndDlg,IDC_NAME),0,0,0,(cx*35)/100,20,SWP_NOZORDER|SWP_NOMOVE);
			SetWindowPos(GetDlgItem(hWndDlg,IDC_ACCOUNTS),0,0,0,(cx*35)/100,20,SWP_NOZORDER|SWP_NOMOVE);
			SetWindowPos(GetDlgItem(hWndDlg,IDC_ADDRESS),0,cx - (cx*35)/100 - 11,5,(cx*35)/100,20,SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWndDlg,IDC_ST_ADDRESS),0,cx - (cx*35)/100 - 68,5,50,20,SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWndDlg,IDC_MULTIPLE),0,cx - ((cx*35)/100 + 35)/2 - 11,30,((cx*35)/100 + 35)/2,20,SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWndDlg,IDC_SAVENUMBER),0,cx - (cx*35)/100 - 58,30,((cx*35)/100 + 35)/2,20,SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWndDlg,IDC_HISTORY),0,0,0,20,20,SWP_NOZORDER|SWP_NOMOVE);
			SetWindowPos(GetDlgItem(hWndDlg,IDC_COUNT),0,cx - 61,52,50,15,SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWndDlg,IDC_ST_CHARS),0,cx - 106,52,40,15,SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWndDlg,IDC_ADDNUMBER),0,cx,5,170,20,SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWndDlg,IDC_NUMBERSLIST),0,cx,25,170,cy - 90,SWP_NOZORDER);
			//SetWindowPos(GetDlgItem(hWndDlg,IDC_REMOVENUMBER),0,cx,cy - 110 + 25,170,20,SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWndDlg,IDOK),0,cx/2 - 87,cy - 60,80,25,SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWndDlg,IDCANCEL),0,cx/2 + 7,cy - 60,80,25,SWP_NOZORDER);
			RedrawWindow(hWndDlg,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);
		}
		break;
	case WM_TIMER:
		if (wParam==TIMERID_MSGSEND)
		{
			HWND hwndTimeOut;
			TCHAR tszMessage[1028],tszPhone[MAX_PHONE_LEN];

			if (psswdWindowData->bMultiple)
			{
				TVITEM tvi;
				tvi.mask=TVIF_TEXT;
				tvi.hItem=SendSMSWindowHItemSendGet(hWndDlg);
				tvi.pszText=tszPhone;
				tvi.cchTextMax=SIZEOF(tszPhone);
				TreeView_GetItem(GetDlgItem(hWndDlg,IDC_NUMBERSLIST),&tvi);
			}else{
				GetDlgItemText(hWndDlg,IDC_ADDRESS,tszPhone,SIZEOF(tszPhone));
			}
			mir_sntprintf(tszMessage,SIZEOF(tszMessage),TranslateT("The SMS message send to %s timed out."),tszPhone);
			KillTimer(hWndDlg,wParam);
			ShowWindow(hWndDlg,SW_SHOWNORMAL);
			EnableWindow(hWndDlg,FALSE);
			hwndTimeOut=CreateDialog(ssSMSSettings.hInstance,MAKEINTRESOURCE(IDD_SENDSMSTIMEDOUT),hWndDlg,SMSTimedOutDlgProc);
			SetDlgItemText(hwndTimeOut,IDC_STATUS,tszMessage);
		}
		break;
	case WM_CTLCOLOREDIT:
		if ((HWND)lParam==GetDlgItem(hWndDlg,IDC_MESSAGE))
		{	
			COLORREF colour;

			LoadMsgDlgFont(MSGFONTID_MESSAGEAREA,NULL,&colour);
			SetTextColor((HDC)wParam,colour);
			SetBkColor((HDC)wParam,db_get_dw(NULL,SRMMMOD,SRMSGSET_INPBKGCOLOUR,SRMSGDEFSET_BKGCOLOUR));
			return((BOOL)psswdWindowData->hBkgBrush);
		}
		break;
	case DM_TIMEOUTDECIDED:
		EnableWindow(hWndDlg,TRUE);
		switch(wParam){
		case TIMEDOUT_CANCEL:
			if (psswdWindowData->bMultiple)
			{
				if (SendSMSWindowNextHItemGet(hWndDlg,psswdWindowData->hItemSend))
				{
					psswdWindowData->hItemSend=SendSMSWindowNextHItemGet(hWndDlg,psswdWindowData->hItemSend);
					SendSMSWindowNext(hWndDlg);
				}else{
					SendSMSWindowRemove(hWndDlg);
				}
			}else{	
				EnableWindow(GetDlgItem(hWndDlg,IDOK),TRUE);
				SendDlgItemMessage(hWndDlg,IDC_MESSAGE,EM_SETREADONLY,FALSE,0);
				EnableWindow(GetDlgItem(hWndDlg,IDC_ACCOUNTS),TRUE);
				EnableWindow(GetDlgItem(hWndDlg,IDC_ADDRESS),TRUE);
				EnableWindow(GetDlgItem(hWndDlg,IDC_SAVENUMBER),TRUE);
				EnableWindow(GetDlgItem(hWndDlg,IDC_MULTIPLE),TRUE);
				EnableWindow(GetDlgItem(hWndDlg,IDC_NUMBERSLIST),TRUE);
				if (psswdWindowData->hMyContact==NULL) EnableWindow(GetDlgItem(hWndDlg,IDC_NAME),TRUE);
				SetFocus(GetDlgItem(hWndDlg,IDC_MESSAGE));
				psswdWindowData->hItemSend=NULL;
			}
			break;
		case TIMEDOUT_RETRY:
			{
				TCHAR tszPhone[MAX_PHONE_LEN];
				size_t dwPhoneSize;

				size_t dwMessageSize=GET_DLG_ITEM_TEXT_LENGTH(hWndDlg,IDC_MESSAGE);
				LPTSTR lpwszMessage=(LPTSTR)MEMALLOC(((dwMessageSize+4)*sizeof(TCHAR)));
				if (lpwszMessage)
				{
					if (psswdWindowData->bMultiple)
					{
						TVITEM tvi;
						tvi.mask=TVIF_TEXT;
						tvi.hItem=psswdWindowData->hItemSend;
						tvi.pszText=tszPhone;
						tvi.cchTextMax=SIZEOF(tszPhone);
						TreeView_GetItem(GetDlgItem(hWndDlg,IDC_NUMBERSLIST),&tvi);
						dwPhoneSize=mir_tstrlen(tszPhone);
					}else{
						dwPhoneSize=GetDlgItemText(hWndDlg,IDC_ADDRESS,tszPhone,SIZEOF(tszPhone));
					}
					dwMessageSize=GetDlgItemText(hWndDlg,IDC_MESSAGE,lpwszMessage,(dwMessageSize+2));
					SendSMSWindowNumberSet(hWndDlg,tszPhone,dwPhoneSize);
					StartSmsSend(hWndDlg,SendDlgItemMessage(hWndDlg,IDC_ACCOUNTS,CB_GETCURSEL,0,0),tszPhone,dwPhoneSize,lpwszMessage,dwMessageSize);
					MEMFREE(lpwszMessage);
				}
			}
			SetTimer(hWndDlg,TIMERID_MSGSEND,TIMEOUT_MSGSEND,NULL);
			break;
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDC_MULTIPLE:
			SendSMSWindowMultipleSet(hWndDlg,!psswdWindowData->bMultiple);
			break;

		case IDC_ADDNUMBER:
			{	
				TCHAR tszPhone[MAX_PHONE_LEN];
				if (IsPhoneW(tszPhone,GetDlgItemText(hWndDlg,IDC_ADDRESS,tszPhone,SIZEOF(tszPhone)))) {
					TVINSERTSTRUCT tvis = {0};
					tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
					tvis.hInsertAfter = TVI_SORT;
					tvis.item.pszText = tszPhone;
					TreeView_InsertItem( GetDlgItem(hWndDlg,IDC_NUMBERSLIST), &tvis);
				}
				else MessageBox(hWndDlg,TranslateT("The phone number should start with a + and consist of numbers, spaces, brackets and hyphens only."),TranslateT("Invalid Phone Number"),MB_OK);
			}
			break;

		case IDC_HISTORY:
			CallService(MS_HISTORY_SHOWCONTACTHISTORY,(WPARAM)psswdWindowData->hMyContact,0);
			break;

		case IDOK:
			if ((size_t)GET_DLG_ITEM_TEXT_LENGTH(hWndDlg,IDC_MESSAGE) > GetSMSMessageLenMax(hWndDlg))
			{
				MessageBox(hWndDlg,TranslateT("Message is too long, press OK to continue."),TranslateT("Error - Message too long"),MB_OK);
			}else{
				if (psswdWindowData->bMultiple)
				{
					HTREEITEM hItem=SendSMSWindowNextHItemGet(hWndDlg,TreeView_GetRoot(GetDlgItem(hWndDlg,IDC_NUMBERSLIST)));
					if (hItem)
					{
						psswdWindowData->hItemSend=hItem;
						EnableWindow(GetDlgItem(hWndDlg,IDOK),FALSE);
						EnableWindow(GetDlgItem(hWndDlg,IDC_NUMBERSLIST),FALSE);
						EnableWindow(GetDlgItem(hWndDlg,IDC_SAVENUMBER),FALSE);
						EnableWindow(GetDlgItem(hWndDlg,IDC_NAME),FALSE);
						EnableWindow(GetDlgItem(hWndDlg,IDC_MULTIPLE),FALSE);
						EnableWindow(GetDlgItem(hWndDlg,IDC_ADDNUMBER),FALSE);
						//EnableWindow(GetDlgItem(hWndDlg,IDC_REMOVENUMBER),FALSE);
						SendDlgItemMessage(hWndDlg,IDC_MESSAGE,EM_SETREADONLY,TRUE,0);
						EnableWindow(GetDlgItem(hWndDlg,IDC_ACCOUNTS),FALSE);
						EnableWindow(GetDlgItem(hWndDlg,IDC_ADDRESS),FALSE);
						SendSMSWindowNext(hWndDlg);
					}else{
						MessageBox(hWndDlg,TranslateT("There must be numbers in the list first."),TranslateT("No Numbers"),MB_OK);
					}
				}else{
					TCHAR tszPhone[MAX_PHONE_LEN];
					size_t dwPhoneSize=GetDlgItemText(hWndDlg,IDC_ADDRESS,tszPhone,SIZEOF(tszPhone));
					if (IsPhoneW(tszPhone,dwPhoneSize))
					{
						size_t dwMessageSize=GET_DLG_ITEM_TEXT_LENGTH(hWndDlg,IDC_MESSAGE);
						LPTSTR lpwszMessage=(LPTSTR)MEMALLOC((dwMessageSize+4)*sizeof(WCHAR));
						if (lpwszMessage)
						{
							dwMessageSize=GetDlgItemText(hWndDlg,IDC_MESSAGE,lpwszMessage,(dwMessageSize+2));
							SendSMSWindowNumberSet(hWndDlg,tszPhone,dwPhoneSize);
							EnableWindow(GetDlgItem(hWndDlg,IDOK),FALSE);
							EnableWindow(GetDlgItem(hWndDlg,IDC_NUMBERSLIST),FALSE);
							EnableWindow(GetDlgItem(hWndDlg,IDC_SAVENUMBER),FALSE);
							EnableWindow(GetDlgItem(hWndDlg,IDC_NAME),FALSE);
							EnableWindow(GetDlgItem(hWndDlg,IDC_MULTIPLE),FALSE);
							EnableWindow(GetDlgItem(hWndDlg,IDC_ACCOUNTS),FALSE);
							EnableWindow(GetDlgItem(hWndDlg,IDC_ADDRESS),FALSE);
							SendDlgItemMessage(hWndDlg,IDC_MESSAGE,EM_SETREADONLY,TRUE,0);
							StartSmsSend(hWndDlg,SendDlgItemMessage(hWndDlg,IDC_ACCOUNTS,CB_GETCURSEL,0,0),tszPhone,dwPhoneSize,lpwszMessage,dwMessageSize);
							SetTimer(hWndDlg,TIMERID_MSGSEND,TIMEOUT_MSGSEND,NULL);
							
							MEMFREE(lpwszMessage);
						}
					}else{
						MessageBox(hWndDlg,TranslateT("Valid phone numbers are of the form '+(country code)(phone number)'. The contents of the phone number portion is dependent on the national layout of phone numbers, but often omits the leading zero."),TranslateT("Invalid phone number"),MB_OK);
						SetFocus(GetDlgItem(hWndDlg,IDC_ADDRESS));
						SendDlgItemMessage(hWndDlg,IDC_ADDRESS,CB_SETEDITSEL,0,MAKELPARAM(0,-1));
					}
				}
			}
			break;
		case IDCANCEL:
			DeleteObject((HFONT)SendDlgItemMessage(hWndDlg,IDC_MESSAGE,WM_GETFONT,0,0));
			KillTimer(GetParent(hWndDlg),TIMERID_MSGSEND);
			SendSMSWindowRemove(hWndDlg);
			break;
		case IDC_MESSAGE:
			if (HIWORD(wParam)==EN_CHANGE)
			{
				TCHAR tszBuff[MAX_PATH];
				size_t dwMessageSize=GET_DLG_ITEM_TEXT_LENGTH(hWndDlg,IDC_MESSAGE);

				EnableWindow(GetDlgItem(hWndDlg,IDOK),dwMessageSize);
				mir_sntprintf(tszBuff, SIZEOF(tszBuff), _T("%d/%d"), dwMessageSize,GetSMSMessageLenMax(hWndDlg));
				SetDlgItemText(hWndDlg,IDC_COUNT,tszBuff);
			}
			break;
		case IDC_SAVENUMBER:
			{
				BOOL bCont=TRUE;
				char szBuff[MAX_PATH];
				TCHAR tszPhone[MAX_PHONE_LEN];
				DBVARIANT dbv;

				size_t dwPhoneSize=GetDlgItemText(hWndDlg,IDC_ADDRESS,tszPhone,(SIZEOF(tszPhone)-4));
				if (IsPhoneW(tszPhone,dwPhoneSize))
				{
					if (IsContactPhone(psswdWindowData->hMyContact,tszPhone,dwPhoneSize)==FALSE)
					{
						mir_tstrcat(tszPhone, _T(" SMS"));
						for(DWORD i=0;bCont;i++)
						{
							mir_snprintf(szBuff,SIZEOF(szBuff),"MyPhone%d",i);
							if (db_get(psswdWindowData->hMyContact,"UserInfo",szBuff,&dbv)) bCont=FALSE;
							db_free(&dbv);
						}
						DB_SetStringW(psswdWindowData->hMyContact,"UserInfo",szBuff,tszPhone);
					}
				}else{
					MessageBox(hWndDlg, TranslateT("The phone number should start with a + and consist of numbers, spaces, brackets and hyphens only."),TranslateT("Invalid Phone Number"),MB_OK);
				}
			}
			break;
		case IDC_NAME:
			if (HIWORD(wParam)==CBN_SELCHANGE) 
			{
				SendDlgItemMessage(hWndDlg,IDC_ADDRESS,CB_RESETCONTENT,0,0);
				if (SendDlgItemMessage(hWndDlg,IDC_NAME,CB_GETCURSEL,0,0))
				{
					MCONTACT hContact = SendSMSWindowSMSContactGet(hWndDlg,(SendDlgItemMessage(hWndDlg,IDC_NAME,CB_GETCURSEL,0,0)-1));
					if (hContact) AddContactPhonesToCombo(hWndDlg,hContact);
				}
			}
			break;
		}
		break;
	case WM_NOTIFY:
		switch(((NMHDR*)lParam)->idFrom){
		case IDC_NUMBERSLIST:
			switch(((NMHDR*)lParam)->code){
			case NM_CLICK:
				{	
					int iImage,iSame=1;
					HWND hWndTree=GetDlgItem(hWndDlg,IDC_NUMBERSLIST);
					TVITEM tvi={0};
					HTREEITEM hParent;
					TVHITTESTINFO hti={0};

					hti.pt.x=(short)LOWORD(GetMessagePos());
					hti.pt.y=(short)HIWORD(GetMessagePos());
					ScreenToClient(((LPNMHDR)lParam)->hwndFrom,&hti.pt);

					if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom,&hti))
					if (hti.flags&TVHT_ONITEMICON) 
					{
						tvi.mask=(TVIF_IMAGE|TVIF_SELECTEDIMAGE);
						tvi.hItem=hti.hItem;
						TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom,&tvi);
						iImage=tvi.iImage=tvi.iSelectedImage=!tvi.iImage;
						TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom,&tvi);

						if (TreeView_GetParent(hWndTree,hti.hItem))
						{
							hParent=TreeView_GetParent(hWndTree,hti.hItem);
							for(tvi.hItem=TreeView_GetChild(hWndTree,hParent);tvi.hItem;tvi.hItem=TreeView_GetNextSibling(hWndTree,tvi.hItem))
							{
								TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom,&tvi);
								if (tvi.iImage!=iImage)
								{
									iSame=0;
									break;
								}								
							}
							tvi.hItem=hParent;
							tvi.iImage=tvi.iSelectedImage=((iSame==1)? iImage:2);
							TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom,&tvi);
						}else{
							for(tvi.hItem=TreeView_GetChild(hWndTree,hti.hItem);tvi.hItem;tvi.hItem=TreeView_GetNextSibling(hWndTree,tvi.hItem))
							{
								TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom,&tvi);
								tvi.iImage=tvi.iSelectedImage=iImage;
								TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom,&tvi);
							}
						}
					}
				}
				break;
			}
		}
		break;			
	case WM_CLOSE:
		DeleteObject((HFONT)SendDlgItemMessage(hWndDlg,IDC_MESSAGE,WM_GETFONT,0,0));
		DeleteObject(psswdWindowData->hBkgBrush);
		////////RemWinHandle(GetDlgItem(hWndDlg,IDC_MESSAGE));
		KillTimer(GetParent(hWndDlg),TIMERID_MSGSEND);
		SendSMSWindowRemove(hWndDlg);
		break;
	}
return FALSE;
}

INT_PTR CALLBACK SMSTimedOutDlgProc(HWND hWndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg){
	case WM_INITDIALOG:
		{
			RECT rc,rcParent;
			TranslateDialogDefault(hWndDlg); 
			GetWindowRect(hWndDlg,&rc);
			GetWindowRect(GetParent(hWndDlg),&rcParent);
			SetWindowPos(hWndDlg,0,(rcParent.left+rcParent.right-(rc.right-rc.left))/2,(rcParent.top+rcParent.bottom-(rc.bottom-rc.top))/2,0,0,SWP_NOZORDER|SWP_NOSIZE);
			KillTimer(GetParent(hWndDlg),TIMERID_MSGSEND);
		}
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDOK:
			SendMessage(GetParent(hWndDlg),DM_TIMEOUTDECIDED,TIMEDOUT_RETRY,0);
			DestroyWindow(hWndDlg);
			break;
		case IDCANCEL:
			SendMessage(GetParent(hWndDlg),DM_TIMEOUTDECIDED,TIMEDOUT_CANCEL,0);
			DestroyWindow(hWndDlg);
			break;
		}
		break;
	}
	return FALSE;
} 

INT_PTR CALLBACK SMSAcceptedDlgProc(HWND hWndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg){
	case WM_INITDIALOG:
		TranslateDialogDefault(hWndDlg); 
		{
			RECT rc,rcParent;
			GetWindowRect(hWndDlg,&rc);
			GetWindowRect(GetParent(hWndDlg),&rcParent);
			SetWindowPos(hWndDlg,0,(rcParent.left+rcParent.right-(rc.right-rc.left))/2,(rcParent.top+rcParent.bottom-(rc.bottom-rc.top))/2,0,0,SWP_NOZORDER|SWP_NOSIZE);
		}
		return TRUE;
	
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK:
			SendSMSWindowRemove(GetParent(hWndDlg));
			DestroyWindow(hWndDlg);
			break;
		}
		break;
	}
	return FALSE;
}

//SMS Send window list functions

//This function create a new SMS send window, and insert it to the list.
//The function gets void and return the window HWND
HWND SendSMSWindowAdd(MCONTACT hContact)
{
	HWND hRet = NULL;
	SEND_SMS_WINDOW_DATA *psswdWindowData=(SEND_SMS_WINDOW_DATA*)MEMALLOC(sizeof(SEND_SMS_WINDOW_DATA));
	if ( !psswdWindowData)
		return NULL;
	psswdWindowData->hMyContact = hContact;
	psswdWindowData->hWnd = CreateDialogParam(ssSMSSettings.hInstance, MAKEINTRESOURCE(IDD_SENDSMS), NULL, SendSmsDlgProc, (LPARAM)psswdWindowData);
	if (psswdWindowData->hWnd) {
		ListMTLock(&ssSMSSettings.lmtSendSMSWindowsListMT);
		ListMTItemAdd(&ssSMSSettings.lmtSendSMSWindowsListMT,&psswdWindowData->lmtListMTItem,psswdWindowData);
		ListMTUnLock(&ssSMSSettings.lmtSendSMSWindowsListMT);

		LPTSTR lptszContactDisplayName=GetContactNameW(hContact);
		TCHAR tszTitle[MAX_PATH];
		mir_sntprintf(tszTitle, SIZEOF(tszTitle), _T("%s - %s"), lptszContactDisplayName, TranslateT("Send SMS"));
		SetWindowText(psswdWindowData->hWnd,tszTitle);
		SendDlgItemMessage(psswdWindowData->hWnd,IDC_NAME,CB_ADDSTRING,0,(LPARAM)lptszContactDisplayName);
		SendDlgItemMessage(psswdWindowData->hWnd,IDC_NAME,CB_SETCURSEL,0,0);
		AddContactPhonesToCombo(psswdWindowData->hWnd,hContact);
		SetFocus(GetDlgItem(psswdWindowData->hWnd,IDC_MESSAGE));
		hRet=psswdWindowData->hWnd;
	}
	else MEMFREE(psswdWindowData);
	return hRet;
}

//This function close the SMS send window that given, and remove it from the list.
//The function gets the HWND of the window that should be removed and return void
void SendSMSWindowRemove(HWND hWndDlg)
{
	SEND_SMS_WINDOW_DATA *psswdWindowData = GET_WINDOW_DATA(hWndDlg);

	if (psswdWindowData)
	{
		DB_SMS_SetDword(NULL,"LastProto",SendDlgItemMessage(hWndDlg,IDC_ACCOUNTS,CB_GETCURSEL,0,0));
		SendSMSWindowMultipleSet(hWndDlg,FALSE);
		Utils_SaveWindowPosition(hWndDlg,(DB_SMS_GetByte(NULL,"SavePerContact",0)? psswdWindowData->hMyContact:NULL),PROTOCOL_NAMEA,"send");
	
		ListMTLock(&ssSMSSettings.lmtSendSMSWindowsListMT);
		ListMTItemDelete(&ssSMSSettings.lmtSendSMSWindowsListMT,&psswdWindowData->lmtListMTItem);
		ListMTUnLock(&ssSMSSettings.lmtSendSMSWindowsListMT);
		SendSMSWindowSMSContactsRemove(hWndDlg);
		MEMFREE(psswdWindowData->pdbei);
		MEMFREE(psswdWindowData);
	}
	DestroyWindow(hWndDlg);
}

//This function return the contact HANDLE for the given to the SMS send window.
//The function gets the HWND of the window and return the HANDLE of the contact.
MCONTACT SendSMSWindowHContactGet(HWND hWndDlg)
{
	MCONTACT hRet=NULL;
	SEND_SMS_WINDOW_DATA *psswdWindowData=GET_WINDOW_DATA(hWndDlg);

	if (psswdWindowData) hRet=psswdWindowData->hMyContact;
	return(hRet);
}

//This function set the contact info of the person we send him the in the given to the SMS send window.
//The function gets the HWND of the window and the HANDLE of the contact and return void
void SendSMSWindowHContactSet(HWND hWndDlg,MCONTACT hContact)
{
	SEND_SMS_WINDOW_DATA *psswdWindowData=GET_WINDOW_DATA(hWndDlg);
	if (psswdWindowData) psswdWindowData->hMyContact=hContact;
}

//This function return the HWND of a SMS send window that have the same process as given.
//The function gets the HANDLE of a process and return the HWND of the SMS send window that has
//the same process
HWND SendSMSWindowHwndByHProcessGet(HANDLE hProcess)
{
	HWND hRet=NULL;
	SEND_SMS_WINDOW_DATA *psswdWindowData;
	LIST_MT_ITERATOR lmtiIterator;

	ListMTLock(&ssSMSSettings.lmtSendSMSWindowsListMT);
	ListMTIteratorMoveFirst(&ssSMSSettings.lmtSendSMSWindowsListMT,&lmtiIterator);
	do
	{// цикл
		if (ListMTIteratorGet(&lmtiIterator,NULL,(LPVOID*)&psswdWindowData)==NO_ERROR)
		if (psswdWindowData->hProcess==hProcess)
		{
			hRet=psswdWindowData->hWnd;
			break;
		}
	}while (ListMTIteratorMoveNext(&lmtiIterator));
	ListMTUnLock(&ssSMSSettings.lmtSendSMSWindowsListMT);
return(hRet);		
}

//This function set the process info of the send procedure we sent with the given SMS send window.
//The function gets the HWND of the window and the HANDLE of the process and return void
void SendSMSWindowHProcessSet(HWND hWndDlg,HANDLE hProcess)
{
	SEND_SMS_WINDOW_DATA *psswdWindowData=GET_WINDOW_DATA(hWndDlg);
	if (psswdWindowData) psswdWindowData->hProcess=hProcess;
}

//
BOOL SendSMSWindowMultipleGet(HWND hWndDlg)
{
	BOOL bRet=FALSE;
	SEND_SMS_WINDOW_DATA *psswdWindowData=GET_WINDOW_DATA(hWndDlg);

	if (psswdWindowData) bRet=psswdWindowData->bMultiple;
return(bRet);
}

//
void SendSMSWindowMultipleSet(HWND hWndDlg,BOOL bMultiple)
{
	SEND_SMS_WINDOW_DATA *psswdWindowData=GET_WINDOW_DATA(hWndDlg);

	if ( !psswdWindowData)
		return;
	if (psswdWindowData->bMultiple == bMultiple)
		return;

	RECT rcWin,rcList;
	WINDOWPLACEMENT wp;

	psswdWindowData->bMultiple=bMultiple;
	wp.length=sizeof(WINDOWPLACEMENT);
	GetWindowRect(hWndDlg,&rcWin);
	GetWindowRect(GetDlgItem(hWndDlg,IDC_NUMBERSLIST),&rcList);
	GetWindowPlacement(hWndDlg,&wp);

	if (bMultiple)
	{
		SendSMSWindowFillTreeView(hWndDlg);

		EnableWindow(GetDlgItem(hWndDlg,IDC_SAVENUMBER),FALSE);
		SetDlgItemText(hWndDlg, IDC_MULTIPLE, TranslateT("<< Single"));
		if (wp.showCmd==SW_MAXIMIZE) SetWindowPos(hWndDlg, 0, 0, 0, rcWin.right - rcWin.left - (rcList.right - rcList.left + 11), rcWin.bottom - rcWin.top, SWP_NOZORDER | SWP_NOMOVE);
		SetWindowPos(hWndDlg, 0 ,rcWin.left, rcWin.top, rcWin.right - rcWin.left + (rcList.right-rcList.left) + 11, rcWin.bottom - rcWin.top, SWP_NOZORDER | SWP_NOMOVE);
	}else{
		if (psswdWindowData->hMyContact) AddContactPhonesToCombo(hWndDlg,psswdWindowData->hMyContact);

		EnableWindow(GetDlgItem(hWndDlg, IDC_SAVENUMBER), TRUE);
		SetDlgItemText(hWndDlg, IDC_MULTIPLE, TranslateT("Multiple >>"));
		SetWindowPos(hWndDlg, 0, rcWin.left, rcWin.top, rcWin.right-rcWin.left - (rcList.right-rcList.left) - 11, rcWin.bottom - rcWin.top,SWP_NOZORDER | SWP_NOMOVE);
		if (wp.showCmd==SW_MAXIMIZE) SetWindowPos(hWndDlg, 0, 0, 0, rcWin.right - rcWin.left + (rcList.right - rcList.left + 11), rcWin.bottom - rcWin.top, SWP_NOZORDER | SWP_NOMOVE);
	}
}

//
void SendSMSWindowNumberSet(HWND hWndDlg, LPWSTR lpwszPhone, size_t dwPhoneSize)
{
	SEND_SMS_WINDOW_DATA *psswdWindowData=GET_WINDOW_DATA(hWndDlg);

	if (psswdWindowData)
	if (psswdWindowData->bMultiple)
	{
		psswdWindowData->hContact=psswdWindowData->hMyContact;
	}else{
		psswdWindowData->hContact=HContactFromPhone(lpwszPhone,dwPhoneSize);
	}
}

//
void SendSMSWindowAsSentSet(HWND hWndDlg)
{
//	LVITEM lvi;
//	char szPhone[MAX_PHONE_LEN];
//	lvi.mask=LVIF_TEXT|LVIF_IMAGE;
//	lvi.iItem=GetSendSMSWindowSMSSend(hWndDlg) - 1;
//	lvi.iSubItem=0;
//	ListView_GetItemText(GetDlgItem(hWndDlg,IDC_NUMBERSLIST),GetSendSMSWindowSMSSend(hWndDlg) - 1,0,szPhone,SIZEOF(szPhone));
//	lvi.pszText=szPhone;
//	lvi.iImage=2;
//	ListView_SetItem(GetDlgItem(hWndDlg,IDC_NUMBERSLIST),&lvi);
//	ListView_RedrawItems(GetDlgItem(hWndDlg,IDC_NUMBERSLIST),0,ListView_GetItemCount(GetDlgItem(hWndDlg,IDC_NUMBERSLIST)) - 1);							
}

//This function set the databsae info of the sent message we sent with the SMS send window.
//The function gets the HWND of the window and the DBEI of the database information of the message
//and return void
void SendSMSWindowDbeiSet(HWND hWndDlg,DBEVENTINFO *pdbei)
{
	SEND_SMS_WINDOW_DATA *psswdWindowData=GET_WINDOW_DATA(hWndDlg);
	if (psswdWindowData) psswdWindowData->pdbei=pdbei;
}

//This function get a HWND of SMS send window and add its database information into Miranda-IM database
//The function gets the HWND of the window and return void
//and return void
void SendSMSWindowDBAdd(HWND hWndDlg)
{
	SEND_SMS_WINDOW_DATA *psswdWindowData=GET_WINDOW_DATA(hWndDlg);
	if (psswdWindowData) {
		psswdWindowData->pdbei->cbSize = sizeof(DBEVENTINFO);
		psswdWindowData->pdbei->szModule = GetModuleName(psswdWindowData->hContact);
		db_event_add(psswdWindowData->hContact, psswdWindowData->pdbei);
		MEMFREE(psswdWindowData->pdbei);
	}
}

//
void SendSMSWindowHItemSendSet(HWND hWndDlg,HTREEITEM hItemSend)
{
	SEND_SMS_WINDOW_DATA *psswdWindowData=GET_WINDOW_DATA(hWndDlg);
	if (psswdWindowData)
		psswdWindowData->hItemSend=hItemSend;
}

//
HTREEITEM SendSMSWindowHItemSendGet(HWND hWndDlg)
{
	HTREEITEM hRet=NULL;
	SEND_SMS_WINDOW_DATA *psswdWindowData=GET_WINDOW_DATA(hWndDlg);

	if (psswdWindowData) hRet=psswdWindowData->hItemSend;
	return hRet;
}

//
HTREEITEM SendSMSWindowNextHItemGet(HWND hWndDlg,HTREEITEM hItem)
{
	int isFound=0;
	HWND hWndTree=GetDlgItem(hWndDlg,IDC_NUMBERSLIST);
	TVITEM tvi;
	HTREEITEM hItemNext,hItemChild;

	if (hItem==TreeView_GetRoot(hWndTree)) isFound=1;

	for(hItemNext=TreeView_GetRoot(hWndTree);hItemNext;hItemNext=TreeView_GetNextSibling(hWndTree,hItemNext))
	{
		if ((hItem==hItemNext) && (hItem!=TreeView_GetRoot(hWndTree)))
		{
			isFound=1;
			continue;
		}

		tvi.mask=(TVIF_IMAGE|TVIF_SELECTEDIMAGE);
		tvi.hItem=hItemNext;
		tvi.iImage=0;
		tvi.iSelectedImage=0;
		TreeView_GetItem(hWndTree,&tvi);

		if (tvi.iImage || tvi.iSelectedImage)
		if (TreeView_GetChild(hWndTree,hItemNext))
		{
			for (hItemChild=TreeView_GetChild(hWndTree,hItemNext);hItemChild;hItemChild=TreeView_GetNextSibling(hWndTree,hItemChild))
			{
				if (hItem==hItemChild)
				{
					isFound=1;
					continue;
				}

				tvi.mask=(TVIF_IMAGE|TVIF_SELECTEDIMAGE);
				tvi.hItem=hItemChild;
				tvi.iImage=0;
				tvi.iSelectedImage=0;
				TreeView_GetItem(hWndTree,&tvi);

				if (tvi.iImage || tvi.iSelectedImage)
				if (isFound)
				{
					return(hItemChild);				
				}else{
					continue;
				}
			}
		}else{
			if (isFound)
			{
				return(hItemNext);				
			}else{
				continue;
			}
		}
	}
return(NULL);
}

//This function get the HANDLE of an user. if there is already a SMS send window for this contact
//it return its HWND else the function return NULL.
//The function gets the HANDLE of a contact and return HWND
HWND SendSMSWindowIsOtherInstanceHContact(MCONTACT hContact)
{
	HWND hRet=NULL;
	SEND_SMS_WINDOW_DATA *psswdWindowData;
	LIST_MT_ITERATOR lmtiIterator;

	ListMTLock(&ssSMSSettings.lmtSendSMSWindowsListMT);
	ListMTIteratorMoveFirst(&ssSMSSettings.lmtSendSMSWindowsListMT,&lmtiIterator);
	do
	{// цикл
		if (ListMTIteratorGet(&lmtiIterator,NULL,(LPVOID*)&psswdWindowData)==NO_ERROR)
		if (psswdWindowData->hMyContact==hContact)
		{
			hRet=psswdWindowData->hWnd;
			break;
		}
	}while (ListMTIteratorMoveNext(&lmtiIterator));
	ListMTUnLock(&ssSMSSettings.lmtSendSMSWindowsListMT);
return(hRet);
}


//
void SendSMSWindowNext(HWND hWndDlg)
{
	TCHAR tszPhone[MAX_PHONE_LEN];
	size_t dwPhoneSize,dwMessageSize;
	TVITEM tvi={0};

	dwMessageSize=GET_DLG_ITEM_TEXT_LENGTH(hWndDlg,IDC_MESSAGE);
	LPTSTR lptszMessage=(LPTSTR)MEMALLOC(((dwMessageSize+4)*sizeof(TCHAR)));
	if ( !lptszMessage)
		return;

	dwMessageSize=GetDlgItemText(hWndDlg,IDC_MESSAGE,lptszMessage,dwMessageSize+2);

//	if (SendSMSWindowNextHItemGet(hWndDlg,SendSMSWindowHItemSendGet(hWndDlg))==NULL) SendSMSWindowMultipleSet(hWndDlg,FALSE);	
	tvi.mask=TVIF_TEXT;
	tvi.hItem=SendSMSWindowHItemSendGet(hWndDlg);
	tvi.pszText=tszPhone;
	tvi.cchTextMax=SIZEOF(tszPhone);
	TreeView_GetItem(GetDlgItem(hWndDlg,IDC_NUMBERSLIST),&tvi);
	TreeView_SelectItem(GetDlgItem(hWndDlg,IDC_NUMBERSLIST),tvi.hItem);
	dwPhoneSize=mir_wstrlen(tszPhone);
	SendSMSWindowNumberSet(hWndDlg,tszPhone,dwPhoneSize);
	StartSmsSend(hWndDlg,SendDlgItemMessage(hWndDlg,IDC_ACCOUNTS,CB_GETCURSEL,0,0),tszPhone,dwPhoneSize,lptszMessage,dwMessageSize);
	SetTimer(hWndDlg,TIMERID_MSGSEND,TIMEOUT_MSGSEND,NULL);
	MEMFREE(lptszMessage);
}


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//This function gets a HANDLE of a contact and add it to a list.
void SendSMSWindowSMSContactAdd(HWND hWndDlg,MCONTACT hContact)
{
	SEND_SMS_WINDOW_DATA *psswdWindowData=GET_WINDOW_DATA(hWndDlg);

	if ( !psswdWindowData)
		return;

	psswdWindowData->dwContactsListCount++;
	if (psswdWindowData->phContactsList)
		psswdWindowData->phContactsList = (MCONTACT*)MEMREALLOC(psswdWindowData->phContactsList,(sizeof(HANDLE)*psswdWindowData->dwContactsListCount));
	else
		psswdWindowData->phContactsList = (MCONTACT*)MEMALLOC((sizeof(HANDLE)*psswdWindowData->dwContactsListCount));

	*(psswdWindowData->phContactsList+psswdWindowData->dwContactsListCount-1)=hContact;
}

//This function gets the number of the given contact in the combo list and return its contact.
MCONTACT SendSMSWindowSMSContactGet(HWND hWndDlg,size_t iNum)
{
	SEND_SMS_WINDOW_DATA *psswdWindowData = GET_WINDOW_DATA(hWndDlg);

	if (psswdWindowData)
		return (*((psswdWindowData->phContactsList) + iNum));

	return NULL;
}

void SendSMSWindowSMSContactsRemove(HWND hWndDlg)
{
	SEND_SMS_WINDOW_DATA *psswdWindowData = GET_WINDOW_DATA(hWndDlg);

	if (psswdWindowData)
	{
		MEMFREE(psswdWindowData->phContactsList);
		psswdWindowData->dwContactsListCount=0;
	}
}

void SendSMSWindowUpdateAccountList(HWND hWndDlg)
{
	if (ssSMSSettings.ppaSMSAccounts && ssSMSSettings.dwSMSAccountsCount)
	{
		size_t i,dwCurSel;

		dwCurSel=SendDlgItemMessage(hWndDlg,IDC_ACCOUNTS,CB_GETCURSEL,0,0);
		if (dwCurSel==-1) dwCurSel=DB_SMS_GetDword(NULL,"LastProto",0);
		if (dwCurSel==-1) dwCurSel=0;

		SendDlgItemMessage(hWndDlg,IDC_ACCOUNTS,CB_RESETCONTENT,0,0);
		for(i=0;i<ssSMSSettings.dwSMSAccountsCount;i++)
		{
			SendDlgItemMessageA(hWndDlg,IDC_ACCOUNTS,CB_ADDSTRING,0,(LPARAM)ssSMSSettings.ppaSMSAccounts[i]->szModuleName);
		}
		SendDlgItemMessage(hWndDlg,IDC_ACCOUNTS,CB_SETCURSEL,(WPARAM)dwCurSel,0);
	}
}


void SendSMSWindowsUpdateAllAccountLists()
{
	SEND_SMS_WINDOW_DATA *psswdWindowData;
	LIST_MT_ITERATOR lmtiIterator;

	ListMTLock(&ssSMSSettings.lmtSendSMSWindowsListMT);
	ListMTIteratorMoveFirst(&ssSMSSettings.lmtSendSMSWindowsListMT,&lmtiIterator);
	do
	{// цикл
		if (ListMTIteratorGet(&lmtiIterator,NULL,(LPVOID*)&psswdWindowData)==NO_ERROR)
		{
			SendSMSWindowUpdateAccountList(psswdWindowData->hWnd);
		}
	}while (ListMTIteratorMoveNext(&lmtiIterator));
	ListMTUnLock(&ssSMSSettings.lmtSendSMSWindowsListMT);
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////




void AddContactPhonesToComboToListParam(MCONTACT hContact,LPSTR lpszModule,LPSTR lpszValueName,HWND hWndList)
{
	char szBuff[MAX_PATH];
	TCHAR tszPhone[MAX_PHONE_LEN],tszPhoneRaw[MAX_PHONE_LEN];
	size_t i,dwPhoneSize;

	if (DB_GetStaticStringW(hContact,lpszModule,lpszValueName,tszPhoneRaw,SIZEOF(tszPhoneRaw),&dwPhoneSize))
	{
		tszPhone[0]='+';
		dwPhoneSize=CopyNumberW(tszPhone+1,tszPhoneRaw,dwPhoneSize);
		if (IsPhoneW(tszPhone,dwPhoneSize))
		{
			if (SendMessage(hWndList,CB_FINDSTRING,-1,(LPARAM)tszPhone)==CB_ERR) SendMessage(hWndList,CB_ADDSTRING,0,(LPARAM)tszPhone);
		}
	}

	for (i=0;TRUE;i++)
	{
		mir_snprintf(szBuff,SIZEOF(szBuff),"%s%ld",lpszValueName,i);
		if (DB_GetStaticStringW(hContact,lpszModule,szBuff,tszPhoneRaw,SIZEOF(tszPhoneRaw),&dwPhoneSize))
		{
			tszPhone[0]='+';
			dwPhoneSize=CopyNumberW(tszPhone+1,tszPhoneRaw,dwPhoneSize);
			if (IsPhoneW(tszPhone,dwPhoneSize))
			{
				if (SendMessage(hWndList,CB_FINDSTRING,-1,(LPARAM)tszPhone)==CB_ERR) SendMessage(hWndList,CB_ADDSTRING,0,(LPARAM)tszPhone);
			}
		}else{
			if (i>PHONES_MIN_COUNT) break;
		}
	}
}


void AddContactPhonesToCombo(HWND hWnd,MCONTACT hContact)
{
	HWND hWndList = GetDlgItem(hWnd,IDC_ADDRESS);
	SendDlgItemMessage(hWnd,IDC_ADDRESS,CB_RESETCONTENT,0,0);

	LPSTR lpszProto = GetContactProto(hContact);
	if (lpszProto) {
		AddContactPhonesToComboToListParam(hContact,lpszProto,"Phone",hWndList);
		AddContactPhonesToComboToListParam(hContact,lpszProto,"Cellular",hWndList);
		AddContactPhonesToComboToListParam(hContact,lpszProto,"Fax",hWndList);
	}
	AddContactPhonesToComboToListParam(hContact,"UserInfo","MyPhone",hWndList);
	AddContactPhonesToComboToListParam(hContact,"UserInfo","Phone",hWndList);
	AddContactPhonesToComboToListParam(hContact,"UserInfo","Cellular",hWndList);
	AddContactPhonesToComboToListParam(hContact,"UserInfo","Fax",hWndList);

	SendDlgItemMessage(hWnd,IDC_ADDRESS,CB_SETCURSEL,0,0);
}



void AddContactPhonesToTreeViewParam(MCONTACT hContact,LPSTR lpszModule,LPSTR lpszValueName,HWND hWndList,HTREEITEM *phParent)
{
	char szBuff[MAX_PATH];
	TCHAR tszPhone[MAX_PHONE_LEN],tszPhoneRaw[MAX_PHONE_LEN];
	size_t i,dwPhoneSize;
	TVINSERTSTRUCT tvis={0};

	if (phParent) tvis.hParent=(*phParent);
	tvis.item.mask=(TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE);
	tvis.hInsertAfter=TVI_SORT;
	tvis.item.iImage=tvis.item.iSelectedImage=0;

	if (DB_GetStaticStringW(hContact,lpszModule,lpszValueName,tszPhoneRaw,SIZEOF(tszPhoneRaw),&dwPhoneSize))
	{
		tszPhone[0]='+';
		dwPhoneSize=CopyNumberW(tszPhone+1,tszPhoneRaw,dwPhoneSize);
		if (IsPhoneW(tszPhone,dwPhoneSize))
		{
			if (tvis.hParent==NULL)
			{
				tvis.item.pszText=GetContactNameW(hContact);
				tvis.hParent=TreeView_InsertItem(hWndList,&tvis);
			}
			tvis.item.pszText=tszPhone;
			TreeView_InsertItem(hWndList,&tvis);											
		}
	}

	for (i=0;TRUE;i++)
	{
		mir_snprintf(szBuff,SIZEOF(szBuff),"%s%ld",lpszValueName,i);
		if (DB_GetStaticStringW(hContact,lpszModule,szBuff,tszPhoneRaw,SIZEOF(tszPhoneRaw),&dwPhoneSize))
		{
			tszPhone[0]='+';
			dwPhoneSize=CopyNumberW(tszPhone+1,tszPhoneRaw,dwPhoneSize);
			if (IsPhoneW(tszPhone,dwPhoneSize))
			{
				if (tvis.hParent==NULL)
				{
					tvis.item.pszText=GetContactNameW(hContact);
					tvis.hParent=TreeView_InsertItem(hWndList,&tvis);
				}
				tvis.item.pszText=tszPhone;
				TreeView_InsertItem(hWndList,&tvis);											
			}
		}else{
			if (i>PHONES_MIN_COUNT) break;
		}
	}
	if (phParent) (*phParent)=tvis.hParent;
}


void SendSMSWindowFillTreeView(HWND hWnd)
{
	HWND hWndTreeView=GetDlgItem(hWnd,IDC_NUMBERSLIST);
	TreeView_DeleteAllItems(hWndTreeView);

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		HTREEITEM hParent = NULL;
		LPSTR lpszProto = GetContactProto(hContact);
		if (lpszProto) {
			AddContactPhonesToTreeViewParam(hContact,lpszProto,"Phone",hWndTreeView,&hParent);
			AddContactPhonesToTreeViewParam(hContact,lpszProto,"Cellular",hWndTreeView,&hParent);
			AddContactPhonesToTreeViewParam(hContact,lpszProto,"Fax",hWndTreeView,&hParent);
		}
		AddContactPhonesToTreeViewParam(hContact,"UserInfo","MyPhone",hWndTreeView,&hParent);
		AddContactPhonesToTreeViewParam(hContact,"UserInfo","Phone",hWndTreeView,&hParent);
		AddContactPhonesToTreeViewParam(hContact,"UserInfo","Cellular",hWndTreeView,&hParent);
		AddContactPhonesToTreeViewParam(hContact,"UserInfo","Fax",hWndTreeView,&hParent);
	}	
}


size_t GetSMSMessageLenMax(HWND hWndDlg)
{
	size_t dwMessageSize,dwLenght=160;
	
	dwMessageSize=GET_DLG_ITEM_TEXT_LENGTH(hWndDlg,IDC_MESSAGE);
	LPTSTR lptszMessage=(LPTSTR)MEMALLOC(((dwMessageSize+4)*sizeof(TCHAR)));
	if (lptszMessage)
	{
		dwMessageSize=GetDlgItemText(hWndDlg,IDC_MESSAGE,lptszMessage,(dwMessageSize+2));
		if (dwMessageSize!=WideCharToMultiByte(CP_UTF8,0,lptszMessage,dwMessageSize,NULL,0,NULL,NULL)) dwLenght=70;
		MEMFREE(lptszMessage);
	}
return(dwLenght);
}
