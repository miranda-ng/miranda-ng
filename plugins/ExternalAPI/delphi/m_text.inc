{$IFNDEF M_MTEXT}
{$DEFINE M_MTEXT}

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
const
  MTEXTCONTROLCLASS  = 'MTextControl';
  MTM_SETUSER = WM_USER;
  MTM_UPDATE  = WM_USER+1;

type
  TMTEXT_INTERFACE = record
    cbSize:size_t;
    version:dword;

    Register   :function(const userTitle:PAnsiChar; options:dword):THANDLE; stdcall;
    Create     :function(userHandle:THANDLE; text:pointer):THANDLE; stdcall;
    CreateEx   :function(userHandle:THANDLE; text:pointer; flags:dword):THANDLE; stdcall;
    Measure    :function(dc:HDC; sz:PSIZE; text:THANDLE):int; stdcall;
    Display    :function(dc:HDC; pos:TPOINT; sz:TSIZE; text:THANDLE):int; stdcall;
    SetParent  :function(text:THANDLE; hwnd:HWND; rect:TRECT):int; stdcall;
    SendMsg    :function(hwnd:HWND; text:THANDLE; msg:uint; wParam:WPARAM; lParam:LPARAM):int; stdcall;
    CreateProxy:function(text:THANDLE):HWND; stdcall;
    Destroy    :function(text:THANDLE):int; stdcall;
  end;

const
// get access to the interface
// wParam = 0
// lParam = (LPARAM)(MTEXT_INTERFACE*)Mtext
// dont vorget to set cbSize before call service
  MS_TEXT_GETINTERFACE:PAnsiChar = 'MText/GetInterface';
(*
__forceinline INT_PTR mir_getMTI( MTEXT_INTERFACE* dest )
{
	dest->cbSize = sizeof(*dest);
	INT_PTR result = CallService(MS_TEXT_GETINTERFACE, 0, (LPARAM)dest);
	return result;
}
*)
const
  // visual text options, used in MS_TEXT_REGISTER
  MTEXT_FANCY_SMILEYS   = $00000010; // SmileyAdd smileys
  MTEXT_FANCY_BBCODES   = $00000020; // [b], [u], [i]
  MTEXT_FANCY_MATHMOD   = $00000040; // enable math module formula parsing
  MTEXT_FANCY_URLS      = $00000080; // underline urls
  MTEXT_FANCY_BBCODES2  = $00000100; // [color], [img], [url], not implemented yet
  MTEXT_FANCY_SIMPLEFMT = $00000200; // simple formatting ("_", "/" and "*")
  MTEXT_FANCY_MASK      = $00007FFF;
  MTEXT_FANCY_DEFAULT   = $00008000; // Use default options

  // text options, used in MS_TEXT_REGISTER
  MTEXT_SYSTEM_HICONS   = $00010000; // [$handle=i<HICON as dword>$]
  MTEXT_SYSTEM_HBITMAPS = $00010000; // [$handle=b<HBITMAP as dword>$], not implemented yet
  MTEXT_SYSTEM_ESCAPED  = $00020000; // passed text is escaped with slashes, not implemented yet
  MTEXT_SYSTEM_MASK     = $7FFF0000;
  MTEXT_SYSTEM_DEFAULT  = $80000000;  // Use default option -- just nothing system is used :)

  // text object flags
  MTEXT_FLG_CHAR        = $00000000;
  MTEXT_FLG_WCHAR       = $00000001;
  MTEXT_FLG_BIDI_RTL    = $00000002;

// used in MS_TEXT_CREATEEX
type
  PMTEXTCREATE = ^TMTEXTCREATE;
  TMTEXTCREATE = record
    cbSize  :dword;
    hContact:TMCONTACT;
    text    :TChar; // this is 'char *' or 'WCHAR *'
    flags   :dword;
  end;

