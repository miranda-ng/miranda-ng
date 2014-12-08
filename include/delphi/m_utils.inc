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

  PUTILRESIZEDIALOG = ^TUTILRESIZEDIALOG;
  TUTILRESIZEDIALOG = record
    cbSize    : int;
    hwndDlg   : HWND;
    hInstance : HINST;
    lpTemplate: PAnsiChar;
    lParam    : LPARAM;
    pfnResizer: TDIALOGRESIZERPROC;
  end;

  PCountryListEntry = ^TCountryListEntry;
  TCountryListEntry = record
    id     : int;
    szName : PAnsiChar;
    ISOcode: array [0..2] of AnsiChar;
  end;

  PWINDOWLISTENTRY = ^TWINDOWLISTENTRY;
  TWINDOWLISTENTRY = record
    hList   : THANDLE;
    hWnd    : HWND;
    hContact: TMCONTACT;
  end;

  PSAVEWINDOWPOS = ^TSAVEWINDOWPOS;
  TSAVEWINDOWPOS = record
    hWnd        : HWND;
    hContact    : TMCONTACT;
    szModule    : PAnsiChar;        // module name eto store the settings in
    szNamePrefix: PAnsiChar;        // text to prefix on 'x', 'width', etc
  end;

const
  OUF_NEWWINDOW = 1;
  OUF_UNICODE   = 2;
  {
    wParam : OUF_* flag
    lParam : Pointer to a null terminated string containing Url
    Affect : Open a URL in the user's default web browser, see notes
    Returns: 0 on success, [non zero on failure]
    Notes  : bOpenInWindow should be zero to open the URL in the browoser window
             the user last used, or nonzero to open in a new browser window,
             if there's no browser running, it will be started to show the URL
    Version: v0.1.0.1+
  }
  MS_UTILS_OPENURL:PAnsiChar = 'Utils/OpenURL';

  {
    wParam : 0
    lParam : Pointer to an initalised TUTILRESIZEDIALOG structure
    Affect : Resize a dialog by calling a custom routine to move each control, see notes
    Returns: 0 on success, [non zero] on failure
    Notes  : Does not support DIALOGTEMPLATEEX dialogboxes, and will return
             failure if you try to resize one.-
             the dialog iteself should have been resized prior to calling this
             service, .pfnResizer is called once for each control in the dialog
             .pfnResizer should return a combination of one RD_ANCHORx_ and one RD_ANCHORy constant
    Version: v0.1.0.1+
  }
  MS_UTILS_RESIZEDIALOG:PAnsiChar = 'Utils/ResizeDialog';

  {
    wParam : countryID
    lParam : 0
    Affect : Get the name of a country given it's number, e.g. 44 = UK
    Returns: Returns a pointer to a string containing the country name on success
             NULL(0) on failure
    Version: v0.1.2.0+
  }
  MS_UTILS_GETCOUNTRYBYNUMBER:PAnsiChar = 'Utils/GetCountryByNumber';

  {
    Gets the name of a country given its ISO code	v0.1.2.0+
    wParam = (char*)ISOcode
    lParam = 0
    Returns a pointer to the string containing the country name on success,
    or NULL on failure
  }
  MS_UTILS_GETCOUNTRYBYISOCODE:PAnsiChar = 'Utils/GetCountryByISOCode';

  {
    wParam : Pointer to an int to be filled with count  -- !TODO! test.
    lParam : Pointer to an PCountryListEntry, see notes
    Affect : Get the full list of country IDs, see notes
    Returns: 0 always
    Notes  : the list is sorted alphabetically by name, on the assumption
             it's quicker to search numbers that are out of outer, than strings
             that are out of order. a NULL(0) entry terminates
             -
             Neither wParam or lParam can be NULL(0)
             -
             lParam is filled with the first entry, it can be accessed as a pointer,
             to get the next entry, increment the pointer by sizeof(Pointer) NOT
             sizeof(TCountryList), only increment the pointer as many times as
             given by iCount.
             -
             this data can NOT be copied if an array of TCountryListEntry's is passed
             so don't try it.
    Version: v0.1.2.0+
  }
  MS_UTILS_GETCOUNTRYLIST:PAnsiChar = 'Utils/GetCountryList';


