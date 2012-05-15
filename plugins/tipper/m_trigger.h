#ifndef __M_TRIGGER_H__
#define __M_TRIGGER_H__

#if !defined(_TCHAR_DEFINED)
#include <tchar.h>
#endif
#include "m_utils.h"

// --------------------------------------------------------------------------
// Triggers
// --------------------------------------------------------------------------

// This section explains how to create your own trigger. A trigger can be seen
// as an event which can result in a set of actions that will be performed.
// Implementing a trigger consists of two parts. First, you register a trigger
// with MS_TRIGGER_REGISTERTRIGGER to allow a user to configure it in the
// options dialog. Second, when the event occurs belonging to your registered
// trigger, you inform the trigger plugin with MS_TRIGGER_REPORTEVENT. You can
// send a 'payload' together with this notification. This payload, called
// 'TriggerData', can consist of a certain contact, protocol, status and/or a
// piece of text.

// --------------------------------------------------------------------------
// Triggers: Register a trigger
// --------------------------------------------------------------------------

#define MS_TRIGGER_REGISTERTRIGGER  "/TriggerPlugin/RegisterTrigger"

// Parameters:
// ------------------------
// wParam = (WPARAM)0
// lParam = (LPARAM)(TRIGGERREGISTER *)&tr
//   Pointer to a structure describing the trigger to add (see below).

// Return Value:
// ------------------------
// Returns 0 on success, nozero otherwise. Registering an already existing
// trigger will replace this previously registered trigger.

typedef struct {
  int cbSize;  // Set to sizeof(TRIGGERREGISTER).
  char *pszName;  // Used as identifier and shown in the options dialog, must 
                  // be unique.
  HINSTANCE hInstance;  // Only needed when options screen is available.
  DLGPROC pfnDlgProc; // Optional, the callback procedure for the options page.
  char *pszTemplate;  // Optional, template for the options page; must be 
                      // WS_CHILD.
  int flags;  // Flags, see below.
  int dFlags;  // Specify the default DF_* flags which your trigger can send
               // (see below).
} TRIGGERREGISTER;

// Flags
#define TRF_NOEXPORT  0x01  // This trigger cannot be exported. Set this flag
                            // in case you stored settings not using the helper
                            // functions at the end of this header. On export,
                            // TriggerPlugin will search for these settings
                            // and export them automatically. Contact-specific
                            // settings are never exported.

// Please specify the dFlags to indicate what kind of data your trigger is
// able to send as TriggerData. Please specify the maximum set, if your trigger
// does not always send a certain data, please specify it anyway.

#define DF_CONTACT  0x01  // The trigger might send a contact handle with the
                          // TriggerData.
#define DF_PROTO    0x02  // The trigger might send a protocol ID with the
                          // TriggerData.
#define DF_STATUS   0x04  // The trigger might send a status code with the
                          // TriggerData.
#define DF_TEXT	    0x08  // The trigger might send a string with the
                          // TriggerData.
#define DF_LPARAM   0x10  // The trigger might send a custom parameter with the
                          // TriggerData.
#define DF_UNICODE  0x20  // The trigger processes WCHAR strings.

#if defined(UNICODE) || defined(_UNICODE)
#define DF_TCHAR   DF_UNICODE  // Strings in structure are TCHAR*.
#else
#define DF_TCHAR   0
#endif

// Dialog Messages
// The following message should be processed by your options dialog procedure,
// if available. You can create an options dialog to give the user the
// possibility to report your event only under certain circumstances. Each
// trigger is assigned a certain ID. This ID can be used to store the settings
// for your trigger. 

// WM_INITDIALOG

// Parameters:
// ------------------------
// lParam = (LPARAM)(DWORD)triggerID
//   The trigger ID for which the options are to be set. This can be a new ID
//   or an ID of a trigger which is being edited. Initialize your options
//   dialog accordingly. There are helper function at the end of this header
//   file to read your settings for a certain trigger ID.

#define TM_ADDTRIGGER  WM_APP+10

// TM_ADDTRIGGER
// 'OK' is pressed and a new trigger will be added. Save your settings using 
// the given trigger ID.

