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

EXTERN_C MIR_APP_DLL(void) Popup_Enable(bool bEnable);
EXTERN_C MIR_APP_DLL(bool) Popup_Enabled();

/*
NOTE! Since Popup 1.0.1.2 there is a main menu group called "Popups" where I
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
struct POPUPACTION
{
	int cbSize;		// sizeof(POPUPACTION)
	HICON lchIcon;	// Action Icon
	// Action title text. Please use module name as prefix
	// (e.g. "Popup Plus/Dismiss Popup") and don't translate
	// This is translates by popup. So no unicode.
	char lpzTitle[MAX_ACTIONTITLE];
	uint32_t flags;	// set of PAF_* flags
	WPARAM wParam;	// wParam for UM_POPUPACTION message
	LPARAM lParam;	// lParam for UM_POPUPACTION message
};

///////////////////////////////////////////////////////////////
// Few notes about new popup api
// ------------------------------
// When you call any ADD service, Popup Plus creates local
// copy of POPUPDATA2 to store the data. Each time you call
// CHANGE service this data is updated. You can use the
// MS_POPUP_GETDATA2 service to retrieve Popups's copy of
// this data, however you MUST NOT chahge that.

// unicode or ansi mode
#define PU2_ANSI         0x00
#define PU2_UNICODE      0x01
#define PU2_CUSTOM_POPUP 0x02

struct POPUPDATA2
{
	// general
	int cbSize;
	uint32_t flags;

	// miranda bindings
	MCONTACT lchContact;
	HANDLE lchEvent;

	// style
	COLORREF colorBack;
	COLORREF colorText;
	HICON lchIcon;
	HBITMAP hbmAvatar;
	MAllStrings szTitle;
	MAllStrings szText;

	char *lpzSkin;

	// time and timeout
	int iSeconds;
	uint32_t dwTimestamp;

	#ifdef _WINDOWS
		// plugin bindings
		WNDPROC PluginWindowProc;
		void *PluginData;
	#endif

	// popup actions
	int actionCount;
	POPUPACTION *lpActions;

	HANDLE lchNotification;
};

#define APF_RETURN_HWND  0x1
#define APF_CUSTOM_POPUP 0x2
#define APF_NO_HISTORY   0x04	// do not log this popup in popup history (useful for previews)
#define APF_NO_POPUP     0x08	// do not show popup. this is useful if you want popup yo be stored in history only

EXTERN_C MIR_APP_DLL(HWND) Popup_Add(const POPUPDATA2 *pData, int flags = 0 /* combination of APF_* */);
EXTERN_C MIR_APP_DLL(void) Popup_Change(HWND hwndPopup, const POPUPDATA2 *pData);

// Creates, adds and shows a popup, given a (valid) POPUPDATA structure pointer.
// Returns: > 0 on success, 0 if creation went bad, INVALID_HANDLE if the PopupData contained unacceptable values.
// NOTE: it returns -1 if the PopupData was not valid, if there were already too many popups, if the module was disabled.

struct POPUPDATA
{
	__forceinline POPUPDATA()
	{	memset(this, 0, sizeof(*this));
	}

	MCONTACT lchContact;
	HICON lchIcon;
	char lpzContactName[MAX_CONTACTNAME];
	char lpzText[MAX_SECONDLINE];
	COLORREF colorBack;
	COLORREF colorText;
	#ifdef _WINDOWS
		WNDPROC PluginWindowProc;
		void *PluginData;
	#endif
	int iSeconds;       // Custom delay time in seconds. -1 means "forever", 0 means "default time".
};

struct POPUPDATAW
{
	__forceinline POPUPDATAW()
	{	memset(this, 0, sizeof(*this));
	}

	MCONTACT lchContact;
	HICON lchIcon;
	wchar_t lpwzContactName[MAX_CONTACTNAME];
	wchar_t lpwzText[MAX_SECONDLINE];
	COLORREF colorBack;
	COLORREF colorText;
	#ifdef _WINDOWS
		WNDPROC PluginWindowProc;
		void *PluginData;
	#endif
	int iSeconds;      // Custom delay time in seconds. -1 means "forever", 0 means "default time".
	HANDLE hNotification;
	int actionCount;
	POPUPACTION *lpActions;
};

