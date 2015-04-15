#include "globals.h"

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

INT_PTR PluginMenuCommandAddNew(WPARAM w, LPARAM l)
{
	STICKYNOTE *PSN = NewNote(0,0,0,0,NULL,NULL,TRUE,TRUE,0);
	if(PSN)
		SetFocus(PSN->REHwnd);
	return 0;
}

INT_PTR PluginMenuCommandDeleteAll(WPARAM w, LPARAM l)
{
	if (g_Stickies && MessageBox(NULL, TranslateT("Are you sure you want to delete all notes?"), TranslateT(SECTIONNAME), MB_OKCANCEL) == IDOK)
		DeleteNotes();
	return 0;
}

INT_PTR PluginMenuCommandShowHide(WPARAM w, LPARAM l)
{
	ShowHideNotes();
	return 0;
}

INT_PTR PluginMenuCommandViewNotes(WPARAM w, LPARAM l)
{
	ListNotes();
	return 0;
}

INT_PTR PluginMenuCommandAllBringFront(WPARAM w, LPARAM l)
{
	BringAllNotesToFront(NULL);
	return 0;
}

INT_PTR PluginMenuCommandNewReminder(WPARAM w, LPARAM l)
{
	NewReminder();
	return 0;
}

INT_PTR PluginMenuCommandViewReminders(WPARAM w, LPARAM l)
{
	ListReminders();
	return 0;
}

INT_PTR PluginMenuCommandDeleteReminders(WPARAM w, LPARAM l)
{
	if (RemindersList && MessageBox(NULL, TranslateT("Are you sure you want to delete all reminders?"), TranslateT(SECTIONNAME), MB_OKCANCEL) == IDOK)
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
	Icon_Register(hinstance, LPGEN("Sticky Notes"), iconList, SIZEOF(iconList), MODULENAME);
}

int OnOptInitialise(WPARAM w, LPARAM L)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 900002000;
	odp.hInstance = hinstance;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_STNOTEOPTIONS);
	odp.ptszTitle = SECTIONNAME;
	odp.ptszGroup = LPGENT("Plugins");
	odp.pfnDlgProc = DlgProcOptions;
	odp.flags = ODPF_TCHAR;
	Options_AddPage(w, &odp);
	return 0;
}

int OnTopToolBarInit(WPARAM w, LPARAM L) 
{
	TTBButton ttb = {0};
	ttb.cbSize = sizeof(TTBButton);
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

	SkinAddNewSoundExT("AlertReminder", LPGENT("Alerts"), LPGENT("Reminder triggered"));
	SkinAddNewSoundExT("AlertReminder2", LPGENT("Alerts"), LPGENT("Reminder triggered (Alternative 1)"));
	SkinAddNewSoundExT("AlertReminder3", LPGENT("Alerts"), LPGENT("Reminder triggered (Alternative 2)"));

	// register menu command services

	CreateServiceFunction(MODULENAME"/MenuCommandAddNew",PluginMenuCommandAddNew);
	CreateServiceFunction(MODULENAME"/MenuCommandShowHide",PluginMenuCommandShowHide);
	CreateServiceFunction(MODULENAME"/MenuCommandViewNotes",PluginMenuCommandViewNotes);
	CreateServiceFunction(MODULENAME"/MenuCommandDeleteAll",PluginMenuCommandDeleteAll);
	CreateServiceFunction(MODULENAME"/MenuCommandBringAllFront",PluginMenuCommandAllBringFront);

	//

	CreateServiceFunction(MODULENAME"/MenuCommandNewReminder",PluginMenuCommandNewReminder);
	CreateServiceFunction(MODULENAME"/MenuCommandViewReminders",PluginMenuCommandViewReminders);
	CreateServiceFunction(MODULENAME"/MenuCommandDeleteReminders",PluginMenuCommandDeleteReminders);

	// register misc

	CreateServiceFunction(MODULENAME"/OpenTriggeredReminder",OpenTriggeredReminder);
}

