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

//Defnition needed to the SMS window list
typedef struct
{
	LIST_MT_ITEM lmtListMTItem;
	HWND hWnd;
	HBRUSH hBkgBrush;
	MCONTACT hContact;
} RECV_SMS_WINDOW_DATA;

INT_PTR CALLBACK	RecvSmsDlgProc	(HWND hWndDlg,UINT message,WPARAM wParam,LPARAM lParam);
#define GET_WINDOW_DATA(hWndDlg)	((RECV_SMS_WINDOW_DATA*)GetWindowLongPtr(hWndDlg,GWLP_USERDATA))
												 
DWORD RecvSMSWindowInitialize()
{
	DWORD dwRetErrorCode;

	dwRetErrorCode=ListMTInitialize(&ssSMSSettings.lmtRecvSMSWindowsListMT,0);
return(dwRetErrorCode);
}

//This function destroy all SMS receive windows
void RecvSMSWindowDestroy()
{
	RECV_SMS_WINDOW_DATA *prswdWindowData;

	ListMTLock(&ssSMSSettings.lmtRecvSMSWindowsListMT);
	while (ListMTItemGetFirst(&ssSMSSettings.lmtRecvSMSWindowsListMT,NULL,(LPVOID*)&prswdWindowData)==NO_ERROR)
	{// цикл
		RecvSMSWindowRemove(prswdWindowData->hWnd);
	}
	ListMTUnLock(&ssSMSSettings.lmtRecvSMSWindowsListMT);
	ListMTDestroy(&ssSMSSettings.lmtRecvSMSWindowsListMT);
}


