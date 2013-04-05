#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <Shlwapi.h>

#include <newpluginapi.h>
#include <m_system_cpp.h>
#include <m_database.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_langpack.h>
#include <m_clist.h>
#include <m_skin.h>
#include <win2k.h>
#include <m_options.h>

#include "resource.h"
#include "Version.h"

#define SETTINGSNAME "XSoundNotify"
#define SETTINGSKEY "XSNPlugin_sound"

struct XSN_Data
{
	HANDLE hContact;
	TCHAR path[MAX_PATH];

	__forceinline XSN_Data(HANDLE _aContact, TCHAR *_path) :
		hContact(_aContact)
	{
		_tcsncpy(path, _path, SIZEOF(path));
	}
};