int OnModulesLoaded(WPARAM wparam, LPARAM lparam)
{
	// register fonts and hotkeys
	RegisterFontServiceFonts();
	RegisterKeyBindings();

	g_AddContListMI = (BOOL)db_get_dw(0,MODULENAME,"AddContactMenuItems",1);
	// register menus
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.pszContactOwner = NULL;
	mi.ptszPopupName = LPGENT("Notes && Reminders");
	mi.flags = CMIF_TCHAR;

	mi.position = 1600000000;
	mi.icolibItem = iconList[2].hIcolib;
	mi.ptszName = LPGENT("New &Note");
	mi.pszService = MODULENAME"/MenuCommandAddNew";
	if (g_AddContListMI) Menu_AddContactMenuItem(&mi);
	Menu_AddMainMenuItem(&mi);

	mi.position = 1600000001;
	mi.icolibItem = iconList[0].hIcolib;
	mi.ptszName = LPGENT("New &Reminder");
	mi.pszService = MODULENAME"/MenuCommandNewReminder";
	if (g_AddContListMI) Menu_AddContactMenuItem(&mi);
	Menu_AddMainMenuItem(&mi);

	mi.position = 1600100000;
	mi.icolibItem = iconList[3].hIcolib;
	mi.ptszName = LPGENT("&Show / Hide Notes");
	mi.pszService = MODULENAME"/MenuCommandShowHide";
	if (g_AddContListMI) Menu_AddContactMenuItem(&mi);
	Menu_AddMainMenuItem(&mi);

	mi.position = 1600100001;
	mi.icolibItem = iconList[13].hIcolib;
	mi.ptszName = LPGENT("Vie&w Notes");
	mi.pszService = MODULENAME"/MenuCommandViewNotes";
	if (g_AddContListMI) Menu_AddContactMenuItem(&mi);
	Menu_AddMainMenuItem(&mi);

	mi.position = 1600100002;
	mi.icolibItem = iconList[1].hIcolib;
	mi.ptszName = LPGENT("&Delete All Notes");
	mi.pszService = MODULENAME"/MenuCommandDeleteAll";
	if (g_AddContListMI) Menu_AddContactMenuItem(&mi);
	Menu_AddMainMenuItem(&mi);

	mi.position = 1600100003;
	mi.icolibItem = iconList[11].hIcolib;
	mi.ptszName = LPGENT("&Bring All to Front");
	mi.pszService = MODULENAME"/MenuCommandBringAllFront";
	if (g_AddContListMI) Menu_AddContactMenuItem(&mi);
	Menu_AddMainMenuItem(&mi);

	mi.position = 1600200000;
	mi.icolibItem = iconList[6].hIcolib;
	mi.ptszName = LPGENT("&View Reminders");
	mi.pszService = MODULENAME"/MenuCommandViewReminders";
	if (g_AddContListMI) Menu_AddContactMenuItem(&mi);
	Menu_AddMainMenuItem(&mi);

	mi.position = 1600200001;
	mi.icolibItem = iconList[5].hIcolib;
	mi.ptszName = LPGENT("D&elete All Reminders");
	mi.pszService = MODULENAME"/MenuCommandDeleteReminders";
	if (g_AddContListMI) Menu_AddContactMenuItem(&mi);
	Menu_AddMainMenuItem(&mi);

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

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
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

	Skin_ReleaseIcon(g_hReminderIcon);
	DeleteObject(hBodyFont);
	DeleteObject(hCaptionFont);

	if (hRichedDll)
		FreeLibrary(hRichedDll);

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD fdwReason, LPVOID lpvReserved)
{
	hinstance = hinst;
	return TRUE;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);
	hmiranda = GetModuleHandle(NULL);

	INITCOMMONCONTROLSEX ctrls = {0};
	ctrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ctrls.dwICC = ICC_DATE_CLASSES;
	InitCommonControlsEx(&ctrls);

	hRichedDll = LoadLibrary(_T("Msftedit.dll"));
	if (!hRichedDll) {
		if (MessageBox(0, TranslateT("Miranda could not load the Notes & Reminders plugin, Msftedit.dll is missing. If you are using WINE, please make sure you have Msftedit.dll installed. Press 'Yes' to continue loading Miranda."), _T(SECTIONNAME), MB_YESNO | MB_ICONINFORMATION) != IDYES)
			return 1;
		return 0;
	}

	InitServices();
	WS_Init();

	hkModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	InitIcons();

	return 0;
}
