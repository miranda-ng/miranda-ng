#include "common.h"
#include "alarmlist.h"

AlarmList alarms;
CRITICAL_SECTION alarm_cs;

unsigned short next_alarm_id = 1; // 0 is used for invalid id

DWORD timer_id;
#define TIMER_PERIOD				5000 // milliseconds delay between alarm checks

static SYSTEMTIME last_check, last_saved_check;

HANDLE hAlarmTriggeredEvent, hAddAlarmService;

bool startup = true;

bool is_idle = false;

void free_alarm_data(ALARM *alarm) {
	if(alarm->szTitle) {free(alarm->szTitle); alarm->szTitle = 0;}
	if(alarm->szDesc) {free(alarm->szDesc); alarm->szDesc = 0;}
	if(alarm->szCommand) {free(alarm->szCommand); alarm->szCommand = 0;}
	if(alarm->szCommandParams) {free(alarm->szCommandParams); alarm->szCommandParams = 0;}
}

void copy_alarm_data(ALARM *dest, ALARM *src) {
	dest->action = src->action;
	dest->flags = src->flags;
	dest->id = src->id;
	dest->occurrence = src->occurrence;
	dest->snoozer = src->snoozer;
	dest->sound_num = src->sound_num;
	dest->time = src->time;
	dest->trigger_id = src->trigger_id;

	free_alarm_data(dest);
	dest->szTitle = _strdup(src->szTitle);
	dest->szDesc = _strdup(src->szDesc);
	dest->szCommand = _strdup(src->szCommand);
	dest->szCommandParams = _strdup(src->szCommandParams);
}

void GetPluginTime(SYSTEMTIME *t) {
	EnterCriticalSection(&alarm_cs);
	*t = last_check;
	LeaveCriticalSection(&alarm_cs);
}

int MinutesInFuture(SYSTEMTIME time, Occurrence occ) {
	if(!UpdateAlarm(time, occ)) return 0;

	SYSTEMTIME now;
	GetPluginTime(&now);

	FILETIME ft_now, ft_then;
	SystemTimeToFileTime(&now, &ft_now);
	SystemTimeToFileTime(&time, &ft_then);
	
	ULARGE_INTEGER uli_now, uli_then, diff;
	uli_now.HighPart = ft_now.dwHighDateTime;
	uli_now.LowPart = ft_now.dwLowDateTime;
	uli_then.HighPart = ft_then.dwHighDateTime;
	uli_then.LowPart = ft_then.dwLowDateTime;

	diff.QuadPart = uli_then.QuadPart - uli_now.QuadPart;
	bool inc = false;
	if(diff.QuadPart % mult.QuadPart >= mult.QuadPart / 2)
		inc = true;
	return (int)(diff.QuadPart / mult.QuadPart + (inc ? 1 : 0));
}

void TimeForMinutesInFuture(int mins, SYSTEMTIME *time) {
	SYSTEMTIME now;
	FILETIME ft_now;
	GetPluginTime(&now);
	SystemTimeToFileTime(&now, &ft_now);

	ULARGE_INTEGER uli_now;
	uli_now.HighPart = ft_now.dwHighDateTime;
	uli_now.LowPart = ft_now.dwLowDateTime;

	uli_now.QuadPart += mult.QuadPart * (ULONGLONG)mins;
	ft_now.dwHighDateTime = uli_now.HighPart;
	ft_now.dwLowDateTime = uli_now.LowPart;

	FileTimeToSystemTime(&ft_now, time);
}


