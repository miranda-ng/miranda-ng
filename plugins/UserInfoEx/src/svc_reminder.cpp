/*
UserinfoEx plugin for Miranda IM

Copyright:
� 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/**
 * System Includes:
 **/

#include "stdafx.h"

/**
* The CEvent structure describes the next anniversary to remind of.
**/

struct CEvent
{
	enum EType { NONE, BIRTHDAY, ANNIVERSARY };

	EType	_eType;
	uint16_t	_wDaysLeft;

	CEvent();
	CEvent(EType eType, uint16_t wDaysLeft);

	uint8_t operator << (const CEvent& e);
};

struct REMINDEROPTIONS
{
	uint16_t	wDaysEarlier;
	uint8_t	bPopups;
	uint8_t	bCListExtraIcon;
	uint8_t	bFlashCList;
	uint8_t	bCheckVisibleOnly;
	uint8_t	RemindState;
	CEvent	evt;
};

static HANDLE ExtraIcon = INVALID_HANDLE_VALUE;

static HANDLE ghCListIA = nullptr;
static HANDLE ghCListIR = nullptr;
static HANDLE ghSettingsChanged = nullptr;

static UINT_PTR ghRemindTimer = 0;
static UINT_PTR ghRemindDateChangeTimer = 0;

HANDLE ghCListAnnivIcons[11];
HANDLE ghCListBirthdayIcons[11];

static REMINDEROPTIONS	gRemindOpts;

static void UpdateTimer(uint8_t bStartup);

/***********************************************************************************************************
 * struct CEvent
 ***********************************************************************************************************/

/**
* This is the default constructor.
*
* @param	none
*
* @return nothing
**/

CEvent::CEvent()
{
	_wDaysLeft = 0xFFFF;
	_eType = NONE;
}

/**
* This is the default constructor.
*
* @param	eType			- initial type
* @param	wDaysLeft		- initial days to event
*
* @return	nothing
**/

CEvent::CEvent(EType eType, uint16_t wDaysLeft)
{
	_wDaysLeft = wDaysLeft;
	_eType = eType;
}

/**
* This operator dups the attributes of the given CEvent object if
* the event comes up earlier then the one of the object.
*
* @param	evt				- the reference to the event object whose attributes to assign.
*
* @retval	TRUE			- The values of @e evt have been assigned.
* @retval	FALSE			- The values are not assigned.
**/

uint8_t CEvent::operator << (const CEvent& evt)
{
	if (_wDaysLeft > evt._wDaysLeft) {
		_wDaysLeft = evt._wDaysLeft;
		_eType = evt._eType;
		return TRUE;
	}
	return FALSE;
}

/***********************************************************************************************************
 * notification functions
 ***********************************************************************************************************/

/**
* This function returns the icon for the given anniversary,
* which is the given number of days in advance.
*
* @param	evt				- structure specifying the next anniversary
*
* @return	The function returns icolib's icon if found or NULL otherwise.
**/

static HICON GetAnnivIcon(const CEvent &evt)
{
	HICON hIcon = nullptr;

	switch (evt._eType) {
	case CEvent::BIRTHDAY:
		if (evt._wDaysLeft > 9)
			hIcon = g_plugin.getIcon(IDI_RMD_DTBX);
		else
			hIcon = g_plugin.getIcon(IDI_RMD_DTB0 + evt._wDaysLeft);
		break;

	case CEvent::ANNIVERSARY:
		if (evt._wDaysLeft > 9)
			hIcon = g_plugin.getIcon(IDI_RMD_DTAX);
		else
			hIcon = g_plugin.getIcon(IDI_RMD_DTA0 + evt._wDaysLeft);
	}
	return hIcon;
}

/**
* Displays an clist extra icon according to the kind of anniversary
* and the days in advance.
*
* @param	evt				- structure specifying the next anniversary
*
* @return	nothing
**/

static void NotifyWithExtraIcon(MCONTACT hContact, const CEvent &evt)
{
	if (gRemindOpts.bCListExtraIcon) {
		int iIcon;
	
		switch (evt._eType) {
		case CEvent::BIRTHDAY:
			if (evt._wDaysLeft > 9)
				iIcon = IDI_RMD_DTBX;
			else
				iIcon = IDI_RMD_DTB0 + evt._wDaysLeft;
			break;

		case CEvent::ANNIVERSARY:
			if (evt._wDaysLeft > 9)
				iIcon = IDI_RMD_DTAX;
			else
				iIcon = IDI_RMD_DTA0 + evt._wDaysLeft;
			break;

		default:	
			return;
		}
		ExtraIcon_SetIcon(ExtraIcon, hContact, g_plugin.getIconHandle(iIcon));
	}
}

