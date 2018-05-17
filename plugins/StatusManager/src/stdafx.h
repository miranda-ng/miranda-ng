#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>
#include <process.h>
#include <winsock.h>
#include <wininet.h>
#include <ipexport.h>
#include <icmpapi.h>
#include <commctrl.h>

#include <newpluginapi.h>

#include <m_core.h>
#include <m_skin.h>
#include <m_clistint.h>
#include <m_utils.h>
#include <m_icolib.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_protocols.h>
#include <m_toptoolbar.h>
#include <m_statusplugins.h>
#include <m_gui.h>

#include "version.h"
#include "resource.h"

#define MODULENAME "StatusManager"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin() :
		PLUGIN<CMPlugin>(MODULENAME)
	{}
};

#include "commonstatus.h"
#include "KeepStatus\keepstatus.h"
#include "StartupStatus\startupstatus.h"
#include "AdvancedAutoAway\advancedautoaway.h"

extern bool g_bMirandaLoaded;
extern int AAALangPack, KSLangPack, SSLangPack;
extern CMOption<bool> g_AAAEnabled, g_KSEnabled, g_SSEnabled;

int OnCommonOptionsInit(WPARAM wParam, LPARAM);
void InitCommonOptions();

#endif //_COMMON_H_
