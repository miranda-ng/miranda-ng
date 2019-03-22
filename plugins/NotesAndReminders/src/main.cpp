#include "stdafx.h"

CMPlugin g_plugin;
HINSTANCE hmiranda = nullptr;

HMODULE hRichedDll = nullptr;

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
	{0x842a6668, 0xf9da, 0x4968, {0xbf, 0xd7, 0xd2, 0xbd, 0x9d, 0xf8, 0x48, 0xee}} // {842A6668-F9DA-4968-BFD7-D2BD9DF848EE}
};

/////////////////////////////////////////////////////////////////////////////////////////

void RegisterFontServiceFonts();
void RegisterKeyBindings();
INT_PTR OpenTriggeredReminder(WPARAM w, LPARAM l);
void CloseNotesList();
void CloseReminderList();

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
	g_plugin.registerIcon(LPGEN("Sticky Notes"), iconList, MODULENAME);
}

int OnTopToolBarInit(WPARAM, LPARAM)
{
	TTBButton ttb = {};
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;

	ttb.hIconHandleUp = iconList[14].hIcolib;
	ttb.pszService = MODULENAME"/MenuCommandAddNew";
	ttb.name = ttb.pszTooltipUp = LPGEN("Add New Note");
	g_plugin.addTTB(&ttb);

	ttb.hIconHandleUp = iconList[15].hIcolib;
	ttb.pszService = MODULENAME"/MenuCommandNewReminder";
	ttb.name = ttb.pszTooltipUp = LPGEN("Add New Reminder");
	g_plugin.addTTB(&ttb);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void addMenuItem(CMenuItem &mi)
{
	if (g_plugin.bAddContListMI) {
		HGENMENU save = mi.root; mi.root = nullptr;
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

	// register menus
	CMenuItem mi(&g_plugin);
	mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("Notes && Reminders"), 1600000000);
	Menu_ConfigureItem(mi.root, MCI_OPT_UID, "A5E140BC-D697-4689-B75B-8ECFB6FE5931");
	mi.flags = CMIF_UNICODE;

	mi.position = 1600000000;
	mi.hIcolibItem = iconList[2].hIcolib;
	mi.name.w = LPGENW("New &Note");
	mi.pszService = MODULENAME"/MenuCommandAddNew";
	addMenuItem(mi);
	CreateServiceFunction(mi.pszService, PluginMenuCommandAddNew);

	mi.position = 1600000001;
	mi.hIcolibItem = iconList[0].hIcolib;
	mi.name.w = LPGENW("New &Reminder");
	mi.pszService = MODULENAME"/MenuCommandNewReminder";
	addMenuItem(mi);
	CreateServiceFunction(mi.pszService, PluginMenuCommandNewReminder);

	mi.position = 1600100000;
	mi.hIcolibItem = iconList[3].hIcolib;
	mi.name.w = LPGENW("&Show / Hide Notes");
	mi.pszService = MODULENAME"/MenuCommandShowHide";
	addMenuItem(mi);
	CreateServiceFunction(mi.pszService, PluginMenuCommandShowHide);

	mi.position = 1600100001;
	mi.hIcolibItem = iconList[13].hIcolib;
	mi.name.w = LPGENW("Vie&w Notes");
	mi.pszService = MODULENAME"/MenuCommandViewNotes";
	addMenuItem(mi);
	CreateServiceFunction(mi.pszService, PluginMenuCommandViewNotes);

	mi.position = 1600100002;
	mi.hIcolibItem = iconList[1].hIcolib;
	mi.name.w = LPGENW("&Delete All Notes");
	mi.pszService = MODULENAME"/MenuCommandDeleteAll";
	addMenuItem(mi);
	CreateServiceFunction(mi.pszService, PluginMenuCommandDeleteNotes);

	mi.position = 1600100003;
	mi.hIcolibItem = iconList[11].hIcolib;
	mi.name.w = LPGENW("&Bring All to Front");
	mi.pszService = MODULENAME"/MenuCommandBringAllFront";
	addMenuItem(mi);
	CreateServiceFunction(mi.pszService, PluginMenuCommandAllBringFront);

	mi.position = 1600200000;
	mi.hIcolibItem = iconList[6].hIcolib;
	mi.name.w = LPGENW("&View Reminders");
	mi.pszService = MODULENAME"/MenuCommandViewReminders";
	addMenuItem(mi);
	CreateServiceFunction(mi.pszService, PluginMenuCommandViewReminders);

	mi.position = 1600200001;
	mi.hIcolibItem = iconList[5].hIcolib;
	mi.name.w = LPGENW("D&elete All Reminders");
	mi.pszService = MODULENAME"/MenuCommandDeleteReminders";
	addMenuItem(mi);
	CreateServiceFunction(mi.pszService, PluginMenuCommandDeleteReminders);

	// register misc
	HookEvent("TopToolBar/ModuleLoaded", OnTopToolBarInit);

	// init vars and load all data
	InitSettings();
	CreateMsgWindow();
	LoadNotes(true);
	LoadReminders();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx),
	bAddContListMI(MODULENAME, "AddContactMenuItems", true),
	bShowNotesAtStart(MODULENAME, "ShowNotesAtStart", true),
	bShowNoteButtons(MODULENAME, "ShowNoteButtons", true),
	bShowScrollbar(MODULENAME, "ShowScrollbar", true),
	bCloseAfterAddReminder(MODULENAME, "CloseAfterAddReminder", true),
	bUseMSI(MODULENAME, "UseMCI", true)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	hmiranda = GetModuleHandle(nullptr);

	INITCOMMONCONTROLSEX ctrls = { 0 };
	ctrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ctrls.dwICC = ICC_DATE_CLASSES;
	InitCommonControlsEx(&ctrls);

	hRichedDll = LoadLibraryA("Msftedit.dll");
	if (!hRichedDll) {
		if (MessageBox(nullptr, TranslateT("Miranda could not load the Notes & Reminders plugin, Msftedit.dll is missing. If you are using WINE, please make sure you have Msftedit.dll installed. Press 'Yes' to continue loading Miranda."), _A2W(SECTIONNAME), MB_YESNO | MB_ICONINFORMATION) != IDYES)
			return 1;
		return 0;
	}

	CreateServiceFunction(MODULENAME"/OpenTriggeredReminder", OpenTriggeredReminder);

	// register sounds
	g_plugin.addSound("AlertReminder", LPGENW("Alerts"), LPGENW("Reminder triggered"));
	g_plugin.addSound("AlertReminder2", LPGENW("Alerts"), LPGENW("Reminder triggered (Alternative 1)"));
	g_plugin.addSound("AlertReminder3", LPGENW("Alerts"), LPGENW("Reminder triggered (Alternative 2)"));

	WS_Init();

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_OPT_INITIALISE, OnOptInitialise);
	InitIcons();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	CloseNotesList();
	CloseReminderList();
	SaveNotes();
	SaveReminders();
	DestroyMsgWindow();
	WS_CleanUp();
	TermSettings();

	IcoLib_ReleaseIcon(g_hReminderIcon);
	DeleteObject(hBodyFont);
	DeleteObject(hCaptionFont);

	if (hRichedDll)
		FreeLibrary(hRichedDll);

	return 0;
}
