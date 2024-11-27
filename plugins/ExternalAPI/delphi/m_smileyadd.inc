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
    hwndRichEditControl:HWND;       // handle to the rich edit control
    rangeToReplace     :PCHARRANGE; // same meaning as for normal Richedit use (NULL = replaceall)
    Protocolname       :PAnsiChar;  // protocol to use... if you have defined a
       // protocol, u can use your own protocol name. SmileyAdd will
       // automatically select the smileypack that is defined for your protocol.
       // Or, use "Standard" for standard smiley set. Or "ICQ"
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

// Event notifies that options have changed
// Message dialogs usually need to redraw it's content on reception of this event
  ME_SMILEYADD_OPTIONSCHANGED:PAnsiChar = 'SmileyAdd/OptionsChanged';

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

{$ENDIF}
