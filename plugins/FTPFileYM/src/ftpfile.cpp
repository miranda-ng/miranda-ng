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

#include "stdafx.h"

CMPlugin g_plugin;

HGENMENU hMenu, hMainMenu, hSubMenu[ServerList::FTP_COUNT], hMainSubMenu[ServerList::FTP_COUNT];

int PrebuildContactMenu(WPARAM wParam, LPARAM lParam);
void PrebuildMainMenu();
int TabsrmmButtonPressed(WPARAM wParam, LPARAM lParam);
int UploadFile(MCONTACT hContact, int m_iFtpNum, UploadJob::EMode mode);

/////////////////////////////////////////////////////////////////////////////////////////

static PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {9502E511-7E5D-49A1-8BA5-B1AEE70FA5BF}
	{0x9502e511, 0x7e5d, 0x49a1, {0x8b, 0xa5, 0xb1, 0xae, 0xe7, 0xf, 0xa5, 0xbf}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] =
{
	{ LPGEN("FTP Server 1"),     "ftp1",       IDI_FTP0       },
	{ LPGEN("FTP Server 2"),     "ftp2",       IDI_FTP1       },
	{ LPGEN("FTP Server 3"),     "ftp3",       IDI_FTP2       },
	{ LPGEN("FTP Server 4"),     "ftp4",       IDI_FTP3       },
	{ LPGEN("FTP Server 5"),     "ftp5",       IDI_FTP4       },
	{ LPGEN("Send file"),        "main",       IDI_MENU       },
	{ LPGEN("Clipboard"),        "clipboard",  IDI_CLIPBOARD  },
	{ LPGEN("Pause"),            "pause",      IDI_PAUSE      },
	{ LPGEN("Resume"),           "resume",     IDI_RESUME     },
	{ LPGEN("Delete from list"), "clear",      IDI_CLEAR      },
	{ LPGEN("Delete from FTP"),  "delete",     IDI_DELETE     }
};

static void InitIcolib()
{
	g_plugin.registerIcon(LPGEN("FTP File"), iconList, MODULENAME);
}

void InitMenuItems()
{
	wchar_t stzName[256];

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0xB7132F5A, 0x65FC, 0x42C5, 0xB4, 0xCB, 0x54, 0xBC, 0xAC, 0x58, 0x34, 0xE9);
	mi.flags = CMIF_UNICODE;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_MENU);
	mi.position = 3000090001;
	mi.name.w = LPGENW("FTP File");

	hMainMenu = Menu_AddMainMenuItem(&mi);
	if (opt.bUseSubmenu)
		hMenu = Menu_AddContactMenuItem(&mi);

	memset(&mi, 0, sizeof(mi));
	mi.name.w = stzName;
	mi.flags = CMIF_UNICODE | CMIF_SYSTEM;

	CMenuItem mi2(&g_plugin);
	mi2.flags = CMIF_UNICODE | CMIF_SYSTEM;
	mi2.pszService = MS_FTPFILE_CONTACTMENU;

	CMStringA frmt;
	for (int i = 0; i < ServerList::FTP_COUNT; i++) {
		ptrA Name(g_plugin.getStringA(frmt.Format("Name%d", i)));
		if (Name)
			mir_snwprintf(stzName, TranslateT("FTP Server %d"), i + 1);

		mi.root = (opt.bUseSubmenu) ? hMenu : nullptr;
		mi.hIcolibItem = iconList[i].hIcolib;
		hSubMenu[i] = Menu_AddContactMenuItem(&mi);
		Menu_ConfigureItem(hSubMenu[i], MCI_OPT_EXECPARAM, i + 1000);

		mi.root = hMainMenu;
		hMainSubMenu[i] = Menu_AddMainMenuItem(&mi);

		mi2.root = hSubMenu[i];
		mi2.pszService = MS_FTPFILE_CONTACTMENU;
		mi2.name.w = LPGENW("Upload file(s)");
		HGENMENU tmp = Menu_AddContactMenuItem(&mi2);
		Menu_ConfigureItem(tmp, MCI_OPT_EXECPARAM, mi2.position = i + UploadJob::FTP_RAWFILE);

		mi2.pszService = MS_FTPFILE_MAINMENU;
		mi2.root = hMainSubMenu[i];
		Menu_AddMainMenuItem(&mi2);

		mi2.root = hSubMenu[i];
		mi2.pszService = MS_FTPFILE_CONTACTMENU;
		mi2.name.w = LPGENW("Zip and upload file(s)");
		tmp = Menu_AddContactMenuItem(&mi2);
		Menu_ConfigureItem(tmp, MCI_OPT_EXECPARAM, i + UploadJob::FTP_ZIPFILE);

		mi2.pszService = MS_FTPFILE_MAINMENU;
		mi2.root = hMainSubMenu[i];
		Menu_AddMainMenuItem(&mi2);

		mi2.root = hSubMenu[i];
		mi2.pszService = MS_FTPFILE_CONTACTMENU;
		mi2.name.w = LPGENW("Zip and upload folder");
		tmp = Menu_AddContactMenuItem(&mi2);
		Menu_ConfigureItem(tmp, MCI_OPT_EXECPARAM, i + UploadJob::FTP_ZIPFOLDER);

		mi2.pszService = MS_FTPFILE_MAINMENU;
		mi2.root = hMainSubMenu[i];
		Menu_AddMainMenuItem(&mi2);
	}

	memset(&mi, 0, sizeof(mi));
	SET_UID(mi, 0x0C17CAD7, 0x7F23, 0x449B, 0xB9, 0xCD, 0xFF, 0x50, 0xDA, 0x69, 0xF3, 0x6F);
	mi.flags = CMIF_UNICODE;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_MENU);
	mi.position = 3000090001;
	mi.name.w = LPGENW("FTP File manager");
	mi.pszService = MS_FTPFILE_SHOWMANAGER;
	mi.root = hMainMenu;
	Menu_AddMainMenuItem(&mi);

	PrebuildMainMenu();

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);
}

