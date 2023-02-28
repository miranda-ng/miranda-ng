/*
Copyright (C) 2006 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

static IconItem mainIcons[] = {
	{ LPGEN("Main"),     "main",    IDI_MAIN    },
	{ LPGEN("Dialpad"),  "dialpad", IDI_DIALPAD },
	{ LPGEN("Secure"),   "secure",  IDI_SECURE  },
};

static IconItem stateIcons[NUM_STATES] = {
	{ LPGEN("Talking"),  "talking", IDI_TALKING },
	{ LPGEN("Ringing"),  "ringing", IDI_RINGING },
	{ LPGEN("Calling"),  "calling", IDI_CALLING },
	{ LPGEN("On Hold"),  "onhold",  IDI_ON_HOLD },
	{ LPGEN("Ended"),    "ended",   IDI_ENDED   },
	{ LPGEN("Busy"),     "busy", 	  IDI_BUSY    },
	{ LPGEN("Ready"),     "ready", 	  IDI_BUSY    },
};

static IconItem actionIcons[] = {
	{ LPGEN("Make Voice Call"),   "call",   IDI_ACTION_CALL   },
	{ LPGEN("Answer Voice Call"), "answer", IDI_ACTION_ANSWER },
	{ LPGEN("Hold Voice Call"),   "hold",   IDI_ACTION_HOLD   },
	{ LPGEN("Drop Voice Call"),   "drop",   IDI_ACTION_DROP   },
};

SoundDescr g_sounds[NUM_STATES] = {
	{ "voice_started", LPGENW("Started talking") },
	{ "voice_ringing", LPGENW("Ringing") },
	{ "voice_calling", LPGENW("Calling a contact") },
	{ "voice_holded",  LPGENW("Put a call on Hold") },
	{ "voice_ended",   LPGENW("End of call") },
	{ "voice_busy",    LPGENW("Busy signal") },
	{ "voice_ready",   LPGENW("Ready") },
};

/////////////////////////////////////////////////////////////////////////////////////////

static vector<HGENMENU> hCMCalls;
static HGENMENU hCMCall = NULL;
static HGENMENU hCMAnswer = NULL;
static HGENMENU hCMDrop = NULL;
static HGENMENU hCMHold = NULL;

OBJLIST<VoiceProvider> modules(1, PtrKeySortT);
OBJLIST<VoiceCall> calls(1, PtrKeySortT);

static INT_PTR CListDblClick(WPARAM wParam, LPARAM lParam);

static INT_PTR Service_CanCall(WPARAM wParam, LPARAM lParam);
static INT_PTR Service_Call(WPARAM wParam, LPARAM lParam);
static INT_PTR CMAnswer(WPARAM wParam, LPARAM lParam);
static INT_PTR CMHold(WPARAM wParam, LPARAM lParam);
static INT_PTR CMDrop(WPARAM wParam, LPARAM lParam);

class CallingMethod
{
public:
	VoiceProvider *provider;
	MCONTACT hContact;
	wchar_t number[128];

	CallingMethod(VoiceProvider *provider, MCONTACT hContact, const wchar_t *number = NULL)
		: provider(provider), hContact(hContact)
	{
		if (number == NULL)
			this->number[0] = 0;
		else
			wcsncpy_s(this->number, number, _TRUNCATE);
	}

	void Call()
	{
		provider->Call(hContact, number);
	}
};

static int sttCompareCallingMethods(const CallingMethod *p1, const CallingMethod *p2)
{
	if (p1->hContact != p2->hContact)
		return (int)p2->hContact - (int)p1->hContact;

	BOOL noNum1 = (IsEmptyW(p1->number) ? 1 : 0);
	BOOL noNum2 = (IsEmptyW(p2->number) ? 1 : 0);
	if (noNum1 != noNum2)
		return noNum2 - noNum1;

	if (!noNum1) {
		int numDif = lstrcmp(p1->number, p2->number);
		if (numDif != 0)
			return numDif;
	}

	BOOL isProto1 = Proto_IsProtoOnContact(p1->hContact, p1->provider->name);
	BOOL isProto2 = Proto_IsProtoOnContact(p2->hContact, p2->provider->name);
	if (isProto1 != isProto2)
		return isProto2 - isProto1;

	return lstrcmp(p1->provider->description, p2->provider->description);
}

static void AddMethodsFrom(OBJLIST<CallingMethod> *list, MCONTACT hContact)
{
	for (auto &provider: modules)
		if (provider->CanCall(hContact))
			list->insert(new CallingMethod(provider, hContact));
}

static void AddMethodsFrom(OBJLIST<CallingMethod> *list, MCONTACT hContact, const wchar_t *number)
{
	for (auto &provider: modules)
		if (provider->CanCall(number))
			list->insert(new CallingMethod(provider, hContact, number));
}

static void BuildCallingMethodsList(OBJLIST<CallingMethod> *list, MCONTACT hContact)
{
	AddMethodsFrom(list, hContact);

	// Fetch contact number
	char *proto = Proto_GetBaseAccountName(hContact);

	CMStringW protoNumber(db_get_wsm(hContact, proto, "Number"));
	if (!protoNumber.IsEmpty())
		AddMethodsFrom(list, hContact, protoNumber);

	for (int i = 0; ; i++) {
		char tmp[128];
		mir_snprintf(tmp, "MyPhone%d", i);

		CMStringW number(db_get_wsm(hContact, "UserInfo", tmp));
		if (!number.IsEmpty())
			AddMethodsFrom(list, hContact, number);

		if (number.IsEmpty() && i >= 4)
			break;
	}
}

// Functions ////////////////////////////////////////////////////////////////////////////

static MCONTACT ConvertMetacontact(MCONTACT hContact)
{
	MCONTACT hTmp = db_mc_getMostOnline(hContact);
	if (hTmp != NULL)
		return hTmp;

	return hContact;
}

static void AddAccount(PROTOACCOUNT *acc)
{
	if (!acc->IsEnabled())
		return;
	if (IsEmptyA(acc->szModuleName))
		return;
	if (!ProtoServiceExists(acc->szModuleName, PS_VOICE_CAPS))
		return;

	int flags = CallProtoService(acc->szModuleName, PS_VOICE_CAPS, 0, 0);

	if ((flags & VOICE_CAPS_VOICE) == 0)
		return;

	VOICE_MODULE vm = { 0 };
	vm.cbSize = sizeof(vm);
	vm.name = acc->szModuleName;
	vm.description = acc->tszAccountName;
	vm.flags = flags;
	VoiceRegister((WPARAM)&vm, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int AccListChanged(WPARAM wParam, LPARAM lParam)
{
	PROTOACCOUNT *acc = (PROTOACCOUNT *)lParam;
	if (acc == NULL)
		return 0;

	VoiceProvider *provider = FindModule(acc->szModuleName);

	switch (wParam) {
	case PRAC_ADDED:
		AddAccount(acc);
		break;

	case PRAC_CHANGED:
		if (provider != NULL)
			wcsncpy_s(provider->description, acc->tszAccountName, _TRUNCATE);
		break;

	case PRAC_CHECKED:
		{
			BOOL enabled = acc->IsEnabled();

			if (!enabled) {
				if (provider != NULL)
					VoiceUnregister((WPARAM)acc->szModuleName, 0);
			}
			else {
				if (provider == NULL)
					AddAccount(acc);
			}
			break;
		}
	case PRAC_REMOVED:
		if (provider != NULL)
			VoiceUnregister((WPARAM)acc->szModuleName, 0);
		break;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR Service_CallItem(WPARAM wParam, LPARAM, LPARAM param)
{
	MCONTACT hContact = (MCONTACT)wParam;
	int index = (int)param;

	if (hContact == NULL)
		return -1;

	hContact = ConvertMetacontact(hContact);

	OBJLIST<CallingMethod> methods(10, sttCompareCallingMethods);
	BuildCallingMethodsList(&methods, hContact);

	if (index < 0 || index >= methods.getCount())
		return -2;

	methods[index].Call();
	return 0;
}

static int PreBuildContactMenu(WPARAM wParam, LPARAM)
{
	Menu_ShowItem(hCMCall, false);
	Menu_ShowItem(hCMAnswer, false);
	Menu_ShowItem(hCMHold, false);
	Menu_ShowItem(hCMDrop, false);
	for (auto &it : hCMCalls)
		Menu_ShowItem(it, false);

	MCONTACT hContact = (MCONTACT)wParam;
	if (hContact == NULL)
		return -1;

	hContact = ConvertMetacontact(hContact);

	// There is a current call already?
	VoiceCall *call = FindVoiceCall(hContact);
	if (call == nullptr) {
		OBJLIST<CallingMethod> methods(10, sttCompareCallingMethods);
		BuildCallingMethodsList(&methods, hContact);

		if (methods.getCount() == 1) {
			CallingMethod *method = &methods[0];

			wchar_t name[128];
			if (!IsEmptyW(method->number))
				mir_snwprintf(name, TranslateT("Call %s with %s"),
				method->number, method->provider->description);
			else
				mir_snwprintf(name, TranslateT("Call with %s"),
				method->provider->description);

			Menu_ModifyItem(hCMCall, name);
			Menu_ShowItem(hCMCall, true);
		}
		else if (methods.getCount() > 1) {
			Menu_ModifyItem(hCMCall, TranslateT("Call"));
			Menu_ShowItem(hCMCall, true);

			for (int i = 0; i < methods.getCount(); ++i) {
				CallingMethod *method = &methods[i];

				HICON hIcon = method->provider->GetIcon();

				wchar_t name[128];
				if (!IsEmptyW(method->number))
					mir_snwprintf(name, TranslateT("%s with %s"),
					method->number, method->provider->description);
				else
					mir_snwprintf(name, TranslateT("with %s"),
					method->provider->description);

				char service[128];
				mir_snprintf(service, "VoiceService/ContactMenu/Call_%d", i);

				if (i == hCMCalls.size()) {
					CreateServiceFunctionParam(service, Service_CallItem, i);

					CMenuItem mi(&g_plugin);
					mi.position = i;
					mi.flags = CMIF_UNICODE;
					mi.name.w = name;
					mi.hIcon = hIcon;
					mi.pszService = service;
					mi.root = hCMCall;
					hCMCalls.push_back(Menu_AddContactMenuItem(&mi));
				}
				else Menu_ModifyItem(hCMCalls[i], name, hIcon);

				method->provider->ReleaseIcon(hIcon);
			}
		}
	}
	else {
		switch (call->state) {
		case VOICE_STATE_CALLING:
			Menu_ShowItem(hCMDrop, true);
			break;

		case VOICE_STATE_TALKING:
			if (call->module->CanHold())
				Menu_ShowItem(hCMHold, true);
			Menu_ShowItem(hCMDrop, true);
			break;

		case VOICE_STATE_RINGING:
		case VOICE_STATE_ON_HOLD:
			Menu_ShowItem(hCMAnswer, true);
			Menu_ShowItem(hCMDrop, true);
			break;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

VoiceProvider* FindModule(const char *szModule)
{
	for (auto &it : modules)
		if (strcmp(it->name, szModule) == 0)
			return it;

	return NULL;
}

static bool IsCall(VoiceCall *call, const char *szModule, const char *id)
{
	return strcmp(call->module->name, szModule) == 0
		&& call->id != NULL && strcmp(call->id, id) == 0;
}

VoiceCall* FindVoiceCall(const char *szModule, const char *id, bool add)
{
	for (auto &call : calls)
		if (IsCall(call, szModule, id))
			return call;

	if (add) {
		VoiceProvider *module = FindModule(szModule);
		if (module == NULL)
			return NULL;

		VoiceCall *tmp = new VoiceCall(module, id);
		calls.insert(tmp);
		return tmp;
	}

	return nullptr;
}

VoiceCall* FindVoiceCall(MCONTACT hContact)
{
	for (auto &call : calls)
		if (call->state != VOICE_STATE_ENDED && call->hContact == hContact)
			return call;

	return nullptr;
}

bool CanCall(MCONTACT hContact, BOOL now)
{
	for (auto &it : modules)
		if (it->CanCall(hContact, now))
			return true;

	return false;
}

bool CanCall(const wchar_t *number)
{
	for (auto &it : modules)
		if (it->CanCall(number))
			return true;

	return false;
}

bool CanCallNumber()
{
	for (auto &it : modules)
		if (it->flags & VOICE_CAPS_CALL_STRING)
			return true;

	return false;
}

bool IsFinalState(int state)
{
	return state == VOICE_STATE_ENDED || state == VOICE_STATE_BUSY;
}

VoiceCall* GetTalkingCall()
{
	for (auto &call : calls)
		if (call->state == VOICE_STATE_TALKING)
			return call;

	return NULL;
}

void HoldOtherCalls(VoiceCall *call)
{
	for (auto &other: calls) {
		if (other == call || other->state != VOICE_STATE_TALKING)
			continue;

		if (other->CanHold())
			other->Hold();
		else
			other->Drop();
	}
}

void Answer(VoiceCall *call)
{
	if (!call->CanAnswer())
		return;

	HoldOtherCalls(call);

	// Now annswer it
	call->Answer();
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR Service_CanCall(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = (MCONTACT)wParam;
	if (hContact == NULL)
		return -1;

	hContact = ConvertMetacontact(hContact);
	return CanCall(hContact) ? 1 : 0;
}

static INT_PTR Service_Call(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = (MCONTACT)wParam;
	if (hContact == NULL)
		return -1;

	hContact = ConvertMetacontact(hContact);

	OBJLIST<CallingMethod> methods(10, sttCompareCallingMethods);
	BuildCallingMethodsList(&methods, hContact);

	if (methods.getCount() < 1)
		return -2;

	CallingMethod *method = &methods[0];
	if (!IsEmptyW(method->number))
		return -2;

	method->Call();
	return 0;
}

static INT_PTR CMAnswer(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = (MCONTACT)wParam;
	if (hContact == NULL)
		return -1;

	hContact = ConvertMetacontact(hContact);

	VoiceCall *call = FindVoiceCall(hContact);
	if (call != NULL)
		Answer(call);

	return 0;
}

static INT_PTR CMHold(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = (MCONTACT)wParam;
	if (hContact == NULL)
		return -1;

	hContact = ConvertMetacontact(hContact);

	VoiceCall *call = FindVoiceCall(hContact);
	if (call != NULL)
		call->Hold();

	return 0;
}

static INT_PTR CMDrop(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = (MCONTACT)wParam;
	if (hContact == NULL)
		return -1;

	hContact = ConvertMetacontact(hContact);

	VoiceCall *call = FindVoiceCall(hContact);
	if (call != NULL)
		call->Drop();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Called when all the modules are loaded

int ModulesLoaded(WPARAM, LPARAM)
{
	// add our modules to the KnownModules list
	CallService("DBEditorpp/RegisterSingleModule", (WPARAM)MODULE_NAME, 0);

	// Init icons
	g_plugin.registerIcon(LPGEN("Voice Calls"), mainIcons, "vc");
	g_plugin.registerIcon(LPGEN("Voice Calls"), stateIcons, "vc");
	g_plugin.registerIcon(LPGEN("Voice Calls"), actionIcons, "vca");

	InitOptions();

	// Add menu items
	CMenuItem mi(&g_plugin);
	mi.position = -2000020000;

	SET_UID(mi, 0x3da00738, 0x563, 0x43c9, 0x80, 0x6c, 0x69, 0x42, 0xaf, 0xe8, 0x91, 0x4b);
	mi.name.a = actionIcons[0].szDescr;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_ACTION_CALL);
	mi.pszService = MS_VOICESERVICE_CM_CALL;
	hCMCall = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, Service_Call);

	SET_UID(mi, 0x646b9a8b, 0xf475, 0x4108, 0xb0, 0x57, 0x80, 0x76, 0xa4, 0xf2, 0x5e, 0x9b);
	mi.position++;
	mi.name.a = actionIcons[1].szDescr;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_ACTION_ANSWER);
	mi.pszService = MS_VOICESERVICE_CM_ANSWER;
	hCMAnswer = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, CMAnswer);

	SET_UID(mi, 0x654fadef, 0xb07b, 0x4619, 0x83, 0xc, 0xf7, 0x66, 0x57, 0x60, 0x2a, 0xc8);
	mi.position++;
	mi.name.a = actionIcons[2].szDescr;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_ACTION_HOLD);
	mi.pszService = MS_VOICESERVICE_CM_HOLD;
	hCMHold = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, CMHold);

	SET_UID(mi, 0x18fbd6d6, 0xe1ad, 0x48bd, 0xa8, 0xdb, 0xf, 0x5d, 0x87, 0xe3, 0x14, 0xfe);
	mi.position++;
	mi.name.a = actionIcons[3].szDescr;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_ACTION_DROP);
	mi.pszService = MS_VOICESERVICE_CM_DROP;
	hCMDrop = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, CMDrop);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PreBuildContactMenu);

	// Util services
	CreateServiceFunction(MS_VOICESERVICE_CALL, Service_Call);
	CreateServiceFunction(MS_VOICESERVICE_CAN_CALL, Service_CanCall);

	// Sounds
	for (auto &it : g_sounds)
		g_plugin.addSound(it.szName, LPGENW("Voice Calls"), it.wszDescr);
	g_plugin.addSound("voice_dialpad", LPGENW("Voice Calls"), LPGENW("Dialpad press"));

	// Accounts
	for (auto *pa : Accounts())
		AddAccount(pa);

	HookEvent(ME_PROTO_ACCLISTCHANGED, AccListChanged);
	return 0;
}

int PreShutdown(WPARAM, LPARAM)
{
	DeInitOptions();
	return 0;
}
