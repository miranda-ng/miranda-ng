/*
===============================================================================
                                PopUp plugin
Plugin Name: PopUp
Plugin authors: Luca Santarelli aka hrk (hrk@users.sourceforge.net)
                Victor Pavlychko (nullbie@gmail.com)
===============================================================================
The purpose of this plugin is to give developers a common "platform/interface"
to show PopUps. It is born from the source code of NewStatusNotify, another
plugin I've made.

Remember that users *must* have this plugin enabled, or they won't get any
popup. Write this in the requirements, do whatever you wish ;-)... but tell
them!
===============================================================================
*/

#ifndef __m_popup_h__
#define __m_popup_h__

/*
NOTE! Since Popup 1.0.1.2 there is a main meun group called "PopUps" where I
have put a "Enable/Disable" item. You can add your own "enable/disable" items
by adding these lines before you call MS_CLIST_ADDMAINMENUITEM:
mi.pszPopUpName = Translate("PopUps");
mi.position = 0; //You don't need it and it's better if you put it to zero.
*/

#ifndef POPUP_VERSION
#define POPUP_VERSION 0x02010003
#endif

#define MAX_CONTACTNAME 2048
#define MAX_SECONDLINE  2048
#define MAX_ACTIONTITLE 64

// Popup Action flags
#define PAF_ENABLED	0x01	// Actions is enabled. You may store one global
							// action set and toggle some items depending on
							// popup you are requesting

// ANSI Popup Action
typedef struct 
{
	int cbSize;		// sizeof(POPUPACTION)
	HICON lchIcon;	// Action Icon
	// Action title text. Please use module name as prefix
	// (e.g. "Popup Plus/Dismiss Popup") and don't translate
	// This is translates by popup. So no unicode.
	char lpzTitle[MAX_ACTIONTITLE];
	DWORD flags;	// set of PAF_* flags
	WPARAM wParam;	// wParam for UM_POPUPACTION message
	LPARAM lParam;	// lParam for UM_POPUPACTION message
} POPUPACTION, *LPPOPUPACTION;

// This is the basic data you'll need to fill and pass to the service function.
typedef struct
{
	HANDLE lchContact;							// Handle to the contact, can be NULL (main contact).
	HICON lchIcon;								// Handle to a icon to be shown. Cannot be NULL.
	union
	{
		char lptzContactName[MAX_CONTACTNAME];	// This is the contact name or the first line in the plugin. Cannot be NULL.
		char lpzContactName[MAX_CONTACTNAME];	
	};
	union
	{
		char lptzText[MAX_SECONDLINE];			// This is the second line text. Users can choose to hide it. Cannot be NULL.
		char lpzText[MAX_SECONDLINE];			
	};
	COLORREF colorBack;							// COLORREF to be used for the background. Can be NULL, default will be used.
	COLORREF colorText;							// COLORREF to be used for the text. Can be NULL, default will be used.
	WNDPROC PluginWindowProc;					// Custom window proceudre. It *MUST* use stdcall (CALLBACK) convention. Please read docs.
	void * PluginData;							// Some custom data to store.
} POPUPDATA, * LPPOPUPDATA;

// Extended popup data
typedef struct
{
	HANDLE lchContact;
	HICON lchIcon;
	union
	{
		char lptzContactName[MAX_CONTACTNAME];
		char lpzContactName[MAX_CONTACTNAME];	
	};
	union
	{
		char lptzText[MAX_SECONDLINE];
		char lpzText[MAX_SECONDLINE];			
	};
	COLORREF colorBack;
	COLORREF colorText;
	WNDPROC PluginWindowProc;
	void * PluginData;
	int iSeconds;								// Custom delay time in seconds. -1 means "forever", 0 means "default time".

#if POPUP_VERSION < 0x02010003
	// For compatability
	char cZero[16];								// Some unused bytes which may come useful in the future.

#else
	// you *MUST* pass APF_NEWDATA flag for services to take care of this data
	HANDLE hNotification;						// Reserved. Must be NULL
	int actionCount;							// Amount of passed actions
	LPPOPUPACTION lpActions;					// Popup Actions
	int cbSize;									// struct size for future
#endif
} POPUPDATAEX, *LPPOPUPDATAEX;

