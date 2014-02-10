/*
	Contact Settings plugin for Miranda IM
	Copyright (c) 2007-2008 Chervov Dmitry

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

#ifndef __M_CONTACTSETTINGS_H
#define __M_CONTACTSETTINGS_H


typedef struct {
	int cbSize; // sizeof(CONTACTSETTINGSINIT)
	int Type; // one of CSIT_ constants; value of this field depends on what settings dialog was opened: contact settings or group settings
	union
	{
		MCONTACT hContact; // used when Type == CSIT_CONTACT
		int groupID; // used when Type == CSIT_GROUP
	};
} CONTACTSETTINGSINIT;

// CONTACTSETTINGSINIT::Type
#define CSIT_CONTACT 1 // CONTACTSETTINGSINIT::hContact is valid
#define CSIT_GROUP 2 // CONTACTSETTINGSINIT::groupID is valid; in general, if Type == CSIT_GROUP, you MUST call MS_CONTACTSETTINGS_ADDCONTROL and add _all_ your per-contact controls, independently of current group contents, because potentially the group can contain any contacts in future.

/* ME_CONTACTSETTINGS_INITIALISE
The user opened a contact settings dialog. Modules should do whatever initialisation they need and call MS_CONTACTSETTINGS_ADDCONTROL one or more times if they want controls displayed in the dialog
wParam=(WPARAM)(CONTACTSETTINGSINIT*)csi
lParam=0
*/
#define ME_CONTACTSETTINGS_INITIALISE   "ContactSettings/Init"


/* MS_CONTACTSETTINGS_ADDCONTROL
Must only be called during a ME_CONTACTSETTINGS_INITIALISE hook
Adds a control to the contact settings dialog
wParam=(WPARAM)(CONTACTSETTINGSINIT*)csi
lParam=(LPARAM)(CONTACTSETTINGSCONTROL*)csc
Returns 0 on success, nonzero on failure
csi must have come straight from the wParam of ME_CONTACTSETTINGS_INITIALISE
Strings in the structure can be released as soon as the service returns.
*/

// CONTACTSETTINGSCONTROL::Position
#define CSPOS_SORTBYALPHABET 0x40000000 // recommended value for Position if you don't need a specific sorting order
// Group = "Notifications":
#define CSPOS_NOTIFICATIONS_SERVICES 0x20000000 // recommended control position for notification SERVICE PROVIDERS (popups, osd, tickers, etc). Notification plugins that use these services should use CSPOS_SORTBYALPHABET, so that notification service provider settings are above any other specific notification settings.

// CONTACTSETTINGSCONTROL::Flags
#define CSCF_UNICODE 1 // string fields in CONTACTSETTINGSCONTROL and CSCONTROLSTATE are WCHAR*
#define CSCF_DONT_TRANSLATE_STRINGS 2 // specifies that strings in CONTACTSETTINGSCONTROL and CSCONTROLSTATE are translated already

#ifdef _UNICODE
#define CSCF_TCHAR CSCF_UNICODE
#else
#define CSCF_TCHAR 0
#endif

// CONTACTSETTINGSCONTROL::ControlType
typedef enum
{
	CSCT_LABEL = -1, // CSCT_LABEL is used internally in ContactSettings, and mustn't be used by other plugins
	CSCT_CHECKBOX, // checkbox control
	CSCT_COMBOBOX // combobox control with a title above it
} CSCONTROLTYPE;

// some common values for CONTACTSETTINGSCONTROL::ptszGroup
#define CSGROUP_NOTIFICATIONS LPGENT("Notifications")

// special values for CONTACTSETTINGSCONTROL::szModule
#define CSMODULE_PROTO "%proto%" // ContactSettings will replace this by contact's protocol module name

