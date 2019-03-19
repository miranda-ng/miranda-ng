#include "stdafx.h"

#define FILETIME_TICKS_PER_SEC ((ULONGLONG)10000000)

#define MAX_REMINDER_LEN	16384

// RemindersData DB data params
#define DATATAG_TEXT	      1 // %s
#define DATATAG_SNDREPEAT  2 // %u (specifies seconds to wait between sound repeats, 0 if repeat is disabled)
#define DATATAG_SNDSEL     3 // %d (which sound to use, default, alt1, alt2, -1 means no sound at all)
#define DATATAG_REPEAT     4 // %d (repeateable reminder)
#define DATATAG_ORIG       5 // %lld (start time)

#define IDM_NEWREMINDER 40001
#define IDM_DELETEREMINDER 40002
#define IDM_DELETEALLREMINDERS 40003

#define WM_RELOAD (WM_USER + 100)

#define NOTIFY_LIST() if (bListReminderVisible) PostMessage(LV,WM_RELOAD,0,0)

/////////////////////////////////////////////////////////////////////////////////////////

static void RemoveReminderSystemEvent(struct REMINDERDATA *p);

struct REMINDERDATA : public MZeroedObject
{
	HWND handle;
	DWORD uid;
	char* Reminder;
	ULARGE_INTEGER Orig, When;	// FILETIME in UTC
	UINT RepeatSound;
	UINT RepeatSoundTTL;
	int  SoundSel;			// -1 if sound disabled
	bool bVisible;
	bool bRepeat;
	bool bSystemEventQueued;

	REMINDERDATA()
	{
		SYSTEMTIME tm;
		GetSystemTime(&tm);
		SystemTimeToFileTime(&tm, (FILETIME*)&Orig);
	}

	~REMINDERDATA()
	{
		// remove pending system event
		if (bSystemEventQueued)
			RemoveReminderSystemEvent(this);
	}
};

static int ReminderSortCb(const REMINDERDATA *v1, const REMINDERDATA *v2)
{
	if (v1->When.QuadPart == v2->When.QuadPart)
		return 0;

	return (v1->When.QuadPart < v2->When.QuadPart) ? -1 : 1;
}

static LIST<REMINDERDATA> arReminders(1, ReminderSortCb);

static bool bNewReminderVisible = false;
static bool bListReminderVisible = false;
static UINT QueuedReminderCount = 0;
static HWND LV;

int WS_Send(SOCKET s, char *data, int datalen);
unsigned long WS_ResolveName(char *name, WORD *port, int defaultPort);

void Send(char *user, char *host, char *Msg, char* server);
wchar_t* GetPreviewString(const char *lpsz);

// time convertsion routines that take local time-zone specific daylight saving configuration into account
// (unlike the standard FileTimeToLocalFileTime functions)

void FileTimeToTzLocalST(const FILETIME *lpUtc, SYSTEMTIME *tmLocal)
{
	SYSTEMTIME tm;
	FileTimeToSystemTime(lpUtc, &tm);
	SystemTimeToTzSpecificLocalTime(nullptr, &tm, tmLocal);
}

void TzLocalSTToFileTime(const SYSTEMTIME *tmLocal, FILETIME *lpUtc)
{
	SYSTEMTIME tm;
	TzSpecificLocalTimeToSystemTime(nullptr, (SYSTEMTIME*)tmLocal, &tm);
	SystemTimeToFileTime(&tm, lpUtc);
}

static REMINDERDATA* FindReminder(DWORD uid)
{
	for (auto &pReminder : arReminders)
		if (pReminder->uid == uid)
			return pReminder;

	return nullptr;
}

static DWORD CreateUid()
{
	if (!arReminders.getCount())
		return 1;

	// check existing reminders if uid is in use
	for (DWORD uid = 1;; uid++) {
		if (!FindReminder(uid)) // uid in use
			return uid;
	}
}

static void RemoveReminderSystemEvent(REMINDERDATA *p)
{
	if (p->bSystemEventQueued) {
		for (int i = 0;; i++) {
			CLISTEVENT *pev = g_clistApi.pfnGetEvent(-1, i);
			if (!pev)
				break;

			if ((ULONG)pev->lParam == p->uid && !pev->hContact && pev->pszService && !mir_strcmp(pev->pszService, MODULENAME"/OpenTriggeredReminder")) {
				if (!g_clistApi.pfnRemoveEvent(pev->hContact, pev->hDbEvent)) {
					p->bSystemEventQueued = false;
					if (QueuedReminderCount)
						QueuedReminderCount--;
				}
				break;
			}
		}
	}
}

void PurgeReminders(void)
{
	char ValueName[32];

	int ReminderCount = g_plugin.getDword("RemindersData", 0);
	for (int i = 0; i < ReminderCount; i++) {
		mir_snprintf(ValueName, "RemindersData%d", i);
		g_plugin.delSetting(ValueName);
	}
}

void JustSaveReminders(void)
{
	int OldReminderCount = g_plugin.getDword("RemindersData", 0);
	g_plugin.setDword("RemindersData", arReminders.getCount());

	int i = 0;
	for (auto &pReminder : arReminders) {
		char *tmpReminder;
		if (mir_strlen(pReminder->Reminder))
			tmpReminder = pReminder->Reminder;
		else
			tmpReminder = "";

		CMStringA szValue;

		// data header (save 'When' with 1-second resolution, it's just a waste to have 100-nanosecond resolution
		// which results in larger DB strings with no use)
		szValue.AppendFormat("X%u:%I64x", pReminder->uid, pReminder->When.QuadPart / FILETIME_TICKS_PER_SEC);

		// orig
		if (pReminder->Orig.QuadPart)
			szValue.AppendFormat("\033""%u:%I64x", DATATAG_ORIG, pReminder->Orig.QuadPart / FILETIME_TICKS_PER_SEC);

		// repeat
		if (pReminder->bRepeat)
			szValue.AppendFormat("\033""%u:%u", DATATAG_REPEAT, (int)pReminder->bRepeat);

		// sound repeat
		if (pReminder->RepeatSound)
			szValue.AppendFormat("\033""%u:%u", DATATAG_SNDREPEAT, pReminder->RepeatSound);

		// sound
		if (pReminder->SoundSel)
			szValue.AppendFormat("\033""%u:%d", DATATAG_SNDSEL, pReminder->SoundSel);

		// reminder text/note (ALWAYS PUT THIS PARAM LAST)
		if (tmpReminder && *tmpReminder)
			szValue.AppendFormat("\033""%u:%s", DATATAG_TEXT, tmpReminder);

		char ValueName[32];
		mir_snprintf(ValueName, "RemindersData%d", i++);
		db_set_blob(0, MODULENAME, ValueName, szValue.GetBuffer(), szValue.GetLength() + 1);
	}

	// delete any left over DB reminder entries
	while (i < OldReminderCount) {
		char ValueName[32];
		mir_snprintf(ValueName, "RemindersData%d", i++);
		g_plugin.delSetting(ValueName);
	}
}

void LoadReminders(void)
{
	char *Value;
	WORD Size;
	char ValueName[32];
	BOOL GenerateUids = FALSE;

	arReminders.destroy();
	int RemindersCount = g_plugin.getDword("RemindersData", 0);

	for (int i = 0; i < RemindersCount; i++) {
		Size = 65535;
		Value = nullptr;
		mir_snprintf(ValueName, "RemindersData%d", i);

		ReadSettingBlob(0, MODULENAME, ValueName, &Size, (void**)&Value);

		// was the blob found
		if (Size && Value) {
			REMINDERDATA rem = {};
			char *TVal;
			REMINDERDATA *TempRem;
			char *DelPos = strchr(Value, 0x1B);

			// ensure that read data is null-terminated
			Value[(UINT)Size - 1] = 0;

			if (Value[0] == 'X') {
				// new eXtended/fleXible data format

				if (DelPos)
					*DelPos = 0;

				// uid:when
				TVal = strchr(Value + 1, ':');
				if (!TVal || (DelPos && TVal > DelPos))
					continue;

				*TVal++ = 0;
				rem.uid = strtoul(Value + 1, nullptr, 10);
				rem.When.QuadPart = _strtoui64(TVal, nullptr, 16) * FILETIME_TICKS_PER_SEC;

				// optional \033 separated params
				while (DelPos) {
					char *sep;
					UINT tag;

					TVal = DelPos + 1;
					// find param end and make sure it's null-terminated (if end of data then it's already null-terminated)
					DelPos = strchr(TVal, 0x1B);
					if (DelPos)
						*DelPos = 0;

					// tag:<data>
					sep = strchr(TVal, ':');
					if (!sep || (DelPos && sep > DelPos))
						goto skip;

					tag = strtoul(TVal, nullptr, 10);
					TVal = sep + 1;

					switch (tag) {
					case DATATAG_TEXT:
						rem.Reminder = _strdup(TVal);
						break;

					case DATATAG_SNDREPEAT:
						rem.RepeatSound = strtoul(TVal, nullptr, 10);
						break;

					case DATATAG_SNDSEL:
						rem.SoundSel = strtol(TVal, nullptr, 10);
						if (rem.SoundSel > 2) rem.SoundSel = 2;
						break;

					case DATATAG_REPEAT:
						rem.bRepeat = strtol(TVal, nullptr, 10) != 0;
						break;

					case DATATAG_ORIG:
						rem.Orig.QuadPart = _strtoui64(TVal, nullptr, 16) * FILETIME_TICKS_PER_SEC;
						break;
					}
				}

				if (rem.SoundSel < 0)
					rem.RepeatSound = 0;
				if (!rem.Reminder)
					rem.Reminder = _strdup("");
			}
			else {
				// old format (for DB backward compatibility)
				if (!DelPos)
					continue;

				DelPos[0] = 0;
				// convert time_t to (local) FILETIME

				time_t tt = (time_t)strtoul(Value, nullptr, 10);
				struct tm *stm = localtime(&tt);

				SYSTEMTIME tm;
				tm.wDayOfWeek = 0;
				tm.wSecond = 0;
				tm.wMilliseconds = 0;
				tm.wHour = stm->tm_hour;
				tm.wMinute = stm->tm_min;
				tm.wSecond = stm->tm_sec;
				tm.wYear = stm->tm_year + 1900;
				tm.wMonth = stm->tm_mon + 1;
				tm.wDay = stm->tm_mday;
				SystemTimeToFileTime(&tm, (FILETIME*)&rem.When);

				TVal = DelPos + 1;
				rem.Reminder = _strdup(TVal);
			}

			// queue uid generation if invalid uid is present
			if (!rem.uid)
				GenerateUids = true;

			TempRem = new REMINDERDATA(rem);
			arReminders.insert(TempRem);
skip:;
		}

		FreeSettingBlob(Size, Value);
	}

	// generate UIDs if there are any items with an invalid UID
	if (GenerateUids && arReminders.getCount()) {
		for (auto &pReminder : arReminders)
			if (!pReminder->uid)
				pReminder->uid = CreateUid();

		JustSaveReminders();
	}
}

