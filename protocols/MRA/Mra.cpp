#include "Mra.h"

PLUGINLINK *pluginLink;
MM_INTERFACE mmi;
MRA_SETTINGS masMraSettings;
int hLangpack;

PLUGININFOEX pluginInfoEx={
	sizeof(PLUGININFOEX),
	PROTOCOL_DISPLAY_NAME_ORIGA,
	PLUGIN_VERSION_DWORD,
	"Provides support for Mail.ru agent Instant Messenger protocol ("__DATE__" "__TIME__")",
	"Rozhuk Ivan",
	"Rozhuk_I@mail.ru",
	"© 2006-2011 Rozhuk Ivan",
	"http://addons.miranda-im.org/details.php?action=viewfile&id=2544",
	UNICODE_AWARE,		//not transient
	0,		//doesn't replace anything built-in
	// {E7C48BAB-8ACE-4CB3-8446-D4B73481F497}
	{ 0xe7c48bab, 0x8ace, 0x4cb3, { 0x84, 0x46, 0xd4, 0xb7, 0x34, 0x81, 0xf4, 0x97 } }
};

static const MUUID interfaces[]={MIID_PROTOCOL,MIID_LAST};


int		OnModulesLoaded		(WPARAM wParam,LPARAM lParam);
int		OnPreShutdown		(WPARAM wParam,LPARAM lParam);
void	VersionConversions	();


BOOL WINAPI DllMain(HINSTANCE hInstance,DWORD dwReason,LPVOID Reserved)
{
	switch(dwReason){
	case DLL_PROCESS_ATTACH:
		bzero(&masMraSettings,sizeof(masMraSettings));
		masMraSettings.hInstance=hInstance;
		masMraSettings.hHeap=HeapCreate(0,0,0);//GetProcessHeap();
		masMraSettings.bLoggedIn=FALSE;
		masMraSettings.dwStatusMode=ID_STATUS_OFFLINE;
		masMraSettings.dwDesiredStatusMode=ID_STATUS_OFFLINE;
		DisableThreadLibraryCalls((HMODULE)hInstance);
		break;
	case DLL_PROCESS_DETACH:
		HeapDestroy(masMraSettings.hHeap);
		masMraSettings.hHeap=NULL;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}

	return(TRUE);
}


extern "C" MRA_API PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return(&pluginInfoEx);
}

extern "C" MRA_API const MUUID* MirandaPluginInterfaces()
{
	return(interfaces);
}