/**
* Message procedure for popup messages
*
* @param	hWnd			- handle to the popupwindow
* @param	uMsg			- message to handle
* @param	wParam			- message specific parameter
* @param	lParam			- message specific parameter
*
* @return	message specific
**/

static LRESULT CALLBACK PopupWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED) {
			PUDeletePopup(hWnd);
			return TRUE;
		}
		break;

	case WM_CONTEXTMENU:
		PUDeletePopup(hWnd);
		return TRUE;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/**
* Displays a popup
*
* @param	hContact		- contact to display popup for
* @param	eventType		- indicates which popup settings to apply
* @param	DaysToAnniv		- days left until anniversary occures
* @param	pszDesc			- this is the headline
* @param	szMsg			- message to display
*
* @return	return value of the popup service
**/

static int NotifyWithPopup(MCONTACT hContact, CEvent::EType eventType, int DaysToAnniv, LPCTSTR pszDesc, LPCTSTR pszMsg)
{
	if (!gRemindOpts.bPopups)
		return 1;

	POPUPDATAW ppd = {};
	ppd.PluginWindowProc = PopupWindowProc;
	ppd.iSeconds = g_plugin.iPopupDelay;

	if (hContact) {
		ppd.lchContact = hContact;
		mir_snwprintf(ppd.lpwzContactName, L"%s - %s", TranslateW(pszDesc), Clist_GetContactDisplayName(hContact));
	}
	else mir_wstrncpy(ppd.lpwzContactName, TranslateT("Reminder"), _countof(ppd.lpwzContactName));

	mir_wstrncpy(ppd.lpwzText, pszMsg, MAX_SECONDLINE);

	ppd.lchIcon = GetAnnivIcon(CEvent(eventType, DaysToAnniv));

	switch (eventType) {
	case CEvent::BIRTHDAY:
		switch (g_plugin.iBirthClrType) {
		case POPUP_COLOR_WINDOWS:
			ppd.colorBack = GetSysColor(COLOR_BTNFACE);
			ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
			break;

		case POPUP_COLOR_CUSTOM:
			ppd.colorBack = g_plugin.clrBback;
			ppd.colorText = g_plugin.clrBtext;
			break;
		}
		break;

	case CEvent::ANNIVERSARY:
		switch (g_plugin.iAnnivClrType) {
		case POPUP_COLOR_WINDOWS:
			ppd.colorBack = GetSysColor(COLOR_BTNFACE);
			ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
			break;

		case POPUP_COLOR_CUSTOM:
			ppd.colorBack = g_plugin.clrAback;
			ppd.colorText = g_plugin.clrAtext;
			break;
		}
	}
	return (INT_PTR)PUAddPopupW(&ppd);
}

/**
* Flash contact list's contact icon.
*
* @param	hContact		- contact whose icon to flash
* @param	evt				- structure specifying the next anniversary
*
* @return	nothing
**/

static void NotifyFlashCListIcon(MCONTACT hContact, const CEvent &evt)
{
	if (!gRemindOpts.bFlashCList || evt._wDaysLeft != 0)
		return;

	wchar_t szMsg[MAX_PATH];

	CLISTEVENT cle = {};
	cle.hContact = hContact;
	cle.flags = CLEF_URGENT|CLEF_UNICODE;
	cle.hDbEvent = NULL;

	switch (evt._eType) {
	case CEvent::BIRTHDAY:
		mir_snwprintf(szMsg, TranslateT("%s has %s today."), Clist_GetContactDisplayName(hContact), TranslateT("Birthday"));
		cle.hIcon = g_plugin.getIcon(IDI_BIRTHDAY);
		break;

	case CEvent::ANNIVERSARY:
		mir_snwprintf(szMsg, TranslateT("%s has %s today."), Clist_GetContactDisplayName(hContact), TranslateT("an anniversary"));
		cle.hIcon = g_plugin.getIcon(IDI_ANNIVERSARY);
		break;

	default:
		return;
	}
	cle.szTooltip.w = szMsg;

	// pszService = NULL get error (crash),
	// pszService = "dummy" get 'service not fount' and continue;
	cle.pszService = "dummy";
	cle.lParam = NULL;
	g_clistApi.pfnAddEvent(&cle);
}