static void DeleteReminder(REMINDERDATA *p)
{
	if (p) {
		arReminders.remove(p);
		delete p;
	}
}

static void PurgeReminderTree()
{
	for (auto &pt : arReminders)  // empty whole tree
		delete pt;

	arReminders.destroy();
}

void SaveReminders(void)
{
	JustSaveReminders();
	PurgeReminderTree();
}

void DeleteReminders(void)
{
	PurgeReminders();
	g_plugin.setDword("RemindersData", 0);
	PurgeReminderTree();
}

void GetTriggerTimeString(const ULARGE_INTEGER *When, wchar_t *s, size_t strSize, BOOL bUtc)
{
	SYSTEMTIME tm = {0};
	LCID lc = GetUserDefaultLCID();

	*s = 0;

	if (bUtc)
		FileTimeToTzLocalST((const FILETIME*)When, &tm);
	else
		FileTimeToSystemTime((FILETIME*)When, &tm);

	if (GetDateFormat(lc, DATE_LONGDATE, &tm, nullptr, s, (int)strSize)) {
		// append time
		size_t n = mir_wstrlen(s);
		s[n++] = ' ';
		s[n] = 0;

		if (!GetTimeFormat(lc, LOCALE_NOUSEROVERRIDE | TIME_NOSECONDS, &tm, nullptr, s + n, int(strSize - n)))
			mir_snwprintf(s + n, strSize - n, L"%02d:%02d", tm.wHour, tm.wMinute);
	}
	else mir_snwprintf(s, strSize, L"%d-%02d-%02d %02d:%02d", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute);
}

static void Skin_PlaySoundPoly(LPCSTR pszSoundName)
{
	if (!g_plugin.bUseMSI) {
		Skin_PlaySound(pszSoundName);
		return;
	}

	if (db_get_b(0, "SkinSoundsOff", pszSoundName, 0) == 0) {
		DBVARIANT dbv;
		if (db_get_s(0, "SkinSounds", pszSoundName, &dbv) == 0) {
			char szFull[MAX_PATH];

			PathToAbsolute(dbv.pszVal, szFull);

			// use MCI device which allows multiple sounds playing at once
			// NOTE: mciSendString does not like long paths names, must convert to short
			char szShort[MAX_PATH];
			char s[512];
			GetShortPathNameA(szFull, szShort, sizeof(szShort));
			mir_snprintf(s, "play \"%s\"", szShort);
			mciSendStringA(s, nullptr, 0, nullptr);

			db_free(&dbv);
		}
	}
}

static void UpdateReminderEvent(REMINDERDATA *pReminder, UINT nElapsedSeconds, DWORD *pHasPlayedSound)
{
	DWORD dwSoundMask;

	if (pReminder->RepeatSound) {
		if (nElapsedSeconds >= pReminder->RepeatSoundTTL) {
			pReminder->RepeatSoundTTL = pReminder->RepeatSound;

			dwSoundMask = 1 << pReminder->SoundSel;

			if (!(*pHasPlayedSound & dwSoundMask)) {
				switch (pReminder->SoundSel) {
				case 1: Skin_PlaySoundPoly("AlertReminder2"); break;
				case 2: Skin_PlaySoundPoly("AlertReminder3"); break;
				default:
					Skin_PlaySoundPoly("AlertReminder");
				}

				*pHasPlayedSound |= dwSoundMask;
			}
		}
		else pReminder->RepeatSoundTTL -= nElapsedSeconds;
	}
}

static void FireReminder(REMINDERDATA *pReminder, DWORD *pHasPlayedSound)
{
	if (pReminder->bSystemEventQueued)
		return;

	// add a system event
	CLISTEVENT ev = {};
	ev.hIcon = g_hReminderIcon;
	ev.flags = CLEF_URGENT;
	ev.lParam = (LPARAM)pReminder->uid;
	ev.pszService = MODULENAME"/OpenTriggeredReminder";
	ev.szTooltip.a = Translate("Reminder");
	g_clistApi.pfnAddEvent(&ev);

	pReminder->bSystemEventQueued = true;
	QueuedReminderCount++;

	if (pReminder->SoundSel < 0) // sound disabled
		return;

	DWORD dwSoundMask = 1 << pReminder->SoundSel;

	pReminder->RepeatSoundTTL = pReminder->RepeatSound;

	if (!(*pHasPlayedSound & dwSoundMask)) {
		switch (pReminder->SoundSel) {
		case 1: Skin_PlaySoundPoly("AlertReminder2"); break;
		case 2: Skin_PlaySoundPoly("AlertReminder3"); break;
		default:
			Skin_PlaySoundPoly("AlertReminder");
		}

		*pHasPlayedSound |= dwSoundMask;
	}
}


bool CheckRemindersAndStart(void)
{
	// returns TRUE if there are any triggered reminder with bSystemEventQueued, this will shorten the update interval
	// allowing sound repeats with shorter intervals

	if (!arReminders.getCount())
		return false;

	ULARGE_INTEGER curT;
	SYSTEMTIME tm;
	GetSystemTime(&tm);
	SystemTimeToFileTime(&tm, (FILETIME*)&curT);

	// NOTE: reminder list is sorted by trigger time, so we can early out on the first reminder > cur time
	// quick check for normal case with no reminder ready to be triggered and no queued triggered reminders
	// (happens 99.99999999999% of the time)
	if (curT.QuadPart < arReminders[0]->When.QuadPart && !QueuedReminderCount)
		return false;

	bool bResult = false;

	// var used to avoid playing multiple alarm sounds during a single update
	DWORD bHasPlayedSound = 0;

	// if there are queued (triggered) reminders then iterate through entire list, becaue of WM_TIMECHANGE events
	// and for example daylight saving changes it's possible for an already triggered event to end up with When>curT
	bool bHasQueuedReminders = (QueuedReminderCount != 0);

	// allthough count should always be correct, it's fool proof to just count them again in the loop below
	QueuedReminderCount = 0;

	for (auto &pReminder : arReminders) {
		if (!bHasQueuedReminders && pReminder->When.QuadPart > curT.QuadPart)
			break;

		if (!pReminder->bVisible) {
			if (pReminder->bSystemEventQueued) {
				UpdateReminderEvent(pReminder, REMINDER_UPDATE_INTERVAL_SHORT / 1000, &bHasPlayedSound);

				QueuedReminderCount++;
				bResult = true;
			}
			else if (pReminder->When.QuadPart <= curT.QuadPart) {
				if (!g_RemindSMS) {
					FireReminder(pReminder, &bHasPlayedSound);

					if (pReminder->bSystemEventQueued)
						bResult = true;
				}
				else {
					char *S2 = strchr(g_RemindSMS, '@');
					char *S1 = (char*)malloc(S2 - g_RemindSMS);

					strncpy(S1, g_RemindSMS, S2 - g_RemindSMS);
					S1[S2 - g_RemindSMS] = 0x0;
					S2++;
					Send(S1, S2, pReminder->Reminder ? pReminder->Reminder : "", NULL);
					SAFE_FREE((void**)&S1);
					DeleteReminder(pReminder);
					JustSaveReminders();
					NOTIFY_LIST();
				}
			}
		}
	}

	return bResult;
}

static LRESULT CALLBACK DatePickerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_CHAR:
	case WM_INITMENUPOPUP:
	case WM_PASTE:
		return TRUE;
	case WM_SYSKEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSCHAR:
		return FALSE;
	}

	return mir_callNextSubclass(hWnd, DatePickerWndProc, message, wParam, lParam);
}

static void InitDatePicker(HWND hwndDlg, UINT nIDDate)
{
	// subclass date picker to prevent user editing (should only use the dropdown calender to ensure valid dates)
	HWND hCtrl = GetDlgItem(hwndDlg, nIDDate);

	// tweak style of picker
	if (IsWinVerVistaPlus()) {
		DWORD dw = SendDlgItemMessage(hwndDlg, nIDDate, DTM_GETMCSTYLE, 0, 0);
		dw |= MCS_WEEKNUMBERS | MCS_NOSELCHANGEONNAV;
		SendDlgItemMessage(hwndDlg, nIDDate, DTM_SETMCSTYLE, 0, dw);
	}

	mir_subclassWindow(hCtrl, DatePickerWndProc);
}

