/*

Tlen Protocol Plugin for Miranda IM
Copyright (C) 2002-2004  Santithorn Bunchua
Copyright (C) 2004-2007  Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "commons.h"
#include "jabber.h"
#include "tlen_muc.h"
#include "tlen_file.h"
#include "tlen_voice.h"
#include "jabber_list.h"
#include "jabber_iq.h"
#include "resource.h"
#include "tlen_picture.h"
#include <m_file.h>
#include <richedit.h>
#include <ctype.h>
#include <m_icolib.h>
#include <m_genmenu.h>
#include "m_mucc.h"
#include "m_updater.h"


struct MM_INTERFACE mmi;
struct SHA1_INTERFACE sha1i;
struct MD5_INTERFACE   md5i;
struct UTF8_INTERFACE utfi;
HINSTANCE hInst;
PLUGINLINK *pluginLink;
HANDLE hMainThread;

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
#ifdef _UNICODE
#ifdef _X64
	"Tlen Protocol (x64, Unicode)",
#else
	"Tlen Protocol (Unicode)",
#endif
#else
	"Tlen Protocol",
#endif
	PLUGIN_MAKE_VERSION(TLEN_MAJOR_VERSION,TLEN_MINOR_VERSION,TLEN_RELEASE_NUM,TLEN_BUILD_NUM),
	"Tlen protocol plugin for Miranda IM (version: "TLEN_VERSION_STRING" ; compiled: "__DATE__" "__TIME__")",
	"Santithorn Bunchua, Adam Strzelecki, Piotr Piastucki",
	"the_leech@users.berlios.de",
	"(c) 2002-2012 Santithorn Bunchua, Piotr Piastucki",
	"http://mtlen.berlios.de",
	0,
	0,
#if defined( _UNICODE )
#ifdef _X64
	{0x3ba4a3c6, 0x27b5, 0x4c78, { 0x92, 0x8c, 0x38, 0x2a, 0xe1, 0xd3, 0x2a, 0xb5 }}
#else
	{0x748f8934, 0x781a, 0x528d, { 0x52, 0x08, 0x00, 0x12, 0x65, 0x40, 0x4a, 0xb3 }}
#endif
#else
    {0x11fc3484, 0x475c, 0x11dc, { 0x83, 0x14, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66 }}
#endif
};

// Main jabber server connection thread global variables

void TlenLoadOptions(TlenProtocol *proto);
int TlenOptionsInit(void *ptr, WPARAM wParam, LPARAM lParam);
int TlenUserInfoInit(void *ptr, WPARAM wParam, LPARAM lParam);
int TlenSystemModulesLoaded(void *ptr, WPARAM wParam, LPARAM lParam);
extern void TlenInitServicesVTbl(TlenProtocol *proto);
extern int JabberContactDeleted(void *ptr, WPARAM wParam, LPARAM lParam);
extern int JabberDbSettingChanged(void *ptr, WPARAM wParam, LPARAM lParam);

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpvReserved)
{
#ifdef _DEBUG
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	hInst = hModule;
	return TRUE;
}

__declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx( DWORD mirandaVersion )
{
	if ( mirandaVersion < PLUGIN_MAKE_VERSION( 0,8,0,15 )) {
		MessageBox( NULL, TranslateT("The Tlen protocol plugin cannot be loaded. It requires Miranda IM 0.8.0.15 or later."), TranslateT("Tlen Protocol Plugin"), MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST );
		return NULL;
	}

	return &pluginInfoEx;
}

static const MUUID interfaces[] = {MIID_PROTOCOL, MIID_LAST};
__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

struct
{
	TCHAR*  szDescr;
	char*  szName;
	int    defIconID;
	HANDLE hIconLibItem;
}
static iconList[] =
{
	{ _T("Protocol icon"),		"PROTO",		IDI_TLEN		},
	{ _T("Tlen inbox"),					"MAIL",		IDI_MAIL		},
	{ _T("Group chats"),			"MUC",		IDI_MUC 		},
	{ _T("Tlen chats"),			"CHATS",		IDI_CHATS		},
	{ _T("Grant authorization"),	"GRANT",		IDI_GRANT		},
	{ _T("Request authorization"),"REQUEST",		IDI_REQUEST	},
	{ _T("Voice chat"),			"VOICE",		IDI_VOICE		},
	{ _T("Microphone"),			"MICROPHONE",	IDI_MICROPHONE	},
	{ _T("Speaker"),				"SPEAKER",		IDI_SPEAKER	},
	{ _T("Send image"),				"IMAGE",		IDI_IMAGE	}
};

static HANDLE GetIconHandle(int iconId) {
	int i;
	for(i = 0; i < SIZEOF(iconList); i++)
		if(iconList[i].defIconID == iconId)
			return iconList[i].hIconLibItem;
	return NULL;
}

HICON GetIcolibIcon(int iconId) {
	HANDLE handle = GetIconHandle(iconId);
	if (handle != NULL) {
		return (HICON) CallService(MS_SKIN2_GETICONBYHANDLE, 0, (LPARAM)handle);
	}
	return NULL;
}

void ReleaseIcolibIcon(HICON hIcon) {
	CallService( MS_SKIN2_RELEASEICON, (WPARAM)hIcon, 0 );
}

/*
static int TlenIconsChanged(void *ptr, WPARAM wParam, LPARAM lParam)
{
	return 0;
}
*/

