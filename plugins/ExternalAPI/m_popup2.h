/*
===============================================================================
                                Popup plugin
Plugin Name: Popup
Plugin authors: Luca Santarelli aka hrk (hrk@users.sourceforge.net)
                Victor Pavlychko (nullbie@gmail.com)
===============================================================================
The purpose of this plugin is to give developers a common "platform/interface"
to show Popups. It is born from the source code of NewStatusNotify, another
plugin I've made.

Remember that users *must* have this plugin enabled, or they won't get any
popup. Write this in the requirements, do whatever you wish ;-)... but tell
them!
===============================================================================
*/

#ifndef __m_popup2_h__
#define __m_popup2_h__

#ifndef POPUP_VERSION
#define POPUP_VERSION 0x02010003
#endif

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

/* Popup/AddPopup
Creates, adds and shows a popup, given a (valid) POPUPDATA structure pointer.

wParam = (WPARAM)(*POPUPDATA)PopupDataAddress
lParam = 0

Returns: > 0 on success, 0 if creation went bad, -1 if the PopupData contained unacceptable values.
NOTE: it returns -1 if the PopupData was not valid, if there were already too many popups, if the module was disabled.
Otherwise, it can return anything else...

Popup Plus 2.0.4.0+
You may pass additional creation flags via lParam:
/* core define see miranda\include\m_popup.h
        APF_RETURN_HWND ....... function returns handle to newly created popup window (however this calls are a bit slower)
        APF_CUSTOM_POPUP ...... new popup is created in hidden state and doesn't obey to popup queue rules.
                                you may control it via UM_* messages and custom window procedure (not yet implemented)
additional APF_ flags */
#define APF_NO_HISTORY   0x04	//do not log this popup in popup history (useful for previews)
#define APF_NO_POPUP     0x08	//do not show popup. this is useful if you want popup yo be stored in history only
#define APF_NEWDATA      0x10	//deprecatet!! only for use with old POPUPDATAEX_V2/POPUPDATAW_V2 structs

static INT_PTR __inline PUAddPopupW(POPUPDATAW_V2* ppdp) {
	return CallService(MS_POPUP_ADDPOPUPW, (WPARAM)ppdp,0);
}

#define MS_POPUP_CHANGEW "Popup/ChangeW"
static int __inline PUChangeW(HWND hWndPopup, POPUPDATAW_V2 *newData) {
	return (int)CallService(MS_POPUP_CHANGEW, (WPARAM)hWndPopup, (LPARAM)newData);
}

/* UM_CHANGEPOPUP
This message is triggered by Change/ChangeText services. You also may post it directly, but make
sure you allocate memory via mir_alloc, because popup will mir_free() them!

wParam = Modification type
lParam = value of type defined by wParam

/* core define see miranda\include\m_popup.h
#define CPT_TEXTW	2 // lParam = (WCHAR *)text
#define CPT_TITLEW	4 // lParam = (WCHAR *)title
#define CPT_DATA	5 // lParam = (POPUPDATA *)data
#define CPT_DATAEX	6 // lParam = (POPUPDATAEX *) or (POPUPDATAEX_V2 *)data see CPT_DATA2
#define CPT_DATAW	7 // lParam = (POPUPDATAW  *) or (POPUPDATAW_V2  *)data see CPT_DATA2
additional CPT_ flag*/
#define CPT_DATA2	8 // lParam = (POPUPDATA2 *)data -- see m_popup2.h for details

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
	return (int)CallService(MS_POPUP_REGISTERACTIONS, (WPARAM)actions,(LPARAM)count);
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
		return (int)CallService(MS_POPUP_UNHOOKEVENTASYNC, (WPARAM)hwndPopup,(LPARAM)hEvent);

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

/* core define see miranda\include\m_popup.h
#define SM_WARNING	0x01	//Triangle icon.
#define SM_NOTIFY	0x02	//Exclamation mark icon.
additional SM_ flags */
#define SM_ERROR	0x03	//Cross icon.
#ifndef MS_POPUP_SHOWMESSAGE
#define MS_POPUP_SHOWMESSAGE "Popup/ShowMessage"
#define MS_POPUP_SHOWMESSAGEW "Popup/ShowMessageW"
#endif

#endif // __m_popup2_h__