// Unicode version of POPUPDATAEX
typedef struct
{
	HANDLE lchContact;
	HICON lchIcon;
	union
	{
		WCHAR lptzContactName[MAX_CONTACTNAME];
		WCHAR lpwzContactName[MAX_CONTACTNAME];	
	};
	union
	{
		WCHAR lptzText[MAX_CONTACTNAME];
		WCHAR lpwzText[MAX_CONTACTNAME];	
	};
	COLORREF colorBack;                   
	COLORREF colorText;
	WNDPROC PluginWindowProc;
	void * PluginData;
	int iSeconds;
#if POPUP_VERSION < 0x02010003
	char cZero[16];
#else
	HANDLE hNotification;
	int actionCount;
	LPPOPUPACTION lpActions;
	int cbSize;
#endif
} POPUPDATAW, *LPPOPUPDATAW;

#if defined(_UNICODE) || defined(UNICODE)
	typedef POPUPDATAW		POPUPDATAT;
	typedef LPPOPUPDATAW	LPPOPUPDATAT;
#else
	typedef POPUPDATAEX	POPUPDATAT;
	typedef LPPOPUPDATAEX	LPPOPUPDATAT;
#endif

/* PopUp/AddPopup
Creates, adds and shows a popup, given a (valid) POPUPDATA structure pointer.

wParam = (WPARAM)(*POPUPDATA)PopUpDataAddress
lParam = 0

Returns: > 0 on success, 0 if creation went bad, -1 if the PopUpData contained unacceptable values.
NOTE: it returns -1 if the PopUpData was not valid, if there were already too many popups, if the module was disabled.
Otherwise, it can return anything else...

Popup Plus 2.0.4.0+
You may pass additional creation flags via lParam:
	APF_RETURN_HWND ....... function returns handle to newly created popup window (however this calls are a bit slower)
	APF_CUSTOM_POPUP ...... new popup is created in hidden state and doesn't obey to popup queue rules.
							you may control it via UM_* messages and custom window procedure (not yet implemented)
	APF_NO_HISTORY ........ do not log this popup in popup history (useful for previews)
	APF_NO_POPUP .......... do not show popup. this is useful if you want popup yo be stored in history only
	APF_NEWDATA ........... use new version of POPUPDATAEX/POPUPDATAW structs
*/
#define APF_RETURN_HWND  0x01
#define APF_CUSTOM_POPUP 0x02
#define APF_NO_HISTORY   0x04
#define APF_NO_POPUP     0x08
#define APF_NEWDATA      0x10

#define MS_POPUP_ADDPOPUP "PopUp/AddPopUp"
static int __inline PUAddPopUp(POPUPDATA* ppdp) {
	return CallService(MS_POPUP_ADDPOPUP, (WPARAM)ppdp,0);
}

#define MS_POPUP_ADDPOPUPEX "PopUp/AddPopUpEx"
static int __inline PUAddPopUpEx(POPUPDATAEX* ppdp) {
	return CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)ppdp,0);
}

#define MS_POPUP_ADDPOPUPW "PopUp/AddPopUpW"
static int __inline PUAddPopUpW(POPUPDATAW* ppdp) {
	return CallService(MS_POPUP_ADDPOPUPW, (WPARAM)ppdp,0);
}

#if defined(_UNICODE) || defined(UNICODE)
	#define MS_POPUP_ADDPOPUPT	MS_POPUP_ADDPOPUPW
	#define PUAddPopUpT			PUAddPopUpW
#else
	#define MS_POPUP_ADDPOPUPT	MS_POPUP_ADDPOPUPEX
	#define PUAddPopUpT			PUAddPopUpEx
#endif


/* PopUp/GetContact
Returns the handle to the contact associated to the specified PopUpWindow.

wParam = (WPARAM)(HWND)hPopUpWindow
lParam = 0;

Returns: the HANDLE of the contact. Can return NULL, meaning it's the main contact. -1 means failure.
*/
#define MS_POPUP_GETCONTACT "PopUp/GetContact"
static HANDLE __inline PUGetContact(HWND hPopUpWindow) {
	return (HANDLE)CallService(MS_POPUP_GETCONTACT, (WPARAM)hPopUpWindow,0);
}

/* PopUp/GetPluginData
Returns custom plugin date associated with popup

wParam = (WPARAM)(HWND)hPopUpWindow
lParam = (LPARAM)(PLUGINDATA*)PluginDataAddress;

Returns: the address of the PLUGINDATA structure. Can return NULL, meaning nothing was given. -1 means failure.

IMPORTANT NOTE: it doesn't seem to work if you do:
CallService(..., (LPARAM)aPointerToAStruct);
and then use that struct.
Do this, instead:
aPointerToStruct = CallService(..., (LPARAM)aPointerToAStruct);
and it will work. Just look at the example I've written above (PopUpDlgProc).

*/
#define MS_POPUP_GETPLUGINDATA "PopUp/GetPluginData"
static void __inline * PUGetPluginData(HWND hPopUpWindow) {
	long * uselessPointer = NULL;
	return (void*)CallService(MS_POPUP_GETPLUGINDATA,(WPARAM)hPopUpWindow,(LPARAM)uselessPointer);
}

