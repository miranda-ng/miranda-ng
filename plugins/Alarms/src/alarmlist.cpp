#include "stdafx.h"

AlarmList alarms;
mir_cs alarm_cs;

unsigned short next_alarm_id = 1; // 0 is used for invalid id

uint32_t timer_id;
#define TIMER_PERIOD				5000 // milliseconds delay between alarm checks

static SYSTEMTIME last_check, last_saved_check;

HANDLE hAlarmTriggeredEvent;

bool startup = true;

bool is_idle = false;

void free_alarm_data(ALARM *alarm)
{
	mir_free(alarm->szTitle); alarm->szTitle = nullptr;
	mir_free(alarm->szDesc); alarm->szDesc = nullptr;
	mir_free(alarm->szCommand); alarm->szCommand = nullptr;
	mir_free(alarm->szCommandParams); alarm->szCommandParams = nullptr;
}

void copy_alarm_data(ALARM *dest, ALARM *src)
{
	dest->action = src->action;
	dest->flags = src->flags;
	dest->day_mask = src->day_mask;
	dest->id = src->id;
	dest->occurrence = src->occurrence;
	dest->snoozer = src->snoozer;
	dest->sound_num = src->sound_num;
	dest->time = src->time;

	free_alarm_data(dest);
	dest->szTitle = mir_wstrdup(src->szTitle);
	dest->szDesc = mir_wstrdup(src->szDesc);
	dest->szCommand = mir_wstrdup(src->szCommand);
	dest->szCommandParams = mir_wstrdup(src->szCommandParams);
}

void GetPluginTime(SYSTEMTIME *t)
{
	mir_cslock lck(alarm_cs);
	*t = last_check;
}

int MinutesInFuture(SYSTEMTIME time, Occurrence occ, int selected_days)
{
	if (!UpdateAlarm(time, occ, selected_days))
		return 0;

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
	if (diff.QuadPart % mult.QuadPart >= mult.QuadPart / 2)
		inc = true;
	return (int)(diff.QuadPart / mult.QuadPart + (inc ? 1 : 0));
}

