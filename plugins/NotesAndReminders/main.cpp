#include "globals.h"

HINSTANCE hinstance = NULL;
HINSTANCE hmiranda = NULL;
int hLangpack;

HANDLE hkOptInit = NULL;
HANDLE hkTopToolbarInit = NULL; 
HANDLE hkModulesLoaded = NULL;
HANDLE hkFontChange = NULL;
HANDLE hkColorChange = NULL;
HMODULE hUserDll = NULL;
HMODULE hRichedDll = NULL;
HMODULE hKernelDll = NULL;

extern TREEELEMENT *g_Stickies;
extern TREEELEMENT *RemindersList;

static PLUGININFOEX pluginInfo =
{
	sizeof(PLUGININFOEX),
	"Sticky Notes & Reminders",
	PLUGIN_MAKE_VERSION(0,0,5,1),
	"Sticky Notes & Reminders Implementation for Miranda IM.",
	"Joe Kucera, Lubomir Kolev Ivanov, Georg Fischer",
	"jokusoftware@users.sourceforge.net; d00mEr@dir.bg",
	"(C) 2003,2005 Joe Kucera, Lubomir Ivanov",
	"http://d00mer.freeshell.org/miranda/",
	UNICODE_AWARE,
	MIID_NNR
};


void RegisterFontServiceFonts();
void RegisterKeyBindings();
int OpenTriggeredReminder(WPARAM w, LPARAM l);
void BringAllNotesToFront(STICKYNOTE *pActive);
void CloseNotesList();
void CloseReminderList();

int PluginMenuCommandAddNew(WPARAM w,LPARAM l)
{
	STICKYNOTE* PSN;
	PSN = NewNote(0,0,0,0,NULL,NULL,TRUE,TRUE,0);
	SetFocus(PSN->REHwnd);
	return 0;
}

int PluginMenuCommandDeleteAll(WPARAM w,LPARAM l)
{
	if (g_Stickies && MessageBox(NULL, Translate("Are you sure you want to delete all notes?"), SECTIONNAME, MB_OKCANCEL) == IDOK)
		DeleteNotes();
	return 0;
}

int PluginMenuCommandShowHide(WPARAM w,LPARAM l)
{
	ShowHideNotes();
	return 0;
}

int PluginMenuCommandViewNotes(WPARAM w,LPARAM l)
{
	ListNotes();
	return 0;
}

int PluginMenuCommandAllBringFront(WPARAM w,LPARAM l)
{
	BringAllNotesToFront(NULL);
	return 0;
}

int PluginMenuCommandNewReminder(WPARAM w,LPARAM l)
{
	NewReminder();
	return 0;
}

int PluginMenuCommandViewReminders(WPARAM w,LPARAM l)
{
	ListReminders();
	return 0;
}

int PluginMenuCommandDeleteReminders(WPARAM w,LPARAM l)
{
	if (RemindersList && MessageBox(NULL, Translate("Are you sure you want to delete all reminders?"), SECTIONNAME, MB_OKCANCEL) == IDOK)
		DeleteReminders();
	return 0;
}

struct
{
	char* szDescr;
	char* szName;
	int   defIconID;
}
static const iconList[] =
{
	{"New Reminder", "AddReminder", IDI_ADDREMINDER},
	{"Delete All Notes", "DeleteIcon", IDI_DELETEICON},
	{"New Note", "NoteIcon", IDI_NOTEICON},
	{"Show/Hide Notes", "ShowHide", IDI_SHOWHIDE},
	{"On Top Caption Icon", "CaptionIcon", IDI_CAPTIONICON},
	{"Delete All Reminders", "DeleteReminder", IDI_DELETEREMINDER},
	{"View Reminders", "ViewReminders", IDI_VIEWREMINDERS},
	{"Not on Top Caption Icon", "CaptionIconNotTop", IDI_CAPTIONICONNOTTOP},
	{"Hide Note Icon", "HideNote", IDI_HIDENOTE},
	{"Remove Note Icon", "RemoveNote", IDI_REMOVENOTE},
	{"Reminder Icon", "Reminder", IDI_REMINDER},
	{"Bring All to Front", "BringFront", IDI_BRINGFRONT},
	{"Play Sound Icon", "PlaySound", IDI_PLAYSOUND},
	{"View Notes", "ViewNotes", IDI_VIEWNOTES},
	{"New Note", "NewNote", IDI_NOTEICON},
	{"New Reminder", "NewReminder", IDI_ADDREMINDER}
};