/* PopUp/IsSecondLineShown
Checks if second line is enable

wParam = 0
lParam = 0

Returns: 0 if the user has chosen not to have the second line, 1 if he choose to have the second line.
*/
#define MS_POPUP_ISSECONDLINESHOWN "PopUp/IsSecondLineShown"
static BOOL __inline PUIsSecondLineShown() {
	return (BOOL)CallService(MS_POPUP_ISSECONDLINESHOWN,0,0);
}

/* PopUp/Query

Requests an action or an answer from PopUp module.

wParam = (WPARAM)wpQuery

returns 0 on success, -1 on error, 1 on stupid calls ;-)
*/

#define PUQS_ENABLEPOPUPS	1	// returns 0 if state was changed, 1 if state wasn't changed
#define PUQS_DISABLEPOPUPS	2	// " "
#define PUQS_GETSTATUS		3	//Returns 1 (TRUE) if popups are enabled, 0 (FALSE) if popups are disabled.

#define MS_POPUP_QUERY "PopUp/Query"

/* UM_FREEPLUGINDATA
Process this message if you have allocated your own memory. (i.e.: POPUPDATA.PluginData != NULL)

wParam = 0
lParam = 0
*/
#define UM_FREEPLUGINDATA        (WM_USER + 0x0200)

/* UM_DESTROYPOPUP
Send this message when you want to destroy the popup, or use the function below.

wParam = 0
lParam = 0
*/
#define UM_DESTROYPOPUP          (WM_USER + 0x0201)
static int __inline PUDeletePopUp(HWND hWndPopUp) {
	return (int)SendMessage(hWndPopUp, UM_DESTROYPOPUP,0,0);
}

/* UM_INITPOPUP
This message is sent to the PopUp when its creation has been finished, so POPUPDATA (and thus your PluginData) is reachable.
Catch it if you needed to catch WM_CREATE or WM_INITDIALOG, which you'll never ever get in your entire popup-life.
Return value: if you process this message, return 0. If you don't process it, return 0. Do whatever you like ;-)

wParam = (WPARAM)(HWND)hPopUpWindow (this is useless, you get message inside your popup window)
lParam = 0
*/
#define UM_INITPOPUP             (WM_USER + 0x0202)

/* PopUp/Changetext
Changes the text displayed in the second line of the popup.

wParam = (WPARAM)(HWND)hPopUpWindow
lParam = (LPARAM)(char*)lpzNewText

returns: > 0 for success, -1 for failure, 0 if the failure is due to second line not being shown. (but you could call
PUIsSecondLineShown() before changing the text...)
*/
#define MS_POPUP_CHANGETEXT "PopUp/Changetext"
static int __inline PUChangeText(HWND hWndPopUp, LPCTSTR lpzNewText) {
	return (int)CallService(MS_POPUP_CHANGETEXT, (WPARAM)hWndPopUp, (LPARAM)lpzNewText);
}

#define MS_POPUP_CHANGETEXTW "PopUp/ChangetextW"
static int __inline PUChangeTextW(HWND hWndPopUp, LPCWSTR lpwzNewText) {
	return (int)CallService(MS_POPUP_CHANGETEXTW, (WPARAM)hWndPopUp, (LPARAM)lpwzNewText);
}

#if defined(_UNICODE) || defined(UNICODE)
	#define MS_POPUP_CHANGETEXTT	MS_POPUP_CHANGETEXTW
	#define PUChangeTextT			PUChangeTextW
#else
	#define MS_POPUP_CHANGETEXTT	MS_POPUP_CHANGETEXT
	#define PUChangeTextT			PUChangeText
#endif

/* PopUp/Change
Changes the entire popup

wParam = (WPARAM)(HWND)hPopUpWindow
lParam = (LPARAM)(POPUPDATAEX*)newData
*/
#define MS_POPUP_CHANGE "PopUp/Change"
static int __inline PUChange(HWND hWndPopUp, POPUPDATAEX *newData) {
	return (int)CallService(MS_POPUP_CHANGE, (WPARAM)hWndPopUp, (LPARAM)newData);
}

#define MS_POPUP_CHANGEW "PopUp/ChangeW"
static int __inline PUChangeW(HWND hWndPopUp, POPUPDATAW *newData) {
	return (int)CallService(MS_POPUP_CHANGE, (WPARAM)hWndPopUp, (LPARAM)newData);
}

#if defined(_UNICODE) || defined(UNICODE)
	#define MS_POPUP_CHANGET	MS_POPUP_CHANGEW
	#define PUChangeT			PUChangeW