void TimeForMinutesInFuture(int mins, SYSTEMTIME *time)
{
	SYSTEMTIME now;
	GetPluginTime(&now);

	FILETIME ft_now;
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

bool is_day_selected(int day, int selected_days)
{
	switch (day) {
	case 0:
		return (selected_days & ALDF_7) != 0;
	case 1:
		return (selected_days & ALDF_1) != 0;
	case 2:
		return (selected_days & ALDF_2) != 0;
	case 3:
		return (selected_days & ALDF_3) != 0;
	case 4:
		return (selected_days & ALDF_4) != 0;
	case 5:
		return (selected_days & ALDF_5) != 0;
	case 6:
		return (selected_days & ALDF_6) != 0;
	}

	return false;
}

bool UpdateAlarm(SYSTEMTIME &time, Occurrence occ, int selected_days)
{
	FILETIME ft_now, ft_then;
	ULARGE_INTEGER uli_then;

	switch (occ) {
	case OC_DAILY:
	case OC_WEEKDAYS:
	case OC_WEEKLY:
	case OC_SELECTED_DAYS:
		time.wDay = last_check.wDay;
	case OC_MONTHLY:
		time.wMonth = last_check.wMonth;
	case OC_YEARLY:
		time.wYear = last_check.wYear;
	case OC_ONCE:
		break; // all fields valid
	}

	SystemTimeToFileTime(&last_check, &ft_now); // consider 'now' to be last check time
	SystemTimeToFileTime(&time, &ft_then);

	switch (occ) {
	case OC_ONCE:
		if (CompareFileTime(&ft_then, &ft_now) < 0)
			return false;
		break;

	case OC_YEARLY:
		while (CompareFileTime(&ft_then, &ft_now) < 0) {
			time.wYear++;
			SystemTimeToFileTime(&time, &ft_then);
		}
		break;

	case OC_MONTHLY:
		while (CompareFileTime(&ft_then, &ft_now) < 0) {
			if (time.wMonth == 12) {
				time.wMonth = 1;
				time.wYear++;
			}
			else
				time.wMonth++;
			SystemTimeToFileTime(&time, &ft_then);
		}
		break;

	case OC_WEEKLY:
		SYSTEMTIME temp;
		uli_then.HighPart = ft_then.dwHighDateTime;
		uli_then.LowPart = ft_then.dwLowDateTime;
		FileTimeToSystemTime(&ft_then, &temp);
		do {
			if (temp.wDayOfWeek != time.wDayOfWeek || CompareFileTime(&ft_then, &ft_now) < 0) {
				uli_then.QuadPart += mult.QuadPart * (ULONGLONG)24 * (ULONGLONG)60;
				ft_then.dwHighDateTime = uli_then.HighPart;
				ft_then.dwLowDateTime = uli_then.LowPart;
				FileTimeToSystemTime(&ft_then, &temp);
			}
		}
			while (temp.wDayOfWeek != time.wDayOfWeek || CompareFileTime(&ft_then, &ft_now) < 0);
		break;

	case OC_WEEKDAYS:
		uli_then.HighPart = ft_then.dwHighDateTime;
		uli_then.LowPart = ft_then.dwLowDateTime;
		do {
			FileTimeToSystemTime(&ft_then, &temp);
			if (temp.wDayOfWeek == 0 || temp.wDayOfWeek == 6 || CompareFileTime(&ft_then, &ft_now) < 0) {
				uli_then.QuadPart += mult.QuadPart * (ULONGLONG)24 * (ULONGLONG)60;
				ft_then.dwHighDateTime = uli_then.HighPart;
				ft_then.dwLowDateTime = uli_then.LowPart;
			}
		}
			while (temp.wDayOfWeek == 0 || temp.wDayOfWeek == 6 || CompareFileTime(&ft_then, &ft_now) < 0);
		break;

	case OC_SELECTED_DAYS:
		uli_then.HighPart = ft_then.dwHighDateTime;
		uli_then.LowPart = ft_then.dwLowDateTime;
		FileTimeToSystemTime(&ft_now, &temp);
		{
			int day = temp.wDayOfWeek;
			while (CompareFileTime(&ft_then, &ft_now) < 0 || (selected_days && !is_day_selected(day, selected_days))) {
				uli_then.QuadPart += mult.QuadPart * (ULONGLONG)24 * (ULONGLONG)60;
				ft_then.dwHighDateTime = uli_then.HighPart;
				ft_then.dwLowDateTime = uli_then.LowPart;
				if (day < 7)
					day++;
				else
					day = 0;
			}
		}
		break;

	case OC_DAILY:
		uli_then.HighPart = ft_then.dwHighDateTime;
		uli_then.LowPart = ft_then.dwLowDateTime;
		while (CompareFileTime(&ft_then, &ft_now) < 0) {
			uli_then.QuadPart += mult.QuadPart * (ULONGLONG)24 * (ULONGLONG)60;
			ft_then.dwHighDateTime = uli_then.HighPart;
			ft_then.dwLowDateTime = uli_then.LowPart;
		}
		break;
	}

	FileTimeToSystemTime(&ft_then, &time);
	return true;
}

void LoadAlarms()
{
	int num_alarms = g_plugin.getWord("Count", 0);
	char buff[256];
	DBVARIANT dbv;
	ALARM alarm;
	SYSTEMTIME now;
	GetLocalTime(&now);

	mir_cslock lck(alarm_cs);
	alarms.clear();

	for (int i = 0; i < num_alarms; i++) {
		memset(&alarm, 0, sizeof(ALARM));

		mir_snprintf(buff, "Title%d", i);
		if (!g_plugin.getWString(buff, &dbv)) {
			alarm.szTitle = mir_wstrdup(dbv.pwszVal);
			db_free(&dbv);
		}
		mir_snprintf(buff, "Desc%d", i);
		if (!g_plugin.getWString(buff, &dbv)) {
			alarm.szDesc = mir_wstrdup(dbv.pwszVal);
			db_free(&dbv);
		}
		mir_snprintf(buff, "Occ%d", i);
		alarm.occurrence = (Occurrence)g_plugin.getWord(buff, 0);

		mir_snprintf(buff, "STHour%d", i);
		alarm.time.wHour = g_plugin.getWord(buff, 0);
		mir_snprintf(buff, "STMinute%d", i);
		alarm.time.wMinute = g_plugin.getWord(buff, 0);
		mir_snprintf(buff, "STSecond%d", i);
		alarm.time.wSecond = g_plugin.getWord(buff, 0);

		switch (alarm.occurrence) {

		case OC_ONCE:
			mir_snprintf(buff, "STYear%d", i);
			alarm.time.wYear = g_plugin.getWord(buff, 0);
			mir_snprintf(buff, "STMonth%d", i);
			alarm.time.wMonth = g_plugin.getWord(buff, 0);
			mir_snprintf(buff, "STDay%d", i);
			alarm.time.wDay = g_plugin.getWord(buff, 0);
			break;
		case OC_WEEKLY:
			mir_snprintf(buff, "STDayOfWeek%d", i);
			alarm.time.wDayOfWeek = g_plugin.getWord(buff, 0);
			break;
		case OC_WEEKDAYS:
			break;
		case OC_SELECTED_DAYS:
			mir_snprintf(buff, "SelectedDays%d", i);
			alarm.day_mask = g_plugin.getDword(buff, alarm.day_mask);
			break;
		case OC_DAILY:
			break;
		case OC_MONTHLY:
			mir_snprintf(buff, "STDay%d", i);
			alarm.time.wDay = g_plugin.getWord(buff, 0);
			break;
		case OC_YEARLY:
			mir_snprintf(buff, "STMonth%d", i);
			alarm.time.wMonth = g_plugin.getWord(buff, 0);
			mir_snprintf(buff, "STDay%d", i);
			alarm.time.wDay = g_plugin.getWord(buff, 0);
			break;
		}

		if (UpdateAlarm(alarm.time, alarm.occurrence, alarm.day_mask)) {
			mir_snprintf(buff, "ActionFlags%d", i);
			alarm.action = (unsigned short)g_plugin.getDword(buff, AAF_POPUP | AAF_SOUND);
			if (alarm.action & AAF_COMMAND) {
				mir_snprintf(buff, "ActionCommand%d", i);
				if (!g_plugin.getWString(buff, &dbv)) {
					alarm.szCommand = mir_wstrdup(dbv.pwszVal);
					db_free(&dbv);
					mir_snprintf(buff, "ActionParams%d", i);
					if (!g_plugin.getWString(buff, &dbv)) {
						alarm.szCommandParams = mir_wstrdup(dbv.pwszVal);
						db_free(&dbv);
					}
				}
			}

			mir_snprintf(buff, "SoundNum%d", i);
			alarm.sound_num = (int)g_plugin.getByte(buff, 1);

			mir_snprintf(buff, "Snoozer%d", i);
			alarm.snoozer = g_plugin.getByte(buff, 0) == 1;

			mir_snprintf(buff, "Hidden%d", i);
			alarm.flags |= (g_plugin.getByte(buff, 0) == 1 ? ALF_HIDDEN : 0);

			mir_snprintf(buff, "Suspended%d", i);
			alarm.flags |= (g_plugin.getByte(buff, 0) == 1 ? ALF_SUSPENDED : 0);

			mir_snprintf(buff, "NoStartup%d", i);
			alarm.flags |= (g_plugin.getByte(buff, 0) == 1 ? ALF_NOSTARTUP : 0);

			mir_snprintf(buff, "Flags%d", i);
			alarm.flags = g_plugin.getDword(buff, alarm.flags);

			alarm.id = next_alarm_id++;
			alarms.push_back(&alarm);
		}
		free_alarm_data(&alarm);
	}
}

int db_enum_settings_sub_cb(const char *szSetting, void *lParam)
{
	//quick and dirty solution, feel free to rewrite
	std::list<char*> *settings = (std::list<char*>*)lParam;
	if (strstr(szSetting, "Title"))
		settings->push_back(mir_strdup(szSetting));
	if (strstr(szSetting, "Desc"))
		settings->push_back(mir_strdup(szSetting));
	if (strstr(szSetting, "Occ"))
		settings->push_back(mir_strdup(szSetting));
	if (strstr(szSetting, "STHour"))
		settings->push_back(mir_strdup(szSetting));
	if (strstr(szSetting, "STMinute"))
		settings->push_back(mir_strdup(szSetting));
	if (strstr(szSetting, "STSecond"))
		settings->push_back(mir_strdup(szSetting));
	if (strstr(szSetting, "SelectedDays"))
		settings->push_back(mir_strdup(szSetting));
	if (strstr(szSetting, "STDayOfWeek"))
		settings->push_back(mir_strdup(szSetting));
	if (strstr(szSetting, "STYear"))
		settings->push_back(mir_strdup(szSetting));
	if (strstr(szSetting, "STMonth"))
		settings->push_back(mir_strdup(szSetting));
	if (strstr(szSetting, "STDay"))
		settings->push_back(mir_strdup(szSetting));
	if (strstr(szSetting, "ActionFlags"))
		settings->push_back(mir_strdup(szSetting));
	if (strstr(szSetting, "ActionCommand"))
		settings->push_back(mir_strdup(szSetting));
	if (strstr(szSetting, "ActionParams"))
		settings->push_back(mir_strdup(szSetting));
	if (strstr(szSetting, "SoundNum"))
		settings->push_back(mir_strdup(szSetting));
	if (strstr(szSetting, "Snoozer"))
		settings->push_back(mir_strdup(szSetting));
	if (strstr(szSetting, "Flags"))
		settings->push_back(mir_strdup(szSetting));

	return 0;
}

void SaveAlarms()
{
	int index = 0;
	char buff[256];

	mir_cslock lck(alarm_cs);

	//clean old data here
	{
		//quick and dirty solution, feel free to rewrite
		//TODO: this should be done on "delete" button press instead, and for selected alrams only
		std::list<char*> settings;
		db_enum_settings(0, &db_enum_settings_sub_cb, MODULENAME, &settings);
		for (std::list<char*>::iterator i = settings.begin(), end = settings.end(); i != end; ++i)
		{
			g_plugin.delSetting(*i);
			mir_free(*i);
		}
	}


	ALARM *i;
	for (alarms.reset(); i = alarms.current(); alarms.next(), index++) {
		mir_snprintf(buff, "Title%d", index);
		g_plugin.setWString(buff, i->szTitle);
		mir_snprintf(buff, "Desc%d", index);
		g_plugin.setWString(buff, i->szDesc);
		mir_snprintf(buff, "Occ%d", index);
		g_plugin.setWord(buff, i->occurrence);

		mir_snprintf(buff, "STHour%d", index);
		g_plugin.setWord(buff, i->time.wHour);
		mir_snprintf(buff, "STMinute%d", index);
		g_plugin.setWord(buff, i->time.wMinute);
		mir_snprintf(buff, "STSecond%d", index);
		g_plugin.setWord(buff, i->time.wSecond);

		switch (i->occurrence) {
		case OC_DAILY:
			break;
		case OC_WEEKDAYS:
			break;
		case OC_SELECTED_DAYS:
			mir_snprintf(buff, "SelectedDays%d", index);
			g_plugin.setDword(buff, i->day_mask);
			break;
		case OC_WEEKLY:
			mir_snprintf(buff, "STDayOfWeek%d", index);
			g_plugin.setWord(buff, i->time.wDayOfWeek);
			break;

		case OC_ONCE:
			mir_snprintf(buff, "STYear%d", index);
			g_plugin.setWord(buff, i->time.wYear);
		case OC_YEARLY:
			mir_snprintf(buff, "STMonth%d", index);
			g_plugin.setWord(buff, i->time.wMonth);
		case OC_MONTHLY:
			mir_snprintf(buff, "STDay%d", index);
			g_plugin.setWord(buff, i->time.wDay);
			break;
		}
		mir_snprintf(buff, "ActionFlags%d", index);
		g_plugin.setDword(buff, i->action);
		if (i->action & AAF_COMMAND) {
			if (mir_wstrlen(i->szCommand)) {
				mir_snprintf(buff, "ActionCommand%d", index);
				g_plugin.setWString(buff, i->szCommand);
				if (mir_wstrlen(i->szCommandParams)) {
					mir_snprintf(buff, "ActionParams%d", index);
					g_plugin.setWString(buff, i->szCommandParams);
				}
			}
		}

		mir_snprintf(buff, "SoundNum%d", index);
		g_plugin.setByte(buff, i->sound_num);

		mir_snprintf(buff, "Snoozer%d", index);
		g_plugin.setByte(buff, i->snoozer ? 1 : 0);

		mir_snprintf(buff, "Flags%d", index);
		g_plugin.setDword(buff, i->flags);
	}
	g_plugin.setWord("Count", index);
}

void copy_list(AlarmList &copy)
{
	copy.clear();
	ALARM *i;
	mir_cslock lck(alarm_cs);
	for (alarms.reset(); i = alarms.current(); alarms.next())
		copy.push_back(i);
}

void copy_list(AlarmList &copy, SYSTEMTIME &start, SYSTEMTIME &end)
{
	copy.clear();
	ALARM *i;
	mir_cslock lck(alarm_cs);
	for (alarms.reset(); i = alarms.current(); alarms.next())
		if (IsBetween(i->time, start, end))
			copy.push_back(i);
}

void set_list(AlarmList &copy)
{
	mir_cslock lck(alarm_cs);
	alarms.clear();
	ALARM *i;
	for (copy.reset(); i = copy.current(); copy.next())
		alarms.push_back(i);

	SaveAlarms();
}

void append_to_list(ALARM *alarm)
{
	mir_cslock lck(alarm_cs);
	if (!alarm->id)
		alarm->id = next_alarm_id++;
	alarms.push_back(alarm);

	SaveAlarms();
}

void alter_alarm_list(ALARM *alarm)
{
	bool found = false;
	mir_cslock lck(alarm_cs);
	if (alarm->id != 0) {
		ALARM *i;
		for (alarms.reset(); i = alarms.current(); alarms.next()) {
			if (i->id == alarm->id) {
				copy_alarm_data(i, alarm);
				found = true;
				break;
			}
		}
	}
	if (!found) {
		if (!alarm->id)
			alarm->id = next_alarm_id++;
		alarms.push_back(alarm);
	}

	SaveAlarms();
}

void remove(unsigned short alarm_id)
{
	mir_cslock lck(alarm_cs);
	ALARM *i;
	for (alarms.reset(); i = alarms.current(); alarms.next()) {
		if (i->id == alarm_id) {
			alarms.erase();
			break;
		}
	}

	SaveAlarms();
}

void suspend(unsigned short alarm_id)
{
	mir_cslock lck(alarm_cs);
	ALARM *i;
	for (alarms.reset(); i = alarms.current(); alarms.next()) {
		if (i->id == alarm_id && i->occurrence != OC_ONCE) {
			i->flags |= ALF_SUSPENDED;
			break;
		}
	}

	SaveAlarms();
}

static LRESULT CALLBACK PopupAlarmDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COMMAND: // snooze
		if (HIWORD(wParam) == STN_CLICKED) { //It was a click on the Popup.
			ALARM *mpd = (ALARM *)PUGetPluginData(hWnd);

			if (mpd->flags & ALF_NOSNOOZE || !options.snooze_minutes)
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

		PUDeletePopup(hWnd);
		return TRUE;

	case WM_CONTEXTMENU:
		PUDeletePopup(hWnd);
		return TRUE;

	case UM_FREEPLUGINDATA:
		ALARM *mpd = (ALARM *)PUGetPluginData(hWnd);
		if (mpd > 0) {
			free_alarm_data(mpd);
			delete mpd;
		}
		return TRUE;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void ShowPopup(ALARM *alarm)
{
	ALARM *data = new ALARM;
	memset(data, 0, sizeof(ALARM));
	copy_alarm_data(data, alarm);

	POPUPDATAW ppd;
	ppd.lchIcon = hIconMenuSet;
	mir_wstrncpy(ppd.lpwzContactName, data->szTitle, MAX_CONTACTNAME);
	mir_wstrncpy(ppd.lpwzText, data->szDesc, MAX_SECONDLINE);
	ppd.PluginWindowProc = PopupAlarmDlgProc;
	ppd.PluginData = data;
	ppd.iSeconds = -1;
	PUAddPopupW(&ppd);
}

void DoAlarm(ALARM *alarm)
{
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

	if (!NotifyEventHooks(hAlarmTriggeredEvent, 0, (LPARAM)&alarminfo)) {
		if (alarm->action & AAF_SOUND) {
			if (alarm->sound_num > 0 && alarm->sound_num <= 3) {
				char buff[128];
				mir_snprintf(buff, "Triggered%d", alarm->sound_num);
				Skin_PlaySound(buff);
			}
			else if (alarm->sound_num == 4) {
				if (alarm->szTitle != nullptr && alarm->szTitle[0] != '\0') {
					if (ServiceExists("Speak/Say")) {
						CallService("Speak/Say", 0, (LPARAM)alarm->szTitle);
					}
				}
			}
		}

		if (alarm->action & AAF_POPUP) {
			if (options.use_popup_module)
				ShowPopup(alarm);
			else {
				HWND hwndDlg = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_ALARM), GetDesktopWindow(), DlgProcAlarm);
				WindowList_Add(hAlarmWindowList, hwndDlg, 0);

				ALARM *data = new ALARM;
				memset(data, 0, sizeof(ALARM));
				copy_alarm_data(data, alarm);
				SendMessage(hwndDlg, WMU_SETALARM, 0, (LPARAM)data);
				if (is_idle || !options.aw_dontstealfocus)
					ShowWindow(hwndDlg, SW_SHOW);
				else
					ShowWindow(hwndDlg, SW_SHOWNOACTIVATE);
				SetWindowPos(hwndDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			}
		}

		if (alarm->action & AAF_COMMAND)
			ShellExecute(nullptr, nullptr, alarm->szCommand, alarm->szCommandParams, nullptr, SW_NORMAL);

		if (alarm->action & AAF_SYSTRAY) {
			CLISTEVENT cle = {};
			cle.hIcon = hIconSystray;
			cle.szTooltip.w = alarm->szTitle;
			cle.flags = CLEF_ONLYAFEW | CLEF_UNICODE;
			g_clistApi.pfnAddEvent(&cle);
		}
	}
}

void WriteLastCheckTime()
{
	// save last-check time
	db_set_blob(0, MODULENAME, "LastCheck", &last_check, sizeof(SYSTEMTIME));
	last_saved_check = last_check;
}

void CheckAlarms()
{
	SYSTEMTIME time;
	GetLocalTime(&time);

	// put triggered alarms in another list - so we don't keep the critical section locked for longer than necessary
	AlarmList triggered_list, remove_list;

	mir_cslock lck(alarm_cs);
	ALARM *i;
	for (alarms.reset(); i = alarms.current(); alarms.next()) {
		if (!UpdateAlarm(i->time, i->occurrence, i->day_mask)) {
			// somehow an expired one-off alarm is in our list
			remove_list.push_back(i);
			continue;
		}

		switch (i->occurrence) {
		case OC_ONCE:
			if (IsBetween(i->time, last_check, time)) {
				if (!startup || !(i->flags & ALF_NOSTARTUP)) triggered_list.push_back(i);
				// erase and fix iterator - alarm has now been triggered and has therefore expired
				remove_list.push_back(i);
			}
			break;

		default:
			if (IsBetween(i->time, last_check, time)) {
				if (i->flags & ALF_SUSPENDED)
					i->flags = i->flags & ~ALF_SUSPENDED;
				else
					if (!startup || !(i->flags & ALF_NOSTARTUP)) triggered_list.push_back(i);
			}
			break;
		}
	}

	last_check = time;
	WriteLastCheckTime();

	startup = false;

	for (triggered_list.reset(); i = triggered_list.current(); triggered_list.next())
		DoAlarm(i);
	for (remove_list.reset(); i = remove_list.current(); remove_list.next())
		remove(i->id);
}

VOID CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	CheckAlarms();
}

