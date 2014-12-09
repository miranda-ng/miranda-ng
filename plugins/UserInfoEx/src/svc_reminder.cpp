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

#include "commonheaders.h"

/**
* The CEvent structure describes the next anniversary to remind of.
**/

struct CEvent
{
	enum EType { NONE, BIRTHDAY, ANNIVERSARY };

	EType	_eType;
	WORD	_wDaysLeft;

	CEvent();
	CEvent(EType eType, WORD wDaysLeft);

	BYTE operator << (const CEvent& e);
};

typedef struct _REMINDEROPTIONS
{
	WORD	wDaysEarlier;
	BYTE	bPopups;
	BYTE	bCListExtraIcon;
	BYTE	bFlashCList;
	BYTE	bCheckVisibleOnly;
	BYTE	RemindState;
	CEvent	evt;
}
REMINDEROPTIONS, *LPREMINDEROPTIONS;

static HANDLE ExtraIcon = INVALID_HANDLE_VALUE;


static HANDLE	ghCListIA = NULL;
static HANDLE	ghCListIR = NULL;
static HANDLE	ghSettingsChanged = NULL;

static UINT_PTR	ghRemindTimer = 0;
static UINT_PTR	ghRemindDateChangeTimer = 0;

HANDLE ghCListAnnivIcons[11];
HANDLE ghCListBirthdayIcons[11];

static REMINDEROPTIONS	gRemindOpts;

static void UpdateTimer(BYTE bStartup);

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

CEvent::CEvent(EType eType, WORD wDaysLeft)
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

BYTE CEvent::operator << (const CEvent& evt)
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
	HICON hIcon = NULL;

	CHAR szIcon[MAXSETTING];

	switch (evt._eType) {
	case CEvent::BIRTHDAY:
		if (evt._wDaysLeft > 9)
			hIcon = Skin_GetIcon(ICO_RMD_DTBX);
		else {
			mir_snprintf(szIcon, SIZEOF(szIcon), MODNAME"_rmd_dtb%u", evt._wDaysLeft);
			hIcon = Skin_GetIcon(szIcon);
		}
		break;

	case CEvent::ANNIVERSARY:
		if (evt._wDaysLeft > 9)
			hIcon = Skin_GetIcon(ICO_RMD_DTAX);
		else {
			mir_snprintf(szIcon, SIZEOF(szIcon), MODNAME"_rmd_dta%u", evt._wDaysLeft);
			hIcon = Skin_GetIcon(szIcon);
		}
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
		char szIcon[MAXSETTING], *icoName;
		
		switch (evt._eType) {
		case CEvent::BIRTHDAY:
			if (evt._wDaysLeft > 9)
				icoName = ICO_RMD_DTAX;
			else {
				mir_snprintf(szIcon, SIZEOF(szIcon), MODNAME"_rmd_dtb%u", evt._wDaysLeft);
				icoName = szIcon;
			}
			break;

		case CEvent::ANNIVERSARY:
			if (evt._wDaysLeft > 9)
				icoName = ICO_RMD_DTAX;
			else {
				mir_snprintf(szIcon, SIZEOF(szIcon), MODNAME"_rmd_dta%u", evt._wDaysLeft);
				icoName = szIcon;
			}
			break;

		default:	
			return;
		}
		ExtraIcon_SetIcon(ExtraIcon, hContact, icoName);
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

	POPUPDATAT ppd = { 0 };
	ppd.PluginWindowProc = PopupWindowProc;
	ppd.iSeconds = (int)db_get_b(NULL, MODNAME, SET_POPUP_DELAY, 0);

	if (hContact) {
		ppd.lchContact = hContact;
		mir_sntprintf(ppd.lptzContactName, SIZEOF(ppd.lptzContactName),
			_T("%s - %s"), TranslateTS(pszDesc), DB::Contact::DisplayName(hContact));
	}
	else mir_tstrncpy(ppd.lptzContactName, TranslateT("Reminder"), SIZEOF(ppd.lptzContactName));

	mir_tstrncpy(ppd.lptzText, pszMsg, MAX_SECONDLINE);

	ppd.lchIcon = GetAnnivIcon(CEvent(eventType, DaysToAnniv));

	switch (eventType) {
	case CEvent::BIRTHDAY:
		switch (db_get_b(NULL, MODNAME, SET_POPUP_BIRTHDAY_COLORTYPE, POPUP_COLOR_CUSTOM)) {
		case POPUP_COLOR_WINDOWS:
			ppd.colorBack = GetSysColor(COLOR_BTNFACE);
			ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
			break;

		case POPUP_COLOR_CUSTOM:
			ppd.colorBack = db_get_dw(NULL, MODNAME, SET_POPUP_BIRTHDAY_COLOR_BACK, RGB(192, 180, 30));
			ppd.colorText = db_get_dw(NULL, MODNAME, SET_POPUP_BIRTHDAY_COLOR_TEXT, 0);
			break;
		}
		break;

	case CEvent::ANNIVERSARY:
		switch (db_get_b(NULL, MODNAME, SET_POPUP_ANNIVERSARY_COLORTYPE, POPUP_COLOR_CUSTOM)) {
		case POPUP_COLOR_WINDOWS:
			ppd.colorBack = GetSysColor(COLOR_BTNFACE);
			ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
			break;

		case POPUP_COLOR_CUSTOM:
			ppd.colorBack = db_get_dw(NULL, MODNAME, SET_POPUP_ANNIVERSARY_COLOR_BACK, RGB(90, 190, 130));
			ppd.colorText = db_get_dw(NULL, MODNAME, SET_POPUP_ANNIVERSARY_COLOR_TEXT, 0);
			break;
		}
	}
	return PUAddPopupT(&ppd);
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

	TCHAR szMsg[MAX_PATH];

	CLISTEVENT cle = { sizeof(cle) };
	cle.hContact = hContact;
	cle.flags = CLEF_URGENT|CLEF_TCHAR;
	cle.hDbEvent = NULL;

	switch (evt._eType) {
	case CEvent::BIRTHDAY:
		mir_sntprintf(szMsg, SIZEOF(szMsg), TranslateT("%s has %s today."), DB::Contact::DisplayName(hContact), TranslateT("Birthday"));
		cle.hIcon = Skin_GetIcon(ICO_COMMON_BIRTHDAY);
		break;

	case CEvent::ANNIVERSARY:
		mir_sntprintf(szMsg, SIZEOF(szMsg), TranslateT("%s has %s today."), DB::Contact::DisplayName(hContact), TranslateT("an anniversary"));
		cle.hIcon = Skin_GetIcon(ICO_COMMON_ANNIVERSARY);
		break;

	default:
		return;
	}
	cle.ptszTooltip = szMsg;

	// pszService = NULL get error (crash),
	// pszService = "dummy" get 'service not fount' and continue;
	cle.pszService = "dummy";
	cle.lParam = NULL;

	CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
}

