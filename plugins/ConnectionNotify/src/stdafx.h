#include <windows.h>
#include <Commctrl.h>
#include <assert.h>
#include <iphlpapi.h>
#include <Tlhelp32.h>

#include <newpluginapi.h>
#include <m_core.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_utils.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_system.h>

#ifdef _DEBUG
#include "debug.h"
#endif
#include "resource.h"
#include "netstat.h"
#include "filter.h"
#include "version.h"
#include "pid2name.h"

#define MAX_SETTING_STR 512
#define PLUGINNAME "ConnectionNotify"
#define MAX_LENGTH 512
#define STATUS_COUNT 9

#define PLUGINNAME_NEWSOUND PLUGINNAME "_new_sound"

#if !defined(MIID_CONNECTIONNOTIFY)
	#define MIID_CONNECTIONNOTIFY  {0x4bb5b4aa, 0xc364, 0x4f23, { 0x97, 0x46, 0xd5, 0xb7, 0x8, 0xa2, 0x86, 0xa5 } }
#endif
// 4BB5B4AA-C364-4F23-9746-D5B708A286A5

// Note: could also use malloc() and free()
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

void showMsg(TCHAR *pName,DWORD pid,TCHAR *intIp,TCHAR *extIp,int intPort,int extPort,int state);
//int __declspec(dllexport) Load(PLUGINLINK *link);
extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
static unsigned __stdcall checkthread(void *dummy);
struct CONNECTION * LoadSettingsConnections();
void saveSettingsConnections(struct CONNECTION *connHead);
