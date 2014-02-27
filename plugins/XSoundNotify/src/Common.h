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
#define SETTINGSIGNOREKEY "XSNPlugin_ignore"

struct XSN_Data
{
	MCONTACT hContact;
	TCHAR path[MAX_PATH];
	BYTE ignore;

	__forceinline XSN_Data(MCONTACT _aContact, TCHAR *_path, BYTE _ignore) :
		hContact(_aContact)
	{
		_tcsncpy(path, _path, SIZEOF(path));
		ignore = _ignore;
	}
};

extern LIST<XSN_Data> XSN_Users;

extern HINSTANCE hInst;
extern HANDLE hChangeSoundDlgList;
extern BYTE isIgnoreSound, isOwnSound;

bool IsSuitableProto(PROTOACCOUNT *pa);

INT OptInit(WPARAM wParam, LPARAM lParam);

INT_PTR ShowDialog(WPARAM wParam, LPARAM lParam);
