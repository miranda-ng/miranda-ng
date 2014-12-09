/*
	Buddy Expectator+ plugin for Miranda-IM (www.miranda-im.org)
	(c)2005 Anar Ibragimoff (ai91@mail.ru)
	(c)2006 Scott Ellis (mail@scottellis.com.au)
	(c)2007,2008 Alexander Turyak (thief@miranda-im.org.ua)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "common.h"

HINSTANCE hInst;
int hLangpack;

DWORD timer_id = 0;

HANDLE hContactReturnedAction, hContactStillAbsentAction, hMissYouAction, hMenuMissYouClick;
HGENMENU hContactMenu;

HICON hIcon;
HANDLE hExtraIcon;

// Popup Actions
POPUPACTION missyouactions[1];
POPUPACTION hideactions[2];

extern int UserinfoInit(WPARAM wparam, LPARAM lparam);

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {DDF8AEC9-7D37-49AF-9D22-BBBC920E6F05}
	{0xddf8aec9, 0x7d37, 0x49af, {0x9d, 0x22, 0xbb, 0xbc, 0x92, 0x0e, 0x6f, 0x05}}
};

static IconItem iconList[] =
{
	{ LPGEN("Tray/popup icon"), "main_icon",      IDI_MAINICON  },
	{ LPGEN("Enabled"),         "enabled_icon",   IDI_ENABLED   },
	{ LPGEN("Disabled"),        "disabled_icon",  IDI_DISABLED  },
	{ LPGEN("Hide"),            "hide_icon",      IDI_HIDE      },
	{ LPGEN("Never hide"),       "neverhide_icon", IDI_NEVERHIDE }
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

time_t getLastSeen(MCONTACT hContact)
{
	return db_get_dw(hContact, MODULE_NAME, "LastSeen", db_get_dw(hContact, MODULE_NAME, "CreationTime", (DWORD)-1));
}

void setLastSeen(MCONTACT hContact)
{
	db_set_dw(hContact, MODULE_NAME, "LastSeen", (DWORD)time(NULL));
	if (db_get_b(hContact, MODULE_NAME, "StillAbsentNotified", 0))
		db_set_b(hContact, MODULE_NAME, "StillAbsentNotified", 0);
}

time_t getLastInputMsg(MCONTACT hContact)
{
	HANDLE hDbEvent = db_event_last(hContact);
	while (hDbEvent) {
		DBEVENTINFO dbei = { sizeof(dbei) };
		db_event_get(hDbEvent, &dbei);
		if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT))
			return dbei.timestamp;
		hDbEvent = db_event_prev(hContact, hDbEvent);
	}
	return -1;
}

/**
 * Popup window procedures
 */

