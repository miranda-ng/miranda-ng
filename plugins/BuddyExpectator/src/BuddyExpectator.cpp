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

	File name      : $URL: http://svn.miranda.im/mainrepo/buddyexpectator/trunk/BuddyExpectator.cpp $
	Revision       : $Rev: 1392 $
	Last change on : $Date: 2009-04-22 17:46:08 +0300 (Ср, 22 апр 2009) $
	Last change by : $Author: Thief $
*/

#include "common.h"
#include "options.h"

HINSTANCE hInst;
int hLangpack;

DWORD timer_id = 0;

HANDLE   hEventContactSetting = NULL;
HANDLE   hEventContactAdded = NULL;
HANDLE   hEventUserInfoInit = NULL;
HANDLE   hPrebuildContactMenu = NULL;
HANDLE   hContactMenu = NULL;
HANDLE	 hIcoLibIconsChanged = NULL;
HANDLE   hContactReturnedAction = NULL;
HANDLE   hContactStillAbsentAction = NULL;
HANDLE   hMissYouAction = NULL;
HANDLE 	 hMenuMissYouClick = NULL;
HANDLE   hModulesLoaded = NULL;
HANDLE   hModulesLoaded2 = NULL;
HANDLE   hSystemOKToExit = NULL;
HANDLE   hHookExtraIconsRebuild = NULL;
HANDLE   hHookExtraIconsApply = NULL;

HICON hIcon;
HANDLE hEnabledIcon = NULL, hDisabledIcon = NULL;

IconExtraColumn g_IECMissYou = {0};
IconExtraColumn g_IECClear = {0};

// Popup Actions
POPUPACTION missyouactions[1];
POPUPACTION hideactions[2];

extern int UserinfoInit(WPARAM wparam, LPARAM lparam);

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"Buddy Expectator+",
	PLUGIN_MAKE_VERSION(0,1,2,3),
	"Reacts to the return of contacts, or the the extended absence of contacts, with notifications and other actions",
	"Anar Ibragimoff, sje, Thief",
	"thief@miranda.im",
	"© 2005 Anar Ibragimoff, © 2006 Scott Ellis, © 2007-2009 Alexander Turyak",
	"http://nightly.miranda.im/",
	UNICODE_AWARE,
	{0xddf8aec9, 0x7d37, 0x49af, {0x9d, 0x22, 0xbb, 0xbc, 0x92, 0x0e, 0x6f, 0x05}} //{DDF8AEC9-7D37-49AF-9D22-BBBC920E6F05}
};

/* 2D84F403-91F3-4E60-BF02-16C2F1716D86 */
#define MIID_BUDDYEXPECTATOR {0x2d84f403, 0x91f3, 0x4e60, {0xbf, 0x02, 0x16, 0xc2, 0xf1, 0x71, 0x6d, 0x86}}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_BUDDYEXPECTATOR, MIID_LAST};

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

time_t getLastSeen(HANDLE hContact)
{
	return DBGetContactSettingDword(hContact, MODULE_NAME, "LastSeen", DBGetContactSettingDword(hContact, MODULE_NAME, "CreationTime", (DWORD)-1));
}

void setLastSeen(HANDLE hContact)
{
	DBWriteContactSettingDword(hContact, MODULE_NAME, "LastSeen", (DWORD)time(NULL));
	if (DBGetContactSettingByte(hContact, MODULE_NAME, "StillAbsentNotified", 0))
		DBWriteContactSettingByte(hContact, MODULE_NAME, "StillAbsentNotified", 0);
}

time_t getLastInputMsg(HANDLE hContact)
{
	HANDLE hDbEvent;
	DBEVENTINFO dbei = {0};

	hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDLAST, (WPARAM)hContact, 0);
    while (hDbEvent)
	{
        dbei.cbSize = sizeof(dbei);
        dbei.pBlob = 0;
        dbei.cbBlob = 0;
        CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&dbei);
        if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT))
            return dbei.timestamp;
        hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDPREV, (WPARAM)hDbEvent, 0);
    }
	return -1;
}

/**
 * PopUp window procedures
 */

