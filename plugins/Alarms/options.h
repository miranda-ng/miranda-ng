#ifndef _OPTIONS_INC
#define _OPTIONS_INC

#include <commctrl.h>
#include "m_alarms.h"
#include "alarmlist.h"
#include "icons.h"
#include "frame.h"
#include "alarm_win.h"

typedef struct Options_tag {
	bool use_popup_module;
	int snooze_minutes;
	int row_height;
	int indent;
	int aw_trans;
	bool aw_roundcorners;
	bool aw_dontstealfocus;
	bool auto_showhide;
	bool hide_with_clist;
	bool loop_sound;
	bool auto_size_vert;
	int reminder_period;
} Options;

extern Options options;

int OptInit(WPARAM wParam,LPARAM lParam);

void LoadOptions();
void SaveOptions();

int NewAlarmMenuFunc(WPARAM wParam, LPARAM lParam);
void EditNonModal(ALARM &alarm);

// provide access the options window, for refresh (== 0 when not displayed)
extern HWND hwndOptionsDialog;
#define		WMU_INITOPTLIST		(WM_USER + 20)


#endif
