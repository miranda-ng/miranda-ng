/*
 ============================================================================== = 
                                Popup plugin
Plugin Name: Popup
Plugin authors: Luca Santarelli aka hrk (hrk@users.sourceforge.net)
                Victor Pavlychko aka zazoo (nullbie@gmail.com)
 ============================================================================== = 
The purpose of this plugin is to give developers a common "platform/interface"
to show Popups. It is born from the source code of NewStatusNotify, another
plugin I've made.

Remember that users *must* have this plugin enabled, or they won't get any
popup. Write this in the requirements, do whatever you wish ;-)... but tell
them!
 ============================================================================== = 
*/

#ifndef M_POPUP_H
#define M_POPUP_H

#include <m_database.h>

/*
NOTE! Since Popup 1.0.1.2 there is a main meun group called "Popups" where I
have put a "Enable/Disable" item. You can add your own "enable/disable" items
by adding these lines before you call MS_CLIST_ADDMAINMENUITEM:
mi.pszPopupName = Translate("Popups");
mi.position = 0; //You don't need it and it's better if you put it to zero.
*/

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

///////////////////////////////////////////////////////////////
// Few notes about new popup api
// ------------------------------
// When you call any ADD service, Popup Plus creates local
// copy of POPUPDATA2 to store the data. Each time you call
// CHANGE service this data is updated. You can use the
// MS_POPUP_GETDATA2 service to retrieve Popups's copy of
// this data, however you MUST NOT chahge that.

// unicode or ansi mode
#define PU2_ANSI    0x00
#define PU2_UNICODE 0x01
#if defined(UNICODE) || defined(_UNICODE)
#define PU2_TCHAR PU2_UNICODE
#else
#define PU2_TCHAR PU2_ANSI
#endif

#define PU2_CUSTOM_POPUP 0x02

typedef struct
{
	// general
	int cbSize;
	DWORD flags;

	// miranda bindings
	MCONTACT lchContact;
	HANDLE lchEvent;

	// style
	COLORREF colorBack;
	COLORREF colorText;
	HICON lchIcon;
	HBITMAP hbmAvatar;
	union
	{
		char *lpzTitle;
		WCHAR *lpwzTitle;
		TCHAR *lptzTitle;
	};
	union
	{
		char *lpzText;
		WCHAR *lpwzText;
		TCHAR *lptzText;
	};
	char *lpzSkin;

	// time and timeout
	int iSeconds;
	DWORD dwTimestamp;

	// plugin bindings
	WNDPROC PluginWindowProc;
	void *PluginData;

	// popup actions
	int actionCount;
	POPUPACTION *lpActions;

	HANDLE lchNotification;
} POPUPDATA2, *LPPOPUPDATA2;

// Creates new popup
//    wParam = (WPARAM)(LPPOPUPDATA2)&ppd2
//    lParam = (LPARAM)(combination of APF_* flags)
//    returns: window handle (if requested) of NULL on success, -1 on failure.
#define MS_POPUP_ADDPOPUP2 "Popup/AddPopup2"

// Update an popup
//    wParam = (WPARAM)(HWND)hwndPopup
//    lParam = (LPARAM)(LPPOPUPDATA2)&ppd2
//    returns: zero on success, -1 on failure.
#define MS_POPUP_CHANGEPOPUP2 "Popup/ChangePopup2"

// deprecatet !!! (only for compatibility) use new POPUPDATA2 struct for extended popup
// Unicode version of POPUPDATAEX_V2
typedef struct
{
	MCONTACT lchContact;
	HICON lchIcon;
	union
	{
		WCHAR lptzContactName[MAX_CONTACTNAME];
		WCHAR lpwzContactName[MAX_CONTACTNAME];
	};
	union
	{
		WCHAR lptzText[MAX_SECONDLINE];
		WCHAR lpwzText[MAX_SECONDLINE];
	};
	COLORREF colorBack;
	COLORREF colorText;
	WNDPROC PluginWindowProc;
	void * PluginData;
	int iSeconds;
	// +2.1.0.3
	// you *MUST* pass APF_NEWDATA flag for services to take care of this data
	HANDLE hNotification;
	int actionCount;
	LPPOPUPACTION lpActions;
	int cbSize;
} POPUPDATAW_V2, *LPPOPUPDATAW_V2;