// Parameters:
// ------------------------
// wParam = (WPARAM)(DWORD)triggerID
//   The trigger ID for which the settings are to be stored. There are helper
//   function at the end of this header file to store your settings with a 
//   certain trigger ID.
// lParam = 0

#define TM_DELTRIGGER  WM_APP+11

// TM_DELTRIGGER
// The trigger addociated with the given trigger ID will be removed.

// Parameters:
// ------------------------
// wParam = (WPARAM)(DWORD)triggerID
//   The trigger ID for which the settings are to be removed. There is a
//   helper service at the end of this header file to easily cleanup settings
//   for a certain trigger ID.
// lParam = 0

// --------------------------------------------------------------------------
// Triggers: Report the Event
// --------------------------------------------------------------------------

// When the event occurs, you report it with MS_TRIGGER_REPORTEVENT. If your
// trigger is configurable, so it has an options screen, you might want to
// report your trigger for certain trigger ID's only. Please use the
// MS_TRIGGER_FINDNEXTTRIGGERID to enumerate over the trigger ID's associated
// with your trigger in the correct order as specified by the user. It's up
// to you to found out whether or not the trigger is to be reported for a
// certain ID.

#define MS_TRIGGER_FINDNEXTTRIGGERID  "/TriggerPlugin/FindNextTriggerID"

// Enumerate over the associated trigger ID's for your trigger in the correct
// order. 

// Parameters:
// ------------------------
// wParam = (WPARAM)(DWORD)triggerID
//   0 to retrieve the first trigger ID for your trigger or the previous ID
//   returned by this service to get the next one.
// lParam = 0

// Return Value:
// ------------------------
// Returns the next trigger ID given the parameter or 0 if no more trigger IDs
// are available.

#define MS_TRIGGER_REPORTEVENT  "/TriggerPlugin/ReportEvent"

// Report your event for further processing. This can be a general event for
// which no individual settings exist, or a specific event for a given
// trigger ID.

// Parameters:
// ------------------------
// wParam = 0
// lParam = (LPARAM)(REPORTINFO *)&ri
//   See below.

// Return Value:
// ------------------------
// Returns CRV_TRUE if all conditions specific to this trigger hold and the
// chain was executed. Returns CRV_FALSE if these conditions did not hold and
// the chain were not processed.

// The structure below can be used to send TriggerData with your trigger. This
// can be used by the associated conditions and actions.

typedef struct {
  int cbSize;  // Set to sizeof(TRIGGERDATA)
  int dFlags;  // Indicate which members are valid using the DF_* flags (see
               // above).
  HANDLE hContact;  // Associate a contact handle to this event.
  char *szProto;  // Associate a protocol ID to this event.
  int status;  // Associcate a status code to this event.
  union {
    char *szText;  // Associate a string to this event.
    TCHAR *tszText;
    WCHAR *wszText;
  };
  LPARAM lParam;  // Associate custom data to this trigger.
} TRIGGERDATA;

typedef struct {
  int cbSize;  // Set to sizeof(REPORTINFO).
  DWORD triggerID;  // The trigger ID of the event to trigger or 0 if this does
                    // not apply.
  char *pszName;  // The name of the trigger (this may be NULL if triggerID is
                  // not 0).
  int flags;  // On of the TRG_* flags, see below.
  TRIGGERDATA *td;  // Optional, the associated TriggerData, see above.
} REPORTINFO;

#define TRG_PERFORM  0x01  // Indicates the event for this trigger actually 
                           // occured and needs to be processed accordingly.
#define TRG_CLEANUP  0x02  // Indicates the trigger instructs to remove the
                           // itself and all associated information. This can
                           // be used for "one time triggers". Remove your own
                           // settings by yourself.

// --------------------------------------------------------------------------
// Actions
// --------------------------------------------------------------------------

// An actions might be performed as a reaction to a reported event by a
// trigger. You first register your action so it can be associated to a
// trigger in the options screen. Next, your provided service or function
// will be called when necessary.

#define MS_TRIGGER_REGISTERACTION  "/TriggerPlugin/RegisterAction"

// Parameters:
// ------------------------
// wParam = (WPARAM)0
// lParam = (LPARAM)(ACTIONREGISTER *)&ar
//   Pointer to a structure describing the action to add (see below).

// Return Value:
// ------------------------
// Returns 0 on success, nozero otherwise. Registering an already existing
// action will replace this previously registered action.

