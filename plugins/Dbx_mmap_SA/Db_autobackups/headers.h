#ifndef _HEADERS_H
#define _HEADERS_H

#define _CRT_SECURE_NO_DEPRECATE
#define MIRANDA_VER 0x0900
#define MIRANDA_CUSTOM_LP

#include <m_stdhdr.h>
#include <windows.h>
#include <newpluginapi.h>
#include <win2k.h>
#include <time.h>
#include <commctrl.h>
#include <m_clist.h>
#include <m_plugins.h>
#include <m_system.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_utils.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_icolib.h>
#include "m_folders.h"

#include "options.h"
#include "resource.h"

#define MS_AB_BACKUP		"AB/Backup"
#define MS_AB_BACKUPTRGR	"AB/Backuptrg"
#define MS_AB_SAVEAS		"AB/SaveAs"

#ifdef _UNICODE
	#define SUB_DIR L"\\AutoBackups"
	#define DIR L"%miranda_userdata%"
#else
	#define SUB_DIR "\\AutoBackups"
	#define DIR "%miranda_userdata%"
#endif

void ShowPopup(TCHAR* text, TCHAR* header);
INT_PTR DBSaveAs(WPARAM wParam, LPARAM lParam);
INT_PTR ABService(WPARAM wParam, LPARAM lParam);
int CreateDirectoryTree(TCHAR *szDir);
int Backup(TCHAR* backup_filename);
int SetBackupTimer(void);
int OptionsInit(WPARAM wParam, LPARAM lParam);
int LoadOptions(void);
HWND CreateToolTip(HWND hwndParent, LPTSTR ptszText, LPTSTR ptszTitle);

extern HINSTANCE hInst;
TCHAR* profilePath;

#endif
