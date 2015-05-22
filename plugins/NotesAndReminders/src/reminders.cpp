#include "globals.h"

#define FILETIME_TICKS_PER_SEC ((ULONGLONG)10000000)

#define MAX_REMINDER_LEN	16384


// RemindersData DB data params
#define DATATAG_TEXT		1	// %s
#define DATATAG_SNDREPEAT	2	// %u (specifies seconds to wait between sound repeats, 0 if repeat is disabled)
#define DATATAG_SNDSEL		3	// %d (which sound to use, default, alt1, alt2, -1 means no sound at all)


#define IDC_DATE 1000
#define IDC_TIME IDC_COMBOREMINDERTIME
#define IDC_ADDREMINDER 1002
#define IDC_CLOSE 1003
#define IDC_REMINDER 1004
#define IDC_LISTREMINDERS 1000
#define IDC_LISTREMINDERS_HEADER 2000
#define IDC_REMINDERDATA 1001
#define IDC_ADDNEWREMINDER 1002
#define IDC_REMDATA 1000
#define IDC_DISMISS 1001
#define IDC_REMINDAGAIN 1002
#define IDC_REMINDAGAININ 1003
#define IDC_AFTER 1004
#define IDC_ONDATE 1005
#define IDC_DATEAGAIN 1006
#define IDC_TIMEAGAIN 1007
#define IDC_VIEWREMINDERS 1007
#define IDC_NONE 1008
#define IDC_DAILY 1009
#define IDC_WEEKLY 1010
#define IDC_MONTHLY 1011
#define IDM_NEWREMINDER 40001
#define IDM_DELETEREMINDER 40002
#define IDM_DELETEALLREMINDERS 40003
#define WM_RELOAD (WM_USER + 100)

#define NOTIFY_LIST() if (ListReminderVisible) PostMessage(LV,WM_RELOAD,0,0)


TREEELEMENT *RemindersList = NULL;
static UINT QueuedReminderCount = 0;
static BOOL ListReminderVisible = FALSE;
static BOOL NewReminderVisible = FALSE;
static REMINDERDATA *pEditReminder = NULL;
static HWND LV;
static SOCKET S;

int WS_Send(SOCKET s,char *data,int datalen);
unsigned long WS_ResolveName(char *name,WORD *port,int defaultPort);