LRESULT CALLBACK HidePopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED) {
			db_set_b(PUGetContact(hWnd), "CList", "Hidden", 1);
			PUDeletePopup(hWnd);
		}
		break;

	case WM_CONTEXTMENU:
		db_set_b(PUGetContact(hWnd), MODULE_NAME, "NeverHide", 1);
		PUDeletePopup(hWnd);
		break;

	case UM_POPUPACTION:
		if (wParam == 2) {
			db_set_b(PUGetContact(hWnd), "CList", "Hidden", 1);
			PUDeletePopup(hWnd);
		}
		if (wParam == 3) {
			db_set_b(PUGetContact(hWnd), MODULE_NAME, "NeverHide", 1);
			PUDeletePopup(hWnd);
		}
		break;

	case UM_FREEPLUGINDATA:
		return TRUE;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK MissYouPopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED) {
			CallServiceSync("BuddyExpectator/actionMissYou", (WPARAM)PUGetContact(hWnd), 0);
			if ( !db_get_b(PUGetContact(hWnd), MODULE_NAME, "MissYouNotifyAlways", 0)) {
				db_set_b(PUGetContact(hWnd), MODULE_NAME, "MissYou", 0);
				ExtraIcon_Clear(hExtraIcon, PUGetContact(hWnd));
			}
			PUDeletePopup(hWnd);
		}
		break;

	case WM_CONTEXTMENU:
		PUDeletePopup(hWnd);
		break;

	case UM_POPUPACTION:
		if (wParam == 1) {
			db_set_b(PUGetContact(hWnd), MODULE_NAME, "MissYou", 0);
			ExtraIcon_Clear(hExtraIcon, PUGetContact(hWnd));
			PUDeletePopup(hWnd);
		}
		break;

	case UM_FREEPLUGINDATA:
		return TRUE;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED) {
			CallServiceSync(MS_CLIST_REMOVEEVENT, (WPARAM)PUGetContact(hWnd), 0);
			CallServiceSync("BuddyExpectator/actionReturned", (WPARAM)PUGetContact(hWnd), 0);
			PUDeletePopup(hWnd);
		}
		break;

	case WM_CONTEXTMENU:
		CallServiceSync(MS_CLIST_REMOVEEVENT, (WPARAM)PUGetContact(hWnd), 0);
		setLastSeen(PUGetContact(hWnd));
		PUDeletePopup(hWnd);
		break;

	case UM_FREEPLUGINDATA:
		if (options.iShowEvent == 0)
			setLastSeen(PUGetContact(hWnd));
		return TRUE;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK PopupDlgProcNoSet(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED) {
			CallServiceSync(MS_CLIST_REMOVEEVENT, (WPARAM)PUGetContact(hWnd), 0);
			CallServiceSync("BuddyExpectator/actionStillAbsent", (WPARAM)PUGetContact(hWnd), 0);
			PUDeletePopup(hWnd);
		}
		break;

	case WM_CONTEXTMENU:
		CallServiceSync(MS_CLIST_REMOVEEVENT, (WPARAM)PUGetContact(hWnd), 0);
		PUDeletePopup(hWnd);
		break;

	case UM_FREEPLUGINDATA:
		return TRUE;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/**
 * Checks - whether user has been gone for specified number of days
 */
bool isContactGoneFor(MCONTACT hContact, int days)
{
	time_t lastSeen = getLastSeen(hContact);
	time_t lastInputMsg = getLastInputMsg(hContact);
	time_t currentTime = time(NULL);

	int daysSinceOnline = -1;
	if (lastSeen != -1) daysSinceOnline = (int)((currentTime - lastSeen)/(60*60*24));

	int daysSinceMessage = -1;
	if (lastInputMsg != -1) daysSinceMessage = (int)((currentTime - lastInputMsg)/(60*60*24));

	if (options.hideInactive)
	if (daysSinceMessage >= options.iSilencePeriod)
	if (!db_get_b(hContact, "CList", "Hidden", 0) && !db_get_b(hContact, MODULE_NAME, "NeverHide", 0)) {
		POPUPDATAT_V2 ppd = {0};
		ppd.cbSize = sizeof(ppd);
		ppd.lchContact = hContact;
		ppd.lchIcon = Skin_GetIcon("enabled_icon");

		mir_sntprintf(ppd.lptzContactName, SIZEOF(ppd.lptzContactName), TranslateT("Hiding %s (%S)"), 
			CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR), 
			GetContactProto(hContact));

		mir_sntprintf(ppd.lptzText, SIZEOF(ppd.lptzText), TranslateT("%d days since last message"), daysSinceMessage);

		if (!options.iUsePopupColors) {
			ppd.colorBack = options.iPopupColorBack;
			ppd.colorText = options.iPopupColorFore;
		}
		ppd.PluginWindowProc = HidePopupDlgProc;
		ppd.iSeconds = -1;

		hideactions[0].flags = hideactions[1].flags = PAF_ENABLED;
		ppd.lpActions = hideactions;
		ppd.actionCount = 2;

		CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&ppd, APF_NEWDATA);

		SkinPlaySound("buddyExpectatorHide");
	}

	return (daysSinceOnline >= days && (daysSinceMessage == -1 || daysSinceMessage >= days));
}

