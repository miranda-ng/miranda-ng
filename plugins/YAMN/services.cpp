
#include "main.h"
#include "yamn.h"
// External icon var for icolib support


//MessageWndCS
//We want to send messages to all windows in the queue
//When we send messages, no other window can register itself to the queue for receiving messages
extern LPCRITICAL_SECTION MessageWndCS;

//Plugin registration CS
//Used if we add (register) plugin to YAMN plugins and when we browse through registered plugins
extern LPCRITICAL_SECTION PluginRegCS;

//AccountWriterCS
//We want to store number of writers of Accounts (number of Accounts used for writing)
//If we want to read all accounts (for saving to file) immidiatelly, we have to wait until no account is changing (no thread writing to account)
extern SCOUNTER *AccountWriterSO;

//NoExitEV
//Event that is signaled when there's a request to exit, so no new pop3 check should be performed
extern HANDLE ExitEV;

//WriteToFileEV
//If this is signaled, write accounts to file is performed. Set this event if you want to actualize your accounts and messages
extern HANDLE WriteToFileEV;

//extern HICON hYamnIconsOrg[];
extern HICON hYamnIcons[];
extern char *iconDescs[];
extern char *iconNames[];
extern HIMAGELIST CSImages;

extern void __stdcall	SSL_DebugLog( const char *fmt, ... );

extern char *ProtoName;
extern INT_PTR YAMN_STATUS;

extern PYAMN_VARIABLES pYAMNVar;
extern HYAMNPROTOPLUGIN POP3Plugin;

static INT_PTR Service_GetCaps(WPARAM wParam, LPARAM lParam)
{
	if(wParam==PFLAGNUM_4)
		return PF4_NOCUSTOMAUTH;
	if(wParam==PFLAG_UNIQUEIDTEXT)
        return (INT_PTR) Translate("Nick");
	if(wParam==PFLAG_MAXLENOFMESSAGE)
        return 400;
	if(wParam==PFLAG_UNIQUEIDSETTING)
        return (INT_PTR) "Id";
	if(wParam==PFLAGNUM_2)
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND;
	if(wParam==PFLAGNUM_5)
		if(DBGetContactSettingByte(NULL, YAMN_DBMODULE, YAMN_SHOWASPROTO, 1))
			return PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND;
		else
			return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND;
	return 0;
}

static INT_PTR Service_GetStatus(WPARAM wParam, LPARAM lParam)
{
	return YAMN_STATUS;	
}

static INT_PTR Service_SetStatus(WPARAM wParam,LPARAM lParam)
{	
	INT_PTR newstatus = (wParam!=ID_STATUS_OFFLINE)?ID_STATUS_ONLINE:ID_STATUS_OFFLINE;
	if (newstatus != YAMN_STATUS){
		INT_PTR oldstatus = YAMN_STATUS;
		YAMN_STATUS=newstatus;
		ProtoBroadcastAck(ProtoName,NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS,(HANDLE)oldstatus,newstatus);
	}
	return 0;

}

static INT_PTR Service_GetName(WPARAM wParam, LPARAM lParam)
{
	lstrcpyn((char *) lParam, ProtoName, wParam);;
	return 0;
}

static INT_PTR Service_LoadIcon(WPARAM wParam,LPARAM lParam)
{
	if ( LOWORD( wParam ) == PLI_PROTOCOL )
		return (INT_PTR)CopyIcon(hYamnIcons[0]); // noone cares about other than PLI_PROTOCOL

	return (INT_PTR)(HICON)NULL;

}
 
/*static*/ INT_PTR ClistContactDoubleclicked(WPARAM wParam, LPARAM lParam)
{
	ContactDoubleclicked(((CLISTEVENT*)lParam)->lParam, lParam);
	return 0;
}

static int Service_ContactDoubleclicked(WPARAM wParam, LPARAM lParam)
{
	ContactDoubleclicked(wParam, lParam);
	return 0;
}