extern "C" MRA_API int Load(PLUGINLINK *link)
{
	SIZE_T dwBuffLen;
	WCHAR szBuff[MAX_FILEPATH];
	LPSTR lpszFullFileName=(LPSTR)szBuff;
	LPWSTR lpwszFileName;
	PROTOCOLDESCRIPTOR pd={0};

	pluginLink=link;
	mir_getLP(&pluginInfoEx);
	mir_getMMI(&mmi);


	// Get module name from DLL file name
	if (GetModuleFileName(masMraSettings.hInstance,szBuff,MAX_FILEPATH))
	{
		WCHAR sztmBuff[MAX_FILEPATH];
		if ((dwBuffLen=GetFullPathName(szBuff,MAX_FILEPATH,sztmBuff,&lpwszFileName)))
		{
			dwBuffLen=(lstrlenW(lpwszFileName)-4);
			//lpwszFileName=L"MRA.dll";
			//dwBuffLen=3;
			masMraSettings.dwModuleNameLen=(dwBuffLen<MAXMODULELABELLENGTH)? dwBuffLen:(MAXMODULELABELLENGTH-1);
			memmove(masMraSettings.wszModuleName,lpwszFileName,(masMraSettings.dwModuleNameLen*sizeof(WCHAR)));
			CharUpperBuff(masMraSettings.wszModuleName,masMraSettings.dwModuleNameLen);
			masMraSettings.wszModuleName[masMraSettings.dwModuleNameLen]=0;
			WideCharToMultiByte(MRA_CODE_PAGE,0,masMraSettings.wszModuleName,(masMraSettings.dwModuleNameLen+1),masMraSettings.szModuleName,MAXMODULELABELLENGTH,NULL,NULL);

			if (DB_Mra_GetByte(NULL,"UseDisplayModuleName",TRUE))
			{
				masMraSettings.dwDisplayModuleNameLen=mir_sntprintf(masMraSettings.wszDisplayModuleName,SIZEOF(masMraSettings.wszDisplayModuleName),L"%s: %S",PROTOCOL_NAMEW,PROTOCOL_DISPLAY_NAME_ORIGA);
			}else{
				memmove(masMraSettings.wszDisplayModuleName,masMraSettings.wszModuleName,(masMraSettings.dwModuleNameLen*sizeof(WCHAR)));
				masMraSettings.dwDisplayModuleNameLen=masMraSettings.dwModuleNameLen;
				masMraSettings.szDisplayModuleName[masMraSettings.dwDisplayModuleNameLen]=0;
			}
			WideCharToMultiByte(MRA_CODE_PAGE,0,masMraSettings.wszDisplayModuleName,(masMraSettings.dwDisplayModuleNameLen+1),masMraSettings.szDisplayModuleName,MAX_PATH,NULL,NULL);
		}
	}

	// load libs
	if (GetModuleFileName(NULL,szBuff,MAX_FILEPATH))
	{// 
		DWORD dwErrorCode;

		masMraSettings.dwMirWorkDirPathLen=GetFullPathName(szBuff,MAX_FILEPATH,masMraSettings.szMirWorkDirPath,&lpwszFileName);
		if (masMraSettings.dwMirWorkDirPathLen)
		{
			masMraSettings.dwMirWorkDirPathLen-=lstrlenW(lpwszFileName);
			masMraSettings.szMirWorkDirPath[masMraSettings.dwMirWorkDirPathLen]=0;

			// load xstatus icons lib
			dwErrorCode=FindFile(masMraSettings.szMirWorkDirPath,masMraSettings.dwMirWorkDirPathLen,L"xstatus_MRA.dll",-1,szBuff,SIZEOF(szBuff),(DWORD*)&dwBuffLen);
			if (dwErrorCode==NO_ERROR)
			{
				masMraSettings.hDLLXStatusIcons=LoadLibraryEx(szBuff,NULL,/*LOAD_LIBRARY_AS_DATAFILE*/0);
				if (masMraSettings.hDLLXStatusIcons)
				{
					if ((dwBuffLen=LoadStringW(masMraSettings.hDLLXStatusIcons,IDS_IDENTIFY,szBuff,MAX_FILEPATH))==0 || CompareStringW(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,L"# Custom Status Icons #",23,szBuff,dwBuffLen)!=CSTR_EQUAL)
					{
						FreeLibrary(masMraSettings.hDLLXStatusIcons);
						masMraSettings.hDLLXStatusIcons=NULL;
					}
				}
			}

			// load zlib
			dwErrorCode=FindFile(masMraSettings.szMirWorkDirPath,masMraSettings.dwMirWorkDirPathLen,L"zlib.dll",-1,szBuff,SIZEOF(szBuff),(DWORD*)&dwBuffLen);
			if (dwErrorCode!=NO_ERROR) dwErrorCode=FindFile(masMraSettings.szMirWorkDirPath,masMraSettings.dwMirWorkDirPathLen,L"zlib1.dll",-1,szBuff,SIZEOF(szBuff),(DWORD*)&dwBuffLen);
			if (dwErrorCode==NO_ERROR)
			{// load xstatus icons lib
				masMraSettings.hDLLZLib=LoadLibraryEx(szBuff,NULL,0);
				if (masMraSettings.hDLLZLib)
				{
					masMraSettings.lpfnCompress2=(HANDLE)GetProcAddress(masMraSettings.hDLLZLib,"compress2");
					masMraSettings.lpfnUncompress=(HANDLE)GetProcAddress(masMraSettings.hDLLZLib,"uncompress");
				}
			}
		}
	}

	InitializeCriticalSectionAndSpinCount(&masMraSettings.csCriticalSectionSend,0);
	MraSendQueueInitialize(0,&masMraSettings.hSendQueueHandle);
	MraFilesQueueInitialize(0,&masMraSettings.hFilesQueueHandle);
	MraMPopSessionQueueInitialize(&masMraSettings.hMPopSessionQueue);

	FifoMTInitialize(&masMraSettings.ffmtAPCQueue,0);
	masMraSettings.hWaitEventThreadAPCHandle=CreateEvent(NULL,FALSE,FALSE,NULL);
	InterlockedExchange((volatile LONG*)&masMraSettings.dwAPCThreadRunning,TRUE);
	masMraSettings.hThreadAPC=(HANDLE)mir_forkthread((pThreadFunc)MraUserAPCThreadProc,NULL);
	if (masMraSettings.hThreadAPC==NULL)
	{
		MraAPCQueueDestroy(&masMraSettings.ffmtAPCQueue);
		CloseHandle(masMraSettings.hWaitEventThreadAPCHandle);
		masMraSettings.hWaitEventThreadAPCHandle=NULL;
		DuplicateHandle(GetCurrentProcess(),GetCurrentThread(),GetCurrentProcess(),&masMraSettings.hThreadAPC,THREAD_SET_CONTEXT,FALSE,0);
		DebugPrintCRLFW(L"Fail on create event APC thread, using miranda main thread");
		MessageBoxW(NULL,L"Fail on create event APC thread, using miranda main thread",PROTOCOL_DISPLAY_NAMEW,(MB_OK|MB_ICONINFORMATION));
	}//*/

	masMraSettings.hHookModulesLoaded=HookEvent(ME_SYSTEM_MODULESLOADED,OnModulesLoaded);
	masMraSettings.hHookPreShutdown=HookEvent(ME_SYSTEM_PRESHUTDOWN,OnPreShutdown);

	LoadServices();

	//pd.cbSize=sizeof(pd);
	pd.cbSize=PROTOCOLDESCRIPTOR_V3_SIZE;
	pd.szName=PROTOCOL_NAMEA;
	pd.type=PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE,0,(LPARAM)&pd);

	DebugPrintCRLFW(L"Load - DONE");
