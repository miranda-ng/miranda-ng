#include <windows.h>
#include <CommCtrl.h>
#include <WinSock.h>

#include <newpluginapi.h>
#include <m_core.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_popup.h>
#include <m_langpack.h>
#include <m_utils.h>
#include <m_icolib.h>
#include <m_options.h>
#include <m_protocols.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include "win2k.h"

#include <m_toptoolbar.h>

#include "version.h"
#include "resource.h"

static INT_PTR AdvSt();
static INT_PTR InitMenu();
void InitSettings();
void LoadSettings();
void DefSettings();
void IsMenu();
void UpdateMenu();
void UpdateTTB();

INT_PTR SturtupSilenceEnabled(WPARAM wParam, LPARAM lParam);
INT_PTR SilenceConnection(WPARAM wParam, LPARAM lParam);

#define MODULE_NAME "StartupSilence"

#define SS_SERVICE_NAME "StartupSilence/ToggleEnabled"

//StartupSilence/SilenceConnection
//external service for mute sound and popups according to plugin settings
//wParam = 2 - SilenceConnection is on
//wParam = 0 - SilenceConnection is off
//lParam = 0 - not in use
#define SS_SILENCE_CONNECTION "StartupSilence/SilenceConnection"

#define POPUPONOFF "Popup/EnableDisableMenuCommand"

#define MENU_NAME			LPGEN("Silence at startup")
#define DISABLE_SILENCE		LPGEN("Toggle Silence OFF")
#define ENABLE_SILENCE		LPGEN("Toggle Silence ON")
#define DISABLE_SILENCETTB	LPGEN("Toggle Silence OFF TTB")
#define ENABLE_SILENCETTB	LPGEN("Toggle Silence ON TTB")
#define ALL_ENABLED			TranslateT("Sounds and popups are enabled")
#define ALL_ENABLED_FLT		LPGEN("Filtered mode ON")
#define ALL_DISABLED		TranslateT("Sounds and popups are disabled temporarily")
#define ALL_DISABLED_FLT	TranslateT("Sounds and popups are disabled temporarily\nAllow mRadio or Weather popups, etc.")
#define S_MODE_CHANGEDON	TranslateT("Silent Startup mode changed to ON")
#define S_MODE_CHANGEDOFF	TranslateT("Silent Startup mode changed to OFF")
#define SS_IS_ON Translate("Silence At Startup is ON")
#define SS_IS_OFF Translate("Silence At Startup is OFF")
#define TTBNAME Translate("Toggle Silence At Startup")
#define NOTICE TranslateT(MENU_NAME)
#define NEEDTTBMOD TranslateT("You have to install TopToolBar to use this option.")
#define NEEDPOPUP TranslateT("You have to install \"YAPP\" or \"Popup+\" to use this option.")

static IconItem iconList[] = {
	{MENU_NAME, MENU_NAME, IDI_SSICON},
	{ALL_ENABLED_FLT, ALL_ENABLED_FLT, IDI_SSFILTEREDON},
	{DISABLE_SILENCE, DISABLE_SILENCE, IDI_SSDISABLE},
	{ENABLE_SILENCE, ENABLE_SILENCE, IDI_SSENABLE}
};

static IconItem iconttbList[] = {
	{DISABLE_SILENCETTB, DISABLE_SILENCETTB, IDI_SSDISABLETTB},
	{ENABLE_SILENCETTB, ENABLE_SILENCETTB, IDI_SSENABLETTB}
};