void InitHotkeys()
{
	HOTKEYDESC hk = {};
	hk.szSection.a = MODULENAME;
	hk.szDescription.a = LPGEN("Show FTPFile manager");
	hk.pszName = "FTP_ShowManager";
	hk.pszService = MS_FTPFILE_SHOWMANAGER;
	g_plugin.addHotkey(&hk);
}

void InitTabsrmmButton()
{
	BBButton btn = {};
	btn.dwButtonID = 1;
	btn.pszModuleName = MODULENAME;
	btn.dwDefPos = 105;
	btn.hIcon = g_plugin.getIconHandle(IDI_MENU);
	btn.bbbFlags = BBBF_ISARROWBUTTON | BBBF_ISIMBUTTON | BBBF_CANBEHIDDEN;
	btn.pwszTooltip = TranslateT("FTP File");
	Srmm_AddButton(&btn, &g_plugin);
	HookEvent(ME_MSG_BUTTONPRESSED, TabsrmmButtonPressed);
}

//------------ MENU & BUTTON HANDLERS ------------//

int PrebuildContactMenu(WPARAM wParam, LPARAM)
{
	bool bIsContact = false;

	char *szProto = Proto_GetBaseAccountName(wParam);
	if (szProto) bIsContact = (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IM) ? true : false;

	bool bHideRoot = opt.bHideInactive;
	for (int i = 0; i < ServerList::FTP_COUNT; i++)
		if (ftpList[i]->m_bEnabled)
			bHideRoot = false;

	if (opt.bUseSubmenu)
		Menu_ShowItem(hMenu, bIsContact && !bHideRoot);

	for (int i = 0; i < ServerList::FTP_COUNT; i++)
		Menu_ShowItem(hSubMenu[i], bIsContact && ftpList[i]->m_bEnabled);
	return 0;
}

void PrebuildMainMenu()
{
	for (int i = 0; i < ServerList::FTP_COUNT; i++)
		if (ftpList[i])
			Menu_ShowItem(hMainSubMenu[i], ftpList[i]->m_bEnabled);
}

int TabsrmmButtonPressed(WPARAM hContact, LPARAM lParam)
{
	CustomButtonClickData *cbc = (CustomButtonClickData *)lParam;

	if (!strcmp(cbc->pszModule, MODULENAME) && cbc->dwButtonId == 1 && hContact) {
		if (cbc->flags == BBCF_ARROWCLICKED) {
			HMENU hPopupMenu = CreatePopupMenu();
			if (hPopupMenu) {
				int iCount = 0;
				for (UINT i = 0; i < ServerList::FTP_COUNT; i++) {
					if (ftpList[i]->m_bEnabled) {
						HMENU hModeMenu = CreatePopupMenu();
						AppendMenu(hModeMenu, MF_STRING, i + UploadJob::FTP_RAWFILE, TranslateT("Upload file"));
						AppendMenu(hModeMenu, MF_STRING, i + UploadJob::FTP_ZIPFILE, TranslateT("Zip and upload file"));
						AppendMenu(hModeMenu, MF_STRING, i + UploadJob::FTP_ZIPFOLDER, TranslateT("Zip and upload folder"));
						AppendMenu(hPopupMenu, MF_STRING | MF_POPUP, (UINT_PTR)hModeMenu, ftpList[i]->m_stzName);
						DestroyMenu(hModeMenu);
						iCount++;
					}
				}

				if (iCount != 0) {
					POINT pt;
					GetCursorPos(&pt);
					HWND hwndBtn = WindowFromPoint(pt);
					if (hwndBtn) {
						RECT rc;
						GetWindowRect(hwndBtn, &rc);
						SetForegroundWindow(cbc->hwndFrom);
						int selected = TrackPopupMenu(hPopupMenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, cbc->hwndFrom, nullptr);
						if (selected != 0) {
							int ftpNum = selected & (1 | 2 | 4);
							int mode = selected & (UploadJob::FTP_RAWFILE | UploadJob::FTP_ZIPFILE | UploadJob::FTP_ZIPFOLDER);
							UploadFile(hContact, ftpNum, (UploadJob::EMode)mode);
						}
					}
				}

				DestroyMenu(hPopupMenu);
			}
		}
		else UploadFile(hContact, 0, UploadJob::FTP_RAWFILE);
	}

	return 0;
}

