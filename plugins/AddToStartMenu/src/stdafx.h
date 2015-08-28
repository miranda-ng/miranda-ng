#pragma once

#include <Windows.h>
#include <sal.h>
#include <Psapi.h>

#include <ObjBase.h>
#include <ShObjIdl.h>
#include <propvarutil.h>
#include <functiondiscoverykeys.h>
#include <intsafe.h>
#include <guiddef.h>

#include <wrl\client.h>
#include <wrl\implements.h>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_langpack.h>
#include <m_icolib.h>
#include <m_extraicons.h>
#include <m_message.h>
#include <m_timezones.h>
#include <m_genmenu.h>
#include <m_clist.h>
#include "Version.h"
#include <strsafe.h>

const wchar_t AppUserModelID[] = _T("MirandaNG");

#define MODULENAME "AddToStartMenu"

extern HINSTANCE g_hInst;


INT_PTR Service(WPARAM, LPARAM);
HRESULT TryCreateShortcut();
HRESULT InstallShortcut(_In_z_ wchar_t *shortcutPath);