typedef int (* ACTIONFUNCTION)(DWORD actionID, REPORTINFO* ri);

typedef struct {
  int cbSize;  // Set to sizeof(ACTIONREGISTER).
  char *pszName;  // The name of this action, it must be a unique string.
  union {
    char *pszService;  // A service (called with wParam = 
                       // (WPARAM)(DWORD)actionID, lParam = 
                       // (LPARAM)(REPORTINFO *)&ri) or function to be called
                       // when the action has to be performed.
	ACTIONFUNCTION actionFunction;
  };
  HINSTANCE hInstance;	// Only needed when an options screen is available.
  DLGPROC pfnDlgProc;  // Optional, the callback procedure for the options
                       // dialog.
  char *pszTemplate;  // Optional, template for the options dialog, must be
                      // WS_CHILD.
  int flags;  // One of the ARF_* flags, see below.
} ACTIONREGISTER;

#define ARF_UNICODE  0x01  // This action processes unicode strings.
#define ARF_FUNCTION  0x02  // The actionFunction will be called instead of
                           // the service.
#define ARF_NOEXPORT  0x04  // This action cannot be exported. Set this flag in
                            // case you stored settings not using the helper
                            // functions at the end of this header. On export,
                            // TriggerPlugin will search for these settings
                            // and export them automatically. Contact-specific
                            // settings are never exported.

#if defined(UNICODE) || defined(_UNICODE)
#define ARF_TCHAR   ARF_UNICODE
#else
#define ARF_TCHAR   0
#endif

// The service or actionFunction will be called with a pointer to a REPORTINFO
// struct, containing information about the trigger event. If you can use
// TriggerData from this struct, always check the ri->td->dFlags before using
// it. It's up to you to deal with an action in case the expected TriggerData
// is not available. It's recommened though, to cancel your action. The
// ri->flags is a combination of the ACT_* flags, indicating how to process the
// call, see below.

#define ACT_PERFORM  0x01  // Your action is to be performed.
#define ACT_CLEANUP  0x02  // The settings associated to this action should be
                           // removed.

// Dialog Messages
// The following messages are to be processed by the options dialog, if there
// is one.

// WM_INITDIALOG

// Parameters:
// ------------------------
// lParam = (LPARAM)(DWORD)actionID
//   The action ID for which the options are to be set. This can be a new ID
//   or an ID of an action which is being edited. Initialize your options
//   dialog accordingly. There are helper function at the end of this header
//   file to read your settings for a certain action ID.

#define	TM_ADDACTION  WM_APP+12

// TM_ADDACTION
// 'OK' is pressed and a new action will be added. Save your settings using
// the given action ID. Helper functions can be found at the end of this
// header file.

// Parameters:
// ------------------------
// wParam = (WPARAM)(DWORD)actionID
//   The action ID for which the settings are to be saved. There are helper
//   functions at the end of this header file to store settings with a certain
//   action ID.
// lParam = 0

// Dialog Messages
// You can send the following messages to the parent window of your dialog.
// When initalizing your dialog, you might be interested in the TriggerData 
// the associated trigger is able to provide, you can do so by sending the
// folowing message to the parent of your dialog.

#define TM_GETTRIGGERINFO  WM_APP+13

// Parameters:
// ------------------------
// wParam = 0
// lParam = (LPARAM)(TRIGGERINFO *)&ti

// Return Value:
// ------------------------
// Returns 0 on success, the struct given will be filled with the requested
// information. Returns any other value on error.

typedef struct {
  int cbSize;  // (in) Set to sizeof(TRIGGERINFO).
  int dFlags;  // (out) The default DF_* flags used by the trigger (as indicated
               // by its TRIGGERREGISTER).
} TRIGGERINFO;

// --------------------------------------------------------------------------
// Conditions
// --------------------------------------------------------------------------

// Depending on the configuration of the user, a condition may need to hold
// for an action to be performed. A condition function is called and its
// return value specifies whether or not the condition holds. A condition
// needs to be registered. After its registered, the condition function might
// be called to check whether or not the condition holds.

#define MS_TRIGGER_REGISTERCONDITION  "/TriggerPlugin/RegisterCondition"

