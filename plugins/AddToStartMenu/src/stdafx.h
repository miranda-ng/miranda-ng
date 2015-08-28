#pragma once

#include <ShObjIdl.h>
#include <propvarutil.h>
#include <wrl\implements.h>
#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_genmenu.h>
#include <m_clist.h>
#include "Version.h"

DEFINE_PROPERTYKEY(PKEY_AppUserModel_ID, 0x9F4C2855, 0x9F79, 0x4B39, 0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3, 5);

const wchar_t AppUserModelID[] = _T("MirandaNG");

#define MODULENAME "AddToStartMenu"

extern HINSTANCE g_hInst;


INT_PTR Service(WPARAM, LPARAM);
HRESULT ShortcutExists();
wchar_t* GetShortcutPath();
HRESULT TryCreateShortcut();
HRESULT InstallShortcut(_In_z_ wchar_t *shortcutPath);
