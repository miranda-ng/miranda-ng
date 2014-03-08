{
  Miranda SmileyAdd Plugin
  Plugin support header file
  Copyright (C) 2004 Rein-Peter de Boer (peacow), and followers

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
}

{$IFNDEF M_SMILEYADD}
{$DEFINE M_SMILEYADD}
const
  SAFLRE_INSERTEMF =  2; // insert smiley as EMF into RichEdit, otherwise bitmap
                         // inserted this flag allows "true" transparency
  SAFLRE_OUTGOING  =  4; // Parsing outgoing message
  SAFLRE_NOCUSTOM  =  8; // Do not use custom smileys
  SAFLRE_FIREVIEW  = 16; // use NM_FIREVIEWCHANGE messages

type
  PSMADD_RICHEDIT3 = ^TSMADD_RICHEDIT3;
  TSMADD_RICHEDIT3 = record
    cbSize             :uint;       // size of the structure
    hwndRichEditControl:HWND;       // handle to the rich edit control
    rangeToReplace     :PCHARRANGE; // same meaning as for normal Richedit use (NULL = replaceall)
    Protocolname       :PAnsiChar;  // protocol to use... if you have defined a
       // protocol, u can use your own protocol name. SmileyAdd will
       // automatically select the smileypack that is defined for your protocol.
       // Or, use "Standard" for standard smiley set. Or "ICQ", "MSN"
       // if you prefer those icons.
       // If not found or NULL, "Standard" will be used
    flags              :uint;       // Flags (SAFLRE_*) that define the behaivior
    disableRedraw      :bool;       // Parameter have been depricated, have no effect on operation
    hContact           :TMCONTACT;  // Contact handle
  end;

{
  Replace smileys in a rich edit control...
  wParam = (WPARAM) 0; not used
  lParam = (LPARAM) (SMADD_RICHEDIT3*) &smre;  //pointer to SMADD_RICHEDIT3
  return: TRUE if API succeeded (all parameters were valid) , FALSE if not.
}
const
  MS_SMILEYADD_REPLACESMILEYS:PAnsiChar = 'SmileyAdd/ReplaceSmileys';

type
  PSMADD_SHOWSEL3 = ^TSMADD_SHOWSEL3;
  TSMADD_SHOWSEL3 = record
    cbSize       : uint;      // size of the structure
    Protocolname : PAnsiChar; // protocol to use... if you have defined a protocol,
      // you can use your own protocol name. Smiley add will automatically
      // select the smileypack that is defined for your protocol.
      // Or, use "Standard" for standard smiley set. Or "ICQ", "MSN"
      // if you prefer those icons.
      // If not found or NULL: "Standard" will be used
    xPosition    : int;       // Postition to place the selectwindow
    yPosition    : int;
    Direction    : int;       // Direction (i.e. size upwards/downwards/etc) of the window 0, 1, 2, 3
    hwndTarget   : HWND;      // Window, where to send the message when smiley is selected.
    targetMessage: uint;      // Target message, to be sent.
    targetWParam : LPARAM;    // Target WParam to be sent (LParam will be PAnsiChar to
                              // select smiley) see the example file.
    hwndParent   : HWND;      // Parent window for smiley dialog
    hContact     : TMCONTACT; // Contact handle
  end;

{
  Show smiley selection window
  wParam = (WPARAM) 0; not used
  lParam = (LPARAM) (SMADD_SHOWSEL3*) &smre;  //pointer to SMADD_SHOWSEL3
  return: TRUE if API succeeded (all parameters were valid) , FALSE if not.
}
const
  MS_SMILEYADD_SHOWSELECTION:PAnsiChar = 'SmileyAdd/ShowSmileySelection';

