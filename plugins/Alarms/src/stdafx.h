#ifndef _COMMON_INC
#define _COMMON_INC

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define VC_EXTRALEAN

#include <windows.h>
#include <shellapi.h>
#include <list>

#include <newpluginapi.h>
#include <m_utils.h>
#include <m_options.h>
#include <m_skin.h>
#include <m_protosvc.h>
#include <m_langpack.h>
#include <m_popup.h>
#include <m_clistint.h>
#include <m_idle.h>
#include <m_cluiframes.h>
#include <m_fontservice.h>
#include <m_icolib.h>

#include <m_toptoolbar.h>
#include <m_alarms.h>

#include "resource.h"
#include "version.h"

#include "alarmlist.h"
#include "options.h"
#include "icons.h"
#include "frame.h"
#include "alarm_win.h"
#include "time_utils.h"

#define MODULENAME	"Alarm"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
};

extern HANDLE hTopToolbarButton;

#endif