// deprecatet !!! (only for compatibility) use new POPUPDATA2 struct for extended popup
#if defined(_UNICODE) || defined(UNICODE)
typedef POPUPDATAW_V2		POPUPDATAT_V2;
typedef LPPOPUPDATAW_V2	LPPOPUPDATAT_V2;
#endif

// Extended popup data
typedef struct
{
	MCONTACT lchContact;
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
	void *PluginData;
	int iSeconds;								// Custom delay time in seconds. -1 means "forever", 0 means "default time".
	char cZero[16];								// Some unused bytes which may come useful in the future.
} POPUPDATA, *LPPOPUPDATA;

// Unicode version of POPUPDATA
typedef struct
{
	MCONTACT lchContact;
	HICON lchIcon;
	union
	{
		WCHAR lptzContactName[MAX_CONTACTNAME];
		WCHAR lpwzContactName[MAX_CONTACTNAME];
	};
	union
	{
		WCHAR lptzText[MAX_SECONDLINE];
		WCHAR lpwzText[MAX_SECONDLINE];
	};
	COLORREF colorBack;
	COLORREF colorText;
	WNDPROC PluginWindowProc;
	void *PluginData;
	int iSeconds;
	char cZero[16];
} POPUPDATAW, *LPPOPUPDATAW;

#if defined(_UNICODE) || defined(UNICODE)
	typedef POPUPDATAW   POPUPDATAT;
	typedef LPPOPUPDATAW	LPPOPUPDATAT;
#else
	typedef POPUPDATA    POPUPDATAT;
	typedef LPPOPUPDATA  LPPOPUPDATAT;
#endif

/* Popup/AddPopup
Creates, adds and shows a popup, given a (valid) POPUPDATA structure pointer.

wParam = (WPARAM)(*POPUPDATA)PopupDataAddress
lParam = 0

Returns: > 0 on success, 0 if creation went bad, -1 if the PopupData contained unacceptable values.
NOTE: it returns -1 if the PopupData was not valid, if there were already too many popups, if the module was disabled.
Otherwise, it can return anything else...

Popup Plus 2.0.4.0+
You may pass additional creation flags via lParam:
	APF_RETURN_HWND ....... function returns handle to newly created popup window (however this calls are a bit slower)
	APF_CUSTOM_POPUP ...... new popup is created in hidden state and doesn't obey to popup queue rules.
							you may control it via UM_* messages and custom window procedure
*/
#define APF_RETURN_HWND  0x1
#define APF_CUSTOM_POPUP 0x2
#define APF_NO_HISTORY   0x04	//do not log this popup in popup history (useful for previews)
#define APF_NO_POPUP     0x08	//do not show popup. this is useful if you want popup yo be stored in history only
#define APF_NEWDATA      0x10	//deprecatet!! only for use with old POPUPDATAEX_V2/POPUPDATAW_V2 structs

#define MS_POPUP_ADDPOPUP "Popup/AddPopupEx"
static INT_PTR __inline PUAddPopup(POPUPDATA* ppdp) {
	return CallService(MS_POPUP_ADDPOPUP, (WPARAM)ppdp, 0);
}

#define MS_POPUP_ADDPOPUPW "Popup/AddPopupW"
static INT_PTR __inline PUAddPopupW(POPUPDATAW* ppdp) {
	return CallService(MS_POPUP_ADDPOPUPW, (WPARAM)ppdp, 0);
}

static INT_PTR __inline PUAddPopupW(POPUPDATAW_V2* ppdp) {
	return CallService(MS_POPUP_ADDPOPUPW, (WPARAM)ppdp, 0);
}

#if defined(_UNICODE) || defined(UNICODE)
	#define MS_POPUP_ADDPOPUPT	MS_POPUP_ADDPOPUPW
	#define PUAddPopupT			PUAddPopupW
#else
	#define MS_POPUP_ADDPOPUPT	MS_POPUP_ADDPOPUP
	#define PUAddPopupT			PUAddPopup
#endif


/* Popup/GetContact
Returns the handle to the contact associated to the specified PopupWindow.

wParam = (WPARAM)(HWND)hPopupWindow
lParam = 0;

Returns: the HANDLE of the contact. Can return NULL, meaning it's the main contact. -1 means failure.
*/
#define MS_POPUP_GETCONTACT "Popup/GetContact"
static MCONTACT __inline PUGetContact(HWND hPopupWindow)
{
	return (MCONTACT)CallService(MS_POPUP_GETCONTACT, (WPARAM)hPopupWindow, 0);
}

