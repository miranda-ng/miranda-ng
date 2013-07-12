#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE
#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <commctrl.h>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_protomod.h>
#include <m_langpack.h>
#include <m_clist.h>
#include <m_options.h>
#include <m_skin.h>
#include <m_popup.h>
#include <m_icolib.h>
#include <m_button.h>
#include <m_netlib.h>
#include <m_file.h>
#include <win2k.h>

#include "dialog.h"
#include "resource.h"
#include "version.h"

#define MAXBUFSIZE 4096
#define SERVICE_TITLE LPGEN("File As Message")
#define SERVICE_NAME "FileAsMessage"

#define SERVICE_PREFIX "<%fAM-0023%>"

#define NOPLUGIN_MESSAGE "If you see this \"garbage\", probably you have no \"fileAsMessage\" plugin installed, see http://miranda-ng.org/p/FileAsMessage/ for more information and download."

extern char *szServiceTitle;
extern char *szServicePrefix;
extern const ulong INITCRC;

#define WM_FE_MESSAGE		WM_USER+100
#define WM_FE_STATUSCHANGE	WM_USER+101
#define WM_FE_SKINCHANGE	WM_USER+102

extern HINSTANCE hInst;
extern HANDLE hFileList;
extern HANDLE hEventNewFile;

extern HICON hIcons[5];

ulong memcrc32(uchar *ptr, int size, ulong crc);
INT_PTR CALLBACK OptionsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
