#ifndef __mtext_h__
#define __mtext_h__

// NEW mtextcontrol interface:
//
// obtain the full mtextcontrol interface from the library. it is much faster as use of 
// miranda core CallService to access to mtextcontrol (no core traffic).
// This interface provides full access to mtextcontrol internal functions,
// thus enabling devs to fully utilize the mtextcontrol API.
// All functions will be exported as miranda services for compatibility.
//
// the interface is populated during the Load(PLUGINLINK *link) handler, so you can assume it is ready when Miranda
// throw the ME_SYSTEM_MODULESLOADED event and you can generate a warning in your ModulesLoaded() when
// it depends on the mtextcontrol interface and the mtextcontrol plugin is missing.
// 
// example:
// 
// MTEXT_INTERFACE MText = {0};
// 
// 	mir_getMTI(&MText);
// 
// all interface function designed as old mtextcontrol helper functions.
// therefore it is easy to convert your old plugin code to new interface.
//
// example:
//
// old code: MTextCreate (...
// new code: MText.Create(...

// Text control
#define MTEXTCONTROLCLASS "MTextControl"
#define MTM_SETUSER WM_USER
#define MTM_UPDATE  WM_USER+1

#if defined(_WIN32) || defined(__WIN32__)
	#define DLL_CALLCONV __stdcall
#endif 

typedef struct _tagMTEXT_interface {
	size_t	cbSize;
	DWORD	version;
	HANDLE	(DLL_CALLCONV *Register)	(const char *userTitle, DWORD options);
	HANDLE	(DLL_CALLCONV *Create)		(HANDLE userHandle, TCHAR *text);
	HANDLE	(DLL_CALLCONV *CreateEx)	(HANDLE userHandle, void *text, DWORD flags);
	int		(DLL_CALLCONV *Measure)		(HDC dc, SIZE *sz, HANDLE text);
	int		(DLL_CALLCONV *Display)		(HDC dc, POINT pos, SIZE sz, HANDLE text);
	int		(DLL_CALLCONV *SetParent)	(HANDLE text, HWND hwnd, RECT rect);
	int		(DLL_CALLCONV *SendMsg)		(HWND hwnd, HANDLE text, UINT msg, WPARAM wParam, LPARAM lParam);
	HWND	(DLL_CALLCONV *CreateProxy)	(HANDLE text);
	int		(DLL_CALLCONV *Destroy)		(HANDLE text);
} MTEXT_INTERFACE;

// get access to the interface
// wParam = 0
// lParam = (LPARAM)(MTEXT_INTERFACE*)Mtext
// dont vorget to set cbSize before call service
#define MS_TEXT_GETINTERFACE "MText/GetInterface"

__forceinline INT_PTR mir_getMTI( MTEXT_INTERFACE* dest )
{
	dest->cbSize = sizeof(*dest);
	INT_PTR result = CallService(MS_TEXT_GETINTERFACE, 0, (LPARAM)dest);
	return result;
}

enum
{
	// visual text options, used in MS_TEXT_REGISTER
	MTEXT_FANCY_SMILEYS   = 0x00000010, // SmileyAdd smileys
	MTEXT_FANCY_BBCODES   = 0x00000020, // [b], [u], [i]
	MTEXT_FANCY_MATHMOD   = 0x00000040, // enable math module formula parsing
	MTEXT_FANCY_URLS      = 0x00000080, // underline urls
	MTEXT_FANCY_BBCODES2  = 0x00000100, // [color], [img], [url], not implemented yet
	MTEXT_FANCY_SIMPLEFMT = 0x00000200, // simple formatting ("_", "/" and "*")
	MTEXT_FANCY_MASK      = 0x00007fff,
	MTEXT_FANCY_DEFAULT   = 0x00008000, // Use default options

	// text options, used in MS_TEXT_REGISTER
	MTEXT_SYSTEM_HICONS   = 0x00010000, // [$handle=i<HICON as dword>$]
	MTEXT_SYSTEM_HBITMAPS = 0x00010000, // [$handle=b<HBITMAP as dword>$], not implemented yet
	MTEXT_SYSTEM_ESCAPED  = 0x00020000, // passed text is escaped with slashes, not implemented yet
	MTEXT_SYSTEM_MASK     = 0x7fff0000,
	MTEXT_SYSTEM_DEFAULT  = 0x80000000,  // Use default option -- just nothing system is used :)

	// text object flags
	MTEXT_FLG_CHAR        = 0x00000000,
	MTEXT_FLG_WCHAR       = 0x00000001,
	MTEXT_FLG_BIDI_RTL    = 0x00000002
};