/**
* Play a sound for the nearest upcoming anniversary
*
* @param	evt				- structure specifying the next anniversary
*
* @retval	0 if sound was played
* @retval	1 otherwise
**/

static BYTE NotifyWithSound(const CEvent &evt)
{
	if (evt._wDaysLeft <= min(db_get_b(NULL, MODNAME, SET_REMIND_SOUNDOFFSET, DEFVAL_REMIND_SOUNDOFFSET), gRemindOpts.wDaysEarlier)) {
		switch (evt._eType) {
		case CEvent::BIRTHDAY:
			SkinPlaySound(evt._wDaysLeft == 0 ? SOUND_BIRTHDAY_TODAY : SOUND_BIRTHDAY_SOON);
			return 0;

		case CEvent::ANNIVERSARY:
			SkinPlaySound(SOUND_ANNIVERSARY);
			return 0;
		}
	}
	return 1;
}

/***********************************************************************************************************
 * "check for anniversary" functions
 ***********************************************************************************************************/

static LPCTSTR ContactGender(MCONTACT hContact)
{
	switch (GenderOf(hContact)) {
		case 'M': return TranslateT("He");
		case 'F': return TranslateT("She");
	}
	return TranslateT("He/She");
}

static BYTE CheckAnniversaries(MCONTACT hContact, MTime &Now, CEvent &evt, BYTE bNotify)
{
	int numAnniversaries = 0;
	int Diff;
	MAnnivDate mta;
	TCHAR szAnniv[MAX_PATH];
	TCHAR strMsg[MAX_SECONDLINE];
	BYTE bOverflow = FALSE;
	WORD wDaysEarlier;

	if (gRemindOpts.RemindState == REMIND_ANNIV || gRemindOpts.RemindState == REMIND_ALL) {
		for (int i = 0; i < ANID_LAST && !mta.DBGetAnniversaryDate(hContact, i); i++) {
			mta.DBGetReminderOpts(hContact);

			if (mta.RemindOption() != BST_UNCHECKED) {
				wDaysEarlier = (mta.RemindOption() == BST_CHECKED) ? mta.RemindOffset() : -1;
				if (wDaysEarlier == (WORD)-1)
					wDaysEarlier = gRemindOpts.wDaysEarlier;

				Diff = mta.CompareDays(Now);
				if ((Diff >= 0) && (Diff <= wDaysEarlier)) {
					if (evt._wDaysLeft > Diff) {
						evt._wDaysLeft = Diff;
						evt._eType = CEvent::ANNIVERSARY;
					}
					numAnniversaries++;

					// create displayed text for popup
					if (bNotify && !bOverflow) {
						// first anniversary found
						if (numAnniversaries == 1) {
							mir_sntprintf(szAnniv, SIZEOF(szAnniv),
								TranslateT("%s has the following anniversaries:\0"),
								ContactGender(hContact));
							mir_tstrncpy(strMsg, szAnniv, mir_tstrlen(szAnniv));
						}

						switch (Diff) {
						case 0:
							mir_sntprintf(szAnniv, SIZEOF(szAnniv), TranslateT("%d. %s today\0"), mta.Age(), mta.Description());
							break;
						case 1:
							mir_sntprintf(szAnniv, SIZEOF(szAnniv), TranslateT("%d. %s tomorrow\0"), mta.Age() + 1, mta.Description());
							break;
						default:
							mir_sntprintf(szAnniv, SIZEOF(szAnniv), TranslateT("%d. %s in %d days\0"), mta.Age() + 1, mta.Description(), Diff);
						}

						if (mir_tstrlen(szAnniv) >= MAX_SECONDLINE - mir_tstrlen(strMsg)) {
							if (strMsg)
								mir_tstrncat(strMsg, _T("\n...\0"), SIZEOF(strMsg));
							else
								mir_tstrncpy(strMsg, _T("\n...\0"), mir_tstrlen(_T("\n...\0")));
							bOverflow = TRUE;
						}
						else {
							if (strMsg)
								mir_tstrncat(strMsg, _T("\n- \0"), SIZEOF(strMsg));
							else
								mir_tstrncpy(strMsg, _T("\n- \0"), mir_tstrlen(_T("\n- \0")));
							mir_tstrncat(strMsg, szAnniv, SIZEOF(strMsg));
						}
					}
				}
			}
		}
	}

	// show one popup for all anniversaries
	if (numAnniversaries != 0 && bNotify)
		NotifyWithPopup(hContact, CEvent::ANNIVERSARY, Diff, LPGENT("Anniversaries"), strMsg);

	return numAnniversaries != 0;
}