static void TlenRegisterIcons()
{
    SKINICONDESC sid = { 0 };
    char path[MAX_PATH];
	TCHAR szSectionName[100];
	int i;
	mir_sntprintf(szSectionName, SIZEOF( szSectionName ), _T("%s/%s"), _T("Protocols"), _T("Tlen"));

    GetModuleFileNameA(hInst, path, MAX_PATH);
    sid.cbSize = sizeof(SKINICONDESC);
    sid.cx = sid.cy = 16;
    sid.ptszSection = szSectionName;
    sid.pszDefaultFile = path;
    sid.flags = SIDF_TCHAR;

	for ( i = 0; i < SIZEOF(iconList); i++ ) {
		char szSettingName[100];
		mir_snprintf( szSettingName, sizeof( szSettingName ), "TLEN_%s", iconList[i].szName );
		sid.pszName = szSettingName;
		sid.pszDescription = (char*)iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].defIconID;
		iconList[i].hIconLibItem = ( HANDLE )CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
	}
}

static int TlenPrebuildContactMenu(void *ptr, WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact;
	DBVARIANT dbv;
	CLISTMENUITEM clmi = {0};
	JABBER_LIST_ITEM *item;
    TlenProtocol *proto = (TlenProtocol *)ptr;
	clmi.cbSize = sizeof(CLISTMENUITEM);
	if ((hContact=(HANDLE) wParam)!=NULL && proto->isOnline) {
		if (!DBGetContactSetting(hContact, proto->iface.m_szModuleName, "jid", &dbv)) {
			if ((item=JabberListGetItemPtr(proto, LIST_ROSTER, dbv.pszVal)) != NULL) {
				if (item->subscription==SUB_NONE || item->subscription==SUB_FROM)
					clmi.flags = CMIM_FLAGS;
				else
					clmi.flags = CMIM_FLAGS|CMIF_HIDDEN;
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) proto->hMenuContactRequestAuth, (LPARAM) &clmi);

				if (item->subscription==SUB_NONE || item->subscription==SUB_TO)
					clmi.flags = CMIM_FLAGS;
				else
					clmi.flags = CMIM_FLAGS|CMIF_HIDDEN;
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) proto->hMenuContactGrantAuth, (LPARAM) &clmi);

				if (item->status!=ID_STATUS_OFFLINE)
					clmi.flags = CMIM_FLAGS;
				else
					clmi.flags = CMIM_FLAGS|CMIF_HIDDEN;
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) proto->hMenuContactMUC, (LPARAM) &clmi);

				if (item->status!=ID_STATUS_OFFLINE && !TlenVoiceIsInUse(proto))
					clmi.flags = CMIM_FLAGS;
				else
					clmi.flags = CMIM_FLAGS|CMIF_HIDDEN;
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) proto->hMenuContactVoice, (LPARAM) &clmi);

				if (item->status!=ID_STATUS_OFFLINE)
					clmi.flags = CMIM_FLAGS;
				else
					clmi.flags = CMIM_FLAGS|CMIF_HIDDEN;
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) proto->hMenuPicture, (LPARAM) &clmi);

				DBFreeVariant(&dbv);
				return 0;
			}
			DBFreeVariant(&dbv);
		}
	}
	clmi.flags = CMIM_FLAGS|CMIF_HIDDEN;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) proto->hMenuContactMUC, (LPARAM) &clmi);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) proto->hMenuContactVoice, (LPARAM) &clmi);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) proto->hMenuContactRequestAuth, (LPARAM) &clmi);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) proto->hMenuContactGrantAuth, (LPARAM) &clmi);
	return 0;
}

