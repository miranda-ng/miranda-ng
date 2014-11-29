/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

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
#ifndef _SVCREMINDER_H_
#define _SVCREMINDER_H_

#define POPUP_TYPE_BIRTHDAY		1
#define POPUP_TYPE_ANNIVERSARY	2

// for PopupDelayType
#define POPUP_DELAY_DEFAULT		3
#define POPUP_DELAY_CUSTOM		4
#define POPUP_DELAY_PERMANENT	5

// for PopupColorType
#define POPUP_COLOR_DEFAULT		6
#define POPUP_COLOR_WINDOWS		7
#define POPUP_COLOR_CUSTOM		8

#define SOUND_BIRTHDAY_TODAY				"Birthday"
#define SOUND_BIRTHDAY_SOON					"BirthdayComing"
#define SOUND_ANNIVERSARY					"Anniversary"

// databbase settings
#define SET_REMIND_LASTCHECK				"RemindLastCheck"
#define SET_REMIND_ENABLED					"RemindEnabled"
#define SET_REMIND_OFFSET					"RemindOffset"
#define SET_REMIND_CHECKVISIBLE				"RemindCheckVisible"
#define SET_REMIND_NOTIFYINTERVAL			"RemindNotifyInterval"
#define SET_REMIND_FLASHICON				"RemindFlashIcon"
#define SET_REMIND_EXTRAICON				"RemindExtraIcon"
#define SET_REMIND_BIRTHMODULE				"RemindBirthModule"
#define SET_REMIND_MENUENABLED				"RemindMenuEnabled"
#define SET_REMIND_BIRTHDAY_ENABLED			"RemindBirthday"
#define SET_REMIND_BIRTHDAY_OFFSET			"RemindBirthdayOffset"
#define SET_REMIND_CHECKON_STARTUP			"RemindStartupCheck"
#define SET_REMIND_SECUREBIRTHDAY			"RemindSecureBirthday"
#define SET_REMIND_BIRTHDAY_IGNORED			"RemindSecureIgnored"
#define SET_REMIND_SOUNDOFFSET				"RemindSoundOffset"
#define SET_POPUP_ENABLED					"PopupEnabled"
#define SET_POPUP_BIRTHDAY_COLORTYPE		"PopupBirthClrType"
#define SET_POPUP_BIRTHDAY_COLOR_TEXT		"PopupBirthClrBirthText"
#define SET_POPUP_BIRTHDAY_COLOR_BACK		"PopupBirthClrBirthBack"
#define SET_POPUP_ANNIVERSARY_COLORTYPE		"PopupAnnivClrType"
#define SET_POPUP_ANNIVERSARY_COLOR_TEXT	"PopupAnnivClrText"
#define SET_POPUP_ANNIVERSARY_COLOR_BACK	"PopupAnnivClrBack"
#define SET_POPUP_DELAY						"PopupDelay"

// default values
#define DEFVAL_REMIND_ENABLED				REMIND_ALL
#define DEFVAL_REMIND_MENUENABLED			1
#define DEFVAL_REMIND_OFFSET				9
#define DEFVAL_REMIND_SOUNDOFFSET			3
#define DEFVAL_REMIND_NOTIFYINTERVAL		12
#define DEFVAL_REMIND_BIRTHMODULE			1
#define DEFVAL_POPUP_ENABLED				1
#define DEFVAL_REMIND_CHECKVISIBLE			0
#define HM_OPENMSG							(WM_USER+1)

/**
 * typedefs:
 **/
enum EEnabled 
{
	REMIND_OFF,
	REMIND_BIRTH,
	REMIND_ANNIV,
	REMIND_ALL,
};

enum ENotify 
{
	NOTIFY_CLIST,		// notify with clist extra icon only
	NOTIFY_POPUP,		// notify with popup and clist extra icon
	NOTIFY_NOANNIV		// notify of no anniversary was found
};

/**
 * Global functions:
 **/
void   SvcReminderCheckAll(const ENotify notify);
LPCSTR SvcReminderGetMyBirthdayModule(void);

void   SvcReminderOnTopToolBarLoaded(void);
void   SvcReminderOnModulesLoaded(void);

void   SvcReminderEnable(BYTE bEnable);
void   SvcReminderLoadModule(void);
void   SvcReminderUnloadModule(void);

#endif /* _SVCREMINDER_H_ */