return(0);
}


extern "C" MRA_API int Unload(void)
{
	UnloadServices();
	//if (hHookOnUserInfoInit)				UnhookEvent(hHookOnUserInfoInit);
	if (masMraSettings.hHookPreShutdown)	{UnhookEvent(masMraSettings.hHookPreShutdown);		masMraSettings.hHookPreShutdown=NULL;}
	if (masMraSettings.hHookModulesLoaded)	{UnhookEvent(masMraSettings.hHookModulesLoaded);	masMraSettings.hHookModulesLoaded=NULL;}

	Netlib_CloseHandle(masMraSettings.hNetlibUser);
	masMraSettings.hNetlibUser=NULL;

	MraMPopSessionQueueDestroy(masMraSettings.hMPopSessionQueue);
	masMraSettings.hMPopSessionQueue=NULL;

	MraFilesQueueDestroy(masMraSettings.hFilesQueueHandle);
	masMraSettings.hFilesQueueHandle=NULL;

	MraSendQueueDestroy(masMraSettings.hSendQueueHandle);
	masMraSettings.hSendQueueHandle=NULL;

	DeleteCriticalSection(&masMraSettings.csCriticalSectionSend);
	SecureZeroMemory(&masMraSettings.csCriticalSectionSend,sizeof(CRITICAL_SECTION));

	if (masMraSettings.hDLLXStatusIcons)
	{
		FreeLibrary(masMraSettings.hDLLXStatusIcons);
		masMraSettings.hDLLXStatusIcons=NULL;
	}

	if (masMraSettings.hDLLZLib)
	{
		FreeLibrary(masMraSettings.hDLLZLib);
		masMraSettings.hDLLZLib=NULL;
		masMraSettings.lpfnCompress2=NULL;
		masMraSettings.lpfnUncompress=NULL;
	}

	SecureZeroMemory(pluginLink,sizeof(pluginLink));
	SecureZeroMemory(&mmi,sizeof(pluginLink));

	DebugPrintCRLFW(L"Unload - DONE");
return(0);
}


static int OnModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	WCHAR szBuffer[MAX_PATH];
	NETLIBUSER nlu={0};

	mir_sntprintf(szBuffer,SIZEOF(szBuffer),L"%s %s",PROTOCOL_NAMEW,TranslateW(L"plugin connections"));
	nlu.cbSize=sizeof(nlu);
	nlu.flags=(NUF_INCOMING|NUF_OUTGOING|NUF_HTTPCONNS|NUF_UNICODE);
	nlu.szSettingsModule=PROTOCOL_NAMEA;
	nlu.ptszDescriptiveName=szBuffer;
	masMraSettings.hNetlibUser=(HANDLE)CallService(MS_NETLIB_REGISTERUSER,0,(LPARAM)&nlu);

