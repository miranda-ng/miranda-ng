/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include "stdafx.h"

CMPlugin g_plugin;

LIST<XSN_Data> XSN_Users(10, HandleKeySortT);
HGENMENU hChangeSound = nullptr;
MWindowList hChangeSoundDlgList = nullptr;
BYTE isIgnoreSound = 0, isOwnSound = 0, isIgnoreAccSound = 0, isAccSound = 0;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {A01E25F7-A6EF-4B40-8CAC-755A2F2E55B5}
	{ 0xa01e25f7, 0xa6ef, 0x4b40,{ 0x8c, 0xac, 0x75, 0x5a, 0x2f, 0x2e, 0x55, 0xb5 }}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

struct
{
	int iStatus;
	const char *szName;
}
static selfSounds[] =
{
	{ ID_STATUS_OFFLINE, "SelfOffline" },
	{ ID_STATUS_ONLINE, "SelfOnline" },
	{ ID_STATUS_AWAY, "SelfAway" },
	{ ID_STATUS_DND, "SelfDND" },
	{ ID_STATUS_NA, "SelfNA" },
	{ ID_STATUS_OCCUPIED, "SelfOccupied" },
	{ ID_STATUS_FREECHAT, "SelfFreeForChat" },
	{ ID_STATUS_INVISIBLE, "SelfInvisible" }
};

void InitSelfSounds()
{
	// initializing self sounds for protocols
	for (auto &pa : Accounts()) {
		for (int j = 0; j < _countof(selfSounds); j++) {
			char namebuf[128];
			mir_snprintf(namebuf, "%s%s", pa->szModuleName, selfSounds[j].szName);

			wchar_t infobuf[256];
			mir_snwprintf(infobuf, L"%s [%s]", TranslateT("Self status"), pa->tszAccountName);
			g_plugin.addSound(namebuf, infobuf, Clist_GetStatusModeDescription(selfSounds[j].iStatus, 0));
		}
	}
}