static BOOL ParseTime(const wchar_t *s, int *hout, int *mout, BOOL bTimeOffset, BOOL bAllowOffsetOverride)
{
	// validate format: <WS><digit>[<digit>]<WS>[':'<WS><digit>[<digit>]]<WS>[p | P].*

	// if bTimeOffset is FALSE the user may still enter a time offset by using + as the first char, the
	// delta time will be returned in minutes (even > 60) and hout will always be -1

	// if bTimeOffset is TRUE time is always interpreted as an offset (ignores PM indicator and defaults to minutes)

	int h, m;
	BOOL bOffset = bTimeOffset;

	// read hour

	while (iswspace(*s)) s++;

	if (*s == '+') {
		if (!bTimeOffset) {
			if (!bAllowOffsetOverride)
				return FALSE;

			// treat value as an offset anyway
			bOffset = TRUE;
		}

		s++;
		while (iswspace(*s)) s++;
	}

	if (!iswdigit(*s))
		return FALSE;
	h = (int)(*s - '0');
	s++;

	if (!bOffset) {
		if (iswdigit(*s)) {
			h = h * 10 + (int)(*s - '0');
			s++;
		}

		if (iswdigit(*s))
			return FALSE;
	}
	else {
		// allow more than 2-digit numbers for offset
		while (iswdigit(*s)) {
			h = h * 10 + (int)(*s - '0');
			s++;
		}
	}

	// find : separator
	while (iswspace(*s))
		s++;

	if (*s == ':') {
		s++;

		// read minutes

		while (iswspace(*s)) s++;

		if (!isdigit(*s))
			return FALSE;
		m = (int)(*s - '0');
		s++;

		if (iswdigit(*s)) {
			m = m * 10 + (int)(*s - '0');
			s++;
		}
	}
	else {
		if (bOffset) {
			// no : separator found, interpret the entered number as minutes and allow > 60

			if (h < 0)
				return FALSE;

			if (bTimeOffset) {
				*hout = h / 60;
				*mout = h % 60;
			}
			else {
				*mout = h;
				*hout = -1;
			}

			return TRUE;
		}
		else {
			m = 0;
		}
	}

	// validate time
	if (bOffset) {
		if (h < 0)
			return FALSE;
		if (m < 0 || m > 59)
			return FALSE;
	}
	else {
		if (h == 24)
			h = 0;
		else if (h < 0 || h > 23)
			return FALSE;
		if (m < 0 || m > 59)
			return FALSE;
	}

	if (!bOffset) {
		// check for PM indicator (not strict, only checks for P char)

		while (iswspace(*s)) s++;

		if (*s == 'p' || *s == 'P') {
			if (h < 13)
				h += 12;
			else if (h == 12)
				h = 0;
		}
	}
	else if (!bTimeOffset) {
		// entered time is an offset

		*mout = h * 60 + m;
		*hout = -1;

		return TRUE;
	}

	*hout = h;
	*mout = m;

	return TRUE;
}

// returns TRUE if combo box list displays time offsets ("23:34 (5 Minutes)" etc.)
__inline static BOOL IsRelativeCombo(HWND hwndDlg, UINT nIDTime)
{
	return (((int)SendDlgItemMessage(hwndDlg, nIDTime, CB_GETITEMDATA, 0, 0)) >= 0);
}