void ReturnNotify(MCONTACT hContact, TCHAR *message)
{
	if (db_get_b(hContact, "CList", "NotOnList", 0) == 1 || db_get_b(hContact, "CList", "Hidden", 0) == 1)
		return;

	SkinPlaySound("buddyExpectatorReturn");

	if (options.iShowPopup > 0) {
		// Display Popup
		POPUPDATAT ppd = { 0 };
		ppd.lchContact = hContact;
		ppd.lchIcon = hIcon;
		_tcsncpy(ppd.lptzContactName, (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,hContact,GCDNF_TCHAR), MAX_CONTACTNAME);
		_tcsncpy(ppd.lptzText, message, MAX_SECONDLINE);
		if (!options.iUsePopupColors) {
			ppd.colorBack = options.iPopupColorBack;
			ppd.colorText = options.iPopupColorFore;
		}
		ppd.PluginWindowProc = PopupDlgProc;
		ppd.PluginData = NULL;
		ppd.iSeconds = options.iPopupDelay;
		PUAddPopupT(&ppd);
	}

	if (options.iShowEvent > 0) {
		CLISTEVENT cle = { sizeof(cle) };
		cle.hContact = hContact;
		cle.hIcon = hIcon;
		cle.pszService = "BuddyExpectator/actionReturned";
		cle.flags = CLEF_TCHAR;

		TCHAR* nick = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,hContact,GCDNF_TCHAR);
		TCHAR tmpMsg[512];
		mir_sntprintf(tmpMsg, SIZEOF(tmpMsg), _T("%s %s"), nick, message);
		cle.ptszTooltip = tmpMsg;

		CallServiceSync(MS_CLIST_ADDEVENT, 0, (LPARAM) &cle);
	}
}

void GoneNotify(MCONTACT hContact, TCHAR *message)
{
	if (db_get_b(hContact, "CList", "NotOnList", 0) == 1 || db_get_b(hContact, "CList", "Hidden", 0) == 1)
		return;

	if (options.iShowPopup2 > 0) {
		// Display Popup
		POPUPDATAT ppd = {0};
		ppd.lchContact = hContact;
		ppd.lchIcon = hIcon;
		_tcsncpy(ppd.lptzContactName, (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,hContact,GCDNF_TCHAR), MAX_CONTACTNAME);
		_tcsncpy(ppd.lptzText, message, MAX_SECONDLINE);
		if (!options.iUsePopupColors) {
			ppd.colorBack = options.iPopupColorBack;
			ppd.colorText = options.iPopupColorFore;
		}
		ppd.PluginWindowProc = PopupDlgProcNoSet;
		ppd.PluginData = NULL;
		ppd.iSeconds = options.iPopupDelay;

		PUAddPopupT(&ppd);
	}

	if (options.iShowEvent2 > 0) {
		CLISTEVENT cle = { sizeof(cle) };
		cle.hContact = hContact;
		cle.hIcon = hIcon;
		cle.pszService = "BuddyExpectator/actionStillAbsent";

		TCHAR* nick = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,hContact,GCDNF_TCHAR);
		TCHAR tmpMsg[512];
		mir_sntprintf(tmpMsg, SIZEOF(tmpMsg), _T("%s %s"), nick, message);
		cle.ptszTooltip = tmpMsg;
		cle.flags = CLEF_TCHAR;

		CallServiceSync(MS_CLIST_ADDEVENT, 0, (LPARAM) &cle);
	}
}

/**
 * Miss you action (clist event click)
 *  when called from popup, wParam = (HANDLE)hContact and lParam == 0,
 *  when called from clist event, wParam = hWndCList, lParam = &CLISTEVENT
 */
INT_PTR MissYouAction(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact;
	if (lParam) {
		CLISTEVENT* cle = (CLISTEVENT*)lParam;
		hContact = cle->hContact;
	}
	else hContact = wParam;

	CallService(MS_MSG_SENDMESSAGET, hContact, 0);
	return 0;
}

/**
 * Contact returned action (clist event click)
 *  when called from popup, wParam = (HANDLE)hContact and lParam == 0,
 *  when called from clist event, wParam = hWndCList, lParam = &CLISTEVENT
 */
INT_PTR ContactReturnedAction(WPARAM hContact, LPARAM lParam)
{
	if (lParam) {
		CLISTEVENT *cle = (CLISTEVENT*)lParam;
		hContact = cle->hContact;
	}

	if (options.iShowMessageWindow>0)
		CallService(MS_MSG_SENDMESSAGET, hContact, 0);

	if (options.iShowUDetails>0)
		CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);

	setLastSeen(hContact);
	return 0;
}

