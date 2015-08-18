#ifndef _COMMON_H_
#define _COMMON_H_

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <Windows.h>
#include <SDKDDKVer.h>
#include <strsafe.h>
#include <intsafe.h>

#include <roapi.h>
#include <wrl\client.h>
#include <wrl\implements.h>
#include <windows.ui.notifications.h>

#include <newpluginapi.h>
#include <m_core.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_avatars.h>
#include <m_protocols.h>
#include <m_popup.h>
#include <m_message.h>

#include "version.h"
#include "resource.h"

typedef void(__cdecl *pEventHandler)(void*);
const wchar_t AppUserModelID[] = L"MirandaNG";

#include "string_reference_wrapper.h"
#include "toast_event_handler.h"
#include "toast_notification.h"

#define MODULE "Toaster"

extern HINSTANCE g_hInstance;

void InitServices();

#endif //_COMMON_H_