/* Popup/GetPluginData
Returns custom plugin date associated with popup

wParam = (WPARAM)(HWND)hPopupWindow
lParam = (LPARAM)(PLUGINDATA*)PluginDataAddress;

Returns: the address of the PLUGINDATA structure. Can return NULL, meaning nothing was given. -1 means failure.

IMPORTANT NOTE: it doesn't seem to work if you do:
CallService(..., (LPARAM)aPointerToAStruct);
and then use that struct.
Do this, instead:
aPointerToStruct = CallService(..., (LPARAM)aPointerToAStruct);
and it will work. Just look at the example I've written above (PopupDlgProc).

*/
#define MS_POPUP_GETPLUGINDATA "Popup/GetPluginData"
static void __inline * PUGetPluginData(HWND hPopupWindow) {
	long * uselessPointer = NULL;
	return (void*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hPopupWindow, (LPARAM)uselessPointer);
}

/* Popup/Query

Requests an action or an answer from Popup module.

wParam = (WPARAM)wpQuery

returns 0 on success, -1 on error, 1 on stupid calls ;-)
*/

#define PUQS_ENABLEPOPUPS	1	// returns 0 if state was changed, 1 if state wasn't changed
#define PUQS_DISABLEPOPUPS	2	// " "
#define PUQS_GETSTATUS		3	//Returns 1 (TRUE) if popups are enabled, 0 (FALSE) if popups are disabled.

#define MS_POPUP_QUERY "Popup/Query"

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
static int __inline PUDeletePopup(HWND hWndPopup) {
	return (int)SendMessage(hWndPopup, UM_DESTROYPOPUP, 0, 0);
}

/* UM_INITPOPUP
This message is sent to the Popup when its creation has been finished, so POPUPDATA (and thus your PluginData) is reachable.
Catch it if you needed to catch WM_CREATE or WM_INITDIALOG, which you'll never ever get in your entire popup-life.
Return value: if you process this message, return 0. If you don't process it, return 0. Do whatever you like ;-)

wParam = (WPARAM)(HWND)hPopupWindow (this is useless, you get message inside your popup window)
lParam = 0
*/
#define UM_INITPOPUP             (WM_USER + 0x0202)

/* Popup/Changetext
Changes the text displayed in the second line of the popup.

wParam = (WPARAM)(HWND)hPopupWindow
lParam = (LPARAM)(char*)lpzNewText

returns: > 0 for success, -1 for failure, 0 if the failure is due to second line not being shown. (but you could call
PUIsSecondLineShown() before changing the text...)
*/

#define MS_POPUP_CHANGETEXTW "Popup/ChangetextW"
static int __inline PUChangeTextW(HWND hWndPopup, LPCWSTR lpwzNewText) {
	return (int)CallService(MS_POPUP_CHANGETEXTW, (WPARAM)hWndPopup, (LPARAM)lpwzNewText);
}

#if defined(_UNICODE) || defined(UNICODE)
	#define MS_POPUP_CHANGETEXTT	MS_POPUP_CHANGETEXTW
	#define PUChangeTextT			PUChangeTextW
#endif

/* Popup/Change
Changes the entire popup

wParam = (WPARAM)(HWND)hPopupWindow
lParam = (LPARAM)(POPUPDATA*)newData
*/

#define MS_POPUP_CHANGEW "Popup/ChangeW"
static int __inline PUChangeW(HWND hWndPopup, POPUPDATAW *newData) {
	return (int)CallService(MS_POPUP_CHANGEW, (WPARAM)hWndPopup, (LPARAM)newData);
}

static int __inline PUChangeW(HWND hWndPopup, POPUPDATAW_V2 *newData) {
	return (int)CallService(MS_POPUP_CHANGEW, (WPARAM)hWndPopup, (LPARAM)newData);
}

#if defined(_UNICODE) || defined(UNICODE)
	#define MS_POPUP_CHANGET MS_POPUP_CHANGEW
	#define PUChangeT        PUChangeW
#endif

