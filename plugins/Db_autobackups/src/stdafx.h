#ifndef _HEADERS_H
#define _HEADERS_H

#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <shlobj.h>
#include <time.h>
#include <vector>

#if defined(_MSC_VER) && (_MSC_VER >= 1800)
#	include <filesystem>
	namespace fs = std::tr2::sys;
#else
#	include <boost/filesystem.hpp>
	namespace fs = boost::filesystem;
#endif

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_icolib.h>
#include <m_autobackups.h>

#include <m_folders.h>
#include <m_dropbox.h>

#include "options.h"
#include "resource.h"
#include "version.h"

#define SUB_DIR L"\\AutoBackups"
#define DIR L"%miranda_userdata%"

int	SetBackupTimer(void);
int	OptionsInit(WPARAM wParam, LPARAM lParam);
int	LoadOptions(void);
void BackupStart(TCHAR *backup_filename);


extern HINSTANCE g_hInstance;
extern TCHAR *profilePath;


static IconItem iconList[] = {
	{ LPGEN("Backup profile"),     "backup", IDI_ICON1 },
	{ LPGEN("Save profile as..."), "saveas", IDI_ICON1 }
};

#endif
