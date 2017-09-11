#include "globals.h"

CLIST_INTERFACE *pcli;
HINSTANCE hinstance = NULL;
HINSTANCE hmiranda = NULL;
int hLangpack;

HANDLE hkOptInit = NULL;
HANDLE hkTopToolbarInit = NULL; 
HANDLE hkModulesLoaded = NULL;
HANDLE hkFontChange = NULL;
HANDLE hkColorChange = NULL;
HMODULE hRichedDll = NULL;

extern TREEELEMENT *g_Stickies;
extern TREEELEMENT *RemindersList;

static PLUGININFOEX pluginInfo =
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
	{0x842a6668, 0xf9da, 0x4968, {0xbf, 0xd7, 0xd2, 0xbd, 0x9d, 0xf8, 0x48, 0xee}} // {842A6668-F9DA-4968-BFD7-D2BD9DF848EE}
};


void RegisterFontServiceFonts();
void RegisterKeyBindings();
INT_PTR OpenTriggeredReminder(WPARAM w, LPARAM l);
void BringAllNotesToFront(STICKYNOTE *pActive);
void CloseNotesList();
void CloseReminderList();

INT_PTR PluginMenuCommandAddNew(WPARAM, LPARAM)
{
	STICKYNOTE *PSN = NewNote(0,0,0,0,NULL,NULL,TRUE,TRUE,0);
	if(PSN)
		SetFocus(PSN->REHwnd);
	return 0;
}

INT_PTR PluginMenuCommandDeleteAll(WPARAM, LPARAM)
{
	if (g_Stickies && MessageBox(NULL, Translate("Are you sure you want to delete all notes?"), Translate(SECTIONNAME), MB_OKCANCEL) == IDOK)
		DeleteNotes();
	return 0;
}

static INT_PTR PluginMenuCommandShowHide(WPARAM, LPARAM)
{
	ShowHideNotes();
	return 0;
}

static INT_PTR PluginMenuCommandViewNotes(WPARAM, LPARAM)
{
	ListNotes();
	return 0;
}

static INT_PTR PluginMenuCommandAllBringFront(WPARAM, LPARAM)
{
	BringAllNotesToFront(NULL);
	return 0;
}

static INT_PTR PluginMenuCommandNewReminder(WPARAM, LPARAM)
{
	NewReminder();
	return 0;
}

static INT_PTR PluginMenuCommandViewReminders(WPARAM, LPARAM)
{
	ListReminders();
	return 0;
}

static INT_PTR PluginMenuCommandDeleteReminders(WPARAM, LPARAM)
{
	if (RemindersList && MessageBox(NULL, Translate("Are you sure you want to delete all reminders?"), Translate(SECTIONNAME), MB_OKCANCEL) == IDOK)
		DeleteReminders();
	return 0;
}

IconItem iconList[] =
{
	{ LPGEN("New Reminder"),            "AddReminder",       IDI_ADDREMINDER },
	{ LPGEN("Delete All Notes"),        "DeleteIcon",        IDI_DELETEICON },
	{ LPGEN("New Note"),                "NoteIcon",          IDI_NOTEICON },
	{ LPGEN("Show/Hide Notes"),         "ShowHide",          IDI_SHOWHIDE },
	{ LPGEN("On Top Caption Icon"),     "CaptionIcon",       IDI_CAPTIONICON },
	{ LPGEN("Delete All Reminders"),    "DeleteReminder",    IDI_DELETEREMINDER },
	{ LPGEN("View Reminders"),          "ViewReminders",     IDI_VIEWREMINDERS },
	{ LPGEN("Not on Top Caption Icon"), "CaptionIconNotTop", IDI_CAPTIONICONNOTTOP },
	{ LPGEN("Hide Note Icon"),          "HideNote",          IDI_HIDENOTE },
	{ LPGEN("Remove Note Icon"),        "RemoveNote",        IDI_REMOVENOTE },
	{ LPGEN("Reminder Icon"),           "Reminder",          IDI_REMINDER },
	{ LPGEN("Bring All to Front"),      "BringFront",        IDI_BRINGFRONT },
	{ LPGEN("Play Sound Icon"),         "PlaySound",         IDI_PLAYSOUND },
	{ LPGEN("View Notes"),              "ViewNotes",         IDI_VIEWNOTES },
	{ LPGEN("New Note"),                "NewNote",           IDI_NOTEICON },
	{ LPGEN("New Reminder"),            "NewReminder",       IDI_ADDREMINDER }
};