#else
	#define MS_POPUP_CHANGET	MS_POPUP_CHANGE
	#define PUChangeT			PUChange
#endif

/* UM_CHANGEPOPUP
This message is triggered by Change/ChangeText services. You also may post it directly, but make
sure you allocate memory via miranda mmi, because popup will mir_free() them!

wParam = Modification type
lParam = value of type defined by wParam
*/

#define CPT_TEXT	1 // lParam = (char *)text
#define CPT_TEXTW	2 // lParam = (WCHAR *)text
#define CPT_TITLE	3 // lParam = (char *)title
#define CPT_TITLEW	4 // lParam = (WCHAR *)title
#define CPT_DATA	5 // lParam = (POPUPDATA *)data
#define CPT_DATAEX	6 // lParam = (POPUPDATAEX *)data
#define CPT_DATAW	7 // lParam = (POPUPDATAW *)data
#define CPT_DATA2	8 // lParam = (POPUPDATA2 *)data -- see m_popup2.h for details

#define UM_CHANGEPOPUP			(WM_USER + 0x0203)

#if defined(_UNICODE) || defined(UNICODE)
	#define CPT_TEXTT	CPT_TEXTW
	#define CPT_TITLET	CPT_TITLEW
	#define CPT_DATAT	CPT_DATAW
#else
	#define CPT_TEXTT	CPT_TEXT
	#define CPT_TITLET	CPT_TITLE
	#define CPT_DATAT	CPT_DATA
#endif

/* UM_POPUPACTION
Popup Action notification

wParam and lParam are specified bu plugin.
wParam = 0 is used buy popup plus internally!
*/

#define UM_POPUPACTION           (WM_USER + 0x0204)

/* UM_POPUPMODIFYACTIONICON
Modify Popup Action Icon

wParam = (WPARAM)(LPPOPUPACTIONID)&actionId
lParam = (LPARAM)(HICON)hIcon
*/

typedef struct
{
	WPARAM wParam;
	LPARAM lParam;
} POPUPACTIONID, *LPPOPUPACTIONID;

#define UM_POPUPMODIFYACTIONICON  (WM_USER + 0x0205)

/* UM_POPUPSHOW
Show popup at position

wParam = x
lParam = y
*/
#define UM_POPUPSHOW			 (WM_USER + 0x0206)

#define MS_POPUP_CHANGEW "PopUp/ChangeW"
static int __inline PUModifyActionIcon(HWND hWndPopUp, WPARAM wParam, LPARAM lParam, HICON hIcon) {
	POPUPACTIONID actionId = { wParam, lParam };
	return (int)SendMessage(hWndPopUp, UM_POPUPMODIFYACTIONICON, (WPARAM)&actionId, (LPARAM)hIcon);
}

/* PopUp/ShowMessage
This is mainly for developers.
Shows a warning message in a PopUp. It's useful if you need a "MessageBox" like function, but you don't want a modal
window (which will interfere with a DialogProcedure. MessageBox steals focus and control, this one not.

wParam = (char *)lpzMessage
lParam = 0;

Returns: 0 if the popup was shown, -1 in case of failure.
*/
#define SM_WARNING	0x01	//Triangle icon.
#define SM_NOTIFY	0x02	//Exclamation mark icon.
#define SM_ERROR	0x03	//Cross icon.
#define MS_POPUP_SHOWMESSAGE "PopUp/ShowMessage"

static int __inline PUShowMessage(char *lpzText, DWORD kind) {
	return (int)CallService(MS_POPUP_SHOWMESSAGE, (WPARAM)lpzText,(LPARAM)kind);
}

/* PopUp/RegisterActions
Registers your action in popup action list

wParam = (WPARAM)(LPPOPUPACTION)actions
lParam = (LPARAM)actionCount

Returns: 0 if the popup was shown, -1 in case of failure.
*/
#define MS_POPUP_REGISTERACTIONS "PopUp/RegisterActions"

static int __inline PURegisterActions(LPPOPUPACTION actions, int count) {
	return (int)CallService(MS_POPUP_REGISTERACTIONS, (WPARAM)actions,(LPARAM)count);
}

/* PopUp/RegisterNotification
Registers your action in popup action list

wParam = (WPARAM)(LPPOPUPNOTIFICATION)info
lParam = 0

Returns: handle of registered notification or sero on failure
*/
#define MS_POPUP_REGISTERNOTIFICATION "PopUp/RegisterNotification"

#define PNAF_CALLBACK			0x01

#define POPUP_ACTION_NOTHING	"Do nothing"
#define POPUP_ACTION_DISMISS	"Dismiss popup"

