#include "common.h"

int LoadServices(void)
{
	char szServiceFunction[MAX_PATH],*pszServiceFunctionName;

	CopyMemory(szServiceFunction,PROTOCOL_NAMEA,PROTOCOL_NAME_SIZE);
	pszServiceFunctionName=szServiceFunction+PROTOCOL_NAME_LEN;

	// Service creation
	for (size_t i=0;i<SIZEOF(siPluginServices);i++)
	{
		CopyMemory(pszServiceFunctionName,siPluginServices[i].lpszName,(mir_strlen(siPluginServices[i].lpszName)+1));
		CreateServiceFunction(szServiceFunction,(MIRANDASERVICE)siPluginServices[i].lpFunc);
	}
	return 0;
}


int LoadModules(void)
{
	HookEvent(ME_OPT_INITIALISE,OptInitialise);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU,SmsRebuildContactMenu);
	HookEvent(ME_PROTO_ACK,handleAckSMS);
	HookEvent(ME_DB_EVENT_ADDED,handleNewMessage);
	HookEvent(ME_PROTO_ACCLISTCHANGED,RefreshAccountList);

	char szServiceFunction[MAX_PATH];
	mir_snprintf(szServiceFunction,sizeof(szServiceFunction),"%s%s",PROTOCOL_NAMEA,SMS_SEND);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 300050000;
	mi.hIcon = LoadSkinnedIcon(SKINICON_OTHER_SMS);
	mi.ptszName = SMS_SEND_STR;
	mi.pszService = szServiceFunction;
	mi.flags = CMIF_TCHAR;
	Menu_AddMainMenuItem(&mi);

	mi.position = -2000070000;
	mi.hIcon = LoadSkinnedIcon(SKINICON_OTHER_SMS);
	mi.ptszName = SMS_SEND_CM_STR;
	mi.pszService = szServiceFunction;
	mi.flags = CMIF_TCHAR;	
	ssSMSSettings.hContactMenuItems[0] = Menu_AddContactMenuItem(&mi);

	SkinAddNewSoundEx("RecvSMSMsg",PROTOCOL_NAMEA,LPGEN("Incoming SMS Message"));
	SkinAddNewSoundEx("RecvSMSConfirmation",PROTOCOL_NAMEA,LPGEN("Incoming SMS Confirmation"));

	RefreshAccountList(NULL,NULL);

	RestoreUnreadMessageAlerts();
	return 0;
}

int SmsRebuildContactMenu(WPARAM wParam,LPARAM lParam)
{
	Menu_ShowItem(ssSMSSettings.hContactMenuItems[0], GetContactPhonesCount(wParam));
	return 0;
}

//This function called when user clicked Menu.
int SendSMSMenuCommand(WPARAM wParam,LPARAM lParam)
{
	HWND hwndSendSms;

	// user clicked on the "SMS Message" on one of the users
	if (wParam) {
		hwndSendSms = SendSMSWindowIsOtherInstanceHContact(wParam);
		if (hwndSendSms)
			SetFocus(hwndSendSms);
		else
			hwndSendSms = SendSMSWindowAdd(wParam);
	}
	// user clicked on the "SMS Send" in the Main Menu
	else{
		hwndSendSms = SendSMSWindowAdd(NULL);
		EnableWindow(GetDlgItem(hwndSendSms,IDC_NAME),TRUE);
		EnableWindow(GetDlgItem(hwndSendSms,IDC_SAVENUMBER),FALSE);

		for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
			if (GetContactPhonesCount(hContact)) {
				SendDlgItemMessage(hwndSendSms,IDC_NAME,CB_ADDSTRING,0,(LPARAM)GetContactNameW(hContact));
				SendSMSWindowSMSContactAdd(hwndSendSms,hContact);
			}
		}
	}
	return 0;
}


//This function used to popup a read SMS window after the user clicked on the received SMS message.
int ReadMsgSMS(WPARAM wParam,LPARAM lParam)
{
	CLISTEVENT *cle = (CLISTEVENT*)lParam;

	DBEVENTINFO dbei = { sizeof(dbei) };
	if ((dbei.cbBlob = db_event_getBlobSize(((CLISTEVENT*)lParam)->hDbEvent)) == -1)
		return 1;
	dbei.pBlob = (PBYTE)_alloca(dbei.cbBlob);

	if (db_event_get(cle->hDbEvent, &dbei) == 0)
	if (dbei.eventType == ICQEVENTTYPE_SMS || dbei.eventType == ICQEVENTTYPE_SMSCONFIRMATION)
	if (dbei.cbBlob > MIN_SMS_DBEVENT_LEN) {
		if (RecvSMSWindowAdd(cle->hContact,ICQEVENTTYPE_SMS,NULL,0,(LPSTR)dbei.pBlob,dbei.cbBlob)) {
			db_event_markRead(cle->hContact, cle->hDbEvent);
			return 0;
		}
	}
	return 1;
}

//This function used to popup a read SMS window after the user clicked on the received SMS confirmation.
int ReadAckSMS(WPARAM wParam,LPARAM lParam)
{
	CLISTEVENT *cle = (CLISTEVENT*)lParam;

	DBEVENTINFO dbei = { sizeof(dbei) };
	if ((dbei.cbBlob = db_event_getBlobSize(cle->hDbEvent)) == -1)
		return 1;
	dbei.pBlob = (PBYTE)_alloca(dbei.cbBlob);

	if (db_event_get(cle->hDbEvent, &dbei) == 0)
	if (dbei.eventType == ICQEVENTTYPE_SMS || dbei.eventType == ICQEVENTTYPE_SMSCONFIRMATION)
	if (dbei.cbBlob > MIN_SMS_DBEVENT_LEN) {
		if (RecvSMSWindowAdd(cle->hContact, ICQEVENTTYPE_SMSCONFIRMATION, NULL, 0, (LPSTR)dbei.pBlob, dbei.cbBlob)) {
			db_event_delete(cle->hContact, cle->hDbEvent);
			return 0;
		}
	}
	return 1;
}

void RestoreUnreadMessageAlerts(void)
{
	DBEVENTINFO dbei = { sizeof(dbei) };

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		for (HANDLE hDbEvent = db_event_firstUnread(hContact); hDbEvent; hDbEvent = db_event_next(hContact, hDbEvent)) {
			dbei.cbBlob = 0;
			if (db_event_get(hDbEvent, &dbei) == 0)
			if ((dbei.flags & (DBEF_SENT|DBEF_READ))==0 && ((dbei.eventType==ICQEVENTTYPE_SMS) || (dbei.eventType==ICQEVENTTYPE_SMSCONFIRMATION)))
			if (dbei.cbBlob>MIN_SMS_DBEVENT_LEN)
				handleNewMessage(hContact,(LPARAM)hDbEvent);
		}

	for (HANDLE hDbEvent = db_event_firstUnread(NULL); hDbEvent; hDbEvent = db_event_next(NULL, hDbEvent)) {
		dbei.cbBlob = 0;
		if (db_event_get(hDbEvent, &dbei) == 0)
		if ((dbei.flags & (DBEF_SENT|DBEF_READ))==0 && ((dbei.eventType==ICQEVENTTYPE_SMS) || (dbei.eventType==ICQEVENTTYPE_SMSCONFIRMATION)))
		if (dbei.cbBlob > MIN_SMS_DBEVENT_LEN)
			handleNewMessage(NULL, (LPARAM)hDbEvent);
	}
}