void InitIcons(void)
{
	Icon_Register(hinstance, LPGEN("Sticky Notes"), iconList, _countof(iconList), MODULENAME);
}

static int OnOptInitialise(WPARAM w, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 900002000;
	odp.hInstance = hinstance;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_STNOTEOPTIONS);
	odp.szTitle.a = SECTIONNAME;
	odp.szGroup.a = LPGEN("Plugins");
	odp.pfnDlgProc = DlgProcOptions;
	Options_AddPage(w, &odp);
	return 0;
}

int OnTopToolBarInit(WPARAM, LPARAM)
{
	TTBButton ttb = { 0 };
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;

	ttb.hIconHandleUp = iconList[14].hIcolib;
	ttb.pszService = MODULENAME"/MenuCommandAddNew";
	ttb.name = ttb.pszTooltipUp = LPGEN("Add New Note");
	TopToolbar_AddButton(&ttb);

	ttb.hIconHandleUp = iconList[15].hIcolib;
	ttb.pszService = MODULENAME"/MenuCommandNewReminder";
	ttb.name = ttb.pszTooltipUp = LPGEN("Add New Reminder");
	TopToolbar_AddButton(&ttb);

	UnhookEvent(hkTopToolbarInit);
	return 0;
}

static void InitServices()
{
	// register sounds

	Skin_AddSound("AlertReminder",  LPGENW("Alerts"), LPGENW("Reminder triggered"));
	Skin_AddSound("AlertReminder2", LPGENW("Alerts"), LPGENW("Reminder triggered (Alternative 1)"));
	Skin_AddSound("AlertReminder3", LPGENW("Alerts"), LPGENW("Reminder triggered (Alternative 2)"));

	// register menu command services

	CreateServiceFunction(MODULENAME"/MenuCommandAddNew", PluginMenuCommandAddNew);
	CreateServiceFunction(MODULENAME"/MenuCommandShowHide", PluginMenuCommandShowHide);
	CreateServiceFunction(MODULENAME"/MenuCommandViewNotes", PluginMenuCommandViewNotes);
	CreateServiceFunction(MODULENAME"/MenuCommandDeleteAll", PluginMenuCommandDeleteAll);
	CreateServiceFunction(MODULENAME"/MenuCommandBringAllFront", PluginMenuCommandAllBringFront);

	//

	CreateServiceFunction(MODULENAME"/MenuCommandNewReminder", PluginMenuCommandNewReminder);
	CreateServiceFunction(MODULENAME"/MenuCommandViewReminders", PluginMenuCommandViewReminders);
	CreateServiceFunction(MODULENAME"/MenuCommandDeleteReminders", PluginMenuCommandDeleteReminders);

	// register misc

	CreateServiceFunction(MODULENAME"/OpenTriggeredReminder", OpenTriggeredReminder);
}

/////////////////////////////////////////////////////////////////////////////////////////

static void addMenuItem(CMenuItem &mi)
{
	if (g_AddContListMI) {
		HGENMENU save = mi.root; mi.root = NULL;
		Menu_AddContactMenuItem(&mi);
		mi.root = save;
	}
	Menu_AddMainMenuItem(&mi);
}