/**
* Play a sound for the nearest upcoming anniversary
*
* @param	evt				- structure specifying the next anniversary
*
* @retval	0 if sound was played
* @retval	1 otherwise
**/

static uint8_t NotifyWithSound(const CEvent &evt)
{
	if (evt._wDaysLeft <= min(g_plugin.wRemindSoundOffset, gRemindOpts.wDaysEarlier)) {
		switch (evt._eType) {
		case CEvent::BIRTHDAY:
			Skin_PlaySound(evt._wDaysLeft == 0 ? SOUND_BIRTHDAY_TODAY : SOUND_BIRTHDAY_SOON);
			return 0;

		case CEvent::ANNIVERSARY:
			Skin_PlaySound(SOUND_ANNIVERSARY);
			return 0;
		}
	}
	return 1;
}

/***********************************************************************************************************
 * "check for anniversary" functions
 ***********************************************************************************************************/

static uint8_t CheckAnniversaries(MCONTACT hContact, MTime &Now, CEvent &evt, uint8_t bNotify)
{
	int numAnniversaries = 0;
	int Diff = 0;
	MAnnivDate mta;
	CMStringW tszMsg;

	if (gRemindOpts.RemindState == REMIND_ANNIV || gRemindOpts.RemindState == REMIND_ALL) {
		for (int i = 0; i < ANID_LAST && !mta.DBGetAnniversaryDate(hContact, i); i++) {
			mta.DBGetReminderOpts(hContact);

			if (mta.RemindOption() != BST_UNCHECKED) {
				uint16_t wDaysEarlier = (mta.RemindOption() == BST_CHECKED) ? mta.RemindOffset() : -1;
				if (wDaysEarlier == (uint16_t)-1)
					wDaysEarlier = gRemindOpts.wDaysEarlier;

				Diff = mta.CompareDays(Now);
				if ((Diff >= 0) && (Diff <= wDaysEarlier)) {
					if (evt._wDaysLeft > Diff) {
						evt._wDaysLeft = Diff;
						evt._eType = CEvent::ANNIVERSARY;
					}
					numAnniversaries++;

					// create displayed text for popup
					if (bNotify) {
						// first anniversary found
						if (numAnniversaries == 1)
							switch (GenderOf(hContact)){
							case 0:
								tszMsg += TranslateT("He/she has the following anniversaries:");
								break;
							case 'M':
								tszMsg += TranslateT("He has the following anniversaries:");
								break;
							case 'F':
								tszMsg += TranslateT("She has the following anniversaries:");
								break;
							}						
						tszMsg.Append(L"\n- ");

						switch (Diff) {
						case 0:
							tszMsg.AppendFormat(TranslateT("%d. %s today"), mta.Age(), mta.Description());
							break;
						case 1:
							tszMsg.AppendFormat(TranslateT("%d. %s tomorrow"), mta.Age() + 1, mta.Description());
							break;
						default:
							tszMsg.AppendFormat(TranslateT("%d. %s in %d days"), mta.Age() + 1, mta.Description(), Diff);
						}
					}
				}
			}
		}
	}

	// show one popup for all anniversaries
	if (numAnniversaries != 0 && bNotify) {
		if (tszMsg.GetLength() >= MAX_SECONDLINE) {
			tszMsg.Truncate(MAX_SECONDLINE - 5);
			tszMsg.Append(L"\n...");
		}

		NotifyWithPopup(hContact, CEvent::ANNIVERSARY, Diff, LPGENW("Anniversaries"), tszMsg);
	}

	return numAnniversaries != 0;
}

/**
* This function checks, whether a contact has a birthday and it is within the period of time to remind of or not.
*
* @param	hContact		- the contact to check
* @param	Now				- current time
* @param	evt				- the reference to a structure, which retrieves the resulting DTB
* @param	bNotify			- if TRUE, a popup will be displayed for a contact having birthday within the next few days.
*
* @retval	TRUE			- contact has a birthday to remind of
* @retval	FALSE			- contact has no birthday or it is not within the desired period of time.
**/