// Parameters:
// ------------------------
// wParam = (WPARAM)0
// lParam = (LPARAM)(CONDITIONREGISTER *)&cr
//   Pointer to a structure describing the condition to add (see below).

// Return Value:
// ------------------------
// Returns 0 on success, nozero otherwise. Registering an already existing
// condition will replace this previously registered condition.

typedef int (* CONDITIONFUNCTION)(DWORD conditionID, REPORTINFO *ri);

typedef struct {
  int cbSize;  // Set to sizeof(CONDITIONREGISTER).
  char *pszName;  // The name identifying this condition, must be unique.
  union {
    char *pszService;  // The service (wParam = (WPARAM)(DWORD)conditionID,
                       // lParam = (LPARAM)(REPORTINFO *)&ri) or function which
                       // is called to see whether the condition holds. Must
                       // return CRV_TRUE if the condition holds, CRV_FALSE
                       // otherwise.
    CONDITIONFUNCTION conditionFunction;
  };
  HINSTANCE hInstance;  // Only needed when an options dialog is available.
  DLGPROC pfnDlgProc;  // Optional, the dialog procedure for the options
                       // dialog.
  char *pszTemplate;  // Optional, template for the options dialog,  must be
                      // WS_CHILD.
  int flags;  // CRF_* flags, see below.
} CONDITIONREGISTER;

// The flags that can be used to register the condition.

#define CRF_UNICODE  0x01  // The condition function or service processes
                           // unicode strings.
#define CRF_FUNCTION  0x02  // The conditionFunction will be called instead of
                            // the service.
#define CRF_NOEXPORT  0x04  // This condition cannot be exported. Set this flag
                            // in case you stored settings not using the helper
                            // functions at the end of this header. On export,
                            // TriggerPlugin will search for these settings
                            // and export them automatically. Contact-specific
                            // settings are never exported.

#if defined(UNICODE) || defined(_UNICODE)
#define CRF_TCHAR   CRF_UNICODE
#else
#define CRF_TCHAR   0
#endif

// The service or conditionFunction will be called with a pointer to a
// REPORTINFO struct, containing information about the trigger event. If you
// can use TriggerData from this struct, always check the ri->td->dFlags before
// using it. It's up to you to deal with an condition in case the expected
// TriggerData is not available. It's recommened though, to return CRV_FALSE in
// those cases. The ri->flags is a combination of the CND_* flags, indicating
// how to process the call, see below.

// Return values for the condition function or service. The condition service
// or function is expected to return one of the following.

#define CRV_FALSE  0  // The condition does not hold.
#define CRV_TRUE  1  // The condition does hold.

// REPORTINFO flags, received by the condition function or service. These
// indicate how to process the call.

#define CND_PERFORM  0x01  // Perform your condition and return either
                           // CRV_TRUE or CRV_FALSE to indicate whether or not
                           // your condition holds at this moment.
#define CND_CLEANUP  0x02  // The condition is deleted. Remove your settings
                           // from the DB. There is a helper service below to
                           // easily remove settings given a condition ID.

// Dialog Messages
// The following messages are to be processed by the options dialog, if there
// is one.

// WM_INITDIALOG

// Parameters:
// ------------------------
// lParam = (LPARAM)(DWORD)conditionID
//   The condition ID for which the options are to be set. This can be a new ID
//   or an ID of a condition which is being edited. Initialize your options
//   dialog accordingly. There are helper function at the end of this header
//   file to read your settings for a certain condition ID.

#define	TM_ADDCONDITION  WM_APP+14

// TM_ADDCONDITION
// 'OK' is pressed and a new condition will be added. Save your settings using
// the given condition ID. Helper functions can be found at the end of this
// header file.

// Parameters:
// ------------------------
// wParam = (WPARAM)(DWORD)conditionID
//   The condition ID for which the settings are to be saved. There are helper
//   functions at the end of this header file to store settings with a certain
//   condition ID.
// lParam = 0

// When initalizing your dialog, you might be interested in the TriggerData the
// associated trigger is able to provide, you can find out by sending a
// TM_GETTRIGGERINFO message to the parent of your dialog. See the section on
// dialog messages for actions for more information (above).

// --------------------------------------------------------------------------
// Misc. Services
// --------------------------------------------------------------------------

