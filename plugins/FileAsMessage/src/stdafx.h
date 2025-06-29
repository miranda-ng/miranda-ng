#pragma once

#define _CRT_NONSTDC_NO_DEPRECATE

#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <commctrl.h>

#include <newpluginapi.h>
#include <m_button.h>
#include <m_clistint.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_file.h>
#include <m_icolib.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protosvc.h>
#include <m_skin.h>

#include "dialog.h"
#include "resource.h"
#include "version.h"

#define MAXBUFSIZE 4096
#define SERVICE_TITLE LPGEN("File As Message")
#define MODULENAME "FileAsMessage"

#define SERVICE_PREFIX "<%fAM-0023%>"

#define NOPLUGIN_MESSAGE "If you see this \"garbage\", probably you have no \"fileAsMessage\" plugin installed, see https://miranda-ng.org/p/FileAsMessage for more information and download."

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

extern char *szServiceTitle;
extern char *szServicePrefix;
extern const ulong INITCRC;

#define WM_FE_MESSAGE		WM_USER+100
#define WM_FE_STATUSCHANGE	WM_USER+101
#define WM_FE_SKINCHANGE	WM_USER+102

extern MWindowList hFileList;
extern HANDLE hEventNewFile;

ulong memcrc32(uchar *ptr, int size, ulong crc);
int OnOptInitialise(WPARAM wParam, LPARAM);
