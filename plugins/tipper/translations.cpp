#include "common.h"
#include "translations.h"
#include "str_utils.h"

int num_tfuncs = 0;
DBVTranslation *translations = 0;

DWORD next_func_id;

HANDLE hServiceAdd;

void AddTranslation(DBVTranslation *new_trans) {
	num_tfuncs++;

	translations = (DBVTranslation *)realloc(translations, sizeof(DBVTranslation) * num_tfuncs);
	translations[num_tfuncs - 1] = *new_trans;

	char setting[256];
#ifdef _UNICODE
	WideCharToMultiByte(code_page, 0, new_trans->name, -1, setting, 256, 0, 0);
#else
	strncpy(setting, new_trans->name, 256);
#endif
	
	if(_tcscmp(new_trans->name, _T("[No translation]")) == 0)
		translations[num_tfuncs - 1].id = 0;
	else {
		DWORD id = DBGetContactSettingDword(0, MODULE, setting, 0);
		if(id != 0) {
			translations[num_tfuncs - 1].id = id;
			if(next_func_id <= id) next_func_id = id + 1;
		} else {
			translations[num_tfuncs - 1].id = next_func_id++;
			DBWriteContactSettingDword(0, MODULE, setting, translations[num_tfuncs - 1].id);
		}

		DBWriteContactSettingDword(0, MODULE, "NextFuncId", next_func_id);
	}
}

TCHAR *null_translation(HANDLE hContact, const char *module_name, const char *setting_name, TCHAR *buff, int bufflen) {
	DBVARIANT dbv;

	buff[0] = 0;

	if (DBGetContactSettingTString(hContact, module_name, setting_name, &dbv))
		return 0;

	_tcsncpy(buff, dbv.ptszVal, bufflen);
	buff[bufflen - 1] = 0;
	
	DBFreeVariant(&dbv);

	return buff[0] ? buff : NULL;
}

TCHAR *timestamp_to_short_date(HANDLE hContact, const char *module_name, const char *setting_name, TCHAR *buff, int bufflen) {
	DWORD ts = DBGetContactSettingDword(hContact, module_name, setting_name, 0);
	if(ts == 0) return 0;
	
	DBTIMETOSTRINGT dbt = {0};
	dbt.cbDest = bufflen;
	dbt.szDest = buff;
	dbt.szFormat = _T("d");
	CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)ts, (LPARAM)&dbt);

	return buff;
}

TCHAR *timestamp_to_long_date(HANDLE hContact, const char *module_name, const char *setting_name, TCHAR *buff, int bufflen) {
	DWORD ts = DBGetContactSettingDword(hContact, module_name, setting_name, 0);
	if(ts == 0) return 0;
	
	DBTIMETOSTRINGT dbt = {0};
	dbt.cbDest = bufflen;
	dbt.szDest = buff;
	dbt.szFormat = _T("D");
	CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)ts, (LPARAM)&dbt);

	return buff;
}

TCHAR *timestamp_to_time(HANDLE hContact, const char *module_name, const char *setting_name, TCHAR *buff, int bufflen) {
	DWORD ts = DBGetContactSettingDword(hContact, module_name, setting_name, 0);
	if(ts == 0) return 0;
	
	DBTIMETOSTRINGT dbt = {0};
	dbt.cbDest = bufflen;
	dbt.szDest = buff;
	dbt.szFormat = _T("s");
	CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)ts, (LPARAM)&dbt);

	return buff;
}

TCHAR *timestamp_to_time_no_secs(HANDLE hContact, const char *module_name, const char *setting_name, TCHAR *buff, int bufflen) {
	DWORD ts = DBGetContactSettingDword(hContact, module_name, setting_name, 0);
	if(ts == 0) return 0;
	
	DBTIMETOSTRINGT dbt = {0};
	dbt.cbDest = bufflen;
	dbt.szDest = buff;
	dbt.szFormat = _T("t");
	CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)ts, (LPARAM)&dbt);

	return buff;
}

