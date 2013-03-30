#include "common.h"

int LoadServices(void)
{
	CHAR szServiceFunction[MAX_PATH],*pszServiceFunctionName;

	CopyMemory(szServiceFunction,PROTOCOL_NAMEA,PROTOCOL_NAME_SIZE);
	pszServiceFunctionName=szServiceFunction+PROTOCOL_NAME_LEN;

	// Service creation
	for (SIZE_T i=0;i<SIZEOF(siPluginServices);i++)
	{
		CopyMemory(pszServiceFunctionName,siPluginServices[i].lpszName,(lstrlenA(siPluginServices[i].lpszName)+1));
		CreateServiceFunction(szServiceFunction,(MIRANDASERVICE)siPluginServices[i].lpFunc);
	}
	return 0;
}


int LoadModules(void)
{
	ssSMSSettings.hHookOptInitialize=HookEvent(ME_OPT_INITIALISE,OptInitialise);
	ssSMSSettings.hHookRebuildCMenu=HookEvent(ME_CLIST_PREBUILDCONTACTMENU,SmsRebuildContactMenu);
	ssSMSSettings.hHookProtoAck=HookEvent(ME_PROTO_ACK,handleAckSMS);
	ssSMSSettings.hHookDbAdd=HookEvent(ME_DB_EVENT_ADDED,handleNewMessage);
	ssSMSSettings.hHookAccListChanged=HookEvent(ME_PROTO_ACCLISTCHANGED,RefreshAccountList);
	

	CLISTMENUITEM mi={0};
	CHAR szServiceFunction[MAX_PATH];

	mir_snprintf(szServiceFunction,sizeof(szServiceFunction),"%s%s",PROTOCOL_NAMEA,SMS_SEND);

	mi.cbSize=sizeof(mi);
	mi.position=300050000;
	mi.hIcon=LoadSkinnedIcon(SKINICON_OTHER_SMS);
	mi.ptszName=SMS_SEND_STR;
	mi.pszService=szServiceFunction;
	mi.flags=CMIF_TCHAR;
	Menu_AddMainMenuItem(&mi);

	mi.position=-2000070000;
	mi.hIcon=LoadSkinnedIcon(SKINICON_OTHER_SMS);
	mi.ptszName=SMS_SEND_CM_STR;
	mi.pszService=szServiceFunction;
	mi.flags=CMIF_TCHAR;	
	ssSMSSettings.hContactMenuItems[0]=Menu_AddContactMenuItem(&mi);

	SkinAddNewSoundEx("RecvSMSMsg",PROTOCOL_NAMEA,LPGEN("Incoming SMS Message"));
	SkinAddNewSoundEx("RecvSMSConfirmation",PROTOCOL_NAMEA,LPGEN("Incoming SMS Confirmation"));


	RefreshAccountList(NULL,NULL);

	RestoreUnreadMessageAlerts();

	return 0;
}


void UnloadModules()
{
	// Main menu destroy
	//CListDestroyMenu(gdiMenuItems,SIZEOF(gdiMenuItems));
	//ZeroMemory(masMraSettings.hMainMenuItems,sizeof(masMraSettings.hMainMenuItems));

	// Contact menu destroy
	//CListDestroyMenu(gdiContactMenuItems,(SIZEOF(gdiContactMenuItems) - ((masMraSettings.heNudgeReceived==NULL)? 0:1)));
	//ZeroMemory(masMraSettings.hContactMenuItems,sizeof(masMraSettings.hContactMenuItems));

	if (ssSMSSettings.hHookAccListChanged)		{UnhookEvent(ssSMSSettings.hHookAccListChanged);		ssSMSSettings.hHookAccListChanged=NULL;}
	if (ssSMSSettings.hHookDbAdd)				{UnhookEvent(ssSMSSettings.hHookDbAdd);					ssSMSSettings.hHookDbAdd=NULL;}
	if (ssSMSSettings.hHookProtoAck)			{UnhookEvent(ssSMSSettings.hHookProtoAck);				ssSMSSettings.hHookProtoAck=NULL;}
	if (ssSMSSettings.hHookRebuildCMenu)		{UnhookEvent(ssSMSSettings.hHookRebuildCMenu);			ssSMSSettings.hHookRebuildCMenu=NULL;}
	if (ssSMSSettings.hHookOptInitialize)		{UnhookEvent(ssSMSSettings.hHookOptInitialize);			ssSMSSettings.hHookOptInitialize=NULL;}

	//IconsUnLoad();

}


void UnloadServices()
{
	CHAR szServiceFunction[MAX_PATH],*pszServiceFunctionName;

	CopyMemory(szServiceFunction,PROTOCOL_NAMEA,PROTOCOL_NAME_SIZE);
	pszServiceFunctionName=szServiceFunction+PROTOCOL_NAME_LEN;

	// destroy plugin services
	for (SIZE_T i=0;i<SIZEOF(siPluginServices);i++)
	{
		CopyMemory(pszServiceFunctionName,siPluginServices[i].lpszName,(lstrlenA(siPluginServices[i].lpszName)+1));
		DestroyServiceFunction(szServiceFunction);
	}
}



