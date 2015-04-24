#include <windows.h>
#include <time.h>
#include <ShObjIdl.h>
#include <ShlGuid.h>

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_avatars.h>
#include <m_database.h>
#include <m_protosvc.h>
#include <m_contacts.h>
#include <m_popup.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_history.h>
#include <m_imgsrvc.h>
#include <m_icolib.h>
#include <win2k.h>

#include <m_folders.h>
#include <m_metacontacts.h>
#include <m_avatarhistory.h>

#include "../utils/mir_options.h"

#include "resource.h"
#include "Version.h"
#include "popup.h"

// Globals
extern HINSTANCE hInst;
extern HGENMENU hMenu;
extern DWORD mirVer;
extern HANDLE hAvatarWindowsList;
extern Options opts;
extern HANDLE hFolder;
extern TCHAR basedir[];

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
HICON createProtoOverlayedIcon(MCONTACT hContact);

int PreBuildContactMenu(WPARAM wParam,LPARAM lParam);
int IcoLibIconsChanged(WPARAM wParam,LPARAM lParam);

int OpenAvatarDialog(MCONTACT hContact, char* fn);

// utils

TCHAR* GetCachedAvatar(char *proto, TCHAR *hash);
BOOL   ProtocolEnabled(const char *proto);
BOOL   ContactEnabled(MCONTACT hContact, char *setting, int def);
BOOL   CopyImageFile(TCHAR *old_file, TCHAR *new_file);
void   ConvertToFilename(TCHAR *str, size_t size);

void   CreateOldStyleShortcut(MCONTACT hContact, TCHAR *history_filename);
BOOL   CreateShortcut(TCHAR *file, TCHAR *shortcut);

#define POPUP_ACTION_DONOTHING 0
#define POPUP_ACTION_CLOSEPOPUP 1
#define POPUP_ACTION_OPENAVATARHISTORY 2
#define POPUP_ACTION_OPENHISTORY 3

#define POPUP_DELAY_DEFAULT 0
#define POPUP_DELAY_CUSTOM 1
#define POPUP_DELAY_PERMANENT 2