/* UM_CHANGEPOPUP
This message is triggered by Change/ChangeText services. You also may post it directly :)

wParam = Modification type
lParam = value of type defined by wParam
*/

#define CPT_TEXTW	 2 // lParam = (WCHAR *)text
#define CPT_TITLEW 4 // lParam = (WCHAR *)title
#define CPT_DATAW	 7 // lParam = (POPUPDATAW *)data
#define CPT_DATA2	8 // lParam = (POPUPDATA2 *)data -- see m_popup2.h for details

#define UM_CHANGEPOPUP			(WM_USER + 0x0203)

#if defined(_UNICODE) || defined(UNICODE)
	#define CPT_TEXTT  CPT_TEXTW
	#define CPT_TITLET CPT_TITLEW
	#define CPT_DATAT  CPT_DATAW
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
static int __inline PUModifyActionIcon(HWND hWndPopup, WPARAM wParam, LPARAM lParam, HICON hIcon) {
	POPUPACTIONID actionId = { wParam, lParam };
	return (int)SendMessage(hWndPopup, UM_POPUPMODIFYACTIONICON, (WPARAM)&actionId, (LPARAM)hIcon);
}

/* UM_POPUPSHOW
Show popup at position

wParam = x
lParam = y
*/
#define UM_POPUPSHOW			 (WM_USER + 0x0206)

/* Popup/RegisterActions
Registers your action in popup action list

wParam = (WPARAM)(LPPOPUPACTION)actions
lParam = (LPARAM)actionCount

Returns: 0 if the popup was shown, -1 in case of failure.
*/
#define MS_POPUP_REGISTERACTIONS "Popup/RegisterActions"

static int __inline PURegisterActions(LPPOPUPACTION actions, int count) {
	return (int)CallService(MS_POPUP_REGISTERACTIONS, (WPARAM)actions, (LPARAM)count);
}

/* Popup/RegisterNotification
Registers your action in popup action list

wParam = (WPARAM)(LPPOPUPNOTIFICATION)info
lParam = 0

Returns: handle of registered notification or sero on failure
*/
#define MS_POPUP_REGISTERNOTIFICATION "Popup/RegisterNotification"

#define PNAF_CALLBACK			0x01

#define POPUP_ACTION_NOTHING	LPGEN("Do nothing")
#define POPUP_ACTION_DISMISS	LPGEN("Dismiss popup")

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
			void(*pfnCallback)(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, DWORD cookie);
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
	return (HANDLE)CallService(MS_POPUP_REGISTERNOTIFICATION, (WPARAM)notification, 0);
}

/* Popup/UnhookEventAsync
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

#define MS_POPUP_UNHOOKEVENTASYNC "Popup/UnhookEventAsync"

/* UM_POPUPUNHOOKCOMPLETE
Modify Popup Action Icon

wParam = 0
lParam = (LPARAM)(HANDLE)hEventUnhooked
*/
#define UM_POPUPUNHOOKCOMPLETE           (WM_USER + 0x0206)

static int __inline PUUnhookEventAsync(HWND hwndPopup, HANDLE hEvent) {
	if (ServiceExists(MS_POPUP_UNHOOKEVENTASYNC))
		return (int)CallService(MS_POPUP_UNHOOKEVENTASYNC, (WPARAM)hwndPopup, (LPARAM)hEvent);

	// old popup plugins: unhook service not found
	UnhookEvent(hEvent);
	PostMessage(hwndPopup, UM_POPUPUNHOOKCOMPLETE, 0, (LPARAM)hEvent);
	return 0;
}

/* Popup/GetStatus
Returns 1 when popups are showen and 0 when not
wParam = 0
lParam = 0
*/
#define MS_POPUP_GETSTATUS	"Popup/GetStatus"

#ifdef __cplusplus
/* Popup/RegisterVfx
Register new animation (fade in/out) effect
wParam = 0
lParam = (LPARAM)(char *)vfx_name
*/

#define MS_POPUP_REGISTERVFX	"Popup/RegisterVfx"

/* Popup/Vfx/<vfx_name>
Define this service to create vfx instance
wParam = 0
lParam = 0
return = (int)(IPopupPlusEffect *)vfx
*/

#define MS_POPUP_CREATEVFX		"Popup/Vfx/"

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