INT_PTR TlenContactMenuHandleRequestAuth(void *ptr, WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact;
	DBVARIANT dbv;
    TlenProtocol *proto = (TlenProtocol *)ptr;
	if ((hContact=(HANDLE) wParam)!=NULL && proto->isOnline) {
		if (!DBGetContactSetting(hContact, proto->iface.m_szModuleName, "jid", &dbv)) {
			JabberSend(proto, "<presence to='%s' type='subscribe'/>", dbv.pszVal);
			DBFreeVariant(&dbv);
		}
	}
	return 0;
}

INT_PTR TlenContactMenuHandleGrantAuth(void *ptr, WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact;
	DBVARIANT dbv;
    TlenProtocol *proto = (TlenProtocol *)ptr;
	if ((hContact=(HANDLE) wParam)!=NULL && proto->isOnline) {
		if (!DBGetContactSetting(hContact, proto->iface.m_szModuleName, "jid", &dbv)) {
			JabberSend(proto, "<presence to='%s' type='subscribed'/>", dbv.pszVal);
			DBFreeVariant(&dbv);
		}
	}
	return 0;
}

INT_PTR TlenContactMenuHandleSendPicture(void *ptr, WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact;
    TlenProtocol *proto = (TlenProtocol *)ptr;
	if ((hContact=(HANDLE) wParam)!=NULL && proto->isOnline) {
        SendPicture(proto, hContact);
	}
	return 0;
}

INT_PTR TlenMenuHandleInbox(void *ptr, WPARAM wParam, LPARAM lParam)
{
	char szFileName[ MAX_PATH ];
	DBVARIANT dbv;
	NETLIBHTTPREQUEST req;
	NETLIBHTTPHEADER headers[2];
    NETLIBHTTPREQUEST *resp;
	char *login = NULL, *password = NULL;
	char form[1024];
	char cookie[1024];
    TlenProtocol *proto = (TlenProtocol *)ptr;
	if (!DBGetContactSetting(NULL, proto->iface.m_szModuleName, "LoginName", &dbv)) {
		login = mir_strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}
    if (DBGetContactSettingByte(NULL, proto->iface.m_szModuleName, "SavePassword", TRUE) == TRUE) {
		if (!DBGetContactSetting(NULL, proto->iface.m_szModuleName, "Password", &dbv)) {
			CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal)+1, (LPARAM) dbv.pszVal);
			password = mir_strdup(dbv.pszVal);
			DBFreeVariant(&dbv);
		}
	} else if (proto->threadData != NULL && strlen(proto->threadData->password) > 0) {
		password = mir_strdup(proto->threadData->password);
	}

	ZeroMemory(&cookie, sizeof(cookie));
	if (login != NULL && password != NULL) {
		mir_snprintf( form, SIZEOF(form), "username=%s&password=%s", login, password);
		headers[0].szName = "Content-Type";
		headers[0].szValue = "application/x-www-form-urlencoded";
		ZeroMemory(&req, sizeof(req));
		req.cbSize = sizeof(req);
		req.requestType = REQUEST_POST;
		req.flags = 0;
		req.headersCount = 1;
		req.headers = headers;
		req.pData = form;
		req.dataLength = (int)strlen(form);
		req.szUrl = "http://poczta.o2.pl/login.html";
		resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)proto->hNetlibUser, (LPARAM)&req);
		if (resp != NULL) {
			if (resp->resultCode/100==2 || resp->resultCode == 302) {
				int i;
				for (i=0; i<resp->headersCount; i++ ) {
					if (strcmpi(resp->headers[i].szName, "Set-Cookie")==0) {
						char *start = strstr(resp->headers[i].szValue, "ssid=");
						if (start != NULL) {
							char *end = strstr(resp->headers[i].szValue, ";");
							start = start + 5;
							if (end == NULL) {
								end = resp->headers[i].szValue + strlen(resp->headers[i].szValue);
							}
							strncpy(cookie, start, (end - start));
							break;
						}
					}
				}
			}
			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
		}
	}
	mir_free(login);
	mir_free(password);
	_snprintf(szFileName, sizeof(szFileName), "http://poczta.o2.pl/login.html?sid=%s", cookie);
	CallService(MS_UTILS_OPENURL, (WPARAM) 1, (LPARAM) szFileName);
	return 0;
}

