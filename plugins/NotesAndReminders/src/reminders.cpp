#include "stdafx.h"

#define FILETIME_TICKS_PER_SEC 10000000ll

#define MAX_REMINDER_LEN	16384

// RemindersData DB data params
#define DATATAG_TEXT	      1 // %s
#define DATATAG_SNDREPEAT  2 // %u (specifies seconds to wait between sound repeats, 0 if repeat is disabled)
#define DATATAG_SNDSEL     3 // %d (which sound to use, default, alt1, alt2, -1 means no sound at all)
#define DATATAG_REPEAT     4 // %d (repeateable reminder)

#define MinutesToFileTime (60ll * FILETIME_TICKS_PER_SEC)
#define DayToFileTime (86400ll * FILETIME_TICKS_PER_SEC)

/////////////////////////////////////////////////////////////////////////////////////////

static void RemoveReminderSystemEvent(struct REMINDERDATA *p);

struct REMINDERDATA : public MZeroedObject
{
	HWND handle;
	DWORD uid;
	CMStringW wszText;
	ULONGLONG When;	// FILETIME in UTC
	UINT RepeatSound;
	UINT RepeatSoundTTL;
	int  SoundSel;			// -1 if sound disabled
	bool bVisible;
	bool bRepeat;
	bool bSystemEventQueued;