/* Popup/ShowMessage
This is mainly for developers.
Shows a warning message in a Popup. It's useful if you need a "MessageBox" like function, but you don't want a modal
window (which will interfere with a DialogProcedure. MessageBox steals focus and control, this one not.

wParam = (char *)lpzMessage
lParam = 0;

Returns: 0 if the popup was shown, -1 in case of failure.
*/
#define SM_WARNING	0x01	//Triangle icon.
#define SM_NOTIFY	0x02	//Exclamation mark icon.
#define SM_ERROR	0x03	//Cross icon.
#define MS_POPUP_SHOWMESSAGE "Popup/ShowMessage"
#define MS_POPUP_SHOWMESSAGEW "Popup/ShowMessageW"

static int __inline PUShowMessage(char *lpzText, DWORD kind) {
	return (int)CallService(MS_POPUP_SHOWMESSAGE, (WPARAM)lpzText, (LPARAM)kind);
}

static int __inline PUShowMessageW(wchar_t *lpwzText, DWORD kind) {
	return (int)CallService(MS_POPUP_SHOWMESSAGEW, (WPARAM)lpwzText, (LPARAM)kind);
}

#ifdef _UNICODE
#define PUShowMessageT	PUShowMessageW
#else
#define PUShowMessageT	PUShowMessage
#endif

/* Popup/Filter
Filters popups out

wParam = (HANDLE)hContact
lParam = (void*)pWindowProc;

returns: 0 = popup allowed, 1 = popup filtered out
*/

#define ME_POPUP_FILTER "Popup/Filter"

//------------- Class API ----------------//

typedef struct {
	int cbSize;
	int flags;
	char *pszName;
	union {
		char *pszDescription;
		wchar_t *pwszDescription;
		TCHAR *ptszDescription;
	};

	HICON hIcon;

	COLORREF colorBack;
	COLORREF colorText;

	WNDPROC PluginWindowProc;

	int iSeconds;
	LPARAM lParam; //APF_RETURN_HWND, APF_CUSTOM_POPUP  ... as above
} POPUPCLASS;

#define PCF_UNICODE			0x0001

#ifdef _UNICODE
#define PCF_TCHAR			PCF_UNICODE
#else
#define PCF_TCHAR			0
#endif

// wParam = 0
// lParam = (POPUPCLASS *)&pc
#define MS_POPUP_REGISTERCLASS   "Popup/RegisterClass"

HANDLE __forceinline Popup_RegisterClass(POPUPCLASS *pc)
{
	if (!ServiceExists(MS_POPUP_REGISTERCLASS))
		return NULL;
	return (HANDLE)CallService(MS_POPUP_REGISTERCLASS, 0, LPARAM(pc));
}

#define MS_POPUP_UNREGISTERCLASS "Popup/UnregisterClass"

void __forceinline Popup_UnregisterClass(HANDLE ppc)
{
	if (ppc)
		CallService(MS_POPUP_UNREGISTERCLASS, 0, LPARAM(ppc));
}

typedef struct {
	int cbSize;
	char *pszClassName;
	union {
		const char *pszTitle;
		const wchar_t *pwszTitle;
		const TCHAR *ptszTitle;
	};
	union {
		const char *pszText;
		const wchar_t *pwszText;
		const TCHAR *ptszText;
	};
	void *PluginData;
	MCONTACT hContact;
} POPUPDATACLASS;

// wParam = 0
// lParam = (POPUPDATACLASS *)&pdc
#define MS_POPUP_ADDPOPUPCLASS	"Popup/AddPopupClass"

static INT_PTR __inline ShowClassPopup(char *name, char *title, char *text) {
	POPUPDATACLASS d = {sizeof(d), name};
	d.pszTitle = title;
	d.pszText = text;
	return CallService(MS_POPUP_ADDPOPUPCLASS, 0, (LPARAM)&d);
}

static INT_PTR __inline ShowClassPopupW(char *name, wchar_t *title, wchar_t *text) {
	POPUPDATACLASS d = {sizeof(d), name};
	d.pwszTitle = title;
	d.pwszText = text;
	return CallService(MS_POPUP_ADDPOPUPCLASS, 0, (LPARAM)&d);
}

#ifdef _UNICODE
#define ShowClassPopupT		ShowClassPopupW
#else
#define ShowClassPopupT		ShowClassPopup
#endif

#endif // __m_popup_h__