int CALLBACK HidePopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_COMMAND:
			if (HIWORD(wParam) == STN_CLICKED)
			{
				DBWriteContactSettingByte(PUGetContact(hWnd), "CList", "Hidden", 1);
				PUDeletePopUp(hWnd);
			}
			break;
        case WM_CONTEXTMENU:
				DBWriteContactSettingByte(PUGetContact(hWnd), MODULE_NAME, "NeverHide", 1);
				PUDeletePopUp(hWnd);
            break;
		case UM_POPUPACTION:
			if (wParam == 2)
			{
				DBWriteContactSettingByte(PUGetContact(hWnd), "CList", "Hidden", 1);
				PUDeletePopUp(hWnd);
			}
			if (wParam == 3)
			{
				DBWriteContactSettingByte(PUGetContact(hWnd), MODULE_NAME, "NeverHide", 1);
				PUDeletePopUp(hWnd);
			}
			break;
		case UM_FREEPLUGINDATA:
			return TRUE;
		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

int CALLBACK MissYouPopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_COMMAND:
			if (HIWORD(wParam) == STN_CLICKED)
			{
				CallServiceSync("BuddyExpectator/actionMissYou", (WPARAM)PUGetContact(hWnd), 0);
				if (!DBGetContactSettingByte(PUGetContact(hWnd), MODULE_NAME, "MissYouNotifyAlways", 0))
				{
					DBWriteContactSettingByte(PUGetContact(hWnd), MODULE_NAME, "MissYou", 0);
					if (options.MissYouIcon) CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM) PUGetContact(hWnd), (LPARAM) &g_IECClear);
				}
				PUDeletePopUp(hWnd);
			}
			break;
        case WM_CONTEXTMENU:
				PUDeletePopUp(hWnd);
            break;
		case UM_POPUPACTION:
			if (wParam == 1)
			{
				DBWriteContactSettingByte(PUGetContact(hWnd), MODULE_NAME, "MissYou", 0);
				if (options.MissYouIcon) CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM) PUGetContact(hWnd), (LPARAM) &g_IECClear);
				PUDeletePopUp(hWnd);
			}
			break;
		case UM_FREEPLUGINDATA:
			return TRUE;
		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

int CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_COMMAND:
			if (HIWORD(wParam) == STN_CLICKED)
			{
                CallServiceSync(MS_CLIST_REMOVEEVENT, (WPARAM)PUGetContact(hWnd), 0);
				CallServiceSync("BuddyExpectator/actionReturned", (WPARAM)PUGetContact(hWnd), 0);
				PUDeletePopUp(hWnd);
			}
			break;
        case WM_CONTEXTMENU:
                CallServiceSync(MS_CLIST_REMOVEEVENT, (WPARAM)PUGetContact(hWnd), 0);
				setLastSeen(PUGetContact(hWnd));
				PUDeletePopUp(hWnd);
            break;
		case UM_FREEPLUGINDATA:
            if (options.iShowEvent == 0)
			{
				setLastSeen(PUGetContact(hWnd));
            }
			return TRUE;
		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

