#ifndef _SERVICES_INC
#define _SERVICES_INC

#include "m_updater.h"
#include "options.h"
#include "xmldata.h"
#include "conf_dialog.h"
#include "conf_comp_dialog.h"
#include "progress_dialog.h"
#include "extern.h"
#include "scan.h"
#include "popups.h"

#include "socket.h"
#include "allocations.h"
#include "utils.h"
#include "updater.h"


void RestoreStatus();

INT_PTR Register(WPARAM wParam, LPARAM lParam);

////////////////////////////
// used internally
/////////////////////

// used to create the 'Check for Updates' trigger action in triggerplugin
#define MS_UPDATE_CHECKFORUPDATESTRGR			"Update/CheckForUpdatesTrigger"


#define MS_UPDATE_CHECKFORUPDATES			"Update/CheckForUpdates"
// wParam = (BOOL)don't confirm
// lParam = (BOOL)don't restart after update
// returns 1 if user cancels or there are no plugins to update, 0 otherwise (implies impending restart)
INT_PTR CheckForUpdates(WPARAM wParam, LPARAM lParam);

#define MS_UPDATE_MENUCHECKFORUPDATES	"Update/MenuCheckForUpdates"
__inline int MenuCheckForUpdates(WPARAM wParam, LPARAM lParam) {
	return CheckForUpdates(0,0);
}

#define MS_UPDATE_MENURESTART			"Update/MenuRestart"
INT_PTR Restart(WPARAM wParam, LPARAM lParam);

#define MS_UPDATE_MENUUPDATEANDEXIT		"Update/MenuUpdateAndExit"
INT_PTR UpdateAndExit(WPARAM wParam, LPARAM lParam);

#define MS_UPDATE_SETUPDATEOPTIONS			"Update/SetUpdateOptions"
// wParam = (char *)szComponentName
// lParam = (UpdateOptions *)&update_options
INT_PTR SetUpdateOptions(WPARAM wParam, LPARAM lParam);

#define MS_UPDATE_GETUPDATEOPTIONS			"Update/GetUpdateOptions"
// wParam = (char *)szComponentName
// lParam = (UpdateOptions *)&update_options
INT_PTR GetUpdateOptions(WPARAM wParam, LPARAM lParam);

void LoadUpdateOptions(char *szComponentName, UpdateOptions *update_options);

// callback function typedef for enumerating installed plugins with their associated options
// (warning - critical section locked during this function call - so don't call other services or functions from this plugin that use the update list)
typedef int (*UpdateEnumerateFunc)(char *szComonentName, UpdateOptions *update_options, LPARAM lParam);
#define MS_UPDATE_ENUMERATE					"Update/EnumerateUpdates"
// wParam = (UpdateEnumerateFunc)enum_func
// lParam = lParam
INT_PTR EnumerateUpdates(WPARAM wParam, LPARAM lParam);
////////////////////////////

bool IsRegistered(int file_id);
bool RegisterForFileListing(int file_id, PLUGININFO *pluginInfo, bool auto_register);
bool RegisterForFileListing(int file_id, const char *fl_name, DWORD version, bool auto_register, const Category cat);

void InitServices();
void PreDeinitServices();
void DeinitServices();

#endif