static INT_PTR ContactApplication(WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;
	char *szProto;
	HACCOUNT ActualAccount;

	szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
	if(szProto != NULL && strcmp(szProto, ProtoName)==0)
	{
		if(!DBGetContactSetting((HANDLE) wParam,ProtoName,"Id",&dbv)) 
		{
			ActualAccount=(HACCOUNT) CallService(MS_YAMN_FINDACCOUNTBYNAME,(WPARAM)POP3Plugin,(LPARAM)dbv.pszVal);
			if(ActualAccount != NULL)
			{
				PROCESS_INFORMATION pi;
				STARTUPINFOW si;

				ZeroMemory(&si,sizeof(si));
				si.cb=sizeof(si);
				
				#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile,"ContactApplication:ActualAccountSO-read wait\n");
				#endif
				if(WAIT_OBJECT_0==WaitToReadFcn(ActualAccount->AccountAccessSO))
				{
					#ifdef DEBUG_SYNCHRO
					DebugLog(SynchroFile,"ContactApplication:ualAccountSO-read enter\n");
					#endif
					if(ActualAccount->NewMailN.App!=NULL)
					{
						WCHAR *Command;
						if(ActualAccount->NewMailN.AppParam!=NULL)
							Command=new WCHAR[wcslen(ActualAccount->NewMailN.App)+wcslen(ActualAccount->NewMailN.AppParam)+6];
						else
							Command=new WCHAR[wcslen(ActualAccount->NewMailN.App)+6];
					
						if(Command!=NULL)
						{
							lstrcpyW(Command,L"\"");
							lstrcatW(Command,ActualAccount->NewMailN.App);
							lstrcatW(Command,L"\" ");
							if(ActualAccount->NewMailN.AppParam!=NULL)
								lstrcatW(Command,ActualAccount->NewMailN.AppParam);
							CreateProcessW(NULL,Command,NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS,NULL,NULL,&si,&pi);
							delete[] Command;
						}
					}

					#ifdef DEBUG_SYNCHRO
					DebugLog(SynchroFile,"ContactApplication:ActualAccountSO-read done\n");
					#endif
					ReadDoneFcn(ActualAccount->AccountAccessSO);
				}
				#ifdef DEBUG_SYNCHRO
				else
					DebugLog(SynchroFile,"ContactApplication:ActualAccountSO-read enter failed\n");
				#endif
			}
			DBFreeVariant(&dbv);
		}
	}
	return 0;
}

DWORD WINAPI SWMRGWaitToRead(PSWMRG pSWMRG, DWORD dwTimeout);
static INT_PTR AccountMailCheck(WPARAM wParam, LPARAM lParam){
	//This service will check/sincronize the account pointed by wParam
	HACCOUNT ActualAccount = (HACCOUNT)wParam;
	HANDLE ThreadRunningEV;
	DWORD tid;
	// copy/paste make mistakes
	if(ActualAccount != NULL)
	{
		//we use event to signal, that running thread has all needed stack parameters copied
		if(NULL==(ThreadRunningEV=CreateEvent(NULL,FALSE,FALSE,NULL)))
			return 0;
		//if we want to close miranda, we get event and do not run pop3 checking anymore
		if(WAIT_OBJECT_0==WaitForSingleObject(ExitEV,0))
			return 0;
		EnterCriticalSection(PluginRegCS);
		#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"AccountCheck:ActualAccountSO-read wait\n");
		#endif
		if(WAIT_OBJECT_0!=SWMRGWaitToRead(ActualAccount->AccountAccessSO,0))
		{
			#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"ForceCheck:ActualAccountSO-read wait failed\n");
			#endif
		}
		else
		{
			#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"ForceCheck:ActualAccountSO-read enter\n");
			#endif
			if((ActualAccount->Flags & YAMN_ACC_ENA) && ActualAccount->Plugin->Fcn->SynchroFcnPtr)
			{
				struct CheckParam ParamToPlugin={YAMN_CHECKVERSION,ThreadRunningEV,ActualAccount,lParam?YAMN_FORCECHECK:YAMN_NORMALCHECK,(void *)0,NULL};
				HANDLE NewThread;

				ActualAccount->TimeLeft=ActualAccount->Interval;
				if(NewThread=CreateThread(NULL,0,(YAMN_STANDARDFCN)ActualAccount->Plugin->Fcn->SynchroFcnPtr,&ParamToPlugin,0,&tid))
				{
					WaitForSingleObject(ThreadRunningEV,INFINITE);
					CloseHandle(NewThread);
				}
				else
				{
					//ReadDoneFcn(ActualAccount->AccountAccessSO);
				}

			}
			ReadDoneFcn(ActualAccount->AccountAccessSO);
		}
		LeaveCriticalSection(PluginRegCS);
		CloseHandle(ThreadRunningEV);
	}
	return 0;
}