{
  get smiley button icon
  wParam = (WPARAM) 0; not used
  lParam = (LPARAM) (SMADD_INFO*) &smgi;  //pointer to SMADD_INFO
}
type
  PSMADD_INFO2 = ^TSMADD_INFO2;
  TSMADD_INFO2 = record
    cbSize                : int;       // size of the structure
    Protocolname          : PAnsiChar;
    ButtonIcon            : HICON;     // RETURN VALUE: this is filled with the
     // icon handle of the smiley that can be used on the button
     // do not destroy! NULL if the buttonicon is not defined...
    NumberOfVisibleSmileys: int;       // Number of visible smileys defined.
    NumberOfSmileys       : int;       // Number of total smileys defined
    hContact              : TMCONTACT; //Contact handle
  end;

const
  MS_SMILEYADD_GETINFO2:PAnsiChar = 'SmileyAdd/GetInfo2';

// Event notifies that options have changed
// Message dialogs usually need to redraw it's content on reception of this event
  ME_SMILEYADD_OPTIONSCHANGED:PAnsiChar = 'SmileyAdd/OptionsChanged';

  SAFL_PATH     = 1;  // provide smiley file path, icon otherwise
  SAFL_UNICODE  = 2;  // string fields in OPTIONSDIALOGPAGE are WCHAR*
  SAFL_OUTGOING = 4;  // Parsing outgoing message
  SAFL_NOCUSTOM = 8;  // Do not use custom smileys

type
  PSMADD_BATCHPARSE2 = ^TSMADD_BATCHPARSE2;
  TSMADD_BATCHPARSE2 = record
    cbSize      :uint;      // size of the structure
    Protocolname:PAnsiChar; // protocol to use... if you have defined a protocol, u can
                            // use your own protocol name. Smiley add wil automatically
                            // select the smileypack that is defined for your protocol.
                            // Or, use "Standard" for standard smiley set. Or "ICQ", "MSN"
                            // if you prefer those icons.
                            // If not found or NULL: "Standard" will be used
    str         :TChar;     // String to parse
    flag        :uint;      // One of the SAFL_ flags specifies parsing requirements
                            // This parameter should be filled by the user
    numSmileys  :uint;      // Number of Smileys found, this parameter filled by SmileyAdd
    oflag       :uint;      // One of the SAFL_ flags specifies content of the parse results
                            // this parameter filled by SmileyAdd
    hContact    :TMCONTACT; // Contact handle
  end;

type
  PSMADD_BATCHPARSERES = ^TSMADD_BATCHPARSERES;
  TSMADD_BATCHPARSERES = record
    startChar:uint; // Starting smiley character
                    // Because of iterative nature of the API caller should
                    // set this parameter to correct value
    size     :uint; // Number of characters in smiley (0 if not found)
                    // Because of iterative nature of the API caller should
                    // set this parameter to correct value
    case boolean of
      false: (filepath:TChar);
      true:  (hIcon:HICON); // User responsible for destroying icon handle
  end;
const
//find all smileys in text, API parses the provided text and returns all smileys found
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_BATCHPARSE2*) &smgp;  //pointer to SMADD_BATCHPARSE2
//function returns pointer to array SMADD_BATCHPARSERES records for each smiley found
//if no smileys found NULL is returned
//if non NULL value returned pointer must be freed with MS_SMILEYADD_BATCHFREE API
  MS_SMILEYADD_BATCHPARSE:PAnsiChar = 'SmileyAdd/BatchParse';

//Free memory allocated by MS_SMILEYADD_BATCHPARSE
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_BATCHPARSERES*) &smgp;  //pointer to SMADD_BATCHPARSERES
  MS_SMILEYADD_BATCHFREE:PAnsiChar = 'SmileyAdd/BatchFree';

type
  PSMADD_REGCAT = ^TSMADD_REGCAT;
  TSMADD_REGCAT = record
    cbSize  :uint;      // size of the structure
    name    :PAnsiChar; // smiley category name for reference
    dispname:PAnsiChar; // smiley category name for display
  end;