TCHAR *timestamp_to_time_difference(HANDLE hContact, const char *module_name, const char *setting_name, TCHAR *buff, int bufflen) {
	DWORD ts = DBGetContactSettingDword(hContact, module_name, setting_name, 0);
	DWORD t = (DWORD)time(0);
	if(ts == 0) return 0;
	
	DWORD diff = (t - ts);
	int d = (diff / 60 / 60 / 24);
	int h = (diff - d * 60 * 60 * 24) / 60 / 60;
	int m = (diff  - d * 60 * 60 * 24 - h * 60 * 60) / 60;
	if(d > 0)
		mir_sntprintf(buff, bufflen, TranslateT("%dd %dh %dm"), d, h, m);
	else if(h > 0)
		mir_sntprintf(buff, bufflen, TranslateT("%dh %dm"), h, m);
	else
		mir_sntprintf(buff, bufflen, TranslateT("%dm"), m);

	return buff;
}

TCHAR *seconds_to_time_difference(HANDLE hContact, const char *module_name, const char *setting_name, TCHAR *buff, int bufflen) {

	DWORD diff = DBGetContactSettingDword(hContact, module_name, setting_name, 0);
	int d = (diff / 60 / 60 / 24);
	int h = (diff - d * 60 * 60 * 24) / 60 / 60;
	int m = (diff  - d * 60 * 60 * 24 - h * 60 * 60) / 60;
	if(d > 0)
		mir_sntprintf(buff, bufflen, TranslateT("%dd %dh %dm"), d, h, m);
	else if(h > 0)
		mir_sntprintf(buff, bufflen, TranslateT("%dh %dm"), h, m);
	else
		mir_sntprintf(buff, bufflen, TranslateT("%dm"), m);

	return buff;
}

TCHAR *word_to_status_desc(HANDLE hContact, const char *module_name, const char *setting_name, TCHAR *buff, int bufflen) {
	int status = DBGetContactSettingWord(hContact, module_name, setting_name, ID_STATUS_OFFLINE);
	char *strptr = (char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)status, (LPARAM)0);
	a2t(strptr, buff, bufflen);
	buff[bufflen - 1] = 0;
	return buff;
}

TCHAR *byte_to_yesno(HANDLE hContact, const char *module_name, const char *setting_name, TCHAR *buff, int bufflen) {
	DBVARIANT dbv;
	if(!DBGetContactSetting(hContact, module_name, setting_name, &dbv)) {
		if(dbv.type == DBVT_BYTE) {
			if(dbv.bVal != 0)
				_tcsncpy(buff, _T("Yes"), bufflen);
			else
				_tcsncpy(buff, _T("No"), bufflen);
			buff[bufflen - 1] = 0;
			DBFreeVariant(&dbv);
			return buff;
		}
		DBFreeVariant(&dbv);
	}
	return 0;
}

TCHAR *byte_to_mf(HANDLE hContact, const char *module_name, const char *setting_name, TCHAR *buff, int bufflen) {
	BYTE val = (BYTE)DBGetContactSettingByte(hContact, module_name, setting_name, 0);
	if(val == 'F')
		_tcsncpy(buff, TranslateT("Female"), bufflen);
	else if(val == 'M')
		_tcsncpy(buff, TranslateT("Male"), bufflen);
	else
		return 0;

	buff[bufflen - 1] = 0;
	return buff;
}

TCHAR *word_to_country(HANDLE hContact, const char *module_name, const char *setting_name, TCHAR *buff, int bufflen) {
	char *cname = 0;
	WORD cid = (WORD)DBGetContactSettingWord(hContact, module_name, setting_name, (WORD)-1);
	if(cid != (WORD)-1 && ServiceExists(MS_UTILS_GETCOUNTRYBYNUMBER) && (cname = (char *)CallService(MS_UTILS_GETCOUNTRYBYNUMBER, cid, 0)) != 0) {
		if(strcmp(cname, "Unknown") == 0)
			return 0;
		a2t(cname, buff, bufflen);
		buff[bufflen - 1] = 0;
		return buff;
	}
	return 0;
}