static bool CheckBirthday(MCONTACT hContact, MTime &Now, CEvent &evt, uint8_t bNotify)
{
	if (gRemindOpts.RemindState == REMIND_BIRTH || gRemindOpts.RemindState == REMIND_ALL) {
		MAnnivDate mtb;
		if (!mtb.DBGetBirthDate(hContact)) {
			int Diff;

			mtb.DBGetReminderOpts(hContact);

			if (mtb.RemindOption() != BST_UNCHECKED) {
				uint16_t wDaysEarlier = (mtb.RemindOption() == BST_CHECKED) ? mtb.RemindOffset() : -1;
				if (wDaysEarlier == (uint16_t)-1)
					wDaysEarlier = gRemindOpts.wDaysEarlier;

				Diff = mtb.CompareDays(Now);
				if ((Diff >= 0) && (Diff <= wDaysEarlier)) {
					if (evt._wDaysLeft > Diff) {
						evt._wDaysLeft = Diff;
						evt._eType = CEvent::BIRTHDAY;
					}

					if (bNotify) {
						wchar_t szMsg[MAXDATASIZE];
						uint16_t cchMsg = 0;

						switch (Diff) {
						case 0:
							cchMsg = mir_snwprintf(szMsg, TranslateT("%s has birthday today."), Clist_GetContactDisplayName(hContact));
							break;
						case 1:
							cchMsg = mir_snwprintf(szMsg, TranslateT("%s has birthday tomorrow."), Clist_GetContactDisplayName(hContact));
							break;
						default:
							cchMsg = mir_snwprintf(szMsg, TranslateT("%s has birthday in %d days."), Clist_GetContactDisplayName(hContact), Diff);
						}
						int age = mtb.Age(&Now);
						if (age > 0)
							switch (GenderOf(hContact)){
							case 0:
								mir_snwprintf(szMsg + cchMsg, _countof(szMsg) - cchMsg,
									TranslateT("\nHe/she becomes %d years old."),
									age + (Diff > 0));
								break;
							case 'M':
								mir_snwprintf(szMsg + cchMsg, _countof(szMsg) - cchMsg,
									TranslateT("\nHe becomes %d years old."),
									age + (Diff > 0));
								break;
							case 'F':
								mir_snwprintf(szMsg + cchMsg, _countof(szMsg) - cchMsg,
									TranslateT("\nShe becomes %d years old."),
									age + (Diff > 0));
								break;
							}
						NotifyWithPopup(hContact, CEvent::BIRTHDAY, Diff, mtb.Description(), szMsg);
					}
					return true;
				}
			}
		}
	}
	return false;
}

/**
* This function checks one contact. It is mainly used for clist extra icon rebuild notification handler.
*
* @param	hContact		- the contact to check
* @param	Now				- current time
* @param	evt				- the reference to a structure, which retrieves the resulting DTB
* @param	bNotify			- if TRUE, a popup will be displayed for a contact having birthday within the next few days.
*
* @return	nothing
**/

static void CheckContact(MCONTACT hContact, MTime &Now, CEvent &evt, uint8_t bNotify)
{
	// ignore meta subcontacts here as their birthday information are collected explicitly
	if (hContact && (!gRemindOpts.bCheckVisibleOnly || !Contact::IsHidden(hContact)) && !db_mc_isSub(hContact)) {
		CEvent ca;

		if (CheckBirthday(hContact, Now, ca, bNotify) || CheckAnniversaries(hContact, Now, ca, bNotify)) {
			evt << ca;
			if (bNotify)
				NotifyFlashCListIcon(hContact, ca);
		}
		NotifyWithExtraIcon(hContact, ca);
	}
}

/**
* This function checks all contacts.
*
* @param	notify			- notification type
*
* @return	nothing
**/

void SvcReminderCheckAll(const ENotify notify)
{
	if (gRemindOpts.RemindState == REMIND_OFF)
		return;

	MTime now;
	now.GetLocalTime();

	// walk through all the contacts stored in the DB
	CEvent evt;
	for (auto &hContact : Contacts())
		CheckContact(hContact, now, evt, notify != NOTIFY_CLIST);

	if (notify != NOTIFY_CLIST) {
		// play sound for the next anniversary
		NotifyWithSound(evt);

		// popup anniversary list
		if (g_plugin.getByte(SET_ANNIVLIST_POPUP, FALSE))
			DlgAnniversaryListShow(0, 0);

		if (evt._wDaysLeft > gRemindOpts.wDaysEarlier && notify == NOTIFY_NOANNIV)
			NotifyWithPopup(NULL, CEvent::NONE, 0, nullptr, TranslateT("No anniversaries to remind of"));
	}
	UpdateTimer(FALSE);
}

/***********************************************************************************************************
 * Event Handler functions
 ***********************************************************************************************************/

