#ifndef M_TICKER_H
#define M_TICKER_H


/* TICKER_ADD_STRING
wParam = (WPARAM)(*TICKERDATA)TickerDataAddress (see below)
lParam = 0... not used..
returns (int)TickerID on success or 0 if it fails
*/
#define TICKER_ADD_STRING "Ticker/AddString"

/* TICKER_ADD_SIMPLESTRING
wParam = (char*) szMessage // the text to display
lParam = (char*) szTooltip // the Items tooltip (can be NULL)
this service function will use default values for the rest of the TICKERDATA values 
returns (int)TickerID on success or 0 if it fails
*/
#define TICKER_ADD_SIMPLESTRING "Ticker/AddSimpleString"

/* TICKER_ADD_FROMPOPUPPLUGIN
This should only be called from the popup plugin before (or after.. makes no difference)
the popup is created and the user wants to display the ticker
wParam = (WPARAM)(*POPUPDATA)PopUpDataAddress
lParam = 0
returns (int)TickerID on success or 0 if it fails
*/
#define TICKER_ADD_FROMPOPUPPLUGIN "Ticker/AddFromPopupPlugin"


typedef struct  {
	char* szMessage;		// the message you want to display,		will be copied to another mem address by me, so u can free your copy
	char* szTooltip;		// a message to display in the items tooltip, can be NULL  will be copied to another mem address by me, so u can free your copy
	COLORREF crForeground;	// the foreground colour.. can be NULL
	COLORREF crBackground;	// the background colour.. can be NULL
	WNDPROC PluginWindowProc; // for the window proc.. can be NULL  -->- these are more for popup compatability but someone might find it usefull
	HANDLE hContact;		// can be NULL                          -/
	void * PluginData;		// other plugins might want this...    -/
} TICKERDATA, *LPTICKERDATA;











#endif