static INT_PTR ContactMailCheck(WPARAM wParam, LPARAM lParam)
{

	DBVARIANT dbv;
	char *szProto;
	HACCOUNT ActualAccount;
	HANDLE ThreadRunningEV;
	DWORD tid;

	szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
	if(szProto != NULL && strcmp(szProto, ProtoName)==0)
	{
		if(!DBGetContactSetting((HANDLE) wParam,ProtoName,"Id",&dbv)) 
		{
			ActualAccount=(HACCOUNT) CallService(MS_YAMN_FINDACCOUNTBYNAME,(WPARAM)POP3Plugin,(LPARAM)dbv.pszVal);
			if(ActualAccount != NULL)
			{
				//we use event to signal, that running thread has all needed stack parameters copied
				if(NULL==(ThreadRunningEV=CreateEvent(NULL,FALSE,FALSE,NULL)))
					return 0;
				//if we want to close miranda, we get event and do not run pop3 checking anymore
				if(WAIT_OBJECT_0==WaitForSingleObject(ExitEV,0))
					return 0;
				EnterCriticalSection(PluginRegCS);
				#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile,"ForceCheck:ActualAccountSO-read wait\n");
				#endif
				if(WAIT_OBJECT_0!=WaitToReadFcn(ActualAccount->AccountAccessSO))
				{
					#ifdef DEBUG_SYNCHRO
					DebugLog(SynchroFile,"ForceCheck:ActualAccountSO-read wait failed\n");
					#endif
				}
				else
				{
					#ifdef DEBUG_SYNCHRO
					DebugLog(SynchroFile,"ForceCheck:ActualAccountSO-read enter\n");
					#endif
					if((ActualAccount->Flags & YAMN_ACC_ENA) && (ActualAccount->StatusFlags & YAMN_ACC_FORCE))			//account cannot be forced to check
					{
						if(ActualAccount->Plugin->Fcn->ForceCheckFcnPtr==NULL)
						{
							ReadDoneFcn(ActualAccount->AccountAccessSO);
						}
						struct CheckParam ParamToPlugin={YAMN_CHECKVERSION,ThreadRunningEV,ActualAccount,YAMN_FORCECHECK,(void *)0,NULL};

						if(NULL==CreateThread(NULL,0,(YAMN_STANDARDFCN)ActualAccount->Plugin->Fcn->ForceCheckFcnPtr,&ParamToPlugin,0,&tid))
						{
							ReadDoneFcn(ActualAccount->AccountAccessSO);
						}
						else
							WaitForSingleObject(ThreadRunningEV,INFINITE);
					}
					ReadDoneFcn(ActualAccount->AccountAccessSO);
				}
				LeaveCriticalSection(PluginRegCS);
				CloseHandle(ThreadRunningEV);
			}
			DBFreeVariant(&dbv);
		}

	}
	return 0;
}


void MainMenuAccountClicked(WPARAM wParam, LPARAM lParam) 
{
	
}

/*static*/ void ContactDoubleclicked(WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;
	char *szProto;
	HACCOUNT ActualAccount;

	szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
	if(szProto != NULL && strcmp(szProto, ProtoName)==0)
	{
		if(!DBGetContactSetting((HANDLE) wParam,ProtoName,"Id",&dbv)) 
		{
			ActualAccount=(HACCOUNT) CallService(MS_YAMN_FINDACCOUNTBYNAME,(WPARAM)POP3Plugin,(LPARAM)dbv.pszVal);
			if(ActualAccount != NULL)
			{
				#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile,"Service_ContactDoubleclicked:ActualAccountSO-read wait\n");
				#endif
				if(WAIT_OBJECT_0==WaitToReadFcn(ActualAccount->AccountAccessSO))
				{
					#ifdef DEBUG_SYNCHRO
					DebugLog(SynchroFile,"Service_ContactDoubleclicked:ActualAccountSO-read enter\n");
					#endif
					YAMN_MAILBROWSERPARAM Param={(HANDLE)0,ActualAccount,ActualAccount->NewMailN.Flags,ActualAccount->NoNewMailN.Flags,0};

					Param.nnflags=Param.nnflags | YAMN_ACC_MSG;			//show mails in account even no new mail in account
					Param.nnflags=Param.nnflags & ~YAMN_ACC_POP;

					Param.nflags=Param.nflags | YAMN_ACC_MSG;			//show mails in account even no new mail in account
					Param.nflags=Param.nflags & ~YAMN_ACC_POP;

					RunMailBrowserSvc((WPARAM)&Param,(LPARAM)YAMN_MAILBROWSERVERSION);
					
					#ifdef DEBUG_SYNCHRO
					DebugLog(SynchroFile,"Service_ContactDoubleclicked:ActualAccountSO-read done\n");
					#endif
					ReadDoneFcn(ActualAccount->AccountAccessSO);
				}
				#ifdef DEBUG_SYNCHRO
				else
					DebugLog(SynchroFile,"Service_ContactDoubleclicked:ActualAccountSO-read enter failed\n");
				#endif
				
			}
			DBFreeVariant(&dbv);
		}

	}
}

