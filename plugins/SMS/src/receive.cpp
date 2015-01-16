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

//This function handles the ACK received from that hooked.
int handleAckSMS(WPARAM wParam, LPARAM lParam)
{
	if ( !lParam)
		return 0;
	if (((ACKDATA*)lParam)->type != ICQACKTYPE_SMS)
		return 0;

	char szPhone[MAX_PHONE_LEN] = { 0 };
	TCHAR tszPhone[MAX_PHONE_LEN] = { 0 };
	LPSTR lpszXML = (LPSTR)((ACKDATA*)lParam)->lParam, lpszData, lpszPhone;
	size_t dwXMLSize = 0, dwDataSize, dwPhoneSize;
	ACKDATA *ack = ((ACKDATA*)lParam);

	if (lpszXML)
		dwXMLSize = mir_strlen(lpszXML);

	if (GetXMLFieldEx(lpszXML,dwXMLSize,&lpszData,&dwDataSize,"sms_message", "text", NULL))
	{
		if (GetXMLFieldEx(lpszXML,dwXMLSize,&lpszPhone,&dwPhoneSize,"sms_message","sender",NULL))
		{
			LPSTR lpszMessageUTF;
			size_t dwBuffLen,dwMessageXMLEncodedSize,dwMessageXMLDecodedSize;
			DBEVENTINFO dbei = { sizeof(dbei) };

			dwBuffLen=(dwDataSize+MAX_PATH);
			dbei.pBlob=(LPBYTE)MEMALLOC((dwBuffLen+dwPhoneSize));
			LPWSTR lpwszMessageXMLEncoded=(LPWSTR)MEMALLOC((dwBuffLen*sizeof(WCHAR)));
			LPWSTR lpwszMessageXMLDecoded=(LPWSTR)MEMALLOC((dwBuffLen*sizeof(WCHAR)));
			if (dbei.pBlob && lpwszMessageXMLEncoded && lpwszMessageXMLDecoded)
			{
				dwMessageXMLEncodedSize=MultiByteToWideChar(CP_UTF8,0,lpszData,dwDataSize,lpwszMessageXMLEncoded,dwBuffLen);
				DecodeXML(lpwszMessageXMLEncoded,dwMessageXMLEncodedSize,lpwszMessageXMLDecoded,dwBuffLen,&dwMessageXMLDecodedSize);
				lpszMessageUTF=(LPSTR)lpwszMessageXMLEncoded;
				WideCharToMultiByte(CP_UTF8,0,lpwszMessageXMLDecoded,dwMessageXMLDecodedSize,lpszMessageUTF,dwBuffLen,NULL,NULL);

				dwPhoneSize=CopyNumberA(szPhone,lpszPhone,dwPhoneSize);
				dwPhoneSize=MultiByteToWideChar(CP_UTF8,0,szPhone,dwPhoneSize,tszPhone,MAX_PHONE_LEN);
				MCONTACT hContact=HContactFromPhone(tszPhone,dwPhoneSize);

				dbei.szModule=GetModuleName(hContact);
				dbei.timestamp=time(NULL);
				dbei.flags = DBEF_UTF;
				dbei.eventType = ICQEVENTTYPE_SMS;
				dbei.cbBlob=(mir_snprintf((LPSTR)dbei.pBlob,((dwBuffLen+dwPhoneSize)),"SMS From: +%s\r\n%s",szPhone,lpszMessageUTF)+sizeof(DWORD));
				//dbei.pBlob=(LPBYTE)lpszBuff;
				(*((DWORD*)(dbei.pBlob+(dbei.cbBlob-sizeof(DWORD)))))=0;
				MEVENT hResult = db_event_add(hContact, &dbei);
				if (hContact==NULL) {	
					if ( RecvSMSWindowAdd(NULL,ICQEVENTTYPE_SMS,tszPhone,dwPhoneSize,(LPSTR)dbei.pBlob,dbei.cbBlob)) {
						db_event_markRead(hContact, hResult);
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
			dwPhoneSize=CopyNumberA(szPhone,lpszPhone,dwPhoneSize);
			dwPhoneSize=MultiByteToWideChar(CP_UTF8,0,szPhone,dwPhoneSize,tszPhone,MAX_PHONE_LEN);
			MCONTACT hContact=HContactFromPhone(tszPhone,dwPhoneSize);

			DBEVENTINFO dbei={0};
			dbei.cbSize=sizeof(dbei);
			dbei.szModule=GetModuleName(hContact);
			dbei.timestamp=time(NULL);
			dbei.flags = DBEF_UTF;
			dbei.eventType = ICQEVENTTYPE_SMSCONFIRMATION;
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
					memcpy((dbei.pBlob+dbei.cbBlob),lpszData,dwDataSize);
					dbei.cbBlob+=(dwDataSize+sizeof(DWORD));
					(*((DWORD*)(dbei.pBlob+(dbei.cbBlob-sizeof(DWORD)))))=0;
				}
			}

			if (dbei.pBlob) {
				if (hContact)
					db_event_add(hContact, &dbei);
				else
					RecvSMSWindowAdd(NULL,ICQEVENTTYPE_SMSCONFIRMATION,tszPhone,dwPhoneSize,(LPSTR)dbei.pBlob,dbei.cbBlob);

				MEMFREE(dbei.pBlob);
			}
		}
	}else
	if ((ack->result == ACKRESULT_FAILED) || GetXMLFieldEx(lpszXML,dwXMLSize,&lpszData,&dwDataSize,"sms_response","deliverable",NULL))
	{
		HWND hWndDlg = SendSMSWindowHwndByHProcessGet(ack->hProcess);
		if (hWndDlg) {
			char szNetwork[MAX_PATH];

			KillTimer(hWndDlg, wParam);
			GetXMLFieldExBuff(lpszXML,dwXMLSize,szNetwork,sizeof(szNetwork),NULL,"sms_response","network",NULL);

			if (ack->result == ACKRESULT_FAILED || CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,lpszData,dwDataSize,"no",2)==CSTR_EQUAL) {	
				char szBuff[1024];
				TCHAR tszErrorMessage[1028];
				LPSTR lpszErrorDescription;

				if (SendSMSWindowMultipleGet(hWndDlg)) {
					TVITEM tvi;
					tvi.mask=TVIF_TEXT;
					tvi.hItem=SendSMSWindowHItemSendGet(hWndDlg);
					tvi.pszText=tszPhone;
					tvi.cchTextMax=SIZEOF(tszPhone);
					TreeView_GetItem(GetDlgItem(hWndDlg,IDC_NUMBERSLIST),&tvi);
				}
				else GetDlgItemText(hWndDlg,IDC_ADDRESS,tszPhone,SIZEOF(szPhone));

				if (ack->result == ACKRESULT_FAILED)
					lpszErrorDescription=lpszXML;
				else {
					lpszErrorDescription=szBuff;
					GetXMLFieldExBuff(lpszXML,dwXMLSize,szBuff,sizeof(szBuff),NULL,"sms_response","error","params","param",NULL);
				}

				mir_sntprintf(tszErrorMessage,SIZEOF(tszErrorMessage),TranslateT("SMS message didn't send by %S to %s because: %S"),szNetwork,tszPhone,lpszErrorDescription);
				ShowWindow(hWndDlg,SW_SHOWNORMAL);
				EnableWindow(hWndDlg,FALSE);
				HWND hwndTimeOut=CreateDialog(ssSMSSettings.hInstance,MAKEINTRESOURCE(IDD_SENDSMSTIMEDOUT),hWndDlg,SMSTimedOutDlgProc);
				SetDlgItemText(hwndTimeOut,IDC_STATUS,tszErrorMessage);
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
								SetDlgItemText(hwndAccepted,IDC_ST_SOURCE,TranslateT("From:"));
								SetDlgItemText(hwndAccepted,IDC_ST_MESSAGEID,TranslateT("To:"));
								GetXMLFieldExBuff(lpszXML,dwXMLSize,szSource,sizeof(szSource),NULL,"sms_response","from",NULL);
								GetXMLFieldExBuff(lpszXML,dwXMLSize,szMessageID,sizeof(szMessageID),NULL,"sms_response","to",NULL);
							}
							SetDlgItemTextA(hwndAccepted,IDC_NETWORK,szNetwork);
							SetDlgItemTextA(hwndAccepted,IDC_SOURCE,szSource);
							SetDlgItemTextA(hwndAccepted,IDC_MESSAGEID,szMessageID);
						}
						else SendSMSWindowRemove(hWndDlg);
					}
					else SendSMSWindowRemove(hWndDlg);
				}
			}
		}
	}
	return 0;
}

