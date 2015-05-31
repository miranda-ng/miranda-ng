/*
Paste It plugin
Copyright (C) 2011 Krzysztof Kral

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

PasteToWeb* pasteToWebs[PasteToWeb::pages];
std::map<MCONTACT, HWND>* contactWindows;
DWORD gMirandaVersion;

HANDLE hModulesLoaded, hTabsrmmButtonPressed;
HANDLE g_hNetlibUser;
HANDLE hPrebuildContactMenu;
HANDLE hServiceContactMenu;
HGENMENU hContactMenu;
HGENMENU hWebPageMenus[PasteToWeb::pages];
HANDLE hOptionsInit;
HANDLE hWindowEvent = NULL;
HINSTANCE hInst;

#define FROM_CLIPBOARD 10
#define FROM_FILE 11
#define DEF_PAGES_START 20
#define MS_PASTEIT_CONTACTMENU	"PasteIt/ContactMenu"

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
	// {1AAC15E8-DCEC-4050-B66F-2AA0E6120C22}
	{ 0x1aac15e8, 0xdcec, 0x4050, { 0xb6, 0x6f, 0x2a, 0xa0, 0xe6, 0x12, 0xc, 0x22 } }
};

static IconItem icon = { LPGEN("Paste It"), "PasteIt_main", IDI_MENU };

XML_API xi = { 0 };
int hLangpack = 0;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	hInst = hModule;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	gMirandaVersion = mirandaVersion;
	return &pluginInfo;
}


std::wstring GetFile()
{
	TCHAR filter[512];
	mir_tstrncpy(filter, TranslateT("All Files (*.*)"), SIZEOF(filter));
	memcpy(filter + mir_tstrlen(filter), _T("\0*.*\0"), 6 * sizeof(TCHAR));
	TCHAR stzFilePath[1024];
	stzFilePath[0] = 0;
	stzFilePath[1] = 0;
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = 0;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = stzFilePath;
	ofn.lpstrTitle = TranslateT("Paste It - Select file");
	ofn.nMaxFile = SIZEOF(stzFilePath);
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR;
	if (GetOpenFileName(&ofn))
	{
		return stzFilePath;
	}

	return L"";
}

void PasteIt(MCONTACT hContact, int mode)
{
	PasteToWeb* pasteToWeb = pasteToWebs[Options::instance->defWeb];
	if (mode == FROM_CLIPBOARD)
	{
		pasteToWeb->FromClipboard();
	}
	else if (mode == FROM_FILE)
	{
		std::wstring file = GetFile();
		if (file.length() > 0)
		{
			pasteToWeb->FromFile(file);
		}
		else return;
	}
	else return;

	if (pasteToWeb->szFileLink[0] == 0 && pasteToWeb->error != NULL)
	{
		MessageBox(NULL, pasteToWeb->error, TranslateT("Error"), MB_OK | MB_ICONERROR);
	}
	else if (hContact != NULL && pasteToWeb->szFileLink[0] != 0)
	{
		char *szProto = GetContactProto(hContact);
		if (szProto && (INT_PTR)szProto != CALLSERVICE_NOTFOUND)
		{
			BOOL isChat = db_get_b(hContact, szProto, "ChatRoom", 0);
			if (Options::instance->autoSend)
			{
				if (!isChat)
				{
					DBEVENTINFO dbei = { 0 };
					dbei.cbSize = sizeof(dbei);
					dbei.eventType = EVENTTYPE_MESSAGE;
					dbei.flags = DBEF_SENT;
					dbei.szModule = szProto;
					dbei.timestamp = (DWORD)time(NULL);
					dbei.cbBlob = (DWORD)mir_strlen(pasteToWeb->szFileLink) + 1;
					dbei.pBlob = (PBYTE)pasteToWeb->szFileLink;
					db_event_add(hContact, &dbei);
					CallContactService(hContact, PSS_MESSAGE, 0, (LPARAM)pasteToWeb->szFileLink);
				}
				else
				{
					// PSS_MESSAGE is not compatible with chat rooms
					// there are no simple method to send text to all users
					// in chat room. 
					// Next step is to get all protocol sessions and find
					// one with correct hContact 
					GC_INFO gci = { 0 };
					GCDEST  gcd = { szProto, NULL, GC_EVENT_SENDMESSAGE };
					GCEVENT gce = { sizeof(gce), &gcd };
					int cnt = (int)CallService(MS_GC_GETSESSIONCOUNT, 0, (LPARAM)szProto);
					for (int i = 0; i < cnt; i++)
					{
						gci.iItem = i;
						gci.pszModule = szProto;
						gci.Flags = GCF_BYINDEX | GCF_HCONTACT | GCF_ID;
						CallService(MS_GC_GETINFO, 0, (LPARAM)&gci);
						if (gci.hContact == hContact)
						{
							// In this place session was finded, gci.pszID contains
							// session ID, but it is in unicode or ascii format,
							// depends on protocol wersion
							gcd.ptszID = gci.pszID;
							gce.bIsMe = TRUE;
							gce.dwFlags = GCEF_ADDTOLOG;
							gce.ptszText = mir_a2u_cp(pasteToWeb->szFileLink, CP_ACP);
							gce.time = time(NULL);
							CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
							mir_free((void*)gce.ptszText);
							break;
						}
					}
				}

				// Send message to focus window
				CallServiceSync(MS_MSG_SENDMESSAGE, hContact, 0);
			}
			else
			{
				if (isChat)
				{
					// MS_MSG_SENDMESSAGE in incompatible with chat rooms,
					// because it sends text to IDC_MESSAGE window,
					// but in chat rooms is only IDC_CHAT_MESSAGE window.
					// contactWindows map contains all opened hContact
					// with assaigned to them chat windows. 
					// This map is prepared in ME_MSG_WINDOWEVENT event. 
					std::map<MCONTACT, HWND>::iterator it = contactWindows->find(hContact);
					if (it != contactWindows->end())
					{
						// it->second is imput window, so now I can send to them 
						// new text. Afterr all is sended MS_MSG_SENDMESSAGE 
						// to focus window.
						SendMessage(it->second, EM_SETSEL, -1, GetWindowTextLength(it->second));
						SendMessageA(it->second, EM_REPLACESEL, FALSE, (LPARAM)pasteToWeb->szFileLink);
						CallServiceSync(MS_MSG_SENDMESSAGE, hContact, NULL);
					}
					else
					{
						// If window do not exist, maybe it is not chat
						CallServiceSync(MS_MSG_SENDMESSAGE, hContact, (LPARAM)pasteToWeb->szFileLink);
					}
				}
				else
				{
					CallServiceSync(MS_MSG_SENDMESSAGE, hContact, (LPARAM)pasteToWeb->szFileLink);
				}
			}
		}
	}
}

int TabsrmmButtonPressed(WPARAM hContact, LPARAM lParam)
{
	CustomButtonClickData *cbc = (CustomButtonClickData *)lParam;

	if (!mir_strcmp(cbc->pszModule, MODULE) && cbc->dwButtonId == 1 && hContact)
	{
		if (cbc->flags == BBCF_ARROWCLICKED)
		{
			HMENU hMenu = CreatePopupMenu();
			if (hMenu)
			{
				AppendMenu(hMenu, MF_STRING, FROM_CLIPBOARD, TranslateT("Paste from clipboard"));
				AppendMenu(hMenu, MF_STRING, FROM_FILE, TranslateT("Paste from file"));
				HMENU hDefMenu = CreatePopupMenu();
				for (int i = 0; i < PasteToWeb::pages; ++i)
				{
					UINT flags = MF_STRING;
					if (Options::instance->defWeb == i)
						flags |= MF_CHECKED;

					AppendMenu(hDefMenu, flags, DEF_PAGES_START + i, pasteToWebs[i]->GetName());
				}

				AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hDefMenu, TranslateT("Default web page"));

				POINT pt;
				GetCursorPos(&pt);
				HWND hwndBtn = WindowFromPoint(pt);
				if (hwndBtn)
				{
					RECT rc;
					GetWindowRect(hwndBtn, &rc);
					SetForegroundWindow(cbc->hwndFrom);
					int selected = TrackPopupMenu(hMenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, cbc->hwndFrom, 0);
					if (selected != 0)
					{
						if (selected >= DEF_PAGES_START)
						{
							Options::instance->SetDefWeb(selected - DEF_PAGES_START);
						}
						else
						{
							PasteIt(hContact, selected);
						}
					}
				}

				DestroyMenu(hDefMenu);
				DestroyMenu(hMenu);
			}
		}
		else
		{
			PasteIt(hContact, FROM_CLIPBOARD);
		}
	}

	return 0;
}

int PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	bool bIsContact = false;

	char *szProto = GetContactProto(wParam);
	if (szProto && (INT_PTR)szProto != CALLSERVICE_NOTFOUND)
		bIsContact = (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IM) != 0;

	Menu_ShowItem(hContactMenu, bIsContact);
	return 0;
}

INT_PTR ContactMenuService(WPARAM hContact, LPARAM lParam)
{
	if (lParam >= DEF_PAGES_START)
		Options::instance->SetDefWeb(lParam - DEF_PAGES_START);
	else
		PasteIt(hContact, lParam);
	return 0;
}

void InitMenuItems()
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_ROOTPOPUP | CMIF_TCHAR;
	mi.icolibItem = icon.hIcolib;
	mi.position = 3000090005;
	mi.ptszName = LPGENT("Paste It");

	hContactMenu = Menu_AddContactMenuItem(&mi);

	memset(&mi, 0, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = CMIF_CHILDPOPUP | CMIF_ROOTHANDLE | CMIF_TCHAR;
	mi.pszService = MS_PASTEIT_CONTACTMENU;
	mi.hParentMenu = hContactMenu;
	mi.popupPosition = FROM_CLIPBOARD;
	mi.ptszName = LPGENT("Paste from clipboard");
	Menu_AddContactMenuItem(&mi);

	mi.popupPosition = FROM_FILE;
	mi.ptszName = LPGENT("Paste from file");
	Menu_AddContactMenuItem(&mi);

	mi.popupPosition = DEF_PAGES_START - 1;
	mi.ptszName = LPGENT("Default web page");
	HGENMENU hDefWebMenu = Menu_AddContactMenuItem(&mi);

	CLISTMENUITEM mi2 = { sizeof(mi2) };
	mi2.pszService = MS_PASTEIT_CONTACTMENU;
	mi2.hParentMenu = hDefWebMenu;
	for (int i = 0; i < PasteToWeb::pages; ++i)
	{
		mi2.flags = CMIF_CHILDPOPUP | CMIF_ROOTHANDLE | CMIF_TCHAR;
		if (Options::instance->defWeb == i)
			mi2.flags |= CMIF_CHECKED;
		mi2.ptszName = pasteToWebs[i]->GetName();
		mi2.popupPosition = mi2.position = DEF_PAGES_START + i;
		hWebPageMenus[i] = Menu_AddContactMenuItem(&mi2);
	}

	hPrebuildContactMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);
}

void DefWebPageChanged()
{
	CLISTMENUITEM mi = { sizeof(mi) };
	for (int i = 0; i < PasteToWeb::pages; i++) {
		mi.flags = CMIM_FLAGS;
		if (Options::instance->defWeb == i)
			mi.flags |= CMIF_CHECKED;

		Menu_ModifyItem(hWebPageMenus[i], &mi);
	}
}

void InitTabsrmmButton()
{
	if (ServiceExists(MS_BB_ADDBUTTON))
	{
		BBButton btn = { 0 };
		btn.cbSize = sizeof(btn);
		btn.dwButtonID = 1;
		btn.pszModuleName = MODULE;
		btn.dwDefPos = 110;
		btn.hIcon = icon.hIcolib;
		btn.bbbFlags = BBBF_ISARROWBUTTON | BBBF_ISIMBUTTON | BBBF_ISLSIDEBUTTON | BBBF_CANBEHIDDEN | BBBF_ISCHATBUTTON;
		btn.ptszTooltip = TranslateT("Paste It");
		CallService(MS_BB_ADDBUTTON, 0, (LPARAM)&btn);

		if (hTabsrmmButtonPressed != NULL)
			UnhookEvent(hTabsrmmButtonPressed);

		hTabsrmmButtonPressed = HookEvent(ME_MSG_BUTTONPRESSED, TabsrmmButtonPressed);
	}
}

int WindowEvent(WPARAM wParam, MessageWindowEventData* lParam)
{
	if (lParam->uType == MSG_WINDOW_EVT_OPEN)
	{
		char *szProto = GetContactProto(lParam->hContact);
		if (szProto && (INT_PTR)szProto != CALLSERVICE_NOTFOUND)
		{
			if (db_get_b(lParam->hContact, szProto, "ChatRoom", 0))
			{
				(*contactWindows)[lParam->hContact] = lParam->hwndInput;
			}
		}
	}
	else if (lParam->uType == MSG_WINDOW_EVT_CLOSE)
	{
		std::map<MCONTACT, HWND>::iterator it = contactWindows->find(lParam->hContact);
		if (it != contactWindows->end())
		{
			contactWindows->erase(it);
		}
	}

	return 0;
}

int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	InitMenuItems();
	InitTabsrmmButton();
	hWindowEvent = HookEvent(ME_MSG_WINDOWEVENT, (MIRANDAHOOK)WindowEvent);

	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getXI(&xi);
	mir_getLP(&pluginInfo);

	Icon_Register(hInst, LPGEN("Paste It"), &icon, 1);

	NETLIBUSER nlu = { 0 };
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_TCHAR | NUF_OUTGOING | NUF_HTTPCONNS;
	nlu.szSettingsModule = MODULE;
	nlu.ptszDescriptiveName = TranslateT("Paste It HTTP connections");
	g_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	pasteToWebs[0] = new PasteToWeb1();
	pasteToWebs[0]->pageIndex = 0;
	pasteToWebs[1] = new PasteToWeb2();
	pasteToWebs[1]->pageIndex = 1;
	Options::instance = new Options();
	pasteToWebs[0]->ConfigureSettings();
	pasteToWebs[1]->ConfigureSettings();
	Options::instance->Load();
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hOptionsInit = HookEvent(ME_OPT_INITIALISE, Options::InitOptions);
	hTabsrmmButtonPressed = NULL;
	hServiceContactMenu = CreateServiceFunction(MS_PASTEIT_CONTACTMENU, ContactMenuService);
	contactWindows = new std::map<MCONTACT, HWND>();
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	UnhookEvent(hModulesLoaded);
	UnhookEvent(hPrebuildContactMenu);
	UnhookEvent(hOptionsInit);
	if (hWindowEvent != NULL)
		UnhookEvent(hWindowEvent);

	DestroyServiceFunction(hServiceContactMenu);
	Netlib_CloseHandle(g_hNetlibUser);
	if (hTabsrmmButtonPressed != NULL)
		UnhookEvent(hTabsrmmButtonPressed);

	for (int i = 0; i < PasteToWeb::pages; ++i)
		if (pasteToWebs[i] != NULL) {
			delete pasteToWebs[i];
			pasteToWebs[i] = NULL;
		}

	if (Options::instance != NULL) {
		delete Options::instance;
		Options::instance = NULL;
	}

	delete contactWindows;
	return 0;
}