int IcoLibIconsChanged(WPARAM wParam, LPARAM lParam)
{
	HICON temp;
	for (int i=0;i<ICONSNUMBER;i++){
		if (temp = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) iconNames[i]))hYamnIcons[i]=temp; 
	}
	{	CLISTMENUITEM mi = {0};
		extern HANDLE hMenuItemMain;
		extern HANDLE hMenuItemCont;
		extern HANDLE hMenuItemContApp;

		mi.cbSize = sizeof(mi);
		mi.flags = CMIM_ICON;

		mi.hIcon = hYamnIcons[5];
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItemMain, (LPARAM)&mi);
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItemCont, (LPARAM)&mi);
		mi.hIcon = hYamnIcons[4];
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItemContApp, (LPARAM)&mi);
	}
	return 0;
}

int SystemModulesLoaded(WPARAM,LPARAM); //in main.cpp
typedef struct { HANDLE hookHandle;	const char *hookName; MIRANDAHOOK mirandaFunction;} HookDataType;
static HookDataType hookData[]={
	{0,ME_TTB_MODULELOADED,AddTopToolbarIcon},
	{0,ME_SYSTEM_MODULESLOADED,SystemModulesLoaded}, //pop3 plugin must be included after all miranda modules are loaded
	{0,ME_OPT_INITIALISE,YAMNOptInitSvc},
	{0,ME_PLUGINUNINSTALLER_UNINSTALL,UninstallQuestionSvc},
	{0,ME_SYSTEM_PRESHUTDOWN,Shutdown},
	{0,ME_CLIST_DOUBLECLICKED, Service_ContactDoubleclicked},
    {0,ME_SKIN2_ICONSCHANGED, IcoLibIconsChanged},
	{0,0,0}//end marker
};


void HookEvents(void)
{
	//We set function which registers needed POP3 accounts. This is a part of internal POP3 plugin.
	//Your plugin should do the same task in your Load fcn. Why we call it in MODULESLOADED? Because netlib
	//user can be registered after all modules are loaded (see m_netlib.h in Miranda)
	for (int i=0;hookData[i].hookName;i++){
		hookData[i].hookHandle = HookEvent(hookData[i].hookName,hookData[i].mirandaFunction);
	}
}
void UnhookEvents(void){
	for (int i=0;i<(sizeof(hookData)/sizeof(hookData[0]));i++){
		if (hookData[i].hookHandle) UnhookEvent(hookData[i].hookHandle);
	}
}