#define MS_TRIGGER_ENABLETRIGGER  "/TriggerPlugin/EnableTrigger"

// Parameters:
// ------------------------
// wParam = (WPARAM)(DWORD)triggerID
//   The triggerID to set or get the state from or 0 for the global state.
// lParam = (LPARAM)(int)type
//   One of ETT_* (see below).

// Return Value:
// ------------------------
// Returns the state (0=disabled) if ETT_GETSTATE is given as lParam.
// Otherwise, it returns 0 if setting the state was succesful or any other on
// failure. The global state must be enabled if a single state is to be
// changed.

#define ETT_DISABLE  0  // Disable the trigger(s).
#define ETT_ENABLE  1  // Enable the trigger(s).
#define ETT_TOGGLE  2  // Toggle the state of the trigger(s).
#define ETT_GETSTATE  3  // Retrieve the state of the trigger (0=disabled).

#define ME_TRIGGER_TRIGGERENABLED	"/TriggerPlugin/TriggerEnabled"

// Parameters:
// ------------------------
// wParam = (WPARAM)(DWORD)triggerID
//   The triggerID to set or get the state from or 0 for the global state.
// lParam = (LPARAM)(int)type
//   Either ETT_DISABLE or ETT_ENABLE describing the new state.

// This event is fired when one or all of the triggers enabled state is 
// changed. This can be used to (de)initialize internal variables. For 
// example if your trigger reads the triggers from the DB in memory during 
// an initialization fase. It is recommended to do this when this event is 
// fired with wParam = 0 and lParam = ETT_ENABLE (remember triggers can be 
// imported, without calling your options screen callback). This event is 
// fired upon startup and shutdown in case the module is enabled.

// --------------------------------------------------------------------------
// Database Helper Services
// --------------------------------------------------------------------------

// The rest of this header file defines helper services and functions to easily
// store and retrieve settings for a certain trigger, action or condition.

#define MS_TRIGGER_REMOVESETTINGS  "/TriggerPlugin/RemoveSettings"

// Parameters:
// ------------------------
// wParam = (WPARAM)0
// lParam = (LPARAM)(REMOVETRIGGERSETTINGS *)&rts
//   Pointer to a structure describing the settings to remove (see below).

// Return Value:
// ------------------------
// Returns the number of settings removed from the database.

// This service helps you remove all settings you have written with the DB
// helper functions, defined at the end of this header file.

typedef struct {
  int cbSize;  // Set to sizeof(REMOVETRIGGERSETTINGS).
  char *prefix;  // A string indicating what kind of setting are to be removed,
                 // see below.
  DWORD id;  // The ID of the set of settings to be removed.
  char *szModule;  // The module where the settings are stored.
  HANDLE hContact;  // The contact for which the setting are to be removed. Can
                    // be INVALID_HANDLE_VALUE to remove the settings for all
                    // contacts and NULL.
} REMOVETRIGGERSETTINGS;

// The following prefixes indicate what kind of settings are to be removed from
// the database.

#define PREFIX_ACTIONID  "aid"  // The prefix for a DB setting associated to
                                // an action.
#define PREFIX_TRIGGERID  "tid"  // The prefix for a DB setting associated to
                                 // a trigger.
#define PREFIX_CONDITIONID  "cid"  // The prefix for a DB setting associated
                                   // to a condition.

#ifndef TRIGGER_NOHELPER

// Helper #1: RemoveAllTriggerSettings
// ------------------------
// Remove all settings from the DB given the triggerID and module.

static __inline int RemoveAllTriggerSettings(DWORD triggerID, char *szModule) {

  REMOVETRIGGERSETTINGS rts;

  rts.cbSize = sizeof(REMOVETRIGGERSETTINGS);
  rts.prefix = PREFIX_TRIGGERID;
  rts.id = triggerID;
  rts.szModule = szModule;
  rts.hContact = INVALID_HANDLE_VALUE;

  return CallService(MS_TRIGGER_REMOVESETTINGS, 0, (LPARAM)&rts);
}

// Helper #2: RemoveAllActionSettings
// ------------------------
// Remove all settings from the DB given the actionID and module.