TCHAR *dword_to_ip(HANDLE hContact, const char *module_name, const char *setting_name, TCHAR *buff, int bufflen) {
	DWORD ip = DBGetContactSettingDword(hContact, module_name, setting_name, 0);
	if (ip) {
		unsigned char *ipc = (unsigned char*)&ip;
		mir_sntprintf(buff, bufflen, _T("%u.%u.%u.%u"), ipc[3], ipc[2], ipc[1], ipc[0]);
		return buff;
	}
	return 0;
}

bool GetInt(const DBVARIANT &dbv, int *val) {
	if(!val) return false;

	switch(dbv.type) {
		case DBVT_BYTE:
			if(val) *val = (int)dbv.bVal;
			return true;
		case DBVT_WORD:
			if(val) *val = (int)dbv.wVal;
			return true;
		case DBVT_DWORD:
			if(val) *val = (int)dbv.dVal;
			return true;
	}
	return false;
}

TCHAR *day_month_year_to_date(HANDLE hContact, const char *module_name, const char *prefix, TCHAR *buff, int bufflen) {
	DBVARIANT dbv;
	char setting_name[256];
	mir_snprintf(setting_name, 256, "%sDay", prefix);
	if(!DBGetContactSetting(hContact, module_name, setting_name, &dbv)) {
		int day = 0;
		if(GetInt(dbv, &day)) {
			DBFreeVariant(&dbv);
			mir_snprintf(setting_name, 256, "%sMonth", prefix);
			int month = 0;
			if(!DBGetContactSetting(hContact, module_name, setting_name, &dbv)) {
				if(GetInt(dbv, &month)) {
					DBFreeVariant(&dbv);
					mir_snprintf(setting_name, 256, "%sYear", prefix);
					int year = 0;
					if(!DBGetContactSetting(hContact, module_name, setting_name, &dbv)) {
						if(GetInt(dbv, &year)) {
							DBFreeVariant(&dbv);

							SYSTEMTIME st = {0};
							st.wDay = day;
							st.wMonth = month;
							st.wYear = year;

							GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, 0, buff, bufflen); 
							return buff;
						} else
							DBFreeVariant(&dbv);
					}
				} else
					DBFreeVariant(&dbv);
			}
		} else
			DBFreeVariant(&dbv);
	}
	return 0;
}

TCHAR *day_month_year_to_age(HANDLE hContact, const char *module_name, const char *prefix, TCHAR *buff, int bufflen) {
	DBVARIANT dbv;
	char setting_name[256];
	mir_snprintf(setting_name, 256, "%sDay", prefix);
	if(!DBGetContactSetting(hContact, module_name, setting_name, &dbv)) {
		int day = 0;
		if(GetInt(dbv, &day)) {
			DBFreeVariant(&dbv);
			mir_snprintf(setting_name, 256, "%sMonth", prefix);
			int month = 0;
			if(!DBGetContactSetting(hContact, module_name, setting_name, &dbv)) {
				if(GetInt(dbv, &month)) {
					DBFreeVariant(&dbv);
					mir_snprintf(setting_name, 256, "%sYear", prefix);
					int year = 0;
					if(!DBGetContactSetting(hContact, module_name, setting_name, &dbv)) {
						if(GetInt(dbv, &year)) {
							DBFreeVariant(&dbv);

							SYSTEMTIME now;
							GetLocalTime(&now);

							int age = now.wYear - year;
							if(now.wMonth < month || (now.wMonth == month && now.wDay < day))
								age--;
							mir_sntprintf(buff, bufflen, _T("%d"), age);
							return buff;
						} else
							DBFreeVariant(&dbv);
					}
				} else
					DBFreeVariant(&dbv);
			}
		} else
			DBFreeVariant(&dbv);
	}
	return 0;
}