#ifdef _DEBUG
	// всех в offline // тк unsaved values сохран€ютс€ их нужно инициализировать
	for(HANDLE hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);hContact!=NULL;hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
	{// функци€ сама провер€ет принадлежность контакта к MRA
		SetContactBasicInfoW(hContact,SCBIFSI_LOCK_CHANGES_EVENTS,(SCBIF_ID|SCBIF_GROUP_ID|SCBIF_SERVER_FLAG|SCBIF_STATUS),-1,-1,0,0,ID_STATUS_OFFLINE,NULL,0,NULL,0,NULL,0);
	}
#else
	// unsaved values
	DB_Mra_CreateResidentSetting("Status");// NOTE: XStatus cannot be temporary
	DB_Mra_CreateResidentSetting("LogonTS");
	DB_Mra_CreateResidentSetting("ContactID");
	DB_Mra_CreateResidentSetting("GroupID");
	DB_Mra_CreateResidentSetting("ContactFlags");
	DB_Mra_CreateResidentSetting("ContactSeverFlags");
	DB_Mra_CreateResidentSetting("HooksLocked");
	DB_Mra_CreateResidentSetting(DBSETTING_CAPABILITIES);
	DB_Mra_CreateResidentSetting(DBSETTING_XSTATUSNAME);
	DB_Mra_CreateResidentSetting(DBSETTING_XSTATUSMSG);
	DB_Mra_CreateResidentSetting(DBSETTING_BLOGSTATUSTIME);
	DB_Mra_CreateResidentSetting(DBSETTING_BLOGSTATUSID);
	DB_Mra_CreateResidentSetting(DBSETTING_BLOGSTATUS);
	DB_Mra_CreateResidentSetting(DBSETTING_BLOGSTATUSMUSIC);
	
	// всех в offline // тк unsaved values сохран€ютс€ их нужно инициализировать
	for(HANDLE hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);hContact!=NULL;hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
	{// функци€ сама провер€ет принадлежность контакта к MRA
		SetContactBasicInfoW(hContact,SCBIFSI_LOCK_CHANGES_EVENTS,(SCBIF_ID|SCBIF_GROUP_ID|SCBIF_SERVER_FLAG|SCBIF_STATUS),-1,-1,0,0,ID_STATUS_OFFLINE,NULL,0,NULL,0,NULL,0);
	}
#endif

	MraAvatarsQueueInitialize(&masMraSettings.hAvatarsQueueHandle);
	
	VersionConversions();

	MraAntiSpamLoadBadWordsW();

	LoadModules();

	CallService(MS_UPDATE_REGISTERFL,(WPARAM)MRA_PLUGIN_UPDATER_ID,(LPARAM)&pluginInfoEx);

	InterlockedExchange((volatile LONG*)&masMraSettings.dwGlobalPluginRunning,TRUE);

	DebugPrintCRLFW(L"OnModulesLoaded - DONE");
return(0);
}


int OnPreShutdown(WPARAM wParam,LPARAM lParam)
{
	InterlockedExchange((volatile LONG*)&masMraSettings.dwGlobalPluginRunning,FALSE);

	MraSetStatus(ID_STATUS_OFFLINE,0);
	
	MraAvatarsQueueDestroy(masMraSettings.hAvatarsQueueHandle);
	masMraSettings.hAvatarsQueueHandle=NULL;

	if (masMraSettings.hThreadWorker)
	{
		if (IsThreadAlive(masMraSettings.hThreadWorker)) WaitForSingleObjectEx(masMraSettings.hThreadWorker,(WAIT_FOR_THREAD_TIMEOUT*1000),FALSE);
		masMraSettings.hThreadWorker=NULL;
	}

	InterlockedExchange((volatile LONG*)&masMraSettings.dwAPCThreadRunning,FALSE);
	if (masMraSettings.hWaitEventThreadAPCHandle)
	if (IsThreadAlive(masMraSettings.hThreadAPC))
	{
		SetEvent(masMraSettings.hWaitEventThreadAPCHandle);
	}else{
		CloseHandle(masMraSettings.hWaitEventThreadAPCHandle);
		//WaitForSingleObjectEx(masMraSettings.hThreadAPC,(WAIT_FOR_THREAD_TIMEOUT*1000),FALSE); // no wait - dead lock.
		masMraSettings.hWaitEventThreadAPCHandle=NULL;
		masMraSettings.hThreadAPC=NULL;
	}

	UnloadModules();

	MraAntiSpamFreeBadWords();

	DebugPrintCRLFW(L"OnPreShutdown - DONE");
return(0);
}