//******************************* Window lists *******************************

  {
    wParam : 0
    lParam : 0
    Affect : Allocate a window list
    Returns: A handle to the new window list
  }
  MS_UTILS_ALLOCWINDOWLIST:PAnsiChar = 'Utils/AllocWindowList';

  {
    wParam : 0
    lParam : Pointer to an initalised TWINDOWLISTENTRY structure
    Affect : Add a window to a given window list handle
    Returns: 0 on success, [non zero] on failure
  }
  MS_UTILS_ADDTOWINDOWLIST:PAnsiChar = 'Utils/AddToWindowList';

  {
    wParam : Handle to window list to remove from
    lParam : Window handle to remove
    Affect : Remove a window from the specified window list
    Returns: 0 on success, [non zero] on failure
  }
  MS_UTILS_REMOVEFROMWINDOWLIST:PAnsiChar = 'Utils/RemoveFromWindowList';

  {
    wParam : Handle to the window list to look in
    lParam : Handle to a TMCONTACT to find in the window list
    Affect : Find a window handle given the hContact
    Returns: The found window handle or NULL(0) on failure
  }
  MS_UTILS_FINDWINDOWINLIST:PAnsiChar = 'Utils/FindWindowInList';

  {
    wParam : Handle to window list
    lParam : Pointer to TMSG (initalised with what to broadcast)
    Affect : sends a message to all windows in a list using SendMessage
    Returns: 0 on success, [non zero] on failure
    Notes  : only TMSG.Message, .wParam, .lParam are used
  }
  MS_UTILS_BROADCASTTOWINDOWLIST:PAnsiChar = 'Utils/BroadcastToWindowList';

  {
  Inline helper: WindowList_BroadcastAsync
  wParam : Handle to window list
  lParam : Pointer to TMSG (initalised with what to broadcast)
  Affect : Sends a message to all windows in a list using PostMessage
  Returns: 0 on success, nonzero on failure, this service does not fail,
           even if PostMessage() fails for whatever reason
  }
  MS_UTILS_BROADCASTTOWINDOWLIST_ASYNC:PAnsiChar = 'Utils/BroadcastToWindowListAsync';

  {
    There aren't any services here, there's no need for them, the control class
    will obey the SS_LEFT (0), SS_CENTER (1), SS_RIGHT(2) styles
    the control will send STN_CLICKED via WM_COMMAND when the link itself is clicked
    -
    These are defined by STATIC controls and STN_CLICKED is sent to standard
    STATIC classes when they're clicked -- look at WINAPI docs for more info
  }
  WNDCLASS_HYPERLINK = 'Hyperlink';

  {
    wParam : 0
    lParam : Pointer to a initialised TSAVEWINDOWPOS structure
    Affect :
    Returns: 0 on success, [non zero] on failure
    Notes  :
    Version: v0.1.1.0+
  }
  MS_UTILS_SAVEWINDOWPOSITION:PAnsiChar = 'Utils/SaveWindowPos';

  {
    wParam : see RWPF_* flags
    lParam : Pointer to a initalised TSAVEWINDOWPOS
    Affect : Restores the position of a window from the database, see notes
    Returns: 0 on success, [non zero] on failure
    Notes  : If no position info was found, the service will return 1.
             The NoSize version won't use stored information size, the window
             is left the same size
             -
             See Utils_RestoreWindowPosition() Helper function, this function is
             a bit different from the C function (which can be inlined too! dammit)
             that there's only one function and not three (which just passed different flags)
    Version: v0.1.1.0+
  }
  MS_UTILS_RESTOREWINDOWPOSITION:PAnsiChar = 'Utils/RestoreWindowPos';

{
    wParam : pointer to RECT
    lParam : 0
    Affect : Moves a RECT inside screen if it is outside.It works with multiple monitors (v0.9.0.4+)
    Returns: <0 on error, 0 if not changed the rect, 1 if changed the rect
}
  MS_UTILS_ASSERTINSIDESCREEN:PAnsiChar = 'Utils/AssertInsideScreen';

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
    Version: v0.1.2.1+
  }
  MS_UTILS_LOADBITMAP :PAnsiChar = 'Utils/LoadBitmap';
  MS_UTILS_LOADBITMAPW:PAnsiChar = 'Utils/LoadBitmapW';

  {
      wParam : byte length of buffer (not to be confused with byte range)
      lParam : Pointer to buffer
      Affect : Get the filter strings for use in the open file dialog, see notes
      Returns: 0 on success [non zero] on failure
      Notes  : See the WINAPI under OPENFILENAME.lpStrFiler for formatting,
               an 'All bitmaps' item is alway first, and 'All files' is always last
               -
               The returned string is always formatted
               -
               To build this filter, the filter string consists of
               filter followed by a descriptive text
               followed by more filters and their descriptive texts -- end with double NULL(0)
               e.g. *.bmp' #0 'All bitmaps' #0 '*.*' #0 'All Files' #0 #0
  }
  MS_UTILS_GETBITMAPFILTERSTRINGS :PAnsiChar = 'Utils/GetBitmapFilterStrings';
  MS_UTILS_GETBITMAPFILTERSTRINGSW:PAnsiChar = 'Utils/GetBitmapFilterStringsW';

  {
    wParam : pszPath
    lParam : pszNewPath
    Affect : Saves a path to a relative path (from the miranda directory)
             Only saves as a relative path if the file is in the miranda
             directory (or sub directory)
    Notes  : pszPath is the path to convert and pszNewPath is the buffer that
             the new path is copied too. pszNewPath MUST be of the size MAX_PATH.
    Returns: numbers of chars copied.
  }
  MS_UTILS_PATHTORELATIVE :PAnsiChar = 'Utils/PathToRelative';
