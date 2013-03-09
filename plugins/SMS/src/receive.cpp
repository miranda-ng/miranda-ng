/*
Miranda-IM SMS Plugin
Copyright (C) 2001-2  Richard Hughes
Copyright (C) 2007-2009  Rozhuk Ivan

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

//This function handles the ACK received from that hooked.
int handleAckSMS(WPARAM wParam,LPARAM lParam)
{
	if (lParam)
	if (((ACKDATA*)lParam)->type==ICQACKTYPE_SMS)
	{
		char szPhone[MAX_PHONE_LEN]={0};
		WCHAR wszPhone[MAX_PHONE_LEN]={0};
		LPSTR lpszXML=(LPSTR)((ACKDATA*)lParam)->lParam,lpszData,lpszPhone;
		SIZE_T dwXMLSize=lstrlenA(lpszXML),dwDataSize,dwPhoneSize;
		ACKDATA *ack=((ACKDATA*)lParam);

		if (GetXMLFieldEx(lpszXML,dwXMLSize,&lpszData,&dwDataSize,"sms_message","text",NULL))
		{
			if (GetXMLFieldEx(lpszXML,dwXMLSize,&lpszPhone,&dwPhoneSize,"sms_message","sender",NULL))
			{
				LPSTR lpszMessageUTF;
				LPWSTR lpwszMessageXMLEncoded,lpwszMessageXMLDecoded;
				SIZE_T dwBuffLen,dwMessageXMLEncodedSize,dwMessageXMLDecodedSize;
				HANDLE hContact;
				DBEVENTINFO dbei={0};

				dwBuffLen=(dwDataSize+MAX_PATH);
				dbei.pBlob=(LPBYTE)MEMALLOC((dwBuffLen+dwPhoneSize));
				lpwszMessageXMLEncoded=(LPWSTR)MEMALLOC((dwBuffLen*sizeof(WCHAR)));
				lpwszMessageXMLDecoded=(LPWSTR)MEMALLOC((dwBuffLen*sizeof(WCHAR)));
				if (dbei.pBlob && lpwszMessageXMLEncoded && lpwszMessageXMLDecoded)
				{
					dwMessageXMLEncodedSize=MultiByteToWideChar(CP_UTF8,0,lpszData,dwDataSize,lpwszMessageXMLEncoded,dwBuffLen);
					DecodeXML(lpwszMessageXMLEncoded,dwMessageXMLEncodedSize,lpwszMessageXMLDecoded,dwBuffLen,&dwMessageXMLDecodedSize);
					lpszMessageUTF=(LPSTR)lpwszMessageXMLEncoded;
					WideCharToMultiByte(CP_UTF8,0,lpwszMessageXMLDecoded,dwMessageXMLDecodedSize,lpszMessageUTF,dwBuffLen,NULL,NULL);

					dwPhoneSize=CopyNumberA(szPhone,lpszPhone,dwPhoneSize);
					dwPhoneSize=MultiByteToWideChar(CP_UTF8,0,szPhone,dwPhoneSize,wszPhone,MAX_PHONE_LEN);
					hContact=HContactFromPhone(wszPhone,dwPhoneSize);

					dbei.cbSize=sizeof(dbei);
					dbei.szModule=GetModuleName(hContact);
					dbei.timestamp=time(NULL);
					dbei.flags=(DBEF_UTF);
					dbei.eventType=ICQEVENTTYPE_SMS;
					dbei.cbBlob=(mir_snprintf((LPSTR)dbei.pBlob,((dwBuffLen+dwPhoneSize)),"SMS From: +%s\r\n%s",szPhone,lpszMessageUTF)+sizeof(DWORD));
					//dbei.pBlob=(LPBYTE)lpszBuff;
					(*((DWORD*)(dbei.pBlob+(dbei.cbBlob-sizeof(DWORD)))))=0;
					CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&dbei);
					if (hContact==NULL)
					{	
						if (RecvSMSWindowAdd(NULL,ICQEVENTTYPE_SMS,wszPhone,dwPhoneSize,(LPSTR)dbei.pBlob,dbei.cbBlob))
						{
							CallService(MS_DB_EVENT_MARKREAD,(WPARAM)hContact,(LPARAM)&dbei);
							SkinPlaySound("RecvSMSMsg");
						}
					}
				}
				MEMFREE(lpwszMessageXMLDecoded);
				MEMFREE(lpwszMessageXMLEncoded);
				MEMFREE(dbei.pBlob);
			}
		}else
		if (GetXMLFieldEx(lpszXML,dwXMLSize,&lpszData,&dwDataSize,"sms_delivery_receipt","delivered",NULL))
		{
			if (GetXMLFieldEx(lpszXML,dwXMLSize,&lpszPhone,&dwPhoneSize,"sms_delivery_receipt","destination",NULL))
			{
				HANDLE hContact;
				DBEVENTINFO dbei={0};

				dwPhoneSize=CopyNumberA(szPhone,lpszPhone,dwPhoneSize);
				dwPhoneSize=MultiByteToWideChar(CP_UTF8,0,szPhone,dwPhoneSize,wszPhone,MAX_PHONE_LEN);
				hContact=HContactFromPhone(wszPhone,dwPhoneSize);

				dbei.cbSize=sizeof(dbei);
				dbei.szModule=GetModuleName(hContact);
				dbei.timestamp=time(NULL);
				dbei.eventType=ICQEVENTTYPE_SMSCONFIRMATION;
				if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpszData,dwDataSize,"yes",3)==CSTR_EQUAL)
				{
					dbei.cbBlob=(MAX_PHONE_LEN+MAX_PATH);
					dbei.pBlob=(PBYTE)MEMALLOC(dbei.cbBlob);
					if (dbei.pBlob) dbei.cbBlob=(mir_snprintf((LPSTR)dbei.pBlob,dbei.cbBlob,"SMS Confirmation From: +%s\r\nSMS was sent succesfully",szPhone)+4);
				}else{
					if (GetXMLFieldEx(lpszXML,dwXMLSize,&lpszData,&dwDataSize,"sms_delivery_receipt","error","params","param",NULL)==FALSE)
					{
						lpszData="";
						dwDataSize=0;
					}
					dbei.cbBlob=(MAX_PHONE_LEN+MAX_PATH+dwDataSize);
					dbei.pBlob=(PBYTE)MEMALLOC(dbei.cbBlob);
					if (dbei.pBlob)
					{
						dbei.cbBlob=mir_snprintf((LPSTR)dbei.pBlob,dbei.cbBlob,"SMS Confirmation From: +%s\r\nSMS was not sent succesfully: ",szPhone);
						CopyMemory((dbei.pBlob+dbei.cbBlob),lpszData,dwDataSize);
						dbei.cbBlob+=(dwDataSize+sizeof(DWORD));
						(*((DWORD*)(dbei.pBlob+(dbei.cbBlob-sizeof(DWORD)))))=0;
					}
				}

				if (dbei.pBlob)
				{
					if (hContact)
					{
						CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&dbei);
					}else{
						RecvSMSWindowAdd(NULL,ICQEVENTTYPE_SMSCONFIRMATION,wszPhone,dwPhoneSize,(LPSTR)dbei.pBlob,dbei.cbBlob);
					}
					MEMFREE(dbei.pBlob);
				}
			}
		}else
		if ((ack->result==ACKRESULT_FAILED) || GetXMLFieldEx(lpszXML,dwXMLSize,&lpszData,&dwDataSize,"sms_response","deliverable",NULL))
		{
			HWND hWndDlg=SendSMSWindowHwndByHProcessGet(ack->hProcess);
			if (hWndDlg) {
				char szNetwork[MAX_PATH];

				KillTimer(hWndDlg,wParam);
				GetXMLFieldExBuff(lpszXML,dwXMLSize,szNetwork,sizeof(szNetwork),NULL,"sms_response","network",NULL);

				if (ack->result==ACKRESULT_FAILED || CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpszData,dwDataSize,"no",2)==CSTR_EQUAL) {	
					HWND hwndTimeOut;
					char szBuff[1024];
					WCHAR wszErrorMessage[1028];
					LPSTR lpszErrorDescription;

					if (SendSMSWindowMultipleGet(hWndDlg)) {
						TVITEM tvi;
						tvi.mask=TVIF_TEXT;
						tvi.hItem=SendSMSWindowHItemSendGet(hWndDlg);
						tvi.pszText=wszPhone;
						tvi.cchTextMax=SIZEOF(wszPhone);
						TreeView_GetItem(GetDlgItem(hWndDlg,IDC_NUMBERSLIST),&tvi);
					}
					else GET_DLG_ITEM_TEXTW(hWndDlg,IDC_ADDRESS,wszPhone,SIZEOF(szPhone));

					if (ack->result == ACKRESULT_FAILED)
						lpszErrorDescription=lpszXML;
					else {
						lpszErrorDescription=szBuff;
						GetXMLFieldExBuff(lpszXML,dwXMLSize,szBuff,sizeof(szBuff),NULL,"sms_response","error","params","param",NULL);
					}

					mir_sntprintf(wszErrorMessage,SIZEOF(wszErrorMessage),TranslateT("SMS message didn't send by %S to %s because: %S"),szNetwork,wszPhone,lpszErrorDescription);
					ShowWindow(hWndDlg,SW_SHOWNORMAL);
					EnableWindow(hWndDlg,FALSE);
					hwndTimeOut=CreateDialog(ssSMSSettings.hInstance,MAKEINTRESOURCE(IDD_SENDSMSTIMEDOUT),hWndDlg,SMSTimedOutDlgProc);
					SET_DLG_ITEM_TEXTW(hwndTimeOut,IDC_STATUS,wszErrorMessage);
				}
				else {
					SendSMSWindowDBAdd(hWndDlg);
					if ( SendSMSWindowMultipleGet(hWndDlg)) {
						if ( SendSMSWindowNextHItemGet(hWndDlg,SendSMSWindowHItemSendGet(hWndDlg))) {
							SendSMSWindowAsSentSet(hWndDlg);
							SendSMSWindowHItemSendSet(hWndDlg,SendSMSWindowNextHItemGet(hWndDlg,SendSMSWindowHItemSendGet(hWndDlg)));
							SendSMSWindowNext(hWndDlg);
						}
						else SendSMSWindowRemove(hWndDlg);
					}
					else {
						if ( CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpszData,dwDataSize,"yes",3)==CSTR_EQUAL ||
							  CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpszData,dwDataSize,"smtp",4)==CSTR_EQUAL) {
							char szSource[MAX_PATH],szMessageID[MAX_PATH];

							if (DB_SMS_GetByte(NULL,"ShowACK",SMS_DEFAULT_SHOWACK)) {	
								HWND hwndAccepted=CreateDialog(ssSMSSettings.hInstance,MAKEINTRESOURCE(IDD_SENDSMSACCEPT),hWndDlg,SMSAcceptedDlgProc);
								if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpszData,dwDataSize,"yes",3)==CSTR_EQUAL) {
									GetXMLFieldExBuff(lpszXML,dwXMLSize,szSource,sizeof(szSource),NULL,"sms_response","source",NULL);
									GetXMLFieldExBuff(lpszXML,dwXMLSize,szMessageID,sizeof(szMessageID),NULL,"sms_response","message_id",NULL);
								}
								else {
									SET_DLG_ITEM_TEXTW(hwndAccepted,IDC_ST_SOURCE,TranslateT("From:"));
									SET_DLG_ITEM_TEXTW(hwndAccepted,IDC_ST_MESSAGEID,TranslateT("To:"));
									GetXMLFieldExBuff(lpszXML,dwXMLSize,szSource,sizeof(szSource),NULL,"sms_response","from",NULL);
									GetXMLFieldExBuff(lpszXML,dwXMLSize,szMessageID,sizeof(szMessageID),NULL,"sms_response","to",NULL);
								}
								SET_DLG_ITEM_TEXTA(hwndAccepted,IDC_NETWORK,szNetwork);
								SET_DLG_ITEM_TEXTA(hwndAccepted,IDC_SOURCE,szSource);
								SET_DLG_ITEM_TEXTA(hwndAccepted,IDC_MESSAGEID,szMessageID);
							}
							else SendSMSWindowRemove(hWndDlg);
						}
						else SendSMSWindowRemove(hWndDlg);
					}
				}
			}
		}
	}
	return 0;
}

//Handles new SMS messages added to the database
int handleNewMessage(WPARAM wParam,LPARAM lParam)
{
	CHAR szServiceFunction[MAX_PATH],*pszServiceFunctionName;
	WCHAR szToolTip[MAX_PATH];
	HANDLE hContact=(HANDLE)wParam,hDbEvent=(HANDLE)lParam;
	CLISTEVENT cle={0};
	DBEVENTINFO dbei={0};

	dbei.cbSize=sizeof(dbei);
	if ((dbei.cbBlob=CallService(MS_DB_EVENT_GETBLOBSIZE,lParam,0))!=-1)
	{
		dbei.pBlob=(PBYTE)MEMALLOC(dbei.cbBlob);
		if (dbei.pBlob)
		{
			CopyMemory(szServiceFunction,PROTOCOL_NAMEA,PROTOCOL_NAME_SIZE);
			pszServiceFunctionName=szServiceFunction+PROTOCOL_NAME_LEN;

			if (CallService(MS_DB_EVENT_GET,lParam,(LPARAM)&dbei)==0)
			if ((dbei.flags&DBEF_SENT)==0)
			if (dbei.eventType==ICQEVENTTYPE_SMS)
			{
				if (dbei.cbBlob>MIN_SMS_DBEVENT_LEN)
				{
					SkinPlaySound("RecvSMSMsg");
					if (DB_SMS_GetByte(NULL,"AutoPopup",0))
					{
						if (RecvSMSWindowAdd(hContact,ICQEVENTTYPE_SMS,NULL,0,(LPSTR)dbei.pBlob,dbei.cbBlob))
						{
							CallService(MS_DB_EVENT_MARKREAD,(WPARAM)hContact,(LPARAM)&dbei);
						}
					}else{
						CopyMemory(pszServiceFunctionName,SMS_READ,sizeof(SMS_READ));

						cle.cbSize=sizeof(cle);
						cle.flags=CLEF_TCHAR;
						cle.hContact=hContact;
						cle.hDbEvent=hDbEvent;
						cle.hIcon=LoadSkinnedIcon(SKINICON_OTHER_SMS);
						cle.pszService=szServiceFunction;
						mir_sntprintf(szToolTip,SIZEOF(szToolTip),TranslateT("SMS Message from %s"),GetContactNameW(hContact));
						cle.ptszTooltip=szToolTip;
						CallService(MS_CLIST_ADDEVENT,0,(LPARAM)&cle);
					}
				}
			}else
			if (dbei.eventType==ICQEVENTTYPE_SMSCONFIRMATION)
			{
				SkinPlaySound("RecvSMSConfirmation");
				if (DB_SMS_GetByte(NULL,"AutoPopup",0)) 
				{
					if (RecvSMSWindowAdd(hContact,ICQEVENTTYPE_SMSCONFIRMATION,NULL,0,(LPSTR)dbei.pBlob,dbei.cbBlob))
					{
						CallService(MS_DB_EVENT_DELETE,(WPARAM)hContact,(LPARAM)&dbei);
					}
				}else{
					UINT iIcon;

					if (GetDataFromMessage((LPSTR)dbei.pBlob,dbei.cbBlob,NULL,NULL,0,NULL,&iIcon))
					{
						CopyMemory(pszServiceFunctionName,SMS_READ_ACK,sizeof(SMS_READ_ACK));

						cle.cbSize=sizeof(cle);
						cle.flags=CLEF_TCHAR;
						cle.hContact=hContact;
						cle.hDbEvent=hDbEvent;
						cle.hIcon=(HICON)LoadImage(ssSMSSettings.hInstance,MAKEINTRESOURCE(iIcon),IMAGE_ICON,0,0,LR_SHARED);
						cle.pszService=szServiceFunction;
						mir_sntprintf(szToolTip,SIZEOF(szToolTip),TranslateT("SMS Confirmation from %s"),GetContactNameW(hContact));
						cle.ptszTooltip=szToolTip;
						CallService(MS_CLIST_ADDEVENT,0,(LPARAM)&cle);
					}
				}
			}
			MEMFREE(dbei.pBlob);
		}
	}
	return 0;
}


