{$IFNDEF M_UTILS}
{$DEFINE M_UTILS}

const
  RD_ANCHORX_CUSTOM = 0; // function did everything required to the x axis, do no more processing
  RD_ANCHORX_LEFT   = 0; // move the control to keep it constant distance from the left edge of the dialog
  RD_ANCHORX_RIGHT  = 1; // move the control to keep it constant distance from the right edge of the dialog
  RD_ANCHORX_WIDTH  = 2; // size the control to keep it constant distance from both edges of the dialog
  RD_ANCHORX_CENTRE = 4; // move the control to keep it constant distance from the centre of the dialog
  RD_ANCHORY_CUSTOM = 0;
  RD_ANCHORY_TOP    = 0;
  RD_ANCHORY_BOTTOM = 8;
  RD_ANCHORY_HEIGHT = 16;
  RD_ANCHORY_CENTRE = 32;

  // for MS_UTILS_RESTOREWINDOWPOSITION
  RWPF_NOSIZE     = 1; // don't use stored size info: leave dialog same size
  RWPF_NOMOVE     = 2; // don't use stored position
  RWPF_NOACTIVATE = 4; // show but don't activate v0.3.3.0+
  RWPF_HIDDEN     = 8; // make it hidden v0.9.0.9+

  // for WNDCLASS_COLOURPICKER
  CPM_SETCOLOUR        = $1000;// lParam=new colour
  CPM_GETCOLOUR        = $1001;// returns colour
  CPM_SETDEFAULTCOLOUR = $1002;// lParam=default, used as first custom colour
  CPM_GETDEFAULTCOLOUR = $1003;// returns colour
  CPN_COLOURCHANGED    = 1;  // sent through WM_COMMAND

  //  Colour picker control, see CPM_* and CPN_* constants above
  WNDCLASS_COLOURPICKER = 'ColourPicker';

  // hyperlink messages
  {
    Use this in a SendMessage to set the color of the url when control is enabled
    wParam=dword color
    lParam=not used
  }
  HLK_SETENABLECOLOUR = WM_USER+101; // added in 0.3.1

  {
    Use this in a SendMessage to set the color of the url when control is disabled
    wParam=dword color
    lParam=not used
  }
  HLK_SETDISABLECOLOUR = WM_USER+102; // added in 0.3.1

type
  PUTILRESIZECONTROL = ^TUTILRESIZECONTROL;
  TUTILRESIZECONTROL = record
    cbSize         : int;
    wId            : int;   // control ID
    rcItem         : TRECT; // original control rectangle, relative to dialog
                            // modify in-placee to specify the new position
    dlgOriginalSize: TSize; // size of dialog client area in template
    dlgNewSize     : TSize; // current size of dialog client area
  end;

  TDIALOGRESIZERPROC = function(hwndDlg: THANDLE; lParam: LPARAM; urc: PUTILRESIZECONTROL): int; cdecl;

  {
    Affect : Open a URL in the user's default web browser, see notes
    Notes  : bOpenInWindow should be zero to open the URL in the browoser window
             the user last used, or nonzero to open in a new browser window,
             if there's no browser running, it will be started to show the URL
  }

procedure Utils_OpenUrl(const url:PAnsiChar; bOpenInWindow:BYTE=1); stdcall;
                 external CoreDLL name 'Utils_OpenUrl';

procedure Utils_OpenUrlW(const url:PWideChar; bOpenInWindow:BYTE=1); stdcall;
                 external CoreDLL name 'Utils_OpenUrlW';
  {
    Affect : Resize a dialog by calling a custom routine to move each control, see notes
    Returns: 0 on success, [non zero] on failure
    Notes  : Does not support DIALOGTEMPLATEEX dialogboxes, and will return
             failure if you try to resize one.-
             the dialog iteself should have been resized prior to calling this
             service, .pfnResizer is called once for each control in the dialog
             .pfnResizer should return a combination of one RD_ANCHORx_ and one RD_ANCHORy constant
    Version: v0.1.0.1+
  }

