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

typedef struct ALARM_tag {
	unsigned short id;
	wchar_t *szTitle;
	wchar_t *szDesc;
	Occurrence occurrence;
	BOOL snoozer;
	SYSTEMTIME time;
	unsigned short action;
	wchar_t *szCommand;
	wchar_t *szCommandParams;
	uint8_t sound_num;
	int flags, day_mask;
} ALARM; 

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