INT_PTR CALLBACK RecvSmsDlgProc(HWND hWndDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	RECV_SMS_WINDOW_DATA *prswdWindowData=GET_WINDOW_DATA(hWndDlg);

	switch(message){
	case WM_INITDIALOG:
		TranslateDialogDefault(hWndDlg); //Translate intially - bid

		prswdWindowData=(RECV_SMS_WINDOW_DATA*)lParam;
		SetWindowLongPtr(hWndDlg,GWLP_USERDATA,(LONG_PTR)lParam);

		mir_subclassWindow(GetDlgItem(hWndDlg,IDC_MESSAGE), MessageSubclassProc);
		{
			LOGFONT lf;
			HFONT hFont=(HFONT)SendDlgItemMessage(hWndDlg,IDC_MESSAGE,WM_GETFONT,0,0);
			if (hFont && hFont!=(HFONT)SendDlgItemMessage(hWndDlg,IDOK,WM_GETFONT,0,0)) DeleteObject(hFont);
			LoadMsgDlgFont(MSGFONTID_YOURMSG,&lf,NULL);
			hFont=CreateFontIndirect(&lf);
			SendDlgItemMessage(hWndDlg,IDC_MESSAGE,WM_SETFONT,(WPARAM)hFont,MAKELPARAM(TRUE,0));

			COLORREF colour=db_get_dw(NULL,SRMMMOD,SRMSGSET_BKGCOLOUR,SRMSGDEFSET_BKGCOLOUR);
			prswdWindowData->hBkgBrush=CreateSolidBrush(colour);
		}

		if (Utils_RestoreWindowPosition(hWndDlg,(DB_SMS_GetByte(NULL,"SavePerContact",0)? prswdWindowData->hContact:NULL),PROTOCOL_NAMEA,"recv"))
		{// def pos
			SetWindowPos(hWndDlg,0,200,200,400,350,SWP_NOZORDER);
		}
		InvalidateRect(GetDlgItem(hWndDlg,IDC_MESSAGE),NULL,FALSE);
		break;

	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam==GetDlgItem(hWndDlg,IDC_MESSAGE)) {	
			COLORREF colour;

			LoadMsgDlgFont(MSGFONTID_YOURMSG,NULL,&colour);
			SetTextColor((HDC)wParam,colour);
			SetBkColor((HDC)wParam,db_get_dw(NULL,SRMMMOD,SRMSGSET_BKGCOLOUR,SRMSGDEFSET_BKGCOLOUR));
			return((BOOL)prswdWindowData->hBkgBrush);
		}
		break;

	case WM_GETMINMAXINFO:
		((LPMINMAXINFO)lParam)->ptMinTrackSize.x=300; 
		((LPMINMAXINFO)lParam)->ptMinTrackSize.y=230;
		break;

	case WM_SIZE:
		{
			int cx,cy;
			RECT rcWin;

			GetWindowRect(hWndDlg,&rcWin);
			cx = rcWin.right - rcWin.left;
			cy = rcWin.bottom - rcWin.top;
			cx -= 10; /* XXX: fix window sizes for aero. */
			cy -= 10;
			SetWindowPos(GetDlgItem(hWndDlg,IDC_MESSAGE),0,0,0,cx - 14,cy - 112,SWP_NOZORDER|SWP_NOMOVE);
			SetWindowPos(GetDlgItem(hWndDlg,IDC_NAME),0,0,0,(cx*35)/100,20,SWP_NOZORDER|SWP_NOMOVE);
			SetWindowPos(GetDlgItem(hWndDlg,IDC_NUMBER),0,cx - (cx*35)/100 - 11,5,(cx*35)/100,20,SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWndDlg,IDC_ST_NUMBER),0,cx - (cx*35)/100 - 58,5,40,20,SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWndDlg,IDC_READNEXT),0,cx - 87, cy - 60,80,25,SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWndDlg,IDOK),0,cx/2 - 87,cy - 60,80,25,SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWndDlg,IDCANCEL),0,cx/2 + 7,cy - 60,80,25,SWP_NOZORDER);

			RedrawWindow(hWndDlg,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDOK:
			{
				WCHAR wszPhone[MAX_PHONE_LEN];
				HWND hwndSendSms;
				MCONTACT hContact;

				hContact=HContactFromPhone(wszPhone,GetDlgItemText(hWndDlg,IDC_NUMBER,wszPhone,SIZEOF(wszPhone)));
				hwndSendSms=SendSMSWindowIsOtherInstanceHContact(hContact);
				if (hwndSendSms)
					SetFocus(hwndSendSms);
				else {
					hwndSendSms=SendSMSWindowAdd(prswdWindowData->hContact);
					SetDlgItemText(hwndSendSms,IDC_ADDRESS,wszPhone);
				}
			}
		case IDCANCEL:
			RecvSMSWindowRemove(hWndDlg);
			break;
		}
		break;

	case WM_CLOSE:
		DeleteObject(prswdWindowData->hBkgBrush);
		RecvSMSWindowRemove(hWndDlg);
		break;
	}

	return FALSE;
}

//SMS Receive window list functions