int CALLBACK PopupDlgProcNoSet(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_COMMAND:
			if (HIWORD(wParam) == STN_CLICKED)
			{
                CallServiceSync(MS_CLIST_REMOVEEVENT, (WPARAM)PUGetContact(hWnd), 0);
				CallServiceSync("BuddyExpectator/actionStillAbsent", (WPARAM)PUGetContact(hWnd), 0);
				PUDeletePopUp(hWnd);
			}
			break;
        case WM_CONTEXTMENU:
                CallServiceSync(MS_CLIST_REMOVEEVENT, (WPARAM)PUGetContact(hWnd), 0);
				PUDeletePopUp(hWnd);
            break;
		case UM_FREEPLUGINDATA:
			return TRUE;
		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/**
 * Checks - whether user has been gone for specified number of days
 */
bool isContactGoneFor(HANDLE hContact, int days)
{
    time_t lastSeen = getLastSeen(hContact);
    time_t lastInputMsg = getLastInputMsg(hContact);
    time_t currentTime = time(NULL);

    int daysSinceOnline = -1;
    if (lastSeen != -1) daysSinceOnline = (int)((currentTime - lastSeen)/(60*60*24));

    int daysSinceMessage = -1;
	if (lastInputMsg != -1) daysSinceMessage = (int)((currentTime - lastInputMsg)/(60*60*24));
	
	if (options.hideInactive)
	{
		if (daysSinceMessage >= options.iSilencePeriod)
			if (!DBGetContactSettingByte(hContact, "CList", "Hidden", 0) && !DBGetContactSettingByte(hContact, MODULE_NAME, "NeverHide", 0))
			{
				char szInfo[200];
				
				POPUPDATAT_V2 ppd = {0};
				ppd.cbSize = sizeof(POPUPDATAT_V2);
		
				ppd.lchContact = hContact;
				ppd.lchIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"enabled_icon");
				
				mir_snprintf(szInfo, 200, Translate("Hiding %s (%s)"), (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,0), (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0));
				strncpy(ppd.lpzContactName, szInfo, MAX_CONTACTNAME);
				mir_snprintf(szInfo, 200, Translate("%d days since last message"), daysSinceMessage);
				strncpy(ppd.lpzText, szInfo, MAX_SECONDLINE);
				if (!options.iUsePopupColors)
				{
					ppd.colorBack = options.iPopUpColorBack;
					ppd.colorText = options.iPopUpColorFore;
				}
				ppd.PluginWindowProc = (WNDPROC)HidePopupDlgProc;
				ppd.PluginData = NULL;
				ppd.iSeconds = -1;
				
				hideactions[0].flags = hideactions[1].flags = PAF_ENABLED;
				ppd.lpActions = hideactions;
				ppd.actionCount = 2;
				
				CallService(MS_POPUP_ADDPOPUPEX, (WPARAM) &ppd, APF_NEWDATA);

				SkinPlaySound("buddyExpectatorHide");
			}
	}

	return (daysSinceOnline >= days && (daysSinceMessage == -1 || daysSinceMessage >= days));
}

void ReturnNotify(HANDLE hContact, char *message)
{
	if (DBGetContactSettingByte(hContact, "CList", "NotOnList", 0) == 1 || DBGetContactSettingByte(hContact, "CList", "Hidden", 0) == 1)
		return;
	
	SkinPlaySound("buddyExpectatorReturn");

	if (options.iShowPopUp > 0)
	{
		// Display PopUp
		POPUPDATAT_V2 ppd = {0};
		ppd.cbSize = sizeof(POPUPDATAT_V2);

		ppd.lchContact = hContact;
		ppd.lchIcon = hIcon;
		strncpy(ppd.lpzContactName, (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,0), MAX_CONTACTNAME);
		strncpy(ppd.lpzText, message, MAX_SECONDLINE);
		if (!options.iUsePopupColors)
		{
			ppd.colorBack = options.iPopUpColorBack;
			ppd.colorText = options.iPopUpColorFore;
		}
		ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;
		ppd.PluginData = NULL;
		ppd.iSeconds = options.iPopUpDelay;

		CallService(MS_POPUP_ADDPOPUPEX, (WPARAM) &ppd, 0);

	}

	if (options.iShowEvent > 0)
	{
		CLISTEVENT cle = {0};

		cle.cbSize = sizeof(cle);
		cle.hContact = hContact;
		cle.hIcon = hIcon;
		cle.pszService = (char *)"BuddyExpectator/actionReturned";

		char* nick = (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,0);
		char tmpMsg[512];
		mir_snprintf(tmpMsg, 512, "%s %s", nick, message);
		cle.pszTooltip = tmpMsg;

		CallServiceSync(MS_CLIST_ADDEVENT, 0, (LPARAM) &cle);
	}
}