static void PopulateTimeCombo(HWND hwndDlg, UINT nIDTime, BOOL bRelative, const SYSTEMTIME *tmUtc)
{
	// NOTE: may seem like a bit excessive time converstion and handling, but this is done in order
	//       to gracefully handle crossing daylight saving boundaries

	SendDlgItemMessage(hwndDlg, nIDTime, CB_RESETCONTENT, 0, 0);

	SYSTEMTIME tm2;
	ULARGE_INTEGER li;
	wchar_t s[64];
	const ULONGLONG MinutesToFileTime = (ULONGLONG)60 * FILETIME_TICKS_PER_SEC;

	if (!bRelative) {

		// ensure that we start on midnight local time
		SystemTimeToTzSpecificLocalTime(nullptr, (SYSTEMTIME*)tmUtc, &tm2);
		tm2.wHour = 0;
		tm2.wMinute = 0;
		tm2.wSecond = 0;
		tm2.wMilliseconds = 0;
		TzSpecificLocalTimeToSystemTime(nullptr, &tm2, &tm2);
		SystemTimeToFileTime(&tm2, (FILETIME*)&li);

		// from 00:00 to 23:30 in 30 minute steps
		for (int i = 0; i < 50; i++) {
			const int h = i >> 1;
			const int m = (i & 1) ? 30 : 0;

			FileTimeToTzLocalST((FILETIME*)&li, &tm2);
			mir_snwprintf(s, L"%02d:%02d", (UINT)tm2.wHour, (UINT)tm2.wMinute);
			int n = SendDlgItemMessage(hwndDlg, nIDTime, CB_ADDSTRING, 0, (LPARAM)s);
			// item data contains time offset from midnight in seconds (bit 31 is set to flag that
			// combo box items are absolute times and not relative times like below
			SendDlgItemMessage(hwndDlg, nIDTime, CB_SETITEMDATA, n, (LPARAM)((ULONG)((h * 60 + m) * 60) | 0x80000000));

			li.QuadPart += (ULONGLONG)30 * MinutesToFileTime;

			if (tm2.wHour == 23 && tm2.wMinute >= 30)
				break;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////

	SystemTimeToFileTime(tmUtc, (FILETIME*)&li);
	ULONGLONG ref = li.QuadPart;

	// NOTE: item data contains offset from reference time (tmUtc) in seconds

	// cur time
	FileTimeToTzLocalST((FILETIME*)&li, &tm2);
	WORD wCurHour = tm2.wHour;
	WORD wCurMinute = tm2.wMinute;
	mir_snwprintf(s, L"%02d:%02d", (UINT)tm2.wHour, (UINT)tm2.wMinute);
	int n = SendDlgItemMessage(hwndDlg, nIDTime, CB_ADDSTRING, 0, (LPARAM)s);
	SendDlgItemMessage(hwndDlg, nIDTime, CB_SETITEMDATA, n, (LPARAM)((li.QuadPart - ref) / FILETIME_TICKS_PER_SEC));

	// 5 minutes
	li.QuadPart += (ULONGLONG)5 * MinutesToFileTime;
	FileTimeToTzLocalST((FILETIME*)&li, &tm2);
	mir_snwprintf(s, L"%02d:%02d (5 %s)", (UINT)tm2.wHour, (UINT)tm2.wMinute, TranslateT("Minutes"));
	n = SendDlgItemMessage(hwndDlg, nIDTime, CB_ADDSTRING, 0, (LPARAM)s);
	SendDlgItemMessage(hwndDlg, nIDTime, CB_SETITEMDATA, n, (LPARAM)((li.QuadPart - ref) / FILETIME_TICKS_PER_SEC));

	// 10 minutes
	li.QuadPart += (ULONGLONG)5 * MinutesToFileTime;
	FileTimeToTzLocalST((FILETIME*)&li, &tm2);
	mir_snwprintf(s, L"%02d:%02d (10 %s)", (UINT)tm2.wHour, (UINT)tm2.wMinute, TranslateT("Minutes"));
	n = SendDlgItemMessage(hwndDlg, nIDTime, CB_ADDSTRING, 0, (LPARAM)s);
	SendDlgItemMessage(hwndDlg, nIDTime, CB_SETITEMDATA, n, (LPARAM)((li.QuadPart - ref) / FILETIME_TICKS_PER_SEC));

	// 15 minutes
	li.QuadPart += (ULONGLONG)5 * MinutesToFileTime;
	FileTimeToTzLocalST((FILETIME*)&li, &tm2);
	mir_snwprintf(s, L"%02d:%02d (15 %s)", (UINT)tm2.wHour, (UINT)tm2.wMinute, TranslateT("Minutes"));
	n = SendDlgItemMessage(hwndDlg, nIDTime, CB_ADDSTRING, 0, (LPARAM)s);
	SendDlgItemMessage(hwndDlg, nIDTime, CB_SETITEMDATA, n, (LPARAM)((li.QuadPart - ref) / FILETIME_TICKS_PER_SEC));

	// 30 minutes
	li.QuadPart += (ULONGLONG)15 * MinutesToFileTime;
	FileTimeToTzLocalST((FILETIME*)&li, &tm2);
	mir_snwprintf(s, L"%02d:%02d (30 %s)", (UINT)tm2.wHour, (UINT)tm2.wMinute, TranslateT("Minutes"));
	n = SendDlgItemMessage(hwndDlg, nIDTime, CB_ADDSTRING, 0, (LPARAM)s);
	SendDlgItemMessage(hwndDlg, nIDTime, CB_SETITEMDATA, n, (LPARAM)((li.QuadPart - ref) / FILETIME_TICKS_PER_SEC));

	// round +1h time to nearest even or half hour
	li.QuadPart += (ULONGLONG)30 * MinutesToFileTime;
	li.QuadPart = (li.QuadPart / (30 * MinutesToFileTime)) * (30 * MinutesToFileTime);

	// add from +1 to +23.5 (in half hour steps) if crossing daylight saving boundary it may be 22.5 or 24.5 hours
	for (int i = 0; i < 50; i++) {
		UINT dt;

		FileTimeToTzLocalST((FILETIME*)&li, &tm2);

		if (i > 40) {
			UINT nLastEntry = ((UINT)wCurHour * 60 + (UINT)wCurMinute) / 30;
			if (nLastEntry)
				nLastEntry *= 30;
			else
				nLastEntry = 23 * 60 + 30;

			if (((UINT)tm2.wHour * 60 + (UINT)tm2.wMinute) == nLastEntry)
				break;
		}

		// icq-style display 1.0, 1.5 etc. hours even though that isn't accurate due to rounding
		//mir_snwprintf(s, L"%02d:%02d (%d.%d %s)", (UINT)tm2.wHour, (UINT)tm2.wMinute, 1+(i>>1), (i&1) ? 5 : 0, lpszHours);
		// display delta time more accurately to match reformatting (that icq doesn't do)
		dt = (UINT)((li.QuadPart / MinutesToFileTime) - (ref / MinutesToFileTime));
		if (dt < 60)
			mir_snwprintf(s, L"%02d:%02d (%d %s)", (UINT)tm2.wHour, (UINT)tm2.wMinute, dt, TranslateT("Minutes"));
		else
			mir_snwprintf(s, L"%02d:%02d (%d.%d %s)", (UINT)tm2.wHour, (UINT)tm2.wMinute, dt / 60, ((dt % 60) * 10) / 60, TranslateT("Hours"));
		n = SendDlgItemMessage(hwndDlg, nIDTime, CB_ADDSTRING, 0, (LPARAM)s);
		SendDlgItemMessage(hwndDlg, nIDTime, CB_SETITEMDATA, n, dt * 60);

		li.QuadPart += (ULONGLONG)30 * MinutesToFileTime;
	}
}

static void PopulateTimeOffsetCombo(HWND hwndDlg, UINT nIDCombo)
{
	SendDlgItemMessage(hwndDlg, nIDCombo, CB_RESETCONTENT, 0, 0);

	// 5 - 55 minutes (in 5 minute steps)
	wchar_t s[MAX_PATH];
	for (int i = 1; i < 12; i++) {
		mir_snwprintf(s, L"%d %s", i * 5, TranslateT("Minutes"));
		int n = SendDlgItemMessage(hwndDlg, nIDCombo, CB_ADDSTRING, 0, (LPARAM)s);
		SendDlgItemMessage(hwndDlg, nIDCombo, CB_SETITEMDATA, n, i * 5);
	}

	// 1 hour
	mir_snwprintf(s, L"1 %s", TranslateT("Hour"));
	int n = SendDlgItemMessage(hwndDlg, nIDCombo, CB_ADDSTRING, 0, (LPARAM)s);
	SendDlgItemMessage(hwndDlg, nIDCombo, CB_SETITEMDATA, n, 60);

	// 2, 4, 8 hours
	for (int i = 2; i <= 8; i += 2) {
		mir_snwprintf(s, L"%d %s", i, TranslateT("Hours"));
		n = SendDlgItemMessage(hwndDlg, nIDCombo, CB_ADDSTRING, 0, (LPARAM)s);
		SendDlgItemMessage(hwndDlg, nIDCombo, CB_SETITEMDATA, n, i * 60);
	}

	// 1 day
	mir_snwprintf(s, L"1 %s", TranslateT("Day"));
	n = SendDlgItemMessage(hwndDlg, nIDCombo, CB_ADDSTRING, 0, (LPARAM)s);
	SendDlgItemMessage(hwndDlg, nIDCombo, CB_SETITEMDATA, n, 24 * 60);

	// 2-4 days
	for (int i = 2; i <= 4; i++) {
		mir_snwprintf(s, L"%d %s", i, TranslateT("Days"));
		n = SendDlgItemMessage(hwndDlg, nIDCombo, CB_ADDSTRING, 0, (LPARAM)s);
		SendDlgItemMessage(hwndDlg, nIDCombo, CB_SETITEMDATA, n, i * 24 * 60);
	}

	// 1 week
	mir_snwprintf(s, L"1 %s", TranslateT("Week"));
	n = SendDlgItemMessage(hwndDlg, nIDCombo, CB_ADDSTRING, 0, (LPARAM)s);
	SendDlgItemMessage(hwndDlg, nIDCombo, CB_SETITEMDATA, n, 7 * 24 * 60);
}

// returns non-zero if specified time was inside "missing" hour of daylight saving
// IMPORTANT: triggerRelUtcOut is only initialized if IsRelativeCombo() is TRUE and return value is 0
static int ReformatTimeInput(HWND hwndDlg, UINT nIDTime, UINT nIDRefTime, int h, int m, const SYSTEMTIME *pDateLocal, ULARGE_INTEGER *triggerRelUtcOut = nullptr)
{
	int n;
	UINT dt;
	const ULONGLONG MinutesToFileTime = (ULONGLONG)60 * FILETIME_TICKS_PER_SEC;

	if (h < 0) {
		// time value is an offset ('m' holds the offset in minutes)

		if (IsRelativeCombo(hwndDlg, nIDTime)) {
			// get reference time (UTC) from hidden control
			wchar_t buf[64];
			GetDlgItemText(hwndDlg, nIDRefTime, buf, 30);

			ULONGLONG ref;
			ULARGE_INTEGER li;
			li.QuadPart = ref = _wcstoui64(buf, nullptr, 16);

			// clamp delta time to 23.5 hours (coule be issues otherwise as relative combo only handles <24)
			if (m > (23 * 60 + 30))
				m = 23 * 60 + 30;

			li.QuadPart += (ULONGLONG)(m * 60) * FILETIME_TICKS_PER_SEC;

			SYSTEMTIME tm;
			FileTimeToTzLocalST((FILETIME*)&li, &tm);
			h = (int)tm.wHour;
			m = (int)tm.wMinute;

			if (triggerRelUtcOut)
				*triggerRelUtcOut = li;

			dt = (UINT)((li.QuadPart / MinutesToFileTime) - (ref / MinutesToFileTime));

			if (dt < 60)
				mir_snwprintf(buf, L"%02d:%02d (%d %s)", h, m, dt, TranslateT("Minutes"));
			else
				mir_snwprintf(buf, L"%02d:%02d (%d.%d %s)", h, m, dt / 60, ((dt % 60) * 10) / 60, TranslateT("Hours"));

			// search for preset
			n = SendDlgItemMessage(hwndDlg, nIDTime, CB_FINDSTRING, (WPARAM)-1, (LPARAM)buf);
			if (n != CB_ERR) {
				SendDlgItemMessage(hwndDlg, nIDTime, CB_SETCURSEL, n, 0);
				return 0;
			}

			SetDlgItemText(hwndDlg, nIDTime, buf);
		}
		else {
			// should never happen
			SendDlgItemMessage(hwndDlg, nIDTime, CB_SETCURSEL, 0, 0);
		}

		return 0;
	}

	wchar_t buf[64];
	mir_snwprintf(buf, L"%02d:%02d", h, m);

	// search for preset first
	n = SendDlgItemMessage(hwndDlg, nIDTime, CB_FINDSTRING, (WPARAM)-1, (LPARAM)buf);
	if (n != CB_ERR) {
		SendDlgItemMessage(hwndDlg, nIDTime, CB_SETCURSEL, n, 0);
		return 0;
	}

	if (IsRelativeCombo(hwndDlg, nIDTime)) {
		// date format is a time offset ("24:43 (5 Minutes)" etc.)

		SYSTEMTIME tmTriggerLocal, tmTriggerLocal2;

		// get reference time (UTC) from hidden control
		GetDlgItemText(hwndDlg, nIDRefTime, buf, 30);
		ULONGLONG ref = _wcstoui64(buf, nullptr, 16);

		SYSTEMTIME tmRefLocal;
		FileTimeToTzLocalST((FILETIME*)&ref, &tmRefLocal);

		ULARGE_INTEGER li;
		const UINT nRefT = (UINT)tmRefLocal.wHour * 60 + (UINT)tmRefLocal.wMinute;
		const UINT nT = h * 60 + m;

		tmTriggerLocal = tmRefLocal;
		tmTriggerLocal.wHour = (WORD)h;
		tmTriggerLocal.wMinute = (WORD)m;
		tmTriggerLocal.wSecond = 0;
		tmTriggerLocal.wMilliseconds = 0;

		if (nT < nRefT) {
			// (this special case only works correctly if time can be returned in triggerRelUtcOut)
			if (tmRefLocal.wHour == tmTriggerLocal.wHour && triggerRelUtcOut) {
				// check for special case if daylight saving ends in this hour, then interpret as within the next hour
				TzLocalSTToFileTime(&tmTriggerLocal, (FILETIME*)&li);
				li.QuadPart += (ULONGLONG)3600 * FILETIME_TICKS_PER_SEC;
				FileTimeToTzLocalST((FILETIME*)&li, &tmTriggerLocal2);
				if ((tmTriggerLocal2.wHour * 60 + tmTriggerLocal2.wMinute) == (tmTriggerLocal.wHour * 60 + tmTriggerLocal.wMinute))
					// special case detected
					goto output_result;
			}

			// tomorrow (add 24h to local time)
			SystemTimeToFileTime(&tmTriggerLocal, (FILETIME*)&li);
			li.QuadPart += (ULONGLONG)(24 * 3600)*FILETIME_TICKS_PER_SEC;
			FileTimeToSystemTime((FILETIME*)&li, &tmTriggerLocal);
		}

		// clean up value for potential daylight saving boundary
		TzLocalSTToFileTime(&tmTriggerLocal, (FILETIME*)&li);
		FileTimeToTzLocalST((FILETIME*)&li, &tmTriggerLocal2);

		// NOTE: win32 time functions will round hour downward if supplied hour does not exist due to daylight saving
		//       for example if supplied time is 02:30 on the night the clock is turned forward at 02:00 to 03:00, thus
		//       there never is a 02:30, the time functions will convert it to 01:30 (and not 03:30 as one might think)
		//       (02:00 would return 01:00)

		// check for special case when the current time and requested time is inside the "missing" hour
		// standard->daylight switch, so that the cleaned up time ends up being earlier than the current
		// time even though it originally wasn't (see note above)
		if ((tmTriggerLocal2.wHour * 60 + tmTriggerLocal2.wMinute) < (tmTriggerLocal.wHour * 60 + tmTriggerLocal.wMinute)) {
			// special case detected, fall back to current time so at least the reminder won't be missed
			// due to ending up at an undesired time (this way the user immediately notices something was wrong)
			SendDlgItemMessage(hwndDlg, nIDTime, CB_SETCURSEL, 0, 0);
invalid_dst:
			MessageBox(hwndDlg, TranslateT("The specified time is invalid due to begin of daylight saving (summer time)."), _A2W(SECTIONNAME), MB_OK | MB_ICONWARNING);
			return 1;
		}

output_result:
		if (triggerRelUtcOut)
			*triggerRelUtcOut = li;

		dt = (UINT)((li.QuadPart / MinutesToFileTime) - (ref / MinutesToFileTime));

		if (dt < 60)
			mir_snwprintf(buf, L"%02d:%02d (%d %s)", h, m, dt, TranslateT("Minutes"));
		else
			mir_snwprintf(buf, L"%02d:%02d (%d.%d %s)", h, m, dt / 60, ((dt % 60) * 10) / 60, TranslateT("Hours"));
	}
	else {
		// absolute time (00:00 to 23:59), clean up time to make sure it's not inside "missing" hour (will be rounded downard)
		SYSTEMTIME Date = *pDateLocal;
		Date.wHour = h;
		Date.wMinute = m;
		Date.wSecond = 0;
		Date.wMilliseconds = 0;

		FILETIME ft;
		TzLocalSTToFileTime(&Date, &ft);
		FileTimeToTzLocalST(&ft, &Date);

		if ((int)Date.wHour != h || (int)Date.wMinute != m) {
			mir_snwprintf(buf, L"%02d:%02d", Date.wHour, Date.wMinute);

			// search for preset again
			n = SendDlgItemMessage(hwndDlg, nIDTime, CB_FINDSTRING, -1, (LPARAM)buf);
			if (n != CB_ERR) {
				SendDlgItemMessage(hwndDlg, nIDTime, CB_SETCURSEL, n, 0);
				goto invalid_dst;
			}

			SetDlgItemText(hwndDlg, nIDTime, buf);
			goto invalid_dst;
		}
	}

	SetDlgItemText(hwndDlg, nIDTime, buf);
	return 0;
}

// in:  pDate contains the desired trigger date in LOCAL time
// out: pDate contains the resulting trigger time and date in UTC
static bool GetTriggerTime(HWND hwndDlg, UINT nIDTime, UINT nIDRefTime, SYSTEMTIME *pDate)
{
	// get reference (UTC) time from hidden control
	wchar_t buf[32];
	GetDlgItemText(hwndDlg, nIDRefTime, buf, 30);
	ULARGE_INTEGER li;
	li.QuadPart = _wcstoui64(buf, nullptr, 16);

	int n = SendDlgItemMessage(hwndDlg, nIDTime, CB_GETCURSEL, 0, 0);
	if (n != CB_ERR) {
		// use preset value
preset_value:;
		if (IsRelativeCombo(hwndDlg, nIDTime)) {
			// time offset from ref time ("24:43 (5 Minutes)" etc.)

			UINT nDeltaSeconds = (UINT)SendDlgItemMessage(hwndDlg, nIDTime, CB_GETITEMDATA, n, 0);
			li.QuadPart += (ULONGLONG)nDeltaSeconds * FILETIME_TICKS_PER_SEC;

			FileTimeToSystemTime((FILETIME*)&li, pDate);

			// if specified time is a small offset (< 10 Minutes) then retain current second count for better accuracy
			// otherwise try to match specified time (which never contains seconds only even minutes) as close as possible
			if (nDeltaSeconds >= 10 * 60) {
				pDate->wSecond = 0;
				pDate->wMilliseconds = 0;
			}
		}
		else {
			// absolute time (offset from midnight on pDate)

			UINT nDeltaSeconds = (UINT)((ULONG)SendDlgItemMessage(hwndDlg, nIDTime, CB_GETITEMDATA, n, 0) & ~0x80000000);
			pDate->wHour = 0;
			pDate->wMinute = 0;
			pDate->wSecond = 0;
			pDate->wMilliseconds = 0;
			TzLocalSTToFileTime(pDate, (FILETIME*)&li);
			li.QuadPart += (ULONGLONG)nDeltaSeconds * FILETIME_TICKS_PER_SEC;

			FileTimeToSystemTime((FILETIME*)&li, pDate);
		}
		return true;
	}

	// user entered a custom value
	GetDlgItemText(hwndDlg, nIDTime, buf, 30);

	int h, m;
	if (!ParseTime(buf, &h, &m, FALSE, IsRelativeCombo(hwndDlg, nIDTime))) {
		MessageBox(hwndDlg, TranslateT("The specified time is invalid."), _A2W(SECTIONNAME), MB_OK | MB_ICONWARNING);
		return false;
	}

	if (IsRelativeCombo(hwndDlg, nIDTime)) {
		// date has not been changed, the specified time is a time between reftime and reftime+24h
		ULARGE_INTEGER li2;
		if (ReformatTimeInput(hwndDlg, nIDTime, nIDRefTime, h, m, pDate, &li2))
			return FALSE;

		// check if reformatted value is a preset
		if ((n = SendDlgItemMessage(hwndDlg, nIDTime, CB_GETCURSEL, 0, 0)) != CB_ERR)
			goto preset_value;

		FileTimeToSystemTime((FILETIME*)&li2, pDate);
		return true;
	}

	if (ReformatTimeInput(hwndDlg, nIDTime, nIDRefTime, h, m, pDate, nullptr))
		return false;

	// check if reformatted value is a preset
	if ((n = SendDlgItemMessage(hwndDlg, nIDTime, CB_GETCURSEL, 0, 0)) != CB_ERR)
		goto preset_value;

	// absolute time (on pDate)
	pDate->wHour = h;
	pDate->wMinute = m;
	pDate->wSecond = 0;
	pDate->wMilliseconds = 0;

	TzLocalSTToFileTime(pDate, (FILETIME*)&li);
	FileTimeToSystemTime((FILETIME*)&li, pDate);
	return true;
}

static void OnDateChanged(HWND hwndDlg, UINT nDateID, UINT nTimeID, UINT nRefTimeID)
{
	// repopulate time combo list with regular times (not offsets like "23:32 (5 minutes)" etc.)
	wchar_t s[32];
	GetDlgItemText(hwndDlg, nTimeID, s, _countof(s));

	int h = -1, m;
	ParseTime(s, &h, &m, FALSE, FALSE);

	SYSTEMTIME Date, DateUtc;
	SendDlgItemMessage(hwndDlg, nDateID, DTM_GETSYSTEMTIME, 0, (LPARAM)&Date);

	TzSpecificLocalTimeToSystemTime(nullptr, &Date, &DateUtc);
	PopulateTimeCombo(hwndDlg, nTimeID, FALSE, &DateUtc);

	if (h < 0) {
		// parsing failed, default to current time
		SYSTEMTIME tm;
		GetLocalTime(&tm);
		h = (UINT)tm.wHour;
		m = (UINT)tm.wMinute;
	}

	ReformatTimeInput(hwndDlg, nTimeID, nRefTimeID, h, m, &Date);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Reminder notification dialog

class CReminderNotifyDlg : public CDlgBase
{
	REMINDERDATA *m_pReminder;

	CCtrlDate dateAgain;
	CCtrlCheck chkAfter, chkOnDate;
	CCtrlCombo cmbTimeAgain, cmbRemindAgainIn;
	CCtrlButton btnDismiss, btnNone, btnRemindAgain;

public:
	CReminderNotifyDlg(REMINDERDATA *pReminder) :
		CDlgBase(g_plugin, IDD_NOTIFYREMINDER),
		m_pReminder(pReminder),
		btnNone(this, IDC_NONE),
		btnDismiss(this, IDC_DISMISS),
		btnRemindAgain(this, IDC_REMINDAGAIN),
		chkAfter(this, IDC_AFTER),
		chkOnDate(this, IDC_ONDATE),
		dateAgain(this, IDC_DATEAGAIN),
		cmbTimeAgain(this, IDC_TIMEAGAIN),
		cmbRemindAgainIn(this, IDC_REMINDAGAININ)
	{
		chkAfter.OnChange = Callback(this, &CReminderNotifyDlg::onChange_After);
		chkOnDate.OnChange = Callback(this, &CReminderNotifyDlg::onChange_OnDate);

		cmbTimeAgain.OnKillFocus = Callback(this, &CReminderNotifyDlg::onKillFocus_TimeAgain);
		cmbRemindAgainIn.OnKillFocus = Callback(this, &CReminderNotifyDlg::onKillFocus_RemindAgain);

		btnNone.OnClick = Callback(this, &CReminderNotifyDlg::onClick_None);
		btnDismiss.OnClick = Callback(this, &CReminderNotifyDlg::onClick_Dismiss);
		btnRemindAgain.OnClick = Callback(this, &CReminderNotifyDlg::onClick_RemindAgain);
	}

	bool OnInitDialog() override
	{
		m_pReminder->handle = m_hwnd;
		m_pReminder->bVisible = true;

		SYSTEMTIME tm;
		ULARGE_INTEGER li = m_pReminder->Orig;
		FileTimeToSystemTime((FILETIME*)&li, &tm);

		// save reference time in hidden control, is needed when adding reminder to properly detect if speicifed
		// time wrapped around to tomorrow or not (dialog could in theory be open for a longer period of time
		// which could potentially mess up things otherwise)
		wchar_t s[32];
		mir_snwprintf(s, L"%I64x", li.QuadPart);
		SetDlgItemText(m_hwnd, IDC_REFTIME, s);

		BringWindowToTop(m_hwnd);

		PopulateTimeOffsetCombo(m_hwnd, IDC_REMINDAGAININ);

		cmbRemindAgainIn.Show();
		dateAgain.Hide();
		cmbTimeAgain.Hide();
		ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_DATE), SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_TIME), SW_HIDE);
		chkAfter.SetState(true);
		chkOnDate.SetState(false);
		cmbRemindAgainIn.SetCurSel(0);

		if (m_pReminder->bRepeat) {
			chkOnDate.Hide();
			chkAfter.Disable();
			cmbRemindAgainIn.Disable();
			cmbRemindAgainIn.SetCurSel(16);
		}

		SendDlgItemMessage(m_hwnd, IDC_REMDATA, EM_LIMITTEXT, MAX_REMINDER_LEN, 0);

		PopulateTimeCombo(m_hwnd, IDC_TIMEAGAIN, TRUE, &tm);

		FileTimeToTzLocalST((FILETIME*)&li, &tm);

		// make sure date picker uses reference time
		dateAgain.SetTime(&tm);
		InitDatePicker(m_hwnd, IDC_DATEAGAIN);

		cmbTimeAgain.SetCurSel(0);

		wchar_t S1[128], S2[MAX_PATH];
		GetTriggerTimeString(&m_pReminder->When, S1, sizeof(S1), TRUE);
		mir_snwprintf(S2, L"%s! - %s", TranslateT("Reminder"), S1);
		SetCaption(S2);

		if (m_pReminder->Reminder)
			SetDlgItemTextA(m_hwnd, IDC_REMDATA, m_pReminder->Reminder);
		return true;
	}

	bool OnClose() override
	{
		if (m_pReminder)
			DeleteReminder(m_pReminder);
		JustSaveReminders();
		NOTIFY_LIST();
		return true;
	}

	void onChange_After(CCtrlCheck*)
	{
		cmbRemindAgainIn.Show();
		dateAgain.Hide();
		cmbTimeAgain.Hide();
		ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_DATE), SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_TIME), SW_HIDE);
	}

	void onChange_OnDate(CCtrlCheck*)
	{
		dateAgain.Show();
		cmbTimeAgain.Show();
		ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_DATE), SW_SHOW);
		ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_TIME), SW_SHOW);
		cmbRemindAgainIn.Hide();
	}

	void onChange_Date(CCtrlDate*)
	{
		OnDateChanged(m_hwnd, IDC_DATEAGAIN, IDC_TIMEAGAIN, IDC_REFTIME);
	}

	void onKillFocus_TimeAgain(CCtrlCombo*)
	{
		// reformat displayed value
		if (cmbTimeAgain.GetCurSel() == -1) {
			wchar_t buf[64];
			cmbTimeAgain.GetText(buf, _countof(buf));

			int h, m;
			if (ParseTime(buf, &h, &m, FALSE, IsRelativeCombo(m_hwnd, IDC_TIMEAGAIN))) {
				SYSTEMTIME Date;
				dateAgain.GetTime(&Date);
				ReformatTimeInput(m_hwnd, IDC_TIMEAGAIN, IDC_REFTIME, h, m, &Date);
			}
			else cmbTimeAgain.SetCurSel(0);
		}
	}

	void onKillFocus_RemindAgain(CCtrlCombo*)
	{
		// reformat displayed value if it has been edited
		if (cmbRemindAgainIn.GetCurSel() == -1) {
			wchar_t buf[64];
			cmbRemindAgainIn.GetText(buf, 30);

			int h, m;
			if (ParseTime(buf, &h, &m, TRUE, TRUE) && h + m) {
				if (h)
					mir_snwprintf(buf, L"%d:%02d %s", h, m, TranslateT("Hours"));
				else
					mir_snwprintf(buf, L"%d %s", m, TranslateT("Minutes"));

				cmbRemindAgainIn.SetText(buf);
			}
			else cmbRemindAgainIn.SetCurSel(0);
		}
	}

	void onClick_Dismiss(CCtrlButton*)
	{
		Close();
	}

	void onClick_RemindAgain(CCtrlButton*)
	{
		if (chkAfter.GetState()) {
			// delta time
			SYSTEMTIME tm;
			ULARGE_INTEGER li;

			GetSystemTime(&tm);
			SystemTimeToFileTime(&tm, (FILETIME*)&li);

			int TT = cmbRemindAgainIn.GetItemData(cmbRemindAgainIn.GetCurSel()) * 60;
			if (TT >= 24 * 3600) {
				// selection is 1 day or more, take daylight saving boundaries into consideration
				// (ie. 24 hour might actually be 23 or 25, in order for reminder to pop up at the
				// same time tomorrow)
				SYSTEMTIME tm2, tm3;
				ULARGE_INTEGER li2 = li;
				FileTimeToTzLocalST((FILETIME*)&li2, &tm2);
				li2.QuadPart += (TT * FILETIME_TICKS_PER_SEC);
				FileTimeToTzLocalST((FILETIME*)&li2, &tm3);
				if (tm2.wHour != tm3.wHour || tm2.wMinute != tm3.wMinute) {
					// boundary crossed

					// do a quick and dirty sanity check that times not more than 2 hours apart
					if (abs((int)(tm3.wHour * 60 + tm3.wMinute) - (int)(tm2.wHour * 60 + tm2.wMinute)) <= 120) {
						// adjust TT so that same HH:MM is set
						tm3.wHour = tm2.wHour;
						tm3.wMinute = tm2.wMinute;
						TzLocalSTToFileTime(&tm3, (FILETIME*)&li2);
						TT = (li2.QuadPart - li.QuadPart) / FILETIME_TICKS_PER_SEC;
					}
				}
			}
			else {
				// parse user input
				wchar_t s[32];
				cmbRemindAgainIn.GetText(s, _countof(s));

				int h = 0, m = 0;
				ParseTime(s, &h, &m, TRUE, TRUE);
				m += h * 60;
				if (!m) {
					MessageBox(m_hwnd, TranslateT("The specified time offset is invalid."), _A2W(SECTIONNAME), MB_OK | MB_ICONWARNING);
					return;
				}

				TT = m * 60;
			}

			// reset When from the current time
			if (!m_pReminder->bRepeat)
				m_pReminder->When = li;
			m_pReminder->When.QuadPart += (TT * FILETIME_TICKS_PER_SEC);
		}
		else if (chkOnDate.GetState()) {
			SYSTEMTIME Date;
			dateAgain.GetTime(&Date);
			if (!GetTriggerTime(m_hwnd, IDC_TIMEAGAIN, IDC_REFTIME, &Date))
				return;

			SystemTimeToFileTime(&Date, (FILETIME*)&m_pReminder->When);
		}

		// update reminder text
		char *ReminderText = nullptr;
		int SzT = SendDlgItemMessage(m_hwnd, IDC_REMDATA, WM_GETTEXTLENGTH, 0, 0);
		if (SzT) {
			if (SzT > MAX_REMINDER_LEN) SzT = MAX_REMINDER_LEN;
			ReminderText = (char*)malloc(SzT + 1);
			GetDlgItemTextA(m_hwnd, IDC_REMDATA, ReminderText, SzT + 1);
		}

		if (m_pReminder->Reminder)
			free(m_pReminder->Reminder);
		m_pReminder->Reminder = ReminderText;

		m_pReminder->bVisible = FALSE;
		m_pReminder->handle = nullptr;

		// re-insert tree item sorted
		arReminders.remove(m_pReminder);
		arReminders.insert(m_pReminder);
		m_pReminder = nullptr; // prevent reminder from being deleted;
		Close();
	}

	void onClick_None(CCtrlButton*)
	{
		// create note from remainder
		char *ReminderText = nullptr;
		int SzT = SendDlgItemMessage(m_hwnd, IDC_REMDATA, WM_GETTEXTLENGTH, 0, 0);
		if (SzT) {
			if (SzT > MAX_REMINDER_LEN) SzT = MAX_REMINDER_LEN;
			ReminderText = (char*)malloc(SzT + 1);
			GetDlgItemTextA(m_hwnd, IDC_REMDATA, ReminderText, SzT + 1);
		}

		NewNote(0, 0, -1, -1, ReminderText, nullptr, TRUE, TRUE, 0);
	}
};

