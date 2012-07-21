/*
===============================================================================
                                PopUp plugin
Plugin Name: PopUp
Plugin authors: Luca Santarelli aka hrk (hrk@users.sourceforge.net)
                Victor Pavlychko aka zazoo (zazoo@ua.fm)
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

//#define MAX_CONTACTNAME 32
//#define MAX_SECONDLINE 40
#define MAX_CONTACTNAME 2048
#define MAX_SECONDLINE 2048

#define POPUP_USE_SKINNED_BG 0xffffffff

//This is the basic data you'll need to fill and pass to the service function.
typedef struct  {
	HANDLE lchContact;                    //Handle to the contact, can be NULL (main contact).
	HICON lchIcon;                        //Handle to a icon to be shown. Cannot be NULL.
	char lpzContactName[MAX_CONTACTNAME]; //This is the contact name or the first line in the plugin. Cannot be NULL.
	char lpzText[MAX_SECONDLINE];         //This is the second line text. Users can choose to hide it. Cannot be NULL.
	COLORREF colorBack;                   //COLORREF to be used for the background. Can be NULL, default will be used.
	COLORREF colorText;                   //COLORREF to be used for the text. Can be NULL, default will be used.
	WNDPROC PluginWindowProc;             //Read below. Can be NULL; default will be used.
	void * PluginData;                    //Read below. Can be NULL.
} POPUPDATA, * LPPOPUPDATA;

typedef struct {
	HANDLE lchContact;
	HICON lchIcon;
	char lpzContactName[MAX_CONTACTNAME];
	char lpzText[MAX_SECONDLINE];
	COLORREF colorBack;                   //Set background to POPUP_USE_SKINNED_BG to turn on skinning
	COLORREF colorText;
	WNDPROC PluginWindowProc;
	void * PluginData;
	int iSeconds;                         //Custom delay time in seconds. -1 means "forever", 0 means "default time".
	LPCTSTR lpzClass;                     //PopUp class. Used with skinning. See PopUp/AddClass for details
	COLORREF skinBack;                    //Background color for colorizable skins
	char cZero[16 - sizeof(LPCTSTR) - sizeof(COLORREF)];
	                                      //some unused bytes which may come useful in the future.
} POPUPDATAEX, *LPPOPUPDATAEX;

/*
When you call MS_POPUP_ADDPOPUP, my plugin will check if the given POPUPDATA structure is filled with acceptable values. If not, the data will be rejected and no popup will be shown.

- lpzText should be given, because it's really bad if a user chooses to have the second line displayed
and it's empty :-) Just write it and let the user choose if it will be displayed or not.

- PluginWindowProc is a WNDPROC address you have to give me. Why? What? Where? Calm down 8)
My plugin will take care of the creation of the popup, of the destruction of the popup, of the come into
view and the hiding of the popup. Transparency, animations... all this stuff.
My plugin will not (as example) open the MessageWindow when you left click on a popup.
Why? Because I don't know if your popup desires to open the MessageWindow :))))
This means that you need to make a WNDPROC which takes care of the WM_messages you need.
For example, WM_COMMAND or WM_CONTEXTMENU or WM_LMOUSEUP or whatever.
At the end of your WNDPROC remember to "return DefWindowProc(hwnd, msg, wParam, lParam);"
When you process a message that needs a return value (an example could be WM_CTLCOLORSTATIC,
but you don't need to catch it 'cause it's my plugin's job), simply return the nedeed value. :)
The default WNDPROC does nothing.

- PluginData is a pointer to a void, which means a pointer to anything. You can make your own structure
to store the data you need (example: a status information, a date, your name, whatever) and give me a
pointer to that struct.
You will need to destroy that structure and free the memory when the PopUp is going to be destroyed. You'll know this when you receive a UM_FREEPLUGINDATA. The name tells it all: free your own plugin data.

Appendix A: Messages my plugin will handle and your WNDPROC will never see.
WM_CREATE, WM_DESTROY, WM_TIMER, WM_ERASEBKGND
WM_CTLCOLOR* [whatever it may be: WM_CTLCOLORDLG, WM_CTLCOLORSTATIC...]
WM_PAINT, WM_PRINT, WM_PRINTCLIENT

Appendix B: "What do I need to do?!?".
Here is an example in C.

//Your plugin is in /plugins/myPlugin/ or in miranda32/something/
#include "../../plugins/PopUp/m_popup.h"

Define your own plugin data if you need it. In this example, we need it and we'll use NewStatusNotify as example: thsi plugin shows a popup when someone in your contact list changes his/hers status. We'll need to know his status, both current and old one.
typedef struct {
	WORD oldStatus;
	WORD newStatus;
} MY_PLUGIN_DATA;

When we need to show the popup, we do:
{
	POPUPDATA ppd;
	hContact = A_VALID_HANDLE_YOU_GOT_FROM_SOMEWHERE;
	hIcon = A_VALID_HANDLE_YOU_GOT_SOMEWHERE;
	char * lpzContactName = (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)lhContact,0);
	//99% of the times you'll just copy this line.
	//1% of the times you may wish to change the contact's name. I don't know why you should, but you can.
	char * lpzText;
	//The text for the second line. You could even make something like: char lpzText[128]; lstrcpy(lpzText, "Hello world!"); It's your choice.
	COLORREF colorBack = GetSysColor(COLOR_BTNFACE); //The colour of Miranda's option Pages (and many other windows...)
	COLORREF colorText = RGB(255,255,255); //White.
	MY_PLUGIN_DATA * mpd = (MY_PLUGIN_DATA*)malloc(sizeof(MY_PLUGIN_DATA));

	ZeroMemory(ppd, sizeof(ppd)); //This is always a good thing to do.
	ppd.lchContact = (HANDLE)hContact; //Be sure to use a GOOD handle, since this will not be checked.
	ppd.lchIcon = hIcon;
	lstrcpy(ppd.lpzContactName, lpzContactName);
	lstrcpy(ppd.lpzText, lpzText);
	ppd.colorBack = colorBack;
	ppd.colorText = colorText;
	ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;

	//Now the "additional" data.
	mpd->oldStatus = ID_STATUS_OFFLINE;
	mpd->newStatus = ID_STATUS_ONLINE;

	//Now that the plugin data has been filled, we add it to the PopUpData.
	ppd.PluginData = mpd;

	//Now that every field has been filled, we want to see the popup.
	CallService(MS_POPUP_ADDPOPUP, (WPARAM)&ppd, 0);
}

Obviously, you have previously declared some:
static int CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
		case WM_COMMAND:
			if ((HIWORD)wParam == STN_CLICKED) { //It was a click on the Popup.
				PUDeletePopUp(hWnd);
				return TRUE;
			}
			break;
		case UM_FREEPLUGINDATA: {
			MY_PLUGIN_DATA * mpd = NULL;
			mpd = (MY_PLUGIN_DATA*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd,(LPARAM)mpd);
			if (mdp > 0) free(mpd);
			return TRUE; //TRUE or FALSE is the same, it gets ignored.
		}
		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
*/