TCHAR *hours_minutes_seconds_to_time(HANDLE hContact, const char *module_name, const char *prefix, TCHAR *buff, int bufflen) {
	DBVARIANT dbv;
	char setting_name[256];
	mir_snprintf(setting_name, 256, "%sHours", prefix);
	if(!DBGetContactSetting(hContact, module_name, setting_name, &dbv)) {
		int hours = 0;
		if(GetInt(dbv, &hours)) {
			DBFreeVariant(&dbv);
			mir_snprintf(setting_name, 256, "%sMinutes", prefix);
			int minutes = 0;
			if(!DBGetContactSetting(hContact, module_name, setting_name, &dbv)) {
				if(GetInt(dbv, &minutes)) {
					DBFreeVariant(&dbv);
					mir_snprintf(setting_name, 256, "%sSeconds", prefix);
					int seconds = 0;
					if(!DBGetContactSetting(hContact, module_name, setting_name, &dbv)) {
						GetInt(dbv, &seconds);
						DBFreeVariant(&dbv);
					}

					SYSTEMTIME st = {0};
					st.wHour = hours;
					st.wMinute = minutes;
					st.wSecond = seconds;

					GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, 0, buff, bufflen); 
					return buff;
				} else
					DBFreeVariant(&dbv);
			}
		} else
			DBFreeVariant(&dbv);
	}
	return 0;
}

TCHAR *hours_minutes_to_time(HANDLE hContact, const char *module_name, const char *prefix, TCHAR *buff, int bufflen) {
	DBVARIANT dbv;
	char setting_name[256];
	mir_snprintf(setting_name, 256, "%sHours", prefix);
	if(!DBGetContactSetting(hContact, module_name, setting_name, &dbv)) {
		int hours = 0;
		if(GetInt(dbv, &hours)) {
			DBFreeVariant(&dbv);
			mir_snprintf(setting_name, 256, "%sMinutes", prefix);
			int minutes = 0;
			if(!DBGetContactSetting(hContact, module_name, setting_name, &dbv)) {
				if(GetInt(dbv, &minutes)) {
					DBFreeVariant(&dbv);

					SYSTEMTIME st = {0};
					st.wHour = hours;
					st.wMinute = minutes;

					GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, 0, buff, bufflen); 
					return buff;
				} else
					DBFreeVariant(&dbv);
			}
		} else
			DBFreeVariant(&dbv);
	}
	return 0;
}