// update an alarm so that the systemtime reflects the next time the alarm will go off, based on the last_check time
bool UpdateAlarm(SYSTEMTIME &time, Occurrence occ) {

	FILETIME ft_now, ft_then;
	ULARGE_INTEGER uli_then;

	switch(occ) {
	case OC_DAILY:
	case OC_WEEKDAYS:
	case OC_WEEKLY:
		time.wDay = last_check.wDay;
	case OC_MONTHLY:
		time.wMonth = last_check.wMonth;
	case OC_YEARLY:
		time.wYear = last_check.wYear;
	case OC_ONCE:
		break; // all fields valid
	};

	SystemTimeToFileTime(&last_check, &ft_now); // consider 'now' to be last check time
	SystemTimeToFileTime(&time, &ft_then);
	
	switch(occ) {
	case OC_ONCE:
		if(CompareFileTime(&ft_then, &ft_now) < 0)
			return false;
		break;
	case OC_YEARLY:
		while(CompareFileTime(&ft_then, &ft_now) < 0) {
			time.wYear++;
			SystemTimeToFileTime(&time, &ft_then);
		}
		break;
	case OC_MONTHLY:
		while(CompareFileTime(&ft_then, &ft_now) < 0) {
			if(time.wMonth == 12) {
				time.wMonth = 1;
				time.wYear++;
			} else
				time.wMonth++;
			SystemTimeToFileTime(&time, &ft_then);
		}
		break;
	case OC_WEEKLY:
		{
			SYSTEMTIME temp;
			uli_then.HighPart = ft_then.dwHighDateTime;
			uli_then.LowPart = ft_then.dwLowDateTime;
			FileTimeToSystemTime(&ft_then, &temp);
			do {
				if(temp.wDayOfWeek != time.wDayOfWeek || CompareFileTime(&ft_then, &ft_now) < 0) {
					uli_then.QuadPart += mult.QuadPart * (ULONGLONG)24 * (ULONGLONG)60;
					ft_then.dwHighDateTime = uli_then.HighPart;
					ft_then.dwLowDateTime = uli_then.LowPart;
					FileTimeToSystemTime(&ft_then, &temp);
				}
			} while(temp.wDayOfWeek != time.wDayOfWeek || CompareFileTime(&ft_then, &ft_now) < 0);
		}
		break;
	case OC_WEEKDAYS:
		{
			SYSTEMTIME temp;
			uli_then.HighPart = ft_then.dwHighDateTime;
			uli_then.LowPart = ft_then.dwLowDateTime;
			do {
				FileTimeToSystemTime(&ft_then, &temp);
				if(temp.wDayOfWeek == 0 || temp.wDayOfWeek == 6 || CompareFileTime(&ft_then, &ft_now) < 0) {
					uli_then.QuadPart += mult.QuadPart * (ULONGLONG)24 * (ULONGLONG)60;
					ft_then.dwHighDateTime = uli_then.HighPart;
					ft_then.dwLowDateTime = uli_then.LowPart;
				}
			} while(temp.wDayOfWeek == 0 || temp.wDayOfWeek == 6 || CompareFileTime(&ft_then, &ft_now) < 0);
		}
		break;
	case OC_DAILY:
		uli_then.HighPart = ft_then.dwHighDateTime;
		uli_then.LowPart = ft_then.dwLowDateTime;
		while(CompareFileTime(&ft_then, &ft_now) < 0) {
			uli_then.QuadPart += mult.QuadPart * (ULONGLONG)24 * (ULONGLONG)60;
			ft_then.dwHighDateTime = uli_then.HighPart;
			ft_then.dwLowDateTime = uli_then.LowPart;
		}
		break;
	}

	FileTimeToSystemTime(&ft_then, &time);
	return true;
}

