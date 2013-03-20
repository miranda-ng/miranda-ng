/*
===============================================================================
                                PopUp plugin
Plugin Name: PopUp
Plugin authors: Luca Santarelli aka hrk (hrk@users.sourceforge.net)
                Victor Pavlychko aka zazoo (nullbie@gmail.com)
===============================================================================
The purpose of this plugin is to give developers a common "platform/interface"
to show PopUps. It is born from the source code of NewStatusNotify, another
plugin I've made.

Remember that users *must* have this plugin enabled, or they won't get any
popup. Write this in the requirements, do whatever you wish ;-)... but tell
them!
===============================================================================
*/

#ifndef M_POPUP_H
#define M_POPUP_H

/*
NOTE! Since Popup 1.0.1.2 there is a main meun group called "PopUps" where I
have put a "Enable/Disable" item. You can add your own "enable/disable" items
by adding these lines before you call MS_CLIST_ADDMAINMENUITEM:
mi.pszPopUpName = Translate("PopUps");
mi.position = 0; //You don't need it and it's better if you put it to zero.
*/

#define MAX_CONTACTNAME 2048
#define MAX_SECONDLINE  2048

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
	WNDPROC PluginWindowProc;					// Read below. Can be NULL; default will be used.
	void * PluginData;							// Read below. Can be NULL.
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
	char cZero[16];								// Some unused bytes which may come useful in the future.
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
		WCHAR lptzText[MAX_SECONDLINE];
		WCHAR lpwzText[MAX_SECONDLINE];
	};
	COLORREF colorBack;
	COLORREF colorText;
	WNDPROC PluginWindowProc;
	void * PluginData;
	int iSeconds;
	char cZero[16];
} POPUPDATAW, *LPPOPUPDATAW;

#if defined(_UNICODE) || defined(UNICODE)
	typedef POPUPDATAW		POPUPDATAT;
	typedef LPPOPUPDATAW	LPPOPUPDATAT;
#else
	typedef POPUPDATAEX		POPUPDATAT;
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
							you may control it via UM_* messages and custom window procedure
*/
#define APF_RETURN_HWND  0x1
#define APF_CUSTOM_POPUP 0x2

#define MS_POPUP_ADDPOPUP "PopUp/AddPopUp"
static INT_PTR __inline PUAddPopUp(POPUPDATA* ppdp) {
	return CallService(MS_POPUP_ADDPOPUP, (WPARAM)ppdp,0);
}

#define MS_POPUP_ADDPOPUPEX "PopUp/AddPopUpEx"
static INT_PTR __inline PUAddPopUpEx(POPUPDATAEX* ppdp) {
	return CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)ppdp,0);
}

#define MS_POPUP_ADDPOPUPW "PopUp/AddPopUpW"
static INT_PTR __inline PUAddPopUpW(POPUPDATAW* ppdp) {
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
This message is triggered by Change/ChangeText services. You also may post it directly :)

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
#define MS_POPUP_SHOWMESSAGE "PopUp/ShowMessage"
#define MS_POPUP_SHOWMESSAGEW "PopUp/ShowMessageW"

static int __inline PUShowMessage(char *lpzText, BYTE kind) {
	return (int)CallService(MS_POPUP_SHOWMESSAGE, (WPARAM)lpzText,(LPARAM)kind);
}

static int __inline PUShowMessageW(wchar_t *lpwzText, BYTE kind) {
	return (int)CallService(MS_POPUP_SHOWMESSAGEW, (WPARAM)lpwzText,(LPARAM)kind);
}

#ifdef _UNICODE
#define PUShowMessageT	PUShowMessageW
#else
#define PUShowMessageT	PUShowMessage
#endif

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
} POPUPCLASS;

#define PCF_UNICODE			0x0001

#ifdef _UNICODE
#define PCF_TCHAR			PCF_UNICODE
#else
#define PCF_TCHAR			0
#endif

// wParam = 0
// lParam = (POPUPCLASS *)&pc
#define MS_POPUP_REGISTERCLASS	"PopUp/RegisterClass"

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
	HANDLE hContact;
} POPUPDATACLASS;

// wParam = 0
// lParam = (POPUPDATACLASS *)&pdc
#define MS_POPUP_ADDPOPUPCLASS	"PopUp/AddPopupClass"

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
