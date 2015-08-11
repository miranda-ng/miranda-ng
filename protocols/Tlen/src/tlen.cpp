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

#include "stdafx.h"
#include "commons.h"
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

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD, LPVOID)
{
	hInst = hModule;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD)
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
	for (int i = 0; i < _countof(iconList); i++)
		if (iconList[i].defIconID == iconId)
			return iconList[i].hIcolib;

	return NULL;
}

HICON GetIcolibIcon(int iconId)
{
	HANDLE handle = GetIconHandle(iconId);
	if (handle != NULL)
		return IcoLib_GetIconByHandle(handle);

	return NULL;
}

void ReleaseIcolibIcon(HICON hIcon) {
	IcoLib_ReleaseIcon(hIcon);
}

static void TlenRegisterIcons()
{
	Icon_Register(hInst, "Protocols/Tlen", iconList, _countof(iconList), "TLEN");
}

int TlenProtocol::PrebuildContactMenu(WPARAM hContact, LPARAM)
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

INT_PTR TlenProtocol::ContactMenuHandleRequestAuth(WPARAM hContact, LPARAM)
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

INT_PTR TlenProtocol::ContactMenuHandleGrantAuth(WPARAM hContact, LPARAM)
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

INT_PTR TlenProtocol::ContactMenuHandleSendPicture(WPARAM hContact, LPARAM)
{
	if (hContact != NULL && isOnline)
		SendPicture(this, hContact);
	return 0;
}

INT_PTR TlenProtocol::MenuHandleInbox(WPARAM, LPARAM)
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
	else if (threadData != NULL && mir_strlen(threadData->password) > 0)
		password = mir_strdup(threadData->password);

	memset(&cookie, 0, sizeof(cookie));
	if (login != NULL && password != NULL) {
		mir_snprintf( form, _countof(form), "username=%s&password=%s", login, password);
		headers[0].szName = "Content-Type";
		headers[0].szValue = "application/x-www-form-urlencoded";
		memset(&req, 0, sizeof(req));
		req.cbSize = sizeof(req);
		req.requestType = REQUEST_POST;
		req.flags = 0;
		req.headersCount = 1;
		req.headers = headers;
		req.pData = form;
		req.dataLength = (int)mir_strlen(form);
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
								end = resp->headers[i].szValue + mir_strlen(resp->headers[i].szValue);
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
	
	mir_snprintf(szFileName, _countof(szFileName), "http://poczta.o2.pl/login.html?sid=%s", cookie);
	Utils_OpenUrl(szFileName);
	return 0;
}

int TlenProtocol::OnModulesLoaded(WPARAM, LPARAM)
{
	char str[128];
	/* Set all contacts to offline */

	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
		if (db_get_w(hContact, m_szModuleName, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
			db_set_w(hContact, m_szModuleName, "Status", ID_STATUS_OFFLINE);

	strncpy_s(str, LPGEN("Incoming mail"), _TRUNCATE);
	SkinAddNewSoundEx("TlenMailNotify", m_szModuleName, str);
	strncpy_s(str, LPGEN("Alert"), _TRUNCATE);
	SkinAddNewSoundEx("TlenAlertNotify", m_szModuleName, str);
	strncpy_s(str, LPGEN("Voice chat"), _TRUNCATE);
	SkinAddNewSoundEx("TlenVoiceNotify", m_szModuleName, str);

	HookProtoEvent(ME_USERINFO_INITIALISE, &TlenProtocol::UserInfoInit);
	return 0;
}


int TlenProtocol::PreShutdown(WPARAM, LPARAM)
{
	debugLogA("TLEN TlenPreShutdown");
	return 0;
}

void TlenProtocol::initMenuItems()
{
	char text[MAX_PATH];
	strncpy_s(text, m_szModuleName, _TRUNCATE);
	char *pSvcName = text + mir_strlen(text);

	CMenuItem mi;
	mi.root = hMenuRoot = Menu_CreateRoot(MO_MAIN, m_tszUserName, -1999901009, GetIconHandle(IDI_TLEN));
	mi.pszService = text;

	hMenuChats = NULL;

	// "Multi-User Conference"
	mir_strcpy(pSvcName, "/MainMenuMUC");
	CreateProtoService(pSvcName, &TlenProtocol::MUCMenuHandleMUC);
	mi.name.a = LPGEN("Multi-User Conference");
	mi.position = 2000050002;
	mi.hIcolibItem = GetIconHandle(IDI_MUC);
	mi.pszService = text;
	hMenuMUC = Menu_AddMainMenuItem(&mi);

	mir_strcpy(pSvcName, "/MainMenuInbox");
	CreateProtoService(pSvcName, &TlenProtocol::MenuHandleInbox);
	mi.name.a = LPGEN("Tlen Mail");
	mi.position = 2000050003;
	mi.hIcolibItem = GetIconHandle(IDI_MAIL);
	mi.pszService = text;
	hMenuInbox = Menu_AddMainMenuItem(&mi);

	// contact menu items
	mi.root = NULL;

	// "Send picture"
	mi.pszService = "/SendPicture";
	CreateProtoService(mi.pszService, &TlenProtocol::ContactMenuHandleSendPicture);
	mi.name.a = LPGEN("Send picture");
	mi.position = -2000019030;
	mi.hIcolibItem = GetIconHandle(IDI_IMAGE);
	hMenuPicture = Menu_AddContactMenuItem(&mi, m_szModuleName);

	// "Invite to MUC"
	mi.pszService = "/ContactMenuMUC";
	CreateProtoService(mi.pszService, &TlenProtocol::MUCContactMenuHandleMUC);
	mi.name.a = LPGEN("Multi-User Conference");
	mi.position = -2000019020;
	mi.hIcolibItem = GetIconHandle(IDI_MUC);
	hMenuContactMUC = Menu_AddContactMenuItem(&mi, m_szModuleName);

	// "Invite to voice chat"
	mi.pszService = "/ContactMenuVoice";
	CreateProtoService(mi.pszService, &TlenProtocol::VoiceContactMenuHandleVoice);
	mi.name.a = LPGEN("Voice Chat");
	mi.position = -2000019010;
	mi.hIcolibItem = GetIconHandle(IDI_VOICE);
	hMenuContactVoice = Menu_AddContactMenuItem(&mi, m_szModuleName);

	// "Request authorization"
	mi.pszService = "/RequestAuth";
	CreateProtoService(mi.pszService, &TlenProtocol::ContactMenuHandleRequestAuth);
	mi.name.a = LPGEN("Request authorization");
	mi.position = -2000001001;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_AUTH_REQUEST);
	hMenuContactRequestAuth = Menu_AddContactMenuItem(&mi, m_szModuleName);

	// "Grant authorization"
	mi.pszService = "/GrantAuth";
	CreateProtoService(mi.pszService, &TlenProtocol::ContactMenuHandleGrantAuth);
	mi.name.a = LPGEN("Grant authorization");
	mi.position = -2000001000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_AUTH_GRANT);
	hMenuContactGrantAuth = Menu_AddContactMenuItem(&mi, m_szModuleName);
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
	PROTOCOLDESCRIPTOR pd = { 0 };
	pd.cbSize = sizeof(pd);
	pd.szName = "TLEN";
	pd.fnInit = ( pfnInitProto )tlenProtoInit;
	pd.fnUninit = ( pfnUninitProto )tlenProtoUninit;
	pd.type = PROTOTYPE_PROTOCOL;
	Proto_RegisterModule(&pd);
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