#if defined(UNICODE) || defined (_UNICODE)
	#define MTEXT_FLG_TCHAR MTEXT_FLG_WCHAR
#else
	#define MTEXT_FLG_TCHAR MTEXT_FLG_CHAR
#endif

// used in MS_TEXT_CREATEEX
typedef struct tagMTEXTCREATE
{
	DWORD cbSize;
	MCONTACT hContact;
	void *text; // this is 'char *' or 'WCHAR *'
	DWORD flags;

	#ifdef __cplusplus
		tagMTEXTCREATE():
			text(0), hContact(0), flags(0)
			{
				cbSize = sizeof(*this);
			}
	#endif
} MTEXTCREATE, *LPMTEXTCREATE;

// used in MS_TEXT_MEASURE and MS_TEXT_DISPLAY
typedef struct tagMTEXTDISPLAY
{
	DWORD cbSize;
	HDC dc;
	POINT pos;
	SIZE sz;
	HANDLE text;

	#ifdef __cplusplus
		tagMTEXTDISPLAY():
			dc(0), text(0)
			{
				cbSize = sizeof(*this);
				pos.x = pos.y = 0;
				sz.cx = sz.cy = 0;
			}
	#endif
} MTEXTDISPLAY, *LPMTEXTDISPLAY;

// used in MS_TEXT_SETPARENT
typedef struct tagMTEXTSETPARENT
{
	HANDLE text;
	HWND hwnd;
	RECT rc;

	#ifdef __cplusplus
		tagMTEXTSETPARENT():
			hwnd(0), text(0)
			{
			}
	#endif
} MTEXTSETPARENT, *LPMTEXTSETPARENT;

// used in MS_TEXT_SENDMESSAGE
typedef struct tagMTEXTMESSAGE
{
	HWND hwnd;
	HANDLE text;
	UINT msg;
	WPARAM wParam;
	LPARAM lParam;

	#ifdef __cplusplus
		tagMTEXTMESSAGE():
			hwnd(0), text(0), msg(0), wParam(0), lParam(0)
			{
			}
	#endif
} MTEXTMESSAGE, *LPMTEXTMESSAGE;

//---------------------------------------------------------------------------
// deprecatet service and helper functions
// replaced by new mtext interface !!!!!!!
//---------------------------------------------------------------------------
#if defined(NOHELPERS) || defined(MIRANDA_NOHELPERS)
	#define MTEXT_NOHELPERS
#endif

// subscribe to MText services
// wParam = (WPARAM)(DOWRD)defaultOptions
// lParam = (LPARAM)(char *)userTitle
// result = (LRESULT)(HANDLE)userHandle
#define MS_TEXT_REGISTER "MText/Register"

#ifndef MTEXT_NOHELPERS
__inline HANDLE MTextRegister(const char *userTitle, DWORD options)
{
	return (HANDLE)CallService(MS_TEXT_REGISTER, (WPARAM)options, (LPARAM)userTitle);
}
#endif // MTEXT_NOHELPERS

// allocate text object (unicode)
// wParam = (WPARAM)(HANDLE)userHandle
// lParam = (LPARAM)(WCHAR *)text
// result = (LRESULT)(HANDLE)textHandle
#define MS_TEXT_CREATEW "MText/CreateW"

#ifndef MTEXT_NOHELPERS
__inline HANDLE MTextCreateW(HANDLE userHandle, WCHAR *text)
{
	return (HANDLE)CallService(MS_TEXT_CREATEW, (WPARAM)userHandle, (LPARAM)text);
}
#endif // MTEXT_NOHELPERS

// allocate text object (advanced)
// wParam = (WPARAM)(HANDLE)userHandle
// lParam = (LPARAM)(LPMTEXTCREATE)createInfo
// result = (LRESULT)(HANDLE)textHandle
#define MS_TEXT_CREATEEX "MText/CreateEx"

#ifndef MTEXT_NOHELPERS
__inline HANDLE MTextCreateEx(HANDLE userHandle, HANDLE hContact, void *text, DWORD flags)
{
	#ifdef __cplusplus
		MTEXTCREATE textCreate;
	#else
		MTEXTCREATE textCreate = {0};
		textCreate.cbSize = sizeof(textCreate);
	#endif
	textCreate.hContact = hContact;
	textCreate.text = text;
	textCreate.flags = flags;
	return (HANDLE)CallService(MS_TEXT_CREATEEX, (WPARAM)userHandle, (LPARAM)&textCreate);
}
#endif // MTEXT_NOHELPERS

