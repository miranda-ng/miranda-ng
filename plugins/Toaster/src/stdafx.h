#ifndef _COMMON_H_
#define _COMMON_H_

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <wrl\implements.h>
#include <windows.ui.notifications.h>
#include <ShObjIdl.h>
#include <propvarutil.h>

#include <newpluginapi.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_core.h>
#include <m_langpack.h>
#include <m_avatars.h>
#include <m_protocols.h>
#include <m_popup.h>
#include <m_message.h>
#include <m_chat.h>
#include <m_string.h>

#include "version.h"
#include "resource.h"

typedef void(__cdecl *pEventHandler)(void*);
const wchar_t AppUserModelID[] = _T("MirandaNG");
DEFINE_PROPERTYKEY(PKEY_AppUserModel_ID, 0x9F4C2855, 0x9F79, 0x4B39, 0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3, 5);

#include "string_reference_wrapper.h"
#include "toast_event_handler.h"
#include "toast_notification.h"

extern mir_cs csNotifications;
extern OBJLIST<ToastNotification> lstNotifications;

struct callbackArg
{
	MCONTACT hContact;
	ToastNotification* notification;
};

#define MODULE "Toaster"

extern HINSTANCE g_hInstance;

void InitServices();
int OnPreShutdown(WPARAM, LPARAM);

INT_PTR Service(WPARAM, LPARAM);
HRESULT ShortcutExists();
wchar_t* GetShortcutPath();
HRESULT TryCreateShortcut();
HRESULT InstallShortcut(_In_z_ wchar_t *shortcutPath);


#endif //_COMMON_H_