void LoadAlarms() {
	int num_alarms = DBGetContactSettingWord(0, MODULE, "Count", 0);
	char buff[256];
	DBVARIANT dbv;
	ALARM alarm;
	SYSTEMTIME now;
	GetLocalTime(&now);

	EnterCriticalSection(&alarm_cs);
	alarms.clear();

	for(int i = 0; i < num_alarms; i++) {
		memset(&alarm, 0, sizeof(ALARM));

		sprintf(buff, "Title%d", i);
		if(!DBGetContactSetting(0, MODULE, buff, &dbv)) {
			if(dbv.pszVal && strlen(dbv.pszVal))
				alarm.szTitle = _strdup(dbv.pszVal);
			DBFreeVariant(&dbv);
		}
		sprintf(buff, "Desc%d", i);
		if(!DBGetContactSetting(0, MODULE, buff, &dbv)) {
			if(dbv.pszVal && strlen(dbv.pszVal))
				alarm.szDesc = _strdup(dbv.pszVal);
			DBFreeVariant(&dbv);
		}
		sprintf(buff, "Occ%d", i);
		alarm.occurrence = (Occurrence)DBGetContactSettingWord(0, MODULE, buff, 0);

		sprintf(buff, "STHour%d", i);
		alarm.time.wHour = DBGetContactSettingWord(0, MODULE, buff, 0);
		sprintf(buff, "STMinute%d", i);
		alarm.time.wMinute = DBGetContactSettingWord(0, MODULE, buff, 0);
		sprintf(buff, "STSecond%d", i);
		alarm.time.wSecond = DBGetContactSettingWord(0, MODULE, buff, 0);

		switch(alarm.occurrence) {

		case OC_ONCE:
			sprintf(buff, "STYear%d", i);
			alarm.time.wYear = DBGetContactSettingWord(0, MODULE, buff, 0);
			sprintf(buff, "STMonth%d", i);
			alarm.time.wMonth = DBGetContactSettingWord(0, MODULE, buff, 0);
			sprintf(buff, "STDay%d", i);
			alarm.time.wDay = DBGetContactSettingWord(0, MODULE, buff, 0);
			break;
		case OC_WEEKLY:
			sprintf(buff, "STDayOfWeek%d", i);
			alarm.time.wDayOfWeek = DBGetContactSettingWord(0, MODULE, buff, 0);
			break;
		case OC_WEEKDAYS:
			break;
		case OC_DAILY:
			break;
		case OC_MONTHLY:
			sprintf(buff, "STDay%d", i);
			alarm.time.wDay = DBGetContactSettingWord(0, MODULE, buff, 0);
			break;
		case OC_YEARLY:
			sprintf(buff, "STMonth%d", i);
			alarm.time.wMonth = DBGetContactSettingWord(0, MODULE, buff, 0);
			sprintf(buff, "STDay%d", i);
			alarm.time.wDay = DBGetContactSettingWord(0, MODULE, buff, 0);
			break;
		}

		sprintf(buff, "TriggerID%d", i);
		alarm.trigger_id = DBGetContactSettingDword(0, MODULE, buff, 0);

		if(UpdateAlarm(alarm.time, alarm.occurrence)) {
			sprintf(buff, "ActionFlags%d", i);
			alarm.action = (unsigned short)DBGetContactSettingDword(0, MODULE, buff, AAF_POPUP | AAF_SOUND);
			if(alarm.action & AAF_COMMAND) {
				sprintf(buff, "ActionCommand%d", i);
				if(!DBGetContactSetting(0, MODULE, buff, &dbv)) {
					if(dbv.pszVal && strlen(dbv.pszVal))
						alarm.szCommand = _strdup(dbv.pszVal);
					DBFreeVariant(&dbv);
					sprintf(buff, "ActionParams%d", i);
					if(!DBGetContactSetting(0, MODULE, buff, &dbv)) {
						if(dbv.pszVal && strlen(dbv.pszVal))
							alarm.szCommandParams = _strdup(dbv.pszVal);
						DBFreeVariant(&dbv);
					}
				}
			}

			sprintf(buff, "SoundNum%d", i);
			alarm.sound_num = (int)DBGetContactSettingByte(0, MODULE, buff, 1);

			sprintf(buff, "Snoozer%d", i);
			alarm.snoozer = DBGetContactSettingByte(0, MODULE, buff, 0) == 1;

			sprintf(buff, "Hidden%d", i);
			alarm.flags |= (DBGetContactSettingByte(0, MODULE, buff, 0) == 1 ? ALF_HIDDEN : 0);

			sprintf(buff, "Suspended%d", i);
			alarm.flags |= (DBGetContactSettingByte(0, MODULE, buff, 0) == 1 ? ALF_SUSPENDED : 0);

			sprintf(buff, "NoStartup%d", i);
			alarm.flags |= (DBGetContactSettingByte(0, MODULE, buff, 0) == 1 ? ALF_NOSTARTUP : 0);

			sprintf(buff, "Flags%d", i);
			alarm.flags = DBGetContactSettingDword(0, MODULE, buff, alarm.flags);

			alarm.id = next_alarm_id++;
			alarms.push_back(&alarm);

		} else { // else ignore it - it's an expired one-off alarm (but clean up triggers)
			if(alarm.trigger_id != 0 && ServiceExists(MS_TRIGGER_REPORTEVENT)) {
				REPORTINFO ri = {0};
				ri.cbSize = sizeof(ri);
				ri.triggerID = alarm.trigger_id;
				ri.flags = TRG_CLEANUP;
				CallService(MS_TRIGGER_REPORTEVENT, 0, (LPARAM)&ri);
			}
		}
		free_alarm_data(&alarm);
	}
	LeaveCriticalSection(&alarm_cs);
}