/*
Creates, adds and shows a popup, given a (valid) POPUPDATA structure pointer.
wParam = (WPARAM)(*POPUPDATA)PopUpDataAddress
lParam = 0
Returns: > 0 on success, 0 if creation went bad, -1 if the PopUpData contained unacceptable values.
NOTE: it returns -1 if the PopUpData was not valid, if there were already too many popups, if the module was disabled.
Otherwise, it can return anything else...
*/
#define MS_POPUP_ADDPOPUP "PopUp/AddPopUp"
static int __inline PUAddPopUp(POPUPDATA* ppdp) {
	return CallService(MS_POPUP_ADDPOPUP, (WPARAM)ppdp,0);
}

#define MS_POPUP_ADDPOPUPEX "PopUp/AddPopUpEx"
static int __inline PUAddPopUpEx(POPUPDATAEX* ppdp) {
	return CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)ppdp,0);
}

/*
Returns the handle to the contact associated to the specified PopUpWindow.
You will probably need to know this handle inside your WNDPROC. Exampole: you want to open the MessageWindow. :-)
Call MS_POPUP_GETCONTACT on the hWnd you were given in the WNDPROC.
wParam = (WPARAM)(HWND)hPopUpWindow
lParam = 0;
Returns: the HANDLE of the contact. Can return NULL, meaning it's the main contact. -1 means failure.
*/
#define MS_POPUP_GETCONTACT "PopUp/GetContact"
static HANDLE __inline PUGetContact(HWND hPopUpWindow) {
	return (HANDLE)CallService(MS_POPUP_GETCONTACT, (WPARAM)hPopUpWindow,0);
}

/*
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

/*
wParam = 0
lParam = 0
Returns: 0 if the user has chosen not to have the second line, 1 if he choose to have the second line.
*/
#define MS_POPUP_ISSECONDLINESHOWN "PopUp/IsSecondLineShown"
static BOOL __inline PUIsSecondLineShown() {
	return (BOOL)CallService(MS_POPUP_ISSECONDLINESHOWN,0,0);
}