/**
* This function checks, whether a contact has a birthday and it is within the period of time to remind of or not.
*
* @param	hContact		- the contact to check
* @param	Now				- current time
* @param	evt				- the reference to a structure, which retrieves the resulting DTB
* @param	bNotify			- if TRUE, a popup will be displayed for a contact having birthday within the next few days.
* @param	LastAnswer		- this parameter is used for the automatic backup function
*
* @retval	TRUE			- contact has a birthday to remind of
* @retval	FALSE			- contact has no birthday or it is not within the desired period of time.
**/

static BYTE CheckBirthday(MCONTACT hContact, MTime &Now, CEvent &evt, BYTE bNotify, PWORD LastAnwer)
{
	BYTE result = FALSE;

	if (gRemindOpts.RemindState == REMIND_BIRTH || gRemindOpts.RemindState == REMIND_ALL) {
		MAnnivDate mtb;
		if (!mtb.DBGetBirthDate(hContact)) {
			int Diff;
			WORD wDaysEarlier;

			mtb.DBGetReminderOpts(hContact);

			// make backup of each protocol based birthday
			if (db_get_b(NULL, MODNAME, SET_REMIND_SECUREBIRTHDAY, TRUE))
				mtb.BackupBirthday(hContact, NULL, 0, LastAnwer);

			if (mtb.RemindOption() != BST_UNCHECKED) {
				wDaysEarlier = (mtb.RemindOption() == BST_CHECKED) ? mtb.RemindOffset() : -1;
				if (wDaysEarlier == (WORD)-1)
					wDaysEarlier = gRemindOpts.wDaysEarlier;

				Diff = mtb.CompareDays(Now);
				if ((Diff >= 0) && (Diff <= wDaysEarlier)) {
					if (evt._wDaysLeft > Diff) {
						evt._wDaysLeft = Diff;
						evt._eType = CEvent::BIRTHDAY;
					}

					if (bNotify) {
						TCHAR szMsg[MAXDATASIZE];
						WORD cchMsg = 0;

						switch (Diff) {
						case 0:
							cchMsg = mir_sntprintf(szMsg, SIZEOF(szMsg), TranslateT("%s has birthday today."), DB::Contact::DisplayName(hContact));
							break;
						case 1:
							cchMsg = mir_sntprintf(szMsg, SIZEOF(szMsg), TranslateT("%s has birthday tomorrow."), DB::Contact::DisplayName(hContact));
							break;
						default:
							cchMsg = mir_sntprintf(szMsg, SIZEOF(szMsg), TranslateT("%s has birthday in %d days."), DB::Contact::DisplayName(hContact), Diff);
						}
						mir_sntprintf(szMsg + cchMsg, SIZEOF(szMsg) - cchMsg,
							TranslateT("\n%s becomes %d years old."),
							ContactGender(hContact), mtb.Age(&Now) + (Diff > 0));

						NotifyWithPopup(hContact, CEvent::BIRTHDAY, Diff, mtb.Description(), szMsg);
					}
					result = TRUE;
				}
			}
		}
	}
	return result;
}

