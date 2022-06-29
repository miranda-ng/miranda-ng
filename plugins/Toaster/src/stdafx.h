#ifndef _COMMON_H_
#define _COMMON_H_

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <wrl/implements.h>
#include <windows.ui.notifications.h>
#include <ShlObj.h>
#include <ShObjIdl.h>
#include <propvarutil.h>
#include <memory>
#include <map>
#include <string>
#include <sstream>

#include <newpluginapi.h>
#include <m_system.h>
#include <m_langpack.h>
#include <m_avatars.h>
#include <m_protocols.h>
#include <m_popup_int.h>
#include <m_message.h>
#include <m_skin.h>
#include <m_imgsrvc.h>
#include <m_xml.h>
#include <m_options.h>
#include <m_gui.h>

#include "version.h"
#include "resource.h"

const wchar_t AppUserModelID[] = L"MirandaNG";

#define MODULENAME "Toaster"
#define CHECKHR(x) if (FAILED(x)) return x;

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
};

extern wchar_t wszTempDir[MAX_PATH];

#include "string_reference_wrapper.h"
#include "utils.h"
#include "options.h"
#include "structs.h"
#include "toast_notification.h"

void CleanupClasses();
void InitServices();
void HideAllToasts();

int OnOptionsInitialized(WPARAM wParam, LPARAM);

#endif //_COMMON_H_