static __inline int RemoveAllActionSettings(DWORD actionID, char *szModule) {

  REMOVETRIGGERSETTINGS rts;

  rts.cbSize = sizeof(REMOVETRIGGERSETTINGS);
  rts.prefix = PREFIX_ACTIONID;
  rts.id = actionID;
  rts.szModule = szModule;
  rts.hContact = INVALID_HANDLE_VALUE;

  return CallService(MS_TRIGGER_REMOVESETTINGS, 0, (LPARAM)&rts);
}

// Helper #1: RemoveAllConditionSettings
// ------------------------
// Remove all settings from the DB given the conditionID and module.

static __inline int RemoveAllConditionSettings(DWORD conditionID, char *szModule) {

  REMOVETRIGGERSETTINGS rts;

  rts.cbSize = sizeof(REMOVETRIGGERSETTINGS);
  rts.prefix = PREFIX_CONDITIONID;
  rts.id = conditionID;
  rts.szModule = szModule;
  rts.hContact = INVALID_HANDLE_VALUE;

  return CallService(MS_TRIGGER_REMOVESETTINGS, 0, (LPARAM)&rts);
}

// --------------------------------------------------------------------------
// Database Helper Functions
// --------------------------------------------------------------------------

// Basically, these function work the same as Miranda's helper functions for
// getting/setting DB settings. There is one extra parameter, the ID for the
// trigger/action/condition. The settings are named as follows:

// DBWriteTriggerSetting*(DWORD triggerID, ...) to write a setting given a
// trigger ID.
// DBGetTriggerSetting*(DWORD triggerID, ...) to read a setting given a
// trigger ID.
// DBWriteActionSetting*(DWORD actionID, ...) to write a setting given an
// action ID.
// DBGetActionSetting*(DWORD actionID, ...) to read a setting given an
// action ID.
// DBWriteConditionSetting*(DWORD conditionID, ...) to write a setting given a
// condition ID.
// DBGetConditionSetting*(DWORD conditionID, ...) to read a setting given a
// condition ID.

#define MAX_SETTING_LEN  255  // Max. length of a DB setting including the
                              // prefix and ID.

// --------------------------------------------------------------------------
// Database Helper Functions: Triggers
// --------------------------------------------------------------------------

static int __inline DBWriteTriggerSettingByte(DWORD triggerID, HANDLE hContact,const char *szModule,const char *szSetting,BYTE val) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBWriteContactSettingByte(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteTriggerSettingWord(DWORD triggerID, HANDLE hContact,const char *szModule,const char *szSetting,WORD val) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBWriteContactSettingWord(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteTriggerSettingDword(DWORD triggerID, HANDLE hContact,const char *szModule,const char *szSetting,DWORD val) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBWriteContactSettingDword(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteTriggerSettingString(DWORD triggerID, HANDLE hContact,const char *szModule,const char *szSetting,const char *val) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBWriteContactSettingString(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteTriggerSettingTString(DWORD triggerID, HANDLE hContact,const char *szModule,const char *szSetting,const TCHAR *val) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBWriteContactSettingTString(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteTriggerSettingWString(DWORD triggerID, HANDLE hContact,const char *szModule,const char *szSetting,const WCHAR *val) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBWriteContactSettingWString(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteTriggerSettingStringUtf(DWORD triggerID, HANDLE hContact,const char *szModule,const char *szSetting,const char *val) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBWriteContactSettingStringUtf(hContact, szModule, dbSetting, val);
}

static int __inline DBGetTriggerSettingByte(DWORD triggerID, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBGetContactSettingByte(hContact, szModule, dbSetting, errorValue);
}

static WORD __inline DBGetTriggerSettingWord(DWORD triggerID, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBGetContactSettingWord(hContact, szModule, dbSetting, errorValue);
}

static DWORD __inline DBGetTriggerSettingDword(DWORD triggerID, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBGetContactSettingDword(hContact, szModule, dbSetting, errorValue);
}

static int __inline DBGetTriggerSetting(DWORD triggerID, HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv) {


	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBGetContactSetting(hContact, szModule, dbSetting, dbv);
}

static int __inline DBGetTriggerSettingW(DWORD triggerID, HANDLE hContact,const char *szModule, const char *szSetting,DBVARIANT *dbv) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBGetContactSettingW(hContact, szModule, dbSetting, dbv);
}

static int __inline DBGetTriggerSettingTString(DWORD triggerID, HANDLE hContact,const char *szModule, const char *szSetting,DBVARIANT *dbv) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBGetContactSettingTString(hContact, szModule, dbSetting, dbv);
}

static int __inline DBGetTriggerSettingWString(DWORD triggerID, HANDLE hContact,const char *szModule,	const char *szSetting,DBVARIANT *dbv) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBGetContactSettingWString(hContact, szModule, dbSetting, dbv);
}

static int __inline DBGetTriggerSettingStringUtf(DWORD triggerID, HANDLE hContact,const char *szModule, const char *szSetting,DBVARIANT *dbv) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBGetContactSettingStringUtf(hContact, szModule, dbSetting, dbv);
}

static int __inline DBDeleteTriggerSetting(DWORD triggerID, HANDLE hContact,const char *szModule,const char *szSetting) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_TRIGGERID, triggerID, szSetting);
	return DBDeleteContactSetting(hContact, szModule, dbSetting);
}