// used in MS_TEXT_MEASURE and MS_TEXT_DISPLAY
type
  PMTEXTDISPLAY = ^TMTEXTDISPLAY;
  TMTEXTDISPLAY = record
    cbSize:dword;
    dc    :HDC;
    pos   :TPOINT;
    sz    :TSIZE;
    text  :THANDLE;
  end;

// used in MS_TEXT_SETPARENT
type
  PMTEXTSETPARENT = ^TMTEXTSETPARENT;
  TMTEXTSETPARENT = record
    text:THANDLE;
    hwnd:HWND;
    rc  :TRECT;
  end;

// used in MS_TEXT_SENDMESSAGE
type
  PMTEXTMESSAGE = ^TMTEXTMESSAGE;
  TMTEXTMESSAGE = record
    hwnd  :HWND;
    text  :THANDLE;
    msg   :uint;
    wParam:WPARAM;
    lParam:LPARAM;
  end;

const
//---------------------------------------------------------------------------
// deprecatet service and helper functions
// replaced by new mtext interface !!!!!!!
//---------------------------------------------------------------------------

// subscribe to MText services
// wParam = (WPARAM)(DOWRD)defaultOptions
// lParam = (LPARAM)(char *)userTitle
// result = (LRESULT)(HANDLE)userHandle
  MS_TEXT_REGISTER:PAnsiChar = 'MText/Register';

// allocate text object (unicode)
// wParam = (WPARAM)(HANDLE)userHandle
// lParam = (LPARAM)(WCHAR *)text
// result = (LRESULT)(HANDLE)textHandle
  MS_TEXT_CREATEW:PAnsiChar = 'MText/CreateW';

// allocate text object (advanced)
// wParam = (WPARAM)(HANDLE)userHandle
// lParam = (LPARAM)(LPMTEXTCREATE)createInfo
// result = (LRESULT)(HANDLE)textHandle
  MS_TEXT_CREATEEX:PAnsiChar = 'MText/CreateEx';

(*
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
*)

// measure text object
// wParam = (LPARAM)(LPMTEXTDISPLAY)displayInfo
// result = 1 (success), 0 (failure)
// displayInfo->size.cx is interpreted as maximum width allowed.
// wrapped text size is stored in displayInfo->size, text
  MS_TEXT_MEASURE:PAnsiChar = 'MText/Measure';

(*
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
*)

// display text object
// wParam = (LPARAM)(LPMTEXTDISPLAY)displayInfo
// result = 1 (success), 0 (failure)
  MS_TEXT_DISPLAY:PAnsiChar = 'MText/Display';

(*
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
*)

// set parent window for text object (this is required for mouse handling, etc)
// wParam = (WPARAM)(LPMTEXTSETPARENT)info
// result = message result
  MS_TEXT_SETPARENT:PAnsiChar = 'MText/SetParent';

(*
__inline int MTextSetParent(HANDLE text, HWND hwnd, RECT rect)
{
	MTEXTSETPARENT info;
	info.text = text;
	info.hwnd = hwnd;
	info.rc = rect;
	return (int)CallService(MS_TEXT_SETPARENT, (WPARAM)&info, 0);
}
*)

// send message to an object
// wParam = (WPARAM)(LPMTEXTMESSAGE)message
// result = message result
  MS_TEXT_SENDMESSAGE:PAnsiChar = 'MText/SendMessage';

(*
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
*)

// create a proxy window
// wParam = (LPARAM)(HANDLE)textHandle
  MS_TEXT_CREATEPROXY:PAnsiChar = 'MText/CreateProxy';


// destroy text object
// wParam = (LPARAM)(HANDLE)textHandle
  MS_TEXT_DESTROY:PAnsiChar = 'MText/Destroy';


//  MS_TEXT_QDISPLAY :PAnsiChar = 'MText/QDisplay';
//  MS_TEXT_QDISPLAYW:PAnsiChar = 'MText/QDisplayW';

{$ENDIF}