void SaveAlarms() {
	int index = 0;
	char buff[256];

	EnterCriticalSection(&alarm_cs);

	ALARM *i;
	for(alarms.reset(); i = alarms.current(); alarms.next(), index++) {
		sprintf(buff, "Title%d", index);
		DBWriteContactSettingString(0, MODULE, buff, i->szTitle);
		sprintf(buff, "Desc%d", index);
		DBWriteContactSettingString(0, MODULE, buff, i->szDesc);
		sprintf(buff, "Occ%d", index);
		DBWriteContactSettingWord(0, MODULE, buff, i->occurrence);

		sprintf(buff, "STHour%d", index);
		DBWriteContactSettingWord(0, MODULE, buff, i->time.wHour);
		sprintf(buff, "STMinute%d", index);
		DBWriteContactSettingWord(0, MODULE, buff, i->time.wMinute);
		sprintf(buff, "STSecond%d", index);
		DBWriteContactSettingWord(0, MODULE, buff, i->time.wSecond);

		switch(i->occurrence) {
		case OC_DAILY:
			break;
		case OC_WEEKDAYS:
			break;
		case OC_WEEKLY:
			sprintf(buff, "STDayOfWeek%d", index);
			DBWriteContactSettingWord(0, MODULE, buff, i->time.wDayOfWeek);
			break;

		case OC_ONCE:
			sprintf(buff, "STYear%d", index);
			DBWriteContactSettingWord(0, MODULE, buff, i->time.wYear);
		case OC_YEARLY:
			sprintf(buff, "STMonth%d", index);
			DBWriteContactSettingWord(0, MODULE, buff, i->time.wMonth);
		case OC_MONTHLY:
			sprintf(buff, "STDay%d", index);
			DBWriteContactSettingWord(0, MODULE, buff, i->time.wDay);
			break;
		}
		sprintf(buff, "ActionFlags%d", index);
		DBWriteContactSettingDword(0, MODULE, buff, i->action);
		if(i->action & AAF_COMMAND) {
			if(strlen(i->szCommand)) {
				sprintf(buff, "ActionCommand%d", index);
				DBWriteContactSettingString(0, MODULE, buff, i->szCommand);
				if(strlen(i->szCommandParams)) {
					sprintf(buff, "ActionParams%d", index);
					DBWriteContactSettingString(0, MODULE, buff, i->szCommandParams);
				}
			}
		}
		
		sprintf(buff, "SoundNum%d", index);
		DBWriteContactSettingByte(0, MODULE, buff, i->sound_num);

		sprintf(buff, "Snoozer%d", index);
		DBWriteContactSettingByte(0, MODULE, buff, i->snoozer ? 1 : 0);

		sprintf(buff, "Flags%d", index);
		DBWriteContactSettingDword(0, MODULE, buff, i->flags);

		sprintf(buff, "TriggerID%d", index);
		DBWriteContactSettingDword(0, MODULE, buff, i->trigger_id);
	}
	DBWriteContactSettingWord(0, MODULE, "Count", index);

	LeaveCriticalSection(&alarm_cs);
}

void copy_list(AlarmList &copy) {
	copy.clear();
	ALARM *i;
	EnterCriticalSection(&alarm_cs);
	for(alarms.reset(); i = alarms.current(); alarms.next()) {
		copy.push_back(i);
	}
	LeaveCriticalSection(&alarm_cs);
}

void copy_list(AlarmList &copy, SYSTEMTIME &start, SYSTEMTIME &end) {
	copy.clear();
	ALARM *i;
	EnterCriticalSection(&alarm_cs);
	for(alarms.reset(); i = alarms.current(); alarms.next()) {
		if(IsBetween(i->time, start, end))
			copy.push_back(i);
	}
	LeaveCriticalSection(&alarm_cs);
}

void set_list(AlarmList &copy) {
	EnterCriticalSection(&alarm_cs);
	alarms.clear();
	ALARM *i;
	for(copy.reset(); i = copy.current(); copy.next()) {
		alarms.push_back(i);
	}
	LeaveCriticalSection(&alarm_cs);

	SaveAlarms();
}

