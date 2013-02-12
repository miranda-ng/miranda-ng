/*
 * YAMN plugin export functions for protocols
 *
 * (c) majvan 2002-2004
 */

#include "yamn.h"

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

PYAMN_PROTOPLUGINQUEUE FirstProtoPlugin=NULL;

INT_PTR RegisterProtocolPluginSvc(WPARAM,LPARAM);

//Removes plugin from queue and deletes registration structures
INT_PTR UnregisterProtocolPlugin(HYAMNPROTOPLUGIN Plugin);

INT_PTR UnregisterProtocolPluginSvc(WPARAM,LPARAM);

//Removes plugins from queue and deletes registration structures
INT_PTR UnregisterProtoPlugins();

//Sets imported functions for an plugin and therefore it starts plugin to be registered and running
// Plugin- plugin, which wants to set its functions
// YAMNFcn- pointer to imported functions with accounts
// YAMNFcnVer- version of YAMN_PROTOIMPORTFCN, use YAMN_PROTOIMPORTFCNVERSION
// YAMNMailFcn- pointer to imported functions with mails
// YAMNMailFcnVer- version of YAMN_MAILIMPORTFCN, use YAMN_MAILIMPORTFCNVERSION
// returns nonzero if success
int WINAPI SetProtocolPluginFcnImportFcn(HYAMNPROTOPLUGIN Plugin,PYAMN_PROTOIMPORTFCN YAMNFcn,DWORD YAMNFcnVer,PYAMN_MAILIMPORTFCN YAMNMailFcn,DWORD YAMNMailFcnVer);

struct CExportedFunctions ProtoPluginExportedFcn[]=
{
	{YAMN_SETPROTOCOLPLUGINFCNIMPORTID,(void *)SetProtocolPluginFcnImportFcn},
};

struct CExportedServices ProtoPluginExportedSvc[]=
{
	{MS_YAMN_REGISTERPROTOPLUGIN,RegisterProtocolPluginSvc},
	{MS_YAMN_UNREGISTERPROTOPLUGIN,UnregisterProtocolPluginSvc},
	{MS_YAMN_GETFILENAME,GetFileNameSvc},
	{MS_YAMN_DELETEFILENAME,DeleteFileNameSvc},
};

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

INT_PTR RegisterProtocolPluginSvc(WPARAM wParam,LPARAM lParam)
{
	PYAMN_PROTOREGISTRATION Registration=(PYAMN_PROTOREGISTRATION)wParam;
	HYAMNPROTOPLUGIN Plugin;

	if (lParam != YAMN_PROTOREGISTRATIONVERSION)
		return 0;
	if ((Registration->Name==NULL) || (Registration->Ver==NULL))
		return (INT_PTR)NULL;
	if (NULL==(Plugin=new YAMN_PROTOPLUGIN))
		return (INT_PTR)NULL;

	Plugin->PluginInfo=Registration;

	Plugin->FirstAccount=NULL;

	Plugin->AccountBrowserSO=new SWMRG;
	SWMRGInitialize(Plugin->AccountBrowserSO,NULL);

	Plugin->Fcn=NULL;
	Plugin->MailFcn=NULL;

#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"::: YAMN- new protocol registered: %0x (%s) :::\n",Plugin,Registration->Name);
#endif
	return (INT_PTR)Plugin;
}

int WINAPI SetProtocolPluginFcnImportFcn(HYAMNPROTOPLUGIN Plugin,PYAMN_PROTOIMPORTFCN YAMNFcn,DWORD YAMNFcnVer,PYAMN_MAILIMPORTFCN YAMNMailFcn,DWORD YAMNMailFcnVer)
{
	PYAMN_PROTOPLUGINQUEUE Parser;

	if (YAMNFcnVer != YAMN_PROTOIMPORTFCNVERSION)
		return 0;
	if (YAMNMailFcnVer != YAMN_MAILIMPORTFCNVERSION)
		return 0;
	if (YAMNFcn==NULL)
		return 0;
	if (YAMNMailFcn==NULL)
		return 0;

#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"::: YAMN- protocol %0x import succeed :::\n",Plugin);
#endif
	Plugin->Fcn=YAMNFcn;
	Plugin->MailFcn=YAMNMailFcn;

	EnterCriticalSection(&PluginRegCS);
//We add protocol to the protocol list
	for (Parser=FirstProtoPlugin;Parser != NULL && Parser->Next != NULL;Parser=Parser->Next);
	if (Parser==NULL)
	{
		FirstProtoPlugin=new YAMN_PROTOPLUGINQUEUE;
		Parser=FirstProtoPlugin;
	}
	else
	{
		Parser->Next=new YAMN_PROTOPLUGINQUEUE;
		Parser=Parser->Next;
	}

	Parser->Plugin=Plugin;
	Parser->Next=NULL;

	LeaveCriticalSection(&PluginRegCS);
	return 1;
}

INT_PTR UnregisterProtocolPlugin(HYAMNPROTOPLUGIN Plugin)
{
	PYAMN_PROTOPLUGINQUEUE Parser,Found;

#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"Entering UnregisterProtocolPlugin\n");
#endif
	if (FirstProtoPlugin->Plugin==Plugin)
	{
		Found=FirstProtoPlugin;
		FirstProtoPlugin=FirstProtoPlugin->Next;
	}
	else
	{
		for (Parser=FirstProtoPlugin;(Parser->Next != NULL) && (Plugin != Parser->Next->Plugin);Parser=Parser->Next);
		if (Parser->Next != NULL)
		{
			Found=Parser->Next;
			Parser->Next=Parser->Next->Next;
		}
		else
			Found=NULL;
	}
	if (Found != NULL)
	{
		StopAccounts(Plugin);
		DeleteAccounts(Plugin);
		if (Plugin->Fcn->UnLoadFcn != NULL)
			Plugin->Fcn->UnLoadFcn((void *)0);
		
		delete Found->Plugin->AccountBrowserSO;
		delete Found->Plugin;
		delete Found;

#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"::: YAMN- protocol %0x unregistered :::\n",Plugin);
#endif
	}
	else
		return 0;
	return 1;
}

INT_PTR UnregisterProtocolPluginSvc(WPARAM wParam,LPARAM lParam)
{
	HYAMNPROTOPLUGIN Plugin=(HYAMNPROTOPLUGIN)wParam;

	EnterCriticalSection(&PluginRegCS);
	UnregisterProtocolPlugin(Plugin);
	LeaveCriticalSection(&PluginRegCS);
	return 1;

}

INT_PTR UnregisterProtoPlugins()
{
	EnterCriticalSection(&PluginRegCS);
//We remove protocols from the protocol list
	while(FirstProtoPlugin != NULL)
		UnregisterProtocolPlugin(FirstProtoPlugin->Plugin);
	LeaveCriticalSection(&PluginRegCS);
	return 1;
}

INT_PTR GetFileNameSvc(WPARAM wParam,LPARAM)
{
	TCHAR *FileName = new TCHAR[MAX_PATH];
	if (FileName == NULL)
		return NULL;

	mir_sntprintf(FileName, MAX_PATH, _T("%s\\yamn-accounts.%s.%s.book"), UserDirectory, wParam, ProfileName);
	return (INT_PTR)FileName;
}

INT_PTR DeleteFileNameSvc(WPARAM wParam,LPARAM)
{
	if (( TCHAR* )wParam != NULL)
		delete[] ( TCHAR* ) wParam;

	return 0;
}
