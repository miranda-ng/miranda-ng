#include "main.h"



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
return(0);
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
	mi.flags=(CMIF_UNICODE);
	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);

	mi.position=-2000070000;
	mi.hIcon=LoadSkinnedIcon(SKINICON_OTHER_SMS);
	mi.ptszName=SMS_SEND_CM_STR;
	mi.pszService=szServiceFunction;
	mi.flags=(CMIF_UNICODE);
	ssSMSSettings.hContactMenuItems[0]=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);



	SkinAddNewSoundEx("RecvSMSMsg",PROTOCOL_NAMEA,Translate("Incoming SMS Message"));
	SkinAddNewSoundEx("RecvSMSConfirmation",PROTOCOL_NAMEA,Translate("Incoming SMS Confirmation"));


	RefreshAccountList(NULL,NULL);

	RestoreUnreadMessageAlerts();

return(0);
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
return(0);
}


//This function called when user clicked Menu.
int SendSMSMenuCommand(WPARAM wParam,LPARAM lParam)
{
	HWND hwndSendSms;

	if (wParam)
	{// user clicked on the "SMS Message" on one of the users
		hwndSendSms=SendSMSWindowIsOtherInstanceHContact((HANDLE)wParam);
		if (hwndSendSms)
		{
			SetFocus(hwndSendSms);
		}else{
			hwndSendSms=SendSMSWindowAdd((HANDLE)wParam);
		}
	}else{// user clicked on the "SMS Send" in the Main Menu
		hwndSendSms=SendSMSWindowAdd(NULL);
		EnableWindow(GetDlgItem(hwndSendSms,IDC_NAME),TRUE);
		EnableWindow(GetDlgItem(hwndSendSms,IDC_SAVENUMBER),FALSE);

		for(HANDLE hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);hContact!=NULL;hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
		{
			if (GetContactPhonesCount(hContact))
			{
				SEND_DLG_ITEM_MESSAGEW(hwndSendSms,IDC_NAME,CB_ADDSTRING,0,(LPARAM)GetContactNameW(hContact));
				SendSMSWindowSMSContactAdd(hwndSendSms,hContact);
			}
		}
	}
return(0);
}


//This function used to popup a read SMS window after the user clicked on the received SMS message.
int ReadMsgSMS(WPARAM wParam,LPARAM lParam)
{
	int iRet=1;
	DBEVENTINFO dbei={0};

	dbei.cbSize=sizeof(dbei);
	if ((dbei.cbBlob=CallService(MS_DB_EVENT_GETBLOBSIZE,(WPARAM)((CLISTEVENT*)lParam)->hDbEvent,0))!=-1)
	{
		dbei.pBlob=(PBYTE)MEMALLOC(dbei.cbBlob);
		if (dbei.pBlob)
		{
			if (CallService(MS_DB_EVENT_GET,(WPARAM)((CLISTEVENT*)lParam)->hDbEvent,(LPARAM)&dbei)==0)
			if (dbei.eventType==ICQEVENTTYPE_SMS || dbei.eventType==ICQEVENTTYPE_SMSCONFIRMATION)
			if (dbei.cbBlob>MIN_SMS_DBEVENT_LEN)
			{
				if (RecvSMSWindowAdd(((CLISTEVENT*)lParam)->hContact,ICQEVENTTYPE_SMS,NULL,0,(LPSTR)dbei.pBlob,dbei.cbBlob))
				{
					CallService(MS_DB_EVENT_MARKREAD,(WPARAM)((CLISTEVENT*)lParam)->hContact,(LPARAM)((CLISTEVENT*)lParam)->hDbEvent);
					iRet=0;
				}
			}
			MEMFREE(dbei.pBlob);
		}
	}
return(iRet);
}

//This function used to popup a read SMS window after the user clicked on the received SMS confirmation.
int ReadAckSMS(WPARAM wParam,LPARAM lParam)
{
	int iRet=1;
	DBEVENTINFO dbei={0};

	dbei.cbSize=sizeof(dbei);
	if ((dbei.cbBlob=CallService(MS_DB_EVENT_GETBLOBSIZE,(WPARAM)((CLISTEVENT*)lParam)->hDbEvent,0))!=-1)
	{
		dbei.pBlob=(PBYTE)MEMALLOC(dbei.cbBlob);
		if (dbei.pBlob)
		{
			if (CallService(MS_DB_EVENT_GET,(WPARAM)((CLISTEVENT*)lParam)->hDbEvent,(LPARAM)&dbei)==0)
			if (dbei.eventType==ICQEVENTTYPE_SMS || dbei.eventType==ICQEVENTTYPE_SMSCONFIRMATION)
			if (dbei.cbBlob>MIN_SMS_DBEVENT_LEN)
			{
				if (RecvSMSWindowAdd(((CLISTEVENT*)lParam)->hContact,ICQEVENTTYPE_SMSCONFIRMATION,NULL,0,(LPSTR)dbei.pBlob,dbei.cbBlob))
				{
					CallService(MS_DB_EVENT_DELETE,(WPARAM)((CLISTEVENT*)lParam)->hContact,(LPARAM)((CLISTEVENT*)lParam)->hDbEvent);
					iRet=0;
				}
			}
			MEMFREE(dbei.pBlob);
		}
	}
return(iRet);
}

void RestoreUnreadMessageAlerts(void)
{
	DBEVENTINFO dbei={0};
	HANDLE hDbEvent,hContact;

	dbei.cbSize=sizeof(dbei);
	for(hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);hContact!=NULL;hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
	for(hDbEvent=(HANDLE)CallService(MS_DB_EVENT_FINDFIRSTUNREAD,(WPARAM)hContact,0);hDbEvent!=NULL;hDbEvent=(HANDLE)CallService(MS_DB_EVENT_FINDNEXT,(WPARAM)hDbEvent,0))
	{
		dbei.cbBlob=0;
		if (CallService(MS_DB_EVENT_GET,(WPARAM)hDbEvent,(LPARAM)&dbei)==0)
		if ((dbei.flags&(DBEF_SENT|DBEF_READ))==0 && ((dbei.eventType==ICQEVENTTYPE_SMS) || (dbei.eventType==ICQEVENTTYPE_SMSCONFIRMATION)))
		if (dbei.cbBlob>MIN_SMS_DBEVENT_LEN)
		{
			handleNewMessage((WPARAM)hContact,(LPARAM)hDbEvent);
		}
	}

	hContact=NULL;
	for(hDbEvent=(HANDLE)CallService(MS_DB_EVENT_FINDFIRSTUNREAD,(WPARAM)hContact,0);hDbEvent!=NULL;hDbEvent=(HANDLE)CallService(MS_DB_EVENT_FINDNEXT,(WPARAM)hDbEvent,0))
	{
		dbei.cbBlob=0;
		if (CallService(MS_DB_EVENT_GET,(WPARAM)hDbEvent,(LPARAM)&dbei)==0)
		if ((dbei.flags&(DBEF_SENT|DBEF_READ))==0 && ((dbei.eventType==ICQEVENTTYPE_SMS) || (dbei.eventType==ICQEVENTTYPE_SMSCONFIRMATION)))
		if (dbei.cbBlob>MIN_SMS_DBEVENT_LEN)
		{
			handleNewMessage((WPARAM)hContact,(LPARAM)hDbEvent);
		}
	}
}
















