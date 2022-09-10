#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>
#include <process.h>
#include <winsock.h>
#include <wininet.h>
#include <ipexport.h>
#include <icmpapi.h>
#include <commctrl.h>
#include <shlobj.h>
#include <uxtheme.h>
#include <stdio.h>

#include <newpluginapi.h>

#include <m_awaymsg.h>
#include <m_database.h>
#include <m_clistint.h>
#include <m_gui.h>
#include <m_idle.h>
#include <m_icolib.h>
#include <m_langpack.h>
#include <m_netlib.h>
#include <m_NewAwaySys.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protosvc.h>
#include <m_protocols.h>
#include <m_toptoolbar.h>
#include <m_statusplugins.h>
#include <m_skin.h>
#include <m_utils.h>
#include <m_variables.h>

#include "version.h"
#include "resource.h"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	bool bMirandaLoaded = false;

	int Load() override;
	int Unload() override;
};

struct CFakePlugin : public CMPluginBase
{
	CFakePlugin(const char *szModuleName);
};

#include "commonstatus.h"
#include "keepstatus.h"
#include "startupstatus.h"
#include "advancedautoaway.h"

extern CFakePlugin AAAPlugin, KSPlugin, SSPlugin;
extern CMOption<bool> g_AAAEnabled, g_KSEnabled, g_SSEnabled;

int OnCommonOptionsInit(WPARAM wParam, LPARAM);
void InitCommonOptions();

char* StatusModeToDbSetting(int status, const char *suffix);
uint32_t StatusModeToProtoFlag(int status);

#endif //_COMMON_H_