//This function create a new SMS receive window, and insert it to the list.
//The function gets void and return the window HWND
HWND RecvSMSWindowAdd(MCONTACT hContact,DWORD dwEventType,LPWSTR lpwszPhone,SIZE_T dwPhoneSize,LPSTR lpszMessage,SIZE_T dwMessageSize)
{
	HWND hRet = NULL;

	if ((dwPhoneSize + dwMessageSize) <= MIN_SMS_DBEVENT_LEN)
		return NULL;

	LPWSTR lpwszMessage;
	lpwszMessage = (LPWSTR)MEMALLOC(((dwMessageSize + MAX_PATH) * sizeof(WCHAR)));
	if ( !lpwszMessage)
		return NULL;

	RECV_SMS_WINDOW_DATA *prswdWindowData;

	prswdWindowData = (RECV_SMS_WINDOW_DATA*)MEMALLOC(sizeof(RECV_SMS_WINDOW_DATA));
	if (prswdWindowData)
	{
		prswdWindowData->hContact=hContact;
		prswdWindowData->hWnd=CreateDialogParam(ssSMSSettings.hInstance,MAKEINTRESOURCE(IDD_RECVSMS),NULL,RecvSmsDlgProc,(LPARAM)prswdWindowData);
		if (prswdWindowData->hWnd)
		{
			HICON hIcon;
			WCHAR wszTitle[MAX_PATH]={0},wszPhoneLocal[MAX_PHONE_LEN]={0};
			UINT iIcon;
			LPWSTR lpwszContactDisplayName,lpwszTitlepart;

			ListMTLock(&ssSMSSettings.lmtRecvSMSWindowsListMT);
			ListMTItemAdd(&ssSMSSettings.lmtRecvSMSWindowsListMT,&prswdWindowData->lmtListMTItem,prswdWindowData);
			ListMTUnLock(&ssSMSSettings.lmtRecvSMSWindowsListMT);

			switch(dwEventType){
			case ICQEVENTTYPE_SMS:
				lpwszTitlepart=TranslateT("Received SMS");
				hIcon=LoadSkinnedIcon(SKINICON_OTHER_SMS);
				break;
			case ICQEVENTTYPE_SMSCONFIRMATION:
				lpwszTitlepart=TranslateT("Received SMS Confirmation");
				GetDataFromMessage(lpszMessage,dwMessageSize,NULL,NULL,0,NULL,&iIcon);
				hIcon=(HICON)LoadImage(ssSMSSettings.hInstance,MAKEINTRESOURCE(iIcon),IMAGE_ICON,0,0,LR_SHARED);
				break;
			default:
				lpwszTitlepart=_T("Unknown event type");
			}

			wszPhoneLocal[0]='+';
			if (dwPhoneSize)
			{
				dwPhoneSize=CopyNumberW((wszPhoneLocal+1),lpwszPhone,dwPhoneSize);
			}else{
				GetDataFromMessage(lpszMessage,dwMessageSize,NULL,(wszPhoneLocal+1),(SIZEOF(wszPhoneLocal)-1),&dwPhoneSize,NULL);
				dwPhoneSize++;
			}

			lpwszContactDisplayName=GetContactNameW(hContact);
			mir_sntprintf(wszTitle,SIZEOF(wszTitle),_T("%s - %s"),lpwszContactDisplayName,lpwszTitlepart);
			MultiByteToWideChar(CP_UTF8,0,lpszMessage,dwMessageSize,lpwszMessage,(dwMessageSize+MAX_PATH));

			SendMessageW(prswdWindowData->hWnd,WM_SETTEXT,NULL,(LPARAM)wszTitle);
			SetDlgItemText(prswdWindowData->hWnd,IDC_NAME,lpwszContactDisplayName);
			SetDlgItemText(prswdWindowData->hWnd,IDC_NUMBER,wszPhoneLocal);
			SetDlgItemText(prswdWindowData->hWnd,IDC_MESSAGE,lpwszMessage);
			SendMessage(prswdWindowData->hWnd,WM_SETICON,ICON_BIG,(LPARAM)hIcon);

			SetFocus(GetDlgItem(prswdWindowData->hWnd,IDC_MESSAGE));
			hRet=prswdWindowData->hWnd;
		}else{
			MEMFREE(prswdWindowData);
		}
	}
	MEMFREE(lpwszMessage);
	return(hRet);
}

//This function close the SMS receive window that given, and remove it from the list.
//The function gets the HWND of the window that should be removed and return void
void RecvSMSWindowRemove(HWND hWndDlg)
{
	RECV_SMS_WINDOW_DATA *prswdWindowData=GET_WINDOW_DATA(hWndDlg);

	if (prswdWindowData)
	{
		Utils_SaveWindowPosition(hWndDlg,(DB_SMS_GetByte(NULL,"SavePerContact",0)? prswdWindowData->hContact:NULL),PROTOCOL_NAMEA,"recv");
	
		ListMTLock(&ssSMSSettings.lmtRecvSMSWindowsListMT);
		ListMTItemDelete(&ssSMSSettings.lmtRecvSMSWindowsListMT,&prswdWindowData->lmtListMTItem);
		ListMTUnLock(&ssSMSSettings.lmtRecvSMSWindowsListMT);
		MEMFREE(prswdWindowData);
	}
	DestroyWindow(hWndDlg);
}

