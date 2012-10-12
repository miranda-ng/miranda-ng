#define _CRT_SECURE_NO_WARNINGS
#define MIRANDA_VER 0x0A00
#include <tchar.h>
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <strsafe.h>
#include <commctrl.h> //for ImageList_*
#include <prsht.h>
#include <ShObjIdl.h>
#include <ShlGuid.h>

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_avatars.h>
#include <m_database.h>
#include <m_system.h>
#include <m_protosvc.h>
#include <m_contacts.h>
#include <m_popup.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_history.h>
#include <m_imgsrvc.h>
#include <m_icolib.h>

#include <m_folders.h>
#include <m_metacontacts.h>
#include "m_avatarhist.h"
#include "m_historyevents.h"

#include "resource.h"
#include "Version.h"
#include "../utils/mir_buffer.h"

// Globals
extern HINSTANCE hInst;
extern HANDLE hMenu;
extern DWORD mirVer;
extern HANDLE hAvatarWindowsList;

#define MODULE_NAME "AvatarHistory"

#define AVH_DEF_POPUPFG 0
#define AVH_DEF_POPUPBG 0x2DB6FF
#define AVH_DEF_AVPOPUPS 0
#define AVH_DEF_LOGTODISK 1
#define AVH_DEF_LOGKEEPSAMEFOLDER 0
#define AVH_DEF_LOGOLDSTYLE 0
#define AVH_DEF_LOGTOHISTORY 1
#define AVH_DEF_DEFPOPUPS 0
#define AVH_DEF_SHOWMENU 1

#define DEFAULT_TEMPLATE_REMOVED LPGENT("removed his/her avatar")
#define DEFAULT_TEMPLATE_CHANGED LPGENT("changed his/her avatar")

void LoadOptions();

 // from icolib.cpp
void SetupIcoLib();

HICON createDefaultOverlayedIcon(BOOL big);
HICON createProtoOverlayedIcon(HANDLE hContact);

int PreBuildContactMenu(WPARAM wParam,LPARAM lParam);
int IcoLibIconsChanged(WPARAM wParam,LPARAM lParam);

int OpenAvatarDialog(HANDLE hContact, char* fn);

#define MAX_REGS(_A_) ( sizeof(_A_) / sizeof(_A_[0]) )

#define POPUP_ACTION_DONOTHING 0
#define POPUP_ACTION_CLOSEPOPUP 1
#define POPUP_ACTION_OPENAVATARHISTORY 2
#define POPUP_ACTION_OPENHISTORY 3

#define POPUP_DELAY_DEFAULT 0
#define POPUP_DELAY_CUSTOM 1
#define POPUP_DELAY_PERMANENT 2


struct Options {
	// Log
	BOOL log_per_contact_folders;
	BOOL log_keep_same_folder;
	BOOL log_store_as_hash;

	// Popup
	BOOL popup_show_changed;
	TCHAR popup_changed[1024];
	BOOL popup_show_removed;
	TCHAR popup_removed[1024];
	WORD popup_delay_type;
	WORD popup_timeout;
	BYTE popup_use_win_colors;
	BYTE popup_use_default_colors;
	COLORREF popup_bkg_color;
	COLORREF popup_text_color;
	WORD popup_left_click_action;
	WORD popup_right_click_action;
};

extern Options opts;


#include "popup.h"
