#pragma once

#include <windows.h>
#include <Shlwapi.h>

#include <newpluginapi.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_langpack.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_options.h>
#include <m_chat_int.h>

#include "resource.h"
#include "version.h"

#define MODULENAME "XSoundNotify"
#define SETTINGSKEY "XSNPlugin_sound"
#define SETTINGSIGNOREKEY "XSNPlugin_ignore"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

struct XSN_Data
{
	LPARAM hContact;
	wchar_t path[MAX_PATH];
	bool bIsContact, bIgnore;

	XSN_Data(MCONTACT _aContact, bool _ignore);
	XSN_Data(const char *szModuleName, bool _ignore);
};

extern LIST<XSN_Data> XSN_Users;

extern MWindowList hChangeSoundDlgList;
extern uint8_t isIgnoreSound, isOwnSound;

bool IsSuitableProto(PROTOACCOUNT *pa);

INT OptInit(WPARAM wParam, LPARAM lParam);

INT_PTR ShowDialog(WPARAM wParam, LPARAM lParam);
