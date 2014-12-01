/*

Tlen Protocol Plugin for Miranda NG
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
#include "tlen.h"
#include "tlen_muc.h"
#include "tlen_file.h"
#include "tlen_voice.h"
#include "tlen_list.h"
#include "tlen_iq.h"
#include "resource.h"
#include "tlen_picture.h"
#include <richedit.h>
#include <ctype.h>
#include <m_icolib.h>
#include <m_genmenu.h>

CLIST_INTERFACE *pcli;
int hLangpack;
HINSTANCE hInst;
HANDLE hMainThread;

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
	// {748F8934-781A-528D-5208-001265404AB3}
	{0x748f8934, 0x781a, 0x528d, {0x52, 0x08, 0x00, 0x12, 0x65, 0x40, 0x4a, 0xb3}}
};

// Main tlen server connection thread global variables

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpvReserved)
{
#ifdef _DEBUG
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	hInst = hModule;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_PROTOCOL, MIID_LAST};

static IconItem iconList[] =
{
	{ LPGEN("Protocol icon"),          "PROTO",        IDI_TLEN       },
	{ LPGEN("Tlen inbox"),             "MAIL",         IDI_MAIL       },
	{ LPGEN("Group chats"),            "MUC",          IDI_MUC        },
	{ LPGEN("Tlen chats"),             "CHATS",        IDI_CHATS      },
	{ LPGEN("Voice chat"),             "VOICE",        IDI_VOICE      },
	{ LPGEN("Microphone"),             "MICROPHONE",   IDI_MICROPHONE },
	{ LPGEN("Speaker"),                "SPEAKER",      IDI_SPEAKER    },
	{ LPGEN("Send image"),             "IMAGE",        IDI_IMAGE      }
};

static HANDLE GetIconHandle(int iconId)
{
	for (int i = 0; i < SIZEOF(iconList); i++)
		if (iconList[i].defIconID == iconId)
			return iconList[i].hIcolib;

	return NULL;
}

HICON GetIcolibIcon(int iconId)
{
	HANDLE handle = GetIconHandle(iconId);
	if (handle != NULL)
		return Skin_GetIconByHandle(handle);

	return NULL;
}

void ReleaseIcolibIcon(HICON hIcon) {
	Skin_ReleaseIcon(hIcon);
}

static void TlenRegisterIcons()
{
	Icon_Register(hInst, "Protocols/Tlen", iconList, SIZEOF(iconList), "TLEN");
}

int TlenProtocol::PrebuildContactMenu(WPARAM hContact, LPARAM lParam)
{
	if (hContact != NULL && isOnline) {
		DBVARIANT dbv;
		if (!db_get(hContact, m_szModuleName, "jid", &dbv)) {
			TLEN_LIST_ITEM *item = TlenListGetItemPtr(this, LIST_ROSTER, dbv.pszVal);
			db_free(&dbv);
			if (item != NULL) {
				Menu_ShowItem(hMenuContactRequestAuth, item->subscription == SUB_NONE || item->subscription == SUB_FROM);
				Menu_ShowItem(hMenuContactGrantAuth, item->subscription == SUB_NONE || item->subscription == SUB_TO);
				Menu_ShowItem(hMenuContactMUC, item->status != ID_STATUS_OFFLINE);
				Menu_ShowItem(hMenuContactVoice, item->status != ID_STATUS_OFFLINE && !TlenVoiceIsInUse(this));
				Menu_ShowItem(hMenuPicture, item->status != ID_STATUS_OFFLINE);
				return 0;
			}
		}
	}

	Menu_ShowItem(hMenuContactMUC, false);
	Menu_ShowItem(hMenuContactVoice, false);
	Menu_ShowItem(hMenuContactRequestAuth, false);
	Menu_ShowItem(hMenuContactGrantAuth, false);
	return 0;
}

INT_PTR TlenProtocol::ContactMenuHandleRequestAuth(WPARAM hContact, LPARAM lParam)
{
	if (hContact != NULL && isOnline) {
		DBVARIANT dbv;
		if (!db_get(hContact, m_szModuleName, "jid", &dbv)) {
			TlenSend(this, "<presence to='%s' type='subscribe'/>", dbv.pszVal);
			db_free(&dbv);
		}
	}
	return 0;
}

INT_PTR TlenProtocol::ContactMenuHandleGrantAuth(WPARAM hContact, LPARAM lParam)
{
	if (hContact != NULL && isOnline) {
		DBVARIANT dbv;
		if (!db_get(hContact, m_szModuleName, "jid", &dbv)) {
			TlenSend(this, "<presence to='%s' type='subscribed'/>", dbv.pszVal);
			db_free(&dbv);
		}
	}
	return 0;
}

INT_PTR TlenProtocol::ContactMenuHandleSendPicture(WPARAM hContact, LPARAM lParam)
{
	if (hContact != NULL && isOnline)
		SendPicture(this, hContact);
	return 0;
}

INT_PTR TlenProtocol::MenuHandleInbox(WPARAM wParam, LPARAM lParam)
{
	char szFileName[ MAX_PATH ];
	DBVARIANT dbv;
	NETLIBHTTPREQUEST req;
	NETLIBHTTPHEADER headers[2];
	NETLIBHTTPREQUEST *resp;
	char *login = NULL, *password = NULL;
	char form[1024];
	char cookie[1024];
	if (!db_get(NULL, m_szModuleName, "LoginName", &dbv)) {
		login = mir_strdup(dbv.pszVal);
		db_free(&dbv);
	}

	if (db_get_b(NULL, m_szModuleName, "SavePassword", TRUE) == TRUE)
		password = db_get_sa(NULL, m_szModuleName, "Password");
	else if (threadData != NULL && strlen(threadData->password) > 0)
		password = mir_strdup(threadData->password);

	memset(&cookie, 0, sizeof(cookie));
	if (login != NULL && password != NULL) {
		mir_snprintf( form, SIZEOF(form), "username=%s&password=%s", login, password);
		headers[0].szName = "Content-Type";
		headers[0].szValue = "application/x-www-form-urlencoded";
		memset(&req, 0, sizeof(req));
		req.cbSize = sizeof(req);
		req.requestType = REQUEST_POST;
		req.flags = 0;
		req.headersCount = 1;
		req.headers = headers;
		req.pData = form;
		req.dataLength = (int)strlen(form);
		req.szUrl = "http://poczta.o2.pl/login.html";
		resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)&req);
		if (resp != NULL) {
			if (resp->resultCode/100 == 2 || resp->resultCode == 302) {
				int i;
				for (i=0; i<resp->headersCount; i++ ) {
					if (strcmpi(resp->headers[i].szName, "Set-Cookie") == 0) {
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
	mir_snprintf(szFileName, sizeof(szFileName), "http://poczta.o2.pl/login.html?sid=%s", cookie);
	CallService(MS_UTILS_OPENURL, (WPARAM) 1, (LPARAM) szFileName);
	return 0;
}

int TlenProtocol::OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	char str[128];
	/* Set all contacts to offline */

	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
		if (db_get_w(hContact, m_szModuleName, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
			db_set_w(hContact, m_szModuleName, "Status", ID_STATUS_OFFLINE);

	mir_snprintf(str, SIZEOF(str), "%s", LPGEN("Incoming mail"));
	SkinAddNewSoundEx("TlenMailNotify", m_szModuleName, str);
	mir_snprintf(str, SIZEOF(str), "%s", LPGEN("Alert"));
	SkinAddNewSoundEx("TlenAlertNotify", m_szModuleName, str);
	mir_snprintf(str, SIZEOF(str), "%s", LPGEN("Voice chat"));
	SkinAddNewSoundEx("TlenVoiceNotify", m_szModuleName, str);

	HookProtoEvent(ME_USERINFO_INITIALISE, &TlenProtocol::UserInfoInit);
	return 0;
}


int TlenProtocol::PreShutdown(WPARAM wParam, LPARAM lParam)
{
	debugLogA("TLEN TlenPreShutdown");
	return 0;
}

void TlenProtocol::initMenuItems()
{
	char text[_MAX_PATH];
	strncpy_s(text, sizeof(text), m_szModuleName, _TRUNCATE);
	char *pSvcName = text + strlen(text);

	CLISTMENUITEM mi = { sizeof(mi) }, clmi = { sizeof(clmi) };
	clmi.flags = CMIM_FLAGS | CMIF_GRAYED;

	mi.pszContactOwner = m_szModuleName;
	mi.popupPosition = 500090000;

	strcpy(text, m_szModuleName);
	mi.pszService = text;
	mi.ptszName = m_tszUserName;
	mi.position = -1999901009;
	mi.hParentMenu = HGENMENU_ROOT;
	mi.flags = CMIF_ROOTPOPUP | CMIF_TCHAR;
	mi.icolibItem = GetIconHandle(IDI_TLEN);
	hMenuRoot = Menu_AddMainMenuItem(&mi);

	mi.flags = CMIF_CHILDPOPUP;
	//mi.pszPopupName = (char *)hMenuRoot;
	mi.hParentMenu = (HGENMENU)hMenuRoot;

	hMenuChats = NULL;

	// "Multi-User Conference"
	strcpy(pSvcName, "/MainMenuMUC");
	CreateProtoService(pSvcName, &TlenProtocol::MUCMenuHandleMUC);
	mi.pszName = LPGEN("Multi-User Conference");
	mi.position = 2000050002;
	mi.icolibItem = GetIconHandle(IDI_MUC);
	mi.pszService = text;
	hMenuMUC = Menu_AddMainMenuItem(&mi);
	Menu_ModifyItem(hMenuMUC, &clmi);

	strcpy(pSvcName, "/MainMenuInbox");
	CreateProtoService(pSvcName, &TlenProtocol::MenuHandleInbox);
	mi.pszName = LPGEN("Tlen Mail");
	mi.position = 2000050003;
	mi.icolibItem = GetIconHandle(IDI_MAIL);
	mi.pszService = text;
	hMenuInbox = Menu_AddMainMenuItem(&mi);

	mi.hParentMenu = NULL;

	// "Send picture"
	strcpy(pSvcName, "/SendPicture");
	CreateProtoService(pSvcName, &TlenProtocol::ContactMenuHandleSendPicture);
	mi.pszName = LPGEN("Send picture");
	mi.position = -2000019030;
	mi.icolibItem = GetIconHandle(IDI_IMAGE);
	hMenuPicture = Menu_AddContactMenuItem(&mi);

	// "Invite to MUC"
	strcpy(pSvcName, "/ContactMenuMUC");
	CreateProtoService(pSvcName, &TlenProtocol::MUCContactMenuHandleMUC);
	mi.pszName = LPGEN("Multi-User Conference");
	mi.position = -2000019020;
	mi.icolibItem = GetIconHandle(IDI_MUC);
	hMenuContactMUC = Menu_AddContactMenuItem(&mi);

	// "Invite to voice chat"
	strcpy(pSvcName, "/ContactMenuVoice");
	CreateProtoService(pSvcName, &TlenProtocol::VoiceContactMenuHandleVoice);
	mi.pszName = LPGEN("Voice Chat");
	mi.position = -2000019010;
	mi.icolibItem = GetIconHandle(IDI_VOICE);
	hMenuContactVoice = Menu_AddContactMenuItem(&mi);

	// "Request authorization"
	strcpy(pSvcName, "/RequestAuth");
	CreateProtoService(pSvcName, &TlenProtocol::ContactMenuHandleRequestAuth);
	mi.pszName = LPGEN("Request authorization");
	mi.position = -2000001001;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_REQUEST);
	hMenuContactRequestAuth = Menu_AddContactMenuItem(&mi);

	// "Grant authorization"
	strcpy(pSvcName, "/GrantAuth");
	CreateProtoService(pSvcName, &TlenProtocol::ContactMenuHandleGrantAuth);
	mi.pszName = LPGEN("Grant authorization");
	mi.position = -2000001000;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_GRANT);
	hMenuContactGrantAuth = Menu_AddContactMenuItem(&mi);
}

TlenProtocol* tlenProtoInit(const char* pszProtoName, const TCHAR* tszUserName)
{
	TlenProtocol* ppro = new TlenProtocol( pszProtoName, tszUserName );
	return ppro;
}

static int tlenProtoUninit(TlenProtocol* ppro)
{
	delete ppro;
	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP( &pluginInfo );
	mir_getCLI();

	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hMainThread, THREAD_SET_CONTEXT, FALSE, 0);

	srand((unsigned) time(NULL));

	TlenRegisterIcons();

	// Register protocol module
	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = "TLEN";
	pd.fnInit = ( pfnInitProto )tlenProtoInit;
	pd.fnUninit = ( pfnUninitProto )tlenProtoUninit;
	pd.type = PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM) &pd);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