EXTERN_C MIR_APP_DLL(HWND) PUAddPopup(POPUPDATA *ppdp, int flags = 0 /* combination of APF_* */);
EXTERN_C MIR_APP_DLL(HWND) PUAddPopupW(POPUPDATAW *ppdp, int flags = 0 /* combination of APF_* */);

// Deletes a popup window
EXTERN_C MIR_APP_DLL(int) PUDeletePopup(HWND hWndPopup);

// Returns the handle to the contact associated to the specified PopupWindow or -1 on failure
EXTERN_C MIR_APP_DLL(MCONTACT) PUGetContact(HWND hPopupWindow);

// Returns custom plugin date associated with popup
EXTERN_C MIR_APP_DLL(void*) PUGetPluginData(HWND hPopupWindow);

// Changes the entire popup
EXTERN_C MIR_APP_DLL(int) PUChangeW(HWND hWndPopup, POPUPDATAW *newData);

// Changes the text displayed in the second line of the popup.
EXTERN_C MIR_APP_DLL(int) PUChangeTextW(HWND hWndPopup, const wchar_t *lpwzNewText);

/////////////////////////////////////////////////////////////////////////////////////////
// Popup messages

// UM_FREEPLUGINDATA
// Process this message if you have allocated your own memory. (i.e.: POPUPDATA.PluginData != NULL)
#define UM_FREEPLUGINDATA        (WM_USER + 0x0200)

// UM_DESTROYPOPUP
// Send this message when you want to destroy the popup, or use the function below.
// wParam = 0
// lParam = HWND
#define UM_DESTROYPOPUP          (WM_USER + 0x0201)

// UM_INITPOPUP
// This message is sent to the Popup when its creation has been finished, so POPUPDATA (and thus your PluginData) is reachable.
// Catch it if you needed to catch WM_CREATE or WM_INITDIALOG, which you'll never ever get in your entire popup-life.
// Return value: if you process this message, return 0. If you don't process it, return 0. Do whatever you like ;-)
// wParam = (WPARAM)(HWND)hPopupWindow (this is useless, you get message inside your popup window)
// lParam = 0
#define UM_INITPOPUP             (WM_USER + 0x0202)

// UM_CHANGEPOPUP
// This message is triggered by Change/ChangeText services. You also may post it directly :)
// wParam = Modification type
// lParam = value of type defined by wParam

#define CPT_TEXTW	 2 // lParam = (wchar_t *)text
#define CPT_TITLEW 4 // lParam = (wchar_t *)title
#define CPT_DATAW	 7 // lParam = (POPUPDATAW *)data
#define CPT_DATA2	8 // lParam = (POPUPDATA2 *)data -- see m_popup2.h for details

#define UM_CHANGEPOPUP			(WM_USER + 0x0203)

// UM_POPUPACTION
// Popup Action notification
// wParam and lParam are specified bu plugin.
// wParam = 0 is used buy popup plus internally!
#define UM_POPUPACTION           (WM_USER + 0x0204)

// UM_POPUPMODIFYACTIONICON
// Modify Popup Action Icon
// wParam = (WPARAM)(LPPOPUPACTIONID)&actionId
// lParam = (LPARAM)(HICON)hIcon

struct POPUPACTIONID
{
	WPARAM wParam;
	LPARAM lParam;
};

#ifdef _WINDOWS
#define UM_POPUPMODIFYACTIONICON  (WM_USER + 0x0205)
__forceinline int PUModifyActionIcon(HWND hWndPopup, WPARAM wParam, LPARAM lParam, HICON hIcon)
{
	POPUPACTIONID actionId = { wParam, lParam };
	return (int)SendMessage(hWndPopup, UM_POPUPMODIFYACTIONICON, (WPARAM)&actionId, (LPARAM)hIcon);
}
#endif

// UM_POPUPSHOW
// Show popup at position
// wParam = x
// lParam = y
#define UM_POPUPSHOW			 (WM_USER + 0x0206)

// Registers your action in popup action list

EXTERN_C MIR_APP_DLL(int) PURegisterActions(POPUPACTION *actions, int count);

