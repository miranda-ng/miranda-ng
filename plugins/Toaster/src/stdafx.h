#ifndef _COMMON_H_
#define _COMMON_H_

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <msapi\wrl\implements.h>
#include <msapi\windows.ui.notifications.h>

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
#endif //_COMMON_H_