INT_PTR OpenTriggeredReminder(WPARAM, LPARAM l)
{
	if (!l)
		return 0;

	l = ((CLISTEVENT*)l)->lParam;

	REMINDERDATA *pReminder = (REMINDERDATA*)FindReminder((DWORD)l);
	if (!pReminder || !pReminder->bSystemEventQueued)
		return 0;

	pReminder->bSystemEventQueued = false;
	if (QueuedReminderCount)
		QueuedReminderCount--;

	(new CReminderNotifyDlg(pReminder))->Show();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

class CReminderFormDlg : public CDlgBase
{
	REMINDERDATA *m_pReminder;

	CCtrlDate date;
	CCtrlCheck chkRepeat;
	CCtrlCombo cmbSound, cmbRepeat, cmbTime;
	CCtrlButton btnAdd, btnView, btnPlaySound;

public:
	CReminderFormDlg(REMINDERDATA *pReminder = nullptr) :
		CDlgBase(g_plugin, IDD_ADDREMINDER),
		m_pReminder(pReminder),
		date(this, IDC_DATE),
		btnAdd(this, IDC_ADDREMINDER),
		btnView(this, IDC_VIEWREMINDERS),
		btnPlaySound(this, IDC_BTN_PLAYSOUND),
		chkRepeat(this, IDC_CHECK_REPEAT),
		cmbTime(this, IDC_COMBOREMINDERTIME),
		cmbSound(this, IDC_COMBO_SOUND),
		cmbRepeat(this, IDC_COMBO_REPEATSND)
	{
		btnAdd.OnClick = Callback(this, &CReminderFormDlg::onClick_Add);
		btnView.OnClick = Callback(this, &CReminderFormDlg::onClick_View);
		btnPlaySound.OnClick = Callback(this, &CReminderFormDlg::onClick_PlaySound);

		date.OnChange = Callback(this, &CReminderFormDlg::onChange_Date);
		cmbSound.OnChange = Callback(this, &CReminderFormDlg::onChange_Sound);

		cmbTime.OnKillFocus = Callback(this, &CReminderFormDlg::onChange_Time);
	}

	bool OnInitDialog() override
	{
		ULARGE_INTEGER li;
		SYSTEMTIME tm;

		if (m_pReminder) {
			// opening the edit reminder dialog (uses same dialog resource as add reminder)
			SetWindowText(m_hwnd, TranslateT("Reminder"));
			SetDlgItemText(m_hwnd, IDC_ADDREMINDER, TranslateT("&Update Reminder"));
			btnView.Hide();

			li = m_pReminder->When;
			FileTimeToSystemTime((FILETIME*)&li, &tm);
		}
		else {
			GetSystemTime(&tm);
			SystemTimeToFileTime(&tm, (FILETIME*)&li);
		}

		// save reference time in hidden control, is needed when adding reminder to properly detect if speicifed
		// time wrapped around to tomorrow or not (dialog could in theory be open for a longer period of time
		// which could potentially mess up things otherwise)
		wchar_t s[64];
		mir_snwprintf(s, L"%I64x", li.QuadPart);
		SetDlgItemText(m_hwnd, IDC_REFTIME, s);

		PopulateTimeCombo(m_hwnd, IDC_COMBOREMINDERTIME, m_pReminder == nullptr, &tm);

		// make sure date picker uses reference time
		FileTimeToTzLocalST((FILETIME*)&li, &tm);
		date.SetTime(&tm);
		InitDatePicker(m_hwnd, IDC_DATE);

		SendDlgItemMessage(m_hwnd, IDC_REMINDER, EM_LIMITTEXT, MAX_REMINDER_LEN, 0);

		if (m_pReminder) {
			mir_snwprintf(s, L"%02d:%02d", tm.wHour, tm.wMinute);

			// search for preset first
			int n = cmbTime.FindString(s);
			if (n != -1)
				cmbTime.SetCurSel(n);
			else
				cmbTime.SetText(s);

			SetDlgItemTextA(m_hwnd, IDC_REMINDER, m_pReminder->Reminder);
		}
		else cmbTime.SetCurSel(0);

		// populate sound repeat combo
		wchar_t *lpszEvery = TranslateT("Every");
		wchar_t *lpszSeconds = TranslateT("Seconds");

		// NOTE: use multiples of REMINDER_UPDATE_INTERVAL_SHORT (currently 5 seconds)
		cmbRepeat.AddString(TranslateT("Never"), 0);

		mir_snwprintf(s, L"%s 5 %s", lpszEvery, lpszSeconds);
		cmbRepeat.AddString(s, 5);

		mir_snwprintf(s, L"%s 10 %s", lpszEvery, lpszSeconds);
		cmbRepeat.AddString(s, 10);

		mir_snwprintf(s, L"%s 15 %s", lpszEvery, lpszSeconds);
		cmbRepeat.AddString(s, 15);

		mir_snwprintf(s, L"%s 20 %s", lpszEvery, lpszSeconds);
		cmbRepeat.AddString(s, 20);

		mir_snwprintf(s, L"%s 30 %s", lpszEvery, lpszSeconds);
		cmbRepeat.AddString(s, 30);

		mir_snwprintf(s, L"%s 60 %s", lpszEvery, lpszSeconds);
		cmbRepeat.AddString(s, 60);

		if (m_pReminder && m_pReminder->RepeatSound) {
			mir_snwprintf(s, L"%s %d %s", lpszEvery, m_pReminder->RepeatSound, lpszSeconds);
			cmbRepeat.SetText(s);
			cmbRepeat.SetCurSel(cmbRepeat.FindString(s));
		}
		else cmbRepeat.SetCurSel(0);

		// populate sound selection combo
		cmbSound.AddString(TranslateT("Default"), 0);
		cmbSound.AddString(TranslateT("Alternative 1"), 1);
		cmbSound.AddString(TranslateT("Alternative 2"), 2);
		cmbSound.AddString(TranslateT("None"), -1);

		if (m_pReminder && m_pReminder->SoundSel) {
			const UINT n2 = m_pReminder->SoundSel < 0 ? 3 : m_pReminder->SoundSel;
			cmbSound.SetCurSel(n2);
		}
		else cmbSound.SetCurSel(0);

		HICON hIcon = IcoLib_GetIconByHandle(iconList[12].hIcolib);
		btnPlaySound.SendMsg(BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);

		if (m_pReminder && m_pReminder->SoundSel) {
			btnPlaySound.Enable(m_pReminder->SoundSel >= 0);
			cmbRepeat.Enable(m_pReminder->SoundSel >= 0);
		}

		if (m_pReminder)
			SetFocus(GetDlgItem(m_hwnd, IDC_ADDREMINDER));
		else
			SetFocus(GetDlgItem(m_hwnd, IDC_REMINDER));
		return true;
	}

	bool OnClose() override
	{
		bNewReminderVisible = false;
		if (m_pReminder)
			m_pReminder->bVisible = false;
		return true;
	}

	void onClick_Add(CCtrlButton*)
	{
		SYSTEMTIME Date;
		date.GetTime(&Date);
		if (!GetTriggerTime(m_hwnd, IDC_COMBOREMINDERTIME, IDC_REFTIME, &Date))
			return;

		int RepeatSound = cmbRepeat.GetCurSel();
		if (RepeatSound != CB_ERR)
			RepeatSound = cmbRepeat.GetItemData(RepeatSound);
		else
			RepeatSound = 0;

		int SzT = SendDlgItemMessage(m_hwnd, IDC_REMINDER, WM_GETTEXTLENGTH, 0, 0);
		char *ReminderText = nullptr;
		if (SzT) {
			if (SzT > MAX_REMINDER_LEN) SzT = MAX_REMINDER_LEN;
			ReminderText = (char*)malloc(SzT + 1);
			GetDlgItemTextA(m_hwnd, IDC_REMINDER, ReminderText, SzT + 1);
		}

		bool bClose = g_plugin.bCloseAfterAddReminder || m_pReminder;
		if (!m_pReminder) {
			// new reminder
			REMINDERDATA *TempRem = new REMINDERDATA();
			TempRem->uid = CreateUid();
			SystemTimeToFileTime(&Date, (FILETIME*)&TempRem->When);
			TempRem->Reminder = ReminderText;
			TempRem->bRepeat = chkRepeat.GetState();
			TempRem->SoundSel = cmbSound.GetItemData(cmbSound.GetCurSel());
			TempRem->RepeatSound = TempRem->SoundSel < 0 ? 0 : (UINT)RepeatSound;
			arReminders.insert(TempRem);
		}
		else {
			// update existing reminder
			SystemTimeToFileTime(&Date, (FILETIME*)&m_pReminder->When);

			if (m_pReminder->Reminder)
				free(m_pReminder->Reminder);
			m_pReminder->Reminder = ReminderText;

			m_pReminder->bRepeat = chkRepeat.GetState();
			m_pReminder->SoundSel = cmbSound.GetItemData(cmbSound.GetCurSel());
			m_pReminder->RepeatSound = m_pReminder->SoundSel < 0 ? 0 : (UINT)RepeatSound;

			// re-insert tree item sorted
			arReminders.remove(m_pReminder);
			arReminders.insert(m_pReminder);

			m_pReminder->bVisible = false;
			m_pReminder = nullptr; // prevent new reminder from being deleted
		}
		SetDlgItemTextA(m_hwnd, IDC_REMINDER, "");
		JustSaveReminders();
		NOTIFY_LIST();
		if (bClose)
			Close();
	}

	void onClick_View(CCtrlButton*)
	{
		PluginMenuCommandViewReminders(0, 0);
	}

	void onClick_PlaySound(CCtrlButton*)
	{
		int n = cmbSound.GetItemData(cmbSound.GetCurSel());
		switch (n) {
		case 0: 
			Skin_PlaySound("AlertReminder"); 
			break;
		case 1:
			Skin_PlaySound("AlertReminder2");
			break;
		case 2:
			Skin_PlaySound("AlertReminder3");
			break;
		}
	}

	void onChange_Date(CCtrlDate*)
	{
		OnDateChanged(m_hwnd, IDC_DATE, IDC_COMBOREMINDERTIME, IDC_REFTIME);
	}

	void onChange_Time(CCtrlCombo*)
	{
		if (cmbTime.GetCurSel() != -1)
			return;

		wchar_t buf[64];
		cmbTime.GetText(buf, _countof(buf));

		int h, m;
		if (ParseTime(buf, &h, &m, FALSE, IsRelativeCombo(m_hwnd, IDC_COMBOREMINDERTIME))) {
			SYSTEMTIME Date;
			date.GetTime(&Date);
			ReformatTimeInput(m_hwnd, IDC_COMBOREMINDERTIME, IDC_REFTIME, h, m, &Date);
		}
		else cmbTime.SetCurSel(0);
	}

	void onChange_Sound(CCtrlCombo*)
	{
		int n = cmbSound.GetItemData(cmbSound.GetCurSel());
		btnPlaySound.Enable(n >= 0);
		cmbRepeat.Enable(n >= 0);
	}
};

void EditReminder(REMINDERDATA *p)
{
	if (!p)
		return;

	if (!bNewReminderVisible && !p->bSystemEventQueued) {
		if (!p->bVisible) {
			p->bVisible = true;
			(new CReminderFormDlg(p))->Show();
		}
		else BringWindowToTop(p->handle);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static void InitListView(HWND AHLV)
{
	ListView_SetHoverTime(AHLV, 700);
	ListView_SetExtendedListViewStyle(AHLV, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TRACKSELECT);
	ListView_DeleteAllItems(AHLV);

	int i = 0;
	for (auto &pReminder : arReminders) {
		LV_ITEM lvTIt;
		lvTIt.mask = LVIF_TEXT;

		wchar_t S1[128];
		GetTriggerTimeString(&pReminder->When, S1, _countof(S1), TRUE);

		lvTIt.iItem = i;
		lvTIt.iSubItem = 0;
		lvTIt.pszText = S1;
		ListView_InsertItem(AHLV, &lvTIt);
		lvTIt.mask = LVIF_TEXT;

		wchar_t *S2 = GetPreviewString(pReminder->Reminder);
		lvTIt.iItem = i;
		lvTIt.iSubItem = 1;
		lvTIt.pszText = S2;
		ListView_SetItem(AHLV, &lvTIt);

		i++;
	}

	ListView_SetItemState(AHLV, 0, LVIS_SELECTED, LVIS_SELECTED);
}

void OnListResize(HWND hwndDlg)
{
	HWND hList, hText, hBtnNew, hBtnClose;
	RECT lr, cr, tr, clsr;
	int th, btnh, btnw, MARGIN;
	POINT org = {0};

	hList = GetDlgItem(hwndDlg, IDC_LISTREMINDERS);
	hText = GetDlgItem(hwndDlg, IDC_REMINDERDATA);
	hBtnNew = GetDlgItem(hwndDlg, IDC_ADDNEWREMINDER);
	hBtnClose = GetDlgItem(hwndDlg, IDC_CLOSE);

	ClientToScreen(hwndDlg, &org);
	GetClientRect(hwndDlg, &cr);

	GetWindowRect(hList, &lr); OffsetRect(&lr, -org.x, -org.y);
	GetWindowRect(hText, &tr); OffsetRect(&tr, -org.x, -org.y);
	GetWindowRect(hBtnClose, &clsr); OffsetRect(&clsr, -org.x, -org.y);

	MARGIN = lr.left;

	th = tr.bottom - tr.top;
	btnh = clsr.bottom - clsr.top;
	btnw = clsr.right - clsr.left;

	clsr.left = cr.right - MARGIN - btnw;
	clsr.top = cr.bottom - MARGIN - btnh;

	tr.left = MARGIN;
	tr.right = cr.right - MARGIN;
	tr.bottom = clsr.top - MARGIN - 2;
	tr.top = tr.bottom - th;

	lr.right = cr.right - MARGIN;
	lr.bottom = tr.top - 5;

	MoveWindow(hList, lr.left, lr.top, lr.right - lr.left, lr.bottom - lr.top, FALSE);
	MoveWindow(hText, tr.left, tr.top, tr.right - tr.left, tr.bottom - tr.top, FALSE);

	MoveWindow(hBtnClose, clsr.left, clsr.top, btnw, btnh, FALSE);
	clsr.left -= btnw + 2;
	MoveWindow(hBtnNew, clsr.left, clsr.top, btnw, btnh, FALSE);

	RedrawWindow(hwndDlg, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}

void UpdateGeomFromWnd(HWND hwndDlg, int *geom, int *colgeom, int nCols)
{
	if (geom) {
		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(hwndDlg, &wp);

		geom[0] = wp.rcNormalPosition.left;
		geom[1] = wp.rcNormalPosition.top;
		geom[2] = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
		geom[3] = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
	}

	if (colgeom) {
		HWND H = GetDlgItem(hwndDlg, IDC_LISTREMINDERS);

		for (int i = 0; i < nCols; i++)
			colgeom[i] = ListView_GetColumnWidth(H, i);
	}
}

static BOOL DoListContextMenu(HWND AhWnd, WPARAM wParam, LPARAM lParam, REMINDERDATA *pReminder)
{
	HWND hwndListView = (HWND)wParam;
	if (hwndListView != GetDlgItem(AhWnd, IDC_LISTREMINDERS))
		return FALSE;
	
	HMENU hMenuLoad = LoadMenuA(g_plugin.getInst(), "MNU_REMINDERPOPUP");
	HMENU FhMenu = GetSubMenu(hMenuLoad, 0);

	MENUITEMINFO mii = {};
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_STATE;
	mii.fState = MFS_DEFAULT;
	if (!pReminder || pReminder->bSystemEventQueued)
		mii.fState |= MFS_GRAYED;
	SetMenuItemInfo(FhMenu, ID_CONTEXTMENUREMINDERLISTVIEW_EDIT, FALSE, &mii);

	if (!pReminder)
		EnableMenuItem(FhMenu, IDM_DELETEREMINDER, MF_GRAYED | MF_BYCOMMAND);

	TranslateMenu(FhMenu);
	TrackPopupMenu(FhMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, LOWORD(lParam), HIWORD(lParam), 0, AhWnd, nullptr);
	DestroyMenu(hMenuLoad);
	return TRUE;
}

static INT_PTR CALLBACK DlgProcViewReminders(HWND hwndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
	HWND H = GetDlgItem(hwndDlg, IDC_LISTREMINDERS);

	switch (Message) {
	case WM_SIZE:
		OnListResize(hwndDlg);
		UpdateGeomFromWnd(hwndDlg, g_reminderListGeom, nullptr, 0);
		break;

	case WM_MOVE:
		UpdateGeomFromWnd(hwndDlg, g_reminderListGeom, nullptr, 0);
		break;

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mm = (MINMAXINFO*)lParam;
			mm->ptMinTrackSize.x = 394;
			mm->ptMinTrackSize.y = 300;
		}
		return 0;

	case WM_RELOAD:
		SetDlgItemTextA(hwndDlg, IDC_REMINDERDATA, "");
		InitListView(GetDlgItem(hwndDlg, IDC_LISTREMINDERS));
		return TRUE;

	case WM_CONTEXTMENU:
		{
			REMINDERDATA *pReminder = nullptr;

			if (ListView_GetSelectedCount(H)) {
				int i = ListView_GetSelectionMark(H);
				if (i != -1)
					pReminder = arReminders[i];
			}

			if (DoListContextMenu(hwndDlg, wParam, lParam, pReminder))
				return TRUE;
		}
		break;

	case WM_INITDIALOG:
		Window_SetIcon_IcoLib(hwndDlg, iconList[6].hIcolib);

		TranslateDialogDefault(hwndDlg);
		SetDlgItemTextA(hwndDlg, IDC_REMINDERDATA, "");

		LV_COLUMN lvCol;
		lvCol.mask = LVCF_TEXT | LVCF_WIDTH;
		lvCol.pszText = TranslateT("Reminder text");
		lvCol.cx = g_reminderListColGeom[1];
		ListView_InsertColumn(H, 0, &lvCol);

		lvCol.mask = LVCF_TEXT | LVCF_WIDTH;
		lvCol.pszText = TranslateT("Date of activation");
		lvCol.cx = g_reminderListColGeom[0];
		ListView_InsertColumn(H, 0, &lvCol);

		InitListView(H);

		SetWindowLongPtr(GetDlgItem(H, 0), GWL_ID, IDC_LISTREMINDERS_HEADER);
		LV = hwndDlg;

		if (g_reminderListGeom[1] && g_reminderListGeom[2]) {
			WINDOWPLACEMENT wp;
			wp.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(hwndDlg, &wp);
			wp.rcNormalPosition.left = g_reminderListGeom[0];
			wp.rcNormalPosition.top = g_reminderListGeom[1];
			wp.rcNormalPosition.right = g_reminderListGeom[2] + g_reminderListGeom[0];
			wp.rcNormalPosition.bottom = g_reminderListGeom[3] + g_reminderListGeom[1];
			SetWindowPlacement(hwndDlg, &wp);
		}
		return TRUE;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		bListReminderVisible = FALSE;
		return TRUE;

	case WM_NOTIFY:
		if (wParam == IDC_LISTREMINDERS) {
			LPNMLISTVIEW NM = (LPNMLISTVIEW)lParam;
			switch (NM->hdr.code) {
			case LVN_ITEMCHANGED:
				SetDlgItemTextA(hwndDlg, IDC_REMINDERDATA, arReminders[NM->iItem]->Reminder);
				break;

			case NM_DBLCLK:
				if (ListView_GetSelectedCount(H)) {
					int i = ListView_GetSelectionMark(H);
					if (i != -1)
						EditReminder(arReminders[i]);
				}
				break;
			}
		}
		else if (wParam == IDC_LISTREMINDERS_HEADER) {
			LPNMHEADER NM = (LPNMHEADER)lParam;
			switch (NM->hdr.code) {
			case HDN_ENDTRACK:
				UpdateGeomFromWnd(hwndDlg, nullptr, g_reminderListColGeom, _countof(g_reminderListColGeom));
				break;
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_CONTEXTMENUREMINDERLISTVIEW_EDIT:
			if (ListView_GetSelectedCount(H)) {
				int i = ListView_GetSelectionMark(H);
				if (i != -1)
					EditReminder(arReminders[i]);
			}
			return TRUE;

		case IDC_CLOSE:
			DestroyWindow(hwndDlg);
			bListReminderVisible = false;
			return TRUE;

		case IDM_NEWREMINDER:
		case IDC_ADDNEWREMINDER:
			PluginMenuCommandNewReminder(0, 0);
			return TRUE;

		case IDM_DELETEALLREMINDERS:
			if (arReminders.getCount() && IDOK == MessageBox(hwndDlg, TranslateT("Are you sure you want to delete all reminders?"), _A2W(SECTIONNAME), MB_OKCANCEL)) {
				SetDlgItemTextA(hwndDlg, IDC_REMINDERDATA, "");
				DeleteReminders();
				InitListView(GetDlgItem(hwndDlg, IDC_LISTREMINDERS));
			}
			return TRUE;

		case IDM_DELETEREMINDER:
			if (ListView_GetSelectedCount(H)) {
				int i = ListView_GetSelectionMark(H);
				if (i != -1 && IDOK == MessageBox(hwndDlg, TranslateT("Are you sure you want to delete this reminder?"), _A2W(SECTIONNAME), MB_OKCANCEL)) {
					SetDlgItemTextA(hwndDlg, IDC_REMINDERDATA, "");
					DeleteReminder(arReminders[i]);
					JustSaveReminders();
					InitListView(H);
				}
			}
			return TRUE;
		}
		break;

	case WM_DESTROY:
		Window_FreeIcon_IcoLib(hwndDlg);
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CloseReminderList()
{
	if (bListReminderVisible) {
		DestroyWindow(LV);
		bListReminderVisible = false;
	}
}

INT_PTR PluginMenuCommandNewReminder(WPARAM, LPARAM)
{
	if (!bNewReminderVisible) {
		bNewReminderVisible = true;
		(new CReminderFormDlg())->Show();
	}
	return 0;
}

INT_PTR PluginMenuCommandViewReminders(WPARAM, LPARAM)
{
	if (!bListReminderVisible) {
		CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_LISTREMINDERS), nullptr, DlgProcViewReminders);
		bListReminderVisible = true;
	}
	else BringWindowToTop(LV);
	return 0;
}

INT_PTR PluginMenuCommandDeleteReminders(WPARAM, LPARAM)
{
	if (arReminders.getCount())
		if (IDOK == MessageBox(nullptr, TranslateT("Are you sure you want to delete all reminders?"), TranslateT(SECTIONNAME), MB_OKCANCEL))
			DeleteReminders();
	return 0;
}
