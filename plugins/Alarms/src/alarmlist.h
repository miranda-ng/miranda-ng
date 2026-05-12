#ifndef _ALARMLIST_INC
#define _ALARMLIST_INC

#include "options.h"
#include "time_utils.h"
#include "alarm_win.h"

#include "m_alarms.h"

struct ALARM
{
	ALARM()
	{
		memset(this, 0, sizeof(*this));
	}

	ALARM(const ALARM &p)
	{
		szTitle = mir_wstrdup(p.szTitle);
		szDesc = mir_wstrdup(p.szDesc);
		szCommand = mir_wstrdup(p.szCommand);
		szCommandParams = mir_wstrdup(p.szCommandParams);
		memcpy(&id, &p.id, FIELD_OFFSET(ALARM, szTitle));
	}

	unsigned short id;
	Occurrence occurrence;
	BOOL snoozer = 0;
	SYSTEMTIME time;
	unsigned short action;
	uint8_t sound_num;
	int flags, day_mask;

	ptrW szTitle, szDesc;
	ptrW szCommand;
	ptrW szCommandParams;
};

int CompareAlarms(const ALARM *a1, const ALARM *a2);

int MinutesInFuture(SYSTEMTIME time, Occurrence occ, int selected_days = 0);
void TimeForMinutesInFuture(int mins, SYSTEMTIME *time);

typedef OBJLIST<ALARM> AlarmList;

//extern AlarmList alarms;

void LoadAlarms();
void SaveAlarms();

void InitList();
void DeinitList();

void copy_list(AlarmList &copy);
void copy_list(AlarmList &copy, SYSTEMTIME &start, SYSTEMTIME &end);

void set_list(AlarmList &copy);

void append_to_list(ALARM *alarm);
void alter_alarm_list(ALARM *alarm);
void remove(unsigned short alarm_id);

void suspend(unsigned short alarm_id);

void GetPluginTime(SYSTEMTIME *t);

// increase 'time' to next occurrence
bool UpdateAlarm(SYSTEMTIME &time, Occurrence occ, int selected_days = 0);

const ULARGE_INTEGER mult = { 600000000, 0}; // number of 100 microsecond blocks in a minute

extern unsigned short next_alarm_id;

#endif
