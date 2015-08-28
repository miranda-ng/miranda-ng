#pragma once

#include <Windows.h>
#include <Psapi.h>

#include <ShObjIdl.h>
#include <propvarutil.h>
#include <functiondiscoverykeys.h>
#include <wrl\client.h>
#include <wrl\implements.h>
#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_genmenu.h>
#include <m_clist.h>
#include "Version.h"

const wchar_t AppUserModelID[] = _T("MirandaNG");

#define MODULENAME "AddToStartMenu"

extern HINSTANCE g_hInst;


INT_PTR Service(WPARAM, LPARAM);
HRESULT ShortcutExists();
wchar_t* GetShortcutPath();
HRESULT TryCreateShortcut();
HRESULT InstallShortcut(_In_z_ wchar_t *shortcutPath);