static int ProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;
	if (ack != nullptr && ack->szModule && ack->type == ACKTYPE_STATUS && ack->result == ACKRESULT_SUCCESS) {
		for (int i = 0; i < _countof(selfSounds); i++) {
			if (selfSounds[i].iStatus == ack->lParam) {
				char buf[128];
				mir_snprintf(buf, "%s%s", ack->szModule, selfSounds[i].szName);
				Skin_PlaySound(buf);
				break;
			}
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

XSN_Data::XSN_Data(MCONTACT _aContact, bool _ignore) :
	hContact(_aContact),
	bIgnore(_ignore),
	bIsContact(true)
{
	ptrW wszPath(g_plugin.getWStringA(hContact, SETTINGSKEY));
	if (wszPath)
		wcsncpy_s(path, wszPath, _TRUNCATE);
	else
		path[0] = 0;
}

XSN_Data::XSN_Data(const char *szModuleName, bool _ignore) :
	hContact((LPARAM)szModuleName),
	bIgnore(_ignore),
	bIsContact(false)
{
	ptrW wszPath(g_plugin.getWStringA(szModuleName));
	if (wszPath)
		wcsncpy_s(path, wszPath, _TRUNCATE);
	else
		path[0] = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static bool isReceiveMessage(MEVENT hDbEvent)
{
	DBEVENTINFO info = {};
	db_event_get(hDbEvent, &info);
	// i don't understand why it works and how it works, but it works correctly - practice way (методом тыка)
	// so, i think correct condition would be : eventType == EVENTTYPE_MESSAGE && info.flags & DBEF_READ, but it really isn't
	return !(((info.eventType != EVENTTYPE_MESSAGE) && !(info.flags & DBEF_READ)) || (info.flags & DBEF_SENT));
}

static void PlayWorker(MCONTACT hContact)
{
	DBVARIANT dbv;

	isIgnoreSound = g_plugin.getByte(hContact, SETTINGSIGNOREKEY, 0);
	if (!isIgnoreSound && !g_plugin.getWString(hContact, SETTINGSKEY, &dbv)) {
		wchar_t PlaySoundPath[MAX_PATH] = { 0 };
		PathToAbsoluteW(dbv.pwszVal, PlaySoundPath);
		isOwnSound = 0;
		Skin_PlaySoundFile(PlaySoundPath);
		db_free(&dbv);
		isOwnSound = 1;
		return;
	}

	char *szProto = Proto_GetBaseAccountName(hContact);
	PROTOACCOUNT *pa = Proto_GetAccount(szProto);
	size_t value_max_len = mir_strlen(pa->szModuleName) + 8;
	char *value = (char *)mir_alloc(sizeof(char) * value_max_len);
	mir_snprintf(value, value_max_len, "%s_ignore", pa->szModuleName);
	isIgnoreAccSound = g_plugin.getByte(value, 0);
	mir_free(value);
	if (!isIgnoreAccSound && !g_plugin.getWString(pa->szModuleName, &dbv)) {
		wchar_t PlaySoundPath[MAX_PATH] = { 0 };
		PathToAbsoluteW(dbv.pwszVal, PlaySoundPath);
		isAccSound = 0;
		Skin_PlaySoundFile(PlaySoundPath);
		db_free(&dbv);
		isAccSound = 1;
	}
}

static int ProcessEvent(WPARAM hContact, LPARAM lParam)
{
	if (isReceiveMessage(lParam))
		PlayWorker(hContact);

	return 0;
}

static int ProcessChatEvent(WPARAM, LPARAM lParam)
{
	GCEVENT *gce = (GCEVENT*)lParam;
	if (gce == nullptr)
		return 0;
	if (gce->iType != GC_EVENT_MESSAGE)
		return 0;

	auto *si = g_chatApi.SM_FindSession(gce->pszID.w, gce->pszModule);
	if (si && g_chatApi.IsHighlighted(si, gce))
		PlayWorker(si->hContact);

	return 0;
}

static int OnPlaySound(WPARAM, LPARAM)
{
	if (isIgnoreSound || isIgnoreAccSound)
		return 1;
	
	if (isOwnSound) {
		isOwnSound = 0;
		return 1;
	}

	if (isAccSound) {
		isAccSound = 0;
		return 1;
	}

	return 0;
}

static int OnLoadInit(WPARAM, LPARAM)
{
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x5d72ca1f, 0xc52, 0x436d, 0x81, 0x47, 0x29, 0xf6, 0xc3, 0x28, 0xb5, 0xd1);
	mi.position = -0x7FFFFFFF;
	mi.flags = CMIF_UNICODE;
	mi.hIcolibItem = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);
	mi.name.w = LPGENW("Custom contact sound");
	mi.pszService = "XSoundNotify/ContactMenuCommand";
	hChangeSound = Menu_AddContactMenuItem(&mi);

	InitSelfSounds();
	return 0;
}

static int PrebuildContactMenu(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = wParam;
	if (hContact) {
		char *szProto = Proto_GetBaseAccountName(hContact);
		PROTOACCOUNT *pa = Proto_GetAccount(szProto);
		Menu_ShowItem(hChangeSound, IsSuitableProto(pa));
	}
	return 0;
}

static int OnPreShutdown(WPARAM, LPARAM)
{
	WindowList_Broadcast(hChangeSoundDlgList, WM_CLOSE, 0, 0);
	return 0;
}

int CMPlugin::Load()
{
	CreateServiceFunction("XSoundNotify/ContactMenuCommand", ShowDialog);

	hChangeSoundDlgList = WindowList_Create();

	HookEvent(ME_PROTO_ACK, ProtoAck);
	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_DB_EVENT_ADDED, ProcessEvent);
	HookEvent(ME_GC_HOOK_EVENT, ProcessChatEvent);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnLoadInit);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);
	HookEvent(ME_SKIN_PLAYINGSOUND, OnPlaySound);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	WindowList_Destroy(hChangeSoundDlgList);
	return 0;
}