void GoneNotify(HANDLE hContact, char *message)
{
	if (DBGetContactSettingByte(hContact, "CList", "NotOnList", 0) == 1 || DBGetContactSettingByte(hContact, "CList", "Hidden", 0) == 1)
		return;

	if (options.iShowPopUp2 > 0)
	{
		// Display PopUp
		POPUPDATAT_V2 ppd = {0};
		ppd.cbSize = sizeof(POPUPDATAT_V2);

		ppd.lchContact = hContact;
		ppd.lchIcon = hIcon;
		strncpy(ppd.lpzContactName, (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,0), MAX_CONTACTNAME);
		strncpy(ppd.lpzText, message, MAX_SECONDLINE);
		if (!options.iUsePopupColors)
		{
			ppd.colorBack = options.iPopUpColorBack;
			ppd.colorText = options.iPopUpColorFore;
		}
		ppd.PluginWindowProc = (WNDPROC)PopupDlgProcNoSet;
		ppd.PluginData = NULL;
		ppd.iSeconds = options.iPopUpDelay;
	
		CallService(MS_POPUP_ADDPOPUPEX, (WPARAM) &ppd, 0);
	}

	if (options.iShowEvent2 > 0)
	{
		CLISTEVENT cle = {0};

		cle.cbSize = sizeof(cle);
		cle.hContact = hContact;
		cle.hIcon = hIcon;
		cle.pszService = (char *)"BuddyExpectator/actionStillAbsent";

		char* nick = (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,0);
		char tmpMsg[512];
		mir_snprintf(tmpMsg, 512, "%s %s", nick, message);
		cle.pszTooltip = tmpMsg;

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
	HANDLE hContact;
	if (lParam)
	{
	    CLISTEVENT* cle = (CLISTEVENT*)lParam;
		hContact = cle->hContact;
	} else
		hContact = (HANDLE)wParam;

    CallService(MS_MSG_SENDMESSAGE, (WPARAM)hContact, 0);

    return 0;
}

/**
 * Contact returned action (clist event click)
 *  when called from popup, wParam = (HANDLE)hContact and lParam == 0,
 *  when called from clist event, wParam = hWndCList, lParam = &CLISTEVENT
 */
INT_PTR ContactReturnedAction(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact;
	if (lParam)
	{
	    CLISTEVENT* cle = (CLISTEVENT*)lParam;
		hContact = cle->hContact;
	} else
		hContact = (HANDLE)wParam;

    if (options.iShowMessageWindow>0)
	{
        CallService(MS_MSG_SENDMESSAGE, (WPARAM)hContact, 0);
    }

    if (options.iShowUDetails>0)
	{
        CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)hContact, 0);
    }

	setLastSeen(hContact);

    return 0;
}

/**
 * Contact not returned action (clist event click)
 *  when called from popup, wParam = (HANDLE)hContact and lParam == 0,
 *  when called from clist event, wParam = hWndCList, lParam = &CLISTEVENT
 */
INT_PTR ContactStillAbsentAction(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact;
	if (lParam)
	{
	    CLISTEVENT* cle = (CLISTEVENT*)lParam;
		hContact = cle->hContact;
	} else
		hContact = (HANDLE)wParam;

	switch (options.action2)
	{
		case GCA_DELETE:
			CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
		break;
		case GCA_UDETAILS:
			CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)hContact, 0);
		break;
		case GCA_MESSAGE:
			CallService(MS_MSG_SENDMESSAGE, (WPARAM)hContact, 0);
		break;
		case GCA_NOACTION:
		break;
	}

	return 0;
}

/**
 * Load icons either from icolib or built in
 */
int onIconsChanged(WPARAM wParam, LPARAM lParam)
{
	hIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"main_icon");
	return 0;
}

/**
 * Menu item click action
 */
INT_PTR MenuMissYouClick(WPARAM wParam, LPARAM lParam)
{
	if (DBGetContactSettingByte((HANDLE)wParam, MODULE_NAME, "MissYou", 0))
	{
		DBWriteContactSettingByte((HANDLE)wParam, MODULE_NAME, "MissYou", 0);
		if (options.MissYouIcon) CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM) wParam, (LPARAM) &g_IECClear);
	}
	else
	{
		DBWriteContactSettingByte((HANDLE)wParam, MODULE_NAME, "MissYou", 1);
		if (options.MissYouIcon) CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM) wParam, (LPARAM) &g_IECMissYou);
	}

	return 0;
}

/**
 * Menu is about to appear
 */
int onPrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
   char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
   if (!proto) return 0;

   CLISTMENUITEM mi = {0};
   mi.cbSize = sizeof(CLISTMENUITEM);
	
   if (DBGetContactSettingByte((HANDLE)wParam, proto, "ChatRoom", 0) || !(CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND))
		mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
   else
	   mi.flags = CMIM_FLAGS;

   if (DBGetContactSettingByte((HANDLE)wParam, MODULE_NAME, "MissYou", 0))
   {
		mi.flags |= CMIM_ICON | CMIM_NAME | CMIF_ICONFROMICOLIB;
		mi.pszName = Translate("Disable Miss You");
		mi.icolibItem = hEnabledIcon;
   }
   else
   {
		mi.flags |= CMIM_ICON | CMIM_NAME | CMIF_ICONFROMICOLIB;
		mi.pszName = Translate("Enable Miss You");
		mi.icolibItem = hDisabledIcon;
   }
   
   CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hContactMenu, (LPARAM)&mi);

   return 0;
}

