/*
Copyright (C) 2006-2007 Scott Ellis
Copyright (C) 2007-2011 Jan Holub

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

int iTransFuncsCount = 0;
DBVTranslation *translations = nullptr;

uint32_t dwNextFuncId;
HANDLE hServiceAdd;

static LISTTYPEDATAITEM languages[] =
{
  {0, LPGEN("None")},
  {55,LPGEN("Afrikaans")},
  {58,LPGEN("Albanian")},
  {1, LPGEN("Arabic")},
  {59,LPGEN("Armenian")},
  {68,LPGEN("Azerbaijani")},
  {72,LPGEN("Belorussian")},
  {2, LPGEN("Bhojpuri")},
  {56,LPGEN("Bosnian")},
  {3, LPGEN("Bulgarian")},
  {4, LPGEN("Burmese")},
  {5, LPGEN("Cantonese")},
  {6, LPGEN("Catalan")},
  {61,LPGEN("Chamorro")},
  {7, LPGEN("Chinese")},
  {8, LPGEN("Croatian")},
  {9, LPGEN("Czech")},
  {10,LPGEN("Danish")},
  {11,LPGEN("Dutch")},
  {12,LPGEN("English")},
  {13,LPGEN("Esperanto")},
  {14,LPGEN("Estonian")},
  {15,LPGEN("Farsi")},
  {16,LPGEN("Finnish")},
  {17,LPGEN("French")},
  {18,LPGEN("Gaelic")},
  {19,LPGEN("German")},
  {20,LPGEN("Greek")},
  {70,LPGEN("Gujarati")},
  {21,LPGEN("Hebrew")},
  {22,LPGEN("Hindi")},
  {23,LPGEN("Hungarian")},
  {24,LPGEN("Icelandic")},
  {25,LPGEN("Indonesian")},
  {26,LPGEN("Italian")},
  {27,LPGEN("Japanese")},
  {28,LPGEN("Khmer")},
  {29,LPGEN("Korean")},
  {69,LPGEN("Kurdish")},
  {30,LPGEN("Lao")},
  {31,LPGEN("Latvian")},
  {32,LPGEN("Lithuanian")},
  {65,LPGEN("Macedonian")},
  {33,LPGEN("Malay")},
  {63,LPGEN("Mandarin")},
  {62,LPGEN("Mongolian")},
  {34,LPGEN("Norwegian")},
  {57,LPGEN("Persian")},
  {35,LPGEN("Polish")},
  {36,LPGEN("Portuguese")},
  {60,LPGEN("Punjabi")},
  {37,LPGEN("Romanian")},
  {38,LPGEN("Russian")},
  {39,LPGEN("Serbo-Croatian")},
  {66,LPGEN("Sindhi")},
  {40,LPGEN("Slovak")},
  {41,LPGEN("Slovenian")},
  {42,LPGEN("Somali")},
  {43,LPGEN("Spanish")},
  {44,LPGEN("Swahili")},
  {45,LPGEN("Swedish")},
  {46,LPGEN("Tagalog")},
  {64,LPGEN("Taiwanese")},
  {71,LPGEN("Tamil")},
  {47,LPGEN("Tatar")},
  {48,LPGEN("Thai")},
  {49,LPGEN("Turkish")},
  {50,LPGEN("Ukrainian")},
  {51,LPGEN("Urdu")},
  {52,LPGEN("Vietnamese")},
  {67,LPGEN("Welsh")},
  {53,LPGEN("Yiddish")},
  {54,LPGEN("Yoruba")},
};

static char *days[7] =
{
	LPGEN("Sunday"), LPGEN("Monday"), LPGEN("Tuesday"), LPGEN("Wednesday"), LPGEN("Thursday"), LPGEN("Friday"), LPGEN("Saturday")
};

static char *months[12] =
{
	LPGEN("January"), LPGEN("February"), LPGEN("March"), LPGEN("April"), LPGEN("May"), LPGEN("June"),
	LPGEN("July"), LPGEN("August"), LPGEN("September"), LPGEN("October"), LPGEN("November"), LPGEN("December")
};

void AddTranslation(DBVTranslation *newTrans)
{
	DBVTranslation *ptranslations = (DBVTranslation *)mir_realloc(translations, sizeof(DBVTranslation) * (iTransFuncsCount + 1));
	if (ptranslations == nullptr)
		return;
	translations = ptranslations;
	iTransFuncsCount++;
	translations[iTransFuncsCount - 1] = *newTrans;

	char *szName = mir_u2a(newTrans->swzName);
	char szSetting[256];
	mir_snprintf(szSetting, sizeof(szSetting), "Trans_%s", szName);

	if (mir_wstrcmp(newTrans->swzName, L"[No translation]") == 0) {
		translations[iTransFuncsCount - 1].id = 0;
	}
	else {
		uint32_t id = db_get_dw(0, MODULE_ITEMS, szSetting, 0);
		if (id != 0) {
			translations[iTransFuncsCount - 1].id = id;
			if (dwNextFuncId <= id) dwNextFuncId = id + 1;
		}
		else {
			translations[iTransFuncsCount - 1].id = dwNextFuncId++;
			db_set_dw(0, MODULE_ITEMS, szSetting, translations[iTransFuncsCount - 1].id);
		}

		db_set_dw(0, MODULE_ITEMS, "NextFuncId", dwNextFuncId);
	}

	mir_free(szName);
}

wchar_t *NullTranslation(MCONTACT hContact, const char *szModuleName, const char *szSettingName, wchar_t *buff, int bufflen)
{
	if (DBGetContactSettingAsString(hContact, szModuleName, szSettingName, buff, bufflen))
		return buff;
	return nullptr;
}

wchar_t* TimestampToShortDate(MCONTACT hContact, const char *szModuleName, const char *szSettingName, wchar_t *buff, int bufflen)
{
	uint32_t ts = db_get_dw(hContact, szModuleName, szSettingName, 0);
	if (ts == 0)
		return nullptr;

	return TimeZone_ToStringT(ts, L"d", buff, bufflen);
}

wchar_t* TimestampToLongDate(MCONTACT hContact, const char *szModuleName, const char *szSettingName, wchar_t *buff, int bufflen)
{
	uint32_t ts = db_get_dw(hContact, szModuleName, szSettingName, 0);
	if (ts == 0)
		return nullptr;

	return TimeZone_ToStringT(ts, L"D", buff, bufflen);
}

wchar_t* TimestampToTime(MCONTACT hContact, const char *szModuleName, const char *szSettingName, wchar_t *buff, int bufflen)
{
	uint32_t ts = db_get_dw(hContact, szModuleName, szSettingName, 0);
	if (ts == 0)
		return nullptr;

	return TimeZone_ToStringT(ts, L"s", buff, bufflen);
}

wchar_t* TimestampToTimeNoSecs(MCONTACT hContact, const char *szModuleName, const char *szSettingName, wchar_t *buff, int bufflen)
{
	uint32_t ts = db_get_dw(hContact, szModuleName, szSettingName, 0);
	if (ts == 0)
		return nullptr;

	return TimeZone_ToStringT(ts, L"t", buff, bufflen);
}

wchar_t* TimestampToTimeDifference(MCONTACT hContact, const char *szModuleName, const char *szSettingName, wchar_t *buff, int bufflen)
{
	uint32_t ts = db_get_dw(hContact, szModuleName, szSettingName, 0);
	uint32_t t = (uint32_t)time(0);
	if (ts == 0) return nullptr;

	uint32_t diff = (ts > t) ? 0 : (t - ts);
	int d = (diff / 60 / 60 / 24);
	int h = (diff - d * 60 * 60 * 24) / 60 / 60;
	int m = (diff - d * 60 * 60 * 24 - h * 60 * 60) / 60;
	if (d > 0)
		mir_snwprintf(buff, bufflen, TranslateT("%dd %dh %dm"), d, h, m);
	else if (h > 0)
		mir_snwprintf(buff, bufflen, TranslateT("%dh %dm"), h, m);
	else
		mir_snwprintf(buff, bufflen, TranslateT("%dm"), m);

	return buff;
}

wchar_t *SecondsToTimeDifference(MCONTACT hContact, const char *szModuleName, const char *szSettingName, wchar_t *buff, int bufflen)
{
	uint32_t diff = db_get_dw(hContact, szModuleName, szSettingName, 0);
	int d = (diff / 60 / 60 / 24);
	int h = (diff - d * 60 * 60 * 24) / 60 / 60;
	int m = (diff - d * 60 * 60 * 24 - h * 60 * 60) / 60;
	if (d > 0)
		mir_snwprintf(buff, bufflen, TranslateT("%dd %dh %dm"), d, h, m);
	else if (h > 0)
		mir_snwprintf(buff, bufflen, TranslateT("%dh %dm"), h, m);
	else
		mir_snwprintf(buff, bufflen, TranslateT("%dm"), m);

	return buff;
}

wchar_t *WordToStatusDesc(MCONTACT hContact, const char *szModuleName, const char *szSettingName, wchar_t *buff, int bufflen)
{
	uint16_t wStatus = db_get_w(hContact, szModuleName, szSettingName, ID_STATUS_OFFLINE);
	wchar_t *szStatus = Clist_GetStatusModeDescription(wStatus, 0);
	wcsncpy_s(buff, bufflen, szStatus, _TRUNCATE);
	return buff;
}

wchar_t *ByteToYesNo(MCONTACT hContact, const char *szModuleName, const char *szSettingName, wchar_t *buff, int bufflen)
{
	DBVARIANT dbv;
	if (!db_get(hContact, szModuleName, szSettingName, &dbv)) {
		if (dbv.type == DBVT_BYTE) {
			if (dbv.bVal != 0)
				wcsncpy(buff, L"Yes", bufflen);
			else
				wcsncpy(buff, L"No", bufflen);
			buff[bufflen - 1] = 0;
			db_free(&dbv);
			return buff;
		}
		db_free(&dbv);
	}
	return nullptr;
}

wchar_t *ByteToGender(MCONTACT hContact, const char *szModuleName, const char *szSettingName, wchar_t *buff, int bufflen)
{
	uint8_t val = (uint8_t)db_get_b(hContact, szModuleName, szSettingName, 0);
	if (val == 'F')
		wcsncpy(buff, TranslateT("Female"), bufflen);
	else if (val == 'M')
		wcsncpy(buff, TranslateT("Male"), bufflen);
	else
		return nullptr;

	buff[bufflen - 1] = 0;
	return buff;
}

wchar_t *WordToCountry(MCONTACT hContact, const char *szModuleName, const char *szSettingName, wchar_t *buff, int bufflen)
{
	char *szCountryName = nullptr;
	uint16_t cid = (uint16_t)db_get_w(hContact, szModuleName, szSettingName, (uint16_t)-1);
	if (cid != (uint16_t)-1 && ServiceExists(MS_UTILS_GETCOUNTRYBYNUMBER) && (szCountryName = (char *)CallService(MS_UTILS_GETCOUNTRYBYNUMBER, cid, 0)) != nullptr) {
		if (mir_strcmp(szCountryName, "Unknown") == 0)
			return nullptr;
		a2t(szCountryName, buff, bufflen);
		buff[bufflen - 1] = 0;
		return buff;
	}
	return nullptr;
}

wchar_t *DwordToIp(MCONTACT hContact, const char *szModuleName, const char *szSettingName, wchar_t *buff, int bufflen)
{
	uint32_t ip = db_get_dw(hContact, szModuleName, szSettingName, 0);
	if (ip) {
		unsigned char *ipc = (unsigned char*)&ip;
		mir_snwprintf(buff, bufflen, L"%u.%u.%u.%u", ipc[3], ipc[2], ipc[1], ipc[0]);
		return buff;
	}
	return nullptr;
}

bool GetInt(const DBVARIANT &dbv, int *iVal)
{
	if (!iVal) return false;

	switch (dbv.type) {
	case DBVT_BYTE:
		if (iVal) *iVal = (int)dbv.bVal;
		return true;
	case DBVT_WORD:
		if (iVal) *iVal = (int)dbv.wVal;
		return true;
	case DBVT_DWORD:
		if (iVal) *iVal = (int)dbv.dVal;
		return true;
	}
	return false;
}

wchar_t *DayMonthYearToDate(MCONTACT hContact, const char *szModuleName, const char *prefix, wchar_t *buff, int bufflen)
{
	DBVARIANT dbv;
	char szSettingName[256];
	mir_snprintf(szSettingName, "%sDay", prefix);
	if (!db_get(hContact, szModuleName, szSettingName, &dbv)) {
		int day = 0;
		if (GetInt(dbv, &day)) {
			db_free(&dbv);
			mir_snprintf(szSettingName, "%sMonth", prefix);
			int month = 0;
			if (!db_get(hContact, szModuleName, szSettingName, &dbv)) {
				if (GetInt(dbv, &month)) {
					db_free(&dbv);
					mir_snprintf(szSettingName, "%sYear", prefix);
					int year = 0;
					db_get(hContact, szModuleName, szSettingName, &dbv);

					GetInt(dbv, &year);
					db_free(&dbv);

					tm time = { 0 };
					time.tm_mday = day;
					time.tm_mon = month - 1;
					time.tm_year = year - 1900;

					wcsftime(buff, bufflen, L"%x", &time);

					return buff;

				}
				else
					db_free(&dbv);
			}
		}
		else
			db_free(&dbv);
	}
	return nullptr;
}

wchar_t *DayMonthYearToAge(MCONTACT hContact, const char *szModuleName, const char *szPrefix, wchar_t *buff, int bufflen)
{
	DBVARIANT dbv;
	char szSettingName[256];
	mir_snprintf(szSettingName, "%sDay", szPrefix);
	if (!db_get(hContact, szModuleName, szSettingName, &dbv)) {
		int day = 0;
		if (GetInt(dbv, &day)) {
			db_free(&dbv);
			mir_snprintf(szSettingName, "%sMonth", szPrefix);
			int month = 0;
			if (!db_get(hContact, szModuleName, szSettingName, &dbv)) {
				if (GetInt(dbv, &month)) {
					db_free(&dbv);
					mir_snprintf(szSettingName, "%sYear", szPrefix);
					int year = 0;
					if (!db_get(hContact, szModuleName, szSettingName, &dbv)) {
						if (GetInt(dbv, &year) && year != 0) {
							db_free(&dbv);

							SYSTEMTIME now;
							GetLocalTime(&now);

							int age = now.wYear - year;
							if (now.wMonth < month || (now.wMonth == month && now.wDay < day))
								age--;
							mir_snwprintf(buff, bufflen, L"%d", age);
							return buff;
						}
						else
							db_free(&dbv);
					}
				}
				else
					db_free(&dbv);
			}
		}
		else
			db_free(&dbv);
	}
	return nullptr;
}

wchar_t *HoursMinutesSecondsToTime(MCONTACT hContact, const char *szModuleName, const char *szPrefix, wchar_t *buff, int bufflen)
{
	DBVARIANT dbv;
	char szSettingName[256];
	mir_snprintf(szSettingName, "%sHours", szPrefix);
	if (!db_get(hContact, szModuleName, szSettingName, &dbv)) {
		int hours = 0;
		if (GetInt(dbv, &hours)) {
			db_free(&dbv);
			mir_snprintf(szSettingName, "%sMinutes", szPrefix);
			int minutes = 0;
			if (!db_get(hContact, szModuleName, szSettingName, &dbv)) {
				if (GetInt(dbv, &minutes)) {
					db_free(&dbv);
					mir_snprintf(szSettingName, "%sSeconds", szPrefix);
					int seconds = 0;
					if (!db_get(hContact, szModuleName, szSettingName, &dbv)) {
						GetInt(dbv, &seconds);
						db_free(&dbv);
					}

					SYSTEMTIME st = { 0 };
					st.wHour = hours;
					st.wMinute = minutes;
					st.wSecond = seconds;

					GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, nullptr, buff, bufflen);
					return buff;
				}
				else
					db_free(&dbv);
			}
		}
		else
			db_free(&dbv);
	}
	return nullptr;
}

wchar_t *HoursMinutesToTime(MCONTACT hContact, const char *szModuleName, const char *szPrefix, wchar_t *buff, int bufflen)
{
	DBVARIANT dbv;
	char szSettingName[256];
	mir_snprintf(szSettingName, "%sHours", szPrefix);
	if (!db_get(hContact, szModuleName, szSettingName, &dbv)) {
		int hours = 0;
		if (GetInt(dbv, &hours)) {
			db_free(&dbv);
			mir_snprintf(szSettingName, "%sMinutes", szPrefix);
			int minutes = 0;
			if (!db_get(hContact, szModuleName, szSettingName, &dbv)) {
				if (GetInt(dbv, &minutes)) {
					db_free(&dbv);

					SYSTEMTIME st = { 0 };
					st.wHour = hours;
					st.wMinute = minutes;

					GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, nullptr, buff, bufflen);
					return buff;
				}
				else
					db_free(&dbv);
			}
		}
		else
			db_free(&dbv);
	}
	return nullptr;
}

wchar_t *DmyToTimeDifference(MCONTACT hContact, const char *szModuleName, const char *szPrefix, wchar_t *buff, int bufflen)
{
	DBVARIANT dbv;
	char szSettingName[256];
	mir_snprintf(szSettingName, "%sDay", szPrefix);
	if (!db_get(hContact, szModuleName, szSettingName, &dbv)) {
		int day = 0;
		if (GetInt(dbv, &day)) {
			db_free(&dbv);
			mir_snprintf(szSettingName, "%sMonth", szPrefix);
			int month = 0;
			if (!db_get(hContact, szModuleName, szSettingName, &dbv)) {
				if (GetInt(dbv, &month)) {
					db_free(&dbv);
					mir_snprintf(szSettingName, "%sYear", szPrefix);
					int year = 0;
					if (!db_get(hContact, szModuleName, szSettingName, &dbv)) {
						if (GetInt(dbv, &year)) {
							db_free(&dbv);
							mir_snprintf(szSettingName, "%sHours", szPrefix);
							if (!db_get(hContact, szModuleName, szSettingName, &dbv)) {
								int hours = 0;
								if (GetInt(dbv, &hours)) {
									db_free(&dbv);
									mir_snprintf(szSettingName, "%sMinutes", szPrefix);
									int minutes = 0;
									if (!db_get(hContact, szModuleName, szSettingName, &dbv)) {
										if (GetInt(dbv, &minutes)) {
											db_free(&dbv);
											mir_snprintf(szSettingName, "%sSeconds", szPrefix);
											int seconds = 0;
											if (!db_get(hContact, szModuleName, szSettingName, &dbv)) {
												GetInt(dbv, &seconds);
												db_free(&dbv);
											}

											SYSTEMTIME st = { 0 }, st_now;
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
											int m = (diff - y * 60 * 60 * 24 * 365 - d * 60 * 60 * 24 - h * 60 * 60) / 60;
											if (y != 0)
												mir_snwprintf(buff, bufflen, TranslateT("%dy %dd %dh %dm"), y, d, h, m);
											else if (d != 0)
												mir_snwprintf(buff, bufflen, TranslateT("%dd %dh %dm"), d, h, m);
											else if (h != 0)
												mir_snwprintf(buff, bufflen, TranslateT("%dh %dm"), h, m);
											else
												mir_snwprintf(buff, bufflen, TranslateT("%dm"), m);

											return buff;
										}
										else
											db_free(&dbv);
									}
								}
								else
									db_free(&dbv);
							}
						}
						else
							db_free(&dbv);
					}
				}
				else
					db_free(&dbv);
			}
		}
		else
			db_free(&dbv);
	}
	return nullptr;
}

wchar_t *DayMonthToDaysToNextBirthday(MCONTACT hContact, const char *szModuleName, const char *szPrefix, wchar_t *buff, int bufflen)
{
	DBVARIANT dbv;
	char szSettingName[256];
	mir_snprintf(szSettingName, "%sDay", szPrefix);
	if (!db_get(hContact, szModuleName, szSettingName, &dbv)) {
		int day = 0;
		if (GetInt(dbv, &day)) {
			db_free(&dbv);
			mir_snprintf(szSettingName, "%sMonth", szPrefix);
			int month = 0;
			if (!db_get(hContact, szModuleName, szSettingName, &dbv)) {
				if (GetInt(dbv, &month)) {
					db_free(&dbv);
					time_t now = time(0);
					struct tm *ti = localtime(&now);
					int yday_now = ti->tm_yday;

					ti->tm_mday = day;
					ti->tm_mon = month - 1;
					mktime(ti);

					int yday_birth = ti->tm_yday;
					if (yday_birth < yday_now) {
						yday_now -= 365;
						yday_now -= (ti->tm_year % 4) ? 0 : 1;
					}

					int diff = yday_birth - yday_now;
					mir_snwprintf(buff, bufflen, TranslateT("%dd"), diff);

					return buff;
				}
				else {
					db_free(&dbv);
				}
			}
		}
		else {
			db_free(&dbv);
		}
	}
	return nullptr;
}


wchar_t *EmptyXStatusToDefaultName(MCONTACT hContact, const char *szModuleName, const char *szSettingName, wchar_t *buff, int bufflen)
{
	wchar_t szDefaultName[1024];
	CUSTOM_STATUS xstatus = { 0 };
	DBVARIANT dbv;

	// translate jabber mood
	if (ProtoServiceExists(szModuleName, "/SendXML")) // jabber protocol?
	{
		if (!db_get_ws(hContact, szModuleName, szSettingName, &dbv)) {
			wcsncpy(buff, TranslateW(dbv.pwszVal), bufflen);
			buff[bufflen - 1] = 0;
			return buff;
		}
	}

	if (NullTranslation(hContact, szModuleName, szSettingName, buff, bufflen))
		return buff;

	int status = db_get_b(hContact, szModuleName, "XStatusId", 0);
	if (!status) return nullptr;

	if (ProtoServiceExists(szModuleName, PS_GETCUSTOMSTATUSEX)) {
		xstatus.cbSize = sizeof(CUSTOM_STATUS);
		xstatus.flags = CSSF_MASK_NAME | CSSF_DEFAULT_NAME | CSSF_UNICODE;
		xstatus.ptszName = szDefaultName;
		xstatus.wParam = (WPARAM *)&status;
		if (CallProtoService(szModuleName, PS_GETCUSTOMSTATUSEX, 0, (LPARAM)&xstatus))
			return nullptr;

		wcsncpy(buff, TranslateW(szDefaultName), bufflen);
		buff[bufflen - 1] = 0;
		return buff;
	}

	return nullptr;
}

wchar_t *TimezoneToTime(MCONTACT hContact, const char *szModuleName, const char *szSettingName, wchar_t *buff, int bufflen)
{
	int timezone = db_get_b(hContact, szModuleName, szSettingName, 256);
	if (timezone == 256 || (char)timezone == -100)
		return nullptr;

	TIME_ZONE_INFORMATION tzi;
	FILETIME ft;
	LARGE_INTEGER lift;
	SYSTEMTIME st;

	timezone = (char)timezone;
	GetSystemTimeAsFileTime(&ft);
	if (GetTimeZoneInformation(&tzi) == TIME_ZONE_ID_DAYLIGHT)
		timezone += tzi.DaylightBias / 30;

	lift.QuadPart = *(__int64*)&ft;
	lift.QuadPart -= (__int64)timezone * 30ll * 60ll * 10000000ll;
	*(__int64*)&ft = lift.QuadPart;
	FileTimeToSystemTime(&ft, &st);
	GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, nullptr, buff, bufflen);

	return buff;
}

wchar_t *ByteToDay(MCONTACT hContact, const char *szModuleName, const char *szSettingName, wchar_t *buff, int bufflen)
{
	int iDay = db_get_w(hContact, szModuleName, szSettingName, -1);
	if (iDay > -1 && iDay < 7) {
		a2t(Translate(days[iDay]), buff, bufflen);
		buff[bufflen - 1] = 0;
		return buff;
	}

	return nullptr;
}

wchar_t *ByteToMonth(MCONTACT hContact, const char *szModuleName, const char *szSettingName, wchar_t *buff, int bufflen)
{
	int iMonth = db_get_w(hContact, szModuleName, szSettingName, 0);
	if (iMonth > 0 && iMonth < 13) {
		a2t(Translate(months[iMonth - 1]), buff, bufflen);
		buff[bufflen - 1] = 0;
		return buff;
	}

	return nullptr;
}

wchar_t *ByteToLanguage(MCONTACT hContact, const char *szModuleName, const char *szSettingName, wchar_t *buff, int bufflen)
{
	int iLang = db_get_b(hContact, szModuleName, szSettingName, 0);
	if (iLang) {
		for (auto &it : languages) {
			if (iLang == it.id) {
				a2t(Translate(it.szValue), buff, bufflen);
				buff[bufflen - 1] = 0;
				return buff;
			}
		}
	}

	return nullptr;
}

INT_PTR ServiceAddTranslation(WPARAM, LPARAM lParam)
{
	if (!lParam) return 1;

	DBVTranslation *trans = (DBVTranslation *)lParam;
	AddTranslation(trans);

	return 0;
}

static DBVTranslation internalTranslations[] =
{
	{	NullTranslation,               LPGENW("[No translation]")                                                },
	{	WordToStatusDesc,              LPGENW("uint16_t to status description")                                      },
	{	TimestampToTime,               LPGENW("uint32_t timestamp to time")                                         },
	{	TimestampToTimeDifference,     LPGENW("uint32_t timestamp to time difference")                              },
	{	ByteToYesNo,                   LPGENW("uint8_t to Yes/No")                                                  },
	{	ByteToGender,                  LPGENW("uint8_t to Male/Female (ICQ)")                                       },
	{	WordToCountry,                 LPGENW("uint16_t to country name")                                            },
	{	DwordToIp,                     LPGENW("uint32_t to IP address")                                             },
	{	DayMonthYearToDate,            LPGENW("<prefix>Day|Month|Year to date")                                  },
	{  DayMonthYearToAge,             LPGENW("<prefix>Day|Month|Year to age")                                   },
	{	HoursMinutesSecondsToTime,     LPGENW("<prefix>Hours|Minutes|Seconds to time")                           },
	{	DmyToTimeDifference,           LPGENW("<prefix>Day|Month|Year|Hours|Minutes|Seconds to time difference") },
	{	DayMonthToDaysToNextBirthday,  LPGENW("<prefix>Day|Month to days to next birthday")                      },
	{	TimestampToTimeNoSecs,         LPGENW("uint32_t timestamp to time (no seconds)")                            },
	{	HoursMinutesToTime,            LPGENW("<prefix>Hours|Minutes to time")                                   },
	{	TimestampToShortDate,          LPGENW("uint32_t timestamp to date (short)")                                 },
	{	TimestampToLongDate,           LPGENW("uint32_t timestamp to date (long)")                                  },
	{	EmptyXStatusToDefaultName,     LPGENW("xStatus: empty xStatus name to default name")                     },
	{	SecondsToTimeDifference,       LPGENW("uint32_t seconds to time difference")                                },
	{	TimezoneToTime,                LPGENW("uint8_t timezone to time")                                           },
	{	ByteToDay,                     LPGENW("uint16_t to name of a day (0..6, 0 is Sunday)")                       },
	{	ByteToMonth,                   LPGENW("uint16_t to name of a month (1..12, 1 is January)")                   },
	{	ByteToLanguage,                LPGENW("uint8_t to language (ICQ)")                                          },
};

void InitTranslations()
{
	dwNextFuncId = db_get_dw(0, MODULE_ITEMS, "NextFuncId", 1);
	for (auto &it : internalTranslations)
		AddTranslation(&it);

	hServiceAdd = CreateServiceFunction(MS_TIPPER_ADDTRANSLATION, ServiceAddTranslation);
}

void DeinitTranslations()
{
	DestroyServiceFunction(hServiceAdd);
	mir_free(translations);
}