/**
 * Contact not returned action (clist event click)
 *  when called from popup, wParam = (HANDLE)hContact and lParam == 0,
 *  when called from clist event, wParam = hWndCList, lParam = &CLISTEVENT
 */
INT_PTR ContactStillAbsentAction(WPARAM hContact, LPARAM lParam)
{
	if (lParam) {
		CLISTEVENT* cle = (CLISTEVENT*)lParam;
		hContact = cle->hContact;
	}

	switch (options.action2) {
	case GCA_DELETE:
		CallService(MS_DB_CONTACT_DELETE, hContact, 0);
		break;
	case GCA_UDETAILS:
		CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);
		break;
	case GCA_MESSAGE:
		CallService(MS_MSG_SENDMESSAGE, hContact, 0);
		break;
	case GCA_NOACTION:
		break;
	}

	return 0;
}

/**
 * Load icons either from icolib or built in
 */
int onIconsChanged(WPARAM, LPARAM)
{
	hIcon = Skin_GetIcon("main_icon");
	return 0;
}

/**
 * Menu item click action
 */
INT_PTR MenuMissYouClick(WPARAM hContact, LPARAM)
{
	if (db_get_b(hContact, MODULE_NAME, "MissYou", 0)) {
		db_set_b(hContact, MODULE_NAME, "MissYou", 0);
		ExtraIcon_Clear(hExtraIcon, hContact);
	}
	else {
		db_set_b(hContact, MODULE_NAME, "MissYou", 1);
		ExtraIcon_SetIcon(hExtraIcon, hContact, "enabled_icon");
	}

	return 0;
}

/**
 * Menu is about to appear
 */
int onPrebuildContactMenu(WPARAM hContact, LPARAM)
{
   char *proto = GetContactProto(hContact);
   if (!proto)
		return 0;

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_ICON | CMIM_NAME | CMIF_TCHAR;
   if (db_get_b(hContact, MODULE_NAME, "MissYou", 0)) {
		mi.ptszName = LPGENT("Disable Miss You");
		mi.icolibItem = iconList[1].hIcolib;
   }
   else {
		mi.ptszName = LPGENT("Enable Miss You");
		mi.icolibItem = iconList[2].hIcolib;
   }
	Menu_ModifyItem(hContactMenu, &mi);
	Menu_ShowItem(hContactMenu, !db_get_b(hContact, proto, "ChatRoom", 0) && (CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND));
   return 0;
}

int onExtraImageApplying(WPARAM hContact, LPARAM)
{
	if ( db_get_b(hContact, MODULE_NAME, "MissYou", 0))
		ExtraIcon_SetIcon(hExtraIcon, hContact, "enabled_icon");

   return 0;
}

/**
 * ContactSettingChanged callback
 */
int SettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *inf = (DBCONTACTWRITESETTING*)lParam;
	if (hContact == NULL || inf->value.type == DBVT_DELETED || strcmp(inf->szSetting, "Status") != 0)
		return 0;

	if (db_get_b(hContact, "CList", "NotOnList", 0) == 1)
		return 0;

	char *proto = GetContactProto(hContact);
	if (proto == 0 || (db_get_b(hContact, proto, "ChatRoom", 0) == 1)
		|| !(CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND))
		return 0;

	int currentStatus = inf->value.wVal;
	int prevStatus = db_get_w(hContact, "UserOnline", "OldStatus", ID_STATUS_OFFLINE);

	if (currentStatus == prevStatus)
		return 0;

	// Last status
	db_set_dw(hContact, MODULE_NAME, "LastStatus", prevStatus);

	if (prevStatus == ID_STATUS_OFFLINE) {
		if (db_get_b(hContact, MODULE_NAME, "MissYou", 0)) {
			// Display Popup
			POPUPDATAT_V2 ppd = {0};
			ppd.cbSize = sizeof(ppd);

			ppd.lchContact = hContact;
			ppd.lchIcon = Skin_GetIcon("enabled_icon");
			_tcsncpy(ppd.lptzContactName, (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,hContact,GCDNF_TCHAR), MAX_CONTACTNAME);
			_tcsncpy(ppd.lptzText, TranslateT("You awaited this contact!"), MAX_SECONDLINE);
			if (!options.iUsePopupColors) {
				ppd.colorBack = options.iPopupColorBack;
				ppd.colorText = options.iPopupColorFore;
			}
			ppd.PluginWindowProc = MissYouPopupDlgProc;
			ppd.PluginData = NULL;
			ppd.iSeconds = -1;

			missyouactions[0].flags = PAF_ENABLED;
			ppd.lpActions = missyouactions;
			ppd.actionCount = 1;

			CallService(MS_POPUP_ADDPOPUPT, (WPARAM) &ppd, APF_NEWDATA);

			SkinPlaySound("buddyExpectatorMissYou");
		}
	}

	if (currentStatus == ID_STATUS_OFFLINE) {
		setLastSeen(hContact);
		return 0;
	}

	if (db_get_dw(hContact, MODULE_NAME, "LastSeen", (DWORD)-1) == (DWORD)-1 && options.notifyFirstOnline) {
		ReturnNotify(hContact, TranslateT("has gone online for the first time."));
		setLastSeen(hContact);
	}

	unsigned int AbsencePeriod = db_get_dw(hContact, MODULE_NAME, "iAbsencePeriod", options.iAbsencePeriod);
	if (isContactGoneFor(hContact, AbsencePeriod)) {
		TCHAR* message = TranslateT("has returned after a long absence.");
		TCHAR tmpBuf[251] = {0};
		time_t tmpTime = getLastSeen(hContact);
		if (tmpTime != -1) {
			_tcsftime(tmpBuf, 250, TranslateT("has returned after being absent since %#x"), gmtime(&tmpTime));
			message = tmpBuf;
		}
		else {
			tmpTime = getLastInputMsg(hContact);
			if (tmpTime != -1) {
				_tcsftime(tmpBuf, 250, TranslateT("has returned after being absent since %#x"), gmtime(&tmpTime));
				message = tmpBuf;
			}
		}

		ReturnNotify(hContact, message);

		if ((options.iShowMessageWindow == 0 && options.iShowUDetails == 0) || (options.iShowEvent == 0 && options.iShowPopup == 0))
			setLastSeen(hContact);
	}
	else setLastSeen(hContact);

	return 0;
}

void CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		char *proto = GetContactProto(hContact);
		if (proto && (db_get_b(hContact, proto, "ChatRoom", 0) == 0) && (CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND) && isContactGoneFor(hContact, options.iAbsencePeriod2) && (db_get_b(hContact, MODULE_NAME, "StillAbsentNotified", 0) == 0))
		{
			db_set_b(hContact, MODULE_NAME, "StillAbsentNotified", 1);
			SkinPlaySound("buddyExpectatorStillAbsent");

			TCHAR* message = TranslateT("has not returned after a long absence.");
			time_t tmpTime;
			TCHAR tmpBuf[251] = {0};
			tmpTime = getLastSeen(hContact);
			if (tmpTime != -1)
			{
				_tcsftime(tmpBuf, 250, TranslateT("has not returned after being absent since %#x"), gmtime(&tmpTime));
				message = tmpBuf;
			}
			else
			{
				tmpTime = getLastInputMsg(hContact);
				if (tmpTime != -1)
				{
					_tcsftime(tmpBuf, 250, TranslateT("has not returned after being absent since %#x"), gmtime(&tmpTime));
					message = tmpBuf;
				}
			}

			GoneNotify(hContact, message);
		}
	}
}
/**
 * Called when all the modules have had their modules loaded event handlers called (dependence of popups on fontservice :( )
 */
int ModulesLoaded2(WPARAM, LPARAM)
{
	// check for 'still absent' contacts  on startup
	TimerProc(0, 0, 0, 0);
	return 0;
}

/**
 * Called when all the modules are loaded
 */

int ModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_USERINFO_INITIALISE, UserinfoInit);

	// add sounds support
	SkinAddNewSoundExT("buddyExpectatorReturn", LPGENT("BuddyExpectator"), LPGENT("Contact returned"));
	SkinAddNewSoundExT("buddyExpectatorStillAbsent", LPGENT("BuddyExpectator"), LPGENT("Contact still absent"));
	SkinAddNewSoundExT("buddyExpectatorMissYou", LPGENT("BuddyExpectator"), LPGENT("Miss you event"));
	SkinAddNewSoundExT("buddyExpectatorHide", LPGENT("BuddyExpectator"), LPGENT("Hide contact event"));

	timer_id = SetTimer(0, 0, 1000 * 60 * 60 * 4, TimerProc); // check every 4 hours

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded2);

	////////////////////////////////////////////////////////////////////////////

	if (options.enableMissYou) {
		HookEvent(ME_CLIST_PREBUILDCONTACTMENU, onPrebuildContactMenu);

		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags = CMIF_TCHAR;
		mi.icolibItem = iconList[2].hIcolib;
		mi.position = 200000;
		mi.ptszName = LPGENT("Enable Miss You");
		mi.pszService = "BuddyExpectator/actionMissYouClick";
		hContactMenu = Menu_AddContactMenuItem(&mi);
	}

	missyouactions[0].cbSize = sizeof(POPUPACTION);
	missyouactions[0].lchIcon = Skin_GetIcon("disabled_icon");
	mir_strcpy(missyouactions[0].lpzTitle, LPGEN("Disable Miss You"));
	missyouactions[0].wParam = missyouactions[0].lParam = 1;

	hideactions[0].cbSize = sizeof(POPUPACTION);
	hideactions[0].lchIcon = Skin_GetIcon("hide_icon");
	mir_strcpy(hideactions[0].lpzTitle, LPGEN("Hide contact"));
	hideactions[0].wParam = hideactions[0].lParam = 2;

	hideactions[1].cbSize = sizeof(POPUPACTION);
	hideactions[1].lchIcon = Skin_GetIcon("neverhide_icon");
	mir_strcpy(hideactions[1].lpzTitle, LPGEN("Never hide this contact"));
	hideactions[1].wParam = hideactions[1].lParam = 3;

	return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

int ContactAdded(WPARAM hContact, LPARAM)
{
	db_set_dw(hContact, MODULE_NAME, "CreationTime", (DWORD)time(0));
	return 0;
}

int onShutdown(WPARAM, LPARAM)
{
	DestroyServiceFunction(hContactReturnedAction);
	DestroyServiceFunction(hContactStillAbsentAction);
	DestroyServiceFunction(hMissYouAction);
	DestroyServiceFunction(hMenuMissYouClick);

	Skin_ReleaseIcon(hIcon);
	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	setlocale(LC_ALL, "English"); // Set English locale

	InitOptions();

	hContactReturnedAction =  CreateServiceFunction("BuddyExpectator/actionReturned", ContactReturnedAction);
	hContactStillAbsentAction = CreateServiceFunction("BuddyExpectator/actionStillAbsent", ContactStillAbsentAction);
	hMissYouAction = CreateServiceFunction("BuddyExpectator/actionMissYou", MissYouAction);
	hMenuMissYouClick = CreateServiceFunction("BuddyExpectator/actionMissYouClick", MenuMissYouClick);

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingChanged);
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, onShutdown);

	HookEvent(ME_DB_CONTACT_ADDED, ContactAdded);

	// ensure all contacts are timestamped
	DBVARIANT dbv;
	DWORD current_time = (DWORD)time(0);

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		if ( !db_get(hContact, MODULE_NAME, "CreationTime", &dbv))
			db_free(&dbv);
		else
			db_set_dw(hContact, MODULE_NAME, "CreationTime", current_time);
	}

	Icon_Register(hInst, "BuddyExpectator", iconList, SIZEOF(iconList));

	HookEvent(ME_SKIN2_ICONSCHANGED, onIconsChanged);

	onIconsChanged(0, 0);

	hExtraIcon = ExtraIcon_Register("buddy_exp", LPGEN("Buddy Expectator"), "enabled_icon");

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	KillTimer(0, timer_id);

	return 0;
}
