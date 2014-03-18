#ifndef _HEADERS_H
#define _HEADERS_H

#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <shlobj.h>
#include <time.h>

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_icolib.h>
#include <win2k.h>

#include <m_folders.h>

#include "options.h"
#include "resource.h"
#include "version.h"

#define MS_AB_BACKUP		"AB/Backup"
#define MS_AB_SAVEAS		"AB/SaveAs"

#define SUB_DIR L"\\AutoBackups"
#define DIR L"%miranda_userdata%"


void ShowPopup(TCHAR* ptszText, TCHAR* ptszHeader, TCHAR* ptszPath);
INT_PTR DBSaveAs(WPARAM wParam, LPARAM lParam);
INT_PTR ABService(WPARAM wParam, LPARAM lParam);
int Backup(TCHAR* backup_filename);
int SetBackupTimer(void);
int OptionsInit(WPARAM wParam, LPARAM lParam);
int LoadOptions(void);
HWND CreateToolTip(HWND hwndParent, LPTSTR ptszText, LPTSTR ptszTitle);
void BackupThread(void* backup_filename);
bool MakeZip(LPCTSTR szSource, LPCTSTR szDest);

extern HINSTANCE hInst;
extern TCHAR* profilePath;

#endif
