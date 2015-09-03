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

#include "version.h"
#include "resource.h"

typedef void(__cdecl *pEventHandler)(void*);
const wchar_t AppUserModelID[] = _T("MirandaNG");
DEFINE_PROPERTYKEY(PKEY_AppUserModel_ID, 0x9F4C2855, 0x9F79, 0x4B39, 0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3, 5);

#define MODULE "Toaster"

#define CHECKHR(x) if (FAILED(x)) return x;

#include "string_reference_wrapper.h"
#include "toast_event_handler.h"
#include "toast_notification.h"
#include "add_to_start_menu.h"
#include "images.h"

extern HINSTANCE g_hInstance;
extern mir_cs csNotifications;
extern OBJLIST<ToastNotification> lstNotifications;
extern wchar_t wszTempDir[MAX_PATH];

struct callbackArg
{
	MCONTACT hContact;
	ToastNotification* notification;
};

struct ToastData
{
	MCONTACT hContact;
	TCHAR *tszTitle;
	TCHAR *tszText;
	HICON hIcon;
	bool bForcehIcon;

	ToastData(MCONTACT _hContact, const TCHAR *_tszTitle, const TCHAR *_tszText, HICON _hIcon = NULL, bool b = false) : hContact(_hContact), tszTitle(mir_tstrdup(_tszTitle)), tszText(mir_tstrdup(_tszText)), hIcon(_hIcon), bForcehIcon(b) {}
	~ToastData()
	{
		mir_free(tszTitle);
		mir_free(tszText);
	}
};

struct ClassData
{
	int iFlags;
	HICON hIcon;
	HANDLE handle;
	ClassData(int f, HICON h = NULL) : iFlags(f), hIcon(h) 
	{
		Utils_GetRandom(&handle, sizeof(handle));
	}
};

void CleanupClasses();
void InitServices();
int OnPreShutdown(WPARAM, LPARAM);
void __stdcall HideAllToasts(void*);
#endif //_COMMON_H_
