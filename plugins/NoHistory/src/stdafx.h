#ifndef _COMMON_INC
#define _COMMON_INC

#include <windows.h>
#include <commctrl.h>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_clist.h>
#include <m_clc.h>
#include <m_contacts.h>
#include <m_gui.h>
#include <m_langpack.h>
#include <m_protocols.h>
#include <m_options.h>
#include <m_message.h>
#include <m_icolib.h>
#include <m_extraicons.h>

#include "resource.h"
#include "icons.h"
#include "options.h"
#include "version.h"

#define MODULENAME	"NoHistory"

#define DBSETTING_REMOVE "RemoveHistory"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	CMOption<bool> bEnabledForNew;
	CMOption<bool> bWipeOnStartup;

	int Load() override;
	int Unload() override;
};

void SrmmMenu_Load();

#endif