/**
* This function checks one contact. It is mainly used for clist extra icon rebuild notification handler.
*
* @param	hContact		- the contact to check
* @param	Now				- current time
* @param	evt				- the reference to a structure, which retrieves the resulting DTB
* @param	bNotify			- if TRUE, a popup will be displayed for a contact having birthday within the next few days.
* @param	LastAnswer		- this parameter is used for the automatic backup function
*
* @return	nothing
**/

static void CheckContact(MCONTACT hContact, MTime &Now, CEvent &evt, BYTE bNotify, PWORD LastAnwer = 0)
{
	// ignore meta subcontacts here as their birthday information are collected explicitly
	if (hContact && (!gRemindOpts.bCheckVisibleOnly || !db_get_b(hContact, MOD_CLIST, "Hidden", FALSE)) && !db_mc_isSub(hContact)) {
		CEvent ca;

		if (CheckBirthday(hContact, Now, ca, bNotify, LastAnwer) || CheckAnniversaries(hContact, Now, ca, bNotify)) {
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
	WORD a1 = 0;
	for (MCONTACT hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact))
		CheckContact(hContact, now, evt, notify != NOTIFY_CLIST, &a1);

	if (notify != NOTIFY_CLIST) {
		// play sound for the next anniversary
		NotifyWithSound(evt);

		// popup anniversary list
		if (db_get_b(NULL, MODNAME, SET_ANNIVLIST_POPUP, FALSE))
			DlgAnniversaryListShow(0, 0);

		if (evt._wDaysLeft > gRemindOpts.wDaysEarlier && notify == NOTIFY_NOANNIV)
			NotifyWithPopup(NULL, CEvent::NONE, 0, NULL, TranslateT("No anniversaries to remind of"));
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
	for (int i = 0; i < SIZEOF(ghCListAnnivIcons); i++)
		ghCListAnnivIcons[i] = INVALID_HANDLE_VALUE;

	for (int k = 0; k < SIZEOF(ghCListBirthdayIcons); k++)
		ghCListBirthdayIcons[k] = INVALID_HANDLE_VALUE;

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
		if (!strcmp(pdbcws->szModule, SvcReminderGetMyBirthdayModule())) {
			WORD LastAnswer = IDNONE;
			CheckContact(hContact, now, evt, FALSE, &LastAnswer);
		}
		else CheckContact(hContact, now, evt, FALSE, 0);
	}
	return 0;
}

#define TBB_IDBTN		"CheckAnniv"
#define TBB_ICONAME	TOOLBARBUTTON_ICONIDPREFIX TBB_IDBTN TOOLBARBUTTON_ICONIDPRIMARYSUFFIX

/**
* This function is called by the ME_TTB_MODULELOADED event.
* It adds a set of buttons to the TopToolbar plugin.
*
* @param	none
*
* @return	nothing
**/

void SvcReminderOnTopToolBarLoaded()
{
	TTBButton ttb = { sizeof(ttb) };
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.pszService = MS_USERINFO_REMINDER_CHECK;
	ttb.name = ttb.pszTooltipUp = LPGEN("Check anniversaries");
	ttb.hIconHandleUp = Skin_GetIconHandle(ICO_COMMON_BIRTHDAY);
	TopToolbar_AddButton(&ttb);
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

/**
* This is the service function for MS_USERINFO_REMINDER_AGGRASIVEBACKUP.
*
* @param	hContact		- handle to single contact or NULL to backup all
* @param	lParam			- if 1, the messagebox will not be displayed
*
* return:	0
**/

static INT_PTR BackupBirthdayService(WPARAM hContact, LPARAM lParam)
{
	MAnnivDate mdb;

	if (hContact) {
		if (!mdb.DBGetBirthDate(hContact))
			mdb.BackupBirthday(hContact, NULL, TRUE);
	}
	else {
		WORD a1 = 0;

		//walk through all the contacts stored in the DB
		for (hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact))
			if (!db_mc_isSub(hContact) && !mdb.DBGetBirthDate(hContact))
				mdb.BackupBirthday(hContact, NULL, TRUE, &a1);
	}

	if (lParam != TRUE) {
		MSGBOX mBox;
		mBox.cbSize = sizeof(MSGBOX);
		mBox.hParent = NULL;
		mBox.hiLogo = Skin_GetIcon(ICO_COMMON_BIRTHDAY);
		mBox.uType = MB_ICON_INFO;
		mBox.ptszTitle = TranslateT("Update custom birthday");
		mBox.ptszMsg = TranslateT("Backing up and syncing all birthdays complete!");
		MsgBoxService(NULL, (LPARAM)&mBox);
	}
	return 0;
}

/**
* This function returns a constant pointer to the module the date should be saved to
*
* @param	none
*
* @return	module to write birthday information to, MOD_MBIRTHDAY by default
**/

LPCSTR SvcReminderGetMyBirthdayModule()
{
	return ((db_get_b(NULL, MODNAME, SET_REMIND_BIRTHMODULE, DEFVAL_REMIND_BIRTHMODULE) == 1) ? USERINFO : MOD_MBIRTHDAY);
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

static void UpdateTimer(BYTE bStartup)
{
	LONG	wNotifyInterval =	60 * 60 * (LONG)db_get_w(NULL, MODNAME, SET_REMIND_NOTIFYINTERVAL, DEFVAL_REMIND_NOTIFYINTERVAL);
	MTime	now, last;

	now.GetTimeUTC();

	if (bStartup) {
		last.DBGetStamp(NULL, MODNAME, SET_REMIND_LASTCHECK);

		// if last check occured at least one day before just do it on startup again
		if (now.Year() > last.Year() ||	now.Month() > last.Month() ||	now.Day() > last.Day() || db_get_b(NULL, MODNAME, SET_REMIND_CHECKON_STARTUP, FALSE))
			wNotifyInterval = 5;
		else
			wNotifyInterval -= now.Compare(last);

		ghRemindDateChangeTimer = SetTimer(0, 0, 1000 * 60 * 5, (TIMERPROC)TimerProc_DateChanged);
	}
	else now.DBWriteStamp(NULL, MODNAME, SET_REMIND_LASTCHECK);

	// wait at least 5 seconds before checking at startup, to give miranda a better chance to load faster
	KillTimer(0, ghRemindTimer);
	ghRemindTimer = SetTimer(0, 0, 1000 * wNotifyInterval, TimerProc_Check);
}

/***********************************************************************************************************
 * module loading & unloading
 ***********************************************************************************************************/

void SvcReminderEnable(BYTE bEnable)
{
	if (bEnable) { // Reminder is on
		// init hooks
		if (!ghCListIA)
			ghCListIA = HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, (MIRANDAHOOK)OnCListApplyIcon);

		if (!ghSettingsChanged && !myGlobals.UseDbxTree)
			ghSettingsChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, (MIRANDAHOOK)OnContactSettingChanged);

		// reinit reminder options
		gRemindOpts.RemindState	= db_get_b(NULL, MODNAME, SET_REMIND_ENABLED, DEFVAL_REMIND_ENABLED);
		gRemindOpts.wDaysEarlier = db_get_w(NULL, MODNAME, SET_REMIND_OFFSET, DEFVAL_REMIND_OFFSET);
		gRemindOpts.bCListExtraIcon = db_get_b(NULL, MODNAME, SET_REMIND_EXTRAICON, 1);
		gRemindOpts.bCheckVisibleOnly = db_get_b(NULL, MODNAME, SET_REMIND_CHECKVISIBLE, DEFVAL_REMIND_CHECKVISIBLE);
		gRemindOpts.bFlashCList = db_get_b(NULL, MODNAME, SET_REMIND_FLASHICON, FALSE);
		gRemindOpts.bPopups = ServiceExists(MS_POPUP_ADDPOPUPT) && db_get_b(NULL, MODNAME, SET_POPUP_ENABLED, DEFVAL_POPUP_ENABLED);

		// init the timer
		UpdateTimer(TRUE);
	}
	else { // Reminder is off
		for (MCONTACT hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact))
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

	SvcReminderEnable(db_get_b(NULL, MODNAME, SET_REMIND_ENABLED, DEFVAL_REMIND_ENABLED) != REMIND_OFF);
}