void append_to_list(ALARM *alarm) {
	EnterCriticalSection(&alarm_cs);
	if(!alarm->id)
		alarm->id = next_alarm_id++;
	alarms.push_back(alarm);
	LeaveCriticalSection(&alarm_cs);

	SaveAlarms();
}

void alter_alarm_list(ALARM *alarm) {
	bool found = false;
	EnterCriticalSection(&alarm_cs);
	if(alarm->id != 0) {
		ALARM *i;
		for(alarms.reset(); i = alarms.current(); alarms.next()) {
			if(i->id == alarm->id) {
				copy_alarm_data(i, alarm);
				found = true;
				break;
			}
		}
	}
	if(!found) {
		if(!alarm->id)
			alarm->id = next_alarm_id++;
		alarms.push_back(alarm);
	}

	LeaveCriticalSection(&alarm_cs);

	SaveAlarms();
}

void remove(unsigned short alarm_id) {
	EnterCriticalSection(&alarm_cs);
	ALARM *i;
	for(alarms.reset(); i = alarms.current(); alarms.next()) {
		if(i->id == alarm_id) {
			if(i->trigger_id != 0 && ServiceExists(MS_TRIGGER_REPORTEVENT)) {
				REPORTINFO ri = {0};
				ri.cbSize = sizeof(ri);
				ri.triggerID = i->trigger_id;
				ri.flags = TRG_CLEANUP;
				CallService(MS_TRIGGER_REPORTEVENT, 0, (LPARAM)&ri);
			}
			alarms.erase();
			break;
		}
	}
	LeaveCriticalSection(&alarm_cs);

	SaveAlarms();
}

void suspend(unsigned short alarm_id) {
	EnterCriticalSection(&alarm_cs);
	ALARM *i;
	for(alarms.reset(); i = alarms.current(); alarms.next()) {
		if(i->id == alarm_id && i->occurrence != OC_ONCE) {
			i->flags |= ALF_SUSPENDED;
			break;
		}
	}
	LeaveCriticalSection(&alarm_cs);

	SaveAlarms();
}

static int CALLBACK PopupAlarmDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
		case WM_COMMAND: // snooze
			if (HIWORD(wParam) == STN_CLICKED) { //It was a click on the Popup.
				ALARM *mpd = NULL;
				mpd = (ALARM *)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd,(LPARAM)mpd);

				if(mpd->flags & ALF_NOSNOOZE)
					return TRUE;

				// add snooze minutes to current time
				FILETIME ft;
				GetLocalTime(&mpd->time);
				SystemTimeToFileTime(&mpd->time, &ft);
				ULARGE_INTEGER uli;
				uli.LowPart = ft.dwLowDateTime;
				uli.HighPart = ft.dwHighDateTime;

				uli.QuadPart += mult.QuadPart * options.snooze_minutes;

				ft.dwHighDateTime = uli.HighPart;
				ft.dwLowDateTime = uli.LowPart;

				FileTimeToSystemTime(&ft, &mpd->time);

				mpd->occurrence = OC_ONCE;
				mpd->snoozer = true;
				mpd->flags = mpd->flags & ~(ALF_NOSTARTUP);

				mpd->id = next_alarm_id++;

				append_to_list(mpd);
			}

			PUDeletePopUp(hWnd);
			return TRUE;
		case WM_CONTEXTMENU: 
			PUDeletePopUp(hWnd);
			return TRUE;
		case UM_FREEPLUGINDATA: 
			{
				ALARM *mpd = NULL;
				mpd = (ALARM *)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd,(LPARAM)mpd);
				if (mpd > 0) {
					free_alarm_data(mpd);
					delete mpd;
				}
				return TRUE;
			}
		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void ShowPopup(ALARM *alarm) {
	if(ServiceExists(MS_POPUP_ADDPOPUP)) {
		ALARM *data = new ALARM;
		memset(data, 0, sizeof(ALARM));
		copy_alarm_data(data, alarm);

		POPUPDATAEX ppd;

		ZeroMemory(&ppd, sizeof(ppd)); 
		ppd.lchContact = 0; 
		ppd.lchIcon = hIconMenuSet;

		lstrcpy(ppd.lpzContactName, data->szTitle);
		lstrcpy(ppd.lpzText, data->szDesc);
		ppd.colorBack = 0;
		ppd.colorText = 0;
		ppd.PluginWindowProc = (WNDPROC)PopupAlarmDlgProc;
		ppd.PluginData = data;
		ppd.iSeconds = -1;

		//Now that every field has been filled, we want to see the popup.
		CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&ppd, 0);
	}
}

