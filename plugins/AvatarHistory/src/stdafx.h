#pragma once

#include <windows.h>
#include <windowsx.h>
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
#include <m_timezones.h>
#include <m_icolib.h>

#include <m_folders.h>
#include <m_metacontacts.h>
#include <m_avatarhistory.h>

#include "../../utils/mir_options.h"

#include "resource.h"
#include "version.h"
#include "popup.h"

// Globals
extern HGENMENU hMenu;
extern uint32_t mirVer;
extern MWindowList hAvatarWindowsList;
extern Options opts;
extern HANDLE hFolder;
extern wchar_t basedir[];

#define MODULENAME "AvatarHistory"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

#define AVH_DEF_POPUPFG 0
#define AVH_DEF_POPUPBG 0x2DB6FF
#define AVH_DEF_AVPOPUPS 0
#define AVH_DEF_LOGTODISK 1
#define AVH_DEF_LOGKEEPSAMEFOLDER 0
#define AVH_DEF_LOGOLDSTYLE 0
#define AVH_DEF_LOGTOHISTORY 1
#define AVH_DEF_DEFPOPUPS 0
#define AVH_DEF_SHOWMENU 1

#define DEFAULT_TEMPLATE_REMOVED LPGENW("removed his/her avatar")
#define DEFAULT_TEMPLATE_CHANGED LPGENW("changed his/her avatar")

void LoadOptions();

 // from icolib.cpp
void SetupIcoLib();

HICON createDefaultOverlayedIcon(BOOL big);
HICON createProtoOverlayedIcon(MCONTACT hContact);

int PreBuildContactMenu(WPARAM wParam,LPARAM lParam);
int IcoLibIconsChanged(WPARAM wParam,LPARAM lParam);

int OpenAvatarDialog(MCONTACT hContact, char* fn);

// utils

wchar_t* GetCachedAvatar(char *proto, wchar_t *hash);
bool   ProtocolEnabled(const char *proto);
bool   ContactEnabled(MCONTACT hContact, char *setting, int def);
BOOL   CopyImageFile(wchar_t *old_file, wchar_t *new_file);
void   ConvertToFilename(wchar_t *str, size_t size);

void   CreateOldStyleShortcut(MCONTACT hContact, wchar_t *history_filename);
BOOL   CreateShortcut(wchar_t *file, wchar_t *shortcut);

#define POPUP_ACTION_DONOTHING 0
#define POPUP_ACTION_CLOSEPOPUP 1
#define POPUP_ACTION_OPENAVATARHISTORY 2
#define POPUP_ACTION_OPENHISTORY 3

#define POPUP_DELAY_DEFAULT 0
#define POPUP_DELAY_CUSTOM 1
#define POPUP_DELAY_PERMANENT 2
