#ifndef _ALARMS_H
#define _ALARMS_H

// flags for alarm action
#define AAF_POPUP		0x01				// show a popup window (or a popup from popups plugin, if installed and activated via options)
#define AAF_SOUND		0x02				// play a sound
#define AAF_COMMAND		0x04				// run a command
#define AAF_SYSTRAY		0x08				// flash systray icon (not implemented)

typedef enum { 
	OC_ONCE,								// all fields in time (see below) are valid
	OC_DAILY,								// only wHour, wMinute, and wSecond are valid
	OC_WEEKLY,								// wHour, wMinute, wSecond, and wDayOfWeek are valid
	OC_WEEKDAYS,							// only wHour, wMinute, and wSecond are valid
	OC_MONTHLY,								// wHour, wMinute, wSecond, and wDay are valid
	OC_YEARLY								// all fields except wYear are valid
} Occurrence;

// flags
#define ALF_HIDDEN		0x01				// do not show in GUI (either options or reminder frame)
#define ALF_NOREMINDER	0x02				// do not show in reminder frame
#define ALF_SUSPENDED	0x04				// do not trigger next occurence
#define ALF_NOSTARTUP	0x08				// do not trigger on startup if it was due when miranda was not running
#define ALF_NOSNOOZE	0x10				// do not allow snoozing of this alarm

typedef struct {
	char *szTitle;
	char *szDesc;
	Occurrence occurrence;
	BOOL snoozer;							// this alarm is a 'once-off', the result of the user pressing the 'snooze' button - the time field no longer contains the original alarm time
	SYSTEMTIME time;						// the time the alarm is triggered at - different fields are valid depending on what the 'occurence' value is set to (see definition of Occurence type above)
	unsigned short action;					// bitwise OR of AAF_* constants above
	char *szCommand;						// passed to ShellExecute (if action & AAF_COMMAND) when the alarm is triggered
	char *szCommandParams;					// passed as parameters for above command
	BYTE sound_num;							// use alarm sound 1, 2, or 3 (if action & AAF_SOUND) (4 == speak, version 0.0.7.0+)
	int flags;								// ALF_* above
} ALARMINFO;

// set an alarm
// wparam = 0
// lparam = (ALARMINFO *)&alarm
#define MS_ALARMS_ADDALARM		"Alarms/AddAlarm"

// event sent when an alarm is triggered
// wparam=0
// lparam=(ALARMINFO *)&alarm
// returning non-zero from your hook will prevent the alarm actions from being carried out
#define ME_ALARMS_TRIGGERED		"Alarms/Triggered"

#endif