int onExtraImageApplying(WPARAM wParam, LPARAM lParam)
{
	if (DBGetContactSettingByte((HANDLE)wParam,MODULE_NAME,"MissYou",0))
		CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM) wParam, (LPARAM) &g_IECMissYou);
   
   return 0;
}

int onExtraImageListRebuild(WPARAM wParam, LPARAM lParam) 
{
	g_IECMissYou.cbSize = sizeof(IconExtraColumn);
	g_IECMissYou.ColumnType = options.MissYouIcon;

	if (ServiceExists(MS_CLIST_EXTRA_ADD_ICON))
		g_IECMissYou.hImage = (HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM)(HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"enabled_icon"), (LPARAM)0);

	return 0;
}

/**
 * ContactSettingChanged callback
 */
int SettingChanged(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE) wParam;
	DBCONTACTWRITESETTING *inf = (DBCONTACTWRITESETTING *) lParam;

	if (hContact == NULL || inf->value.type == DBVT_DELETED || strcmp(inf->szSetting, "Status") != 0) 
		return 0;

	if (DBGetContactSettingByte(hContact, "CList", "NotOnList", 0) == 1)
        return 0;

	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if (proto == 0 || (DBGetContactSettingByte(hContact, proto, "ChatRoom", 0) == 1)
	   || !(CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND))
		return 0;

	int currentStatus = inf->value.wVal;
	int prevStatus = DBGetContactSettingWord(hContact, "UserOnline", "OldStatus", ID_STATUS_OFFLINE);
	
	if (currentStatus == prevStatus)
	{
        return 0;
	}
	
	// Last status
	DBWriteContactSettingDword(hContact, MODULE_NAME, "LastStatus", prevStatus);
	
	if (prevStatus == ID_STATUS_OFFLINE)
	{
		if (DBGetContactSettingByte(hContact, MODULE_NAME, "MissYou", 0))
		{
			// Display PopUp
			POPUPDATAT_V2 ppd = {0};
			ppd.cbSize = sizeof(POPUPDATAT_V2);
			
			ppd.lchContact = hContact;
			ppd.lchIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"enabled_icon");
			strncpy(ppd.lpzContactName, (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,0), MAX_CONTACTNAME);
			strncpy(ppd.lpzText, Translate("You awaited this contact!"), MAX_SECONDLINE);
			if (!options.iUsePopupColors)
			{
				ppd.colorBack = options.iPopUpColorBack;
				ppd.colorText = options.iPopUpColorFore;
			}
			ppd.PluginWindowProc = (WNDPROC)MissYouPopupDlgProc;
			ppd.PluginData = NULL;
			ppd.iSeconds = -1;
			
			missyouactions[0].flags = PAF_ENABLED;
			ppd.lpActions = missyouactions;
			ppd.actionCount = 1;

			CallService(MS_POPUP_ADDPOPUPEX, (WPARAM) &ppd, APF_NEWDATA);
			
			SkinPlaySound("buddyExpectatorMissYou");			
		}
	}

    if (currentStatus == ID_STATUS_OFFLINE)
	{
		setLastSeen(hContact);
        return 0;
    }
	
	/*
	if (CallService(MS_IGNORE_ISIGNORED, (WPARAM) hContact, (LPARAM) IGNOREEVENT_USERONLINE))
	{
		setLastSeen(hContact);
        return 0;
	}
	*/

	if (DBGetContactSettingDword(hContact, MODULE_NAME, "LastSeen", (DWORD)-1) == (DWORD)-1 && options.notifyFirstOnline)
	{
		ReturnNotify(hContact, Translate("has gone online for the first time."));

		setLastSeen(hContact);
	}
	
	unsigned int AbsencePeriod = DBGetContactSettingDword(hContact, MODULE_NAME, "iAbsencePeriod", options.iAbsencePeriod);
    if (isContactGoneFor(hContact, AbsencePeriod))
	{
        char* message = Translate("has returned after a long absence.");
        time_t tmpTime;
		char tmpBuf[251] = {0};
        tmpTime = getLastSeen(hContact);
        if (tmpTime != -1)
		{
            strftime(tmpBuf, 250, Translate("has returned after being absent since %#x"), gmtime(&tmpTime));
            message = tmpBuf;
        }
		else
		{
            tmpTime = getLastInputMsg(hContact);
            if (tmpTime != -1)
			{
                strftime(tmpBuf, 250, Translate("has returned after being absent since %#x"), gmtime(&tmpTime));
                message = tmpBuf;
            }
        }
		
		ReturnNotify(hContact, message);

        if ((options.iShowMessageWindow == 0 && options.iShowUDetails == 0) || (options.iShowEvent == 0 && options.iShowPopUp == 0))
		{
			setLastSeen(hContact);
		}
    } else
		setLastSeen(hContact);

	return 0;
}

void CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD)
{	
	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	char *proto;
	while (hContact != 0)
	{
		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		if (proto && (DBGetContactSettingByte(hContact, proto, "ChatRoom", 0) == 0) && (CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND) && isContactGoneFor(hContact, options.iAbsencePeriod2) && (DBGetContactSettingByte(hContact, MODULE_NAME, "StillAbsentNotified", 0) == 0))
		{
			DBWriteContactSettingByte(hContact, MODULE_NAME, "StillAbsentNotified", 1);
			SkinPlaySound("buddyExpectatorStillAbsent");

			char* message = Translate("has not returned after a long absence.");
			time_t tmpTime;
			char tmpBuf[251] = {0};
			tmpTime = getLastSeen(hContact);
			if (tmpTime != -1)
			{
				strftime(tmpBuf, 250, Translate("has not returned after being absent since %#x"), gmtime(&tmpTime));
				message = tmpBuf;
			}
			else
			{
				tmpTime = getLastInputMsg(hContact);
				if (tmpTime != -1)
				{
					strftime(tmpBuf, 250, Translate("has not returned after being absent since %#x"), gmtime(&tmpTime));
					message = tmpBuf;
				}
			}

			GoneNotify(hContact, message);
		}

		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
}
/**
 * Called when all the modules have had their modules loaded event handlers called (dependence of popups on fontservice :( )
 */
int ModulesLoaded2(WPARAM wParam, LPARAM lParam)
{
	// check for 'still absent' contacts  on startup
	TimerProc(0, 0, 0, 0);

	return 0;
}

/**
 * Called when all the modules are loaded
 */
int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	hEventUserInfoInit = HookEvent(ME_USERINFO_INITIALISE, UserinfoInit);
	
	// add sounds support
	SkinAddNewSoundEx("buddyExpectatorReturn", LPGEN("BuddyExpectator"), LPGEN("Contact returned"));
	SkinAddNewSoundEx("buddyExpectatorStillAbsent", LPGEN("BuddyExpectator"), LPGEN("Contact still absent"));
	SkinAddNewSoundEx("buddyExpectatorMissYou", LPGEN("BuddyExpectator"), LPGEN("Miss you event"));
	SkinAddNewSoundEx("buddyExpectatorHide", LPGEN("BuddyExpectator"), LPGEN("Hide contact event"));
	
	timer_id = SetTimer(0, 0, 1000 * 60 * 60 * 4, TimerProc); // check every 4 hours
	
	hModulesLoaded2 = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded2);
	if (options.MissYouIcon)
	{
		hHookExtraIconsRebuild = HookEvent(ME_CLIST_EXTRA_LIST_REBUILD, onExtraImageListRebuild);
		hHookExtraIconsApply = HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, onExtraImageApplying);
		g_IECClear.cbSize = sizeof(IconExtraColumn);
	    g_IECClear.ColumnType = options.MissYouIcon;
	    g_IECClear.hImage = (HANDLE) -1;
	}
	
	TCHAR szFile[MAX_PATH];
	GetModuleFileNameA(hInst, szFile, MAX_PATH);

	// IcoLib support
	SKINICONDESC sid = {0};
	sid.pszDefaultFile = szFile;
	sid.cbSize = sizeof(sid);
	sid.pszSection = "BuddyExpectator";
		
	sid.pszDescription = "Tray/popup icon";
	sid.pszName = "main_icon";
	sid.iDefaultIndex = -IDI_MAINICON;
	Skin_AddIcon(&sid);
		
	sid.pszDescription = "Enabled";
	sid.pszName = "enabled_icon";
	sid.iDefaultIndex = -IDI_ENABLED;
	hEnabledIcon = Skin_AddIcon(&sid);
		
	sid.pszDescription = "Disabled";
	sid.pszName = "disabled_icon";
	sid.iDefaultIndex = -IDI_DISABLED;
	hDisabledIcon = Skin_AddIcon(&sid);
		
	sid.pszDescription = "Hide";
	sid.pszName = "hide_icon";
	sid.iDefaultIndex = -IDI_HIDE;
	Skin_AddIcon(&sid);
		
	sid.pszDescription = "NeverHide";
	sid.pszName = "neverhide_icon";
	sid.iDefaultIndex = -IDI_NEVERHIDE;
	Skin_AddIcon(&sid);
		
	hIcoLibIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, onIconsChanged);

	onIconsChanged(0,0);

	if (options.enableMissYou) {
		hPrebuildContactMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, onPrebuildContactMenu);

		CLISTMENUITEM mi = {0};
		mi.cbSize = sizeof(CLISTMENUITEM);
		mi.flags = CMIF_ICONFROMICOLIB;
		mi.icolibItem = hDisabledIcon;
		mi.position = 200000;
		mi.pszName = Translate("Enable Miss You");
		mi.pszService = "BuddyExpectator/actionMissYouClick";
		hContactMenu = Menu_AddContactMenuItem(&mi);
	}

	missyouactions[0].cbSize = sizeof(POPUPACTION);
	missyouactions[0].lchIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"disabled_icon");
	lstrcpy(missyouactions[0].lpzTitle, Translate("Disable Miss You"));
	missyouactions[0].wParam = missyouactions[0].lParam = 1;

	hideactions[0].cbSize = sizeof(POPUPACTION);
	hideactions[0].lchIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"hide_icon");
	lstrcpy(hideactions[0].lpzTitle, Translate("Hide contact"));
	hideactions[0].wParam = hideactions[0].lParam = 2;

	hideactions[1].cbSize = sizeof(POPUPACTION);
	hideactions[1].lchIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"neverhide_icon");
	lstrcpy(hideactions[1].lpzTitle, Translate("Never hide this contact"));
	hideactions[1].wParam = hideactions[1].lParam = 3;

	return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

