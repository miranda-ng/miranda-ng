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

HNETLIBUSER g_hNetlibUser;
HANDLE hModulesLoaded, hTabsrmmButtonPressed;
HANDLE hPrebuildContactMenu;
HGENMENU hContactMenu;
HGENMENU hWebPageMenus[PasteToWeb::pages];
HANDLE hOptionsInit;
HANDLE hWindowEvent = nullptr;

static IconItem iconList[] =
{
	{ LPGEN("Paste It"), "PasteIt_main", IDI_MENU }
};

CMPlugin g_plugin;

#define FROM_CLIPBOARD 10
#define FROM_FILE 11
#define DEF_PAGES_START 20
#define MS_PASTEIT_CONTACTMENU	"PasteIt/ContactMenu"

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
	// {1AAC15E8-DCEC-4050-B66F-2AA0E6120C22}
	{ 0x1aac15e8, 0xdcec, 0x4050, { 0xb6, 0x6f, 0x2a, 0xa0, 0xe6, 0x12, 0xc, 0x22 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

std::wstring GetFile()
{
	wchar_t filter[512];
	mir_wstrncpy(filter, TranslateT("All Files (*.*)"), _countof(filter));
	memcpy(filter + mir_wstrlen(filter), L"\0*.*\0", 6 * sizeof(wchar_t));
	wchar_t stzFilePath[1024];
	stzFilePath[0] = 0;
	stzFilePath[1] = 0;
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = stzFilePath;
	ofn.lpstrTitle = TranslateT("Paste It - Select file");
	ofn.nMaxFile = _countof(stzFilePath);
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR;
	if (GetOpenFileName(&ofn)) {
		return stzFilePath;
	}

	return L"";
}

static void PasteIt(MCONTACT hContact, int mode)
{
	PasteToWeb* pasteToWeb = pasteToWebs[Options::instance->defWeb];
	if (mode == FROM_CLIPBOARD) {
		pasteToWeb->FromClipboard();
	}
	else if (mode == FROM_FILE) {
		std::wstring file = GetFile();
		if (file.length() > 0) {
			pasteToWeb->FromFile(file);
		}
		else return;
	}
	else return;

	if (pasteToWeb->szFileLink[0] == 0 && pasteToWeb->error != nullptr) {
		MessageBox(nullptr, pasteToWeb->error, TranslateT("Error"), MB_OK | MB_ICONERROR);
	}
	else if (hContact != NULL && pasteToWeb->szFileLink[0] != 0) {
		char *szProto = Proto_GetBaseAccountName(hContact);
		if (szProto && (INT_PTR)szProto != CALLSERVICE_NOTFOUND) {
			bool isChat = Contact::IsGroupChat(hContact, szProto);
			if (Options::instance->autoSend) {
				if (!isChat) {
					DBEVENTINFO dbei = {};
					dbei.eventType = EVENTTYPE_MESSAGE;
					dbei.flags = DBEF_SENT;
					dbei.szModule = szProto;
					dbei.timestamp = (uint32_t)time(0);
					dbei.cbBlob = (uint32_t)mir_strlen(pasteToWeb->szFileLink) + 1;
					dbei.pBlob = (uint8_t*)pasteToWeb->szFileLink;
					db_event_add(hContact, &dbei);
					ProtoChainSend(hContact, PSS_MESSAGE, 0, (LPARAM)pasteToWeb->szFileLink);
				}
				else {
					// PSS_MESSAGE is not compatible with chat rooms
					// there are no simple method to send text to all users
					// in chat room. 
					// Next step is to get all protocol sessions and find
					// one with correct hContact 
					int cnt = g_chatApi.SM_GetCount(szProto);
					for (int i = 0; i < cnt; i++) {
						GC_INFO gci = {};
						gci.iItem = i;
						gci.pszModule = szProto;
						gci.Flags = GCF_BYINDEX | GCF_HCONTACT | GCF_ID;
						Chat_GetInfo(&gci);
						if (gci.hContact == hContact) {
							// In this place session was finded, gci.pszID contains
							// session ID, but it is in unicode or ascii format,
							// depends on protocol wersion
							Chat_SendUserMessage(szProto, gci.pszID, _A2T(pasteToWeb->szFileLink));
							break;
						}
					}
				}

				// Send message to focus window
				CallServiceSync(MS_MSG_SENDMESSAGE, hContact, 0);
			}
			else {
				if (isChat) {
					// MS_MSG_SENDMESSAGE in incompatible with chat rooms,
					// because it sends text to IDC_MESSAGE window,
					// but in chat rooms is only IDC_CHAT_MESSAGE window.
					// contactWindows map contains all opened hContact
					// with assaigned to them chat windows. 
					// This map is prepared in ME_MSG_WINDOWEVENT event. 
					std::map<MCONTACT, HWND>::iterator it = contactWindows->find(hContact);
					if (it != contactWindows->end()) {
						// it->second is imput window, so now I can send to them 
						// new text. Afterr all is sended MS_MSG_SENDMESSAGE 
						// to focus window.
						SendMessage(it->second, EM_SETSEL, -1, GetWindowTextLength(it->second));
						SendMessageA(it->second, EM_REPLACESEL, FALSE, (LPARAM)pasteToWeb->szFileLink);
						CallServiceSync(MS_MSG_SENDMESSAGE, hContact, NULL);
					}
					else {
						// If window do not exist, maybe it is not chat
						CallServiceSync(MS_MSG_SENDMESSAGE, hContact, (LPARAM)pasteToWeb->szFileLink);
					}
				}
				else {
					CallServiceSync(MS_MSG_SENDMESSAGE, hContact, (LPARAM)pasteToWeb->szFileLink);
				}
			}
		}
	}
}

static int TabsrmmButtonPressed(WPARAM hContact, LPARAM lParam)
{
	CustomButtonClickData *cbc = (CustomButtonClickData *)lParam;

	if (!mir_strcmp(cbc->pszModule, MODULENAME) && cbc->dwButtonId == 1 && hContact) {
		if (cbc->flags == BBCF_ARROWCLICKED) {
			HMENU hMenu = CreatePopupMenu();
			if (hMenu) {
				AppendMenu(hMenu, MF_STRING, FROM_CLIPBOARD, TranslateT("Paste from clipboard"));
				AppendMenu(hMenu, MF_STRING, FROM_FILE, TranslateT("Paste from file"));
				HMENU hDefMenu = CreatePopupMenu();
				for (int i = 0; i < PasteToWeb::pages; ++i) {
					UINT flags = MF_STRING;
					if (Options::instance->defWeb == i)
						flags |= MF_CHECKED;

					AppendMenu(hDefMenu, flags, DEF_PAGES_START + i, pasteToWebs[i]->GetName());
				}

				AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hDefMenu, TranslateT("Default web page"));

				POINT pt;
				GetCursorPos(&pt);
				HWND hwndBtn = WindowFromPoint(pt);
				if (hwndBtn) {
					RECT rc;
					GetWindowRect(hwndBtn, &rc);
					SetForegroundWindow(cbc->hwndFrom);
					int selected = TrackPopupMenu(hMenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, cbc->hwndFrom, nullptr);
					if (selected != 0) {
						if (selected >= DEF_PAGES_START) {
							Options::instance->SetDefWeb(selected - DEF_PAGES_START);
						}
						else {
							PasteIt(hContact, selected);
						}
					}
				}

				DestroyMenu(hDefMenu);
				DestroyMenu(hMenu);
			}
		}
		else {
			PasteIt(hContact, FROM_CLIPBOARD);
		}
	}

	return 0;
}

static int PrebuildContactMenu(WPARAM wParam, LPARAM)
{
	bool bIsContact = false;

	char *szProto = Proto_GetBaseAccountName(wParam);
	if (szProto && (INT_PTR)szProto != CALLSERVICE_NOTFOUND)
		bIsContact = (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IM) != 0;

	Menu_ShowItem(hContactMenu, bIsContact);
	return 0;
}

static INT_PTR ContactMenuService(WPARAM hContact, LPARAM lParam)
{
	if (lParam >= DEF_PAGES_START)
		Options::instance->SetDefWeb(lParam - DEF_PAGES_START);
	else
		PasteIt(hContact, lParam);
	return 0;
}

static void InitMenuItems()
{
	CMenuItem mi(&g_plugin);

	SET_UID(mi, 0x33ecc112, 0x6, 0x487d, 0xbb, 0x8b, 0x76, 0xb4, 0x17, 0x9b, 0xdb, 0xc5);
	mi.flags = CMIF_UNICODE;
	mi.hIcolibItem = iconList[0].hIcolib;
	mi.position = 3000090005;
	mi.name.w = LPGENW("Paste It");
	hContactMenu = Menu_AddContactMenuItem(&mi);

	memset(&mi, 0, sizeof(mi));
	SET_UID(mi, 0xedc0456d, 0x5aa8, 0x4a61, 0xbe, 0xfd, 0xed, 0x34, 0xb2, 0xcc, 0x6, 0x54);
	mi.flags = CMIF_UNICODE;
	mi.pszService = MS_PASTEIT_CONTACTMENU;
	mi.root = hContactMenu;
	mi.name.w = LPGENW("Paste from clipboard");
	Menu_ConfigureItem(Menu_AddContactMenuItem(&mi), MCI_OPT_EXECPARAM, FROM_CLIPBOARD);

	SET_UID(mi, 0x7af7c3cb, 0xedc4, 0x4f3e, 0xb5, 0xe1, 0x42, 0x64, 0x5b, 0x7d, 0xd8, 0x1e);
	mi.name.w = LPGENW("Paste from file");
	Menu_ConfigureItem(Menu_AddContactMenuItem(&mi), MCI_OPT_EXECPARAM, FROM_FILE);

	SET_UID(mi, 0xe58410ad, 0xa723, 0x48b9, 0xab, 0x7e, 0x5e, 0x42, 0x8b, 0x8, 0x32, 0x2f);
	mi.name.w = LPGENW("Default web page");
	HGENMENU hDefWebMenu = Menu_AddContactMenuItem(&mi);
	Menu_ConfigureItem(hDefWebMenu, MCI_OPT_EXECPARAM, DEF_PAGES_START - 1);

	CMenuItem mi2(&g_plugin);
	mi2.pszService = MS_PASTEIT_CONTACTMENU;
	mi2.root = hDefWebMenu;
	for (int i = 0; i < PasteToWeb::pages; ++i) {
		mi2.flags = CMIF_UNICODE | CMIF_UNMOVABLE;
		if (Options::instance->defWeb == i)
			mi2.flags |= CMIF_CHECKED;
		mi2.name.w = pasteToWebs[i]->GetName();
		hWebPageMenus[i] = Menu_AddContactMenuItem(&mi2);
		Menu_ConfigureItem(hWebPageMenus[i], MCI_OPT_EXECPARAM, mi2.position = DEF_PAGES_START + i);
	}

	hPrebuildContactMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);
}