HANDLE hIconLibItem[SIZEOF(iconList)];

void InitIcons(void)
{
	int i;
	char szSettingName[100];
	SKINICONDESC sid = {0};
	TCHAR szFile[MAX_PATH];

	GetModuleFileName(hinstance, szFile, SIZEOF(szFile));

	sid.cbSize = sizeof(SKINICONDESC);
	sid.ptszDefaultFile = szFile;
	sid.pszName = szSettingName;
	sid.pszSection = MODULENAME;
	sid.flags = SIDF_PATH_TCHAR;

	for (i = 0; i < SIZEOF(iconList); i++) 
	{
		mir_snprintf(szSettingName, SIZEOF(szSettingName), "%s_%s", MODULENAME, iconList[i].szName);

		sid.pszDescription = iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].defIconID;
		hIconLibItem[i] = Skin_AddIcon(&sid);
	}	
}

int OnOptInitialise(WPARAM w, LPARAM L)
{
	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof(odp);
	odp.position = 900002000;
	odp.hInstance = hinstance;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_STNOTEOPTIONS);
	odp.pszTitle = _T(SECTIONNAME);
	odp.pszGroup = _T("Plugins");
	odp.pfnDlgProc = DlgProcOptions;
	Options_AddPage(w, &odp);
	return 0;
}

int OnTopToolBarInit(WPARAM w,LPARAM L) 
{
	TTBButton ttb = {0};
	ttb.cbSize = sizeof(TTBButton);
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP | TTBBF_ICONBYHANDLE;

	ttb.hIconHandleUp = ttb.hIconHandleDn = hIconLibItem[14];
	ttb.pszService = MODULENAME"/MenuCommandAddNew";
	ttb.name = "Add New Note";
	TopToolbar_AddButton(&ttb);

	ttb.hIconHandleUp = ttb.hIconHandleDn = hIconLibItem[15];
	ttb.pszService = MODULENAME"/MenuCommandNewReminder";
	ttb.name = "Add New Reminder";
	TopToolbar_AddButton(&ttb);

	UnhookEvent(hkTopToolbarInit);
	return 0;
}

