/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include "Common.h"

HINSTANCE hInst;
int hLangpack;
LIST<XSN_Data> XSN_Users(10, HandleKeySortT);
HGENMENU hChangeSound = NULL;
HANDLE hChangeSoundDlgList = NULL;
BYTE isIgnoreSound = 0, isOwnSound = 0;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {08B86253-EC6E-4D09-B7A9-64ACDF0627B8}
	{0x8b86253, 0xec6e, 0x4d09, {0xb7, 0xa9, 0x64, 0xac, 0xdf, 0x6, 0x27, 0xb8}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct {
	int iStatus;
	const char *szName;
	const TCHAR *szInfo;
}
static selfSounds[] =
{
	{ ID_STATUS_OFFLINE,	"SelfOffline",		 LPGENT("Offline")			},
	{ ID_STATUS_ONLINE,		"SelfOnline",		 LPGENT("Online")			},
	{ ID_STATUS_AWAY,		"SelfAway",			 LPGENT("Away")				},
	{ ID_STATUS_DND,		"SelfDND",			 LPGENT("Do Not Disturb")	},
	{ ID_STATUS_NA,			"SelfNA",			 LPGENT("Not Available")	},
	{ ID_STATUS_OCCUPIED,  	"SelfOccupied",		 LPGENT("Occupied")			},
	{ ID_STATUS_FREECHAT,	"SelfFreeForChat",	 LPGENT("Free For Chat")	},
	{ ID_STATUS_INVISIBLE,	"SelfInvisible",	 LPGENT("Invisible")		},
	{ ID_STATUS_ONTHEPHONE,	"SelfOnThePhone",	 LPGENT("On The Phone")		},
	{ ID_STATUS_OUTTOLUNCH,	"SelfOutToLunch",	 LPGENT("Out To Lunch")		}
};

void InitSelfSounds()
{
	// initializing self sounds for protocols
	int protoCount=0;
	PROTOACCOUNT** protos = 0;

	CallService(MS_PROTO_ENUMACCOUNTS, (WPARAM)&protoCount, (LPARAM)&protos);
	for (int i = 0; i < protoCount; i++) {
		for(int j = 0; j < SIZEOF(selfSounds); j++) {
			char  namebuf[128];
			mir_snprintf(namebuf, sizeof(namebuf), "%s%s", protos[i]->szModuleName, selfSounds[j].szName);
	
			TCHAR infobuf[256];
			mir_sntprintf(infobuf, SIZEOF(infobuf), _T("%s [%s]"), TranslateT("Self status"), protos[i]->tszAccountName);
			SkinAddNewSoundExT(namebuf, infobuf, selfSounds[j].szInfo);
		}
	}
}

static int ProtoAck(WPARAM wParam, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*) lParam;
	if (ack != 0 && ack->szModule && ack->type == ACKTYPE_STATUS && ack->result == ACKRESULT_SUCCESS) {
		for(int i = 0; i < SIZEOF(selfSounds); i++) {
			if(selfSounds[i].iStatus == ack->lParam) {
				char buf[128];
				_snprintf(buf, sizeof(buf), "%s%s", ack->szModule, selfSounds[i].szName);
				SkinPlaySound(buf);
				break;
			}
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static bool isReceiveMessage(HANDLE hDbEvent)
{
	DBEVENTINFO info = { sizeof(info) };
	db_event_get(hDbEvent, &info);
	// i don't understand why it works and how it works, but it works correctly - practice way (методом тыка)
	// so, i think correct condition would be : eventType == EVENTTYPE_MESSAGE && info.flags & DBEF_READ, but it really isn't
	return !(((info.eventType != EVENTTYPE_MESSAGE) && !(info.flags & DBEF_READ)) || (info.flags & DBEF_SENT));
}

static int ProcessEvent(WPARAM hContact, LPARAM lParam)
{
	if (!isReceiveMessage(HANDLE(lParam)))
		return 0;

	isIgnoreSound = db_get_b(hContact, SETTINGSNAME, SETTINGSIGNOREKEY, 0);
	DBVARIANT dbv;
	if (!isIgnoreSound && !db_get_ts(hContact, SETTINGSNAME, SETTINGSKEY, &dbv)) {
		TCHAR PlaySoundPath[MAX_PATH] = {0};
		PathToAbsoluteT(dbv.ptszVal, PlaySoundPath);
		SkinPlaySoundFile(PlaySoundPath);
		db_free(&dbv);
		isOwnSound = 1;
	}

	return 0;
}

static int OnPlaySound(WPARAM wParam, LPARAM lParam)
{
	if (isIgnoreSound)
		return 1;
	if (isOwnSound) {
		isOwnSound = 0;
		return 1;
	}

	return 0;
}

static int OnLoadInit(WPARAM wParam, LPARAM lParam)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = -0x7FFFFFFF; 
	mi.flags = CMIF_TCHAR; 
	mi.hIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA); 
	mi.ptszName = LPGENT("Custom contact sound"); 
	mi.pszService = "XSoundNotify/ContactMenuCommand"; 
	hChangeSound = Menu_AddContactMenuItem(&mi); 

	InitSelfSounds();
	return 0;
}

static int PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = wParam;
	if (hContact) {
		char* szProto = GetContactProto(hContact);
		PROTOACCOUNT *pa = ProtoGetAccount(szProto);
		Menu_ShowItem(hChangeSound, IsSuitableProto(pa));
	}
	return 0;
}

static int OnPreShutdown(WPARAM wParam, LPARAM lParam)
{
	WindowList_Broadcast(hChangeSoundDlgList, WM_CLOSE, 0, 0);
	return 0;
}

extern "C" int __declspec(dllexport) Load()
{
	mir_getLP(&pluginInfo);

	CreateServiceFunction("XSoundNotify/ContactMenuCommand", ShowDialog);

	hChangeSoundDlgList = WindowList_Create();

	HookEvent(ME_PROTO_ACK, ProtoAck);
	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_DB_EVENT_ADDED, ProcessEvent);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnLoadInit);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);
	HookEvent(ME_SKIN_PLAYINGSOUND, OnPlaySound);
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	WindowList_Destroy(hChangeSoundDlgList);
	return 0;
}