int TlenOnModulesLoaded(void *ptr, WPARAM wParam, LPARAM lParam) {

	char str[128];
    TlenProtocol *proto = (TlenProtocol *)ptr;
    /* Set all contacts to offline */
	HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact != NULL) {
		char *szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
		if(szProto!=NULL && !strcmp(szProto, proto->iface.m_szModuleName)) {
			if (DBGetContactSettingWord(hContact, proto->iface.m_szModuleName, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE) {
				DBWriteContactSettingWord(hContact, proto->iface.m_szModuleName, "Status", ID_STATUS_OFFLINE);
			}
		}
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}
	TlenMUCInit(proto);
	sprintf(str, "%s", Translate("Incoming mail"));
	SkinAddNewSoundEx("TlenMailNotify", proto->iface.m_szProtoName, str);
	sprintf(str, "%s", Translate("Alert"));
	SkinAddNewSoundEx("TlenAlertNotify", proto->iface.m_szProtoName, str);
	sprintf(str, "%s", Translate("Voice chat"));
	SkinAddNewSoundEx("TlenVoiceNotify", proto->iface.m_szProtoName, str);

    HookEventObj_Ex(ME_USERINFO_INITIALISE, proto, TlenUserInfoInit);

    return 0;
}

int TlenPreShutdown(void *ptr, WPARAM wParam, LPARAM lParam)
{
	JabberLog(ptr, "TLEN TlenPreShutdown");
    //TlenProtocol *proto = (TlenProtocol *)ptr;
	return 0;
}


static void initMenuItems(TlenProtocol *proto)
{

	char text[_MAX_PATH];
	CLISTMENUITEM mi, clmi;
	memset(&mi, 0, sizeof(CLISTMENUITEM));
	mi.cbSize = sizeof(CLISTMENUITEM);
	memset(&clmi, 0, sizeof(CLISTMENUITEM));
	clmi.cbSize = sizeof(CLISTMENUITEM);
	clmi.flags = CMIM_FLAGS | CMIF_GRAYED;

	mi.pszContactOwner = proto->iface.m_szModuleName;
	mi.popupPosition = 500090000;

	strcpy(text, proto->iface.m_szModuleName);
	mi.pszService = text;
	mi.ptszName = proto->iface.m_tszUserName;
	mi.position = -1999901009;
	mi.pszPopupName = (char *)-1;
	mi.flags = CMIF_ROOTPOPUP | CMIF_TCHAR | CMIF_ICONFROMICOLIB;
	mi.icolibItem = GetIconHandle(IDI_TLEN);
	proto->hMenuRoot = (HANDLE)CallService( MS_CLIST_ADDMAINMENUITEM,  (WPARAM)0, (LPARAM)&mi);

    mi.flags = CMIF_CHILDPOPUP | CMIF_ICONFROMICOLIB;
    //mi.pszPopupName = (char *)proto->hMenuRoot;
    mi.hParentMenu = (HGENMENU)proto->hMenuRoot;

    proto->hMenuChats = NULL;
    if(ServiceExists(MS_MUCC_NEW_WINDOW))
	{
    	sprintf(text, "%s/MainMenuChats", proto->iface.m_szModuleName);
		CreateServiceFunction_Ex(text, proto, TlenMUCMenuHandleChats);
		mi.pszName = "Tlen Chats";
		mi.position = 2000050001;
		mi.icolibItem = GetIconHandle(IDI_CHATS);
		mi.pszService = text;
		proto->hMenuChats = (HANDLE) CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM) &mi);
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) proto->hMenuChats, (LPARAM) &clmi);
	}

	// "Multi-User Conference"
	sprintf(text, "%s/MainMenuMUC", proto->iface.m_szModuleName);
	CreateServiceFunction_Ex(text, proto, TlenMUCMenuHandleMUC);
	mi.pszName = "Multi-User Conference";
	mi.position = 2000050002;
	mi.icolibItem = GetIconHandle(IDI_MUC);
	mi.pszService = text;
	proto->hMenuMUC = (HANDLE) CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM) &mi);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) proto->hMenuMUC, (LPARAM) &clmi);

	sprintf(text, "%s/MainMenuInbox", proto->iface.m_szModuleName);
	CreateServiceFunction_Ex(text, proto, TlenMenuHandleInbox);
	mi.pszName = "Tlen Mail";
	mi.position = 2000050003;
	mi.icolibItem = GetIconHandle(IDI_MAIL);
	mi.pszService = text;
	proto->hMenuInbox = (HANDLE) CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM) &mi);

	mi.hParentMenu = NULL;


	// "Send picture"
	sprintf(text, "%s/SendPicture", proto->iface.m_szModuleName);
	CreateServiceFunction_Ex(text, proto, TlenContactMenuHandleSendPicture);
	mi.flags = CMIF_ICONFROMICOLIB;
	mi.pszName = "Send picture";
	mi.position = -2000019030;
	mi.icolibItem = GetIconHandle(IDI_IMAGE);
	mi.pszService = text;
	proto->hMenuPicture = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) &mi);

	// "Invite to MUC"
	sprintf(text, "%s/ContactMenuMUC", proto->iface.m_szModuleName);
	CreateServiceFunction_Ex(text, proto, TlenMUCContactMenuHandleMUC);
	mi.pszName = "Multi-User Conference";
	mi.position = -2000019020;
	mi.icolibItem = GetIconHandle(IDI_MUC);
	mi.pszService = text;
	proto->hMenuContactMUC = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) &mi);

	// "Invite to voice chat"
	sprintf(text, "%s/ContactMenuVoice", proto->iface.m_szModuleName);
	CreateServiceFunction_Ex(text, proto, TlenVoiceContactMenuHandleVoice);
	mi.pszName = "Voice Chat";
	mi.position = -2000019010;
	mi.icolibItem = GetIconHandle(IDI_VOICE);
	mi.pszService = text;
	proto->hMenuContactVoice = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) &mi);

	// "Request authorization"
	sprintf(text, "%s/RequestAuth", proto->iface.m_szModuleName);
	CreateServiceFunction_Ex(text, proto, TlenContactMenuHandleRequestAuth);
	mi.pszName = "Request authorization";
	mi.position = -2000001001;
	mi.icolibItem = GetIconHandle(IDI_REQUEST);
	mi.pszService = text;
	proto->hMenuContactRequestAuth = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) &mi);

	// "Grant authorization"
	sprintf(text, "%s/GrantAuth", proto->iface.m_szModuleName);
	CreateServiceFunction_Ex(text, proto, TlenContactMenuHandleGrantAuth);
	mi.pszName = "Grant authorization";
	mi.position = -2000001000;
	mi.icolibItem = GetIconHandle(IDI_GRANT);
	mi.pszService = text;
	proto->hMenuContactGrantAuth = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) &mi);
}