void DefWebPageChanged()
{
	for (int i = 0; i < PasteToWeb::pages; i++) {
		int flags = (Options::instance->defWeb == i) ? CMIF_CHECKED : 0;
		Menu_ModifyItem(hWebPageMenus[i], nullptr, INVALID_HANDLE_VALUE, flags);
	}
}

static void InitTabsrmmButton()
{
	BBButton btn = {};
	btn.dwButtonID = 1;
	btn.pszModuleName = MODULENAME;
	btn.dwDefPos = 110;
	btn.hIcon = iconList[0].hIcolib;
	btn.bbbFlags = BBBF_ISARROWBUTTON | BBBF_ISIMBUTTON | BBBF_CANBEHIDDEN | BBBF_ISCHATBUTTON;
	btn.pwszTooltip = TranslateT("Paste It");
	Srmm_AddButton(&btn, &g_plugin);

	if (hTabsrmmButtonPressed != nullptr)
		UnhookEvent(hTabsrmmButtonPressed);

	hTabsrmmButtonPressed = HookEvent(ME_MSG_BUTTONPRESSED, TabsrmmButtonPressed);
}

static int WindowEvent(WPARAM, MessageWindowEventData* lParam)
{
	if (lParam->uType == MSG_WINDOW_EVT_OPEN) {
		char *szProto = Proto_GetBaseAccountName(lParam->hContact);
		if (szProto && (INT_PTR)szProto != CALLSERVICE_NOTFOUND) {
			if (Contact::IsGroupChat(lParam->hContact, szProto)) {
				(*contactWindows)[lParam->hContact] = lParam->hwndInput;
			}
		}
	}
	else if (lParam->uType == MSG_WINDOW_EVT_CLOSE) {
		std::map<MCONTACT, HWND>::iterator it = contactWindows->find(lParam->hContact);
		if (it != contactWindows->end()) {
			contactWindows->erase(it);
		}
	}

	return 0;
}