/**
* This is the notification handler to tell reminder to reload required icons.
* The reminder only loads icons to clist, which are really required at the moment.
* This should help to save a bit memory.
*
* @param:	wParam			- not used
* @param:	lParam			- not used
*
* @return	This function must return 0 in order to continue in the notification chain.
**/

static int OnCListRebuildIcons(WPARAM, LPARAM)
{
	for (auto &it : ghCListAnnivIcons)
		it = INVALID_HANDLE_VALUE;

	for (auto &it : ghCListBirthdayIcons)
		it = INVALID_HANDLE_VALUE;

	return 0;
}

/**
* This function is the notification handler for clist extra icons to be applied for a contact.
*
* @param	hContact		- handle to the contact whose extra icon is to apply
* @param	lParam			- not used
*
* @return	This function must return 0 in order to continue in the notification chain.
**/

int OnCListApplyIcon(MCONTACT hContact, LPARAM)
{
	if (gRemindOpts.RemindState != REMIND_OFF) {
		CEvent evt;
		MTime now;
		now.GetLocalTime();
		CheckContact(hContact, now, evt, FALSE);
	}
	return 0;
}

/**
* This is a notification handler for changed contact settings.
* If any anniversary setting has changed for a meta sub contact,
* the parental meta contact is rescanned.
*
* @param	hContact		- handle of the contect the notification was fired for
* @param	pdbcws			- pointer to a DBCONTACTWRITESETTING structure
*
* @return	This function must return 0 in order to continue in the notification chain.
**/

static int OnContactSettingChanged(MCONTACT hContact, DBCONTACTWRITESETTING* pdbcws)
{
	if (hContact &&										// valid contact not owner!
			ghCListIA &&								// extraicons active
			pdbcws && pdbcws->szSetting &&				// setting structure valid
			(pdbcws->value.type < DBVT_DWORD) &&		// anniversary datatype
			(gRemindOpts.RemindState != REMIND_OFF) &&	// reminder active
			(!strncmp(pdbcws->szSetting, "Birth", 5) ||
			 !strncmp(pdbcws->szSetting, "Anniv", 5) ||
			 !strncmp(pdbcws->szSetting, "DOB", 3)))
	{
		// check metacontact instead of subcontact
		hContact = db_mc_tryMeta(hContact);

		CEvent evt;
		MTime now;
		now.GetLocalTime();
		CheckContact(hContact, now, evt, FALSE);
	}
	return 0;
}

/***********************************************************************************************************
 * services
 ***********************************************************************************************************/

/**
* This is the service function for MS_USERINFO_REMINDER_CHECK.
*
* @param:	wParam			- not used
* @param:	lParam			- not used
*
* @return	0
**/

static INT_PTR CheckService(WPARAM, LPARAM)
{
	if (gRemindOpts.RemindState != REMIND_OFF)
		SvcReminderCheckAll(NOTIFY_NOANNIV);
	return 0;
}

/***********************************************************************************************************
 * timer stuff
 ***********************************************************************************************************/

/**
* Timer procedure, called if date changed. This updates clist icons.
*
* @param	hwnd			- not used
* @param	uMsg			- not used
* @param	idEvent			- not used
* @param	dwTime			- not used
* @return	nothing
**/

static void CALLBACK TimerProc_DateChanged(HWND, UINT, UINT_PTR, DWORD)
{
	static MTime last;
	MTime now;
	now.GetLocalTime();
	if (now.Day() > last.Day() || now.Month() > last.Month() || now.Year() > last.Year()) {
		SvcReminderCheckAll(NOTIFY_CLIST);
		last = now;
	}
}

/**
* Timer procedure, called again and again if the notification interval ellapsed
*
* @param	hwnd			- not used
* @param	uMsg			- not used
* @param	idEvent			- not used
* @param	dwTime			- not used
*
* @return	nothing
**/

static void CALLBACK TimerProc_Check(HWND, UINT, UINT_PTR, DWORD)
{
	SvcReminderCheckAll(NOTIFY_POPUP);
}

/**
* Load timers or update them.
*
* @param	bStartup		- is only TRUE if module is loaded to indicate startup process
*
* @return	nothing
**/