/**
* This function initially loads all required stuff for reminder.
*
* @param	none
*
* @return	nothing
**/

void SvcReminderLoadModule(void)
{
	// init sounds
	SKINSOUNDDESCEX ssd = { 0 };
	ssd.cbSize = sizeof(ssd);
	ssd.pszSection = MODNAME;

	ssd.pszName = SOUND_BIRTHDAY_TODAY;
	ssd.pszDescription = LPGEN("Birthday reminder");
	ssd.pszDefaultFile = "Sounds\\BirthDay.wav";
	Skin_AddSound(&ssd);

	ssd.pszName = SOUND_BIRTHDAY_SOON;
	ssd.pszDescription = LPGEN("Birthday reminder: it's coming");
	ssd.pszDefaultFile = "Sounds\\BirthDayComing.wav";
	Skin_AddSound(&ssd);

	ssd.pszName = SOUND_ANNIVERSARY;
	ssd.pszDescription = LPGEN("Anniversary Reminder");
	ssd.pszDefaultFile = "Sounds\\Reminder.wav";
	Skin_AddSound(&ssd);

	// create service functions
	CreateServiceFunction(MS_USERINFO_REMINDER_CHECK, CheckService);
	CreateServiceFunction(MS_USERINFO_REMINDER_AGGRASIVEBACKUP, BackupBirthdayService);

	// register hotkey
	HOTKEYDESC hk = { 0 };
	hk.cbSize = sizeof(HOTKEYDESC);
	hk.pszSection = MODNAME;
	hk.pszName = "ReminderCheck";
	hk.pszDescription = LPGEN("Check anniversaries");
	hk.pszService = MS_USERINFO_REMINDER_CHECK;
	Hotkey_Register(&hk);

	if (db_get_b(NULL, MODNAME, SET_REMIND_ENABLED, DEFVAL_REMIND_ENABLED) != REMIND_OFF && ExtraIcon == INVALID_HANDLE_VALUE)
		ExtraIcon = ExtraIcon_Register("Reminder", LPGEN("Reminder (uinfoex)"), ICO_COMMON_ANNIVERSARY);

}

/**
* This function unloads the reminder module.
*
* @param	none
*
* @return	nothing
**/

void SvcReminderUnloadModule(void)
{
	// kill timers
	KillTimer(0, ghRemindTimer);
	ghRemindTimer = 0;
	KillTimer(0, ghRemindDateChangeTimer);
	ghRemindDateChangeTimer = 0;

	// unhook event handlers
	UnhookEvent(ghCListIR);
	ghCListIR = 0;
	UnhookEvent(ghCListIA);
	ghCListIA = 0;
	UnhookEvent(ghSettingsChanged);
	ghSettingsChanged = 0;
}