static int ModulesLoaded(WPARAM, LPARAM)
{
	InitMenuItems();
	InitTabsrmmButton();
	hWindowEvent = HookEvent(ME_MSG_WINDOWEVENT, (MIRANDAHOOK)WindowEvent);

	return 0;
}

int CMPlugin::Load()
{
	g_plugin.registerIcon(LPGEN("Paste It"), iconList);

	NETLIBUSER nlu = {};
	nlu.flags = NUF_UNICODE | NUF_OUTGOING | NUF_HTTPCONNS;
	nlu.szSettingsModule = MODULENAME;
	nlu.szDescriptiveName.w = TranslateT("Paste It");
	g_hNetlibUser = Netlib_RegisterUser(&nlu);

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
	hTabsrmmButtonPressed = nullptr;
	CreateServiceFunction(MS_PASTEIT_CONTACTMENU, ContactMenuService);
	contactWindows = new std::map<MCONTACT, HWND>();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	UnhookEvent(hModulesLoaded);
	UnhookEvent(hPrebuildContactMenu);
	UnhookEvent(hOptionsInit);
	if (hWindowEvent != nullptr)
		UnhookEvent(hWindowEvent);

	Netlib_CloseHandle(g_hNetlibUser);
	if (hTabsrmmButtonPressed != nullptr)
		UnhookEvent(hTabsrmmButtonPressed);

	for (int i = 0; i < PasteToWeb::pages; ++i)
		if (pasteToWebs[i] != nullptr) {
			delete pasteToWebs[i];
			pasteToWebs[i] = nullptr;
		}

	if (Options::instance != nullptr) {
		delete Options::instance;
		Options::instance = nullptr;
	}

	delete contactWindows;
	return 0;
}