INT_PTR CALLBACK DlgProcNotifyReminder(HWND Dialog,UINT Message,
								   WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK DlgProcNewReminder(HWND Dialog,UINT Message,WPARAM wParam,
								LPARAM lParam);
INT_PTR CALLBACK DlgProcViewReminders(HWND Dialog,UINT Message,WPARAM wParam,
								  LPARAM lParam);

void Send(char *user, char *host, char *Msg, char* server);
char* GetPreviewString(const char *lpsz);


static int ReminderSortCb(TREEELEMENT *v1, TREEELEMENT *v2)
{
	return (((REMINDERDATA*)v1->ptrdata)->When.QuadPart < ((REMINDERDATA*)v2->ptrdata)->When.QuadPart) ? -1 : 1;
}

// time convertsion routines that take local time-zone specific daylight saving configuration into account
// (unlike the standard FileTimeToLocalFileTime functions)

void UtcToTzLocalFT(const FILETIME *lpUtc, FILETIME *lpLocal)
{
	SYSTEMTIME tm, tmLocal;
	FILETIMEtoSYSTEMTIME(lpUtc, &tm);
	SystemTimeToTzSpecificLocalTime(NULL, &tm, &tmLocal);
	SYSTEMTIMEtoFILETIME(&tmLocal, lpLocal);
}

void TzLocalToUtcFT(const FILETIME *lpLocal, FILETIME *lpUtc)
{
	SYSTEMTIME tm, tmUtc;
	FILETIMEtoSYSTEMTIME(lpLocal, &tm);
	TzSpecificLocalTimeToSystemTime(NULL, &tm, &tmUtc);
	SYSTEMTIMEtoFILETIME(&tmUtc, lpUtc);
}

void FileTimeToTzLocalST(const FILETIME *lpUtc, SYSTEMTIME *tmLocal)
{
	SYSTEMTIME tm;
	FILETIMEtoSYSTEMTIME(lpUtc, &tm);
	SystemTimeToTzSpecificLocalTime(NULL, &tm, tmLocal);
}

void TzLocalSTToFileTime(const SYSTEMTIME *tmLocal, FILETIME *lpUtc)
{
	SYSTEMTIME tm;
	TzSpecificLocalTimeToSystemTime(NULL, (SYSTEMTIME*)tmLocal, &tm);
	SYSTEMTIMEtoFILETIME(&tm, lpUtc);
}

/*void AddToTzLocalFT(FILETIME *lpLocal, UINT nSeconds)
{
	ULARGE_INTEGER utc;
	TzLocalToUtcFT(lpLocal, (FILETIME*)&utc);
	utc.QuadPart += (ULONGLONG)nSeconds * FILETIME_TICKS_PER_SEC;
	UtcToTzLocalFT((FILETIME*)&utc, lpLocal);
}*/

/*static void AddToSystemTime(SYSTEMTIME *tm, UINT nSeconds)
{
	ULARGE_INTEGER li;
	FILETIME ft;

	SYSTEMTIMEtoFILETIME(tm, &ft);

	li.HighPart = ft.dwHighDateTime;
	li.LowPart = ft.dwLowDateTime;
	li.QuadPart += (ULONGLONG)nSeconds * FILETIME_TICKS_PER_SEC;

	FILETIMEtoSYSTEMTIME((FILETIME*)&li, tm);
}*/


static DWORD CreateUid()
{
	DWORD uid;
	TREEELEMENT *TTE;

	if (!RemindersList)
		return 1;

	for (uid = 1; ; uid++)
	{
		// check existing reminders if uid is in use
		TTE = RemindersList;
		while (TTE)
		{
			if (((REMINDERDATA*)TTE->ptrdata)->uid == uid)
				// uid in use
				goto try_next;

			TTE = (TREEELEMENT*)TTE->next;
		}

		return uid;

try_next:;
	}

	// should never get here (unless someone has 4294967295 reminders)
	return 0;
}

static REMINDERDATA* FindReminder(DWORD uid)
{
	TREEELEMENT *TTE;

	if (!RemindersList)
		return NULL;

	TTE = RemindersList;
	while (TTE)
	{
		REMINDERDATA *pReminder = (REMINDERDATA*)TTE->ptrdata;

		if (pReminder->uid == uid)
		{
			return pReminder;
		}

		TTE = (TREEELEMENT*)TTE->next;
	}

	return NULL;
}


static void RemoveReminderSystemEvent(REMINDERDATA *p)
{
	if (p->SystemEventQueued)
	{
		int i;

		for (i=0; ; i++)
		{
			CLISTEVENT *pev;

			pev = (CLISTEVENT*) CallService(MS_CLIST_GETEVENT,(WPARAM)INVALID_HANDLE_VALUE,i);
			if (!pev)
				break;

			if ((ULONG)pev->lParam == p->uid && !pev->hContact
				&& pev->pszService && !mir_strcmp(pev->pszService, MODULENAME"/OpenTriggeredReminder"))
			{
				if ( !CallService(MS_CLIST_REMOVEEVENT,(WPARAM)pev->hContact,(LPARAM)pev->hDbEvent) )
				{
					p->SystemEventQueued = FALSE;
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
	int ReminderCount,I;
	char ValueName[32];

	ReminderCount = db_get_dw(0,MODULENAME,"RemindersData",0);
	for(I = 0;I < ReminderCount;I++)
	{
		mir_snprintf(ValueName, SIZEOF(ValueName), "RemindersData%d", I);
		db_unset(0, MODULENAME, ValueName);
	}
}

void JustSaveReminders(void)
{
	TREEELEMENT *TTE;
	int I, n, l;
	char *tmpReminder = NULL,*Value;
	char ValueName[32];
	int ReminderCount;
	REMINDERDATA *pReminder;

	const int OldReminderCount = db_get_dw(0, MODULENAME, "RemindersData", 0);

	ReminderCount = TreeGetCount(RemindersList);

	db_set_dw(0,MODULENAME, "RemindersData", ReminderCount);

	for (TTE = RemindersList, I = 0; TTE; TTE = (TREEELEMENT*)TTE->next, I++)
	{
		pReminder = (REMINDERDATA*)TTE->ptrdata;
		if (pReminder->Reminder && mir_strlen(pReminder->Reminder))
			tmpReminder = pReminder->Reminder;
		else
			tmpReminder = NULL;

		if (!tmpReminder)
			tmpReminder = "";

		Value = (char*)malloc(mir_strlen(tmpReminder) + 512);

		if (!Value)
			continue;

		n = 0;

		// data header (save 'When' with 1-second resolution, it's just a waste to have 100-nanosecond resolution
		// which results in larger DB strings with no use)
		l = sprintf(Value, "X%u:%I64x", pReminder->uid, pReminder->When.QuadPart/FILETIME_TICKS_PER_SEC); //!!!!!!!!!
		if (l > 0) n += l;

		// sound repeat
		if (pReminder->RepeatSound)
		{
			l = sprintf(Value+n, "\033""%u:%u", DATATAG_SNDREPEAT, pReminder->RepeatSound); //!!!!!!!!!!
			if (l > 0) n += l;
		}

		// sound
		if (pReminder->SoundSel)
		{
			l = sprintf(Value+n, "\033""%u:%d", DATATAG_SNDSEL, pReminder->SoundSel); //!!!!!!!!!
			if (l > 0) n += l;
		}

		// reminder text/note (ALWAYS PUT THIS PARAM LAST)
		if (tmpReminder && *tmpReminder)
		{
			l = sprintf(Value+n, "\033""%u:%s", DATATAG_TEXT, tmpReminder); //!!!!!!!!!!!
			if (l > 0) n += l;
		}

		// clamp data size to WORD (including null terminator)
		if (n >= 0xffff)
		{
			// huston, we have a problem, strip some reminder text
			n = 0xfffe;
			Value[0xffff] = 0;
		}

		mir_snprintf(ValueName, SIZEOF(ValueName), "RemindersData%d", ReminderCount - I - 1); // do not want to reverse in DB

		db_set_blob(0, MODULENAME, ValueName, Value, n+1);

		SAFE_FREE((void**)&Value);
	}

	// delete any left over DB reminder entries
	for(; I < OldReminderCount; I++)
	{
		mir_snprintf(ValueName, SIZEOF(ValueName), "RemindersData%d", I);
		db_unset(0, MODULENAME, ValueName);
	}
}

void LoadReminders(void)
{
	int I,RemindersCount;
	char *Value;
	WORD Size;
	char ValueName[32];
	BOOL GenerateUids = FALSE;

	RemindersList = NULL;
	RemindersCount = db_get_dw(0, MODULENAME, "RemindersData", 0);

	for (I = 0; I < RemindersCount; I++)
	{
		Size = 65535;
		Value = NULL;
		mir_snprintf(ValueName, SIZEOF(ValueName), "RemindersData%d", I);

		ReadSettingBlob(0, MODULENAME, ValueName, &Size, (void**)&Value);

		if (Size && Value) // was the blob found
		{
			REMINDERDATA rem = {0};
			char *TVal;
			REMINDERDATA *TempRem;
			char *DelPos = strchr(Value, 0x1B);

			// ensure that read data is null-terminated
			Value[(UINT)Size-1] = 0;

			if (Value[0] == 'X')
			{
				// new eXtended/fleXible data format

				if (DelPos)
					*DelPos = 0;

				// uid:when

				TVal = strchr(Value+1, ':');
				if (!TVal || (DelPos && TVal > DelPos))
					continue;
				*TVal++ = 0;

				rem.uid = strtoul(Value+1, NULL, 10);
				rem.When.QuadPart = _strtoui64(TVal, NULL, 16) * FILETIME_TICKS_PER_SEC;

				// optional \033 separated params
				while (DelPos)
				{
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

					tag = strtoul(TVal, NULL, 10);
					TVal = sep + 1;

					switch (tag)
					{
					case DATATAG_TEXT:
						rem.Reminder = _strdup(TVal);
						break;

					case DATATAG_SNDREPEAT:
						rem.RepeatSound = strtoul(TVal, NULL, 10);
						break;

					case DATATAG_SNDSEL:
						rem.SoundSel = strtol(TVal, NULL, 10);
						if (rem.SoundSel > 2) rem.SoundSel = 2;
						break;
					}
				}

				if (rem.SoundSel < 0)
					rem.RepeatSound = 0;
				if (!rem.Reminder)
					rem.Reminder = _strdup("");
			}
			else
			{
				// old format (for DB backward compatibility)

				if (!DelPos)
					continue;

				DelPos[0] = 0;
				// convert time_t to (local) FILETIME
				{
				SYSTEMTIME tm;
				struct tm *stm;
				time_t tt;

				tt = (time_t)strtoul(Value, NULL, 10);
				stm = localtime(&tt);
				tm.wDayOfWeek = 0;
				tm.wSecond = 0;
				tm.wMilliseconds = 0;
				tm.wHour = stm->tm_hour;
				tm.wMinute = stm->tm_min;
				tm.wSecond = stm->tm_sec;
				tm.wYear = stm->tm_year + 1900;
				tm.wMonth = stm->tm_mon + 1;
				tm.wDay = stm->tm_mday;
				SYSTEMTIMEtoFILETIME(&tm, (FILETIME*)&rem.When);
				}
				TVal = DelPos + 1;
				rem.Reminder = _strdup(TVal);
			}

			// queue uid generation if invalid uid is present
			if (!rem.uid)
				GenerateUids = TRUE;

			TempRem = (REMINDERDATA*)malloc(sizeof(REMINDERDATA));
			if (TempRem)
			{
				*TempRem = rem;
				TreeAddSorted(&RemindersList, TempRem, ReminderSortCb);
			}
			else if (rem.Reminder)
			{
				free(rem.Reminder);
			}
skip:;
		}

		FreeSettingBlob(Size, Value);
	}

	// generate UIDs if there are any items with an invalid UID
	if (GenerateUids && RemindersList)
	{
		TREEELEMENT *TTE;

		TTE = RemindersList;
		while (TTE)
		{
			REMINDERDATA *pReminder = (REMINDERDATA*)TTE->ptrdata;

			if (!pReminder->uid)
				pReminder->uid = CreateUid();

			TTE = (TREEELEMENT*)TTE->next;
		}

		JustSaveReminders();
	}
}

void NewReminder(void)
{
	if (!NewReminderVisible)
	{
		NewReminderVisible = TRUE;
		CreateDialog(hinstance, MAKEINTRESOURCE(IDD_ADDREMINDER), 0, DlgProcNewReminder);
	}
}

void EditReminder(REMINDERDATA *p)
{
	if (!p)
		return;

	if (!NewReminderVisible && !p->SystemEventQueued)
	{
		if (!p->RemVisible)
		{
			p->RemVisible = TRUE;
			NewReminderVisible = 2;
			pEditReminder = p;
			CreateDialog(hinstance, MAKEINTRESOURCE(IDD_ADDREMINDER), 0, DlgProcNewReminder);
		}
		else
		{
			BringWindowToTop(p->handle);
		}
	}
}

static void DeleteReminder(REMINDERDATA *p)
{
	if (!p)
		return;

	if (p->SystemEventQueued)
	{
		// remove pending system event
		RemoveReminderSystemEvent(p);
	}

	TreeDelete(&RemindersList, p);
	SAFE_FREE((void**)&p->Reminder);
	SAFE_FREE((void**)&p);
}

void CloseReminderList()
{
	if (ListReminderVisible)
	{
		DestroyWindow(LV);
		ListReminderVisible = FALSE;
	}
}

static void PurgeReminderTree()
{
	REMINDERDATA *pt;

	while (RemindersList) // empty whole tree
	{
		pt = (REMINDERDATA*)RemindersList->ptrdata;
		if (pt->handle) DestroyWindow(pt->handle);
		DeleteReminder(pt);
	}
	RemindersList = NULL;
}

void SaveReminders(void)
{
	JustSaveReminders();
	PurgeReminderTree();
}

void DeleteReminders(void)
{
	PurgeReminders();
	db_set_dw(0,MODULENAME,"RemindersData",0);
	PurgeReminderTree();
}

void ListReminders(void)
{
	if (!ListReminderVisible)
	{
		CreateDialog(hinstance, MAKEINTRESOURCE(IDD_LISTREMINDERS), 0, DlgProcViewReminders);
		ListReminderVisible = TRUE;
	}
	else
	{
		BringWindowToTop(LV);
	}
}


void GetTriggerTimeString(const ULARGE_INTEGER *When, char *s, UINT strSize, BOOL bUtc)
{
	SYSTEMTIME tm = { 0 };
	LCID lc = GetUserDefaultLCID();

	*s = 0;

	if (bUtc)
		FileTimeToTzLocalST((const FILETIME*)When, &tm);
	else
		FILETIMEtoSYSTEMTIME((FILETIME*)When, &tm);

	if ( GetDateFormat(lc, DATE_LONGDATE, &tm, NULL, s, strSize)) {
		// append time
		int n = (int)mir_strlen(s);
		s[n++] = ' ';
		s[n] = 0;

		if ( !GetTimeFormat(lc, LOCALE_NOUSEROVERRIDE|TIME_NOSECONDS, &tm, NULL, s+n, strSize-n) )
			mir_snprintf(s+n, strSize-n, "%02d:%02d", tm.wHour, tm.wMinute);
	}
	else mir_snprintf(s, strSize, "%d-%02d-%02d %02d:%02d", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute);
}


INT_PTR OpenTriggeredReminder(WPARAM w, LPARAM l)
{
	if (!l)
		return 0;

	l = ((CLISTEVENT*)l)->lParam;

	REMINDERDATA *pReminder = (REMINDERDATA*)FindReminder((DWORD)l);
	if (!pReminder || !pReminder->SystemEventQueued)
		return 0;

	pReminder->SystemEventQueued = FALSE;
	if (QueuedReminderCount)
		QueuedReminderCount--;

	{
	char S[MAX_PATH];
	char S1[128];
	HWND H;
	GetTriggerTimeString(&pReminder->When, S1, sizeof(S1), TRUE);

	pReminder->RemVisible = TRUE;

	pReminder->handle = H = CreateDialog(hinstance, MAKEINTRESOURCE(IDD_NOTIFYREMINDER), 0, DlgProcNotifyReminder);

	mir_snprintf(S, SIZEOF(S), "%s! - %s", Translate("Reminder"), S1);
	SetWindowText(H, S);

	if (pReminder->Reminder)
		SetDlgItemText(H, IDC_REMDATA, pReminder->Reminder);

	BringWindowToTop(H);
	}

	return 0;
}

static void SkinPlaySoundPoly(LPCSTR pszSoundName)
{
	if (g_UseDefaultPlaySound)
	{
		SkinPlaySound(pszSoundName);
		return;
	}

	if (db_get_b(NULL, "SkinSoundsOff", pszSoundName, 0)==0) {
		DBVARIANT dbv;

		if (db_get_s(NULL, "SkinSounds", pszSoundName, &dbv)==0) {
			char szFull[MAX_PATH];

			PathToAbsolute(dbv.pszVal, szFull);

			//NotifyEventHooks(hPlayEvent, 0, (LPARAM)szFull);
			{
				// use MCI device which allows multiple sounds playing at once
				// NOTE: mciSendString does not like long paths names, must convert to short
				char szShort[MAX_PATH];
				char s[512];
				GetShortPathNameA(szFull, szShort, sizeof(szShort));
				mir_snprintf(s, SIZEOF(s), "play \"%s\"", szShort);
				mciSendStringA(s, NULL, 0, NULL);
			}

			db_free(&dbv);
		}
	}
}

static void UpdateReminderEvent(REMINDERDATA *pReminder, UINT nElapsedSeconds, BOOL *pHasPlayedSound)
{
	DWORD dwSoundMask;

	if (pReminder->RepeatSound)
	{
		if (nElapsedSeconds >= pReminder->RepeatSoundTTL)
		{
			pReminder->RepeatSoundTTL = pReminder->RepeatSound;

			dwSoundMask = 1 << pReminder->SoundSel;

			if ( !(*pHasPlayedSound & dwSoundMask) )
			{
				switch (pReminder->SoundSel)
				{
				case 1: SkinPlaySoundPoly("AlertReminder2"); break;
				case 2: SkinPlaySoundPoly("AlertReminder3"); break;
				default:
					SkinPlaySoundPoly("AlertReminder");
				}

				*pHasPlayedSound |= dwSoundMask;
			}
		}
		else
		{
			pReminder->RepeatSoundTTL -= nElapsedSeconds;
		}
	}
}

static void FireReminder(REMINDERDATA *pReminder, BOOL *pHasPlayedSound)
{
	DWORD dwSoundMask;

	if (pReminder->SystemEventQueued)
		return;

	// add a system event
	{
	CLISTEVENT ev = { 0 };

	ev.cbSize = sizeof(ev);
	ev.hIcon = g_hReminderIcon;
	ev.flags = CLEF_URGENT;
	ev.lParam = (LPARAM)pReminder->uid;
	ev.pszService = MODULENAME"/OpenTriggeredReminder";
	ev.pszTooltip = Translate("Reminder");

	CallService(MS_CLIST_ADDEVENT,0,(LPARAM)&ev);
	}

	pReminder->SystemEventQueued = TRUE;
	QueuedReminderCount++;

	if (pReminder->SoundSel < 0)
	{
		// sound disabled
		return;
	}

	dwSoundMask = 1 << pReminder->SoundSel;

	pReminder->RepeatSoundTTL = pReminder->RepeatSound;

	if ( !(*pHasPlayedSound & dwSoundMask) )
	{
		switch (pReminder->SoundSel)
		{
		case 1: SkinPlaySoundPoly("AlertReminder2"); break;
		case 2: SkinPlaySoundPoly("AlertReminder3"); break;
		default:
			SkinPlaySoundPoly("AlertReminder");
		}

		*pHasPlayedSound |= dwSoundMask;
	}
}


BOOL CheckRemindersAndStart(void)
{
	// returns TRUE if there are any triggered reminder with SystemEventQueued, this will shorten the update interval
	// allowing sound repeats with shorter intervals

	TREEELEMENT *TTE;
	ULARGE_INTEGER curT;
	BOOL bHasPlayedSound;
	BOOL bResult;
	BOOL bHasQueuedReminders;

	if (!RemindersList)
		return FALSE;

	{
	SYSTEMTIME tm;
	GetSystemTime(&tm);
	SYSTEMTIMEtoFILETIME(&tm, (FILETIME*)&curT);
	}

	// NOTE: reminder list is sorted by trigger time, so we can early out on the first reminder > cur time

	// quick check for normal case with no reminder ready to be triggered and no queued triggered reminders
	// (happens 99.99999999999% of the time)
	if (curT.QuadPart < ((REMINDERDATA*)RemindersList->ptrdata)->When.QuadPart && !QueuedReminderCount)
	{
		return FALSE;
	}

	bResult = FALSE;

	// var used to avoid playing multiple alarm sounds during a single update
	bHasPlayedSound = FALSE;

	// if there are queued (triggered) reminders then iterate through entire list, becaue of WM_TIMECHANGE events
	// and for example daylight saving changes it's possible for an already triggered event to end up with When>curT
	bHasQueuedReminders = (QueuedReminderCount != 0);

	// allthough count should always be correct, it's fool proof to just count them again in the loop below
	QueuedReminderCount = 0;

	TTE = RemindersList;
	while (TTE && (bHasQueuedReminders || ((REMINDERDATA*)TTE->ptrdata)->When.QuadPart <= curT.QuadPart))
	{
		REMINDERDATA *pReminder = (REMINDERDATA*)TTE->ptrdata;

		if (!pReminder->RemVisible)
		{
			if (pReminder->SystemEventQueued)
			{
				UpdateReminderEvent(pReminder, REMINDER_UPDATE_INTERVAL_SHORT/1000, &bHasPlayedSound);

				QueuedReminderCount++;
				bResult = TRUE;
			}
			else if (((REMINDERDATA*)TTE->ptrdata)->When.QuadPart <= curT.QuadPart)
			{
				if (!g_RemindSMS)
				{
					FireReminder(pReminder, &bHasPlayedSound);

					if (pReminder->SystemEventQueued)
						bResult = TRUE;
				}
				else
				{
					char* S2 = strchr(g_RemindSMS, '@');
					char* S1 = (char*)malloc(S2 - g_RemindSMS);

					strncpy(S1, g_RemindSMS, S2 - g_RemindSMS);
					S1[S2 - g_RemindSMS]= 0x0;
					S2++;
					Send(S1, S2, pReminder->Reminder ? pReminder->Reminder : "", NULL);
					SAFE_FREE((void**)&S1);
					DeleteReminder(pReminder);
					JustSaveReminders();
					NOTIFY_LIST();
				}
			}
		}

		TTE = (TREEELEMENT*)TTE->next;
	}

	return bResult;
}


static LRESULT CALLBACK DatePickerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
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

static void InitDatePicker(HWND Dialog, UINT nIDDate)
{
	// subclass date picker to prevent user editing (should only use the dropdown calender to ensure valid dates)
	HWND hCtrl = GetDlgItem(Dialog, nIDDate);

	// tweak style of picker
	if ( IsWinVerVistaPlus() )
	{
		DWORD dw = SendDlgItemMessage(Dialog,nIDDate,DTM_GETMCSTYLE,0,0);
		dw |= MCS_WEEKNUMBERS | MCS_NOSELCHANGEONNAV;
		SendDlgItemMessage(Dialog,nIDDate,DTM_SETMCSTYLE,0,dw);
	}

	mir_subclassWindow(hCtrl, DatePickerWndProc);
}

static BOOL ParseTime(LPCSTR s, int *hout, int *mout, BOOL bTimeOffset, BOOL bAllowOffsetOverride)
{
	// validate format: <WS><digit>[<digit>]<WS>[':'<WS><digit>[<digit>]]<WS>[p | P].*

	// if bTimeOffset is FALSE the user may still enter a time offset by using + as the first char, the
	// delta time will be returned in minutes (even > 60) and hout will always be -1

	// if bTimeOffset is TRUE time is always interpreted as an offset (ignores PM indicator and defaults to minutes)

	int h, m;
	BOOL bOffset = bTimeOffset;

	// read hour

	while ( iswspace(*s) ) s++;

	if (*s == '+')
	{
		if (!bTimeOffset)
		{
			if (!bAllowOffsetOverride)
				return FALSE;

			// treat value as an offset anyway
			bOffset = TRUE;
		}

		s++;
		while ( iswspace(*s) ) s++;
	}

	if ( !isdigit(*s) )
		return FALSE;
	h = (int)(*s-'0');
	s++;

	if (!bOffset)
	{
		if ( isdigit(*s) )
		{
			h = h * 10 + (int)(*s-'0');
			s++;
		}

		if ( isdigit(*s) )
			return FALSE;
	}
	else
	{
		// allow more than 2-digit numbers for offset
		while ( isdigit(*s) )
		{
			h = h * 10 + (int)(*s-'0');
			s++;
		}
	}

	// find : separator

	while ( iswspace(*s) ) s++;

	if (*s == ':')
	{
		s++;

		// read minutes

		while ( iswspace(*s) ) s++;

		if ( !isdigit(*s) )
			return FALSE;
		m = (int)(*s-'0');
		s++;

		if ( isdigit(*s) )
		{
			m = m * 10 + (int)(*s-'0');
			s++;
		}
	}
	else
	{
		if (bOffset)
		{
			// no : separator found, interpret the entered number as minutes and allow > 60

			if (h < 0)
				return FALSE;

			if (bTimeOffset)
			{
				*hout = h / 60;
				*mout = h % 60;
			}
			else
			{
				*mout = h;
				*hout = -1;
			}

			return TRUE;
		}
		else
		{
			m = 0;
		}
	}

	// validate time
	if (bOffset)
	{
		if (h < 0)
			return FALSE;
		if (m < 0 || m > 59)
			return FALSE;
	}
	else
	{
		if (h == 24)
			h = 0;
		else if (h < 0 || h > 23)
			return FALSE;
		if (m < 0 || m > 59)
			return FALSE;
	}

	if (!bOffset)
	{
		// check for PM indicator (not strict, only checks for P char)

		while ( iswspace(*s) ) s++;

		if (*s == 'p' || *s == 'P')
		{
			if (h < 13)
				h += 12;
			else if (h == 12)
				h = 0;
		}
	}
	else if (!bTimeOffset)
	{
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
__inline static BOOL IsRelativeCombo(HWND Dialog, UINT nIDTime)
{
	return (((int)SendDlgItemMessage(Dialog,nIDTime,CB_GETITEMDATA,0,0)) >= 0);
}

static void PopulateTimeCombo(HWND Dialog, UINT nIDTime, BOOL bRelative, const SYSTEMTIME *tmUtc)
{
	// NOTE: may seem like a bit excessive time converstion and handling, but this is done in order
	//       to gracefully handle crossing daylight saving boundaries

	SYSTEMTIME tm2;
	ULARGE_INTEGER li;
	ULONGLONG ref;
	int i, n;
	char s[64];
	const ULONGLONG MinutesToFileTime = (ULONGLONG)60 * FILETIME_TICKS_PER_SEC;
	LPCSTR lpszMinutes;
	LPCSTR lpszHours;
	WORD wCurHour, wCurMinute;

	if (!bRelative)
	{
		SendDlgItemMessage(Dialog,nIDTime,CB_RESETCONTENT,0,0);

		// ensure that we start on midnight local time
		SystemTimeToTzSpecificLocalTime(NULL, (SYSTEMTIME*)tmUtc, &tm2);
		tm2.wHour = 0;
		tm2.wMinute = 0;
		tm2.wSecond = 0;
		tm2.wMilliseconds = 0;
		TzSpecificLocalTimeToSystemTime(NULL, &tm2, &tm2);
		SYSTEMTIMEtoFILETIME(&tm2, (FILETIME*)&li);

		// from 00:00 to 23:30 in 30 minute steps
		for (i=0; i<50; i++)
		{
			const int h = i>>1;
			const int m = (i&1) ? 30 : 0;

			FileTimeToTzLocalST((FILETIME*)&li, &tm2);
			mir_snprintf(s, SIZEOF(s), "%02d:%02d", (UINT)tm2.wHour, (UINT)tm2.wMinute);
			n = SendDlgItemMessage(Dialog,nIDTime,CB_ADDSTRING,0,(LPARAM)s);
			// item data contains time offset from midnight in seconds (bit 31 is set to flag that
			// combo box items are absolute times and not relative times like below
			SendDlgItemMessage(Dialog,nIDTime,CB_SETITEMDATA,n, (LPARAM)((ULONG)((h*60+m)*60) | 0x80000000));

			li.QuadPart += (ULONGLONG)30 * MinutesToFileTime;

			if (tm2.wHour == 23 && tm2.wMinute >= 30)
				break;
		}

		return;
	}

	//

	SendDlgItemMessage(Dialog, nIDTime, CB_RESETCONTENT, 0, 0);

	lpszMinutes = Translate("Minutes");
	lpszHours = Translate("Hours");

	SYSTEMTIMEtoFILETIME(tmUtc, (FILETIME*)&li);
	ref = li.QuadPart;

	// NOTE: item data contains offset from reference time (tmUtc) in seconds

	// cur time
	FileTimeToTzLocalST((FILETIME*)&li, &tm2);
	wCurHour = tm2.wHour;
	wCurMinute = tm2.wMinute;
	mir_snprintf(s, SIZEOF(s), "%02d:%02d", (UINT)tm2.wHour, (UINT)tm2.wMinute);
	n = SendDlgItemMessage(Dialog,nIDTime,CB_ADDSTRING,0,(LPARAM)s);
	SendDlgItemMessage(Dialog, nIDTime, CB_SETITEMDATA, n, (LPARAM)((li.QuadPart-ref)/FILETIME_TICKS_PER_SEC));

	// 5 minutes
	li.QuadPart += (ULONGLONG)5 * MinutesToFileTime;
	FileTimeToTzLocalST((FILETIME*)&li, &tm2);
	mir_snprintf(s, SIZEOF(s), "%02d:%02d (5 %s)", (UINT)tm2.wHour, (UINT)tm2.wMinute, lpszMinutes);
	n = SendDlgItemMessage(Dialog,nIDTime,CB_ADDSTRING,0,(LPARAM)s);
	SendDlgItemMessage(Dialog, nIDTime, CB_SETITEMDATA, n, (LPARAM)((li.QuadPart-ref)/FILETIME_TICKS_PER_SEC));

	// 10 minutes
	li.QuadPart += (ULONGLONG)5 * MinutesToFileTime;
	FileTimeToTzLocalST((FILETIME*)&li, &tm2);
	mir_snprintf(s, SIZEOF(s), "%02d:%02d (10 %s)", (UINT)tm2.wHour, (UINT)tm2.wMinute, lpszMinutes);
	n = SendDlgItemMessage(Dialog,nIDTime,CB_ADDSTRING,0,(LPARAM)s);
	SendDlgItemMessage(Dialog, nIDTime, CB_SETITEMDATA, n, (LPARAM)((li.QuadPart-ref)/FILETIME_TICKS_PER_SEC));

	// 15 minutes
	li.QuadPart += (ULONGLONG)5 * MinutesToFileTime;
	FileTimeToTzLocalST((FILETIME*)&li, &tm2);
	mir_snprintf(s, SIZEOF(s), "%02d:%02d (15 %s)", (UINT)tm2.wHour, (UINT)tm2.wMinute, lpszMinutes);
	n = SendDlgItemMessage(Dialog,nIDTime,CB_ADDSTRING,0,(LPARAM)s);
	SendDlgItemMessage(Dialog, nIDTime, CB_SETITEMDATA, n, (LPARAM)((li.QuadPart-ref)/FILETIME_TICKS_PER_SEC));

	// 30 minutes
	li.QuadPart += (ULONGLONG)15 * MinutesToFileTime;
	FileTimeToTzLocalST((FILETIME*)&li, &tm2);
	mir_snprintf(s, SIZEOF(s), "%02d:%02d (30 %s)", (UINT)tm2.wHour, (UINT)tm2.wMinute, lpszMinutes);
	n = SendDlgItemMessage(Dialog,nIDTime,CB_ADDSTRING,0,(LPARAM)s);
	SendDlgItemMessage(Dialog, nIDTime, CB_SETITEMDATA, n, (LPARAM)((li.QuadPart-ref)/FILETIME_TICKS_PER_SEC));

	// round +1h time to nearest even or half hour
	li.QuadPart += (ULONGLONG)30 * MinutesToFileTime;
	li.QuadPart = (li.QuadPart / (30 * MinutesToFileTime)) * (30 * MinutesToFileTime);

	// add from +1 to +23.5 (in half hour steps) if crossing daylight saving boundary it may be 22.5 or 24.5 hours
	for (i=0; i<50; i++)
	{
		UINT dt;

		FileTimeToTzLocalST((FILETIME*)&li, &tm2);

		if (i > 40)
		{
			UINT nLastEntry = ((UINT)wCurHour * 60 + (UINT)wCurMinute) / 30;
			if (nLastEntry)
				nLastEntry *= 30;
			else
				nLastEntry = 23*60 + 30;

			if (((UINT)tm2.wHour * 60 + (UINT)tm2.wMinute) == nLastEntry)
				break;
		}

		// icq-style display 1.0, 1.5 etc. hours even though that isn't accurate due to rounding
		//mir_snprintf(s, SIZEOF(s), "%02d:%02d (%d.%d %s)", (UINT)tm2.wHour, (UINT)tm2.wMinute, 1+(i>>1), (i&1) ? 5 : 0, lpszHours);
		// display delta time more accurately to match reformatting (that icq doesn't do)
		dt = (UINT)((li.QuadPart/MinutesToFileTime) - (ref/MinutesToFileTime));
		if (dt < 60)
			mir_snprintf(s, SIZEOF(s), "%02d:%02d (%d %s)", (UINT)tm2.wHour, (UINT)tm2.wMinute, dt, lpszMinutes);
		else
			mir_snprintf(s, SIZEOF(s), "%02d:%02d (%d.%d %s)", (UINT)tm2.wHour, (UINT)tm2.wMinute, dt/60, ((dt%60)*10)/60, lpszHours);
		n = SendDlgItemMessage(Dialog,nIDTime,CB_ADDSTRING,0,(LPARAM)s);
		SendDlgItemMessage(Dialog,nIDTime,CB_SETITEMDATA,n, (LPARAM)(dt*60));

		li.QuadPart += (ULONGLONG)30 * MinutesToFileTime;
	}
}

static void PopulateTimeOffsetCombo(HWND Dialog, UINT nIDCombo)
{
	int i, n;
	LPCSTR lpszMinutes;
	LPCSTR lpszHour;
	LPCSTR lpszHours;
	LPCSTR lpszDay;
	LPCSTR lpszDays;
	LPCSTR lpszWeek;
	char s[MAX_PATH];

	SendDlgItemMessage(Dialog,nIDCombo,CB_RESETCONTENT,0,0);

	lpszMinutes = Translate("Minutes");
	lpszHour = Translate("Hour");
	lpszHours = Translate("Hours");
	lpszDay = Translate("Day");
	lpszDays = Translate("Days");
	lpszWeek = Translate("Week");

	// 5 - 55 minutes (in 5 minute steps)
	for (i = 1; i < 12; i++)
	{
		mir_snprintf(s, SIZEOF(s), "%d %s", i*5, lpszMinutes);
		n = SendDlgItemMessage(Dialog,nIDCombo,CB_ADDSTRING,0,(LPARAM)s);
		SendDlgItemMessage(Dialog,nIDCombo,CB_SETITEMDATA,n, (LPARAM)(i*5));
	}

	// 1 hour
	mir_snprintf(s, SIZEOF(s), "1 %s", lpszHour);
	n = SendDlgItemMessage(Dialog,nIDCombo,CB_ADDSTRING,0,(LPARAM)s);
	SendDlgItemMessage(Dialog,nIDCombo,CB_SETITEMDATA,n, (LPARAM)60);

	// 2, 4, 8 hours
	for (i = 2; i <= 8; i+=2)
	{
		mir_snprintf(s, SIZEOF(s), "%d %s", i, lpszHours);
		n = SendDlgItemMessage(Dialog,nIDCombo,CB_ADDSTRING,0,(LPARAM)s);
		SendDlgItemMessage(Dialog,nIDCombo,CB_SETITEMDATA,n, (LPARAM)(i*60));
	}

	// 1 day
	mir_snprintf(s, SIZEOF(s), "1 %s", lpszDay);
	n = SendDlgItemMessage(Dialog,nIDCombo,CB_ADDSTRING,0,(LPARAM)s);
	SendDlgItemMessage(Dialog,nIDCombo,CB_SETITEMDATA,n, (LPARAM)(24*60));

	// 2-4 days
	for (i = 2; i <= 4; i++)
	{
		mir_snprintf(s, SIZEOF(s), "%d %s", i, lpszDays);
		n = SendDlgItemMessage(Dialog,nIDCombo,CB_ADDSTRING,0,(LPARAM)s);
		SendDlgItemMessage(Dialog,nIDCombo,CB_SETITEMDATA,n, (LPARAM)(i*24*60));
	}

	// 1 week
	mir_snprintf(s, SIZEOF(s), "1 %s", lpszWeek);
	n = SendDlgItemMessage(Dialog,nIDCombo,CB_ADDSTRING,0,(LPARAM)s);
	SendDlgItemMessage(Dialog,nIDCombo,CB_SETITEMDATA,n, (LPARAM)(7*24*60));
}

// returns non-zero if specified time was inside "missing" hour of daylight saving
// IMPORTANT: triggerRelUtcOut is only initialized if IsRelativeCombo() is TRUE and return value is 0
static int ReformatTimeInputEx(HWND Dialog, UINT nIDTime, UINT nIDRefTime, int h, int m, const SYSTEMTIME *pDateLocal, ULARGE_INTEGER *triggerRelUtcOut)
{
	int n;
	UINT dt;
	char buf[64];
	const ULONGLONG MinutesToFileTime = (ULONGLONG)60 * FILETIME_TICKS_PER_SEC;

	if (h < 0)
	{
		// time value is an offset ('m' holds the offset in minutes)

		if ( IsRelativeCombo(Dialog, nIDTime) )
		{
			ULONGLONG ref;
			ULARGE_INTEGER li;
			SYSTEMTIME tm;

			// get reference time (UTC) from hidden control
			{
				GetDlgItemText(Dialog, nIDRefTime, buf, 30);
				li.QuadPart = ref = _strtoui64(buf, NULL, 16);
			}

			// clamp delta time to 23.5 hours (coule be issues otherwise as relative combo only handles <24)
			if (m > (23*60+30))
				m = 23*60+30;

			li.QuadPart += (ULONGLONG)(m * 60) * FILETIME_TICKS_PER_SEC;

			FileTimeToTzLocalST((FILETIME*)&li, &tm);
			h = (int)tm.wHour;
			m = (int)tm.wMinute;

			if (triggerRelUtcOut)
				*triggerRelUtcOut = li;

			dt = (UINT)((li.QuadPart/MinutesToFileTime) - (ref/MinutesToFileTime));

			if (dt < 60)
				mir_snprintf(buf, SIZEOF(buf), "%02d:%02d (%d %s)", h, m, dt, Translate("Minutes"));
			else
				mir_snprintf(buf, SIZEOF(buf), "%02d:%02d (%d.%d %s)", h, m, dt/60, ((dt%60)*10)/60, Translate("Hours"));

			// search for preset
			n = SendDlgItemMessage(Dialog, nIDTime, CB_FINDSTRING, (WPARAM)-1, (LPARAM)buf);
			if (n != CB_ERR)
			{
				SendDlgItemMessage(Dialog, nIDTime, CB_SETCURSEL, n, 0);
				return 0;
			}

			SetDlgItemText(Dialog, nIDTime, buf);
		}
		else
		{
			// should never happen
			SendDlgItemMessage(Dialog, nIDTime, CB_SETCURSEL, 0, 0);
		}

		return 0;
	}

	//

	mir_snprintf(buf, SIZEOF(buf), "%02d:%02d", h, m);

	// search for preset first
	n = SendDlgItemMessage(Dialog, nIDTime, CB_FINDSTRING, (WPARAM)-1, (LPARAM)buf);
	if (n != CB_ERR)
	{
		SendDlgItemMessage(Dialog, nIDTime, CB_SETCURSEL, n, 0);
		return 0;
	}

	if ( IsRelativeCombo(Dialog, nIDTime) )
	{
		// date format is a time offset ("24:43 (5 Minutes)" etc.)

		ULONGLONG ref;
		SYSTEMTIME tmRefLocal;
		SYSTEMTIME tmTriggerLocal, tmTriggerLocal2;

		// get reference time (UTC) from hidden control
		{
			GetDlgItemText(Dialog, nIDRefTime, buf, 30);
			ref = _strtoui64(buf, NULL, 16);
		}

		FileTimeToTzLocalST((FILETIME*)&ref, &tmRefLocal);

		{
			ULARGE_INTEGER li;
			const UINT nRefT = (UINT)tmRefLocal.wHour * 60 + (UINT)tmRefLocal.wMinute;
			const UINT nT = h * 60 + m;

			tmTriggerLocal = tmRefLocal;
			tmTriggerLocal.wHour = (WORD)h;
			tmTriggerLocal.wMinute = (WORD)m;
			tmTriggerLocal.wSecond = 0;
			tmTriggerLocal.wMilliseconds = 0;

			if (nT < nRefT)
			{
				// (this special case only works correctly if time can be returned in triggerRelUtcOut)
				if (tmRefLocal.wHour == tmTriggerLocal.wHour && triggerRelUtcOut)
				{
					// check for special case if daylight saving ends in this hour, then interpret as within the next hour
					TzLocalSTToFileTime(&tmTriggerLocal, (FILETIME*)&li);
					li.QuadPart += (ULONGLONG)3600*FILETIME_TICKS_PER_SEC;
					FileTimeToTzLocalST((FILETIME*)&li, &tmTriggerLocal2);
					if ((tmTriggerLocal2.wHour*60+tmTriggerLocal2.wMinute) == (tmTriggerLocal.wHour*60+tmTriggerLocal.wMinute))
						// special case detected
						goto output_result;
				}

				// tomorrow (add 24h to local time)
				SYSTEMTIMEtoFILETIME(&tmTriggerLocal, (FILETIME*)&li);
				li.QuadPart += (ULONGLONG)(24*3600)*FILETIME_TICKS_PER_SEC;
				FILETIMEtoSYSTEMTIME((FILETIME*)&li, &tmTriggerLocal);
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
			if ((tmTriggerLocal2.wHour*60+tmTriggerLocal2.wMinute) < (tmTriggerLocal.wHour*60+tmTriggerLocal.wMinute))
			{
				// special case detected, fall back to current time so at least the reminder won't be missed
				// due to ending up at an undesired time (this way the user immediately notices something was wrong)
				SendDlgItemMessage(Dialog, nIDTime, CB_SETCURSEL, 0, 0);
invalid_dst:
				MessageBox(Dialog, Translate("The specified time is invalid due to begin of daylight saving (summer time)."), SECTIONNAME, MB_OK|MB_ICONWARNING);
				return 1;
			}

output_result:
			if (triggerRelUtcOut)
				*triggerRelUtcOut = li;

			dt = (UINT)((li.QuadPart/MinutesToFileTime) - (ref/MinutesToFileTime));

			if (dt < 60)
				mir_snprintf(buf, SIZEOF(buf), "%02d:%02d (%d %s)", h, m, dt, Translate("Minutes"));
			else
				mir_snprintf(buf, SIZEOF(buf), "%02d:%02d (%d.%d %s)", h, m, dt/60, ((dt%60)*10)/60, Translate("Hours"));
		}
	}
	else
	{
		// absolute time (00:00 to 23:59), clean up time to make sure it's not inside "missing" hour (will be rounded downard)

		FILETIME ft;
		SYSTEMTIME Date = *pDateLocal;
		Date.wHour = h;
		Date.wMinute = m;
		Date.wSecond = 0;
		Date.wMilliseconds = 0;

		TzLocalSTToFileTime(&Date, &ft);
		FileTimeToTzLocalST(&ft, &Date);

		if ((int)Date.wHour != h || (int)Date.wMinute != m)
		{
			mir_snprintf(buf, SIZEOF(buf), "%02d:%02d", (UINT)Date.wHour, (UINT)Date.wMinute);

			// search for preset again
			n = SendDlgItemMessage(Dialog, nIDTime, CB_FINDSTRING, (WPARAM)-1, (LPARAM)buf);
			if (n != CB_ERR)
			{
				SendDlgItemMessage(Dialog, nIDTime, CB_SETCURSEL, n, 0);
				goto invalid_dst;
			}

			SetDlgItemText(Dialog, nIDTime, buf);

			goto invalid_dst;
		}
	}

	SetDlgItemText(Dialog, nIDTime, buf);

	return 0;
}

static __inline int ReformatTimeInput(HWND Dialog, UINT nIDTime, UINT nIDRefTime, int h, int m, const SYSTEMTIME *pDateLocal)
{
	return ReformatTimeInputEx(Dialog, nIDTime, nIDRefTime, h, m, pDateLocal, NULL);
}

// in:  pDate contains the desired trigger date in LOCAL time
// out: pDate contains the resulting trigger time and date in UTC
static BOOL GetTriggerTime(HWND Dialog, UINT nIDTime, UINT nIDRefTime, SYSTEMTIME *pDate)
{
	ULARGE_INTEGER li;
	char buf[32];
	int n;
	int h, m;

	// get reference (UTC) time from hidden control
	{
		GetDlgItemText(Dialog, nIDRefTime, buf, 30);
		li.QuadPart = _strtoui64(buf, NULL, 16);
	}

	if ((n = SendDlgItemMessage(Dialog, nIDTime, CB_GETCURSEL, 0, 0)) != CB_ERR)
	{
		// use preset value
preset_value:;
		if ( IsRelativeCombo(Dialog, nIDTime) )
		{
			// time offset from ref time ("24:43 (5 Minutes)" etc.)

			UINT nDeltaSeconds = (UINT)SendDlgItemMessage(Dialog, nIDTime, CB_GETITEMDATA, n, 0);
			li.QuadPart += (ULONGLONG)nDeltaSeconds * FILETIME_TICKS_PER_SEC;

			FILETIMEtoSYSTEMTIME((FILETIME*)&li, pDate);

			// if specified time is a small offset (< 10 Minutes) then retain current second count for better accuracy
			// otherwise try to match specified time (which never contains seconds only even minutes) as close as possible
			if (nDeltaSeconds >= 10*60)
			{
				pDate->wSecond = 0;
				pDate->wMilliseconds = 0;
			}
		}
		else
		{
			// absolute time (offset from midnight on pDate)

			UINT nDeltaSeconds = (UINT)((ULONG)SendDlgItemMessage(Dialog, nIDTime, CB_GETITEMDATA, n, 0) & ~0x80000000);
			pDate->wHour = 0;
			pDate->wMinute = 0;
			pDate->wSecond = 0;
			pDate->wMilliseconds = 0;
			TzLocalSTToFileTime(pDate, (FILETIME*)&li);
			li.QuadPart += (ULONGLONG)nDeltaSeconds * FILETIME_TICKS_PER_SEC;

			FILETIMEtoSYSTEMTIME((FILETIME*)&li, pDate);
		}

		return TRUE;
	}

	// user entered a custom value

	GetDlgItemText(Dialog, nIDTime, buf, 30);

	if ( !ParseTime(buf, &h, &m, FALSE, IsRelativeCombo(Dialog, nIDTime)) )
	{
		MessageBox(Dialog, Translate("The specified time is invalid."), SECTIONNAME, MB_OK|MB_ICONWARNING);
		return FALSE;
	}

	if ( IsRelativeCombo(Dialog, nIDTime) )
	{
		// date has not been changed, the specified time is a time between reftime and reftime+24h

		ULARGE_INTEGER li2;

		if ( ReformatTimeInputEx(Dialog, nIDTime, nIDRefTime, h, m, pDate, &li2) )
			return FALSE;

		// check if reformatted value is a preset
		if ((n = SendDlgItemMessage(Dialog, nIDTime, CB_GETCURSEL, 0, 0)) != CB_ERR)
			goto preset_value;

		FILETIMEtoSYSTEMTIME((FILETIME*)&li2, pDate);

		return TRUE;
	}
	else
	{
		if ( ReformatTimeInputEx(Dialog, nIDTime, nIDRefTime, h, m, pDate, NULL) )
			return FALSE;

		// check if reformatted value is a preset
		if ((n = SendDlgItemMessage(Dialog, nIDTime, CB_GETCURSEL, 0, 0)) != CB_ERR)
			goto preset_value;
	}

	// absolute time (on pDate)

	pDate->wHour = h;
	pDate->wMinute = m;
	pDate->wSecond = 0;
	pDate->wMilliseconds = 0;

	TzLocalSTToFileTime(pDate, (FILETIME*)&li);
	FILETIMEtoSYSTEMTIME((FILETIME*)&li, pDate);

	return TRUE;
}

static void OnDateChanged(HWND Dialog, UINT nDateID, UINT nTimeID, UINT nRefTimeID)
{
	// repopulate time combo list with regular times (not offsets like "23:32 (5 minutes)" etc.)

	SYSTEMTIME Date, DateUtc;
	int h = -1, m;
	char s[32];

	GetDlgItemText(Dialog, nTimeID, s, 30);

	ParseTime(s, &h, &m, FALSE, FALSE);

	SendDlgItemMessage(Dialog, nDateID, DTM_GETSYSTEMTIME, 0, (LPARAM)&Date);

	TzSpecificLocalTimeToSystemTime(NULL, &Date, &DateUtc);
	PopulateTimeCombo(Dialog, nTimeID, FALSE, &DateUtc);

	if (h < 0)
	{
		// parsing failed, default to current time
		SYSTEMTIME tm;
		GetLocalTime(&tm);
		h = (UINT)tm.wHour;
		m = (UINT)tm.wMinute;
	}

	ReformatTimeInput(Dialog, nTimeID, nRefTimeID, h, m, &Date);
}


INT_PTR CALLBACK DlgProcNotifyReminder(HWND Dialog,UINT Message,WPARAM wParam,LPARAM lParam)
{
	int I;

	switch (Message)
	{
	case WM_INITDIALOG:
		{
			SYSTEMTIME tm;
			ULARGE_INTEGER li;

			GetSystemTime(&tm);
			SYSTEMTIMEtoFILETIME(&tm, (FILETIME*)&li);

			TranslateDialogDefault(Dialog);

			// save reference time in hidden control, is needed when adding reminder to properly detect if speicifed
			// time wrapped around to tomorrow or not (dialog could in theory be open for a longer period of time
			// which could potentially mess up things otherwise)
			{
				char s[32];
				mir_snprintf(s, SIZEOF(s), "%I64x", li.QuadPart);
				SetDlgItemText(Dialog, IDC_REFTIME, s);
			}

			BringWindowToTop(Dialog);

			PopulateTimeOffsetCombo(Dialog, IDC_REMINDAGAININ);

			ShowWindow(GetDlgItem(Dialog,IDC_REMINDAGAININ),SW_SHOW);
			ShowWindow(GetDlgItem(Dialog,IDC_DATEAGAIN),SW_HIDE);
			ShowWindow(GetDlgItem(Dialog,IDC_TIMEAGAIN),SW_HIDE);
			ShowWindow(GetDlgItem(Dialog,IDC_STATIC_DATE),SW_HIDE);
			ShowWindow(GetDlgItem(Dialog,IDC_STATIC_TIME),SW_HIDE);
			CheckDlgButton(Dialog, IDC_AFTER, BST_CHECKED);
			CheckDlgButton(Dialog, IDC_ONDATE, BST_UNCHECKED);
			SendDlgItemMessage(Dialog,IDC_REMINDAGAININ,CB_SETCURSEL,0,0);

			SendDlgItemMessage(Dialog, IDC_REMDATA, EM_LIMITTEXT, MAX_REMINDER_LEN, 0);

			PopulateTimeCombo(Dialog, IDC_TIMEAGAIN, TRUE, &tm);

			FileTimeToTzLocalST((FILETIME*)&li, &tm);

			// make sure date picker uses reference time
			SendDlgItemMessage(Dialog,IDC_DATEAGAIN,DTM_SETSYSTEMTIME,0,(LPARAM)&tm);
			InitDatePicker(Dialog, IDC_DATEAGAIN);

			SendDlgItemMessage(Dialog,IDC_TIMEAGAIN,CB_SETCURSEL,0,0);

			return TRUE;
		}
	case WM_NCDESTROY:
		RemoveProp(GetDlgItem(Dialog, IDC_DATEAGAIN), TEXT("OldWndProc"));
		return TRUE;
	case WM_NOTIFY:
		{
			if (wParam == IDC_DATEAGAIN)
			{
				NMLISTVIEW *NM = (NMLISTVIEW*)lParam;

				switch (NM->hdr.code)
				{
				case DTN_DATETIMECHANGE:
					OnDateChanged(Dialog, IDC_DATEAGAIN, IDC_TIMEAGAIN, IDC_REFTIME);
					break;
				}
			}
		}
		break;
	case WM_CLOSE:
			{
				int ReminderCount = TreeGetCount(RemindersList);
				for (I = 0; I < ReminderCount; I++)
				{
					REMINDERDATA *pReminder = (REMINDERDATA*)TreeGetAt(RemindersList, I);

					if (pReminder->handle == Dialog)
					{
						DeleteReminder(pReminder);
						JustSaveReminders();
						break;
					}
				}
				NOTIFY_LIST();
			}
			DestroyWindow(Dialog);
			return TRUE;
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_TIMEAGAIN:
				switch (HIWORD(wParam))
				{
				case CBN_KILLFOCUS:
					// reformat displayed value
					if (SendDlgItemMessage(Dialog, IDC_TIMEAGAIN, CB_GETCURSEL, 0, 0) == CB_ERR)
					{
						char buf[64];
						int h, m;
						GetDlgItemText(Dialog, IDC_TIMEAGAIN, buf, 30);

						if ( ParseTime(buf, &h, &m, FALSE, IsRelativeCombo(Dialog, IDC_TIMEAGAIN)) )
						{
							SYSTEMTIME Date;
							SendDlgItemMessage(Dialog,IDC_DATEAGAIN,DTM_GETSYSTEMTIME,0,(LPARAM)&Date);

							ReformatTimeInput(Dialog, IDC_TIMEAGAIN, IDC_REFTIME, h, m, &Date);
						}
						else
						{
							SendDlgItemMessage(Dialog, IDC_TIMEAGAIN, CB_SETCURSEL, 0, 0);
						}
					}
					break;
				}
				break;

			case IDC_REMINDAGAININ:
				switch (HIWORD(wParam))
				{
				case CBN_KILLFOCUS:
					// reformat displayed value if it has been edited
					if (SendDlgItemMessage(Dialog,IDC_REMINDAGAININ,CB_GETCURSEL,0,0) == CB_ERR)
					{
						char buf[64];
						int h, m;
						GetDlgItemText(Dialog, IDC_REMINDAGAININ, buf, 30);

						if (ParseTime(buf, &h, &m, TRUE, TRUE) && h+m)
						{
							if (h)
							{
								LPCSTR lpszHours = Translate("Hours");
								mir_snprintf(buf, SIZEOF(buf), "%d:%02d %s", h, m, lpszHours);
							}
							else
							{
								LPCSTR lpszMinutes = Translate("Minutes");
								mir_snprintf(buf, SIZEOF(buf), "%d %s", m, lpszMinutes);
							}
							SetDlgItemText(Dialog, IDC_REMINDAGAININ, buf);
						}
						else
						{
							SendDlgItemMessage(Dialog,IDC_REMINDAGAININ,CB_SETCURSEL,0,0);
						}
					}
					break;
				}
				break;

			case IDC_AFTER:
				{
					ShowWindow(GetDlgItem(Dialog, IDC_REMINDAGAININ), SW_SHOW);
					ShowWindow(GetDlgItem(Dialog, IDC_DATEAGAIN), SW_HIDE);
					ShowWindow(GetDlgItem(Dialog, IDC_TIMEAGAIN), SW_HIDE);
					ShowWindow(GetDlgItem(Dialog, IDC_STATIC_DATE), SW_HIDE);
					ShowWindow(GetDlgItem(Dialog, IDC_STATIC_TIME), SW_HIDE);
					return TRUE;
				}
			case IDC_ONDATE:
				{
					ShowWindow(GetDlgItem(Dialog, IDC_DATEAGAIN), SW_SHOW);
					ShowWindow(GetDlgItem(Dialog, IDC_TIMEAGAIN), SW_SHOW);
					ShowWindow(GetDlgItem(Dialog, IDC_STATIC_DATE), SW_SHOW);
					ShowWindow(GetDlgItem(Dialog, IDC_STATIC_TIME), SW_SHOW);
					ShowWindow(GetDlgItem(Dialog, IDC_REMINDAGAININ), SW_HIDE);
					return TRUE;
				}
			case IDC_DISMISS:
				{
					int ReminderCount = TreeGetCount(RemindersList);
					for (I = 0; I < ReminderCount; I++)
					{
						REMINDERDATA *pReminder = (REMINDERDATA*)TreeGetAt(RemindersList, I);

						if (pReminder->handle == Dialog)
						{
							DeleteReminder(pReminder);
							JustSaveReminders();
							break;
						}
					}
					NOTIFY_LIST();
					DestroyWindow(Dialog);
					return TRUE;
				}
			case IDC_REMINDAGAIN:
				{
					int ReminderCount = TreeGetCount(RemindersList);
					for (I = 0; I < ReminderCount; I++)
					{
						REMINDERDATA *pReminder = (REMINDERDATA*)TreeGetAt(RemindersList, I);

						if (pReminder->handle == Dialog)
						{
							if (IsDlgButtonChecked(Dialog, IDC_AFTER) == BST_CHECKED)
							{
								// delta time

								ULONGLONG TT;
								SYSTEMTIME tm;
								ULARGE_INTEGER li;
								int n;

								GetSystemTime(&tm);
								SYSTEMTIMEtoFILETIME(&tm, (FILETIME*)&li);

								n = SendDlgItemMessage(Dialog, IDC_REMINDAGAININ, CB_GETCURSEL, 0, 0);
								if (n != CB_ERR)
								{
									TT = SendDlgItemMessage(Dialog, IDC_REMINDAGAININ, CB_GETITEMDATA, n, 0) * 60;

									if (TT >= 24*3600)
									{
										// selection is 1 day or more, take daylight saving boundaries into consideration
										// (ie. 24 hour might actually be 23 or 25, in order for reminder to pop up at the
										// same time tomorrow)
										SYSTEMTIME tm2, tm3;
										ULARGE_INTEGER li2 = li;
										FileTimeToTzLocalST((FILETIME*)&li2, &tm2);
										li2.QuadPart += (TT * FILETIME_TICKS_PER_SEC);
										FileTimeToTzLocalST((FILETIME*)&li2, &tm3);
										if (tm2.wHour != tm3.wHour || tm2.wMinute != tm3.wMinute)
										{
											// boundary crossed

											// do a quick and dirty sanity check that times not more than 2 hours apart
											if (abs((int)(tm3.wHour*60+tm3.wMinute)-(int)(tm2.wHour*60+tm2.wMinute)) <= 120)
											{
												// adjust TT so that same HH:MM is set
												tm3.wHour = tm2.wHour;
												tm3.wMinute = tm2.wMinute;
												TzLocalSTToFileTime(&tm3, (FILETIME*)&li2);
												TT = (li2.QuadPart - li.QuadPart) / FILETIME_TICKS_PER_SEC;
											}
										}
									}
								}
								else
								{
									// parse user input
									char s[32];
									int h = 0, m = 0;
									GetDlgItemText(Dialog, IDC_REMINDAGAININ, s, 30);
									ParseTime(s, &h, &m, TRUE, TRUE);
									m += h * 60;
									if (!m)
									{
										MessageBox(Dialog, TranslateT("The specified time offset is invalid."), _T(SECTIONNAME), MB_OK | MB_ICONWARNING);
										return TRUE;
									}

									TT = m * 60;
								}

								pReminder->When = li;
								pReminder->When.QuadPart += (TT * FILETIME_TICKS_PER_SEC);
							}
							else if (IsDlgButtonChecked(Dialog, IDC_ONDATE) == BST_CHECKED)
							{
								SYSTEMTIME Date;

								SendDlgItemMessage(Dialog, IDC_DATEAGAIN, DTM_GETSYSTEMTIME, 0, (LPARAM)&Date);

								if ( !GetTriggerTime(Dialog, IDC_TIMEAGAIN, IDC_REFTIME, &Date) )
									break;

								SYSTEMTIMEtoFILETIME(&Date, (FILETIME*)&pReminder->When);
							}

							// update reminder text
							{
							char *ReminderText = NULL;
							int SzT = SendDlgItemMessage(Dialog, IDC_REMDATA, WM_GETTEXTLENGTH, 0, 0);
							if (SzT)
							{
								if (SzT > MAX_REMINDER_LEN) SzT = MAX_REMINDER_LEN;
								ReminderText = (char*)malloc(SzT + 1);
								GetDlgItemText(Dialog, IDC_REMDATA, ReminderText, SzT + 1);
							}
							if (pReminder->Reminder)
								free(pReminder->Reminder);
							pReminder->Reminder = ReminderText;
							}

							pReminder->RemVisible = FALSE;
							pReminder->handle = NULL;

							// re-insert tree item sorted
							TreeDelete(&RemindersList,pReminder);
							TreeAddSorted(&RemindersList, pReminder, ReminderSortCb);
							JustSaveReminders();
							break;
						}
					}
					NOTIFY_LIST();
					DestroyWindow(Dialog);
					return TRUE;
				}
			case IDC_NONE:
				{// create note from remainder
					int ReminderCount = TreeGetCount(RemindersList);
					for (I = 0; I < ReminderCount; I++)
					{
						REMINDERDATA *pReminder = (REMINDERDATA*)TreeGetAt(RemindersList, I);

						if (pReminder->handle == Dialog)
						{
							// get up-to-date reminder text
							char *ReminderText = NULL;
							int SzT = SendDlgItemMessage(Dialog, IDC_REMDATA, WM_GETTEXTLENGTH, 0, 0);
							if (SzT)
							{
								if (SzT > MAX_REMINDER_LEN) SzT = MAX_REMINDER_LEN;
								ReminderText = (char*)malloc(SzT+1);
								GetDlgItemText(Dialog, IDC_REMDATA, ReminderText, SzT + 1);
							}

							SetFocus(NewNote(0, 0, -1, -1, ReminderText, 0, TRUE, TRUE, 0)->REHwnd);
							break;
						}
					}
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

INT_PTR CALLBACK DlgProcNewReminder(HWND Dialog,UINT Message,WPARAM wParam,LPARAM lParam)
{
	HICON hIcon = NULL;
	switch (Message)
	{
    case WM_INITDIALOG:
		{
			ULARGE_INTEGER li;
			SYSTEMTIME tm;

			if (NewReminderVisible == 2)
			{
				// opening the edit reminder dialog (uses same dialog resource as add reminder)
				SetWindowText(Dialog, TranslateT("Reminder"));
				SetDlgItemText(Dialog, IDC_ADDREMINDER, TranslateT("&Update Reminder"));
				ShowWindow(GetDlgItem(Dialog, IDC_VIEWREMINDERS), SW_HIDE);

				li = pEditReminder->When;
				FILETIMEtoSYSTEMTIME((FILETIME*)&li, &tm);
			}
			else
			{
				GetSystemTime(&tm);
				SYSTEMTIMEtoFILETIME(&tm, (FILETIME*)&li);
			}

			TranslateDialogDefault(Dialog);

			// save reference time in hidden control, is needed when adding reminder to properly detect if speicifed
			// time wrapped around to tomorrow or not (dialog could in theory be open for a longer period of time
			// which could potentially mess up things otherwise)
			{
				char s[32];
				mir_snprintf(s, SIZEOF(s), "%I64x", li.QuadPart);
				SetDlgItemText(Dialog, IDC_REFTIME, s);
			}

			/*CheckDlgButton(Dialog, IDC_NONE, BST_CHECKED);
			CheckDlgButton(Dialog, IDC_DAILY, BST_UNCHECKED);
			CheckDlgButton(Dialog, IDC_WEEKLY, BST_UNCHECKED);
			CheckDlgButton(Dialog, IDC_MONTHLY, BST_UNCHECKED);*/

			PopulateTimeCombo(Dialog, IDC_TIME, NewReminderVisible != 2, &tm);

			FileTimeToTzLocalST((FILETIME*)&li, &tm);

			// make sure date picker uses reference time
			SendDlgItemMessage(Dialog, IDC_DATE, DTM_SETSYSTEMTIME, 0, (LPARAM)&tm);
			InitDatePicker(Dialog, IDC_DATE);

			SendDlgItemMessage(Dialog, IDC_REMINDER, EM_LIMITTEXT, MAX_REMINDER_LEN, 0);

			if (NewReminderVisible == 2)
			{
				int n;
				char s[32];
				mir_snprintf(s, SIZEOF(s), "%02d:%02d", (UINT)tm.wHour, (UINT)tm.wMinute);

				// search for preset first
				n = SendDlgItemMessage(Dialog, IDC_TIME, CB_FINDSTRING, (WPARAM)-1, (LPARAM)s);
				if (n != CB_ERR)
					SendDlgItemMessage(Dialog, IDC_TIME, CB_SETCURSEL, n, 0);
				else
					SetDlgItemText(Dialog, IDC_TIME, s);

				SetDlgItemText(Dialog, IDC_REMINDER, pEditReminder->Reminder);
			}
			else
			{
				SendDlgItemMessage(Dialog,IDC_TIME,CB_SETCURSEL,0,0);
			}

			// populate sound repeat combo
			{
				char s[64];
				int n;
				LPCSTR lpszEvery = Translate("Every");
				LPCSTR lpszSeconds = Translate("Seconds");

				// NOTE: use multiples of REMINDER_UPDATE_INTERVAL_SHORT (currently 5 seconds)

				n = SendDlgItemMessage(Dialog, IDC_COMBO_REPEATSND, CB_ADDSTRING, 0, (LPARAM)Translate("Never"));
				SendDlgItemMessage(Dialog,IDC_COMBO_REPEATSND,CB_SETITEMDATA, n, 0);

				mir_snprintf(s, SIZEOF(s), "%s 5 %s", lpszEvery, lpszSeconds);
				n = SendDlgItemMessage(Dialog,IDC_COMBO_REPEATSND, CB_ADDSTRING, 0, (LPARAM)s);
				SendDlgItemMessage(Dialog, IDC_COMBO_REPEATSND, CB_SETITEMDATA, n, (LPARAM)5);

				mir_snprintf(s, SIZEOF(s), "%s 10 %s", lpszEvery, lpszSeconds);
				n = SendDlgItemMessage(Dialog,IDC_COMBO_REPEATSND,CB_ADDSTRING, 0, (LPARAM)s);
				SendDlgItemMessage(Dialog, IDC_COMBO_REPEATSND, CB_SETITEMDATA, n, (LPARAM)10);

				mir_snprintf(s, SIZEOF(s), "%s 15 %s", lpszEvery, lpszSeconds);
				n = SendDlgItemMessage(Dialog,IDC_COMBO_REPEATSND,CB_ADDSTRING, 0, (LPARAM)s);
				SendDlgItemMessage(Dialog, IDC_COMBO_REPEATSND, CB_SETITEMDATA, n, (LPARAM)15);

				mir_snprintf(s, SIZEOF(s), "%s 20 %s", lpszEvery, lpszSeconds);
				n = SendDlgItemMessage(Dialog,IDC_COMBO_REPEATSND,CB_ADDSTRING, 0, (LPARAM)s);
				SendDlgItemMessage(Dialog, IDC_COMBO_REPEATSND, CB_SETITEMDATA, n, (LPARAM)20);

				mir_snprintf(s, SIZEOF(s), "%s 30 %s", lpszEvery, lpszSeconds);
				n = SendDlgItemMessage(Dialog,IDC_COMBO_REPEATSND,CB_ADDSTRING, 0, (LPARAM)s);
				SendDlgItemMessage(Dialog, IDC_COMBO_REPEATSND, CB_SETITEMDATA, n, (LPARAM)30);

				mir_snprintf(s, SIZEOF(s), "%s 60 %s", lpszEvery, lpszSeconds);
				n = SendDlgItemMessage(Dialog, IDC_COMBO_REPEATSND, CB_ADDSTRING, 0, (LPARAM)s);
				SendDlgItemMessage(Dialog, IDC_COMBO_REPEATSND, CB_SETITEMDATA, n, (LPARAM)60);

				if (NewReminderVisible == 2 && pEditReminder->RepeatSound)
				{
					mir_snprintf(s, SIZEOF(s), "%s %d %s", lpszEvery, pEditReminder->RepeatSound, lpszSeconds);
					SetDlgItemText(Dialog, IDC_COMBO_REPEATSND, s);
					SendDlgItemMessage(Dialog, IDC_COMBO_REPEATSND, CB_SETCURSEL, SendDlgItemMessage(Dialog, IDC_COMBO_REPEATSND, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)s), 0);
				}
				else
				{
					SendDlgItemMessage(Dialog,IDC_COMBO_REPEATSND,CB_SETCURSEL,0,0);
				}
			}

			// populate sound selection combo
			{
				int n = SendDlgItemMessage(Dialog, IDC_COMBO_SOUND, CB_ADDSTRING, 0, (LPARAM)Translate("Default"));
				SendDlgItemMessage(Dialog, IDC_COMBO_SOUND, CB_SETITEMDATA, n, 0);
				n = SendDlgItemMessage(Dialog, IDC_COMBO_SOUND, CB_ADDSTRING, 0, (LPARAM)Translate("Alternative 1"));
				SendDlgItemMessage(Dialog, IDC_COMBO_SOUND, CB_SETITEMDATA, n, 1);
				n = SendDlgItemMessage(Dialog, IDC_COMBO_SOUND, CB_ADDSTRING, 0, (LPARAM)Translate("Alternative 2"));
				SendDlgItemMessage(Dialog, IDC_COMBO_SOUND, CB_SETITEMDATA, n, (LPARAM)2);
				n = SendDlgItemMessage(Dialog, IDC_COMBO_SOUND, CB_ADDSTRING, 0, (LPARAM)Translate("None"));
				SendDlgItemMessage(Dialog, IDC_COMBO_SOUND, CB_SETITEMDATA, n, (LPARAM)-1);

				if (NewReminderVisible == 2 && pEditReminder->SoundSel)
				{
					const UINT n = pEditReminder->SoundSel<0 ? 3 : pEditReminder->SoundSel;
					SendDlgItemMessage(Dialog,IDC_COMBO_SOUND,CB_SETCURSEL,n,0);
				}
				else
				{
					SendDlgItemMessage(Dialog,IDC_COMBO_SOUND,CB_SETCURSEL,0,0);
				}
			}

			hIcon = Skin_GetIconByHandle(iconList[12].hIcolib);
			SendDlgItemMessage(Dialog,IDC_BTN_PLAYSOUND,BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)hIcon);

			if (NewReminderVisible == 2 && pEditReminder->SoundSel)
			{
				EnableWindow(GetDlgItem(Dialog, IDC_BTN_PLAYSOUND), pEditReminder->SoundSel>=0);
				EnableWindow(GetDlgItem(Dialog, IDC_COMBO_REPEATSND), pEditReminder->SoundSel>=0);
			}

			if (NewReminderVisible == 2)
				SetFocus( GetDlgItem(Dialog, IDC_ADDREMINDER) );
			else
				SetFocus( GetDlgItem(Dialog, IDC_REMINDER) );

			return FALSE;
		}
	case WM_NCDESTROY:
		RemoveProp(GetDlgItem(Dialog, IDC_DATE), TEXT("OldWndProc"));
		return TRUE;
	case WM_CLOSE:
		{
			if (NewReminderVisible == 2)
			{
				pEditReminder->RemVisible = FALSE;
			}
			DestroyWindow(Dialog);
			NewReminderVisible = FALSE;
			pEditReminder = NULL;
			return TRUE;
		}
	case WM_NOTIFY:
		{
			if (wParam == IDC_DATE)
			{
				NMLISTVIEW *NM = (NMLISTVIEW*)lParam;

				switch (NM->hdr.code)
				{
				case DTN_DATETIMECHANGE:
					OnDateChanged(Dialog, IDC_DATE, IDC_TIME, IDC_REFTIME);
					break;
				}
			}
		}
		break;
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_TIME:
				switch (HIWORD(wParam))
				{
				case CBN_KILLFOCUS:
					// reformat displayed value
					if (SendDlgItemMessage(Dialog, IDC_TIME, CB_GETCURSEL, 0, 0) == CB_ERR)
					{
						char buf[64];
						int h, m;
						GetDlgItemText(Dialog, IDC_TIME, buf, 30);

						if ( ParseTime(buf, &h, &m, FALSE, IsRelativeCombo(Dialog, IDC_TIME)) )
						{
							SYSTEMTIME Date;
							SendDlgItemMessage(Dialog,IDC_DATE,DTM_GETSYSTEMTIME,0,(LPARAM)&Date);
							ReformatTimeInput(Dialog, IDC_TIME, IDC_REFTIME, h, m, &Date);
						}
						else
						{
							SendDlgItemMessage(Dialog, IDC_TIME, CB_SETCURSEL, 0, 0);
						}
					}
					break;
				}
				break;
			case IDC_COMBO_SOUND:
				switch (HIWORD(wParam))
				{
				case CBN_SELENDOK:
					{
						int n = SendDlgItemMessage(Dialog, IDC_COMBO_SOUND, CB_GETCURSEL, 0, 0);
						n = (int)SendDlgItemMessage(Dialog, IDC_COMBO_SOUND, CB_GETITEMDATA, n, 0);

						EnableWindow(GetDlgItem(Dialog, IDC_BTN_PLAYSOUND), n>=0);
						EnableWindow(GetDlgItem(Dialog, IDC_COMBO_REPEATSND), n>=0);
					}
					break;
				}
				break;

			case IDC_BTN_PLAYSOUND:
				{
					int n = SendDlgItemMessage(Dialog, IDC_COMBO_SOUND, CB_GETCURSEL, 0, 0);
					n = (int)SendDlgItemMessage(Dialog, IDC_COMBO_SOUND, CB_GETITEMDATA, n, 0);
					switch (n)
					{
					case 0: SkinPlaySound("AlertReminder"); break;
					case 1: SkinPlaySound("AlertReminder2"); break;
					case 2: SkinPlaySound("AlertReminder3"); break;
					}
				}
				return TRUE;
			case IDC_CLOSE:
				{
					if (NewReminderVisible == 2)
					{
						pEditReminder->RemVisible = FALSE;
					}
					DestroyWindow(Dialog);
					NewReminderVisible = FALSE;
					pEditReminder = NULL;
					return TRUE;
				}
			case IDC_VIEWREMINDERS:
				{
					ListReminders();
					return TRUE;
				}
			case IDC_ADDREMINDER:
				{
					char *ReminderText = NULL;
					int SzT;
					SYSTEMTIME Date;
					REMINDERDATA* TempRem;
					int RepeatSound;

					SendDlgItemMessage(Dialog,IDC_DATE,DTM_GETSYSTEMTIME,0,(LPARAM)&Date);

					if ( !GetTriggerTime(Dialog, IDC_TIME, IDC_REFTIME, &Date) )
						break;

					RepeatSound = SendDlgItemMessage(Dialog,IDC_COMBO_REPEATSND,CB_GETCURSEL,0,0);
					if (RepeatSound != CB_ERR)
						RepeatSound = SendDlgItemMessage(Dialog,IDC_COMBO_REPEATSND,CB_GETITEMDATA,(WPARAM)RepeatSound,0);
					else
						RepeatSound = 0;

					SzT = SendDlgItemMessage(Dialog,IDC_REMINDER,WM_GETTEXTLENGTH,0,0);
					if (SzT)
					{
						if (SzT > MAX_REMINDER_LEN) SzT = MAX_REMINDER_LEN;
						ReminderText = (char*)malloc(SzT + 1);
						GetDlgItemText(Dialog, IDC_REMINDER, ReminderText, SzT + 1);
					}

					if (NewReminderVisible != 2)
					{
						// new reminder
						TempRem = (REMINDERDATA*)malloc(sizeof(REMINDERDATA));
						TempRem->uid = CreateUid();
						SYSTEMTIMEtoFILETIME(&Date, (FILETIME*)&TempRem->When);
						TempRem->Reminder = ReminderText;
						TempRem->RemVisible = FALSE;
						TempRem->SystemEventQueued = 0;
						TempRem->RepeatSoundTTL = 0;
						TempRem->SoundSel = (int)SendDlgItemMessage(Dialog, IDC_COMBO_SOUND, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(Dialog, IDC_COMBO_SOUND, CB_GETCURSEL, 0, 0), 0);
						TempRem->RepeatSound = TempRem->SoundSel<0 ? 0 : (UINT)RepeatSound;
						TreeAddSorted(&RemindersList,TempRem,ReminderSortCb);
					}
					else
					{
						// update existing reminder

						SYSTEMTIMEtoFILETIME(&Date, (FILETIME*)&pEditReminder->When);
						if (pEditReminder->Reminder)
							free(pEditReminder->Reminder);
						pEditReminder->Reminder = ReminderText;
						pEditReminder->SoundSel = (int)SendDlgItemMessage(Dialog, IDC_COMBO_SOUND, CB_GETITEMDATA, (WPARAM)SendDlgItemMessage(Dialog, IDC_COMBO_SOUND, CB_GETCURSEL, 0, 0), 0);
						pEditReminder->RepeatSound = pEditReminder->SoundSel<0 ? 0 : (UINT)RepeatSound;

						// re-insert tree item sorted
						TreeDelete(&RemindersList,pEditReminder);
						TreeAddSorted(&RemindersList,pEditReminder,ReminderSortCb);

						pEditReminder->RemVisible = FALSE;
					}
					SetDlgItemText(Dialog,IDC_REMINDER,"");
					JustSaveReminders();
					NOTIFY_LIST();

					if (g_CloseAfterAddReminder || NewReminderVisible == 2)
					{
						DestroyWindow(Dialog);
						NewReminderVisible = FALSE;
						pEditReminder = NULL;
					}
				}
			}
		}
	case WM_DESTROY:
		{
			Skin_ReleaseIcon(hIcon);
			break;
		}
	}
  return FALSE;
}

static void InitListView(HWND AHLV)
{
	LV_ITEM lvTIt;
	int I;
    char *S;
	char S1[128];
	REMINDERDATA *pReminder;
	TREEELEMENT *TTE;

	ListView_SetHoverTime(AHLV,700);
	ListView_SetExtendedListViewStyle(AHLV,LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TRACKSELECT);
	ListView_DeleteAllItems(AHLV);

	I = 0;
	TTE = RemindersList;
	while (TTE)
	{
		pReminder = (REMINDERDATA*)TTE->ptrdata;

		lvTIt.mask = LVIF_TEXT;

		GetTriggerTimeString(&pReminder->When, S1, sizeof(S1), TRUE);

		lvTIt.iItem = I;
		lvTIt.iSubItem = 0;
		lvTIt.pszText = S1;
		ListView_InsertItem(AHLV,&lvTIt);
		lvTIt.mask = LVIF_TEXT;
		S = GetPreviewString(pReminder->Reminder);
		lvTIt.iItem = I;
		lvTIt.iSubItem = 1;
		lvTIt.pszText = S;
		ListView_SetItem(AHLV,&lvTIt);

		I++;
		TTE = (TREEELEMENT*)TTE->next;
	}

	ListView_SetItemState(AHLV,0,LVIS_SELECTED,LVIS_SELECTED);
}

void OnListResize(HWND Dialog)
{
	HWND hList, hText, hBtnNew, hBtnClose;
	RECT lr, cr, tr, clsr;
	int th, btnh, btnw, MARGIN;
	POINT org = {0};

	hList = GetDlgItem(Dialog, IDC_LISTREMINDERS);
	hText = GetDlgItem(Dialog, IDC_REMINDERDATA);
	hBtnNew = GetDlgItem(Dialog, IDC_ADDNEWREMINDER);
	hBtnClose = GetDlgItem(Dialog, IDC_CLOSE);

	ClientToScreen(Dialog, &org);
	GetClientRect(Dialog, &cr);

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

	MoveWindow(hList, lr.left, lr.top, lr.right-lr.left, lr.bottom-lr.top, FALSE);
	MoveWindow(hText, tr.left, tr.top, tr.right-tr.left, tr.bottom-tr.top, FALSE);

	MoveWindow(hBtnClose, clsr.left, clsr.top, btnw, btnh, FALSE);
	clsr.left -= btnw + 2;
	MoveWindow(hBtnNew, clsr.left, clsr.top, btnw, btnh, FALSE);

	RedrawWindow(Dialog, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	//UpdateWindow(Dialog);
}

void UpdateGeomFromWnd(HWND Dialog, int *geom, int *colgeom, int nCols)
{
	if (geom)
	{
		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);

		GetWindowPlacement(Dialog, &wp);

		geom[0] = wp.rcNormalPosition.left;
		geom[1] = wp.rcNormalPosition.top;
		geom[2] = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
		geom[3] = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
	}

	if (colgeom)
	{
		int i;
		HWND H = GetDlgItem(Dialog, IDC_LISTREMINDERS);

		for (i=0; i<nCols; i++)
		{
			colgeom[i] = ListView_GetColumnWidth(H, i);
		}
	}
}

static BOOL DoListContextMenu(HWND AhWnd,WPARAM wParam,LPARAM lParam,REMINDERDATA *pReminder)
{
	HWND hwndListView = (HWND)wParam;
	if (hwndListView != GetDlgItem(AhWnd, IDC_LISTREMINDERS)) return FALSE;
	HMENU hMenuLoad = LoadMenu(hinstance, _T("MNU_REMINDERPOPUP"));
	HMENU FhMenu = GetSubMenu(hMenuLoad, 0);

	MENUITEMINFO mii;
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_STATE;
	mii.fState = MFS_DEFAULT;
	if (!pReminder || pReminder->SystemEventQueued)
		mii.fState |= MFS_GRAYED;
	SetMenuItemInfo(FhMenu, ID_CONTEXTMENUREMINDERLISTVIEW_EDIT, FALSE, &mii);

	if (!pReminder)
		EnableMenuItem(FhMenu, IDM_DELETEREMINDER, MF_GRAYED|MF_BYCOMMAND);

    CallService(MS_LANGPACK_TRANSLATEMENU,(DWORD)FhMenu,0);
	TrackPopupMenu(FhMenu,TPM_LEFTALIGN | TPM_RIGHTBUTTON,LOWORD(lParam),HIWORD(lParam),0,AhWnd,0);
	DestroyMenu(hMenuLoad);
	return TRUE;
}

INT_PTR CALLBACK DlgProcViewReminders(HWND Dialog,UINT Message,WPARAM wParam,LPARAM lParam)
{
    LV_COLUMN lvCol;
    NMLISTVIEW *NM;
    char *S;
    int I;

	switch (Message)
	{
	case WM_SIZE:
		{
			OnListResize(Dialog);
			UpdateGeomFromWnd(Dialog, g_reminderListGeom, NULL, 0);
			break;
		}
	case WM_MOVE:
		UpdateGeomFromWnd(Dialog, g_reminderListGeom, NULL, 0);
		break;
	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mm = (MINMAXINFO*)lParam;
			mm->ptMinTrackSize.x = 394;
			mm->ptMinTrackSize.y = 300;
		}
		return 0;
	case WM_RELOAD:
		{
			SetDlgItemText(Dialog, IDC_REMINDERDATA, _T(""));
			InitListView(GetDlgItem(Dialog, IDC_LISTREMINDERS));
			return TRUE;
		}
	case WM_CONTEXTMENU:
		{
			HWND H;
			REMINDERDATA *pReminder = NULL;

			H = GetDlgItem(Dialog,IDC_LISTREMINDERS);
			if ( ListView_GetSelectedCount(H) )
			{
				I = ListView_GetSelectionMark(H);
				if (I != -1)
				{
					pReminder = (REMINDERDATA*)TreeGetAt(RemindersList,I);
				}
			}

			if ( DoListContextMenu(Dialog, wParam, lParam, pReminder) )
				return TRUE;
		}
		break;
	case WM_INITDIALOG:
		{
			HICON hIcon = Skin_GetIconByHandle(iconList[6].hIcolib, ICON_SMALL);
			SendMessage(Dialog, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIcon);
			hIcon = Skin_GetIconByHandle(iconList[6].hIcolib, ICON_BIG);
			SendMessage(Dialog, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIcon);

			TranslateDialogDefault(Dialog);
			SetDlgItemText(Dialog,IDC_REMINDERDATA, _T(""));
			HWND H = GetDlgItem(Dialog,IDC_LISTREMINDERS);
			lvCol.mask = LVCF_TEXT | LVCF_WIDTH;
			S = Translate("Reminder text");
			lvCol.pszText = S;
			lvCol.cx = g_reminderListColGeom[1];
			ListView_InsertColumn(H,0,&lvCol);
			lvCol.mask = LVCF_TEXT | LVCF_WIDTH;
			S = Translate("Date of activation");
			lvCol.pszText = S;
			lvCol.cx = g_reminderListColGeom[0];
			ListView_InsertColumn(H,0,&lvCol);
			InitListView(H);
			SetWindowLongPtr(GetDlgItem(H, 0), GWL_ID, IDC_LISTREMINDERS_HEADER);
			LV = Dialog;

			if (g_reminderListGeom[1] && g_reminderListGeom[2])
			{
				WINDOWPLACEMENT wp;
				wp.length = sizeof(WINDOWPLACEMENT);
				GetWindowPlacement(Dialog, &wp);
				wp.rcNormalPosition.left = g_reminderListGeom[0];
				wp.rcNormalPosition.top = g_reminderListGeom[1];
				wp.rcNormalPosition.right = g_reminderListGeom[2] + g_reminderListGeom[0];
				wp.rcNormalPosition.bottom = g_reminderListGeom[3] + g_reminderListGeom[1];
				SetWindowPlacement(Dialog, &wp);
			}
			return TRUE;
		}
	case WM_CLOSE:
		{
			DestroyWindow(Dialog);
			ListReminderVisible = FALSE;
			return TRUE;
		}
	case WM_NOTIFY:
		{
			if (wParam == IDC_LISTREMINDERS)
			{
				NM = (NMLISTVIEW *)lParam;
				switch (NM->hdr.code)
				{
				case LVN_ITEMCHANGED:
					{
					    S = ((REMINDERDATA*)TreeGetAt(RemindersList,NM->iItem))->Reminder;
					    SetDlgItemText(Dialog,IDC_REMINDERDATA,S);
					}
					break;
				case NM_DBLCLK:
					{
						HWND H;

						H = GetDlgItem(Dialog,IDC_LISTREMINDERS);
						if ( ListView_GetSelectedCount(H) )
						{
							I = ListView_GetSelectionMark(H);
							if (I != -1)
							{
								EditReminder((REMINDERDATA*)TreeGetAt(RemindersList, I));
							}
						}
					}
					break;
				}
			}
			else if (wParam == IDC_LISTREMINDERS_HEADER)
			{
				NMHEADER *NM = (NMHEADER*)lParam;
				switch (NM->hdr.code)
				{
				case HDN_ENDTRACK:
					UpdateGeomFromWnd(Dialog, NULL, g_reminderListColGeom, SIZEOF(g_reminderListColGeom));
					break;
				}
			}
		}
		break;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case ID_CONTEXTMENUREMINDERLISTVIEW_EDIT:
				{
					HWND H;

					H = GetDlgItem(Dialog,IDC_LISTREMINDERS);
					if ( ListView_GetSelectedCount(H) )
					{
						I = ListView_GetSelectionMark(H);
						if (I != -1)
						{
							EditReminder((REMINDERDATA*)TreeGetAt(RemindersList, I));
						}
					}
				}
				return TRUE;
			case IDC_CLOSE:
				{
					DestroyWindow(Dialog);
					ListReminderVisible = FALSE;
					return TRUE;
				}
			case IDM_NEWREMINDER:
			case IDC_ADDNEWREMINDER:
				{
					NewReminder();
					return TRUE;
				}
			case IDM_DELETEALLREMINDERS:
				if (RemindersList && MessageBox(Dialog, TranslateT("Are you sure you want to delete all reminders?"), TranslateT(SECTIONNAME), MB_OKCANCEL) == IDOK)
				{
					SetDlgItemText(Dialog, IDC_REMINDERDATA, _T(""));
					DeleteReminders();
					InitListView(GetDlgItem(Dialog,IDC_LISTREMINDERS));
				}
				return TRUE;
			case IDM_DELETEREMINDER:
				{
					HWND H;

					H = GetDlgItem(Dialog,IDC_LISTREMINDERS);
					if ( ListView_GetSelectedCount(H) )
					{
						I = ListView_GetSelectionMark(H);
						if (I != -1
							&& MessageBox(Dialog, TranslateT("Are you sure you want to delete this reminder?"), TranslateT(SECTIONNAME), MB_OKCANCEL) == IDOK)
						{
							SetDlgItemText(Dialog, IDC_REMINDERDATA, _T(""));
							DeleteReminder((REMINDERDATA*)TreeGetAt(RemindersList, I));
							JustSaveReminders();
							InitListView(H);
						}
					}
					return TRUE;
				}
			}
		}
	case WM_DESTROY:
		Skin_ReleaseIcon((HICON)SendMessage(Dialog, WM_SETICON, ICON_BIG, 0));
		Skin_ReleaseIcon((HICON)SendMessage(Dialog, WM_SETICON, ICON_SMALL, 0));
		break;
	}
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// Email/SMS and WinSock functions

BOOL WS_Init()
{
	WSADATA wsd;
	if (WSAStartup(MAKEWORD(2,2),&wsd)!=0) return FALSE;
	return TRUE;
}

void WS_CleanUp()
{
	WSACleanup();
}

int WS_Send(SOCKET s,char *data,int datalen)
{
	int rlen;
	if ((rlen = send(s,data,datalen,0)) == SOCKET_ERROR) return FALSE;
	return TRUE;
}

unsigned long WS_ResolveName(char *name,WORD *port,int defaultPort)
{
	HOSTENT *lk;
	char *pcolon,*nameCopy;
	DWORD ret;
	nameCopy=_strdup(name);
	if(port != NULL) *port = defaultPort;
	pcolon = strchr(nameCopy,':');
	if(pcolon != NULL)
	{
		if(port != NULL) *port = atoi(pcolon+1);
		*pcolon = 0;
	}
	if (inet_addr(nameCopy) == INADDR_NONE)
	{
		lk = gethostbyname(nameCopy);
		if(lk == 0) return SOCKET_ERROR;
		else {free(nameCopy); return *(u_long*)lk->h_addr_list[0];}
	}
	ret=inet_addr(nameCopy);
	free(nameCopy);
	return ret;
}

void Send(char *user, char *host, char *Msg, char *server)
{
	SOCKADDR_IN sockaddr;
	WORD port;
	char *ch = NULL;
	S = socket(AF_INET,SOCK_STREAM,0);
	if (!server) server = host;
	if ((sockaddr.sin_addr.S_un.S_addr = WS_ResolveName(server,
		&port,25))==SOCKET_ERROR) return;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_family = AF_INET;
	if(connect(S,(SOCKADDR*)&sockaddr,sizeof(sockaddr)) == SOCKET_ERROR) return;
	ch = (char*)malloc(mir_strlen(user) + mir_strlen(host) + 16);
	ch = (char*)realloc(ch,sprintf(ch,"rcpt to:%s@%s\r\n",user,host)); //!!!!!!!!!!
	WS_Send(S,"mail from: \r\n",13);
	WS_Send(S,ch,(int)mir_strlen(ch));
	WS_Send(S,"data\r\n",6);
	WS_Send(S,"From:<REM>\r\n\r\n",14);
	WS_Send(S,Msg,(int)mir_strlen(Msg));
	WS_Send(S,"\r\n.\r\n",5);
	WS_Send(S,"quit\r\n",6);
	SAFE_FREE((void**)&ch);
	closesocket(S);
}
