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

#include "version.h"
#include "resource.h"

#include "commonstatus.h"
#include "keepstatus.h"
#include "startupstatus.h"
#include "advancedautoaway.h"

int KSCSModuleLoaded(WPARAM, LPARAM);
int SSCSModuleLoaded(WPARAM, LPARAM);
int AAACSModuleLoaded(WPARAM, LPARAM);

#endif //_COMMON_H_