static void InitServices()
{
	// register sounds

	SkinAddNewSoundEx("AlertReminder", "Alerts", "Reminder triggered");
	SkinAddNewSoundEx("AlertReminder2", "Alerts", "Reminder triggered (Alternative 1)");
	SkinAddNewSoundEx("AlertReminder3", "Alerts", "Reminder triggered (Alternative 2)");

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

int OnModulesLoaded(WPARAM wparam,LPARAM lparam)
{
	CLISTMENUITEM cmi = {0};

	// register fonts and hotkeys

	RegisterFontServiceFonts();
	RegisterKeyBindings();

	// register menus

	cmi.cbSize = sizeof(cmi);
	cmi.pszContactOwner = NULL;
	cmi.pszPopupName = "Notes && Reminders";
	cmi.flags = CMIM_ICON | CMIF_ICONFROMICOLIB;

	cmi.position = 1600000000;
	cmi.icolibItem = hIconLibItem[2];
	cmi.pszName = "New &Note";
	cmi.pszService = MODULENAME"/MenuCommandAddNew";
	if (g_AddContListMI) Menu_AddContactMenuItem(&cmi);
	Menu_AddMainMenuItem(&cmi);

	cmi.position = 1600000001;
	cmi.icolibItem = hIconLibItem[0];
	cmi.pszName = "New &Reminder";
	cmi.pszService = MODULENAME"/MenuCommandNewReminder";
	if (g_AddContListMI) Menu_AddContactMenuItem(&cmi);
	Menu_AddMainMenuItem(&cmi);

	//

	cmi.position = 1600100000;
	cmi.icolibItem = hIconLibItem[3];
	cmi.pszName = "&Show / Hide Notes";
	cmi.pszService = MODULENAME"/MenuCommandShowHide";
	if (g_AddContListMI) Menu_AddContactMenuItem(&cmi);
	Menu_AddMainMenuItem(&cmi);

	cmi.position = 1600100001;
	cmi.icolibItem = hIconLibItem[13];
	cmi.pszName = "Vie&w Notes";
	cmi.pszService = MODULENAME"/MenuCommandViewNotes";
	if (g_AddContListMI) Menu_AddContactMenuItem(&cmi);
	Menu_AddMainMenuItem(&cmi);

	cmi.position = 1600100002;
	cmi.icolibItem = hIconLibItem[1];
	cmi.pszName = "&Delete All Notes";
	cmi.pszService = MODULENAME"/MenuCommandDeleteAll";
	if (g_AddContListMI) Menu_AddContactMenuItem(&cmi);
	Menu_AddMainMenuItem(&cmi);

	cmi.position = 1600100003;
	cmi.icolibItem = hIconLibItem[11];
	cmi.pszName = "&Bring All to Front";
	cmi.pszService = MODULENAME"/MenuCommandBringAllFront";
	if (g_AddContListMI) Menu_AddContactMenuItem(&cmi);
	Menu_AddMainMenuItem(&cmi);

	//

	cmi.position = 1600200000;
	cmi.icolibItem = hIconLibItem[6];
	cmi.pszName = "&View Reminders";
	cmi.pszService = MODULENAME"/MenuCommandViewReminders";
	if (g_AddContListMI) Menu_AddContactMenuItem(&cmi);
	Menu_AddMainMenuItem(&cmi);

	cmi.position = 1600200001;
	cmi.icolibItem = hIconLibItem[5];
	cmi.pszName = "D&elete All Reminders";
	cmi.pszService = MODULENAME"/MenuCommandDeleteReminders";
	if (g_AddContListMI) Menu_AddContactMenuItem(&cmi);
	Menu_AddMainMenuItem(&cmi);

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
	if (hUserDll)
		FreeLibrary(hUserDll);
	if (hKernelDll)
		FreeLibrary(hKernelDll);

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

	INITCOMMONCONTROLSEX ctrls = {0};
	ctrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ctrls.dwICC = ICC_DATE_CLASSES;
	InitCommonControlsEx(&ctrls);

	g_isWin2kPlus = IsWinVer2000Plus();

	hRichedDll = LoadLibrary("RICHED20.DLL");
	if (!hRichedDll)
	{
		if (MessageBox(0, Translate("Miranda could not load the Note & Reminders plugin, RICHED20.DLL is missing. If you are using Windows 95 or WINE please make sure you have riched20.dll installed. Press 'Yes' to continue loading Miranda."), SECTIONNAME, MB_YESNO | MB_ICONINFORMATION) != IDYES)
			return 1;
		return 0;
	}

	hUserDll = LoadLibrary("user32.dll");
	if (hUserDll) 
	{
		MySetLayeredWindowAttributes = (BOOL (WINAPI *)(HWND,COLORREF,BYTE,DWORD))GetProcAddress(hUserDll,"SetLayeredWindowAttributes");
		MyMonitorFromWindow = (HANDLE (WINAPI*)(HWND,DWORD))GetProcAddress(hUserDll,"MonitorFromWindow");
	}
	else
	{
		MySetLayeredWindowAttributes = NULL;
		MyMonitorFromWindow = NULL;
	}

	hKernelDll = LoadLibrary("kernel32.dll");
	if (hKernelDll) 
	{
		MyTzSpecificLocalTimeToSystemTime = (BOOL (WINAPI*)(LPTIME_ZONE_INFORMATION,LPSYSTEMTIME,LPSYSTEMTIME))GetProcAddress(hKernelDll,"TzSpecificLocalTimeToSystemTime");
		MySystemTimeToTzSpecificLocalTime = (BOOL (WINAPI*)(LPTIME_ZONE_INFORMATION,LPSYSTEMTIME,LPSYSTEMTIME))GetProcAddress(hKernelDll,"SystemTimeToTzSpecificLocalTime");
	}
	else
	{
		MyTzSpecificLocalTimeToSystemTime = NULL;
		MySystemTimeToTzSpecificLocalTime = NULL;
	}

	InitServices();
	WS_Init();

	hkModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED,OnModulesLoaded);
	InitIcons();

	return 0;
}