const
//Register smiley category
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_REGCAT*) &smgp;  //pointer to SMADD_REGCAT
  MS_SMILEYADD_REGISTERCATEGORY:PAnsiChar = 'SmileyAdd/RegisterCategory';

//Register smiley category
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) Pointer to protocol name or NULL for all;
  MS_SMILEYADD_RELOAD:PAnsiChar = 'SmileyAdd/Reload';

{
 NM_FIREVIEWCHANGE is WM_NOTIFY Message for notify parent of host window about smiley are going to be repaint

 The proposed action is next: Owner of RichEdit windows received NM_FIREVIEWCHANGE through WM_NOTIFY
 twice first time before painting|invalidating (FVCN_PREFIRE) and second time - after (FVCN_POSTFIRE).
 The Owner window may change any values of received FVCNDATA_NMHDR structure in order to raise needed action.
 For example it may substitute FVCA_INVALIDATE to FVCA_CUSTOMDRAW event to force painting on self offscreen context.
 
 It can be:
 FVCA_CUSTOMDRAW - in this case you need to provide valid HDC to draw on and valid RECT of smiley
 FVCA_INVALIDATE - to invalidate specified rect of window 
 FVCA_NONE    - skip any action. But be aware - animation will be stopped till next repainting of smiley.
 FVCA_SENDVIEWCHANGE - to notify richedit ole about object changed. Be aware Richedit will fully reconstruct itself

 Another point is moment of received smiley rect - it is only valid if FVCA_DRAW is initially set, 
 and it is PROBABLY valid if FVCA_INVALIDATE is set. And it most probably invalid in case of FVCA_SENDVIEWCHANGE.
 The smiley position is relative last full paint HDC. Usually it is relative to top-left corner of host 
 richedit (NOT it client area) in windows coordinates. 
}

const
// Type of Event one of
  FVCN_PREFIRE    = 1;
  FVCN_POSTFIRE   = 2;

// Action of event are going to be done
  FVCA_NONE       = 0;
  FVCA_DRAW       = 1;   // do not modify hdc in case of _DRAW, Use _CUSTOMDRAW
  FVCA_CUSTOMDRAW = 2;
//  FVCA_INVALIDATE     = 3; // (not supported)
//  FVCA_SENDVIEWCHANGE = 4; // (not supported)
  FVCA_SKIPDRAW   = 5;

// Extended NMHDR structure for WM_NOTIFY
type
  PFVCNDATA_NMHDR = ^TFVCNDATA_NMHDR;
  TFVCNDATA_NMHDR = record
    //NMHDR structure
    hwndFrom     :HWND;     // Window of smiley host
    idFrom       :uint_ptr; // ignored
    code         :uint;     // NM_FIREVIEWCHANGE
    
    cbSize       :size_t;
    bEvent       :byte;     // FVCN_ value - pre- or post- painting
    bAction      :byte;     // FVCA_ keys
    hDC          :HDC;      // Canvas to draw on
    rcRect       :TRECT;    // Valid/should be in case of FVCA_DRAW
    clrBackground:COLORREF; // color to fill background if fTransparent is not set
    fTransparent :bool;     // if need to fill back color (not supported)
    lParam       :LPARAM;   // used by host window PreFire and PostFire event
  end;

const
// Code of WM_NOTIFY message (code)
  NM_FIREVIEWCHANGE =  NM_FIRST+1;

type
  TSMADD_CONT = record
    cbSize  :uint;
    hContact:TMCONTACT;
    _type   :int;       // 0 - directory, 1 - file;
    path    :TChar;     // smiley category name for reference
  end;
const
//Loads all smileys for the contact
//wParam = (WPARAM) 0; not used
//lParam = (LPARAM) (SMADD_CONT*) &dir;  // pointer to directory to load smiley from
  MS_SMILEYADD_LOADCONTACTSMILEYS:PAnsiChar = 'SmileyAdd/LoadContactSmileys';

{$ENDIF}