typedef struct { HANDLE serviceHandle;	const char *serviceName; MIRANDASERVICE serviceFunction;} ServiceDataType;
static ServiceDataType serviceData[]={
	{0,YAMN_DBMODULE PS_GETCAPS,Service_GetCaps},
	{0,YAMN_DBMODULE PS_GETSTATUS,Service_GetStatus},
	{0,YAMN_DBMODULE PS_SETSTATUS,Service_SetStatus},
	{0,YAMN_DBMODULE PS_GETNAME,Service_GetName},
	{0,YAMN_DBMODULE PS_LOADICON,Service_LoadIcon},

	//Function with which protocol plugin can register
	{0,MS_YAMN_GETFCNPTR,GetFcnPtrSvc},
	
	//Function returns pointer to YAMN variables
	{0,MS_YAMN_GETVARIABLES,GetVariablesSvc},
	
	//Function with which protocol plugin can register
	{0,MS_YAMN_REGISTERPROTOPLUGIN,RegisterProtocolPluginSvc},
	
	//Function with which protocol plugin can unregister
	{0,MS_YAMN_UNREGISTERPROTOPLUGIN,UnregisterProtocolPluginSvc},
	
	//Function creates an account for plugin
	{0,MS_YAMN_CREATEPLUGINACCOUNT,CreatePluginAccountSvc},
	
	//Function deletes plugin account 
	{0,MS_YAMN_DELETEPLUGINACCOUNT,DeletePluginAccountSvc},
	
	//Finds account for plugin by name
	{0,MS_YAMN_FINDACCOUNTBYNAME,FindAccountByNameSvc},
	
	//Creates next account for plugin
	{0,MS_YAMN_GETNEXTFREEACCOUNT,GetNextFreeAccountSvc},
	
	//Function removes account from YAMN queue. Does not delete it from memory
	{0,MS_YAMN_DELETEACCOUNT,DeleteAccountSvc},
	
	//Function finds accounts for specified plugin
	{0,MS_YAMN_READACCOUNTSA,AddAccountsFromFileASvc},
	
	//Function that reads all plugin mails from file
	{0,MS_YAMN_READACCOUNTSW,AddAccountsFromFileWSvc},
	
	//Function that stores all plugin mails to one file 
	{0,MS_YAMN_WRITEACCOUNTSA,WriteAccountsToFileASvc},
	
	//Function that stores all plugin mails to one file 
	{0,MS_YAMN_WRITEACCOUNTSW,WriteAccountsToFileWSvc},
	
	//Function that returns user's filename
	{0,MS_YAMN_GETFILENAMEA,GetFileNameASvc},
	
	//Function that returns user's filename (unicode input)
	{0,MS_YAMN_GETFILENAMEW,GetFileNameWSvc},
	
	//Releases unicode string from memory
	{0,MS_YAMN_DELETEFILENAME,DeleteFileNameSvc},
	
	//Checks mail
	{0,MS_YAMN_FORCECHECK,ForceCheckSvc},
	
	//Runs YAMN's mail browser
	{0,MS_YAMN_MAILBROWSER,RunMailBrowserSvc},
	
	//Runs YAMN's bad conenction window
	{0,MS_YAMN_BADCONNECTION,RunBadConnectionSvc},
	
	//Function creates new mail for plugin
	{0,MS_YAMN_CREATEACCOUNTMAIL,CreateAccountMailSvc},
	
	//Function deletes plugin account 
	{0,MS_YAMN_DELETEACCOUNTMAIL,DeleteAccountMailSvc},
	
	//Function with which filter plugin can register
	{0,MS_YAMN_REGISTERFILTERPLUGIN,RegisterFilterPluginSvc},
	
	//Function with which filter plugin can unregister
	{0,MS_YAMN_UNREGISTERFILTERPLUGIN,UnregisterFilterPluginSvc},
	
	//Function filters mail
	{0,MS_YAMN_FILTERMAIL,FilterMailSvc},

	//Function contact list double click
	{0,MS_YAMN_CLISTDBLCLICK,ClistContactDoubleclicked},

	//Function to check individual account
	{0,MS_YAMN_ACCOUNTCHECK,AccountMailCheck},

	//Function contact list context menu click
	{0,MS_YAMN_CLISTCONTEXT,ContactMailCheck},

	//Function contact list context menu click
	{0,MS_YAMN_CLISTCONTEXTAPP,ContactApplication},

	{0,0,0}//end marker
};

void CreateServiceFunctions(void)
{
	for (int i=0;serviceData[i].serviceName;i++){
		serviceData[i].serviceHandle = CreateServiceFunction(serviceData[i].serviceName,serviceData[i].serviceFunction);
	}
};

void DestroyServiceFunctions(void)
{
	for (int i=0;serviceData[i].serviceName;i++){
		if (serviceData[i].serviceHandle) DestroyServiceFunction(serviceData[i].serviceHandle);
	}
};

//Function to put all enabled contact to the Online status
void RefreshContact(void)
{
	HACCOUNT Finder;

	for(Finder=POP3Plugin->FirstAccount;Finder!=NULL;Finder=Finder->Next)
	{
		if(Finder->hContact != NULL)
		{
			if((Finder->Flags & YAMN_ACC_ENA) && (Finder->NewMailN.Flags & YAMN_ACC_CONT))
			{
				DBDeleteContactSetting(Finder->hContact, "CList", "Hidden");
			}
			else
			{
				DBWriteContactSettingByte(Finder->hContact, "CList", "Hidden", 1);
			}
		}
		else
		{
			if((Finder->Flags & YAMN_ACC_ENA) && (Finder->NewMailN.Flags & YAMN_ACC_CONT))
			{
				Finder->hContact =(HANDLE) CallService(MS_DB_CONTACT_ADD, 0, 0);
				CallService(MS_PROTO_ADDTOCONTACT,(WPARAM)Finder->hContact,(LPARAM)ProtoName);
				DBWriteContactSettingString(Finder->hContact,ProtoName,"Id",Finder->Name);
				DBWriteContactSettingString(Finder->hContact,ProtoName,"Nick",Finder->Name);
				DBWriteContactSettingString(Finder->hContact,"Protocol","p",ProtoName);
				DBWriteContactSettingWord(Finder->hContact, ProtoName, "Status", ID_STATUS_ONLINE);
				DBWriteContactSettingString(Finder->hContact, "CList", "StatusMsg", Translate("No new mail message"));
			}

		}
	}

}