static void UpdateTimer(uint8_t bStartup)
{
	LONG	wNotifyInterval =	60 * 60 * (LONG)g_plugin.wRemindNotifyInterval;
	MTime	now, last;

	now.GetTimeUTC();

	if (bStartup) {
		last.DBGetStamp(0, MODULENAME, SET_REMIND_LASTCHECK);

		// if last check occured at least one day before just do it on startup again
		if (now.Year() > last.Year() ||	now.Month() > last.Month() ||	now.Day() > last.Day() || g_plugin.bRemindFlashIcon)
			wNotifyInterval = 5;
		else
			wNotifyInterval -= now.Compare(last);

		ghRemindDateChangeTimer = SetTimer(nullptr, 0, 1000 * 60 * 5, (TIMERPROC)TimerProc_DateChanged);
	}
	else now.DBWriteStamp(0, MODULENAME, SET_REMIND_LASTCHECK);

	// wait at least 5 seconds before checking at startup, to give miranda a better chance to load faster
	KillTimer(nullptr, ghRemindTimer);
	ghRemindTimer = SetTimer(nullptr, 0, 1000 * wNotifyInterval, TimerProc_Check);
}

/***********************************************************************************************************
 * module loading & unloading
 ***********************************************************************************************************/

void SvcReminderEnable(bool bEnable)
{
	if (bEnable) { // Reminder is on
		// init hooks
		if (!ghCListIA)
			ghCListIA = HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, (MIRANDAHOOK)OnCListApplyIcon);

		if (!ghSettingsChanged)
			ghSettingsChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, (MIRANDAHOOK)OnContactSettingChanged);

		// reinit reminder options
		gRemindOpts.RemindState	= g_plugin.iRemindState;
		gRemindOpts.wDaysEarlier = g_plugin.wRemindOffset;
		gRemindOpts.bCListExtraIcon = g_plugin.bRemindExtraIcon;
		gRemindOpts.bCheckVisibleOnly = g_plugin.bRemindCheckVisible;
		gRemindOpts.bFlashCList = g_plugin.bRemindFlashIcon;
		gRemindOpts.bPopups = g_plugin.bPopupEnabled;

		// init the timer
		UpdateTimer(TRUE);
	}
	else { // Reminder is off
		for (auto &hContact : Contacts())
			NotifyWithExtraIcon(hContact, CEvent());

		gRemindOpts.RemindState	= REMIND_OFF;
		SvcReminderUnloadModule();
	}
}

/**
* This function is called by Miranda just after loading all system modules.
*
* @param	none
*
* @return	nothing
**/

void SvcReminderOnModulesLoaded(void)
{
	// init clist extra icon structure
	OnCListRebuildIcons(0, 0);

	SvcReminderEnable(g_plugin.iRemindState != REMIND_OFF);
}

/**
* This function initially loads all required stuff for reminder.
**/

void SvcReminderLoadModule(void)
{
	// init sounds
	g_plugin.addSound(SOUND_BIRTHDAY_TODAY, LPGENW("Reminders"), LPGENW("Birthday reminder"), L"Sounds\\BirthDay.wav");
	g_plugin.addSound(SOUND_BIRTHDAY_SOON, LPGENW("Reminders"), LPGENW("Birthday reminder: it's coming"), L"Sounds\\BirthDayComing.wav");
	g_plugin.addSound(SOUND_ANNIVERSARY, LPGENW("Reminders"), LPGENW("Anniversary reminder"), L"Sounds\\Reminder.wav");

	// create service functions
	CreateServiceFunction(MS_USERINFO_REMINDER_CHECK, CheckService);

	// register hotkey
	HOTKEYDESC hk = {};
	hk.pszName = "ReminderCheck";
	hk.szSection.a = MODULENAME;
	hk.szDescription.a = LPGEN("Check anniversaries");
	hk.pszService = MS_USERINFO_REMINDER_CHECK;
	g_plugin.addHotkey(&hk);

	if (g_plugin.iRemindState != REMIND_OFF && ExtraIcon == INVALID_HANDLE_VALUE)
		ExtraIcon = ExtraIcon_RegisterIcolib("Reminder", LPGEN("Reminder (UInfoEx)"), g_plugin.getIconHandle(IDI_ANNIVERSARY));
}

/**
* This function unloads the reminder module.
**/

void SvcReminderUnloadModule(void)
{
	// kill timers
	KillTimer(nullptr, ghRemindTimer);
	ghRemindTimer = 0;
	KillTimer(nullptr, ghRemindDateChangeTimer);
	ghRemindDateChangeTimer = 0;

	// unhook event handlers
	UnhookEvent(ghCListIR);
	ghCListIR = nullptr;
	UnhookEvent(ghCListIA);
	ghCListIA = nullptr;
	UnhookEvent(ghSettingsChanged);
	ghSettingsChanged = nullptr;
}