int ContactAdded(WPARAM wParam, LPARAM lParam)
{
	DBWriteContactSettingDword((HANDLE)wParam, MODULE_NAME, "CreationTime", (DWORD)time(0));
	return 0;
}

int onSystemOKToExit(WPARAM wParam,LPARAM lParam)
{
	UnhookEvent(hEventContactSetting);
	UnhookEvent(hEventContactAdded);
	UnhookEvent(hEventUserInfoInit);
	if (hPrebuildContactMenu) UnhookEvent(hPrebuildContactMenu);
	UnhookEvent(hIcoLibIconsChanged);
	UnhookEvent(hModulesLoaded);
	UnhookEvent(hModulesLoaded2);
	UnhookEvent(hSystemOKToExit);
	UnhookEvent(hHookExtraIconsRebuild);
	UnhookEvent(hHookExtraIconsApply);

	DestroyServiceFunction(hContactReturnedAction);
	DestroyServiceFunction(hContactStillAbsentAction);
	DestroyServiceFunction(hMissYouAction);
	DestroyServiceFunction(hMenuMissYouClick);

	DeinitOptions();

	if (hIcoLibIconsChanged)
		CallService(MS_SKIN2_RELEASEICON, (WPARAM)hIcon, 0);
	else
		DestroyIcon(hIcon);

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

	hEventContactSetting = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingChanged);
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hSystemOKToExit = HookEvent(ME_SYSTEM_OKTOEXIT,onSystemOKToExit);

	hEventContactAdded = HookEvent(ME_DB_CONTACT_ADDED, ContactAdded);

	// ensure all contacts are timestamped
	DBVARIANT dbv;
	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	DWORD current_time = (DWORD)time(0);
	while (hContact != 0) {
		if ( !DBGetContactSetting(hContact, MODULE_NAME, "CreationTime", &dbv))
			DBFreeVariant(&dbv);
		else
			DBWriteContactSettingDword(hContact, MODULE_NAME, "CreationTime", current_time);

		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	KillTimer(0, timer_id);

	return 0;
}
