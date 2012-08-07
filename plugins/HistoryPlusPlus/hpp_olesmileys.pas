(*
    History++ plugin for Miranda IM: the free IM client for Microsoft* Windows*

    Copyright (C) 2006-2009 theMIROn, 2003-2006 Art Fedorov.
    History+ parts (C) 2001 Christian Kastner

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*)

{-----------------------------------------------------------------------------
 hpp_olesmileys (historypp project)

 Version:   1.5
 Created:   04.02.2007
 Author:    theMIROn

 [ Description ]

 [ History ]

 1.5 (04.02.2007)
   First version

 [ Modifications ]
 none

 [ Known Issues ]
 none

 Contributors: theMIROn
-----------------------------------------------------------------------------}

unit hpp_olesmileys;

interface

uses Windows,CommCtrl,ActiveX;

const
  IID_ITooltipData:     TGUID = '{58B32D03-1BD2-4840-992E-9AE799FD4ADE}';
  IID_IGifSmileyCtrl:   TGUID = '{CB64102B-8CE4-4A55-B050-131C435A3A3F}';
  IID_ISmileyAddSmiley: TGUID = '{105C56DF-6455-4705-A501-51F1CCFCF688}';
  IID_IEmoticonsImage:  TGUID = '{2FD9449B-7EBB-476a-A9DD-AE61382CCE08}';

type
  ITooltipData = interface(IUnknown)
   ['{58B32D03-1BD2-4840-992E-9AE799FD4ADE}']
   function SetTooltip(const bstrHint: WideString): HRESULT; stdcall;
   function GetTooltip(out bstrHint: WideString): HRESULT; stdcall;
  end;

  IGifSmileyCtrl = interface(IUnknown)
    ['{CB64102B-8CE4-4A55-B050-131C435A3A3F}']
  end;

  ISmileyAddSmiley = interface(IUnknown)
    ['{105C56DF-6455-4705-A501-51F1CCFCF688}']
  end;

  IEmoticonsImage = interface(IUnknown)
    ['{2FD9449B-7EBB-476a-A9DD-AE61382CCE08}']
  end;

(*
	NM_FIREVIEWCHANGE is WM_NOTIFY Message for notify parent of host window about smiley are going to be repaint

	The proposed action is next: Owner of RichEdit windows received NM_FIREVIEWCHANGE through WM_NOTIFY
	twice first time before painting|invalidating (FVCN_PREFIRE) and second time - after (FVCN_POSTFIRE).
	The Owner window may change any values of received FVCNDATA_NMHDR structure in order to raise needed action.
	For example it may substitute FVCA_INVALIDATE to FVCA_CUSTOMDRAW event to force painting on self offscreen context.
	
	It can be:
	FVCA_CUSTOMDRAW - in this case you need to provide valid HDC to draw on and valid RECT of smiley
	FVCA_INVALIDATE - to invalidate specified rect of window 
	FVCA_NONE		- skip any action. But be aware - animation will be stopped till next repainting of smiley.
	FVCA_SENDVIEWCHANGE - to notify richedit ole about object changed. Be aware Richedit will fully reconstruct itself

	Another point is moment of received smiley rect - it is only valid if FVCA_DRAW is initially set, 
	and it is PROBABLY valid if FVCA_INVALIDATE is set. And it most probably invalid in case of FVCA_SENDVIEWCHANGE.
	The smiley position is relative last full paint HDC. Usually it is relative to top-left corner of host 
	richedit (NOT it client area) in windows coordinates. 

*)

const // additional for animated smileys (idk where introduced)
    // Type of Event one of
    FVCN_GETINFO  = 255;
    // Action of event are going to be done
    FVCA_INFO           = 255;

implementation

end.
