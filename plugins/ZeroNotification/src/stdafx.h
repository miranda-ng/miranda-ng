#pragma once

#include <windows.h>
#include <commctrl.h>

#include <newpluginapi.h>
#include <m_options.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_clist.h>
#include <m_protosvc.h>

#include "version.h"
#include "resource.h"

#define MODULENAME "ZeroNotify"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
};

#define DEFAULT_NOSOUND		0x00000000
#define DEFAULT_NOBLINK 	0x00000000
#define DEFAULT_NOCLCBLINK 0x00000000

#define DISABLE_SOUND LPGENW("Disable &Sounds")
#define ENABLE_SOUND LPGENW("Enable &Sounds")

int SetNotify(const long status);