void VersionConversions()
{
	/*switch(DB_Mra_GetDword(NULL,"LastPluginVersion",PLUGIN_MAKE_VERSION(0,0,0,0))) {
	default:
	case PLUGIN_MAKE_VERSION(1,14,0,0):
	case PLUGIN_MAKE_VERSION(1,14,0,1):
	case PLUGIN_MAKE_VERSION(1,14,0,2):
		DB_Mra_DeleteValue(NULL,"MirVer");
		DB_Mra_DeleteValue(NULL,"MessagesCountSend");
		DB_Mra_DeleteValue(NULL,"MRAClient");
		DB_Mra_DeleteValue(NULL,"MRAClientVer");
		DB_Mra_DeleteValue(NULL,"MRAClientCapabilities");
		DB_Mra_DeleteValue(NULL,"Status");
		DB_Mra_DeleteValue(NULL,"ContactID");
		DB_Mra_DeleteValue(NULL,"GroupID");
		DB_Mra_DeleteValue(NULL,"ContactSeverFlags");
		DB_Mra_DeleteValue(NULL,"HooksLocked");
		for(HANDLE hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);hContact!=NULL;hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
		{
			DB_Mra_DeleteValue(hContact,"MessagesCountSend");
			DB_Mra_DeleteValue(hContact,"MRAClient");
			DB_Mra_DeleteValue(hContact,"MRAClientVer");
			DB_Mra_DeleteValue(hContact,"MRAClientCapabilities");
			DB_Mra_DeleteValue(hContact,"Status");
			DB_Mra_DeleteValue(hContact,"ContactID");
			DB_Mra_DeleteValue(hContact,"GroupID");
			DB_Mra_DeleteValue(hContact,"ContactSeverFlags");
			DB_Mra_DeleteValue(hContact,"HooksLocked");
		}
	case PLUGIN_MAKE_VERSION(1,14,0,3):
	case PLUGIN_MAKE_VERSION(1,14,0,4):
	case PLUGIN_MAKE_VERSION(1,14,0,5):
	case PLUGIN_MAKE_VERSION(1,14,0,6):
	case PLUGIN_MAKE_VERSION(1,14,0,7):
	//case PLUGIN_MAKE_VERSION(1,14,1,0):
		DB_Mra_SetByte(NULL,DBSETTING_XSTATUSID,(BYTE)DB_Mra_GetDword(NULL,DBSETTING_XSTATUSID,MRA_MIR_XSTATUS_NONE));
		for(HANDLE hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);hContact!=NULL;hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
		{
			if (IsContactMra(hContact)) DB_Mra_SetByte(hContact,DBSETTING_XSTATUSID,(BYTE)DB_Mra_GetDword(hContact,DBSETTING_XSTATUSID,MRA_MIR_XSTATUS_NONE));
		}
	case PLUGIN_MAKE_VERSION(1,16,0,2):
		DB_Mra_DeleteValue(NULL,"CvtSmilesToTags");
	case PLUGIN_MAKE_VERSION(1,16,0,3):
		for(HANDLE hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);hContact!=NULL;hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
		{
			if (IsContactMra(hContact)==FALSE) DB_Mra_DeleteValue(hContact,DBSETTING_XSTATUSID);
		}
	case PLUGIN_MAKE_VERSION(0,0,0,0):// no previos version
		MraAntiSpamResetBadWordsList();
	case PLUGIN_VERSION_DWORD:// current version
		break;
	}*/

	DB_Mra_SetDword(NULL,"LastPluginVersion",PLUGIN_VERSION_DWORD);



	{// pass conv
		char szPass[MAX_PATH];
		SIZE_T dwPassSize;

		if(DB_Mra_GetStaticStringA(NULL,"Password",(LPSTR)szPass,SIZEOF(szPass),&dwPassSize))
		{//bit of a security hole here, since it's easy to extract a password from an edit box
			CallService(MS_DB_CRYPT_DECODESTRING,(dwPassSize+1),(LPARAM)szPass);

			SetPassDB(szPass,dwPassSize);

			SecureZeroMemory(szPass,sizeof(szPass));
			DB_Mra_DeleteValue(NULL,"Password");
		}
	}
}