// measure text object
// wParam = (LPARAM)(LPMTEXTDISPLAY)displayInfo
// result = 1 (success), 0 (failure)
// displayInfo->size.cx is interpreted as maximum width allowed.
// wrapped text size is stored in displayInfo->size, text
#define MS_TEXT_MEASURE "MText/Measure"

#ifndef MTEXT_NOHELPERS
__inline int MTextMeasure(HDC dc, SIZE *sz, HANDLE text)
{
	#ifdef __cplusplus
		MTEXTDISPLAY displayInfo;
	#else
		MTEXTDISPLAY displayInfo = {0};
		displayInfo.cbSize = sizeof(displayInfo);
	#endif
	displayInfo.dc = dc;
	displayInfo.pos.x = displayInfo.pos.y = 0;
	displayInfo.sz = *sz;
	displayInfo.text = text;
	int result = (int)CallService(MS_TEXT_MEASURE, (WPARAM)&displayInfo, 0);
	*sz = displayInfo.sz;
	return result;
}
#endif // MTEXT_NOHELPERS

// display text object
// wParam = (LPARAM)(LPMTEXTDISPLAY)displayInfo
// result = 1 (success), 0 (failure)
#define MS_TEXT_DISPLAY "MText/Display"

#ifndef MTEXT_NOHELPERS
__inline int MTextDisplay(HDC dc, POINT pos, SIZE sz, HANDLE text)
{
	#ifdef __cplusplus
		MTEXTDISPLAY displayInfo;
	#else
		MTEXTDISPLAY displayInfo = {0};
		displayInfo.cbSize = sizeof(displayInfo);
	#endif
	displayInfo.dc = dc;
	displayInfo.pos = pos;
	displayInfo.sz = sz;
	displayInfo.text = text;
	return (int)CallService(MS_TEXT_DISPLAY, (WPARAM)&displayInfo, 0);
}
#endif // MTEXT_NOHELPERS

// set parent window for text object (this is required for mouse handling, etc)
// wParam = (WPARAM)(LPMTEXTSETPARENT)info
// result = message result
#define MS_TEXT_SETPARENT "MText/SetParent"

#ifndef MTEXT_NOHELPERS
__inline int MTextSetParent(HANDLE text, HWND hwnd, RECT rect)
{
	MTEXTSETPARENT info;
	info.text = text;
	info.hwnd = hwnd;
	info.rc = rect;
	return (int)CallService(MS_TEXT_SETPARENT, (WPARAM)&info, 0);
}
#endif // MTEXT_NOHELPERS

// send message to an object
// wParam = (WPARAM)(LPMTEXTMESSAGE)message
// result = message result
#define MS_TEXT_SENDMESSAGE "MText/SendMessage"

#ifndef MTEXT_NOHELPERS
__inline int MTextSendMessage(HWND hwnd, HANDLE text, UINT msg, WPARAM wParam, LPARAM lParam)
{
	#ifdef __cplusplus
		MTEXTMESSAGE message;
	#else
		MTEXTMESSAGE message = {0};
	#endif
	message.hwnd = hwnd;
	message.text = text;
	message.msg = msg;
	message.wParam = wParam;
	message.lParam = lParam;
	return (int)CallService(MS_TEXT_SENDMESSAGE, (WPARAM)&message, 0);
}
#endif // MTEXT_NOHELPERS

// create a proxy window
// wParam = (LPARAM)(HANDLE)textHandle
#define MS_TEXT_CREATEPROXY "MText/CreateProxy"

#ifndef MTEXT_NOHELPERS
__inline HWND MTextCreateProxy(HANDLE text)
{
	return (HWND)CallService(MS_TEXT_CREATEPROXY, (WPARAM)text, 0);
}
#endif // MTEXT_NOHELPERS

// destroy text object
// wParam = (LPARAM)(HANDLE)textHandle
#define MS_TEXT_DESTROY "MText/Destroy"

#ifndef MTEXT_NOHELPERS
__inline int MTextDestroy(HANDLE text)
{
	return (int)CallService(MS_TEXT_DESTROY, (WPARAM)text, 0);
}
#endif // MTEXT_NOHELPERS

//#define MS_TEXT_QDISPLAY "MText/QDisplay"
//#define MS_TEXT_QDISPLAYW "MText/QDisplayW"

// T-definitions for unicode
#if defined(UNICODE) || defined (_UNICODE)
	#define MS_TEXT_CREATET MS_TEXT_CREATEW
	#ifndef MTEXT_NOHELPERS
		#define MTextCreateT MTextCreateW
	#endif
#else
	#define MS_TEXT_CREATET MS_TEXT_CREATE
	#ifndef MTEXT_NOHELPERS
		#define MTextCreateT MTextCreate
	#endif
#endif

#endif // __mtext_h__