//Unicode versions (0.6.2+)
  MS_UTILS_PATHTORELATIVEW:PAnsiChar = 'Utils/PathToRelativeW';

  {
    Affect : Saves a path to a absolute path (from the miranda directory)
    wParam : pszPath
    lParam : pszNewPath
    Notes  : pszPath is the path to convert and pszNewPath is the buffer that
             the new path is copied too. pszNewPath MUST be of the size MAX_PATH.
    Returns: numbers of chars copied.
  }
  MS_UTILS_PATHTOABSOLUTE :PAnsiChar = 'Utils/PathToAbsolute';
//Unicode versions (0.6.2+)
  MS_UTILS_PATHTOABSOLUTEW:PAnsiChar = 'Utils/PathToAbsoluteW';

{
  Creates a directory tree (even more than one directories levels are missing) 0.7.0+
  wParam=0 (unused)
  lParam=pszPath - directory to be created
  Returns 0 on success error code otherwise
  Unicode version is available since 0.7.0
}
  MS_UTILS_CREATEDIRTREE :PAnsiChar = 'Utils/CreateDirTree';
  MS_UTILS_CREATEDIRTREEW:PAnsiChar = 'Utils/CreateDirTreeW';

{
   Generates Random number of any length
  wParam=size - length of the random number to generate
  lParam=(LPARAM)(char*)pszArray - pointer to array to fill with random number
  Always returns 0
}
  MS_UTILS_GETRANDOM:PAnsiChar = 'Utils/GetRandom';

//Replace variables in text
//wParam=(char*/TCHAR*/WCHAR*)string (depends on RVF_UNICODE/RVF_TCHAR flag)
//lParam=(REPLACEVARSDATA *) data about variables, item with key=0 terminates the list
//returns new string, use mir_free to destroy
type
  PREPLACEVARSARRAY = ^TREPLACEVARSARRAY;
  TREPLACEVARSARRAY = record
    szKey  :TCHAR;
    szValue:TCHAR;
  end;

type
  TREPLACEVARSDATA = record
    cbSize   :int;
    dwFlags  :dword;
    hContact :TMCONTACT;
    variables:PREPLACEVARSARRAY;
  end;

const
  RVF_UNICODE = 1;

  MS_UTILS_REPLACEVARS:PAnsiChar = 'Utils/ReplaceVars';

{
  variables known by the core:
  ----------------------------
  folders comes without trailing slash

  %miranda_profile%     -> same as MS_DB_GETPROFILEPATH, base folder for all profiles
  %miranda_userdata%    -> the active profile folder (home of the .dat file and all
                           profile data)
  %miranda_path%        -> home path of the miranda installation (installation path
                           of miranda32/64.exe
  %miranda_profilename% -> Name of the profile in use. Essentially, the name of the
                           .dat file without file name extension. Also: the folder name
                           relative to %miranda_profile% where all profile data is stored.
  %miranda_logpath%     -> base folder for log files. This is \Logs relative to the
                           current profile folder.
  %miranda_avatarcache% -> base folder for all protocol avatars. internal use only.

  the following variables operate on contacts. REPLACEVARSDATA::hContact must be
  supplied by the caller.

  %nick%                -> a contact nick name.
  %proto%               -> internal protocol name for a given contact. NOT the user-
                           defined account name.
  %accountname%         -> user-defined account name for a given contact.
  %userid%              -> Unique ID for a given contact (UIN, JID etc.)

  the following variables are system variables - unrelated to miranda profiles.

  %appdata%             -> same as %APPDATA% environment variable.
  %destkop%             -> location of the desktop folder in a user's profile.
  %mydocuments%         -> location of the "My Documents" shell folder.
}

/////////////////////////////////////////////////////////////////////////////////////////
// one field form

const
  ESF_MULTILINE = 1;
  ESF_COMBO     = 2;
  ESF_RICHEDIT  = 3;
  ESF_PASSWORD  = 4;

type
  PENTER_STRING = ^TENTER_STRING;
  TENTER_STRING = record
    cbSize      :int;       // structure size
    _type       :int;       // one of ESF_* constants
    szModuleName:PAnsiChar; // module name to save window size and combobox strings
    szDataPrefix:PAnsiChar; // prefix for stored database variables
    caption     :PAnsiChar; // window caption
    // next TChar fields is on same place
    // ptszInitVal:TChar;   // initial value (note: the core DOES NOT free it)
    ptszResult  :TChar;     // result entered (must be freed via mir_free)
    recentCount :int;       // number of combobox strings to store
    timeout     :int;       // timeout for the form auto-close
  end;

const
// enters one string
// wParam = 0 (unused)
// lParam = PENTER_STRING (form description)
// returns TRUE on pressing OK or FALSE if Cancel was pressed
  MS_UTILS_ENTERSTRING:PAnsiChar = 'Utils/EnterString';

{$ENDIF}
