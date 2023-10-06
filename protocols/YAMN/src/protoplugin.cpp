/*
 * YAMN plugin export functions for protocols
 *
 * (c) majvan 2002-2004
 */

#include "stdafx.h"

 //--------------------------------------------------------------------------------------------------

YAMN_PROTOPLUGINQUEUE *FirstProtoPlugin = nullptr;

INT_PTR RegisterProtocolPluginSvc(WPARAM, LPARAM);

//Removes plugin from queue and deletes registration structures
INT_PTR UnregisterProtocolPlugin(YAMN_PROTOPLUGIN *Plugin);

INT_PTR UnregisterProtocolPluginSvc(WPARAM, LPARAM);

//Removes plugins from queue and deletes registration structures
INT_PTR UnregisterProtoPlugins();

//Sets imported functions for an plugin and therefore it starts plugin to be registered and running
// Plugin- plugin, which wants to set its functions
// YAMNFcn- pointer to imported functions with accounts
// YAMNFcnVer- version of YAMN_PROTOIMPORTFCN, use YAMN_PROTOIMPORTFCNVERSION
// YAMNMailFcn- pointer to imported functions with mails
// YAMNMailFcnVer- version of YAMN_MAILIMPORTFCN, use YAMN_MAILIMPORTFCNVERSION
// returns nonzero if success
int WINAPI SetProtocolPluginFcnImportFcn(YAMN_PROTOPLUGIN *Plugin, PYAMN_PROTOIMPORTFCN YAMNFcn, uint32_t YAMNFcnVer, PYAMN_MAILIMPORTFCN YAMNMailFcn, uint32_t YAMNMailFcnVer);

struct CExportedServices ProtoPluginExportedSvc[] =
{
	{MS_YAMN_REGISTERPROTOPLUGIN, RegisterProtocolPluginSvc},
	{MS_YAMN_UNREGISTERPROTOPLUGIN, UnregisterProtocolPluginSvc},
	{MS_YAMN_GETFILENAME, GetFileNameSvc},
	{MS_YAMN_DELETEFILENAME, DeleteFileNameSvc},
};

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

INT_PTR RegisterProtocolPluginSvc(WPARAM wParam, LPARAM lParam)
{
	PYAMN_PROTOREGISTRATION Registration = (PYAMN_PROTOREGISTRATION)wParam;
	YAMN_PROTOPLUGIN *Plugin;

	if (lParam != YAMN_PROTOREGISTRATIONVERSION)
		return 0;
	if ((Registration->Name == nullptr) || (Registration->Ver == nullptr))
		return (INT_PTR)NULL;
	if (nullptr == (Plugin = new YAMN_PROTOPLUGIN))
		return (INT_PTR)NULL;

	Plugin->PluginInfo = Registration;
	Plugin->FirstAccount = nullptr;
	Plugin->Fcn = nullptr;
	Plugin->MailFcn = nullptr;

	return (INT_PTR)Plugin;
}

int WINAPI SetProtocolPluginFcnImportFcn(YAMN_PROTOPLUGIN *Plugin, PYAMN_PROTOIMPORTFCN YAMNFcn, uint32_t YAMNFcnVer, PYAMN_MAILIMPORTFCN YAMNMailFcn, uint32_t YAMNMailFcnVer)
{
	if (YAMNFcnVer != YAMN_PROTOIMPORTFCNVERSION)
		return 0;
	if (YAMNMailFcnVer != YAMN_MAILIMPORTFCNVERSION)
		return 0;
	if (YAMNFcn == nullptr)
		return 0;
	if (YAMNMailFcn == nullptr)
		return 0;

	Plugin->Fcn = YAMNFcn;
	Plugin->MailFcn = YAMNMailFcn;

	mir_cslock lck(PluginRegCS);

	// We add protocol to the protocol list
	YAMN_PROTOPLUGINQUEUE *Parser;
	for (Parser = FirstProtoPlugin; Parser != nullptr && Parser->Next != nullptr; Parser = Parser->Next);
	if (Parser == nullptr) {
		FirstProtoPlugin = new YAMN_PROTOPLUGINQUEUE;
		Parser = FirstProtoPlugin;
	}
	else {
		Parser->Next = new YAMN_PROTOPLUGINQUEUE;
		Parser = Parser->Next;
	}

	Parser->Plugin = Plugin;
	Parser->Next = nullptr;
	return 1;
}

INT_PTR UnregisterProtocolPlugin(YAMN_PROTOPLUGIN *Plugin)
{
	YAMN_PROTOPLUGINQUEUE *Parser, *Found;

	if (FirstProtoPlugin->Plugin == Plugin) {
		Found = FirstProtoPlugin;
		FirstProtoPlugin = FirstProtoPlugin->Next;
	}
	else {
		for (Parser = FirstProtoPlugin; (Parser->Next != nullptr) && (Plugin != Parser->Next->Plugin); Parser = Parser->Next);
		if (Parser->Next != nullptr) {
			Found = Parser->Next;
			Parser->Next = Parser->Next->Next;
		}
		else
			Found = nullptr;
	}
	
	if (!Found) 
		return 1;

	StopAccounts(Plugin);
	DeleteAccounts(Plugin);
	if (Plugin->Fcn->UnLoadFcn != nullptr)
		Plugin->Fcn->UnLoadFcn((void *)nullptr);

	delete Found->Plugin;
	delete Found;
	return 0;
}

INT_PTR UnregisterProtocolPluginSvc(WPARAM wParam, LPARAM)
{
	YAMN_PROTOPLUGIN *Plugin = (YAMN_PROTOPLUGIN*)wParam;

	mir_cslock lck(PluginRegCS);
	UnregisterProtocolPlugin(Plugin);
	return 1;
}

INT_PTR UnregisterProtoPlugins()
{
	mir_cslock lck(PluginRegCS);
	// We remove protocols from the protocol list
	while (FirstProtoPlugin != nullptr)
		UnregisterProtocolPlugin(FirstProtoPlugin->Plugin);
	return 1;
}

INT_PTR GetFileNameSvc(WPARAM wParam, LPARAM)
{
	wchar_t *FileName = new wchar_t[MAX_PATH];
	if (FileName == nullptr)
		return NULL;

	mir_snwprintf(FileName, MAX_PATH, L"%s\\yamn-accounts.%s.%s.book", UserDirectory, (wchar_t *)wParam, ProfileName);
	return (INT_PTR)FileName;
}

INT_PTR DeleteFileNameSvc(WPARAM wParam, LPARAM)
{
	if ((wchar_t *)wParam != nullptr)
		delete[](wchar_t *) wParam;

	return 0;
}