void DoAlarm(ALARM *alarm) {
	ALARMINFO alarminfo;
	alarminfo.szTitle = alarm->szTitle;
	alarminfo.szDesc = alarm->szDesc;
	alarminfo.szCommand = alarm->szCommand;
	alarminfo.szCommandParams = alarm->szCommandParams;
	alarminfo.occurrence = alarm->occurrence;
	alarminfo.snoozer = alarm->snoozer;
	alarminfo.time = alarm->time;
	alarminfo.flags = alarm->flags;
	alarminfo.action = alarm->action;
	alarminfo.sound_num = alarm->sound_num;

	if(!NotifyEventHooks(hAlarmTriggeredEvent, 0, (LPARAM)&alarminfo)) {

		if(alarm->action & AAF_SOUND) {
			if(alarm->sound_num > 0 && alarm->sound_num <= 3) {
				char buff[128];
				sprintf(buff, "Triggered%d", alarm->sound_num);
				SkinPlaySound(buff);
			} else if(alarm->sound_num == 4) {
				if (alarm->szTitle != NULL && alarm->szTitle[0] != '\0') {
					if (ServiceExists("Speak/Say")) {
						CallService("Speak/Say", 0, (LPARAM)alarm->szTitle);
					}
				}
			}
		}
		if(alarm->action & AAF_POPUP) {
			if(options.use_popup_module && ServiceExists(MS_POPUP_ADDPOPUP)) 
				ShowPopup(alarm);
			else {
				HWND hwndDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_ALARM), GetDesktopWindow(), DlgProcAlarm);
				WindowList_Add(hAlarmWindowList, hwndDlg, 0);

				ALARM *data = new ALARM;
				memset(data, 0, sizeof(ALARM));
				copy_alarm_data(data, alarm);
				SendMessage(hwndDlg, WMU_SETALARM, 0, (LPARAM)data);
				if(is_idle || !options.aw_dontstealfocus)
					ShowWindow(hwndDlg, SW_SHOW);
				else
					ShowWindow(hwndDlg, SW_SHOWNOACTIVATE);
				SetWindowPos(hwndDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			}
		}

		if(alarm->trigger_id != 0 && ServiceExists(MS_TRIGGER_REPORTEVENT)) {
			REPORTINFO ri = {0};
			ri.cbSize = sizeof(ri);
			ri.triggerID = alarm->trigger_id;
			ri.flags = TRG_PERFORM;
			if(alarm->occurrence == OC_ONCE)
				ri.flags |= TRG_CLEANUP;
			CallService(MS_TRIGGER_REPORTEVENT, 0, (LPARAM)&ri);
		}
		
		if(alarm->action & AAF_COMMAND) {
			ShellExecute(0, 0, alarm->szCommand, alarm->szCommandParams, 0, SW_NORMAL);
		}

		if(alarm->action & AAF_SYSTRAY)
		{
			CLISTEVENT cle = {0};
			cle.cbSize = sizeof(cle);
			cle.hContact = 0;
			cle.hIcon = hIconSystray;
			cle.pszTooltip = alarm->szTitle;
			cle.flags = CLEF_ONLYAFEW;
			CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
		}
	}
}

void WriteLastCheckTime() {
	// save last-check time
	DBCONTACTWRITESETTING dbcws;
	dbcws.szModule = MODULE;
	dbcws.szSetting = "LastCheck";
	dbcws.value.type = DBVT_BLOB;
	dbcws.value.cpbVal = sizeof(SYSTEMTIME);
	dbcws.value.pbVal = (BYTE *)&last_check;
	CallService(MS_DB_CONTACT_WRITESETTING, 0, (LPARAM)&dbcws);

	last_saved_check = last_check;
}

