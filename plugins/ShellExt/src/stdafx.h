#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <CommCtrl.h>
#include <ShlObj.h>
#include <Wincodec.h>

#include <string.h>
#include <stddef.h>

#include <newpluginapi.h>

#include <m_system_cpp.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_file.h>
#include <m_ignore.h>
#include <m_skin.h>
#include <m_clist.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_database.h>
#include <win2k.h>

#include "Version.h"

#define SHLExt_Name            "shlext15"
#define SHLExt_MRU             "MRU"
#define SHLExt_UseGroups       "UseGroups"
#define SHLExt_UseCListSetting "UseCLGroups"
#define SHLExt_UseHITContacts  "UseHITContacts"
#define SHLExt_UseHIT2Contacts "UseHIT2Contacts"
#define SHLExt_ShowNoProfile   "ShowNoProfile"
#define SHLExt_ShowNoIcons     "ShowNoIcons"
#define SHLExt_ShowNoOffline   "ShowNoOffline"

#define COMREG_UNKNOWN  0
#define COMREG_OK       1
#define COMREG_APPROVED 2

void    CheckRegisterServer();
void    CheckUnregisterServer();
void    InvokeThreadServer();
int     IsCOMRegistered();
HRESULT RemoveCOMRegistryEntries();

extern  HINSTANCE hInst;
extern  HANDLE hLogger;
extern  bool bIsVistaPlus;
extern  int DllFactoryCount, DllObjectCount;

int OnOptionsInit(WPARAM wParam, LPARAM lParam);

#ifdef _DEBUG
#define LOG_ENABLED
#endif

#ifdef LOG_ENABLED
void logA(const char *format, ...);
#else
#define logA(A, ...) 
#endif

UINT murmur_hash(const char *str);