/*
Requests an action or an answer from PopUp module.
wParam = (WPARAM)wpQuery
returns 0 on success, -1 on error, 1 on stupid calls ;-)
*/
#define PUQS_ENABLEPOPUPS 1 //returns 0 if state was changed, 1 if state wasn't changed
#define PUQS_DISABLEPOPUPS 2 // " "
#define PUQS_GETSTATUS 3 //Returns 1 (TRUE) if popups are enabled, 0 (FALSE) if popups are disabled.

#define MS_POPUP_QUERY "PopUp/Query"

/*
UM_FREEPLUGINDATA
wParam = lParam = 0. Process this message if you have allocated your own memory. (i.e.: POPUPDATA.PluginData != NULL)
*/
#define UM_FREEPLUGINDATA        (WM_USER + 0x0200)

/*
UM_DESTROYPOPUP
wParam = lParam = 0. Send this message when you want to destroy the popup, or use the function below.
*/
#define UM_DESTROYPOPUP          (WM_USER + 0x0201)
static int __inline PUDeletePopUp(HWND hWndPopUp) {
	return (int)SendMessage(hWndPopUp, UM_DESTROYPOPUP,0,0);
}

/*
UM_INITPOPUP
wParam = (WPARAM)(HWND)hPopUpWindow (but this is useless, since I'll directly send it to your hPopUpWindow
lParam = 0.
This message is sent to the PopUp when its creation has been finished, so POPUPDATA (and thus your PluginData) is reachable.
Catch it if you needed to catch WM_CREATE or WM_INITDIALOG, which you'll never ever get in your entire popup-life.
Return value: if you process this message, return 0. If you don't process it, return 0. Do whatever you like ;-)
*/
#define UM_INITPOPUP             (WM_USER + 0x0202)

/*
wParam = (WPARAM)(HWND)hPopUpWindow
lParam = (LPARAM)(char*)lpzNewText
returns: > 0 for success, -1 for failure, 0 if the failure is due to second line not being shown. (but you could call PUIsSecondLineShown() before changing the text...)
Changes the text displayed in the second line of the popup.
*/
#define MS_POPUP_CHANGETEXT "PopUp/Changetext"
static int __inline PUChangeText(HWND hWndPopUp, LPCTSTR lpzNewText) {
	return (int)CallService(MS_POPUP_CHANGETEXT, (WPARAM)hWndPopUp, (LPARAM)lpzNewText);
}

/*
This is mainly for developers.
Shows a warning message in a PopUp. It's useful if you need a "MessageBox" like function, but you don't want a modal window (which will interfere with a DialogProcedure. MessageBox steals focus and control, this one not.
wParam = (char*) lpzMessage
lParam = 0;
Returns: 0 if the popup was shown, -1 in case of failure.
*/
#define SM_WARNING         0x01 //Triangle icon.
#define SM_NOTIFY     0x02 //Exclamation mark icon.
#define MS_POPUP_SHOWMESSAGE "PopUp/ShowMessage"

static int __inline PUShowMessage(char* lpzText, BYTE kind) {
	return (int)CallService(MS_POPUP_SHOWMESSAGE, (WPARAM)lpzText,(LPARAM)kind);
}

/*
Each skinned popup (e.g. with colorBack == POPUP_USE_SKINNED_BG) should have
class set. Then you can choose separate skin for each class (for example, you
can create separate class for your plugin and use it for all ypu popups. User
would became able to choose skin for your popups independently from others)

You have to register popup class before using it. To do so call "PopUp/AddClass"
with lParam = (LPARAM)(const char *)popUpClassName.

All class names are translated (via Translate()) before being added to list. You
should use english names for them.

There are three predefined classes and one for backward compatability.

Note that you can add clases after popup wal loaded, e.g. you shoul intercept
ME_SYSTEM_MODULESLOADED event
*/
#define MS_POPUP_ADDCLASS "PopUp/AddClass"
#define POPUP_CLASS_DEFAULT "Default"
#define POPUP_CLASS_WARNING "Warning"
#define POPUP_CLASS_NOTIFY  "Notify"
#define POPUP_CLASS_OLDAPI  "PopUp 1.0.1.x compatability" // for internal purposes

static void __inline PUAddClass(const char *lpzClass){
	CallService(MS_POPUP_ADDCLASS, 0, (LPARAM)lpzClass);
}

#endif