	REMINDERDATA()
	{
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
	if (v1->When == v2->When)
		return 0;

	return (v1->When < v2->When) ? -1 : 1;
}

static LIST<REMINDERDATA> arReminders(1, ReminderSortCb);

static class CReminderListDlg *pListDialog;

static bool bNewReminderVisible = false;
static UINT QueuedReminderCount = 0;

int WS_Send(SOCKET s, char *data, int datalen);
unsigned long WS_ResolveName(char *name, WORD *port, int defaultPort);

void Send(char *user, char *host, const char *Msg, char* server);
wchar_t* GetPreviewString(const wchar_t *lpsz);

static void NotifyList();

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

static void InsertReminder(REMINDERDATA *p)
{
	while (arReminders.find(p))
		p->When++;
	arReminders.insert(p);
}

static DWORD CreateUid()
{
	if (!arReminders.getCount())
		return 1;

	// check existing reminders if uid is in use
	for (DWORD uid = 1;; uid++)
		if (!FindReminder(uid)) // uid in use
			return uid;
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
		// data header (save 'When' with 1-second resolution, it's just a waste to have 100-nanosecond resolution
		// which results in larger DB strings with no use)
		CMStringA szValue;
		szValue.Format("X%u:%I64x", pReminder->uid, pReminder->When / FILETIME_TICKS_PER_SEC);

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
		if (!pReminder->wszText.IsEmpty())
			szValue.AppendFormat("\033""%u:%s", DATATAG_TEXT, ptrA(mir_utf8encodeW(pReminder->wszText)).get());

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

static bool LoadReminder(char *Value)
{
	char *DelPos = strchr(Value, 0x1B);
	if (DelPos)
		*DelPos = 0;

	// uid:when
	char *TVal = strchr(Value + 1, ':');
	if (!TVal || (DelPos && TVal > DelPos))
		return false;
	*TVal++ = 0;

	REMINDERDATA *TempRem = new REMINDERDATA();
	TempRem->uid = strtoul(Value + 1, nullptr, 10);
	TempRem->When = _strtoui64(TVal, nullptr, 16) * FILETIME_TICKS_PER_SEC;

	// optional \033 separated params
	while (DelPos) {
		TVal = DelPos + 1;
		// find param end and make sure it's null-terminated (if end of data then it's already null-terminated)
		DelPos = strchr(TVal, 0x1B);
		if (DelPos)
			*DelPos = 0;

		// tag:<data>
		char *sep = strchr(TVal, ':');
		if (!sep || (DelPos && sep > DelPos))
			break;

		UINT tag = strtoul(TVal, nullptr, 10);
		TVal = sep + 1;

		switch (tag) {
		case DATATAG_TEXT:
			if (auto *szText = mir_utf8decodeW(TVal)) {
				TempRem->wszText = szText;
				mir_free(szText);
			}
			else TempRem->wszText = _A2T(TVal);
			break;

		case DATATAG_SNDREPEAT:
			TempRem->RepeatSound = strtoul(TVal, nullptr, 10);
			break;

		case DATATAG_SNDSEL:
			TempRem->SoundSel = strtol(TVal, nullptr, 10);
			if (TempRem->SoundSel > 2) TempRem->SoundSel = 2;
			break;

		case DATATAG_REPEAT:
			TempRem->bRepeat = strtol(TVal, nullptr, 10) != 0;
			break;
		}
	}

	if (TempRem->SoundSel < 0)
		TempRem->RepeatSound = 0;

	// queue uid generation if invalid uid is present
	InsertReminder(TempRem);
	return TempRem->uid == 0;
}

void LoadReminders(void)
{
	bool GenerateUids = false;

	arReminders.destroy();
	int RemindersCount = g_plugin.getDword("RemindersData", 0);

	for (int i = 0; i < RemindersCount; i++) {
		char ValueName[32];
		mir_snprintf(ValueName, "RemindersData%d", i);

		DBVARIANT dbv = { DBVT_BLOB };
		if (db_get(0, MODULENAME, ValueName, &dbv))
			continue;

		// was the blob found
		if (!dbv.cpbVal || !dbv.pbVal)
			continue;

		if (dbv.pbVal[0] == 'X')
			GenerateUids |= LoadReminder((char*)dbv.pbVal);
		db_free(&dbv);
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

void GetTriggerTimeString(const ULONGLONG *When, wchar_t *s, size_t strSize, BOOL bUtc)
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

/////////////////////////////////////////////////////////////////////////////////////////
// Basic reminder dialog class

class CReminderBaseDlg : public CDlgBase
{
	typedef CDlgBase CSuper;

protected:
	CCtrlDate m_date;
	CCtrlCombo cmbTime;

	bool m_bManualTime = false, m_bRelativeCombo = false;
	__int64 m_savedLi = 0;

	CReminderBaseDlg(int iDlgId) :
		CSuper(g_plugin, iDlgId),
		m_date(this, IDC_DATE),
		cmbTime(this, IDC_TIMECOMBO)
	{
		m_date.OnChange = Callback(this, &CReminderBaseDlg::onChange_Date);

		cmbTime.OnChange = Callback(this, &CReminderBaseDlg::onChange_Time);
		cmbTime.OnSelChanged = Callback(this, &CReminderBaseDlg::onSelChange_Time);
		cmbTime.OnKillFocus = Callback(this, &CReminderBaseDlg::onKillFocus_Time);
	}

	void onChange_Time(CCtrlCombo *)
	{
		m_bManualTime = true;
	}

	void onSelChange_Time(CCtrlCombo *)
	{
		m_bManualTime = false;
	}

	void onKillFocus_Time(CCtrlCombo*)
	{
		if (cmbTime.GetCurSel() != -1)
			return;

		wchar_t buf[64];
		cmbTime.GetText(buf, _countof(buf));

		int h, m;
		if (ParseTime(buf, &h, &m, FALSE, m_bRelativeCombo)) {
			SYSTEMTIME Date;
			m_date.GetTime(&Date);
			ReformatTimeInput(m_savedLi, h, m, &Date);
		}
		else cmbTime.SetCurSel(0);
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// repopulate time combo list with regular times (not offsets like "23:32 (5 minutes)" etc.)

	void onChange_Date(CCtrlDate*)
	{
		wchar_t s[32];
		cmbTime.GetText(s, _countof(s));

		int h = -1, m;
		ParseTime(s, &h, &m, FALSE, FALSE);

		SYSTEMTIME Date, DateUtc;
		m_date.GetTime(&Date);

		TzSpecificLocalTimeToSystemTime(nullptr, &Date, &DateUtc);
		PopulateTimeCombo(&DateUtc);

		if (h < 0) {
			// parsing failed, default to current time
			SYSTEMTIME tm;
			GetLocalTime(&tm);
			h = (UINT)tm.wHour;
			m = (UINT)tm.wMinute;
		}

		ReformatTimeInput(m_savedLi, h, m, &Date);
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// in:  pDate contains the desired trigger date in LOCAL time
	// out: pDate contains the resulting trigger time and date in UTC

	bool GetTriggerTime(ULONGLONG savedLi, SYSTEMTIME &pDate)
	{
		// absolute time specified in combobox item data
		if (!m_bManualTime) {
			// use preset value
			UINT nDeltaSeconds = cmbTime.GetCurData();
			if (m_bRelativeCombo) {
				// combine date from pDate (local) and time from savedLi (utc)
				SYSTEMTIME st, st2;
				FileTimeToSystemTime((FILETIME *)&savedLi, &st);
				SystemTimeToTzSpecificLocalTime(nullptr, &st, &st2);
				st2.wYear = pDate.wYear; st2.wMonth = pDate.wMonth; st2.wDay = pDate.wDay; st2.wDayOfWeek = pDate.wDayOfWeek;
				SystemTimeToFileTime(&st2, (FILETIME*)&savedLi);

				// time offset from ref time ("24:43 (5 Minutes)" etc.)
				savedLi += (ULONGLONG)nDeltaSeconds * FILETIME_TICKS_PER_SEC;

				FileTimeToSystemTime((FILETIME*)&savedLi, &st);
				TzSpecificLocalTimeToSystemTime(nullptr, &st, &pDate);

				// if specified time is a small offset (< 10 Minutes) then retain current second count for better accuracy
				// otherwise try to match specified time (which never contains seconds only even minutes) as close as possible
				if (nDeltaSeconds >= 10 * 60) {
					pDate.wSecond = 0;
					pDate.wMilliseconds = 0;
				}
			}
			else {
				// absolute time (offset from midnight on pDate)
				pDate.wHour = pDate.wMinute = pDate.wSecond = pDate.wMilliseconds = 0;
				TzLocalSTToFileTime(&pDate, (FILETIME*)&savedLi);
				savedLi += (ULONGLONG)nDeltaSeconds * FILETIME_TICKS_PER_SEC;

				FileTimeToSystemTime((FILETIME*)&savedLi, &pDate);
			}
			return true;
		}

		// user entered a custom value
		wchar_t buf[32];
		cmbTime.GetText(buf, _countof(buf));

		int h, m;
		if (!ParseTime(buf, &h, &m, FALSE, m_bRelativeCombo)) {
			MessageBox(cmbTime.GetParent()->GetHwnd(), TranslateT("The specified time is invalid."), _A2W(SECTIONNAME), MB_OK | MB_ICONWARNING);
			return false;
		}

		// absolute time (on pDate)
		if (ReformatTimeInput(savedLi, h, m, &pDate, nullptr))
			return false;

		pDate.wHour = h;
		pDate.wMinute = m;
		pDate.wSecond = 0;
		pDate.wMilliseconds = 0;

		ULONGLONG li;
		TzLocalSTToFileTime(&pDate, (FILETIME*)&li);
		FileTimeToSystemTime((FILETIME*)&li, &pDate);
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Initialize date control

	void InitDatePicker(LONGLONG li)
	{
		SYSTEMTIME tm;
		FileTimeToTzLocalST((FILETIME*)&li, &tm);
		m_date.SetTime(&tm);

		// tweak style of picker
		if (IsWinVerVistaPlus()) {
			DWORD dw = m_date.SendMsg(DTM_GETMCSTYLE, 0, 0);
			dw |= MCS_WEEKNUMBERS | MCS_NOSELCHANGEONNAV;
			m_date.SendMsg(DTM_SETMCSTYLE, 0, dw);
		}

		mir_subclassWindow(m_date.GetHwnd(), DatePickerWndProc);
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// NOTE: may seem like a bit excessive time converstion and handling, but this is 
	// done in order to gracefully handle crossing daylight saving boundaries

	void PopulateTimeCombo(const SYSTEMTIME *tmUtc)
	{
		cmbTime.ResetContent();

		ULONGLONG li;
		wchar_t s[64];

		// generate absolute time table for date different than today
		SYSTEMTIME tm2;
		GetSystemTime(&tm2);
		if (tmUtc->wDay != tm2.wDay || tmUtc->wMonth != tm2.wMonth || tmUtc->wYear != tm2.wYear) {
			// ensure that we start on midnight local time
			SystemTimeToTzSpecificLocalTime(nullptr, tmUtc, &tm2);
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

				// item data contains time offset from midnight in seconds (bit 31 is set to flag that
				// combo box items are absolute times and not relative times like below
				cmbTime.AddString(s, ((h * 60 + m) * 60));

				li += 30ll * MinutesToFileTime;

				if (tm2.wHour == 23 && tm2.wMinute >= 30)
					break;
			}

			m_bRelativeCombo = true; // time is relative to the current time
			return;
		}

		///////////////////////////////////////////////////////////////////////////////////////

		SystemTimeToFileTime(tmUtc, (FILETIME*)&li);
		ULONGLONG ref = li;
		m_bRelativeCombo = false; // absolute time

		// NOTE: item data contains offset from reference time (tmUtc) in seconds

		// cur time
		FileTimeToTzLocalST((FILETIME*)&li, &tm2);
		WORD wCurHour = tm2.wHour;
		WORD wCurMinute = tm2.wMinute;
		mir_snwprintf(s, L"%02d:%02d", (UINT)tm2.wHour, (UINT)tm2.wMinute);
		cmbTime.AddString(s, (li - ref) / FILETIME_TICKS_PER_SEC);

		// 5 minutes
		li += (ULONGLONG)5 * MinutesToFileTime;
		FileTimeToTzLocalST((FILETIME*)&li, &tm2);
		mir_snwprintf(s, L"%02d:%02d (5 %s)", (UINT)tm2.wHour, (UINT)tm2.wMinute, TranslateT("Minutes"));
		cmbTime.AddString(s, (li - ref) / FILETIME_TICKS_PER_SEC);

		// 10 minutes
		li += (ULONGLONG)5 * MinutesToFileTime;
		FileTimeToTzLocalST((FILETIME*)&li, &tm2);
		mir_snwprintf(s, L"%02d:%02d (10 %s)", (UINT)tm2.wHour, (UINT)tm2.wMinute, TranslateT("Minutes"));
		cmbTime.AddString(s, (li - ref) / FILETIME_TICKS_PER_SEC);

		// 15 minutes
		li += (ULONGLONG)5 * MinutesToFileTime;
		FileTimeToTzLocalST((FILETIME*)&li, &tm2);
		mir_snwprintf(s, L"%02d:%02d (15 %s)", (UINT)tm2.wHour, (UINT)tm2.wMinute, TranslateT("Minutes"));
		cmbTime.AddString(s, (li - ref) / FILETIME_TICKS_PER_SEC);

		// 30 minutes
		li += (ULONGLONG)15 * MinutesToFileTime;
		FileTimeToTzLocalST((FILETIME*)&li, &tm2);
		mir_snwprintf(s, L"%02d:%02d (30 %s)", (UINT)tm2.wHour, (UINT)tm2.wMinute, TranslateT("Minutes"));
		cmbTime.AddString(s, (li - ref) / FILETIME_TICKS_PER_SEC);

		// round +1h time to nearest even or half hour
		li += (ULONGLONG)30 * MinutesToFileTime;
		li = (li / (30 * MinutesToFileTime)) * (30 * MinutesToFileTime);

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
			dt = (UINT)((li / MinutesToFileTime) - (ref / MinutesToFileTime));
			if (dt < 60)
				mir_snwprintf(s, L"%02d:%02d (%d %s)", (UINT)tm2.wHour, (UINT)tm2.wMinute, dt, TranslateT("Minutes"));
			else
				mir_snwprintf(s, L"%02d:%02d (%d.%d %s)", (UINT)tm2.wHour, (UINT)tm2.wMinute, dt / 60, ((dt % 60) * 10) / 60, TranslateT("Hours"));
			cmbTime.AddString(s, dt * 60);

			li += 30ll * MinutesToFileTime;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// returns non-zero if specified time was inside "missing" hour of daylight saving
	// IMPORTANT: triggerRelUtcOut is only initialized if IsRelativeCombo() is TRUE and return value is 0

	int ReformatTimeInput(ULONGLONG savedLi, int h, int m, const SYSTEMTIME *pDateLocal, ULONGLONG *triggerRelUtcOut = nullptr)
	{
		if (h < 0) {
			// time value is an offset ('m' holds the offset in minutes)
			if (m_bRelativeCombo) {
				ULONGLONG ref, li;
				li = ref = savedLi;

				// clamp delta time to 23.5 hours (coule be issues otherwise as relative combo only handles <24)
				if (m > (23 * 60 + 30))
					m = 23 * 60 + 30;

				li += (ULONGLONG)(m * 60) * FILETIME_TICKS_PER_SEC;

				SYSTEMTIME tm;
				FileTimeToTzLocalST((FILETIME*)&li, &tm);
				h = (int)tm.wHour;
				m = (int)tm.wMinute;

				if (triggerRelUtcOut)
					*triggerRelUtcOut = li;

				wchar_t buf[64];
				UINT dt = (UINT)((li / MinutesToFileTime) - (ref / MinutesToFileTime));
				if (dt < 60)
					mir_snwprintf(buf, L"%02d:%02d (%d %s)", h, m, dt, TranslateT("Minutes"));
				else
					mir_snwprintf(buf, L"%02d:%02d (%d.%d %s)", h, m, dt / 60, ((dt % 60) * 10) / 60, TranslateT("Hours"));

				// search for preset
				int n = cmbTime.FindString(buf);
				if (n != -1) {
					cmbTime.SetCurSel(n);
					return 0;
				}

				cmbTime.SetText(buf);
			}
			else // should never happen
				cmbTime.SetCurSel(0);

			return 0;
		}

		// search for preset first
		wchar_t buf[64];
		mir_snwprintf(buf, L"%02d:%02d", h, m);
		int n = cmbTime.FindString(buf);
		if (n != -1) {
			cmbTime.SetCurSel(n);
			return 0;
		}

		// date format is a time offset ("24:43 (5 Minutes)" etc.)
		if (m_bRelativeCombo) {
			// get reference time (UTC) from hidden control
			ULONGLONG ref = savedLi;

			SYSTEMTIME tmRefLocal;
			FileTimeToTzLocalST((FILETIME*)&ref, &tmRefLocal);

			ULONGLONG li;
			const UINT nRefT = (UINT)tmRefLocal.wHour * 60 + (UINT)tmRefLocal.wMinute;
			const UINT nT = h * 60 + m;

			SYSTEMTIME tmTriggerLocal, tmTriggerLocal2;
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
					li += (ULONGLONG)3600 * FILETIME_TICKS_PER_SEC;
					FileTimeToTzLocalST((FILETIME*)&li, &tmTriggerLocal2);
					if ((tmTriggerLocal2.wHour * 60 + tmTriggerLocal2.wMinute) == (tmTriggerLocal.wHour * 60 + tmTriggerLocal.wMinute))
						// special case detected
						goto output_result;
				}

				// tomorrow (add 24h to local time)
				SystemTimeToFileTime(&tmTriggerLocal, (FILETIME*)&li);
				li += (ULONGLONG)(24 * 3600)*FILETIME_TICKS_PER_SEC;
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
				cmbTime.SetCurSel(0);
invalid_dst:
				MessageBox(cmbTime.GetParent()->GetHwnd(),
					TranslateT("The specified time is invalid due to begin of daylight saving (summer time)."),
					_A2W(SECTIONNAME), MB_OK | MB_ICONWARNING);
				return 1;
			}

output_result:
			if (triggerRelUtcOut)
				*triggerRelUtcOut = li;

			UINT dt = (UINT)((li / MinutesToFileTime) - (ref / MinutesToFileTime));
			if (dt < 60)
				mir_snwprintf(buf, L"%02d:%02d (%d %s)", h, m, dt, TranslateT("Minutes"));
			else
				mir_snwprintf(buf, L"%02d:%02d (%d.%d %s)", h, m, dt / 60, ((dt % 60) * 10) / 60, TranslateT("Hours"));
		}
		else {
			// absolute time (00:00 to 23:59), clean up time to make sure it's not inside "missing" hour (will be rounded downward)
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
				n = cmbTime.FindString(buf);
				if (n != -1) {
					cmbTime.SetCurSel(n);
					goto invalid_dst;
				}

				goto invalid_dst;
			}
		}

		cmbTime.SetText(buf);
		return 0;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Reminder notification dialog

class CReminderNotifyDlg : public CReminderBaseDlg
{
	typedef CReminderBaseDlg CSuper;

	REMINDERDATA *m_pReminder;

	void PopulateTimeOffsetCombo()
	{
		cmbRemindAgainIn.ResetContent();

		// 5 - 55 minutes (in 5 minute steps)
		wchar_t s[MAX_PATH];
		for (int i = 1; i < 12; i++) {
			mir_snwprintf(s, L"%d %s", i * 5, TranslateT("Minutes"));
			cmbRemindAgainIn.AddString(s, i * 5);
		}

		// 1 hour
		mir_snwprintf(s, L"1 %s", TranslateT("Hour"));
		cmbRemindAgainIn.AddString(s, 60);

		// 2, 4, 8 hours
		for (int i = 2; i <= 8; i += 2) {
			mir_snwprintf(s, L"%d %s", i, TranslateT("Hours"));
			cmbRemindAgainIn.AddString(s, i * 60);
		}

		// 1 day
		mir_snwprintf(s, L"1 %s", TranslateT("Day"));
		cmbRemindAgainIn.AddString(s, 24 * 60);

		// 2-4 days
		for (int i = 2; i <= 4; i++) {
			mir_snwprintf(s, L"%d %s", i, TranslateT("Days"));
			cmbRemindAgainIn.AddString(s, i * 24 * 60);
		}

		// 1 week
		mir_snwprintf(s, L"1 %s", TranslateT("Week"));
		cmbRemindAgainIn.AddString(s, 7 * 24 * 60);
	}

	CCtrlEdit edtText;
	CCtrlCheck chkAfter, chkOnDate;
	CCtrlCombo cmbRemindAgainIn;
	CCtrlButton btnDismiss, btnNone, btnRemindAgain;

public:
	CReminderNotifyDlg(REMINDERDATA *pReminder) :
		CSuper(IDD_NOTIFYREMINDER),
		m_pReminder(pReminder),
		btnNone(this, IDC_NONE),
		btnDismiss(this, IDC_DISMISS),
		btnRemindAgain(this, IDC_REMINDAGAIN),
		edtText(this, IDC_REMDATA),
		chkAfter(this, IDC_AFTER),
		chkOnDate(this, IDC_ONDATE),
		cmbRemindAgainIn(this, IDC_REMINDAGAININ)
	{
		m_savedLi = pReminder->When;

		chkAfter.OnChange = Callback(this, &CReminderNotifyDlg::onChange_After);
		chkOnDate.OnChange = Callback(this, &CReminderNotifyDlg::onChange_OnDate);

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
		ULONGLONG li = m_pReminder->When;
		FileTimeToSystemTime((FILETIME*)&li, &tm);

		BringWindowToTop(m_hwnd);

		PopulateTimeOffsetCombo();

		cmbRemindAgainIn.SetCurSel(0);
		chkAfter.SetState(true);
		chkOnDate.SetState(false);

		if (m_pReminder->bRepeat) {
			chkOnDate.Hide();
			chkAfter.Disable();
			cmbRemindAgainIn.Disable();
			cmbRemindAgainIn.SetCurSel(16);
		}

		edtText.SendMsg(EM_LIMITTEXT, MAX_REMINDER_LEN, 0);

		PopulateTimeCombo(&tm);
		InitDatePicker(li);

		cmbTime.SetCurSel(0);

		wchar_t S1[128], S2[MAX_PATH];
		GetTriggerTimeString(&m_pReminder->When, S1, sizeof(S1), TRUE);
		mir_snwprintf(S2, L"%s! - %s", TranslateT("Reminder"), S1);
		SetCaption(S2);

		edtText.SetText(m_pReminder->wszText);
		return true;
	}

	bool OnClose() override
	{
		if (m_pReminder)
			DeleteReminder(m_pReminder);
		JustSaveReminders();
		NotifyList();
		return true;
	}

	void onChange_After(CCtrlCheck*)
	{
		cmbRemindAgainIn.Show();
		m_date.Hide();
		cmbTime.Hide();
		ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_DATE), SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_TIME), SW_HIDE);
	}

	void onChange_OnDate(CCtrlCheck*)
	{
		if (!m_bInitialized)
			return;

		m_date.Show();
		cmbTime.Show();
		ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_DATE), SW_SHOW);
		ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_TIME), SW_SHOW);
		cmbRemindAgainIn.Hide();
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
		arReminders.remove(m_pReminder);
		if (chkAfter.GetState()) {
			// delta time
			SYSTEMTIME tm;
			GetSystemTime(&tm);

			ULONGLONG li;
			SystemTimeToFileTime(&tm, (FILETIME*)&li);

			int TT = cmbRemindAgainIn.GetCurData() * 60;
			if (TT >= 24 * 3600) {
				// selection is 1 day or more, take daylight saving boundaries into consideration
				// (ie. 24 hour might actually be 23 or 25, in order for reminder to pop up at the
				// same time tomorrow)
				SYSTEMTIME tm2, tm3;
				ULONGLONG li2 = li;
				FileTimeToTzLocalST((FILETIME*)&li2, &tm2);
				li2 += (TT * FILETIME_TICKS_PER_SEC);
				FileTimeToTzLocalST((FILETIME*)&li2, &tm3);
				if (tm2.wHour != tm3.wHour || tm2.wMinute != tm3.wMinute) {
					// boundary crossed
					// do a quick and dirty sanity check that times not more than 2 hours apart
					if (abs((int)(tm3.wHour * 60 + tm3.wMinute) - (int)(tm2.wHour * 60 + tm2.wMinute)) <= 120) {
						// adjust TT so that same HH:MM is set
						tm3.wHour = tm2.wHour;
						tm3.wMinute = tm2.wMinute;
						TzLocalSTToFileTime(&tm3, (FILETIME*)&li2);
						TT = (li2 - li) / FILETIME_TICKS_PER_SEC;
					}
				}
			}

			// reset When from the current time
			if (!m_pReminder->bRepeat)
				m_pReminder->When = li;
			m_pReminder->When += (TT * FILETIME_TICKS_PER_SEC);
		}
		else if (chkOnDate.GetState()) {
			SYSTEMTIME Date;
			m_date.GetTime(&Date);
			if (!GetTriggerTime(m_savedLi, Date))
				return;

			SystemTimeToFileTime(&Date, (FILETIME*)&m_pReminder->When);
		}

		// update reminder text
		m_pReminder->wszText = ptrW(edtText.GetText());
		m_pReminder->bVisible = false;
		m_pReminder->handle = nullptr;

		// re-insert tree item sorted
		InsertReminder(m_pReminder);
		m_pReminder = nullptr; // prevent reminder from being deleted;
		Close();
	}

