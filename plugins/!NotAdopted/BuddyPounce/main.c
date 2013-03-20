#include "headers.h"

PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
	"Buddy Pounce",
	PLUGIN_MAKE_VERSION(0,3,2,1),
	"Allows you to send a message to contacts if they change status and your not there to say hi. Bassically offline messaging for protocols that dont have it.",
	"Jonathan Gordon",
	"ICQ 98791178, MSN jonnog@hotmail.com",
	"© 2004 Jonathan Gordon, jdgordy@gmail.com",
	"http://jdgordy.tk",		// www
	0,		//not transient
	0		//doesn't replace anything built-in
};

//========================
//  WINAPI DllMain
//========================

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}
//========================
//  MirandaPluginInfo
//========================


__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}

//===================
// MainInit
//===================

int MainInit(WPARAM wParam,LPARAM lParam)
{
	return 0;
}
HANDLE hWindowList;

int MsgAck(WPARAM wParam,LPARAM lParam) 
{ 
	ACKDATA *ack=(ACKDATA*)lParam; 

    if (ack && ack->cbSize==sizeof(ACKDATA) 
        && ack->type==ACKTYPE_MESSAGE 
        && ack->hProcess==(HANDLE)WindowList_Find(hWindowList,ack->hContact)) 
	{ 
		if (DBGetContactSettingByte(NULL, modname, "ShowDeliveryMessages", 1))
			CreateMessageAcknowlegedWindow(ack->hContact,ack->result == ACKRESULT_SUCCESS);
		if (ack->result == ACKRESULT_SUCCESS)
		{
			// wrtie it to the DB
			DBEVENTINFO dbei = { 0 };
			DBVARIANT dbv;
			int reuse = DBGetContactSettingByte(ack->hContact,modname, "Reuse", 0);
			if (!DBGetContactSetting(ack->hContact, modname, "PounceMsg", &dbv) && (dbv.pszVal[0] != '\0'))
			{

				dbei.cbSize = sizeof(dbei);
				dbei.eventType = EVENTTYPE_MESSAGE;
				dbei.flags = DBEF_SENT;
				dbei.szModule = ack->szModule;
				dbei.timestamp = time(NULL);
				dbei.cbBlob = lstrlenA(dbv.pszVal) + 1;
				dbei.pBlob = (PBYTE) dbv.pszVal;
				CallService(MS_DB_EVENT_ADD, (WPARAM) ack->hContact, (LPARAM) & dbei);
			}
			// check to reuse
			if (reuse >1)
				DBWriteContactSettingByte(ack->hContact,modname, "Reuse", (BYTE)(reuse-1));
			else 
			{
				DBWriteContactSettingByte(ack->hContact,modname, "Reuse", 0);
				DBWriteContactSettingString(ack->hContact,modname, "PounceMsg","");
			}
		}
		WindowList_Remove(hWindowList,(HWND)ack->hProcess);
   } 
   return 0; 
} 

int BuddyPounceOptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize=sizeof(odp);
	odp.position=0;
	odp.hInstance=hInst;
	odp.pszTemplate=MAKEINTRESOURCE(IDD_OPTIONS);
	odp.pszGroup= "Plugins";
	odp.pszTitle="Buddy Pounce";
	odp.pfnDlgProc=BuddyPounceOptionsDlgProc;
	odp.expertOnlyControls=NULL;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	
	return 0;
}