//Handles new SMS messages added to the database
int handleNewMessage(WPARAM hContact, LPARAM hDbEvent)
{
	char szServiceFunction[MAX_PATH], *pszServiceFunctionName;
	TCHAR szToolTip[MAX_PATH];
	DBEVENTINFO dbei = { sizeof(dbei) };
	
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == -1)
		return 0;

	dbei.pBlob = (PBYTE)MEMALLOC(dbei.cbBlob);
	if ( !dbei.pBlob)
		return 0;
	memcpy(szServiceFunction,PROTOCOL_NAMEA,PROTOCOL_NAME_SIZE);
	pszServiceFunctionName = szServiceFunction + PROTOCOL_NAME_LEN;

	if (db_event_get(hDbEvent, &dbei) == 0)
	if ((dbei.flags & DBEF_SENT) == 0)
	if (dbei.eventType == ICQEVENTTYPE_SMS) {
		if (dbei.cbBlob>MIN_SMS_DBEVENT_LEN) {
			SkinPlaySound("RecvSMSMsg");
			if (DB_SMS_GetByte(NULL,"AutoPopup",0)) {
				if (RecvSMSWindowAdd(hContact,ICQEVENTTYPE_SMS,NULL,0,(LPSTR)dbei.pBlob,dbei.cbBlob))
					db_event_markRead(hContact, hDbEvent);
			}
			else {
				memcpy(pszServiceFunctionName,SMS_READ,sizeof(SMS_READ));
				mir_sntprintf(szToolTip,SIZEOF(szToolTip),TranslateT("SMS Message from %s"),GetContactNameW(hContact));

				CLISTEVENT cle = { sizeof(cle) };
				cle.flags = CLEF_TCHAR;
				cle.hContact = hContact;
				cle.hDbEvent = hDbEvent;
				cle.hIcon = LoadSkinnedIcon(SKINICON_OTHER_SMS);
				cle.pszService = szServiceFunction;
				cle.ptszTooltip = szToolTip;
				CallService(MS_CLIST_ADDEVENT,0,(LPARAM)&cle);
			}
		}
	}
	else if (dbei.eventType == ICQEVENTTYPE_SMSCONFIRMATION) {
		SkinPlaySound("RecvSMSConfirmation");
		if (DB_SMS_GetByte(NULL, "AutoPopup", 0)) {
			if (RecvSMSWindowAdd(hContact,ICQEVENTTYPE_SMSCONFIRMATION,NULL,0,(LPSTR)dbei.pBlob,dbei.cbBlob))
				db_event_delete(hContact, hDbEvent);
		}
		else {
			UINT iIcon;
			if (GetDataFromMessage((LPSTR)dbei.pBlob, dbei.cbBlob, NULL, NULL, 0, NULL, &iIcon)) {
				memcpy(pszServiceFunctionName,SMS_READ_ACK,sizeof(SMS_READ_ACK));
				mir_sntprintf(szToolTip,SIZEOF(szToolTip),TranslateT("SMS Confirmation from %s"),GetContactNameW(hContact));

				CLISTEVENT cle = { sizeof(cle) };
				cle.flags = CLEF_TCHAR;
				cle.hContact = hContact;
				cle.hDbEvent = hDbEvent;
				cle.hIcon = (HICON)LoadImage(ssSMSSettings.hInstance,MAKEINTRESOURCE(iIcon),IMAGE_ICON,0,0,LR_SHARED);
				cle.pszService = szServiceFunction;
				cle.ptszTooltip = szToolTip;
				CallService(MS_CLIST_ADDEVENT,0,(LPARAM)&cle);
			}
		}
	}
	MEMFREE(dbei.pBlob);
	return 0;
}