int OnModulesLoaded(WPARAM, LPARAM)
{
	// register fonts and hotkeys
	RegisterFontServiceFonts();
	RegisterKeyBindings();

	g_AddContListMI = (BOOL)db_get_dw(0, MODULENAME, "AddContactMenuItems", 1);

	// register menus
	CMenuItem mi;
	mi.root = Menu_CreateRoot(MO_MAIN, LPGENW("Notes && Reminders"), 1600000000);
	Menu_ConfigureItem(mi.root, MCI_OPT_UID, "A5E140BC-D697-4689-B75B-8ECFB6FE5931");
	mi.flags = CMIF_UNICODE;

	mi.position = 1600000000;
	mi.hIcolibItem = iconList[2].hIcolib;
	mi.name.w = LPGENW("New &Note");
	mi.pszService = MODULENAME"/MenuCommandAddNew";
	addMenuItem(mi);

	mi.position = 1600000001;
	mi.hIcolibItem = iconList[0].hIcolib;
	mi.name.w = LPGENW("New &Reminder");
	mi.pszService = MODULENAME"/MenuCommandNewReminder";
	addMenuItem(mi);

	mi.position = 1600100000;
	mi.hIcolibItem = iconList[3].hIcolib;
	mi.name.w = LPGENW("&Show / Hide Notes");
	mi.pszService = MODULENAME"/MenuCommandShowHide";
	addMenuItem(mi);

	mi.position = 1600100001;
	mi.hIcolibItem = iconList[13].hIcolib;
	mi.name.w = LPGENW("Vie&w Notes");
	mi.pszService = MODULENAME"/MenuCommandViewNotes";
	addMenuItem(mi);

	mi.position = 1600100002;
	mi.hIcolibItem = iconList[1].hIcolib;
	mi.name.w = LPGENW("&Delete All Notes");
	mi.pszService = MODULENAME"/MenuCommandDeleteAll";
	addMenuItem(mi);

	mi.position = 1600100003;
	mi.hIcolibItem = iconList[11].hIcolib;
	mi.name.w = LPGENW("&Bring All to Front");
	mi.pszService = MODULENAME"/MenuCommandBringAllFront";
	addMenuItem(mi);

	mi.position = 1600200000;
	mi.hIcolibItem = iconList[6].hIcolib;
	mi.name.w = LPGENW("&View Reminders");
	mi.pszService = MODULENAME"/MenuCommandViewReminders";
	addMenuItem(mi);

	mi.position = 1600200001;
	mi.hIcolibItem = iconList[5].hIcolib;
	mi.name.w = LPGENW("D&elete All Reminders");
	mi.pszService = MODULENAME"/MenuCommandDeleteReminders";
	addMenuItem(mi);

	// register misc
	hkOptInit = HookEvent(ME_OPT_INITIALISE, OnOptInitialise);
	hkTopToolbarInit = HookEvent("TopToolBar/ModuleLoaded", OnTopToolBarInit);
	UnhookEvent(hkModulesLoaded);

	// init vars and load all data
	InitSettings();
	CreateMsgWindow();
	LoadNotes(TRUE);
	LoadReminders();
	return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	CloseNotesList();
	CloseReminderList();
	SaveNotes();
	SaveReminders();
	DestroyMsgWindow();
	WS_CleanUp();
	TermSettings();

	UnhookEvent(hkFontChange);
	UnhookEvent(hkColorChange);

	UnhookEvent(hkOptInit);

	IcoLib_ReleaseIcon(g_hReminderIcon);
	DeleteObject(hBodyFont);
	DeleteObject(hCaptionFont);

	if (hRichedDll)
		FreeLibrary(hRichedDll);

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD, LPVOID)
{
	hinstance = hinst;
	return TRUE;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);
	pcli = Clist_GetInterface();
	hmiranda = GetModuleHandle(NULL);

	INITCOMMONCONTROLSEX ctrls = { 0 };
	ctrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ctrls.dwICC = ICC_DATE_CLASSES;
	InitCommonControlsEx(&ctrls);

	hRichedDll = LoadLibrary("Msftedit.dll");
	if (!hRichedDll) {
		if (MessageBox(0, Translate("Miranda could not load the Notes & Reminders plugin, Msftedit.dll is missing. If you are using WINE, please make sure you have Msftedit.dll installed. Press 'Yes' to continue loading Miranda."), SECTIONNAME, MB_YESNO | MB_ICONINFORMATION) != IDYES)
			return 1;
		return 0;
	}

	InitServices();
	WS_Init();

	hkModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	InitIcons();

	return 0;
}