// --------------------------------------------------------------------------
// Database Helper Functions: Actions
// --------------------------------------------------------------------------

static int __inline DBWriteActionSettingByte(DWORD actionID, HANDLE hContact,const char *szModule,const char *szSetting,BYTE val) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBWriteContactSettingByte(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteActionSettingWord(DWORD actionID, HANDLE hContact,const char *szModule,const char *szSetting,WORD val) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBWriteContactSettingWord(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteActionSettingDword(DWORD actionID, HANDLE hContact,const char *szModule,const char *szSetting,DWORD val) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBWriteContactSettingDword(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteActionSettingString(DWORD actionID, HANDLE hContact,const char *szModule,const char *szSetting,const char *val) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBWriteContactSettingString(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteActionSettingTString(DWORD actionID, HANDLE hContact,const char *szModule,const char *szSetting,const TCHAR *val) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBWriteContactSettingTString(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteActionSettingWString(DWORD actionID, HANDLE hContact,const char *szModule,const char *szSetting,const WCHAR *val) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBWriteContactSettingWString(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteActionSettingStringUtf(DWORD actionID, HANDLE hContact,const char *szModule,const char *szSetting,const char *val) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBWriteContactSettingStringUtf(hContact, szModule, dbSetting, val);
}

static int __inline DBGetActionSettingByte(DWORD actionID, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBGetContactSettingByte(hContact, szModule, dbSetting, errorValue);
}

static WORD __inline DBGetActionSettingWord(DWORD actionID, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBGetContactSettingWord(hContact, szModule, dbSetting, errorValue);
}

static DWORD __inline DBGetActionSettingDword(DWORD actionID, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBGetContactSettingDword(hContact, szModule, dbSetting, errorValue);
}

static int __inline DBGetActionSetting(DWORD actionID, HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv) {


	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBGetContactSetting(hContact, szModule, dbSetting, dbv);
}

static int __inline DBGetActionSettingW(DWORD actionID, HANDLE hContact,const char *szModule, const char *szSetting,DBVARIANT *dbv) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBGetContactSettingW(hContact, szModule, dbSetting, dbv);
}

static int __inline DBGetActionSettingTString(DWORD actionID, HANDLE hContact,const char *szModule, const char *szSetting,DBVARIANT *dbv) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBGetContactSettingTString(hContact, szModule, dbSetting, dbv);
}

static int __inline DBGetActionSettingWString(DWORD actionID, HANDLE hContact,const char *szModule, const char *szSetting,DBVARIANT *dbv) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBGetContactSettingWString(hContact, szModule, dbSetting, dbv);
}

static int __inline DBGetActionSettingStringUtf(DWORD actionID, HANDLE hContact,const char *szModule, const char *szSetting,DBVARIANT *dbv) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBGetContactSettingStringUtf(hContact, szModule, dbSetting, dbv);
}

static int __inline DBDeleteActionSetting(DWORD actionID, HANDLE hContact,const char *szModule,const char *szSetting) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_ACTIONID, actionID, szSetting);
	return DBDeleteContactSetting(hContact, szModule, dbSetting);
}

// --------------------------------------------------------------------------
// Database Helper Functions: Conditions
// --------------------------------------------------------------------------

