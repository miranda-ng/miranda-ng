#include "stdafx.h"

CMPlugin g_plugin;
HINSTANCE hmiranda = nullptr;

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
	ttb.pszService = MS_NOTES_NEW;
	ttb.name = ttb.pszTooltipUp = LPGEN("Add New Note");
	g_plugin.addTTB(&ttb);

	ttb.hIconHandleUp = iconList[15].hIcolib;
	ttb.pszService = MS_REMINDER_NEW;
	ttb.name = ttb.pszTooltipUp = LPGEN("Add New Reminder");
	g_plugin.addTTB(&ttb);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void addMenuItem(CMenuItem &mi)
{
	HGENMENU save = mi.root; mi.root = nullptr;
	Menu_AddContactMenuItem(&mi);
	mi.root = save;

	Menu_AddMainMenuItem(&mi);
}

static int OnModulesLoaded(WPARAM, LPARAM)
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
	mi.name.w = LPGENW("New Note");
	mi.pszService = MS_NOTES_NEW;
	SET_UID(mi, 0x1175be03, 0x1c2b, 0x4d87, 0xa7, 0xf3, 0x93, 0x5, 0xa6, 0xa6, 0x3d, 0xa1);
	addMenuItem(mi);
	CreateServiceFunction(mi.pszService, PluginMenuCommandAddNew);

	mi.position = 1600000001;
	mi.hIcolibItem = iconList[0].hIcolib;
	mi.name.w = LPGENW("New Reminder");
	mi.pszService = MS_REMINDER_NEW;
	SET_UID(mi, 0x535d76e7, 0x3874, 0x4aec, 0x87, 0x63, 0xc0, 0x3e, 0xf4, 0x3e, 0xd0, 0x2d);
	addMenuItem(mi);
	CreateServiceFunction(mi.pszService, PluginMenuCommandNewReminder);

	mi.position = 1600100000;
	mi.hIcolibItem = iconList[3].hIcolib;
	mi.name.w = LPGENW("Toggle Notes Visibility");
	mi.pszService = MS_NOTES_SHOWHIDE;
	SET_UID(mi, 0xa6380dc6, 0xf788, 0x451f, 0xa7, 0x9b, 0x78, 0x2f, 0xfd, 0x9e, 0x36, 0x7c);
	addMenuItem(mi);
	CreateServiceFunction(mi.pszService, PluginMenuCommandShowHide);

	mi.position = 1600100001;
	mi.hIcolibItem = iconList[13].hIcolib;
	mi.name.w = LPGENW("View Notes");
	mi.pszService = MODULENAME"/MenuCommandViewNotes";
	SET_UID(mi, 0xcff6908a, 0x713, 0x42c1, 0xb0, 0xa3, 0x1e, 0x36, 0x51, 0xa9, 0xd7, 0xdb);
	addMenuItem(mi);
	CreateServiceFunction(mi.pszService, PluginMenuCommandViewNotes);

	mi.position = 1600100002;
	mi.hIcolibItem = iconList[1].hIcolib;
	mi.name.w = LPGENW("Delete All Notes");
	mi.pszService = MODULENAME"/MenuCommandDeleteAll";
	SET_UID(mi, 0xdee35dce, 0x43a3, 0x46b7, 0xb3, 0x86, 0x6f, 0xc9, 0x8f, 0xa1, 0x46, 0xd9);
	addMenuItem(mi);
	CreateServiceFunction(mi.pszService, PluginMenuCommandDeleteNotes);

	mi.position = 1600100003;
	mi.hIcolibItem = iconList[11].hIcolib;
	mi.name.w = LPGENW("Bring All to Front");
	mi.pszService = MS_NOTES_DISPLAY;
	SET_UID(mi, 0x1b71f069, 0x7788, 0x488b, 0xb1, 0x7a, 0x74, 0x57, 0x14, 0xac, 0x20, 0x47);
	addMenuItem(mi);
	CreateServiceFunction(mi.pszService, PluginMenuCommandAllBringFront);

	mi.position = 1600200000;
	mi.hIcolibItem = iconList[6].hIcolib;
	mi.name.w = LPGENW("View Reminders");
	mi.pszService = MS_REMINDER_VIEW;
	SET_UID(mi, 0x87f35fa7, 0x375d, 0x44e6, 0x89, 0xca, 0x37, 0x30, 0xe, 0x1f, 0xe7, 0x93);
	addMenuItem(mi);
	CreateServiceFunction(mi.pszService, PluginMenuCommandViewReminders);

	mi.position = 1600200001;
	mi.hIcolibItem = iconList[5].hIcolib;
	mi.name.w = LPGENW("Delete All Reminders");
	mi.pszService = MODULENAME"/MenuCommandDeleteReminders";
	SET_UID(mi, 0x20ca5f76, 0xc4c1, 0x4bd2, 0xbf, 0xc3, 0x13, 0xfe, 0x39, 0x9, 0x79, 0x52);
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

static int OnPreShutdown(WPARAM, LPARAM)
{
	CloseNotesList();
	CloseReminderList();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx),
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

	CreateServiceFunction(MODULENAME"/OpenTriggeredReminder", OpenTriggeredReminder);

	// register sounds
	g_plugin.addSound("AlertReminder", LPGENW("Alerts"), LPGENW("Reminder triggered"));
	g_plugin.addSound("AlertReminder2", LPGENW("Alerts"), LPGENW("Reminder triggered (Alternative 1)"));
	g_plugin.addSound("AlertReminder3", LPGENW("Alerts"), LPGENW("Reminder triggered (Alternative 2)"));

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);
	HookEvent(ME_OPT_INITIALISE, OnOptInitialise);
	InitIcons();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	SaveNotes();
	SaveReminders();
	DestroyMsgWindow();
	TermSettings();

	IcoLib_ReleaseIcon(g_hReminderIcon);
	DeleteObject(hBodyFont);
	DeleteObject(hCaptionFont);
	return 0;
}
