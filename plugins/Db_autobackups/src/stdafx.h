#ifndef _HEADERS_H
#define _HEADERS_H

#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <shlobj.h>
#include <time.h>

#include <string>
#include <vector>
#include <functional>
#include <filesystem>

namespace fs = std::filesystem;

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_database.h>
#include <m_db_int.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_skin.h>
#include <m_icolib.h>
#include <m_autobackups.h>
#include <m_gui.h>
#include <m_timezones.h>
#include <m_variables.h>

#include <m_folders.h>
#include <m_cloudfile.h>

#define MODULENAME "AutoBackups"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	HANDLE hevBackup;
	wchar_t folder[MAX_PATH];

	CMOption<uint8_t>	    backup_types;
	CMOption<uint16_t>	    period;
	CMOption<uint8_t>	    period_type;
	CMOption<wchar_t*> file_mask;
	CMOption<uint16_t>	    num_backups;
	CMOption<uint8_t>	    disable_progress;
	CMOption<uint8_t>	    disable_popups;
	CMOption<uint8_t>	    use_zip;
	CMOption<uint8_t>	    backup_profile;
	CMOption<uint8_t>	    use_cloudfile;
	CMOption<char*>    cloudfile_service;

	int Load() override;
};

#include "options.h"
#include "resource.h"
#include "version.h"

#define SUB_DIR L"\\AutoBackups"
#define DIR L"%miranda_userdata%"

#define ME_AUTOBACKUP_DONE "Autobackup/Done"

int  SetBackupTimer(void);
int  OptionsInit(WPARAM wParam, LPARAM lParam);
void BackupStart(wchar_t *backup_filename);
int  BackupStatus(void);

struct ZipFile
{
	std::wstring sPath;
	std::wstring sZipPath;
	__forceinline ZipFile(const std::wstring &path, const std::wstring &zpath) : sPath(path), sZipPath(zpath) {}
};

int CreateZipFile(const wchar_t *szDestPath, OBJLIST<ZipFile> &lstFiles, const std::function<bool(size_t)> &fnCallback);

extern char g_szMirVer[];

#endif