static int __inline DBWriteConditionSettingByte(DWORD conditionID, HANDLE hContact,const char *szModule,const char *szSetting,BYTE val) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_CONDITIONID, conditionID, szSetting);
	return DBWriteContactSettingByte(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteConditionSettingWord(DWORD conditionID, HANDLE hContact,const char *szModule,const char *szSetting,WORD val) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_CONDITIONID, conditionID, szSetting);
	return DBWriteContactSettingWord(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteConditionSettingDword(DWORD conditionID, HANDLE hContact,const char *szModule,const char *szSetting,DWORD val) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_CONDITIONID, conditionID, szSetting);
	return DBWriteContactSettingDword(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteConditionSettingString(DWORD conditionID, HANDLE hContact,const char *szModule,const char *szSetting,const char *val) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_CONDITIONID, conditionID, szSetting);
	return DBWriteContactSettingString(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteConditionSettingTString(DWORD conditionID, HANDLE hContact,const char *szModule,const char *szSetting,const TCHAR *val) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_CONDITIONID, conditionID, szSetting);
	return DBWriteContactSettingTString(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteConditionSettingWString(DWORD conditionID, HANDLE hContact,const char *szModule,const char *szSetting,const WCHAR *val) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_CONDITIONID, conditionID, szSetting);
	return DBWriteContactSettingWString(hContact, szModule, dbSetting, val);
}

static int __inline DBWriteConditionSettingStringUtf(DWORD conditionID, HANDLE hContact,const char *szModule,const char *szSetting,const char *val) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_CONDITIONID, conditionID, szSetting);
	return DBWriteContactSettingStringUtf(hContact, szModule, dbSetting, val);
}

static int __inline DBGetConditionSettingByte(DWORD conditionID, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_CONDITIONID, conditionID, szSetting);
	return DBGetContactSettingByte(hContact, szModule, dbSetting, errorValue);
}

static WORD __inline DBGetConditionSettingWord(DWORD conditionID, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_CONDITIONID, conditionID, szSetting);
	return DBGetContactSettingWord(hContact, szModule, dbSetting, errorValue);
}

static DWORD __inline DBGetConditionSettingDword(DWORD conditionID, HANDLE hContact, const char *szModule, const char *szSetting, int errorValue) {


	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_CONDITIONID, conditionID, szSetting);
	return DBGetContactSettingDword(hContact, szModule, dbSetting, errorValue);
}

static int __inline DBGetConditionSetting(DWORD conditionID, HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv) {


	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_CONDITIONID, conditionID, szSetting);
	return DBGetContactSetting(hContact, szModule, dbSetting, dbv);
}

static int __inline DBGetConditionSettingW(DWORD conditionID, HANDLE hContact,const char *szModule, const char *szSetting,DBVARIANT *dbv) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_CONDITIONID, conditionID, szSetting);
	return DBGetContactSettingW(hContact, szModule, dbSetting, dbv);
}

static int __inline DBGetConditionSettingTString(DWORD conditionID, HANDLE hContact,const char *szModule, const char *szSetting,DBVARIANT *dbv) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_CONDITIONID, conditionID, szSetting);
	return DBGetContactSettingTString(hContact, szModule, dbSetting, dbv);
}

static int __inline DBGetConditionSettingWString(DWORD conditionID, HANDLE hContact,const char *szModule, const char *szSetting,DBVARIANT *dbv) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_CONDITIONID, conditionID, szSetting);
	return DBGetContactSettingWString(hContact, szModule, dbSetting, dbv);
}

static int __inline DBGetConditionSettingStringUtf(DWORD conditionID, HANDLE hContact,const char *szModule, const char *szSetting,DBVARIANT *dbv) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_CONDITIONID, conditionID, szSetting);
	return DBGetContactSettingStringUtf(hContact, szModule, dbSetting, dbv);
}

static int __inline DBDeleteConditionSetting(DWORD conditionID, HANDLE hContact,const char *szModule,const char *szSetting) {

	char dbSetting[MAX_SETTING_LEN];

	mir_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s", PREFIX_CONDITIONID, conditionID, szSetting);
	return DBDeleteContactSetting(hContact, szModule, dbSetting);
}

#endif // nohelper
#endif // m_trigger