INT_PTR AddAlarmService(WPARAM, LPARAM lParam)
{
	ALARMINFO *alarm_info = (ALARMINFO *)lParam;
	ALARM alarm = { 0 };
	alarm.action = alarm_info->action;
	alarm.flags = alarm_info->flags;
	alarm.id = next_alarm_id++;
	alarm.occurrence = alarm_info->occurrence;
	alarm.snoozer = alarm_info->snoozer;
	alarm.sound_num = alarm_info->sound_num;
	alarm.szCommand = mir_wstrdup(alarm_info->szCommand);
	alarm.szCommandParams = mir_wstrdup(alarm_info->szCommandParams);
	alarm.szDesc = mir_wstrdup(alarm_info->szDesc);
	alarm.szTitle = mir_wstrdup(alarm_info->szTitle);
	alarm.time = alarm_info->time;

	append_to_list(&alarm);
	return 0;
}

int IdleChanged(WPARAM, LPARAM lParam)
{
	is_idle = (lParam & IDF_ISIDLE);
	return 0;
}

void InitList()
{
	g_plugin.addSound("Triggered1", LPGENW("Alarms"), LPGENW("Alert 1"));
	g_plugin.addSound("Triggered2", LPGENW("Alarms"), LPGENW("Alert 2"));
	g_plugin.addSound("Triggered3", LPGENW("Alarms"), LPGENW("Alert 3"));

	// load last checked time	
	DBVARIANT dbv;
	dbv.type = DBVT_BLOB;
	dbv.cpbVal = sizeof(SYSTEMTIME);
	if (!db_get(0, MODULENAME, "LastCheck", &dbv)) {
		memcpy(&last_check, dbv.pbVal, sizeof(SYSTEMTIME));
		db_free(&dbv);
	}
	else GetLocalTime(&last_check);

	last_saved_check = last_check;

	LoadAlarms();

	hAlarmTriggeredEvent = CreateHookableEvent(ME_ALARMS_TRIGGERED);
	CreateServiceFunction(MS_ALARMS_ADDALARM, AddAlarmService);

	InitAlarmWin();

	timer_id = SetTimer(nullptr, 0, TIMER_PERIOD, TimerProc);

	HookEvent(ME_IDLE_CHANGED, IdleChanged);
}


void DeinitList()
{
	DeinitAlarmWin();

	// i don't think this should be necessary, but...
	mir_cslock lck(alarm_cs);
	KillTimer(nullptr, timer_id);

	DestroyHookableEvent(hAlarmTriggeredEvent);

	SaveAlarms(); // we may have erased some 'cause they were once-offs that were triggeredf
}