static void uninitMenuItems(TlenProtocol *proto) {
	CallService(MS_CLIST_REMOVEMAINMENUITEM, (WPARAM)proto->hMenuChats, (LPARAM) 0);
	CallService(MS_CLIST_REMOVEMAINMENUITEM, (WPARAM)proto->hMenuMUC, (LPARAM) 0);
	CallService(MS_CLIST_REMOVEMAINMENUITEM, (WPARAM)proto->hMenuInbox, (LPARAM) 0);
	CallService(MS_CLIST_REMOVEMAINMENUITEM, (WPARAM)proto->hMenuRoot, (LPARAM) 0);
	CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)proto->hMenuContactMUC, (LPARAM) 0);
	CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)proto->hMenuPicture, (LPARAM) 0);
	CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)proto->hMenuContactVoice, (LPARAM) 0);
	CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)proto->hMenuContactRequestAuth, (LPARAM) 0);
	CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)proto->hMenuContactGrantAuth, (LPARAM) 0);
}


static TlenProtocol *tlenProtoInit( const char* pszProtoName, const TCHAR* tszUserName )
{
	DBVARIANT dbv;
    char text[_MAX_PATH];
	TlenProtocol *proto = (TlenProtocol *)mir_alloc(sizeof(TlenProtocol));
    memset(proto, 0, sizeof(TlenProtocol));
    proto->iface.m_tszUserName = mir_tstrdup( tszUserName );
    proto->iface.m_szModuleName = mir_strdup(pszProtoName);
    proto->iface.m_szProtoName = mir_strdup(pszProtoName);
    _strlwr( proto->iface.m_szProtoName );
	proto->iface.m_szProtoName[0] = toupper( proto->iface.m_szProtoName[0] );
    proto->iface.m_iStatus = ID_STATUS_OFFLINE;
    TlenInitServicesVTbl(proto);

	InitializeCriticalSection(&proto->modeMsgMutex);
	InitializeCriticalSection(&proto->csSend);

	sprintf(text, "%s/%s", proto->iface.m_szModuleName, "Nudge");
	proto->hTlenNudge = CreateHookableEvent(text);

	HookEventObj_Ex(ME_SYSTEM_MODULESLOADED, proto, TlenSystemModulesLoaded);
	HookEventObj_Ex(ME_OPT_INITIALISE, proto, TlenOptionsInit);
	HookEventObj_Ex(ME_DB_CONTACT_SETTINGCHANGED, proto, JabberDbSettingChanged);
	HookEventObj_Ex(ME_DB_CONTACT_DELETED, proto, JabberContactDeleted);
	HookEventObj_Ex(ME_CLIST_PREBUILDCONTACTMENU, proto, TlenPrebuildContactMenu);
//	HookEventObj_Ex(ME_SKIN2_ICONSCHANGED, proto, TlenIconsChanged);
	HookEventObj_Ex(ME_SYSTEM_PRESHUTDOWN, proto, TlenPreShutdown);

	if (!DBGetContactSetting(NULL, proto->iface.m_szModuleName, "LoginServer", &dbv)) {
		DBFreeVariant(&dbv);
	} else {
		DBWriteContactSettingString(NULL, proto->iface.m_szModuleName, "LoginServer", "tlen.pl");
	}
	if (!DBGetContactSetting(NULL, proto->iface.m_szModuleName, "ManualHost", &dbv)) {
		DBFreeVariant(&dbv);
	} else {
		DBWriteContactSettingString(NULL, proto->iface.m_szModuleName, "ManualHost", "s1.tlen.pl");
	}

	TlenLoadOptions(proto);

    JabberWsInit(proto);
	JabberSerialInit(proto);
	JabberIqInit(proto);
	JabberListInit(proto);

    return proto;
}

