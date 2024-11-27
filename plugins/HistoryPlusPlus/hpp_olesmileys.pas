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

 1.5 (04.02.2007)
   First version

 Contributors: theMIROn
-----------------------------------------------------------------------------}

unit hpp_olesmileys;

interface

uses Windows,CommCtrl,ActiveX;

const
  IID_ITooltipData:     TGUID = '{58B32D03-1BD2-4840-992E-9AE799FD4ADE}';

type
  ITooltipData = interface(IUnknown)
   ['{58B32D03-1BD2-4840-992E-9AE799FD4ADE}']
   function SetTooltip(const bstrHint: WideString): HRESULT; stdcall;
   function GetTooltip(out bstrHint: WideString): HRESULT; stdcall;
  end;

implementation

end.
