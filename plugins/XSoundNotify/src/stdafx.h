#include <windows.h>
#include <Shlwapi.h>

#include <newpluginapi.h>
#include <m_system.h>
#include <m_database.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_langpack.h>
#include <m_clist.h>
#include <m_skin.h>
#include <win2k.h>
#include <m_options.h>
#include <m_chat_int.h>

#include "resource.h"
#include "version.h"

#define SETTINGSNAME "XSoundNotify"
#define SETTINGSKEY "XSNPlugin_sound"
#define SETTINGSIGNOREKEY "XSNPlugin_ignore"

struct XSN_Data
{
	LPARAM hContact;
	wchar_t path[MAX_PATH];
	BYTE ignore;
	BYTE iscontact;

	__forceinline XSN_Data(LPARAM _aContact, wchar_t *_path, BYTE _ignore, BYTE _iscontact) :
		hContact(_aContact)
	{
		wcsncpy(path, _path, _countof(path));
		ignore = _ignore;
		iscontact = _iscontact;
	}
};

extern LIST<XSN_Data> XSN_Users;

extern HINSTANCE hInst;
extern MWindowList hChangeSoundDlgList;
extern BYTE isIgnoreSound, isOwnSound;

bool IsSuitableProto(PROTOACCOUNT *pa);

INT OptInit(WPARAM wParam, LPARAM lParam);

INT_PTR ShowDialog(WPARAM wParam, LPARAM lParam);
