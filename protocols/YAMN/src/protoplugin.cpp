/*
 * YAMN plugin export functions for protocols
 *
 * (c) majvan 2002-2004
 */

#include "stdafx.h"

 //--------------------------------------------------------------------------------------------------

YAMN_PROTOPLUGINQUEUE *FirstProtoPlugin = nullptr;

// Removes plugins from queue and deletes registration structures
INT_PTR UnregisterProtoPlugins();

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

YAMN_PROTOPLUGIN* RegisterProtocolPlugin(YAMN_PROTOREGISTRATION *Registration)
{
	if ((Registration->Name == nullptr) || (Registration->Ver == nullptr))
		return (INT_PTR)NULL;

	auto *Plugin = new YAMN_PROTOPLUGIN();
	Plugin->PluginInfo = Registration;
	return Plugin;
}

// Sets imported functions for an plugin and therefore it starts plugin to be registered and running
// Plugin- plugin, which wants to set its functions
// YAMNFcn- pointer to imported functions with accounts
// YAMNMailFcn- pointer to imported functions with mails
// returns nonzero if success

int SetProtocolPluginFcnImportFcn(YAMN_PROTOPLUGIN *Plugin, YAMN_PROTOIMPORTFCN *YAMNFcn, YAMN_MAILIMPORTFCN *YAMNMailFcn)
{
	if (YAMNFcn == nullptr || YAMNMailFcn == nullptr)
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

int UnregisterProtocolPlugin(YAMN_PROTOPLUGIN *Plugin)
{
	mir_cslock lck(PluginRegCS);

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

INT_PTR UnregisterProtoPlugins()
{
	mir_cslock lck(PluginRegCS);
	// We remove protocols from the protocol list
	while (FirstProtoPlugin != nullptr)
		UnregisterProtocolPlugin(FirstProtoPlugin->Plugin);
	return 1;
}

CMStringW GetFileName(wchar_t *pwszPlugin)
{
	return CMStringW(FORMAT, L"%s\\yamn-accounts.%s.%s.book", UserDirectory, pwszPlugin, ProfileName);
}
