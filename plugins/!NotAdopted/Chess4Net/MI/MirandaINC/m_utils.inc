(*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*)

{$IFNDEF M_UTILS}
{$DEFINE M_UTILS}

const

    RD_ANCHORX_CUSTOM           = 0;	// function did everything required to the x axis, do no more processing
    RD_ANCHORX_LEFT             = 0;	// move the control to keep it constant distance from the left edge of the dialog
    RD_ANCHORX_RIGHT            = 1;	// move the control to keep it constant distance from the right edge of the dialog
    RD_ANCHORX_WIDTH            = 2;	// size the control to keep it constant distance from both edges of the dialog
    RD_ANCHORX_CENTRE           = 4;	// move the control to keep it constant distance from the centre of the dialog
    RD_ANCHORY_CUSTOM           = 0;
    RD_ANCHORY_TOP              = 0;
    RD_ANCHORY_BOTTOM           = 8;
    RD_ANCHORY_HEIGHT           = 16;
    RD_ANCHORY_CENTRE           = 32;

    // for MS_UTILS_RESTOREWINDOWPOSITION

    RWPF_NOSIZE                 = 1;    // don't use stored size info: leave dialog same size
    RWPF_NOMOVE                 = 2;    // don't use stored position

    // for WNDCLASS_COLOURPICKER

    CPM_SETCOLOUR               = $1000;// lParam=new colour
    CPM_GETCOLOUR               = $1001;// returns colour
    CPM_SETDEFAULTCOLOUR        = $1002;// lParam=default, used as first custom colour
    CPM_GETDEFAULTCOLOUR        = $1003;// returns colour
    CPN_COLOURCHANGED           = 1;	// sent through WM_COMMAND

type

    PUTILRESIZECONTROL = ^TUTILRESIZECONTROL;
    TUTILRESIZECONTROL = record
        cbSize: int;
        wId: int;               // control ID
        rcItem: TRect;          // original control rectangle, relative to dialog
                                // modify in-placee to specify the new position
        dlgOriginalSize: TSize; // size of dialog client area in template
        dlgNewSize: TSize;      // current size of dialog client area
    end;

    TDIALOGRESIZERPROC = function(hwndDlg: THandle; lParam: LPARAM; urc: PUTILRESIZECONTROL): int; cdecl;

    PUTILRESIZEDIALOG = ^TUTILRESIZEDIALOG;
    TUTILRESIZEDIALOG = record
        cbSize: int;
        hwndDlg: THandle;
        hInstance: THandle;
        lpTemplate: PChar;
        lParam: LPARAM;
        pfnResizer: TDIALOGRESIZERPROC;
    end;

    PCountryListEntry = ^TCountryListEntry;
    TCountryListEntry = record
        id: int;
        szName: PChar;
    end;

    PWINDOWLISTENTRY = ^TWINDOWLISTENTRY;
    TWINDOWLISTENTRY = record
        hList: THandle;
        hWnd: THandle;
        hContact: THandle;
    end;

    PSAVEWINDOWPOS = ^TSAVEWINDOWPOS;
    TSAVEWINDOWPOS = record
        hWnd: THandle;
        hContact: THandle;
        szModule: PChar;            // module name eto store the settings in
        szNamePrefix: PChar;        // text to prefix on 'x', 'width', etc
    end;

const

    {
        wParam : bOpenInNewWindow
        lParam : Pointer to a null terminated string containing Url
        Affect : Open a URRL in the user's default web browser, see notes
        Returns: 0 on success, [non zero on failure]
        Notes  : bOpenInWindow should be zero to open the URL in the browoser window
                 the user last used, or nonzero to open in a new browser window,
                 if there's no browser running, it will be started to show the URL
        Version: v0.1.0.1+
    }
    MS_UTILS_OPENURL                = 'Utils/OpenURL';

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
    MS_UTILS_RESIZEDIALOG	        = 'Utils/ResizeDialog';

    {
        wParam : countryID
        lParam : 0
        Affect : Get the name of a country given it's number, e.g. 44 = UK
        Returns: Returns a pointer to a string containing the country name on success
                 NULL(0) on failure
        Version: v0.1.2.0+
    }
    MS_UTILS_GETCOUNTRYBYNUMBER     = 'Utils/GetCountryByNumber';

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
    MS_UTILS_GETCOUNTRYLIST         = 'Utils/GetCountryList';

    // see WindowList_* functions below

    {
        wParam : 0
        lParam : 0
        Affect : Allocate a window list
        Returns: A handle to the new window list
        Version: v0.1.0.1+
    }
    MS_UTILS_ALLOCWINDOWLIST        = 'Utils/AllocWindowList';

    {
        wParam : 0
        lParam : Pointer to an initalised TWINDOWLISTENTRY structure
        Affect : Add a window to a given window list handle
        Returns: 0 on success, [non zero] on failure
        Version: v0.1.0.1+
    }
    MS_UTILS_ADDTOWINDOWLIST        = 'Utils/AddToWindowList';

    {
        wParam : Handle to window list to remove from
        lParam : Window handle to remove
        Affect : Remove a window from the specified window list
        Returns: 0 on success, [non zero] on failure
        Version: v0.1.0.1+
    }
    MS_UTILS_REMOVEFROMWINDOWLIST   = 'Utils/RemoveFromWindowList';

    {
        wParam : Handle to the window list to look in
        lParam : Handle to a MCONTACT to find in the window list
        Affect : Find a window handle given the hContact
        Returns: The found window handle or NULL(0) on failure
        Version: v0.1.0.1+
    }
    MS_UTILS_FINDWINDOWINLIST       = 'Utils/FindWindowInList';

    {
        wParam : Handle to window list
        lParam : Pointer to TMSG (initalised with what to broadcast)
        Affect : Broadcast a message to all windows in a list, see notes
        Returns: 0 on success, [non zero] on failure
        Notes  : only TMSG.Message, .wParam, .lParam are used
        Version: v0.1.0.1+
    }
    MS_UTILS_BROADCASTTOWINDOWLIST  = 'Utils/BroadcastToWindowList';

    {
        There aren't any services here, there's no need for them, the control class
        will obey the SS_LEFT (0), SS_CENTER (1), SS_RIGHT(2) styles
        the control will send STN_CLICKED via WM_COMMAND when the link itself is clicked
        -
        These are defined by STATIC controls and STN_CLICKED is sent to standard
        STATIC classes when they're clicked -- look at WINAPI docs for more info
    }
    WNDCLASS_HYPERLINK              = 'Hyperlink';

    {
        wParam : 0
        lParam : Pointer to a initialised TSAVEWINDOWPOS structure
        Affect :
        Returns: 0 on success, [non zero] on failure
        Notes  :
        Version: v0.1.1.0+
    }
    MS_UTILS_SAVEWINDOWPOSITION     = 'Utils/SaveWindowPos';

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
    MS_UTILS_RESTOREWINDOWPOSITION  = 'Utils/RestoreWindowPos';

    {
        Colour picker control, see CPM_* and CPN_* constants above
    }
    WNDCLASS_COLOURPICKER           = 'ColourPicker';

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
    MS_UTILS_LOADBITMAP             = 'Utils/LoadBitmap';

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
    MS_UTILS_GETBITMAPFILTERSTRINGS = 'Utils/GetBitmapFilterStrings';

{$endif}