struct CSCONTROLSTATE
{
#ifdef __cplusplus
	CSCONTROLSTATE(): ptszName(NULL) {dbvValue.type = 0; dbvValue.dVal = 0; dbvValue.pbVal = 0; }
	CSCONTROLSTATE(char *pszName, BYTE Value) { this->pszName = pszName; dbvValue.type = DBVT_BYTE; dbvValue.bVal = Value; }
	CSCONTROLSTATE(char *pszName, char Value) { this->pszName = pszName; dbvValue.type = DBVT_BYTE; dbvValue.cVal = Value; }
	CSCONTROLSTATE(char *pszName, WORD Value) { this->pszName = pszName; dbvValue.type = DBVT_WORD; dbvValue.wVal = Value; }
	CSCONTROLSTATE(char *pszName, short Value) { this->pszName = pszName; dbvValue.type = DBVT_WORD; dbvValue.sVal = Value; }
	CSCONTROLSTATE(char *pszName, DWORD Value) { this->pszName = pszName; dbvValue.type = DBVT_DWORD; dbvValue.dVal = Value; }
	CSCONTROLSTATE(char *pszName, long Value) { this->pszName = pszName; dbvValue.type = DBVT_DWORD; dbvValue.lVal = Value; }
	CSCONTROLSTATE(char *pszName, const char *szValue) { this->pszName = pszName; dbvValue.type = DBVT_ASCIIZ; dbvValue.pszVal = (char*)szValue; }
	CSCONTROLSTATE(char *pszName, const WCHAR *wszValue) { this->pszName = pszName; dbvValue.type = DBVT_WCHAR; dbvValue.pwszVal = (WCHAR*)wszValue; }
	CSCONTROLSTATE(WCHAR *pwszName, BYTE Value) { this->pwszName = pwszName; dbvValue.type = DBVT_BYTE; dbvValue.bVal = Value; }
	CSCONTROLSTATE(WCHAR *pwszName, char Value) { this->pwszName = pwszName; dbvValue.type = DBVT_BYTE; dbvValue.cVal = Value; }
	CSCONTROLSTATE(WCHAR *pwszName, WORD Value) { this->pwszName = pwszName; dbvValue.type = DBVT_WORD; dbvValue.wVal = Value; }
	CSCONTROLSTATE(WCHAR *pwszName, short Value) { this->pwszName = pwszName; dbvValue.type = DBVT_WORD; dbvValue.sVal = Value; }
	CSCONTROLSTATE(WCHAR *pwszName, DWORD Value) { this->pwszName = pwszName; dbvValue.type = DBVT_DWORD; dbvValue.dVal = Value; }
	CSCONTROLSTATE(WCHAR *pwszName, long Value) { this->pwszName = pwszName; dbvValue.type = DBVT_DWORD; dbvValue.lVal = Value; }
	CSCONTROLSTATE(WCHAR *pwszName, const char *szValue) { this->pwszName = pwszName; dbvValue.type = DBVT_ASCIIZ; dbvValue.pszVal = (char*)szValue; }
	CSCONTROLSTATE(WCHAR *pwszName, const WCHAR *wszValue) { this->pwszName = pwszName; dbvValue.type = DBVT_WCHAR; dbvValue.pwszVal = (WCHAR*)wszValue; }
#endif

	union
	{
		TCHAR *ptszName; // item text for CSCT_COMBOBOX; not used for CSCT_CHECKBOX
		char *pszName;
		WCHAR *pwszName;
	};
	DBVARIANT dbvValue; // database value for this state
};

// WARNING: do not use Translate(TS) for ptszTitle, ptszGroup, ptszTooltip or CSCONTROLSTATE::ptszName as they are translated by ContactSettings. The only exception is when you use CSCF_DONT_TRANSLATE_STRINGS flag
typedef struct {
	int cbSize; // sizeof(CONTACTSETTINGSCONTROL)
	int cbStateSize; // sizeof(CSCONTROLSTATE)
	int Position; // position in the group, lower numbers are topmost. CSPOS_SORTBYALPHABET is recommended if you don't need a specific sorting order
	DWORD Flags; // a combination of CSCF_ constants
	CSCONTROLTYPE ControlType; // one of CSCT_ constants
	union
	{
		TCHAR *ptszTitle; // label above the control
		char *pszTitle;
		WCHAR *pwszTitle;
	};
	union
	{
		TCHAR *ptszGroup; // group title (several controls may be grouped together); may be NULL.
		char *pszGroup;
		WCHAR *pwszGroup;
	};
	union
	{
		TCHAR *ptszTooltip; // tooltip for the control; may be NULL
		char *pszTooltip;
		WCHAR *pwszTooltip;
	};
	const char *szModule; // database module; may contain variables (see above; currently the only existing variable is CSMODULE_PROTO)
	const char *szSetting; // database setting
	int StateNum; // number of possible states; always 2 or 3 for CSCT_CHECKBOX, and can be any number starting from 2 for CSCT_COMBOBOX
	int DefState; // default state index
	CSCONTROLSTATE *pStates; // array of StateNum items, describing all possible control states. Can be NULL for CSCT_COMBOBOX, in this case ContactSettings will use DBVT_BYTE database values, 0 is unchecked, 1 is checked, 2 is indeterminate. Can't be NULL for CSCT_COMBOBOX.
	DWORD ValueMask; // in most cases simply set this to 0. when not 0, it allows to define a bit mask to access separate bits of a db value instead of reading/writing the whole value. is valid only for DBVT_BYTE, DBVT_WORD and DBVT_DWORD values
} CONTACTSETTINGSCONTROL;

