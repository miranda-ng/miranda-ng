/*
FTP File YM plugin
Copyright (C) 2007-2010 Jan Holub

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

#include "common.h"

HINSTANCE hInst;
int hLangpack;

HANDLE hModulesLoaded, hEventPreShutdown, hOptionsInit, hPrebuildContactMenu, hTabsrmmButtonPressed;
HANDLE hServiceUpload, hServiceShowManager, hServiceContactMenu, hServiceMainMenu;
HGENMENU hMenu, hMainMenu, hSubMenu[ServerList::FTP_COUNT], hMainSubMenu[ServerList::FTP_COUNT];

extern UploadDialog *uDlg;
extern Manager *manDlg;

extern DeleteTimer &deleteTimer;
extern ServerList &ftpList;
extern Options &opt;

int PrebuildContactMenu(WPARAM wParam, LPARAM lParam);
void PrebuildMainMenu();
int TabsrmmButtonPressed(WPARAM wParam, LPARAM lParam);
int UploadFile(MCONTACT hContact, int iFtpNum, UploadJob::EMode mode);

static PLUGININFOEX pluginInfoEx = 
{
	sizeof(PLUGININFOEX), 
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {9502E511-7E5D-49A1-8BA5-B1AEE70FA5BF}
	{0x9502e511, 0x7e5d, 0x49a1, {0x8b, 0xa5, 0xb1, 0xae, 0xe7, 0xf, 0xa5, 0xbf}}
};

//------------ BASIC STAFF ------------//

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{	
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

//------------ INIT FUNCTIONS ------------//

static IconItem iconList[] =
{
	{ LPGEN("FTP Server 1"),     "ftp1", 		IDI_FTP0       },
	{ LPGEN("FTP Server 2"),     "ftp2", 		IDI_FTP1       },
	{ LPGEN("FTP Server 3"),     "ftp3", 		IDI_FTP2       },
	{ LPGEN("FTP Server 4"),     "ftp4", 		IDI_FTP3       },
	{ LPGEN("FTP Server 5"),     "ftp5", 		IDI_FTP4       },
	{ LPGEN("Send file"),        "main",       IDI_MENU       },
	{ LPGEN("Clipboard"),        "clipboard",  IDI_CLIPBOARD  },
	{ LPGEN("Pause"),            "pause",      IDI_PAUSE      },
	{ LPGEN("Resume"),           "resume",     IDI_RESUME     },
	{ LPGEN("Delete from list"), "clear",      IDI_CLEAR      },
	{ LPGEN("Delete from FTP"),  "delete",     IDI_DELETE     }
};

static void InitIcolib()
{
	Icon_Register(hInst, LPGEN("FTP File"), iconList, SIZEOF(iconList), MODULE);
}

void InitMenuItems()
{
	TCHAR stzName[256];

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_ROOTPOPUP | CMIF_TCHAR;
	mi.icolibItem = iconList[ServerList::FTP_COUNT].hIcolib;
	mi.position = 3000090001;
	mi.ptszName = LPGENT("FTP File");

	hMainMenu = Menu_AddMainMenuItem(&mi);
	if (opt.bUseSubmenu)
		hMenu = Menu_AddContactMenuItem(&mi);

	memset(&mi, 0, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.ptszName = stzName;

	CLISTMENUITEM mi2 = { sizeof(mi2) };
	mi2.flags = CMIF_CHILDPOPUP | CMIF_ROOTHANDLE | CMIF_TCHAR;
	mi2.pszService = MS_FTPFILE_CONTACTMENU;

	for (int i = 0; i < ServerList::FTP_COUNT; i++) 
	{
		if (DB::getStringF(0, MODULE, "Name%d", i, stzName))
			mir_sntprintf(stzName, SIZEOF(stzName), TranslateT("FTP Server %d"), i + 1);

		mi.flags = CMIF_TCHAR;
		mi.hParentMenu = 0; 
		if (opt.bUseSubmenu)
		{
			mi.flags |= CMIF_CHILDPOPUP | CMIF_ROOTHANDLE;
			mi.hParentMenu = hMenu;
		}

		mi.icolibItem = iconList[i].hIcolib;
		mi.popupPosition = i + 1000;
		hSubMenu[i] = Menu_AddContactMenuItem(&mi);

		mi.flags |= CMIF_CHILDPOPUP | CMIF_ROOTHANDLE;
		mi.hParentMenu = hMainMenu;
		hMainSubMenu[i] = Menu_AddMainMenuItem(&mi);
		
		mi2.hParentMenu = hSubMenu[i];
		mi2.pszService = MS_FTPFILE_CONTACTMENU;
		mi2.popupPosition = mi2.position = i + UploadJob::FTP_RAWFILE;
		mi2.ptszName = LPGENT("Upload file(s)");		
		Menu_AddContactMenuItem(&mi2);

		mi2.pszService = MS_FTPFILE_MAINMENU;
		mi2.hParentMenu = hMainSubMenu[i];
		Menu_AddMainMenuItem(&mi2);

		mi2.hParentMenu = hSubMenu[i];
		mi2.pszService = MS_FTPFILE_CONTACTMENU;
		mi2.popupPosition = i + UploadJob::FTP_ZIPFILE;
		mi2.ptszName = LPGENT("Zip and upload file(s)");
		Menu_AddContactMenuItem(&mi2);

		mi2.pszService = MS_FTPFILE_MAINMENU;
		mi2.hParentMenu = hMainSubMenu[i];
		Menu_AddMainMenuItem(&mi2);

		mi2.hParentMenu = hSubMenu[i];
		mi2.pszService = MS_FTPFILE_CONTACTMENU;
		mi2.popupPosition = i + UploadJob::FTP_ZIPFOLDER;
		mi2.ptszName = LPGENT("Zip and upload folder");
		Menu_AddContactMenuItem(&mi2);

		mi2.pszService = MS_FTPFILE_MAINMENU;
		mi2.hParentMenu = hMainSubMenu[i];
		Menu_AddMainMenuItem(&mi2);
	}

	memset(&mi, 0, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = CMIF_CHILDPOPUP | CMIF_ROOTHANDLE | CMIF_TCHAR;
	mi.icolibItem = iconList[ServerList::FTP_COUNT].hIcolib;
	mi.position = 3000090001;
	mi.ptszName = LPGENT("FTP File manager");
	mi.pszService = MS_FTPFILE_SHOWMANAGER;
	mi.hParentMenu = hMainMenu;
	Menu_AddMainMenuItem(&mi);

	PrebuildMainMenu();

	hPrebuildContactMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);
}

void InitHotkeys()
{
	HOTKEYDESC hk = {0};
	hk.cbSize = sizeof(hk);
	hk.pszDescription = LPGEN("Show FTPFile manager");
	hk.pszName = "FTP_ShowManager";
	hk.pszSection = MODULE;
	hk.pszService = MS_FTPFILE_SHOWMANAGER;
	Hotkey_Register(&hk);
}

void InitTabsrmmButton()
{
	if (ServiceExists(MS_BB_ADDBUTTON)) 
	{
		BBButton btn = {0};
		btn.cbSize = sizeof(btn);
		btn.dwButtonID = 1;
		btn.pszModuleName = MODULE;
		btn.dwDefPos = 105;
		btn.hIcon = iconList[ServerList::FTP_COUNT].hIcolib;
		btn.bbbFlags = BBBF_ISARROWBUTTON | BBBF_ISIMBUTTON | BBBF_ISLSIDEBUTTON | BBBF_CANBEHIDDEN;
		btn.ptszTooltip = TranslateT("FTP File");
		CallService(MS_BB_ADDBUTTON, 0, (LPARAM)&btn);
		hTabsrmmButtonPressed = HookEvent(ME_MSG_BUTTONPRESSED, TabsrmmButtonPressed);
	}
}

//------------ MENU & BUTTON HANDLERS ------------//

int PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	bool bIsContact = false;

	char *szProto = GetContactProto(wParam);
	if (szProto) bIsContact = (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IM) ? true : false;

	bool bHideRoot = opt.bHideInactive;
	for (int i = 0; i < ServerList::FTP_COUNT; i++) 
		if (ftpList[i]->bEnabled)
			bHideRoot = false;

	if (opt.bUseSubmenu)
		Menu_ShowItem(hMenu, bIsContact && !bHideRoot);

	for (int i = 0; i < ServerList::FTP_COUNT; i++) 
		Menu_ShowItem(hSubMenu[i], bIsContact && ftpList[i]->bEnabled);
	return 0;
}

void PrebuildMainMenu()
{
	for (int i=0; i < ServerList::FTP_COUNT; i++)
		if (ftpList[i])
			Menu_ShowItem(hMainSubMenu[i], ftpList[i]->bEnabled);
}

int TabsrmmButtonPressed(WPARAM hContact, LPARAM lParam) 
{
	CustomButtonClickData *cbc = (CustomButtonClickData *)lParam;

	if (!strcmp(cbc->pszModule, MODULE) && cbc->dwButtonId == 1 && hContact) 
	{
		if (cbc->flags == BBCF_ARROWCLICKED) 
		{
			HMENU hMenu = CreatePopupMenu();
			if (hMenu) 
			{
				int iCount = 0;
				for (UINT i = 0; i < ServerList::FTP_COUNT; i++) 
				{
					if (ftpList[i]->bEnabled)
					{
						HMENU hModeMenu = CreatePopupMenu();
						AppendMenu(hModeMenu, MF_STRING, i + UploadJob::FTP_RAWFILE, TranslateT("Upload file"));
						AppendMenu(hModeMenu, MF_STRING, i + UploadJob::FTP_ZIPFILE, TranslateT("Zip and upload file"));
						AppendMenu(hModeMenu, MF_STRING, i + UploadJob::FTP_ZIPFOLDER, TranslateT("Zip and upload folder"));
						AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hModeMenu, ftpList[i]->stzName);
						DestroyMenu(hModeMenu);
						iCount++;
					}
				}

				if (iCount != 0) 
				{
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
							int ftpNum = selected & (1|2|4);
							int mode = selected & (UploadJob::FTP_RAWFILE | UploadJob::FTP_ZIPFILE | UploadJob::FTP_ZIPFOLDER); 
							UploadFile(hContact, ftpNum, (UploadJob::EMode)mode);
						}
					}
				}

				DestroyMenu(hMenu);
			}
		} 
		else
		{
			UploadFile(hContact, 0, UploadJob::FTP_RAWFILE);
		}
	}

	return 0;
}

int UploadFile(MCONTACT hContact, int iFtpNum, GenericJob::EMode mode, void **objects, int objCount, DWORD flags) 
{
	if (!ftpList[iFtpNum]->isValid()) 
	{
		Utils::msgBox(TranslateT("You have to fill FTP server setting before upload a file."), MB_OK | MB_ICONERROR);
		return 1;
	}

	GenericJob *job;
	if (mode == GenericJob::FTP_RAWFILE)
		job = new UploadJob(hContact, iFtpNum, mode);
	else
		job = new PackerJob(hContact, iFtpNum, mode);

	int result;
	if (objects != NULL)
		result = job->getFiles(objects, objCount, flags);
	else
		result = job->getFiles();

	if (result != 0)
	{
		uDlg = UploadDialog::getInstance();
		if (!uDlg->hwnd || !uDlg->hwndTabs)
		{
			Utils::msgBox(TranslateT("Error has occurred while trying to create a dialog!"), MB_OK | MB_ICONERROR);
			delete uDlg;
			return 1;
		}

		job->addToUploadDlg();
		uDlg->show();
	}
	else
	{
		delete job;
		return 1;
	}

	return 0;
}

int UploadFile(MCONTACT hContact, int iFtpNum, GenericJob::EMode mode)
{
	return UploadFile(hContact, iFtpNum, mode, NULL, 0, 0); 
}

//------------ MIRANDA SERVICES ------------//

INT_PTR UploadService(WPARAM wParam, LPARAM lParam) 
{
	FTPUPLOAD* ftpu = (FTPUPLOAD *)lParam;
	if (ftpu == NULL || ftpu->cbSize != sizeof(FTPUPLOAD))
		return 1;

	int ftpNum = (ftpu->ftpNum == FNUM_DEFAULT) ? opt.defaultFTP : ftpu->ftpNum - 1;
	int mode = (ftpu->mode * GenericJob::FTP_RAWFILE);

	UploadFile(ftpu->hContact, ftpNum, (GenericJob::EMode)mode, (void**)ftpu->pstzObjects, ftpu->objectCount, ftpu->flags);
	return 0; 
}

INT_PTR ShowManagerService(WPARAM wParam, LPARAM lParam) 
{
	manDlg = Manager::getInstance();
	manDlg->init();
	return 0;
}

INT_PTR ContactMenuService(WPARAM hContact, LPARAM lParam) 
{
	int ftpNum = lParam & (1|2|4);
	int mode = lParam & (UploadJob::FTP_RAWFILE | UploadJob::FTP_ZIPFILE | UploadJob::FTP_ZIPFOLDER); 
	return UploadFile(hContact, ftpNum, (UploadJob::EMode)mode);
}

INT_PTR MainMenuService(WPARAM wParam, LPARAM lParam) 
{
	int ftpNum = wParam & (1|2|4);
	int mode = wParam & (UploadJob::FTP_RAWFILE | UploadJob::FTP_ZIPFILE | UploadJob::FTP_ZIPFOLDER); 
	return UploadFile(0, ftpNum, (UploadJob::EMode)mode);
}

//------------ START & EXIT STUFF ------------//

int ModulesLoaded(WPARAM, LPARAM)
{
	InitMenuItems();
	InitTabsrmmButton();

	SkinAddNewSoundEx(SOUND_UPCOMPLETE, LPGEN("FTP File"), LPGEN("File upload complete"));
	SkinAddNewSoundEx(SOUND_CANCEL, LPGEN("FTP File"), LPGEN("Upload canceled"));

	curl_global_init(CURL_GLOBAL_ALL);

	return 0;
}

int Shutdown(WPARAM wParam, LPARAM lParam) 
{
	deleteTimer.deinit();

	delete manDlg;
	if (uDlg) SendMessage(uDlg->hwnd, WM_CLOSE, 0, 0);

	UploadJob::jobDone.release();
	DeleteJob::jobDone.release();
	DBEntry::cleanupDB();

	curl_global_cleanup();	

	ftpList.deinit();
	opt.deinit();

	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfoEx);

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	CoInitialize(NULL);

	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hEventPreShutdown = HookEvent(ME_SYSTEM_PRESHUTDOWN, Shutdown);
	hOptionsInit = HookEvent(ME_OPT_INITIALISE, Options::InitOptions);

	hServiceUpload = CreateServiceFunction(MS_FTPFILE_UPLOAD, UploadService);
	hServiceShowManager = CreateServiceFunction(MS_FTPFILE_SHOWMANAGER, ShowManagerService);
	hServiceContactMenu = CreateServiceFunction(MS_FTPFILE_CONTACTMENU, ContactMenuService);
	hServiceMainMenu = CreateServiceFunction(MS_FTPFILE_MAINMENU, MainMenuService);

	InitIcolib();
	InitHotkeys();

	opt.loadOptions();
	deleteTimer.init();
	ftpList.init();
	
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void) 
{	
	UnhookEvent(hModulesLoaded);
	UnhookEvent(hEventPreShutdown);
	UnhookEvent(hOptionsInit);
	UnhookEvent(hPrebuildContactMenu);
	UnhookEvent(hTabsrmmButtonPressed);
	
	DestroyServiceFunction(hServiceUpload);
	DestroyServiceFunction(hServiceShowManager);
	DestroyServiceFunction(hServiceContactMenu);
	DestroyServiceFunction(hServiceMainMenu);

	return 0;
}