// Popup/RegisterNotification
// Registers your action in popup action list
// wParam = (WPARAM)(LPPOPUPNOTIFICATION)info
// lParam = 0
// Returns: handle of registered notification or sero on failure

#define PNAF_CALLBACK			0x01

#define POPUP_ACTION_NOTHING	LPGEN("Do nothing")
#define POPUP_ACTION_DISMISS	LPGEN("Dismiss popup")

struct POPUPNOTIFYACTION
{
	char lpzTitle[64];
	uint32_t dwFlags;
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
			uint32_t dwCookie;
			void(*pfnCallback)(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, uint32_t cookie);
		};
	};
};

#define PNF_CONTACT				0x01

struct POPUPNOTIFICATION
{
	int cbSize;
	uint32_t dwFlags;			// set of PNF_* flags
	char lpzGroup[MAXMODULELABELLENGTH];
	char lpzName[MAXMODULELABELLENGTH];
	HANDLE lchIcoLib;			// gotten from icolib
	COLORREF colorBack;		// this will be registered in fontservice
	COLORREF colorText;		// this will be registered in fontservice
	int iSeconds;			// default timeout
	int actionCount;		// for unified action comboboxes
	POPUPNOTIFYACTION *lpActions;
	char *lpzLAction;
	char *lpzRAction;
	char *pszReserved1;		// reserved for future use
	#ifdef _WINDOWS
		DLGPROC pfnReserved2;	// reserved for future use
	#endif
};

EXTERN_C MIR_APP_DLL(HANDLE) PURegisterNotification(POPUPNOTIFICATION *notification);

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

#ifdef _WINDOWS
#define UM_POPUPUNHOOKCOMPLETE           (WM_USER + 0x0206)

__forceinline int PUUnhookEventAsync(HWND hwndPopup, HANDLE hEvent)
{
	if (ServiceExists(MS_POPUP_UNHOOKEVENTASYNC))
		return (int)CallService(MS_POPUP_UNHOOKEVENTASYNC, (WPARAM)hwndPopup, (LPARAM)hEvent);

	// old popup plugins: unhook service not found
	UnhookEvent(hEvent);
	PostMessage(hwndPopup, UM_POPUPUNHOOKCOMPLETE, 0, (LPARAM)hEvent);
	return 0;
}
#endif

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

EXTERN_C MIR_APP_DLL(int) PUShowMessage(const char *lpzText, uint32_t kind);
EXTERN_C MIR_APP_DLL(int) PUShowMessageW(const wchar_t *lpwzText, uint32_t kind);

/////////////////////////////////////////////////////////////////////////////////////////
// Popup/Filter
// Filters popups out
// wParam = (MCONTACT)hContact
// lParam = (void*)pWindowProc;
// returns: 0 = popup allowed, 1 = popup filtered out

#define ME_POPUP_FILTER "Popup/Filter"

/////////////////////////////////////////////////////////////////////////////////////////
// Popup classes API

#define PCF_UNICODE  0x0001

struct POPUPCLASS
{
	char *pszName;
	int flags;      // PCF_* constants
	int iSeconds;
	COLORREF colorBack, colorText;
	MAllStrings pszDescription;
	HICON hIcon;
	#ifdef _WINDOWS
		WNDPROC PluginWindowProc;
	#endif
	LPARAM lParam; //APF_RETURN_HWND, APF_CUSTOM_POPUP  ... as above
};

EXTERN_C MIR_APP_DLL(HANDLE) Popup_RegisterClass(POPUPCLASS *pc);
EXTERN_C MIR_APP_DLL(void) Popup_UnregisterClass(HANDLE ppc);

/////////////////////////////////////////////////////////////////////////////////////////
// structure to add a popup using given class

struct POPUPDATACLASS 
{
	MCONTACT hContact;
	const char *pszClassName;
	MAllCStrings szTitle;
	MAllCStrings szText;
	void *PluginData;
};

EXTERN_C MIR_APP_DLL(HWND) Popup_AddClass(POPUPDATACLASS*);

EXTERN_C MIR_APP_DLL(HWND) ShowClassPopup(const char *name, const char *title, const char *text);
EXTERN_C MIR_APP_DLL(HWND) ShowClassPopupW(const char *name, const wchar_t *title, const wchar_t *text);

#endif // __m_popup_h__
