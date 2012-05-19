#ifndef _OPTIONS_INC
#define _OPTIONS_INC

#include "services.h"
#include "icons.h"

typedef enum {VR_MAJOR, VR_MINOR, VR_RELEASE, VR_BUILD} VersionRequirement;

typedef struct Options_tag {
	bool check_on_startup;
	bool check_once;
	bool check_daily;
	bool no_conf_idle;
	bool backup;
	TCHAR temp_folder[MAX_PATH];
	TCHAR backup_folder[MAX_PATH];
	TCHAR data_folder[MAX_PATH];
	bool use_xml_backend;
	bool start_offline;
	bool popup_notify;
	bool set_colours;
	bool restart_menu_item;
	bool update_and_exit_menu_item;
	COLORREF bkCol;
	COLORREF textCol;
	VersionRequirement ver_req;
	bool save_zips;
	bool no_unzip;
	bool auto_dll_only;
	TCHAR zip_folder[MAX_PATH];
} Options;

extern Options options;

extern HWND hwndOptions;

#define WMU_CHECKING		(WM_USER + 41)
#define WMU_DONECHECKING	(WM_USER + 42)


int OptInit(WPARAM wParam, LPARAM lParam);

void LoadOptions();
void SaveOptions();

// do this after icolib support has been handled
void InitOptionsMenuItems();

#endif