void CheckAlarms() {
	SYSTEMTIME time;
	GetLocalTime(&time);

	// put triggered alarms in another list - so we don't keep the critical section locked for longer than necessary
	AlarmList triggered_list, remove_list;

	EnterCriticalSection(&alarm_cs);
	ALARM *i;
	for(alarms.reset(); i = alarms.current(); alarms.next()) {
		if(!UpdateAlarm(i->time, i->occurrence)) { 
			// somehow an expired one-off alarm is in our list
			remove_list.push_back(i);
			continue;
		}

		switch(i->occurrence) {
		case OC_ONCE:
			if(IsBetween(i->time, last_check, time)) {
				if(!startup || !(i->flags & ALF_NOSTARTUP)) triggered_list.push_back(i);
				// erase and fix iterator - alarm has now been triggered and has therefore expired
				remove_list.push_back(i);
			}
			break;
		default:
			if(IsBetween(i->time, last_check, time)) {
				if(i->flags & ALF_SUSPENDED)
					i->flags = i->flags & ~ALF_SUSPENDED;
				else
					if(!startup || !(i->flags & ALF_NOSTARTUP)) triggered_list.push_back(i);
			}
			break;
		}
	}
	
	last_check = time;
	WriteLastCheckTime();

	startup = false;
	LeaveCriticalSection(&alarm_cs);

	for(triggered_list.reset(); i = triggered_list.current(); triggered_list.next())
		DoAlarm(i);
	for(remove_list.reset(); i = remove_list.current(); remove_list.next())
		remove(i->id);


}

VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	CheckAlarms();
}


int AddAlarmService(WPARAM wParam, LPARAM lParam) {
	ALARMINFO *alarm_info = (ALARMINFO *)lParam;
	ALARM alarm = {0};
	alarm.action = alarm_info->action;
	alarm.flags = alarm_info->flags;
	alarm.id = next_alarm_id++;
	alarm.occurrence = alarm_info->occurrence;
	alarm.snoozer = alarm_info->snoozer;
	alarm.sound_num = alarm_info->sound_num;
	alarm.szCommand = _strdup(alarm_info->szCommand);
	alarm.szCommandParams = _strdup(alarm_info->szCommandParams);
	alarm.szDesc = _strdup(alarm_info->szDesc);
	alarm.szTitle = _strdup(alarm_info->szTitle);
	alarm.time = alarm_info->time;

	append_to_list(&alarm);
	return 0;
}

int IdleChanged(WPARAM wParam, LPARAM lParam) {
	is_idle = (lParam & IDF_ISIDLE);

	return 0;
}

void InitList() {
	InitializeCriticalSection(&alarm_cs);

	SkinAddNewSoundEx("Triggered1", Translate("Alarms"), Translate("Alert 1"));
	SkinAddNewSoundEx("Triggered2", Translate("Alarms"), Translate("Alert 2"));
	SkinAddNewSoundEx("Triggered3", Translate("Alarms"), Translate("Alert 3"));

	// load last checked time	
	DBCONTACTGETSETTING dbcgs;
	DBVARIANT dbv;
	dbcgs.szModule = MODULE;
	dbcgs.szSetting = "LastCheck";
	dbcgs.pValue = &dbv;
	dbv.type = DBVT_BLOB;
	dbv.cpbVal = sizeof(SYSTEMTIME);

	if(!CallService(MS_DB_CONTACT_GETSETTING, 0, (LPARAM)&dbcgs)) {
		memcpy(&last_check, dbv.pbVal, sizeof(SYSTEMTIME));
		DBFreeVariant(&dbv);
	} else {
		GetLocalTime(&last_check);
	}

	last_saved_check = last_check;

	LoadAlarms();

	hAlarmTriggeredEvent = CreateHookableEvent(ME_ALARMS_TRIGGERED);
	hAddAlarmService = CreateServiceFunction(MS_ALARMS_ADDALARM, AddAlarmService);

	InitAlarmWin();

	timer_id = SetTimer(0, 0, TIMER_PERIOD, TimerProc);

	HookEvent(ME_IDLE_CHANGED, IdleChanged);
}


void DeinitList() {

	DeinitAlarmWin();

	// i don't think this should be necessary, but...
	EnterCriticalSection(&alarm_cs);
	KillTimer(0, timer_id);
	LeaveCriticalSection(&alarm_cs);

	DestroyHookableEvent(hAlarmTriggeredEvent);
	DestroyServiceFunction(hAddAlarmService);

	SaveAlarms(); // we may have erased some 'cause they were once-offs that were triggered

	//WriteLastCheckTime(); // moved to the CheckAlarms function - for virt db and general crash problems

	// delete this after save alarms above
	DeleteCriticalSection(&alarm_cs);

}