int TlenSystemModulesLoaded(void *ptr, WPARAM wParam, LPARAM lParam)
{

    TlenProtocol *proto = (TlenProtocol *)ptr;
    initMenuItems(proto);

    // updater plugin support
    if(ServiceExists(MS_UPDATE_REGISTERFL))
	{
    	PLUGININFO* pluginInfoPtr = (PLUGININFO*)&pluginInfoEx;
		#ifdef _UNICODE
		#ifdef _X64
		CallService(MS_UPDATE_REGISTERFL, (WPARAM)4435, (LPARAM)pluginInfoPtr); //x64 version
		#else
		CallService(MS_UPDATE_REGISTERFL, (WPARAM)4405, (LPARAM)pluginInfoPtr); //Unicode version
		#endif
		#else
		CallService(MS_UPDATE_REGISTERFL, (WPARAM)4404, (LPARAM)pluginInfoPtr); //ANSI version
		#endif
	}

	return 0;
}

static int tlenProtoUninit( TlenProtocol *proto )
{
	uninitMenuItems(proto);
	TlenVoiceCancelAll(proto);
	TlenFileCancelAll(proto);
	if (proto->hTlenNudge)
		DestroyHookableEvent(proto->hTlenNudge);
    UnhookEvents_Ex(proto);
	JabberListUninit(proto);
	JabberIqUninit(proto);
	JabberSerialUninit(proto);
	DeleteCriticalSection(&proto->modeMsgMutex);
    DeleteCriticalSection(&proto->csSend);
    //DestroyServices_Ex(proto);
	JabberWsUninit(proto);
	mir_free(proto->modeMsgs.szOnline);
	mir_free(proto->modeMsgs.szAway);
	mir_free(proto->modeMsgs.szNa);
	mir_free(proto->modeMsgs.szDnd);
	mir_free(proto->modeMsgs.szFreechat);
	mir_free(proto->modeMsgs.szInvisible);
    mir_free(proto);
 	return 0;
}

int __declspec(dllexport) Load(PLUGINLINK *link)
{
	PROTOCOLDESCRIPTOR pd;

	pluginLink = link;
	mir_getMMI( &mmi );
    mir_getMD5I( &md5i );
    mir_getSHA1I( &sha1i );
	mir_getUTFI( &utfi );

	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hMainThread, THREAD_SET_CONTEXT, FALSE, 0);

    srand((unsigned) time(NULL));

	TlenRegisterIcons();

	// Register protocol module
	ZeroMemory(&pd, sizeof(PROTOCOLDESCRIPTOR));
	pd.cbSize = sizeof(PROTOCOLDESCRIPTOR);
	pd.szName = "TLEN";
    pd.fnInit = ( pfnInitProto )tlenProtoInit;
	pd.fnUninit = ( pfnUninitProto )tlenProtoUninit;
	pd.type = PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM) &pd);

	return 0;
}

int __declspec(dllexport) Unload(void)
{
	return 0;
}