#define MS_CONTACTSETTINGS_ADDCONTROL "ContactSettings/AddControl"


typedef struct {
	int cbSize; // sizeof(CONTACTSETTINGSCHANGEINFO)
	HANDLE hContact;
	const char *szModule; // variables in szModule and szSetting are NOT parsed, i.e. ContactSettings copies the values straight from CONTACTSETTINGSCONTROL
	const char *szSetting;
} CONTACTSETTINGSCHANGEINFO;

/* ME_CONTACTSETTINGS_SETTINGCHANGED
Called for every changed setting when the user applied changes in a contact settings dialog
wParam=(WPARAM)(CONTACTSETTINGSCHANGEINFO*)csci
lParam=0
This event will be triggered many times rapidly when a whole bunch of values are set.
Modules which hook this should be aware of this fact and quickly return if they are not interested in the value that has been changed.
*/
#define ME_CONTACTSETTINGS_SETTINGCHANGED  "ContactSettings/SettingChanged"


/* MS_CONTACTSETTINGS_SHOWDIALOG
Shows Contact Settings dialog for a contact
wParam=hContact
lParam=0
*/
#define MS_CONTACTSETTINGS_SHOWDIALOG "ContactSettings/ShowDialog"

/* MS_CONTACTSETTINGS_SHOWGROUPDIALOG. Not implemented yet.
Shows dialog for a group
wParam=groupId (0 is the main group)
lParam=0
*/
#define MS_CONTACTSETTINGS_SHOWGROUPDIALOG "ContactSettings/ShowGroupDialog"


/*
Example of use:

// in ME_SYSTEM_MODULESLOADED handler:
	HookEvent(ME_CONTACTSETTINGS_INITIALISE, ContactSettingsInit);

// ME_CONTACTSETTINGS_INITIALISE handler
static int ContactSettingsInit(WPARAM wParam, LPARAM lParam)
{
	CONTACTSETTINGSINIT *csi = (CONTACTSETTINGSINIT*)wParam;

// check first whether to add controls or not
// we need to get a protocol name for that, if it's a contact settings dialog:
	char *szProto = (csi->Type == CSIT_CONTACT) ? (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)csi->hContact, 0) : NULL;
// in general, you MUST add all your controls if csi->Type == CSIT_GROUP (i.e. user opened a group settings dialog), otherwise your plugin won't support group settings well:
	if ((csi->Type == CSIT_GROUP) ||
// otherwise (for a contact settings dialog), you can add controls depending on some conditions, like protocol caps, etc.
// in this example, we check whether the protocol has a PS_GETCUSTOMSTATUSICON service (i.e. does it support xstatuses or not):
		(csi->Type == CSIT_CONTACT && szProto && ProtoServiceExists(szProto, PS_GETCUSTOMSTATUSICON)))
// your plugin must ignore cases when csi->Type is neither CSIT_GROUP nor CSIT_CONTACT
	{
		CONTACTSETTINGSCONTROL csc = {0};
		csc.cbSize = sizeof(csc);
		csc.Position = CSPOS_SORTBYALPHABET;
		csc.Flags = CSCF_TCHAR;
		csc.ControlType = CSCT_COMBOBOX;
		csc.StateNum = 3;
		csc.ptszTitle = LPGENT("XStatus change notifications:") _T("\0") LPGENT("Ignore") _T("\0") LPGENT("Notify always") _T("\0") LPGENT("Use global settings") _T("\0");
		csc.ptszGroup = CSGROUP_NOTIFICATIONS;
		csc.ptszTooltip = LPGENT("Tooltip text");
		csc.pszDBSetting = "ModuleName/XSNotifications";
		csc.DefValue = 2; // "Use global settings"
		CallService(MS_CONTACTSETTINGS_ADDCONTROL, wParam, (LPARAM)&csc);

	// and CSCT_CHECKBOX example:
		csc.Position = CSPOS_SORTBYALPHABET;
		csc.Flags = CSCF_TCHAR;
		csc.ControlType = CSCT_CHECKBOX;
		csc.StateNum = 3;
		csc.ptszTitle = LPGENT("Other setting");
		csc.ptszGroup = LPGENT("Some group");
		csc.ptszTooltip = LPGENT("Tooltip text");
		csc.pszDBSetting = "ModuleName/OtherSetting";
		csc.DefValue = 2; // BST_INDETERMINATE
		CallService(MS_CONTACTSETTINGS_ADDCONTROL, wParam, (LPARAM)&csc);
	}
	return 0;
}

*/

#endif // __M_CONTACTSETTINGS_H