int SmsRebuildContactMenu(WPARAM wParam,LPARAM lParam)
{
	CListShowMenuItem(ssSMSSettings.hContactMenuItems[0],(BOOL)GetContactPhonesCount((HANDLE)wParam));
	return 0;
}


//This function called when user clicked Menu.
int SendSMSMenuCommand(WPARAM wParam,LPARAM lParam)
{
	HWND hwndSendSms;

	if (wParam)
	{// user clicked on the "SMS Message" on one of the users
		hwndSendSms=SendSMSWindowIsOtherInstanceHContact((HANDLE)wParam);
		if (hwndSendSms)
			SetFocus(hwndSendSms);
		else
			hwndSendSms=SendSMSWindowAdd((HANDLE)wParam);
	}
	else{// user clicked on the "SMS Send" in the Main Menu
		hwndSendSms=SendSMSWindowAdd(NULL);
		EnableWindow(GetDlgItem(hwndSendSms,IDC_NAME),TRUE);
		EnableWindow(GetDlgItem(hwndSendSms,IDC_SAVENUMBER),FALSE);

		for (HANDLE hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
			if (GetContactPhonesCount(hContact)) {
				SEND_DLG_ITEM_MESSAGEW(hwndSendSms,IDC_NAME,CB_ADDSTRING,0,(LPARAM)GetContactNameW(hContact));
				SendSMSWindowSMSContactAdd(hwndSendSms,hContact);
			}
	}
	return 0;
}


//This function used to popup a read SMS window after the user clicked on the received SMS message.
int ReadMsgSMS(WPARAM wParam,LPARAM lParam)
{
	CLISTEVENT *cle = (CLISTEVENT*)lParam;

	DBEVENTINFO dbei = { sizeof(dbei) };
	if ((dbei.cbBlob = db_event_getBlobSize(((CLISTEVENT*)lParam)->hDbEvent)) != -1) {
		dbei.pBlob = (PBYTE)_alloca(dbei.cbBlob);

		if (db_event_get(cle->hDbEvent, &dbei) == 0)
		if (dbei.eventType == ICQEVENTTYPE_SMS || dbei.eventType == ICQEVENTTYPE_SMSCONFIRMATION)
		if (dbei.cbBlob > MIN_SMS_DBEVENT_LEN) {
			if (RecvSMSWindowAdd(cle->hContact,ICQEVENTTYPE_SMS,NULL,0,(LPSTR)dbei.pBlob,dbei.cbBlob)) {
				db_event_markRead(cle->hContact, cle->hDbEvent);
				return 0;
			}
		}
	}
	return 1;
}

//This function used to popup a read SMS window after the user clicked on the received SMS confirmation.
int ReadAckSMS(WPARAM wParam,LPARAM lParam)
{
	CLISTEVENT *cle = (CLISTEVENT*)lParam;

	DBEVENTINFO dbei = { sizeof(dbei) };
	if ((dbei.cbBlob = db_event_getBlobSize(cle->hDbEvent)) != -1) {
		dbei.pBlob = (PBYTE)_alloca(dbei.cbBlob);

		if (db_event_get(cle->hDbEvent, &dbei) == 0)
		if (dbei.eventType == ICQEVENTTYPE_SMS || dbei.eventType == ICQEVENTTYPE_SMSCONFIRMATION)
		if (dbei.cbBlob > MIN_SMS_DBEVENT_LEN) {
			if (RecvSMSWindowAdd(cle->hContact, ICQEVENTTYPE_SMSCONFIRMATION, NULL, 0, (LPSTR)dbei.pBlob, dbei.cbBlob)) {
				db_event_delete(cle->hContact, cle->hDbEvent);
				return 0;
			}
		}
	}
	return 1;
}

void RestoreUnreadMessageAlerts(void)
{
	DBEVENTINFO dbei = { sizeof(dbei) };

	for (HANDLE hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		for (HANDLE hDbEvent = db_event_firstUnread(hContact); hDbEvent; hDbEvent = db_event_next(hDbEvent)) {
			dbei.cbBlob=0;
			if (db_event_get(hDbEvent, &dbei) == 0)
			if ((dbei.flags & (DBEF_SENT|DBEF_READ))==0 && ((dbei.eventType==ICQEVENTTYPE_SMS) || (dbei.eventType==ICQEVENTTYPE_SMSCONFIRMATION)))
			if (dbei.cbBlob>MIN_SMS_DBEVENT_LEN)
				handleNewMessage((WPARAM)hContact,(LPARAM)hDbEvent);
		}

	for (HANDLE hDbEvent = db_event_firstUnread(NULL); hDbEvent; hDbEvent = db_event_next(hDbEvent)) {
		dbei.cbBlob=0;
		if (db_event_get(hDbEvent, &dbei) == 0)
		if ((dbei.flags & (DBEF_SENT|DBEF_READ))==0 && ((dbei.eventType==ICQEVENTTYPE_SMS) || (dbei.eventType==ICQEVENTTYPE_SMSCONFIRMATION)))
		if (dbei.cbBlob > MIN_SMS_DBEVENT_LEN)
			handleNewMessage(NULL, (LPARAM)hDbEvent);
	}
}
