function Utils_ResizeDialog(hwndDlg:THANDLE; hInst:HINST; pTemplate:PAnsiChar; pFunc:TDIALOGRESIZERPROC; lParam:LPARAM=0) : int; stdcall;
                 external CoreDLL name 'Utils_ResizeDialog';

//******************************* Window lists *******************************

{
  Affect : Allocate a window list
  Returns: A handle to the new window list
}

function WindowList_Create() : Thandle; stdcall;
                 external CoreDLL name 'WindowList_Create';

procedure WindowList_Destroy(hList:Thandle); stdcall;
                 external CoreDLL name 'WindowList_Destroy';

{
  Affect : Add a window to a given window list handle
  Returns: 0 on success, [non zero] on failure
}

function WindowList_Add(hList:Thandle; hwnd:HWND; hContact:TMCONTACT) : int; stdcall;
                 external CoreDLL name 'WindowList_Add';

{
  Affect : Remove a window from the specified window list
  Returns: 0 on success, [non zero] on failure
}

function WindowList_Remove(hList:Thandle; hwnd:HWND) : int; stdcall;
                 external CoreDLL name 'WindowList_Remove';

{
  Affect : Find a window handle given the hContact
  Returns: The found window handle or NULL(0) on failure
}

function WindowList_Find(hList:Thandle; hContact:TMCONTACT) : HWND; stdcall;
                 external CoreDLL name 'WindowList_Find';

{
  Affect : sends a message to all windows in a list using SendMessage
  Returns: 0 on success, [non zero] on failure
}

function WindowList_Broadcast(hList:Thandle; message:UINT; wParam:TWPARAM; lParam:TLPARAM) : int; stdcall;
                 external CoreDLL name 'WindowList_Broadcast';

{
  Affect : Sends a message to all windows in a list using PostMessage
  Returns: 0 on success, nonzero on failure, this service does not fail,
         even if PostMessage() fails for whatever reason
}

function WindowList_BroadcastAsync(hList:Thandle; message:UINT; wParam:TWPARAM; lParam:TLPARAM) : int; stdcall;
                 external CoreDLL name 'WindowList_BroadcastAsync';

{
  There aren't any services here, there's no need for them, the control class
  will obey the SS_LEFT (0), SS_CENTER (1), SS_RIGHT(2) styles
  the control will send STN_CLICKED via WM_COMMAND when the link itself is clicked
  -
  These are defined by STATIC controls and STN_CLICKED is sent to standard
  STATIC classes when they're clicked -- look at WINAPI docs for more info
}

const
  WNDCLASS_HYPERLINK = 'Hyperlink';

{
  Returns: 0 on success, [non zero] on failure
}

function Utils_SaveWindowPosition(hwnd:HWND; hContact:TMCONTACT; const szModule,szNamePrefix:PAnsiChar) : int; stdcall;
                 external CoreDLL name 'Utils_SaveWindowPosition';

{
  Affect : Restores the position of a window from the database, see notes
  Returns: 0 on success, [non zero] on failure
  Notes  : If no position info was found, the service will return 1.
           The NoSize version won't use stored information size, the window
           is left the same size
           -
           See Utils_RestoreWindowPosition() Helper function, this function is
           a bit different from the C function (which can be inlined too! dammit)
           that there's only one function and not three (which just passed different flags)
}

function Utils_RestoreWindowPosition(hwnd:HWND; hContact:TMCONTACT; const szModule,szNamePrefix:PAnsiChar; flags:int) : int; stdcall;
                 external CoreDLL name 'Utils_RestoreWindowPosition';

{
    Affect : Moves a RECT inside screen if it is outside.It works with multiple monitors (v0.9.0.4+)
    Returns: <0 on error, 0 if not changed the rect, 1 if changed the rect
}

function Utils_AssertInsideScreen(var rc:TRECT) : int; stdcall;
                 external CoreDLL name 'Utils_AssertInsideScreen';

