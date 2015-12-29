#ifndef _COMMON_H_
#define _COMMON_H_

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <wrl\implements.h>
#include <windows.ui.notifications.h>
#include <ShObjIdl.h>
#include <propvarutil.h>
#include <memory>
#include <map>
#include <newpluginapi.h>
#include <m_system_cpp.h>
#include <m_langpack.h>
#include <m_avatars.h>
#include <m_protocols.h>
#include <m_popup.h>
#include <m_message.h>
#include <m_chat.h>
#include <m_skin.h>
#include <m_imgsrvc.h>
#include <m_netlib.h>
#include <m_xml.h>
#include <m_options.h>
#include <m_gui.h>

#include "version.h"
#include "resource.h"

const wchar_t AppUserModelID[] = _T("MirandaNG");

#define MODULE "Toaster"
#define CHECKHR(x) if (FAILED(x)) return x;

#include "string_reference_wrapper.h"
#include "toast_notification.h"
#include "add_to_start_menu.h"
#include "options.h"
#include "structs.h"
#include "images.h"

extern HINSTANCE g_hInstance;
extern mir_cs csNotifications;
extern OBJLIST<ToastNotification> lstNotifications;
extern wchar_t wszTempDir[MAX_PATH];

void CleanupClasses();
void InitServices();
int OnPreShutdown(WPARAM, LPARAM);
void __stdcall HideAllToasts(void*);

int OnOptionsInitialized(WPARAM wParam, LPARAM);

#endif //_COMMON_H_