int UploadFile(MCONTACT hContact, int m_iFtpNum, GenericJob::EMode mode, void **objects, int objCount, uint32_t flags)
{
	if (!ftpList[m_iFtpNum]->isValid()) {
		Utils::msgBox(TranslateT("You have to fill FTP server setting before upload a file."), MB_OK | MB_ICONERROR);
		return 1;
	}

	GenericJob *job;
	if (mode == GenericJob::FTP_RAWFILE)
		job = new UploadJob(hContact, m_iFtpNum, mode);
	else
		job = new PackerJob(hContact, m_iFtpNum, mode);

	int result;
	if (objects != nullptr)
		result = job->getFiles(objects, objCount, flags);
	else
		result = job->getFiles();

	if (result != 0) {
		if (uDlg == nullptr)
			uDlg = new UploadDialog();
		if (!uDlg->m_hwnd || !uDlg->m_hwndTabs) {
			Utils::msgBox(TranslateT("Error has occurred while trying to create a dialog!"), MB_OK | MB_ICONERROR);
			delete uDlg;
			return 1;
		}

		job->addToUploadDlg();
		uDlg->show();
	}
	else {
		delete job;
		return 1;
	}

	return 0;
}

int UploadFile(MCONTACT hContact, int m_iFtpNum, GenericJob::EMode mode)
{
	return UploadFile(hContact, m_iFtpNum, mode, nullptr, 0, 0);
}

//------------ MIRANDA SERVICES ------------//

INT_PTR UploadService(WPARAM, LPARAM lParam)
{
	FTPUPLOAD *ftpu = (FTPUPLOAD *)lParam;
	if (ftpu == nullptr || ftpu->cbSize != sizeof(FTPUPLOAD))
		return 1;

	int ftpNum = (ftpu->ftpNum == FNUM_DEFAULT) ? opt.defaultFTP : ftpu->ftpNum - 1;
	int mode = (ftpu->mode * GenericJob::FTP_RAWFILE);

	UploadFile(ftpu->hContact, ftpNum, (GenericJob::EMode)mode, (void**)ftpu->pstzObjects, ftpu->objectCount, ftpu->flags);
	return 0;
}

INT_PTR ShowManagerService(WPARAM, LPARAM)
{
	manDlg = new Manager();
	manDlg->init();
	return 0;
}

INT_PTR ContactMenuService(WPARAM hContact, LPARAM lParam)
{
	int ftpNum = lParam & (1 | 2 | 4);
	int mode = lParam & (UploadJob::FTP_RAWFILE | UploadJob::FTP_ZIPFILE | UploadJob::FTP_ZIPFOLDER);
	return UploadFile(hContact, ftpNum, (UploadJob::EMode)mode);
}

INT_PTR MainMenuService(WPARAM wParam, LPARAM)
{
	int ftpNum = wParam & (1 | 2 | 4);
	int mode = wParam & (UploadJob::FTP_RAWFILE | UploadJob::FTP_ZIPFILE | UploadJob::FTP_ZIPFOLDER);
	return UploadFile(0, ftpNum, (UploadJob::EMode)mode);
}

//------------ START & EXIT STUFF ------------//

static int ModulesLoaded(WPARAM, LPARAM)
{
	InitMenuItems();
	InitTabsrmmButton();

	g_plugin.addSound(SOUND_UPCOMPLETE, LPGENW("FTP File"), LPGENW("File upload complete"));
	g_plugin.addSound(SOUND_CANCEL, LPGENW("FTP File"), LPGENW("Upload canceled"));

	curl_global_init(CURL_GLOBAL_ALL);

	return 0;
}

static int Shutdown(WPARAM, LPARAM)
{
	deleteTimer.deinit();

	delete manDlg;
	if (uDlg)
		SendMessage(uDlg->m_hwnd, WM_CLOSE, 0, 0);

	UploadJob::jobDone.release();
	DeleteJob::jobDone.release();
	DBEntry::cleanupDB();

	curl_global_cleanup();

	ftpList.deinit();
	return 0;
}

int CMPlugin::Load()
{
	CoInitialize(nullptr);

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, Shutdown);
	HookEvent(ME_OPT_INITIALISE, Options::InitOptions);

	CreateServiceFunction(MS_FTPFILE_UPLOAD, UploadService);
	CreateServiceFunction(MS_FTPFILE_SHOWMANAGER, ShowManagerService);
	CreateServiceFunction(MS_FTPFILE_CONTACTMENU, ContactMenuService);
	CreateServiceFunction(MS_FTPFILE_MAINMENU, MainMenuService);

	InitIcolib();
	InitHotkeys();

	opt.loadOptions();
	deleteTimer.init();
	ftpList.init();
	return 0;
}
