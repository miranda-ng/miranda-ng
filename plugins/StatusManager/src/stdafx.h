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
#include <m_clist.h>
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

#include "commonstatus.h"
#include "KeepStatus\keepstatus.h"
#include "StartupStatus\startupstatus.h"
#include "AdvancedAutoAway\advancedautoaway.h"

extern HINSTANCE hInst;

bool IsSubPluginEnabled(const char *name);

class CSubPluginsOptionsDlg : CPluginDlgBase
{
private:
	CCtrlCheck m_enableKeepStatus;
	CCtrlCheck m_enableStartupStatus;
	CCtrlCheck m_enableAdvancedAutoAway;

protected:
	void OnInitDialog();
	void OnApply();

public:
	CSubPluginsOptionsDlg();

	static int OnOptionsInit(WPARAM wParam, LPARAM);
	static CDlgBase *CreateOptionsPage() { return new CSubPluginsOptionsDlg(); }
};

#endif //_COMMON_H_