typedef struct
{
	char lpzTitle[64];
	DWORD dwFlags;
	union
	{
		struct
		{
			char lpzLModule[MAXMODULELABELLENGTH];
			char lpzLSetting[MAXMODULELABELLENGTH];
			DBVARIANT dbvLData;
			char lpzRModule[MAXMODULELABELLENGTH];
			char lpzRSetting[MAXMODULELABELLENGTH];
			DBVARIANT dbvRData;
		};
		struct
		{
			DWORD dwCookie;
			void (*pfnCallback)(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, DWORD cookie);
		};
	};
} POPUPNOTIFYACTION, *LPPOPUPNOTIFYACTION;

#define PNF_CONTACT				0x01

typedef struct
{
	int cbSize;
	DWORD dwFlags;			// set of PNF_* flags
	char lpzGroup[MAXMODULELABELLENGTH];
	char lpzName[MAXMODULELABELLENGTH];
	HICON lchIcon;			// this will be registered in icolib
	COLORREF colorBack;		// this will be registered in fontservice
	COLORREF colorText;		// this will be registered in fontservice
	int iSeconds;			// default timeout
	int actionCount;		// for unified action comboboxes
	LPPOPUPNOTIFYACTION lpActions;
	char *lpzLAction;
	char *lpzRAction;
	char *pszReserved1;		// reserved for future use
	DLGPROC pfnReserved2;	// reserved for future use
} POPUPNOTIFICATION, *LPPOPUPNOTIFICATION;

static HANDLE __inline PURegisterNotification(LPPOPUPNOTIFICATION notification) {
	return (HANDLE)CallService(MS_POPUP_REGISTERNOTIFICATION, (WPARAM)notification, (LPARAM)0);
}

/* PopUp/UnhookEventAsync
Using of "UnhookEvent" inside PluginWindowProc in conjunction with HookEventMessage
may cause deadlocks. Use this service instead. It will queue event unhook into main
thread and notify you when everything has finished.

Deadlock scenario:
   1. Event is fired with NotifyEventHooks in the main thread
   2. Miranda core calls EnterCriticalSection(csHooks) and starts notifications
   3. You decide to unhook event, therefore call UnhookEvent
   4. Miranda core *INSIDE YOUR THREAD* calls EnterCriticalSection(csHooks) and
      waits for main thread to finish processing
   5. Main thread calls SendMessage(hwnd, ...) to notify your window
   6. Your window's thread is busy waiting for main thread to leave critical section
   7. deadlock....

wParam = (WPARAM)(HWND)hwndPopup
lParam = (LPARAM)(HANDLE)hEvent

Returns: 0 if everything gone ok. -1 if service was not found (and unsafe unhook was performed)
*/

#define MS_POPUP_UNHOOKEVENTASYNC "PopUp/UnhookEventAsync"

/* UM_POPUPUNHOOKCOMPLETE
Modify Popup Action Icon

wParam = 0
lParam = (LPARAM)(HANDLE)hEventUnhooked
*/
#define UM_POPUPUNHOOKCOMPLETE           (WM_USER + 0x0206)

static int __inline PUUnhookEventAsync(HWND hwndPopup, HANDLE hEvent) {
	if (ServiceExists(MS_POPUP_UNHOOKEVENTASYNC))
		return (int)CallService(MS_POPUP_UNHOOKEVENTASYNC, (WPARAM)hwndPopup,(LPARAM)hEvent);

	// old popup plugins: unhook service not found
	UnhookEvent(hEvent);
	PostMessage(hwndPopup, UM_POPUPUNHOOKCOMPLETE, 0, (LPARAM)hEvent);
	return 0;
}

#ifdef __cplusplus
/* PopUp/RegisterVfx
Register new animation (fade in/out) effect
wParam = 0
lParam = (LPARAM)(char *)vfx_name
*/

#define MS_POPUP_REGISTERVFX	"PopUp/RegisterVfx"

/* PopUp/Vfx/<vfx_name>
Define this service to create vfx instance
wParam = 0
lParam = 0
return = (int)(IPopupPlusEffect *)vfx
*/

#define MS_POPUP_CREATEVFX		"PopUp/Vfx/"

class IPopupPlusEffect
{
public:
	virtual void beginEffect(int w, int h, int alpha0, int alpha1, int frameCount) = 0;
	virtual void beginFrame(int frame) = 0;
	virtual int getPixelAlpha(int x, int y) = 0;
	virtual void endFrame() = 0;
	virtual void endEffect() = 0;
	virtual void destroy() = 0;
};
#endif // __cplusplus

#endif // __m_popup_h__