int statusCheck(int statusFlag, int status)
{
	if (statusFlag == ANY || !statusFlag) return 1;
	switch(status)
	{
		case ID_STATUS_OFFLINE:
			return 0;
		case ID_STATUS_ONLINE:
			return (statusFlag & ONLINE);
		case ID_STATUS_AWAY:
			return (statusFlag & AWAY);
		case ID_STATUS_NA:
			return (statusFlag & NA);
		case ID_STATUS_OCCUPIED:
			return (statusFlag & OCCUPIED);
		case ID_STATUS_DND:
			return (statusFlag & DND);
		case ID_STATUS_FREECHAT:
			return (statusFlag & FFC);
		case ID_STATUS_INVISIBLE:
			return (statusFlag & INVISIBLE);
		case ID_STATUS_OUTTOLUNCH:
			return (statusFlag & LUNCH);
		case ID_STATUS_ONTHEPHONE:
			return (statusFlag & PHONE);
	}
	return 0;
}
int CheckDate(HANDLE hContact)
{	
	time_t curtime = time (NULL);
	if(!DBGetContactSettingByte(hContact,modname,"GiveUpDays",0))
		return 1;
	if(DBGetContactSettingByte(hContact,modname,"GiveUpDays",0) && ( abs(DBGetContactSettingDword(hContact,modname,"GiveUpDate",0)) > curtime))
		return 1;
	return 0;
}

void SendPounce(char* text, HANDLE hContact)
{
	HANDLE hSendId;
	if (hSendId = (HANDLE)CallContactService(hContact, PSS_MESSAGE, 0, (LPARAM)text)) 
		WindowList_Add(hWindowList,(HWND)hSendId,hContact);
}

int UserOnlineSettingChanged(WPARAM wParam,LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws=(DBCONTACTWRITESETTING*)lParam;
	int newStatus,oldStatus;
	DBVARIANT dbv;
	HANDLE hContact;
	char* szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)wParam,0);
	char* message;
	if((HANDLE)wParam==NULL || strcmp(cws->szSetting,"Status")) return 0;
	if (szProto && (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IM))
	{
		newStatus=cws->value.wVal;
		oldStatus=DBGetContactSettingWord((HANDLE)wParam,"UserOnline","OldStatus",ID_STATUS_OFFLINE);
		
		if ( ( newStatus != oldStatus ) && ( (HANDLE)wParam != NULL) && ( newStatus != ID_STATUS_OFFLINE)  ) 
		{
			hContact=(HANDLE)wParam;
			if (!DBGetContactSetting(hContact, modname, "PounceMsg", &dbv) && (dbv.pszVal[0] != '\0'))
			{
				// check my status
				if (statusCheck(DBGetContactSettingWord(hContact, modname, "SendIfMyStatusIsFLAG", 0), CallProtoService(szProto, PS_GETSTATUS,0,0)) 
				// check the contacts status
				&& statusCheck(DBGetContactSettingWord(hContact, modname, "SendIfTheirStatusIsFLAG", 0), newStatus) )
				{
					// check if we r giving up after x days
					if (CheckDate(hContact)) 
					{
						
						if (DBGetContactSettingByte(hContact, modname, "ConfirmTimeout", 0))
						{
							struct SendPounceDlgProcStruct *spdps = (struct SendPounceDlgProcStruct *)malloc(sizeof(struct SendPounceDlgProcStruct));
							message = strdup(dbv.pszVal); // will get free()ed in the send confirm window proc
							spdps->hContact = hContact;
							spdps->message = message;
							CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_CONFIRMSEND),0,SendPounceDlgProc, (LPARAM)spdps);
							// set the confirmation window to send the msg when the timeout is done
						}
						else SendPounce(dbv.pszVal, hContact);
					}
				}
			}
			
		}
	}
	DBFreeVariant(&dbv);
	return 0;
}
HANDLE hHookSettingChanged = NULL;
HANDLE hHookoptsinit = NULL;
HANDLE hHookAck = NULL;

int BuddyPounceMenuCommand(WPARAM wParam,LPARAM lParam)
{
	if (DBGetContactSettingByte(NULL, modname, "UseAdvanced", 0) || DBGetContactSettingByte((HANDLE)wParam, modname, "UseAdvanced", 0))
		CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_POUNCE),0,BuddyPounceDlgProc, wParam);
	else CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_POUNCE_SIMPLE),0,BuddyPounceSimpleDlgProc, wParam);
	return 0;
}