	void onClick_None(CCtrlButton*)
	{
		// create note from reminder
		NewNote(0, 0, -1, -1, ptrW(edtText.GetText()), nullptr, TRUE, TRUE, 0);
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

class CReminderFormDlg : public CReminderBaseDlg
{
	typedef CReminderBaseDlg CSuper;

	REMINDERDATA *m_pReminder;

	CCtrlEdit edtText;
	CCtrlCheck chkRepeat;
	CCtrlCombo cmbSound, cmbRepeat;
	CCtrlButton btnAdd, btnView, btnPlaySound;

public:
	CReminderFormDlg(REMINDERDATA *pReminder = nullptr) :
		CSuper(IDD_ADDREMINDER),
		m_pReminder(pReminder),
		btnAdd(this, IDC_ADDREMINDER),
		btnView(this, IDC_VIEWREMINDERS),
		btnPlaySound(this, IDC_BTN_PLAYSOUND),
		edtText(this, IDC_REMINDER),
		chkRepeat(this, IDC_CHECK_REPEAT),
		cmbSound(this, IDC_COMBO_SOUND),
		cmbRepeat(this, IDC_COMBO_REPEATSND)
	{
		btnAdd.OnClick = Callback(this, &CReminderFormDlg::onClick_Add);
		btnView.OnClick = Callback(this, &CReminderFormDlg::onClick_View);
		btnPlaySound.OnClick = Callback(this, &CReminderFormDlg::onClick_PlaySound);

		cmbSound.OnChange = Callback(this, &CReminderFormDlg::onChange_Sound);
	}

	bool OnInitDialog() override
	{
		SYSTEMTIME tm;

		if (m_pReminder) {
			// opening the edit reminder dialog (uses same dialog resource as add reminder)
			SetWindowText(m_hwnd, TranslateT("Reminder"));
			SetDlgItemText(m_hwnd, IDC_ADDREMINDER, TranslateT("&Update Reminder"));
			btnView.Hide();

			m_savedLi = m_pReminder->When;
			FileTimeToSystemTime((FILETIME*)&m_savedLi, &tm);
		}
		else {
			GetSystemTime(&tm);
			SystemTimeToFileTime(&tm, (FILETIME*)&m_savedLi);
		}

		PopulateTimeCombo(&tm);
		InitDatePicker(m_savedLi);

		edtText.SendMsg(EM_LIMITTEXT, MAX_REMINDER_LEN, 0);

		wchar_t s[64];
		if (m_pReminder) {
			mir_snwprintf(s, L"%02d:%02d", tm.wHour, tm.wMinute);

			// search for preset first
			int n = cmbTime.FindString(s);
			if (n != -1)
				cmbTime.SetCurSel(n);
			else
				cmbTime.SetText(s);

			edtText.SetText(m_pReminder->wszText);
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
			cmbRepeat.SetCurSel(cmbRepeat.FindString(s, -1, true));
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
			SetFocus(edtText.GetHwnd());
		return true;
	}

	void OnDestroy() override
	{
		bNewReminderVisible = false;
		if (m_pReminder)
			m_pReminder->bVisible = false;
	}

	void onClick_Add(CCtrlButton*)
	{
		SYSTEMTIME Date;
		m_date.GetTime(&Date);
		if (!GetTriggerTime(m_savedLi, Date))
			return;

		int RepeatSound = cmbRepeat.GetCurData();
		if (RepeatSound == -1)
			RepeatSound = 0;

		bool bClose = g_plugin.bCloseAfterAddReminder || m_pReminder;
		if (!m_pReminder) {
			// new reminder
			REMINDERDATA *TempRem = new REMINDERDATA();
			TempRem->uid = CreateUid();
			SystemTimeToFileTime(&Date, (FILETIME*)&TempRem->When);
			TempRem->wszText = ptrW(edtText.GetText());
			TempRem->bRepeat = chkRepeat.GetState();
			TempRem->SoundSel = cmbSound.GetCurData();
			TempRem->RepeatSound = TempRem->SoundSel < 0 ? 0 : (UINT)RepeatSound;
			InsertReminder(TempRem);
		}
		else {
			// update existing reminder
			arReminders.remove(m_pReminder);
			SystemTimeToFileTime(&Date, (FILETIME*)&m_pReminder->When);

			m_pReminder->wszText = ptrW(edtText.GetText());
			m_pReminder->bRepeat = chkRepeat.GetState();
			m_pReminder->SoundSel = cmbSound.GetCurData();
			m_pReminder->RepeatSound = m_pReminder->SoundSel < 0 ? 0 : (UINT)RepeatSound;

			// re-insert tree item sorted
			InsertReminder(m_pReminder);

			m_pReminder->bVisible = false;
			m_pReminder = nullptr; // prevent new reminder from being deleted
		}
		
		edtText.SetTextA("");
		JustSaveReminders();
		NotifyList();
		if (bClose)
			Close();
	}

	void onClick_View(CCtrlButton*)
	{
		PluginMenuCommandViewReminders(0, 0);
	}

	void onClick_PlaySound(CCtrlButton*)
	{
		int n = cmbSound.GetCurData();
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

	void onChange_Sound(CCtrlCombo*)
	{
		int n = cmbSound.GetCurData();
		btnPlaySound.Enable(n >= 0);
		cmbRepeat.Enable(n >= 0);
	}
};

static void EditReminder(REMINDERDATA *p)
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

class CReminderListDlg : public CDlgBase
{
	typedef CDlgBase CSuper;

	CMStringW m_wszFilter;

	REMINDERDATA* getData(int idx)
	{
		return (REMINDERDATA*)m_list.GetItemData(idx);
	}

	void RefreshList()
	{
		m_list.DeleteAllItems();

		int i = 0;
		for (auto &pReminder : arReminders) {
			if (!m_wszFilter.IsEmpty())
				if (pReminder->wszText.Find(m_wszFilter, 0) == -1)
					continue;

			LV_ITEM lvTIt;
			lvTIt.mask = LVIF_TEXT | LVIF_PARAM;

			wchar_t S1[128];
			GetTriggerTimeString(&pReminder->When, S1, _countof(S1), TRUE);

			lvTIt.iItem = i;
			lvTIt.iSubItem = 0;
			lvTIt.pszText = S1;
			lvTIt.lParam = LPARAM(pReminder);
			m_list.InsertItem(&lvTIt);
			lvTIt.mask = LVIF_TEXT;

			wchar_t *S2 = GetPreviewString(pReminder->wszText);
			lvTIt.iItem = i;
			lvTIt.iSubItem = 1;
			lvTIt.pszText = S2;
			m_list.SetItem(&lvTIt);

			i++;
		}

		if (i > 0) {
			m_list.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
			SetDlgItemTextW(m_hwnd, IDC_REMINDERDATA, getData(0)->wszText);
		}
	}

	CCtrlEdit edtFilter;
	CCtrlButton btnNew;
	CCtrlListView m_list;

public:
	CReminderListDlg() :
		CSuper(g_plugin, IDD_LISTREMINDERS),
		m_list(this, IDC_LISTREMINDERS),
		btnNew(this, IDC_ADDNEWREMINDER),
		edtFilter(this, IDC_FILTER)
	{
		SetMinSize(394, 300);

		btnNew.OnClick = Callback(this, &CReminderListDlg::onClick_New);

		m_list.OnBuildMenu = Callback(this, &CReminderListDlg::onContextMenu);
		m_list.OnDoubleClick = Callback(this, &CReminderListDlg::list_onDblClick);
		m_list.OnItemChanged = Callback(this, &CReminderListDlg::list_onItemChanged);

		edtFilter.OnChange = Callback(this, &CReminderListDlg::onChange_Filter);
	}

	bool OnInitDialog() override
	{
		pListDialog = this;
		Window_SetIcon_IcoLib(m_hwnd, iconList[6].hIcolib);

		TranslateDialogDefault(m_hwnd);
		SetDlgItemTextA(m_hwnd, IDC_REMINDERDATA, "");

		int cx1 = 150, cx2 = 205;
		ptrA colWidth(g_plugin.getStringA("ColWidth"));
		if (colWidth != 0) {
			int tmp1 = 0, tmp2 = 0;
			if (2 == sscanf(colWidth, "%d,%d", &tmp1, &tmp2))
				cx1 = tmp1, cx2 = tmp2;
		}

		LV_COLUMN lvCol;
		lvCol.mask = LVCF_TEXT | LVCF_WIDTH;

		lvCol.pszText = TranslateT("Date of activation");
		lvCol.cx = cx1;
		m_list.InsertColumn(0, &lvCol);

		lvCol.pszText = TranslateT("Reminder text");
		lvCol.cx = cx2;
		m_list.InsertColumn(1, &lvCol);

		m_list.SetHoverTime(700);
		m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
		RefreshList();

		SetWindowLongPtr(GetDlgItem(m_list.GetHwnd(), 0), GWL_ID, IDC_LISTREMINDERS_HEADER);

		Utils_RestoreWindowPosition(m_hwnd, 0, MODULENAME, "ListReminders");
		return true;
	}

	void OnDestroy() override
	{
		int cx1 = m_list.GetColumnWidth(0);
		int cx2 = m_list.GetColumnWidth(1);
		g_plugin.setString("ColWidth", CMStringA(FORMAT, "%d,%d", cx1, cx2));

		Utils_SaveWindowPosition(m_hwnd, 0, MODULENAME, "ListReminders");
		Window_FreeIcon_IcoLib(m_hwnd);
		pListDialog = nullptr;
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_LISTREMINDERS:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

		case IDC_REMINDERDATA:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM;

		case IDC_FILTER:
			return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;
		}
		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
	}

	void onContextMenu(CCtrlListView*)
	{
		HMENU hMenuLoad = LoadMenuA(g_plugin.getInst(), "MNU_REMINDERPOPUP");
		HMENU FhMenu = GetSubMenu(hMenuLoad, 0);

		int idx = m_list.GetSelectionMark();
		REMINDERDATA *pReminder = (idx == -1) ? nullptr : getData(idx);

		MENUITEMINFO mii = {};
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_STATE;
		mii.fState = MFS_DEFAULT;
		if (!pReminder || pReminder->bSystemEventQueued)
			mii.fState |= MFS_GRAYED;
		SetMenuItemInfo(FhMenu, ID_CONTEXTMENUREMINDER_EDIT, FALSE, &mii);

		if (!pReminder)
			EnableMenuItem(FhMenu, ID_CONTEXTMENUREMINDER_DELETE, MF_GRAYED | MF_BYCOMMAND);

		RECT rc;
		m_list.GetItemRect(idx, &rc, LVIR_LABEL);
		POINT pt = { rc.left, rc.bottom };
		ClientToScreen(m_list.GetHwnd(), &pt);

		TranslateMenu(FhMenu);
		int iSel = TrackPopupMenu(FhMenu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hwnd, nullptr);
		DestroyMenu(hMenuLoad);

		switch(iSel) {
		case ID_CONTEXTMENUREMINDER_EDIT:
			idx = m_list.GetSelectionMark();
			if (idx != -1)
				EditReminder(getData(idx));
			break;

		case ID_CONTEXTMENUREMINDER_NEW:
			PluginMenuCommandNewReminder(0, 0);
			break;

		case ID_CONTEXTMENUREMINDER_DELETEALL:
			if (arReminders.getCount() && IDOK == MessageBox(m_hwnd, TranslateT("Are you sure you want to delete all reminders?"), _A2W(SECTIONNAME), MB_OKCANCEL)) {
				SetDlgItemTextA(m_hwnd, IDC_REMINDERDATA, "");
				DeleteReminders();
				RefreshList();
			}
			break;

		case ID_CONTEXTMENUREMINDER_DELETE:
			idx = m_list.GetSelectionMark();
			if (idx != -1 && IDOK == MessageBox(m_hwnd, TranslateT("Are you sure you want to delete this reminder?"), _A2W(SECTIONNAME), MB_OKCANCEL)) {
				SetDlgItemTextA(m_hwnd, IDC_REMINDERDATA, "");
				DeleteReminder(getData(idx));
				JustSaveReminders();
				RefreshList();
			}
			break;
		}
	}

	void Reload()
	{
		SetDlgItemTextA(m_hwnd, IDC_REMINDERDATA, "");
		RefreshList();
	}

	void list_onItemChanged(CCtrlListView::TEventInfo *ev)
	{
		SetDlgItemTextW(m_hwnd, IDC_REMINDERDATA, getData(ev->nmlv->iItem)->wszText);
	}

	void list_onDblClick(CCtrlListView::TEventInfo*)
	{
		int i = m_list.GetSelectionMark();
		if (i != -1)
			EditReminder(getData(i));
	}

	void onClick_New(CCtrlButton *)
	{
		PluginMenuCommandNewReminder(0, 0);
	}

	void onChange_Filter(CCtrlEdit *)
	{
		m_wszFilter = ptrW(edtFilter.GetText());
		RefreshList();
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		RECT rc;

		switch (msg) {
		case WM_NOTIFY:
			if (wParam == IDC_LISTREMINDERS_HEADER) {
				LPNMHEADER NM = (LPNMHEADER)lParam;
				if (NM->hdr.code == HDN_ENDTRACK) {
					GetWindowRect(m_list.GetHwnd(), &rc);
					m_list.SetColumnWidth(1, rc.right - rc.left - m_list.GetColumnWidth(0) - 4);
				}
			}
			break;

		case WM_SIZE:
			CSuper::DlgProc(msg, wParam, lParam);

			GetWindowRect(m_list.GetHwnd(), &rc);
			int nWidth = rc.right - rc.left - m_list.GetColumnWidth(0) - 4;
			if (GetWindowLong(m_list.GetHwnd(), GWL_STYLE) & WS_VSCROLL)
				nWidth -= GetSystemMetrics(SM_CXVSCROLL);
			m_list.SetColumnWidth(1, nWidth);
			return 0;
		}

		return CSuper::DlgProc(msg, wParam, lParam);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

bool CheckRemindersAndStart(void)
{
	// returns TRUE if there are any triggered reminder with bSystemEventQueued, this will shorten the update interval
	// allowing sound repeats with shorter intervals

	if (!arReminders.getCount())
		return false;

	ULONGLONG curT;
	SYSTEMTIME tm;
	GetSystemTime(&tm);
	SystemTimeToFileTime(&tm, (FILETIME *)&curT);

	// NOTE: reminder list is sorted by trigger time, so we can early out on the first reminder > cur time
	// quick check for normal case with no reminder ready to be triggered and no queued triggered reminders
	// (happens 99.99999999999% of the time)
	if (curT < arReminders[0]->When && !QueuedReminderCount)
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
		if (!bHasQueuedReminders && pReminder->When > curT)
			break;

		if (pReminder->bVisible)
			continue;

		if (pReminder->bSystemEventQueued) {
			UpdateReminderEvent(pReminder, REMINDER_UPDATE_INTERVAL_SHORT / 1000, &bHasPlayedSound);

			QueuedReminderCount++;
			bResult = true;
		}
		else if (pReminder->When <= curT) {
			if (!mir_strlen(g_RemindSMS)) {
				FireReminder(pReminder, &bHasPlayedSound);

				if (pReminder->bSystemEventQueued)
					bResult = true;
			}
			else {
				char *p = strchr(g_RemindSMS, '@');
				if (p) {
					Send(g_RemindSMS, p + 1, _T2A(pReminder->wszText), NULL);
					*p = '@';

					DeleteReminder(pReminder);
					JustSaveReminders();
					if (pListDialog)
						pListDialog->Reload();
				}
			}
		}
	}

	return bResult;
}

void CloseReminderList()
{
	if (pListDialog)
		pListDialog->Close();
}

static void NotifyList()
{
	if (pListDialog)
		pListDialog->Reload();
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
	if (!pListDialog)
		(new CReminderListDlg())->Show();
	else 
		BringWindowToTop(pListDialog->GetHwnd());
	return 0;
}

INT_PTR PluginMenuCommandDeleteReminders(WPARAM, LPARAM)
{
	if (arReminders.getCount())
		if (IDOK == MessageBox(nullptr, TranslateT("Are you sure you want to delete all reminders?"), TranslateT(SECTIONNAME), MB_OKCANCEL))
			DeleteReminders();
	return 0;
}