{
  wParam : 0
  lParam : Pointer to a null terminated string containing filename
  Affect : Loads a bitmap (or other graphic type, see Notes
  Returns: HBITMAP on success, NULL(0) on failure
  Notes  : This function also supports JPEG, GIF (and maybe PNG too)
           For speed, if the file extention is .bmp or .rle it will use LoadImage()
           and not load OLE for the extra image support
           -
           Remember to delete the returned handle with DeleteObject (see GDI documentation for WINAPI)
}

const
  MS_UTILS_LOADBITMAP :PAnsiChar = 'Utils/LoadBitmap';
  MS_UTILS_LOADBITMAPW:PAnsiChar = 'Utils/LoadBitmapW';

{
  Affect : Saves a path to a relative path (from the miranda directory)
           Only saves as a relative path if the file is in the miranda
           directory (or sub directory)
  Notes  : pszPath is the path to convert and pszNewPath is the buffer that
           the new path is copied too. pszNewPath MUST be of the size MAX_PATH.
  Returns: numbers of chars copied.
}

function PathToRelative(const pSrc:PAnsiChar; pOut:PAnsiChar; base:PAnsiChar=nil):int; stdcall;
                 external CoreDLL name 'PathToRelative';
function PathToRelativeW(const pSrc:PWideChar; pOut:PWideChar; base:PWideChar=nil):int; stdcall;
                 external CoreDLL name 'PathToRelativeW';

{
  Affect : Saves a path to a absolute path (from the miranda directory)
  Notes  : pszPath is the path to convert and pszNewPath is the buffer that
           the new path is copied too. pszNewPath MUST be of the size MAX_PATH.
  Returns: numbers of chars copied.
}

function PathToAbsolute(const pSrc:PAnsiChar; pOut:PAnsiChar; base:PAnsiChar=nil):int; stdcall;
                 external CoreDLL name 'PathToAbsolute';
function PathToAbsoluteW(const pSrc:PWideChar; pOut:PWideChar; base:PWideChar=nil):int; stdcall;
                 external CoreDLL name 'PathToAbsoluteW';

{
  Creates a directory tree (even more than one directories levels are missing) 0.7.0+
  Returns 0 on success error code otherwise
}

function CreateDirectoryTree(const szDir:PAnsiChar):int; stdcall;
                 external CoreDLL name 'CreateDirectoryTree';
function CreateDirectoryTreeW(const szDir:PWideChar):int; stdcall;
                 external CoreDLL name 'CreateDirectoryTreeW';

{
  Creates all subdirectories required to create a file with the file name given
  Returns 0 on success or an error code otherwise
}

procedure CreatePathToFile(wszFilePath:PAnsiChar); stdcall;
                 external CoreDLL name 'CreatePathToFile';
procedure CreatePathToFileW(wszFilePath:PWideChar); stdcall;
                 external CoreDLL name 'CreatePathToFileW';
{
  Checks if a file name is absolute or not
  returns TRUE if yes or FALSE if not
}

function PathIsAbsolute(const pSrc:PAnsiChar):int; stdcall;
                 external CoreDLL name 'PathIsAbsolute';
function PathIsAbsoluteW(const pSrc:PWideChar):int; stdcall;
                 external CoreDLL name 'PathIsAbsoluteW';

{
  Generates Random number of any length
  wParam=size - length of the random number to generate
  lParam=(LPARAM)(char*)pszArray - pointer to array to fill with random number
  Always returns 0
}

procedure Utils_GetRandom(pSrc:pointer; size:size_t); stdcall;
                 external CoreDLL name 'Utils_GetRandom';

// Replace variables in text
// returns new string, use mir_free to destroy

type
  PREPLACEVARSARRAY = ^TREPLACEVARSARRAY;
  TREPLACEVARSARRAY = record
    szKey  :TCHAR;
    szValue:TCHAR;
  end;

function Utils_ReplaceVars(const pSrc:PAnsiChar; hContact:TMCONTACT = 0; vars:PREPLACEVARSARRAY = nil):PAnsiChar; stdcall;
                 external AppDLL name 'Utils_ReplaceVars';
function Utils_ReplaceVarsW(const pSrc:PWideChar; hContact:TMCONTACT = 0; vars:PREPLACEVARSARRAY = nil):PWideChar; stdcall;
                 external AppDLL name 'Utils_ReplaceVarsW';

{$ENDIF}