int AddSimpleMessage(WPARAM wParam,LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	char* message = (char*)lParam;
	time_t today = time(NULL);
	DBWriteContactSettingString(hContact, modname, "PounceMsg", message);
	DBWriteContactSettingWord(hContact, modname, "SendIfMyStatusIsFLAG", (WORD)DBGetContactSettingWord(NULL, modname, "SendIfMyStatusIsFLAG",1));
	DBWriteContactSettingWord(hContact, modname, "SendIfTheirStatusIsFLAG", (WORD)DBGetContactSettingWord(NULL, modname, "SendIfTheirStatusIsFLAG",1));
	DBWriteContactSettingByte(hContact, modname, "Reuse", (BYTE)DBGetContactSettingByte(NULL, modname, "Reuse",0));
	DBWriteContactSettingByte(hContact, modname, "GiveUpDays", (BYTE)DBGetContactSettingByte(NULL, modname, "GiveUpDays",0));
	DBWriteContactSettingDword(hContact, modname, "GiveUpDate", (DWORD)(DBGetContactSettingByte(hContact, modname, "GiveUpDays",0)*SECONDSINADAY));
	return 0;
}

int AddToPounce(WPARAM wParam,LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	char* message = (char*)lParam;
	DBVARIANT dbv;
	if (!DBGetContactSetting(hContact, modname, "PounceMsg",&dbv))
	{
		char* newPounce = (char*)malloc(strlen(dbv.pszVal) + strlen(message) + 1);
		if (!newPounce) return 1;
		strcpy(newPounce, dbv.pszVal);
		strcat(newPounce, message);
		DBWriteContactSettingString(hContact, modname, "PounceMsg", newPounce);
		free(newPounce);
		DBFreeVariant(&dbv);
	}
	else AddSimpleMessage((WPARAM)hContact, (LPARAM)message);
	return 0;
}

//===========================
// Load (hook ModulesLoaded)
//===========================
int __declspec(dllexport) Load(PLUGINLINK *link)
{ 	
	CLISTMENUITEM mi;
	pluginLink = link; 
	hHookSettingChanged =HookEvent(ME_DB_CONTACT_SETTINGCHANGED,UserOnlineSettingChanged); 
	hHookoptsinit = HookEvent(ME_OPT_INITIALISE,BuddyPounceOptInit);
	hHookAck = HookEvent(ME_PROTO_ACK,MsgAck);
	CreateServiceFunction("BuddyPounce/MenuCommand",BuddyPounceMenuCommand);
	hWindowList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);

	/*     service funcitons for other devs...					*/
	CreateServiceFunction("BuddyPounce/AddSimplePounce",AddSimpleMessage); // add a simple pounce to a contact
	CreateServiceFunction("BuddyPounce/AddToPounce",AddToPounce); // add to the exsisitng pounce, if there isnt 1 then add a new simple pounce.
	/* ******************************************************** */

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=10;
	mi.flags=0;
	mi.hIcon= LoadIcon(hInst,MAKEINTRESOURCE(IDI_POUNCE));
	mi.pszName="&Buddy Pounce";
	mi.pszService="BuddyPounce/MenuCommand";
	mi.pszContactOwner=NULL;

	CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	{	// known modules list
		DBVARIANT dbv;
		if (DBGetContactSetting(NULL,"KnownModules","Buddy Pounce", &dbv))
			DBWriteContactSettingString(NULL,"KnownModules","Buddy Pounce",modname);
		DBFreeVariant(&dbv);
	}
	return 0; 
}


int __declspec(dllexport) Unload(void) 
{ 
	if (hHookSettingChanged) 
		UnhookEvent(hHookSettingChanged); 
	return 0;
} 

//uninstall support
int __declspec(dllexport) UninstallEx(PLUGINUNINSTALLPARAMS* ppup) 
{ 
    // Delete Files 
    const char* apszFiles[] = {"buddypounce_readme.txt", 0}; 
    PUIRemoveFilesInDirectory(ppup->pszPluginsPath, apszFiles); 
	
	  if((ppup->bDoDeleteSettings == TRUE) && (ppup->bIsMirandaRunning == TRUE)) 
		{
			PUICallService(MS_PLUGINUNINSTALLER_REMOVEDBMODULE, (WPARAM)modname, (LPARAM)NULL);  
		}
	return 0;
}