TCHAR *day_month_year_hours_minutes_seconds_to_time_difference(HANDLE hContact, const char *module_name, const char *prefix, TCHAR *buff, int bufflen) {
	DBVARIANT dbv;
	char setting_name[256];
	mir_snprintf(setting_name, 256, "%sDay", prefix);
	if(!DBGetContactSetting(hContact, module_name, setting_name, &dbv)) {
		int day = 0;
		if(GetInt(dbv, &day)) {
			DBFreeVariant(&dbv);
			mir_snprintf(setting_name, 256, "%sMonth", prefix);
			int month = 0;
			if(!DBGetContactSetting(hContact, module_name, setting_name, &dbv)) {
				if(GetInt(dbv, &month)) {
					DBFreeVariant(&dbv);
					mir_snprintf(setting_name, 256, "%sYear", prefix);
					int year = 0;
					if(!DBGetContactSetting(hContact, module_name, setting_name, &dbv)) {
						if(GetInt(dbv, &year)) {
							DBFreeVariant(&dbv);
							mir_snprintf(setting_name, 256, "%sHours", prefix);
							if(!DBGetContactSetting(hContact, module_name, setting_name, &dbv)) {
								int hours = 0;
								if(GetInt(dbv, &hours)) {
									DBFreeVariant(&dbv);
									mir_snprintf(setting_name, 256, "%sMinutes", prefix);
									int minutes = 0;
									if(!DBGetContactSetting(hContact, module_name, setting_name, &dbv)) {
										if(GetInt(dbv, &minutes)) {
											DBFreeVariant(&dbv);
											mir_snprintf(setting_name, 256, "%sSeconds", prefix);
											int seconds = 0;
											if(!DBGetContactSetting(hContact, module_name, setting_name, &dbv)) {
												GetInt(dbv, &seconds);
												DBFreeVariant(&dbv);
											}

											SYSTEMTIME st = {0}, st_now;
											st.wDay = day;
											st.wMonth = month;
											st.wYear = year;
											st.wHour = hours;
											st.wMinute = minutes;
											st.wSecond = seconds;
											GetLocalTime(&st_now);

											FILETIME ft, ft_now;
											SystemTimeToFileTime(&st, &ft);
											SystemTimeToFileTime(&st_now, &ft_now);
											
											LARGE_INTEGER li, li_now;
											li.HighPart = ft.dwHighDateTime; li.LowPart = ft.dwLowDateTime;
											li_now.HighPart = ft_now.dwHighDateTime; li_now.LowPart = ft_now.dwLowDateTime;

											long diff = (long)((li_now.QuadPart - li.QuadPart) / (LONGLONG)10000000L);
											int y = diff / 60 / 60 / 24 / 365;
											int d = (diff - y * 60 * 60 * 24 * 365) / 60 / 60 / 24;
											int h = (diff - y * 60 * 60 * 24 * 365 - d * 60 * 60 * 24) / 60 / 60;
											int m = (diff - y * 60 * 60 * 24 * 365  - d * 60 * 60 * 24 - h * 60 * 60) / 60;
											if(y != 0)
												mir_sntprintf(buff, bufflen, TranslateT("%dy %dd %dh %dm"), y, d, h, m);
											else if(d != 0)
												mir_sntprintf(buff, bufflen, TranslateT("%dd %dh %dm"), d, h, m);
											else if(h != 0)
												mir_sntprintf(buff, bufflen, TranslateT("%dh %dm"), h, m);
											else
												mir_sntprintf(buff, bufflen, TranslateT("%dm"), m);

											return buff;
										} else
											DBFreeVariant(&dbv);
									}
								} else
									DBFreeVariant(&dbv);
							}
						} else
							DBFreeVariant(&dbv);
					}
				} else
					DBFreeVariant(&dbv);
			}
		} else
			DBFreeVariant(&dbv);
	}
	return 0;
}

TCHAR *empty_xStatus_name_to_default_name(HANDLE hContact, const char *module_name, const char *setting_name, TCHAR *buff, int bufflen) {
	TCHAR szDefaultName[1024];
	ICQ_CUSTOM_STATUS xstatus={0};
	
	if(null_translation(hContact, module_name, setting_name, buff, bufflen))
	   return buff;
	
	int status = (int)DBGetContactSettingByte(hContact, module_name, "XStatusId", 0);
	if(!status) return 0;
	
	xstatus.cbSize = sizeof(ICQ_CUSTOM_STATUS);
	xstatus.flags = CSSF_MASK_NAME|CSSF_DEFAULT_NAME|CSSF_TCHAR;
	xstatus.ptszName = szDefaultName;
	xstatus.wParam = (WPARAM *)&status;
	if(CallProtoService(module_name, PS_ICQ_GETCUSTOMSTATUSEX, 0, (LPARAM)&xstatus))
	   return 0;
	
	_tcsncpy(buff, TranslateTS(szDefaultName), bufflen);
	buff[bufflen - 1] = 0;
	
	return buff;
}

TCHAR *timezone_to_time(HANDLE hContact, const char *module_name, const char *setting_name, TCHAR *buff, int bufflen) {
	int timezone = DBGetContactSettingByte(hContact,module_name,setting_name,256);
	if(timezone==256 || (char)timezone==-100) {
		return 0;
	}

	TIME_ZONE_INFORMATION tzi;
	FILETIME ft;
	LARGE_INTEGER lift;
	SYSTEMTIME st;

	timezone=(char)timezone;
	GetSystemTimeAsFileTime(&ft);
	if(GetTimeZoneInformation(&tzi) == TIME_ZONE_ID_DAYLIGHT)
		timezone += tzi.DaylightBias / 30;

	lift.QuadPart = *(__int64*)&ft;
	lift.QuadPart -= (__int64)timezone * BIGI(30) * BIGI(60) * BIGI(10000000);
	*(__int64*)&ft = lift.QuadPart;
	FileTimeToSystemTime(&ft, &st);
	GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, buff, bufflen);

	return buff;
}

INT_PTR ServiceAddTranslation(WPARAM wParam, LPARAM lParam) {
	if(!lParam) return 1;

	DBVTranslation *trans = (DBVTranslation *)lParam;
	AddTranslation(trans);

	return 0;
}

void InitTranslations() {
	next_func_id = DBGetContactSettingDword(0, MODULE, "NextFuncId", 1);

#define INT_TRANS_COUNT 19
	DBVTranslation internal_translations[INT_TRANS_COUNT] = {
		{
			(TranslateFunc*)null_translation,
			_T("[No translation]"),
		},
		{
			(TranslateFunc*)word_to_status_desc,
			_T("WORD to status description")
		},
		{
			(TranslateFunc*)timestamp_to_time,
			_T("DWORD timestamp to time")
		},
		{
			(TranslateFunc*)timestamp_to_time_difference,
			_T("DWORD timestamp to time difference")
		},
		{
			(TranslateFunc*)byte_to_yesno,
			_T("BYTE to Yes/No")
		},
		{
			(TranslateFunc*)byte_to_mf,
			_T("BYTE to Male/Female (ICQ)")
		},
		{
			(TranslateFunc*)word_to_country,
			_T("WORD to country name")
		},
		{
			(TranslateFunc*)dword_to_ip,
			_T("DWORD to ip address")
		},
		{
			(TranslateFunc*)day_month_year_to_date,
			_T("<prefix>Day|Month|Year to date")
		},
		{
			(TranslateFunc*)day_month_year_to_age,
			_T("<prefix>Day|Month|Year to age")
		},
		{
			(TranslateFunc*)hours_minutes_seconds_to_time,
			_T("<prefix>Hours|Minutes|Seconds to time")
		},
		{
			(TranslateFunc*)day_month_year_hours_minutes_seconds_to_time_difference,
			_T("<prefix>Day|Month|Year|Hours|Minutes|Seconds to time difference")
		},
		{
			(TranslateFunc*)timestamp_to_time_no_secs,
			_T("DWORD timestamp to time (no seconds)")
		},
		{
			(TranslateFunc*)hours_minutes_to_time,
			_T("<prefix>Hours|Minutes to time")
		},
		{
			(TranslateFunc*)timestamp_to_short_date,
			_T("DWORD timestamp to date (short)")
		},
		{
			(TranslateFunc*)timestamp_to_long_date,
			_T("DWORD timestamp to date (long)")
		},
		{
			(TranslateFunc*)empty_xStatus_name_to_default_name,
			_T("xStatus: empty xStatus name to default name")
		},
		{
			(TranslateFunc*)seconds_to_time_difference,
			_T("DWORD seconds to time difference")
		},
		{
			(TranslateFunc*)timezone_to_time,
			_T("BYTE timezone to time")
		}
	};

	for(int i = 0; i < INT_TRANS_COUNT; i++) AddTranslation(&internal_translations[i]);

	hServiceAdd = CreateServiceFunction(MS_TIPPER_ADDTRANSLATION, ServiceAddTranslation);
}

void DeinitTranslations() {
	DestroyServiceFunction(hServiceAdd);
	free(translations);
}

